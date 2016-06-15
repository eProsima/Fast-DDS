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

#ifndef _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKPARENTEVENT_H_
#define  _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKPARENTEVENT_H_

#include <fastrtps/rtps/resources/TimedEvent.h>
#include "MockEvent.h"

#include <atomic>
#include <condition_variable>
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include <boost/asio.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif  // _MSC_VER
#include <boost/thread.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER

class MockParentEvent : public eprosima::fastrtps::rtps::TimedEvent
{
    public:

        MockParentEvent(boost::asio::io_service &service, const boost::thread& event_thread, double milliseconds, unsigned int countUntilDestruction,
                TimedEvent::AUTODESTRUCTION_MODE autodestruction = TimedEvent::NONE);

        virtual ~MockParentEvent();

        void event(EventCode code, const char* msg= nullptr);

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
        MockEvent *event_;
        unsigned int countUntilDestruction_;
        unsigned int currentCount_;
};

#endif // _TEST_RTPS_RESOURCES_TIMEDEVENT_MOCKPARENTEVENT_H_

