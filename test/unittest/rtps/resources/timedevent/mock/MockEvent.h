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

class MockEvent : public eprosima::fastrtps::rtps::TimedEvent
{
    public:

        MockEvent(asio::io_service &service, const std::thread& event_thread, double milliseconds, bool autorestart, TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);

        virtual ~MockEvent();

        void event(EventCode code, const char* msg= nullptr);

        void wait();

        bool wait(unsigned int milliseconds);

        std::atomic<int> successed_;
        std::atomic<int> cancelled_;
        static int destructed_;
        static std::mutex destruction_mutex_;
        static std::condition_variable destruction_cond_;

    private:

        int sem_count_;
        std::mutex sem_mutex_;
        std::condition_variable sem_cond_;
        bool autorestart_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKEVENT_H_
