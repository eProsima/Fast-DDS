// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#include <fastrtps/utils/TimeConversion.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include <rtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;
using test_UDPv4TransportDescriptor = eprosima::fastdds::rtps::test_UDPv4TransportDescriptor;

void reliability_disable_heartbeat_piggyback(
        bool disable_heartbeat_piggyback)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    bool heartbeat_found = false;
    bool start_reception = false;
    EntityId_t writer_id;

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->drop_heartbeat_messages_filter_ = [&heartbeat_found, &writer_id, &start_reception](CDRMessage_t& msg)
            -> bool
            {
                if (start_reception)
                {
                    auto old_pos = msg.pos;
                    EntityId_t writer_id_msg;
                    msg.pos += 4;
                    CDRMessage::readEntityId(&msg, &writer_id_msg);
                    if (writer_id == writer_id_msg)
                    {
                        heartbeat_found = true;
                    }
                    msg.pos = old_pos;
                }
                return false;
            };

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .heartbeat_period_seconds(180000)
            .disable_heartbeat_piggyback(disable_heartbeat_piggyback)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .init();
    writer_id = writer.datawriter_guid().entityId;

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto data = default_keyedhelloworld_data_generator(10, true);
    reader.startReception(data);
    start_reception = true;
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    ASSERT_EQ(heartbeat_found, !disable_heartbeat_piggyback);
}

TEST(Reliability, DisableHeartbeatPiggybackFalse)
{
    reliability_disable_heartbeat_piggyback(false);
}

TEST(Reliability, DisableHeartbeatPiggybackTrue)
{
    reliability_disable_heartbeat_piggyback(true);
}

/**
 * Regression test for the byte accounting of piggyback heartbeats when samples are delivered through an
 * asynchronous flow controller.
 *
 * Asynchronous flow controllers reuse a single RTPSMessageGroup across deliveries, so the number of bytes
 * processed by the group is cumulative. The writer must only charge the bytes added by each delivery against
 * its send buffer budget. Otherwise the budget is exhausted after a handful of samples and a heartbeat is
 * piggybacked onto almost every DATA message.
 */
TEST(Reliability, AsyncPiggybackHeartbeatByteAccounting)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Small samples: the total number of bytes sent stays well below the send buffer size, so no piggyback
    // heartbeat should be sent while the history is not full.
    static constexpr size_t NUM_SAMPLES = 300;

    std::atomic<uint32_t> heartbeats_found{ 0 };
    std::atomic<bool> start_reception{ false };
    EntityId_t writer_id;

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->drop_heartbeat_messages_filter_ =
            [&heartbeats_found, &writer_id, &start_reception](CDRMessage_t& msg) -> bool
            {
                if (start_reception)
                {
                    auto old_pos = msg.pos;
                    msg.pos += 4;
                    EntityId_t writer_id_msg;
                    CDRMessage::readEntityId(&msg, &writer_id_msg);
                    if (writer_id == writer_id_msg)
                    {
                        ++heartbeats_found;
                    }
                    msg.pos = old_pos;
                }
                return false;
            };

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(2 * NUM_SAMPLES)
            .heartbeat_period_seconds(180000)
            .asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .init();
    writer_id = writer.datawriter_guid().entityId;

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Wait for the initial heartbeat/acknack exchange to finish
    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto data = default_helloworld_data_generator(NUM_SAMPLES);
    reader.startReception(data);
    start_reception = true;
    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    EXPECT_EQ(0u, heartbeats_found.load());
}
