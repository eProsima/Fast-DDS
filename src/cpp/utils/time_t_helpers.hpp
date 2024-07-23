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

#ifndef FASTDDS_UTILS__TIMETHELPERS_HPP
#define FASTDDS_UTILS__TIMETHELPERS_HPP

#include <chrono>
#include <cstdlib>
// unnamed namespace for inline functions in compilation unit. Better practice than static inline.

constexpr uint64_t C_FRACTIONS_PER_SEC = 4294967296ULL;
constexpr uint64_t C_NANOSECONDS_PER_SEC = 1000000000ULL;

inline uint32_t frac_to_nano(
        uint32_t fractions)
{
    return static_cast<uint32_t>((fractions * C_NANOSECONDS_PER_SEC) / C_FRACTIONS_PER_SEC);
}

inline uint32_t nano_to_frac(
        uint32_t nanosecs)
{
    return static_cast<uint32_t>((nanosecs * C_FRACTIONS_PER_SEC) / C_NANOSECONDS_PER_SEC);
}

static void current_time_since_unix_epoch(
        int32_t& secs,
        uint32_t& nanosecs)
{
    using namespace std::chrono;

    // Get time since epoch
    auto t_since_epoch = system_clock::now().time_since_epoch();
    // Get seconds
    auto secs_t = duration_cast<seconds>(t_since_epoch);
    // Remove seconds from time
    t_since_epoch -= secs_t;

    // Get seconds and nanoseconds
    secs = static_cast<int32_t>(secs_t.count());
    nanosecs = static_cast<uint32_t>(duration_cast<nanoseconds>(t_since_epoch).count());
}

#endif // FASTDDS_UTILS__TIMETHELPERS_HPP
