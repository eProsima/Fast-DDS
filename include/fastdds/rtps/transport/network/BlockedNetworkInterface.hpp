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
 * @file BlockedNetworkInterface.hpp
 */

#ifndef FASTDDS_RTPS_TRANSPORT_NETWORK__BLOCKEDNETWORKINTERFACE_HPP
#define FASTDDS_RTPS_TRANSPORT_NETWORK__BLOCKEDNETWORKINTERFACE_HPP

#include <fastdds/rtps/transport/network/NetworkInterface.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Structure extending \c NetworkInterface with information specific to blocked interfaces.
 *
 * @note When using this structure to interact with Fast-DDS, \c name is the only attribute the user needs to provide.
 * The rest of the attributes are internally filled, and are in fact ignored even if already provided by the user.
 */
struct BlockedNetworkInterface : public NetworkInterface
{
    using NetworkInterface::NetworkInterface;

    //! Destructor
    virtual FASTDDS_EXPORTED_API ~BlockedNetworkInterface() = default;

    //! Copy constructor
    FASTDDS_EXPORTED_API BlockedNetworkInterface(
            const BlockedNetworkInterface& iface) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API BlockedNetworkInterface& operator =(
            const BlockedNetworkInterface& iface) = default;

    //! Move constructor
    FASTDDS_EXPORTED_API BlockedNetworkInterface(
            BlockedNetworkInterface&& iface) = default;

    //! Move assignment
    FASTDDS_EXPORTED_API BlockedNetworkInterface& operator =(
            BlockedNetworkInterface&& iface) = default;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_NETWORK__BLOCKEDNETWORKINTERFACE_HPP
