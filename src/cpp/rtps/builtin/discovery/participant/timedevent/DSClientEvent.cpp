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
 * @file DSClientEvent.cpp
 *
 */

#include <rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>

#include <fastdds/dds/log/Log.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDPClient.h>
#include <rtps/builtin/discovery/participant/PDPClient.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <utils/shared_mutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {


DSClientEvent::DSClientEvent(
        PDPClient* p_PDP,
        double interval)
    : TimedEvent(p_PDP->getRTPSParticipant()->getEventResource(),
            [this]()
            {
                return event();
            }, interval)
    , mp_PDP(p_PDP)
    , mp_EDP(static_cast<EDPClient*>(mp_PDP->get_edp()))
{
}

DSClientEvent::~DSClientEvent()
{
}

bool DSClientEvent::event()
{
    // EPROSIMA_LOG_INFO(CLIENT_PDP_THREAD, "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " DSClientEvent Period");
    bool restart = false;

    // Iterate over remote servers to check for new unmatched servers
    ParticipantProxyData* part_proxy_data;
    eprosima::shared_lock<eprosima::shared_mutex> lock(mp_PDP->mp_builtin->getDiscoveryMutex());

    for (auto server: mp_PDP->connected_servers())
    {
        std::unique_lock<std::recursive_mutex> pdp_lock(*mp_PDP->getMutex());
        // Get the participant proxy data of the server
        part_proxy_data = mp_PDP->get_participant_proxy_data(server.guidPrefix);
        if (nullptr != part_proxy_data)
        {
            // Match EDP endpoints with this server if necessary
            if (!mp_EDP->areRemoteEndpointsMatched(part_proxy_data))
            {
                mp_EDP->assignRemoteEndpoints(*(part_proxy_data), true);
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(CLIENT_PDP_THREAD,
                    "Error while retrieving Participant Proxy Data of a connected server: "
                    << server.guidPrefix);
        }
    }

    if (mp_PDP->connected_servers().size() < mp_PDP->remote_server_locators().size())
    {
        // If there are pending servers, we need to run the event again
        restart = true;
    }

    // If we are still not connected to all servers, we need to keep pinging the unmatched ones
    if (restart)
    {
        // This marks to announceParticipantState that the announcement is only meant for missing servers,
        // so it is not a periodic announcement
        mp_PDP->_serverPing = true;
        WriteParams __wp = WriteParams::write_params_default();
        mp_PDP->announceParticipantState(false, false, __wp);
        EPROSIMA_LOG_INFO(CLIENT_PDP_THREAD,
                "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " PDP announcement");
    }

    return restart;
}

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */
