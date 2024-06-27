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
 * @file LocatorsIterator.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__LOCATORSITERATOR_HPP
#define FASTDDS_RTPS_COMMON__LOCATORSITERATOR_HPP

#include <fastdds/rtps/common/Locator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Provides a Locator's iterator interface that can be used by different Locator's
 * containers
 */
struct LocatorsIterator
{
    /**
     * @brief Increment operator.
     *
     * @return LocatorsIterator& reference to the next LocatorsIterator.
     */
    virtual LocatorsIterator& operator ++() = 0;
    /**
     * @brief Equal to operator.
     *
     * @param other LocatorsIterator to compare.
     * @return true if equal.
     * @return false otherwise.
     */
    virtual bool operator ==(
            const LocatorsIterator& other) const = 0;
    /**
     * @brief Not equal to operator.
     *
     * @param other LocatorsIterator to compare.
     * @return true if not equal.
     * @return false otherwise.
     */
    virtual bool operator !=(
            const LocatorsIterator& other) const = 0;
    /**
     * @brief Dereference operator.
     *
     * @return const Locator& Reference to the locator pointed by the LocatorsIterator.
     */
    virtual const Locator& operator *() const = 0;
};

using LocatorsIterator = eprosima::fastdds::rtps::LocatorsIterator;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__LOCATORSITERATOR_HPP
