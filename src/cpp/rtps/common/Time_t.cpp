// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Time_t.cpp
 */
#include <fastdds/rtps/common/Time_t.hpp>

#include <cstdlib>
#include <chrono>

#include <utils/time_t_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

Time_t::Time_t(
        int32_t sec,
        uint32_t frac)
{
    seconds_ = sec;
    set_fraction(frac);
}

Time_t::Time_t(
        long double sec)
{
    seconds_ = static_cast<int32_t>(sec);
    set_fraction(static_cast<uint32_t>((sec - seconds_) * C_FRACTIONS_PER_SEC));
}

Time_t::Time_t(
        const eprosima::fastdds::dds::Time_t& time)
{
    seconds_ = time.seconds;
    set_nanosec(time.nanosec);
}

int64_t Time_t::to_ns() const
{
    // handle special cases
    // - infinite
    if ( *this == c_RTPSTimeInfinite )
    {
        return -1;
    }
    // - invalid value
    else if ( *this == c_RTPSTimeInvalid )
    {
        return -2;
    }

    int64_t nano = seconds_ * static_cast<int64_t>(C_NANOSECONDS_PER_SEC);
    nano += nanosec_;
    return nano;
}

void Time_t::from_ns(
        int64_t nanosecs)
{
    // handle special cases
    // - infinite
    if ( nanosecs == -1 )
    {
        *this = c_RTPSTimeInfinite;
    }
    else if ( nanosecs == -2 )
    {
        *this = c_RTPSTimeInvalid;
    }
    else
    {
        auto res = std::lldiv(nanosecs, 1000000000ull);
        seconds(static_cast<int32_t>(res.quot));
        nanosec(static_cast<uint32_t>(res.rem));
    }
}

int32_t Time_t::seconds() const
{
    return seconds_;
}

int32_t& Time_t::seconds()
{
    return seconds_;
}

void Time_t::seconds(
        int32_t sec)
{
    seconds_ = sec;
}

uint32_t Time_t::nanosec() const
{
    return nanosec_;
}

void Time_t::nanosec(
        uint32_t nanos)
{
    const uint32_t s_to_nano = static_cast<uint32_t>(C_NANOSECONDS_PER_SEC);
    if (nanos >= s_to_nano)
    {
        nanos %= s_to_nano; // Remove the seconds
    }
    set_nanosec(nanos);
}

uint32_t Time_t::fraction() const
{
    return fraction_;
}

uint32_t& Time_t::fraction()
{
    return fraction_;
}

void Time_t::fraction(
        uint32_t frac)
{
    set_fraction(frac);
}

dds::Duration_t Time_t::to_duration_t() const
{
    return dds::Duration_t(seconds_, nanosec_);
}

void Time_t::from_duration_t(
        const dds::Duration_t& duration)
{
    seconds_ = duration.seconds;
    set_nanosec(duration.nanosec);
}

void Time_t::set_fraction(
        uint32_t frac)
{
    fraction_ = frac;
    nanosec_ = (fraction_ == 0xffffffff)
        ? 0xffffffff
        : frac_to_nano(fraction_);
}

void Time_t::set_nanosec(
        uint32_t nanos)
{
    nanosec_ = nanos;

    fraction_ = (nanos == 0xffffffff)
        ? 0xffffffff
        : nano_to_frac(nanos);

    if (fraction_ != 0xffffffff)
    {
        uint32_t nano_check = frac_to_nano(fraction_);
        while (nano_check != nanosec_)
        {
            nano_check = frac_to_nano(++fraction_);
        }
    }
}

void Time_t::now(
        Time_t& ret)
{
    current_time_since_unix_epoch(ret.seconds_, ret.nanosec_);
    ret.set_nanosec(ret.nanosec_);
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
