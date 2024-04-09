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

#include <algorithm>
#include <forward_list>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/ReaderProxy.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/utils/shared_mutex.hpp>
#include <rtps/builtin/discovery/endpoint/EDPClient.h>
#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/builtin/discovery/participant/DS/FakeWriter.hpp>
#include <rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.hpp>
#include <rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <utils/SystemInfo.hpp>
#include <vector>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace fastrtps::rtps;

static void direct_send(
        RTPSParticipantImpl* participant,
        LocatorList& locators,
        std::vector<GUID_t>& remote_readers,
        const CacheChange_t& change,
        fastrtps::rtps::Endpoint& sender_endpt)
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
        const CacheChange_t& change)
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

    if (
        getRTPSParticipant()->getAttributes().builtin.discovery_config.discoveryProtocol
        != DiscoveryProtocol_t::CLIENT
        &&
        getRTPSParticipant()->getAttributes().builtin.discovery_config.discoveryProtocol
        != DiscoveryProtocol_t::SUPER_CLIENT    )
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Using a PDP client object with another user's settings");
    }

    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP.
                    use_PublicationWriterANDSubscriptionReader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    }

    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP.
                    use_PublicationReaderANDSubscriptionWriter)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    }

    // Set discovery server version property
    participant_data->m_properties.push_back(std::pair<std::string,
            std::string>({fastdds::dds::parameter_property_ds_version,
                          fastdds::dds::parameter_property_current_ds_version}));

#if HAVE_SECURITY
    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP
                    .enable_builtin_secure_publications_writer_and_subscriptions_reader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    }

    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP
                    .enable_builtin_secure_subscriptions_writer_and_publications_reader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
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

    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (data_matches_with_prefix(svr.guidPrefix, participant_data))
            {
                is_server = true;
            }
        }
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.m_guid, is_server, &participant_data);
    if (pdata != nullptr)
    {
        // Clients only assert its server lifeliness, other clients liveliness is provided
        // through server's PDP discovery data
        if (is_server)
        {
            pdata->lease_duration_event->update_interval(pdata->m_leaseDuration);
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
    const RTPSParticipantAttributes& pattr = mp_RTPSParticipant->getRTPSParticipantAttributes();

    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    endpoints.stateless_reader.history_.reset(new ReaderHistory(hatt));

    ReaderAttributes ratt;
    ratt.expectsInlineQos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.external_unicast_locators = mp_builtin->m_att.metatraffic_external_unicast_locators;
    ratt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    ratt.endpoint.topicKind = WITH_KEY;

    // change depending of backup mode
    ratt.endpoint.durabilityKind = VOLATILE;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;

    endpoints.stateless_reader.listener_.reset(new PDPSecurityInitiatorListener(this));

    // Create PDP Reader
    RTPSReader* reader = nullptr;
    if (mp_RTPSParticipant->createReader(&reader, ratt, endpoints.stateless_reader.history_.get(),
            endpoints.stateless_reader.listener_.get(), c_EntityId_SPDPReader, true, false))
    {
        endpoints.stateless_reader.reader_ = dynamic_cast<fastrtps::rtps::StatelessReader*>(reader);
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

    const RTPSParticipantAttributes& pattr = mp_RTPSParticipant->getRTPSParticipantAttributes();

    /***********************************
    * PDP READER
    ***********************************/

    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    endpoints.reader.history_.reset(new ReaderHistory(hatt));

    ReaderAttributes ratt;
    ratt.expectsInlineQos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.external_unicast_locators = mp_builtin->m_att.metatraffic_external_unicast_locators;
    ratt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.times.heartbeatResponseDelay = pdp_heartbeat_response_delay;
#if HAVE_SECURITY
    if (is_discovery_protected)
    {
        ratt.endpoint.security_attributes().is_submessage_protected = true;
        ratt.endpoint.security_attributes().plugin_endpoint_attributes =
                PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
#endif // HAVE_SECURITY

    endpoints.reader.listener_.reset(new PDPListener(this));

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
        endpoints.reader.reader_ = dynamic_cast<fastrtps::rtps::StatefulReader*>(reader);

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

    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    watt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    watt.endpoint.external_unicast_locators = mp_builtin->m_att.metatraffic_external_unicast_locators;
    watt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    watt.times.heartbeatPeriod = pdp_heartbeat_period;
    watt.times.nackResponseDelay = pdp_nack_response_delay;
    watt.times.nackSupressionDuration = pdp_nack_supression_duration;

#if HAVE_SECURITY
    if (is_discovery_protected)
    {
        watt.endpoint.security_attributes().is_submessage_protected = true;
        watt.endpoint.security_attributes().plugin_endpoint_attributes =
                PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    }
#endif // HAVE_SECURITY

    if (pattr.throughputController.bytesPerPeriod != UINT32_MAX && pattr.throughputController.periodMillisecs != 0)
    {
        watt.mode = ASYNCHRONOUS_WRITER;
    }

    RTPSWriter* wout = nullptr;
#if HAVE_SECURITY
    EntityId_t writer_entity =
            is_discovery_protected ? c_EntityId_spdp_reliable_participant_secure_writer : c_EntityId_SPDPWriter;
#else
    EntityId_t writer_entity = c_EntityId_SPDPWriter;
#endif // if HAVE_SECURITY
    if (mp_RTPSParticipant->createWriter(&wout, watt, endpoints.writer.history_.get(), nullptr, writer_entity, true))
    {
        endpoints.writer.writer_ = dynamic_cast<fastrtps::rtps::StatefulWriter*>(wout);

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

    // Perform matching with remote servers and ensure output channels are open in the transport for the corresponding
    // locators
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            auto entry = LocatorSelectorEntry::create_fully_selected_entry(
                it.metatrafficUnicastLocatorList, it.metatrafficMulticastLocatorList);
            mp_RTPSParticipant->createSenderResources(entry);

#if HAVE_SECURITY
            if (!mp_RTPSParticipant->is_secure())
            {
                match_pdp_writer_nts_(it);
                match_pdp_reader_nts_(it);
            }
            else if (!is_discovery_protected)
            {
                endpoints.reader.reader_->enableMessagesFromUnkownWriters(true);
            }
#else
            if (!is_discovery_protected)
            {
                match_pdp_writer_nts_(it);
                match_pdp_reader_nts_(it);
            }
#endif // HAVE_SECURITY
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

            // Verify if this participant is a server
            for (auto& svr : mp_builtin->m_DiscoveryServers)
            {
                if (data_matches_with_prefix(svr.guidPrefix, *pdata))
                {
                    svr.is_connected = true;
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
}

void PDPClient::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata,
        bool /*notify_secure_endpoints*/)
{
#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // Verify if this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (data_matches_with_prefix(svr.guidPrefix, pdata))
            {
                if (!svr.is_connected && nullptr != get_participant_proxy_data(svr.guidPrefix))
                {
                    //! mark proxy as connected from an unmangled prefix in case
                    //! it could not be done in assignRemoteEndpoints()
                    svr.is_connected = true;
                }

                match_pdp_reader_nts_(svr, pdata.m_guid.guidPrefix);
                match_pdp_writer_nts_(svr, pdata.m_guid.guidPrefix);
                break;
            }
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
        endpoints->reader.reader_->matched_writer_add(remote_writer_data);
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
        endpoints->writer.writer_->matched_reader_add(remote_reader_data);
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
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
                std::unique_lock<std::recursive_mutex> lock(*getMutex());
                svr.is_connected = false;
                is_server = true;
                mp_sync->restart_timer(); // enable announcement and sync mechanism till this server reappears
            }
        }
    }

    if (is_server)
    {
        // We should unmatch and match the PDP endpoints to renew the PDP reader and writer associated proxies
        EPROSIMA_LOG_INFO(RTPS_PDP, "For unmatching for server: " << pdata->m_guid);
        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        uint32_t endp = pdata->m_availableBuiltinEndpoints;
        uint32_t auxendp = endp;
        auxendp &= (DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER);

        if (auxendp != 0)
        {
            GUID_t wguid;

            wguid.guidPrefix = pdata->m_guid.guidPrefix;
            wguid.entityId = endpoints->writer.writer_->getGuid().entityId;
            endpoints->reader.reader_->matched_writer_remove(wguid);

#if HAVE_SECURITY
            if (!should_protect_discovery())
#endif // HAVE_SECURITY
            {
                // rematch but discarding any previous state of the server
                // because we know the server shutdown intentionally
                auto temp_writer_data = get_temporary_writer_proxies_pool().get();

                temp_writer_data->clear();
                temp_writer_data->guid(wguid);
                temp_writer_data->persistence_guid(pdata->get_persistence_guid());
                temp_writer_data->set_persistence_entity_id(c_EntityId_SPDPWriter);
                temp_writer_data->set_remote_locators(pdata->metatraffic_locators, network, true);
                temp_writer_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
                temp_writer_data->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
                endpoints->reader.reader_->matched_writer_add(*temp_writer_data);
            }
        }

        auxendp = endp;
        auxendp &= (DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR);

        if (auxendp != 0)
        {
            GUID_t rguid;
            rguid.guidPrefix = pdata->m_guid.guidPrefix;
            rguid.entityId = endpoints->reader.reader_->getGuid().entityId;
            endpoints->writer.writer_->matched_reader_remove(rguid);

#if HAVE_SECURITY
            if (!should_protect_discovery())
#endif // HAVE_SECURITY
            {
                auto temp_reader_data = get_temporary_reader_proxies_pool().get();

                temp_reader_data->clear();
                temp_reader_data->m_expectsInlineQos = false;
                temp_reader_data->guid(rguid);
                temp_reader_data->set_remote_locators(pdata->metatraffic_locators, network, true);
                temp_reader_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
                temp_reader_data->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
                endpoints->writer.writer_->matched_reader_add(*temp_reader_data);
            }
        }
    }
}

bool PDPClient::all_servers_acknowledge_PDP()
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());

    // check if already initialized
    assert(endpoints->writer.history_ && endpoints->writer.writer_);

    // get a reference to client proxy data
    CacheChange_t* pPD;
    if (endpoints->writer.history_->get_min_change(&pPD))
    {
        return endpoints->writer.writer_->is_acked_by_all(pPD);
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
    return endpoints->reader.reader_->isInCleanState();
}

void PDPClient::announceParticipantState(
        bool new_change,
        bool dispose,
        WriteParams& )
{
    if (enabled_)
    {
        auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
        fastrtps::rtps::StatefulWriter& writer = *(endpoints->writer.writer_);
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
            // we must assure when the server is dying that all client are send at least a DATA(p)
            // note here we can no longer receive and DATA or ACKNACK from clients.
            // In order to avoid that we send the message directly as in the standard stateless PDP

            CacheChange_t* change = nullptr;

            if ((change = writer.new_change(
                        [this]() -> uint32_t
                        {
                            return mp_builtin->m_att.writerPayloadSize;
                        },
                        NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key)))
            {
                // update the sequence number
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
                    // temporary workaround
                    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

                    for (auto& svr : mp_builtin->m_DiscoveryServers)
                    {
                        // if we are matched to a server report demise
                        if (svr.is_connected)
                        {
                            //locators.push_back(svr.metatrafficMulticastLocatorList);
                            locators.push_back(svr.metatrafficUnicastLocatorList);
                            remote_readers.emplace_back(svr.guidPrefix,
                                    endpoints->reader.reader_->getGuid().entityId);
                        }
                    }
                }

                if (!remote_readers.empty())
                {
                    direct_send(getRTPSParticipant(), locators, remote_readers, *change, *endpoints->writer.writer_);
                }
            }

            // free change
            writer.release_change(change);
        }
        else
        {
            PDP::announceParticipantState(writer, history, new_change, dispose, wp);

            if (!new_change)
            {
                // retrieve the participant discovery data
                CacheChange_t* pPD;
                if (history.get_min_change(&pPD))
                {
                    LocatorList locators;

                    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

                    for (auto& svr : mp_builtin->m_DiscoveryServers)
                    {
                        // non-pinging announcements like lease duration ones must be
                        // broadcast to all servers
                        if (!svr.is_connected || !_serverPing)
                        {
                            locators.push_back(svr.metatrafficMulticastLocatorList);
                            locators.push_back(svr.metatrafficUnicastLocatorList);
                        }
                    }

                    direct_send(getRTPSParticipant(), locators, *pPD);

                    // ping done independtly of which triggered the announcement
                    // note all event callbacks are currently serialized
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

        for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            if (!endpoints->reader.reader_->matched_writer_is_matched(it.GetPDPWriter()) ||
                    !endpoints->writer.writer_->matched_reader_is_matched(it.GetPDPReader()))
            {
                auto entry = LocatorSelectorEntry::create_fully_selected_entry(
                    it.metatrafficUnicastLocatorList, it.metatrafficMulticastLocatorList);
                mp_RTPSParticipant->createSenderResources(entry);
            }

            if (!endpoints->reader.reader_->matched_writer_is_matched(it.GetPDPWriter()))
            {
                match_pdp_writer_nts_(it);
            }

            if (!endpoints->writer.writer_->matched_reader_is_matched(it.GetPDPReader()))
            {
                match_pdp_reader_nts_(it);
            }
        }
    }
    mp_sync->restart_timer();
}

void PDPClient::match_pdp_writer_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att)
{
    match_pdp_writer_nts_(server_att, server_att.guidPrefix);
}

void PDPClient::match_pdp_writer_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
        const eprosima::fastdds::rtps::GuidPrefix_t& prefix_override)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_writer_data = get_temporary_writer_proxies_pool().get();

    temp_writer_data->clear();
    temp_writer_data->guid({ prefix_override, endpoints->writer.writer_->getGuid().entityId });
    temp_writer_data->set_multicast_locators(server_att.metatrafficMulticastLocatorList, network);
    temp_writer_data->set_remote_unicast_locators(server_att.metatrafficUnicastLocatorList, network);
    temp_writer_data->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
    temp_writer_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
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
        endpoints->reader.reader_->matched_writer_add(*temp_writer_data);
    }
}

void PDPClient::match_pdp_reader_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att)
{
    match_pdp_reader_nts_(server_att, server_att.guidPrefix);
}

void PDPClient::match_pdp_reader_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
        const eprosima::fastdds::rtps::GuidPrefix_t& prefix_override)
{
    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(builtin_endpoints_.get());
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_reader_data = get_temporary_reader_proxies_pool().get();

    temp_reader_data->clear();
    temp_reader_data->guid({ prefix_override, endpoints->reader.reader_->getGuid().entityId });
    temp_reader_data->set_multicast_locators(server_att.metatrafficMulticastLocatorList, network);
    temp_reader_data->set_remote_unicast_locators(server_att.metatrafficUnicastLocatorList, network);
    temp_reader_data->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_reader_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
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
        endpoints->writer.writer_->matched_reader_add(*temp_reader_data);
    }
}

bool ros_super_client_env()
{
    std::string super_client_str;
    bool super_client = false;
    std::vector<std::string> true_vec = {"TRUE", "true", "True", "1"};
    std::vector<std::string> false_vec = {"FALSE", "false", "False", "0"};

    SystemInfo::get_env(ROS_SUPER_CLIENT, super_client_str);
    if (super_client_str != "")
    {
        if (find(true_vec.begin(), true_vec.end(), super_client_str) != true_vec.end())
        {
            super_client = true;
        }
        else if (find(false_vec.begin(), false_vec.end(), super_client_str) != false_vec.end())
        {
            super_client = false;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_PDP,
                    "Invalid value for ROS_SUPER_CLIENT environment variable : " << super_client_str);
        }
    }
    return super_client;
}

const std::string& ros_discovery_server_env()
{
    static std::string servers;
    SystemInfo::get_env(DEFAULT_ROS2_MASTER_URI, servers);
    return servers;
}

bool load_environment_server_info(
        RemoteServerList_t& attributes)
{
    return load_environment_server_info(ros_discovery_server_env(), attributes);
}

bool load_environment_server_info(
        const std::string& list,
        RemoteServerList_t& attributes)
{
    attributes.clear();
    if (list.empty())
    {
        return true;
    }

    /* Parsing ancillary regex
     * Addresses should be ; separated. IPLocator functions are used to identify them in the order:
     * IPv4, IPv6 or try dns resolution.
     **/
    const static std::regex ROS2_SERVER_LIST_PATTERN(R"(([^;]*);?)");
    const static std::regex ROS2_IPV4_ADDRESSPORT_PATTERN(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");
    const static std::regex ROS2_IPV6_ADDRESSPORT_PATTERN(
        R"(^\[?((?:[0-9a-fA-F]{0,4}\:){0,7}[0-9a-fA-F]{0,4})?(?:\])?:?(?:(\d+))?$)");
    // Regex to handle DNS and UDPv4/6 expressions
    const static std::regex ROS2_DNS_DOMAINPORT_PATTERN(R"(^(UDPv[46]?:\[[\w\.:-]{0,63}\]|[\w\.-]{0,63}):?(?:(\d+))?$)");
    // Regex to handle TCPv4/6 expressions
    const static std::regex ROS2_DNS_DOMAINPORT_PATTERN_TCP(
        R"(^(TCPv[46]?:\[[\w\.:-]{0,63}\]):?(?:(\d+))?$)");

    // Filling port info
    auto process_port = [](int port, Locator_t& server)
            {
                if (port > std::numeric_limits<uint16_t>::max())
                {
                    throw std::out_of_range("Too large udp port passed into the server's list");
                }

                if (!IPLocator::setPhysicalPort(server, static_cast<uint16_t>(port)))
                {
                    std::stringstream ss;
                    ss << "Wrong udp port passed into the server's list " << port;
                    throw std::invalid_argument(ss.str());
                }
            };

    // Add new server
    auto add_server2qos = [](int id, std::forward_list<Locator>&& locators, RemoteServerList_t& attributes)
            {
                RemoteServerAttributes server_att;

                // add the server to the list
                if (!get_server_client_default_guidPrefix(id, server_att.guidPrefix))
                {
                    throw std::invalid_argument("The maximum number of default discovery servers has been reached");
                }

                // split multi and unicast locators
                auto unicast = std::partition(locators.begin(), locators.end(), IPLocator::isMulticast);

                LocatorList mlist;
                std::copy(locators.begin(), unicast, std::back_inserter(mlist));
                if (!mlist.empty())
                {
                    server_att.metatrafficMulticastLocatorList.push_back(std::move(mlist));
                }

                LocatorList ulist;
                std::copy(unicast, locators.end(), std::back_inserter(ulist));
                if (!ulist.empty())
                {
                    server_att.metatrafficUnicastLocatorList.push_back(std::move(ulist));
                }

                attributes.push_back(std::move(server_att));
            };

    try
    {
        // Do the parsing and populate the list
        Locator_t server_locator(LOCATOR_KIND_UDPv4, DEFAULT_ROS2_SERVER_PORT);
        int server_id = 0;

        std::sregex_iterator server_it(
            list.begin(),
            list.end(),
            ROS2_SERVER_LIST_PATTERN,
            std::regex_constants::match_not_null);

        while (server_it != std::sregex_iterator())
        {
            // Retrieve the address (IPv4, IPv6 or DNS name)
            const std::smatch::value_type sm = *++(server_it->cbegin());

            if (sm.matched)
            {
                // now we must parse the inner expression
                std::smatch mr;
                std::string locator(sm);

                if (locator.empty())
                {
                    // it's intencionally empty to hint us to ignore this server
                }
                // Try first with IPv4
                else if (std::regex_match(locator, mr, ROS2_IPV4_ADDRESSPORT_PATTERN,
                        std::regex_constants::match_not_null))
                {
                    std::smatch::iterator it = mr.cbegin();

                    // traverse submatches
                    if (++it != mr.cend())
                    {
                        std::string address = it->str();
                        server_locator.kind = LOCATOR_KIND_UDPv4;
                        server_locator.set_Invalid_Address();

                        if (!IPLocator::setIPv4(server_locator, address))
                        {
                            std::stringstream ss;
                            ss << "Wrong ipv4 address passed into the server's list " << address;
                            throw std::invalid_argument(ss.str());
                        }

                        if (IPLocator::isAny(server_locator))
                        {
                            // A server cannot be reach in all interfaces, it's clearly a localhost call
                            IPLocator::setIPv4(server_locator, "127.0.0.1");
                        }

                        // get port if any
                        int port = DEFAULT_ROS2_SERVER_PORT;
                        if (++it != mr.cend() && it->matched)
                        {
                            port = stoi(it->str());
                        }

                        process_port( port, server_locator);
                    }

                    // add server to the list
                    add_server2qos(server_id, std::forward_list<Locator>{server_locator}, attributes);
                }
                // Try IPv6 next
                else if (std::regex_match(locator, mr, ROS2_IPV6_ADDRESSPORT_PATTERN,
                        std::regex_constants::match_not_null))
                {
                    std::smatch::iterator it = mr.cbegin();

                    // traverse submatches
                    if (++it != mr.cend())
                    {
                        std::string address = it->str();
                        server_locator.kind = LOCATOR_KIND_UDPv6;
                        server_locator.set_Invalid_Address();

                        if (!IPLocator::setIPv6(server_locator, address))
                        {
                            std::stringstream ss;
                            ss << "Wrong ipv6 address passed into the server's list " << address;
                            throw std::invalid_argument(ss.str());
                        }

                        if (IPLocator::isAny(server_locator))
                        {
                            // A server cannot be reach in all interfaces, it's clearly a localhost call
                            IPLocator::setIPv6(server_locator, "::1");
                        }

                        // get port if any
                        int port = DEFAULT_ROS2_SERVER_PORT;
                        if (++it != mr.cend() && it->matched)
                        {
                            port = stoi(it->str());
                        }

                        process_port( port, server_locator);
                    }

                    // add server to the list
                    add_server2qos(server_id, std::forward_list<Locator>{server_locator}, attributes);
                }
                // try resolve DNS
                else if (std::regex_match(locator, mr, ROS2_DNS_DOMAINPORT_PATTERN,
                        std::regex_constants::match_not_null))
                {
                    std::forward_list<Locator> flist;

                    {
                        std::stringstream new_locator(locator,
                                std::ios_base::in |
                                std::ios_base::out |
                                std::ios_base::ate);

                        // first try the formal notation, add default port if necessary
                        if (!mr[2].matched)
                        {
                            new_locator << ":" << DEFAULT_ROS2_SERVER_PORT;
                        }

                        new_locator >> server_locator;
                    }

                    // Otherwise add all resolved locators
                    switch ( server_locator.kind )
                    {
                        case LOCATOR_KIND_UDPv4:
                        case LOCATOR_KIND_UDPv6:
                            flist.push_front(server_locator);
                            break;
                        case LOCATOR_KIND_INVALID:
                        {
                            std::smatch::iterator it = mr.cbegin();

                            // traverse submatches
                            if (++it != mr.cend())
                            {
                                std::string domain_name = it->str();
                                std::set<std::string> ipv4, ipv6;
                                std::tie(ipv4, ipv6) = IPLocator::resolveNameDNS(domain_name);

                                // get port if any
                                int port = DEFAULT_ROS2_SERVER_PORT;
                                if (++it != mr.cend() && it->matched)
                                {
                                    port = stoi(it->str());
                                }

                                for ( const std::string& loc : ipv4 )
                                {
                                    server_locator.kind = LOCATOR_KIND_UDPv4;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv4(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv4(server_locator, "127.0.0.1");
                                    }

                                    process_port( port, server_locator);
                                    flist.push_front(server_locator);
                                }

                                for ( const std::string& loc : ipv6 )
                                {
                                    server_locator.kind = LOCATOR_KIND_UDPv6;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv6(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv6(server_locator, "::1");
                                    }

                                    process_port( port, server_locator);
                                    flist.push_front(server_locator);
                                }
                            }
                        }
                    }

                    if (flist.empty())
                    {
                        std::stringstream ss;
                        ss << "Wrong domain name passed into the server's list " << locator;
                        throw std::invalid_argument(ss.str());
                    }

                    // add server to the list
                    add_server2qos(server_id, std::move(flist), attributes);
                }
                // try resolve TCP DNS
                else if (std::regex_match(locator, mr, ROS2_DNS_DOMAINPORT_PATTERN_TCP,
                        std::regex_constants::match_not_null))
                {
                    std::forward_list<Locator> flist;

                    {
                        std::stringstream new_locator(locator,
                                std::ios_base::in |
                                std::ios_base::out |
                                std::ios_base::ate);

                        // first try the formal notation, add default port if necessary
                        if (!mr[2].matched)
                        {
                            new_locator << ":" << DEFAULT_TCP_SERVER_PORT;
                        }

                        new_locator >> server_locator;
                    }

                    // Otherwise add all resolved locators
                    switch ( server_locator.kind )
                    {
                        case LOCATOR_KIND_TCPv4:
                        case LOCATOR_KIND_TCPv6:
                            IPLocator::setLogicalPort(server_locator, static_cast<uint16_t>(server_locator.port));
                            flist.push_front(server_locator);
                            break;
                        case LOCATOR_KIND_INVALID:
                        {
                            std::smatch::iterator it = mr.cbegin();

                            // traverse submatches
                            if (++it != mr.cend())
                            {
                                std::string domain_name = it->str();
                                std::set<std::string> ipv4, ipv6;
                                std::tie(ipv4, ipv6) = IPLocator::resolveNameDNS(domain_name);

                                // get port if any
                                int port = DEFAULT_TCP_SERVER_PORT;
                                if (++it != mr.cend() && it->matched)
                                {
                                    port = stoi(it->str());
                                }

                                for ( const std::string& loc : ipv4 )
                                {
                                    server_locator.kind = LOCATOR_KIND_TCPv4;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv4(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv4(server_locator, "127.0.0.1");
                                    }

                                    process_port( port, server_locator);
                                    IPLocator::setLogicalPort(server_locator, static_cast<uint16_t>(port));
                                    flist.push_front(server_locator);
                                }

                                for ( const std::string& loc : ipv6 )
                                {
                                    server_locator.kind = LOCATOR_KIND_TCPv6;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv6(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv6(server_locator, "::1");
                                    }

                                    process_port( port, server_locator);
                                    IPLocator::setLogicalPort(server_locator, static_cast<uint16_t>(port));
                                    flist.push_front(server_locator);
                                }
                            }
                        }
                    }

                    if (flist.empty())
                    {
                        std::stringstream ss;
                        ss << "Wrong domain name passed into the server's list " << locator;
                        throw std::invalid_argument(ss.str());
                    }

                    // add server to the list
                    add_server2qos(server_id, std::move(flist), attributes);
                }
                else
                {
                    std::stringstream ss;
                    ss << "Wrong locator passed into the server's list " << locator;
                    throw std::invalid_argument(ss.str());
                }
            }

            // advance to the next server if any
            ++server_id;
            ++server_it;
        }

        // Check for server info
        if (attributes.empty())
        {
            throw std::invalid_argument("No default server locators were provided.");
        }
    }
    catch (std::exception& e)
    {
        EPROSIMA_LOG_ERROR(SERVER_CLIENT_DISCOVERY, e.what());
        attributes.clear();
        return false;
    }

    return true;
}

GUID_t RemoteServerAttributes::GetParticipant() const
{
    return GUID_t(guidPrefix, c_EntityId_RTPSParticipant);
}

GUID_t RemoteServerAttributes::GetPDPReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SPDPReader);
}

GUID_t RemoteServerAttributes::GetPDPWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SPDPWriter);
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
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
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

    update_remote_servers_list();

    return false;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
