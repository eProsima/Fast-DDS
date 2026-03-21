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
 * @file Time_t.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__TIME_T_HPP
#define FASTDDS_RTPS_COMMON__TIME_T_HPP

#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/fastdds_dll.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Structure Time_t, used to describe times at RTPS protocol.
 * @ingroup COMMON_MODULE
 */
class FASTDDS_EXPORTED_API Time_t
{
public:

    //! Default constructor. Sets values to zero.
    Time_t() = default;

    /**
     * @param sec Seconds
     * @param frac Fraction of second
     */
    Time_t(
            int32_t sec,
            uint32_t frac);

    /**
     * @param sec Seconds. The fractional part is converted to nanoseconds.
     */
    Time_t(
            long double sec);

    /**
     * @param time fastdds::dds::Time_t, aka. dds::Duration_t.
     */
    Time_t(
            const eprosima::fastdds::dds::Time_t& time);

    /**
     *  Returns stored time as nanoseconds (including seconds)
     */
    int64_t to_ns() const;

    /**
     *  @param nanosecs Stores given time as nanoseconds (including seconds)
     */
    void from_ns(
            int64_t nanosecs);

    /**
     * Retrieve the seconds field.
     */
    int32_t seconds() const;

    /**
     * Retrieve the seconds field by ref.
     */
    int32_t& seconds();

    /**
     * Sets seconds field.
     */
    void seconds(
            int32_t sec);

    /**
     * Retrieve the nanosec field.
     */
    uint32_t nanosec() const;

    /**
     * Sets nanoseconds field and updates the fraction.
     */
    void nanosec(
            uint32_t nanos);

    /**
     * Retrieve the fraction field.
     */
    uint32_t fraction() const;

    /**
     * Retrieve the fraction field by ref.
     */
    uint32_t& fraction();

    /**
     * Sets fraction field and updates the nanoseconds.
     */
    void fraction(
            uint32_t frac);

    eprosima::fastdds::dds::Duration_t to_duration_t() const;

    void from_duration_t(
            const eprosima::fastdds::dds::Duration_t& duration);

    /**
     * Fills a Time_t struct with a representation of the current time.
     *
     * @param ret Reference to the structure to be filled in.
     */
    static void now(
            Time_t& ret);

private:

    //!Seconds
    int32_t seconds_ = 0;

    //!Fraction of second (1 fraction = 1/(2^32) seconds)
    uint32_t fraction_ = 0;

    //!Nanoseconds
    uint32_t nanosec_ = 0;

    void set_fraction(
            uint32_t frac);

    void set_nanosec(
            uint32_t nanos);
};

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
    if (t1.seconds() != t2.seconds())
    {
        return false;
    }
    if (t1.fraction() != t2.fraction())
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
static inline bool operator <(
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
 * Checks if a Time_t is greater than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is greater than the second
 */
static inline bool operator >(
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
static inline bool operator <=(
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
 * Checks if a Time_t is greater or equal than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is greater or equal than the second
 */
static inline bool operator >=(
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

inline std::ostream& operator <<(
        std::ostream& output,
        const Time_t& t)
{
    return output << t.seconds() << "." << t.nanosec();
}

inline std::istream& operator >>(
        std::istream& input,
        Time_t& t)
{
    std::istream::sentry s(input);

    if (s)
    {
        char point;
        int32_t sec = 0;
        uint32_t nano = 0;
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            input >> sec;
            input >> point >> nano;
            // nano could not be bigger or equal than 1 sec
            if ( point != '.' || nano >= 1000000000 )
            {
                input.setstate(std::ios_base::failbit);
                nano = 0;
            }
        }
        catch (std::ios_base::failure& )
        {
        }

        t.seconds(sec);
        t.nanosec(nano);

        input.exceptions(excp_mask);
    }

    return input;
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
    Time_t result(ta.seconds() + tb.seconds(), ta.fraction() + tb.fraction());
    if (result.fraction() < ta.fraction()) // Overflow is detected by any of them
    {
        ++result.seconds();
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
    Time_t result(ta.seconds() - tb.seconds(), ta.fraction() - tb.fraction());
    if (result.fraction() > ta.fraction()) // Overflow is detected by ta
    {
        --result.seconds();
    }
    return result;
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

const Time_t c_RTPSTimeInfinite{0x7fffffff, 0xffffffff};
const Time_t c_RTPSTimeZero{0, 0};
const Time_t c_RTPSTimeInvalid{-1, 0xffffffff};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

// defines to avoid the "static initialization order fiasco"
#define TIME_T_INFINITE_SECONDS (eprosima::fastdds::dds::Time_t::INFINITE_SECONDS)
#define TIME_T_INFINITE_NANOSECONDS (eprosima::fastdds::dds::Time_t::INFINITE_NANOSECONDS)

#endif // FASTDDS_RTPS_COMMON__TIME_T_HPP
