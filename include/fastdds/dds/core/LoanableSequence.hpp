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

#ifndef _FASTDDS_DDS_CORE_LOANABLESEQUENCE_HPP_
#define _FASTDDS_DDS_CORE_LOANABLESEQUENCE_HPP_

#include <cassert>
#include <cstdint>
#include <vector>

#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

template<typename T>
class LoanableSequence : public LoanableCollection
{
public:

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
     * When the input parameter is 0, the behavior is equivalent to the default constructor.
     * Otherwise, the post-conditions below will apply.
     *
     * @param max Number of elements to pre-allocate.
     *
     * @post buffer() != nullptr
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == max
     */
    LoanableSequence(
            size_type max)
    {
        if (!max)
        {
            return;
        }

        resize(max);
    }

    ~LoanableSequence()
    {
        if (elements_ && !has_ownership_)
        {
            logWarning(SUBSCRIBER, "Sequence destroyed with active loan");
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
     * @param other The sequence from where contents are to be copied.
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
     * @param other The sequence from where contents are to be copied.
     *
     * @post has_ownership() == true
     * @post maximum() >= other.length()
     * @post length() == other.length()
     * @post buffer() != nullptr when other.length() > 0
     */
    LoanableSequence& operator=(
            const LoanableSequence& other)
    {
        if (!has_ownership_)
        {
            release();
        }

        length(other.length());
        const element_type* other_buf = other.buffer();
        for (size_type n = 0; n < length_; ++n)
        {
            *static_cast<T*>(elements_[n]) = *static_cast<const T*>(other_buf[n]);
        }

        return *this;
    }

    T& operator[](
            size_type index)
    {
        if (index >= length_)
        {
            throw std::out_of_range("");
        }

        return *static_cast<T*>(elements_[index]);
    }

    const T& operator[](
            size_type index) const
    {
        if (index >= length_)
        {
            throw std::out_of_range("");
        }

        return *static_cast<const T*>(elements_[index]);
    }

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
            for (size_t n = 0; n < maximum_; ++n)
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

#endif // _FASTDDS_DDS_CORE_LOANABLESEQUENCE_HPP_
