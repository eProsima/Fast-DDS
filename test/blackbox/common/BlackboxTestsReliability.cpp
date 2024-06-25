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

#include <thread>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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
                    msg.pos += 4;
                    EntityId_t writer_id_msg = eprosima::fastdds::helpers::cdr_parse_entity_id(
                        (char*)&msg.buffer[msg.pos]);
                    if (writer_id == writer_id_msg)
                    {
                        heartbeat_found = true;
                    }
                    msg.pos = old_pos;
                }
                return false;
            };

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .heartbeat_period_seconds(180000)
            .disable_heartbeat_piggyback(disable_heartbeat_piggyback)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .init();
    writer_id = writer.datawriter_guid().entityId;

    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
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
