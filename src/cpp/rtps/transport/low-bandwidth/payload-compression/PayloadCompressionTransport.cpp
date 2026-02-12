/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include "PayloadCompressionTransport.hpp"

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>

#include "PayloadCompressionImpl.hpp"
#include "../utilities.hpp"

#if HAVE_ZLIB || HAVE_BZIP2

namespace eprosima {
namespace fastdds {
namespace rtps {

// ------------------------- Utilities
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

static PayloadCompressionLibrary read_library_kind(
        const std::string* prop)
{
    PayloadCompressionLibrary ret_val = COMPRESS_LIB_ZLIB;

    if (prop->compare("BZIP2") == 0)
    {
        ret_val = COMPRESS_LIB_BZIP2;
    }
    else if (prop->compare("AUTOMATIC") == 0)
    {
        ret_val = COMPRESS_LIB_AUTO;
    }
    else if (prop->compare("ZLIB") != 0)
    {
        EPROSIMA_LOG_WARNING(PAYLOAD_COMPRESSION_TRANSPORT, "Unknown compression library "
                << *prop << " declared in xml, using ZLIB");
    }

    return ret_val;
}

// ------------------------- Utilities
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// ------------------------- Descriptor
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

PayloadCompressionTransportDescriptor::PayloadCompressionTransportDescriptor(
        std::shared_ptr<TransportDescriptorInterface> low_level)
    : ChainingTransportDescriptor(low_level)
{
}

PayloadCompressionTransportDescriptor::PayloadCompressionTransportDescriptor(
        const PayloadCompressionTransportDescriptor& t)
    : ChainingTransportDescriptor(t)
{
}

TransportInterface* PayloadCompressionTransportDescriptor::create_transport() const
{
    return new PayloadCompressionTransport(*this);
}

// ------------------------- Descriptor
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

PayloadCompressionTransport::PayloadCompressionTransport(
        const PayloadCompressionTransportDescriptor& descriptor)
    : ChainingTransport(descriptor)
    , buffer_size_(descriptor.max_message_size())
    , configuration_(descriptor)
{
    PayloadCompression_init();
}

PayloadCompressionTransport::~PayloadCompressionTransport()
{
    if (nullptr != compress_buffer_)
    {
        free(compress_buffer_);
        compress_buffer_ = nullptr;
        compress_buffer_len_ = 0;
    }

    if (nullptr != assembly_buffer_)
    {
        free(assembly_buffer_);
        assembly_buffer_ = nullptr;
    }
}

bool PayloadCompressionTransport::init(
        const fastdds::rtps::PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    bool ret_val = ChainingTransport::init(properties, max_msg_size_no_frag);
    if (ret_val)
    {
        // Prepare options
        options_.reset(new PayloadCompressionOptions COMPRESSION_OPTIONS_DEFAULT);
        if (properties != nullptr)
        {
            // Read properties
            fastdds::rtps::PropertyPolicy props = fastdds::rtps::PropertyPolicyHelper::get_properties_with_prefix(
                *properties,
                "rtps.payload_compression.");
            const std::string* property_value;
            uint32_t level;

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "compression_library");
            if (property_value != nullptr)
            {
                options_->small_compression.library = read_library_kind(property_value);
                options_->medium_compression.library = options_->small_compression.library;
                options_->large_compression.library = options_->small_compression.library;
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "compression_level");
            if (property_value != nullptr)
            {
                if (read_uint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->small_compression.level = level;
                    options_->medium_compression.level = level;
                    options_->large_compression.level = level;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_PAYLOADCOMPRESSION_TRANSPORT,
                            "Invalid value in rtps.payload_compression.compression_level, using default");
                }
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_library.small_packets");
            if (property_value != nullptr)
            {
                options_->small_compression.library = read_library_kind(property_value);
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_level.small_packets");
            if (property_value != nullptr)
            {
                if (read_uint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->small_compression.level = level;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_PAYLOADCOMPRESSION_TRANSPORT,
                            "Invalid value in rtps.payload_compression.compression_level.small_packets, using default");
                }
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_library.medium_packets");
            if (property_value != nullptr)
            {
                options_->medium_compression.library = read_library_kind(property_value);
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_level.medium_packets");
            if (property_value != nullptr)
            {
                if (read_uint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->medium_compression.level = level;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_PAYLOADCOMPRESSION_TRANSPORT,
                            "Invalid value in rtps.payload_compression.compression_level.medium_packets, using default");
                }
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_library.large_packets");
            if (property_value != nullptr)
            {
                options_->large_compression.library = read_library_kind(property_value);
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_level.large_packets");
            if (property_value != nullptr)
            {
                if (read_uint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->large_compression.level = level;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_PAYLOADCOMPRESSION_TRANSPORT,
                            "Invalid value in rtps.payload_compression.compression_level.large_packets, using default");
                }
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "low_mark");
            if (property_value != nullptr)
            {
                if (!read_uint32(options_->low_mark, *property_value))
                {
                    EPROSIMA_LOG_WARNING(RTPS_PAYLOADCOMPRESSION_TRANSPORT,
                            "Invalid value in rtps.payload_compression.low_mark, using default");
                }
            }

            property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "high_mark");
            if (property_value != nullptr)
            {
                if (!read_uint32(options_->high_mark, *property_value))
                {
                    EPROSIMA_LOG_WARNING(RTPS_PAYLOADCOMPRESSION_TRANSPORT,
                            "Invalid value in rtps.payload_compression.high_mark, using default");
                }
            }
        }

        // Create compression buffer
        bool has_auto = options_->small_compression.library == COMPRESS_LIB_AUTO
                || options_->medium_compression.library == COMPRESS_LIB_AUTO
                || options_->large_compression.library == COMPRESS_LIB_AUTO;
        compress_buffer_len_ = PayloadCompression_buffer_alloc(compress_buffer_, buffer_size_, has_auto);
        if ((nullptr == compress_buffer_) || (compress_buffer_len_ < 1))
        {
            ret_val = false;
        }

        assembly_buffer_ = (fastdds::rtps::octet*)calloc(buffer_size_, sizeof(fastdds::rtps::octet));

        if (nullptr == assembly_buffer_)
        {
            ret_val = false;
        }
    }

    return ret_val;
}

bool PayloadCompressionTransport::send(
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

bool PayloadCompressionTransport::send_w_priority(
        fastdds::rtps::SenderResource* low_sender_resource,
        const std::vector<NetworkBuffer>& buffers,
        uint32_t send_buffer_size,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& timeout,
        int32_t transport_priority)
{
    std::lock_guard<std::mutex> scopedLock(compress_buffer_mutex_);

    const fastdds::rtps::octet* send_buffer {nullptr};

    if (1 == buffers.size())
    {
        assert(send_buffer_size == buffers.at(0).size);
        send_buffer = reinterpret_cast<const fastdds::rtps::octet*>(buffers.at(0).buffer);
    }
    else //Assembly buffers in one.
    {
        size_t pos {0};
        for (size_t count_of_buffers {0}; count_of_buffers < buffers.size(); ++count_of_buffers)
        {
            memcpy(&assembly_buffer_[pos], buffers.at(count_of_buffers).buffer, buffers.at(count_of_buffers).size);
            pos += buffers.at(count_of_buffers).size;
        }
        send_buffer = assembly_buffer_;
    }

    uint32_t send_size = compress_buffer_len_;
    const PayloadCompressionLevel& level =
            send_buffer_size < options_->low_mark ? options_->small_compression :
            send_buffer_size < options_->high_mark ? options_->medium_compression : options_->large_compression;
    bool ret_val = PayloadCompression_compress(compress_buffer_, send_size, send_buffer, send_buffer_size, level);

    if (ret_val)
    {
        if (send_size < send_buffer_size)
        {
            NetworkBuffer network_buffer;
            network_buffer.buffer = compress_buffer_;
            network_buffer.size = send_size;
            ret_val = low_sender_resource->send({network_buffer}, send_size,
                            destination_locators_begin, destination_locators_end, timeout, transport_priority);
        }
        else
        {
            if (strncmp((const char*) send_buffer, "RTPS", 4) == 0)
            {
                ret_val = low_sender_resource->send(buffers, send_buffer_size,
                                destination_locators_begin, destination_locators_end, timeout, transport_priority);
            }
            else
            {
                compress_buffer_[0] = (fastdds::rtps::octet) 'R';
                memcpy(&compress_buffer_[1], send_buffer, send_buffer_size);
                NetworkBuffer network_buffer;
                network_buffer.buffer = compress_buffer_;
                network_buffer.size = send_buffer_size + 1;
                ret_val = low_sender_resource->send({network_buffer}, send_buffer_size + 1,
                                destination_locators_begin, destination_locators_end, timeout, transport_priority);
            }
        }
    }
    return ret_val;
}

void PayloadCompressionTransport::receive(
        TransportReceiverInterface* next_receiver,
        const fastdds::rtps::octet* receive_buffer,
        uint32_t receive_buffer_size,
        const fastdds::rtps::Locator_t& local_locator,
        const fastdds::rtps::Locator_t& remote_locator)
{
    bool success = false;
    fastdds::rtps::octet* recv_buffer = (fastdds::rtps::octet*)receive_buffer;
    uint32_t recv_size = compress_buffer_len_;

    if (receive_buffer_size > 0)
    {
        std::lock_guard<std::mutex> scopedLock(compress_buffer_mutex_);

        if (PayloadCompression_uncompress(compress_buffer_, recv_size, recv_buffer, receive_buffer_size) == true)
        {
            memcpy(recv_buffer, compress_buffer_, recv_size);
            success = true;
        }
        else
        {
            recv_size = receive_buffer_size;
            if (strncmp((const char*)recv_buffer, "RTPS", 4) != 0)
            {
                --recv_size;
                recv_buffer = (fastdds::rtps::octet*)receive_buffer + 1;
            }

            success = true;
        }
    }

    if (success)
    {
        next_receiver->OnDataReceived(recv_buffer, recv_size, local_locator, remote_locator);
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // if HAVE_ZLIB || HAVE_BZIP2
