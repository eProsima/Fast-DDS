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
 * @file ResourceEvent.cpp
 */

#include <rtps/resources/ResourceEvent.h>

#include <cassert>

#include <fastdds/dds/log/Log.hpp>

#include "TimedEventImpl.h"
#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

static bool event_compare(
        TimedEventImpl* lhs,
        TimedEventImpl* rhs)
{
    return lhs->next_trigger_time() < rhs->next_trigger_time();
}

ResourceEvent::ResourceEvent()
    : thread_(new eprosima::thread())
{
}

ResourceEvent::~ResourceEvent()
{
    // All timer should be unregistered before destroying this object.
    assert(pending_timers_.empty());
    assert(timers_count_ == 0);

    stop_thread();
}

void ResourceEvent::stop_thread()
{
    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "Removing event thread");
    if (thread_->joinable())
    {
        {
            std::lock_guard<TimedMutex> guard(mutex_);
            stop_.store(true);
            cv_.notify_one();
        }
        thread_->join();
    }
}

void ResourceEvent::register_timer(
        TimedEventImpl* /*event*/)
{
    {
        std::lock_guard<TimedMutex> lock(mutex_);
        ++timers_count_;
    }

    // Notify the execution thread that something changed
    cv_.notify_one();
}

void ResourceEvent::unregister_timer(
        TimedEventImpl* event)
{
    std::unique_lock<TimedMutex> lock(mutex_);

    bool is_service_thread = thread_->is_calling_thread();

    //! Let the service thread to manipulate resources
    if (!is_service_thread)
    {
        cv_manipulation_.wait(lock, [&]()
                {
                    return allow_vector_manipulation_;
                });
    }

    bool should_notify = false;
    std::vector<TimedEventImpl*>::iterator it;

    // Remove from pending
    it = std::find(pending_timers_.begin(), pending_timers_.end(), event);
    if (it != pending_timers_.end())
    {
        pending_timers_.erase(it);
        should_notify = true;
    }

    // Remove from active
    it = std::find(active_timers_.begin(), active_timers_.end(), event);
    if (it != active_timers_.end())
    {
        active_timers_.erase(it);

        if (is_service_thread)
        {
            //! Warn the do_timer_actions loop to skip checking the rest of active_timers
            //! in this iteration to prevent iterator invalidation
            skip_checking_active_timers_.store(true);
        }

        should_notify = true;
    }

    // Decrement counter of created timers
    --timers_count_;

    if (should_notify)
    {
        // Notify the execution thread that something changed
        cv_.notify_one();
    }
}

void ResourceEvent::notify(
        TimedEventImpl* event)
{
    std::lock_guard<TimedMutex> lock(mutex_);

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
#if HAVE_STRICT_REALTIME
    std::unique_lock<TimedMutex> lock(mutex_, std::defer_lock);
    if (lock.try_lock_until(timeout))
#else
    static_cast<void>(timeout);
    std::lock_guard<TimedMutex> _(mutex_);
#endif  // HAVE_STRICT_REALTIME
    {
        if (register_timer_nts(event))
        {
            // Notify the execution thread that something changed
            cv_.notify_one();
        }
    }
}

bool ResourceEvent::register_timer_nts(
        TimedEventImpl* event)
{
    if (std::find(pending_timers_.begin(), pending_timers_.end(), event) == pending_timers_.end())
    {
        pending_timers_.push_back(event);
        return true;
    }

    return false;
}

void ResourceEvent::event_service()
{
    while (!stop_.load())
    {
        // Perform update and execution of timers
        update_current_time();
        do_timer_actions();

        std::unique_lock<TimedMutex> lock(mutex_);

        // If the thread has already been instructed to stop, do it.
        if (stop_.load())
        {
            break;
        }

        // If pending timers exist, there is some work to be done, so no need to wait.
        if (!pending_timers_.empty())
        {
            continue;
        }

        // Allow other threads to manipulate the timer collections while we wait.
        allow_vector_manipulation_ = true;
        cv_manipulation_.notify_all();

        // Wait for the first timer to be triggered
        std::chrono::steady_clock::time_point next_trigger =
                active_timers_.empty() ?
                current_time_ + std::chrono::seconds(1) :
                active_timers_[0]->next_trigger_time();

        auto current_time = std::chrono::steady_clock::now();
        if (current_time > next_trigger)
        {
            next_trigger = current_time + std::chrono::microseconds(10);
        }

        cv_.wait_until(lock, next_trigger);

        // Don't allow other threads to manipulate the timer collections
        allow_vector_manipulation_ = false;
        resize_collections();
    }

    // Thread being stopped. Allow other threads to manipulate the timer collections.
    {
        std::lock_guard<TimedMutex> guard(mutex_);
        allow_vector_manipulation_ = true;
    }
    cv_manipulation_.notify_all();
}

void ResourceEvent::sort_timers()
{
    std::sort(active_timers_.begin(), active_timers_.end(), event_compare);
}

void ResourceEvent::update_current_time()
{
    current_time_ = std::chrono::steady_clock::now();
}

void ResourceEvent::do_timer_actions()
{
    std::chrono::steady_clock::time_point cancel_time =
            current_time_ + std::chrono::hours(24);

    bool did_something = false;

    // Process pending orders
    {
        std::lock_guard<TimedMutex> lock(mutex_);
        for (TimedEventImpl* tp : pending_timers_)
        {
            // Remove item from active timers
            auto current_pos = std::lower_bound(active_timers_.begin(), active_timers_.end(), tp, event_compare);
            current_pos = std::find(current_pos, active_timers_.end(), tp);
            if (current_pos != active_timers_.end())
            {
                active_timers_.erase(current_pos);
            }

            // Update timer info
            if (tp->update(current_time_, cancel_time))
            {
                // Timer has to be activated: add to active timers
                std::vector<TimedEventImpl*>::iterator low_bound;

                // Insert on correct position
                low_bound = std::lower_bound(active_timers_.begin(), active_timers_.end(), tp, event_compare);
                active_timers_.emplace(low_bound, tp);
            }
        }
        pending_timers_.clear();
    }

    // Trigger active timers
    skip_checking_active_timers_.store(false);
    for (TimedEventImpl* tp : active_timers_)
    {
        if (tp->next_trigger_time() <= current_time_)
        {
            did_something = true;
            tp->trigger(current_time_, cancel_time);

            //! skip this iteration as active_timers has been manipulated
            if (skip_checking_active_timers_.load())
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    // If an action was made, keep active_timers_ sorted
    if (did_something)
    {
        sort_timers();
        active_timers_.erase(
            std::lower_bound(active_timers_.begin(), active_timers_.end(), nullptr,
            [cancel_time](
                TimedEventImpl* a,
                TimedEventImpl* b)
            {
                (void)b;
                return a->next_trigger_time() < cancel_time;
            }),
            active_timers_.end()
            );
    }
}

void ResourceEvent::init_thread(
        const fastdds::rtps::ThreadSettings& thread_cfg,
        const char* name_fmt,
        uint32_t thread_id)
{
    std::lock_guard<TimedMutex> lock(mutex_);

    allow_vector_manipulation_ = false;
    stop_.store(false);
    resize_collections();

    *thread_ = eprosima::create_thread([this]()
                    {
                        event_service();
                    }, thread_cfg, name_fmt, thread_id);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
