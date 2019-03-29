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
 * @file Time_t.h
 */

#ifndef TIME_T_H_
#define TIME_T_H_
#include "../../fastrtps_dll.h"
#include <cmath>
#include <cstdint>
#include <iostream>

namespace eprosima{
namespace fastrtps{
namespace rtps{

// 1 fraction = 1/(2^32) seconds
constexpr long double FRACTION_TO_NANO = 0.23283064365386962890625; // 1000000000 / 4294967296
constexpr long double NANO_TO_FRACTION = 4.294967296; // 4294967296 / 1000000000

/**
 * Structure Time_t, used to describe times.
 * @ingroup COMMON_MODULE
 */
struct RTPS_DllAPI Time_t
{
    //!Seconds
    int32_t seconds;

    //!Fraction of second (1 fraction = 1/(2^32) seconds)
    uint32_t fraction;

    //! Default constructor. Sets values to zero.
    Time_t()
    {
        seconds = 0;
        fraction = 0;
    }
    /**
    * @param sec Seconds
    * @param frac Fraction of second
    */
    Time_t(
            int32_t sec,
            uint32_t frac)
    {
        seconds = sec;
        fraction = frac;
    }

    Time_t(long double sec)
    {
        seconds = static_cast<int32_t>(sec);
        fraction = static_cast<uint32_t>((sec - seconds) * 4294967296ULL);
    }

    /**
     *  Returns stored time as nanoseconds
     */
    inline int64_t to_ns() const
    {
        int64_t nano = seconds * 1000000000ULL;
        nano += fraction * FRACTION_TO_NANO;
        return nano;
    }

    /**
     * Retrieve the nanosec equivalent field.
     * Converts the internal fraction to nanoseconds.
     */
    uint32_t nanosec() const
    {
        return static_cast<uint32_t>(fraction * FRACTION_TO_NANO);
    }

    /**
     * Sets fraction field as nanoseconds. If nanos is greater or equal than 1.000.000.000 (one second)
     * discards all seconds.
     */
    void nanosec(uint32_t nanos)
    {
        const uint32_t s_to_nano = 1000000000UL;
        if (nanos >= s_to_nano)
        {
            nanos %= s_to_nano; // Remove the seconds
        }
        fraction = static_cast<uint32_t>(nanos * NANO_TO_FRACTION);
    }
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Comparison assignment
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if equal
 */
static inline bool operator==(
        const Time_t& t1,
        const Time_t& t2)
{
    if(t1.seconds != t2.seconds)
    {
        return false;
    }
    if(t1.fraction != t2.fraction)
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
static inline bool operator!=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds != t2.seconds)
    {
        return true;
    }
    if (t1.fraction != t2.fraction)
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
static inline bool operator<(
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
        if (t1.fraction < t2.fraction)
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
 * Checks if a Time_t is greather than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is greather than the second
 */
static inline bool operator>(
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
        if (t1.fraction > t2.fraction)
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
static inline bool operator<=(
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
        if (t1.fraction <= t2.fraction)
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
 * Checks if a Time_t is greather or equal than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is greather or equal than the second
 */
static inline bool operator>=(
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
        if (t1.fraction >= t2.fraction)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

inline std::ostream& operator<<(
        std::ostream& output,
        const Time_t& t)
{
    return output << t.seconds << "." << t.fraction;
}

/**
 * Adds two Time_t.
 * @param ta First Time_t to add
 * @param tb Second Time_t to add
 * @return A new Time_t with the result.
 */
static inline Time_t operator+(
        const Time_t &ta,
        const Time_t &tb)
{
    Time_t result(ta.seconds + tb.seconds, ta.fraction + tb.fraction);
    if (result.fraction < ta.fraction) // Overflow is detected by any of them
    {
        ++result.seconds;
    }
    return result;
}

/**
 * Substracts two Time_t.
 * @param ta First Time_t to substract
 * @param tb Second Time_t to substract
 * @return A new Time_t with the result.
 */
static inline Time_t operator-(
        const Time_t &ta,
        const Time_t &tb)
{
    Time_t result(ta.seconds - tb.seconds, ta.fraction - tb.fraction);
    if (result.fraction > ta.fraction) // Overflow is detected by ta
    {
        --result.seconds;
    }
    return result;
}

#endif

const Time_t c_TimeInfinite(0x7fffffff,0xffffffff);
const Time_t c_TimeZero(0,0);
const Time_t c_TimeInvalid(-1,0xffffffff);

//typedef Time_t Duration_t;
struct Duration_t
{
    int32_t seconds;
    uint32_t nanosec;

    //! Default constructor. Sets values to zero.
    Duration_t()
    {
        seconds = 0;
        nanosec = 0;
    }

    //! Constructor from a Time_t
    Duration_t(const Time_t& time)
    {
        seconds = time.seconds;
        if (time.fraction == c_TimeInfinite.fraction)
        {
            nanosec = time.fraction;
        }
        else
        {
            nanosec = time.nanosec();
        }
    }

    /**
    * @param sec Seconds
    * @param nsec Nanoseconds
    */
    Duration_t(
            int32_t sec,
            uint32_t nsec)
    {
        seconds = sec;
        nanosec = nsec;
    }

    Duration_t(long double sec)
    {
        seconds = static_cast<int32_t>(sec);
        nanosec = static_cast<uint32_t>((sec - seconds) * 1000000000ULL);
    }

    Time_t to_time() const
    {
        Time_t time;
        time.seconds = seconds;
        if (time.fraction == c_TimeInfinite.fraction)
        {
            time.fraction = nanosec;
        }
        else
        {
            time.nanosec(nanosec);
        }
        return time;
    }
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Comparison assignment
 * @param t1 First Duration_t to compare
 * @param t2 Second Duration_t to compare
 * @return True if equal
 */
static inline bool operator==(
        const Duration_t& t1,
        const Duration_t& t2)
{
    if(t1.seconds != t2.seconds)
    {
        return false;
    }
    if(t1.nanosec != t2.nanosec)
    {
        return false;
    }
    return true;
}

/**
 * Comparison assignment
 * @param t1 First Duration_t to compare
 * @param t2 Second Duration_t to compare
 * @return True if not equal
 */
static inline bool operator!=(
        const Duration_t& t1,
        const Duration_t& t2)
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
 * Checks if a Duration_t is less than other.
 * @param t1 First Duration_t to compare
 * @param t2 Second Duration_t to compare
 * @return True if the first Duration_t is less than the second
 */
static inline bool operator<(
        const Duration_t& t1,
        const Duration_t& t2)
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
 * Checks if a Duration_t is greather than other.
 * @param t1 First Duration_t to compare
 * @param t2 Second Duration_t to compare
 * @return True if the first Duration_t is greather than the second
 */
static inline bool operator>(
        const Duration_t& t1,
        const Duration_t& t2)
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
 * Checks if a Duration_t is less or equal than other.
 * @param t1 First Duration_t to compare
 * @param t2 Second Duration_t to compare
 * @return True if the first Duration_t is less or equal than the second
 */
static inline bool operator<=(
        const Duration_t& t1,
        const Duration_t& t2)
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
 * Checks if a Duration_t is greather or equal than other.
 * @param t1 First Duration_t to compare
 * @param t2 Second Duration_t to compare
 * @return True if the first Duration_t is greather or equal than the second
 */
static inline bool operator>=(
        const Duration_t& t1,
        const Duration_t& t2)
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

inline std::ostream& operator<<(
        std::ostream& output,
        const Duration_t& t)
{
    return output << t.seconds << "." << t.nanosec;
}

/**
 * Adds two Duration_t.
 * @param ta First Duration_t to add
 * @param tb Second Duration_t to add
 * @return A new Duration_t with the result.
 */
static inline Duration_t operator+(
        const Duration_t &ta,
        const Duration_t &tb)
{
    Duration_t result(ta.seconds + tb.seconds, ta.nanosec + tb.nanosec);
    if (result.nanosec < ta.nanosec) // Overflow is detected by any of them
    {
        ++result.seconds;
    }
    return result;
}

/**
 * Substracts two Duration_t.
 * @param ta First Duration_t to substract
 * @param tb Second Duration_t to substract
 * @return A new Duration_t with the result.
 */
static inline Duration_t operator-(
        const Duration_t &ta,
        const Duration_t &tb)
{
    Duration_t result(ta.seconds - tb.seconds, ta.nanosec - tb.nanosec);
    if (result.nanosec > ta.nanosec) // Overflow is detected by ta
    {
        --result.seconds;
    }
    return result;
}

#endif

}
}
}

#endif /* TIME_T_H_ */
