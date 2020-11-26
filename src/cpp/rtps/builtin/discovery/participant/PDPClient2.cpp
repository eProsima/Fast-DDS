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
 * @file PDPClient2.cpp
 *
 */
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

#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>
#include "./PDPClient2.hpp"
#include "./DSClientEvent2.hpp"
#include "../endpoint/EDPClient2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPClient2::PDPClient2(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation)
    : PDP(builtin, allocation)
    , mp_sync(nullptr)
{
}

PDPClient2::~PDPClient2()
{
    if (mp_sync != nullptr)
    {
        delete mp_sync;
    }
}

void PDPClient2::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data);

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
}

bool PDPClient2::init(
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
    mp_EDP = new EDPClient2(this, mp_RTPSParticipant);
    if (!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP, "Endpoint discovery configuration failed");
        return false;
    }

    mp_sync =
            new DSClientEvent2(this, TimeConv::Duration_t2MilliSecondsDouble(
                        m_discovery.discovery_config.discoveryServer_client_syncperiod));
    mp_sync->restart_timer();

    return true;
}

ParticipantProxyData* PDPClient2::createParticipantProxyData(
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

bool PDPClient2::createPDPEndpoints()
{
    // TODO DISCOVERY SERVER VERSION 2
    return true;
}

void PDPClient2::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    (void)pdata;
    // TODO DISCOVERY SERVER VERSION 2
}

void PDPClient2::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata)
{
    (void)pdata;
    // TODO DISCOVERY SERVER VERSION 2

    // No EDP notification needed. EDP endpoints would be match when PDP synchronization is granted
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
    }
}

void PDPClient2::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    (void)pdata;
    // TODO DISCOVERY SERVER VERSION 2
}

void PDPClient2::announceParticipantState(
        bool new_change,
        bool dispose,
        WriteParams& )
{
    (void)new_change;
    (void)dispose;
    // TODO DISCOVERY SERVER VERSION 2
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
