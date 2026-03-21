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

#include <gtest/gtest.h>

#include <fastdds/rtps/common/CacheChange.hpp>

#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>

#include <tuple>

using namespace eprosima::fastdds::rtps;
using namespace ::testing;
using namespace std;

class BasicPoolsTest : public TestWithParam<tuple<uint32_t, uint32_t, uint32_t, MemoryManagementPolicy>>
{
protected:

    BasicPoolsTest()
        : pool_size_(10)
        , max_pool_size_(0)
        , payload_size_(128)
        , memory_policy_(MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE)
    {
    }

    virtual ~BasicPoolsTest()
    {
    }

    virtual void SetUp()
    {
        pool_size_     = get<0>(GetParam());
        max_pool_size_ = get<1>(GetParam());
        payload_size_  = get<2>(GetParam());
        memory_policy_ = get<3>(GetParam());

        PoolConfig cfg { memory_policy_, payload_size_, pool_size_, max_pool_size_ };
        payload_pool_ = BasicPayloadPool::get(cfg, change_pool_);
    }

    virtual void TearDown()
    {
        change_pool_.reset();
        payload_pool_.reset();
    }

    bool reserve_cache(
            CacheChange_t*& change,
            uint32_t data_size)
    {
        if (!change_pool_->reserve_cache(change))
        {
            return false;
        }

        if (!payload_pool_->get_payload(data_size, change->serializedPayload))
        {
            change_pool_->release_cache(change);
            return false;
        }

        return true;
    }

    void release_cache(
            CacheChange_t*& change)
    {
        payload_pool_->release_payload(change->serializedPayload);
        change_pool_->release_cache(change);
    }

    uint32_t pool_size_;
    uint32_t max_pool_size_;
    uint32_t payload_size_;
    MemoryManagementPolicy memory_policy_;

    std::shared_ptr<IPayloadPool> payload_pool_;
    std::shared_ptr<IChangePool> change_pool_;
};

TEST_P(BasicPoolsTest, reserve_cache)
{
    CacheChange_t* ch = nullptr;

    uint32_t payload = payload_size_;
    uint32_t size = static_cast<uint32_t>(pool_size_);
    uint32_t max_size = static_cast<uint32_t>(max_pool_size_);

    uint32_t num_inserts;
    if (max_size == 0)
    {
        num_inserts = 1000U;
    }
    else if (max_size <= size)
    {
        max_size = size;
        num_inserts = size;
    }
    else // max_size > size
    {
        num_inserts = max_size;
    }

    for (uint32_t i = 0; i < num_inserts; i++)
    {
        uint32_t data_size = i * 16;
        ASSERT_TRUE(reserve_cache(ch, data_size));

        switch (memory_policy_)
        {
            case MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, payload);
                break;
            case MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, max(payload, data_size));
                break;
            case MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, data_size);
                break;
            case MemoryManagementPolicy::DYNAMIC_REUSABLE_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, data_size);
                break;
        }
    }

    ASSERT_EQ(max_size == 0, reserve_cache(ch, payload));
}

TEST_P(BasicPoolsTest, change_reset_on_release)
{
    CacheChange_t* ch = nullptr;

    uint32_t data_size = 16;

    ASSERT_TRUE(reserve_cache(ch, data_size));

    // Check that cache change is initilized
    ASSERT_EQ(ch->kind, ALIVE);
    ASSERT_EQ(ch->sequenceNumber.high, 0);
    ASSERT_EQ(ch->sequenceNumber.low, 0U);
    ASSERT_EQ(ch->writerGUID, c_Guid_Unknown);
    ASSERT_EQ(ch->serializedPayload.length, 0U);
    ASSERT_EQ(ch->serializedPayload.pos, 0U);
    ASSERT_FALSE(ch->instanceHandle.isDefined());
    for (uint8_t i = 0; i < 16; ++i)
    {
        ASSERT_EQ(ch->instanceHandle.value[i], 0U);
    }
    ASSERT_FALSE(ch->isRead);
    ASSERT_EQ(ch->sourceTimestamp.seconds(), 0);
    ASSERT_EQ(ch->sourceTimestamp.fraction(), 0U);
    ASSERT_EQ(ch->vendor_id, c_VendorId_Unknown);

    // Modify cache change
    ch->kind = NOT_ALIVE_DISPOSED;
    ch->sequenceNumber.high = 1;
    ch->sequenceNumber.low = 1;
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1);
    ch->serializedPayload.length = 1;
    ch->serializedPayload.pos = 1;
    for (uint8_t i = 0; i < 16; ++i)
    {
        ch->instanceHandle.value[i] = 1;
    }
    ASSERT_TRUE(ch->instanceHandle.isDefined());
    ch->isRead = true;
    ch->sourceTimestamp.seconds(1);
    ch->sourceTimestamp.fraction(1);
    ch->vendor_id = c_VendorId_eProsima;

    release_cache(ch);

    if (memory_policy_ != MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE)
    {
        // Check that cache change is initialized again
        ASSERT_EQ(ch->kind, ALIVE);
        ASSERT_EQ(ch->sequenceNumber.high, 0);
        ASSERT_EQ(ch->sequenceNumber.low, 0U);
        ASSERT_EQ(ch->writerGUID, c_Guid_Unknown);
        ASSERT_EQ(ch->serializedPayload.length, 0U);
        ASSERT_EQ(ch->serializedPayload.pos, 0U);
        ASSERT_FALSE(ch->instanceHandle.isDefined());
        for (uint8_t i = 0; i < 16; ++i)
        {
            ASSERT_EQ(ch->instanceHandle.value[i], 0U);
        }
        ASSERT_FALSE(ch->isRead);
        ASSERT_EQ(ch->sourceTimestamp.seconds(), 0);
        ASSERT_EQ(ch->sourceTimestamp.fraction(), 0U);
        ASSERT_EQ(ch->vendor_id, c_VendorId_Unknown);
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z, )
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    BasicPoolsTest,
    BasicPoolsTest,
    Combine(Values(0, 10, 20, 30),
    Values(0, 10, 20, 30),
    Values(128, 256, 512, 1024),
    Values(MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE,
    MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
    MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE,
    MemoryManagementPolicy::DYNAMIC_REUSABLE_MEMORY_MODE))
    );

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
