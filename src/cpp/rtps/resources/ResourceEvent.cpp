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

#include <fastdds/rtps/resources/ResourceEvent.h>
#include <rtps/resources/TimedEventImpl.h>

#include <asio.hpp>
#include <thread>
#include <functional>
#include <future>
#include <fastrtps/log/Log.h>
#include <asio/steady_timer.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {


ResourceEvent::ResourceEvent()
    : stop_(false)
    , allow_to_delete_(false)
    , front_(nullptr)
    , back_(nullptr)
    , io_service_(
#if ASIO_VERSION >= 101200
        ASIO_CONCURRENCY_HINT_UNSAFE_IO
#else
        1
#endif
        )
{
}

ResourceEvent::~ResourceEvent()
{
    // All timer should be unregistered before destroying this object.
    assert(front_ == nullptr);
    assert(back_ == nullptr);

    logInfo(RTPS_PARTICIPANT, "Removing event thread");
    stop_.store(true);
    io_service_.stop();

    if (thread_.joinable())
    {
        thread_.join();
    }
}

bool ResourceEvent::register_timer_nts(
        TimedEventImpl* event)
{
    TimedEventImpl* curr = front_;

    while (curr != nullptr)
    {
        if (curr == event)
        {
            return false;
        }

        curr = curr->next();
    }

    if (back_)
    {
        back_->next(event);
        back_ = event;
    }
    else
    {
        assert(front_ == nullptr);
        front_ = event;
        back_ = event;
    }

    return true;
}

void ResourceEvent::unregister_timer(
        TimedEventImpl* event)
{
    assert(!stop_.load());

    std::unique_lock<TimedMutex> lock(mutex_);

    cv_.wait(lock, [&]()
                {
                    return allow_to_delete_;
                });

    TimedEventImpl* prev = nullptr, * curr = front_;

    while (curr && curr != event && curr->next())
    {
        prev = curr;
        curr = curr->next();
    }

    if (curr)
    {
        if (prev)
        {
            prev->next(curr->next());
        }
        else
        {
            front_ = curr->next();
        }

        if (!curr->next())
        {
            back_ = prev;
        }

        curr->next(nullptr);
        curr->go_cancel();
        curr->update();
    }
}

void ResourceEvent::notify(
        TimedEventImpl* event)
{
    std::unique_lock<TimedMutex> lock(mutex_);

    if (register_timer_nts(event))
    {
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
            cv_.notify_one();
        }
    }
}

void ResourceEvent::run_io_service()
{
    while (!stop_.load())
    {
#if ASIO_VERSION >= 101200
        io_service_.restart();
#else
        io_service_.reset();
#endif
        io_service_.poll();

        std::unique_lock<TimedMutex> lock(mutex_);

        allow_to_delete_ = true;
        cv_.notify_one();

        if (cv_.wait_for(lock, std::chrono::nanoseconds(1000000), [&]()
                    {
                        return front_ != nullptr;
                    }))
        {
            TimedEventImpl* curr = front_;

            while (curr)
            {
                curr->update();
                curr = curr->next(nullptr);
            }

            front_ = nullptr;
            back_ = nullptr;
        }

        allow_to_delete_ = false;
    }
}

void ResourceEvent::init_thread()
{
    thread_ = std::thread(&ResourceEvent::run_io_service, this);
    std::future<void> ready_fut = ready.get_future();
    io_service_.post([this]()
                {
                    ready.set_value();
                });
    ready_fut.wait();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
