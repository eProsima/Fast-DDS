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
    , io_service_(ASIO_CONCURRENCY_HINT_UNSAFE_IO)
    , timer_(io_service_, std::chrono::seconds(1))
    {
    }

ResourceEvent::~ResourceEvent() {
    logInfo(RTPS_PARTICIPANT,"Removing event thread");
    stop_ = true,
    io_service_.stop();
    thread_.join();

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

        if (cv_.wait_for(lock, std::chrono::milliseconds(1), [&]()
            {
                return !queue_.empty();
            }))
        {
            do
            {
                queue_.back().first->timer_.async_wait(std::bind(&TimedEventImpl::event, queue_.back().first,
                            std::placeholders::_1, queue_.back().second));
                queue_.pop_back();
            }
            while (!queue_.empty());
        }
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

void ResourceEvent::push(
        TimedEventImpl* event,
        std::shared_ptr<TimerState> state)
{
    std::unique_lock<std::timed_mutex> lock(mutex_);
    queue_.emplace_back(event, state);
    cv_.notify_one();
}

void ResourceEvent::push(
        TimedEventImpl* event,
        std::shared_ptr<TimerState> state,
        const std::chrono::steady_clock::time_point timeout)
{
    std::unique_lock<std::timed_mutex> lock(mutex_, std::defer_lock);

    if(lock.try_lock_until(timeout))
    {
        queue_.emplace_back(event, state);
        cv_.notify_one();
    }
}

}
} /* namespace */
} /* namespace eprosima */
