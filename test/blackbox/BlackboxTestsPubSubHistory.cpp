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

    while (data.size() > 1)
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

    while (data.size() > 1)
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

    while (!data.empty())
    {
        auto expected_data(data);

        // Send data
        writer.send(data);

        for (auto& value : data)
        {
            expected_data.remove(value);
        }

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

    while (!data.empty())
    {
        auto expected_data(data);

        // Send data
        writer.send(data);

        for (auto& value : data)
        {
            expected_data.remove(value);
        }

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
void send_async_data(
        PubSubWriter<T>& writer,
        std::list<typename T::type> data_to_send)
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

// Test created to check bug #6319 (Github #708)
TEST(BlackBox, PubSubAsReliableKeepLastReaderSmallDepthTwoPublishers)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer2(TEST_TOPIC_NAME);

    reader
    .reliability(RELIABLE_RELIABILITY_QOS)
    .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
    .history_depth(1)
    .resource_limits_allocated_samples(1)
    .resource_limits_max_samples(1);

    writer.max_blocking_time({ 120, 0 });
    writer2.max_blocking_time({ 120, 0 });

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    writer2.init();
    ASSERT_TRUE(writer2.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    writer2.wait_discovery();
    reader.wait_discovery();

    HelloWorld data;
    data.message("Hello world!");

    // First writer sends two samples (reader would only keep the last one)
    data.index(1u);
    ASSERT_TRUE(writer.send_sample(data));
    data.index(2u);
    ASSERT_TRUE(writer.send_sample(data));

    // Wait for reader to acknowledge samples
    while (!writer.waitForAllAcked(std::chrono::milliseconds(100)))
    {
    }

    // Second writer sends one sample (reader should discard previous one)
    data.index(3u);
    ASSERT_TRUE(writer2.send_sample(data));

    // Wait for reader to acknowledge sample
    while (!writer2.waitForAllAcked(std::chrono::milliseconds(100)))
    {
    }

    // Only last sample should be present
    HelloWorld received;
    ASSERT_TRUE(reader.takeNextData(&received, nullptr));
    ASSERT_EQ(received.index(), 3u);
}

bool comparator(
        HelloWorld first,
        HelloWorld second)
{
    return first.index() < second.index();
}

/*!
 * @fn TEST(PubSubHistory, ReliableTransientLocalKeepLast1)
 * @brief This test checks the correct functionality of transient local, keep last 1 late-joiners.
 *
 * The test creates a Reliable, Transient Local Writer with a Keep Last 10 history, and send ten samples throughout the
 * network.
 * Then it creates a Reliable, Transient Local Reader with a Keep Last 1 history and waits until both of them discover
 * each other.
 * When the Writer discovers the late-joiner, resends the samples it has on the history (in this case all). And the
 * reader upon receiving them, as it is Keep Last 1, discards all the samples except the last one.
 * Finally, the test checks that the last sequence number received is the one corresponding to the tenth sample.
 */
TEST(PubSubHistory, ReliableTransientLocalKeepLast1)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
    .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
    .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
    .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
    .history_depth(10)
    .resource_limits_allocated_samples(10)
    .resource_limits_max_samples(10).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();
    auto expected_data = data;
    writer.send(data);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
    .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
    .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
    .history_depth(1).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_FALSE(expected_data.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    reader.startReception(expected_data);
    eprosima::fastrtps::rtps::SequenceNumber_t seq;
    seq.low = 10;
    reader.block_for_seq(seq);

    ASSERT_EQ(reader.get_last_sequence_received().low, 10u);
}

TEST(PubSubHistory, ReliableTransientLocalKeepLast1Data300Kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    auto data = default_data300kb_data_generator();
    auto reader_data = data;

    writer
    .history_depth(static_cast<int32_t>(data.size()))
    .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
    .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
    .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
    .init();

    ASSERT_TRUE(writer.isInitialized());

    // Send data
    writer.send(data);
    ASSERT_TRUE(data.empty());
    ASSERT_FALSE(reader_data.empty());

    reader
    .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
    .history_depth(1)
    .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
    .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
    .init();

    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(reader_data);
    eprosima::fastrtps::rtps::SequenceNumber_t seq;
    seq.low = 10;
    reader.block_for_seq(seq);

    ASSERT_EQ(reader.get_last_sequence_received().low, 10u);
}

// Test created to check bug #8945
/*!
 * @fn TEST(PubSubHistory, WriterWithoutReadersTransientLocal)
 * @brief This test checks that the writer doesn't fill its history while there are no readers matched.
 *
 * The test creates a Reliable, Transient Local Writer with a Keep All history, and its resources limited to
 * 13 samples.
 * Then it creates a Reliable, Transient Local Reader with a Keep Last 1 history and waits until both of them discover
 * each other.
 * When both of them are matched, the writer sends 13 samples, asserting that all of them have been sent and the
 * reader starts its reception.
 * After that, the reader is destroyed, meaning that the writer runs out of readers. Even if there are no readers,
 * it has to continue sending the remaining samples, deleting the old ones if the history is filled up.
 */
TEST(PubSubHistory, WriterWithoutReadersTransientLocal)
{
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer
    .history_kind(KEEP_ALL_HISTORY_QOS)
    .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS)
    .asynchronously(ASYNCHRONOUS_PUBLISH_MODE)
    .reliability(RELIABLE_RELIABILITY_QOS)
    .resource_limits_allocated_samples(13)
    .resource_limits_max_samples(13)
    .init();

    ASSERT_TRUE(writer.isInitialized());

    // Remove the reader
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    reader
    .reliability(RELIABLE_RELIABILITY_QOS)
    .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS)
    .init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data1 = default_data300kb_data_generator(13);
    auto data2 = default_data300kb_data_generator(14);

    auto unreceived_data = default_data300kb_data_generator(27);

    // Send data
    writer.send(data1);

    // In this test all data should be sent.
    ASSERT_TRUE(data1.empty());

    reader.startReception(unreceived_data);

    reader.destroy();

    // Send data
    writer.send(data2);
    // In this test all data should be sent.
    ASSERT_TRUE(data2.empty());

}

