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

using namespace eprosima::fastrtps::rtps;
using namespace ::testing;
using namespace std;

class CacheChangePoolAttributes
{
public:
    CacheChangePoolAttributes()
        : pool_size(10)
        , max_pool_size(0)
        , payload_size(128)
        , memory_policy(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
    {
    }

    CacheChangePoolAttributes(
            int32_t pool_size_,
            int32_t max_pool_size_,
            uint32_t payload_size_,
            MemoryManagementPolicy_t memory_policy_)
        : pool_size(pool_size_)
        , max_pool_size(max_pool_size_)
        , payload_size(payload_size_)
        , memory_policy(memory_policy_)
    {
    }

    int32_t pool_size;
    int32_t max_pool_size;
    uint32_t payload_size;
    MemoryManagementPolicy_t memory_policy;

    friend ostream& operator<<(ostream& os, const CacheChangePoolAttributes& attrs) {
        return os << attrs.to_string();
    }

private:
    string to_string() const
    {
        string memory_policy_str;
        switch (memory_policy)
        {
        case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
            memory_policy_str = "PREALLOC";
            break;
        case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            memory_policy_str = "REALLOC";
            break;
        case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
            memory_policy_str = "DYNAMIC";
            break;
        }

        std::stringstream ss;
        ss << "[pool_size: " << pool_size
        << ", max_pool_size: " << max_pool_size
        << ", payload_size: " << payload_size
        << ", memory_policy: " << memory_policy_str << "]";

        return ss.str();
    }
};

class CacheChangePoolTests : public TestWithParam<CacheChangePoolAttributes>
{
protected:
    CacheChangePoolTests()
        : pool(nullptr)
    {
    }

    virtual ~CacheChangePoolTests() {}

    virtual void SetUp()
    {
        attrs = GetParam();
        pool = new CacheChangePool(
                attrs.pool_size,
                attrs.payload_size,
                attrs.max_pool_size,
                attrs.memory_policy);
    }

    virtual void TearDown()
    {
        delete pool;
    }

    CacheChangePoolAttributes attrs;
    CacheChangePool* pool;
};

TEST_P(CacheChangePoolTests, init_pool)
{
    ASSERT_EQ(pool->getInitialPayloadSize(), attrs.payload_size);

    size_t expected_size;
    if (attrs.memory_policy == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
    {
        expected_size = 0;
    }
    else {
        expected_size = static_cast<size_t>(attrs.pool_size + 1U);
    }

    ASSERT_EQ(pool->get_allCachesSize(), expected_size);
    ASSERT_EQ(pool->get_freeCachesSize(), expected_size);
}

TEST_P(CacheChangePoolTests, reserve_cache)
{
    CacheChange_t* ch = nullptr;

    uint32_t payload = attrs.payload_size;
    uint32_t pool_size = static_cast<uint32_t>(attrs.pool_size);
    uint32_t max_pool_size = static_cast<uint32_t>(attrs.max_pool_size);

    uint32_t num_inserts;
    if (attrs.max_pool_size == 0)
    {
        num_inserts = 1000U;
    }
    else if (attrs.max_pool_size <= attrs.pool_size)
    {
        max_pool_size = pool_size;
        num_inserts = pool_size + 1;
    }
    else // max_pool_size > pool_size
    {
        num_inserts = max_pool_size + 1;
    }

    for (uint32_t i=0; i<num_inserts; i++)
    {
        EXPECT_TRUE(pool->reserve_Cache(&ch, payload));
        EXPECT_EQ(pool->getInitialPayloadSize(), payload);
        EXPECT_GE(pool->get_freeCachesSize(), 0);

        if(max_pool_size > 0)
        {
            EXPECT_LE(pool->get_allCachesSize(), max_pool_size + 1U);
        }
    }

    if (max_pool_size == 0)
    {
        EXPECT_TRUE(pool->reserve_Cache(&ch, payload));
    }
    else
    {
        EXPECT_FALSE(pool->reserve_Cache(&ch, payload));
    }
}

INSTANTIATE_TEST_CASE_P(
    instanciation,
    CacheChangePoolTests,
    Values( CacheChangePoolAttributes(10, 10,  128,  MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 100, 128,  MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 0,   128,  MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
           ,CacheChangePoolAttributes(20, 10,  128,  MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 10,  128,  MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 100, 128,  MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 0,   128,  MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE)
           ,CacheChangePoolAttributes(20, 10,  128,  MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 10,  128,  MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 100, 128,  MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
           ,CacheChangePoolAttributes(10, 0,   128,  MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
           ,CacheChangePoolAttributes(20, 10,  128,  MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
    )
);

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
