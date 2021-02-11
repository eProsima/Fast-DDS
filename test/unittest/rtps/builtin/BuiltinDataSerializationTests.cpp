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

namespace eprosima {
namespace fastrtps {
namespace rtps {

constexpr size_t max_unicast_locators = 4u;
constexpr size_t max_multicast_locators = 1u;

NetworkFactory network;

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
