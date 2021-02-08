// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file fixed_size_bitmap.hpp
 *
 */

#ifndef FASTRTPS_UTILS_FIXED_SIZE_BITMAP_HPP_
#define FASTRTPS_UTILS_FIXED_SIZE_BITMAP_HPP_

#include <array>
#include <cstdint>
#include <string.h>
#include <limits>

#if _MSC_VER
#include <intrin.h>

#if defined(max)
#pragma push_macro("max")
#undef max
#define FASTDDS_RESTORE_MAX
#endif // defined(max)

#if defined(min)
#pragma push_macro("min")
#undef min
#define FASTDDS_RESTORE_MIN
#endif // defined(min)

#endif // if _MSC_VER

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
namespace eprosima {
namespace fastrtps {

using std::uint32_t;

template <class T>
struct DiffFunction
{
    constexpr auto operator () (
            T a,
            T b) const
    -> decltype(a - b)
    {
        return a - b;
    }

};

/**
 * Template class to hold a range of items using a custom bitmap.
 * @tparam T      Type of the elements in the range.
 *                Should have `>=` operator and `T + uint32_t` returning T.
 * @tparam Diff   Functor calculating the difference of two T.
 *                The result should be assignable to a uint32_t.
 * @tparam NBITS  Size in bits of the bitmap.
 *                Range of items will be [base, base + NBITS - 1].
 * @ingroup UTILITIES_MODULE
 */
template<class T, class Diff = DiffFunction<T>, uint32_t NBITS = 256>
class BitmapRange
{
    #define NITEMS ((NBITS + 31u) / 32u)

public:

    // Alias to improve readability.
    using bitmap_type = std::array<uint32_t, NITEMS>;

    /**
     * Default constructor.
     * Constructs an empty range with default base.
     */
    BitmapRange() noexcept
        : base_()
        , range_max_(base_ + (NBITS - 1))
        , bitmap_()
        , num_bits_(0u)
    {
    }

    /**
     * Base-specific constructor.
     * Constructs an empty range with specified base.
     *
     * @param base   Specific base value for the created range.
     */
    explicit BitmapRange(
            T base) noexcept
        : base_(base)
        , range_max_(base + (NBITS - 1))
        , bitmap_()
        , num_bits_(0u)
    {
    }

    // We don't need to define copy/move constructors/assignment operators as the default ones would be enough

    /**
     * Get base of the range.
     * @return a copy of the range base.
     */
    T base() const noexcept
    {
        return base_;
    }

    /**
     * Set a new base for the range.
     * This method resets the range and sets a new value for its base.
     *
     * @param base   New base value to set.
     */
    void base(
            T base) noexcept
    {
        base_ = base;
        range_max_ = base_ + (NBITS - 1);
        num_bits_ = 0;
        bitmap_.fill(0u);
    }

    /**
     * Set a new base for the range, keeping old values where possible.
     * This method implements a sliding window mechanism for changing the base of the range.
     *
     * @param base   New base value to set.
     */
    void base_update(
            T base) noexcept
    {
        // Do nothing if base is not changing
        if (base == base_)
        {
            return;
        }

        Diff d_func;
        if (base > base_)
        {
            // Current content should move left
            uint32_t n_bits = d_func(base, base_);
            shift_map_left(n_bits);
        }
        else
        {
            // Current content should move right
            uint32_t n_bits = d_func(base_, base);
            shift_map_right(n_bits);
        }

        // Update base and range
        base_ = base;
        range_max_ = base_ + (NBITS - 1);
    }

    /**
     * Returns whether the range is empty (i.e. has all bits unset).
     *
     * @return true if the range is empty, false otherwise.
     */
    bool empty() const noexcept
    {
        return num_bits_ == 0u;
    }

    /**
     * Returns the highest value set in the range.
     *
     * @return the highest value set in the range. If the range is empty, the result is undetermined.
     */
    T max() const noexcept
    {
        return base_ + (num_bits_ - 1);
    }

    /**
     * Returns the lowest value set in the range.
     *
     * @return the lowest value set in the range. If the range is empty, the result is undetermined.
     */
    T min() const noexcept
    {
        // Traverse through the significant items on the bitmap
        T item = base_;
        uint32_t n_longs = (num_bits_ + 31u) / 32u;
        for (uint32_t i = 0; i < n_longs; i++)
        {
            // Check if item has at least one bit set
            uint32_t bits = bitmap_[i];
            if (bits)
            {
                // We use an intrinsic to find the index of the highest bit set.
                // Most modern CPUs have an instruction to count the leading zeroes of a word.
                // The number of leading zeroes will give us the index we need.
#if _MSC_VER
                unsigned long bit;
                _BitScanReverse(&bit, bits);
                uint32_t offset = 31u ^ bit;
#else
                uint32_t offset = static_cast<uint32_t>(__builtin_clz(bits));
#endif // if _MSC_VER

                // Found first bit set in bitmap
                return item + offset;
            }

            // There are 32 items on each bitmap item.
            item = item + 32u;
        }

        return base_;
    }

    /**
     * Checks if an element is present in the bitmap.
     *
     * @param item   Value to be checked.
     *
     * @return true if the item is present in the bitmap, false otherwise.
     */
    bool is_set(
            const T& item) const noexcept
    {
        // Check item is inside the allowed range.
        if ((item >= base_) && (range_max_ >= item))
        {
            // Calc distance from base to item, and check the corresponding bit.
            Diff d_func;
            uint32_t diff = d_func(item, base_);
            if (diff < num_bits_)
            {
                uint32_t pos = diff >> 5;
                diff &= 31u;
                return (bitmap_[pos] & (1u << (31u - diff))) != 0;
            }
        }

        return false;
    }

    /**
     * Adds an element to the range.
     * Adds an element to the bitmap if it is in the allowed range.
     *
     * @param item   Value to be added.
     *
     * @return true if the item has been added (i.e. is in the allowed range), false otherwise.
     */
    bool add(
            const T& item) noexcept
    {
        // Check item is inside the allowed range.
        if ((item >= base_) && (range_max_ >= item))
        {
            // Calc distance from base to item, and set the corresponding bit.
            Diff d_func;
            uint32_t diff = d_func(item, base_);
            num_bits_ = std::max(diff + 1, num_bits_);
            uint32_t pos = diff >> 5;
            diff &= 31u;
            bitmap_[pos] |= (1u << (31u - diff));
            return true;
        }

        return false;
    }

    /**
     * Adds a range of elements to the range.
     *
     * Add all elements in [from, to) to the range.
     * Equivalent to for(T i = from; i < to; i++) add(i);
     *
     * @param from   Starting value of the range to add.
     * @param to     Ending value of the range to add.
     */
    void add_range(
            const T& from,
            const T& to)
    {
        constexpr uint32_t full_mask = std::numeric_limits<uint32_t>::max();

        // Adapt incoming range to range limits
        T min = (base_ >= from) ? base_ : from;
        T max = (to >= base_ + NBITS) ? base_ + NBITS : to;

        // Check precondition. Max should be explicitly above min.
        if (min >= max)
        {
            return;
        }

        // Calc offset (distance from base) and num_bits (bits to be set)
        Diff d_func;
        uint32_t offset = d_func(min, base_);   // Bit position from base
        uint32_t n_bits = d_func(max, min);     // Number of bits pending

        num_bits_ = std::max(num_bits_, offset + n_bits);

        uint32_t pos = offset >> 5;             // Item position
        offset &= 31u;                          // Bit position inside item
        uint32_t mask = full_mask;              // Mask with all bits set
        mask >>= offset;                        // Remove first 'offset' bits from mask
        uint32_t bits_in_mask = 32u - offset;   // Take note of number of set bits in mask

        // This loop enters whenever the whole mask should be added
        while (n_bits >= bits_in_mask)
        {
            bitmap_[pos] |= mask;               // Set whole mask of bits
            pos++;                              // Go to next position in the array
            n_bits -= bits_in_mask;             // Decrease number of pending bits
            mask = full_mask;                   // Mask with all bits set
            bits_in_mask = 32u;                 // All bits set in mask (32)
        }

        // This condition will be true if the last bits of the mask should not be used
        if (n_bits > 0)
        {
            bitmap_[pos] |= mask & (full_mask << (bits_in_mask - n_bits));
        }
    }

    /**
     * Removes an element from the range.
     * Removes an element from the bitmap.
     *
     * @param item   Value to be removed.
     */
    void remove(
            const T& item) noexcept
    {
        // Check item is inside the allowed range.
        T max_value = max();
        if ((item >= base_) && (max_value >= item))
        {
            // Calc distance from base to item, and set the corresponding bit.
            Diff d_func;
            uint32_t diff = d_func(item, base_);
            uint32_t pos = diff >> 5;
            diff &= 31u;
            bitmap_[pos] &= ~(1u << (31u - diff));

            if (item == max_value)
            {
                calc_maximum_bit_set(pos + 1, 0);
            }
        }
    }

    /**
     * Gets the current value of the bitmap.
     * This method is designed to be used when performing serialization of a bitmap range.
     *
     * @param num_bits         Upon return, it will contain the number of significant bits in the bitmap.
     * @param bitmap           Upon return, it will contain the current value of the bitmap.
     * @param num_longs_used   Upon return, it will contain the number of valid elements on the returned bitmap.
     */
    void bitmap_get(
            uint32_t& num_bits,
            bitmap_type& bitmap,
            uint32_t& num_longs_used) const noexcept
    {
        num_bits = num_bits_;
        num_longs_used = (num_bits_ + 31u) / 32u;
        bitmap = bitmap_;
    }

    /**
     * Sets the current value of the bitmap.
     * This method is designed to be used when performing deserialization of a bitmap range.
     *
     * @param num_bits   Number of significant bits in the input bitmap.
     * @param bitmap     Points to the beginning of a uint32_t array holding the input bitmap.
     */
    void bitmap_set(
            uint32_t num_bits,
            const uint32_t* bitmap) noexcept
    {
        num_bits_ = std::min(num_bits, NBITS);
        uint32_t num_items = ((num_bits_ + 31u) / 32u);
        uint32_t num_bytes = num_items * static_cast<uint32_t>(sizeof(uint32_t));
        bitmap_.fill(0u);
        memcpy(bitmap_.data(), bitmap, num_bytes);
        if (0 < num_bits)
        {
            bitmap_[num_items - 1] &= ~(std::numeric_limits<uint32_t>::max() >> (num_bits & 31u));
        }
        calc_maximum_bit_set(num_items, 0);
    }

    /**
     * Apply a function on every item on the range.
     *
     * @param f   Function to apply on each item.
     */
    template<class UnaryFunc>
    void for_each(
            UnaryFunc f) const
    {
        T item = base_;

        // Traverse through the significant items on the bitmap
        uint32_t n_longs = (num_bits_ + 31u) / 32u;
        for (uint32_t i = 0; i < n_longs; i++)
        {
            // Traverse through the bits set on the item, msb first.
            // Loop will stop when there are no bits set.
            uint32_t bits = bitmap_[i];
            while (bits)
            {
                // We use an intrinsic to find the index of the highest bit set.
                // Most modern CPUs have an instruction to count the leading zeroes of a word.
                // The number of leading zeroes will give us the index we need.
#if _MSC_VER
                unsigned long bit;
                _BitScanReverse(&bit, bits);
                uint32_t offset = 31u ^ bit;
#else
                uint32_t offset = static_cast<uint32_t>(__builtin_clz(bits));
                uint32_t bit = 31u ^ offset;
#endif // if _MSC_VER

                // Call the function for the corresponding item
                f(item + offset);

                // Clear the most significant bit
                bits &= ~(1u << bit);
            }

            // There are 32 items on each bitmap item.
            item = item + 32u;
        }
    }

protected:

    T base_;               ///< Holds base value of the range.
    T range_max_;          ///< Holds maximum allowed value of the range.
    bitmap_type bitmap_;   ///< Holds the bitmap values.
    uint32_t num_bits_;    ///< Holds the highest bit set in the bitmap.

private:

    void shift_map_left(
            uint32_t n_bits)
    {
        if (n_bits >= num_bits_)
        {
            // Shifting more than most significant. Clear whole bitmap.
            num_bits_ = 0;
            bitmap_.fill(0u);
        }
        else
        {
            // Significant bit will move left by n_bits
            num_bits_ -= n_bits;

            // Div and mod by 32
            uint32_t n_items = n_bits >> 5;
            n_bits &= 31u;
            if (n_bits == 0)
            {
                // Shifting a multiple of 32 bits, just move the bitmap integers
                std::copy(bitmap_.begin() + n_items, bitmap_.end(), bitmap_.begin());
                std::fill_n(bitmap_.rbegin(), n_items, 0);
            }
            else
            {
                // Example. Shifting 44 bits. Should shift one complete word and 12 bits.
                // Need to iterate forward and take 12 bits from next word (shifting it 20 bits).
                // aaaaaaaa bbbbbbbb cccccccc dddddddd
                // bbbbbccc bbbbbbbb cccccccc dddddddd
                // bbbbbccc cccccddd ddddd000 dddddddd
                // bbbbbccc cccccddd ddddd000 00000000
                uint32_t overflow_bits = 32u - n_bits;
                size_t last_index = NITEMS - 1u;
                for (size_t i = 0, n = n_items; n < last_index; i++, n++)
                {
                    bitmap_[i] = (bitmap_[n] << n_bits) | (bitmap_[n + 1] >> overflow_bits);
                }
                // Last one does not have next word
                bitmap_[last_index - n_items] = bitmap_[last_index] << n_bits;
                // Last n_items will become 0
                std::fill_n(bitmap_.rbegin(), n_items, 0);
            }
        }
    }

    void shift_map_right(
            uint32_t n_bits)
    {
        if (n_bits >= NBITS)
        {
            // Shifting more than total bitmap size. Clear whole bitmap.
            num_bits_ = 0;
            bitmap_.fill(0u);
        }
        else
        {
            // Detect if highest bit will be dropped and take note, as we will need
            // to find new maximum bit in that case
            uint32_t new_num_bits = num_bits_ + n_bits;
            bool find_new_max = new_num_bits > NBITS;

            // Div and mod by 32
            uint32_t n_items = n_bits >> 5;
            n_bits &= 31u;
            if (n_bits == 0)
            {
                // Shifting a multiple of 32 bits, just move the bitmap integers
                std::copy(bitmap_.rbegin() + n_items, bitmap_.rend(), bitmap_.rbegin());
                std::fill_n(bitmap_.begin(), n_items, 0);
            }
            else
            {
                // Example. Shifting 44 bits. Should shift one complete word and 12 bits.
                // Need to iterate backwards and take 12 bits from previous word (shifting it 20 bits).
                // aaaaaaaa bbbbbbbb cccccccc dddddddd
                // aaaaaaaa bbbbbbbb cccccccc bbbccccc
                // aaaaaaaa bbbbbbbb aaabbbbb bbbccccc
                // aaaaaaaa 000aaaaa aaabbbbb bbbccccc
                // 00000000 000aaaaa aaabbbbb bbbccccc
                uint32_t overflow_bits = 32u - n_bits;
                size_t last_index = NITEMS - 1u;
                for (size_t i = last_index, n = last_index - n_items; n > 0; i--, n--)
                {
                    bitmap_[i] = (bitmap_[n] >> n_bits) | (bitmap_[n - 1] << overflow_bits);
                }
                // First item does not have previous word
                bitmap_[n_items] = bitmap_[0] >> n_bits;
                // First n_items will become 0
                std::fill_n(bitmap_.begin(), n_items, 0);
            }

            num_bits_ = new_num_bits;
            if (find_new_max)
            {
                calc_maximum_bit_set(NITEMS, n_items);
            }
        }
    }

    void calc_maximum_bit_set(
            uint32_t starting_index,
            uint32_t min_index)
    {
        num_bits_ = 0;
        for (uint32_t i = starting_index; i > min_index;)
        {
            --i;
            uint32_t bits = bitmap_[i];
            if (bits != 0)
            {
                bits = (bits & ~(bits - 1));
#if _MSC_VER
                unsigned long bit;
                _BitScanReverse(&bit, bits);
                uint32_t offset = (31u ^ bit) + 1;
#else
                uint32_t offset = static_cast<uint32_t>(__builtin_clz(bits)) + 1u;
#endif // if _MSC_VER
                num_bits_ = (i << 5u) + offset;
                break;
            }
        }
    }

};

}   // namespace fastrtps
}   // namespace eprosima

#endif   // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#if _MSC_VER

#if defined(FASTDDS_RESTORE_MIN)
#pragma pop_macro("min")
#undef FASTDDS_RESTORE_MIN
#endif // defined(FASTDDS_RESTORE_MIN)

#if defined(FASTDDS_RESTORE_MAX)
#pragma pop_macro("max")
#undef FASTDDS_RESTORE_MAX
#endif // defined(FASTDDS_RESTORE_MAX)

#endif // if _MSC_VER

#endif   // FASTRTPS_UTILS_FIXED_SIZE_BITMAP_HPP_
