// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file TimedEventImpl.cpp
 *
 */


#include "TimedEventImpl.h"
#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>

#include <cassert>
#include <functional>
#include <atomic>
#include <system_error>

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
                    autodestruction_(autodestruction), forwardRestart_(false) {}

                    std::atomic<StateCode> code_;

                    TimedEvent::AUTODESTRUCTION_MODE autodestruction_;

                    bool forwardRestart_;
            };
        }
    }
}

using namespace eprosima::fastrtps::rtps;

TimedEventImpl::TimedEventImpl(TimedEvent* event, asio::io_service &service, const std::thread& event_thread, std::chrono::microseconds interval, TimedEvent::AUTODESTRUCTION_MODE autodestruction) :
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
    std::unique_lock<std::mutex> lock(mutex_);
    // Exchange state to avoid race conditions. Any state will go to state TimerState::DESTROYED.
    TimerState::StateCode code = state_.get()->code_.exchange(TimerState::DESTROYED, std::memory_order_relaxed);

    // code's value cannot be TimerState::DESTROYED. In this case other destructor was called.
    assert(code != TimerState::DESTROYED);

    // If the event is waiting, cancel it.
    if(code == TimerState::WAITING)
        timer_.cancel();

    // If the event is waiting or running, wait it finishes.
    // Don't wait if it is the event thread.
    if(code == TimerState::RUNNING && event_thread_id_ != std::this_thread::get_id())
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
    std::unique_lock<std::mutex> lock(mutex_);

    // Exchange state to avoid race conditions. Only TimerState::WAITING state can be set to TimerState::CANCELLED.
    bool ret = state_.get()->code_.compare_exchange_strong(code, TimerState::CANCELLED, std::memory_order_relaxed);

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
    std::unique_lock<std::mutex> lock(mutex_);

    // Get current state.
    TimerState::StateCode code = state_.get()->code_.load(std::memory_order_relaxed);

    // if the code is executed in the event thread, and the event is being destroyed, don't start other event.
    // if the code indicate an event is already waiting, don't start other event.
    if(code != TimerState::DESTROYED && code != TimerState::WAITING)
    {
        bool restartTimer = true;

        // If there is an event running, desattach state and set to not notify.
        if(code == TimerState::RUNNING)
        {
            if(state_.get()->forwardRestart_)
                restartTimer = false;
            else
                state_.get()->forwardRestart_ = true;
        }
        else
            state_.get()->code_.store(TimerState::WAITING, std::memory_order_relaxed);

        if(restartTimer)
        {
            timer_.expires_from_now(m_interval_microsec);
            timer_.async_wait(std::bind(&TimedEventImpl::event,this,std::placeholders::_1, state_));
        }
    }
}

bool TimedEventImpl::update_interval(const Duration_t& inter)
{
    std::unique_lock<std::mutex> lock(mutex_);
	m_interval_microsec = std::chrono::microseconds(TimeConv::Time_t2MicroSecondsInt64(inter));
	return true;
}

bool TimedEventImpl::update_interval_millisec(double time_millisec)
{
    std::unique_lock<std::mutex> lock(mutex_);
	m_interval_microsec = std::chrono::microseconds((int64_t)(time_millisec*1000));
	return true;
}

void TimedEventImpl::event(const std::error_code& ec, const std::shared_ptr<TimerState>& state)
{
    TimerState::StateCode scode = TimerState::WAITING;

    // First step is exchange the state, to prevent race condition from the destruction case.
    bool ret =  state.get()->code_.compare_exchange_strong(scode, TimerState::RUNNING, std::memory_order_relaxed);

    // Check bad preconditions
    assert(!(ret && scode == TimerState::DESTROYED));

    if(scode != TimerState::WAITING || !ret || ec == asio::error::operation_aborted)
    {
        // If autodestruction is TimedEvent::ALLWAYS, delete the event.
        if(scode != TimerState::DESTROYED && state.get()->autodestruction_ == TimedEvent::ALLWAYS)
        {
            delete this->mp_event;
        }

        return;
    }

    TimedEvent::EventCode code = TimedEvent::EVENT_MSG;
    std::string message;

    if(!ec)
        code = TimedEvent::EVENT_SUCCESS;
    else
        message = ec.message();

    this->mp_event->event(code, message.c_str());

    // If the destructor is waiting, signal it.
    std::unique_lock<std::mutex> lock(mutex_);

    scode = TimerState::RUNNING;
    if(!state.get()->forwardRestart_)
    {
        ret =  state.get()->code_.compare_exchange_strong(scode, TimerState::INACTIVE, std::memory_order_relaxed);
    }
    else
    {
        state.get()->forwardRestart_ = false;
        ret =  state.get()->code_.compare_exchange_strong(scode, TimerState::WAITING, std::memory_order_relaxed);
    }

    if(scode == TimerState::DESTROYED)
        cond_.notify_one();

    //Unlock mutex
    lock.unlock();

    if(state.get()->autodestruction_ == TimedEvent::ALLWAYS ||
            (code == TimedEvent::EVENT_SUCCESS && state.get()->autodestruction_ == TimedEvent::ON_SUCCESS))
    {
        delete this->mp_event;
    }
}
