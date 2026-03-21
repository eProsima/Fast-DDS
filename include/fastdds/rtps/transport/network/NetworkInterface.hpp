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
 * @file NetworkInterface.hpp
 */

#ifndef FASTDDS_RTPS_TRANSPORT_NETWORK__NETWORKINTERFACE_HPP
#define FASTDDS_RTPS_TRANSPORT_NETWORK__NETWORKINTERFACE_HPP

#include <string>

#include <fastdds/rtps/common/LocatorWithMask.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Structure encapsulating relevant network interface information.
 *
 * @note When using this structure to interact with Fast-DDS, \c name is the only attribute the user needs to provide.
 * The rest of the attributes are internally filled, and are in fact ignored even if already provided by the user.
 */
struct NetworkInterface
{
    //! Constructor by name
    FASTDDS_EXPORTED_API NetworkInterface(
            const std::string& name);

    //! Constructor by device name, IP address string and locator with mask
    FASTDDS_EXPORTED_API NetworkInterface(
            const std::string& device,
            const std::string& ip,
            const LocatorWithMask& locator);

    //! Destructor
    virtual FASTDDS_EXPORTED_API ~NetworkInterface() = default;

    //! Copy constructor
    FASTDDS_EXPORTED_API NetworkInterface(
            const NetworkInterface& iface) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API NetworkInterface& operator =(
            const NetworkInterface& iface) = default;

    //! Move constructor
    FASTDDS_EXPORTED_API NetworkInterface(
            NetworkInterface&& iface) = default;

    //! Move assignment
    FASTDDS_EXPORTED_API NetworkInterface& operator =(
            NetworkInterface&& iface) = default;

    //! Comparison operator
    FASTDDS_EXPORTED_API bool operator ==(
            const NetworkInterface& iface) const;

    //! Interface device name or IP address in string format (to be filled by the user)
    std::string name;

    //! Interface device name
    std::string device;
    //! IP address in string format (includes scope ID in the IPv6 case)
    std::string ip;
    //! IP address with network mask
    LocatorWithMask locator;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_NETWORK__NETWORKINTERFACE_HPP
