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

int MockEvent::destructed_ = 0;
std::mutex MockEvent::destruction_mutex_;
std::condition_variable MockEvent::destruction_cond_;

MockEvent::MockEvent(asio::io_service& service, const std::thread& event_thread, double milliseconds, bool autorestart, TimedEvent::AUTODESTRUCTION_MODE autodestruction) :
    TimedEvent(service, event_thread, milliseconds, autodestruction), successed_(0), cancelled_(0), sem_count_(0), autorestart_(autorestart)
{
}

MockEvent::~MockEvent()
{
    destroy();

    destruction_mutex_.lock();
    ++destructed_;
    destruction_mutex_.unlock();
    destruction_cond_.notify_one();
}

void MockEvent::event(EventCode code, const char* msg)
{
    (void)msg;

    if(code == EventCode::EVENT_SUCCESS)
    {
        successed_.fetch_add(1, std::memory_order_relaxed);

        if(autorestart_)
        {
            restart_timer();
        }
    }
    else if(code == EventCode::EVENT_ABORT)
    {
        cancelled_.fetch_add(1, std::memory_order_relaxed);
    }

    sem_mutex_.lock();
    ++sem_count_;
    sem_mutex_.unlock();
    sem_cond_.notify_one();
}

void MockEvent::wait()
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    if(sem_count_ == 0)
    {
        sem_cond_.wait(lock, [&]() -> bool { return sem_count_ != 0; } );
    }

    --sem_count_;
}

bool MockEvent::wait(unsigned int milliseconds)
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    if(sem_count_ == 0)
    {
        if(!sem_cond_.wait_for(lock, std::chrono::milliseconds(milliseconds),
                    [&]() -> bool { return sem_count_ != 0; } ))
        {
            return false;
        }
    }

    --sem_count_;
    return true;
}
