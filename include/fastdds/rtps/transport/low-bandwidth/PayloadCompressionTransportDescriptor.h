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

#ifndef PAYLOAD_COMPRESSION_TRANSPORT_DESCRIPTOR
#define PAYLOAD_COMPRESSION_TRANSPORT_DESCRIPTOR

#include "../ChainingTransportDescriptor.h"

#if HAVE_ZLIB || HAVE_BZIP2

#include <memory>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Transport configuration
 * - low_level_descriptor: Descriptor for lower level transport.
 * @ingroup TRANSPORT_MODULE
 */
typedef struct PayloadCompressionTransportDescriptor : public ChainingTransportDescriptor
{
    RTPS_DllAPI PayloadCompressionTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level);

    RTPS_DllAPI PayloadCompressionTransportDescriptor(
            const PayloadCompressionTransportDescriptor& t);

    /**
     * Factory method pattern.
     * @return A new TransportInterface corresponding to this descriptor.
     */
    virtual TransportInterface* create_transport() const override;

    virtual ~PayloadCompressionTransportDescriptor()
    {
    }

} PayloadCompressionTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

#endif
