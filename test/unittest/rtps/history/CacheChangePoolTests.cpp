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

using namespace eprosima::fastrtps::rtps;
using namespace ::testing;
using namespace std;

class CacheChangePoolAttributes
{
public:
    CacheChangePoolAttributes()
        :   pool_size(10),
            max_pool_size(0),
            payload_size(128) {}

    CacheChangePoolAttributes(
            int32_t pool_size_,
            int32_t max_pool_size_,
            uint32_t payload_size_)
        :   pool_size(pool_size_),
            max_pool_size(max_pool_size_),
            payload_size(payload_size_) {}

    int32_t pool_size;
    int32_t max_pool_size;
    uint32_t payload_size;

    friend ostream& operator<<(ostream& os, const CacheChangePoolAttributes& attrs) {
        return os << "[pool_size: " << attrs.pool_size
                  << ", max_pool_size: " << attrs.max_pool_size
                  << ", payload_size: " << attrs.payload_size << "]" ;
    }
};

class CacheChangePoolTests : public TestWithParam<CacheChangePoolAttributes>
{
protected:
    CacheChangePoolTests(MemoryManagementPolicy_t memory_policy_)
        : pool(nullptr),
          memory_policy(memory_policy_) {}

    virtual ~CacheChangePoolTests() {}

    virtual void SetUp()
    {
        attrs = GetParam();
        pool = new CacheChangePool(
                attrs.pool_size,
                attrs.payload_size,
                attrs.max_pool_size,
                memory_policy);
    }

    virtual void TearDown()
    {
        delete pool;
    }

    CacheChangePoolAttributes attrs;
    CacheChangePool* pool;
    MemoryManagementPolicy_t memory_policy;
};

class PreallocatedTests : public CacheChangePoolTests
{
public:
    PreallocatedTests() : CacheChangePoolTests(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE) {}

    virtual ~PreallocatedTests() {}
};

class ReallocTests : public CacheChangePoolTests
{
public:
    ReallocTests() : CacheChangePoolTests(MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE) {}

    virtual ~ReallocTests() {}
};

class DynamicTests : public CacheChangePoolTests
{
public:
    DynamicTests() : CacheChangePoolTests(MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE) {}

    virtual ~DynamicTests() {}
};

TEST_P(PreallocatedTests, init_pool)
{
    ASSERT_EQ(pool->getInitialPayloadSize(), attrs.payload_size);
    ASSERT_EQ(pool->get_allCachesSize(), attrs.pool_size + 1U);
    ASSERT_EQ(pool->get_freeCachesSize(), attrs.pool_size + 1U);
}

TEST_P(ReallocTests, init_pool)
{
    ASSERT_EQ(pool->getInitialPayloadSize(), attrs.payload_size);
    ASSERT_EQ(pool->get_allCachesSize(), attrs.pool_size + 1U);
    ASSERT_EQ(pool->get_freeCachesSize(), attrs.pool_size + 1U);
}

TEST_P(DynamicTests, init_pool)
{
    ASSERT_EQ(pool->getInitialPayloadSize(), attrs.payload_size);
    ASSERT_EQ(pool->get_allCachesSize(), 0U);
    ASSERT_EQ(pool->get_freeCachesSize(), 0U);
}

INSTANTIATE_TEST_CASE_P(
    init_pool_instanciation,
    PreallocatedTests,
    Values(CacheChangePoolAttributes(10, 10, 128),
           CacheChangePoolAttributes(10, 20, 128),
           CacheChangePoolAttributes(10, 0,  128),
           CacheChangePoolAttributes(20, 10, 128),
           CacheChangePoolAttributes(20, 10, 128)));

INSTANTIATE_TEST_CASE_P(
    init_pool_instanciation,
    ReallocTests,
    Values(CacheChangePoolAttributes(10, 10, 128),
           CacheChangePoolAttributes(10, 20, 128),
           CacheChangePoolAttributes(10, 0,  128),
           CacheChangePoolAttributes(20, 10, 128),
           CacheChangePoolAttributes(20, 10, 128)));

INSTANTIATE_TEST_CASE_P(
    init_pool_instanciation,
    DynamicTests,
    Values(CacheChangePoolAttributes(10, 10, 128),
           CacheChangePoolAttributes(10, 20, 128),
           CacheChangePoolAttributes(10, 0,  128),
           CacheChangePoolAttributes(20, 10, 128),
           CacheChangePoolAttributes(20, 10, 128)));

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
