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

#ifndef _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
#define  _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>

#include <atomic>
#include <condition_variable>
#include <asio.hpp>
#include <thread>

class MockEvent
{
    public:

        MockEvent(
                eprosima::fastrtps::rtps::ResourceEvent& service,
                double milliseconds,
                bool autorestart);

        virtual ~MockEvent();

        eprosima::fastrtps::rtps::TimedEvent& event() { return event_; }

        bool callback();

        void wait();

        void wait_success();

        bool wait(unsigned int milliseconds);

        std::atomic<int> successed_;

    private:

        int sem_count_;
        std::mutex sem_mutex_;
        std::condition_variable sem_cond_;
        bool autorestart_;
        eprosima::fastrtps::rtps::TimedEvent event_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
