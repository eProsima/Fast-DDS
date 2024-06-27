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

#include <rtps/history/TopicPayloadPool.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>

#include <tuple>

using namespace eprosima::fastdds::rtps;
using namespace ::testing;
using namespace std;

constexpr uint32_t max_num_reserves = 1000u;

class TopicPayloadPoolTests : public TestWithParam<tuple<uint32_t, uint32_t, uint32_t, MemoryManagementPolicy_t>>
{
protected:

    TopicPayloadPoolTests()
        : pool(nullptr)
        , test_input_pool_size(10)
        , test_input_max_pool_size(0)
        , payload_size(128)
        , memory_policy(MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
        , expected_pool_size_for_writers(0)
        , expected_pool_size_for_readers(0)
        , expected_finite_max_pool_size(0)
        , num_of_infinite_histories(0)
    {
    }

    virtual ~TopicPayloadPoolTests()
    {
    }

    virtual void SetUp()
    {
        test_input_pool_size     = get<0>(GetParam());
        test_input_max_pool_size = get<1>(GetParam());
        payload_size             = get<2>(GetParam());
        memory_policy            = get<3>(GetParam());

        std::cout << "[init:" << test_input_pool_size
                  << " max:" << test_input_max_pool_size
                  << " size:" << payload_size
                  << " policy:" << memory_policy
                  << "]" << std::endl;

        PoolConfig config{ memory_policy, payload_size, test_input_pool_size, test_input_max_pool_size };
        pool = TopicPayloadPool::get(config);
    }

    virtual void TearDown()
    {
        pool.reset();
    }

    /**
     * Checks the sizes of the pool when no payload has been ever reserved.
     * In order to perform a check on the maximum size of the pool, this method
     * reserves all the possible payloads, thus affecting to the final size of the pool.
     * So to this, calling this method twice may fail, depending on the configuration.
     */
    void check_initial_sizes()
    {
        // Compute the expected sizes
        uint32_t expected_max_pool_size =
                num_of_infinite_histories == 0 ? expected_finite_max_pool_size : 0;

        uint32_t expected_pool_size = 0;
        switch (memory_policy)
        {
            case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
            case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                expected_pool_size = expected_pool_size_for_writers + expected_pool_size_for_readers;
                break;
            case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
            case MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE:
                expected_pool_size = 0;
                break;
        }


        check_sizes(expected_pool_size, expected_max_pool_size);
    }

    /**
     * Checks the sizes of the pool after the maximum size of the pool has been
     * reached and all payloads have been released later.
     * Calling this method twice should not fail.
     */
    void check_final_sizes()
    {
        // Compute the expected sizes
        uint32_t expected_max_pool_size =
                num_of_infinite_histories == 0 ? expected_finite_max_pool_size : 0;

        uint32_t expected_pool_size = 0;
        switch (memory_policy)
        {
            // These policies do not free released payloads
            case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
            case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            case MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE:
                expected_pool_size = expected_max_pool_size;
                if (expected_max_pool_size == 0)
                {
                    expected_pool_size = max_num_reserves + 1;
                }

                break;
            // This policy frees released payloads
            case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
                expected_pool_size = 0;
                break;
        }

        check_sizes(expected_pool_size, expected_max_pool_size);
    }

    void check_sizes(
            uint32_t expected_pool_size,
            uint32_t expected_max_pool_size)
    {
        // Check the reserved sizes
        ASSERT_EQ(pool->payload_pool_allocated_size(), expected_pool_size);

        // Check the maximum sizes
        // As there is no public interface exposing this data,
        //we reserve caches until the pool reaches its maximum
        std::vector<CacheChange_t*> cache_changes;
        uint32_t num_reserves = expected_max_pool_size;
        if (expected_max_pool_size == 0)
        {
            num_reserves = max_num_reserves;
        }

        //Reserve to the expected maximum
        for (uint32_t i = 0; i < num_reserves; i++)
        {
            uint32_t data_size = i * 16 + 1u;
            CacheChange_t* ch = new CacheChange_t();
            cache_changes.push_back(ch);

            ASSERT_TRUE(pool->get_payload(data_size, ch->serializedPayload));
            ASSERT_NE(ch->serializedPayload.data, nullptr);

            switch (memory_policy)
            {
                case MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE:
                    ASSERT_EQ(ch->serializedPayload.max_size, payload_size);
                    break;
                case MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                    ASSERT_GE(ch->serializedPayload.max_size, max(payload_size, data_size));
                    break;
                case MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE:
                    ASSERT_EQ(ch->serializedPayload.max_size, data_size);
                    break;
                case MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE:
                    ASSERT_GE(ch->serializedPayload.max_size, data_size);
                    break;
            }
        }

        //Try to reserve one more
        if (expected_max_pool_size == 0)
        {
            CacheChange_t* ch = new CacheChange_t();
            cache_changes.push_back(ch);

            ASSERT_TRUE(pool->get_payload(payload_size, ch->serializedPayload));
        }
        else
        {
            CacheChange_t* ch = new CacheChange_t();

            ASSERT_FALSE(pool->get_payload(payload_size, ch->serializedPayload));
            delete ch;
        }


        // Get the same payloads for another cache change
        for (uint32_t i = 0; i < num_reserves; i++)
        {
            CacheChange_t* ch = new CacheChange_t();
            cache_changes.push_back(ch);

            ch->writerGUID = GUID_t(GuidPrefix_t(), 1);
            ch->sequenceNumber = SequenceNumber_t(0, i);
            IPayloadPool* owner = cache_changes[i]->serializedPayload.payload_owner;
            ASSERT_TRUE(pool->get_payload(cache_changes[i]->serializedPayload, ch->serializedPayload));
            ASSERT_NE(ch->serializedPayload.data, nullptr);
            ASSERT_EQ(ch->serializedPayload.data, cache_changes[i]->serializedPayload.data);
            ASSERT_EQ(ch->serializedPayload.payload_owner, owner);
        }

        for (CacheChange_t* ch : cache_changes)
        {
            ASSERT_TRUE(pool->release_payload(ch->serializedPayload));
            delete ch;
        }
        cache_changes.clear();
    }

    void do_reserve_history(
            uint32_t new_reserved_size,
            uint32_t new_reserved_max_size,
            bool is_reader)
    {
        // Do the reserve
        PoolConfig config{ memory_policy, 0, new_reserved_size, new_reserved_max_size };
        ASSERT_TRUE(pool->reserve_history(config, is_reader));

        // Update the expected pool sizes
        if (new_reserved_max_size == 0)
        {
            ++num_of_infinite_histories;
        }
        else
        {
            expected_finite_max_pool_size +=
                    (new_reserved_max_size < new_reserved_size ? new_reserved_size : new_reserved_max_size);
        }

        if (is_reader)
        {
            expected_pool_size_for_readers += new_reserved_size;
        }
        else
        {
            expected_pool_size_for_writers += new_reserved_size;
        }
    }

    void do_release_history(
            uint32_t new_released_size,
            uint32_t new_released_max_size,
            bool is_reader)
    {
        // Do the release
        PoolConfig config{ memory_policy, 0, new_released_size, new_released_max_size };
        ASSERT_TRUE(pool->release_history(config, is_reader));

        // Update the expected pool sizes
        if (new_released_max_size == 0)
        {
            --num_of_infinite_histories;
        }
        else
        {
            expected_finite_max_pool_size -=
                    (new_released_max_size < new_released_size ? new_released_size : new_released_max_size);
        }

        if (is_reader)
        {
            expected_pool_size_for_readers -= new_released_size;
        }
        else
        {
            expected_pool_size_for_writers -= new_released_size;
        }
    }

    /**
     * - Reserves a reader history with the limits configured on the fixture
     * - Reserves a writer history with the limits configured on the fixture
     * - Reserves the testing history with the configuration provided on the arguments
     * - Checks that the pool size and limits are correct
     * - Releases the testing history
     * - Checks that the pool size and limits are correct
     */
    void do_history_test (
            uint32_t size,
            uint32_t max_size,
            bool is_reader,
            uint32_t num_times = 2u)
    {
        for (uint32_t i = 0; i < num_times; i++)
        {
            // First history reserved for a reader.
            do_reserve_history(test_input_pool_size, test_input_max_pool_size, true);

            // Another history reserved for a writer.
            do_reserve_history(test_input_pool_size, test_input_max_pool_size, false);

            // Another history reserved requested limits.
            do_reserve_history(size, max_size, is_reader);
            check_initial_sizes();

            // Release the last history
            do_release_history(size, max_size, is_reader);
            check_final_sizes();

            // Release the first two histories
            do_release_history(test_input_pool_size, test_input_max_pool_size, true);
            do_release_history(test_input_pool_size, test_input_max_pool_size, false);
        }

        EXPECT_EQ(pool->payload_pool_available_size(), 0u);
        EXPECT_EQ(pool->payload_pool_allocated_size(), 0u);
    }

    std::unique_ptr<ITopicPayloadPool> pool;    //< The pool under test

    uint32_t test_input_pool_size;              //< Pool size given to the parametric test
    uint32_t test_input_max_pool_size;          //< Max pool size given to the parametric test
    uint32_t payload_size;                      //< Payload size given to the parametric test
    MemoryManagementPolicy_t memory_policy;     //< Memory policy size given to the parametric test

    uint32_t expected_pool_size_for_writers;    //< Initial pool size due to writers (sum of all writers)
    uint32_t expected_pool_size_for_readers;    //< Initial pool size due to readers (max of all readers)
    uint32_t expected_finite_max_pool_size;     //< Expected max pool size without counting the infinite histories
    uint32_t num_of_infinite_histories;         //< Number of infinite histories reserved
};

void do_dynamic_topic_payload_pool_zero_size_test(
        const PoolConfig& config)
{
    //! Create Pool
    std::unique_ptr<ITopicPayloadPool> pool = TopicPayloadPool::get(config);

    //! Update maximum size of the pool
    pool->reserve_history(config, false);

    //! Temporal CacheChange with no payload owner
    CacheChange_t* change = new CacheChange_t();

    //! A CacheChange to copy to
    CacheChange_t* change_to_add = new CacheChange_t();

    //! Fill minimum fields
    change_to_add->writerGUID = GUID_t(GuidPrefix_t(), 1);
    change_to_add->sequenceNumber = SequenceNumber_t(0, 1);

    //! get the payload of size 0.
    //! Allocate it on the pool
    //! Set change_to_add owner
    ASSERT_TRUE(pool->get_payload(change->serializedPayload, change_to_add->serializedPayload));

    //! Now set the payload ownership on the source change
    pool->get_payload(change_to_add->serializedPayload, change->serializedPayload);

    //! Release the payload from the source change
    pool->release_payload(change->serializedPayload);

    //! Temporal CacheChange whose payload is owned by the pool
    CacheChange_t* another_change = new CacheChange_t();

    //! Max size was reached, should fail
    ASSERT_FALSE(pool->get_payload(0, another_change->serializedPayload));

    //! Release the second payload owner
    pool->release_payload(change_to_add->serializedPayload);

    //! Now a free cache is avaiable
    ASSERT_TRUE(pool->get_payload(0, another_change->serializedPayload));

    ASSERT_TRUE(pool->get_payload(another_change->serializedPayload, change_to_add->serializedPayload));

    //! Release
    pool->release_payload(another_change->serializedPayload);
    pool->release_payload(change_to_add->serializedPayload);

    //! Release history
    pool->release_history(config, false);

    //! Expect the available sizes after releasing
    EXPECT_EQ(pool->payload_pool_available_size(), 0u);
    EXPECT_EQ(pool->payload_pool_allocated_size(), 0u);

    delete change_to_add;
    delete another_change;
    delete change;
}

TEST_P(TopicPayloadPoolTests, reserve_history_reader_same_size)
{
    // A new history reserved for a reader. Same limits as fixture.
    uint32_t reserve_size = test_input_pool_size;
    uint32_t reserve_max_size = test_input_max_pool_size;
    do_history_test(reserve_size, reserve_max_size, true);
}

TEST_P(TopicPayloadPoolTests, reserve_history_reader_lower_size)
{
    // A new history reserved for a reader. Lower limits than fixture.
    uint32_t reserve_size = test_input_pool_size / 2;
    uint32_t reserve_max_size = test_input_max_pool_size / 5;
    do_history_test(reserve_size, reserve_max_size, true);
}

TEST_P(TopicPayloadPoolTests, reserve_history_reader_larger_size)
{
    // A new history reserved for a reader. Larger limits than fixture.
    uint32_t reserve_size = test_input_pool_size * 2;
    uint32_t reserve_max_size = test_input_max_pool_size * 5;
    do_history_test(reserve_size, reserve_max_size, true);
}

TEST_P(TopicPayloadPoolTests, reserve_history_writer_same_size)
{
    // A new history reserved for a writer. Same limits as fixture.
    uint32_t reserve_size = test_input_pool_size;
    uint32_t reserve_max_size = test_input_max_pool_size;
    do_history_test(reserve_size, reserve_max_size, false);
}

TEST_P(TopicPayloadPoolTests, reserve_history_writer_lower_size)
{
    // A new history reserved for a writer. Lower limits than fixture.
    uint32_t reserve_size = test_input_pool_size / 2;
    uint32_t reserve_max_size = test_input_max_pool_size / 5;
    do_history_test(reserve_size, reserve_max_size, false);
}

TEST_P(TopicPayloadPoolTests, reserve_history_writer_larger_size)
{
    // A new history reserved for a writer. Larger limits than fixture.
    uint32_t reserve_size = test_input_pool_size * 2;
    uint32_t reserve_max_size = test_input_max_pool_size * 5;
    do_history_test(reserve_size, reserve_max_size, false);
}

TEST_P(TopicPayloadPoolTests, release_history_reader_infinite_size)
{
    // A history with infinite size reserved for a reader is released.
    uint32_t reserve_size = test_input_pool_size;
    uint32_t reserve_max_size = 0;
    do_history_test(reserve_size, reserve_max_size, true);
}

TEST_P(TopicPayloadPoolTests, release_history_reader_finite_size)
{
    // A history with finite size reserved for a reader is released.
    uint32_t reserve_size = test_input_pool_size;
    uint32_t reserve_max_size = 100;
    do_history_test(reserve_size, reserve_max_size, true);
}

TEST_P(TopicPayloadPoolTests, release_history_writer_infinite_size)
{
    // A history with infinite size reserved for a reader is releasde.
    uint32_t reserve_size = test_input_pool_size;
    uint32_t reserve_max_size = 0;
    do_history_test(reserve_size, reserve_max_size, false);
}

TEST_P(TopicPayloadPoolTests, release_history_writer_finite_size)
{
    // A history with finite size reserved for a reader is released.
    uint32_t reserve_size = test_input_pool_size;
    uint32_t reserve_max_size = 100;
    do_history_test(reserve_size, reserve_max_size, false);
}

//! This unittest was introduced as part of #1929
//! Claiming a payload with size 0 is properly handled with DYNAMIC_RESERVED Memory Mode
TEST(TopicPayloalPoolTests, dynamic_reserve_memory_zero_size)
{
    PoolConfig config{ DYNAMIC_RESERVE_MEMORY_MODE, 128, 0, 1};
    do_dynamic_topic_payload_pool_zero_size_test(config);
}

//! This unittest was introduced as part of #1929
//! Claiming a payload with size 0 is properly handled with DYNAMIC_REUSABLE Memory Modes
TEST(TopicPayloalPoolTests, dynamic_reusable_memory_zero_size)
{
    PoolConfig config{ DYNAMIC_REUSABLE_MEMORY_MODE, 128, 0, 1};
    do_dynamic_topic_payload_pool_zero_size_test(config);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z, )
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    TopicPayloadPoolTests,
    TopicPayloadPoolTests,
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
