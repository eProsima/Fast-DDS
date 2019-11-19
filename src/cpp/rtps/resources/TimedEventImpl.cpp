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
#include <fastrtps/rtps/resources/ResourceEvent.h>
#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>

#include <cassert>
#include <functional>
#include <atomic>
#include <system_error>

using namespace eprosima::fastrtps::rtps;

TimedEventImpl::TimedEventImpl(
        Callback callback,
        std::chrono::microseconds interval)
    : interval_microsec_(interval)
    , callback_(callback)
    , enabled_(false)
{
}

TimedEventImpl::~TimedEventImpl()
{
}

bool TimedEventImpl::go_ready()
{
    bool ret_val = enabled_.exchange(true) == false;
    if (ret_val)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        next_trigger_time_ = std::chrono::steady_clock::now() + interval_microsec_;
    }

    return ret_val;
}

bool TimedEventImpl::go_cancel()
{
    return enabled_.exchange(false) == true;
}

void TimedEventImpl::trigger(
        std::chrono::steady_clock::time_point current_time,
        std::chrono::steady_clock::time_point cancel_time)
{
    if (go_cancel())
    {
        if (callback_)
        {
            bool restart = callback_ ? callback_() : false;
            restart &= interval_microsec_.count() > 0;

            std::unique_lock<std::mutex> lock(mutex_);
            next_trigger_time_ = restart ?
                current_time + interval_microsec_ :
                cancel_time;
            enabled_.store(restart);
        }
    }
}

bool TimedEventImpl::update_interval(
        const eprosima::fastrtps::Duration_t& inter)
{
    std::unique_lock<std::mutex> lock(mutex_);
    interval_microsec_ = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(inter));
    return true;
}

bool TimedEventImpl::update_interval_millisec(
        double time_millisec)
{
    std::unique_lock<std::mutex> lock(mutex_);
    interval_microsec_ = std::chrono::microseconds((int64_t)(time_millisec * 1000));
    return true;
}
