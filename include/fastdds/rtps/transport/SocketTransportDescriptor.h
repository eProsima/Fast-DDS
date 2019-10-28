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

#ifndef _FASTDDS_SOCKET_TRANSPORT_DESCRIPTOR_H_
#define _FASTDDS_SOCKET_TRANSPORT_DESCRIPTOR_H_

#include <fastdds/rtps/transport/TransportDescriptorInterface.h>

#ifdef _WIN32
#include <cstdint>
#endif
#include <vector>
#include <string>

namespace eprosima{
namespace fastdds{
namespace rtps{

class TransportInterface;

static const uint8_t s_defaultTTL = 1;

/**
 * Virtual base class for the data type used to define configuration of transports using sockets.
 * @ingroup RTPS_MODULE
 * */
struct SocketTransportDescriptor : public TransportDescriptorInterface
{
    SocketTransportDescriptor(
            uint32_t maximumMessageSize,
            uint32_t maximumInitialPeersRange)
        : TransportDescriptorInterface(maximumMessageSize, maximumInitialPeersRange)
        , sendBufferSize(0)
        , receiveBufferSize(0)
        , TTL(s_defaultTTL)
    {}

    SocketTransportDescriptor(const SocketTransportDescriptor& t)
        : TransportDescriptorInterface(t)
        , sendBufferSize(t.sendBufferSize)
        , receiveBufferSize(t.receiveBufferSize)
        , TTL(t.TTL)
    {}

    virtual ~SocketTransportDescriptor(){}

    virtual uint32_t min_send_buffer_size() const override { return sendBufferSize; }

    //! Length of the send buffer.
    uint32_t sendBufferSize;
    //! Length of the receive buffer.
    uint32_t receiveBufferSize;
    //! Allowed interfaces in an IP string format.
    std::vector<std::string> interfaceWhiteList;
    //! Specified time to live (8bit - 255 max TTL)
    uint8_t TTL;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //_FASTDDS_SOCKET_TRANSPORT_DESCRIPTOR_H_
