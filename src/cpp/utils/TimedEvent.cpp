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


#include "eprosimartps/utils/TimedEvent.h"
#include <boost/asio/placeholders.hpp>


namespace eprosima{
namespace rtps{


TimedEvent::TimedEvent(boost::asio::io_service* serv,boost::posix_time::milliseconds interval):
		timer(new boost::asio::deadline_timer(*serv,interval)),
		m_interval_msec(interval),
		m_isWaiting(false)
{
	//TIME_INFINITE(m_timeInfinite);
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

bool TimedEvent::update_interval(const Duration_t& inter)
{
	m_interval_msec = boost::posix_time::milliseconds(Time_t2MilliSec(inter));
	return true;
}

bool TimedEvent::update_interval_millisec(double time_millisec)
{
	m_interval_msec = boost::posix_time::milliseconds(time_millisec);
	return true;
}


}
}


