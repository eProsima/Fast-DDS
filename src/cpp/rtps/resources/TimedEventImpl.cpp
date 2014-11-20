/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEventImpl.cpp
 *
 */


#include "eprosimartps/rtps/resources/TimedEventImpl.h"
#include "eprosimartps/rtps/resources/TimedEvent.h"
#include "eprosimartps/utils/TimeConversion.h"


namespace eprosima{
namespace rtps{


TimedEventImpl::TimedEventImpl(TimedEvent* event,boost::asio::io_service* serv,boost::posix_time::milliseconds interval):
				timer(new boost::asio::deadline_timer(*serv,interval)),
				m_interval_msec(interval),
				mp_event(event),
				m_isWaiting(false),
				mp_stopSemaphore(new boost::interprocess::interprocess_semaphore(0))
{
	//TIME_INFINITE(m_timeInfinite);
}

TimedEventImpl::~TimedEventImpl()
{
	stop_timer();
	delete(timer);
	delete(mp_stopSemaphore);
}

void TimedEventImpl::restart_timer()
{
	if(!m_isWaiting)
	{
		m_isWaiting = true;
		timer->expires_from_now(m_interval_msec);
		timer->async_wait(boost::bind(&TimedEventImpl::event,this,boost::asio::placeholders::error));
	}
}

void TimedEventImpl::stop_timer()
{
	if(m_isWaiting)
	{
		timer->cancel();
		this->mp_stopSemaphore->wait();
	}
}

bool TimedEventImpl::update_interval(const Duration_t& inter)
{
	m_interval_msec = boost::posix_time::milliseconds(TimeConv::Time_t2MilliSecondsInt64(inter));
	return true;
}

bool TimedEventImpl::update_interval_millisec(int64_t time_millisec)
{
	m_interval_msec = boost::posix_time::milliseconds(time_millisec);
	return true;
}

void TimedEventImpl::event(const boost::system::error_code& ec)
{
	m_isWaiting = false;
	if(ec == boost::system::errc::success)
	{
		this->mp_event->event(TimedEvent::EVENT_SUCCESS);
	}
	else if(ec==boost::asio::error::operation_aborted)
	{
		this->mp_event->event(TimedEvent::EVENT_ABORT);
	}
	else
	{
		this->mp_event->event(TimedEvent::EVENT_MSG,ec.message().c_str());
	}
}


}
}


