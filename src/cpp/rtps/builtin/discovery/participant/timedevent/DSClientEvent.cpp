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

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/rtps/resources/ResourceEvent.h>

#include "../../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>


namespace eprosima {
namespace fastrtps{
namespace rtps {


 DSClientEvent::DSClientEvent(PDPServer* p_PDP,
        double interval):
    TimedEvent(p_PDP->getRTPSParticipant()->getEventResource().getIOService(),
            p_PDP->getRTPSParticipant()->getEventResource().getThread(), interval),
    mp_PDP(p_PDP)
    {

    }

 DSClientEvent::~DSClientEvent()
{
    destroy();
}

void DSClientEvent::event(EventCode code, const char* msg)
{
    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_PDP,"DSClientEvent Period");

        // Check if all servers received my discovery data
        if (mp_PDP->all_servers_acknowledge_PDP())
        {
            // Wait until we have received all network discovery info currently available
            if (mp_PDP->is_all_servers_PDPdata_updated())
            {
                mp_PDP->match_all_server_EDP_endpoints();
                // we must keep this TimedEvent alive to cope with servers' shutdown
            }
        }
        else
        { 
            // Not all servers have yet received our DATA(p) thus resend
            mp_PDP->announceParticipantState(false);
        }

        restart_timer();
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_PDP,"DSClientEvent aborted");
    }
    else
    {
        logInfo(RTPS_PDP,"message: " <<msg);
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
