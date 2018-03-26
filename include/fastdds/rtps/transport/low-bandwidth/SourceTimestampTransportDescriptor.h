// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef SOURCE_TIMESTAMP_TRANSPORT_DESCRIPTOR
#define SOURCE_TIMESTAMP_TRANSPORT_DESCRIPTOR

#include "../ChainingTransportDescriptor.h"

#include <memory>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Transport configuration
 * - low_level_descriptor: Descriptor for lower level transport.
 * - callback: Function called each time a packet is received.
 * - callback_parameter: Parameter to pass as first argument to callback.
 * @ingroup TRANSPORT_MODULE
 */
typedef struct SourceTimestampTransportDescriptor : public ChainingTransportDescriptor
{
    RTPS_DllAPI SourceTimestampTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level);

    RTPS_DllAPI SourceTimestampTransportDescriptor(
            const SourceTimestampTransportDescriptor& t);

    //! Returns the maximum size expected for received messages.
    virtual uint32_t max_message_size() const override
    {
        return low_level_descriptor->max_message_size() + 5;
    }

    /**
     * Factory method pattern.
     * @return A new TransportInterface corresponding to this descriptor.
     */
    virtual TransportInterface* create_transport() const override;

    virtual ~SourceTimestampTransportDescriptor()
    {
    }

    /**
     * Function called upon reception of data
     * @param parameter User callback parameter
     * @param source_sec Source timestamp seconds on the remote endpoint when sending the packet
     * @param recep_sec Source timestamp seconds on the local endpoint when receiving the packet
     * @param recv_bytes Number of bytes of the received packet (including the 5 bytes of overhead)
     */
    void (* callback)(
            void* parameter,
            int32_t source_sec,
            int32_t recep_sec,
            uint32_t recv_bytes);

    //! Parameter used as first argument for callback
    void* callback_parameter;
} SourceTimestampTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
