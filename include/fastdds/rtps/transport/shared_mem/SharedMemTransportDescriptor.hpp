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

/**
 * @file SharedMemTransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT_SHARED_MEM__SHAREDMEMTRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT_SHARED_MEM__SHAREDMEMTRANSPORTDESCRIPTOR_HPP

#include <cstdint>
#include <string>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TransportInterface;

/**
 * Shared memory transport configuration.
 * The kind is given by eprosima::fastdds::rtps::LOCATOR_KIND_SHM.
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
struct SharedMemTransportDescriptor : public PortBasedTransportDescriptor
{
    static constexpr uint32_t shm_default_segment_size = 0;
    static constexpr uint32_t shm_default_port_queue_capacity = 512;
    static constexpr uint32_t shm_default_healthy_check_timeout_ms = 1000;
    static constexpr uint32_t shm_implicit_segment_size = 512 * 1024;

    //! Destructor
    virtual ~SharedMemTransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Minimum size of the send buffer
    uint32_t min_send_buffer_size() const override
    {
        return segment_size_;
    }

    //! Constructor
    FASTDDS_EXPORTED_API SharedMemTransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API SharedMemTransportDescriptor(
            const SharedMemTransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API SharedMemTransportDescriptor& operator =(
            const SharedMemTransportDescriptor& t) = default;

    //! Return the size of the shared memory segment
    FASTDDS_EXPORTED_API uint32_t segment_size() const
    {
        return segment_size_;
    }

    //! Set the size of the shared memory segment
    FASTDDS_EXPORTED_API void segment_size(
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
    FASTDDS_EXPORTED_API void max_message_size(
            uint32_t max_message_size)
    {
        maxMessageSize = max_message_size;
    }

    //! Return the size of the listening port (in messages)
    FASTDDS_EXPORTED_API uint32_t port_queue_capacity() const
    {
        return port_queue_capacity_;
    }

    //! Set the size of the listening port (in messages)
    FASTDDS_EXPORTED_API void port_queue_capacity(
            uint32_t port_queue_capacity)
    {
        port_queue_capacity_ = port_queue_capacity;
    }

    //! Return the timeout for the health check of ports (ms)
    FASTDDS_EXPORTED_API uint32_t healthy_check_timeout_ms() const
    {
        return healthy_check_timeout_ms_;
    }

    //! Set the timeout for the health check of ports (ms)
    FASTDDS_EXPORTED_API void healthy_check_timeout_ms(
            uint32_t healthy_check_timeout_ms)
    {
        healthy_check_timeout_ms_ = healthy_check_timeout_ms;
    }

    //! Return the full path of the protocol dump file
    FASTDDS_EXPORTED_API std::string rtps_dump_file() const
    {
        return rtps_dump_file_;
    }

    //! Set the full path of the protocol dump file
    FASTDDS_EXPORTED_API void rtps_dump_file(
            const std::string& rtps_dump_file)
    {
        rtps_dump_file_ = rtps_dump_file;
    }

    //! Return the thread settings for the transport dump thread
    FASTDDS_EXPORTED_API ThreadSettings dump_thread() const
    {
        return dump_thread_;
    }

    //! Set the thread settings for the transport dump thread
    FASTDDS_EXPORTED_API void dump_thread(
            const ThreadSettings& dump_thread)
    {
        dump_thread_ = dump_thread;
    }

    //! Comparison operator
    FASTDDS_EXPORTED_API bool operator ==(
            const SharedMemTransportDescriptor& t) const;

private:

    uint32_t segment_size_ = shm_default_segment_size;
    uint32_t port_queue_capacity_ = shm_default_port_queue_capacity;
    uint32_t healthy_check_timeout_ms_ = shm_default_healthy_check_timeout_ms;
    std::string rtps_dump_file_ {""};

    //! Thread settings for the transport dump thread
    ThreadSettings dump_thread_ {};

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_SHARED_MEM__SHAREDMEMTRANSPORTDESCRIPTOR_HPP
