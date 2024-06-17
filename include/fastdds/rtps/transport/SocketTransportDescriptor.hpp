// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SocketTransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__SOCKETTRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__SOCKETTRANSPORTDESCRIPTOR_HPP

#include <cstdint>
#include <vector>
#include <string>

#include <fastdds/rtps/transport/network/AllowedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/BlockedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

//! Default time to live (TTL)
constexpr uint8_t s_defaultTTL = 1;

/**
 * Virtual base class for the data type used to define configuration of transports using sockets.
 *
 * - \c sendBufferSize: size of the sending buffer of the socket (in octets).
 *
 * - \c receiveBufferSize: size of the receiving buffer of the socket (in octets).
 *
 * - \c interfaceWhiteList: list of allowed interfaces.
 *
 * - \c TTL: time to live, in number of hops.
 *
 * @ingroup RTPS_MODULE
 * */
struct SocketTransportDescriptor : public PortBasedTransportDescriptor
{
    //! Constructor
    FASTDDS_EXPORTED_API SocketTransportDescriptor(
            uint32_t maximumMessageSize,
            uint32_t maximumInitialPeersRange)
        : PortBasedTransportDescriptor(maximumMessageSize, maximumInitialPeersRange)
        , sendBufferSize(0)
        , receiveBufferSize(0)
        , netmask_filter(NetmaskFilterKind::AUTO)
        , TTL(s_defaultTTL)
    {
    }

    //! Copy constructor
    FASTDDS_EXPORTED_API SocketTransportDescriptor(
            const SocketTransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API SocketTransportDescriptor& operator =(
            const SocketTransportDescriptor& t) = default;

    //! Destructor
    virtual FASTDDS_EXPORTED_API ~SocketTransportDescriptor() = default;

    virtual FASTDDS_EXPORTED_API uint32_t min_send_buffer_size() const override
    {
        return sendBufferSize;
    }

    //! Comparison operator
    bool FASTDDS_EXPORTED_API operator ==(
            const SocketTransportDescriptor& t) const
    {
        return (this->sendBufferSize == t.min_send_buffer_size() &&
               this->receiveBufferSize == t.receiveBufferSize &&
               this->interfaceWhiteList == t.interfaceWhiteList &&
               this->netmask_filter == t.netmask_filter &&
               this->interface_allowlist == t.interface_allowlist &&
               this->interface_blocklist == t.interface_blocklist &&
               this->TTL == t.TTL &&
               PortBasedTransportDescriptor::operator ==(t));
    }

    //! Length of the send buffer.
    uint32_t sendBufferSize;
    //! Length of the receive buffer.
    uint32_t receiveBufferSize;
    //! Allowed interfaces in an IP or device name string format.
    std::vector<std::string> interfaceWhiteList;
    //! Transport's netmask filter configuration.
    NetmaskFilterKind netmask_filter;
    //! Allowed interfaces in an IP or device name string format, each with a specific netmask filter configuration.
    std::vector<AllowedNetworkInterface> interface_allowlist;
    //! Blocked interfaces in an IP or device name string format.
    std::vector<BlockedNetworkInterface> interface_blocklist;
    //! Specified time to live (8bit - 255 max TTL)
    uint8_t TTL;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //FASTDDS_RTPS_TRANSPORT__SOCKETTRANSPORTDESCRIPTOR_HPP
