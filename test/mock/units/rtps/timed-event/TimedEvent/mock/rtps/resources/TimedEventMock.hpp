// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_RTPS_RESOURCES__TIMEDEVENTMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_RESOURCES__TIMEDEVENTMOCK_HPP

#include <chrono>

#include <gmock/gmock.h>

#include <fastdds/rtps/common/Time_t.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/resources/TimedEvent.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a TimedEvent created with the mock implementation, reached with mock::mock_of().
 * No timer is registered on the ResourceEvent, so the callback is never called.
 */
class TimedEventMock
{
public:

    MOCK_METHOD0(restart_timer, void());
    MOCK_METHOD1(restart_timer, void(const std::chrono::steady_clock::time_point& timeout));
    MOCK_METHOD0(cancel_timer, void());
    MOCK_METHOD0(recreate_timer, void());
    MOCK_METHOD1(update_interval, bool(const dds::Duration_t&));
    MOCK_METHOD1(update_interval_millisec, bool(double));
    MOCK_METHOD0(getIntervalMilliSec, double());
    MOCK_METHOD0(getRemainingTimeMilliSec, double());

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::TimedEvent>
{
    using type = rtps::TimedEventMock;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_RESOURCES__TIMEDEVENTMOCK_HPP
