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

#include <atomic>
#include <thread>
#include <vector>

#include <rtps/history/TopicPayloadPoolRegistry.hpp>

#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolProxy.hpp>

using namespace eprosima::fastdds::rtps;
using namespace ::testing;
using namespace std;

TEST(TopicPayloadPoolRegistryTests, basic_checks)
{
    PoolConfig cfg{ PREALLOCATED_MEMORY_MODE, 4u, 4u, 4u };

    // Same topic, same config should result on same pool
    auto pool_a1 = TopicPayloadPoolRegistry::get("topic_a", cfg);
    auto pool_a2 = TopicPayloadPoolRegistry::get("topic_a", cfg);
    EXPECT_EQ(pool_a1, pool_a2);

    // Same topic, same config should result on same pool
    auto pool_b1 = TopicPayloadPoolRegistry::get("topic_b", cfg);
    auto pool_b2 = TopicPayloadPoolRegistry::get("topic_b", cfg);
    EXPECT_EQ(pool_b1, pool_b2);

    // Different topics should be different pools
    EXPECT_NE(pool_a1, pool_b1);

    cfg.memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;

    // Same topic, different policy should result on different pool.
    auto pool_a3 = TopicPayloadPoolRegistry::get("topic_a", cfg);
    EXPECT_NE(pool_a1, pool_a3);
    // And be different from the other topic.
    EXPECT_NE(pool_b1, pool_a3);

    // Releasing all references to a topic pool should automatically release the entry
    std::weak_ptr<ITopicPayloadPool> pool_wa = pool_a1;
    pool_a1.reset();
    pool_a2.reset();
    pool_a3.reset();
    EXPECT_TRUE(pool_wa.expired());

    // Destructor should have been called a certain number of times
    EXPECT_EQ(detail::TopicPayloadPoolProxy::DestructorHelper::instance().get(), 2u);
}

// get() used to answer an empty pool when the last reference to the proxy of the topic
// was released on another thread between expired() and lock().
TEST(TopicPayloadPoolRegistryTests, get_while_another_thread_releases_the_pool)
{
    static constexpr uint32_t num_threads = 4u;
    static constexpr uint32_t gets_per_thread = 5000u;

    PoolConfig cfg{ PREALLOCATED_MEMORY_MODE, 4u, 4u, 4u };

    // Each thread drops its reference on every iteration, so the proxy is created and
    // destroyed continuously and the race window is walked often enough to be seen.
    std::vector<std::thread> threads;
    std::atomic<uint32_t> empty_results(0u);

    for (uint32_t i = 0; i < num_threads; i++)
    {
        threads.emplace_back([&]
                {
                    for (uint32_t j = 0; j < gets_per_thread; j++)
                    {
                        if (!TopicPayloadPoolRegistry::get("race_topic", cfg))
                        {
                            empty_results.fetch_add(1);
                        }
                    }
                });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(empty_results.load(), 0u);
}
