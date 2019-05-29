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
#include <fastrtps/log/Log.h>
#include <asio/steady_timer.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {


ResourceEvent::ResourceEvent()
    : stop_(false)
    , notified_(false)
    , allow_to_delete_(false)
    , front_(nullptr)
    , back_(nullptr)
    , io_service_(ASIO_CONCURRENCY_HINT_UNSAFE_IO)
    , timer_(io_service_, std::chrono::seconds(1))
{
}

ResourceEvent::~ResourceEvent() 
{
    // All timer should be unregistered before destroying this object.
    assert(front_ == nullptr);
    assert(back_ == nullptr);

    logInfo(RTPS_PARTICIPANT,"Removing event thread");
    stop_ = true,
    io_service_.stop();
    thread_.join();
}

void ResourceEvent::register_timer(TimedEventImpl* event)
{
    std::unique_lock<std::timed_mutex> lock(mutex_);

    if(back_)
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
}

void ResourceEvent::unregister_timer(TimedEventImpl* event)
{
    assert(front_ != nullptr);
    assert(back_ != nullptr);

    std::unique_lock<std::timed_mutex> lock(mutex_);

    cv_.wait(lock, [&]()
    {
        return allow_to_delete_;
    });

    TimedEventImpl *prev = nullptr, *curr = front_;

    while(curr != event && curr->next())
    {
        prev = curr;
        curr = curr->next();
    }

    assert(curr);

    if(curr)
    {
        if(prev)
        {
            prev->next(curr->next());
        }
        else
        {
            front_ = curr->next();
        }

        if(!curr->next())
        {
            back_ = prev;
        }

        curr->next(nullptr);
        curr->go_cancel();
        curr->update();
        curr->update();
    }
}

void ResourceEvent::notify()
{
    std::unique_lock<std::timed_mutex> lock(mutex_);
    notified_ = true;
    cv_.notify_one();
}

void ResourceEvent::notify(const std::chrono::steady_clock::time_point& timeout)
{
    std::unique_lock<std::timed_mutex> lock(mutex_, std::defer_lock);

    if (lock.try_lock_until(timeout))
    {
        notified_ = true;
        cv_.notify_one();
    }
}

void ResourceEvent::event()
{
    if(!stop_)
    {
        timer_.async_wait(std::bind(&ResourceEvent::event, this));
    }
}

void ResourceEvent::run_io_service()
{
    timer_.async_wait(std::bind(&ResourceEvent::event, this));

    while (!stop_)
    {
        io_service_.poll();

        std::unique_lock<std::timed_mutex> lock(mutex_);

        allow_to_delete_ = true;
        cv_.notify_one();

        if (cv_.wait_for(lock, std::chrono::nanoseconds(1000000), [&]()
            {
                return notified_;
            }))
        {
            TimedEventImpl* curr = front_;

            while(curr)
            {
                curr->update();
                curr = curr->next();
            }

            notified_ = false;
        }

        allow_to_delete_ = false;
    }
}

void ResourceEvent::init_thread()
{
    thread_ = std::thread(&ResourceEvent::run_io_service, this);
    std::promise<bool> ready;
    std::future<bool> ready_fut = ready.get_future();
    io_service_.post([&ready]()
        {
            ready.set_value(true);
        });
    ready_fut.wait();
}

}
} /* namespace */
} /* namespace eprosima */
