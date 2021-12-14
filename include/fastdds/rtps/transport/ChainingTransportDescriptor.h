// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_RTPS_TRANSPORT_CHAININGTRANSPORTDESCRIPTOR_H_
#define _FASTDDS_RTPS_TRANSPORT_CHAININGTRANSPORTDESCRIPTOR_H_

#include <memory>
#include <vector>

#include "TransportInterface.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Base class for the descriptors of chaining transports. A chaining transport
 * allows for the manipulation of data before sending or after receiving from
 * another transport.
 *
 * Transport configuration:
 *
 * - low_level_descriptor: Descriptor for lower level transport.
 * @ingroup TRANSPORT_MODULE
 */
typedef struct ChainingTransportDescriptor : public TransportDescriptorInterface
{
    RTPS_DllAPI ChainingTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level)
        : TransportDescriptorInterface(low_level->maxMessageSize, low_level->maxInitialPeersRange)
        , low_level_descriptor(low_level)
    {
    }

    RTPS_DllAPI ChainingTransportDescriptor(
            const ChainingTransportDescriptor& t)
        : TransportDescriptorInterface(t)
        , low_level_descriptor(t.low_level_descriptor)
    {
    }

    //! Returns the minimum size required for a send operation.
    RTPS_DllAPI virtual uint32_t min_send_buffer_size() const override
    {
        return low_level_descriptor->min_send_buffer_size();
    }

    //! Returns the maximum size expected for received messages.
    RTPS_DllAPI virtual uint32_t max_message_size() const override
    {
        return low_level_descriptor->max_message_size();
    }

    RTPS_DllAPI virtual ~ChainingTransportDescriptor() = default;

    //! Descriptor for lower level transport
    std::shared_ptr<TransportDescriptorInterface> low_level_descriptor;
} ChainingTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_CHAININGTRANSPORTDESCRIPTOR_H_
