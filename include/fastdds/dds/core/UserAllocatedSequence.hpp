// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file UserAllocatedSequence.hpp
 */

#ifndef FASTDDS_DDS_CORE__USERALLOCATEDSEQUENCE_HPP
#define FASTDDS_DDS_CORE__USERALLOCATEDSEQUENCE_HPP

#include <cassert>
#include <cstdint>
#include <stdexcept>

#include <fastdds/dds/core/LoanableCollection.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A collection of generic opaque pointers allocated by the user.
 *
 * This kind of collection would always return @c true for @c has_ownership(),
 * and thus would not be able to receive loans.
 * It would also have an inmutable @c maximum(), so it would not allow @c length() to grow beyond the maximum
 * value indicated on construction.
 */
struct UserAllocatedSequence : public LoanableCollection
{
    using size_type = LoanableCollection::size_type;
    using element_type = LoanableCollection::element_type;

    /**
     * Construct a UserAllocatedSequence.
     *
     * @param [in] items      Pointer to the beginning of an array of @c num_items opaque pointers.
     * @param [in] num_items  Number of opaque pointers in @c items.
     *
     * @post buffer() == items
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == num_items
     */
    UserAllocatedSequence(
            element_type* items,
            size_type num_items)
    {
        has_ownership_ = true;
        maximum_ = num_items;
        length_ = 0;
        elements_ = items;
    }

    ~UserAllocatedSequence() = default;

    // Non-copyable
    UserAllocatedSequence(
            const UserAllocatedSequence&) = delete;
    UserAllocatedSequence& operator = (
            const UserAllocatedSequence&) = delete;

    // Non-moveable
    UserAllocatedSequence(
            UserAllocatedSequence&&) = delete;
    UserAllocatedSequence& operator = (
            UserAllocatedSequence&&) = delete;

protected:

    using LoanableCollection::maximum_;
    using LoanableCollection::length_;
    using LoanableCollection::elements_;
    using LoanableCollection::has_ownership_;

    void resize(
            size_type new_length) override
    {
        // This kind of collection cannot grow above its stack-allocated size
        if (new_length > maximum_)
        {
            throw std::bad_alloc();
        }
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE__USERALLOCATEDSEQUENCE_HPP
