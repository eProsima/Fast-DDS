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
 * @file network.hpp
 */

#ifndef _RTPS_NETWORK_UTILS_NETWORK_HPP_
#define _RTPS_NETWORK_UTILS_NETWORK_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace network {

/**
 * Checks whether two given addresses are equal in their first \c num_bits bits.
 *
 * @param [in] addr1     First address to compare.
 * @param [in] addr2     Second address to compare.
 * @param [in] num_bits  Number of bits to be taken into consideration.
 *
 * @return true if they match, false otherwise.
 */
bool address_matches(
        const uint8_t* addr1,
        const uint8_t* addr2,
        uint64_t num_bits);

} // namespace network
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // _RTPS_NETWORK_UTILS_NETWORK_HPP_
