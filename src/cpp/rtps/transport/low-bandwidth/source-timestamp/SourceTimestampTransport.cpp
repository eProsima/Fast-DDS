/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include "SourceTimestampTransport.hpp"

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/rtps/common/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

// ------------------------- Descriptor
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

SourceTimestampTransportDescriptor::SourceTimestampTransportDescriptor(
        std::shared_ptr<TransportDescriptorInterface> low_level)
    : ChainingTransportDescriptor(low_level)
    , callback(nullptr)
    , callback_parameter(nullptr)
{
}

SourceTimestampTransportDescriptor::SourceTimestampTransportDescriptor(
        const SourceTimestampTransportDescriptor& t)
    : ChainingTransportDescriptor(t)
    , callback(t.callback)
    , callback_parameter(t.callback_parameter)
{
}

TransportInterface* SourceTimestampTransportDescriptor::create_transport() const
{
    return new SourceTimestampTransport(*this);
}

// ------------------------- Descriptor
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SourceTimestampTransport::SourceTimestampTransport(
        const SourceTimestampTransportDescriptor& descriptor)
    : ChainingTransport(descriptor)
    , configuration_(descriptor)
{
}

SourceTimestampTransport::~SourceTimestampTransport()
{
    if (nullptr != compress_buffer_)
    {
        free(compress_buffer_);
        compress_buffer_ = nullptr;
    }

    if (nullptr != assembly_buffer_)
    {
        free(assembly_buffer_);
        assembly_buffer_ = nullptr;
    }
}

bool SourceTimestampTransport::init(
        const fastdds::rtps::PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    bool ret_val = ChainingTransport::init(properties, max_msg_size_no_frag);
    if (ret_val)
    {
        compress_buffer_ = (fastdds::rtps::octet*) calloc(
            configuration_.max_message_size(), sizeof(fastdds::rtps::octet));
        assembly_buffer_ = (fastdds::rtps::octet*) calloc(
            configuration_.max_message_size(), sizeof(fastdds::rtps::octet));
        ret_val = compress_buffer_ && assembly_buffer_;
    }

    return ret_val;
}

bool SourceTimestampTransport::send(
        fastdds::rtps::SenderResource* low_sender_resource,
        const std::vector<NetworkBuffer>& buffers,
        uint32_t send_buffer_size,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& timeout)
{
    return send_w_priority(low_sender_resource, buffers, send_buffer_size,
                   destination_locators_begin, destination_locators_end, timeout, 0);
}

bool SourceTimestampTransport::send_w_priority(
        fastdds::rtps::SenderResource* low_sender_resource,
        const std::vector<NetworkBuffer>& buffers,
        uint32_t send_buffer_size,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& timeout,
        int32_t transport_priority)
{
    std::lock_guard<std::recursive_mutex> scopedLock(compress_buffer_mutex_);
    fastdds::rtps::Time_t send_time;
    fastdds::rtps::Time_t::now(send_time);

    fastcdr::FastBuffer write_buffer(reinterpret_cast<char*>(&compress_buffer_[1]), 4);
    fastcdr::Cdr write_cdr(write_buffer, fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS, fastcdr::CdrVersion::DDS_CDR);
    write_cdr << send_time.seconds();
    compress_buffer_[0] = (fastdds::rtps::octet) 'S';
    size_t pos {5};
    for (const NetworkBuffer& buffer : buffers)
    {
        memcpy(&compress_buffer_[pos], buffer.buffer, buffer.size);
        pos += buffer.size;
    }
    NetworkBuffer network_buffer;
    network_buffer.buffer = compress_buffer_;
    network_buffer.size = send_buffer_size + 5;
    return low_sender_resource->send({network_buffer}, send_buffer_size + 5,
                   destination_locators_begin, destination_locators_end, timeout, transport_priority);
}

void SourceTimestampTransport::receive(
        TransportReceiverInterface* next_receiver,
        const fastdds::rtps::octet* receive_buffer,
        uint32_t receive_buffer_size,
        const fastdds::rtps::Locator_t& local_locator,
        const fastdds::rtps::Locator_t& remote_locator)
{

    bool ret_val = (receive_buffer_size > 5) && (receive_buffer[0] == (fastdds::rtps::octet) 'S');

    if (ret_val)
    {
        if ((configuration_.callback != nullptr) && (configuration_.callback_parameter != nullptr))
        {
            int rec_sec = 0;
            fastcdr::FastBuffer read_buffer(const_cast<char*>(reinterpret_cast<const char*>(&receive_buffer[1])), 4);
            fastcdr::Cdr read_cdr(read_buffer, fastcdr::Cdr::Endianness::LITTLE_ENDIANNESS,
                    fastcdr::CdrVersion::DDS_CDR);
            read_cdr >> rec_sec;

            fastdds::rtps::Time_t curr_time;
            fastdds::rtps::Time_t::now(curr_time);

            configuration_.callback(configuration_.callback_parameter, rec_sec, curr_time.seconds(),
                    receive_buffer_size);
        }

        receive_buffer_size -= 5;
        next_receiver->OnDataReceived(receive_buffer + 5, receive_buffer_size, local_locator, remote_locator);
    }
    else
    {
        next_receiver->OnDataReceived(receive_buffer, receive_buffer_size, local_locator, remote_locator);
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
