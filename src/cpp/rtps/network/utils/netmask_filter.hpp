// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file netmask_filter.hpp
 */

#ifndef _RTPS_NETWORK_UTILS_NETMASK_FILTER_HPP_
#define _RTPS_NETWORK_UTILS_NETMASK_FILTER_HPP_

#include <vector>
#include <string>

#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace network {
namespace netmask_filter {

/**
 * Convert string to \c NetmaskFilterKind if possible.
 *
 * @param [in] netmask_filter_str  String to be converted.
 *
 * @throws std::invalid_argument Thrown on failure.
 *
 * @return Netmask filter kind resulting from conversion.
 */
NetmaskFilterKind string_to_netmask_filter_kind(
        const std::string& netmask_filter_str);

/**
 * Checks whether container and container netmask filter configurations are compatible,
 * and transforms the contained one if applies.
 *
 * Two netmask filter configurations are incompatible if one is ON and the other is OFF,
 * irrespective of which is container and which is contained.
 *
 * The contained netmask filter value will adopt the container's one if the former is AUTO (and the latter is not).
 *
 * @param [in, out] contained_netmask_filter  Contained netmask filter value.
 * @param [in] container_netmask_filter  Container netmask filter value.
 *
 * @return true if container and contained netmask filter configurations are compatible, false otherwise.
 */
bool validate_and_transform(
        NetmaskFilterKind& contained_netmask_filter,
        const NetmaskFilterKind& container_netmask_filter);

/**
 * Check whether netmask filtering configuration is consistent with \c ignore_non_matching_locators parameter.
 *
 * A network factory's netmask filtering configuration is considered invalid if any of its registered transports
 * presents a netmask filter value ON and an empty allowlist, and \c ignore_non_matching_locators is false.
 *
 * @param [in]  factory_netmask_filter_info   Network factory's netmask filter information.
 * @param [in]  ignore_non_matching_locators  Ignore non matching locator configuration parameter.
 * @param [out] error_msg                     Variable where to insert an error message if any present.
 *
 * @return true if valid, false otherwise.
 */
bool check_preconditions(
        const std::vector<TransportNetmaskFilterInfo>& factory_netmask_filter_info,
        bool ignore_non_matching_locators,
        std::string& error_msg);

/**
 * Check whether netmask filtering configuration is consistent with a collection of external locators.
 *
 * Given an external locator, a network factory's netmask filtering configuration is considered invalid if all of its
 * registered transports (of the same kind as the locator) present an allowlist with all entries having
 * netmask filter value set to ON.
 *
 * A network factory's netmask filtering configuration is considered valid if the condition above is not met by any
 * of the external locators present in the collection.
 *
 * @param [in]  factory_netmask_filter_info   Network factory's netmask filter information.
 * @param [in]  external_locators             Collection of external locators.
 * @param [out] error_msg                     Variable where to insert an error message if any present.
 *
 * @return true if valid, false otherwise.
 */
bool check_preconditions(
        const std::vector<TransportNetmaskFilterInfo>& factory_netmask_filter_info,
        const ExternalLocators& external_locators,
        std::string& error_msg);

} // namespace netmask_filter
} // namespace network
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // _RTPS_NETWORK_UTILS_NETMASK_FILTER_HPP_
