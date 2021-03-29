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

#ifndef _FASTDDS_SHAREDMEM_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_SHAREDMEM_TRANSPORT_DESCRIPTOR_

#include "fastdds/rtps/transport/TransportDescriptorInterface.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

class TransportInterface;

/**
 * Shared memory transport configuration
 *
 * @ingroup TRANSPORT_MODULE
 */
typedef struct SharedMemTransportDescriptor : public TransportDescriptorInterface
{
    virtual ~SharedMemTransportDescriptor()
    {

    }

    virtual TransportInterface* create_transport() const override;
    uint32_t min_send_buffer_size() const override
    {
        return segment_size_;
    }

    RTPS_DllAPI SharedMemTransportDescriptor();

    RTPS_DllAPI SharedMemTransportDescriptor(
            const SharedMemTransportDescriptor& t);

    RTPS_DllAPI uint32_t segment_size() const
    {
        return segment_size_;
    }

    RTPS_DllAPI void segment_size(
            uint32_t segment_size)
    {
        segment_size_ = segment_size;
    }

    virtual uint32_t max_message_size() const override
    {
        return maxMessageSize;
    }

    RTPS_DllAPI void max_message_size(
            uint32_t max_message_size)
    {
        maxMessageSize = max_message_size;
    }

    RTPS_DllAPI uint32_t port_queue_capacity() const
    {
        return port_queue_capacity_;
    }

    RTPS_DllAPI void port_queue_capacity(
            uint32_t port_queue_capacity)
    {
        port_queue_capacity_ = port_queue_capacity;
    }

    RTPS_DllAPI uint32_t healthy_check_timeout_ms() const
    {
        return healthy_check_timeout_ms_;
    }

    RTPS_DllAPI void healthy_check_timeout_ms(
            uint32_t healthy_check_timeout_ms)
    {
        healthy_check_timeout_ms_ = healthy_check_timeout_ms;
    }

    RTPS_DllAPI std::string rtps_dump_file() const
    {
        return rtps_dump_file_;
    }

    RTPS_DllAPI void rtps_dump_file(
            const std::string& rtps_dump_file)
    {
        rtps_dump_file_ = rtps_dump_file;
    }

private:

    uint32_t segment_size_;
    uint32_t port_queue_capacity_;
    uint32_t healthy_check_timeout_ms_;
    std::string rtps_dump_file_;

}SharedMemTransportDescriptor;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_TRANSPORT_DESCRIPTOR_
