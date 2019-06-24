// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ResendDataPeriod.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include "../../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>


namespace eprosima {
namespace fastrtps{
namespace rtps {


ResendParticipantProxyDataPeriod::ResendParticipantProxyDataPeriod(PDPSimple* p_SPDP,
        double interval):
    TimedEvent(p_SPDP->getRTPSParticipant()->getEventResource().getIOService(),
            p_SPDP->getRTPSParticipant()->getEventResource().getThread(), interval),
    mp_PDP(p_SPDP)
    {


    }

ResendParticipantProxyDataPeriod::~ResendParticipantProxyDataPeriod()
{
    destroy();
}

void ResendParticipantProxyDataPeriod::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_PDP,"ResendDiscoveryData Period");
        //FIXME: Change for liveliness protocol
        mp_PDP->getMutex()->lock();
        mp_PDP->getLocalParticipantProxyData()->m_manualLivelinessCount++;
        mp_PDP->getMutex()->unlock();
        mp_PDP->announceParticipantState(false);

        this->restart_timer();
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_PDP,"Response Data Period aborted");
    }
    else
    {
        logInfo(RTPS_PDP,"message: " <<msg);
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
