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
 * @file ExternalLocators.hpp
 */

#ifndef FASTDDS_RTPS_ATTRIBUTES__EXTERNALLOCATORS_HPP
#define FASTDDS_RTPS_ATTRIBUTES__EXTERNALLOCATORS_HPP

#include <map>
#include <vector>

#include <fastdds/rtps/common/LocatorWithMask.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * A collection of LocatorWithMask grouped by externality and cost.
 */
using ExternalLocators = std::map<
    uint8_t, // externality_index
    std::map<
        uint8_t, // cost
        std::vector<LocatorWithMask> // locators with their mask
        >,
    std::greater<uint8_t> // Ordered by greater externality_index
    >;


} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES__EXTERNALLOCATORS_HPP
