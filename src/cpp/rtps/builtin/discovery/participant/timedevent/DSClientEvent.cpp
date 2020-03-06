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

#include <fastdds/rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPClient.h>

#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastdds/rtps/resources/ResourceEvent.h>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>


namespace eprosima {
namespace fastrtps{
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
{
}

 DSClientEvent::~DSClientEvent()
{
}

bool DSClientEvent::event()
{
    logInfo(CLIENT_PDP_THREAD, "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " DSClientEvent Period");
    bool restart = true;

    // Check if all servers received my discovery data
    if (mp_PDP->all_servers_acknowledge_PDP())
    {
        // Wait until we have received all network discovery info currently available
        if (mp_PDP->is_all_servers_PDPdata_updated())
        {
            restart = !mp_PDP->match_servers_EDP_endpoints();
            // we must keep this TimedEvent alive to cope with servers' shutdown
            // PDPClient::removeRemoteEndpoints would restart_timer if a server vanishes
            logInfo(CLIENT_PDP_THREAD,"Client " << mp_PDP->getRTPSParticipant()->getGuid() << " matching servers EDP endpoints")
        }
        else
        {
            logInfo(CLIENT_PDP_THREAD, "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " not all servers acknowledge PDP info")
        }
    }
    else
    {
        // Not all servers have yet received our DATA(p) thus resend
        mp_PDP->_serverPing = true;
        mp_PDP->announceParticipantState(false);
        logInfo(CLIENT_PDP_THREAD, "Client " << mp_PDP->getRTPSParticipant()->getGuid() << " PDP announcement")
    }

    return restart;
}

}
} /* namespace rtps */
} /* namespace eprosima */
