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


#include <rtps/resources/TimedEventImpl.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>

#include <cassert>
#include <functional>
#include <atomic>
#include <system_error>

using namespace eprosima::fastrtps::rtps;

TimedEventImpl::TimedEventImpl(
        asio::io_service& service,
        Callback callback,
        std::chrono::microseconds interval)
    : m_interval_microsec(interval)
    , timer_(service, interval)
    , callback_(callback)
    , callback_ptr_(&callback, [](Callback*){
})
    , state_(StateCode::INACTIVE)
    , cancel_(false)
    , next_(nullptr)
{
    //TIME_INFINITE(m_timeInfinite);
}

TimedEventImpl::~TimedEventImpl()
{
}

bool TimedEventImpl::go_ready()
{
    bool returned_value = false;
    StateCode expected = StateCode::INACTIVE;

    if (state_.compare_exchange_strong(expected, StateCode::READY))
    {
        returned_value = true;
    }

    return returned_value;
}

/* In this function we don't need to exchange the state,
 * because this function try to cancel, but if the event is running
 * in the middle of the operation, it doesn't bother.
 */
bool TimedEventImpl::go_cancel()
{
    bool returned_value = false;
    StateCode prev_code = StateCode::INACTIVE;

    if ((prev_code = state_.exchange(StateCode::INACTIVE)) != StateCode::INACTIVE)
    {
        cancel_.store(true);
        returned_value = true;

        if (prev_code == StateCode::READY)
        {
            callback_(TimedEvent::EVENT_ABORT);
        }
    }

    return returned_value;
}

void TimedEventImpl::update()
{
    StateCode expected = StateCode::READY;

    if (cancel_.exchange(false))
    {
        timer_.cancel();
    }

    if (state_.compare_exchange_strong(expected, StateCode::WAITING))
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            timer_.expires_from_now(m_interval_microsec);
        }

        std::weak_ptr<Callback> callback_weak_ptr = callback_ptr_;
        timer_.async_wait(std::bind(&TimedEventImpl::event, this, callback_weak_ptr, std::placeholders::_1));
    }
}

bool TimedEventImpl::update_interval(
        const eprosima::fastrtps::Duration_t& inter)
{
    std::unique_lock<std::mutex> lock(mutex_);
    m_interval_microsec = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(inter));
    return true;
}

bool TimedEventImpl::update_interval_millisec(
        double time_millisec)
{
    std::unique_lock<std::mutex> lock(mutex_);
    m_interval_microsec = std::chrono::microseconds((int64_t)(time_millisec * 1000));
    return true;
}

void TimedEventImpl::event(
        std::weak_ptr<Callback> callback_weak_ptr,
        const std::error_code& ec)
{
    std::shared_ptr<Callback> callback_ptr = callback_weak_ptr.lock();

    if (callback_ptr)
    {
        if (ec != asio::error::operation_aborted)
        {
            StateCode expected = StateCode::WAITING;
            state_.compare_exchange_strong(expected, StateCode::INACTIVE);

            //Exec
            bool restart = callback_(TimedEvent::EVENT_SUCCESS);

            if (restart)
            {
                expected = StateCode::INACTIVE;
                if (state_.compare_exchange_strong(expected, StateCode::WAITING))
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        timer_.expires_from_now(m_interval_microsec);
                    }

                    timer_.async_wait(std::bind(&TimedEventImpl::event, this, callback_weak_ptr,
                            std::placeholders::_1));
                }
            }
        }
        else
        {
            callback_(TimedEvent::EVENT_ABORT);
        }
    }
}
