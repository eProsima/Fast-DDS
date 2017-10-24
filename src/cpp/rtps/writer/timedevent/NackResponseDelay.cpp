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
 * @file NackResponseDelay.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>
#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/rtps/writer/timedevent/PeriodicHeartbeat.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/writer/ReaderProxy.h>
#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <mutex>

using namespace eprosima::fastrtps::rtps;

NackResponseDelay::~NackResponseDelay()
{
    destroy();
}

NackResponseDelay::NackResponseDelay(ReaderProxy* p_RP,double millisec):
    TimedEvent(p_RP->mp_SFW->getRTPSParticipant()->getEventResource().getIOService(),
            p_RP->mp_SFW->getRTPSParticipant()->getEventResource().getThread(), millisec),
    mp_RP(p_RP)
{
}

void NackResponseDelay::event(EventCode code, const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_WRITER,"Responding to Acknack msg";);
        std::lock_guard<std::recursive_mutex> guardW(*mp_RP->mp_SFW->getMutex());
        std::lock_guard<std::recursive_mutex> guard(*mp_RP->mp_mutex);
        mp_RP->convert_status_on_all_changes(REQUESTED, UNSENT);
    }
}
