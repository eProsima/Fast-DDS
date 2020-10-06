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

DServerEvent2::DServerEvent2(
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

DServerEvent2::~DServerEvent2()
{
}

bool DServerEvent2::server_routine_event()
{
    // logInfo(SERVER_PDP_THREAD, "Server " << pdp->getRTPSParticipant()->getGuid() << " DServerEvent Period");

    /*
     * TODO: Management of other server should be done here
     */

    bool pending_work = pdp_->server_update_routine();
    if (pending_work)
    {
        // Update timer to the server routine period (Default: 450 ms)
        pdp_->awake_server_thread(server_routine_period_);
    }

    return pending_work;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
