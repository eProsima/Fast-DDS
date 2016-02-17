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

TimedEventImpl::TimedEventImpl(TimedEvent* event, boost::asio::io_service &service, const boost::thread& event_thread, boost::posix_time::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction) :
timer_(service, interval), m_interval_microsec(interval), mp_event(event),
autodestruction_(autodestruction), state_(std::make_shared<TimerState>()), event_thread_id_(event_thread.get_id())
{
	//TIME_INFINITE(m_timeInfinite);
}

TimedEventImpl::~TimedEventImpl()
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    // Exchange state to avoid race conditions. Any state will go to state DESTROYED.
    TimerState::StateCode code = state_.get()->code_.exchange(TimerState::DESTROYED, boost::memory_order_relaxed);

    // code's value cannot be DESTROYED. In this case other destructor was called.
    assert(code != TimerState::DESTROYED);

    // If the event is running, wait it finishes.
    // Don't bother to wait here. If running the event, this is not the event thread.
    if(code == TimerState::RUNNING && event_thread_id_ != boost::this_thread::get_id())
        cond_.wait(lock);
}


/* In this function we don't need to exchange the state,
 * because this function try to cancel, but if the event is running
 * in the middle of the operation, it doesn't bother.
 */
void TimedEventImpl::cancel_timer()
{
    TimerState::StateCode code = TimerState::WAITING;

    // Lock timer to protect state_ and timer_ objects.
    boost::unique_lock<boost::mutex> lock(mutex_);

    // Exchange state to avoid race conditions. Only WAITING state can be set to CANCELLED.
    bool ret = state_.get()->code_.compare_exchange_strong(code, TimerState::CANCELLED, boost::memory_order_relaxed);

    if(ret)
    {
        timer_.cancel();
        mp_event->event(TimedEvent::EVENT_ABORT, nullptr);
    }
}

void TimedEventImpl::restart_timer()
{
    // Lock timer to protect state_ and timer_ objects.
    boost::unique_lock<boost::mutex> lock(mutex_);

    // Get current state.
    TimerState::StateCode code = state_.get()->code_.load(boost::memory_order_relaxed);

    // if the code is executed in the event thread, and the event is being destroyed, don't start other event.
    // if the code indicate an event is already waiting, don't start other event.
    if(code != TimerState::DESTROYED && code != TimerState::WAITING)
    {
        // If the event is running, reuse the state. In other case the event can be destroyed but there is no more
        // access to this running event information.
        if(code != TimerState::RUNNING && code != TimerState::INACTIVE)
            state_.reset(new TimerState());
        state_.get()->code_.store(TimerState::WAITING, boost::memory_order_relaxed);

        timer_.expires_from_now(m_interval_microsec);
        timer_.async_wait(boost::bind(&TimedEventImpl::event,this,boost::asio::placeholders::error, state_));
    }
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
    TimerState::StateCode scode = TimerState::WAITING;

    // First step is exchange the state, to prevent race condition from the destruction case.
    bool ret =  state.get()->code_.compare_exchange_strong(scode, TimerState::RUNNING, boost::memory_order_relaxed);

    // Check bad preconditions
    assert(!(ret && scode == TimerState::DESTROYED));

    if(scode != TimerState::WAITING || !ret || ec == boost::asio::error::operation_aborted)
    {
        if(scode != TimerState::DESTROYED && autodestruction_ == TimedEvent::ALLWAYS)
            delete this->mp_event;
        
        return;
    }

    TimedEvent::EventCode code = TimedEvent::EVENT_MSG;
    const char *message = nullptr;

    if(ec == boost::system::errc::success)
        code = TimedEvent::EVENT_SUCCESS;
    else
        message = ec.message().c_str();

    this->mp_event->event(code, message);

    // If the destructor is waiting, signal it.
    boost::unique_lock<boost::mutex> lock(mutex_);

    scode = TimerState::RUNNING;
    ret =  state.get()->code_.compare_exchange_strong(scode, TimerState::INACTIVE, boost::memory_order_relaxed);

    cond_.notify_one();

    if(autodestruction_ == TimedEvent::ALLWAYS ||
            (code == TimedEvent::EVENT_SUCCESS && autodestruction_ == TimedEvent::ON_SUCCESS))
    {
        //Unlock mutex before delete the event.
        lock.unlock();
        delete this->mp_event;
    }
}
