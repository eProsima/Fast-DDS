// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/messages/InlineQos.h>
#include <gtest/gtest.h>

using namespace eprosima;

TEST(InlineQosApi, OriginalWriterInfo)
{
    // Set guid prefix
    fastrtps::rtps::GuidPrefix_t guid_prefix;
    std::memcpy(guid_prefix.value, "0123456789AB", 12);

    // Set entity id
    fastrtps::rtps::EntityId_t entity_id(5126);

    // Set GUID
    fastrtps::rtps::GUID_t guid(guid_prefix, entity_id);

    // Set sequence number
    fastrtps::rtps::SequenceNumber_t sequence_number(68u, 287u);

    // Blank inline qos
    fastrtps::rtps::SerializedPayload_t inline_qos;

    ASSERT_EQ(inline_qos.max_size, 0);

    // Serialize
    bool append_success = fastrtps::rtps::InlineQoS::append_parameters(
        inline_qos,
        fastdds::dds::ParameterOriginalWriterInfo_t(guid, sequence_number)
        );

    ASSERT_TRUE(append_success);
 
    // Pid (2 bytes) + Length (2 bytes) + OriginalWriterInfo (24 bytes)
    ASSERT_EQ(inline_qos.max_size, 2 + 2 + 24);
    ASSERT_EQ(inline_qos.length, 2 + 2 + 24);

    // position starts at the beginning of the buffer
    ASSERT_EQ(inline_qos.pos, 0);

    // Deserialize
    auto params = fastrtps::rtps::InlineQoS::read_parameters<fastdds::dds::ParameterOriginalWriterInfo_t>(
        inline_qos
        );

    ASSERT_EQ(fastrtps::rtps::get_parameter<fastdds::dds::ParameterOriginalWriterInfo_t>(params).writer_guid, guid);
    ASSERT_EQ(fastrtps::rtps::get_parameter<fastdds::dds::ParameterOriginalWriterInfo_t>(params).sequence_number, sequence_number);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
