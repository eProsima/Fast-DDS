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

#ifndef _FASTDDS_RTPS_TRANSPORT_NETWORK_NETWORKINTERFACE_HPP_
#define _FASTDDS_RTPS_TRANSPORT_NETWORK_NETWORKINTERFACE_HPP_

#include <string>

#include <fastdds/rtps/common/LocatorWithMask.hpp>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Structure encapsulating relevant network interface information.
 *
 * @note When using this structure to interact with Fast-DDS, \c name is the only attribute the user needs to provide.
 * The rest of the attributes are internally filled, and are in fact ignored even if already provided by the user.
 */
struct RTPS_DllAPI NetworkInterface
{
    //! Constructor by name
    NetworkInterface(
            const std::string& name)
        : name(name)
    {
    }

    //! Constructor by device name and IP address
    NetworkInterface(
            const std::string& device,
            const LocatorWithMask& locator)
        : device(device)
        , locator(locator)
    {
    }

    //! Destructor
    virtual ~NetworkInterface() = default;

    //! Copy constructor
    NetworkInterface(
            const NetworkInterface& iface) = default;

    //! Copy assignment
    NetworkInterface& operator =(
            const NetworkInterface& iface) = default;

    //! Move constructor
    NetworkInterface(
            NetworkInterface&& iface) = default;

    //! Move assignment
    NetworkInterface& operator =(
            NetworkInterface&& iface) = default;

    //! Comparison operator
    bool operator ==(
            const NetworkInterface& iface) const
    {
        return (this->name == iface.name &&
           this->device == iface.device &&
           this->locator == iface.locator);
    }

    //! Interface device name or IP address in string format (to be filled by the user)
    std::string name;

    //! Interface device name
    std::string device;
    //! IP address with network mask
    LocatorWithMask locator;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_NETWORK_NETWORKINTERFACE_HPP_
