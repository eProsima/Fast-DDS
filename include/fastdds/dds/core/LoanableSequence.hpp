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
 * @file LoanableSequence.hpp
 */

#ifndef FASTDDS_DDS_CORE__LOANABLESEQUENCE_HPP
#define FASTDDS_DDS_CORE__LOANABLESEQUENCE_HPP

#include <cassert>
#include <cstdint>
#include <vector>
#include <type_traits>

#include <fastdds/dds/core/LoanableTypedCollection.hpp>
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
 * Fast DDS, a '<tt>using FooSeq = LoanableSequence<Foo></tt>' is generated. The sequence offers a subset of the
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
template<typename T, typename _NonConstEnabler = std::true_type>
class LoanableSequence : public LoanableTypedCollection<T, _NonConstEnabler>
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
    LoanableSequence() = default;

    /**
     * Pre-allocation constructor.
     *
     * Creates the sequence with an initial number of allocated elements.
     * When the input parameter is less than or equal to 0, the behavior is equivalent to the default constructor.
     * Otherwise, the post-conditions below will apply.
     *
     * @param [in] max Number of elements to pre-allocate.
     *
     * @post buffer() != nullptr
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == max
     */
    LoanableSequence(
            size_type max)
    {
        if (max <= 0)
        {
            return;
        }

        resize(max);
    }

    /**
     * Deallocate this sequence's buffer.
     *
     * @pre has_ownership() == true. If this precondition is not met, no memory will be released and
     *      a warning will be logged.
     * @post maximum() == 0 and the underlying buffer is released.
     */
    ~LoanableSequence()
    {
        if (elements_ && !has_ownership_)
        {
            EPROSIMA_LOG_WARNING(SUBSCRIBER, "Sequence destroyed with active loan");
            return;
        }

        release();
    }

    /**
     * Construct a sequence with the contents of another sequence.
     *
     * This method performs a deep copy of the sequence received into this one.
     * Allocations will happen when other.length() > 0
     *
     * @param [in] other The sequence from where contents are to be copied.
     *
     * @post has_ownership() == true
     * @post maximum() == other.length()
     * @post length() == other.length()
     * @post buffer() != nullptr when other.length() > 0
     */
    LoanableSequence(
            const LoanableSequence& other)
    {
        *this = other;
    }

    /**
     * Copy the contents of another sequence into this one.
     *
     * This method performs a deep copy of the sequence received into this one.
     * If this sequence had a buffer loaned, it will behave as if @ref unloan has been called.
     * Allocations will happen when
     * (a) has_ownership() == false and other.length() > 0
     * (b) has_ownership() == true and other.length() > maximum()
     *
     * @param [in] other The sequence from where contents are to be copied.
     *
     * @post has_ownership() == true
     * @post maximum() >= other.length()
     * @post length() == other.length()
     * @post buffer() != nullptr when other.length() > 0
     */
    LoanableSequence& operator =(
            const LoanableSequence& other)
    {
        if (!has_ownership_)
        {
            release();
        }

        LoanableCollection::length(other.length());
        const element_type* other_buf = other.buffer();
        for (size_type n = 0; n < length_; ++n)
        {
            *static_cast<T*>(elements_[n]) = *static_cast<const T*>(other_buf[n]);
        }

        return *this;
    }

protected:

    using LoanableCollection::maximum_;
    using LoanableCollection::length_;
    using LoanableCollection::elements_;
    using LoanableCollection::has_ownership_;

private:

    void resize(
            size_type maximum) override
    {
        assert(has_ownership_);

        // Resize collection and get new pointer
        data_.reserve(maximum);
        data_.resize(maximum);
        elements_ = reinterpret_cast<element_type*>(data_.data());

        // Allocate individual elements
        while (maximum_ < maximum)
        {
            data_[maximum_++] = new T();
        }
    }

    void release()
    {
        if (has_ownership_ && elements_)
        {
            for (size_type n = 0; n < maximum_; ++n)
            {
                T* elem = data_[n];
                delete elem;
            }
            std::vector<T*>().swap(data_);
        }

        maximum_ = 0u;
        length_ = 0u;
        elements_ = nullptr;
        has_ownership_ = true;
    }

    std::vector<T*> data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

// Macro to easily declare a LoanableSequence for a data type
#define FASTDDS_SEQUENCE(FooSeq, Foo) using FooSeq = eprosima::fastdds::dds::LoanableSequence<Foo>
#define FASTDDS_CONST_SEQUENCE(FooSeq, Foo) using FooSeq = eprosima::fastdds::dds::LoanableSequence<Foo, \
                    std::false_type>

#endif // FASTDDS_DDS_CORE__LOANABLESEQUENCE_HPP
