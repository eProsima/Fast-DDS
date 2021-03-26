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
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TEST(BlackBox, PubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

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

TEST(BlackBox, AsyncPubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

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

TEST(BlackBox, PubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).init();

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
    reader.block_for_all();
}

TEST(BlackBox, AsyncPubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

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
    reader.block_for_all();
}

TEST(BlackBox, ReqRepAsReliableHelloworld)
{
    ReqRepAsReliableHelloWorldRequester requester;
    ReqRepAsReliableHelloWorldReplier replier;
    const uint16_t nmsgs = 10;

    requester.init();

    ASSERT_TRUE(requester.isInitialized());

    replier.init();

    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(replier.isInitialized());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block(std::chrono::seconds(5));
    }
}

TEST(BlackBox, PubSubAsReliableData64kb)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(10).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(BlackBox, PubSubMoreThan256Unacknowledged)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator(600);
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.startReception(expected_data);
    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(10)));
}

TEST(BlackBox, PubSubAsReliableHelloworldMulticastDisabled)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        disable_multicast(0).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        disable_multicast(1).init();

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
    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(10)));
}

