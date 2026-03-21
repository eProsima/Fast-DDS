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
 * @file ResourceEvent.h
 *
 */

#ifndef FASTDDS_RTPS_RESOURCES__RESOURCEEVENT_H
#define FASTDDS_RTPS_RESOURCES__RESOURCEEVENT_H

#include <gmock/gmock.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TimedEventImpl;


class ResourceEvent
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD0(init_thread, void());

    MOCK_METHOD1(register_timer, void(TimedEventImpl* event));

    MOCK_METHOD1(unregister_timer, void(TimedEventImpl* event));

    MOCK_METHOD1(notify, void(TimedEventImpl* event));

    MOCK_METHOD2(notify, void(
                TimedEventImpl* event,
                const std::chrono::steady_clock::time_point& timeout));
    // *INDENT-ON*

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //FASTDDS_RTPS_RESOURCES__RESOURCEEVENT_H

