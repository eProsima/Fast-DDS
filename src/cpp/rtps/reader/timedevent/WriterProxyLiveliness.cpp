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
 * @file WriterProxyLiveliness.cpp
 *
 */

#include <fastrtps/rtps/reader/timedevent/WriterProxyLiveliness.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/common/MatchingInfo.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>



namespace eprosima {
namespace fastrtps{
namespace rtps {


WriterProxyLiveliness::WriterProxyLiveliness(StatefulReader* reader)
    : TimedEvent(
        reader->getRTPSParticipant()->getEventResource().getIOService(),
        reader->getRTPSParticipant()->getEventResource().getThread(), 0)
    , reader_(reader)
    , writer_guid_()
{
}

WriterProxyLiveliness::~WriterProxyLiveliness()
{
    destroy();
}

void WriterProxyLiveliness::start(
        const GUID_t& writer_guid,
        const Duration_t& interval)
{
    writer_guid_ = writer_guid;
    update_interval(interval);
    cancel_timer();
    restart_timer();
}

void WriterProxyLiveliness::event(
        EventCode code, 
        const char* msg)
{

    // Unused in release mode.
    (void) msg;

	if(code == EVENT_SUCCESS)
    {
        logInfo(RTPS_LIVELINESS, "Deleting Writer: " << writer_guid_);
		reader_->liveliness_expired(writer_guid_);
	}
	else if(code == EVENT_ABORT)
	{
        logInfo(RTPS_LIVELINESS, "WriterProxyLiveliness aborted");
	}
	else
	{
		logInfo(RTPS_LIVELINESS,"message: " <<msg);
	}
}


}
} /* namespace rtps */
} /* namespace eprosima */
