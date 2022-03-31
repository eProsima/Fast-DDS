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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastdds/core/policy/ParameterSerializer.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

constexpr size_t max_unicast_locators = 4u;
constexpr size_t max_multicast_locators = 1u;

NetworkFactory network;

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

TEST(BuiltinDataSerializationTests, ok_with_defaults)
{
    {
        WriterProxyData in(max_unicast_locators, max_multicast_locators);
        WriterProxyData out(max_unicast_locators, max_multicast_locators);

        // Topic and type name cannot be empty
        in.topicName("TEST");
        in.typeName("TestType");

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_TRUE(in.writeToCDRMessage(&msg, true));

        // Perform deserialization
        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));
        // EXPECT_EQ(in, out);
    }

    {
        ReaderProxyData in(max_unicast_locators, max_multicast_locators);
        ReaderProxyData out(max_unicast_locators, max_multicast_locators);

        // Topic and type name cannot be empty
        in.topicName("TEST");
        in.typeName("TestType");

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_TRUE(in.writeToCDRMessage(&msg, true));

        // Perform deserialization
        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));
    }
}

// Regression test for redmine issue #10547
TEST(BuiltinDataSerializationTests, ignore_unsupported_type_info)
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
            // Type information
            0x75, 0x00, 0x50, 0x00,
            0x4c, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
            0x14, 0x00, 0x00, 0x00, 0xf1, 0x80, 0x99, 0x5e, 0xfc, 0xdb, 0xda, 0xbe, 0xd5, 0xb3, 0x3d, 0xe3,
            0xea, 0x3a, 0x4b, 0x00, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x10, 0x00, 0x40, 0x18, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Type name
            0x07, 0x00, 0x10, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
            // Reliability
            0x1a, 0x00, 0x0c, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0xf5, 0x05,
            // Data representation
            0x73, 0x00, 0x08, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            // Endpoint GUID
            0x5a, 0x00, 0x10, 0x00,
            0x01, 0x03, 0x08, 0x00, 0x27, 0x5c, 0x4f, 0x05, 0x0f, 0x19, 0x05, 0xea, 0x00, 0x00, 0x00, 0x02,
            // Multicast locator
            0x30, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xe9, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xef, 0xff, 0x00, 0x02,
            // Unicast locator
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x3e, 0xcd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0xb4,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_w_buffer, static_cast<uint32_t>(sizeof(data_w_buffer)));
        msg.length = msg.max_size;

        WriterProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.readFromCDRMessage(&msg, network, false)));
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
            0x75, 0x00, 0x50, 0x00,
            0x4c, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
            0x14, 0x00, 0x00, 0x00, 0xf1, 0x80, 0x99, 0x5e, 0xfc, 0xdb, 0xda, 0xbe, 0xd5, 0xb3, 0x3d, 0xe3,
            0xea, 0x3a, 0x4b, 0x00, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x02, 0x10, 0x00, 0x40, 0x18, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            // Type name
            0x07, 0x00, 0x10, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x53, 0x68, 0x61, 0x70, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x00, 0x00,
            // Reliability
            0x1a, 0x00, 0x0c, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f,
            // Endpoint GUID
            0x5a, 0x00, 0x10, 0x00,
            0x01, 0x03, 0x08, 0x00, 0x27, 0x5c, 0x4f, 0x05, 0x0f, 0x40, 0x29, 0x9d, 0x00, 0x00, 0x00, 0x07,
            // Data representation
            0x73, 0x00, 0x08, 0x00,
            0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            // Multicast locator
            0x30, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0xe9, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xef, 0xff, 0x00, 0x02,
            // Unicast locator
            0x2f, 0x00, 0x18, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x45, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0xb4,
            // Type consistency
            0x74, 0x00, 0x08, 0x00,
            0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
            // Sentinel
            0x01, 0x00, 0x00, 0x00
        };

        CDRMessage_t msg(0);
        msg.init(data_r_buffer, static_cast<uint32_t>(sizeof(data_r_buffer)));
        msg.length = msg.max_size;

        ReaderProxyData out(max_unicast_locators, max_multicast_locators);
        EXPECT_NO_THROW(EXPECT_TRUE(out.readFromCDRMessage(&msg, network, false)));
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
        EXPECT_NO_THROW(EXPECT_TRUE(out.readFromCDRMessage(&msg, network, false)));
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
    in.topicName("TEST");
    in.typeName("TestType");

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "MyFilterClass";
    content_filter_property.filter_expression = "This is a custom test filter expression";
    in.content_filter(content_filter_property);

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.writeToCDRMessage(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

    ASSERT_EQ(in.content_filter().content_filtered_topic_name, out.content_filter().content_filtered_topic_name);
    ASSERT_EQ(in.content_filter().related_topic_name, out.content_filter().related_topic_name);
    ASSERT_EQ(in.content_filter().filter_class_name, out.content_filter().filter_class_name);
    ASSERT_EQ(in.content_filter().filter_expression, out.content_filter().filter_expression);
    ASSERT_EQ(in.content_filter().expression_parameters.size(), out.content_filter().expression_parameters.size());
}

/*!
 * \test RTPS-CFT-CFP-02 Tests serialization of `ContentFilterProperty_t` works successfully with parameters.
 */
TEST(BuiltinDataSerializationTests, contentfilterproperty_with_parameters)
{
    ReaderProxyData in(max_unicast_locators, max_multicast_locators);
    ReaderProxyData out(max_unicast_locators, max_multicast_locators);

    // Topic and type name cannot be empty
    in.topicName("TEST");
    in.typeName("TestType");

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
    in.content_filter(content_filter_property);

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.writeToCDRMessage(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

    ASSERT_EQ(in.content_filter().content_filtered_topic_name, out.content_filter().content_filtered_topic_name);
    ASSERT_EQ(in.content_filter().related_topic_name, out.content_filter().related_topic_name);
    ASSERT_EQ(in.content_filter().filter_class_name, out.content_filter().filter_class_name);
    ASSERT_EQ(in.content_filter().filter_expression, out.content_filter().filter_expression);
    ASSERT_EQ(in.content_filter().expression_parameters.size(), out.content_filter().expression_parameters.size());
    ASSERT_EQ(in.content_filter().expression_parameters[0], out.content_filter().expression_parameters[0]);
    ASSERT_EQ(in.content_filter().expression_parameters[1], out.content_filter().expression_parameters[1]);
    ASSERT_EQ(in.content_filter().expression_parameters[2], out.content_filter().expression_parameters[2]);
    ASSERT_EQ(in.content_filter().expression_parameters[3], out.content_filter().expression_parameters[3]);
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
        in.topicName("TEST");
        in.typeName("TestType");

        // Fill ContentFilterProperty_t without parameters.
        fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
        fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
        content_filter_property.related_topic_name = "TEST";
        content_filter_property.filter_class_name = "MyFilterClass";
        content_filter_property.filter_expression = "This is a custom test filter expression";
        in.content_filter(content_filter_property);

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_FALSE(in.writeToCDRMessage(&msg, true));
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
        in.topicName("TEST");
        in.typeName("TestType");

        // Fill ContentFilterProperty_t without parameters.
        fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
        fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
        content_filter_property.content_filtered_topic_name = "CFT_TEST";
        content_filter_property.filter_class_name = "MyFilterClass";
        content_filter_property.filter_expression = "This is a custom test filter expression";
        in.content_filter(content_filter_property);

        // Perform serialization
        uint32_t msg_size = in.get_serialized_size(true);
        CDRMessage_t msg(msg_size);
        EXPECT_FALSE(in.writeToCDRMessage(&msg, true));
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
    in.topicName("TEST");
    in.typeName("TestType");

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "";
    content_filter_property.filter_expression = "This is a custom test filter expression";
    in.content_filter(content_filter_property);

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.writeToCDRMessage(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

    ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
    ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
    ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
    ASSERT_EQ("", out.content_filter().filter_expression);
    ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
    in.topicName("TEST");
    in.typeName("TestType");

    // Fill ContentFilterProperty_t without parameters.
    fastdds::rtps::ContentFilterProperty::AllocationConfiguration content_filter_allocation;
    fastdds::rtps::ContentFilterProperty content_filter_property(content_filter_allocation);
    content_filter_property.content_filtered_topic_name = "CFP_TEST";
    content_filter_property.related_topic_name = "TEST";
    content_filter_property.filter_class_name = "";
    content_filter_property.filter_expression = "This is a custom test filter expression";
    in.content_filter(content_filter_property);

    // Perform serialization
    uint32_t msg_size = in.get_serialized_size(true);
    CDRMessage_t msg(msg_size);
    EXPECT_TRUE(in.writeToCDRMessage(&msg, true));

    // Perform deserialization
    msg.pos = 0;
    EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

    ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
    ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
    ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
    ASSERT_EQ("", out.content_filter().filter_expression);
    ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY));
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4)));
            // content_filtered_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name));
            // related_topic_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name));
            // filter_class_name
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name));
            // filter_expression
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression));
            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(expression_parameters.size());
            EXPECT_TRUE(fastrtps::rtps::CDRMessage::addUInt32(&msg, num_params));
            // Add all parameters
            for (const std::string& param : expression_parameters)
            {
                EXPECT_TRUE(fastrtps::rtps::CDRMessage::add_string(&msg, param));
            }
        }
        EXPECT_TRUE(fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(&msg));

        msg.pos = 0;
        EXPECT_TRUE(out.readFromCDRMessage(&msg, network, true));

        ASSERT_EQ("", out.content_filter().content_filtered_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().related_topic_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_class_name.to_string());
        ASSERT_EQ("", out.content_filter().filter_expression);
        ASSERT_EQ(0, out.content_filter().expression_parameters.size());
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
    const std::string related_topic_name(260, 'a');
    const std::string filter_class_name("MyFilterClass");
    const std::string filter_expression("This is a custom test filter expression");
    const std::vector<std::string> expression_parameters = {};
    uint32_t len = manual_content_filter_cdr_serialized_size(
        content_filtered_topic_name,
        related_topic_name,
        filter_class_name,
        filter_expression,
        expression_parameters
        );

    // Not initialized
    {
        CDRMessage_t msg(0);
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::read_from_cdr_message(
                    content_filter_property, &msg, len));
    }
    //Empty buffer but not enough memory.
    {
        CDRMessage_t msg(20);
        fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY);
        fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4));
        // content_filtered_topic_name
        fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name);
        // related_topic_name
        fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name);
        // filter_class_name
        fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name);
        // filter_expression
        fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression);
        ASSERT_FALSE(fastdds::dds::ParameterSerializer<fastdds::rtps::ContentFilterProperty>::read_from_cdr_message(
                    content_filter_property, &msg, len));
    }
    // Used buffer but not enough memory left.
    {
        CDRMessage_t msg(30);
        msg.pos = 10;
        fastrtps::rtps::CDRMessage::addUInt16(&msg, fastdds::dds::PID_CONTENT_FILTER_PROPERTY);
        fastrtps::rtps::CDRMessage::addUInt16(&msg, static_cast<uint16_t>(len - 4));
        // content_filtered_topic_name
        fastrtps::rtps::CDRMessage::add_string(&msg, content_filtered_topic_name);
        // related_topic_name
        fastrtps::rtps::CDRMessage::add_string(&msg, related_topic_name);
        // filter_class_name
        fastrtps::rtps::CDRMessage::add_string(&msg, filter_class_name);
        // filter_expression
        fastrtps::rtps::CDRMessage::add_string(&msg, filter_expression);
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
    EXPECT_NO_THROW(EXPECT_TRUE(out.readFromCDRMessage(&msg, network, false)));

    ASSERT_EQ("ContentFilter_0", out.content_filter().content_filtered_topic_name.to_string());
    ASSERT_EQ("Square", out.content_filter().related_topic_name.to_string());
    ASSERT_EQ("DDSSQL", out.content_filter().filter_class_name.to_string());
    ASSERT_EQ("x > %0 and x < %1 and y > %2 and y < %3", out.content_filter().filter_expression);
    ASSERT_EQ(4, out.content_filter().expression_parameters.size());
    ASSERT_EQ("100", out.content_filter().expression_parameters[0].to_string());
    ASSERT_EQ("200", out.content_filter().expression_parameters[1].to_string());
    ASSERT_EQ("100", out.content_filter().expression_parameters[2].to_string());
    ASSERT_EQ("200", out.content_filter().expression_parameters[3].to_string());
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
