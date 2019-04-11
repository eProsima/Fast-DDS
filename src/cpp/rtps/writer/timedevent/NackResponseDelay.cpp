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
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

NackResponseDelay::~NackResponseDelay()
{
    destroy();
}

NackResponseDelay::NackResponseDelay(
        StatefulWriter* writer,
        double interval_in_ms)
    : TimedEvent(
            writer->getRTPSParticipant()->getEventResource().getIOService(),
            writer->getRTPSParticipant()->getEventResource().getThread(), 
            interval_in_ms)
    , writer_(writer)
{
}

void NackResponseDelay::event(
        EventCode code, 
        const char* msg)
{
    // Unused in release mode.
    (void)msg;

    if (code == EVENT_SUCCESS)
    {
        logInfo(RTPS_WRITER, "Responding to Acknack messages";);
        writer_->perform_nack_response();
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
