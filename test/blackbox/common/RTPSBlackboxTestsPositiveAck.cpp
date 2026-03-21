// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

TEST(RTPSAck, EnableUpdatabilityOfPositiveAcksPeriodRTPSLayer)
{
    // This test checks that only the positive ACKs
    // period is updatable at runtime on the RTPS Layer.

    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).
            disable_positive_acks(true).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            durability(eprosima::fastdds::rtps::DurabilityKind_t::VOLATILE).
            add_to_multicast_locator_list(ip, global_port).
            auto_remove_on_volatile().
            disable_positive_acks_seconds(true, 1).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    // Check history is not empty
    EXPECT_FALSE(writer.is_history_empty());
    // Check history after keep_duration period
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    EXPECT_TRUE(writer.is_history_empty());

    // Update attributes at RTPS layer
    WriterAttributes w_att;
    w_att.disable_positive_acks = true;
    w_att.keep_duration = eprosima::fastdds::dds::Duration_t(2, 0);

    writer.update_attributes(w_att);

    data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    // Check history is not empty
    EXPECT_FALSE(writer.is_history_empty());
    // Check history before keep_duration period
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    EXPECT_FALSE(writer.is_history_empty());
    // Check history after keep_duration period
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    EXPECT_TRUE(writer.is_history_empty());

    // Update attributes at RTPS layer
    w_att.disable_positive_acks = false;

    writer.update_attributes(w_att);

    // Check that positive_acks feature is not changed at runtime
    EXPECT_TRUE(writer.get_disable_positive_acks());
}