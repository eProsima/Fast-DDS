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

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace testing;

class TestPayloadPool : public IPayloadPool
{

public:

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        bool result = get_payload_delegate(size, payload);
        if (result)
        {
            payload.payload_owner = this;
        }
        return result;
    }

    MOCK_METHOD2(get_payload_delegate,
            bool(uint32_t size, SerializedPayload_t & payload));


    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        bool result = get_payload_delegate(data, payload);
        if (result)
        {
            payload.payload_owner = this;
        }
        return result;
    }

    MOCK_METHOD2(get_payload_delegate,
            bool(const SerializedPayload_t& data, SerializedPayload_t & payload));

    bool release_payload (
            SerializedPayload_t& payload) override
    {
        bool result = release_payload_delegate(payload);
        if (result)
        {
            payload.payload_owner = nullptr;
        }
        return result;
    }

    MOCK_METHOD1(release_payload_delegate,
            bool(SerializedPayload_t & payload));
};

class TestDataType
{
public:

    constexpr static size_t data_size = 250;

};

void pool_initialization_test (
        MemoryManagementPolicy_t policy)
{
    uint32_t domain_id = 0;
    uint32_t initial_reserved_caches = 10;

    RTPSParticipantAttributes p_attr;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    ASSERT_NE(participant, nullptr);

    std::shared_ptr<TestPayloadPool> pool;

    HistoryAttributes h_attr;
    h_attr.memoryPolicy = policy;
    h_attr.initialReservedCaches = initial_reserved_caches;
    h_attr.payloadMaxSize = TestDataType::data_size;
    WriterHistory* history = new WriterHistory(h_attr, pool);

    WriterAttributes w_attr;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, history);
    EXPECT_EQ(nullptr, writer);

    delete history;
    pool = std::make_shared<TestPayloadPool>();
    history = new WriterHistory(h_attr, pool);

    // Creating the Writer initializes the PayloadPool with the initial reserved size
    EXPECT_CALL(*pool, get_payload_delegate(TestDataType::data_size, _))
            .Times(0);
    EXPECT_CALL(*pool, release_payload_delegate(_))
            .Times(0);

    writer = RTPSDomain::createRTPSWriter(participant, w_attr, history);

    Mock::VerifyAndClearExpectations(pool.get());

    // Changes requested to the writer have a payload taken from the pool
    EXPECT_CALL(*pool, get_payload_delegate(TestDataType::data_size, _))
            .Times(1)
            .WillOnce(Return(true));

    TestDataType data;
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);
    size_t current_alignment{ 0 };
    uint32_t payload_size = (uint32_t)calculator.calculate_serialized_size(data, current_alignment);
    CacheChange_t* ch = history->create_change(payload_size, ALIVE);
    ASSERT_NE(ch, nullptr);

    // Changes released to the writer have the payload returned to the pool
    EXPECT_CALL(*pool, release_payload_delegate(_))
            .Times(1)
            .WillOnce(Return(true));

    history->release_change(ch);

    RTPSDomain::removeRTPSWriter(writer);
    RTPSDomain::removeRTPSParticipant(participant);
    delete(history);
}

TEST(RTPSWriterTests, WriterWithCustomPayloadPool_InitializesPool_WhenPreallocatedMode)
{
    pool_initialization_test(PREALLOCATED_MEMORY_MODE);
}

TEST(RTPSWriterTests, WriterWithCustomPayloadPool_InitializesPool_WhenPreallocatedWithReallocMode)
{
    pool_initialization_test(PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
}

TEST(RTPSWriterTests, WriterWithCustomPayloadPool_DoesNotInitializePool_WhenDynamicMode)
{
    pool_initialization_test(DYNAMIC_RESERVE_MEMORY_MODE);
}

TEST(RTPSWriterTests, WriterWithCustomPayloadPool_DoesNotInitializePool_WhenDynamicReusableMode)
{
    pool_initialization_test(DYNAMIC_REUSABLE_MEMORY_MODE);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

namespace eprosima {
namespace fastcdr {
template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastdds::rtps::TestDataType&,
        size_t& current_alignment)
{
    size_t calculated_size {eprosima::fastdds::rtps::TestDataType::data_size};
    current_alignment += calculated_size;
    return calculated_size;
}

} // namespace fastcdr
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
