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

#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransport.h>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastrtps/utils/StringMatching.h>

#include "HeaderReductionImpl.h"

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

#endif

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
    , buffer_size_(descriptor.max_message_size() )
    , options_()
    , compress_buffer_mutex_()
    , compress_buffer_(nullptr)
    , configuration_(descriptor)
#if HEAD_REDUCTION_DEBUG_DUMP
    , dump_file_(nullptr)
    , dump_file_low_(nullptr)
#endif
{
}

HeaderReductionTransport::~HeaderReductionTransport()
{
    if (compress_buffer_ != nullptr)
    {
        free(compress_buffer_);
        compress_buffer_ = nullptr;
    }
}

bool HeaderReductionTransport::init(
        const fastrtps::rtps::PropertyPolicy* properties)
{
    bool ret_val = ChainingTransport::init(properties);
    if (ret_val)
    {
        compress_buffer_ = (fastrtps::rtps::octet*) malloc(buffer_size_);
        if (compress_buffer_ == nullptr)
        {
            ret_val = false;
        }
        else
        {
            options_.reset(new HeaderReductionOptions HRCONFIG_RTPS_PACKET_DEFAULT);
            if (properties != nullptr)
            {
                // Read properties
                fastrtps::rtps::PropertyPolicy props = fastrtps::rtps::PropertyPolicyHelper::get_properties_with_prefix(
                    *properties, "rtps.header_reduction.");
                const std::string* property_value;

#if HEAD_REDUCTION_DEBUG_DUMP
                property_value = PropertyPolicyHelper::find_property(props, "dump_file");
                if (property_value != NULL)
                {
                    dump_file_ = fopen(property_value->c_str(), "a");
                    std::string low = (*property_value) + ".low";
                    dump_file_low_ = fopen(low.c_str(), "a");
                }
#endif

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "remove_protocol");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->rtps_header.eliminate_protocol = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.remove_protocol\n");
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "remove_version");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->rtps_header.eliminate_version = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.remove_version\n");
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "remove_vendor_id");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->rtps_header.eliminate_vendorId = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.remove_vendor_id\n");
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props, "compress_guid_prefix");
                if (property_value != NULL)
                {
                    auto values = fastrtps::rtps::StringMatching::split(*property_value, ",");
                    if ( (values.size() < 3)
                            || !fastrtps::rtps::StringMatching::readUint32(options_->rtps_header.reduce_guidPrefix[0],
                            values[0])
                            || !fastrtps::rtps::StringMatching::readUint32(options_->rtps_header.reduce_guidPrefix[1],
                            values[1])
                            || !fastrtps::rtps::StringMatching::readUint32(options_->rtps_header.reduce_guidPrefix[2],
                            values[2])
                            || options_->rtps_header.reduce_guidPrefix[0] > 32
                            || options_->rtps_header.reduce_guidPrefix[1] > 32
                            || options_->rtps_header.reduce_guidPrefix[2] > 32 )
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.compress_guid_prefix\n");
                        options_->rtps_header.reduce_guidPrefix[0] = 32;
                        options_->rtps_header.reduce_guidPrefix[1] = 32;
                        options_->rtps_header.reduce_guidPrefix[2] = 32;
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.combine_id_and_flags");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->submessage_header.combine_submessageId_with_flags = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.submessage.combine_id_and_flags\n");
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.remove_extra_flags");
                if (property_value != NULL)
                {
                    if (property_value->compare("true") == 0)
                    {
                        options_->submessage_body.eliminate_extraFlags = true;
                    }
                    else if (property_value->compare("false") != 0)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.submessage.remove_extra_flags\n");
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.compress_entitiy_ids");
                if (property_value != NULL)
                {
                    auto values = fastrtps::rtps::StringMatching::split(*property_value, ",");
                    if ((values.size() < 2)
                            || !fastrtps::rtps::StringMatching::readUint32(options_->submessage_body.reduce_entitiesId[0
                            ], values[0])
                            || !fastrtps::rtps::StringMatching::readUint32(options_->submessage_body.reduce_entitiesId[1
                            ], values[1])
                            || options_->submessage_body.reduce_entitiesId[0] > 32
                            || options_->submessage_body.reduce_entitiesId[1] > 32
                            || options_->submessage_body.reduce_entitiesId[0] < 8
                            || options_->submessage_body.reduce_entitiesId[1] < 8)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.submessage.compress_entitiy_ids\n");
                        options_->submessage_body.reduce_entitiesId[0] = 32;
                        options_->submessage_body.reduce_entitiesId[1] = 32;
                    }
                }

                property_value = fastrtps::rtps::PropertyPolicyHelper::find_property(props,
                                "submessage.compress_sequence_number");
                if (property_value != NULL)
                {
                    if (!fastrtps::rtps::StringMatching::readUint32(options_->submessage_body.reduce_sequenceNumber,
                            *property_value)
                            || options_->submessage_body.reduce_sequenceNumber > 64
                            || options_->submessage_body.reduce_sequenceNumber < 16)
                    {
                        //printf("ERROR: Invalid value in rtps.header_reduction.submessage.compress_sequence_number\n");
                        options_->submessage_body.reduce_sequenceNumber = 64;
                    }
                }
            }
        }
    }

    return ret_val;
}

bool HeaderReductionTransport::send(
        fastrtps::rtps::SenderResource* low_sender_resource,
        const fastrtps::rtps::octet* send_buffer,
        uint32_t send_buffer_size,
        fastrtps::rtps::LocatorsIterator* destination_locators_begin,
        fastrtps::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& timeout)
{
    std::unique_lock<std::mutex> scopedLock(compress_buffer_mutex_);

#if HEAD_REDUCTION_DEBUG_DUMP
    __dump_packet(dump_file_, localLocator, remoteLocator, sendBuffer, sendBufferSize);
#endif

    uint32_t send_size = send_buffer_size;
    bool ret_val = HeaderReduction_Reduce(compress_buffer_, send_buffer, send_size, *options_);

#if HEAD_REDUCTION_DEBUG_DUMP
    __dump_packet(dump_file_low_, localLocator, remoteLocator, compress_buffer_, send_size);
#endif

    return ret_val ? low_sender_resource->send(compress_buffer_, send_size,
                   destination_locators_begin, destination_locators_end, timeout) : false;
}

void HeaderReductionTransport::receive(
        TransportReceiverInterface* next_receiver,
        const fastrtps::rtps::octet* receive_buffer,
        uint32_t receive_buffer_size,
        const fastrtps::rtps::Locator_t& local_locator,
        const fastrtps::rtps::Locator_t& remote_locator)
{
    bool success = false;
    fastrtps::rtps::octet* recv_buffer = (fastrtps::rtps::octet*)receive_buffer;

    {
        std::unique_lock<std::mutex> scopedLock(compress_buffer_mutex_);

        memcpy(compress_buffer_, recv_buffer, receive_buffer_size);

#if HEAD_REDUCTION_DEBUG_DUMP
        __dump_packet(dump_file_low_, remote_locator, local_locator, compress_buffer_, receive_buffer_size);
#endif

        if ((success = HeaderReduction_Recover(recv_buffer, compress_buffer_, receive_buffer_size, *options_)))
        {
#if HEAD_REDUCTION_DEBUG_DUMP
            __dump_packet(dump_file_, remote_locator, local_locator, recv_buffer, receive_buffer_size);
#endif
        }
    }

    if (success)
    {
        next_receiver->OnDataReceived(recv_buffer, receive_buffer_size, local_locator, remote_locator);
    }

}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
