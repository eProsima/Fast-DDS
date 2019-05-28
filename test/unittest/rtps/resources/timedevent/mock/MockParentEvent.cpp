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

#include "MockParentEvent.h"

using namespace eprosima::fastrtps::rtps;

int MockParentEvent::destructed_ = 0;
std::mutex MockParentEvent::destruction_mutex_;
std::condition_variable MockParentEvent::destruction_cond_;

MockParentEvent::MockParentEvent(
        eprosima::fastrtps::rtps::ResourceEvent& service,
        double milliseconds,
        unsigned int countUntilDestruction)
    : successed_(0)
    , cancelled_(0)
    , event_(service, std::bind(&MockParentEvent::callback, this, std::placeholders::_1), milliseconds)
    , mock_(nullptr)
    , sem_count_(0)
    , countUntilDestruction_(countUntilDestruction)
    , currentCount_(0)
{
    mock_ = new MockEvent(service, milliseconds / 2.0, false);
    mock_->event().restart_timer();
}

MockParentEvent::~MockParentEvent()
{
    destruction_mutex_.lock();
    ++destructed_;
    destruction_mutex_.unlock();
    destruction_cond_.notify_one();
}

bool MockParentEvent::callback(TimedEvent::EventCode code)
{
    if(code == TimedEvent::EventCode::EVENT_SUCCESS)
    {
        successed_.fetch_add(1, std::memory_order_relaxed);

        if(mock_ != nullptr)
        {
            if(++currentCount_ == countUntilDestruction_)
            {
                delete mock_;
                mock_ = nullptr;
            }
        }

        event_.restart_timer();

    }
    else if(code == TimedEvent::EventCode::EVENT_ABORT)
        cancelled_.fetch_add(1, std::memory_order_relaxed);

    sem_mutex_.lock();
    ++sem_count_;
    sem_mutex_.unlock();
    sem_cond_.notify_one();

    return true;
}

bool MockParentEvent::wait(unsigned int milliseconds)
{
    std::unique_lock<std::mutex> lock(sem_mutex_);

    if(sem_count_ == 0)
    {
        if(sem_cond_.wait_for(lock, std::chrono::milliseconds(milliseconds)) != std::cv_status::no_timeout)
            return false;
    }

    --sem_count_;
    return true;
}
