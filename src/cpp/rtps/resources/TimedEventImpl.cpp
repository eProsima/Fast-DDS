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

#include <fastrtps/utils/TimeConversion.h>

#include <atomic>
#include <functional>

namespace eprosima {
namespace fastrtps {
namespace rtps {

TimedEventImpl::TimedEventImpl(
        Callback callback,
        std::chrono::microseconds interval)
    : interval_microsec_(interval)
    , callback_(std::move(callback))
    , state_(StateCode::INACTIVE)
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

bool TimedEventImpl::go_cancel()
{
    bool returned_value = false;
    StateCode prev_code = StateCode::INACTIVE;

    if ((prev_code = state_.exchange(StateCode::INACTIVE)) != StateCode::INACTIVE)
    {
        returned_value = true;
    }

    return returned_value;
}

bool TimedEventImpl::update(
        std::chrono::steady_clock::time_point current_time,
        std::chrono::steady_clock::time_point cancel_time)
{
    StateCode expected = StateCode::READY;
    bool set_time = state_.compare_exchange_strong(expected, StateCode::WAITING);

    if (set_time)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        next_trigger_time_ = current_time + interval_microsec_;
    }
    else if (expected == StateCode::INACTIVE)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        next_trigger_time_ = cancel_time;
    }

    return expected != StateCode::INACTIVE;
}

void TimedEventImpl::trigger(
        std::chrono::steady_clock::time_point current_time,
        std::chrono::steady_clock::time_point cancel_time)
{
    if (callback_)
    {
        StateCode expected = StateCode::WAITING;
        state_.compare_exchange_strong(expected, StateCode::INACTIVE);

        //Exec
        bool restart = callback_();

        if (restart)
        {
            expected = StateCode::INACTIVE;
            if (state_.compare_exchange_strong(expected, StateCode::WAITING))
            {
                std::lock_guard<std::mutex> lock(mutex_);
                next_trigger_time_ = current_time + interval_microsec_;
                return;
            }
        }

        std::lock_guard<std::mutex> lock(mutex_);
        next_trigger_time_ = cancel_time;
    }
}

bool TimedEventImpl::update_interval(
        const eprosima::fastrtps::Duration_t& interval)
{
    std::lock_guard<std::mutex> lock(mutex_);
    interval_microsec_ = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(interval));
    return true;
}

bool TimedEventImpl::update_interval_millisec(
        double interval)
{
    std::lock_guard<std::mutex> lock(mutex_);
    interval_microsec_ = std::chrono::microseconds(static_cast<int64_t>(interval * 1000));
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
