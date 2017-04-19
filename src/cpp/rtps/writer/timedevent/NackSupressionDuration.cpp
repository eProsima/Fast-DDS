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
 * @file NackSupressionDuration.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
#include "../../participant/RTPSParticipantImpl.h"
#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


NackSupressionDuration::~NackSupressionDuration()
{
    destroy();
}

NackSupressionDuration::NackSupressionDuration(ReaderProxy* p_RP,double millisec):
TimedEvent(p_RP->mp_SFW->getRTPSParticipant()->getEventResource().getIOService(),
p_RP->mp_SFW->getRTPSParticipant()->getEventResource().getThread(), millisec),
mp_RP(p_RP)
{

}

void NackSupressionDuration::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        std::lock_guard<std::recursive_mutex> guard(*mp_RP->mp_mutex);

        logInfo(RTPS_WRITER,"Changing underway to unacked for Reader: "<<mp_RP->m_att.guid);

        if(mp_RP->m_att.endpoint.reliabilityKind == RELIABLE)
        {
            mp_RP->convert_status_on_all_changes(UNDERWAY,UNACKNOWLEDGED);
            mp_RP->mp_SFW->mp_periodicHB->restart_timer();
        }
    }
    else if(code == EVENT_ABORT)
    {
        logInfo(RTPS_WRITER,"Aborted");
    }
    else
    {
        logInfo(RTPS_WRITER,"Event message: " <<msg);
    }
}
}
} /* namespace dds */
} /* namespace eprosima */
