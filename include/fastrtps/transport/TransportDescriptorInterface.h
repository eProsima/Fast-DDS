// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TRANSPORT_DESCRIPTOR_INTERFACE_H
#define TRANSPORT_DESCRIPTOR_INTERFACE_H

#ifdef _WIN32
#include <cstdint>
#endif
#include <vector>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TransportInterface;

static const uint8_t s_defaultTTL = 1;

/**
 * Virtual base class for the data type used to define transport configuration.
 * @ingroup RTPS_MODULE
 * */
struct TransportDescriptorInterface
{
    TransportDescriptorInterface(uint32_t maximumMessageSize) : maxMessageSize(maximumMessageSize)
        , sendBufferSize(0)
        , receiveBufferSize(0)
        , TTL(s_defaultTTL)
    {}

    TransportDescriptorInterface(const TransportDescriptorInterface& t) : maxMessageSize(t.maxMessageSize)
        , sendBufferSize(t.sendBufferSize)
        , receiveBufferSize(t.receiveBufferSize)
        , TTL(t.TTL)
        //, rtpsParticipantGuidPrefix(t.rtpsParticipantGuidPrefix)
    {}

    virtual ~TransportDescriptorInterface(){}

    virtual TransportInterface* create_transport() const = 0;

    uint32_t maxMessageSize;

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
} // namespace fastrtps
} // namespace eprosima

#endif //TRANSPORT_DESCRIPTOR_INTERFACE_H
