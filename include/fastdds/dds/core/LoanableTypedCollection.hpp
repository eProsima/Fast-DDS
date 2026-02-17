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
 * @file LoanableTypedCollection.hpp
 */

#ifndef FASTDDS_DDS_CORE__LOANABLETYPEDCOLLECTION_HPP
#define FASTDDS_DDS_CORE__LOANABLETYPEDCOLLECTION_HPP

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include <fastdds/dds/core/LoanableCollection.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * A type-safe accessible collection of generic opaque pointers that can receive the buffer from outside (loan).
 *
 * This is an abstract class. See @ref LoanableSequence for details.
 */
template<typename T, typename _NonConstEnabler = std::true_type>
class LoanableTypedCollection : public LoanableCollection
{
public:

    /**
     * Set an element of the sequence.
     *
     * This is the operator that is invoked when the application indexes into a @em non-const sequence:
     * @code{.cpp}
     * element = sequence[n];
     * sequence[n] = element;
     * @endcode
     *
     * Note that a @em reference to the element is returned (and not a copy)
     *
     * @param [in] n index of element to access, must be >= 0 and less than length().
     *
     * @return a reference to the element at position @c n
     */
    template <typename Enabler = _NonConstEnabler>
    typename std::enable_if<Enabler::value, T>::type& operator [](
            size_type n)
    {
        if (n >= length_)
        {
            throw std::out_of_range("");
        }

        return *static_cast<T*>(elements_[n]);
    }

    /**
     * Get an element of the sequence.
     *
     * This is the operator that is invoked when the application indexes into a @em const sequence:
     * @code{.cpp}
     * element = sequence[n];
     * @endcode
     *
     * Note that a @em reference to the element is returned (and not a copy)
     *
     * @param [in] n index of element to access, must be >= 0 and less than length().
     *
     * @return a const reference to the element at position @n
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

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_CORE__LOANABLETYPEDCOLLECTION_HPP
