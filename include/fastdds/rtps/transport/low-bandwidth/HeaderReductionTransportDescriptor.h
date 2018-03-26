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

#ifndef HEADER_REDUCTION_TRANSPORT_DESCRIPTOR
#define HEADER_REDUCTION_TRANSPORT_DESCRIPTOR

#include "../ChainingTransportDescriptor.h"

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
typedef struct HeaderReductionTransportDescriptor : public ChainingTransportDescriptor
{
    RTPS_DllAPI HeaderReductionTransportDescriptor(
            std::shared_ptr<TransportDescriptorInterface> low_level);

    RTPS_DllAPI HeaderReductionTransportDescriptor(
            const HeaderReductionTransportDescriptor& t);

    /**
     * Factory method pattern.
     * @return A new TransportInterface corresponding to this descriptor.
     */
    virtual TransportInterface* create_transport() const override;

    virtual ~HeaderReductionTransportDescriptor()
    {
    }

} HeaderReductionTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
