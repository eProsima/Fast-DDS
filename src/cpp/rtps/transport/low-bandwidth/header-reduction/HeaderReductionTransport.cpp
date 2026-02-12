/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include "HeaderReductionTransport.hpp"

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>

#include "HeaderReductionImpl.hpp"
#include "../utilities.hpp"

#include <stdio.h>


namespace eprosima {
namespace fastdds {
namespace rtps {

#if HEAD_REDUCTION_DEBUG_DUMP

static uint16_t __dump_id = 0;

static void __dump_packet(
        FILE* f,
        const Locator_t& from,
        const Locator_t& to,
        const octet* buf,
        const uint32_t len)
{

    if (f != NULL && from.kind == 1 && to.kind == 1)
    {
        uint32_t ipSize = len + 28;
        uint32_t udpSize = len + 8;

        __dump_id++;
        if (__dump_id == 0)
        {
            __dump_id++;
        }

        // IP header
        fprintf(f, "000000 45 00 %02x %02x %02x %02x 00 00 11 11 00 00\n", (ipSize >> 8) & 0xFF, ipSize & 0xFF,
                (__dump_id >> 8) & 0xFF, __dump_id & 0xFF);
        if (IsAddressDefined(from))
        {
            fprintf(f, "00000c %02x %02x %02x %02x\n", from.address[12], from.address[13], from.address[14],
                    from.address[15]);
        }
        else
        {
            fprintf(f, "00000c %02x %02x %02x %02x\n", 192, 168, 1, 254);
        }
        if (IsAddressDefined(to))
        {
            fprintf(f, "000010 %02x %02x %02x %02x\n", to.address[12], to.address[13], to.address[14], to.address[15]);
        }
        else
        {
            fprintf(f, "000010 %02x %02x %02x %02x\n", 192, 168, 1, 254);
        }

        // UDP header
        fprintf(f, "000014 %02x %02x %02x %02x\n", (from.port >> 8) & 0xFF, from.port & 0xFF, (to.port >> 8) & 0xFF,
                to.port & 0xFF);
        fprintf(f, "000018 %02x %02x 00 00", (udpSize >> 8) & 0xFF, udpSize & 0xFF);

        // Data
        for (uint32_t i = 0; i < len; i++)
        {
            if ((i & 15) == 0)
            {
                fprintf(f, "\n%06x", i + 28);
            }
            fprintf(f, " %02x", buf[i]);
        }

        fprintf(f, "\n\n");
    }
}

#endif // if HEAD_REDUCTION_DEBUG_DUMP

static std::vector<std::string> split(
        const std::string& str,
        const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos)
        {
            pos = str.length();
        }
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty())
        {
            tokens.push_back(token);
        }
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

// ------------------------- Descriptor
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

HeaderReductionTransportDescriptor::HeaderReductionTransportDescriptor(
        std::shared_ptr<TransportDescriptorInterface> low_level)
    : ChainingTransportDescriptor(low_level)
{
}

HeaderReductionTransportDescriptor::HeaderReductionTransportDescriptor(
        const HeaderReductionTransportDescriptor& t)
    : ChainingTransportDescriptor(t)
{
}

TransportInterface* HeaderReductionTransportDescriptor::create_transport() const
{
    return new HeaderReductionTransport(*this);
}

// ------------------------- Descriptor
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

HeaderReductionTransport::HeaderReductionTransport(
        const HeaderReductionTransportDescriptor& descriptor)
    : ChainingTransport(descriptor)
    , buffer_size_(descriptor.max_message_size())
    , configuration_(descriptor)
{
}

HeaderReductionTransport::~HeaderReductionTransport()
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

bool HeaderReductionTransport::init(
        const fastdds::rtps::PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    bool ret_val = ChainingTransport::init(properties, max_msg_size_no_frag);
    if (ret_val)
    {
        compress_buffer_ = (fastdds::rtps::octet*)calloc(buffer_size_, sizeof(fastdds::rtps::octet));
        assembly_buffer_ = (fastdds::rtps::octet*)calloc(buffer_size_, sizeof(fastdds::rtps::octet));
        if (nullptr == compress_buffer_ || nullptr == assembly_buffer_)
        {
            ret_val = false;
        }
        else
        {
            options_.reset(new HeaderReductionOptions HRCONFIG_RTPS_PACKET_DEFAULT);
            if (properties != nullptr)
            {
                // Read properties
                fastdds::rtps::PropertyPolicy props =
                        fastdds::rtps::PropertyPolicyHelper::get_properties_with_prefix(*properties,
                                "rtps.header_reduction.");
                const std::string* property_value;

#if HEAD_REDUCTION_DEBUG_DUMP
                property_value = PropertyPolicyHelper::find_property(props, "dump_file");
                if (property_value != NULL)
                {
                    dump_file_ = fopen(property_value->c_str(), "a");
                    std::string low = (*property_value) + ".low";
                    dump_file_low_ = fopen(low.c_str(), "a");
                }
#endif // if HEAD_REDUCTION_DEBUG_DUMP

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "remove_protocol");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->rtps_header.eliminate_protocol = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.remove_protocol");
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "remove_version");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->rtps_header.eliminate_version = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.remove_version");
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "remove_vendor_id");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->rtps_header.eliminate_vendorId = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.remove_vendor_id");
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props, "compress_guid_prefix");
                if (property_value != NULL)
                {
                    auto values = split(*property_value, ",");
                    if ((values.size() < 3) || !read_uint32(options_->rtps_header.reduce_guidPrefix[0], values[0])
                            || !read_uint32(options_->rtps_header.reduce_guidPrefix[1], values[1])
                            || !read_uint32(options_->rtps_header.reduce_guidPrefix[2], values[2])
                            || options_->rtps_header.reduce_guidPrefix[0] > 32 ||
                            options_->rtps_header.reduce_guidPrefix[1] > 32 ||
                            options_->rtps_header.reduce_guidPrefix[2] > 32 )
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.compress_guid_prefix");
                        options_->rtps_header.reduce_guidPrefix[0] = 32;
                        options_->rtps_header.reduce_guidPrefix[1] = 32;
                        options_->rtps_header.reduce_guidPrefix[2] = 32;
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.combine_id_and_flags");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->submessage_header.combine_submessageId_with_flags = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.submessage.combine_id_and_flags");
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.remove_extra_flags");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->submessage_body.eliminate_extraFlags = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.submessage.remove_extra_flags");
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.compress_entitiy_ids");
                if (property_value != NULL)
                {
                    auto values = split(*property_value, ",");
                    if ((values.size() < 2) || !read_uint32(options_->submessage_body.reduce_entitiesId[
                                0], values[0])
                            || !read_uint32(options_->submessage_body.reduce_entitiesId[
                                1], values[1])
                            || options_->submessage_body.reduce_entitiesId[0] > 32 ||
                            options_->submessage_body.reduce_entitiesId[1] > 32 ||
                            options_->submessage_body.reduce_entitiesId[0] < 8 ||
                            options_->submessage_body.reduce_entitiesId[1] < 8)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.submessage.compress_entitiy_ids");
                        options_->submessage_body.reduce_entitiesId[0] = 32;
                        options_->submessage_body.reduce_entitiesId[1] = 32;
                    }
                }

                property_value = fastdds::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.compress_sequence_number");
                if (property_value != NULL)
                {
                    if (!read_uint32(options_->submessage_body.reduce_sequenceNumber, *property_value)
                            || options_->submessage_body.reduce_sequenceNumber > 64 ||
                            options_->submessage_body.reduce_sequenceNumber < 16)
                    {
                        EPROSIMA_LOG_WARNING(RTPS_HEADERREDUCTION_TRANSPORT,
                                "Invalid value in rtps.header_reduction.submessage.compress_sequence_number");
                        options_->submessage_body.reduce_sequenceNumber = 64;
                    }
                }
            }
        }
    }

    return ret_val;
}

bool HeaderReductionTransport::send(
        fastdds::rtps::SenderResource* low_sender_resource,
        const std::vector<NetworkBuffer>& buffers,
        uint32_t send_buffer_size,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& timeout)
{
    return send_w_priority(low_sender_resource, buffers, send_buffer_size, destination_locators_begin,
                   destination_locators_end, timeout, 0);
}

bool HeaderReductionTransport::send_w_priority(
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

#if HEAD_REDUCTION_DEBUG_DUMP
    __dump_packet(dump_file_, localLocator, remoteLocator, send_buffer, send_buffer_size);
#endif // if HEAD_REDUCTION_DEBUG_DUMP

    uint32_t send_size = send_buffer_size;
    bool ret_val = HeaderReduction_Reduce(compress_buffer_, send_buffer, send_size, *options_);

#if HEAD_REDUCTION_DEBUG_DUMP
    __dump_packet(dump_file_low_, localLocator, remoteLocator, compress_buffer_, send_size);
#endif // if HEAD_REDUCTION_DEBUG_DUMP

    NetworkBuffer network_buffer;
    network_buffer.buffer = compress_buffer_;
    network_buffer.size = send_size;
    return ret_val ? low_sender_resource->send({network_buffer}, send_size, destination_locators_begin,
                   destination_locators_end, timeout, transport_priority) : false;
}

void HeaderReductionTransport::receive(
        TransportReceiverInterface* next_receiver,
        const fastdds::rtps::octet* receive_buffer,
        uint32_t receive_buffer_size,
        const fastdds::rtps::Locator_t& local_locator,
        const fastdds::rtps::Locator_t& remote_locator)
{
    bool success = false;
    fastdds::rtps::octet* recv_buffer = (fastdds::rtps::octet*)receive_buffer;

    {
        std::lock_guard<std::mutex> scopedLock(compress_buffer_mutex_);

        memcpy(compress_buffer_, recv_buffer, receive_buffer_size);

#if HEAD_REDUCTION_DEBUG_DUMP
        __dump_packet(dump_file_low_, remote_locator, local_locator, compress_buffer_, receive_buffer_size);
#endif // if HEAD_REDUCTION_DEBUG_DUMP

        success = HeaderReduction_Recover(recv_buffer, compress_buffer_, receive_buffer_size, *options_);
        if (success)
        {
#if HEAD_REDUCTION_DEBUG_DUMP
            __dump_packet(dump_file_, remote_locator, local_locator, recv_buffer, receive_buffer_size);
#endif // if HEAD_REDUCTION_DEBUG_DUMP
        }
        else
        {
            success = 0 == strncmp((const char*)receive_buffer, "RTPS", 4);
        }
    }

    if (success)
    {
        next_receiver->OnDataReceived(recv_buffer, receive_buffer_size, local_locator, remote_locator);
    }

}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
