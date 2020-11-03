// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file PDPServer2.cpp
 *
 */

#include <iostream>
#include <fstream>
#include <mutex>

#include <fastrtps/utils/TimedMutex.hpp>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/History.h>

#include <fastrtps/utils/TimeConversion.h>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>

#include "./PDPServer2.hpp"
#include "./PDPServerListener2.hpp"
#include "./DServerEvent2.hpp"
#include "../endpoint/EDPServer2.hpp"
#include "../endpoint/EDPServerListeners2.hpp"

#include "../database/backup/SharedBackupFunctions.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPServer2::PDPServer2(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation,
        DurabilityKind_t durability_kind /* TRANSIENT_LOCAL */)
    : PDP(builtin, allocation)
    , routine_(nullptr)
    , ping_(nullptr)
    , discovery_db_(builtin->mp_participantImpl->getGuid().guidPrefix,
            servers_prefixes())
    , _durability (durability_kind)
{
}

PDPServer2::~PDPServer2()
{
    // Stop timed events
    routine_->cancel_timer();
    ping_->cancel_timer();

    // Disable database
    discovery_db_.disable();

    // Delete timed events
    delete(routine_);
    delete(ping_);

    // Clear ddb and release its changes
    process_changes_release_(discovery_db_.clear());
}

bool PDPServer2::init(
        RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    mp_EDP = new EDPServer2(this, mp_RTPSParticipant, _durability);
    if (!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP_SERVER, "Endpoint discovery configuration failed");
        return false;
    }

    std::vector<nlohmann::json> backup_queue;
    if (_durability == TRANSIENT)
    {
        nlohmann::json backup_json;
        // if the DS is BACKUP, try to restore DDB from file
        discovery_db().backup_in_progress(true);
        if (read_backup(backup_json, backup_queue))
        {
            if (process_discovery_database_restore_(backup_json))
            {
                logInfo(RTPS_PDP_SERVER, "DiscoveryDataBase restored correctly");
            }
        }
        else
        {
            logInfo(RTPS_PDP_SERVER, "Error reading backup file. Corrupted or unmissing file, restarting from scratch");
        }

        discovery_db().backup_in_progress(false);

        discovery_db_.persistence_enable(get_ddb_queue_persistence_file_name());

        restore_queue(backup_queue);
    }
    else
    {
        // Allows the ddb to process new messages from this point
        discovery_db_.enable();
    }

    // Activate listeners
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    getRTPSParticipant()->enableReader(mp_PDPReader);
    getRTPSParticipant()->enableReader(edp->subscriptions_reader_.first);
    getRTPSParticipant()->enableReader(edp->publications_reader_.first);

    // Initialize server dedicated thread.
    resource_event_thread_.init_thread();

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
     */
    routine_ = new DServerRoutineEvent2(this,
                    TimeConv::Duration_t2MilliSecondsDouble(
                        m_discovery.discovery_config.discoveryServer_client_syncperiod));

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
     */
    ping_ = new DServerPingEvent2(this,
                    TimeConv::Duration_t2MilliSecondsDouble(
                        m_discovery.discovery_config.discoveryServer_client_syncperiod));
    ping_->restart_timer();

    return true;
}

ParticipantProxyData* PDPServer2::createParticipantProxyData(
        const ParticipantProxyData& participant_data,
        const GUID_t& writer_guid)
{
    std::lock_guard<std::recursive_mutex> lock(*getMutex());

    // lease duration is controlled for owned clients or linked servers
    // other clients liveliness is provided through server's PDP discovery data

    // check if the DATA msg is relayed by another server
    bool do_lease = participant_data.m_guid.guidPrefix == writer_guid.guidPrefix;

    if (!do_lease)
    {
        // if not a client verify this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == participant_data.m_guid.guidPrefix)
            {
                do_lease = true;
            }
        }
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.m_guid, do_lease);
    if (pdata != nullptr)
    {
        pdata->copy(participant_data);
        pdata->isAlive = true;
        if (do_lease)
        {
            pdata->lease_duration_event->update_interval(pdata->m_leaseDuration);
            pdata->lease_duration_event->restart_timer();
        }
    }

    return pdata;
}

bool PDPServer2::createPDPEndpoints()
{
    logInfo(RTPS_PDP_SERVER, "Beginning PDPServer Endpoints creation");

    const NetworkFactory& network = mp_RTPSParticipant->network_factory();

    /***********************************
    * PDP READER
    ***********************************/
    // PDP Reader History
    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    mp_PDPReaderHistory = new ReaderHistory(hatt);

    // PDP Reader Attributes
    ReaderAttributes ratt;
    ratt.expectsInlineQos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    // change depending of backup mode
    ratt.endpoint.durabilityKind = _durability;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.times.heartbeatResponseDelay = pdp_heartbeat_response_delay;

#if HAVE_SQLITE3
    ratt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    ratt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            get_reader_persistence_file_name()));
#endif // HAVE_SQLITE3

    // PDP Listener
    mp_listener = new PDPServerListener2(this);

    // Create PDP Reader
    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory,
            mp_listener, c_EntityId_SPDPReader, true, false))
    {
        // Enable unknown clients to reach this reader
        mp_PDPReader->enableMessagesFromUnkownWriters(true);

        // Initial peer list doesn't make sense in server scenario. Client should match its server list
        for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_writer_data_.clear();
            temp_writer_data_.guid(it.GetPDPWriter());
            temp_writer_data_.set_multicast_locators(it.metatrafficMulticastLocatorList, network);
            temp_writer_data_.set_remote_unicast_locators(it.metatrafficUnicastLocatorList, network);
            // TODO check if this is correct, it is equal as PDPServer, but we do not know like this the durKind of the
            // other server
            temp_writer_data_.m_qos.m_durability.durabilityKind(_durability);
            temp_writer_data_.m_qos.m_reliability.kind = fastrtps::RELIABLE_RELIABILITY_QOS;

            mp_PDPReader->matched_writer_add(temp_writer_data_);
        }
    }
    // Could not create PDP Reader, so return false
    else
    {
        logError(RTPS_PDP_SERVER, "PDPServer Reader creation failed");
        delete(mp_PDPReaderHistory);
        mp_PDPReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    /***********************************
    * PDP WRITER
    ***********************************/

    // PDP Writer History
    hatt.payloadMaxSize = mp_builtin->m_att.writerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_PDPWriterHistory = new WriterHistory(hatt);

    // PDP Writer Attributes
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    // VOLATILE durability to highlight that on steady state the history is empty (except for announcement DATAs)
    // this setting is incompatible with CLIENTs TRANSIENT_LOCAL PDP readers but not validation is done on builitin
    // endpoints
    watt.endpoint.durabilityKind = _durability;

#if HAVE_SQLITE3
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            get_writer_persistence_file_name()));
#endif // HAVE_SQLITE3

    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    watt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    watt.times.heartbeatPeriod = pdp_heartbeat_period;
    watt.times.nackResponseDelay = pdp_nack_response_delay;
    watt.times.nackSupressionDuration = pdp_nack_supression_duration;
    watt.mode = ASYNCHRONOUS_WRITER;

    // Create PDP Writer
    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory,
            nullptr, c_EntityId_SPDPWriter, true))
    {
        // Set pdp filter to writer
        IReaderDataFilter* pdp_filter = static_cast<ddb::PDPDataFilter<ddb::DiscoveryDataBase>*>(&discovery_db_);
        static_cast<StatefulWriter*>(mp_PDPWriter)->reader_data_filter(pdp_filter);
        // Enable separate sending so the filter can be called for each change and reader proxy
        mp_PDPWriter->set_separate_sending(true);

        for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_reader_data_.clear();
            temp_reader_data_.guid(it.GetPDPReader());
            temp_reader_data_.set_multicast_locators(it.metatrafficMulticastLocatorList, network);
            temp_reader_data_.set_remote_unicast_locators(it.metatrafficUnicastLocatorList, network);
            temp_reader_data_.m_qos.m_durability.kind = fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
            temp_reader_data_.m_qos.m_reliability.kind = fastrtps::RELIABLE_RELIABILITY_QOS;

            mp_PDPWriter->matched_reader_add(temp_reader_data_);
        }
    }
    // Could not create PDP Writer, so return false
    else
    {
        logError(RTPS_PDP_SERVER, "PDPServer Writer creation failed");
        delete(mp_PDPWriterHistory);
        mp_PDPWriterHistory = nullptr;
        return false;
    }
    // TODO check if this should be done here or before this point in creation
    mp_PDPWriterHistory->remove_all_changes();

    logInfo(RTPS_PDP_SERVER, "PDPServer Endpoints creation finished");
    return true;
}

void PDPServer2::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data);

    if (!(getRTPSParticipant()->getAttributes().builtin.discovery_config.discoveryProtocol !=
            DiscoveryProtocol_t::CLIENT))
    {
        logError(RTPS_PDP_SERVER, "Using a PDP Server object with another user's settings");
    }

    // A PDP server should always be provided with all EDP endpoints
    // because it must relay all clients EDP info
    participant_data->m_availableBuiltinEndpoints
        |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER
            | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR
            | DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR
            | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;

    const SimpleEDPAttributes& se = getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP;

    if (!(se.use_PublicationWriterANDSubscriptionReader && se.use_PublicationReaderANDSubscriptionWriter))
    {
        logWarning(RTPS_PDP_SERVER, "SERVER or BACKUP PDP requires always all EDP endpoints creation.");
    }

    // Set participant type and discovery server version properties
    participant_data->m_properties.push_back(
        std::pair<std::string, std::string>(
            {dds::parameter_property_participant_type, ParticipantType::SERVER}));
    participant_data->m_properties.push_back(
        std::pair<std::string,
        std::string>({dds::parameter_property_ds_version, dds::parameter_property_current_ds_version}));
}

void PDPServer2::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP_SERVER, "Assigning remote endpoint for RTPSParticipant: " << pdata->m_guid.guidPrefix);

    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    bool use_multicast_locators = !mp_RTPSParticipant->getAttributes().builtin.avoid_builtin_multicast ||
            pdata->metatraffic_locators.unicast.empty();

    // only SERVER and CLIENT participants will be received. All builtin must be there
    uint32_t auxendp = endp & DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if (0 != auxendp)
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        temp_writer_data_.clear();
        temp_writer_data_.guid().guidPrefix = pdata->m_guid.guidPrefix;
        temp_writer_data_.guid().entityId = c_EntityId_SPDPWriter;
        temp_writer_data_.persistence_guid(pdata->get_persistence_guid());
        temp_writer_data_.set_persistence_entity_id(c_EntityId_SPDPWriter);
        temp_writer_data_.set_remote_locators(pdata->metatraffic_locators, network, use_multicast_locators);
        temp_writer_data_.m_qos.m_reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
        temp_writer_data_.m_qos.m_durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_PDPReader->matched_writer_add(temp_writer_data_);
    }
    else
    {
        logError(RTPS_PDP_SERVER, "Participant " << pdata->m_guid.guidPrefix
                                                 << " did not send information about builtin writers");
        return;
    }

    // only SERVER and CLIENT participants will be received. All builtin must be there
    auxendp = endp & DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if (0 != auxendp)
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        temp_reader_data_.clear();
        temp_reader_data_.m_expectsInlineQos = false;
        temp_reader_data_.guid().guidPrefix = pdata->m_guid.guidPrefix;
        temp_reader_data_.guid().entityId = c_EntityId_SPDPReader;
        temp_reader_data_.set_remote_locators(pdata->metatraffic_locators, network, use_multicast_locators);
        temp_reader_data_.m_qos.m_reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
        temp_reader_data_.m_qos.m_durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_PDPWriter->matched_reader_add(temp_reader_data_);
    }
    else
    {
        logError(RTPS_PDP_SERVER, "Participant " << pdata->m_guid.guidPrefix
                                                 << " did not send information about builtin readers");
        return;
    }

    //Inform EDP of new RTPSParticipant data:
    notifyAboveRemoteEndpoints(*pdata);
}

void PDPServer2::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata)
{
    //Inform EDP of new RTPSParticipant data:
    if (mp_EDP != nullptr)
    {
        mp_EDP->assignRemoteEndpoints(pdata);
    }

    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
    }
}

void PDPServer2::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP_SERVER, "For RTPSParticipant: " << pdata->m_guid);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;

    if (endp & DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER)
    {
        GUID_t writer_guid(pdata->m_guid.guidPrefix, c_EntityId_SPDPWriter);
        mp_PDPReader->matched_writer_remove(writer_guid);
    }
    else
    {
        logError(RTPS_PDP_SERVER, "Participant " << pdata->m_guid.guidPrefix
                                                 << " did not send information about builtin writers");
        return;
    }

    if (endp & DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR)
    {
        GUID_t reader_guid(pdata->m_guid.guidPrefix, c_EntityId_SPDPReader);
        mp_PDPWriter->matched_reader_remove(reader_guid);
    }
    else
    {
        logError(RTPS_PDP_SERVER, "Participant " << pdata->m_guid.guidPrefix
                                                 << " did not send information about builtin readers");
        return;
    }
}

std::ostringstream PDPServer2::get_persistence_file_name_() const
{
    assert(getRTPSParticipant());

    std::ostringstream filename(std::ios_base::ate);
    std::string prefix;

    // . is not suitable separator for filenames
    filename << "server-" << getRTPSParticipant()->getGuid().guidPrefix;
    prefix = filename.str();
    std::replace(prefix.begin(), prefix.end(), '.', '-');
    filename.str(std::move(prefix));
    //filename << ".json";

    return filename;
}

std::string PDPServer2::get_writer_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << "_writer.db";
    return filename.str();
}

std::string PDPServer2::get_reader_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << "_reader.db";
    return filename.str();
}

std::string PDPServer2::get_ddb_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << ".json";
    return filename.str();
}

std::string PDPServer2::get_ddb_queue_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << "_queue.json";
    return filename.str();
}

void PDPServer2::announceParticipantState(
        bool new_change,
        bool dispose /* = false */,
        WriteParams& )
{
    logInfo(RTPS_PDP_SERVER, "Announcing Server " << mp_RTPSParticipant->getGuid() << " (new change: " << new_change << ")");
    CacheChange_t* change = nullptr;

    StatefulWriter* pW = dynamic_cast<StatefulWriter*>(mp_PDPWriter);
    assert(pW);

    /*
       Protect writer sequence number. Make sure in order to prevent AB BA deadlock that the
       writer mutex is systematically lock before the PDP one (if needed):
        - transport callbacks on PDPListener
        - initialization and removal on BuiltinProtocols::initBuiltinProtocols and ~BuiltinProtocols
        - DSClientEvent (own thread)
        - ResendParticipantProxyDataPeriod (participant event thread)
     */
    std::lock_guard<fastrtps::RecursiveTimedMutex> wlock(pW->getMutex());

    if (!dispose)
    {
        // Create the CacheChange_t if necessary
        if (m_hasChangedLocalPDP.exchange(false) || new_change)
        {
            getMutex()->lock();

            // Copy the participant data
            ParticipantProxyData proxy_data_copy(*getLocalParticipantProxyData());

            // Prepare identity
            WriteParams wp;
            SequenceNumber_t sn = mp_PDPWriterHistory->next_sequence_number();
            {
                SampleIdentity local;
                local.writer_guid(mp_PDPWriter->getGuid());
                local.sequence_number(sn);
                wp.sample_identity(local);
                wp.related_sample_identity(local);
            }

            getMutex()->unlock();

            uint32_t cdr_size = proxy_data_copy.get_serialized_size(true);
            change = mp_PDPWriter->new_change(
                [cdr_size]() -> uint32_t
                {
                    return cdr_size;
                },
                ALIVE, proxy_data_copy.m_key);

            if (change != nullptr)
            {
                CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
                aux_msg.msg_endian = BIGEND;
#else
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
                aux_msg.msg_endian =  LITTLEEND;
#endif // if __BIG_ENDIAN__

                if (proxy_data_copy.writeToCDRMessage(&aux_msg, true))
                {
                    change->serializedPayload.length = (uint16_t)aux_msg.length;
                }
                else
                {
                    logError(RTPS_PDP_SERVER, "Cannot serialize ParticipantProxyData.");
                    return;
                }

                // assign identity
                change->sequenceNumber = sn;

                // Create a RemoteLocatorList for metatraffic_locators
                fastrtps::rtps::RemoteLocatorList metatraffic_locators(
                    mp_builtin->m_metatrafficUnicastLocatorList.size(),
                    mp_builtin->m_metatrafficMulticastLocatorList.size());

                // Populate with server's unicast locators
                for (auto locator : mp_builtin->m_metatrafficUnicastLocatorList)
                {
                    metatraffic_locators.add_unicast_locator(locator);
                }
                // Populate with server's multicast locators
                for (auto locator : mp_builtin->m_metatrafficMulticastLocatorList)
                {
                    metatraffic_locators.add_multicast_locator(locator);
                }

                // If the DATA is already in the writer's history, then remove it, but do not release the change.
                remove_change_from_history_nts(mp_PDPWriterHistory, change, false);

                // Add our change to PDPWriterHistory
                mp_PDPWriterHistory->add_change(change, wp);
                change->write_params = wp;

                // Update the database with our own data
                if (discovery_db().update(
                            change,
                            ddb::DiscoveryParticipantChangeData(metatraffic_locators, false, true)))
                {
                    // Distribute
                    awake_routine_thread();
                }
                else
                {
                    // Already there, dispose
                    logError(RTPS_PDP_SERVER, "DiscoveryDatabase already initialized with local DATA(p) on creation");
                    mp_PDPWriterHistory->release_Cache(change);
                }
            }
            // Doesn't make sense to send the DATA directly if it hasn't been introduced in the history yet (missing
            // sequence number.
            return;
        }
        else
        {
            // Retrieve the CacheChange_t from the database
            change = discovery_db().cache_change_own_participant();
            if (nullptr == change)
            {
                logError(RTPS_PDP_SERVER, "DiscoveryDatabase uninitialized with local DATA(p) on announcement");
                return;
            }
        }
    }
    else
    {
        getMutex()->lock();

        // Copy the participant data
        ParticipantProxyData* local_participant = getLocalParticipantProxyData();
        InstanceHandle_t key = local_participant->m_key;
        uint32_t cdr_size = local_participant->get_serialized_size(true);
        local_participant = nullptr;

        // Prepare identity
        WriteParams wp;
        SequenceNumber_t sn = mp_PDPWriterHistory->next_sequence_number();
        {
            SampleIdentity local;
            local.writer_guid(mp_PDPWriter->getGuid());
            local.sequence_number(sn);
            wp.sample_identity(local);
            wp.related_sample_identity(local);
        }

        getMutex()->unlock();

        change = pW->new_change(
            [cdr_size]() -> uint32_t
            {
                return cdr_size;
            },
            NOT_ALIVE_DISPOSED_UNREGISTERED, key);

        // Generate the Data(Up)
        if (nullptr != change)
        {
            // assign identity
            change->sequenceNumber = sn;
            change->write_params = std::move(wp);

            // Update the database with our own data
            if (discovery_db().update(change, ddb::DiscoveryParticipantChangeData()))
            {
                // distribute
                awake_routine_thread();
            }
            else
            {
                // already there, dispose. If participant is not removed fast enought may happen
                mp_PDPWriterHistory->release_Cache(change);
                return;
            }
        }
        else
        {
            // failed to create the disposal change
            logError(RTPS_PDP_SERVER, "Server failed to create its DATA(Up)");
            return;
        }
    }

    assert(nullptr != change);

    // Force send the announcement

    // Create a list of receivers based on the remote participants known by the discovery database that are direct
    // clients or servers of this server. Add the locators of those remote participants.
    std::vector<GUID_t> remote_readers;
    LocatorList_t locators;

    std::vector<GuidPrefix_t> direct_clients_and_servers = discovery_db_.direct_clients_and_servers();
    for (GuidPrefix_t participant_prefix: direct_clients_and_servers)
    {
        // Add remote reader
        GUID_t remote_guid(participant_prefix, c_EntityId_SPDPReader);
        remote_readers.push_back(remote_guid);

        locators.push_back(discovery_db_.participant_metatraffic_locators(participant_prefix));
    }
    send_announcement(change, remote_readers, locators, dispose);
}

/**
 * This method removes a remote RTPSParticipant and all its writers and readers.
 * @param participant_guid GUID_t of the remote RTPSParticipant.
 * @param reason Why the participant is being removed (dropped vs removed)
 * @return true if correct.
 */
bool PDPServer2::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
{
    // Notify the DiscoveryDataBase on lease duration removal because the listener
    // has already notified the database in all other cases
    if (ParticipantDiscoveryInfo::DROPPED_PARTICIPANT == reason)
    {
        CacheChange_t* pC = nullptr;

        // We must create the corresponding DATA(p[UD])
        if ((pC = mp_PDPWriter->new_change(
                    [this]() -> uint32_t
                    {
                        return mp_builtin->m_att.writerPayloadSize;
                    },
                    NOT_ALIVE_DISPOSED_UNREGISTERED, partGUID)))
        {
            // Use this server identity in order to hint clients it's a lease duration demise
            WriteParams& wp = pC->write_params;
            SampleIdentity local;
            local.writer_guid(mp_PDPWriter->getGuid());
            local.sequence_number(mp_PDPWriterHistory->next_sequence_number());
            wp.sample_identity(local);
            wp.related_sample_identity(local);

            // Notify the database
            if (discovery_db_.update(pC, ddb::DiscoveryParticipantChangeData()))
            {
                // assure processing time for the cache
                awake_routine_thread();

                // the discovery database takes ownership of the CacheChange_t
                // henceforth there are no references to the CacheChange_t
            }
            else
            {
                // if the database doesn't take the ownership remove
                mp_PDPWriterHistory->release_Cache(pC);
            }
        }
    }

    // check if is a server who has been disposed
    awake_server_thread();

    // delegate into the base class for inherited proxy database removal
    return PDP::remove_remote_participant(partGUID, reason);
}

bool PDPServer2::process_data_queues()
{
    logInfo(RTPS_PDP_SERVER, "process_data_queues start");
    discovery_db_.process_pdp_data_queue();
    return discovery_db_.process_edp_data_queue();
}

void PDPServer2::awake_routine_thread(
        double interval_ms /*= 0*/)
{
    routine_->update_interval_millisec(interval_ms);
    routine_->cancel_timer();
    routine_->restart_timer();
}

void PDPServer2::awake_server_thread()
{
    ping_->restart_timer();
}

bool PDPServer2::server_update_routine()
{
    // There is pending work to be done by the server if there are changes that have not been acknowledged.
    bool pending_work = true;

    // Must lock the mutes to unlock it in the loop
    discovery_db().lock_incoming_data();

    // Execute the server routine
    do
    {
        discovery_db().unlock_incoming_data();

        logInfo(RTPS_PDP_SERVER, "");
        logInfo(RTPS_PDP_SERVER, "-------------------- Server routine start --------------------");
        logInfo(RTPS_PDP_SERVER, "-------------------- " << mp_RTPSParticipant->getGuid() << " --------------------");

        process_writers_acknowledgements();     // server + ddb(functor_with_ddb)
        process_data_queues();                  // all ddb
        process_dirty_topics();                 // all ddb
        process_changes_release();              // server + ddb(changes_to_release(), clear_changes_to_release())
        process_disposals();                    // server + ddb(changes_to_dispose(), clear_changes_to_disposes())
        process_to_send_lists();                // server + ddb(get_to_send, remove_to_send_this)
        pending_work = pending_ack();           // all server

        logInfo(RTPS_PDP_SERVER, "-------------------- " << mp_RTPSParticipant->getGuid() << " --------------------");
        logInfo(RTPS_PDP_SERVER, "-------------------- Server routine end --------------------");
        logInfo(RTPS_PDP_SERVER, "");

        // Lock new data to check emptyness and to do the dump of the backup in case it is empty
        discovery_db().lock_incoming_data();
    }
    // If the data queue is not empty re-start the routine.
    // A non-empty queue means that the server has received a change while it is running the processing routine.
    // If not considering disabled and there are changes in queue when disable, it will get in an infinite loop
    while (!discovery_db_.data_queue_empty() && discovery_db_.is_enabled());

    // Must restart the routine after the period time

    // It uses the free time (no messages to process) to save the new state of the ddb
    // This only will be called when:
    // -there are not new data in queue
    // -there has been any modification in the DDB (if not, this routine is not called)
    if (_durability == TRANSIENT && discovery_db_.is_enabled())
    {
        process_ddb_backup();
    }
    // Unlock the incoming data after finishing the backuo storage
    discovery_db().unlock_incoming_data();

    return pending_work && discovery_db_.is_enabled();
}

bool PDPServer2::process_writers_acknowledgements()
{
    logInfo(RTPS_PDP_SERVER, "process_writers_acknowledgements start");

    // Execute first ACK for endpoints because PDP acked changes relevance in EDP,
    //  which can result in false positives in EDP acknowledgements.

    /* EDP Subscriptions Writer's History */
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    bool pending = process_history_acknowledgement(edp->subscriptions_writer_.first, edp->subscriptions_writer_.second);

    /* EDP Publications Writer's History */
    pending |= process_history_acknowledgement(edp->publications_writer_.first, edp->publications_writer_.second);

    /* PDP Writer's History */
    pending |= process_history_acknowledgement(
        static_cast<fastrtps::rtps::StatefulWriter*>(mp_PDPWriter), mp_PDPWriterHistory);

    return pending;
}

bool PDPServer2::process_history_acknowledgement(
        fastrtps::rtps::StatefulWriter* writer,
        fastrtps::rtps::WriterHistory* writer_history)
{
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(writer->getMutex());

    // Iterate over changes in writer's history
    for (auto it = writer_history->changesBegin(); it != writer_history->changesEnd();)
    {
        it = process_change_acknowledgement(
            it,
            writer,
            writer_history);
    }
    return writer_history->getHistorySize() > 1;
}

History::iterator PDPServer2::process_change_acknowledgement(
        fastrtps::rtps::History::iterator cit,
        fastrtps::rtps::StatefulWriter* writer,
        fastrtps::rtps::WriterHistory* writer_history)
{
    // DATA(p|w|r) case
    CacheChange_t* c = *cit;

    if (c->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
    {

        logInfo(RTPS_PDP_SERVER, "Processing ack data alive " << c->instanceHandle);

        // If the change is a DATA(p), and it's the server's DATA(p), and the database knows that
        // it had been acked by all, then skip the change acked check for every reader proxy
        if (discovery_db_.is_participant(c) &&
                discovery_db_.guid_from_change(c) == mp_builtin->mp_participantImpl->getGuid() &&
                discovery_db_.server_acked_by_all())
        {
            logInfo(RTPS_PDP_SERVER, "Server's DATA(p) already acked by all. Skipping check for every ReaderProxy");
        }
        else
        {
            // Call to `StatefulWriter::for_each_reader_proxy()`. This will update
            // `participants_|writers_|readers_[guid_prefix]::relevant_participants_builtin_ack_status`, and will also set
            // `pending` to whether the change is has been acknowledged by all readers.
            if (!writer->for_each_reader_proxy(discovery_db_.functor(c)))
            {
                // in case there is not pending acks for our DATA(p) server, we notify the ddb that it is acked by all
                if (discovery_db_.is_participant(c) &&
                        discovery_db_.guid_from_change(c) == mp_builtin->mp_participantImpl->getGuid())
                {
                    discovery_db_.server_acked_by_all(true);
                }
                else
                {
                    // Remove the entry from writer history, but do not release the cache.
                    // This CacheChange will only be released in the case that is substituted by a DATA(Up|Uw|Ur).
                    return writer_history->remove_change(cit, false);
                }
            }
        }
    }
    // DATA(Up|Uw|Ur) case. Currently, Fast DDS only considers CacheChange kinds ALIVE and NOT_ALIVE_DISPOSED, so if the
    // kind is not ALIVE, then we know is NOT_ALIVE_DISPOSED and so a DATA(Up|Uw|Ur). In the future we should handle the
    // cases where kind is NOT_ALIVE_UNREGISTERED or NOT_ALIVE_DISPOSED_UNREGISTERED.
    else
    {
        // If `StatefulWriter::is_acked_by_all()`:
        if (writer->is_acked_by_all(c))
        {
            // Remove entry from `participants_|writers_|readers_`
            // AND put the change in changes_release
            discovery_db_.delete_entity_of_change(c);
            // Remove from writer's history
            // remove false because the clean of the change is made by release_change of ddb
            return writer_history->remove_change(cit, false);
        }
    }

    // proceed to the next change
    return ++cit;
}

bool PDPServer2::process_disposals()
{
    logInfo(RTPS_PDP_SERVER, "process_disposals start");
    // logInfo(RTPS_PDP_SERVER, "process_disposals start");
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    fastrtps::rtps::WriterHistory* pubs_history = edp->publications_writer_.second;
    fastrtps::rtps::WriterHistory* subs_history = edp->subscriptions_writer_.second;

    // Get list of disposals from database
    std::vector<fastrtps::rtps::CacheChange_t*> disposals = discovery_db_.changes_to_dispose();
    // Iterate over disposals
    for (auto change: disposals)
    {
        logInfo(RTPS_PDP_SERVER, "Process disposal change from: " << change->instanceHandle);
        // No check is performed on whether the change is an actual disposal, leaving the responsability of correctly
        // populating the disposals list to discovery_db_.process_data_queue().

        // Get the identity of the participant from which the change came.
        fastrtps::rtps::GuidPrefix_t change_guid_prefix = discovery_db_.guid_from_change(change).guidPrefix;

        change->writerGUID.guidPrefix = mp_PDPWriter->getGuid().guidPrefix;

        // DATA(Up) case
        if (discovery_db_.is_participant(change))
        {
            // Lock PDP writer
            std::unique_lock<fastrtps::RecursiveTimedMutex> lock(mp_PDPWriter->getMutex());

            // Remove all DATA(p) with the same sample identity as the DATA(Up) from PDP writer's history.
            remove_related_alive_from_history_nts(mp_PDPWriterHistory, change_guid_prefix);

            // Add DATA(Up) to PDP writer's history
            eprosima::fastrtps::rtps::WriteParams wp = change->write_params;
            mp_PDPWriterHistory->add_change(change, wp);
        }
        // DATA(Uw) case
        else if (discovery_db_.is_writer(change))
        {
            // Lock EDP publications writer
            std::unique_lock<fastrtps::RecursiveTimedMutex> lock(edp->publications_writer_.first->getMutex());

            // Remove all DATA(w) with the same sample identity as the DATA(Uw) from EDP publications writer's history
            remove_related_alive_from_history_nts(pubs_history, change_guid_prefix);

            // Check whether disposals contains a DATA(Up) from the same participant as the DATA(Uw).
            // If it does, then there is no need of adding the DATA(Uw).
            if (!announcement_from_same_participant_in_disposals(disposals, change_guid_prefix))
            {
                // Add DATA(Uw) to EDP publications writer's history.
                eprosima::fastrtps::rtps::WriteParams wp = change->write_params;
                pubs_history->add_change(change, wp);
            }
        }
        // DATA(Ur) case
        else if (discovery_db_.is_reader(change))
        {
            // Lock EDP subscriptions writer
            std::unique_lock<fastrtps::RecursiveTimedMutex> lock(edp->subscriptions_writer_.first->getMutex());

            // Remove all DATA(r) with the same sample identity as the DATA(Ur) from EDP subscriptions writer's history
            remove_related_alive_from_history_nts(subs_history, change_guid_prefix);

            // Check whether disposals contains a DATA(Up) from the same participant as the DATA(Ur).
            // If it does, then there is no need of adding the DATA(Ur).
            if (!announcement_from_same_participant_in_disposals(disposals, change_guid_prefix))
            {
                // Add DATA(Ur) to EDP subscriptions writer's history.
                eprosima::fastrtps::rtps::WriteParams wp = change->write_params;
                subs_history->add_change(change, wp);
            }
        }
        else
        {
            logError(RTPS_PDP_SERVER, "Wrong DATA received from disposals " << change->instanceHandle);
        }
    }
    // Clear database disposals list
    discovery_db_.clear_changes_to_dispose();
    return false;
}

bool PDPServer2::process_changes_release()
{
    logInfo(RTPS_PDP_SERVER, "process_changes_release start");
    process_changes_release_(discovery_db_.changes_to_release());
    discovery_db_.clear_changes_to_release();
    return false;
}

void PDPServer2::process_changes_release_(
        const std::vector<fastrtps::rtps::CacheChange_t*>& changes)
{
    // We will need the EDP publications/subscriptions writers, readers, and histories
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);

    // For each change to erase, first try to erase in case is in writer history and then it releases it
    for (auto ch : changes)
    {
        // Check if change owner is this participant. In that case, the change comes from a writer pool (PDP, EDP
        // publications or EDP subscriptions)
        if (ch->write_params.sample_identity().writer_guid() == mp_PDPWriter->getGuid())
        {
            if (discovery_db_.is_participant(ch))
            {
                // The change must return to the pool even if not present in the history
                // Normally Data(Up) will not be in history except in Own Server destruction
                if (!remove_change_from_writer_history(mp_PDPWriter, mp_PDPWriterHistory, ch))
                {
                    mp_PDPWriterHistory->release_Cache(ch);
                }
            }
            else if (discovery_db_.is_writer(ch))
            {
                // The change must return to the pool even if not present in the history
                // Normally Data(Uw) will not be in history except in Own Server destruction
                if (!remove_change_from_writer_history(
                            edp->publications_writer_.first,
                            edp->publications_writer_.second,
                            ch))
                {
                    edp->publications_writer_.second->release_Cache(ch);
                }
            }
            else if (discovery_db_.is_reader(ch))
            {
                // The change must return to the pool even if not present in the history
                // Normally Data(Ur) will not be in history except in Own Server destruction
                if (!remove_change_from_writer_history(
                            edp->subscriptions_writer_.first,
                            edp->subscriptions_writer_.second,
                            ch))
                {
                    edp->subscriptions_writer_.second->release_Cache(ch);
                }
            }
            else
            {
                logError(RTPS_PDP_SERVER, "Wrong DATA received to remove from this participant: "
                        << ch->instanceHandle);
            }
        }
        // The change is not from this participant. In that case, the change comes from a reader pool (PDP, EDP
        // publications or EDP subscriptions)
        else
        {
            // If the change is from a remote participant,
            // then it is never on any of the readers' histories, since we
            // take it out upon reading it in the listeners
            if (discovery_db_.is_participant(ch))
            {

                remove_change_from_writer_history(mp_PDPWriter, mp_PDPWriterHistory, ch, false);
                mp_PDPReaderHistory->release_Cache(ch);
            }
            else if (discovery_db_.is_writer(ch))
            {
                remove_change_from_writer_history(
                    edp->publications_writer_.first,
                    edp->publications_writer_.second,
                    ch,
                    false);
                edp->publications_reader_.second->release_Cache(ch);
            }
            else if (discovery_db_.is_reader(ch))
            {
                remove_change_from_writer_history(
                    edp->subscriptions_writer_.first,
                    edp->subscriptions_writer_.second,
                    ch,
                    false);
                edp->subscriptions_reader_.second->release_Cache(ch);
            }
            else
            {
                logError(PDPServer2, "Wrong DATA received to remove");
            }
        }
    }
}

void PDPServer2::remove_related_alive_from_history_nts(
        fastrtps::rtps::WriterHistory* writer_history,
        const fastrtps::rtps::GuidPrefix_t& entity_guid_prefix)
{
    // Iterate over changes in writer_history
    for (auto chit = writer_history->changesBegin(); chit != writer_history->changesEnd();)
    {
        // Remove all DATA whose original sender was entity_guid_prefix from writer_history
        if (entity_guid_prefix == discovery_db_.guid_from_change(*chit).guidPrefix)
        {
            chit = writer_history->remove_change(chit, false);
            continue;
        }
        chit++;
    }
}

bool PDPServer2::announcement_from_same_participant_in_disposals(
        const std::vector<fastrtps::rtps::CacheChange_t*>& disposals,
        const fastrtps::rtps::GuidPrefix_t& participant)
{
    for (auto change_: disposals)
    {
        if (discovery_db_.is_participant(change_) &&
                (discovery_db_.guid_from_change(change_).guidPrefix == participant))
        {
            return true;
        }
    }
    return false;
}

bool PDPServer2::process_dirty_topics()
{
    logInfo(RTPS_PDP_SERVER, "process_dirty_topics start");
    return discovery_db_.process_dirty_topics();
}

fastdds::rtps::ddb::DiscoveryDataBase& PDPServer2::discovery_db()
{
    return discovery_db_;
}

const RemoteServerList_t& PDPServer2::servers()
{
    return mp_builtin->m_DiscoveryServers;
}

bool PDPServer2::process_to_send_lists()
{
    logInfo(RTPS_PDP_SERVER, "process_to_send_lists start");
    // Process pdp_to_send_
    logInfo(RTPS_PDP_SERVER, "Processing pdp_to_send");
    process_to_send_list(discovery_db_.pdp_to_send(), mp_PDPWriter, mp_PDPWriterHistory);
    discovery_db_.clear_pdp_to_send();

    // Process edp_publications_to_send_
    logInfo(RTPS_PDP_SERVER, "Processing edp_publications_to_send");
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    process_to_send_list(
        discovery_db_.edp_publications_to_send(),
        edp->publications_writer_.first,
        edp->publications_writer_.second);
    discovery_db_.clear_edp_publications_to_send();

    // Process edp_subscriptions_to_send_
    logInfo(RTPS_PDP_SERVER, "Processing edp_subscriptions_to_send");
    process_to_send_list(
        discovery_db_.edp_subscriptions_to_send(),
        edp->subscriptions_writer_.first,
        edp->subscriptions_writer_.second);
    discovery_db_.clear_edp_subscriptions_to_send();

    return false;
}

bool PDPServer2::process_to_send_list(
        const std::vector<eprosima::fastrtps::rtps::CacheChange_t*>& send_list,
        fastrtps::rtps::RTPSWriter* writer,
        fastrtps::rtps::WriterHistory* history)
{
    // Iterate over DATAs in send_list
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(writer->getMutex());
    for (auto change: send_list)
    {
        // If the DATA is already in the writer's history, then remove it, but do not release the change.
        remove_change_from_history_nts(history, change, false);
        // Set change's writer GUID so it matches with this writer
        change->writerGUID = writer->getGuid();
        // Add DATA to writer's history.
        logInfo(RTPS_PDP_SERVER, "Adding change from " << change->instanceHandle << " to history");
        eprosima::fastrtps::rtps::WriteParams wp = change->write_params;
        history->add_change(change, wp);
    }
    return true;
}

bool PDPServer2::remove_change_from_writer_history(
        fastrtps::rtps::RTPSWriter* writer,
        fastrtps::rtps::WriterHistory* history,
        fastrtps::rtps::CacheChange_t* change,
        bool release_change /*= true*/)
{
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(writer->getMutex());
    return remove_change_from_history_nts(history, change, release_change);
}

bool PDPServer2::remove_change_from_history_nts(
        fastrtps::rtps::WriterHistory* history,
        fastrtps::rtps::CacheChange_t* change,
        bool release_change /*= true*/)
{
    for (auto chit = history->changesRbegin(); chit != history->changesRend(); chit++)
    {
        if (change->instanceHandle == (*chit)->instanceHandle &&
                change->write_params.sample_identity() == (*chit)->write_params.sample_identity())
        {
            if (release_change)
            {
                history->remove_change(*chit);
            }
            else
            {
                history->remove_change_and_reuse((*chit)->sequenceNumber);
            }
            return true;
        }
    }
    return false;
}

bool PDPServer2::pending_ack()
{
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    bool ret = (!discovery_db_.server_acked_by_all() ||
            mp_PDPWriterHistory->getHistorySize() > 1 ||
            edp->publications_writer_.second->getHistorySize() > 0 ||
            edp->subscriptions_writer_.second->getHistorySize() > 0);
    logInfo(RTPS_PDP_SERVER, "Is server acked by all? " << discovery_db_.server_acked_by_all());
    logInfo(RTPS_PDP_SERVER, "EDP pub history size " << edp->publications_writer_.second->getHistorySize());
    logInfo(RTPS_PDP_SERVER, "EDP sub history size " << edp->subscriptions_writer_.second->getHistorySize());
    logInfo(RTPS_PDP_SERVER, "PDP history size " << mp_PDPWriterHistory->getHistorySize());
    logInfo(RTPS_PDP_SERVER, "Are there pending changes? " << ret);
    return ret;
}

std::vector<fastrtps::rtps::GuidPrefix_t> PDPServer2::servers_prefixes()
{
    std::vector<GuidPrefix_t> servers;
    for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
    {
        servers.push_back(it.guidPrefix);
    }
    return servers;
}

eprosima::fastrtps::rtps::ResourceEvent& PDPServer2::get_resource_event_thread()
{
    return resource_event_thread_;
}

bool PDPServer2::all_servers_acknowledge_pdp()
{
    // check if already initialized
    assert(mp_PDPWriterHistory && mp_PDPWriter);

    return discovery_db_.server_acked_by_my_servers();
}

void PDPServer2::ping_remote_servers()
{
    // Get the servers that have not ACKed this server's DATA(p)
    std::vector<GuidPrefix_t> ack_pending_servers = discovery_db_.ack_pending_servers();
    std::vector<GUID_t> remote_readers;
    LocatorList_t locators;

    // Iterate over the list of servers
    for (auto& server : mp_builtin->m_DiscoveryServers)
    {

        // If the server is the the ack_pending list, then add its GUID and locator to send the announcement
        auto server_it = std::find(ack_pending_servers.begin(), ack_pending_servers.end(), server.guidPrefix);
        if (server_it != ack_pending_servers.end())
        {
            // get the info to send to this already known locators
            remote_readers.push_back(GUID_t(server.guidPrefix, c_EntityId_SPDPReader));
            locators.push_back(server.metatrafficUnicastLocatorList);
        }
    }
    send_announcement(discovery_db().cache_change_own_participant(), remote_readers, locators);
}

void PDPServer2::send_announcement(
        CacheChange_t* change,
        std::vector<GUID_t> remote_readers,
        LocatorList_t locators,
        bool dispose /* = false */)
{

    if (nullptr == change)
    {
        return;
    }

    DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);
    RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, sender);

    if (dispose)
    {
        fastrtps::rtps::StatefulWriter* writer = static_cast<fastrtps::rtps::StatefulWriter*>(mp_PDPWriter);
        writer->fastrtps::rtps::StatefulWriter::incrementHBCount();
        group.add_heartbeat(
            change->sequenceNumber,
            change->sequenceNumber,
            writer->getHeartbeatCount(),
            true,
            false);
    }

    if (!group.add_data(*change, false))
    {
        logError(RTPS_PDP_SERVER, "Error sending announcement from server to clients");
    }
}

bool PDPServer2::read_backup(nlohmann::json& ddb_json, std::vector<nlohmann::json>& new_changes)
{
    std::ifstream myfile;
    try
    {
        myfile.open(get_ddb_persistence_file_name(), std::ios_base::in);
        // read json object
        myfile >> ddb_json;
        myfile.close();

        myfile.open(get_ddb_queue_persistence_file_name(), std::ios_base::in);

        std::string line;
        while (std::getline(myfile, line))
        {
            nlohmann::json change_json(line);
            // Read every change, and store it in json format in a vector
            new_changes.push_back(change_json);
        }

        myfile.close();
    }
    catch(const std::exception& e)
    {
        return false;
    }
    return true;
}


bool PDPServer2::process_discovery_database_restore_(nlohmann::json& j)
{
    logInfo(RTPS_PDP_SERVER, "Restoring DiscoveryDataBase from backup");

    // load every change and create it from its respective history
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    EDPServerPUBListener2* edp_pub_listener = static_cast<EDPServerPUBListener2*>(edp->publications_listener_);
    EDPServerSUBListener2* edp_sub_listener = static_cast<EDPServerSUBListener2*>(edp->subscriptions_listener_);

    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(mp_PDPReader->getMutex());
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock_edpp(edp->publications_reader_.first->getMutex());
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock_edps(edp->subscriptions_reader_.first->getMutex());

    std::map<eprosima::fastrtps::rtps::InstanceHandle_t, fastrtps::rtps::CacheChange_t*> changes_map;
    fastrtps::rtps::SampleIdentity sample_identity_aux;
    uint32_t length;

    try
    {
        fastrtps::rtps::CacheChange_t* change_aux;
        // Create every participant change. If it is external creates it from Reader,
        // if it is created from the server, it is created from writer
        for (auto it = j["participants"].begin(); it != j["participants"].end(); ++it)
        {
            length = it.value()["change"]["serialized_payload"]["length"].get<std::uint32_t>();
            (std::istringstream) it.value()["change"]["sample_identity"].get<std::string>() >> sample_identity_aux;

            // Belongs to own server
            if (sample_identity_aux.writer_guid() == mp_PDPWriter->getGuid())
            {
                if (!mp_PDPWriterHistory->reserve_Cache(&change_aux, length))
                {
                    logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }
            // It came from outside
            else
            {
                if (!mp_PDPReaderHistory->reserve_Cache(&change_aux, length))
                {
                    logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }

            // deserialize from json to change already created
            ddb::from_json(it.value()["change"], *change_aux);

            // if the change was read as is_local we must pass it to listener with his own writer_guid
            if (it.value()["is_local"].get<bool>())
            {
                change_aux->writerGUID = change_aux->write_params.sample_identity().writer_guid();
            }
            else
            {
                change_aux->writerGUID = fastrtps::rtps::c_Guid_Unknown;
            }


            changes_map.insert(
                    std::make_pair(change_aux->instanceHandle, change_aux));

            // call listener to create proxy info for other entities different than server
            if (change_aux->write_params.sample_identity().writer_guid().guidPrefix !=
                    mp_PDPWriter->getGuid().guidPrefix
                    && change_aux->kind == fastrtps::rtps::ALIVE)
            {
                // p_PDPReader->change_received(change_aux, nullptr);
                mp_listener->onNewCacheChangeAdded(mp_PDPReader, change_aux);
            }
        }

        // Create every writer change. If it is external creates it from Reader,
        // if it is created from the server, it is created from writer
        for (auto it = j["writers"].begin(); it != j["writers"].end(); ++it)
        {
            fastrtps::rtps::CacheChange_t* change_aux;
            length = it.value()["change"]["serialized_payload"]["length"].get<std::uint32_t>();
            (std::istringstream) it.value()["change"]["sample_identity"].get<std::string>() >> sample_identity_aux;

            // Belongs to own server
            if (sample_identity_aux.writer_guid() == mp_PDPWriter->getGuid())
            {
                if (!edp->publications_writer_.second->reserve_Cache(&change_aux, length))
                {
                    logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }
            // It came from outside
            else
            {
                if (!edp->publications_reader_.second->reserve_Cache(&change_aux, length))
                {
                    logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }

            // deserialize from json to change already created
            ddb::from_json(it.value()["change"], *change_aux);

            changes_map.insert(
                    std::make_pair(change_aux->instanceHandle, change_aux));

            // call listener to create proxy info for other entities different than server
            if (change_aux->write_params.sample_identity().writer_guid().guidPrefix !=
                    mp_PDPWriter->getGuid().guidPrefix
                    && change_aux->kind == fastrtps::rtps::ALIVE)
            {
                edp_pub_listener->onNewCacheChangeAdded(edp->publications_reader_.first, change_aux);
            }
        }

        // Create every reader change. If it is external creates it from Reader,
        // if it is created from the server, it is created from writer
        for (auto it = j["readers"].begin(); it != j["readers"].end(); ++it)
        {
            fastrtps::rtps::CacheChange_t* change_aux;
            length = it.value()["change"]["serialized_payload"]["length"].get<std::uint32_t>();
            (std::istringstream) it.value()["change"]["sample_identity"].get<std::string>() >> sample_identity_aux;

            // Belongs to own server
            if (sample_identity_aux.writer_guid() == mp_PDPWriter->getGuid())
            {
                if (!edp->subscriptions_writer_.second->reserve_Cache(&change_aux, length))
                {
                    logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }
            // It came from outside
            else
            {
                if (!edp->subscriptions_reader_.second->reserve_Cache(&change_aux, length))
                {
                    logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }

            // deserialize from json to change already created
            ddb::from_json(it.value()["change"], *change_aux);

            changes_map.insert(
                    std::make_pair(change_aux->instanceHandle, change_aux));

            // call listener to create proxy info for other entities different than server
            if (change_aux->write_params.sample_identity().writer_guid().guidPrefix !=
                    mp_PDPWriter->getGuid().guidPrefix
                    && change_aux->kind == fastrtps::rtps::ALIVE)
            {
                edp_sub_listener->onNewCacheChangeAdded(edp->subscriptions_reader_.first, change_aux);
            }
        }

        // load database
        discovery_db_.from_json(j, changes_map);
    }
    catch (std::ios_base::failure&)
    {
        // TODO clean changes in case it has been an error
        logError(DISCOVERY_DATABASE, "BACKUP CORRUPTED");
        return false;
    }
    return true;
}

bool PDPServer2::restore_queue(std::vector<nlohmann::json>& new_changes)
{
    fastrtps::rtps::SampleIdentity sample_identity_aux;
    uint32_t length;

    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    EDPServerPUBListener2* edp_pub_listener = static_cast<EDPServerPUBListener2*>(edp->publications_listener_);
    EDPServerSUBListener2* edp_sub_listener = static_cast<EDPServerSUBListener2*>(edp->subscriptions_listener_);

    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(mp_PDPReader->getMutex());
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock_edpp(edp->publications_reader_.first->getMutex());
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock_edps(edp->subscriptions_reader_.first->getMutex());

    try
    {
        // Read every change and push it to the listener that it belongs
        for (auto json_change : new_changes)
        {

            fastrtps::rtps::CacheChange_t* change_aux;
            length = json_change["serialized_payload"]["length"].get<std::uint32_t>();
            (std::istringstream) json_change["sample_identity"].get<std::string>() >> sample_identity_aux;



            // Belongs to own server
            if (sample_identity_aux.writer_guid() == mp_PDPWriter->getGuid())
            {
                if (discovery_db_.is_participant(sample_identity_aux.writer_guid()))
                {
                    if (!mp_PDPWriterHistory->reserve_Cache(&change_aux, length))
                    {
                        logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                        // TODO release changes and exit
                    }
                    else
                    {
                        ddb::from_json(new_changes, *change_aux);
                        mp_listener->onNewCacheChangeAdded(mp_PDPReader, change_aux);
                    }

                }
                else if (discovery_db_.is_writer(sample_identity_aux.writer_guid()))
                {
                    if (!edp->publications_writer_.second->reserve_Cache(&change_aux, length))
                    {
                        logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                        // TODO release changes and exit
                    }
                    else
                    {
                        ddb::from_json(new_changes, *change_aux);
                        edp_pub_listener->onNewCacheChangeAdded(edp->publications_reader_.first, change_aux);
                    }
                }
                else if (discovery_db_.is_reader(sample_identity_aux.writer_guid()))
                {
                    if (!edp->subscriptions_writer_.second->reserve_Cache(&change_aux, length))
                    {
                        logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                        // TODO release changes and exit
                    }
                    else
                    {
                        ddb::from_json(new_changes, *change_aux);
                        edp_sub_listener->onNewCacheChangeAdded(edp->subscriptions_reader_.first, change_aux);
                    }
                }
            }
            // It came from outside
            else
            {
                if (discovery_db_.is_participant(sample_identity_aux.writer_guid()))
                {
                    if (!mp_PDPReaderHistory->reserve_Cache(&change_aux, length))
                    {
                        logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                        // TODO release changes and exit
                    }
                    else
                    {
                        ddb::from_json(new_changes, *change_aux);
                        mp_listener->onNewCacheChangeAdded(mp_PDPReader, change_aux);
                    }

                }
                else if (discovery_db_.is_writer(sample_identity_aux.writer_guid()))
                {
                    if (!edp->publications_reader_.second->reserve_Cache(&change_aux, length))
                    {
                        logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                        // TODO release changes and exit
                    }
                    else
                    {
                        ddb::from_json(new_changes, *change_aux);
                        edp_pub_listener->onNewCacheChangeAdded(edp->publications_reader_.first, change_aux);
                    }
                }
                else if (discovery_db_.is_reader(sample_identity_aux.writer_guid()))
                {
                    if (!edp->subscriptions_reader_.second->reserve_Cache(&change_aux, length))
                    {
                        logError(RTPS_PDP_SERVER, "Error creating CacheChange");
                        // TODO release changes and exit
                    }
                    else
                    {
                        ddb::from_json(new_changes, *change_aux);
                        edp_sub_listener->onNewCacheChangeAdded(edp->subscriptions_reader_.first, change_aux);
                    }
                }
            }
        }
    }
    catch (std::ios_base::failure&)
    {
        // TODO clean changes in case it has been an error
        logError(DISCOVERY_DATABASE, "QUEUE BACKUP CORRUPTED");
        return false;
    }
    return true;
}

void PDPServer2::process_ddb_backup()
{
    logInfo(DISCOVERY_DATABASE, "Dump DDB in json backup");

    // This will erase the last backup stored
    std::ofstream backup_json_file;
    backup_json_file.open(get_ddb_persistence_file_name(), std::ios_base::out);

    // Set j with the json from database dump
    nlohmann::json j;
    discovery_db().to_json(j);
    // setw makes pretty print for json
    backup_json_file << std::setw(4) << j << std::endl;
    backup_json_file.close();

    discovery_db_.clean_backup();
}



} // namespace rtps
} // namespace fastdds
} // namespace eprosima
