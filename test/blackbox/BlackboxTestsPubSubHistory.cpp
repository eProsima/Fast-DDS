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

// Test created to check bug #1568 (Github #34)
TEST(BlackBox, PubSubAsNonReliableKeepLastReaderSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    while(data.size() > 1)
    {
        auto expected_data(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        size_t current_received = reader.block_for_at_least(2);
        reader.stopReception();
        // Should be received only two samples.
        ASSERT_EQ(current_received, 2u);
        data = reader.data_not_received();
    }
}

//Test created to deal with Issue 39 on Github
TEST(BlackBox, CacheChangeReleaseTest)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    //Reader Config
    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    reader.history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS);
    reader.history_depth(1);
    reader.resource_limits_allocated_samples(1);
    reader.resource_limits_max_samples(1);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    writer.resource_limits_allocated_samples(1);
    writer.resource_limits_max_samples(1);
    writer.history_kind(KEEP_LAST_HISTORY_QOS);
    writer.history_depth(1);
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());


    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    size_t current_received = reader.block_for_all(std::chrono::seconds(3));

    ASSERT_GE(current_received, static_cast<size_t>(1));
}

// Test created to check bug #1555 (Github #31)
TEST(BlackBox, PubSubAsReliableKeepLastReaderSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    while(data.size() > 1)
    {
        auto data_backup(data);
        decltype(data) expected_data;
        expected_data.push_back(data_backup.back()); data_backup.pop_back();
        expected_data.push_back(data_backup.back()); data_backup.pop_back();

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        writer.waitForAllAcked(std::chrono::seconds(300));
        // Should be received only two samples.
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        reader.block_for_all();
        reader.stopReception();

        data = data_backup;
    }
}

// Test created to check bug #1738 (Github #54)
TEST(BlackBox, PubSubAsReliableKeepLastWriterSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    reader.reliability(RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.
        history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

// Test created to check bug #1558 (Github #33)
TEST(BlackBox, PubSubKeepAll)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        max_blocking_time({0, 0}).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    while(!data.empty())
    {
        auto expected_data(data);

        // Send data
        writer.send(data);

        for(auto& value : data)
            expected_data.remove(value);

        // In this test the history has 20 max_samples.
        ASSERT_LE(expected_data.size(), 2u);
        writer.waitForAllAcked(std::chrono::seconds(300));
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        reader.block_for_all();
        reader.stopReception();
    }
}

// Test created to check bug #1558 (Github #33)
TEST(BlackBox, PubSubKeepAllTransient)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
        max_blocking_time({0, 0}).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    while(!data.empty())
    {
        auto expected_data(data);

        // Send data
        writer.send(data);

        for(auto& value : data)
            expected_data.remove(value);

        // In this test the history has 20 max_samples.
        ASSERT_LE(expected_data.size(), 2u);
        writer.waitForAllAcked(std::chrono::seconds(300));
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        reader.block_for_all();
        reader.stopReception();
    }
}

TEST(BlackBox, PubReliableKeepAllSubNonReliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(1).
        resource_limits_max_samples(1).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

//Verify that Cachechanges are removed from History when the a Writer unmatches
TEST(BlackBox, StatefulReaderCacheChangeRelease){
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(2).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());
    writer.history_depth(2).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(2);
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    writer.waitForAllAcked(std::chrono::seconds(300));
    writer.destroy();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    reader.startReception(expected_data);

    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

template<typename T>
void send_async_data(PubSubWriter<T>& writer, std::list<typename T::type> data_to_send)
{
    // Send data
    writer.send(data_to_send);
    // In this test all data should be sent.
    ASSERT_TRUE(data_to_send.empty());
}

TEST(BlackBox, PubSubAsReliableMultithreadKeepLast1)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(1).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(1).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(300);

    reader.startReception(data);

    std::thread thr1(&send_async_data<HelloWorldType>, std::ref(writer),
            std::list<HelloWorld>(data.begin(), std::next(data.begin(), 100)));
    std::thread thr2(&send_async_data<HelloWorldType>, std::ref(writer),
            std::list<HelloWorld>(std::next(data.begin(), 100), std::next(data.begin(), 200)));
    std::thread thr3(&send_async_data<HelloWorldType>, std::ref(writer),
            std::list<HelloWorld>(std::next(data.begin(), 200), data.end()));

    thr1.join();
    thr2.join();
    thr3.join();

    // Block reader until reception finished or timeout.
    reader.block_for_at_least(105);
}

