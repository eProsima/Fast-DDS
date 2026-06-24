// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cstddef>
#include <cstdlib>
#include <utility>

#include <gtest/gtest.h>

#include <fastdds/rtps/common/SerializedPayload.hpp>

using namespace eprosima::fastdds::rtps;

class TestingPayloadPool : public IPayloadPool
{
public:

    ~TestingPayloadPool() override = default;

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        payload.data = static_cast<octet*>(calloc(size, sizeof(octet)));
        if (payload.data == nullptr)
        {
            return false;
        }

        payload.payload_owner = this;
        payload.max_size = size;
        payload.length = size;
        return true;
    }

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        if (!get_payload(data.length, payload))
        {
            return false;
        }

        payload.copy(&data);
        return true;
    }

    bool release_payload(
            SerializedPayload_t& payload) override
    {
        ++release_payload_calls;
        EXPECT_EQ(this, payload.payload_owner);

        free(payload.data);
        payload.data = nullptr;
        payload.payload_owner = nullptr;
        payload.length = 0;
        payload.max_size = 0;
        return true;
    }

    size_t release_payload_calls = 0;
};

/*!
 * @fn TEST(SerializedPayload, MoveAssignmentOperatorReleasesCurrentPayload)
 * @brief This test checks that move assignment operator releases an already owned destination payload.
 */
TEST(SerializedPayload, MoveAssignmentOperatorReleasesCurrentPayload)
{
    TestingPayloadPool payload_pool;
    SerializedPayload_t destination;
    ASSERT_TRUE(payload_pool.get_payload(4, destination));

    SerializedPayload_t source(8);
    source.length = 8;

    destination = std::move(source);

    EXPECT_EQ(1u, payload_pool.release_payload_calls);
    EXPECT_EQ(8u, destination.length);
    EXPECT_EQ(8u, destination.max_size);
    EXPECT_NE(nullptr, destination.data);
    EXPECT_EQ(nullptr, destination.payload_owner);
    EXPECT_EQ(0u, source.length);
    EXPECT_EQ(0u, source.max_size);
    EXPECT_EQ(nullptr, source.data);
}

/*!
 * @fn TEST(SerializedPayload, MoveAssignmentOperatorReleasesCurrentUnownedPayload)
 * @brief This test checks that move assignment operator releases an already allocated destination payload.
 */
TEST(SerializedPayload, MoveAssignmentOperatorReleasesCurrentUnownedPayload)
{
    SerializedPayload_t destination(4);
    ASSERT_NE(nullptr, destination.data);

    SerializedPayload_t source(8);
    source.length = 8;

    destination = std::move(source);

    EXPECT_EQ(8u, destination.length);
    EXPECT_EQ(8u, destination.max_size);
    EXPECT_NE(nullptr, destination.data);
    EXPECT_EQ(nullptr, destination.payload_owner);
    EXPECT_EQ(0u, source.length);
    EXPECT_EQ(0u, source.max_size);
    EXPECT_EQ(nullptr, source.data);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
