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

class CacheChangePoolTests : public TestWithParam<tuple<int32_t, int32_t, uint32_t, MemoryManagementPolicy_t> >
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

    virtual ~CacheChangePoolTests()
    {
    }

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

#if 0
TEST_P(CacheChangePoolTests, init_pool)
{
    ASSERT_EQ(pool->getInitialPayloadSize(), payload_size);

    size_t expected_size;
    if (memory_policy == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE ||
            memory_policy == MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE)
    {
        expected_size = 0;
    }
    else
    {
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

    for (uint32_t i = 0; i < num_inserts; i++)
    {
        uint32_t data_size = i * 16;
        ASSERT_TRUE(pool->reserve_Cache(&ch, [data_size]() -> uint32_t
                {
                    return data_size;
                }));
        ASSERT_EQ(pool->getInitialPayloadSize(), payload);
        ASSERT_GE(pool->get_freeCachesSize(), 0U);

        switch (memory_policy)
        {
            case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, payload);
                break;
            case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, max(payload, data_size));
                break;
            case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, data_size);
                break;
            case MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE:
                ASSERT_EQ(ch->serializedPayload.max_size, data_size);
                break;
        }

        if (max_size > 0)
        {
            ASSERT_LE(pool->get_allCachesSize(), max_size + 1U);
        }
    }

    if (max_size == 0)
    {
        ASSERT_TRUE(pool->reserve_Cache(&ch, payload));
    }
    else
    {
        ASSERT_FALSE(pool->reserve_Cache(&ch, payload));
    }
}

TEST_P(CacheChangePoolTests, release_cache)
{
    CacheChange_t* ch = nullptr;

    uint32_t num_inserts = 10;
    for (uint32_t i = 0; i < num_inserts; i++)
    {
        size_t all_caches_size = pool->get_allCachesSize();
        size_t free_caches_size = pool->get_freeCachesSize();
        uint32_t data_size = i * 16;

        pool->reserve_Cache(&ch, [data_size]() -> uint32_t
                {
                    return data_size;
                });

        if (memory_policy == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE ||
                memory_policy == MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE)
        {
            ASSERT_EQ(pool->get_allCachesSize(), 1U);
            ASSERT_EQ(pool->get_freeCachesSize(), 0U);
        }
        else
        {
            ASSERT_EQ(pool->get_allCachesSize(), all_caches_size);
            ASSERT_EQ(pool->get_freeCachesSize(), free_caches_size - 1U);
        }

        pool->release_Cache(ch);

        if (memory_policy == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
        {
            ASSERT_EQ(pool->get_allCachesSize(), 0U);
            ASSERT_EQ(pool->get_freeCachesSize(), 0U);
        }
        else if (memory_policy == MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE)
        {
            ASSERT_EQ(pool->get_allCachesSize(), 1U);
            ASSERT_EQ(pool->get_freeCachesSize(), 1U);
        }
        else
        {
            ASSERT_EQ(pool->get_allCachesSize(), all_caches_size);
            ASSERT_EQ(pool->get_freeCachesSize(), free_caches_size);
        }
    }
}

TEST_P(CacheChangePoolTests, chage_change)
{
    CacheChange_t* ch = nullptr;

    uint32_t data_size = 16;

    pool->reserve_Cache(&ch, [data_size]() -> uint32_t
            {
                return data_size;
            });

    // Check that cache change is initilized
    ASSERT_EQ(ch->kind, ALIVE);
    ASSERT_EQ(ch->sequenceNumber.high, 0);
    ASSERT_EQ(ch->sequenceNumber.low, 0U);
    ASSERT_EQ(ch->writerGUID, c_Guid_Unknown);
    ASSERT_EQ(ch->serializedPayload.length, 0U);
    ASSERT_EQ(ch->serializedPayload.pos, 0U);
    for (uint8_t i = 0; i < 16; ++i)
    {
        ASSERT_EQ(ch->instanceHandle.value[i], 0U);
    }

    ASSERT_FALSE(ch->isRead);
    ASSERT_EQ(ch->sourceTimestamp.seconds(), 0);
    ASSERT_EQ(ch->sourceTimestamp.fraction(), 0U);

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
    ch->isRead = true;
    ch->sourceTimestamp.seconds(1);
    ch->sourceTimestamp.fraction(1);

    pool->release_Cache(ch);

    if (memory_policy != MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
    {
        // Check that cache change is initialized again
        ASSERT_EQ(ch->kind, ALIVE);
        ASSERT_EQ(ch->sequenceNumber.high, 0);
        ASSERT_EQ(ch->sequenceNumber.low, 0U);
        ASSERT_EQ(ch->writerGUID, c_Guid_Unknown);
        ASSERT_EQ(ch->serializedPayload.length, 0U);
        ASSERT_EQ(ch->serializedPayload.pos, 0U);
        for (uint8_t i = 0; i < 16; ++i)
        {
            ASSERT_EQ(ch->instanceHandle.value[i], 0U);
        }

        ASSERT_FALSE(ch->isRead);
        ASSERT_EQ(ch->sourceTimestamp.seconds(), 0);
        ASSERT_EQ(ch->sourceTimestamp.fraction(), 0U);
    }
}

#endif // 0

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z, )
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    CacheChangePoolTests,
    CacheChangePoolTests,
    Combine(Values(0, 10, 20, 30),
    Values(0, 10, 20, 30),
    Values(128, 256, 512, 1024),
    Values(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE,
    MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
    MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE,
    MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE))
    );

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
