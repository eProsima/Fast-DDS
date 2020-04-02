// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_TEST_SHAREDMEM_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_TEST_SHAREDMEM_TRANSPORT_DESCRIPTOR_

#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Shared memory transport configuration
 *
 * @ingroup TRANSPORT_MODULE
 */
typedef struct test_SharedMemTransportDescriptor : public SharedMemTransportDescriptor
{
    virtual ~test_SharedMemTransportDescriptor() {}

    RTPS_DllAPI test_SharedMemTransportDescriptor();
    RTPS_DllAPI test_SharedMemTransportDescriptor(
            const test_SharedMemTransportDescriptor& t);

    virtual TransportInterface* create_transport() const override;

    uint32_t big_buffer_size_;
    uint32_t* big_buffer_size_count_;

}test_SharedMemTransportDescriptor;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //_FASTDDS_TEST_SHAREDMEM_TRANSPORT_DESCRIPTOR_
