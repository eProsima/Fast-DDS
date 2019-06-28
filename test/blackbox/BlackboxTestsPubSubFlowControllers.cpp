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

TEST(BlackBox, AsyncPubSubAsReliableData64kbWithParticipantFlowControl)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 68000;
    uint32_t periodInMs = 500;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    writer.history_depth(3).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(BlackBox, AsyncPubSubAsReliableData64kbWithParticipantFlowControlAndUserTransport)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 65000;
    uint32_t periodInMs = 500;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(3).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(BlackBox, AsyncPubSubWithFlowController64kb)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> slowWriter(TEST_TOPIC_NAME);

    reader.history_depth(2).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    uint32_t sizeToClear = 68000; //68kb
    uint32_t periodInMs = 1000; //1sec

    slowWriter.history_depth(2).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs).init();
    ASSERT_TRUE(slowWriter.isInitialized());

    slowWriter.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator(2);

    reader.startReception(data);
    slowWriter.send(data);
    // In 1 second only one of the messages has time to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(reader.getReceivedCount(), 1u);
}

TEST(BlackBox, FlowControllerIfNotAsync)
{
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    uint32_t size = 10000;
    uint32_t periodInMs = 1000;
    writer.add_throughput_controller_descriptor_to_pparams(size, periodInMs).init();
    ASSERT_FALSE(writer.isInitialized());
}
