/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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
