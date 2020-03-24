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

#ifndef OMG_DDS_CORE_DURATION_HPP_
#define OMG_DDS_CORE_DURATION_HPP_

#include <dds/core/types.hpp>
#include <dds/core/detail/Duration.hpp>

namespace dds {
namespace core {

/**
 * @file
 * This class represents a time interval.
 */

/**
 * Duration represents a time interval and -
 * * Can be incremented by durations expressed as seconds, nanoseconds
 *   milliseconds, or Duration objects.
 *
 * * Can be converted to and from Durations expressed in
 *   milliseconds (or other units) as integer types.
 */
class OMG_DDS_API Duration : public dds::core::Value<detail::Duration>
{
public:

    /**
     * Create a Duration elapsing zero seconds.
     */
    static const Duration zero()
    {
        static Duration zero (0, 0);
        return zero;
    }

    /**
     * Create an infinite Duration.
     */
    static const Duration infinite()
    {
        static Duration inf (TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS);
        return inf;
    }

    /**
     * Create a Duration elapsing the default amount of time (zero seconds).
     */
    Duration()
        : Value<detail::Duration>(0, 0)
    {
    }

    /**
     * Create a Duration elapsing a specific amount of time.
     *
     * @param sec     Amount of seconds for the Duration.
     * @param nanosec Amount of nanoseconds for the Duration.
     */
    explicit Duration(
            int32_t sec,
            uint32_t nanosec = 0)
        : Value<detail::Duration>(sec, nanosec)
    {
    }

#if __cplusplus >= 199711L
    /** @copydoc dds::core::Duration::Duration(int32_t sec, uint32_t nanosec = 0) */
    explicit Duration(
            int64_t sec,
            uint32_t nanosec = 0)
       : Value<detail::Duration>(sec, nanosec)
   {
   }
#endif

    /**
     * Create a Duration from a number of microseconds
     * @param microseconds number of microseconds
     */
    static const Duration from_microsecs(
            int64_t microseconds)
    {
        int32_t sec = microseconds/1000000;
        uint32_t nano = (microseconds - (sec*1000000)) * 1000;
        return Duration(sec, nano);
    }

    /**
     * Create a Duration from a number of milliseconds
     * @param miliseconds number of milliseconds
     */
    static const Duration from_millisecs(
            int64_t milliseconds)
    {
        int32_t sec = milliseconds/1000;
        uint32_t nano = (milliseconds - (sec*1000)) * 1000000;
        return Duration(sec, nano);
    }


    /**
     * Create a Duration from a number of seconds
     * @param seconds number of seconds
     */
    static const Duration from_secs(
            double seconds)
    {
        int32_t sec = seconds;
        uint32_t nano = (seconds - sec) * 1000000000;
        return Duration(sec, nano);
    }

    /**
     * Get seconds part of the Duration.
     * @return number of seconds
     */
    int64_t sec() const
    {
        return delegate().seconds;
    }
    /**
     * Set number of seconds
     * @param s number of seconds
     */
    void sec(
            int64_t s)
    {
        delegate().seconds = s;
    }

    /**
     * Get nanoseconds part of the Duration.
     * @return number of nanoseconds
     */
    uint32_t nanosec() const
    {
        return delegate().nanosec;
    }
    /**
     * Set number of nanoseconds
     * @param ns number of nanoseconds
     */
    void nanosec(
            uint32_t ns)
    {
        delegate().nanosec = ns;
    }

    /**
     * Returns an integer value for a comparison of two Durations:
     * 1 if this Duration is greater than the comparator (that)
     * -1 if the Duration is less than the comparator (that)
     * 0 if the Duration matches the comparator (that)
     *
     * @param that Duration to compare
     *
     * @return comparison result
     */
    int compare(
            const Duration& that) const
    {
        if (delegate() > that.delegate())
        {
            return 1;
        }
        if (delegate() < that.delegate())
        {
            return -1;
        }
        return 0;
    }

    /**
     * Returns true if the Duration is greater than the comparator
     *
     * @param that Duration to compare
     * @return comparison result
     */
    bool operator >(
            const Duration& that) const
    {
        return delegate() > that.delegate();
    }

    /**
     * Returns true if the Duration is greater than or equal to the comparator
     * @param Duration &that
     */
    bool operator >=(
            const Duration& that) const
    {
        return delegate() >= that.delegate();
    }


    /**
     * Returns true if the Duration is not equal to the comparator
     *
     * @param that Duration to compare
     * @return comparison result
     */
    bool operator !=(
            const Duration& that) const
    {
        return delegate() != that.delegate();
    }


    /**
     * Returns true if the Duration is equal to the comparator
     *
     * @param that Duration to compare
     * @return comparison result
     */
    bool operator ==(
            const Duration& that) const
    {
        return delegate() == that.delegate();
    }


    /**
     * Returns true if the Duration is less than or equal to the comparator
     *
     * @param that Duration to compare
     * @return comparison result
     */
    bool operator <=(
            const Duration& that) const
    {
        return delegate() <= that.delegate();
    }


    /**
     * Returns true if the Duration is less than the comparator
     *
     * @param that Duration to compare
     * @return comparison result
     */
    bool operator <(
            const Duration& that) const
    {
        return delegate() < that.delegate();
    }


    /**
     * Add a Duration to this Duration
     *
     * @param a_ti Duration to add
     * @return this Duration + a_ti
     */
    Duration& operator +=(
            const Duration& a_ti)
    {
        delegate() = delegate() + a_ti.delegate();
        return *this;
    }

    /**
     * Subtract a Duration from this Duration
     *
     * @param a_ti Duration to subtract
     * @return this Duration - a_ti
     */
    Duration& operator -=(
            const Duration& a_ti)
    {
        delegate() = delegate() - a_ti.delegate();
        return *this;
    }

    /**
     * Add a Duration to Duration
     *
     * @param other a Duration
     * @return Duration + other
     */
    const Duration operator +(
            const Duration& other) const
    {
        Duration result (*this);
        return (result += other);
    }


    /**
     * Subtract a Duration from Duration
     *
     * @param other a Duration
     * @return the Duration - other
     */
    const Duration operator -(
            const Duration& other) const
    {
        Duration result (*this);
        return (result -= other);
    }

    /**
     * Returns this Duration in milliseconds.
     *
     * @return the duration in milliseconds
     */
    int64_t to_millisecs() const
    {
        int64_t ms = delegate().nanosec/1000000;
        return ms + delegate().seconds*1000;
    }

    /**
     * Returns this Duration in micro-seconds.
     *
     * @return the duration in micro-seconds
     */
    int64_t to_microsecs() const
    {
        int64_t ms = delegate().nanosec/1000;
        return ms + delegate().seconds*1000000;
    }

    /**
     * Returns this Duration in seconds.
     *
     * @return the duration in seconds
     */
    double to_secs() const
    {
        double sec = delegate().nanosec/1000000000.0;
        return sec + delegate().seconds;
    }

};

} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_DURATION_HPP_
