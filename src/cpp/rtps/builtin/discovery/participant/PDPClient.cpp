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
#include <rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <utils/SystemInfo.hpp>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace fastrtps::rtps;

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
        logError(RTPS_PDP, "Using a PDP client object with another user's settings");
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

    //#if HAVE_SECURITY
    //    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP
    //    .enable_builtin_secure_publications_writer_and_subscriptions_reader)
    //    {
    //        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
    //        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
    //    }
    //
    //    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP
    //    .enable_builtin_secure_subscriptions_writer_and_publications_reader)
    //    {
    //        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
    //        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
    //    }
    //#endif

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
        logError(RTPS_PDP, "Endpoint discovery configuration failed");
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
            if (svr.guidPrefix == participant_data.m_guid.guidPrefix)
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

bool PDPClient::createPDPEndpoints()
{
    logInfo(RTPS_PDP, "Beginning PDPClient Endpoints creation");

    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    mp_PDPReaderHistory = new ReaderHistory(hatt);

    ReaderAttributes ratt;
    ratt.expectsInlineQos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.times.heartbeatResponseDelay = pdp_heartbeat_response_delay;

    mp_listener = new PDPListener(this);

    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory, mp_listener,
            c_EntityId_SPDPReader, true, false))
    {
        //#if HAVE_SECURITY
        //        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rout, false);
        //#endif
        // Initial peer list doesn't make sense in server scenario. Client should match its server list
        {
            eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

            for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
            {
                match_pdp_writer_nts_(it);
            }
        }
    }
    else
    {
        logError(RTPS_PDP, "PDPClient Reader creation failed");
        delete(mp_PDPReaderHistory);
        mp_PDPReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    hatt.payloadMaxSize = mp_builtin->m_att.writerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_PDPWriterHistory = new WriterHistory(hatt);

    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    watt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    watt.times.heartbeatPeriod = pdp_heartbeat_period;
    watt.times.nackResponseDelay = pdp_nack_response_delay;
    watt.times.nackSupressionDuration = pdp_nack_supression_duration;

    if (mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
    {
        watt.mode = ASYNCHRONOUS_WRITER;
    }

    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory, nullptr,
            c_EntityId_SPDPWriter, true))
    {
        //#if HAVE_SECURITY
        //        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
        //#endif
        {
            eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

            for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
            {
                match_pdp_reader_nts_(it);
            }
        }
    }
    else
    {
        logError(RTPS_PDP, "PDPClient Writer creation failed");
        delete(mp_PDPWriterHistory);
        mp_PDPWriterHistory = nullptr;
        return false;
    }
    logInfo(RTPS_PDP, "PDPClient Endpoints creation finished");
    return true;
}

// the ParticipantProxyData* pdata must be the one kept in PDP database
void PDPClient::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // Verify if this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
                std::unique_lock<std::recursive_mutex> lock(*getMutex());
                svr.proxy = pdata;
            }
        }
    }

    notifyAboveRemoteEndpoints(*pdata);
}

void PDPClient::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata)
{
    // No EDP notification needed. EDP endpoints would be match when PDP synchronization is granted
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
    }
}

void PDPClient::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    // EDP endpoints have been already unmatch by the associated listener
    assert(!mp_EDP->areRemoteEndpointsMatched(pdata));

    bool is_server = false;
    {
        eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

        // Verify if this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
                std::unique_lock<std::recursive_mutex> lock(*getMutex());
                svr.proxy = nullptr; // reasign when we receive again server DATA(p)
                is_server = true;
                mp_sync->restart_timer(); // enable announcement and sync mechanism till this server reappears
            }
        }
    }

    if (is_server)
    {
        // We should unmatch and match the PDP endpoints to renew the PDP reader and writer associated proxies
        logInfo(RTPS_PDP, "For unmatching for server: " << pdata->m_guid);
        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        uint32_t endp = pdata->m_availableBuiltinEndpoints;
        uint32_t auxendp = endp;
        auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;

        if (auxendp != 0)
        {
            GUID_t wguid;

            wguid.guidPrefix = pdata->m_guid.guidPrefix;
            wguid.entityId = c_EntityId_SPDPWriter;
            mp_PDPReader->matched_writer_remove(wguid);

            // rematch but discarding any previous state of the server
            // because we know the server shutdown intencionally
            auto temp_writer_data = get_temporary_writer_proxies_pool().get();

            temp_writer_data->clear();
            temp_writer_data->guid(wguid);
            temp_writer_data->persistence_guid(pdata->get_persistence_guid());
            temp_writer_data->set_persistence_entity_id(c_EntityId_SPDPWriter);
            temp_writer_data->set_remote_locators(pdata->metatraffic_locators, network, true);
            temp_writer_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            temp_writer_data->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
            mp_PDPReader->matched_writer_add(*temp_writer_data);
        }

        auxendp = endp;
        auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

        if (auxendp != 0)
        {
            GUID_t rguid;
            rguid.guidPrefix = pdata->m_guid.guidPrefix;
            rguid.entityId = c_EntityId_SPDPReader;
            mp_PDPWriter->matched_reader_remove(rguid);

            auto temp_reader_data = get_temporary_reader_proxies_pool().get();

            temp_reader_data->clear();
            temp_reader_data->m_expectsInlineQos = false;
            temp_reader_data->guid(rguid);
            temp_reader_data->set_remote_locators(pdata->metatraffic_locators, network, true);
            temp_reader_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            temp_reader_data->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            mp_PDPWriter->matched_reader_add(*temp_reader_data);
        }
    }
}

bool PDPClient::all_servers_acknowledge_PDP()
{
    // check if already initialized
    assert(mp_PDPWriterHistory && mp_PDPWriter);

    // get a reference to client proxy data
    CacheChange_t* pPD;
    if (mp_PDPWriterHistory->get_min_change(&pPD))
    {
        return mp_PDPWriter->is_acked_by_all(pPD);
    }
    else
    {
        logError(RTPS_PDP, "ParticipantProxy data should have been added to client PDP history cache "
                "by a previous call to announceParticipantState()");
    }

    return false;
}

bool PDPClient::is_all_servers_PDPdata_updated()
{
    // Assess all server DATA has been received
    fastrtps::rtps::StatefulReader* pR = dynamic_cast<fastrtps::rtps::StatefulReader*>(mp_PDPReader);
    assert(pR);
    return pR->isInCleanState();
}

void PDPClient::announceParticipantState(
        bool new_change,
        bool dispose,
        WriteParams& )
{
    if (enabled_)
    {
        /*
           Protect writer sequence number. Make sure in order to prevent AB BA deadlock that the
           PDP mutex is systematically lock before the writer one (if needed):
            - transport callbacks on PDPListener
            - initialization and removal on BuiltinProtocols::initBuiltinProtocols and ~BuiltinProtocols
            - DSClientEvent (own thread)
            - ResendParticipantProxyDataPeriod (participant event thread)
         */

        std::lock_guard<std::recursive_mutex> lock(*getMutex());

        std::lock_guard<RecursiveTimedMutex> wlock(mp_PDPWriter->getMutex());

        WriteParams wp;
        SampleIdentity local;
        local.writer_guid(mp_PDPWriter->getGuid());
        local.sequence_number(mp_PDPWriterHistory->next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        // Add the write params to the sample
        if (dispose)
        {
            // we must assure when the server is dying that all client are send at least a DATA(p)
            // note here we can no longer receive and DATA or ACKNACK from clients.
            // In order to avoid that we send the message directly as in the standard stateless PDP

            fastrtps::rtps::StatefulWriter* pW = dynamic_cast<fastrtps::rtps::StatefulWriter*>(mp_PDPWriter);
            assert(pW);

            CacheChange_t* change = nullptr;

            if ((change = pW->new_change(
                        [this]() -> uint32_t
                        {
                            return mp_builtin->m_att.writerPayloadSize;
                        },
                        NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key)))
            {
                // update the sequence number
                change->sequenceNumber = mp_PDPWriterHistory->next_sequence_number();
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
                        if (svr.proxy != nullptr)
                        {
                            remote_readers.push_back(svr.GetPDPReader());
                            //locators.push_back(svr.metatrafficMulticastLocatorList);
                            locators.push_back(svr.metatrafficUnicastLocatorList);
                        }
                    }
                }

                DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);
                RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, &sender);
                if (!group.add_data(*change, false))
                {
                    logError(RTPS_PDP, "Error sending announcement from client to servers");
                }
            }

            // free change
            mp_PDPWriter->release_change(change);
        }
        else
        {
            PDP::announceParticipantState(new_change, dispose, wp);

            if (!new_change)
            {
                // retrieve the participant discovery data
                CacheChange_t* pPD;
                if (mp_PDPWriterHistory->get_min_change(&pPD))
                {
                    std::vector<GUID_t> remote_readers;
                    LocatorList locators;

                    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

                    for (auto& svr : mp_builtin->m_DiscoveryServers)
                    {
                        // non-pinging announcements like lease duration ones must be
                        // broadcast to all servers
                        if (svr.proxy == nullptr || !_serverPing)
                        {
                            remote_readers.push_back(svr.GetPDPReader());
                            locators.push_back(svr.metatrafficMulticastLocatorList);
                            locators.push_back(svr.metatrafficUnicastLocatorList);
                        }
                    }

                    DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);
                    RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, &sender);

                    if (!group.add_data(*pPD, false))
                    {
                        logError(RTPS_PDP, "Error sending announcement from client to servers");
                    }

                    // ping done independtly of which triggered the announcement
                    // note all event callbacks are currently serialized
                    _serverPing = false;
                }
                else
                {
                    logError(RTPS_PDP, "ParticipantProxy data should have been added to client PDP history "
                            "cache by a previous call to announceParticipantState()");
                }
            }
        }
    }
}

bool PDPClient::match_servers_EDP_endpoints()
{
    // PDP must have been initialize
    assert(mp_EDP);

    bool all = true; // have all servers been discovered?

    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());
    for (auto& svr : mp_builtin->m_DiscoveryServers)
    {
        all &= (svr.proxy != nullptr);

        if (svr.proxy && !mp_EDP->areRemoteEndpointsMatched(svr.proxy))
        {
            logInfo(RTPS_PDP, "Client "
                    << mp_EDP->mp_PDP->getRTPSParticipant()->getGuid()
                    << " matching servers EDP endpoints");
            mp_EDP->assignRemoteEndpoints(*svr.proxy);
        }
    }

    return all;
}

void PDPClient::update_remote_servers_list()
{
    if (!mp_PDPReader || !mp_PDPWriter)
    {
        logError(SERVER_CLIENT_DISCOVERY, "Cannot update server list within an uninitialized Client");
        return;
    }

    eprosima::shared_lock<eprosima::shared_mutex> disc_lock(mp_builtin->getDiscoveryMutex());

    for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
    {
        if (!mp_PDPReader->matched_writer_is_matched(it.GetPDPWriter()))
        {
            match_pdp_writer_nts_(it);
        }

        if (!mp_PDPWriter->matched_reader_is_matched(it.GetPDPReader()))
        {
            match_pdp_reader_nts_(it);
        }
    }
    mp_sync->restart_timer();
}

void PDPClient::match_pdp_writer_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att)
{
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_writer_data = get_temporary_writer_proxies_pool().get();

    temp_writer_data->clear();
    temp_writer_data->guid(server_att.GetPDPWriter());
    temp_writer_data->set_multicast_locators(server_att.metatrafficMulticastLocatorList, network);
    temp_writer_data->set_remote_unicast_locators(server_att.metatrafficUnicastLocatorList, network);
    temp_writer_data->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
    temp_writer_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_PDPReader->matched_writer_add(*temp_writer_data);
}

void PDPClient::match_pdp_reader_nts_(
        const eprosima::fastdds::rtps::RemoteServerAttributes& server_att)
{
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    auto temp_reader_data = get_temporary_reader_proxies_pool().get();

    temp_reader_data->clear();
    temp_reader_data->guid(server_att.GetPDPReader());
    temp_reader_data->set_multicast_locators(server_att.metatrafficMulticastLocatorList, network);
    temp_reader_data->set_remote_unicast_locators(server_att.metatrafficUnicastLocatorList, network);
    temp_reader_data->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    temp_reader_data->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_PDPWriter->matched_reader_add(*temp_reader_data);
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
    const static std::regex ROS2_IPV6_ADDRESSPORT_PATTERN(R"(^\[?((?:[0-9a-fA-F]{0,4}\:){0,7}[0-9a-fA-F]{0,4})?(?:\])?:?(?:(\d+))?$)");
    const static std::regex ROS2_DNS_DOMAINPORT_PATTERN(R"(^([\w\.-]{0,63}):?(?:(\d+))?$)");

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

                if(locator.empty())
                {
                    // it's intencionally empty to hint us to ignore this server
                }
                // Try first with IPv4
                else if (std::regex_match(locator, mr, ROS2_IPV4_ADDRESSPORT_PATTERN, std::regex_constants::match_not_null))
                {
                    std::smatch::iterator it = mr.cbegin();

                    // traverse submatches
                    if (++it != mr.cend())
                    {
                        std::string address = it->str();
                        server_locator.kind = LOCATOR_KIND_UDPv4;

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
                else if (std::regex_match(locator, mr, ROS2_IPV6_ADDRESSPORT_PATTERN, std::regex_constants::match_not_null))
                {
                    std::smatch::iterator it = mr.cbegin();

                    // traverse submatches
                    if (++it != mr.cend())
                    {
                        std::string address = it->str();
                        server_locator.kind = LOCATOR_KIND_UDPv6;

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
                else if (std::regex_match(locator, mr, ROS2_DNS_DOMAINPORT_PATTERN, std::regex_constants::match_not_null))
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

                        std::forward_list<Locator> flist;
                        for( const std::string& loc : ipv4 )
                        {
                            server_locator.kind = LOCATOR_KIND_UDPv4;
                            IPLocator::setIPv4(server_locator, loc);

                            if (IPLocator::isAny(server_locator))
                            {
                                // A server cannot be reach in all interfaces, it's clearly a localhost call
                                IPLocator::setIPv4(server_locator, "127.0.0.1");
                            }

                            process_port( port, server_locator);
                            flist.push_front(server_locator);
                        }

                        for( const std::string& loc : ipv6 )
                        {
                            server_locator.kind = LOCATOR_KIND_UDPv6;
                            IPLocator::setIPv6(server_locator, loc);

                            if (IPLocator::isAny(server_locator))
                            {
                                // A server cannot be reach in all interfaces, it's clearly a localhost call
                                IPLocator::setIPv6(server_locator, "::1");
                            }

                            process_port( port, server_locator);
                            flist.push_front(server_locator);
                        }

                        if(flist.empty())
                        {
                            std::stringstream ss;
                            ss << "Wrong domain name passed into the server's list " << locator;
                            throw std::invalid_argument(ss.str());
                        }

                        // add server to the list
                        add_server2qos(server_id, std::move(flist), attributes);
                    }
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
        logError(SERVER_CLIENT_DISCOVERY, e.what());
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

    // Erase Proxies created before having the Participant
    GUID_t wguid;
    wguid.guidPrefix = partGUID.guidPrefix;
    wguid.entityId = c_EntityId_SPDPWriter;
    mp_PDPReader->matched_writer_remove(wguid);

    GUID_t rguid;
    rguid.guidPrefix = partGUID.guidPrefix;
    rguid.entityId = c_EntityId_SPDPReader;
    mp_PDPWriter->matched_reader_remove(rguid);

    update_remote_servers_list();

    return false;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
