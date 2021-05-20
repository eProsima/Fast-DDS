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
 * Shared memory transport configuration.
 * The kind value for SharedMemTransportDescriptor is given by eprosima::fastrtps::rtps::LOCATOR_KIND_SHM.
 *
 * - segment_size_: size of the shared memory segment (in octets).
 *
 * - port_queue_capacity_: size of the listening port (in messages).
 *
 * - healthy_check_timeout_ms_: timeout for the health check of ports (ms).
 *
 * - rtps_dump_file_: full path of the protocol dump file.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct SharedMemTransportDescriptor : public TransportDescriptorInterface
{
    //! Destructor
    virtual ~SharedMemTransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Minimum size of the send buffer
    uint32_t min_send_buffer_size() const override
    {
        return segment_size_;
    }

    //! Constructor
    RTPS_DllAPI SharedMemTransportDescriptor();

    //! Copy constructor
    RTPS_DllAPI SharedMemTransportDescriptor(
            const SharedMemTransportDescriptor& t) = default;

    //! Copy assignment
    RTPS_DllAPI SharedMemTransportDescriptor& operator =(
            const SharedMemTransportDescriptor& t) = default;

    //! Return the size of the shared memory segment
    RTPS_DllAPI uint32_t segment_size() const
    {
        return segment_size_;
    }

    //! Set the size of the shared memory segment
    RTPS_DllAPI void segment_size(
            uint32_t segment_size)
    {
        segment_size_ = segment_size;
    }

    //! Return the maximum size of a single message in the transport (in octets)
    virtual uint32_t max_message_size() const override
    {
        return maxMessageSize;
    }

    //! Set the maximum size of a single message in the transport (in octets)
    RTPS_DllAPI void max_message_size(
            uint32_t max_message_size)
    {
        maxMessageSize = max_message_size;
    }

    //! Return the size of the listening port (in messages)
    RTPS_DllAPI uint32_t port_queue_capacity() const
    {
        return port_queue_capacity_;
    }

    //! Set the size of the listening port (in messages)
    RTPS_DllAPI void port_queue_capacity(
            uint32_t port_queue_capacity)
    {
        port_queue_capacity_ = port_queue_capacity;
    }

    //! Return the timeout for the health check of ports (ms)
    RTPS_DllAPI uint32_t healthy_check_timeout_ms() const
    {
        return healthy_check_timeout_ms_;
    }

    //! Set the timeout for the health check of ports (ms)
    RTPS_DllAPI void healthy_check_timeout_ms(
            uint32_t healthy_check_timeout_ms)
    {
        healthy_check_timeout_ms_ = healthy_check_timeout_ms;
    }

    //! Return the full path of the protocol dump file
    RTPS_DllAPI std::string rtps_dump_file() const
    {
        return rtps_dump_file_;
    }

    //! Set the full path of the protocol dump file
    RTPS_DllAPI void rtps_dump_file(
            const std::string& rtps_dump_file)
    {
        rtps_dump_file_ = rtps_dump_file;
    }

    //! Comparison operator
    RTPS_DllAPI bool operator ==(
            const SharedMemTransportDescriptor& t) const;

private:

    uint32_t segment_size_;
    uint32_t port_queue_capacity_;
    uint32_t healthy_check_timeout_ms_;
    std::string rtps_dump_file_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_TRANSPORT_DESCRIPTOR_
