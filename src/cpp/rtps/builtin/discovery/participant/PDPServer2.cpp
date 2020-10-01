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

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPServer2::PDPServer2(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation)
    : PDP(builtin, allocation)
    , mp_sync(nullptr)
    , discovery_db_(builtin->mp_participantImpl->getGuid().guidPrefix)
{
}

PDPServer2::~PDPServer2()
{
    delete(mp_sync);
}

bool PDPServer2::init(
        RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    mp_EDP = new EDPServer2(this, mp_RTPSParticipant);
    if (!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP_SERVER, "Endpoint discovery configuration failed");
        return false;
    }

    // Initialize server dedicated thread.
    resource_event_thread_.init_thread();

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
     */
    mp_sync = new DServerEvent2(this,
                    TimeConv::Duration_t2MilliSecondsDouble(
                        m_discovery.discovery_config.discoveryServer_client_syncperiod));

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
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.times.heartbeatResponseDelay = pdp_heartbeat_response_delay;

    // PDP Listener
    mp_listener = new PDPServerListener2(this);

    // Create PDP Reader
    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory,
            mp_listener, c_EntityId_SPDPReader, true, false))
    {
        // Enable unknown clients to reach this reader
        mp_PDPReader->enableMessagesFromUnkownWriters(true);
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
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
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
    }
    // Could not create PDP Writer, so return false
    else
    {
        logError(RTPS_PDP_SERVER, "PDPServer Writer creation failed");
        delete(mp_PDPWriterHistory);
        mp_PDPWriterHistory = nullptr;
        return false;
    }
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

        // TODO: remove when the Writer API issue is resolved
        std::lock_guard<std::recursive_mutex> lock(*getMutex());
        // Waiting till we remove C++11 restriction:
        // clients_.insert_or_assign(temp_reader_data_.guid(), temp_reader_data_);
        auto emplace_result = clients_.emplace(temp_reader_data_.guid(), temp_reader_data_);
        if (!emplace_result.second)
        {
            emplace_result.first->second = temp_reader_data_;
        }
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

    std::unique_lock<std::recursive_mutex> lock(*getMutex());
    // TODO: remove when the Writer API issue is resolved
    clients_.erase(pdata->m_guid);
}

#if HAVE_SQLITE3
std::string PDPServer2::GetPersistenceFileName()
{
    assert(getRTPSParticipant());

    std::ostringstream filename(std::ios_base::ate);
    std::string prefix;

    // . is not suitable separator for filenames
    filename << "server-" << getRTPSParticipant()->getGuid().guidPrefix;
    prefix = filename.str();
    std::replace(prefix.begin(), prefix.end(), '.', '-');
    filename.str(std::move(prefix));
    filename << ".db";

    return filename.str();
}

#endif // HAVE_SQLITE3

void PDPServer2::announceParticipantState(
        bool new_change,
        bool dispose /* = false */,
        WriteParams& )
{
    // logInfo(RTPS_PDP_SERVER, "Announcing Server (new change: " << new_change << ")");
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
            // TODO: Make sure in the database that this CacheChange_t is given this sequence number
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
                change->write_params = std::move(wp);

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

                // Update the database with our own data
                if (discovery_db().update(
                    change,
                    ddb::DiscoveryParticipantChangeData(metatraffic_locators, false, false)))
                {
                    // Distribute
                    awake_server_thread();
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
        // TODO: Make sure in the database that this CacheChange_t is given this sequence number
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
                awake_server_thread();
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

    // Create a list of receivers based on the remote participants known by the discovery database. Add the locators
    // of those remote participants.
    LocatorList_t locators;
    std::vector<GUID_t> remote_readers;

    // // Iterate over clients
    // for (auto client: clients_)
    // {
    //     fastrtps::rtps::ReaderProxyData& rat = client.second;
    //     remote_readers.push_back(rat.guid());

    //     // Add default unicast locators of the remote reader
    //     for (const Locator_t& locator: client.second.remote_locators().unicast)
    //     {
    //         locators.push_back(locator);
    //     }
    // }

    // std::vector<GuidPrefix_t> remote_participants = discovery_db_.remote_participants();
    // for (GuidPrefix_t participant_prefix: remote_participants)
    // {
    //     // Add remote reader
    //     GUID_t remote_guid(participant_prefix, c_EntityId_SPDPReader);
    //     remote_readers.push_back(remote_guid);

    //     // Iterate over participant_proxies to find the reader
    //     for (ParticipantProxyData* proxy: participant_proxies_)
    //     {
    //         // Check if this is the participant for which we are looking
    //         if (proxy->m_guid == remote_guid)
    //         {
    //             // Add default unicast locators of the remote reader
    //             for (Locator_t locator: proxy->metatraffic_locators.unicast)
    //             {
    //                 locators.push_back(locator);
    //             }
    //             // The participant will be there only once, so we can stop looking when found
    //             break;
    //         }
    //     }
    // }

    // // Add own default multicast address
    // for (Locator_t locator: participant_proxies_[0]->metatraffic_locators.multicast)
    // {
    //     locators.push_back(locator);
    // }
    // for (Locator_t locator: participant_proxies_[0]->metatraffic_locators.unicast)
    // {
    //     locators.push_back(locator);
    // }

    DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);
    RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, sender);

    if (!group.add_data(*change, false))
    {
        logError(RTPS_PDP_SERVER, "Error sending announcement from server to clients");
    }
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
                awake_server_thread();

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

    // delegate into the base class for inherited proxy database removal
    return PDP::remove_remote_participant(partGUID, reason);
}

bool PDPServer2::process_data_queues()
{
    logInfo(RTPS_PDP_SERVER, "process_data_queues start");
    discovery_db_.process_pdp_data_queue();
    return discovery_db_.process_edp_data_queue();
}

void PDPServer2::awake_server_thread(
        double interval_ms /*= 0*/)
{
    mp_sync->update_interval_millisec(interval_ms);
    mp_sync->cancel_timer();
    mp_sync->restart_timer();
}

bool PDPServer2::server_update_routine()
{
    // There is pending work to be done by the server if there are changes that have not been acknowledged.
    bool pending_work = true;

    // Execute the server routine
    do
    {
        logInfo(RTPS_PDP_SERVER, "");
        logInfo(RTPS_PDP_SERVER, "-------------------- Server routine start --------------------");
        process_writers_acknowledgements();     // server + ddb(functor_with_ddb)
        process_data_queues();                   // all ddb
        process_disposals();                    // server + ddb(changes_to_dispose(), clear_changes_to_disposes())
        process_dirty_topics();                 // all ddb
        process_to_send_lists();                // server + ddb(get_to_send, remove_to_send_this)
        process_changes_release();              // server + ddb(changes_to_release(), clear_changes_to_release())
        pending_work = pending_ack();           // all server
        logInfo(RTPS_PDP_SERVER, "-------------------- Server routine end --------------------");
        logInfo(RTPS_PDP_SERVER, "");
    }
    // If the data queue is not empty re-start the routine.
    // A non-empty queue means that the server has received a change while it is running the processing routine.
    while (!discovery_db_.data_queue_empty());

    return pending_work;
}

bool PDPServer2::process_writers_acknowledgements()
{
    // logInfo(RTPS_PDP_SERVER, "process_writers_acknowledgements start");
    /* PDP Writer's History */
    bool pending = process_history_acknowledgement(
        static_cast<fastrtps::rtps::StatefulWriter*>(mp_PDPWriter), mp_PDPWriterHistory);

    /* EDP Publications Writer's History */
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    pending |= process_history_acknowledgement(edp->publications_writer_.first, edp->publications_writer_.second);

    /* EDP Subscriptions Writer's History */
    pending |= process_history_acknowledgement(edp->subscriptions_writer_.first, edp->subscriptions_writer_.second);

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
            fastdds::rtps::ddb::DiscoveryDataBase::AckedFunctor func = discovery_db_.functor(c);
            writer->for_each_reader_proxy(c, func);

            // If the change has been acknowledge by everyone
            if (!func)
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
            discovery_db_.delete_entity_of_change(c);
            // Remove from writer's history
            return writer_history->remove_change(cit);
        }
    }

    // proceed to the next change
    return ++cit;
}

bool PDPServer2::process_disposals()
{
    // logInfo(RTPS_PDP_SERVER, "process_disposals start");
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    fastrtps::rtps::WriterHistory* pubs_history = edp->publications_writer_.second;
    fastrtps::rtps::WriterHistory* subs_history = edp->subscriptions_writer_.second;

    // Get list of disposals from database
    std::vector<fastrtps::rtps::CacheChange_t*> disposals = discovery_db_.changes_to_dispose();
    // Iterate over disposals
    for (auto change: disposals)
    {
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
            mp_PDPWriterHistory->add_change(change);
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
                pubs_history->add_change(change);
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
                subs_history->add_change(change);
            }
        }
        else
        {
            logError(PDPServer2, "Wrong DATA received from disposals");
        }
    }
    // Clear database disposals list
    discovery_db_.clear_changes_to_dispose();
    return false;
}

bool PDPServer2::process_changes_release()
{
    // logInfo(RTPS_PDP_SERVER, "process_changes_release start");
    // We will need the EDP publications/subscriptions writers, readers, and histories
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);

    // For each change to erase, first try to erase in case is in writer history and then it releases it
    for (auto ch : discovery_db_.changes_to_release())
    {
        // Check if change owner is this participant. In that case, the change comes from a writer pool (PDP, EDP
        // publications or EDP subscriptions)
        if (discovery_db_.guid_from_change(ch).guidPrefix == mp_builtin->mp_participantImpl->getGuid().guidPrefix)
        {
            if (discovery_db_.is_participant(ch))
            {
                // DATA(Up) will not be in the history, since they are only added here once the change is acked by
                // everyone, time at which it is removed from history.
                if (ch->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
                {
                    // The change must return to the pool even if not present in the history
                    if (!remove_change_from_writer_history(mp_PDPWriter, mp_PDPWriterHistory, ch))
                    {
                        mp_PDPWriterHistory->release_Cache(ch);
                    }
                }
                else
                {
                    mp_PDPWriterHistory->release_Cache(ch);
                }
            }
            else if (discovery_db_.is_writer(ch))
            {
                // DATA(Uw) will not be in the history, since they are only added here once the change is acked by
                // everyone, time at which it is removed from history.
                if (ch->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
                {
                    // The change must return to the pool even if not present in the history
                    if (!remove_change_from_writer_history(
                                edp->publications_writer_.first,
                                edp->publications_writer_.second,
                                ch))
                    {
                        edp->publications_writer_.second->release_Cache(ch);
                    }
                }
                else
                {
                    edp->publications_writer_.second->release_Cache(ch);
                }
            }
            else if (discovery_db_.is_reader(ch))
            {
                // DATA(Ur) will not be in the history, since they are only added here once the change is acked by
                // everyone, time at which it is removed from history.
                if (ch->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
                {
                    // The change must return to the pool even if not present in the history
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
                    edp->subscriptions_writer_.second->release_Cache(ch);
                }
            }
            else
            {
                logError(PDPServer2, "Wrong DATA received to remove");
            }
        }
        // The change is not from this participant. In that case, the change comes from a reader pool (PDP, EDP
        // publications or EDP subscriptions)
        else
        {
            // If the change is from a remote participant, then it is never on any of the readers' histories, since we
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
    discovery_db_.clear_changes_to_release();
    return false;
}

void PDPServer2::remove_related_alive_from_history_nts(
        fastrtps::rtps::WriterHistory* writer_history,
        const fastrtps::rtps::GuidPrefix_t& entity_guid_prefix)
{
    // Iterate over changes in writer_history
    for (auto chit = writer_history->changesBegin(); chit != writer_history->changesEnd(); chit++)
    {
        // Remove all DATA whose original sender was entity_guid_prefix from writer_history
        if (entity_guid_prefix == discovery_db_.guid_from_change(*chit).guidPrefix)
        {
            writer_history->remove_change(*chit);
        }
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
        // If the DATA is already in the writer's history, then remove it.
        remove_change_from_history_nts(history, change);
        // Add DATA to writer's history.
        change->writerGUID.guidPrefix = mp_PDPWriter->getGuid().guidPrefix;
        history->add_change(change);
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
        if (change->instanceHandle == (*chit)->instanceHandle)
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
    bool ret = (mp_PDPWriterHistory->getHistorySize() > 1 ||
            edp->publications_writer_.second->getHistorySize() > 0 ||
            edp->subscriptions_writer_.second->getHistorySize() > 0);
    logInfo(RTPS_PDP_SERVER, "Are there pending changes? " << ret);
    return ret;
}

eprosima::fastrtps::rtps::ResourceEvent& PDPServer2::get_resource_event_thread()
{
    return resource_event_thread_;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
