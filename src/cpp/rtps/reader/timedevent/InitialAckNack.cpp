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
 * @file InitialAckNack.cpp
 *
 */

#include <fastrtps/rtps/reader/timedevent/InitialAckNack.h>
#include <mutex>

#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps{

InitialAckNack::~InitialAckNack()
{
    logInfo(RTPS_WRITER,"Destroying InitialAckNack");
    destroy();
}

InitialAckNack::InitialAckNack(
        WriterProxy* writer_proxy,
        double interval)
    : TimedEvent(
            writer_proxy->get_participant()->getEventResource().getIOService(),
            writer_proxy->get_participant()->getEventResource().getThread(),
            interval)
    , message_buffer_(
            writer_proxy->get_participant()->getMaxMessageSize(),
            writer_proxy->get_participant()->getGuid().guidPrefix)
    , writer_proxy_(writer_proxy)
{
}

void InitialAckNack::event(
        EventCode code,
        const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        writer_proxy_->perform_initial_ack_nack(message_buffer_);
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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
