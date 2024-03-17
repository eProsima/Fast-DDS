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
 * @file AllowedNetworkInterface.hpp
 */

#ifndef _FASTDDS_RTPS_TRANSPORT_NETWORK_ALLOWEDNETWORKINTERFACE_HPP_
#define _FASTDDS_RTPS_TRANSPORT_NETWORK_ALLOWEDNETWORKINTERFACE_HPP_

#include <fastdds/rtps/transport/network/NetworkInterfaceWithFilter.hpp>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Structure extending \c NetworkInterfaceWithFilter with information specific to allowed interfaces.
 *
 * @note When using this structure to interact with Fast-DDS, \c name is the only attribute the user needs to provide.
 * The rest of the attributes are internally filled, and are in fact ignored even if already provided by the user.
 */
struct AllowedNetworkInterface : public NetworkInterfaceWithFilter
{
    using NetworkInterfaceWithFilter::NetworkInterfaceWithFilter;

    //! Destructor
    virtual RTPS_DllAPI ~AllowedNetworkInterface() = default;

    //! Copy constructor
    RTPS_DllAPI AllowedNetworkInterface(
            const AllowedNetworkInterface& iface) = default;

    //! Copy assignment
    RTPS_DllAPI AllowedNetworkInterface& operator =(
            const AllowedNetworkInterface& iface) = default;

    //! Move constructor
    RTPS_DllAPI AllowedNetworkInterface(
            AllowedNetworkInterface&& iface) = default;

    //! Move assignment
    RTPS_DllAPI AllowedNetworkInterface& operator =(
            AllowedNetworkInterface&& iface) = default;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_NETWORK_ALLOWEDNETWORKINTERFACE_HPP_
