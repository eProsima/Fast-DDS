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
 * @file Time_t.h
 */

#ifndef _FASTDDS_RTPS_TIME_T_H_
#define _FASTDDS_RTPS_TIME_T_H_
#include <fastrtps/fastrtps_dll.h>
#include <cmath>
#include <cstdint>
#include <iostream>

// defines to avoid the "static initialization order fiasco"
#define TIME_T_INFINITE_SECONDS 0x7fffffff
#define TIME_T_INFINITE_NANOSECONDS 0xffffffff

namespace eprosima {
namespace fastrtps {

/**
 * Structure Time_t, used to describe times.
 * @ingroup COMMON_MODULE
 */
struct RTPS_DllAPI Time_t
{
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

    /**
     * Fills a Time_t struct with a representation of the current time.
     *
     * @param ret Reference to the structure to be filled in.
     */
    static void now(
            Time_t& ret);


#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

    /**
     * Comparison assignment
     * @param other Second Time_t to compare
     * @return True if equal
     */
    inline bool operator ==(
            const Time_t& other) const
    {
        if (seconds != other.seconds)
        {
            return false;
        }
        if (nanosec != other.nanosec)
        {
            return false;
        }
        return true;
    }

    /**
     * Comparison assignment
     * @param other Second Time_t to compare
     * @return True if not equal
     */
    inline bool operator !=(
            const Time_t& other) const
    {
        return !(*this == other);
    }

    /**
     * Checks if a Time_t is less than other.
     * @param other Second Time_t to compare
     * @return True if the first Time_t is less than the second
     */
    inline bool operator <(
            const Time_t& other) const
    {
        if (seconds < other.seconds)
        {
            return true;
        }
        else if (seconds > other.seconds)
        {
            return false;
        }
        else
        {
            if (nanosec < other.nanosec)
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
     * @param other Second Time_t to compare
     * @return True if the first Time_t is greater than the second
     */
    inline bool operator >(
            const Time_t& other) const
    {
        return other < *this;
    }

    /**
     * Checks if a Time_t is less or equal than other.
     * @param other Second Time_t to compare
     * @return True if the first Time_t is less or equal than the second
     */
    inline bool operator <=(
            const Time_t& other) const
    {
        return !(*this > other);
    }

    /**
     * Checks if a Time_t is greater or equal than other.
     * @param other Second Time_t to compare
     * @return True if the first Time_t is greater or equal than the second
     */
    inline bool operator >=(
            const Time_t& other) const
    {
        return !(*this < other);
    }

    /**
     * Adds two Time_t.
     * @param other Second Time_t to add
     * @return A new Time_t with the result.
     */
    inline Time_t operator +(
            const Time_t& other) const
    {
        Time_t result(seconds + other.seconds, nanosec + other.nanosec);
        if (result.nanosec < nanosec) // Overflow is detected by any of them
        {
            ++result.seconds;
        }
        return result;
    }

    /**
     * Subtracts two Time_t.
     * @param other Second Time_t to subtract
     * @return A new Time_t with the result.
     */
    inline Time_t operator -(
            const Time_t& other) const
    {
        Time_t result(seconds - other.seconds, nanosec - other.nanosec);
        if (result.nanosec > nanosec) // Overflow is detected by ta
        {
            --result.seconds;
        }
        return result;
    }

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

};

using Duration_t = Time_t;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

inline std::ostream& operator <<(
        std::ostream& output,
        const Time_t& t)
{
    long double t_aux = t.seconds + (((long double)t.nanosec) / 1000000000ULL);
    return output << t_aux;
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

//! Time_t (Duration_t) representing an infinite time. DONT USE IT IN CONSTRUCTORS
const Time_t c_TimeInfinite{TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS};
//! Time_t (Duration_t) representing a zero time. DONT USE IT IN CONSTRUCTORS
const Time_t c_TimeZero{0, 0};
//! Time_t (Duration_t) representing an invalid time. DONT USE IT IN CONSTRUCTORS
const Time_t c_TimeInvalid{-1, TIME_T_INFINITE_NANOSECONDS};

namespace rtps {

/**
 * Structure Time_t, used to describe times at RTPS protocol.
 * @ingroup COMMON_MODULE
 */
class RTPS_DllAPI Time_t
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
     * @param time fastrtps::Time_t, aka. Duration_t.
     */
    Time_t(
            const eprosima::fastrtps::Time_t& time);

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
     * Sets fraction field and updates the nanoseconds.
     */
    void fraction(
            uint32_t frac);

    Duration_t to_duration_t() const;

    void from_duration_t(
            const Duration_t& duration);

    /**
     * Fills a Time_t struct with a representation of the current time.
     *
     * @param ret Reference to the structure to be filled in.
     */
    static void now(
            Time_t& ret);

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

    /**
     * Comparison assignment
     * @param other Second Time_t to compare
     * @return True if equal
     */
    inline bool operator ==(
            const Time_t& other) const
    {
        if (seconds() != other.seconds())
        {
            return false;
        }
        if (fraction() != other.fraction())
        {
            return false;
        }
        return true;
    }

    /**
     * Comparison assignment
     * @param other Second Time_t to compare
     * @return True if not equal
     */
    inline bool operator !=(
            const Time_t& other) const
    {
        return !(*this == other);
    }

    /**
     * Checks if a Time_t is less than other.
     * @param other Second Time_t to compare
     * @return True if the first Time_t is less than the second
     */
    inline bool operator <(
            const Time_t& other) const
    {
        if (seconds() < other.seconds())
        {
            return true;
        }
        else if (seconds() > other.seconds())
        {
            return false;
        }
        else
        {
            if (fraction() < other.fraction())
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
     * @param other Second Time_t to compare
     * @return True if the first Time_t is greater than the second
     */
    inline bool operator >(
            const Time_t& other) const
    {
        return other < *this;
    }

    /**
     * Checks if a Time_t is less or equal than other.
     * @param other Second Time_t to compare
     * @return True if the first Time_t is less or equal than the second
     */
    inline bool operator <=(
            const Time_t& other) const
    {
        return !(*this > other);
    }

    /**
     * Checks if a Time_t is greater or equal than other.
     * @param other Second Time_t to compare
     * @return True if the first Time_t is greater or equal than the second
     */
    inline bool operator >=(
            const Time_t& other) const
    {
        return !(*this < other);
    }

    /**
     * Adds two Time_t.
     * @param other Second Time_t to add
     * @return A new Time_t with the result.
     */
    inline Time_t operator +(
            const Time_t& other) const
    {
        Time_t result(seconds() + other.seconds(), fraction() + other.fraction());
        if (result.fraction() < fraction()) // Overflow is detected by any of them
        {
            ++result.seconds();
        }
        return result;
    }

    /**
     * Subtracts two Time_t.
     * @param other Second Time_t to subtract
     * @return A new Time_t with the result.
     */
    inline Time_t operator -(
            const Time_t& other) const
    {
        Time_t result(seconds() - other.seconds(), fraction() - other.fraction());
        if (result.fraction() > fraction()) // Overflow is detected by ta
        {
            --result.seconds();
        }
        return result;
    }

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

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
        int32_t nano = 0;
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            input >> sec;
            input >> point >> nano;
            // nano could not be bigger than 1 sec
            if ( point != '.' || nano > 1000000000 )
            {
                input.setstate(std::ios_base::failbit);
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

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

const Time_t c_RTPSTimeInfinite{0x7fffffff, 0xffffffff};
const Time_t c_RTPSTimeZero{0, 0};
const Time_t c_RTPSTimeInvalid{-1, 0xffffffff};

} // namespace rtps

} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_TIME_T_H_ */
