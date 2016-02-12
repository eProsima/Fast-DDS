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

#include <boost/atomic.hpp>
#include <cassert>

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            class TimerState
            {
                public:

                    typedef enum
                    {
                        INACTIVE = 0,
                        WAITING,
                        CANCELLED,
                        RUNNING,
                        DESTROYED
                    } StateCode;

                    TimerState() : code_(INACTIVE) {}

                    boost::atomic<StateCode> code_;
            };
        }
    }
}

using namespace eprosima::fastrtps::rtps;

TimedEventImpl::TimedEventImpl(TimedEvent* event,boost::asio::io_service* serv, boost::posix_time::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction):
				timer_(*serv,interval),
				m_interval_microsec(interval),
				mp_event(event),
                autodestruction_(autodestruction),
                state_(std::make_shared<TimerState>())
{
	//TIME_INFINITE(m_timeInfinite);
}

TimedEventImpl::~TimedEventImpl()
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    // Exchange state to avoid race conditions.
    TimerState::StateCode code = state_.get()->code_.exchange(TimerState::DESTROYED, boost::memory_order_relaxed);
    // code cannot be DESTROYED. In this case it is wrong usage of class.
    assert(code != TimerState::DESTROYED);


    // Don't bother to wait here. If running the event, this is not the event thread.
    if(code == TimerState::RUNNING)
        cond_.wait(lock);
}


/* In this function we don't need to exchange the state,
 * because this function try to cancel, but if the event is running
 * in the middle of the operation, it doesn't bother.
 */
void TimedEventImpl::cancel_timer()
{
    // Lock timer to protect state_ and timer_ objects.
    boost::unique_lock<boost::mutex> lock(mutex_);

    // Exchange state to avoid race conditions.
    TimerState::StateCode code = state_.get()->code_.exchange(TimerState::CANCELLED, boost::memory_order_relaxed);
    // code cannot be DESTROYED. In this case it is wrong usage of class.
    assert(code != TimerState::DESTROYED);

    if(code == TimerState::WAITING)
        timer_.cancel();
}

void TimedEventImpl::restart_timer()
{
    cancel_timer();

    // Lock timer to protect state_ and timer_ objects.
    boost::unique_lock<boost::mutex> lock(mutex_);
    state_.reset(new TimerState());
    state_.get()->code_.store(TimerState::WAITING, boost::memory_order_relaxed);

    timer_.expires_from_now(m_interval_microsec);
    timer_.async_wait(boost::bind(&TimedEventImpl::event,this,boost::asio::placeholders::error, state_));
}

bool TimedEventImpl::update_interval(const Duration_t& inter)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
	m_interval_microsec = boost::posix_time::microseconds(TimeConv::Time_t2MicroSecondsInt64(inter));
	return true;
}

bool TimedEventImpl::update_interval_millisec(double time_millisec)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
	m_interval_microsec = boost::posix_time::microseconds((int64_t)(time_millisec*1000));
	return true;
}

void TimedEventImpl::event(const boost::system::error_code& ec, const std::shared_ptr<TimerState>& state)
{
    // First step is exchange the state, to prevent race condition from the destruction case.
    TimerState::StateCode scode = state.get()->code_.exchange(TimerState::RUNNING, boost::memory_order_relaxed);

    if(scode == TimerState::DESTROYED)
        return;

    TimedEvent::EventCode code = TimedEvent::EVENT_MSG;
    const char *message = nullptr;

    if(ec == boost::asio::error::operation_aborted || scode != TimerState::WAITING)
        code = TimedEvent::EVENT_ABORT;
    else if(ec == boost::system::errc::success)
        code = TimedEvent::EVENT_SUCCESS;
    else
        message = ec.message().c_str();

    this->mp_event->event(code, message);

    
    state.get()->code_.store(TimerState::INACTIVE, boost::memory_order_relaxed);
    // If the destructor is waiting, signal it.
    cond_.notify_one();

    if(autodestruction_ == TimedEvent::ALLWAYS ||
            (code == TimedEvent::EVENT_SUCCESS && autodestruction_ == TimedEvent::ON_SUCCESS))
        delete this->mp_event;
}
