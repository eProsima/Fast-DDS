// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file StackAllocatedSequence.hpp
 */

#ifndef _FASTDDS_DDS_CORE_STACKALLOCATEDSEQUENCE_HPP_
#define _FASTDDS_DDS_CORE_STACKALLOCATEDSEQUENCE_HPP_

#include <cassert>
#include <cstdint>
#include <array>

#include <fastdds/dds/core/LoanableArray.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A type-safe, ordered collection of elements allocated on the stack.
 */
template<typename T, LoanableCollection::size_type num_items>
struct StackAllocatedSequence : public LoanableCollection
{
    StackAllocatedSequence()
    {
        has_ownership_ = true;
        maximum_ = num_items;
        length_ = 0;
        elements_ = data_.buffer_for_loans();
    }

    ~StackAllocatedSequence() = default;

    // Non-copyable
    StackAllocatedSequence(const StackAllocatedSequence&) = delete;
    StackAllocatedSequence& operator = (const StackAllocatedSequence&) = delete;

    // Non-moveable
    StackAllocatedSequence(StackAllocatedSequence&&) = delete;
    StackAllocatedSequence& operator = (StackAllocatedSequence&&) = delete;

    /**
     * Set the n-th element of the sequence.
     *
     * This is the operator that is invoked when the application indexes into a @em non-const sequence:
     * @code{.cpp}
     * element = sequence[n];
     * sequence[n] = element;
     * @endcode
     *
     * Note that a @em reference to the n-th element is returned (and not a copy)
     *
     * @param [in] n index of element to access, must be >= 0 and less than length().
     *
     * @return a reference to the n-th element
     */
    T& operator [](
            size_type n)
    {
        if (n >= length_)
        {
            throw std::out_of_range("");
        }

        return *static_cast<T*>(elements_[n]);
    }

    /**
     * Get the n-th element of the sequence.
     *
     * This is the operator that is invoked when the application indexes into a @em const sequence:
     * @code{.cpp}
     * element = sequence[n];
     * @endcode
     *
     * Note that a @em reference to the n-th element is returned (and not a copy)
     *
     * @param [in] n index of element to access, must be >= 0 and less than length().
     *
     * @return a const reference to the n-th element
     */
    const T& operator [](
            size_type n) const
    {
        if (n >= length_)
        {
            throw std::out_of_range("");
        }

        return *static_cast<const T*>(elements_[n]);
    }

protected:

    void resize(
            LoanableCollection::size_type new_length) override
    {
        // This kind of collection cannot grow above its stack-allocated size
        static_cast<void>(new_length);
        assert(new_length <= num_items);
    }

private:

    LoanableArray<T, num_items> data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_CORE_STACKALLOCATEDSEQUENCE_HPP_
