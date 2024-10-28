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

#include <rtps/builtin/discovery/participant/timedevent/DServerEvent.hpp>

#include <fastdds/dds/log/Log.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/ResourceEvent.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

DServerRoutineEvent::DServerRoutineEvent(
        PDPServer* pdp,
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

DServerRoutineEvent::~DServerRoutineEvent()
{
}

bool DServerRoutineEvent::server_routine_event()
{
    bool pending_work = pdp_->server_update_routine();
    if (pending_work)
    {
        // Update timer to the server routine period (Default: 450 ms)
        pdp_->awake_routine_thread(server_routine_period_);
    }

    return pending_work;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
