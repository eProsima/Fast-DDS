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

#ifndef FASTDDS_DDS_CORE__STACKALLOCATEDSEQUENCE_HPP
#define FASTDDS_DDS_CORE__STACKALLOCATEDSEQUENCE_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

#include <fastdds/dds/core/LoanableArray.hpp>
#include <fastdds/dds/core/LoanableTypedCollection.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A type-safe, ordered collection of elements allocated on the stack.
 */
template<typename T, LoanableCollection::size_type num_items>
struct StackAllocatedSequence : public LoanableTypedCollection<T>
{
    using size_type = LoanableCollection::size_type;
    using element_type = LoanableCollection::element_type;

    StackAllocatedSequence()
    {
        has_ownership_ = true;
        maximum_ = num_items;
        length_ = 0;
        elements_ = data_.buffer_for_loans();
    }

    ~StackAllocatedSequence() = default;

    // Non-copyable
    StackAllocatedSequence(
            const StackAllocatedSequence&) = delete;
    StackAllocatedSequence& operator = (
            const StackAllocatedSequence&) = delete;

    // Non-moveable
    StackAllocatedSequence(
            StackAllocatedSequence&&) = delete;
    StackAllocatedSequence& operator = (
            StackAllocatedSequence&&) = delete;

protected:

    using LoanableCollection::maximum_;
    using LoanableCollection::length_;
    using LoanableCollection::elements_;
    using LoanableCollection::has_ownership_;

    void resize(
            LoanableCollection::size_type new_length) override
    {
        // This kind of collection cannot grow above its stack-allocated size
        if (new_length > num_items)
        {
            throw std::bad_alloc();
        }
    }

private:

    LoanableArray<T, num_items> data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE__STACKALLOCATEDSEQUENCE_HPP
