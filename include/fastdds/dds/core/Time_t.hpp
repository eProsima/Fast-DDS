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

/**
 * @file Time_t.hpp
 */

#ifndef FASTDDS_DDS_CORE__TIME_T_HPP
#define FASTDDS_DDS_CORE__TIME_T_HPP

#include <fastdds/fastdds_dll.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Structure Time_t, used to describe times at a DDS level.
 */
struct FASTDDS_EXPORTED_API Time_t
{
    static constexpr int32_t INFINITE_SECONDS = 0x7fffffff;
    static constexpr uint32_t INFINITE_NANOSECONDS = 0xffffffffu;

    int32_t seconds;
    uint32_t nanosec;

    //! Default constructor. Sets values to zero.
    Time_t();

    /**
     * @param sec Seconds
     * @param nsec Nanoseconds
     */
    Time_t(
            int32_t sec,
            uint32_t nsec);

    /**
     * @param sec Seconds. The fractional part is converted to nanoseconds.
     */
    Time_t(
            long double sec);

    void fraction(
            uint32_t frac);

    uint32_t fraction() const;

    /**
     *  Returns stored time as nanoseconds (including seconds)
     */
    int64_t to_ns() const;

    inline bool is_infinite() const noexcept
    {
        return is_infinite(*this);
    }

    /**
     * Fills a Time_t struct with a representation of the current time.
     *
     * @param ret Reference to the structure to be filled in.
     */
    static void now(
            Time_t& ret);

    static inline constexpr bool is_infinite(
            const Time_t& t) noexcept
    {
        return (INFINITE_SECONDS == t.seconds) || (INFINITE_NANOSECONDS == t.nanosec);
    }

};

using Duration_t = Time_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Comparison assignment
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if equal
 */
static inline bool operator ==(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds != t2.seconds)
    {
        return false;
    }
    if (t1.nanosec != t2.nanosec)
    {
        return false;
    }
    return true;
}

/**
 * Comparison assignment
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if not equal
 */
static inline bool operator !=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds != t2.seconds)
    {
        return true;
    }
    if (t1.nanosec != t2.nanosec)
    {
        return true;
    }
    return false;
}

/**
 * Checks if a Time_t is less than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is less than the second
 */
static inline bool operator <(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds < t2.seconds)
    {
        return true;
    }
    else if (t1.seconds > t2.seconds)
    {
        return false;
    }
    else
    {
        if (t1.nanosec < t2.nanosec)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

/**
 * Checks if a Time_t is greater than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is greater than the second
 */
static inline bool operator >(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds > t2.seconds)
    {
        return true;
    }
    else if (t1.seconds < t2.seconds)
    {
        return false;
    }
    else
    {
        if (t1.nanosec > t2.nanosec)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

/**
 * Checks if a Time_t is less or equal than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is less or equal than the second
 */
static inline bool operator <=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds < t2.seconds)
    {
        return true;
    }
    else if (t1.seconds > t2.seconds)
    {
        return false;
    }
    else
    {
        if (t1.nanosec <= t2.nanosec)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

/**
 * Checks if a Time_t is greater or equal than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is greater or equal than the second
 */
static inline bool operator >=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds > t2.seconds)
    {
        return true;
    }
    else if (t1.seconds < t2.seconds)
    {
        return false;
    }
    else
    {
        if (t1.nanosec >= t2.nanosec)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

inline std::ostream& operator <<(
        std::ostream& output,
        const Time_t& t)
{
    long double t_aux = t.seconds + (((long double)t.nanosec) / 1000000000ULL);
    return output << t_aux;
}

/**
 * Adds two Time_t.
 * @param ta First Time_t to add
 * @param tb Second Time_t to add
 * @return A new Time_t with the result.
 */
static inline Time_t operator +(
        const Time_t& ta,
        const Time_t& tb)
{
    Time_t result(ta.seconds + tb.seconds, ta.nanosec + tb.nanosec);
    if (result.nanosec < ta.nanosec) // Overflow is detected by any of them
    {
        ++result.seconds;
    }
    return result;
}

/**
 * Subtracts two Time_t.
 * @param ta First Time_t to subtract
 * @param tb Second Time_t to subtract
 * @return A new Time_t with the result.
 */
static inline Time_t operator -(
        const Time_t& ta,
        const Time_t& tb)
{
    Time_t result(ta.seconds - tb.seconds, ta.nanosec - tb.nanosec);
    if (result.nanosec > ta.nanosec) // Overflow is detected by ta
    {
        --result.seconds;
    }
    return result;
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

//! Time_t (dds::Duration_t) representing an infinite time. DONT USE IT IN CONSTRUCTORS
const Time_t c_TimeInfinite{Time_t::INFINITE_SECONDS, Time_t::INFINITE_NANOSECONDS};
//! Time_t (dds::Duration_t) representing a zero time. DONT USE IT IN CONSTRUCTORS
const Time_t c_TimeZero{0, 0};
//! Time_t (dds::Duration_t) representing an invalid time. DONT USE IT IN CONSTRUCTORS
const Time_t c_TimeInvalid{-1, Time_t::INFINITE_NANOSECONDS};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE__TIME_T_HPP
