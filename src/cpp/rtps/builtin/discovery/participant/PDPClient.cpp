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

#include <fastdds/rtps/builtin/discovery/participant/PDPClient.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>
#include <fastdds/rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>

#include <fastdds/rtps/builtin/discovery/endpoint/EDPClient.h>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/StatefulReader.h>

#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/rtps/writer/ReaderProxy.h>

#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/TimeConversion.h>

#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>


using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPClient::PDPClient(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation)
    : PDP(builtin, allocation)
    , mp_sync(nullptr)
    , _serverPing(false)
{
}

PDPClient::~PDPClient()
{
    logInfo(RTPS_PDP, "Destroying Client: " << getRTPSParticipant()->getGuid())
    if (mp_sync != nullptr)
    {
        delete mp_sync;
    }
}

void PDPClient::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data); // TODO: Remember that the PDP version USES security

    if (getRTPSParticipant()->getAttributes().builtin.discovery_config.discoveryProtocol != DiscoveryProtocol_t::CLIENT)
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
    for (auto& svr : mp_builtin->m_DiscoveryServers)
    {
        if (svr.guidPrefix == participant_data.m_guid.guidPrefix)
        {
            is_server = true;
        }
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.m_guid, is_server);
    if (pdata != nullptr)
    {
        pdata->copy(participant_data);
        pdata->isAlive = true;

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

    const NetworkFactory& network = mp_RTPSParticipant->network_factory();

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
        for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_writer_data_.clear();
            temp_writer_data_.guid(it.GetPDPWriter());
            temp_writer_data_.set_multicast_locators(it.metatrafficMulticastLocatorList, network);
            temp_writer_data_.set_remote_unicast_locators(it.metatrafficUnicastLocatorList, network);
            temp_writer_data_.m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;  // Server Information must be persistent
            temp_writer_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

            mp_PDPReader->matched_writer_add(temp_writer_data_);
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
        for (const eprosima::fastdds::rtps::RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_reader_data_.clear();
            temp_reader_data_.guid(it.GetPDPReader());
            temp_reader_data_.set_multicast_locators(it.metatrafficMulticastLocatorList, network);
            temp_reader_data_.set_remote_unicast_locators(it.metatrafficUnicastLocatorList, network);
            temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            temp_reader_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

            mp_PDPWriter->matched_reader_add(temp_reader_data_);
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
        std::unique_lock<std::recursive_mutex> lock(*getMutex());

        // Verify if this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
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
        std::unique_lock<std::recursive_mutex> lock(*getMutex());

        // Verify if this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
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
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_writer_data_.clear();
            temp_writer_data_.guid(wguid);
            temp_writer_data_.persistence_guid(pdata->get_persistence_guid());
            temp_writer_data_.set_persistence_entity_id(c_EntityId_SPDPWriter);
            temp_writer_data_.set_remote_locators(pdata->metatraffic_locators, network, true);
            temp_writer_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            temp_writer_data_.m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
            mp_PDPReader->matched_writer_add(temp_writer_data_);
        }

        auxendp = endp;
        auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

        if (auxendp != 0)
        {
            GUID_t rguid;
            rguid.guidPrefix = pdata->m_guid.guidPrefix;
            rguid.entityId = c_EntityId_SPDPReader;
            mp_PDPWriter->matched_reader_remove(rguid);

            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_reader_data_.clear();
            temp_reader_data_.m_expectsInlineQos = false;
            temp_reader_data_.guid(rguid);
            temp_reader_data_.set_remote_locators(pdata->metatraffic_locators, network, true);
            temp_reader_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            mp_PDPWriter->matched_reader_add(temp_reader_data_);
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
    StatefulReader* pR = dynamic_cast<StatefulReader*>(mp_PDPReader);
    assert(pR);
    return pR->isInCleanState();
}

void PDPClient::announceParticipantState(
        bool new_change,
        bool dispose,
        WriteParams& )
{
    /*
       Protect writer sequence number. Make sure in order to prevent AB BA deadlock that the
       writer mutex is systematically lock before the PDP one (if needed):
        - transport callbacks on PDPListener
        - initialization and removal on BuiltinProtocols::initBuiltinProtocols and ~BuiltinProtocols
        - DSClientEvent (own thread)
        - ResendParticipantProxyDataPeriod (participant event thread)
     */
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

        StatefulWriter* pW = dynamic_cast<StatefulWriter*>(mp_PDPWriter);
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
            LocatorList_t locators;

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
                std::lock_guard<std::recursive_mutex> lock(*getMutex());

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
            RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, sender);
            if (!group.add_data(*change, false))
            {
                logError(RTPS_PDP, "Error sending announcement from client to servers");
            }
        }

        // free change
        mp_PDPWriterHistory->release_Cache(change);
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
                std::lock_guard<std::recursive_mutex> lock(*getMutex());

                std::vector<GUID_t> remote_readers;
                LocatorList_t locators;

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
                RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, sender);

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

bool PDPClient::match_servers_EDP_endpoints()
{
    // PDP must have been initialize
    assert(mp_EDP);

    std::lock_guard<std::recursive_mutex> lock(*getMutex());
    bool all = true; // have all servers been discovered?

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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
