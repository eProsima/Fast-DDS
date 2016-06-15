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
 * @file UnsentChangesNotEmptyEvent.cpp
 *
 */

#include <fastrtps/rtps/writer/timedevent/UnsentChangesNotEmptyEvent.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include "../../participant/RTPSParticipantImpl.h"

#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "UnsentChangesNotEmptyEvent";

UnsentChangesNotEmptyEvent::UnsentChangesNotEmptyEvent(RTPSWriter* writer,
		double interval):
        TimedEvent(writer->getRTPSParticipant()->getEventResource().getIOService(),
        writer->getRTPSParticipant()->getEventResource().getThread(), interval, NONE),
        mp_writer(writer)
{
	// TODO Auto-generated constructor stub
}

UnsentChangesNotEmptyEvent::~UnsentChangesNotEmptyEvent()
{
    destroy();
}

void UnsentChangesNotEmptyEvent::event(EventCode code, const char* msg)
{
	const char* const METHOD_NAME = "event";

    // Unused in release mode.
    (void)msg;

	if(code == EVENT_SUCCESS)
	{
        mp_writer->unsent_changes_not_empty();
	}
	else if(code == EVENT_ABORT)
	{
		logInfo(RTPS_WRITER,"UnsentChangesNotEmpty aborted");
	}
	else
	{
		logInfo(RTPS_WRITER,"UnsentChangesNotEmpty boost message: " <<msg);
	}
}

}
} /* namespace rtps */
} /* namespace eprosima */
