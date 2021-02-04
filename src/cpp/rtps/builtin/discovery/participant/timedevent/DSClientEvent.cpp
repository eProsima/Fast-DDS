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

#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastdds/rtps/resources/ResourceEvent.h>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>

#include <rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <rtps/builtin/discovery/participant/PDPClient.h>
#include <rtps/builtin/discovery/endpoint/EDPClient.h>

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
    , mp_EDP(static_cast<EDPClient*>(mp_PDP->getEDP()))
{
}

DSClientEvent::~DSClientEvent()
{
}

bool DSClientEvent::event()
{
    // logInfo(CLIENT_PDP_THREAD, "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " DSClientEvent Period");
    bool restart = false;

    // Iterate over remote servers to check for new unmatched servers
    for (auto server: mp_PDP->remote_server_attributes())
    {
        // If the server is known, it means that this client has received the server's DATA(p),
        // which in turn means that the server has received the client's DATA(p)
        if (mp_PDP->is_known_participant(server.guidPrefix))
        {
            // Match EDP endpoints with this server if necessary
            if (!mp_EDP->areRemoteEndpointsMatched(server.guidPrefix))
            {
                mp_EDP->assignRemoteEndpoints(*(mp_PDP->get_participant_proxy_data(server.guidPrefix)));
            }
        }
        // If the server is not known, we need to run the event again
        else
        {
            restart = true;
        }
    }

    // If we are still not connected to all servers, we need to keep pinging the unmatched ones
    if (restart)
    {
        // This marks to announceParticipantState that the announcement is only meant for missing servers,
        // so it is not a periodic announcement
        mp_PDP->_serverPing = true;
        mp_PDP->announceParticipantState(false);
        logInfo(CLIENT_PDP_THREAD, "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " PDP announcement");
    }

    return restart;
}

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */
