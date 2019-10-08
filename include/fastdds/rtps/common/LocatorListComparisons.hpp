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

#ifndef _FASTDDS_RTPS_COMMON_LOCATORLISTCOMPARISONS_HPP_
#define _FASTDDS_RTPS_COMMON_LOCATORLISTCOMPARISONS_HPP_

#include <fastdds/rtps/common/Locator.h>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>
#include <algorithm>

namespace eprosima {
namespace fastrtps {
namespace rtps {

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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_COMMON_LOCATORLISTCOMPARISONS_HPP_ */
