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

#include <array>
#include <cstdint>
#include <iostream>
#include <set>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/config.hpp>
#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/network/NetworkFactory.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

constexpr size_t max_unicast_locators = 4u;
constexpr size_t max_multicast_locators = 1u;

RTPSParticipantAttributes participant_attributes;
NetworkFactory network {participant_attributes};

/*** Auxiliary functions ***/
inline uint32_t string_cdr_serialized_size(
        const std::string& str)
{
    // Size including NUL char at the end
    uint32_t str_siz = static_cast<uint32_t>(str.size()) + 1;
    // Align to next 4 byte
    str_siz = (str_siz + 3u) & ~3u;
    // str_length + str_data
    return 4u + str_siz;
}

uint32_t manual_content_filter_cdr_serialized_size(
        const std::string& content_filtered_topic_name,
        const std::string& related_topic_name,
        const std::string& filter_class_name,
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters)
{
    uint32_t ret_val = 0;

    // p_id + p_length
    ret_val = 2 + 2;
    // content_filtered_topic_name
    ret_val += string_cdr_serialized_size(content_filtered_topic_name);
    // related_topic_name
    ret_val += string_cdr_serialized_size(related_topic_name);
    // filter_class_name
    ret_val += string_cdr_serialized_size(filter_class_name);

    // filter_expression
    // str_len + null_char + str_data
    ret_val += 4 + 1 + static_cast<uint32_t>(filter_expression.size());
    // align
    ret_val = (ret_val + 3) & ~3;

    // expression_parameters
    // sequence length
    ret_val += 4;
    // Add all parameters
    for (const std::string& param : expression_parameters)
    {
        ret_val += string_cdr_serialized_size(param);
    }

    return ret_val;
}

void assert_is_empty_content_filter(
        const fastdds::rtps::ContentFilterProperty& filter_property)
{
    ASSERT_EQ("", filter_property.content_filtered_topic_name.to_string());
    ASSERT_EQ("", filter_property.related_topic_name.to_string());
    ASSERT_EQ("", filter_property.filter_class_name.to_string());
    ASSERT_EQ("", filter_property.filter_expression);
    ASSERT_EQ(0u, filter_property.expression_parameters.size());
}

TEST(BuiltinDataSerializationTests, ok_with_defaults)
{
    {
        WriterProxyData in(max_unicast_locators, max_multicast_locators);
        WriterProxyData out(max_unicast_locators, max_multicast_locators);

        // Topic and type name cannot be empty
        in.topic_name = "TEST";
        in.type_name = "TestType";

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_TRUE(in.write_to_cdr_message(&msg, true));

        // Perform deserialization
        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));
        // EXPECT_EQ(in, out);
    }

    {
        ReaderProxyData in(max_unicast_locators, max_multicast_locators);
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Topic and type name cannot be empty
        in.topic_name = "TEST";
        in.type_name = "TestType";

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_TRUE(in.write_to_cdr_message(&msg, true));

        // Perform deserialization
        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));
    }
}

TEST(BuiltinDataSerializationTests, msg_with_product_version)
{
    /* Convenient functions to group code */
    auto participant_read = [](octet* buffer, uint32_t buffer_length, ParticipantProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, true, network, false,
                        c_VendorId_eProsima)));
            };

    auto update_cache_change =
            [](CacheChange_t& change, octet* buffer, uint32_t buffer_length, uint32_t qos_size) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_TRUE(fastdds::dds::ParameterList::updateCacheChangeFromInlineQos(change, &msg, qos_size));
            };

    // PID_PRODUCT_VERSION
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_VENDORID
            0x16, 0x00, 0x04, 0x00,
            0x01, 0x0f, 0x00, 0x00,
            // PID_PRODUCT_VERSION
            0x00, 0x80, 0x04, 0x00,
            FASTDDS_VERSION_MAJOR, FASTDDS_VERSION_MINOR, FASTDDS_VERSION_MICRO, 0,
            // PID_SENTINEL
            0x01, 0x00, 0x00, 0x00
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);
        EXPECT_EQ(FASTDDS_VERSION_MAJOR, participant_pdata.product_version.major);
        EXPECT_EQ(FASTDDS_VERSION_MINOR, participant_pdata.product_version.minor);
        EXPECT_EQ(FASTDDS_VERSION_MICRO, participant_pdata.product_version.patch);
        EXPECT_EQ(0, participant_pdata.product_version.tweak);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }
}

TEST(BuiltinDataSerializationTests, msg_without_datasharing)
{
    {
        uint8_t data_r_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        out.read_from_cdr_message(&msg);
        ASSERT_EQ(out.data_sharing.kind(), dds::OFF);
    }

    {
        uint8_t data_w_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00

        };

        CDRMessage_t msg(0);
        msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        out.read_from_cdr_message(&msg);
        ASSERT_EQ(out.data_sharing.kind(), dds::OFF);
    }
}

TEST(BuiltinDataSerializationTests, msg_with_datasharing)
{
    {
        uint8_t data_r_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            //Data Sharing
            0x06, 0x80, 0x0c, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x9b, 0xf9, 0xbe, 0x1c, 0xb8

        };

        CDRMessage_t msg(0);
        msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        out.read_from_cdr_message(&msg);
        ASSERT_EQ(out.data_sharing.kind(), dds::ON);
    }

    {
        uint8_t data_w_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            //Data Sharing
            0x06, 0x80, 0x0c, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x9b, 0xf9, 0xbe, 0x1c, 0xb8

        };

        CDRMessage_t msg(0);
        msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        out.read_from_cdr_message(&msg);
        ASSERT_EQ(out.data_sharing.kind(), dds::ON);
    }
}


// Regression test for redmine issue #10547.
// Update against OpenDDS 3.27. With this version we can read the remote DATA(w).
TEST(BuiltinDataSerializationTests, interoperability_with_opendds_3_27)
{
    // DATA(w)
    {
        // This was captured with wireshark from OpenDDS iShapes 3.16
        octet data_w_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // Topic name
            0x05, 0x00, 0x0c, 0x00,
            0x07, 0x00, 0x00, 0x00, 0x43, 0x69, 0x72, 0x63, 0x6c, 0x65, 0x00, 0x00,
            // Type name
            0x07, 0x00, 0x10, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
            // Type information
            0x75, 0x00, 0x58, 0x00,
            0x54, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x40, 0x28, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
            0x14, 0x00, 0x00, 0x00, 0xf1, 0x8b, 0x4b, 0x28, 0x4d, 0xe3, 0xa2, 0x4e, 0x5f, 0x86, 0x58, 0x5c,
            0x57, 0x88, 0xf6, 0x00, 0x57, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x40, 0x1c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
            0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Reliability
            0x1a, 0x00, 0x0c, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0xf5, 0x05,
            // Data representation
            0x73, 0x00, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            // Endpoint GUID
            0x5a, 0x00, 0x10, 0x00,
            0x01, 0x03, 0x74, 0x04, 0xf1, 0x0b, 0x6b, 0x16, 0x94, 0x6c, 0x26, 0x73, 0x00, 0x00, 0x00, 0x02,
            // Multicast locator
            0x30, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xe9, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xef, 0xff, 0x00, 0x02,
            // Unicast locator
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x27,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xac, 0x11, 0x00, 0x01,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x0a, 0x05, 0x00, 0x01,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x50, 0x01,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x8b,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
        msg.length = msg.max_size;

        WriterProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg)));
    }

    // DATA(r)
    {
        // This was captured with wireshark from OpenDDS iShapes 3.16
        uint8_t data_r_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // Topic name
            0x05, 0x00, 0x0c, 0x00,
            0x07, 0x00, 0x00, 0x00, 0x43, 0x69, 0x72, 0x63, 0x6c, 0x65, 0x00, 0x00,
            // Type information
            0x75, 0x00, 0x58, 0x00,
            0x54, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x40, 0x28, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
            0x14, 0x00, 0x00, 0x00, 0xf1, 0x8b, 0x4b, 0x28, 0x4d, 0xe3, 0xa2, 0x4e, 0x5f, 0x86, 0x58, 0x5c,
            0x57, 0x88, 0xf6, 0x00, 0x57, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x40, 0x1c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
            0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Type name
            0x07, 0x00, 0x10, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
            // Reliability
            0x1a, 0x00, 0x0c, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f,
            // Data representation
            0x73, 0x00, 0x08, 0x00,
            0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x94, 0xd0, 0x00, 0x00,
            // Endpoint GUID
            0x5a, 0x00, 0x10, 0x00,
            0x01, 0x03, 0x74, 0x04, 0xf1, 0x0b, 0x6b, 0x16, 0x84, 0x3e, 0x9d, 0x2b, 0x00, 0x00, 0x00, 0x07,
            // Multicast locator
            0x30, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xe9, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xef, 0xff, 0x00, 0x02,
            // Unicast locator
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x27,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xac, 0x11, 0x00, 0x01,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x0a, 0x05, 0x00, 0x01,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x50, 0x01,
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x67, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x8b,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg)));
    }
}

// Regression test for redmine issue #10955
TEST(BuiltinDataSerializationTests, ignore_unsupported_type_object)
{
    // DATA(w)
    {
        // This was captured with wireshark from RTI Shapes Demo 5.3.1
        octet data_w_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // Endpoint GUID
            0x5a, 0x00, 0x10, 0x00,
            0xc0, 0xa8, 0x01, 0x3a, 0x00, 0x00, 0x41, 0xa4, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x42,
            // Topic name
            0x05, 0x00, 0x10, 0x00,
            0x0c, 0x00, 0x00, 0x00, 0x72, 0x74, 0x69, 0x2f, 0x64, 0x69, 0x73, 0x74, 0x6c, 0x6f, 0x67, 0x00,
            // Type name
            0x07, 0x00, 0x20, 0x00,
            0x19, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x3a, 0x3a, 0x72, 0x74, 0x69, 0x3a, 0x3a, 0x64, 0x6c,
            0x3a, 0x3a, 0x4c, 0x6f, 0x67, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00, 0x00, 0x00, 0x00,
            // Type object
            0x72, 0x00, 0xfc, 0x04,
            0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x04, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
            0x28, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
            0x04, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x44, 0x4c, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
            0xe8, 0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x41, 0x44, 0x4d, 0x49, 0x4e, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
            0x09, 0x00, 0x00, 0x00, 0xd0, 0x01, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x24, 0x00, 0x00, 0x00, 0x02, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x51, 0x83, 0x23,
            0x55, 0x8c, 0x53, 0x3a, 0x10, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x52,
            0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00,
            0x70, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x2c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xfe, 0x29, 0x56, 0xb1, 0x97, 0x58, 0xdf, 0x3f, 0x0d, 0x00, 0x00, 0x00,
            0x68, 0x6f, 0x73, 0x74, 0x41, 0x6e, 0x64, 0x41, 0x70, 0x70, 0x49, 0x64, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xfe, 0x29, 0x56, 0xb1, 0x97, 0x58, 0xdf, 0x3f, 0x17, 0x00, 0x00, 0x00, 0x6f, 0x72, 0x69, 0x67,
            0x69, 0x6e, 0x61, 0x74, 0x6f, 0x72, 0x48, 0x6f, 0x73, 0x74, 0x41, 0x6e, 0x64, 0x41, 0x70, 0x70,
            0x49, 0x64, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00,
            0x0b, 0x00, 0x00, 0x00, 0x69, 0x6e, 0x76, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xd9, 0xdc, 0x15, 0x0b, 0x91, 0x99, 0x13, 0x0e, 0x0e, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x6d,
            0x61, 0x6e, 0x64, 0x52, 0x65, 0x73, 0x75, 0x6c, 0x74, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xdc, 0x5c, 0x98,
            0xa5, 0x08, 0x32, 0x91, 0x08, 0x00, 0x00, 0x00, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
            0x0e, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x05, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xd9, 0xdc, 0x15, 0x0b, 0x91, 0x99, 0x13, 0x0e, 0x0e, 0x00, 0x00, 0x00,
            0x43, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x52, 0x65, 0x73, 0x75, 0x6c, 0x74, 0x00, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x5f, 0x44, 0x4c, 0x5f, 0x43,
            0x4f, 0x4d, 0x4d, 0x41, 0x4e, 0x44, 0x5f, 0x52, 0x45, 0x53, 0x55, 0x4c, 0x54, 0x5f, 0x4f, 0x4b,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x5f,
            0x44, 0x4c, 0x5f, 0x43, 0x4f, 0x4d, 0x4d, 0x41, 0x4e, 0x44, 0x5f, 0x52, 0x45, 0x53, 0x55, 0x4c,
            0x54, 0x5f, 0x4e, 0x4f, 0x54, 0x5f, 0x53, 0x55, 0x50, 0x50, 0x4f, 0x52, 0x54, 0x45, 0x44, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x52, 0x54, 0x49, 0x5f, 0x44, 0x4c, 0x5f, 0x43,
            0x4f, 0x4d, 0x4d, 0x41, 0x4e, 0x44, 0x5f, 0x52, 0x45, 0x53, 0x55, 0x4c, 0x54, 0x5f, 0x45, 0x52,
            0x52, 0x4f, 0x52, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
            0x00, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00,
            0xe0, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x29, 0x56, 0xb1, 0x97, 0x58, 0xdf, 0x3f,
            0x0d, 0x00, 0x00, 0x00, 0x48, 0x6f, 0x73, 0x74, 0x41, 0x6e, 0x64, 0x41, 0x70, 0x70, 0x49, 0x64,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x64, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x0d, 0x00, 0x00, 0x00,
            0x72, 0x74, 0x70, 0x73, 0x5f, 0x68, 0x6f, 0x73, 0x74, 0x5f, 0x69, 0x64, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x7f, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x00, 0x00,
            0x72, 0x74, 0x70, 0x73, 0x5f, 0x61, 0x70, 0x70, 0x5f, 0x69, 0x64, 0x00, 0x01, 0x7f, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
            0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00,
            0x00, 0x00, 0x04, 0x00, 0x13, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x74, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xdc, 0x5c, 0x98, 0xa5, 0x08, 0x32, 0x91,
            0x16, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x32, 0x30, 0x34, 0x38, 0x5f,
            0x63, 0x68, 0x61, 0x72, 0x61, 0x63, 0x74, 0x65, 0x72, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
            0x64, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x65, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
            0x00, 0x08, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x01, 0x7f, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x8c, 0x51, 0x83, 0x23, 0x55, 0x8c, 0x53, 0x3a, 0x02, 0x7f, 0x00, 0x00,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
        msg.length = msg.max_size;

        WriterProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg)));
    }
}

TEST(BuiltinDataSerializationTests, property_list_with_binary_properties)
{
    octet data_p_buffer[] =
    {
        // Encapsulation
        0x00, 0x03, 0x00, 0x00,

        // PID_PROPERTY_LIST
        0x59, 0, 104, 0,
        // 3 properties
        0x03, 0x00, 0x00, 0x00,
        // key-1
        0x0e, 0x00, 0x00, 0x00,
        0x5f, 0x5f, 0x50, 0x72, 0x6f, 0x63, 0x65, 0x73, 0x73, 0x4e, 0x61, 0x6d, 0x65, 0x00, 0x00, 0x00,
        // value-1
        0x07, 0x00, 0x00, 0x00,
        0x74, 0x61, 0x6c, 0x6b, 0x65, 0x72, 0x00, 0x00,
        // key-2
        0x06, 0x00, 0x00, 0x00,
        0x5f, 0x5f, 0x50, 0x69, 0x64, 0x00, 0x00, 0x00,
        // value-2
        0x05, 0x00, 0x00, 0x00,
        0x32, 0x35, 0x31, 0x39, 0x00, 0x00, 0x00, 0x00,
        // key-3
        0x0b, 0x00, 0x00, 0x00,
        0x5f, 0x5f, 0x48, 0x6f, 0x73, 0x74, 0x6e, 0x61, 0x6d, 0x65, 0x00, 0x00,
        // value-3
        0x11, 0x00, 0x00, 0x00,
        0x6e, 0x6f, 0x6e, 0x5f, 0x77, 0x6f, 0x72, 0x6b, 0x69, 0x6e, 0x67, 0x5f, 0x68, 0x61, 0x73, 0x68,
        0x00, 0x00, 0x00, 0x00,
        // 0 binary properties
        0x00, 0x00, 0x00, 0x00,

        // PID_PROTOCOL_VERSION
        0x15, 0, 4, 0,
        2, 1, 0, 0,

        // PID_VENDORID
        0x16, 0, 4, 0,
        1, 16, 0, 0,

        // PID_PARTICIPANT_LEASE_DURATION
        0x02, 0, 8, 0,
        10, 0, 0, 0, 0, 0, 0, 0,

        // PID_PARTICIPANT_GUID
        0x50, 0, 16, 0,
        1, 16, 54, 83, 136, 247, 149, 252, 47, 105, 174, 141, 0, 0, 1, 193,

        // PID_BUILTIN_ENDPOINT_SET
        0x58, 0, 4, 0,
        63, 12, 0, 0,

        // PID_DOMAIN_ID
        0x0f, 0, 4, 0,
        0, 0, 0, 0,

        // PID_DEFAULT_UNICAST_LOCATOR
        0x31, 0, 24, 0,
        1, 0, 0, 0, 68, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 31, 133, 54,

        // PID_METATRAFFIC_UNICAST_LOCATOR
        0x32, 0, 24, 0,
        1, 0, 0, 0, 68, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 31, 133, 54,

        // PID_SENTINEL
        0x01, 0, 0, 0
    };

    CDRMessage_t msg(0);
    msg.init(data_p_buffer, static_cast<uint32_t>(sizeof(data_p_buffer)));
    msg.length = msg.max_size;

    ParticipantProxyData out(RTPSParticipantAllocationAttributes{});
    EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, true, network, true)));
}

// Regression test for redmine tickets 20306 and 20307
TEST(BuiltinDataSerializationTests, other_vendor_parameter_list_with_custom_pids)
{
    /* Convenient functions to group code */
    auto participant_read = [](octet* buffer, uint32_t buffer_length, ParticipantProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, true, network, false,
                        fastdds::rtps::VendorId_t({2, 0}))));
            };

    auto writer_read = [](octet* buffer, uint32_t buffer_length, WriterProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, fastdds::rtps::VendorId_t({2, 0}))));
            };

    auto reader_read = [](octet* buffer, uint32_t buffer_length, ReaderProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, fastdds::rtps::VendorId_t({2, 0}))));
            };

    auto update_cache_change =
            [](CacheChange_t& change, octet* buffer, uint32_t buffer_length, uint32_t qos_size) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_TRUE(fastdds::dds::ParameterList::updateCacheChangeFromInlineQos(change, &msg, qos_size));
            };

    /* Custom PID tests */

    // PID_PERSISTENCE_GUID
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_PERSISTENCE_GUID
            0x02, 0x80, 8, 0,
            1, 2, 3, 4, 5, 6, 7, 8,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_pdata.set_persistence_guid(c_Guid_Unknown);
        participant_read(data_buffer, buffer_length, participant_pdata);
        ASSERT_EQ(participant_pdata.get_persistence_guid(), c_Guid_Unknown);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_pdata.persistence_guid = c_Guid_Unknown;
        writer_read(data_buffer, buffer_length, writer_pdata);
        ASSERT_EQ(writer_pdata.persistence_guid, c_Guid_Unknown);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_read(data_buffer, buffer_length, reader_pdata);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }

    // PID_CUSTOM_RELATED_SAMPLE_IDENTITY
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_CUSTOM_RELATED_SAMPLE_IDENTITY
            0x0f, 0x80, 8, 0,
            1, 2, 3, 4, 5, 6, 7, 8,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_read(data_buffer, buffer_length, writer_pdata);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_read(data_buffer, buffer_length, reader_pdata);

        // CacheChange_t check
        CacheChange_t change;
        GuidPrefix_t prefix;
        prefix.value[0] = 1;
        change.write_params.related_sample_identity().writer_guid(GUID_t(prefix, 1));
        change.write_params.sample_identity().sequence_number() = {2, 0};
        update_cache_change(change, data_buffer, buffer_length, 0);
        ASSERT_EQ(change.write_params.related_sample_identity().writer_guid(), GUID_t(prefix, 1));
        ASSERT_EQ(change.write_params.sample_identity().sequence_number(), SequenceNumber_t(2, 0));
    }

    // PID_DISABLE_POSITIVE_ACKS
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_DISABLE_POSITIVE_ACKS
            0x05, 0x80, 8, 0,
            1, 2, 3, 4, 5, 6, 7, 8,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_pdata.disable_positive_acks.enabled = false;
        writer_read(data_buffer, buffer_length, writer_pdata);
        ASSERT_EQ(writer_pdata.disable_positive_acks.enabled, false);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_pdata.disable_positive_acks.enabled = false;
        reader_read(data_buffer, buffer_length, reader_pdata);
        ASSERT_EQ(reader_pdata.disable_positive_acks.enabled, false);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }

    // PID_DATASHARING
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_DATASHARING
            0x06, 0x80, 8, 0,
            1, 2, 3, 4, 5, 6, 7, 8,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_pdata.data_sharing.off();
        writer_pdata.data_sharing.set_max_domains(0);
        writer_read(data_buffer, buffer_length, writer_pdata);
        ASSERT_EQ(writer_pdata.data_sharing.kind(), dds::OFF);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_pdata.data_sharing.off();
        reader_pdata.data_sharing.set_max_domains(0);
        reader_pdata.disable_positive_acks.enabled = false;
        reader_read(data_buffer, buffer_length, reader_pdata);
        ASSERT_EQ(reader_pdata.data_sharing.kind(), dds::OFF);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }

    // PID_NETWORK_CONFIGURATION_SET
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_NETWORK_CONFIGURATION_SET
            0x07, 0x80, 8, 0,
            1, 2, 3, 4, 5, 6, 7, 8,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_pdata.m_network_configuration = 0;
        participant_read(data_buffer, buffer_length, participant_pdata);
        ASSERT_EQ(participant_pdata.m_network_configuration, 0u);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_pdata.networkConfiguration(0);
        writer_read(data_buffer, buffer_length, writer_pdata);
        ASSERT_EQ(writer_pdata.networkConfiguration(), 0u);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_pdata.network_configuration(0);
        reader_read(data_buffer, buffer_length, reader_pdata);
        ASSERT_EQ(reader_pdata.network_configuration(), 0u);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }

    // PID_PRODUCT_VERSION
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_PRODUCT_VERSION
            0x00, 0x80, 4, 0,
            7, 1, 0, 0,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);
        EXPECT_EQ(0, participant_pdata.product_version.major);
        EXPECT_EQ(0, participant_pdata.product_version.minor);
        EXPECT_EQ(0, participant_pdata.product_version.patch);
        EXPECT_EQ(0, participant_pdata.product_version.tweak);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }
}

// Check interoperability of compatible custom PIDs when vendor ID is RTI Connext
TEST(BuiltinDataSerializationTests, rti_parameter_list_with_custom_pids)
{
    /* Convenient functions to group code */
    auto participant_read = [](octet* buffer, uint32_t buffer_length, ParticipantProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, true, network, false,
                        fastdds::rtps::c_VendorId_rti_connext)));
            };

    auto writer_read = [](octet* buffer, uint32_t buffer_length, WriterProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, fastdds::rtps::c_VendorId_rti_connext)));
            };

    auto reader_read = [](octet* buffer, uint32_t buffer_length, ReaderProxyData& out) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, fastdds::rtps::c_VendorId_rti_connext)));
            };

    auto update_cache_change =
            [](CacheChange_t& change, octet* buffer, uint32_t buffer_length, uint32_t qos_size) -> void
            {
                CDRMessage_t msg(0);
                msg.init(buffer, buffer_length);
                msg.length = msg.max_size;

                EXPECT_TRUE(fastdds::dds::ParameterList::updateCacheChangeFromInlineQos(change, &msg, qos_size));
            };

    /* Custom PID tests */

    // PID_PERSISTENCE_GUID
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_PERSISTENCE_GUID
            0x02, 0x80, 16, 0,
            0x52, 0x54, 0x49, 0x5F, 0x47, 0x55, 0x49, 0x44, 0x5F, 0x54, 0x45, 0x53, 0x54, 0x50, 0x49, 0x44,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // Define guid for checking the read data
        GuidPrefix_t prefix;
        prefix.value[0] = 0x52;
        prefix.value[1] = 0x54;
        prefix.value[2] = 0x49;
        prefix.value[3] = 0x5F;
        prefix.value[4] = 0x47;
        prefix.value[5] = 0x55;
        prefix.value[6] = 0x49;
        prefix.value[7] = 0x44;
        prefix.value[8] = 0x5F;
        prefix.value[9] = 0x54;
        prefix.value[10] = 0x45;
        prefix.value[11] = 0x53;

        EntityId_t entity_id;
        entity_id.value[0] = 0x54;
        entity_id.value[1] = 0x50;
        entity_id.value[2] = 0x49;
        entity_id.value[3] = 0x44;

        GUID_t guid(prefix, entity_id);

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_pdata.set_persistence_guid(c_Guid_Unknown);
        participant_read(data_buffer, buffer_length, participant_pdata);

        ASSERT_EQ(participant_pdata.get_persistence_guid(), c_Guid_Unknown);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_pdata.persistence_guid = c_Guid_Unknown;
        writer_read(data_buffer, buffer_length, writer_pdata);
        ASSERT_EQ(writer_pdata.persistence_guid, guid);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_read(data_buffer, buffer_length, reader_pdata);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }

    // PID_CUSTOM_RELATED_SAMPLE_IDENTITY
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_CUSTOM_RELATED_SAMPLE_IDENTITY
            0x0f, 0x80, 24, 0,
            // Writer guid
            0x52, 0x54, 0x49, 0x5F, 0x47, 0x55, 0x49, 0x44, 0x5F, 0x54, 0x45, 0x53, 0x54, 0x50, 0x49, 0x44,
            // Sequence number high
            0x00, 0x00, 0x00, 0x00,
            // Sequence number low
            0x05, 0x00, 0x00, 0x00,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // Define sample identity for checking the read data
        GuidPrefix_t prefix;
        prefix.value[0] = 0x52;
        prefix.value[1] = 0x54;
        prefix.value[2] = 0x49;
        prefix.value[3] = 0x5F;
        prefix.value[4] = 0x47;
        prefix.value[5] = 0x55;
        prefix.value[6] = 0x49;
        prefix.value[7] = 0x44;
        prefix.value[8] = 0x5F;
        prefix.value[9] = 0x54;
        prefix.value[10] = 0x45;
        prefix.value[11] = 0x53;

        EntityId_t entity_id;
        entity_id.value[0] = 0x54;
        entity_id.value[1] = 0x50;
        entity_id.value[2] = 0x49;
        entity_id.value[3] = 0x44;

        GUID_t guid(prefix, entity_id);

        SequenceNumber_t sn = {0, 5};

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_read(data_buffer, buffer_length, writer_pdata);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_read(data_buffer, buffer_length, reader_pdata);

        // CacheChange_t check
        CacheChange_t change;
        change.vendor_id = fastdds::rtps::c_VendorId_rti_connext;
        GuidPrefix_t init_prefix;
        prefix.value[0] = 1;
        change.write_params.sample_identity().writer_guid(GUID_t(init_prefix, 1));
        change.write_params.sample_identity().sequence_number() = {2, 0};
        update_cache_change(change, data_buffer, buffer_length, 0);
        ASSERT_EQ(change.write_params.sample_identity().writer_guid(), guid);
        ASSERT_EQ(change.write_params.sample_identity().sequence_number(), sn);
    }

    // PID_DISABLE_POSITIVE_ACKS
    {
        octet data_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // PID_DISABLE_POSITIVE_ACKS
            0x05, 0x80, 4, 0,
            1, 0, 0, 0,
            // PID_SENTINEL
            0x01, 0, 0, 0
        };

        uint32_t buffer_length = static_cast<uint32_t>(sizeof(data_buffer));

        // ParticipantProxyData check
        ParticipantProxyData participant_pdata(RTPSParticipantAllocationAttributes{});
        participant_read(data_buffer, buffer_length, participant_pdata);

        // WriterProxyData check
        WriterProxyData writer_pdata(max_unicast_locators, max_multicast_locators);
        writer_pdata.disable_positive_acks.enabled = false;
        writer_read(data_buffer, buffer_length, writer_pdata);
        ASSERT_EQ(writer_pdata.disable_positive_acks.enabled, true);

        // ReaderProxyData check
        ReaderProxyData reader_pdata(max_unicast_locators, max_multicast_locators);
        reader_pdata.disable_positive_acks.enabled = false;
        reader_read(data_buffer, buffer_length, reader_pdata);
        ASSERT_EQ(reader_pdata.disable_positive_acks.enabled, true);

        // CacheChange_t check
        CacheChange_t change;
        update_cache_change(change, data_buffer, buffer_length, 0);
    }
}

/*!
 * \test RTPS-CFT-CFP-01 Tests serialization of `ContentFilterProperty_t` works successfully without parameters.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_without_parameters)
{
    ReaderProxyData in(max_unicast_locators, max_multicast_locators);
    ReaderProxyData out(max_unicast_locators, max_multicast_locators);

    // Topic and type name cannot be empty
    in.topic_name = "TEST";
    in.type_name = "TestType";

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "MyFilterClass";
    content_filter_property.filter_expression = "This is a custom test filter expression";
    in.content_filter = content_filter_property;

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.write_to_cdr_message(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.read_from_cdr_message(&msg));

    ASSERT_EQ(in.content_filter.content_filtered_topic_name, out.content_filter.content_filtered_topic_name);
    ASSERT_EQ(in.content_filter.related_topic_name, out.content_filter.related_topic_name);
    ASSERT_EQ(in.content_filter.filter_class_name, out.content_filter.filter_class_name);
    ASSERT_EQ(in.content_filter.filter_expression, out.content_filter.filter_expression);
    ASSERT_EQ(in.content_filter.expression_parameters.size(), out.content_filter.expression_parameters.size());
}

/*!
 * \test RTPS-CFT-CFP-02 Tests serialization of `ContentFilterProperty_t` works successfully with parameters.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_with_parameters)
{
    ReaderProxyData in(max_unicast_locators, max_multicast_locators);
    ReaderProxyData out(max_unicast_locators, max_multicast_locators);

    // Topic and type name cannot be empty
    in.topic_name = "TEST";
    in.type_name = "TestType";

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "MyFilterClass";
    content_filter_property.filter_expression = "%1 a custom test filter expression: %2 %3 %4";
    content_filter_property.expression_parameters.push_back("parameter 1");
    content_filter_property.expression_parameters.push_back("parameter 2");
    content_filter_property.expression_parameters.push_back("parameter 3");
    content_filter_property.expression_parameters.push_back("parameter 4");
    in.content_filter = content_filter_property;

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.write_to_cdr_message(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.read_from_cdr_message(&msg));

    ASSERT_EQ(in.content_filter.content_filtered_topic_name, out.content_filter.content_filtered_topic_name);
    ASSERT_EQ(in.content_filter.related_topic_name, out.content_filter.related_topic_name);
    ASSERT_EQ(in.content_filter.filter_class_name, out.content_filter.filter_class_name);
    ASSERT_EQ(in.content_filter.filter_expression, out.content_filter.filter_expression);
    ASSERT_EQ(in.content_filter.expression_parameters.size(), out.content_filter.expression_parameters.size());
    ASSERT_EQ(in.content_filter.expression_parameters[0], out.content_filter.expression_parameters[0]);
    ASSERT_EQ(in.content_filter.expression_parameters[1], out.content_filter.expression_parameters[1]);
    ASSERT_EQ(in.content_filter.expression_parameters[2], out.content_filter.expression_parameters[2]);
    ASSERT_EQ(in.content_filter.expression_parameters[3], out.content_filter.expression_parameters[3]);
}

/*!
 * \test RTPS-CFT-CFP-03 Tests serialization of `ContentFilterProperty_t` fails with a wrong `contentFilteredTopicName`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_topic_name_ser)
{
    // Empty value
    {
        ReaderProxyData in(max_unicast_locators, max_multicast_locators);
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Topic and type name cannot be empty
        in.topic_name = "TEST";
        in.type_name = "TestType";

        // Fill ContentFilterProperty_t without parameters.
        fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
        fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
        content_filter_property.related_topic_name = "TEST";
        content_filter_property.filter_class_name = "MyFilterClass";
        content_filter_property.filter_expression = "This is a custom test filter expression";
        in.content_filter = content_filter_property;

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_FALSE(in.write_to_cdr_message(&msg, true));
    }
}

/*!
 * \test RTPS-CFT-CFP-04 Tests deserialization of `ContentFilterProperty_t` doesn't do anything with a CDRMessage_t
 * containing a wrong `contentFilteredTopicName`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_topic_name_deser)
{
    // Empty value
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name;
        const std::string related_topic_name("TEST");
        const std::string filter_class_name("MyFilterClass");
        const std::string filter_expression("This is a custom test filter expression");
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }

    // Larger string than 256 characters.
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name(260, 'a');
        const std::string related_topic_name("TEST");
        const std::string filter_class_name("MyFilterClass");
        const std::string filter_expression("This is a custom test filter expression");
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }
}

/*!
 * \test RTPS-CFT-CFP-05 Tests serialization of `ContentFilterProperty_t` fails with a wrong `relatedTopicName`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_related_topic_name_ser)
{
    // Empty value
    {
        ReaderProxyData in(max_unicast_locators, max_multicast_locators);
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Topic and type name cannot be empty
        in.topic_name = "TEST";
        in.type_name = "TestType";

        // Fill ContentFilterProperty_t without parameters.
        fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
        fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
        content_filter_property.content_filtered_topic_name = "CFT_TEST";
        content_filter_property.filter_class_name = "MyFilterClass";
        content_filter_property.filter_expression = "This is a custom test filter expression";
        in.content_filter = content_filter_property;

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_FALSE(in.write_to_cdr_message(&msg, true));
    }
}

/*!
 * \test RTPS-CFT-CFP-06 Tests deserialization of `ContentFilterProperty_t` doesn't do anything with a CDRMessage_t
 * containing a wrong `relatedTopicName`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_related_topic_name_deser)
{
    // Empty value
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name("CFT_TEST");
        const std::string related_topic_name;
        const std::string filter_class_name("MyFilterClass");
        const std::string filter_expression("This is a custom test filter expression");
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }

    // Larger string than 256 characters.
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name("CFT_TEST");
        const std::string related_topic_name(260, 'a');
        const std::string filter_class_name("MyFilterClass");
        const std::string filter_expression("This is a custom test filter expression");
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }
}

/*!
 * \test RTPS-CFT-CFP-07 Tests serialization of `ContentFilterProperty_t` when `filterClassName` is empty.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_empty_filter_class)
{
    ReaderProxyData in(max_unicast_locators, max_multicast_locators);
    ReaderProxyData out(max_unicast_locators, max_multicast_locators);

    // Topic and type name cannot be empty
    in.topic_name = "TEST";
    in.type_name = "TestType";

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "";
    content_filter_property.filter_expression = "This is a custom test filter expression";
    in.content_filter = content_filter_property;

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.write_to_cdr_message(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.read_from_cdr_message(&msg));

    assert_is_empty_content_filter(out.content_filter);
}

/*!
 * \test RTPS-CFT-CFP-08 Tests deserialization of `ContentFilterProperty_t` doesn't do anything with a CDRMessage_t containing a wrong
 *`filterClassName`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_filter_class_deser)
{
    // Empty value
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name("CFT_TEST");
        const std::string related_topic_name("TEST");
        const std::string filter_class_name;
        const std::string filter_expression("This is a custom test filter expression");
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }

    // Larger string than 256 characters.
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name("CFT_TEST");
        const std::string related_topic_name("TEST");
        const std::string filter_class_name(260, 'a');
        const std::string filter_expression("This is a custom test filter expression");
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }
}

/*!
 * \test RTPS-CFT-CFP-09 Tests serialization of `ContentFilterProperty_t` when `filterExpression` is empty.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_empty_filter_expression)
{
    ReaderProxyData in(max_unicast_locators, max_multicast_locators);
    ReaderProxyData out(max_unicast_locators, max_multicast_locators);

    // Topic and type name cannot be empty
    in.topic_name = "TEST";
    in.type_name = "TestType";

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "MyFilterClass";
    content_filter_property.filter_expression = "";
    in.content_filter = content_filter_property;

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.write_to_cdr_message(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.read_from_cdr_message(&msg));

    assert_is_empty_content_filter(out.content_filter);
}

/*!
 * \test RTPS-CFT-CFP-10 Tests deserialization of `ContentFilterProperty_t` doesn't do anything with a CDRMessage_t
 * containing a wrong `filterExpression`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_filter_expression_deser)
{
    // Empty value
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(400);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name("CFT_TEST");
        const std::string related_topic_name("TEST");
        const std::string filter_class_name("MyFilterClass");
        const std::string filter_expression;
        const std::vector<std::string> expression_parameters = {};
        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        assert_is_empty_content_filter(out.content_filter);
    }
}

/*!
 * \test RTPS-CFT-CFP-11 Tests serialization of `ContentFilterProperty_t` fails with a wrong `CDRMessage_t`
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_cdr_message_ser)
{
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "MyFilterClass";
    content_filter_property.filter_expression = "This is a custom test filter expression";

    // Not initialized
    {
        CDRMessage_t msg(0);
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::add_to_cdr_message(
                    content_filter_property, &msg));
    }
    //Empty buffer but not enough memory.
    {
        CDRMessage_t msg(20);
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::add_to_cdr_message(
                    content_filter_property, &msg));
    }
    // Used buffer but not enough memory left.
    {
        CDRMessage_t msg(30);
        msg.pos = 10;
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::add_to_cdr_message(
                    content_filter_property, &msg));
    }

}

/*!
 * \test RTPS-CFT-CFP-12 Tests deserialization of `ContentFilterProperty_t` fails with a wrong `CDRMessage_t`
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_wrong_cdr_message_deser)
{
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);

    const std::string content_filtered_topic_name("CFT_TEST");
    const std::string related_topic_name("TEST");
    const std::string filter_class_name("MyFilterClass");
    const std::string filter_expression("This is a custom test filter expression");
    const std::vector<std::string> expression_parameters = {};
    uint16_t len = static_cast<uint16_t>(manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                ));

    // Not initialized
    {
        CDRMessage_t msg(0);
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::read_from_cdr_message(
                    content_filter_property, &msg, len));
    }
    //Empty buffer but not enough memory.
    {
        CDRMessage_t msg(20);
        fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY);
        fastdds::rtps::CDRMessage::addUInt16(&msg, len - 4);
        // content_filtered_topic_name
        fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name);
        // related_topic_name
        fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name);
        // filter_class_name
        fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name);
        // filter_expression
        fastdds::rtps::CDRMessage::add_string(&msg, filter_expression);
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::read_from_cdr_message(
                    content_filter_property, &msg, len));
    }
    // Used buffer but not enough memory left.
    {
        CDRMessage_t msg(30);
        msg.pos = 10;
        fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY);
        fastdds::rtps::CDRMessage::addUInt16(&msg, len - 4);
        // content_filtered_topic_name
        fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name);
        // related_topic_name
        fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name);
        // filter_class_name
        fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name);
        // filter_expression
        fastdds::rtps::CDRMessage::add_string(&msg, filter_expression);
        msg.pos = 10;
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::read_from_cdr_message(
                    content_filter_property, &msg, len));
    }
}

/*!
 * \test RTPS-CFT-CFP-13 Tests the interoperability with RTI in the propagation of the RTPS's `ContentFilterProperty_t`.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_interoperability)
{
    // pcap file located in task #14171. Captured from RTI Shapes Demo 6.1.0
    octet data_r_buffer[] =
    {
        // Encapsulation
        0x00, 0x03, 0x00, 0x00,
        // Endpoing guid
        0x5a, 0x00, 0x10, 0x00, 0x01, 0x01, 0x96, 0x4b, 0xa1, 0x2e, 0xb6, 0x10, 0xeb, 0x47, 0x54, 0x76,
        0x80, 0x00, 0x00, 0x07,
        // Protocol version
        0x15, 0x00, 0x04, 0x00, 0x02, 0x03, 0x00, 0x00,
        // Vendor id
        0x16, 0x00, 0x04, 0x00, 0x01, 0x01, 0x00, 0x00,
        // Product version
        0x00, 0x80, 0x04, 0x00, 0x06, 0x01, 0x00, 0x00,
        // Topic name
        0x05, 0x00, 0x0c, 0x00, 0x07, 0x00, 0x00, 0x00, 0x53, 0x71, 0x75, 0x61, 0x72, 0x65, 0x00, 0x00,
        // Type name
        0x07, 0x00, 0x10, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70,
        0x65, 0x00, 0x00, 0x00,
        // Content filter property
        0x35, 0x00, 0x7c, 0x00, 0x10, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x46,
        0x69, 0x6c, 0x74, 0x65, 0x72, 0x5f, 0x30, 0x00, 0x07, 0x00, 0x00, 0x00, 0x53, 0x71, 0x75, 0x61,
        0x72, 0x65, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x44, 0x44, 0x53, 0x53, 0x51, 0x4c, 0x00, 0x00,
        0x28, 0x00, 0x00, 0x00, 0x78, 0x20, 0x3e, 0x20, 0x25, 0x30, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x78,
        0x20, 0x3c, 0x20, 0x25, 0x31, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x79, 0x20, 0x3e, 0x20, 0x25, 0x32,
        0x20, 0x61, 0x6e, 0x64, 0x20, 0x79, 0x20, 0x3c, 0x20, 0x25, 0x33, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x31, 0x30, 0x30, 0x00, 0x04, 0x00, 0x00, 0x00, 0x32, 0x30, 0x30, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x31, 0x30, 0x30, 0x00, 0x04, 0x00, 0x00, 0x00, 0x32, 0x30, 0x30, 0x00,
        //
        0x18, 0x00, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff,
        // Data representation
        0x73, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        // Group entity id
        0x53, 0x00, 0x04, 0x00, 0x80, 0x00, 0x00, 0x09,
        // Persistence guid
        0x02, 0x80, 0x10, 0x00, 0x01, 0x01, 0x96, 0x4b, 0xa1, 0x2e, 0xb6, 0x10, 0xeb, 0x47, 0x54, 0x76,
        0x80, 0x00, 0x00, 0x07,
        //
        0x09, 0x80, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Type consistency
        0x74, 0x00, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
        //
        0x15, 0x80, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Type object
        0x21, 0x80, 0xa0, 0x01, 0x01, 0x00, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0x92, 0x01, 0x00, 0x00,
        0x78, 0xda, 0x63, 0xac, 0xe7, 0x60, 0x00, 0x81, 0x27, 0xcc, 0x0c, 0x0c, 0x2c, 0x60, 0x16, 0x0b,
        0x83, 0x18, 0x90, 0x64, 0x04, 0x8a, 0x73, 0x02, 0xe9, 0x2f, 0x50, 0x36, 0x08, 0x68, 0x80, 0x49,
        0x31, 0x30, 0x79, 0xb9, 0xba, 0x83, 0xad, 0x41, 0x42, 0xc3, 0x59, 0x08, 0xc8, 0x0e, 0xce, 0x48,
        0x2c, 0x48, 0x0d, 0xa9, 0x2c, 0x48, 0x75, 0xad, 0x28, 0x49, 0xcd, 0x4b, 0x49, 0x4d, 0x81, 0xea,
        0x61, 0x64, 0x80, 0x99, 0x09, 0xe1, 0x83, 0xc4, 0x05, 0xe0, 0x26, 0x30, 0x30, 0x1c, 0x7d, 0xbd,
        0xf1, 0xde, 0xb1, 0x90, 0xbf, 0x99, 0x20, 0xb9, 0x54, 0x20, 0xbf, 0x05, 0x88, 0x99, 0x30, 0xec,
        0x83, 0x98, 0xc1, 0x07, 0x65, 0x3f, 0x39, 0xfd, 0xa0, 0x31, 0xf9, 0xd7, 0x15, 0x7e, 0x90, 0xdb,
        0xd2, 0x32, 0x73, 0x72, 0xbc, 0x33, 0xf3, 0x52, 0x18, 0xb0, 0xd8, 0xc7, 0x54, 0x8f, 0x30, 0x47,
        0x02, 0x2a, 0xc6, 0x0a, 0xc4, 0x9c, 0x40, 0xc8, 0x06, 0xa4, 0x13, 0xf3, 0xd2, 0x73, 0x52, 0x71,
        0xe8, 0x83, 0x61, 0xf4, 0xb0, 0xf0, 0x60, 0x44, 0x98, 0xa9, 0x80, 0x14, 0x16, 0x30, 0x7f, 0x70,
        0x21, 0x87, 0x05, 0x9e, 0x30, 0x40, 0xe6, 0x83, 0xd4, 0xbd, 0x81, 0x8a, 0xc1, 0xcc, 0x56, 0x01,
        0xb1, 0xa1, 0x6a, 0x84, 0xa1, 0xf4, 0x55, 0xd3, 0x5f, 0xd3, 0xd6, 0x6d, 0x9b, 0x08, 0x76, 0x7b,
        0x72, 0x7e, 0x4e, 0x7e, 0x11, 0x01, 0x3f, 0x8b, 0xc0, 0xec, 0x00, 0xfb, 0x9b, 0x15, 0x1c, 0xae,
        0x15, 0x44, 0xea, 0x61, 0x42, 0xd2, 0x53, 0x49, 0x40, 0x8f, 0x0c, 0x54, 0x8c, 0x19, 0xaa, 0x07,
        0x14, 0x06, 0xc5, 0xa0, 0x30, 0x28, 0xce, 0xac, 0x22, 0x26, 0x7c, 0x85, 0xa1, 0x6a, 0x40, 0xa6,
        0x95, 0x20, 0x85, 0x81, 0x0e, 0xd8, 0x1d, 0xc2, 0x28, 0x7e, 0x17, 0x05, 0x99, 0x5d, 0x52, 0x94,
        0x99, 0x97, 0x1e, 0x6f, 0x68, 0x64, 0x11, 0x9f, 0x9c, 0x91, 0x58, 0x94, 0x98, 0x5c, 0x92, 0x5a,
        0xc4, 0x40, 0x20, 0xac, 0x79, 0x80, 0x30, 0x15, 0x2a, 0x03, 0x12, 0x3f, 0x01, 0x15, 0x6f, 0x60,
        0x40, 0x75, 0x0b, 0x1f, 0x54, 0x1e, 0x94, 0x4e, 0x2e, 0xa0, 0xc5, 0x07, 0x2c, 0x05, 0xc2, 0xd2,
        0x1f, 0x1f, 0x2c, 0xae, 0xdd, 0x10, 0x89, 0x10, 0x9f, 0x1b, 0x14, 0x90, 0xe2, 0xbb, 0x00, 0x49,
        0x0d, 0x37, 0xc8, 0x1c, 0x7f, 0x1f, 0x4f, 0x97, 0x78, 0x37, 0x4f, 0x1f, 0x1f, 0x48, 0x7c, 0x09,
        0x02, 0x71, 0x48, 0x90, 0xa3, 0x5f, 0x70, 0x80, 0x63, 0x90, 0xab, 0x5f, 0x08, 0x54, 0x06, 0x12,
        0x2f, 0xa0, 0x14, 0xe7, 0xe1, 0x1f, 0xe4, 0x19, 0xe5, 0xef, 0x17, 0xe2, 0xe8, 0x13, 0xef, 0xe1,
        0x18, 0xe2, 0xec, 0x01, 0x53, 0xc0, 0x0c, 0x8d, 0xc3, 0x30, 0xd7, 0xa0, 0x10, 0x4f, 0x67, 0x54,
        0x59, 0x98, 0x3f, 0x61, 0x6e, 0x44, 0xce, 0x87, 0xb0, 0xbc, 0x0c, 0x92,
        // Sentinel
        0x07, 0x00, 0x16, 0x2a, 0x83, 0x1a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    CDRMessage_t msg(0);
    msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
    msg.length = msg.max_size;

    ReaderProxyData out(max_unicast_locators, max_multicast_locators);
    EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg)));

    ASSERT_EQ("ContentFilter_0", out.content_filter.content_filtered_topic_name.to_string());
    ASSERT_EQ("Square", out.content_filter.related_topic_name.to_string());
    ASSERT_EQ("DDSSQL", out.content_filter.filter_class_name.to_string());
    ASSERT_EQ("x > %0 and x < %1 and y > %2 and y < %3", out.content_filter.filter_expression);
    ASSERT_EQ(4, out.content_filter.expression_parameters.size());
    ASSERT_EQ("100", out.content_filter.expression_parameters[0].to_string());
    ASSERT_EQ("200", out.content_filter.expression_parameters[1].to_string());
    ASSERT_EQ("100", out.content_filter.expression_parameters[2].to_string());
    ASSERT_EQ("200", out.content_filter.expression_parameters[3].to_string());
}

/*!
 * \test Test for deserialization of messages with expression parameters list
 *       with sizes of 100 or greater
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_max_parameter_check)
{
    // Empty value
    {
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Perform serialization
        CDRMessage_t msg(5000);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg));
        const std::string content_filtered_topic_name("CFT_TEST");
        const std::string related_topic_name("TEST");
        const std::string filter_class_name("MyFilterClass");
        const std::string filter_expression("This is a custom test filter expression");
        std::vector<std::string> expression_parameters;
        for (int i = 0; i < 100; ++i)
        {
            expression_parameters.push_back("Parameter");
        }

        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_EQ(num_params, 100u);
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.read_from_cdr_message(&msg));

        ASSERT_EQ(100, out.content_filter.expression_parameters.size());

        CDRMessage_t msg_fault(5000);
        EXPECT_TRUE(fastdds::dds::ParameterList::writeEncapsulationToCDRMsg(&msg_fault));

        // Manual serialization of a ContentFilterProperty.
        {
            uint32_t len = manual_content_filter_cdr_serialized_size(
                content_filtered_topic_name,
                related_topic_name,
                filter_class_name,
                filter_expression,
                expression_parameters
                );
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg_fault, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt16(&msg_fault, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg_fault, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg_fault, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg_fault, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg_fault, filter_expression));
            // expression_parameters
            // sequence length

            // Add the 101st parameter to the list
            expression_parameters.push_back("Parameter");
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_EQ(num_params, 101u);
            EXPECT_TRUE(fastdds::rtps::CDRMessage::addUInt32(&msg_fault, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastdds::rtps::CDRMessage::add_string(&msg_fault, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg_fault));

        msg_fault.pos = 0;
        // Deserialization of messages with more than 100 parameters should fail
        ASSERT_FALSE(out.read_from_cdr_message(&msg_fault));
    }
}

TEST(BuiltinDataSerializationTests, null_checks)
{
    {
        // Test deserialization sanity checks
        uint32_t msg_size = 100;
        CDRMessage_t msg(msg_size);

        ASSERT_FALSE(CDRMessage::addData(nullptr, (octet*) &msg, msg_size));
        ASSERT_FALSE(CDRMessage::addData(&msg, nullptr, msg_size));
        ASSERT_TRUE(CDRMessage::addData(&msg, nullptr, 0));

        // Test deserialization sanity checks

        ASSERT_FALSE(CDRMessage::readData(nullptr, (octet*) &msg, msg_size));
        ASSERT_FALSE(CDRMessage::readData(&msg, nullptr, msg_size));
        ASSERT_TRUE(CDRMessage::readData(&msg, nullptr, 0));
    }
}

/*!
 * This is a regression test for redmine issue #21537
 *
 * It checks that Fast DDS can deserialize DATA(p), DATA(w), and DATA(r) messages captured from InterCom DDS.
 */
TEST(BuiltinDataSerializationTests, interoperability_with_intercomdds)
{
    const VendorId_t intercom_vendor_id = { 1, 5 };

    // DATA(p)
    {
        // This was captured with wireshark from intercom_dds-3.16.2.0_shape_main_linux taken from
        // https://github.com/omg-dds/dds-rtps/releases/tag/v1.2.2024
        octet data_p_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // Participant GUID
            0x50, 0x00, 0x10, 0x00,
            0x01, 0x05, 0xfa, 0xd5, 0x7a, 0x09, 0x6a, 0x22, 0x84, 0xfb, 0x23, 0xa2, 0x00, 0x00, 0x01, 0xc1,
            // Custom (0x8003)
            0x03, 0x80, 0x20, 0x00,
            0x38, 0x63, 0x33, 0x64, 0x36, 0x62, 0x61, 0x39, 0x32, 0x61, 0x35, 0x38, 0x39, 0x31, 0x62, 0x62,
            0x62, 0x64, 0x62, 0x34, 0x35, 0x62, 0x67, 0x66, 0x32, 0x61, 0x35, 0x38, 0x39, 0x63, 0x62, 0x36,
            // Custom (0x8005)
            0x05, 0x80, 0x18, 0x00,
            0x12, 0x00, 0x00, 0x00, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x43, 0x4f, 0x4d, 0x20, 0x33, 0x5f, 0x31,
            0x36, 0x5f, 0x32, 0x5f, 0x30, 0x00, 0x00, 0x00,
            // Custom (0x8006)
            0x06, 0x80, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Custom (0x8007)
            0x07, 0x80, 0x0c, 0x00,
            0x01, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
            // Custom (0x8008)
            0x08, 0x80, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Custom (0x8009)
            0x09, 0x80, 0x34, 0x00,
            0x2e, 0x00, 0x00, 0x00, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x63, 0x6f, 0x6d, 0x5f, 0x64, 0x64, 0x73,
            0x2d, 0x33, 0x2e, 0x31, 0x36, 0x2e, 0x32, 0x2e, 0x30, 0x5f, 0x73, 0x68, 0x61, 0x70, 0x65, 0x5f,
            0x6d, 0x61, 0x69, 0x6e, 0x5f, 0x6c, 0x69, 0x6e, 0x75, 0x78, 0x20, 0x28, 0x37, 0x30, 0x34, 0x35,
            0x29, 0x00, 0x00, 0x00,
            // Domain ID
            0x0f, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Builtin endpoint QoS
            0x77, 0x00, 0x04, 0x00,
            0x01, 0x00, 0x00, 0x00,
            // Builtin endpoint set
            0x58, 0x00, 0x04, 0x00,
            0x3f, 0xfc, 0x00, 0x00,
            // Protocol version
            0x15, 0x00, 0x04, 0x00,
            0x02, 0x05, 0x00, 0x00,
            // Vendor ID
            0x16, 0x00, 0x04, 0x00,
            0x01, 0x05, 0x00, 0x00,
            // Default Unicast Locator
            0x31, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xf5, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xac, 0x1a, 0x9f, 0xd5,
            // Default Multicast Locator
            0x48, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xe9, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xef, 0xff, 0x00, 0x01,
            // Metatraffic unicast locator
            0x32, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xf4, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xac, 0x1a, 0x9f, 0xd5,
            // Metatraffic multicast locator
            0x33, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xe8, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xef, 0xff, 0x00, 0x01,
            // Participant lease duration
            0x02, 0x00, 0x08, 0x00,
            0x2c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_p_buffer, static_cast<uint32_t>(sizeof(data_p_buffer)));
        msg.length = msg.max_size;

        ParticipantProxyData out({});
        EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, true, network, false, intercom_vendor_id)));
    }

    // DATA(w)
    {
        // This was captured with wireshark from intercom_dds-3.16.2.0_shape_main_linux taken from
        // https://github.com/omg-dds/dds-rtps/releases/tag/v1.2.2024
        octet data_w_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // Writer GUID
            0x5a, 0x00, 0x10, 0x00,
            0x01, 0x05, 0xfa, 0xd5, 0x7a, 0x09, 0x6a, 0x22, 0x84, 0xfb, 0x23, 0xa2, 0x00, 0x00, 0x01, 0x02,
            // Participant GUID
            0x50, 0x00, 0x10, 0x00,
            0x01, 0x05, 0xfa, 0xd5, 0x7a, 0x09, 0x6a, 0x22, 0x84, 0xfb, 0x23, 0xa2, 0x00, 0x00, 0x01, 0xc1,
            // Topic name
            0x05, 0x00, 0x0c, 0x00,
            0x07, 0x00, 0x00, 0x00, 0x53, 0x71, 0x75, 0x61, 0x72, 0x65, 0x00, 0x00,
            // Type name
            0x07, 0x00, 0x10, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
            // Type information
            0x75, 0x00, 0x5c, 0x00,
            0x58, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
            0xf1, 0xa5, 0x12, 0xf3, 0x95, 0xe2, 0xba, 0xb0, 0xb9, 0xfc, 0x83, 0x8e, 0x08, 0x6e, 0x2c, 0x00,
            0x57, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xf2, 0x77, 0x32, 0x07,
            0xfb, 0x72, 0x38, 0x6e, 0x0d, 0xdb, 0x0e, 0x1a, 0x2b, 0x4f, 0xbe, 0x00, 0x84, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Topic data
            0x2e, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Data representation
            0x73, 0x00, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Property list
            0x59, 0x00, 0x08, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Data tags
            0x03, 0x10, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Service instance name
            0x80, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Related entity GUID
            0x81, 0x00, 0x10, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // History
            0x40, 0x00, 0x08, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            // Custom (0x8030)
            0x30, 0x80, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
        msg.length = msg.max_size;

        WriterProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, intercom_vendor_id)));
    }

    // DATA(r)
    {
        // This was captured with wireshark from intercom_dds-3.16.2.0_shape_main_linux taken from
        // https://github.com/omg-dds/dds-rtps/releases/tag/v1.2.2024
        uint8_t data_r_buffer[] =
        {
            // Encapsulation
            0x00, 0x03, 0x00, 0x00,
            // Reader GUID
            0x5a, 0x00, 0x10, 0x00,
            0x01, 0x05, 0x0f, 0xda, 0x14, 0xdd, 0x32, 0x62, 0x74, 0xef, 0x08, 0xeb, 0x00, 0x00, 0x01, 0x07,
            // Participant GUID
            0x50, 0x00, 0x10, 0x00,
            0x01, 0x05, 0x0f, 0xda, 0x14, 0xdd, 0x32, 0x62, 0x74, 0xef, 0x08, 0xeb, 0x00, 0x00, 0x01, 0xc1,
            // Topic name
            0x05, 0x00, 0x0c, 0x00,
            0x07, 0x00, 0x00, 0x00, 0x53, 0x71, 0x75, 0x61, 0x72, 0x65, 0x00, 0x00,
            // Type name
            0x07, 0x00, 0x10, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
            // Type information
            0x75, 0x00, 0x5c, 0x00,
            0x58, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
            0xf1, 0xa5, 0x12, 0xf3, 0x95, 0xe2, 0xba, 0xb0, 0xb9, 0xfc, 0x83, 0x8e, 0x08, 0x6e, 0x2c, 0x00,
            0x57, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xf2, 0x77, 0x32, 0x07,
            0xfb, 0x72, 0x38, 0x6e, 0x0d, 0xdb, 0x0e, 0x1a, 0x2b, 0x4f, 0xbe, 0x00, 0x84, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Reliability
            0x1a, 0x00, 0x10, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9a, 0x99, 0x99, 0x19, 0x00, 0x00, 0x00, 0x00,
            // Topic data
            0x2e, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Data representation
            0x73, 0x00, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Type consistency
            0x74, 0x00, 0x08, 0x00,
            0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
            // Property list
            0x59, 0x00, 0x08, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Data tags
            0x03, 0x10, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Service instance name
            0x80, 0x00, 0x04, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Related entity GUID
            0x81, 0x00, 0x10, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // History
            0x40, 0x00, 0x08, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.read_from_cdr_message(&msg, intercom_vendor_id)));
    }
}

/*!
 * This is a regression test for redmine issue #21537
 *
 * It checks deserialization of builtin data with big parameters.
 */
TEST(BuiltinDataSerializationTests, deserialization_of_big_parameters)
{
    constexpr size_t encapsulation_length = 4;
    constexpr size_t guid_length = 20;
    constexpr size_t topic_name_length = 16;
    constexpr size_t type_name_length = 16;
    constexpr size_t parameter_length = 65536;
    constexpr size_t sentinel_length = 4;
    constexpr size_t total_length =
            encapsulation_length + // encapsulation
            parameter_length +   // Big parameter
            guid_length +        // Participant GUID
            guid_length +        // Endpoint GUID
            topic_name_length +  // Topic name
            type_name_length +   // Type name
            sentinel_length;     // Sentinel
    std::array<octet, total_length> buffer{{0}};

    // Encapsulation (PL_CDR_LE)
    size_t pos = 0;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x03;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    // Room for the big parameter
    pos += parameter_length;

    // Participant GUID
    buffer[pos++] = 0x50;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x10;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x05;
    buffer[pos++] = 0x0f;
    buffer[pos++] = 0xda;
    buffer[pos++] = 0x14;
    buffer[pos++] = 0xdd;
    buffer[pos++] = 0x32;
    buffer[pos++] = 0x62;
    buffer[pos++] = 0x74;
    buffer[pos++] = 0xef;
    buffer[pos++] = 0x08;
    buffer[pos++] = 0xeb;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x01;
    buffer[pos++] = 0xc1;

    // Endpoint GUID
    buffer[pos++] = 0x5a;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x10;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x05;
    buffer[pos++] = 0x0f;
    buffer[pos++] = 0xda;
    buffer[pos++] = 0x14;
    buffer[pos++] = 0xdd;
    buffer[pos++] = 0x32;
    buffer[pos++] = 0x62;
    buffer[pos++] = 0x74;
    buffer[pos++] = 0xef;
    buffer[pos++] = 0x08;
    buffer[pos++] = 0xeb;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x07;

    // Topic name ("Square")
    buffer[pos++] = 0x05;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x0c;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x07;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x53;
    buffer[pos++] = 0x71;
    buffer[pos++] = 0x75;
    buffer[pos++] = 0x61;
    buffer[pos++] = 0x72;
    buffer[pos++] = 0x65;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    // Type name ("MyType")
    buffer[pos++] = 0x07;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x0c;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x07;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x4d;
    buffer[pos++] = 0x79;
    buffer[pos++] = 0x54;
    buffer[pos++] = 0x79;
    buffer[pos++] = 0x70;
    buffer[pos++] = 0x65;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    // Sentinel
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;

    ASSERT_EQ(total_length, pos);

    std::set<uint16_t> failed_for_data_p;
    std::set<uint16_t> failed_for_data_w;
    std::set<uint16_t> failed_for_data_r;

    // Loop all parameter IDs in the standard range
    for (uint16_t pid = 0x2; pid <= 0x7FFF; ++pid)
    {
        // Clear big parameter
        octet zero = 0u;
        auto param_begin = buffer.begin() + encapsulation_length;
        std::fill(param_begin, param_begin + parameter_length, zero);

        // Set the parameter ID of the big parameter
        constexpr uint16_t big_parameter_plength = parameter_length - 4;
        buffer[encapsulation_length] = static_cast<octet>(pid & 0xFF);
        buffer[encapsulation_length + 1] = static_cast<octet>((pid >> 8) & 0xFF);
        buffer[encapsulation_length + 2] = static_cast<octet>(big_parameter_plength & 0xFF);
        buffer[encapsulation_length + 3] = static_cast<octet>((big_parameter_plength >> 8) & 0xFF);

        // Beware of semantically incorrect parameters
        switch (pid)
        {
            // The protocol version should be 2
            case eprosima::fastdds::dds::PID_PROTOCOL_VERSION:
            {
                buffer[encapsulation_length + 4] = 0x02;
            }
            break;

            // The length of some string parameters should be lower than 256
            case eprosima::fastdds::dds::PID_ENTITY_NAME:
            case eprosima::fastdds::dds::PID_TYPE_NAME:
            case eprosima::fastdds::dds::PID_TOPIC_NAME:
            {
                buffer[encapsulation_length + 2] = 0xFF;
                buffer[encapsulation_length + 3] = 0x00;
            }
            break;

            // Data parameters should fill the whole parameter
            case eprosima::fastdds::dds::PID_USER_DATA:
            case eprosima::fastdds::dds::PID_TOPIC_DATA:
            case eprosima::fastdds::dds::PID_GROUP_DATA:
            {
                constexpr uint16_t inner_data_length = big_parameter_plength - 4;
                buffer[encapsulation_length + 4] = static_cast<octet>(inner_data_length & 0xFF);
                buffer[encapsulation_length + 5] = static_cast<octet>((inner_data_length >> 8) & 0xFF);
            }
            break;

            // Custom content for partition
            case eprosima::fastdds::dds::PID_PARTITION:
            {
                // Number of partitions (1)
                buffer[encapsulation_length + 4] = 0x01;
                buffer[encapsulation_length + 5] = 0x00;
                buffer[encapsulation_length + 6] = 0x00;
                buffer[encapsulation_length + 7] = 0x00;
                // Partition name length (fills the rest of the parameter)
                constexpr uint16_t partition_length = big_parameter_plength - 4 - 4;
                buffer[encapsulation_length + 8] = static_cast<octet>(partition_length & 0xFF);
                buffer[encapsulation_length + 9] = static_cast<octet>((partition_length >> 8) & 0xFF);
                buffer[encapsulation_length + 10] = 0x00;
                buffer[encapsulation_length + 11] = 0x00;
            }
            break;

            // Custom content for security tokens
            case eprosima::fastdds::dds::PID_IDENTITY_TOKEN:
            case eprosima::fastdds::dds::PID_PERMISSIONS_TOKEN:
            {
                // Content is a string + properties (0) + binary properties (0)
                // Should fill the whole parameter
                constexpr uint16_t token_string_length = big_parameter_plength - 4 - 4 - 4;
                buffer[encapsulation_length + 4] = static_cast<octet>(token_string_length & 0xFF);
                buffer[encapsulation_length + 5] = static_cast<octet>((token_string_length >> 8) & 0xFF);
            }
            break;
        }

        // Deserialize a DATA(p)
        {
            CDRMessage_t msg(0);
            msg.init(buffer.data(), static_cast<uint32_t>(buffer.size()));
            msg.length = msg.max_size;

            RTPSParticipantAllocationAttributes att;
            att.data_limits.max_user_data = parameter_length;
            ParticipantProxyData out({});
            EXPECT_NO_THROW(
                if (!out.read_from_cdr_message(&msg, true, network, false))
                        {
                            failed_for_data_p.insert(pid);
                        }
                );
        }

        // Deserialize a DATA(w)
        {
            CDRMessage_t msg(0);
            msg.init(buffer.data(), static_cast<uint32_t>(buffer.size()));
            msg.length = msg.max_size;

            VariableLengthDataLimits limits;
            limits.max_user_data = parameter_length;
            WriterProxyData out(max_unicast_locators, max_multicast_locators, limits);
            EXPECT_NO_THROW(
                if (!out.read_from_cdr_message(&msg))
                        {
                            failed_for_data_w.insert(pid);
                        }
                );
        }

        // Deserialize a DATA(r)
        {
            CDRMessage_t msg(0);
            msg.init(buffer.data(), static_cast<uint32_t>(buffer.size()));
            msg.length = msg.max_size;

            VariableLengthDataLimits limits;
            limits.max_user_data = parameter_length;
            ReaderProxyData out(max_unicast_locators, max_multicast_locators, limits);
            EXPECT_NO_THROW(
                if (!out.read_from_cdr_message(&msg))
                        {
                            failed_for_data_r.insert(pid);
                        }
                );
        }
    }

    // Check if any parameter ID failed
    EXPECT_EQ(failed_for_data_p.size(), 0u);
    EXPECT_EQ(failed_for_data_w.size(), 0u);
    EXPECT_EQ(failed_for_data_r.size(), 0u);

    // Print the failed parameter IDs
    if (!failed_for_data_p.empty())
    {
        std::cout << "Failed for DATA(p): ";
        for (uint16_t pid : failed_for_data_p)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << pid << " ";
        }
        std::cout << std::endl;
    }
    if (!failed_for_data_w.empty())
    {
        std::cout << "Failed for DATA(w): ";
        for (uint16_t pid : failed_for_data_w)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << pid << " ";
        }
        std::cout << std::endl;
    }
    if (!failed_for_data_r.empty())
    {
        std::cout << "Failed for DATA(r): ";
        for (uint16_t pid : failed_for_data_r)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << pid << " ";
        }
        std::cout << std::endl;
    }
}

/*!
 * This is a regression test for redmine issue #19927
 *
 * It checks that proxy data for readers and writers can only be updated if the security attributes are equal.
 */
TEST(BuiltinDataSerializationTests, security_attributes_update)
{
    // Only if security is enabled
#if HAVE_SECURITY

    // Test for ReaderProxyData
    {
        ReaderProxyData original(max_unicast_locators, max_multicast_locators);
        original.security_attributes_ = 0x01;
        original.plugin_security_attributes_ = 0x02;

        ReaderProxyData updated(original);
        EXPECT_TRUE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.security_attributes_ + 10;
        updated.plugin_security_attributes_ = original.plugin_security_attributes_;
        EXPECT_FALSE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.security_attributes_;
        updated.plugin_security_attributes_ = original.plugin_security_attributes_ + 10;
        EXPECT_FALSE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.plugin_security_attributes_;
        updated.plugin_security_attributes_ = original.plugin_security_attributes_;
        EXPECT_FALSE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.security_attributes_;
        updated.plugin_security_attributes_ = original.security_attributes_;
        EXPECT_FALSE(original.is_update_allowed(updated));
    }

    // Test for WriterProxyData
    {
        WriterProxyData original(max_unicast_locators, max_multicast_locators);
        original.security_attributes_ = 0x01;
        original.plugin_security_attributes_ = 0x02;

        WriterProxyData updated(original);
        EXPECT_TRUE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.security_attributes_ + 10;
        updated.plugin_security_attributes_ = original.plugin_security_attributes_;
        EXPECT_FALSE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.security_attributes_;
        updated.plugin_security_attributes_ = original.plugin_security_attributes_ + 10;
        EXPECT_FALSE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.plugin_security_attributes_;
        updated.plugin_security_attributes_ = original.plugin_security_attributes_;
        EXPECT_FALSE(original.is_update_allowed(updated));

        updated.security_attributes_ = original.security_attributes_;
        updated.plugin_security_attributes_ = original.security_attributes_;
        EXPECT_FALSE(original.is_update_allowed(updated));
    }

#endif  // HAVE_SECURITY
}

/*!
 * This test checks that a correct ReaderProxyData is obtained
 * from eProsima's optional qos extensions in SubscriptionBuiltinTopicData
 */
TEST(BuiltinDataSerializationTests, optional_qos_extensions_reader)
{
    // DATA(r)
    uint8_t data_r_buffer[] =
    {
        // Encapsulation
        0x00, 0x03, 0x00, 0x00,
        // Reader GUID
        0x5a, 0x00, 0x10, 0x00,
        0x01, 0x05, 0x0f, 0xda, 0x14, 0xdd, 0x32, 0x62, 0x74, 0xef, 0x08, 0xeb, 0x00, 0x00, 0x01, 0x07,
        // Participant GUID
        0x50, 0x00, 0x10, 0x00,
        0x01, 0x05, 0x0f, 0xda, 0x14, 0xdd, 0x32, 0x62, 0x74, 0xef, 0x08, 0xeb, 0x00, 0x00, 0x01, 0xc1,
        // Topic name
        0x05, 0x00, 0x0c, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x53, 0x71, 0x75, 0x61, 0x72, 0x65, 0x00, 0x00,
        // Type name
        0x07, 0x00, 0x10, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
        // Type information
        0x75, 0x00, 0x5c, 0x00,
        0x58, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
        0xf1, 0xa5, 0x12, 0xf3, 0x95, 0xe2, 0xba, 0xb0, 0xb9, 0xfc, 0x83, 0x8e, 0x08, 0x6e, 0x2c, 0x00,
        0x57, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xf2, 0x77, 0x32, 0x07,
        0xfb, 0x72, 0x38, 0x6e, 0x0d, 0xdb, 0x0e, 0x1a, 0x2b, 0x4f, 0xbe, 0x00, 0x84, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // History
        0x40, 0x00, 0x08, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        // Resource limits
        0x41, 0x00, 0x0c, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        // Reader Data Lifecycle
        0x00, 0x82, 0x10, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // RTPS Reliable Reader
        0x01, 0x82, 0x1c, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        // Endpoint
        0x10, 0x80, 0x38, 0x00,//56 (1 locator)
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        // Reader Resource Limits
        0x02, 0x82, 0x4C, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        // Sentinel
        0x01, 0x00, 0x00, 0x00
    };

    CDRMessage_t msg(0);
    msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
    msg.length = msg.max_size;

    ReaderProxyData rpd(max_unicast_locators, max_multicast_locators);
    EXPECT_NO_THROW(EXPECT_TRUE(rpd.read_from_cdr_message(&msg, c_VendorId_eProsima)));

    ASSERT_TRUE(rpd.history);
    ASSERT_EQ(rpd.history->kind, dds::KEEP_ALL_HISTORY_QOS);
    ASSERT_EQ(rpd.history->depth, 1);

    ASSERT_TRUE(rpd.resource_limits);
    ASSERT_EQ(rpd.resource_limits->max_samples, 1);
    ASSERT_EQ(rpd.resource_limits->max_instances, 2);
    ASSERT_EQ(rpd.resource_limits->max_samples_per_instance, 3);

    ASSERT_TRUE(rpd.reader_data_lifecycle);
    ASSERT_EQ(rpd.reader_data_lifecycle->autopurge_no_writer_samples_delay.seconds, 2);
    ASSERT_EQ(rpd.reader_data_lifecycle->autopurge_disposed_samples_delay.seconds, 4);

    ASSERT_TRUE(rpd.rtps_reliable_reader);
    ASSERT_EQ(rpd.rtps_reliable_reader->times.initial_acknack_delay.seconds, 3);
    ASSERT_EQ(rpd.rtps_reliable_reader->times.heartbeat_response_delay.seconds, 5);
    ASSERT_TRUE(rpd.rtps_reliable_reader->disable_positive_acks.enabled);
    ASSERT_EQ(rpd.rtps_reliable_reader->disable_positive_acks.duration.seconds, 16);

    ASSERT_TRUE(rpd.endpoint);
    ASSERT_EQ(rpd.endpoint->unicast_locator_list.size(), 1u);
    ASSERT_EQ(rpd.endpoint->unicast_locator_list.begin()->kind, LOCATOR_KIND_UDPv4);
    ASSERT_EQ(rpd.endpoint->unicast_locator_list.begin()->port, 5u);
    ASSERT_EQ(rpd.endpoint->unicast_locator_list.begin()->address[0], (octet)127);
    ASSERT_EQ(rpd.endpoint->multicast_locator_list.size(), 0u);
    ASSERT_EQ(rpd.endpoint->remote_locator_list.size(), 0u);
    ASSERT_TRUE(rpd.endpoint->ignore_non_matching_locators);
    ASSERT_EQ(rpd.endpoint->entity_id, 16);
    ASSERT_EQ(rpd.endpoint->user_defined_id, 16);
    ASSERT_EQ(rpd.endpoint->history_memory_policy, DYNAMIC_RESERVE_MEMORY_MODE);

    ASSERT_TRUE(rpd.reader_resource_limits);
    ASSERT_EQ(rpd.reader_resource_limits->matched_publisher_allocation.initial, 1u);
    ASSERT_EQ(rpd.reader_resource_limits->matched_publisher_allocation.maximum, 2u);
    ASSERT_EQ(rpd.reader_resource_limits->matched_publisher_allocation.increment, 3u);
    ASSERT_EQ(rpd.reader_resource_limits->sample_infos_allocation.initial, 4u);
    ASSERT_EQ(rpd.reader_resource_limits->sample_infos_allocation.maximum, 5u);
    ASSERT_EQ(rpd.reader_resource_limits->sample_infos_allocation.increment, 6u);
    ASSERT_EQ(rpd.reader_resource_limits->outstanding_reads_allocation.initial, 7u);
    ASSERT_EQ(rpd.reader_resource_limits->outstanding_reads_allocation.maximum, 8u);
    ASSERT_EQ(rpd.reader_resource_limits->outstanding_reads_allocation.increment, 9u);
    ASSERT_EQ(rpd.reader_resource_limits->max_samples_per_read, 16);
}

/*!
 * This test checks that a correct WriterProxyData is obtained
 * from eProsima's optional qos extensions in PublicationBuiltinTopicData
 */
TEST(BuiltinDataSerializationTests, optional_qos_extensions_writer)
{
    // DATA(w)
    octet data_w_buffer[] =
    {
        // Encapsulation
        0x00, 0x03, 0x00, 0x00,
        // Writer GUID
        0x5a, 0x00, 0x10, 0x00,
        0x01, 0x05, 0xfa, 0xd5, 0x7a, 0x09, 0x6a, 0x22, 0x84, 0xfb, 0x23, 0xa2, 0x00, 0x00, 0x01, 0x02,
        // Participant GUID
        0x50, 0x00, 0x10, 0x00,
        0x01, 0x05, 0xfa, 0xd5, 0x7a, 0x09, 0x6a, 0x22, 0x84, 0xfb, 0x23, 0xa2, 0x00, 0x00, 0x01, 0xc1,
        // Topic name
        0x05, 0x00, 0x0c, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x53, 0x71, 0x75, 0x61, 0x72, 0x65, 0x00, 0x00,
        // Type name
        0x07, 0x00, 0x10, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
        // Type information
        0x75, 0x00, 0x5c, 0x00,
        0x58, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
        0xf1, 0xa5, 0x12, 0xf3, 0x95, 0xe2, 0xba, 0xb0, 0xb9, 0xfc, 0x83, 0x8e, 0x08, 0x6e, 0x2c, 0x00,
        0x57, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x10, 0x00, 0x50, 0x24, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xf2, 0x77, 0x32, 0x07,
        0xfb, 0x72, 0x38, 0x6e, 0x0d, 0xdb, 0x0e, 0x1a, 0x2b, 0x4f, 0xbe, 0x00, 0x84, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // History
        0x40, 0x00, 0x08, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        // Resource limits
        0x41, 0x00, 0x0c, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        // Endpoint
        0x10, 0x80, 0x38, 0x00,//56 (1 locator)
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        // Writer Data Lifecycle
        0x00, 0x81, 0x04, 0x00,
        0x01, 0x00, 0x00, 0x00,
        // Publish Mode
        0x01, 0x81, 0x10, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, '\0',
        // RTPS Reliable Writer
        0x02, 0x81, 0x30, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        // Writer Resource Limits
        0x03, 0x81, 0x30, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Sentinel
        0x01, 0x00, 0x00, 0x00
    };

    CDRMessage_t msg(0);
    msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
    msg.length = msg.max_size;

    WriterProxyData wpd(max_unicast_locators, max_multicast_locators);
    EXPECT_NO_THROW(EXPECT_TRUE(wpd.read_from_cdr_message(&msg, c_VendorId_eProsima)));

    ASSERT_TRUE(wpd.history);
    ASSERT_EQ(wpd.history->kind, dds::KEEP_ALL_HISTORY_QOS);
    ASSERT_EQ(wpd.history->depth, 1);

    ASSERT_TRUE(wpd.resource_limits);
    ASSERT_EQ(wpd.resource_limits->max_samples, 1);
    ASSERT_EQ(wpd.resource_limits->max_instances, 2);
    ASSERT_EQ(wpd.resource_limits->max_samples_per_instance, 3);

    ASSERT_TRUE(wpd.endpoint);
    ASSERT_EQ(wpd.endpoint->unicast_locator_list.size(), 1u);
    ASSERT_EQ(wpd.endpoint->unicast_locator_list.begin()->kind, LOCATOR_KIND_UDPv4);
    ASSERT_EQ(wpd.endpoint->unicast_locator_list.begin()->port, 5u);
    ASSERT_EQ(wpd.endpoint->unicast_locator_list.begin()->address[0], (octet)127);
    ASSERT_EQ(wpd.endpoint->multicast_locator_list.size(), 0u);
    ASSERT_EQ(wpd.endpoint->remote_locator_list.size(), 0u);
    ASSERT_TRUE(wpd.endpoint->ignore_non_matching_locators);
    ASSERT_EQ(wpd.endpoint->entity_id, 16);
    ASSERT_EQ(wpd.endpoint->user_defined_id, 16);
    ASSERT_EQ(wpd.endpoint->history_memory_policy, DYNAMIC_RESERVE_MEMORY_MODE);

    ASSERT_TRUE(wpd.writer_data_lifecycle);
    ASSERT_TRUE(wpd.writer_data_lifecycle->autodispose_unregistered_instances);

    ASSERT_TRUE(wpd.publish_mode);
    ASSERT_EQ(wpd.publish_mode->kind, dds::ASYNCHRONOUS_PUBLISH_MODE);
    ASSERT_EQ(wpd.publish_mode->flow_controller_name, "example");

    ASSERT_TRUE(wpd.rtps_reliable_writer);
    ASSERT_EQ(wpd.rtps_reliable_writer->times.initial_heartbeat_delay.seconds, 1);
    ASSERT_EQ(wpd.rtps_reliable_writer->times.heartbeat_period.seconds, 3);
    ASSERT_EQ(wpd.rtps_reliable_writer->times.nack_response_delay.seconds, 5);
    ASSERT_EQ(wpd.rtps_reliable_writer->times.nack_supression_duration.seconds, 7);
    ASSERT_TRUE(wpd.rtps_reliable_writer->disable_positive_acks.enabled);
    ASSERT_EQ(wpd.rtps_reliable_writer->disable_positive_acks.duration.seconds, 9);
    ASSERT_TRUE(wpd.rtps_reliable_writer->disable_heartbeat_piggyback);

    ASSERT_TRUE(wpd.writer_resource_limits);
    ASSERT_EQ(wpd.writer_resource_limits->matched_subscriber_allocation.initial, 1u);
    ASSERT_EQ(wpd.writer_resource_limits->matched_subscriber_allocation.maximum, 2u);
    ASSERT_EQ(wpd.writer_resource_limits->matched_subscriber_allocation.increment, 3u);
    ASSERT_EQ(wpd.writer_resource_limits->reader_filters_allocation.initial, 4u);
    ASSERT_EQ(wpd.writer_resource_limits->reader_filters_allocation.maximum, 5u);
    ASSERT_EQ(wpd.writer_resource_limits->reader_filters_allocation.increment, 6u);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
