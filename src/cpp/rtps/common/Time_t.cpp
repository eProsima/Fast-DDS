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
#include <fastrtps/rtps/common/Time_t.h>

using namespace eprosima::fastrtps;

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
    nanosec = static_cast<uint32_t>((sec - seconds) * 1000000000ULL);
}

void Time_t::fraction(
        uint32_t frac)
{
    nanosec = (frac == 0xffffffff)
        ? 0xffffffff
        : static_cast<uint32_t>(std::lroundl(frac * rtps::FRACTION_TO_NANO));
}

uint32_t Time_t::fraction() const
{
    if (nanosec == 0xffffffff)
    {
        return nanosec;
    }

    uint32_t fraction = static_cast<uint32_t>(std::lroundl(nanosec * rtps::NANO_TO_FRACTION));
    uint32_t nano_check = static_cast<uint32_t>(std::lroundl(fraction * rtps::FRACTION_TO_NANO));
    while (nano_check != nanosec)
    {
        nano_check = static_cast<uint32_t>(std::lroundl(++fraction * rtps::FRACTION_TO_NANO));
    }

    return fraction;
}

int64_t Time_t::to_ns() const
{
    int64_t nano = seconds * 1000000000ULL;
    nano += nanosec;
    return nano;
}

rtps::Time_t::Time_t()
{
    seconds_ = 0;
    fraction_ = 0;
    nanosec_ = 0;
}

rtps::Time_t::Time_t(
        int32_t sec,
        uint32_t frac)
{
    seconds_ = sec;
    set_fraction(frac);
}

rtps::Time_t::Time_t(
        long double sec)
{
    seconds_ = static_cast<int32_t>(sec);
    set_fraction(static_cast<uint32_t>((sec - seconds_) * 4294967296ULL));
}

rtps::Time_t::Time_t(
        const eprosima::fastrtps::Time_t& time)
{
    seconds_ = time.seconds;
    set_nanosec(time.nanosec);
}

int64_t rtps::Time_t::to_ns() const
{
    int64_t nano = seconds_ * 1000000000ULL;
    nano += nanosec_;
    return nano;
}

int32_t rtps::Time_t::seconds() const
{
    return seconds_;
}

int32_t& rtps::Time_t::seconds()
{
    return seconds_;
}

void rtps::Time_t::seconds(
        int32_t sec)
{
    seconds_ = sec;
}

uint32_t rtps::Time_t::nanosec() const
{
    return nanosec_;
}

void rtps::Time_t::nanosec(
        uint32_t nanos)
{
    const uint32_t s_to_nano = 1000000000UL;
    if (nanos >= s_to_nano)
    {
        nanos %= s_to_nano; // Remove the seconds
    }
    set_nanosec(nanos);
}

uint32_t rtps::Time_t::fraction() const
{
    return fraction_;
}

void rtps::Time_t::fraction(
        uint32_t frac)
{
    set_fraction(frac);
}

Duration_t rtps::Time_t::to_duration_t() const
{
    return Duration_t(seconds_, nanosec_);
}

void rtps::Time_t::from_duration_t(const Duration_t& duration)
{
    seconds_ = duration.seconds;
    set_nanosec(duration.nanosec);
}

void rtps::Time_t::set_fraction(
        uint32_t frac)
{
    fraction_ = frac;
    nanosec_ = (fraction_ == 0xffffffff)
        ? 0xffffffff
        : static_cast<uint32_t>(std::lroundl(fraction_ * FRACTION_TO_NANO));
}

void rtps::Time_t::set_nanosec(
        uint32_t nanos)
{
    nanosec_ = nanos;
    fraction_ = (nanosec_ == 0xffffffff)
        ? 0xffffffff
        : static_cast<uint32_t>(std::lroundl(nanosec_ * NANO_TO_FRACTION));

    if (fraction_ != 0xffffffff)
    {
        uint32_t nano_check = static_cast<uint32_t>(std::lroundl(fraction_ * FRACTION_TO_NANO));
        while (nano_check != nanosec_)
        {
            nano_check = static_cast<uint32_t>(std::lroundl(++fraction_ * FRACTION_TO_NANO));
        }
    }
}