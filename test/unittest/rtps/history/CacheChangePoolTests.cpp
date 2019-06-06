// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <fastrtps/rtps/history/CacheChangePool.h>
#include <fastrtps/rtps/common/CacheChange.h>

#include <tuple>

using namespace eprosima::fastrtps::rtps;
using namespace ::testing;
using namespace std;

class CacheChangePoolTests : public TestWithParam<tuple<int32_t, int32_t, uint32_t, MemoryManagementPolicy_t>>
{
protected:
    CacheChangePoolTests()
        : pool(nullptr)
        , pool_size(10)
        , max_pool_size(0)
        , payload_size(128)
        , memory_policy(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
    {
    }

    virtual ~CacheChangePoolTests() {}

    virtual void SetUp()
    {
        pool_size     = get<0>(GetParam());
        max_pool_size = get<1>(GetParam());
        payload_size  = get<2>(GetParam());
        memory_policy = get<3>(GetParam());

        pool = new CacheChangePool(
                pool_size,
                payload_size,
                max_pool_size,
                memory_policy);
    }

    virtual void TearDown()
    {
        delete pool;
    }

    CacheChangePool* pool;

    int32_t pool_size;
    int32_t max_pool_size;
    uint32_t payload_size;
    MemoryManagementPolicy_t memory_policy;
};

TEST_P(CacheChangePoolTests, init_pool)
{
    ASSERT_EQ(pool->getInitialPayloadSize(), payload_size);

    size_t expected_size;
    if (memory_policy == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
    {
        expected_size = 0;
    }
    else {
        expected_size = static_cast<size_t>(pool_size + 1U);
    }

    ASSERT_EQ(pool->get_allCachesSize(), expected_size);
    ASSERT_EQ(pool->get_freeCachesSize(), expected_size);
}

TEST_P(CacheChangePoolTests, reserve_cache)
{
    CacheChange_t* ch = nullptr;

    uint32_t payload = payload_size;
    uint32_t size = static_cast<uint32_t>(pool_size);
    uint32_t max_size = static_cast<uint32_t>(max_pool_size);

    uint32_t num_inserts;
    if (max_size == 0)
    {
        num_inserts = 1000U;
    }
    else if (max_size <= size)
    {
        max_size = size;
        num_inserts = size + 1;
    }
    else // max_size > size
    {
        num_inserts = max_size + 1;
    }

    for (uint32_t i=0; i<num_inserts; i++)
    {
        uint32_t data_size = i*16;
        EXPECT_TRUE(pool->reserve_Cache(&ch, [data_size]() -> uint32_t {return data_size;}));
        EXPECT_EQ(pool->getInitialPayloadSize(), payload);
        EXPECT_GE(pool->get_freeCachesSize(), 0);

        switch (memory_policy)
        {
            case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
                EXPECT_EQ(ch->serializedPayload.max_size, payload);
                break;
            case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                EXPECT_EQ(ch->serializedPayload.max_size, max(payload, data_size));
                break;
            case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
                EXPECT_EQ(ch->serializedPayload.max_size, data_size);
                break;
        }

        if (max_size > 0)
        {
            EXPECT_LE(pool->get_allCachesSize(), max_size + 1U);
        }
    }

    if (max_size == 0)
    {
        EXPECT_TRUE(pool->reserve_Cache(&ch, payload));
    }
    else
    {
        EXPECT_FALSE(pool->reserve_Cache(&ch, payload));
    }
}

TEST_P(CacheChangePoolTests, release_cache)
{
    CacheChange_t* ch = nullptr;

    uint32_t num_inserts = 10;
    for (uint32_t i=0; i<num_inserts; i++)
    {
        uint32_t data_size = i*16;
        pool->reserve_Cache(&ch, [data_size]() -> uint32_t {return data_size;});
        pool->release_Cache(ch);

        switch (memory_policy)
        {
            case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
               //EXPECT_EQ(ch->serializedPayload.max_size, payload);
               break;
            case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
               //EXPECT_EQ(ch->serializedPayload.max_size, max(payload, data_size));
               break;
            case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
               //EXPECT_EQ(&ch, nullptr);
               break;
        }
    }
}

INSTANTIATE_TEST_CASE_P(
    instance_1,
    CacheChangePoolTests,
    Combine(Values(0, 10, 20, 30),
            Values(0, 10, 20, 30),
            Values(128, 256, 512, 1024),
            Values(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE,
                   MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
                   MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)));

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
