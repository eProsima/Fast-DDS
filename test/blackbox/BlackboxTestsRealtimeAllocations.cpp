// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

// This tests only make sense when not allocating data on the history pool
#ifndef DYNAMIC_RESERVE_MEMORY_MODE_TEST

BLACKBOXTEST(BlackBox, PubSubReliableWithLimitedSubscribers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .matched_readers_allocation(1u, 1u)
        .expect_no_allocs()
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, AsyncPubSubReliableWithLimitedSubscribers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .matched_readers_allocation(1u, 1u)
        .expect_no_allocs()
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, PubSubBestEffortWithLimitedSubscribers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
        .matched_readers_allocation(1u, 1u)
        .expect_no_allocs()
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, AsyncPubSubBestEffortWithLimitedSubscribers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
        .matched_readers_allocation(1u, 1u)
        .expect_no_allocs()
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, PubSubReliableWithLimitedPublishers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer2(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .matched_writers_allocation(1u, 1u)
        .expect_no_allocs()
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, AsyncPubSubReliableWithLimitedPublishers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer2(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .matched_writers_allocation(1u, 1u)
        .expect_no_allocs()
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, PubSubBestEffortWithLimitedPublishers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer2(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .matched_writers_allocation(1u, 1u)
        .expect_no_allocs()
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
        .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

BLACKBOXTEST(BlackBox, AsyncPubSubBestEffortWithLimitedPublishers)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer2(TEST_TOPIC_NAME);

    reader
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .matched_writers_allocation(1u, 1u)
        .expect_no_allocs()
        .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
        .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
        .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .history_depth(10)
        .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
        .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

#endif