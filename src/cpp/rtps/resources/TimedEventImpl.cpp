/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEventImpl.cpp
 *
 */


#include "TimedEventImpl.h"
#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>


using namespace eprosima::fastrtps::rtps;

TimedEventImpl::TimedEventImpl(TimedEvent* event,boost::asio::io_service* serv, boost::posix_time::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction):
				timer(new boost::asio::deadline_timer(*serv,interval)),
				m_interval_microsec(interval),
				mp_event(event),
				m_isWaiting(false),
                isRunning_(false),
                autodestruction_(autodestruction),
				mp_stopSemaphore(new boost::interprocess::interprocess_semaphore(0))
{
	//TIME_INFINITE(m_timeInfinite);
}

TimedEventImpl::~TimedEventImpl()
{
	delete(timer);
	delete(mp_stopSemaphore);
}

void TimedEventImpl::restart_timer()
{
    boost::unique_lock<boost::mutex> lock(mutex_);

    // If not running restart the event.
	if(!m_isWaiting)
	{
		m_isWaiting = true;
		timer->expires_from_now(m_interval_microsec);
		timer->async_wait(boost::bind(&TimedEventImpl::event,this,boost::asio::placeholders::error));
	}
}

void TimedEventImpl::stop_timer()
{
    boost::unique_lock<boost::mutex> lock(mutex_);

    // If the event is in execution, wait to be finished
    if(isRunning_)
    {
        cond_.wait(lock);
    }

    // If the event is waiting to be executed, cancel it and wait until the abortion.
	if(m_isWaiting)
	{
        m_isWaiting = false;
        lock.unlock();
        timer->cancel();
        this->mp_stopSemaphore->wait();
	}
}

bool TimedEventImpl::update_interval(const Duration_t& inter)
{
	m_interval_microsec = boost::posix_time::microseconds(TimeConv::Time_t2MicroSecondsInt64(inter));
	return true;
}

bool TimedEventImpl::update_interval_millisec(double time_millisec)
{
	m_interval_microsec = boost::posix_time::microseconds((int64_t)(time_millisec*1000));
	return true;
}

void TimedEventImpl::event(const boost::system::error_code& ec)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    TimedEvent::EventCode code = TimedEvent::EVENT_MSG;
    const char *message = nullptr;

    if(ec == boost::system::errc::success)
        code = TimedEvent::EVENT_SUCCESS;
    else if(ec == boost::asio::error::operation_aborted || !m_isWaiting)
        code = TimedEvent::EVENT_ABORT;
    else
        message = ec.message().c_str();

    m_isWaiting = false;
    isRunning_ = true;
    lock.unlock();

    this->mp_event->event(code, message);

    // In a normal execution, warn the execution is finished.
    lock.lock();
    isRunning_ = false;
    lock.unlock();
    cond_.notify_one();

    if(autodestruction_ == TimedEvent::ALLWAYS)
        delete this->mp_event;
    else if(code == TimedEvent::EVENT_SUCCESS && autodestruction_ == TimedEvent::ON_SUCCESS)
        delete this->mp_event;
    else if(code == TimedEvent::EVENT_ABORT)
        mp_stopSemaphore->post();
}
