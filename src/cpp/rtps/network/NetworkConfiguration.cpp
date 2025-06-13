// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cassert>

#include <fastdds/rtps/common/Locator.h>

#include <rtps/network/NetworkConfiguration.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace network {

void add_localhost_capability(
        int32_t kind,
        fastrtps::rtps::NetworkConfigSet_t& network_config)
{
    // Only add localhost capability for transports that support it
    if (kind == LOCATOR_KIND_UDPv4 || kind == LOCATOR_KIND_UDPv6 ||
            kind == LOCATOR_KIND_TCPv4 || kind == LOCATOR_KIND_TCPv6)
    {
        // Ensure the kind is a power of two to perform safe bitwise operations
        assert(kind > 0 && (kind & (kind - 1)) == 0);

        network_config |= kind;
    }
}

} // namespace network
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
