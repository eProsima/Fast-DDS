// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LoanableConstSequence.hpp
 */

#ifndef _FASTDDS_DDS_CORE_LOANABLECONSTSEQUENCE_HPP_
#define _FASTDDS_DDS_CORE_LOANABLECONSTSEQUENCE_HPP_

#include <cassert>
#include <cstdint>
#include <vector>

#include <fastdds/dds/core/LoanableTypedConstCollection.hpp>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A type-safe, ordered collection of elements that can receive the buffer from outside (loan).
 *
 * For users who define data types in OMG IDL, this type corresponds to the IDL express sequence<T>.
 *
 * For any user-data type Foo that an application defines for the purpose of data-distribution with
 * Fast DDS, a '<tt>using FooConstSeq = LoanableConstSequence<Foo></tt>' is generated. The sequence offers a subset of the
 * methods defined by the standard OMG IDL to C++ mapping for sequences.
 * We refer to an IDL '<tt>sequence<Foo></tt>' as \c FooSeq.
 *
 * The state of a sequence is described by the properties 'maximum', 'length' and 'has_ownership'.
 * @li The 'maximum' represents the size of the underlying buffer; this is the maximum number of elements
 *     it can possibly hold. It is returned by the maximum() operation.
 * @li The 'length' represents the actual number of elements it currently holds. It is returned by the
 *     length() operation.
 * @li The 'has_ownership' flag represents whether the sequence owns the underlying buffer. It is returned
 *     by the has_ownership() operation. If the sequence does not own the underlying buffer, the underlying
 *     buffer is loaned from somewhere else. This flag influences the lifecycle of the sequence and what
 *     operations are allowed on it. The general guidelines are provided below and more details are described
 *     in detail as pre-conditions and post-conditions of each of the sequence's operations:
 *
 *     @li If has_ownership == true, the sequence has ownership on the buffer. It is then responsible for
 *         destroying the buffer when the sequence is destroyed.
 *     @li If has_ownership == false, the sequence does not have ownership on the buffer. This implies that
 *         the sequence is loaning the buffer. The sequence should not be destroyed until the loan is returned.
 *     @li A sequence with a zero maximum always has has_ownership == true
 */
template<typename T>
class LoanableConstSequence : public LoanableTypedConstCollection<T>
{
public:

    using size_type = LoanableCollection::size_type;
    using element_type = LoanableCollection::element_type;

    /**
     * Default constructor.
     *
     * Creates the sequence with no data.
     *
     * @post buffer() == nullptr
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == 0
     */
    LoanableConstSequence() = default;

    /**
     * Deallocate this sequence's buffer.
     *
     * @pre has_ownership() == true. If this precondition is not met, no memory will be released and
     *      a warning will be logged.
     * @post maximum() == 0 and the underlying buffer is released.
     */
    ~LoanableConstSequence()
    {
        if (elements_ && !has_ownership_)
        {
            logWarning(SUBSCRIBER, "Sequence destroyed with active loan");
            return;
        }
    }

    // Non-copyable
    LoanableConstSequence(
            const LoanableConstSequence& other) = delete;
    LoanableConstSequence& operator =(
            const LoanableConstSequence& other) = delete;

    // Non-moveable
    LoanableConstSequence(
            LoanableConstSequence&& other) = delete;
    LoanableConstSequence& operator =(
            LoanableConstSequence&& other) = delete;

protected:

    using LoanableCollection::maximum_;
    using LoanableCollection::length_;
    using LoanableCollection::elements_;
    using LoanableCollection::has_ownership_;

private:

    void resize(
            size_type maximum) override
    {
        static_cast<void>(maximum);
        throw std::bad_alloc();
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_CORE_LOANABLESEQUENCE_HPP_
