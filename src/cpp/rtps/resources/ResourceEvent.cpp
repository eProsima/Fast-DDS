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
 * @file ThreadEvent.cpp
 *
 */

#include <fastrtps/rtps/resources/ResourceEvent.h>
#include "TimedEventImpl.h"

#include <asio.hpp>
#include <thread>
#include <functional>
#include <future>
#include <fastrtps/log/Log.h>
#include <asio/steady_timer.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

static bool event_compare(
        TimedEventImpl* lhs,
        TimedEventImpl* rhs)
{
    return lhs->next_trigger_time() < rhs->next_trigger_time();
}

ResourceEvent::ResourceEvent()
    : stop_(false)
{
}

ResourceEvent::~ResourceEvent()
{
    // All timer should be unregistered before destroying this object.
    assert(timers_.empty());

    logInfo(RTPS_PARTICIPANT, "Removing event thread");
    stop_.store(true);

    if (thread_.joinable())
    {
        thread_.join();
    }
}

bool ResourceEvent::register_timer_nts(
        TimedEventImpl* event)
{
    std::vector<TimedEventImpl*>::iterator low_bound;
    std::vector<TimedEventImpl*>::iterator end_it = timers_.end();
    
    // Find insertion position
    low_bound = std::lower_bound(timers_.begin(), end_it, event, event_compare);

    // If event is not found from there onwards ...
    if (std::find(low_bound, end_it, event) == end_it)
    {
        // ... add it on its place
        timers_.emplace(low_bound, event);
        return true;
    }

    // It was already present, no need to add again
    return false;
}

void ResourceEvent::unregister_timer(
        TimedEventImpl* event)
{
    assert(!stop_.load());

    std::unique_lock<TimedMutex> lock(mutex_);

    std::vector<TimedEventImpl*>::iterator it;
    std::vector<TimedEventImpl*>::iterator end_it = timers_.end();

    // Find with binary search
    it = std::lower_bound(timers_.begin(), end_it, event, event_compare);

    // Find the event on the list
    for(; it != end_it; ++it)
    {
        if (*it == event)
        {
            // Remove from list
            timers_.erase(it);

            // Notify the execution thread that something changed
            cv_.notify_one();
            break;
        }
    }
}

void ResourceEvent::notify(
        TimedEventImpl* event)
{
    std::unique_lock<TimedMutex> lock(mutex_);

    if (register_timer_nts(event))
    {
        // Notify the execution thread that something changed
        cv_.notify_one();
    }
}

void ResourceEvent::notify(
        TimedEventImpl* event,
        const std::chrono::steady_clock::time_point& timeout)
{
    std::unique_lock<TimedMutex> lock(mutex_, std::defer_lock);

    if (lock.try_lock_until(timeout))
    {
        if (register_timer_nts(event))
        {
            // Notify the execution thread that something changed
            cv_.notify_one();
        }
    }
}

void ResourceEvent::run_io_service()
{
    while (!stop_.load())
    {
        std::unique_lock<TimedMutex> lock(mutex_);

        update_current_time();
        do_timer_actions();

        std::chrono::steady_clock::time_point next_trigger =
            (timers_.size() > 0) ?
                timers_[0]->next_trigger_time() :
                current_time_ + std::chrono::seconds(1);

        cv_.wait_until(lock, next_trigger);
    }
}

void ResourceEvent::sort_timers()
{
    std::sort(timers_.begin(), timers_.end(), event_compare);
}

void ResourceEvent::update_current_time()
{
    current_time_ = std::chrono::steady_clock::now();
}

void ResourceEvent::do_timer_actions()
{
    if (!timers_.empty())
    {
        TimedEventImpl* last = timers_.back();
        std::chrono::steady_clock::time_point cancel_time =
            last->next_trigger_time() + std::chrono::hours(24);

        bool did_something = false;
        for (TimedEventImpl* tp : timers_)
        {
            if (tp->next_trigger_time() <= current_time_)
            {
                did_something = true;
                tp->trigger(current_time_, cancel_time);
            }
            else
            {
                break;
            }
        }

        if (did_something)
        {
            sort_timers();

            timers_.erase(
                std::lower_bound(timers_.begin(), timers_.end(), nullptr,
                    [cancel_time](
                            TimedEventImpl* a,
                            TimedEventImpl* b)
                    {
                        (void)b;
                        return a->next_trigger_time() < cancel_time;
                    }),
                timers_.end()
            );
        }
    }
}

void ResourceEvent::init_thread()
{
    thread_ = std::thread(&ResourceEvent::run_io_service, this);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
