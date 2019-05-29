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

constexpr uint64_t C_FRACTIONS = 4294967296;
constexpr uint64_t C_SECONDS = 1000000000;

#define frac_to_nano(x) (static_cast<uint32_t>(((x) * C_SECONDS) / C_FRACTIONS))
#define nano_to_frac(x) (static_cast<uint32_t>(((x) * C_FRACTIONS) / C_SECONDS))

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
    nanosec = static_cast<uint32_t>((sec - seconds) * C_SECONDS);
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
    int64_t nano = seconds * C_SECONDS;
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
    set_fraction(static_cast<uint32_t>((sec - seconds_) * C_FRACTIONS));
}

rtps::Time_t::Time_t(
        const eprosima::fastrtps::Time_t& time)
{
    seconds_ = time.seconds;
    set_nanosec(time.nanosec);
}

int64_t rtps::Time_t::to_ns() const
{
    int64_t nano = seconds_ * C_SECONDS;
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
    const uint32_t s_to_nano = static_cast<uint32_t>(C_SECONDS);
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
        : frac_to_nano(fraction_);
}

void rtps::Time_t::set_nanosec(
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