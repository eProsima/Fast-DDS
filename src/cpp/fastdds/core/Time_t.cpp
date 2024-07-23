// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/core/Time_t.hpp>

#include <chrono>
#include <cstdlib>

#include <utils/time_t_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr int32_t Time_t::INFINITE_SECONDS;
constexpr uint32_t Time_t::INFINITE_NANOSECONDS;

Time_t::Time_t()
{
    seconds = 0;
    nanosec = 0;
}

Time_t::Time_t(
        int32_t sec,
        uint32_t nsec)
{
    seconds = sec;
    nanosec = nsec;
}

Time_t::Time_t(
        long double sec)
{
    seconds = static_cast<int32_t>(sec);
    nanosec = static_cast<uint32_t>((sec - seconds) * C_NANOSECONDS_PER_SEC);
}

void Time_t::fraction(
        uint32_t frac)
{
    nanosec = (frac == 0xffffffff)
        ? 0xffffffff
        : frac_to_nano(frac);
}

uint32_t Time_t::fraction() const
{
    uint32_t fraction = (nanosec == 0xffffffff)
        ? 0xffffffff
        : nano_to_frac(nanosec);

    if (fraction != 0xffffffff)
    {
        uint32_t nano_check = frac_to_nano(fraction);
        while (nano_check != nanosec)
        {
            nano_check = frac_to_nano(++fraction);
        }
    }

    return fraction;
}

int64_t Time_t::to_ns() const
{
    int64_t nano = seconds * static_cast<int64_t>(C_NANOSECONDS_PER_SEC);
    nano += nanosec;
    return nano;
}

void Time_t::now(
        Time_t& ret)
{
    current_time_since_unix_epoch(ret.seconds, ret.nanosec);
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
