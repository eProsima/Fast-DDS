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

#ifndef FASTDDS_RTPS_RESOURCES__TIMEDEVENT_H
#define FASTDDS_RTPS_RESOURCES__TIMEDEVENT_H

#include <chrono>

#include <gmock/gmock.h>

#include <fastdds/rtps/common/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ResourceEvent;

class TimedEvent
{
public:

    TimedEvent(
            ResourceEvent&,
            std::function<bool()>,
            double)
    {
    }

    MOCK_METHOD0(restart_timer, void());
    MOCK_METHOD1(restart_timer, void(const std::chrono::steady_clock::time_point& timeout));
    MOCK_METHOD0(cancel_timer, void());
    MOCK_METHOD0(recreate_timer, void());
    MOCK_METHOD1(update_interval, bool(const dds::Duration_t&));
    MOCK_METHOD1(update_interval_millisec, bool(double));
    MOCK_METHOD0(getIntervalMilliSec, double());

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_RESOURCES__TIMEDEVENT_H

