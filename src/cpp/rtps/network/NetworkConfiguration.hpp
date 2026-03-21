// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file NetworkConfiguration.hpp
 */

#ifndef FASTDDS_RTPS_NETWORK__NETWORKCONFIGURATION_HPP
#define FASTDDS_RTPS_NETWORK__NETWORKCONFIGURATION_HPP

#include <fastdds/rtps/common/Types.hpp>

#define DISC_NETWORK_CONFIGURATION_LISTENING_LOCALHOST_ALL                         (0x0000000F)

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace network {

/**
 * @brief Add the capability to use localhost for the given transport kind.
 *
 * This function adds the capability to use localhost for the given transport kind
 * in the provided network configuration.
 *
 * @param kind The transport kind to add the localhost capability for.
 * @param network_config The network configuration to modify.
 */
void add_localhost_capability(
        int32_t kind,
        NetworkConfigSet_t& network_config);

} // namespace network
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_NETWORK__NETWORKCONFIGURATION_HPP
