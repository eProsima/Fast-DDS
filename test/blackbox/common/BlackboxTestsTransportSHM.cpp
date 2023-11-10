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

#ifndef FASTDDS_SHM_TRANSPORT_DISABLED

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include "./mock/BlackboxMockConsumer.h"

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>

#include <rtps/transport/shared_mem/test_SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

using namespace eprosima::fastrtps;

using SharedMemTransportDescriptor = eprosima::fastdds::rtps::SharedMemTransportDescriptor;
using test_SharedMemTransportDescriptor = eprosima::fastdds::rtps::test_SharedMemTransportDescriptor;

TEST(SHM, TransportPubSub)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 15;

    auto testTransport = std::make_shared<SharedMemTransportDescriptor>();

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    writer.disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader.disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::list<HelloWorld> data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

// Regression test for redmine #19500
TEST(SHM, IgnoreNonExistentSegment)
{
    using namespace eprosima::fastdds::dds;

    // Set up log
    BlackboxMockConsumer* helper_consumer = new BlackboxMockConsumer();
    Log::ClearConsumers();  // Remove default consumers
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(helper_consumer)); // Registering a consumer transfer ownership
    // Filter specific message
    Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);
    Log::SetCategoryFilter(std::regex("RTPS_TRANSPORT_SHM"));
    Log::SetErrorStringFilter(std::regex("Error receiving data.*"));

    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    writer
            .asynchronously(eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(std::make_shared<SharedMemTransportDescriptor>())
            .init();
    ASSERT_TRUE(writer.isInitialized());

    reader
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(std::make_shared<SharedMemTransportDescriptor>())
            .init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery();

    // Create and quickly destroy several participants in several threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10; i++)
    {
        threads.push_back(std::thread([]()
                {
                    constexpr size_t num_parts = 10;
                    for (size_t i = 0; i < num_parts; ++i)
                    {
                        PubSubWriter<Data1mbPubSubType> late_writer(TEST_TOPIC_NAME);
                        late_writer
                                .asynchronously(eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE)
                                .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                                .disable_builtin_transport()
                                .add_user_transport_to_pparams(std::make_shared<SharedMemTransportDescriptor>())
                                .init();
                        ASSERT_TRUE(late_writer.isInitialized());
                    }
                }));
    }

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();

    for (auto& thread : threads)
    {
        thread.join();
    }
    // Check logs
    Log::Flush();
    EXPECT_EQ(helper_consumer->ConsumedEntries().size(), 0);

    // Clean-up
    Log::Reset();  // This calls to ClearConsumers, which deletes the registered consumer
}

TEST(SHM, Test300KFragmentation)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    auto data = default_data300kb_data_generator(1);
    auto data_size = data.front().data().size();

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = static_cast<uint32_t>(data_size * 3 / 4);
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = shm_transport->segment_size() / 3;
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer
            .asynchronously(eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE)
            .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .init();

    reader
            .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data, 300);

    ASSERT_EQ(big_buffers_send_count, 2u);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, Test300KNoFragmentation)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    auto data = default_data300kb_data_generator(1);
    auto data_size = data.front().data().size();

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = static_cast<uint32_t>(data_size);
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer
            .asynchronously(eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE)
            .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .init();

    reader
            .reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data, 300);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    ASSERT_EQ(big_buffers_send_count, 1u);
    ASSERT_EQ(big_buffers_recv_count, 1u);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, SHM_UDP_300KFragmentation)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 1;

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = 32 * 1024; // 32K
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    writer
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .add_user_transport_to_pparams(udp_transport)
            .init();

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .add_user_transport_to_pparams(udp_transport)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(writer_samples);
    auto data_size = data.front().data().size();
    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data, 300);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    ASSERT_EQ(big_buffers_send_count, std::ceil(data_size / (float)udp_transport->maxMessageSize));
    ASSERT_EQ(big_buffers_recv_count, std::ceil(data_size / (float)udp_transport->maxMessageSize));

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, UDPvsSHM_UDP)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 1;

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = 32 * 1024; // 32K
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    writer
            .disable_builtin_transport()
            .add_user_transport_to_pparams(udp_transport)
            .init();

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .add_user_transport_to_pparams(udp_transport)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(writer_samples);
    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data, 300);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    ASSERT_EQ(big_buffers_send_count, 0u);
    ASSERT_EQ(big_buffers_recv_count, 0u);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, SHM_UDPvsUDP)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 1;

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    writer
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .add_user_transport_to_pparams(udp_transport)
            .init();

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader
            .disable_builtin_transport()
            .add_user_transport_to_pparams(udp_transport)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(writer_samples);
    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data, 300);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

// Test == operator for SHM
TEST(BlackBox, SHM_equal_operator)
{
    // SharedMemTransportDescriptor
    SharedMemTransportDescriptor shm_transport_1;
    SharedMemTransportDescriptor shm_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(shm_transport_1, shm_transport_2);

    // Modify some default values in 1
    shm_transport_1.segment_size(shm_transport_1.segment_size() * 10u); // change default value
    shm_transport_1.max_message_size(shm_transport_1.max_message_size() + 20u); // change default value
    shm_transport_1.healthy_check_timeout_ms(shm_transport_1.healthy_check_timeout_ms() - 30u); // change default value
    shm_transport_1.rtps_dump_file("test"); // change default value

    ASSERT_FALSE(shm_transport_1 == shm_transport_2); // operator== != operator!=, using operator== == false instead

    // Modify default values in 2
    shm_transport_2.segment_size(shm_transport_2.segment_size() * 10u); // change default value
    shm_transport_2.max_message_size(shm_transport_2.max_message_size() + 20u); // change default value
    shm_transport_2.healthy_check_timeout_ms(shm_transport_2.healthy_check_timeout_ms() - 30u); // change default value
    shm_transport_2.rtps_dump_file("test"); // change default value

    ASSERT_EQ(shm_transport_1, shm_transport_2);
}

// Test copy constructor and copy assignment
TEST(SHM, SHM_copy)
{
    SharedMemTransportDescriptor shm_transport;
    shm_transport.segment_size(shm_transport.segment_size() * 10u); // change default value
    shm_transport.max_message_size(shm_transport.max_message_size() + 20u); // change default value
    shm_transport.healthy_check_timeout_ms(shm_transport.healthy_check_timeout_ms() - 30u); // change default value
    shm_transport.rtps_dump_file("test"); // change default value

    // Copy constructor
    SharedMemTransportDescriptor shm_transport_copy_constructor(shm_transport);
    EXPECT_EQ(shm_transport_copy_constructor, shm_transport);

    // Copy assignment
    SharedMemTransportDescriptor shm_transport_copy = shm_transport;
    EXPECT_EQ(shm_transport_copy, shm_transport);
}

#endif // EPROSIMA_SHM_TRANSPORT_DISABLED


