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

namespace eprosima {
namespace rtps {

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
	pDebugInfo("TimedEvent: UnsentChangesNotEmpty"<<endl;);
	this->m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		mp_writer->unsent_changes_not_empty();
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		pWarning("UnsentChangesNotEmpty aborted"<<endl);
		this->mp_stopSemaphore->post();
	}
	else
	{
		pInfo("UnsentChangesNotEmpty boost message: " <<ec.message()<<endl);
	}
	delete(this);
}


} /* namespace rtps */
} /* namespace eprosima */
