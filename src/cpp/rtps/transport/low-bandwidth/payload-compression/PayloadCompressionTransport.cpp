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

#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransport.h>
#include <fastrtps/rtps/attributes/PropertyPolicy.h>
#include <fastrtps/utils/StringMatching.h>

#include "PayloadCompressionImpl.h"

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
        //printf("ERROR: Unknown compression library %s declared in xml\n", prop->c_str());
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
    , options_()
    , compress_buffer_mutex_()
    , compress_buffer_(nullptr)
    , compress_buffer_len_(0)
    , configuration_(descriptor)
{
}

PayloadCompressionTransport::~PayloadCompressionTransport()
{
    if (compress_buffer_ != nullptr)
    {
        free(compress_buffer_);
        compress_buffer_ = nullptr;
        compress_buffer_len_ = 0;
    }
}

bool PayloadCompressionTransport::init(
        const fastrtps::rtps::PropertyPolicy* properties)
{
    bool ret_val = ChainingTransport::init(properties);
    if (ret_val)
    {
        // Prepare options
        options_.reset(new PayloadCompressionOptions COMPRESSION_OPTIONS_DEFAULT);
        if (properties != nullptr)
        {
            // Read properties
            fastrtps::rtps::PropertyPolicy props = fastrtps::rtps::PropertyPolicyHelper::get_properties_with_prefix(
                *properties,
                "rtps.payload_compression.");
            const std::string* property_value;
            uint32_t level;

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "compression_library");
            if (property_value != nullptr)
            {
                options_->small_compression.library = read_library_kind(property_value);
                options_->medium_compression.library = options_->small_compression.library;
                options_->large_compression.library = options_->small_compression.library;
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "compression_level");
            if (property_value != nullptr)
            {
                if (fastrtps::rtps::StringMatching::readUint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->small_compression.level = level;
                    options_->medium_compression.level = level;
                    options_->large_compression.level = level;
                }
                else
                {
                    //printf("ERROR: Invalid value in rtps.payload_compression.compression_level\n");
                }
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_library.small_packets");
            if (property_value != nullptr)
            {
                options_->small_compression.library = read_library_kind(property_value);
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_level.small_packets");
            if (property_value != nullptr)
            {
                if (fastrtps::rtps::StringMatching::readUint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->small_compression.level = level;
                }
                else
                {
                    //printf("ERROR: Invalid value in rtps.payload_compression.compression_level.small_packets\n");
                }
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_library.medium_packets");
            if (property_value != nullptr)
            {
                options_->medium_compression.library = read_library_kind(property_value);
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_level.medium_packets");
            if (property_value != nullptr)
            {
                if (fastrtps::rtps::StringMatching::readUint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->medium_compression.level = level;
                }
                else
                {
                    //printf("ERROR: Invalid value in rtps.payload_compression.compression_level.medium_packets\n");
                }
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_library.large_packets");
            if (property_value != nullptr)
            {
                options_->large_compression.library = read_library_kind(property_value);
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                            "compression_level.large_packets");
            if (property_value != nullptr)
            {
                if (fastrtps::rtps::StringMatching::readUint32(level, *property_value) && level >= 1 && level <= 9)
                {
                    options_->large_compression.level = level;
                }
                else
                {
                    //printf("ERROR: Invalid value in rtps.payload_compression.compression_level.large_packets\n");
                }
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "low_mark");
            if (property_value != nullptr)
            {
                if (!fastrtps::rtps::StringMatching::readUint32(options_->low_mark, *property_value))
                {
                    //printf("ERROR: Invalid value in rtps.payload_compression.low_mark\n");
                }
            }

            property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "high_mark");
            if (property_value != nullptr)
            {
                if (!fastrtps::rtps::StringMatching::readUint32(options_->high_mark, *property_value))
                {
                    //printf("ERROR: Invalid value in rtps.payload_compression.low_mark\n");
                }
            }
        }

        // Create compression buffer
        bool has_auto = options_->small_compression.library == COMPRESS_LIB_AUTO
                || options_->medium_compression.library == COMPRESS_LIB_AUTO
                || options_->large_compression.library == COMPRESS_LIB_AUTO;
        compress_buffer_len_ = PayloadCompression_buffer_alloc(compress_buffer_, buffer_size_, has_auto);
        if ( (compress_buffer_ == nullptr) || (compress_buffer_len_ < 1) )
        {
            ret_val = false;
        }
    }

    return ret_val;
}

bool PayloadCompressionTransport::send(
        fastrtps::rtps::SenderResource* low_sender_resource,
        const fastrtps::rtps::octet* send_buffer,
        uint32_t send_buffer_size,
        const fastrtps::rtps::Locator_t& remote_locator,
        const std::chrono::microseconds& timeout)
{
    std::unique_lock<std::mutex> scopedLock(compress_buffer_mutex_);
    uint32_t send_size = compress_buffer_len_;
    const PayloadCompressionLevel& level =
            send_buffer_size < options_->low_mark ? options_->small_compression :
            send_buffer_size < options_->high_mark ? options_->medium_compression : options_->large_compression;
    bool ret_val = PayloadCompression_compress(compress_buffer_, send_size, send_buffer, send_buffer_size, level);

    if (ret_val)
    {
        if (send_size < send_buffer_size)
        {
            ret_val = low_sender_resource->send(compress_buffer_, send_size, remote_locator, timeout);
        }
        else
        {
            if (strncmp((const char*) send_buffer, "RTPS", 4) == 0)
            {
                ret_val = low_sender_resource->send(send_buffer, send_buffer_size, remote_locator, timeout);
            }
            else
            {
                compress_buffer_[0] = (fastrtps::rtps::octet) 'R';
                memcpy(&compress_buffer_[1], send_buffer, send_buffer_size);
                ret_val = low_sender_resource->send(compress_buffer_, send_buffer_size + 1, remote_locator, timeout);
            }
        }
    }
    return ret_val;
}

void PayloadCompressionTransport::receive(
        TransportReceiverInterface* next_receiver,
        const fastrtps::rtps::octet* receive_buffer,
        uint32_t receive_buffer_size,
        const fastrtps::rtps::Locator_t& local_locator,
        const fastrtps::rtps::Locator_t& remote_locator)
{
    bool success = false;
    fastrtps::rtps::octet* recv_buffer = (fastrtps::rtps::octet*)receive_buffer;
    uint32_t recv_size = compress_buffer_len_;

    if (receive_buffer_size > 0)
    {
        std::unique_lock<std::mutex> scopedLock(compress_buffer_mutex_);

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
                recv_buffer = (fastrtps::rtps::octet*)receive_buffer + 1;
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
} // namespace fastrtps
} // namespace eprosima

#endif
