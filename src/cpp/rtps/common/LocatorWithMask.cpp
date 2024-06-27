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

/*!
 * @file LocatorWithMask.cpp
 */

#include <cassert>
#include <string>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <rtps/network/utils/network.hpp>

#include <fastdds/rtps/common/LocatorWithMask.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

uint8_t LocatorWithMask::mask() const
{
    return mask_;
}

void LocatorWithMask::mask(
        uint8_t mask)
{
    mask_ = mask;
}

bool LocatorWithMask::matches(
        const Locator& loc) const
{
    if (kind == loc.kind)
    {
        switch (kind)
        {
            case LOCATOR_KIND_UDPv4:
            case LOCATOR_KIND_TCPv4:
                assert(32 >= mask());
                return network::address_matches(loc.address + 12, address + 12, mask());

            case LOCATOR_KIND_UDPv6:
            case LOCATOR_KIND_TCPv6:
            case LOCATOR_KIND_SHM:
                assert(128 >= mask());
                return network::address_matches(loc.address, address, mask());
        }
    }

    return false;
}

LocatorWithMask& LocatorWithMask::operator =(
        const Locator& loc)
{
    kind = loc.kind;
    port = loc.port;
    std::memcpy(address, loc.address, 16 * sizeof(octet));
    return *this;
}

std::ostream& operator <<(
        std::ostream& output,
        const LocatorWithMask& loc)
{
    std::stringstream ss;
    operator <<(ss, static_cast<const Locator&>(loc));
    std::string loc_str = ss.str();

    if (loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv4 ||
            loc.kind == LOCATOR_KIND_TCPv6)
    {
        size_t ip_end = loc_str.find("]");
        if (ip_end != std::string::npos)
        {
            std::string netmask_suffix = "/" + std::to_string(loc.mask());
            loc_str.insert(ip_end, netmask_suffix);
        }
    }
    return output << loc_str;
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
