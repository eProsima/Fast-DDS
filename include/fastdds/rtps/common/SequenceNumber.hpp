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
 * @file SequenceNumber.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__SEQUENCENUMBER_HPP
#define FASTDDS_RTPS_COMMON__SEQUENCENUMBER_HPP

#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/utils/fixed_size_bitmap.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

//!@brief Structure SequenceNumber_t, different for each change in the same writer.
//!@ingroup COMMON_MODULE
struct FASTDDS_EXPORTED_API SequenceNumber_t
{
    //!
    int32_t high = 0;
    //!
    uint32_t low = 0;

    //! Default constructor
    SequenceNumber_t() noexcept
    {
        high = 0;
        low = 0;
    }

    /*!
     * @param hi
     * @param lo
     */
    SequenceNumber_t(
            int32_t hi,
            uint32_t lo) noexcept
        : high(hi)
        , low(lo)
    {
    }

    /*!
     * @param u
     */
    explicit SequenceNumber_t(
            uint64_t u) noexcept
        : high(static_cast<int32_t>(u >> 32u))
        , low(static_cast<uint32_t>(u))
    {
    }

    /*! Convert the number to 64 bit.
     * @return 64 bit representation of the SequenceNumber
     */
    uint64_t to64long() const noexcept
    {
        return (static_cast<uint64_t>(high) << 32u) + low;
    }

    //! Increase SequenceNumber in 1.
    SequenceNumber_t& operator ++() noexcept
    {
        ++low;
        if (low == 0)
        {
            assert(std::numeric_limits<decltype(high)>::max() > high);
            ++high;
        }

        return *this;
    }

    SequenceNumber_t operator ++(
            int) noexcept
    {
        SequenceNumber_t result(*this);
        ++(*this);
        return result;
    }

    /**
     * Increase SequenceNumber.
     * @param inc Number to add to the SequenceNumber
     */
    SequenceNumber_t& operator +=(
            int inc) noexcept
    {
        assert(inc >= 0);
        uint32_t aux_low = low;
        low += static_cast<uint32_t>(inc);

        if (low < aux_low)
        {
            // Being the type of the parameter an 'int', the increment of 'high' will be as much as 1.
            assert(std::numeric_limits<decltype(high)>::max() > high);
            ++high;
        }

        return *this;
    }

    static SequenceNumber_t unknown() noexcept
    {
        return {-1, 0};
    }

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Compares two SequenceNumber_t.
 * @param sn1 First SequenceNumber_t to compare
 * @param sn2 Second SequenceNumber_t to compare
 * @return True if equal
 */
inline bool operator ==(
        const SequenceNumber_t& sn1,
        const SequenceNumber_t& sn2) noexcept
{
    return (sn1.low == sn2.low) && (sn1.high == sn2.high);
}

/**
 * Compares two SequenceNumber_t.
 * @param sn1 First SequenceNumber_t to compare
 * @param sn2 Second SequenceNumber_t to compare
 * @return True if not equal
 */
inline bool operator !=(
        const SequenceNumber_t& sn1,
        const SequenceNumber_t& sn2) noexcept
{
    return (sn1.low != sn2.low) || (sn1.high != sn2.high);
}

/**
 * Checks if a SequenceNumber_t is greater than other.
 * @param seq1 First SequenceNumber_t to compare
 * @param seq2 Second SequenceNumber_t to compare
 * @return True if the first SequenceNumber_t is greater than the second
 */
inline bool operator >(
        const SequenceNumber_t& seq1,
        const SequenceNumber_t& seq2) noexcept
{
    if (seq1.high == seq2.high)
    {
        return seq1.low > seq2.low;
    }

    return seq1.high > seq2.high;
}

/**
 * Checks if a SequenceNumber_t is less than other.
 * @param seq1 First SequenceNumber_t to compare
 * @param seq2 Second SequenceNumber_t to compare
 * @return True if the first SequenceNumber_t is less than the second
 */
inline bool operator <(
        const SequenceNumber_t& seq1,
        const SequenceNumber_t& seq2) noexcept
{
    if (seq1.high == seq2.high)
    {
        return seq1.low < seq2.low;
    }

    return seq1.high < seq2.high;
}

/**
 * Checks if a SequenceNumber_t is greater or equal than other.
 * @param seq1 First SequenceNumber_t to compare
 * @param seq2 Second SequenceNumber_t to compare
 * @return True if the first SequenceNumber_t is greater or equal than the second
 */
inline bool operator >=(
        const SequenceNumber_t& seq1,
        const SequenceNumber_t& seq2) noexcept
{
    if (seq1.high == seq2.high)
    {
        return seq1.low >= seq2.low;
    }

    return seq1.high > seq2.high;
}

/**
 * Checks if a SequenceNumber_t is less or equal than other.
 * @param seq1 First SequenceNumber_t to compare
 * @param seq2 Second SequenceNumber_t to compare
 * @return True if the first SequenceNumber_t is less or equal than the second
 */
inline bool operator <=(
        const SequenceNumber_t& seq1,
        const SequenceNumber_t& seq2) noexcept
{
    if (seq1.high == seq2.high)
    {
        return seq1.low <= seq2.low;
    }

    return seq1.high < seq2.high;
}

/**
 * Subtract one uint32_t from a SequenceNumber_t
 * @param seq Base SequenceNumber_t
 * @param inc uint32_t to subtract
 * @return Result of the subtraction
 */
inline SequenceNumber_t operator -(
        const SequenceNumber_t& seq,
        const uint32_t inc) noexcept
{
    SequenceNumber_t res(seq.high, seq.low - inc);

    if (inc > seq.low)
    {
        // Being the type of the parameter an 'uint32_t', the decrement of 'high' will be as much as 1.
        assert(0 < res.high);
        --res.high;
    }

    return res;
}

/**
 * Add one uint32_t to a SequenceNumber_t
 * @param [in] seq Base sequence number
 * @param inc value to add to the base
 * @return Result of the addition
 */
inline SequenceNumber_t operator +(
        const SequenceNumber_t& seq,
        const uint32_t inc) noexcept
{
    SequenceNumber_t res(seq.high, seq.low + inc);

    if (res.low < seq.low)
    {
        // Being the type of the parameter an 'uint32_t', the increment of 'high' will be as much as 1.
        assert(std::numeric_limits<decltype(res.high)>::max() > res.high);
        ++res.high;
    }

    return res;
}

/**
 * Subtract one SequenceNumber_t to another
 * @param minuend Minuend. Has to be greater than or equal to subtrahend.
 * @param subtrahend Subtrahend.
 * @return Result of the subtraction
 */
inline SequenceNumber_t operator -(
        const SequenceNumber_t& minuend,
        const SequenceNumber_t& subtrahend) noexcept
{
    assert(minuend >= subtrahend);
    SequenceNumber_t res(minuend.high - subtrahend.high, minuend.low - subtrahend.low);

    if (minuend.low < subtrahend.low)
    {
        assert(0 < res.high);
        --res.high;
    }

    return res;
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

const SequenceNumber_t c_SequenceNumber_Unknown{-1, 0};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Sorts two instances of SequenceNumber_t
 * @param s1 First SequenceNumber_t to compare
 * @param s2 First SequenceNumber_t to compare
 * @return True if s1 is less than s2
 */
inline bool sort_seqNum(
        const SequenceNumber_t& s1,
        const SequenceNumber_t& s2) noexcept
{
    return s1 < s2;
}

/**
 *
 * @param output
 * @param seqNum
 * @return
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const SequenceNumber_t& seqNum)
{
    return output << seqNum.to64long();
}

inline std::ostream& operator <<(
        std::ostream& output,
        const std::vector<SequenceNumber_t>& seqNumSet)
{
    for (const SequenceNumber_t& sn : seqNumSet)
    {
        output << sn << " ";
    }

    return output;
}

/*!
 * @brief Defines the STL hash function for type SequenceNumber_t.
 */
struct SequenceNumberHash
{
    std::size_t operator ()(
            const SequenceNumber_t& sequence_number) const noexcept
    {
        return static_cast<std::size_t>(sequence_number.to64long());
    }

};

struct SequenceNumberDiff
{
    uint32_t operator ()(
            const SequenceNumber_t& a,
            const SequenceNumber_t& b) const noexcept
    {
        SequenceNumber_t diff = a - b;
        return diff.low;
    }

};

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

//! Structure SequenceNumberSet_t, contains a group of sequencenumbers.
using SequenceNumberSet_t = BitmapRange<SequenceNumber_t, SequenceNumberDiff, 256>;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Prints a sequence Number set
 * @param output Output Stream
 * @param sns SequenceNumber set
 * @return OStream.
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const SequenceNumberSet_t& sns)
{
    output << sns.base().to64long() << ":";
    sns.for_each([&output](
                SequenceNumber_t it)
            {
                output << it.to64long() << "-";
            });

    return output;
}

/**
 *
 * @param input
 * @param seqNum
 * @return
 */
inline std::istream& operator >>(
        std::istream& input,
        SequenceNumber_t& seqNum)
{
    uint64_t aux;

    if (input >> aux)
    {
        seqNum = SequenceNumber_t(aux);
    }

    return input;
}

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__SEQUENCENUMBER_HPP
