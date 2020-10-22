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
 * @file DServerEvent2.cpp
 *
 */

#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastdds/rtps/resources/ResourceEvent.h>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>

#include "./DServerEvent2.hpp"
#include "./PDPServer2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

DServerRoutineEvent2::DServerRoutineEvent2(
        PDPServer2* pdp,
        double server_routine_period)
    : TimedEvent(pdp->get_resource_event_thread(),
            [this]()
            {
                return server_routine_event();
            }, 0)
    , pdp_(pdp)
    , server_routine_period_(server_routine_period)
{

}

DServerRoutineEvent2::~DServerRoutineEvent2()
{
}

bool DServerRoutineEvent2::server_routine_event()
{
    // logInfo(SERVER_PDP_THREAD, "Server " << pdp->getRTPSParticipant()->getGuid() << " DServerEvent Period");

    bool pending_work = pdp_->server_update_routine();
    if (pending_work)
    {
        // Update timer to the server routine period (Default: 450 ms)
        pdp_->awake_routine_thread(server_routine_period_);
    }

    return pending_work;
}

DServerPingEvent2::DServerPingEvent2(
        PDPServer2* pdp,
        double interval)
    : TimedEvent(pdp->getRTPSParticipant()->getEventResource(),
            [this]()
            {
                return server_ping_event();
            }, interval)
    , pdp_(pdp)
{

}

DServerPingEvent2::~DServerPingEvent2()
{
}

bool DServerPingEvent2::server_ping_event()
{
    // Check if all servers received my discovery data
    if (!pdp_->all_servers_acknowledge_pdp())
    {
        // Not all servers have yet received our DATA(p) thus resend
        pdp_->ping_remote_servers();
        logInfo(SERVER_PING_THREAD, "Server " << pdp_->getRTPSParticipant()->getGuid() << " PDP announcement");

        // restart
        return true;
    }

    // do not restart
    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
