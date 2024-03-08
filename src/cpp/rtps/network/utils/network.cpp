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
 * @file network.cpp
 */

#include <algorithm>
#include <cstdint>

#include <rtps/network/utils/network.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace network {

bool address_matches(
        const uint8_t* addr1,
        const uint8_t* addr2,
        uint64_t num_bits)
{
    uint64_t full_bytes = num_bits / 8;
    if ((0 == full_bytes) || std::equal(addr1, addr1 + full_bytes, addr2))
    {
        uint64_t rem_bits = num_bits % 8;
        if (rem_bits == 0)
        {
            return true;
        }

        uint8_t mask = 0xFF << (8 - rem_bits);
        return (addr1[full_bytes] & mask) == (addr2[full_bytes] & mask);
    }

    return false;
}

} // namespace network
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
