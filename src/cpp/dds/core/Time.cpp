/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/Time.hpp>
#include <dds/core/Duration.hpp>

namespace dds {
namespace core {

const Time Time::invalid()
{
    return Time(-1, 0xffffffff);
}

const Time Time::from_microsecs(
        int64_t microseconds)
{
    return Time(microseconds / 1000000, (microseconds % 1000000) * 1000);
}

const Time Time::from_millisecs(
        int64_t milliseconds)
{
    return Time(milliseconds / 1000, (milliseconds % 1000) * 1000000);
}

const Time Time::from_secs(
        double seconds)
{
    int64_t sec = static_cast<int64_t>(seconds);
    uint32_t nanos = static_cast<uint32_t>((seconds - sec) * 1000000000);

    return Time(sec, nanos);
}

Time::Time()
{
}

Time::Time(
        int64_t sec,
        uint32_t nanosec)
    : sec_(sec)
    , nsec_(nanosec)
{
}

int64_t Time::sec() const
{
    return sec_;
}

void Time::sec(
        int64_t s)
{
    sec_ = s;
}

uint32_t Time::nanosec() const
{
    return nsec_;
}

void Time::nanosec(
        uint32_t ns)
{
    nsec_ = ns;
}

int Time::compare(
        const Time& that) const
{
    if (sec_ == that.sec_)
    {
        if (nsec_ < that.nsec_)
        {
            return -1;
        }
        else if (nsec_ > that.nsec_)
        {
            return 1;
        }
        return 0;
    }

    if (sec_ < that.sec_)
    {
        return -1;
    }
    return 1;
}

bool Time::operator >(
        const Time& that) const
{
    if (sec_ > that.sec_)
    {
        return true;
    }
    else if (sec_ == that.sec_)
    {
        return nsec_ > that.nsec_;
    }
    return false;
}

bool Time::operator >=(
        const Time& that) const
{
    if (sec_ > that.sec_)
    {
        return true;
    }
    else if (sec_ == that.sec_)
    {
        return nsec_ >= that.nsec_;
    }
    return false;
}

bool Time::operator !=(
        const Time& that) const
{
    return sec_ != that.sec_ || nsec_ != that.nsec_;
}

bool Time::operator ==(
        const Time& that) const
{
    return sec_ == that.sec_ && nsec_ == that.nsec_;
}

bool Time::operator <=(
        const Time& that) const
{
    if (sec_ < that.sec_)
    {
        return true;
    }
    else if (sec_ == that.sec_)
    {
        return nsec_ <= that.nsec_;
    }
    return false;
}

bool Time::operator <(
        const Time& that) const
{
    if (sec_ < that.sec_)
    {
        return true;
    }
    else if (sec_ == that.sec_)
    {
        return nsec_ < that.nsec_;
    }
    return false;
}

Time& Time::operator +=(
        const Duration& a_ti)
{
    sec_ += a_ti.sec();
    nsec_ += a_ti.nanosec();
    if (nsec_ >= 1000000000)
    {
        ++sec_;
        nsec_ %= 1000000000;
    }
    return *this;
}

Time& Time::operator -=(
        const Duration& a_ti)
{
    // Check for negative results
    if (sec_ < a_ti.sec() || (sec_ == a_ti.sec() && nsec_ < a_ti.nanosec()))
    {
        sec_ = -1;
        nsec_ = 0xffffffff;
    }
    else
    {
        sec_ -= a_ti.sec();
        nsec_ -= a_ti.nanosec();
    }
    return *this;
}

int64_t Time::to_millisecs() const
{
    int64_t millis = sec_ * 1000;
    if (millis < sec_)
    {
        return -1;
    }
    millis += nsec_ / 1000000;
    return millis;
}

int64_t Time::to_microsecs() const
{
    int64_t micros = sec_ * 1000000;
    if (micros < sec_)
    {
        return -1;
    }
    micros += nsec_ / 1000;
    return micros;
}

double Time::to_secs() const
{
    double seconds = (double) sec_;
    double nanos = nsec_;
    seconds += nanos / 1000000000.;
    return seconds;
}

const Time operator +(
        const Time& lhs,
        const Duration& rhs)
{
    Time result(lhs);
    result += rhs;
    return result;
}

const Time operator +(
        const Duration& lhs,
        const Time& rhs)
{
    Time result(rhs);
    result += lhs;
    return result;
}

const Time operator -(
        const Time& lhs,
        const Duration& rhs)
{
    Time result(lhs);
    result -= rhs;
    return result;
}

} //namespace core
} //namespace dds

