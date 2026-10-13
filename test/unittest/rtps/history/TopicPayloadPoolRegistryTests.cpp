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

#include <rtps/history/TopicPayloadPoolRegistry.hpp>

#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolProxy.hpp>
#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolRegistryEntry.hpp>
#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolRegistry.hpp>

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

TEST(TopicPayloadPoolRegistryTests, entries_released_with_last_pool)
{
    const auto& registry = detail::TopicPayloadPoolRegistry::instance();
    const size_t initial_size = registry->size();

    PoolConfig cfg{ PREALLOCATED_MEMORY_MODE, 4u, 4u, 4u };
    auto pool_preallocated = TopicPayloadPoolRegistry::get("topic_released", cfg);
    cfg.memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;
    auto pool_dynamic = TopicPayloadPoolRegistry::get("topic_released", cfg);
    EXPECT_EQ(registry->size(), initial_size + 1u);

    // The entry is kept while any pool of the topic is in use
    pool_preallocated.reset();
    EXPECT_EQ(registry->size(), initial_size + 1u);

    // and removed when the last one is released
    pool_dynamic.reset();
    EXPECT_EQ(registry->size(), initial_size);

    // Topics used once do not accumulate entries
    for (int i = 0; i < 100; ++i)
    {
        auto pool = TopicPayloadPoolRegistry::get("topic_" + std::to_string(i), cfg);
        EXPECT_TRUE(pool);
    }
    EXPECT_EQ(registry->size(), initial_size);
}
