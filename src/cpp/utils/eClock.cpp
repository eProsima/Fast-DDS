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
 * @file eClock.cpp
 *
 */

#include <fastrtps/utils/eClock.h>

#include <chrono>
#include <thread>

using namespace std::chrono;

namespace eprosima {
namespace fastrtps {

eClock::eClock()
{
}

eClock::~eClock()
{
}

bool eClock::setTimeNow(
        Time_t* tnow)
{
    Time_t::now(*tnow);
    return true;
}

bool eClock::setTimeNow(
        rtps::Time_t* tnow)
{
    rtps::Time_t::now(*tnow);
    return true;
}

void eClock::my_sleep(
    uint32_t ms)
{
    std::this_thread::sleep_for(milliseconds(ms));
}

void eClock::intervalStart()
{
    auto now = high_resolution_clock::now().time_since_epoch();
    interval_start = static_cast<uint64_t>(duration_cast<microseconds>(now).count());
}
uint64_t eClock::intervalEnd()
{
    auto now = high_resolution_clock::now().time_since_epoch();
    return static_cast<uint64_t>(duration_cast<microseconds>(now).count()) - interval_start;
}

} // namespace fastrtps
} // namespace eprosima
