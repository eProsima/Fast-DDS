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

                    TimerState(TimedEvent::AUTODESTRUCTION_MODE autodestruction) : code_(INACTIVE),
                    autodestruction_(autodestruction), notify_(true) {}

                    boost::atomic<StateCode> code_;

                    TimedEvent::AUTODESTRUCTION_MODE autodestruction_;

                    bool notify_;
            };
        }
    }
}

using namespace eprosima::fastrtps::rtps;

TimedEventImpl::TimedEventImpl(TimedEvent* event, boost::asio::io_service &service, const boost::thread& event_thread, boost::posix_time::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction) :
timer_(service, interval), m_interval_microsec(interval), mp_event(event),
autodestruction_(autodestruction), state_(std::make_shared<TimerState>(autodestruction)), event_thread_id_(event_thread.get_id())
{
	//TIME_INFINITE(m_timeInfinite);
}

TimedEventImpl::~TimedEventImpl()
{
}

void TimedEventImpl::destroy()
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    // Exchange state to avoid race conditions. Any state will go to state TimerState::DESTROYED.
    TimerState::StateCode code = state_.get()->code_.exchange(TimerState::DESTROYED, boost::memory_order_relaxed);

    // code's value cannot be TimerState::DESTROYED. In this case other destructor was called.
    assert(code != TimerState::DESTROYED);

    // If the event is waiting, cancel it.
    if(code == TimerState::WAITING)
        timer_.cancel();

    // If the destructor is executed in the event thread, tell the future cancelling or running event to not notify.
    if(event_thread_id_ == boost::this_thread::get_id())
    {
        // It is safe to modify this variable because we are in the event thread.
        state_.get()->notify_ = false;
    }

    // If the event is waiting or running, wait it finishes.
    // Don't wait if it is the event thread.
    if((code == TimerState::WAITING || code == TimerState::RUNNING) && event_thread_id_ != boost::this_thread::get_id())
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

    // Exchange state to avoid race conditions. Only TimerState::WAITING state can be set to TimerState::CANCELLED.
    bool ret = state_.get()->code_.compare_exchange_strong(code, TimerState::CANCELLED, boost::memory_order_relaxed);

    if(ret)
    {
        // Unattach the event state from future event execution.
        state_.reset(new TimerState(autodestruction_));
        // Cancel the event.
        timer_.cancel();
        // Alert to user.
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
        // If there is an event running, desattach state and set to not notify.
        if(code == TimerState::RUNNING)
        {
            state_.get()->notify_ = false;
            state_.reset(new TimerState(autodestruction_));
        }

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
        // If autodestruction is TimedEvent::ALLWAYS, delete the event.
        if(scode != TimerState::DESTROYED && state.get()->autodestruction_ == TimedEvent::ALLWAYS)
        {
            delete this->mp_event;
        }
        // If code is TimerState::DESTROYED, then the destructor is waiting because this event were in state TimerState::WAITING.
        else if(scode == TimerState::DESTROYED && state.get()->notify_)
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            cond_.notify_one();
            lock.unlock();
        }

        
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

    if(scode == TimerState::DESTROYED && state.get()->notify_)
        cond_.notify_one();

    //Unlock mutex
    lock.unlock();

    if(state.get()->autodestruction_ == TimedEvent::ALLWAYS ||
            (code == TimedEvent::EVENT_SUCCESS && state.get()->autodestruction_ == TimedEvent::ON_SUCCESS))
    {
        delete this->mp_event;
    }
}
