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

#ifndef _FASTDDS_TRANSPORT_DESCRIPTOR_INTERFACE_H_
#define _FASTDDS_TRANSPORT_DESCRIPTOR_INTERFACE_H_

#ifdef _WIN32
#include <cstdint>
#endif
#include <vector>
#include <string>

namespace eprosima{
namespace fastdds{
namespace rtps{

class TransportInterface;

/**
 * Virtual base class for the data type used to define transport configuration.
 * @ingroup RTPS_MODULE
 * */
struct TransportDescriptorInterface
{
    TransportDescriptorInterface(uint32_t maximumMessageSize, uint32_t maximumInitialPeersRange)
        : maxMessageSize(maximumMessageSize)
        , maxInitialPeersRange(maximumInitialPeersRange)
    {}

    TransportDescriptorInterface(const TransportDescriptorInterface& t)
        : maxMessageSize(t.maxMessageSize)
        , maxInitialPeersRange(t.maxInitialPeersRange)
    {}

    virtual ~TransportDescriptorInterface(){}

    /**
     * Factory method pattern. It will create and return a TransportInterface
     * corresponding to this descriptor. This provides an interface to the NetworkFactory
     * to create the transports without the need to know about their type
     */
    virtual TransportInterface* create_transport() const = 0;

     //! Returns the minimum size required for a send operation.
    virtual uint32_t min_send_buffer_size() const = 0;

    //! Returns the maximum size expected for received messages.
    virtual uint32_t max_message_size() const { return maxMessageSize; }

    virtual uint32_t max_initial_peers_range() const { return maxInitialPeersRange; }

    uint32_t maxMessageSize;

    uint32_t maxInitialPeersRange;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TRANSPORT_DESCRIPTOR_INTERFACE_H_
