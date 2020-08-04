// Copyright 2019, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/log/Log.h>
#include <fastrtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TEST(BlackBox, PubSubAsNonReliableData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

TEST(PubSubFragments, PubSubAsNonReliableVolatileData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer
    .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
    .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
    .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

TEST(PubSubFragments, PubSubAsNonReliableTransientLocalData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer
    .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
    .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
    .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

TEST(BlackBox, PubSubAsReliableData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

TEST(PubSubFragments, PubSubAsReliableVolatileData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer
    .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
    .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

TEST(PubSubFragments, PubSubAsReliableTransientLocalData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer
    .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
    .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

TEST(BlackBox, AsyncPubSubAsNonReliableData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(10).
    reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
    add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST(PubSubFragments, AsyncPubSubAsNonReliableVolatileData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.
    durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
    init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(10).
    reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
    durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
    add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST(PubSubFragments, AsyncPubSubAsNonReliableTransientLocalData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
    init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(10).
    reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
    add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST(BlackBox, AsyncPubSubAsReliableData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
    add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(PubSubFragments, AsyncPubSubAsReliableVolatileData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
    durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
    durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
    add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(PubSubFragments, AsyncPubSubAsReliableTransientLocalData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
    add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(BlackBox, AsyncPubSubAsReliableData300kbInLossyConditions)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        eprosima::fastrtps::rtps::test_UDPv4Transport::test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST(PubSubFragments, AsyncPubSubAsReliableVolatileData300kbInLossyConditions)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
    durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        eprosima::fastrtps::rtps::test_UDPv4Transport::test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST(PubSubFragments, AsyncPubSubAsReliableVolatileData300kbInLossyConditionsSmallFragments)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 1024;
    testTransport->maxMessageSize = 1024;
    testTransport->receiveBufferSize = 65536;
    // We are sending around 300 fragments per sample.
    // We drop 1% of all data frags
    testTransport->dropDataFragMessagesPercentage = 1;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
    durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
    asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(
        eprosima::fastrtps::rtps::test_UDPv4Transport::test_UDPv4Transport_DropLog.size(),
        testTransport->dropLogLength);
}

TEST(BlackBox, AsyncFragmentSizeTest)
{
    // ThroghputController size large than maxMessageSize.
    {
        PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
        PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

        reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        // When doing fragmentation, it is necessary to have some degree of
        // flow control not to overrun the receive buffer.
        uint32_t size = 32536;
        uint32_t periodInMs = 500;
        writer.add_throughput_controller_descriptor_to_pparams(size, periodInMs);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 32000;
        testTransport->sendBufferSize = 65536;
        testTransport->receiveBufferSize = 65536;
        writer.disable_builtin_transport();
        writer.add_user_transport_to_pparams(testTransport);
        writer.history_depth(10).asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

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
        std::this_thread::sleep_for(std::chrono::seconds(3));
        size_t current_received = reader.getReceivedCount();
        ASSERT_GE(current_received, static_cast<size_t>(1));
        ASSERT_LE(current_received, static_cast<size_t>(3));

    }
    // ThroghputController size smaller than maxMessageSize.
    {
        PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
        PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

        reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        // When doing fragmentation, it is necessary to have some degree of
        // flow control not to overrun the receive buffer.
        uint32_t size = 32000;
        uint32_t periodInMs = 500;
        writer.add_throughput_controller_descriptor_to_pparams(size, periodInMs);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 32536;
        testTransport->sendBufferSize = 65536;
        testTransport->receiveBufferSize = 65536;
        writer.disable_builtin_transport();
        writer.add_user_transport_to_pparams(testTransport);
        writer.history_depth(10).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

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
        std::this_thread::sleep_for(std::chrono::seconds(3));
        size_t current_received = reader.getReceivedCount();
        ASSERT_GE(current_received, static_cast<size_t>(1));
        ASSERT_LE(current_received, static_cast<size_t>(3));
    }
}
