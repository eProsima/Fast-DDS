/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEvent.cpp
 *
 */


#include "eprosimartps/timedevent/TimedEvent.h"
#include <boost/asio/placeholders.hpp>


namespace eprosima{
namespace rtps{


TimedEvent::TimedEvent(boost::asio::io_service* serv,boost::posix_time::milliseconds interval):
		timer(new boost::asio::deadline_timer(*serv,interval)),
		m_interval_msec(interval),
		m_isWaiting(false)
{

}

void TimedEvent::restart_timer()
{

	if(!m_isWaiting)
	{

		m_isWaiting = true;
		timer->expires_from_now(m_interval_msec);
		timer->async_wait(boost::bind(&TimedEvent::event,this,boost::asio::placeholders::error));
	}
}

void TimedEvent::stop_timer()
{
	m_isWaiting = false;
	timer->cancel();
}

bool TimedEvent::update_interval_sec(uint32_t inter)
{
	m_interval_msec = boost::posix_time::milliseconds(inter*1000);
	return true;
}


}
}


