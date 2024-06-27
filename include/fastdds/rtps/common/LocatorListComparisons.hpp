// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocatorListComparisons.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__LOCATORLISTCOMPARISONS_HPP
#define FASTDDS_RTPS_COMMON__LOCATORLISTCOMPARISONS_HPP

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <algorithm>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Equal to operator to compare two locator lists.
 *
 * @param lhs Locator list to be compared.
 * @param rhs Other locator list to be compared.
 * @return true if the list are equal.
 * @return false otherwise.
 */
static inline bool operator == (
        const ResourceLimitedVector<Locator_t>& lhs,
        const ResourceLimitedVector<Locator_t>& rhs)
{
    if (lhs.size() == rhs.size())
    {
        for (const Locator_t& locator : lhs)
        {
            if (std::find(rhs.begin(), rhs.end(), locator) == rhs.end())
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__LOCATORLISTCOMPARISONS_HPP
