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
 * @file NetworkInterfaceWithFilter.cpp
 */

#include <fastdds/rtps/transport/network/NetworkInterfaceWithFilter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

NetworkInterfaceWithFilter::NetworkInterfaceWithFilter(
        const std::string& name,
        NetmaskFilterKind netmask_filter)
    : NetworkInterface(name)
    , netmask_filter(netmask_filter)
{
}

NetworkInterfaceWithFilter::NetworkInterfaceWithFilter(
        const std::string& name)
    : NetworkInterfaceWithFilter(name, NetmaskFilterKind::AUTO)
{
}

NetworkInterfaceWithFilter::NetworkInterfaceWithFilter(
        const std::string& device,
        const std::string& ip,
        const LocatorWithMask& locator,
        NetmaskFilterKind netmask_filter)
    : NetworkInterface(device, ip, locator)
    , netmask_filter(netmask_filter)
{
}

NetworkInterfaceWithFilter::NetworkInterfaceWithFilter(
        const std::string& device,
        const std::string& ip,
        const LocatorWithMask& locator)
    : NetworkInterfaceWithFilter(device, ip, locator, NetmaskFilterKind::AUTO)
{
}

bool NetworkInterfaceWithFilter::operator ==(
        const NetworkInterfaceWithFilter& iface) const
{
    return (this->netmask_filter == iface.netmask_filter &&
           NetworkInterface::operator ==(iface));
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
