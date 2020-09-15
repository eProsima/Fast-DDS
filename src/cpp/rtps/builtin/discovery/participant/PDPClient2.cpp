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

GUID_t RemoteServerAttributes::GetEDPPublicationsReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPPubReader);
}

GUID_t RemoteServerAttributes::GetEDPSubscriptionsWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPSubWriter);
}

GUID_t RemoteServerAttributes::GetEDPPublicationsWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPPubWriter);
}

GUID_t RemoteServerAttributes::GetEDPSubscriptionsReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPSubReader);
}

bool get_server_client_default_guidPrefix(
        int id,
        GuidPrefix_t& guid)
{
    if ( id >= 0
            && id < 256
            && std::istringstream(DEFAULT_ROS2_SERVER_GUIDPREFIX) >> guid)
    {
        // Last octet denotes the default server id but to ease debugging it starts on char '0' = 48
        guid.value[11] = static_cast<octet>((48 + id) % 256);

        return true;
    }

    return false;
}

bool load_environment_server_info(
        std::string list,
        RemoteServerList_t& attributes)
{
    using namespace std;

    // parsing ancillary regex
    const regex ROS2_IPV4_PATTERN(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");
    const regex ROS2_SERVER_LIST_PATTERN(R"(([^;]*);?)");

    try
    {
        // Do the parsing and populate the list
        attributes.clear();
        RemoteServerAttributes server_att;
        Locator_t server_locator(LOCATOR_KIND_UDPv4, DEFAULT_ROS2_SERVER_PORT);
        int server_id = 0;

        sregex_iterator server_it(
            list.begin(),
            list.end(),
            ROS2_SERVER_LIST_PATTERN,
            regex_constants::match_not_null);

        while (server_it != sregex_iterator())
        {
            const smatch::value_type sm = *++(server_it->cbegin());

            if (sm.matched)
            {
                // now we must parse the inner expression
                smatch mr;
                string locator(sm);
                if (regex_match(locator, mr, ROS2_IPV4_PATTERN, regex_constants::match_not_null))
                {
                    smatch::iterator it = mr.cbegin();

                    while (++it != mr.cend())
                    {
                        if ( !IPLocator::setIPv4(server_locator, it->str()) )
                        {
                            stringstream ss;
                            ss << "Wrong ipv4 address passed into the server's list " << it->str();
                            throw std::invalid_argument(ss.str());
                        }

                        if (IPLocator::isAny(server_locator))
                        {
                            // A server cannot be reach in all interfaces, it's clearly a localhost call
                            IPLocator::setIPv4(server_locator, "127.0.0.1");
                        }

                        if ( ++it != mr.cend() )
                        {
                            // reset the locator to default
                            IPLocator::setPhysicalPort(server_locator, DEFAULT_ROS2_SERVER_PORT);

                            if ( it->matched)
                            {
                                // note stoi throws also an invalid_argument
                                int port = stoi(it->str());

                                if ( port > std::numeric_limits<uint16_t>::max() )
                                {
                                    throw out_of_range("Too larget udp port passed into the server's list");
                                }

                                if ( !IPLocator::setPhysicalPort(server_locator, static_cast<uint16_t>(port)) )
                                {
                                    stringstream ss;
                                    ss << "Wrong udp port passed into the server's list " << it->str();
                                    throw invalid_argument(ss.str());
                                }
                            }
                        }
                    }

                    // add the server to the list
                    if (!get_server_client_default_guidPrefix(server_id, server_att.guidPrefix))
                    {
                        throw std::invalid_argument("The maximum number of default discovery servers have been reached");
                    }

                    server_att.metatrafficUnicastLocatorList.clear();
                    server_att.metatrafficUnicastLocatorList.push_back(server_locator);
                    attributes.push_back(server_att);
                }
                else
                {
                    if (!locator.empty())
                    {
                        stringstream ss;
                        ss << "Wrong locator passed into the server's list " << locator;
                        throw std::invalid_argument(ss.str());
                    }
                    // else: it's intencionally empty to hint us to ignore this server
                }
            }
            // advance to the next server if any
            ++server_it;
            ++server_id;
        }

        // Check for server info
        if (attributes.empty())
        {
            throw std::invalid_argument("No default server locators were provided.");
        }
    }
    catch ( std::exception& e )
    {
        logError(SERVER_CLIENT_DISCOVERY, e.what());
        attributes.clear();
        return false;
    }

    return true;
}

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
