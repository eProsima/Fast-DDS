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

#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include "./mock/BlackboxMockConsumer.h"

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/shared_mem/test_SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

using namespace eprosima::fastdds;

TEST(SHM, TransportPubSub)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 15;

    auto testTransport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
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

/* Regression test for redmine issue #20701
 *
 * This test checks that the SHM transport will not listen on the same port
 * in unicast and multicast at the same time.
 * It does so by specifying custom default locators on a DataReader and then
 * checking that the port mutation took place, thus producing a different port.
 */
TEST(SHM, SamePortUnicastMulticast)
{
    PubSubReader<HelloWorldPubSubType> participant(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::Locator locator;
    locator.kind = LOCATOR_KIND_SHM;
    locator.port = global_port;

    eprosima::fastdds::rtps::LocatorList unicast_list;
    eprosima::fastdds::rtps::LocatorList multicast_list;

    // Note: this is using knowledge of the SHM locator address format since
    // SHMLocator is not exposed to the user.
    locator.address[0] = 'U';
    unicast_list.push_back(locator);

    // Note: this is using knowledge of the SHM locator address format since
    // SHMLocator is not exposed to the user.
    locator.address[0] = 'M';
    multicast_list.push_back(locator);

    // Create the reader with the custom transport and locators
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
    participant
            .disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .set_default_unicast_locators(unicast_list)
            .set_default_multicast_locators(multicast_list)
            .init();

    ASSERT_TRUE(participant.isInitialized());

    // Retrieve the listening locators and check that one port is different
    eprosima::fastdds::rtps::LocatorList reader_locators;
    participant.get_native_reader().get_listening_locators(reader_locators);

    ASSERT_LE(reader_locators.size(), 2u);
    auto it = reader_locators.begin();
    uint32_t first_port = it->port;
    uint32_t second_port = 0;
    if (reader_locators.size() == 2)
    {
        ++it;
        second_port = it->port;
    }
    EXPECT_NE(first_port, second_port);
    EXPECT_TRUE(first_port == global_port || second_port == global_port);
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
            .asynchronously(eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>())
            .init();
    ASSERT_TRUE(writer.isInitialized());

    reader
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>())
            .init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery();

    // Create and quickly destroy several participants in several threads
#ifdef _WIN32
    constexpr size_t num_threads = 1;
#else
    constexpr size_t num_threads = 10;
#endif  // _WIN32
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; i++)
    {
        threads.push_back(std::thread([]()
                {
#ifdef _WIN32
                    constexpr size_t num_parts = 2;
#else
                    constexpr size_t num_parts = 10;
#endif  // _WIN32
                    for (size_t i = 0; i < num_parts; ++i)
                    {
                        PubSubWriter<Data1mbPubSubType> late_writer(TEST_TOPIC_NAME);
                        late_writer
                                .asynchronously(eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE)
                                .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                                .disable_builtin_transport()
                                .add_user_transport_to_pparams(std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>())
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
    EXPECT_EQ(helper_consumer->ConsumedEntries().size(), 0u);

    // Clean-up
    Log::Reset();  // This calls to ClearConsumers, which deletes the registered consumer
}

TEST(SHM, Test300KFragmentation)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    auto data = default_data300kb_data_generator(1);
    auto data_size = data.front().data().size();

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = static_cast<uint32_t>(data_size * 3 / 4);
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = shm_transport->segment_size() / 3;
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer
            .asynchronously(eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE)
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .init();

    reader
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
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

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = static_cast<uint32_t>(data_size);
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer
            .asynchronously(eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE)
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .init();

    reader
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
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

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = 32 * 1024; // 32K
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer.asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .add_user_transport_to_pparams(udp_transport)
            .init();

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
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

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    uint32_t big_buffers_send_count = 0;
    uint32_t big_buffers_recv_count = 0;
    shm_transport->big_buffer_size_ = 32 * 1024; // 32K
    shm_transport->big_buffer_size_send_count_ = &big_buffers_send_count;
    shm_transport->big_buffer_size_recv_count_ = &big_buffers_recv_count;

    writer.asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer
            .disable_builtin_transport()
            .add_user_transport_to_pparams(udp_transport)
            .init();

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
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

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    writer.asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer
            .disable_builtin_transport()
            .add_user_transport_to_pparams(shm_transport)
            .add_user_transport_to_pparams(udp_transport)
            .init();

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
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
    eprosima::fastdds::rtps::SharedMemTransportDescriptor shm_transport_1;
    eprosima::fastdds::rtps::SharedMemTransportDescriptor shm_transport_2;

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
    eprosima::fastdds::rtps::SharedMemTransportDescriptor shm_transport;
    shm_transport.segment_size(shm_transport.segment_size() * 10u); // change default value
    shm_transport.max_message_size(shm_transport.max_message_size() + 20u); // change default value
    shm_transport.healthy_check_timeout_ms(shm_transport.healthy_check_timeout_ms() - 30u); // change default value
    shm_transport.rtps_dump_file("test"); // change default value

    // Copy constructor
    eprosima::fastdds::rtps::SharedMemTransportDescriptor shm_transport_copy_constructor(shm_transport);
    EXPECT_EQ(shm_transport_copy_constructor, shm_transport);

    // Copy assignment
    eprosima::fastdds::rtps::SharedMemTransportDescriptor shm_transport_copy = shm_transport;
    EXPECT_EQ(shm_transport_copy, shm_transport);
}

#endif // EPROSIMA_SHM_TRANSPORT_DISABLED


