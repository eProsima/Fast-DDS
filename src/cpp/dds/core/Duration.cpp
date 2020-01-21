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

#include <dds/core/Duration.hpp>

namespace dds {
namespace core {

const Duration Duration::zero()
{
    return Duration();
}

const Duration Duration::infinite()   // {0x7fffffff, 0x7fffffff}
{
    return Duration(0x7fffffff, 0x7fffffff);
}

Duration::Duration()
{
}

Duration::Duration(
        int32_t sec,
        uint32_t nanosec)
    : sec_(sec)
    , nsec_(nanosec)
{
}

#if __cplusplus >= 199711L
Duration::Duration(
        int64_t sec,
        uint32_t nanosec)
    : sec_(static_cast<int32_t>(sec))
    , nsec_(nanosec)
{
}

#endif

const Duration Duration::from_microsecs(
        int64_t microseconds)
{
    return Duration(microseconds / 1000000, (microseconds % 1000000) * 1000);
}

const Duration Duration::from_millisecs(
        int64_t milliseconds)
{
    return Duration(milliseconds / 1000, (milliseconds % 1000) * 1000000);
}

const Duration Duration::from_secs(
        double seconds)
{
    int64_t sec = static_cast<int64_t>(seconds);
    uint32_t nanos = static_cast<uint32_t>((seconds - sec) * 1000000000);

    return Duration(sec, nanos);
}

int64_t Duration::sec() const
{
    return sec_;
}

void Duration::sec(
        int64_t s)
{
    sec_ = static_cast<int32_t>(s);
}

uint32_t Duration::nanosec() const
{
    return nsec_;
}

void Duration::nanosec(
        uint32_t ns)
{
    nsec_ = ns;
}

int Duration::compare(
        const Duration& that) const
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

bool Duration::operator >(
        const Duration& that) const
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

bool Duration::operator >=(
        const Duration& that) const
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

bool Duration::operator !=(
        const Duration& that) const
{
    return sec_ != that.sec_ || nsec_ != that.nsec_;
}

bool Duration::operator ==(
        const Duration& that) const
{
    return sec_ == that.sec_ && nsec_ == that.nsec_;
}

bool Duration::operator <=(
        const Duration& that) const
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

bool Duration::operator <(
        const Duration& that) const
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

Duration& Duration::operator +=(
        const Duration& a_ti)
{
    sec_ += (int32_t) a_ti.sec();
    nsec_ += a_ti.nanosec();
    if (nsec_ >= 1000000000)
    {
        ++sec_;
        nsec_ %= 1000000000;
    }
    return *this;
}

Duration& Duration::operator -=(
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
        sec_ -= (int32_t) a_ti.sec();
        nsec_ -= a_ti.nanosec();
    }
    return *this;
}

Duration& Duration::operator *=(
        uint64_t factor)
{
    uint64_t nanos = static_cast<uint64_t>(nsec_ * factor);
    int64_t seconds = static_cast<int64_t>(sec_ * factor);
    if (nanos < nsec_)
    {
        seconds += nanos / 1000000000;
        nanos %= 1000000000;
    }
    if (seconds < sec_)
    {
        *this = infinite();
    }
    return *this;
}

const Duration Duration::operator +(
        const Duration& other) const
{
    Duration result(*this);
    result += other;
    return result;
}

const Duration Duration::operator -(
        const Duration& other) const
{
    Duration result(*this);
    result -= other;
    return result;
}

int64_t Duration::to_millisecs() const
{
    int64_t millis = sec_ * 1000;
    if (millis < sec_)
    {
        return -1;
    }
    millis += nsec_ / 1000000;
    return millis;
}

int64_t Duration::to_microsecs() const
{
    int64_t micros = sec_ * 1000000;
    if (micros < sec_)
    {
        return -1;
    }
    micros += nsec_ / 1000;
    return micros;
}

double Duration::to_secs() const
{
    double seconds = sec_;
    double nanos = nsec_;
    seconds += nanos / 1000000000.;
    return seconds;
}

const Duration OMG_DDS_API operator *(
        uint64_t lhs,
        const Duration& rhs)
{
    Duration result(rhs);
    result *= lhs;
    return result;
}

const Duration OMG_DDS_API operator *(
        const Duration& lhs,
        uint64_t rhs)
{
    Duration result(lhs);
    result *= rhs;
    return result;
}

const Duration OMG_DDS_API operator /(
        const Duration& lhs,
        uint64_t rhs)
{
    return Duration(static_cast<int64_t>(lhs.sec() / static_cast<int64_t>(rhs)), lhs.nanosec() / (uint32_t) rhs);
}

} //namespace core
} //namespace dds

