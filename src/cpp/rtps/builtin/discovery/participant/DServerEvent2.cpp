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
        PDPServer2* p_PDP,
        double interval)
    : TimedEvent(p_PDP->getRTPSParticipant()->getEventResource(),
            [this]()
            {
                return event();
            }, interval)
    , mp_PDP(p_PDP)
{

}

DServerEvent2::~DServerEvent2()
{
}

bool DServerEvent2::event()
{
    logInfo(SERVER_PDP_THREAD, "Server " << mp_PDP->getRTPSParticipant()->getGuid() << " DServerEvent Period");

    /*
     * TODO: Management of other server should be done here
     */

    bool pending_work = mp_PDP->server_update_routine();
    if (pending_work)
    {
        mp_PDP->awakeServerThread();
    }

    return pending_work;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
