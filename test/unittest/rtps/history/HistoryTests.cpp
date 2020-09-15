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

#include <fastdds/rtps/history/History.h>
#include <fastdds/rtps/common/CacheChange.h>

#include <tuple>

using namespace eprosima::fastrtps::rtps;
using namespace ::testing;
using namespace std;

class HistoryTests : public TestWithParam<tuple<int32_t, int32_t, uint32_t, MemoryManagementPolicy_t> >
{
protected:

    class HistoryForTest : public History
    {
    public:

        explicit HistoryForTest(
                const HistoryAttributes& att)
            : History(att)
        {
            mp_mutex = &mutex_;
        }

        virtual ~HistoryForTest()
        {
        }

        bool remove_change(
                CacheChange_t* ch) override
        {
            if (ch == nullptr)
            {
                return false;
            }

            std::lock_guard<eprosima::fastrtps::RecursiveTimedMutex> guard(*mp_mutex);
            auto it = std::remove(m_changes.begin(), m_changes.end(), ch);
            if (it == m_changes.end())
            {
                return false;
            }

            m_changes.erase(it, m_changes.end());
            return true;
        }

    private:

        eprosima::fastrtps::RecursiveTimedMutex mutex_;
    };

    HistoryTests()
        : pool(nullptr)
        , pool_size(10)
        , max_pool_size(0)
        , payload_size(128)
        , memory_policy(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
    {
    }

    virtual ~HistoryTests()
    {
    }

    virtual void SetUp()
    {
        pool_size     = get<0>(GetParam());
        max_pool_size = get<1>(GetParam());
        payload_size  = get<2>(GetParam());
        memory_policy = get<3>(GetParam());

        HistoryAttributes att;
        att.initialReservedCaches = pool_size;
        att.maximumReservedCaches = max_pool_size;
        att.payloadMaxSize = payload_size;
        att.memoryPolicy = memory_policy;

        pool = new HistoryForTest(att);
    }

    virtual void TearDown()
    {
        delete pool;
    }

    HistoryForTest* pool;

    int32_t pool_size;
    int32_t max_pool_size;
    uint32_t payload_size;
    MemoryManagementPolicy_t memory_policy;
};

TEST_P(HistoryTests, reserve_cache)
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

TEST_P(HistoryTests, chage_change)
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

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z, )
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    HistoryTests,
    HistoryTests,
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
