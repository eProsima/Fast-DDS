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
 * @file NetworkInterfaceWithFilter.hpp
 */

#ifndef _FASTDDS_RTPS_TRANSPORT_NETWORK_NETWORKINTERFACEWITHFILTER_HPP_
#define _FASTDDS_RTPS_TRANSPORT_NETWORK_NETWORKINTERFACEWITHFILTER_HPP_

#include <string>

#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/network/NetworkInterface.hpp>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Structure extending \c NetworkInterface with netmask filter information.
 *
 * @note When using this structure to interact with Fast-DDS, \c name is the only attribute the user needs to provide.
 * The rest of the attributes are internally filled, and are in fact ignored even if already provided by the user.
 */
struct NetworkInterfaceWithFilter : public NetworkInterface
{
    //! Constructor by name and netmask filter
    RTPS_DllAPI NetworkInterfaceWithFilter(
            const std::string& name,
            NetmaskFilterKind netmask_filter);

    //! Constructor by name
    RTPS_DllAPI NetworkInterfaceWithFilter(
            const std::string& name);

    //! Constructor by device name, IP address string, locator with mask and netmask filter
    NetworkInterfaceWithFilter(
            const std::string& device,
            const std::string& ip,
            const LocatorWithMask& locator,
            NetmaskFilterKind netmask_filter);

    //! Constructor by device name, IP address string and locator with mask
    RTPS_DllAPI NetworkInterfaceWithFilter(
            const std::string& device,
            const std::string& ip,
            const LocatorWithMask& locator);

    //! Destructor
    virtual RTPS_DllAPI ~NetworkInterfaceWithFilter() = default;

    //! Copy constructor
    RTPS_DllAPI NetworkInterfaceWithFilter(
            const NetworkInterfaceWithFilter& iface) = default;

    //! Copy assignment
    RTPS_DllAPI NetworkInterfaceWithFilter& operator =(
            const NetworkInterfaceWithFilter& iface) = default;

    //! Move constructor
    RTPS_DllAPI NetworkInterfaceWithFilter(
            NetworkInterfaceWithFilter&& iface) = default;

    //! Move assignment
    RTPS_DllAPI NetworkInterfaceWithFilter& operator =(
            NetworkInterfaceWithFilter&& iface) = default;

    //! Comparison operator
    RTPS_DllAPI bool operator ==(
            const NetworkInterfaceWithFilter& iface) const;

    //! Netmask filter configuration
    NetmaskFilterKind netmask_filter;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_NETWORK_NETWORKINTERFACEWITHFILTER_HPP_
