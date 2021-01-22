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
 * @file LoanableCollection.hpp
 */

#ifndef _FASTDDS_DDS_CORE_LOANABLECOLLECTION_HPP_
#define _FASTDDS_DDS_CORE_LOANABLECOLLECTION_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A collection of generic opaque pointers that can receive the buffer from outside (loan).
 *
 * This is an abstract class. See @ref LoanableSequence for details.
 */
class LoanableCollection
{
public:

    using size_type = int32_t;
    using element_type = void*;

    /**
     * Get the pointer to the elements buffer.
     *
     * The returned value may be nullptr if maximum() is 0.
     * Otherwise it is guaranteed that up to maximum() elements can be accessed.
     *
     * @return the pointer to the elements buffer.
     */
    const element_type* buffer() const
    {
        return elements_;
    }

    /**
     * Get the ownership flag.
     *
     * @return whether the collection has ownership of the buffer.
     */
    bool has_ownership() const
    {
        return has_ownership_;
    }

    /**
     * Get the maximum number of elements currently allocated.
     *
     * @return the maximum number of elements currently allocated.
     */
    size_type maximum() const
    {
        return maximum_;
    }

    /**
     * Get the number of elements currently accessible.
     *
     * @return the number of elements currently accessible.
     */
    size_type length() const
    {
        return length_;
    }

    /**
     * Set the number of elements currently accessible.
     *
     * This method tells the collection that a certain number of elements should be accessible.
     * If the new length is greater than the current @ref maximum() the collection should allocate
     * space for the new elements. If this is the case and the collection does not own the buffer
     * (i.e. @ref has_ownership() is false) then no allocation will be performed, the length will
     * remain unchanged, and false will be returned.
     *
     * @pre new_length >= 0
     *
     * @param [in] new_length New number of elements to be accessible.
     *
     * @return true if the new length was correctly set.
     *
     * @post length() == new_length
     * @post maximum() >= new_length
     */
    bool length(
            size_type new_length)
    {
        if (new_length < 0)
        {
            return false;
        }

        if (new_length <= maximum_)
        {
            length_ = new_length;
            return true;
        }

        if (!has_ownership_)
        {
            return false;
        }

        resize(new_length);
        length_ = new_length;
        return true;
    }

    /**
     * Loan a buffer to the collection.
     *
     * @param [in] buffer       pointer to the buffer to be loaned.
     * @param [in] new_maximum  number of allocated elements in buffer.
     * @param [in] new_length   number of accessible elements in buffer.
     *
     * @pre (has_ownership() == false) || (maximum() == 0)
     * @pre new_maximum > 0
     * @pre new_maximum >= new_length
     * @pre buffer != nullptr
     *
     * @return false if preconditions are not met.
     * @return true if operation succeeds.
     *
     * @post buffer() == buffer
     * @post has_ownership() == false
     * @post maximum() == new_maximum
     * @post length() == new_length
     */
    bool loan(
            element_type* buffer,
            size_type new_maximum,
            size_type new_length)
    {
        if (has_ownership_ && maximum_ > 0)
        {
            return false;
        }

        if ((nullptr == buffer) || (new_maximum < new_length) || (new_maximum < 1))
        {
            return false;
        }

        maximum_ = new_maximum;
        length_ = new_length;
        elements_ = buffer;
        has_ownership_ = false;
        return true;
    }

    /**
     * Remove the loan from the collection.
     *
     * @param [out] maximum  number of allocated elements on the returned buffer.
     * @param [out] length   number of accessible elements on the returned buffer.
     *
     * @pre has_ownership() == false
     *
     * @return nullptr if preconditions are not met.
     * @return pointer to the previously loaned buffer of elements.
     *
     * @post buffer() == nullptr
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == 0
     */
    element_type* unloan(
            size_type& maximum,
            size_type& length)
    {
        if (has_ownership_)
        {
            return nullptr;
        }

        element_type* ret_val = elements_;
        maximum = maximum_;
        length = length_;

        maximum_ = 0;
        length_ = 0;
        elements_ = nullptr;
        has_ownership_ = true;

        return ret_val;
    }

    /**
     * Remove the loan from the collection.
     *
     * @pre has_ownership() == false
     *
     * @return nullptr if preconditions are not met.
     * @return pointer to the previously loaned buffer of elements.
     *
     * @post buffer() == nullptr
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == 0
     */
    element_type* unloan()
    {
        size_type max, len;
        return unloan(max, len);
    }

protected:

    /**
     * Default constructor.
     *
     * Creates the loanable collection with no data.
     *
     * @post buffer() == nullptr
     * @post has_ownership() == true
     * @post length() == 0
     * @post maximum() == 0
     */
    LoanableCollection() = default;

    virtual void resize(
            size_type new_length) = 0;

    size_type maximum_ = 0;
    size_type length_ = 0;
    element_type* elements_ = nullptr;
    bool has_ownership_ = true;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_CORE_LOANABLECOLLECTION_HPP_
