/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEvent.cpp
 *
 */

#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/rtps/resources/TimedEventImpl.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

TimedEvent::TimedEvent(boost::asio::io_service* serv,double milliseconds)
{
	mp_impl = new TimedEventImpl(this,serv,boost::posix_time::milliseconds(milliseconds));
}

TimedEvent::~TimedEvent()
{
	delete(mp_impl);
}

void TimedEvent::restart_timer()
{
	mp_impl->restart_timer();
}

void TimedEvent::stop_timer()
{
	mp_impl->stop_timer();
}

bool TimedEvent::update_interval(const Duration_t& inter)
{
	return mp_impl->update_interval(inter);
}

bool TimedEvent::update_interval_millisec(int64_t time_millisec)
{
	return mp_impl->update_interval_millisec(time_millisec);
}

double TimedEvent::getIntervalMilliSec()
{
	return mp_impl->getIntervalMsec();
}
}
} /* namespace rtps */
} /* namespace eprosima */
