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
} // namespace rtps

/**
 * Structure Time_t, used to describe times.
 * @ingroup COMMON_MODULE
 */
struct RTPS_DllAPI Time_t
{
    int32_t seconds;
    uint32_t nanosec;

    //! Default constructor. Sets values to zero.
    Time_t()
    {
        seconds = 0;
        nanosec = 0;
    }

    /**
    * @param sec Seconds
    * @param nsec Nanoseconds
    */
    Time_t(
            int32_t sec,
            uint32_t nsec)
    {
        seconds = sec;
        nanosec = nsec;
    }

    /**
     * @param sec Seconds. The fractional part is converted to nanoseconds.
     */
    Time_t(
            long double sec)
    {
        seconds = static_cast<int32_t>(sec);
        nanosec = static_cast<uint32_t>((sec - seconds) * 1000000000ULL);
    }

    void fraction(
            uint32_t frac)
    {
        nanosec = static_cast<uint32_t>(frac * rtps::FRACTION_TO_NANO);
    }

    uint32_t fraction() const
    {
        return static_cast<uint32_t>(nanosec * rtps::NANO_TO_FRACTION);
    }
};

typedef Time_t Duration_t;

namespace rtps{

/**
 * Structure Time_t, used to describe times at RTPS protocol.
 * @ingroup COMMON_MODULE
 */
class RTPS_DllAPI Time_t
{
public:

    //! Default constructor. Sets values to zero.
    Time_t()
    {
        seconds_ = 0;
        fraction_ = 0;
        nanosec_ = 0;
    }
    /**
    * @param sec Seconds
    * @param frac Fraction of second
    */
    Time_t(
            int32_t sec,
            uint32_t frac)
    {
        seconds_ = sec;
        set_fraction(frac);
    }

    /**
     * @param sec Seconds. The fractional part is converted to nanoseconds.
     */
    Time_t(
            long double sec)
    {
        seconds_ = static_cast<int32_t>(sec);
        set_fraction(static_cast<uint32_t>((sec - seconds_) * 4294967296ULL));
    }

    /**
     * @param sec Seconds. The fractional part is converted to nanoseconds.
     */
    Time_t(
            fastrtps::Time_t time)
    {
        seconds_ = static_cast<int32_t>(time.seconds);
        set_fraction(static_cast<uint32_t>((time.seconds - seconds_) * 4294967296ULL));
    }

    /**
     *  Returns stored time as nanoseconds (including seconds)
     */
    int64_t to_ns() const
    {
        int64_t nano = seconds_ * 1000000000ULL;
        nano += nanosec_;
        return nano;
    }

    /**
     * Retrieve the seconds field.
     */
    int32_t seconds() const
    {
        return seconds_;
    }

    /**
     * Retrieve the seconds field by ref.
     */
    int32_t& seconds()
    {
        return seconds_;
    }

    /**
     * Sets seconds field.
     */
    void seconds(
            int32_t sec)
    {
        seconds_ = sec;
    }

    /**
     * Retrieve the nanosec field.
     */
    uint32_t nanosec() const
    {
        return nanosec_;
    }

    /**
     * Sets nanoseconds field and updates the fraction.
     */
    void nanosec(
            uint32_t nanos)
    {
        const uint32_t s_to_nano = 1000000000UL;
        if (nanos >= s_to_nano)
        {
            nanos %= s_to_nano; // Remove the seconds
        }
        set_nanosec(nanos);
    }

    /**
     * Retrieve the fraction field.
     */
    uint32_t fraction() const
    {
        return fraction_;
    }

    /**
     * Sets fraction field and updates the nanoseconds.
     */
    void fraction(
            uint32_t frac)
    {
        set_fraction(frac);
    }

    Duration_t to_duration_t() const
    {
        return Duration_t(seconds_, nanosec_);
    }

    void from_duration_t(const Duration_t& duration)
    {
        seconds_ = duration.seconds;
        set_nanosec(duration.nanosec);
    }

private:
    //!Seconds
    int32_t seconds_;

    //!Fraction of second (1 fraction = 1/(2^32) seconds)
    uint32_t fraction_;

    //!Nanoseconds
    uint32_t nanosec_;

    void set_fraction(
            uint32_t frac)
    {
        fraction_ = frac;
        nanosec_ = static_cast<uint32_t>(fraction_ * FRACTION_TO_NANO);
    }

    void set_nanosec(
            uint32_t nanos)
    {
        nanosec_ = nanos;
        fraction_ = static_cast<uint32_t>(nanosec_ * NANO_TO_FRACTION);
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
    if(t1.seconds() != t2.seconds())
    {
        return false;
    }
    if(t1.fraction() != t2.fraction())
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
    if (t1.seconds() != t2.seconds())
    {
        return true;
    }
    if (t1.fraction() != t2.fraction())
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
    if (t1.seconds() < t2.seconds())
    {
        return true;
    }
    else if (t1.seconds() > t2.seconds())
    {
        return false;
    }
    else
    {
        if (t1.fraction() < t2.fraction())
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
    if (t1.seconds() > t2.seconds())
    {
        return true;
    }
    else if (t1.seconds() < t2.seconds())
    {
        return false;
    }
    else
    {
        if (t1.fraction() > t2.fraction())
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
    if (t1.seconds() < t2.seconds())
    {
        return true;
    }
    else if (t1.seconds() > t2.seconds())
    {
        return false;
    }
    else
    {
        if (t1.fraction() <= t2.fraction())
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
    if (t1.seconds() > t2.seconds())
    {
        return true;
    }
    else if (t1.seconds() < t2.seconds())
    {
        return false;
    }
    else
    {
        if (t1.fraction() >= t2.fraction())
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
    long double t_aux = t.seconds() + (((long double)t.nanosec()) / 1000000000ULL);
    return output << t_aux;
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
    Time_t result(ta.seconds() + tb.seconds(), ta.fraction() + tb.fraction());
    if (result.fraction() < ta.fraction()) // Overflow is detected by any of them
    {
        ++result.seconds();
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
    Time_t result(ta.seconds() - tb.seconds(), ta.fraction() - tb.fraction());
    if (result.fraction() > ta.fraction()) // Overflow is detected by ta
    {
        --result.seconds();
    }
    return result;
}

#endif

const Time_t c_RTPSTimeInfinite(0x7fffffff,0xffffffff);
const Time_t c_RTPSTimeZero(0,0);
const Time_t c_RTPSTimeInvalid(-1,0xffffffff);

} // namespace rtps

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
    if(t1.nanosec != t2.nanosec)
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
static inline Time_t operator+(
        const Time_t &ta,
        const Time_t &tb)
{
    Time_t result(ta.seconds + tb.seconds, ta.nanosec + tb.nanosec);
    if (result.nanosec < ta.nanosec) // Overflow is detected by any of them
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
    Time_t result(ta.seconds - tb.seconds, ta.nanosec - tb.nanosec);
    if (result.nanosec > ta.nanosec) // Overflow is detected by ta
    {
        --result.seconds;
    }
    return result;
}

#endif

const Time_t c_TimeInfinite(0x7fffffff,0xffffffff);
const Time_t c_TimeZero(0,0);
const Time_t c_TimeInvalid(-1,0xffffffff);

} // namespace fastrtps
} // namespace eprosima

#endif /* TIME_T_H_ */
