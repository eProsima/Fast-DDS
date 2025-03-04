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
 * @file PDPClient.cpp
 *
 */

#include <rtps/builtin/discovery/participant/PDPClient.h>
#include <rtps/builtin/discovery/participant/PDPClientListener.hpp>

#include <algorithm>
#include <forward_list>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/discovery/endpoint/EDPClient.h>
#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/builtin/discovery/participant/DS/FakeWriter.hpp>
#include <rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.hpp>
#include <rtps/builtin/discovery/participant/PDPListener.h>
#include <rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/ReaderProxy.hpp>
#include <rtps/writer/StatefulWriter.hpp>
#include <utils/shared_mutex.hpp>
#include <rtps/common/GuidUtils.hpp>
#include <utils/SystemInfo.hpp>
#include <utils/TimeConversion.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

static void direct_send(
        RTPSParticipantImpl* participant,
        LocatorList& locators,
        std::vector<GUID_t>& remote_readers,
        CacheChange_t& change,
        fastdds::rtps::Endpoint& sender_endpt)
{
    DirectMessageSender sender(participant, &remote_readers, &locators);
    RTPSMessageGroup group(participant, &sender_endpt, &sender);
    if (!group.add_data(change, false))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Error sending announcement from client to servers");
    }
}

static void direct_send(
        RTPSParticipantImpl* participant,
        LocatorList& locators,
        CacheChange_t& change)
{
    FakeWriter writer(participant, c_EntityId_SPDPWriter);
    std::vector<GUID_t> remote_readers;
    direct_send(participant, locators, remote_readers, change, writer);
}

PDPClient::PDPClient(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation,
        bool super_client)
    : PDP(builtin, allocation)
    , mp_sync(nullptr)
    , _serverPing(false)
    , _super_client(super_client)
{
}

PDPClient::~PDPClient()
{
    if (mp_sync != nullptr)
    {
        delete mp_sync;
    }
}

void PDPClient::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data); // TODO: Remember that the PDP version USES security

    auto discovery_config = getRTPSParticipant()->get_attributes().builtin.discovery_config;

    if ((DiscoveryProtocol::CLIENT != discovery_config.discoveryProtocol) &&
            (DiscoveryProtocol::SUPER_CLIENT != discovery_config.discoveryProtocol))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Using a PDP client object with another user's settings");
    }

    if (discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    }

    if (discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    }

    // Set discovery server version property
    participant_data->properties.push_back(std::pair<std::string,
            std::string>({fastdds::dds::parameter_property_ds_version,
                          fastdds::dds::parameter_property_current_ds_version}));

#if HAVE_SECURITY
    if (discovery_config.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
    {
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    }

    if (discovery_config.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
    {
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
        participant_data->m_available_builtin_endpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    }
#endif // HAVE_SECURITY

}

bool PDPClient::init(
        RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    /* We keep using EPDSimple notwithstanding its method EDPSimple::assignRemoteEndpoints regards
       all server EDPs as TRANSIENT_LOCAL. Server builtin Writers are actually TRANSIENT.
       Currently this mistake is not an issue but must be kept in mind if further development
       justifies the creation of an EDPClient class.
     */
    mp_EDP = new EDPClient(this, mp_RTPSParticipant);
    if (!mp_EDP->initEDP(m_discovery))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Endpoint discovery configuration failed");
        return false;
    }

    mp_sync =
            new DSClientEvent(this, TimeConv::Duration_t2MilliSecondsDouble(
                        m_discovery.discovery_config.discoveryServer_client_syncperiod));
    mp_sync->restart_timer();

    return true;
}

ParticipantProxyData* PDPClient::createParticipantProxyData(
        const ParticipantProxyData& participant_data,
        const GUID_t&)
{
    std::unique_lock<std::recursive_mutex> lock(*getMutex());

    // Verify if this participant is a server
    bool is_server = false;
    std::string part_type = check_participant_type(participant_data.properties);
    if (part_type == ParticipantType::SERVER || part_type == ParticipantType::BACKUP)
    {
        is_server = true;
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.guid, is_server, &participant_data);
    if (pdata != nullptr)
    {
        // Clients only assert its server liveliness, other clients liveliness is provided
        // through server's PDP discovery data
        if (is_server)
        {
            pdata->lease_duration_event->update_interval(pdata->lease_duration);
            pdata->lease_duration_event->restart_timer();
        }
    }

    return pdata;
}

void PDPClient::update_builtin_locators()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    mp_builtin->updateMetatrafficLocators(endpoints->reader.reader_->getAttributes().unicastLocatorList);
}

bool PDPClient::createPDPEndpoints()
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
bool PDPClient::should_protect_discovery()
{
    return mp_RTPSParticipant->is_secure() && mp_RTPSParticipant->security_attributes().is_discovery_protected;
}

bool PDPClient::create_secure_ds_pdp_endpoints()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Beginning PDPClient Secure PDP Endpoints creation");

    auto endpoints = new fastdds::rtps::DiscoveryServerPDPEndpointsSecure();
    builtin_endpoints_.reset(endpoints);

    bool ret_val = create_ds_pdp_reliable_endpoints(*endpoints, true) && create_ds_pdp_best_effort_reader(*endpoints);

    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "PDPClient Secure PDP Endpoints creation finished");

    return ret_val;
}

bool PDPClient::create_ds_pdp_best_effort_reader(
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

    endpoints.stateless_reader.listener_.reset(new PDPSecurityInitiatorListener(this));

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

bool PDPClient::create_ds_pdp_endpoints()
{
    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "Beginning PDPCLient Endpoints creation");

    auto endpoints = new fastdds::rtps::DiscoveryServerPDPEndpoints();
    builtin_endpoints_.reset(endpoints);

    bool ret_val = create_ds_pdp_reliable_endpoints(*endpoints, false);

    EPROSIMA_LOG_INFO(RTPS_PDP_SERVER, "PDPCLient Endpoints creation finished");

    return ret_val;
}

bool PDPClient::create_ds_pdp_reliable_endpoints(
        DiscoveryServerPDPEndpoints& endpoints,
        bool is_discovery_protected)
{

    EPROSIMA_LOG_INFO(RTPS_PDP, "Beginning PDPClient Endpoints creation");

    /***********************************
    * PDP READER
    ***********************************/

    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    endpoints.reader.history_.reset(new ReaderHistory(hatt));

    ReaderAttributes ratt = create_builtin_reader_attributes();

#if HAVE_SECURITY
    if (is_discovery_protected)
    {
        ratt.endpoint.security_attributes().is_submessage_protected = true;
        ratt.endpoint.security_attributes().plugin_endpoint_attributes =
                PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
#endif // HAVE_SECURITY

    endpoints.reader.listener_.reset(new PDPClientListener(this));

    RTPSReader* reader = nullptr;
#if HAVE_SECURITY
    EntityId_t reader_entity =
            is_discovery_protected ? c_EntityId_spdp_reliable_participant_secure_reader : c_EntityId_SPDPReader;
#else
    EntityId_t reader_entity = c_EntityId_SPDPReader;
#endif // if HAVE_SECURITY
    if (mp_RTPSParticipant->createReader(&reader, ratt, endpoints.reader.history_.get(),
            endpoints.reader.listener_.get(),
            reader_entity, true, false))
    {
        endpoints.reader.reader_ = dynamic_cast<fastdds::rtps::StatefulReader*>(reader);

#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(reader, false);
#endif // if HAVE_SECURITY
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "PDPClient Reader creation failed");
        endpoints.reader.release();
        return false;
    }

    /***********************************
    * PDP WRITER
    ***********************************/

    hatt.payloadMaxSize = mp_builtin->m_att.writerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    endpoints.writer.history_.reset(new WriterHistory(hatt));

    WriterAttributes watt = create_builtin_writer_attributes();

#if HAVE_SECURITY
    if (is_discovery_protected)
    {
        watt.endpoint.security_attributes().is_submessage_protected = true;
        watt.endpoint.security_attributes().plugin_endpoint_attributes =
                PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
#endif // HAVE_SECURITY

    RTPSWriter* wout = nullptr;
#if HAVE_SECURITY
    EntityId_t writer_entity =
            is_discovery_protected ? c_EntityId_spdp_reliable_participant_secure_writer : c_EntityId_SPDPWriter;
#else
    EntityId_t writer_entity = c_EntityId_SPDPWriter;
#endif // if HAVE_SECURITY
    if (mp_RTPSParticipant->createWriter(&wout, watt, endpoints.writer.history_.get(), nullptr, writer_entity, true))
    {
        endpoints.writer.writer_ = dynamic_cast<fastdds::rtps::StatefulWriter*>(wout);

#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
#endif // if HAVE_SECURITY
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "PDPClient Writer creation failed");
        endpoints.writer.release();
        return false;
    }

    // Ensure output channels are open in the transport for the corresponding locators
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        auto entry = LocatorSelectorEntry::create_fully_selected_entry(
            mp_builtin->m_DiscoveryServers);
        mp_RTPSParticipant->createSenderResources(entry);

        // If SECURITY is disabled, this condition is ALWAYS true
        if (!is_discovery_protected)
        {
            BaseReader::downcast(endpoints.reader.reader_)->allow_unknown_writers();
        }
    }

    EPROSIMA_LOG_INFO(RTPS_PDP, "PDPClient Endpoints creation finished");
    return true;
}

void PDPClient::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    bool ignored = false;
    notify_and_maybe_ignore_new_participant(pdata, ignored);
    if (!ignored)
    {
        {
            eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

            std::string part_type = check_participant_type(pdata->properties);
            if (part_type == ParticipantType::SERVER || part_type == ParticipantType::BACKUP)
            {
                // Add new servers to the connected list
                EPROSIMA_LOG_INFO(RTPS_PDP_CLIENT, "Server [" << pdata->guid.guidPrefix << "] matched.");
                RemoteServerAttributes server;
                server.guidPrefix = pdata->guid.guidPrefix;
                for (const Locator_t& locator : pdata->metatraffic_locators.multicast)
                {
                    server.metatrafficMulticastLocatorList.push_back(locator);
                }
                for (const Locator_t& locator : pdata->metatraffic_locators.unicast)
                {
                    server.metatrafficUnicastLocatorList.push_back(locator);
                }
                connected_servers_.push_back(server);

                // Match incoming server
#if HAVE_SECURITY
                if (!should_protect_discovery())
#endif // HAVE_SECURITY
                {
                    match_pdp_writer_nts_(server, pdata->is_from_this_host());
                    match_pdp_reader_nts_(server, pdata->is_from_this_host());
                }
            }
        }

#if HAVE_SECURITY
        if (mp_RTPSParticipant->security_manager().discovered_participant(*pdata))
#endif // HAVE_SECURITY
        {
            perform_builtin_endpoints_matching(*pdata);
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_PDP, "Ignoring new participant " << pdata->guid);
    }
}

void PDPClient::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata,
        bool /*notify_secure_endpoints*/)
{
#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        std::string part_type = check_participant_type(pdata.properties);
        if (part_type == ParticipantType::SERVER || part_type == ParticipantType::BACKUP)
        {
            // Add new servers to the connected list
            EPROSIMA_LOG_INFO(RTPS_PDP_CLIENT, "Secure Server [" << pdata.guid.guidPrefix << "] matched.");
            RemoteServerAttributes server;
            server.guidPrefix = pdata.guid.guidPrefix;
            for (const Locator_t& locator : pdata.metatraffic_locators.multicast)
            {
                server.metatrafficMulticastLocatorList.push_back(locator);
            }
            for (const Locator_t& locator : pdata.metatraffic_locators.unicast)
            {
                server.metatrafficUnicastLocatorList.push_back(locator);
            }

            // Match incoming server
            match_pdp_writer_nts_(server, pdata.is_from_this_host());
            match_pdp_reader_nts_(server, pdata.is_from_this_host());
        }
    }
#endif // HAVE_SECURITY

    perform_builtin_endpoints_matching(pdata);
}

#if HAVE_SECURITY
bool PDPClient::pairing_remote_writer_with_local_reader_after_security(
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

bool PDPClient::pairing_remote_reader_with_local_writer_after_security(
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

void PDPClient::perform_builtin_endpoints_matching(
        const ParticipantProxyData& pdata)
{
    // No EDP notification needed. EDP endpoints would be match when PDP synchronization is granted
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata, true);
    }

    if (nullptr != mp_builtin->typelookup_manager_)
    {
        mp_builtin->typelookup_manager_->assign_remote_endpoints(pdata);
    }
}

void PDPClient::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    // EDP endpoints have been already unmatch by the associated listener
    assert(!mp_EDP->areRemoteEndpointsMatched(pdata));

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    bool is_server = false;
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // Verify if this participant is a server
        auto it = connected_servers_.begin();
        while (it != connected_servers_.end())
        {
            if (it->guidPrefix == pdata->guid.guidPrefix)
            {
                std::unique_lock<std::recursive_mutex> lock(*getMutex());
                it = connected_servers_.erase(it);
                is_server = true;
                mp_sync->restart_timer(); // enable announcement and sync mechanism till this server reappears

                // Avoid incrementing iterator after item removal
                continue;
            }

            ++it;
        }
    }

    if (is_server)
    {
        // We should unmatch and match the PDP endpoints to renew the PDP reader and writer associated proxies
        EPROSIMA_LOG_INFO(RTPS_PDP, "For unmatching for server: " << pdata->guid);
        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        uint32_t endp = pdata->m_available_builtin_endpoints;
        uint32_t auxendp = endp;
        auxendp &= (DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER);

        if (auxendp != 0)
        {
            GUID_t wguid;

            wguid.guidPrefix = pdata->guid.guidPrefix;
            wguid.entityId = endpoints->writer.writer_->getGuid().entityId;
            endpoints->reader.reader_->matched_writer_remove(wguid);

#if HAVE_SECURITY
            if (!should_protect_discovery())
#endif // HAVE_SECURITY
            {
                // Rematch but discarding any previous state of the server
                // because we know the server shutdown intentionally
                auto temp_writer_data = get_temporary_writer_proxies_pool().get();

                temp_writer_data->clear();
                temp_writer_data->guid = wguid;
                temp_writer_data->persistence_guid = pdata->get_persistence_guid();
                temp_writer_data->set_persistence_entity_id(c_EntityId_SPDPWriter);
                temp_writer_data->set_remote_locators(pdata->metatraffic_locators, network, true,
                        pdata->is_from_this_host());
                temp_writer_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
                temp_writer_data->durability.kind = dds::TRANSIENT_DURABILITY_QOS;
                endpoints->reader.reader_->matched_writer_add_edp(*temp_writer_data);
            }
        }

        auxendp = endp;
        auxendp &= (DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR);

        if (auxendp != 0)
        {
            GUID_t rguid;
            rguid.guidPrefix = pdata->guid.guidPrefix;
            rguid.entityId = endpoints->reader.reader_->getGuid().entityId;
            endpoints->writer.writer_->matched_reader_remove(rguid);

#if HAVE_SECURITY
            if (!should_protect_discovery())
#endif // HAVE_SECURITY
            {
                auto temp_reader_data = get_temporary_reader_proxies_pool().get();

                temp_reader_data->clear();
                temp_reader_data->expects_inline_qos = false;
                temp_reader_data->guid = rguid;
                temp_reader_data->set_remote_locators(pdata->metatraffic_locators, network, true,
                        pdata->is_from_this_host());
                temp_reader_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
                temp_reader_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
                endpoints->writer.writer_->matched_reader_add_edp(*temp_reader_data);
            }
        }
    }
}

bool PDPClient::all_servers_acknowledge_PDP()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    // Check if already initialized
    assert(endpoints->writer.history_ && endpoints->writer.writer_);

    // Get a reference to client proxy data
    CacheChange_t* pPD;
    if (endpoints->writer.history_->get_min_change(&pPD))
    {
        return endpoints->writer.writer_->is_acked_by_all(pPD->sequenceNumber);
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "ParticipantProxy data should have been added to client PDP history cache "
                "by a previous call to announceParticipantState()");
    }

    return false;
}

bool PDPClient::is_all_servers_PDPdata_updated()
{
    // Assess all server DATA has been received
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    assert(endpoints->reader.reader_);
    return endpoints->reader.reader_->is_in_clean_state();
}

void PDPClient::announceParticipantState(
        bool new_change,
        bool dispose,
        WriteParams& )
{
    if (enabled_)
    {
        auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
        fastdds::rtps::StatefulWriter& writer = *(endpoints->writer.writer_);
        WriterHistory& history = *endpoints->writer.history_;

        /*
           Protect writer sequence number. Make sure in order to prevent AB BA deadlock that the
           PDP mutex is systematically lock before the writer one (if needed):
            - transport callbacks on PDPListener
            - initialization and removal on BuiltinProtocols::initBuiltinProtocols and ~BuiltinProtocols
            - DSClientEvent (own thread)
            - ResendParticipantProxyDataPeriod (participant event thread)
         */

        std::lock_guard<std::recursive_mutex> lock(*getMutex());

        std::lock_guard<RecursiveTimedMutex> wlock(writer.getMutex());

        WriteParams wp;
        SampleIdentity local;
        local.writer_guid(writer.getGuid());
        local.sequence_number(history.next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        // Add the write params to the sample
        if (dispose)
        {
            // When the server is dying we must ensure that every client is sent at least a DATA(p).
            // Note here we can no longer receive and DATA or ACKNACK from clients.
            // In order to avoid that we send the message directly as in the standard stateless PDP.

            CacheChange_t* change = nullptr;
            change = history.create_change(
                mp_builtin->m_att.writerPayloadSize,
                NOT_ALIVE_DISPOSED_UNREGISTERED,
                getLocalParticipantProxyData()->m_key);

            if (nullptr != change)
            {
                // Update the sequence number
                change->sequenceNumber = history.next_sequence_number();
                change->write_params = wp;

                std::vector<GUID_t> remote_readers;
                LocatorList locators;

                //  TODO: modify announcement mechanism to allow direct message sending
                //for (auto it = pW->matchedReadersBegin(); it != pW->matchedReadersEnd(); ++it)
                //{
                //    RemoteReaderAttributes & att = (*it)->m_att;
                //    remote_readers.push_back(att.guid);

                //    EndpointAttributes & ep = att.endpoint;
                //    locators.push_back(ep.unicastLocatorList);
                //    //locators.push_back(ep.multicastLocatorList);
                //}
                {
                    // Temporary workaround
                    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

                    for (auto& svr: connected_servers_)
                    {
                        locators.push_back(svr.metatrafficUnicastLocatorList);
                        remote_readers.emplace_back(svr.guidPrefix,
                                endpoints->reader.reader_->getGuid().entityId);
                    }
                }

                if (!remote_readers.empty())
                {
                    direct_send(getRTPSParticipant(), locators, remote_readers, *change, *endpoints->writer.writer_);
                }
            }

            // Free change
            history.release_change(change);
        }
        else
        {
            PDP::announceParticipantState(history, new_change, dispose, wp);

            if (!new_change)
            {
                // Retrieve the participant discovery data
                CacheChange_t* pPD;
                if (history.get_min_change(&pPD))
                {
                    LocatorList locators;

                    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

                    // An already connected server will be pinged again if there exists a non-connected server (2 or more servers scenario).
                    // This is because we no longer use the GUID to match servers, so we cannot discern which servers are connected
                    // and which are not. We cannot map servers in m_DiscoveryServers to connected_servers_.

                    // Ping always not-connected servers. This is done to ensure ping is sent to new servers after a list update.
                    locators = mp_builtin->m_DiscoveryServers;

                    // Announce liveliness (lease duration) to all servers
                    if (!_serverPing)
                    {
                        for (auto& svr : connected_servers_)
                        {
                            locators.push_back(svr.metatrafficMulticastLocatorList);
                            locators.push_back(svr.metatrafficUnicastLocatorList);
                        }
                    }

                    direct_send(getRTPSParticipant(), locators, *pPD);

                    // Ping done independently of which triggered the announcement.
                    // Note all event callbacks are currently serialized
                    _serverPing = false;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP, "ParticipantProxy data should have been added to client PDP history "
                            "cache by a previous call to announceParticipantState()");
                }
            }
        }
    }
}

void PDPClient::update_remote_servers_list()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    if (!endpoints->reader.reader_ || !endpoints->writer.writer_)
    {
        EPROSIMA_LOG_ERROR(SERVER_CLIENT_DISCOVERY, "Cannot update server list within an uninitialized Client");
        return;
    }

#if HAVE_SECURITY
    if (!should_protect_discovery())
#endif  // HAVE_SECURITY
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // Create resources for remote servers. If a sender resource is already created, this step will be skipped for
        // that locator.
        auto entry = LocatorSelectorEntry::create_fully_selected_entry(
            mp_builtin->m_DiscoveryServers);
        mp_RTPSParticipant->createSenderResources(entry);

        BaseReader::downcast(endpoints->reader.reader_)->allow_unknown_writers();
    }
    // Make at least one ping to the new servers.
    _serverPing = true;
    WriteParams __wp = WriteParams::write_params_default();
    announceParticipantState(false, false, __wp);
    mp_sync->restart_timer();
}

void PDPClient::match_pdp_writer_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
        bool from_this_host)
{
    match_pdp_writer_nts_(server_att, server_att.guidPrefix, from_this_host);
}

void PDPClient::match_pdp_writer_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
        const eprosima::fastdds::rtps::GuidPrefix_t& prefix_override,
        bool from_this_host)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_writer_data = get_temporary_writer_proxies_pool().get();

    temp_writer_data->clear();
    temp_writer_data->guid = { prefix_override, endpoints->writer.writer_->getGuid().entityId };
    temp_writer_data->set_multicast_locators(server_att.metatrafficMulticastLocatorList, network, from_this_host);
    temp_writer_data->set_remote_unicast_locators(server_att.metatrafficUnicastLocatorList, network, from_this_host);
    temp_writer_data->durability.kind = dds::TRANSIENT_DURABILITY_QOS;
    temp_writer_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
#if HAVE_SECURITY
    if (should_protect_discovery())
    {
        mp_RTPSParticipant->security_manager().discovered_builtin_writer(
            endpoints->reader.reader_->getGuid(), { prefix_override, c_EntityId_RTPSParticipant },
            *temp_writer_data, endpoints->reader.reader_->getAttributes().security_attributes());
    }
    else
#endif // HAVE_SECURITY
    {
        endpoints->reader.reader_->matched_writer_add_edp(*temp_writer_data);
    }
}

void PDPClient::match_pdp_reader_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
        bool from_this_host)
{
    match_pdp_reader_nts_(server_att, server_att.guidPrefix, from_this_host);
}

void PDPClient::match_pdp_reader_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
        const eprosima::fastdds::rtps::GuidPrefix_t& prefix_override,
        bool from_this_host)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_reader_data = get_temporary_reader_proxies_pool().get();

    temp_reader_data->clear();
    temp_reader_data->guid = { prefix_override, endpoints->reader.reader_->getGuid().entityId };
    temp_reader_data->set_multicast_locators(server_att.metatrafficMulticastLocatorList, network, from_this_host);
    temp_reader_data->set_remote_unicast_locators(server_att.metatrafficUnicastLocatorList, network, from_this_host);
    temp_reader_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_reader_data->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
#if HAVE_SECURITY
    if (should_protect_discovery())
    {
        mp_RTPSParticipant->security_manager().discovered_builtin_reader(
            endpoints->writer.writer_->getGuid(), { prefix_override, c_EntityId_RTPSParticipant },
            *temp_reader_data, endpoints->writer.writer_->getAttributes().security_attributes());
    }
    else
#endif // HAVE_SECURITY
    {
        endpoints->writer.writer_->matched_reader_add_edp(*temp_reader_data);
    }
}

bool get_server_client_default_guidPrefix(
        int id,
        GuidPrefix_t& guid)
{
    if ( id >= 0
            && id < 256
            && std::istringstream(DEFAULT_ROS2_SERVER_GUIDPREFIX) >> guid)
    {
        // Third octet denotes the server id
        guid.value[2] = static_cast<octet>(id);

        return true;
    }

    return false;
}

bool PDPClient::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryStatus reason)
{
    if (PDP::remove_remote_participant(partGUID, reason))
    {
        // If it works fine, return
        return true;
    }

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    // Erase Proxies created before having the Participant
    GUID_t wguid;
    wguid.guidPrefix = partGUID.guidPrefix;
    wguid.entityId = endpoints->writer.writer_->getGuid().entityId;
    endpoints->reader.reader_->matched_writer_remove(wguid);

    GUID_t rguid;
    rguid.guidPrefix = partGUID.guidPrefix;
    rguid.entityId = endpoints->reader.reader_->getGuid().entityId;
    endpoints->writer.writer_->matched_reader_remove(rguid);

    // Reactivate ping routine
    mp_sync->restart_timer();

    return false;
}

const std::list<eprosima::fastdds::rtps::RemoteServerAttributes>& PDPClient::connected_servers()
{
    return connected_servers_;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
