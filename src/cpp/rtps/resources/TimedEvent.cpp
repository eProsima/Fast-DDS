/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEvent.cpp
 *
 */

#include <fastrtps/rtps/resources/TimedEvent.h>
#include "TimedEventImpl.h"



namespace eprosima {
namespace fastrtps{
namespace rtps {

    TimedEvent::TimedEvent(boost::asio::io_service &service, const boost::thread& event_thread, double milliseconds, TimedEvent::AUTODESTRUCTION_MODE autodestruction)
{
	mp_impl = new TimedEventImpl(this, service, event_thread, boost::posix_time::microseconds((int64_t)(milliseconds*1000)), autodestruction);
}

TimedEvent::~TimedEvent()
{
	delete(mp_impl);
}

void TimedEvent::cancel_timer()
{
	mp_impl->cancel_timer();
}


void TimedEvent::restart_timer()
{
	mp_impl->restart_timer();
}

bool TimedEvent::update_interval(const Duration_t& inter)
{
	return mp_impl->update_interval(inter);
}

bool TimedEvent::update_interval_millisec(double time_millisec)
{
	return mp_impl->update_interval_millisec(time_millisec);
}

double TimedEvent::getIntervalMilliSec()
{
	return mp_impl->getIntervalMsec();
}

double TimedEvent::getRemainingTimeMilliSec()
{
	return mp_impl->getRemainingTimeMilliSec();
}

}
} /* namespace rtps */
} /* namespace eprosima */
