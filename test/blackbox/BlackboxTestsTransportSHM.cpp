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

#include <gtest/gtest.h>

#include <fastrtps/transport/UDPv4Transport.h>
#include "../cpp/rtps/transport/shared_mem/test_SharedMemTransportDescriptor.h"

using namespace eprosima::fastrtps;

using SharedMemTransportDescriptor = eprosima::fastdds::rtps::SharedMemTransportDescriptor;
using test_SharedMemTransportDescriptor = eprosima::fastdds::rtps::test_SharedMemTransportDescriptor;

TEST(SHM, TransportPubSub)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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

TEST(SHM, Test300KFragmentation)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    auto data = default_data300kb_data_generator(1);
    auto data_size = data.front().data().size();

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = static_cast<uint32_t>(data_size * 3 / 4);
    shm_transport->segment_size(segment_size, segment_size);
    
    writer
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .disable_builtin_transport()
        .add_user_transport_to_pparams(shm_transport)
        .init();

    uint32_t big_buffers_count = 0;
    shm_transport->big_buffer_size_ = shm_transport->segment_size()/3;
    shm_transport->big_buffer_size_count_ = &big_buffers_count;

    reader
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
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

    ASSERT_EQ(big_buffers_count, 2u);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, Test300KNoFragmentation)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    auto data = default_data300kb_data_generator(1);
    auto data_size = data.front().data().size();
    
    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size, segment_size);
    
    writer
        .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
        .disable_builtin_transport()
        .add_user_transport_to_pparams(shm_transport)
        .init();

    uint32_t big_buffers_count = 0;
    shm_transport->big_buffer_size_ = static_cast<uint32_t>(data_size);
    shm_transport->big_buffer_size_count_ = &big_buffers_count;

    reader
        .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
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

    ASSERT_EQ(big_buffers_count, 1u);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, SHM_UDP_300KFragmentation)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 1;

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size, segment_size);

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    writer
        .disable_builtin_transport()
        .add_user_transport_to_pparams(shm_transport)
        .add_user_transport_to_pparams(udp_transport)
        .init();

    uint32_t big_buffers_count = 0;
    shm_transport->big_buffer_size_ = 32*1024; // 32K
    shm_transport->big_buffer_size_count_ = &big_buffers_count;

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

    ASSERT_EQ(big_buffers_count, std::ceil(data_size / (float)udp_transport->maxMessageSize));

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, UDPvsSHM_UDP)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 1;

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size, segment_size);
    
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE);
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    writer
        .disable_builtin_transport()
        .add_user_transport_to_pparams(udp_transport)
        .init();

    uint32_t big_buffers_count = 0;
    shm_transport->big_buffer_size_ = 32*1024; // 32K
    shm_transport->big_buffer_size_count_ = &big_buffers_count;

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

    ASSERT_EQ(big_buffers_count, 0u);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

TEST(SHM, SHM_UDPvsUDP)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 1;

    auto shm_transport = std::make_shared<test_SharedMemTransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size, segment_size);
    
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

#endif // EPROSIMA_SHM_TRANSPORT_DISABLED


