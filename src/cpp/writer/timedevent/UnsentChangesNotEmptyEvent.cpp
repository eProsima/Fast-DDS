/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file UnsentChangesNotEmptyEvent.cpp
 *
 */

#include "eprosimartps/writer/timedevent/UnsentChangesNotEmptyEvent.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/resources/ResourceEvent.h"
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "UnsentChangesNotEmptyEvent";

UnsentChangesNotEmptyEvent::UnsentChangesNotEmptyEvent(RTPSWriter* writer,boost::posix_time::milliseconds interval)
: TimedEvent(&writer->mp_event_thr->io_service,interval),
  mp_writer(writer)
{
	// TODO Auto-generated constructor stub

}

UnsentChangesNotEmptyEvent::~UnsentChangesNotEmptyEvent()
{
	stop_timer();
	delete(timer);
}

void UnsentChangesNotEmptyEvent::event(const boost::system::error_code& ec)
{
	const char* const METHOD_NAME = "event";
	logInfo(RTPS_WRITER,"");
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		mp_writer->unsent_changes_not_empty();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_WRITER,"UnsentChangesNotEmpty aborted");
		this->mp_stopSemaphore->post();
	}
	else
	{
		logInfo(RTPS_WRITER,"UnsentChangesNotEmpty boost message: " <<ec.message());
	}
	delete(this);
}


} /* namespace rtps */
} /* namespace eprosima */
