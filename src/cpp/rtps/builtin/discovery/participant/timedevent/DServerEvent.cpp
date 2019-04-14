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
 * @file DServerEvent.cpp
 *
 */

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/rtps/resources/ResourceEvent.h>

#include "../../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/DServerEvent.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


DServerEvent::DServerEvent(PDPServer* p_PDP,
        double interval):
    TimedEvent(p_PDP->getRTPSParticipant()->getEventResource().getIOService(),
            p_PDP->getRTPSParticipant()->getEventResource().getThread(), interval),
    mp_PDP(p_PDP)
    {

    }

DServerEvent::~DServerEvent()
{
    destroy();
}

void DServerEvent::event(EventCode code, const char* msg)
{
    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(SERVER_PDP_THREAD,"DServerEvent Period");

        std::lock_guard<std::recursive_mutex> lock(*mp_PDP->getMutex());
        bool restart = false;

        // Check Server matching
        if (mp_PDP->all_servers_acknowledge_PDP())
        {
            // Wait until we have received all network discovery info currently available
            if (mp_PDP->is_all_servers_PDPdata_updated())
            {
                restart |= !mp_PDP->match_servers_EDP_endpoints();
                // we must keep this TimedEvent alive to cope with servers' shutdown
                // PDPServer::removeRemoteEndpoints would restart_timer if a server vanishes
            }
            else
            {
                logInfo(SERVER_PDP_THREAD, "Not all servers acknowledge PDP info")
                restart = true;
            }
        }
        else
        {   // awake the other servers
            mp_PDP->announceParticipantState(false);
            restart = true;
        }

        // Check EDP matching
        if (mp_PDP->pendingEDPMatches())
        {
            if (mp_PDP->all_clients_acknowledge_PDP())
            {
                // Do the matching
                mp_PDP->match_all_clients_EDP_endpoints();
                // Whenever new clients appear restart_timer()
                // see PDPServer::queueParticipantForEDPMatch

                logInfo(SERVER_PDP_THREAD, "Client PDP points matched")
            }
            else
            {   // keep trying the match
                restart = true;  

                logInfo(SERVER_PDP_THREAD, "Not all clients acknowledge PDP info")
            }
        }  

        if (mp_PDP->pendingHistoryCleaning())
        {
            restart |= !mp_PDP->trimWriterHistory();

            logInfo(SERVER_PDP_THREAD, "trimming PDP history from removed endpoints")
        }

        if (restart)
        {
            restart_timer();
        }

    }
    else if(code == EVENT_ABORT)
    {
        logInfo(SERVER_PDP_THREAD,"DServerEvent aborted");
    }
    else
    {
        logInfo(SERVER_PDP_THREAD,"message: " <<msg);
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
