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
 * @file NetworkInterface.cpp
 */

#include <fastdds/rtps/transport/network/NetworkInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

NetworkInterface::NetworkInterface(
        const std::string& name)
    : name(name)
{
}

NetworkInterface::NetworkInterface(
        const std::string& device,
        const std::string& ip,
        const LocatorWithMask& locator)
    : device(device)
    , ip(ip)
    , locator(locator)
{
}

bool NetworkInterface::operator ==(
        const NetworkInterface& iface) const
{
    return (this->name == iface.name &&
           this->device == iface.device &&
           this->ip == iface.ip &&
           this->locator == iface.locator);
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
