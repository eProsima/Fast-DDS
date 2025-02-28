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

namespace eprosima {
namespace fastdds {
namespace rtps {

class TransportInterface;

/**
 * Shared memory transport configuration
 *
 * @ingroup TRANSPORT_MODULE
 */
struct SharedMemTransportDescriptor : public PortBasedTransportDescriptor
{
    static constexpr uint32_t shm_default_segment_size = 0;
    static constexpr uint32_t shm_default_port_queue_capacity = 512;
    static constexpr uint32_t shm_default_healthy_check_timeout_ms = 1000;

    virtual ~SharedMemTransportDescriptor()
    {

    }

    FASTDDS_EXPORTED_API SharedMemTransportDescriptor()
        : PortBasedTransportDescriptor(0, 0)
    {

    }

    virtual TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    uint32_t min_send_buffer_size() const override
    {
        return 0;
    }

    FASTDDS_EXPORTED_API uint32_t segment_size() const
    {
        return segment_size_;
    }

    FASTDDS_EXPORTED_API void segment_size(
            uint32_t segment_size)
    {
        segment_size_ = segment_size;
    }

    virtual uint32_t max_message_size() const override
    {
        return maxMessageSize;
    }

    FASTDDS_EXPORTED_API void max_message_size(
            uint32_t max_message_size)
    {
        maxMessageSize = max_message_size;
    }

    FASTDDS_EXPORTED_API uint32_t port_queue_capacity() const
    {
        return port_queue_capacity_;
    }

    FASTDDS_EXPORTED_API void port_queue_capacity(
            uint32_t port_queue_capacity)
    {
        port_queue_capacity_ = port_queue_capacity;
    }

    FASTDDS_EXPORTED_API uint32_t healthy_check_timeout_ms() const
    {
        return healthy_check_timeout_ms_;
    }

    FASTDDS_EXPORTED_API void healthy_check_timeout_ms(
            uint32_t healthy_check_timeout_ms)
    {
        healthy_check_timeout_ms_ = healthy_check_timeout_ms;
    }

    FASTDDS_EXPORTED_API std::string rtps_dump_file() const
    {
        return rtps_dump_file_;
    }

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

private:

    uint32_t segment_size_ = shm_default_segment_size;
    uint32_t port_queue_capacity_ = shm_default_port_queue_capacity;
    uint32_t healthy_check_timeout_ms_ = shm_default_healthy_check_timeout_ms;
    std::string rtps_dump_file_;
    ThreadSettings dump_thread_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_SHARED_MEM__SHAREDMEMTRANSPORTDESCRIPTOR_HPP
