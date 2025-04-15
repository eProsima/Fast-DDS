// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPServer.cpp
 *
 */

#include <fstream>
#include <iostream>
#include <mutex>
#include <set>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/history/History.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/discovery/database/backup/SharedBackupFunctions.hpp>
#include <rtps/builtin/discovery/endpoint/EDPServer.hpp>
#include <rtps/builtin/discovery/endpoint/EDPServerListeners.hpp>
#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpoints.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpointsSecure.hpp>
#include <rtps/builtin/discovery/participant/DS/FakeWriter.hpp>
#include <rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.hpp>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>
#include <rtps/builtin/discovery/participant/PDPServerListener.hpp>
#include <rtps/builtin/discovery/participant/timedevent/DServerEvent.hpp>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/StatefulWriter.hpp>
#include <utils/TimeConversion.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPServer::PDPServer(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation,
        DurabilityKind_t durability_kind /* TRANSIENT_LOCAL */)
    : PDP(builtin, allocation)
    , routine_(nullptr)
    , discovery_db_(builtin->mp_participantImpl->getGuid().guidPrefix)
    , durability_ (durability_kind)
{
    // Add remote servers from environment variable
    LocatorList_t env_servers;
    {
        std::lock_guard<std::recursive_mutex> lock(*getMutex());
        if (load_environment_server_info(env_servers))
        {
            for (auto server : env_servers)
            {
                {
                    std::unique_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());
                    mp_builtin->m_DiscoveryServers.push_back(server);
                }
                m_discovery.discovery_config.m_DiscoveryServers.push_back(server);
            }
        }
    }
}

PDPServer::~PDPServer()
{
    // Stop timed events
    routine_->cancel_timer();

    // Disable database
    discovery_db_.disable();

    // Delete timed events
    delete(routine_);

    // Clear ddb and release its changes
    process_changes_release_(discovery_db_.clear());
}

bool PDPServer::init(
        RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    // INIT EDP
    mp_EDP = new EDPServer(this, mp_RTPSParticipant, durability_);
    if (!mp_EDP->initEDP(m_discovery))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Endpoint discovery configuration failed");
        return false;
    }

    // Activate listeners
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);
    builtin_endpoints_->enable_pdp_readers(getRTPSParticipant());
    getRTPSParticipant()->enableReader(edp->subscriptions_reader_.first);
    getRTPSParticipant()->enableReader(edp->publications_reader_.first);

    // Initialize server dedicated thread.
    const RTPSParticipantAttributes& part_attr = getRTPSParticipant()->get_attributes();
    uint32_t id_for_thread = static_cast<uint32_t>(part_attr.participantID);
    const fastdds::rtps::ThreadSettings& thr_config = part_attr.discovery_server_thread;
    resource_event_thread_.init_thread(thr_config, "dds.ds_ev.%u", id_for_thread);

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
     */
    routine_ = new DServerRoutineEvent(this,
                    TimeConv::Duration_t2MilliSecondsDouble(
                        m_discovery.discovery_config.discoveryServer_client_syncperiod));

    return true;
}

void PDPServer::pre_enable_actions()
{
    std::vector<nlohmann::json> backup_queue;
    // Restore the DDB from file if this is a BACKUP server
    if (durability_ == TRANSIENT)
    {
        nlohmann::json backup_json;
        // If the DS is BACKUP, try to restore DDB from file
        discovery_db().backup_in_progress(true);
        if (read_backup(backup_json, backup_queue))
        {
            if (process_backup_discovery_database_restore(backup_json))
            {
                EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "DiscoveryDataBase restored correctly");
            }
        }
        else
        {
            EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                    "Error reading backup file. Corrupted or unmissing file, restarting from scratch");
        }

        discovery_db().backup_in_progress(false);

        discovery_db_.persistence_enable(get_ddb_queue_persistence_file_name());
    }
    else
    {
        // Allows the ddb to process new messages from this point
        discovery_db_.enable();
    }

    // Restoring the queue must be done after starting the routine
    if (durability_ == TRANSIENT)
    {
        // This vector is empty till backup queue is implemented
        process_backup_restore_queue(backup_queue);
    }
}

ParticipantProxyData* PDPServer::createParticipantProxyData(
        const ParticipantProxyData& participant_data,
        const GUID_t& writer_guid)
{
    std::lock_guard<std::recursive_mutex> lock(*getMutex());

    // Lease duration is controlled for owned clients or linked servers.
    // Other clients liveliness is provided through server's PDP discovery data

    // Check if the DATA msg is relayed by another server
    bool do_lease = participant_data.guid.guidPrefix == writer_guid.guidPrefix;

    if (!do_lease)
    {
        // If not a client verify this participant is a server
        std::string part_type = check_participant_type(participant_data.properties);
        if (part_type == ParticipantType::SERVER || part_type == ParticipantType::BACKUP)
        {
            do_lease = true;
        }
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.guid, do_lease, &participant_data);
    if (pdata != nullptr)
    {
        if (do_lease)
        {
            pdata->lease_duration_event->update_interval(pdata->lease_duration);
            pdata->lease_duration_event->restart_timer();
        }
    }

    return pdata;
}

void PDPServer::update_builtin_locators()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    mp_builtin->updateMetatrafficLocators(endpoints->reader.reader_->getAttributes().unicastLocatorList);
}

bool PDPServer::createPDPEndpoints()
{
#if HAVE_SECURITY
    if (should_protect_discovery())
    {
        return create_secure_ds_pdp_endpoints();
    }
#endif  // HAVE_SECURITY

    return create_ds_pdp_endpoints();
}

#if HAVE_SECURITY
bool PDPServer::should_protect_discovery()
{
    return mp_RTPSParticipant->is_secure() && mp_RTPSParticipant->security_attributes().is_discovery_protected;
}

bool PDPServer::create_secure_ds_pdp_endpoints()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Beginning PDPServer Endpoints creation");

    auto endpoints = new fastdds::rtps::DiscoveryServerPDPEndpointsSecure();
    builtin_endpoints_.reset(endpoints);

    bool ret_val = create_ds_pdp_reliable_endpoints(*endpoints, true) && create_ds_pdp_best_effort_reader(*endpoints);

    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "PDPServer Endpoints creation finished");

    return ret_val;
}

bool PDPServer::create_ds_pdp_best_effort_reader(
        DiscoveryServerPDPEndpointsSecure& endpoints)
{
    const RTPSParticipantAttributes& pattr = mp_RTPSParticipant->get_attributes();

    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    endpoints.stateless_reader.history_.reset(new ReaderHistory(hatt));

    ReaderAttributes ratt;
    ratt.expects_inline_qos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.external_unicast_locators = mp_builtin->m_att.metatraffic_external_unicast_locators;
    ratt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    ratt.endpoint.topicKind = WITH_KEY;
    // Change depending on backup mode
    ratt.endpoint.durabilityKind = VOLATILE;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;

    endpoints.stateless_reader.listener_.reset(new PDPSecurityInitiatorListener(this,
            [this](const ParticipantProxyData& participant_data)
            {
                auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
                std::lock_guard<fastdds::RecursiveTimedMutex> wlock(endpoints->writer.writer_->getMutex());

                CacheChange_t* change = discovery_db().cache_change_own_participant();
                if (change != nullptr)
                {
                    std::vector<GUID_t> remote_readers;
                    LocatorList locators;

                    remote_readers.emplace_back(participant_data.guid.guidPrefix, c_EntityId_SPDPReader);

                    for (auto& locator : participant_data.metatraffic_locators.unicast)
                    {
                        locators.push_back(locator);
                    }

                    send_announcement(change, remote_readers, locators, false);

                }
            }));

    // Create PDP Reader
    RTPSReader* reader = nullptr;
    if (mp_RTPSParticipant->createReader(&reader, ratt, endpoints.stateless_reader.history_.get(),
            endpoints.stateless_reader.listener_.get(), c_EntityId_SPDPReader, true, false))
    {
        endpoints.stateless_reader.reader_ = dynamic_cast<fastdds::rtps::StatelessReader*>(reader);
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(reader, false);
    }
    // Could not create PDP Reader, so return false
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "PDPServer security initiation Reader creation failed");
        endpoints.stateless_reader.release();
        return false;
    }

    return true;
}

#endif  // HAVE_SECURITY

bool PDPServer::create_ds_pdp_endpoints()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Beginning PDPServer Endpoints creation");

    auto endpoints = new fastdds::rtps::DiscoveryServerPDPEndpoints();
    builtin_endpoints_.reset(endpoints);

    bool ret_val = create_ds_pdp_reliable_endpoints(*endpoints, false);

    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "PDPServer Endpoints creation finished");

    return ret_val;
}

bool PDPServer::create_ds_pdp_reliable_endpoints(
        DiscoveryServerPDPEndpoints& endpoints,
        bool secure)
{
    static_cast<void>(secure);

    /***********************************
    * PDP READER
    ***********************************/
    // PDP Reader History
    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    endpoints.reader.history_.reset(new ReaderHistory(hatt));

    // PDP Reader Attributes
    ReaderAttributes ratt = create_builtin_reader_attributes();
    // Change depending on backup mode
    ratt.endpoint.durabilityKind = durability_;

#if HAVE_SECURITY
    if (secure)
    {
        ratt.endpoint.security_attributes().is_submessage_protected = true;
        ratt.endpoint.security_attributes().plugin_endpoint_attributes =
                PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
#endif // HAVE_SECURITY

#if HAVE_SQLITE3
    ratt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    ratt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            get_reader_persistence_file_name()));
#endif // HAVE_SQLITE3

    // PDP Listener
    endpoints.reader.listener_.reset(new PDPServerListener(this));

    // Create PDP Reader
    RTPSReader* reader = nullptr;
#if HAVE_SECURITY
    EntityId_t reader_entity = secure ? c_EntityId_spdp_reliable_participant_secure_reader : c_EntityId_SPDPReader;
#else
    EntityId_t reader_entity = c_EntityId_SPDPReader;
#endif // if HAVE_SECURITY
    if (mp_RTPSParticipant->createReader(&reader, ratt, endpoints.reader.history_.get(),
            endpoints.reader.listener_.get(), reader_entity, true, false))
    {
        endpoints.reader.reader_ = dynamic_cast<fastdds::rtps::StatefulReader*>(reader);

#if HAVE_SECURITY
        if (!secure)
#endif  // HAVE_SECURITY
        {
            // Enable unknown clients to reach this reader
            BaseReader::downcast(endpoints.reader.reader_)->allow_unknown_writers();
        }

#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(reader, false);
#endif // if HAVE_SECURITY
    }
    // Could not create PDP Reader, so return false
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "PDPServer Reader creation failed");
        endpoints.reader.release();
        return false;
    }

    /***********************************
    * PDP WRITER
    ***********************************/

    // PDP Writer History
    hatt.payloadMaxSize = mp_builtin->m_att.writerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    endpoints.writer.history_.reset(new WriterHistory(hatt));

    // PDP Writer Attributes
    WriterAttributes watt = create_builtin_writer_attributes();

    // VOLATILE durability to highlight that on steady state the history is empty (except for announcement DATAs)
    // this setting is incompatible with CLIENTs TRANSIENT_LOCAL PDP readers but not validation is done on builitin
    // endpoints
    watt.endpoint.durabilityKind = durability_;

#if HAVE_SQLITE3
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            get_writer_persistence_file_name()));
#endif // HAVE_SQLITE3

    watt.mode = ASYNCHRONOUS_WRITER; //

    // Enable separate sending so the filter can be called for each change and reader proxy
    watt.separate_sending = true;
#if HAVE_SECURITY
    if (secure)
    {
        watt.endpoint.security_attributes().is_submessage_protected = true;
        watt.endpoint.security_attributes().plugin_endpoint_attributes =
                PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
#endif // HAVE_SECURITY

    // Create PDP Writer
    RTPSWriter* wout = nullptr;
#if HAVE_SECURITY
    EntityId_t writer_entity = secure ? c_EntityId_spdp_reliable_participant_secure_writer : c_EntityId_SPDPWriter;
#else
    EntityId_t writer_entity = c_EntityId_SPDPWriter;
#endif // if HAVE_SECURITY
    if (mp_RTPSParticipant->createWriter(&wout, watt, endpoints.writer.history_.get(), nullptr, writer_entity, true))
    {
        endpoints.writer.writer_ = dynamic_cast<fastdds::rtps::StatefulWriter*>(wout);

#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
#endif // if HAVE_SECURITY

        // Set pdp filter to writer
        IReaderDataFilter* pdp_filter = static_cast<ddb::PDPDataFilter<ddb::DiscoveryDataBase>*>(&discovery_db_);
        wout->reader_data_filter(pdp_filter);
    }
    // Could not create PDP Writer, so return false
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "PDPServer Writer creation failed");
        endpoints.writer.release();
        return false;
    }
    // TODO check if this should be done here or before this point in creation
    endpoints.writer.history_->remove_all_changes();

    // Ensure output channels are open in the transport for the corresponding locators
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        auto entry = LocatorSelectorEntry::create_fully_selected_entry(
            mp_builtin->m_DiscoveryServers);
        mp_RTPSParticipant->createSenderResources(entry);
    }

    return true;
}

void PDPServer::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data);

    auto discovery_config = getRTPSParticipant()->get_attributes().builtin.discovery_config;

    if (discovery_config.discoveryProtocol != DiscoveryProtocol::SERVER &&
            discovery_config.discoveryProtocol != DiscoveryProtocol::BACKUP)
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Using a PDP Server object with another user's settings");
    }

    // A PDP server should always be provided with all EDP endpoints
    // because it must relay all clients EDP info
    participant_data->m_available_builtin_endpoints
        |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER
            | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR
            | DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR
            | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
#if HAVE_SECURITY
    if (getRTPSParticipant()->is_secure())
    {
        participant_data->m_available_builtin_endpoints
            |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER
                | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR
                | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER
                | DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    }
#endif //HAVE_SECURITY

    const SimpleEDPAttributes& se = discovery_config.m_simpleEDP;

    if (!(se.use_PublicationWriterANDSubscriptionReader && se.use_PublicationReaderANDSubscriptionWriter))
    {
        EPROSIMA_LOG_WARNING(RTPS_PDP_SERVER, "SERVER or BACKUP PDP requires always all EDP endpoints creation.");
    }

    // Set discovery server version property
    participant_data->properties.push_back(
        std::pair<std::string,
        std::string>({dds::parameter_property_ds_version, dds::parameter_property_current_ds_version}));
}

void PDPServer::match_reliable_pdp_endpoints(
        const ParticipantProxyData& pdata)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    uint32_t endp = pdata.m_available_builtin_endpoints;
    bool use_multicast_locators = !mp_RTPSParticipant->get_attributes().builtin.avoid_builtin_multicast ||
            pdata.metatraffic_locators.unicast.empty();

    // Only SERVER and CLIENT participants will be received. All builtin must be there
    uint32_t auxendp = endp &
            (DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
            DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER);
    if (0 != auxendp)
    {
        auto temp_writer_data = get_temporary_writer_proxies_pool().get();

        temp_writer_data->clear();
        temp_writer_data->guid.guidPrefix = pdata.guid.guidPrefix;
        temp_writer_data->guid.entityId = endpoints->writer.writer_->getGuid().entityId;
        temp_writer_data->persistence_guid = pdata.get_persistence_guid();
        temp_writer_data->set_persistence_entity_id(c_EntityId_SPDPWriter);
        temp_writer_data->set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators,
                pdata.is_from_this_host());
        temp_writer_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
        temp_writer_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
#if HAVE_SECURITY
        if (should_protect_discovery())
        {
            mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                endpoints->reader.reader_->getGuid(), pdata.guid,
                *temp_writer_data, endpoints->reader.reader_->getAttributes().security_attributes());
        }
        else
#endif // HAVE_SECURITY
        {
            endpoints->reader.reader_->matched_writer_add_edp(*temp_writer_data);
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Participant " << pdata.guid.guidPrefix
                                                           << " did not send information about builtin writers");
        return;
    }

    // Only SERVER and CLIENT participants will be received. All builtin must be there
    auxendp = endp & (DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR);
    if (0 != auxendp)
    {
        auto temp_reader_data = get_temporary_reader_proxies_pool().get();

        temp_reader_data->clear();
        temp_reader_data->expects_inline_qos = false;
        temp_reader_data->guid.guidPrefix = pdata.guid.guidPrefix;
        temp_reader_data->guid.entityId = endpoints->reader.reader_->getGuid().entityId;
        temp_reader_data->set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators,
                pdata.is_from_this_host());
        temp_reader_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
        temp_reader_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
#if HAVE_SECURITY
        if (should_protect_discovery())
        {
            mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                endpoints->writer.writer_->getGuid(), pdata.guid,
                *temp_reader_data, endpoints->writer.writer_->getAttributes().security_attributes());
        }
        else
#endif // HAVE_SECURITY
        {
            endpoints->writer.writer_->matched_reader_add_edp(*temp_reader_data);
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Participant " << pdata.guid.guidPrefix
                                                           << " did not send information about builtin readers");
        return;
    }
}

void PDPServer::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    std::string part_type;
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // If the received participant GUID is from a server, update the servers list and DB and match the endpoints
        part_type = check_participant_type(pdata->properties);
        if (part_type == ParticipantType::SERVER || part_type == ParticipantType::BACKUP)
        {
            // Update DisvoveryDataBase
            discovery_db_.add_server(pdata->guid.guidPrefix);

#if HAVE_SECURITY
            if (!should_protect_discovery())
#endif // HAVE_SECURITY
            {
                match_pdp_writer_nts_(*pdata);
                match_pdp_reader_nts_(*pdata);
            }
        }
    }
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
            "Assigning remote endpoint for " << part_type << ": " << pdata->guid.guidPrefix);
    match_reliable_pdp_endpoints(*pdata);

#if HAVE_SECURITY
    if (mp_RTPSParticipant->security_manager().discovered_participant(*pdata))
#endif // HAVE_SECURITY
    {
        perform_builtin_endpoints_matching(*pdata);
    }

    // Send the Data(p) to the client
    if (part_type == ParticipantType::CLIENT || part_type == ParticipantType::SUPER_CLIENT)
    {
        send_own_pdp(pdata);
    }
}

void PDPServer::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata,
        bool /*notify_secure_endpoints*/)
{
    static_cast<void>(pdata);
#if HAVE_SECURITY
    match_reliable_pdp_endpoints(pdata);
#endif // HAVE_SECURITY
}

#if HAVE_SECURITY
bool PDPServer::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    if (local_reader == endpoints->reader.reader_->getGuid())
    {
        endpoints->reader.reader_->matched_writer_add_edp(remote_writer_data);
        return true;
    }

    return PDP::pairing_remote_writer_with_local_reader_after_security(local_reader, remote_writer_data);
}

bool PDPServer::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    if (local_writer == endpoints->writer.writer_->getGuid())
    {
        endpoints->writer.writer_->matched_reader_add_edp(remote_reader_data);
        return true;
    }

    return PDP::pairing_remote_reader_with_local_writer_after_security(local_writer, remote_reader_data);
}

#endif // HAVE_SECURITY

void PDPServer::perform_builtin_endpoints_matching(
        const ParticipantProxyData& pdata)
{
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata, true);
    }

    if (nullptr != mp_builtin->typelookup_manager_)
    {
        mp_builtin->typelookup_manager_->assign_remote_endpoints(pdata);
    }

    // Inform EDP of new RTPSParticipant data:
    if (mp_EDP != nullptr)
    {
        mp_EDP->assignRemoteEndpoints(pdata, true);
    }
}

void PDPServer::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "For RTPSParticipant: " << pdata->guid);
    uint32_t endp = pdata->m_available_builtin_endpoints;

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    if (endp & (DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER))
    {
        GUID_t writer_guid(pdata->guid.guidPrefix, endpoints->writer.writer_->getGuid().entityId);
        endpoints->reader.reader_->matched_writer_remove(writer_guid);
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Participant " << pdata->guid.guidPrefix
                                                           << " did not send information about builtin writers");
        return;
    }

    if (endp & (DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR))
    {
        GUID_t reader_guid(pdata->guid.guidPrefix, endpoints->reader.reader_->getGuid().entityId);
        endpoints->writer.writer_->matched_reader_remove(reader_guid);
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Participant " << pdata->guid.guidPrefix
                                                           << " did not send information about builtin readers");
        return;
    }

    // Remove remote server
    std::string part_type = check_participant_type(pdata->properties);
    if (part_type == ParticipantType::SERVER || part_type == ParticipantType::BACKUP)
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());
        discovery_db_.remove_server(pdata->guid.guidPrefix);
    }
}

std::ostringstream PDPServer::get_persistence_file_name_() const
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

std::string PDPServer::get_writer_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << "_writer.db";
    return filename.str();
}

std::string PDPServer::get_reader_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << "_reader.db";
    return filename.str();
}

std::string PDPServer::get_ddb_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << ".json";
    return filename.str();
}

std::string PDPServer::get_ddb_queue_persistence_file_name() const
{
    std::ostringstream filename = get_persistence_file_name_();
    filename << "_queue.json";
    return filename.str();
}

void PDPServer::announceParticipantState(
        bool new_change,
        bool dispose /* = false */,
        WriteParams& )
{
    if (enabled_)
    {
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                "Announcing Server " << mp_RTPSParticipant->getGuid() << " (new change: " << new_change << ")");
        CacheChange_t* change = nullptr;

        /*
           Protect writer sequence number. Make sure in order to prevent AB BA deadlock that the
           PDP mutex is systematically locked before the writer one (if needed):
            - transport callbacks on PDPListener
            - initialization and removal on BuiltinProtocols::initBuiltinProtocols and ~BuiltinProtocols
            - DSClientEvent (own thread)
            - ResendParticipantProxyDataPeriod (participant event thread)
         */

        getMutex()->lock();

        auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
        assert(endpoints->writer.writer_);

        fastdds::rtps::StatefulWriter& writer = *(endpoints->writer.writer_);
        WriterHistory& history = *endpoints->writer.history_;

        std::lock_guard<fastdds::RecursiveTimedMutex> wlock(writer.getMutex());

        if (!dispose)
        {
            // Create the CacheChange_t if necessary
            if (m_hasChangedLocalPDP.exchange(false) || new_change)
            {
                // Copy the participant data
                ParticipantProxyData proxy_data_copy(*getLocalParticipantProxyData());

                // Prepare identity
                WriteParams wp;
                SequenceNumber_t sn = history.next_sequence_number();
                {
                    SampleIdentity local;
                    local.writer_guid(writer.getGuid());
                    local.sequence_number(sn);
                    wp.sample_identity(local);
                    wp.related_sample_identity(local);
                }

                // Unlock PDP mutex since it's no longer needed.
                getMutex()->unlock();

                uint32_t cdr_size = proxy_data_copy.get_serialized_size(true);
                change = history.create_change(cdr_size, ALIVE, proxy_data_copy.m_key);
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

                    if (proxy_data_copy.write_to_cdr_message(&aux_msg, true))
                    {
                        change->serializedPayload.length = (uint16_t)aux_msg.length;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Cannot serialize ParticipantProxyData.");
                        return;
                    }

                    // Assign identity
                    change->sequenceNumber = sn;

                    // Create a RemoteLocatorList for metatraffic_locators
                    fastdds::rtps::RemoteLocatorList metatraffic_locators(
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

                    // Add our change to PDPWriterHistory
                    history.add_change(change, wp);
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
                        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER,
                                "DiscoveryDatabase already initialized with local DATA(p) on creation");
                        history.release_change(change);
                    }
                }
                // Doesn't make sense to send the DATA directly if it hasn't been introduced in the history yet (missing
                // sequence number).
                return;
            }
            else
            {
                // Unlock PDP mutex since it's no longer needed.
                getMutex()->unlock();

                // Retrieve the CacheChange_t from the database
                change = discovery_db().cache_change_own_participant();
                if (nullptr == change)
                {
                    // This case is when the local Server DATA(P) has been included already in database by update method
                    // but the routine thread has not consumed it yet.
                    // This would happen when the routine thread is busy in initializing, i.e. it already has other
                    // DATA(P) to parse before the own one is inserted by update.
                    EPROSIMA_LOG_WARNING(RTPS_PDP_SERVER, "Local Server DATA(p) uninitialized before local on announcement. "
                            << "It will be sent in next announce iteration.");
                    return;
                }
            }

            // Ping remote servers. It is done separately to avoid setting wrong remote_readers
            ping_remote_servers();
        }
        else
        {
            // Copy the participant data
            ParticipantProxyData* local_participant = getLocalParticipantProxyData();
            InstanceHandle_t key = local_participant->m_key;
            uint32_t cdr_size = local_participant->get_serialized_size(true);
            local_participant = nullptr;

            // Prepare identity
            WriteParams wp;
            SequenceNumber_t sn = history.next_sequence_number();
            {
                SampleIdentity local;
                local.writer_guid(writer.getGuid());
                local.sequence_number(sn);
                wp.sample_identity(local);
                wp.related_sample_identity(local);
            }

            // Unlock PDP mutex since it's no longer needed.
            getMutex()->unlock();

            // Generate the Data(Up)
            change = history.create_change(cdr_size, NOT_ALIVE_DISPOSED_UNREGISTERED, key);
            if (nullptr != change)
            {
                // Assign identity
                change->sequenceNumber = sn;
                change->write_params = std::move(wp);

                // Update the database with our own data
                if (discovery_db().update(change, ddb::DiscoveryParticipantChangeData()))
                {
                    // Distribute
                    awake_routine_thread();
                }
                else
                {
                    // Dispose if already there.
                    // It may happen if the participant is not removed fast enough
                    history.release_change(change);
                    return;
                }
            }
            else
            {
                // Failed to create the disposal change
                EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Server failed to create its DATA(Up)");
                return;
            }
        }

        assert(nullptr != change);

        // Force send the announcement

        // Create a list of receivers based on the remote participants known by the discovery database that are direct
        // clients or servers of this server. Add the locators of those remote participants.
        std::vector<GUID_t> remote_readers;
        LocatorList locators;

        std::vector<GuidPrefix_t> direct_clients_and_servers = discovery_db_.direct_clients_and_servers();
        for (GuidPrefix_t participant_prefix: direct_clients_and_servers)
        {
            // Add corresponding remote reader and locator
            remote_readers.emplace_back(participant_prefix, endpoints->reader.reader_->getGuid().entityId);
            locators.push_back(discovery_db_.participant_metatraffic_locators(participant_prefix));
        }

        //! Send announcement only if we have someone to inform
        if (!remote_readers.empty())
        {
            send_announcement(change, remote_readers, locators, dispose);
        }
    }
}

/**
 * This method removes a remote RTPSParticipant and all its writers and readers.
 * @param participant_guid GUID_t of the remote RTPSParticipant.
 * @param reason Why the participant is being removed (dropped vs removed)
 * @return true if correct.
 */
bool PDPServer::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryStatus reason)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    // Notify the DiscoveryDataBase on lease duration removal because the listener
    // has already notified the database in all other cases
    if (ParticipantDiscoveryStatus::DROPPED_PARTICIPANT == reason)
    {
        CacheChange_t* pC = nullptr;

        // TODO check in standard if DROP payload is always 0
        // We create the drop from Reader to make release simplier
        endpoints->reader.reader_->reserve_cache(mp_builtin->m_att.writerPayloadSize, pC);

        // We must create the corresponding DATA(p[UD])
        if (nullptr != pC)
        {
            pC->instanceHandle = partGUID;
            pC->kind = NOT_ALIVE_DISPOSED_UNREGISTERED;
            pC->writerGUID = endpoints->writer.writer_->getGuid();
            // Reset the internal CacheChange_t union.
            pC->writer_info.next = nullptr;
            pC->writer_info.previous = nullptr;
            pC->writer_info.num_sent_submessages = 0;

            // Use this server identity in order to hint clients it's a lease duration demise
            WriteParams& wp = pC->write_params;
            SampleIdentity local;
            local.writer_guid(endpoints->writer.writer_->getGuid());
            local.sequence_number(endpoints->writer.history_->next_sequence_number());
            wp.sample_identity(local);
            wp.related_sample_identity(local);

            // Notify the database
            if (discovery_db_.update(pC, ddb::DiscoveryParticipantChangeData()))
            {
                // Ensure processing time for the cache
                awake_routine_thread();

                // The discovery database takes ownership of the CacheChange_t
                // Henceforth, there are no references to the CacheChange_t
            }
            else
            {
                // If the database doesn't take the ownership remove
                endpoints->reader.reader_->release_cache(pC);
            }
        }
    }

    // Resend participant announcements to try to reconnect faster
    resend_ininitial_announcements();

    // Delegate into the base class for inherited proxy database removal
    return PDP::remove_remote_participant(partGUID, reason);
}

bool PDPServer::process_data_queues()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "process_data_queues start");
    discovery_db_.process_pdp_data_queue();
    return discovery_db_.process_edp_data_queue();
}

void PDPServer::awake_routine_thread(
        double interval_ms /*= 0*/)
{
    routine_->update_interval_millisec(interval_ms);
    routine_->cancel_timer();
    routine_->restart_timer();
}

bool PDPServer::server_update_routine()
{
    // There is pending work to be done by the server if there are changes that have not been acknowledged.
    bool pending_work = true;

    // Must lock the mutes to unlock it in the loop
    discovery_db().lock_incoming_data();

    // Execute the server routine
    do
    {
        discovery_db().unlock_incoming_data();

        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "");
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "-------------------- Server routine start --------------------");
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                "-------------------- " << mp_RTPSParticipant->getGuid() << " --------------------");

        process_writers_acknowledgements();     // server + ddb(functor_with_ddb)
        process_data_queues();                  // all ddb
        process_dirty_topics();                 // all ddb
        process_changes_release();              // server + ddb(changes_to_release(), clear_changes_to_release())
        process_disposals();                    // server + ddb(changes_to_dispose(), clear_changes_to_disposes())
        process_to_send_lists();                // server + ddb(get_to_send, remove_to_send_this)
        pending_work = pending_ack();           // all server

        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                "-------------------- " << mp_RTPSParticipant->getGuid() << " --------------------");
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "-------------------- Server routine end --------------------");
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "");

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
    if (durability_ == TRANSIENT && discovery_db_.is_enabled())
    {
        process_backup_store();
    }
    // Unlock the incoming data after finishing the backuo storage
    discovery_db().unlock_incoming_data();

    return pending_work && discovery_db_.is_enabled();
}

void PDPServer::update_remote_servers_list()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    if (!endpoints->reader.reader_ || !endpoints->writer.writer_)
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Cannot update server list within an uninitialized Server");
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(*getMutex());

        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // Create resources for remote servers. If a sender resource is already created, this step will be skipped for
        // that locator.
        auto entry = LocatorSelectorEntry::create_fully_selected_entry(
            mp_builtin->m_DiscoveryServers);
        mp_RTPSParticipant->createSenderResources(entry);
    }

    // Need to reactivate the server thread to send the DATA(p) to the new servers
    resend_ininitial_announcements();
}

bool PDPServer::process_writers_acknowledgements()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "process_writers_acknowledgements start");

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    // Execute first ACK for endpoints because PDP acked changes relevance in EDP,
    // which can result in false positives in EDP acknowledgements.

    /* EDP Subscriptions Writer's History */
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);
    bool pending = process_history_acknowledgement(edp->subscriptions_writer_.first, edp->subscriptions_writer_.second);

    /* EDP Publications Writer's History */
    pending |= process_history_acknowledgement(edp->publications_writer_.first, edp->publications_writer_.second);

    /* PDP Writer's History */
    pending |= process_history_acknowledgement(endpoints->writer.writer_, endpoints->writer.history_.get());

    return pending;
}

bool PDPServer::process_history_acknowledgement(
        fastdds::rtps::StatefulWriter* writer,
        fastdds::rtps::WriterHistory* writer_history)
{
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(writer->getMutex());

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

History::iterator PDPServer::process_change_acknowledgement(
        fastdds::rtps::History::iterator cit,
        fastdds::rtps::StatefulWriter* writer,
        fastdds::rtps::WriterHistory* writer_history)
{
    // DATA(p|w|r) case
    CacheChange_t* c = *cit;

    if (c->kind == fastdds::rtps::ChangeKind_t::ALIVE)
    {

        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Processing ack data alive " << c->instanceHandle);

        // If the change is a DATA(p), and it's the server's DATA(p), and the database knows that
        // it had been acked by all, then skip the change acked check for every reader proxy
        if (discovery_db_.is_participant(c) &&
                discovery_db_.guid_from_change(c) == mp_builtin->mp_participantImpl->getGuid() &&
                discovery_db_.server_acked_by_all())
        {
            EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                    "Server's DATA(p) already acked by all. Skipping check for every ReaderProxy");
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
                    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Removing change " << c->instanceHandle
                                                                          << " from history as it has been acked for everyone");
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
        if (writer->is_acked_by_all(c->sequenceNumber))
        {
            // Remove entry from `participants_|writers_|readers_`
            // AND put the change in changes_release
            discovery_db_.delete_entity_of_change(c);
            // Remove from writer's history.
            // Using `false` on `release` since the clean-up of the change is made by release_change of ddb
            return writer_history->remove_change(cit, false);
        }
    }

    // proceed to the next change
    return ++cit;
}

bool PDPServer::process_disposals()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "process_disposals start");

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);

    // Get list of disposals from database
    std::vector<fastdds::rtps::CacheChange_t*> disposals = discovery_db_.changes_to_dispose();
    // Iterate over disposals
    for (auto change: disposals)
    {
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Process disposal change from: " << change->instanceHandle);
        // No check is performed on whether the change is an actual disposal, leaving the responsibility of correctly
        // populating the disposals list to discovery_db_.process_data_queue().

        // Get the identity of the participant from which the change came.
        fastdds::rtps::GuidPrefix_t change_guid_prefix = discovery_db_.guid_from_change(change).guidPrefix;

        change->writerGUID.guidPrefix = endpoints->writer.writer_->getGuid().guidPrefix;

        // DATA(Up) case
        if (discovery_db_.is_participant(change))
        {
            // Lock PDP writer
            std::unique_lock<fastdds::RecursiveTimedMutex> lock(endpoints->writer.writer_->getMutex());

            // Remove all DATA(p) with the same sample identity as the DATA(Up) from PDP writer's history.
            discovery_db_.remove_related_alive_from_history_nts(endpoints->writer.history_.get(), change_guid_prefix);

            // Add DATA(Up) to PDP writer's history
            eprosima::fastdds::rtps::WriteParams wp = change->write_params;
            endpoints->writer.history_->add_change(change, wp);
        }
        // Data(Uw|Ur) case
        else
        {
            // Check whether disposals contains a DATA(Up) from the same participant as the DATA(Uw/r).
            // If it does, then there is no need of adding the DATA(Uw/r).
            bool should_publish_disposal = !announcement_from_same_participant_in_disposals(disposals,
                            change_guid_prefix);
            auto direct_participants = discovery_db_.direct_clients_and_servers();
            bool our_client =
                    (std::find(direct_participants.begin(), direct_participants.end(),
                    change_guid_prefix) != direct_participants.end());
            bool is_server_responsible =
                    (our_client || (change_guid_prefix == mp_RTPSParticipant->getGuid().guidPrefix));
            should_publish_disposal = should_publish_disposal && is_server_responsible;
            if (!edp->process_disposal(change, discovery_db_, change_guid_prefix, should_publish_disposal))
            {
                EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER_DISPOSAL,
                        "Wrong DATA received from disposals " << change->instanceHandle);
            }
        }
    }
    // Clear database disposals list
    discovery_db_.clear_changes_to_dispose();
    return false;
}

bool PDPServer::process_changes_release()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "process_changes_release start");
    process_changes_release_(discovery_db_.changes_to_release());
    discovery_db_.clear_changes_to_release();
    return false;
}

void PDPServer::process_changes_release_(
        const std::vector<fastdds::rtps::CacheChange_t*>& changes)
{
    // We will need the EDP publications/subscriptions writers, readers, and histories
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);

    // For each change to erase, first try to erase in case is in writer history and then it releases it
    for (auto ch : changes)
    {
        // Check if change owner is this participant. In that case, the change comes from a writer pool (PDP, EDP
        // publications or EDP subscriptions).
        // We compare the instance handle, as the only changes from our own server are its owns
        if (discovery_db().guid_from_change(ch) == endpoints->writer.writer_->getGuid())
        {
            if (discovery_db_.is_participant(ch))
            {
                // The change must return to the pool even if not present in the history
                // Normally Data(Up) will not be in history except in Own Server destruction
                if (!remove_change_from_writer_history(endpoints->writer.writer_, endpoints->writer.history_.get(), ch))
                {
                    endpoints->writer.history_->release_change(ch);
                }
            }
            else
            {
                bool ret = (discovery_db_.is_writer(ch) || discovery_db_.is_reader(ch));

                if (!ret || !edp->process_and_release_change(ch, false))
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Wrong DATA received to remove from this participant: "
                            << ch->instanceHandle);
                }
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
                remove_change_from_writer_history(
                    endpoints->writer.writer_,
                    endpoints->writer.history_.get(),
                    ch,
                    false);
                endpoints->reader.reader_->release_cache(ch);
            }
            else
            {
                bool ret = (discovery_db_.is_writer(ch) || discovery_db_.is_reader(ch));

                if (!ret || !edp->process_and_release_change(ch, true))
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Wrong DATA received to remove from this participant: "
                            << ch->instanceHandle);
                }
            }
        }
    }
}

bool PDPServer::announcement_from_same_participant_in_disposals(
        const std::vector<fastdds::rtps::CacheChange_t*>& disposals,
        const fastdds::rtps::GuidPrefix_t& participant)
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

bool PDPServer::process_dirty_topics()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "process_dirty_topics start");
    return discovery_db_.process_dirty_topics();
}

fastdds::rtps::ddb::DiscoveryDataBase& PDPServer::discovery_db()
{
    return discovery_db_;
}

const LocatorList_t& PDPServer::servers()
{
    return mp_builtin->m_DiscoveryServers;
}

bool PDPServer::process_to_send_lists()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "process_to_send_lists start");

    if (discovery_db_.updates_since_last_checked() > 0)
    {
        // Process pdp_to_send_
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Processing pdp_to_send");
        auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
        process_to_send_list(discovery_db_.pdp_to_send(), endpoints->writer.writer_, endpoints->writer.history_.get());
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                "Skiping sending PDP data because no entities have been discovered or updated");
    }
    discovery_db_.clear_pdp_to_send();

    // Process edp_publications_to_send_
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Processing edp_publications_to_send");
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);
    process_to_send_list(
        discovery_db_.edp_publications_to_send(),
        edp->publications_writer_.first,
        edp->publications_writer_.second);
    discovery_db_.clear_edp_publications_to_send();

    // Process edp_subscriptions_to_send_
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Processing edp_subscriptions_to_send");
    process_to_send_list(
        discovery_db_.edp_subscriptions_to_send(),
        edp->subscriptions_writer_.first,
        edp->subscriptions_writer_.second);
    discovery_db_.clear_edp_subscriptions_to_send();

    return false;
}

bool PDPServer::process_to_send_list(
        const std::vector<eprosima::fastdds::rtps::CacheChange_t*>& send_list,
        fastdds::rtps::RTPSWriter* writer,
        fastdds::rtps::WriterHistory* history)
{
    // Iterate over DATAs in send_list
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(writer->getMutex());
    for (auto change: send_list)
    {
        // If the DATA is already in the writer's history, then remove it, but do not release the change.
        remove_change_from_history_nts(history, change, false);
        // Set change's writer GUID so it matches with this writer
        change->writerGUID = writer->getGuid();
        // Add DATA to writer's history.
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Adding change from " << change->instanceHandle << " to history");
        eprosima::fastdds::rtps::WriteParams wp = change->write_params;
        history->add_change(change, wp);
    }
    return true;
}

bool PDPServer::remove_change_from_writer_history(
        fastdds::rtps::RTPSWriter* writer,
        fastdds::rtps::WriterHistory* history,
        fastdds::rtps::CacheChange_t* change,
        bool release_change /*= true*/)
{
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(writer->getMutex());
    return remove_change_from_history_nts(history, change, release_change);
}

bool PDPServer::remove_change_from_history_nts(
        fastdds::rtps::WriterHistory* history,
        fastdds::rtps::CacheChange_t* change,
        bool release_change /*= true*/)
{
    for (auto chit = history->changesRbegin(); chit != history->changesRend(); chit++)
    {
        // We compare by pointer because we maintain the same pointer everywhere and it is unique
        // We cannot compare by cache info because there is no distinct attributes for the same change arrived
        // from different servers, and one of them could be in the history while the other arrive to db
        if (change == (*chit))
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

bool PDPServer::pending_ack()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);
    bool ret = (!discovery_db_.server_acked_by_all() ||
            endpoints->writer.history_->getHistorySize() > 1 ||
            edp->publications_writer_.second->getHistorySize() > 0 ||
            edp->subscriptions_writer_.second->getHistorySize() > 0);

    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "PDP writer history length " << endpoints->writer.history_->getHistorySize());
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
            "is server " << endpoints->writer.writer_->getGuid() << " acked by all? " <<
            discovery_db_.server_acked_by_all());
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Are there pending changes? " << ret);
    return ret;
}

eprosima::fastdds::rtps::ResourceEvent& PDPServer::get_resource_event_thread()
{
    return resource_event_thread_;
}

void PDPServer::ping_remote_servers()
{
    LocatorList locators_ping;
    std::vector<GUID_t> remote_readers;

    // Iterate over the list of servers
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());
        // Get the info to send to these already known locators
        locators_ping = mp_builtin->m_DiscoveryServers;
    }

    if (!locators_ping.empty())
    {
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER,
                "Server " << getRTPSParticipant()->getGuid() << " PDP announcement");
        send_announcement(discovery_db().cache_change_own_participant(), remote_readers, locators_ping);
    }
}

void PDPServer::send_announcement(
        CacheChange_t* change,
        std::vector<GUID_t> remote_readers,
        LocatorList locators,
        bool dispose /* = false */)
{

    if (nullptr == change)
    {
        return;
    }

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);

    if (dispose)
    {
        RTPSMessageGroup group(getRTPSParticipant(), endpoints->writer.writer_, &sender);
        endpoints->writer.writer_->fastdds::rtps::StatefulWriter::increment_hb_count();
        group.add_heartbeat(
            change->sequenceNumber,
            change->sequenceNumber,
            endpoints->writer.writer_->get_heartbeat_count(),
            true,
            false);

        if (!group.add_data(*change, false))
        {
            EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error sending announcement from server to clients");
        }
    }
    else
    {
        FakeWriter writer(getRTPSParticipant(), c_EntityId_SPDPWriter);
        RTPSMessageGroup group(getRTPSParticipant(), &writer, &sender);
        if (!group.add_data(*change, false))
        {
            EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error sending announcement from server to clients");
        }
    }
}

void PDPServer::send_own_pdp(
        ParticipantProxyData* pdata)
{
    std::vector<GUID_t> remote_readers;
    LocatorList locators;

    remote_readers.emplace_back(pdata->guid.guidPrefix, c_EntityId_SPDPReader);

    for (auto& locator : pdata->metatraffic_locators.unicast)
    {
        locators.push_back(locator);
    }

    send_announcement(
        discovery_db().cache_change_own_participant(),
        remote_readers,
        locators
        );
}

bool PDPServer::read_backup(
        nlohmann::json& ddb_json,
        std::vector<nlohmann::json>& /* new_changes */)
{
    std::ifstream myfile;
    bool ret = true;
    try
    {
        myfile.open(get_ddb_persistence_file_name(), std::ios_base::in);
        // read json object
        myfile >> ddb_json;
        myfile.close();
    }
    catch (const std::exception& /* e */)
    {
        ret = false;
    }

    // TODO uncomment this part when recover queues is finish
    // try{
    //     myfile.open(get_ddb_queue_persistence_file_name(), std::ios_base::in);

    //     std::string line;
    //     while (std::getline(myfile, line))
    //     {
    //         nlohmann::json change_json = nlohmann::json::parse(line);

    //         // Read every change, and store it in json format in a vector
    //         new_changes.push_back(change_json);
    //     }

    //     myfile.close();
    // }
    // catch(const std::exception& e)
    // {
    //     return ret;
    // }
    return ret;
}

bool PDPServer::process_backup_discovery_database_restore(
        nlohmann::json& j)
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Restoring DiscoveryDataBase from backup");

    // We need every listener to resend the changes of every entity (ALIVE) in the DDB, so the PaticipantProxy
    // is restored
    EDPServer* edp = static_cast<EDPServer*>(mp_EDP);
    EDPServerPUBListener* edp_pub_listener = static_cast<EDPServerPUBListener*>(edp->publications_listener_);
    EDPServerSUBListener* edp_sub_listener = static_cast<EDPServerSUBListener*>(edp->subscriptions_listener_);

    // These mutexes are necessary to send messages to the listeners
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    std::unique_lock<fastdds::RecursiveTimedMutex> lock(endpoints->reader.reader_->getMutex());
    std::unique_lock<fastdds::RecursiveTimedMutex> lock_edpp(edp->publications_reader_.first->getMutex());
    std::unique_lock<fastdds::RecursiveTimedMutex> lock_edps(edp->subscriptions_reader_.first->getMutex());

    // Auxiliar variables to load info from json
    std::map<eprosima::fastdds::rtps::InstanceHandle_t, fastdds::rtps::CacheChange_t*> changes_map;
    fastdds::rtps::SampleIdentity sample_identity_aux;
    uint32_t length = 0;
    fastdds::rtps::CacheChange_t* change_aux;

    try
    {
        // Create every participant change. If it is external creates it from Reader,
        // if it is from the server, it is created from the writer
        for (auto it = j["participants"].begin(); it != j["participants"].end(); ++it)
        {
            length = it.value()["change"]["serialized_payload"]["length"].get<std::uint32_t>();
            std::istringstream(it.value()["change"]["sample_identity"].get<std::string>()) >> sample_identity_aux;

            // Reserve memory for new change. There will not be changes from own server
            if (!endpoints->reader.reader_->reserve_cache(length, change_aux))
            {
                EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
                // TODO release changes and exit
            }

            // Deserialize from json to change already created
            ddb::from_json(it.value()["change"], *change_aux);

            // Insert into the map so the DDB can store it
            changes_map.insert(
                std::make_pair(change_aux->instanceHandle, change_aux));

            // If the change was read as is_local we must pass it to listener with his own writer_guid
            if (it.value()["is_local"].get<bool>() &&
                    change_aux->write_params.sample_identity().writer_guid().guidPrefix !=
                    endpoints->writer.writer_->getGuid().guidPrefix &&
                    change_aux->kind == fastdds::rtps::ALIVE)
            {
                change_aux->writerGUID = change_aux->write_params.sample_identity().writer_guid();
                change_aux->sequenceNumber = change_aux->write_params.sample_identity().sequence_number();
                builtin_endpoints_->main_listener()->on_new_cache_change_added(endpoints->reader.reader_, change_aux);
            }
        }

        // Create every writer change. If it is external creates it from Reader,
        // if it is from the server, it is created from writer
        for (auto it = j["writers"].begin(); it != j["writers"].end(); ++it)
        {
            length = it.value()["change"]["serialized_payload"]["length"].get<std::uint32_t>();
            std::istringstream(it.value()["change"]["sample_identity"].get<std::string>()) >> sample_identity_aux;

            if (it.value()["topic"] == discovery_db().virtual_topic())
            {
                change_aux = new fastdds::rtps::CacheChange_t();
            }
            else
            {
                // Reserve memory for new change. There will not be changes from own server
                if (!edp->publications_reader_.first->reserve_cache(length, change_aux))
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }

            // Deserialize from json to change already created
            ddb::from_json(it.value()["change"], *change_aux);

            changes_map.insert(
                std::make_pair(change_aux->instanceHandle, change_aux));

            // TODO refactor for multiple servers
            // Should not send the virtual changes by the listener.
            // Should store in DDB if it is local even for endpoints.
            // Call listener to create proxy info for other entities different than server.
            if (change_aux->write_params.sample_identity().writer_guid().guidPrefix !=
                    endpoints->writer.writer_->getGuid().guidPrefix
                    && change_aux->kind == fastdds::rtps::ALIVE
                    && it.value()["topic"] != discovery_db().virtual_topic())
            {
                edp_pub_listener->on_new_cache_change_added(edp->publications_reader_.first, change_aux);
            }
        }

        // Create every reader change. If it is external creates it from Reader,
        // if it is created from the server, it is created from writer
        for (auto it = j["readers"].begin(); it != j["readers"].end(); ++it)
        {
            std::istringstream(it.value()["change"]["sample_identity"].get<std::string>()) >> sample_identity_aux;

            if (it.value()["topic"] == discovery_db().virtual_topic())
            {
                change_aux = new fastdds::rtps::CacheChange_t();
            }
            else
            {
                // Reserve memory for new change. There will not be changes from own server
                if (!edp->subscriptions_reader_.first->reserve_cache(length, change_aux))
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
                    // TODO release changes and exit
                }
            }

            // Deserialize from json to change already created
            ddb::from_json(it.value()["change"], *change_aux);

            changes_map.insert(
                std::make_pair(change_aux->instanceHandle, change_aux));

            // Call listener to create proxy info for other entities different than server
            if (change_aux->write_params.sample_identity().writer_guid().guidPrefix !=
                    endpoints->writer.writer_->getGuid().guidPrefix
                    && change_aux->kind == fastdds::rtps::ALIVE
                    && it.value()["topic"] != discovery_db().virtual_topic())
            {
                edp_sub_listener->on_new_cache_change_added(edp->subscriptions_reader_.first, change_aux);
            }
        }

        // Load database
        discovery_db_.from_json(j, changes_map);
    }
    catch (std::ios_base::failure&)
    {
        // TODO clean changes in case it has been an error
        EPROSIMA_LOG_ERROR(DISCOVERY_DATABASE, "BACKUP CORRUPTED");
        return false;
    }
    return true;
}

bool PDPServer::process_backup_restore_queue(
        std::vector<nlohmann::json>& /* new_changes */)
{
    // fastdds::rtps::SampleIdentity sample_identity_aux;
    // fastdds::rtps::InstanceHandle_t instance_handle_aux;
    // uint32_t length;

    // EDPServer* edp = static_cast<EDPServer*>(mp_EDP);
    // EDPServerPUBListener* edp_pub_listener = static_cast<EDPServerPUBListener*>(edp->publications_listener_);
    // EDPServerSUBListener* edp_sub_listener = static_cast<EDPServerSUBListener*>(edp->subscriptions_listener_);

    // std::unique_lock<fastdds::RecursiveTimedMutex> lock(endpoints->reader.reader_->getMutex());
    // std::unique_lock<fastdds::RecursiveTimedMutex> lock_edpp(edp->publications_reader_.first->getMutex());
    // std::unique_lock<fastdds::RecursiveTimedMutex> lock_edps(edp->subscriptions_reader_.first->getMutex());

    // TODO uncomment this funcionality and update with pools when queue functionality is implemented
    // try
    // {
    //     // Read every change and push it to the listener that it belongs
    //     for (nlohmann::json& json_change : new_changes)
    //     {
    //         // std::cout << json_change << std::endl;
    //         fastdds::rtps::CacheChange_t* change_aux;
    //         length = json_change["serialized_payload"]["length"].get<std::uint32_t>();
    //         (std::istringstream) json_change["sample_identity"].get<std::string>() >> sample_identity_aux;
    //         (std::istringstream) json_change["instance_handle"].get<std::string>() >> instance_handle_aux;

    //         // Belongs to own server
    //         if (sample_identity_aux.writer_guid() == endpoints->writer.writer_->getGuid())
    //         {
    //             if (discovery_db_.is_participant(iHandle2GUID(instance_handle_aux)))
    //             {
    //                 if (!endpoints->writer.history_->reserve_Cache(&change_aux, length))
    //                             [this]() -> uint32_t
    //                             {
    //                                 return mp_PDP->builtin_attributes().readerPayloadSize;
    //                             },
    //                             &change_aux, length))
    //                 {
    //                     EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
    //                     // TODO release changes and exit
    //                 }
    //                 else
    //                 {
    //                     ddb::from_json(json_change, *change_aux);
    //                     listener_->on_new_cache_change_added(endpoints->reader.reader_, change_aux);
    //                 }

    //             }
    //             else if (discovery_db_.is_writer(iHandle2GUID(instance_handle_aux)))
    //             {
    //                 if (!edp->publications_writer_.second->reserve_Cache(&change_aux, length))
    //                 {
    //                     EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
    //                     // TODO release changes and exit
    //                 }
    //                 else
    //                 {
    //                     ddb::from_json(json_change, *change_aux);
    //                     edp_pub_listener->on_new_cache_change_added(edp->publications_reader_.first, change_aux);
    //                 }
    //             }
    //             else if (discovery_db_.is_reader(iHandle2GUID(instance_handle_aux)))
    //             {
    //                 if (!edp->subscriptions_writer_.second->reserve_Cache(&change_aux, length))
    //                 {
    //                     EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
    //                     // TODO release changes and exit
    //                 }
    //                 else
    //                 {
    //                     ddb::from_json(json_change, *change_aux);
    //                     edp_sub_listener->on_new_cache_change_added(edp->subscriptions_reader_.first, change_aux);
    //                 }
    //             }
    //         }
    //         // It came from outside
    //         else
    //         {
    //             if (discovery_db_.is_participant(iHandle2GUID(instance_handle_aux)))
    //             {
    //                 if (!endpoints->reader.history_->reserve_Cache(&change_aux, length))
    //                 {
    //                     EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
    //                     // TODO release changes and exit
    //                 }
    //                 else
    //                 {
    //                     ddb::from_json(json_change, *change_aux);
    //                     listener_->on_new_cache_change_added(endpoints->reader.reader_, change_aux);
    //                 }

    //             }
    //             else if (discovery_db_.is_writer(iHandle2GUID(instance_handle_aux)))
    //             {
    //                 if (!edp->publications_reader_.second->reserve_Cache(&change_aux, length))
    //                 {
    //                     EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
    //                     // TODO release changes and exit
    //                 }
    //                 else
    //                 {
    //                     ddb::from_json(json_change, *change_aux);
    //                     edp_pub_listener->on_new_cache_change_added(edp->publications_reader_.first, change_aux);
    //                 }
    //             }
    //             else if (discovery_db_.is_reader(iHandle2GUID(instance_handle_aux)))
    //             {
    //                 if (!edp->subscriptions_reader_.second->reserve_Cache(&change_aux, length))
    //                 {
    //                     EPROSIMA_LOG_ERROR(RTPS_PDP_SERVER, "Error creating CacheChange");
    //                     // TODO release changes and exit
    //                 }
    //                 else
    //                 {
    //                     ddb::from_json(json_change, *change_aux);
    //                     edp_sub_listener->on_new_cache_change_added(edp->subscriptions_reader_.first, change_aux);
    //                 }
    //             }
    //         }
    //     }
    // }
    // catch (std::ios_base::failure&)
    // {
    //     // TODO clean changes in case it has been an error
    //     EPROSIMA_LOG_ERROR(DISCOVERY_DATABASE, "QUEUE BACKUP CORRUPTED");
    //     return false;
    // }
    return true;
}

void PDPServer::process_backup_store()
{
    EPROSIMA_LOG_INFO(DISCOVERY_DATABASE, "Dump DDB in json backup");

    // This will erase the last backup stored
    std::ofstream backup_json_file;
    backup_json_file.open(get_ddb_persistence_file_name(), std::ios_base::out);

    // Set j with the json from database dump
    nlohmann::json j;
    discovery_db().to_json(j);
    // setw makes pretty print for json
    backup_json_file << std::setw(4) << j << std::endl;
    backup_json_file.close();

    // Clear queue ddb backup
    discovery_db_.clean_backup();
}

void PDPServer::match_pdp_writer_nts_(
        const ParticipantProxyData& pdata)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_writer_data = get_temporary_writer_proxies_pool().get();

    temp_writer_data->clear();
    temp_writer_data->guid = { pdata.guid.guidPrefix, endpoints->writer.writer_->getGuid().entityId };
    temp_writer_data->set_remote_locators(pdata.metatraffic_locators, network, true, pdata.is_from_this_host());
    temp_writer_data->durability.durabilityKind(durability_);
    temp_writer_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
#if HAVE_SECURITY
    if (should_protect_discovery())
    {
        mp_RTPSParticipant->security_manager().discovered_builtin_writer(
            endpoints->reader.reader_->getGuid(), { pdata.guid.guidPrefix, c_EntityId_RTPSParticipant },
            *temp_writer_data, endpoints->reader.reader_->getAttributes().security_attributes());
    }
    else
#endif // HAVE_SECURITY
    {
        endpoints->reader.reader_->matched_writer_add_edp(*temp_writer_data);
    }
}

void PDPServer::match_pdp_reader_nts_(
        const ParticipantProxyData& pdata)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_reader_data = get_temporary_reader_proxies_pool().get();

    temp_reader_data->clear();
    temp_reader_data->guid = { pdata.guid.guidPrefix, endpoints->reader.reader_->getGuid().entityId };
    temp_reader_data->set_remote_locators(pdata.metatraffic_locators, network, true, pdata.is_from_this_host());
    temp_reader_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_reader_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
#if HAVE_SECURITY
    if (should_protect_discovery())
    {
        mp_RTPSParticipant->security_manager().discovered_builtin_reader(
            endpoints->writer.writer_->getGuid(), { pdata.guid.guidPrefix, c_EntityId_RTPSParticipant },
            *temp_reader_data, endpoints->writer.writer_->getAttributes().security_attributes());
    }
    else
#endif // HAVE_SECURITY
    {
        endpoints->writer.writer_->matched_reader_add_edp(*temp_reader_data);
    }
}

void PDPServer::release_change_from_writer(
        eprosima::fastdds::rtps::CacheChange_t* change)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    endpoints->writer.history_->release_change(change);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
