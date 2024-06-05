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

#include "MockEvent.h"

using namespace eprosima::fastdds::rtps;

MockEvent::MockEvent(
        ResourceEvent& service,
        double milliseconds,
        bool autorestart,
        std::function<void()> inner_callback)
    : successed_(0)
    , sem_count_(0)
    , autorestart_(autorestart)
    , inner_callback_(inner_callback)
    , event_(service, std::bind(&MockEvent::callback, this), milliseconds)
{
}

MockEvent::~MockEvent()
{
}

bool MockEvent::callback()
{
    bool restart = false;

    successed_.fetch_add(1, std::memory_order_relaxed);

    if (autorestart_)
    {
        restart = true;
    }

    sem_mutex_.lock();
    ++sem_count_;
    sem_mutex_.unlock();
    sem_cond_.notify_one();

    if (inner_callback_)
    {
        inner_callback_();
    }

    return restart;
}

void MockEvent::wait()
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    sem_cond_.wait(lock, [&]() -> bool
            {
                return sem_count_ != 0;
            } );

    --sem_count_;
}

void MockEvent::wait_success()
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    while (successed_.load(std::memory_order_relaxed) == 0)
    {
        sem_cond_.wait(lock, [&]() -> bool
                {
                    return sem_count_ != 0;
                } );
        --sem_count_;
    }
}

bool MockEvent::wait(
        unsigned int milliseconds)
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    if (!sem_cond_.wait_for(lock, std::chrono::milliseconds(milliseconds),
            [&]() -> bool
            {
                return sem_count_ != 0;
            } ))
    {
        return false;
    }

    --sem_count_;
    return true;
}