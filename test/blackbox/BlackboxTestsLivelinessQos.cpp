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
#include "PubSubParticipant.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

//! Tests that when kind is automatic liveliness is never lost, even if the writer never sends data
TEST(LivelinessQos, Liveliness_Automatic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Liveliness lease duration and announcement period
    uint32_t liveliness_ms = 20;
    Duration_t liveliness_s(liveliness_ms * 1e-3);
    Duration_t announcement_period(liveliness_ms * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::this_thread::sleep_for(std::chrono::milliseconds(liveliness_ms * 10));

    // When using automatic kind, liveliness on both publisher and subscriber should never be lost
    // It would only be lost if the publishing application crashed, which can't be reproduced in the tests
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
}

//! Same as above using best-effort reliability
TEST(LivelinessQos, Liveliness_Automatic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Liveliness lease duration and announcement period
    uint32_t liveliness_ms = 20;
    Duration_t liveliness_s(liveliness_ms * 1e-3);
    Duration_t announcement_period(liveliness_ms * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::this_thread::sleep_for(std::chrono::milliseconds(liveliness_ms * 10));

    // When using automatic kind, liveliness on both publisher and subscriber should never be lost
    // It would only be lost if the publishing application crashed, which can't be reproduced in the tests
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
}

//! Tests liveliness with the following paramters
//! Writer is reliable, and MANUAL_BY_PARTICIPANT
//! Reader is reliable, and MANUAL_BY_PARTICIPANT
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ShortLiveliness_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following paramters
//! Writer is best-effort, and MANUAL_BY_PARTICIPANT
//! Reader is best-effort, and MANUAL_BY_PARTICIPANT
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ShortLiveliness_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count<num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following paramters
//! Writer is best-effort, and MANUAL_BY_PARTICIPANT
//! Reader is best-effort, and MANUAL_BY_PARTICIPANT
//! Liveliness lease duration is long in comparison to writer write/assert rate
TEST(LivelinessQos, LongLiveliness_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 10;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 2.0 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 2.0 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests liveliness with the following paramters
//! Writer is best-effort, and MANUAL_BY_PARTICIPANT
//! Reader is best-effort, and MANUAL_BY_PARTICIPANT
//! Liveliness lease duration is long in comparison to writer write/assert rate
TEST(LivelinessQos, LongLiveliness_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 10;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 2.0 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 2.0 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    for (count = 0; count < writer_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests liveliness with the following paramters
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ShortLiveliness_ManualByTopic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following paramters
//! Writer is best-effort, and MANUAL_BY_TOPIC
//! Reader is best-effort, and MANUAL_BY_TOPIC
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ShortLiveliness_ManualByTopic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    // Note that in MANUAL_BY_TOPIC liveliness, the assert_liveliness() method relies on sending a heartbeat
    // However best-effort writers don't send heartbeats, so the reader in this case will never get notified
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness with the following paramters
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
//! Liveliness lease duration is long in comparison to writer write/assert rate
TEST(LivelinessQos, LongLiveliness_ManualByTopic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 10;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    for (count=0; count<num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests liveliness with the following paramters
//! Writer is best-effort, and MANUAL_BY_TOPIC
//! Reader is best-effort, and MANUAL_BY_TOPIC
//! Liveliness lease duration is long in comparison to writer write/assert rate
TEST(LivelinessQos, LongLiveliness_ManualByTopic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 10;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    for(count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 2u);
    // Note that MANUAL_BY_TOPIC liveliness relies on sending heartbeats when using the assert method
    // However best-effor writers do not send heartbeats, so the reader will never get notified
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, liveliness is manual by participant
//! Reader is reliable, liveliness is automatic
//! Liveliness lease duration is long in comparison to the writer write/assert rate
TEST(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 10;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, liveliness is manual by participant
//! Reader is reliable, liveliness is automatic
//! Liveliness is short in comparison to the writer write/assert rate
TEST(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, liveliness is manual by participant
//! Reader is best-effort, liveliness is automatic
//! Liveliness is long in comparison to the writer write/assert rate
TEST(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 10;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 5));
    EXPECT_EQ(writer.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 2u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, liveliness is manual by participant
//! Reader is best-effort, liveliness is automatic
//! Liveliness is short in comparison to the writer write/assert rate
TEST(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, and uses manual by topic liveliness kind
//! Reader is reliable, and uses automatic liveliness kind
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ManualByTopic_Automatic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, and uses manual by topic liveliness kind
//! Reader is best-effort, and uses automatic liveliness kind
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ManualByTopic_Automatic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    // As best-effort readers do not process heartbeats the expected number of times liveliness was lost
    // and recovered corresponds to the bit in the test when we sent samples (not when we asserted liveliness)
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, and uses manual by topic liveliness kind
//! Reader is reliable, and uses manual by participant liveliness kind
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ManualByTopic_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, and uses manual by topic liveliness kind
//! Reader is best-effort, and uses manual by participant liveliness kind
//! Liveliness lease duration is short in comparison to writer write/assert rate
TEST(LivelinessQos, ManualByTopic_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    size_t count = 0;
    for (auto data_sample : data)
    {
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    // Note that, as best-effor readers do not proccess heartbeats and assert_liveliness() relies on sending a
    // heartbeat to assess liveliness, the expected number of times liveliness was lost and recovered
    // corresponds only to the bit in the test when the writer wrote samples
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness in the following scenario
//! A participant with two publishers (AUTOMATIC and MANUAL_BY_PARTICIPANT) and a single topic
//! A participant with one subscriber (AUTOMATIC)
TEST(LivelinessQos, TwoWriters_OneReader_ManualByTopic)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 1;
    unsigned int lease_duration_ms = 500;
    unsigned int announcement_period_ms = 250;

    // Publishers
    PubSubParticipant<HelloWorldType> publishers(num_pub, 0, num_sub, 0);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1));

    // Subscribers
    PubSubParticipant<HelloWorldType> subscribers(0, num_sub, 0, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();
    // Just sleep a bit to give the subscriber the chance to detect that writers are alive
    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms));

    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 0u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 2u);
    // Note that from the subscriber point of view both writers recovered liveliness, even if the
    // MANUAL_BY_PARTICIPANT one didn't assert liveliness explicitly. This is the expected
    // behaviour according to the RTPS standard, section 2.2.3.11 LIVELINESS
}

//! Tests liveliness in the following scenario
//! A participant with two publishers and two topics
//! A participant with two subscribers and two topics
//! Manual by participant liveliness
//! Only one publisher asserts liveliness manually
TEST(LivelinessQos, TwoWriters_TwoReaders_ManualByParticipant)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 2;
    unsigned int lease_duration_s = 1;
    unsigned int announcement_period_s = 0.5;

    // Publishers
    PubSubParticipant<HelloWorldType> publishers(num_pub, 0, num_sub, 0);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME + "1")
            .pub_liveliness_announcement_period(announcement_period_s)
            .pub_liveliness_lease_duration(lease_duration_s)
            .pub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0));
    publishers.pub_topic_name(TEST_TOPIC_NAME + "2")
            .pub_liveliness_announcement_period(announcement_period_s)
            .pub_liveliness_lease_duration(lease_duration_s)
            .pub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1));

    // Subscribers
    PubSubParticipant<HelloWorldType> subscribers(0, num_sub, 0, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "1")
            .sub_liveliness_lease_duration(lease_duration_s)
            .sub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0));
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "2")
            .sub_liveliness_lease_duration(lease_duration_s)
            .sub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    unsigned int num_assertions = 4;
    unsigned int assert_rate_ms = 50;
    for (unsigned int count = 0; count < num_assertions; count++)
    {
        publishers.assert_liveliness(0u);
        std::this_thread::sleep_for(std::chrono::milliseconds(assert_rate_ms));
    }
    // Only one publisher asserts liveliness but the other should be asserted by the QoS, as
    // liveliness kind is manual by participant
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 0u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), num_pub);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 0u);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), num_pub);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), num_pub);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), num_pub);
}

//! Tests liveliness in the same scenario as above but using manual by topic liveliness
//! A participant with two publishers and two topics
//! A participant with two subscribers and two topics
//! Manual by topic liveliness
//! Only one publisher asserts liveliness manually
TEST(LivelinessQos, TwoWriters_TwoReaders_ManualByTopic)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 500;
    unsigned int announcement_period_ms = 250;

    // Publishers
    PubSubParticipant<HelloWorldType> publishers(num_pub, 0, num_sub, 0);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME + "1")
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0));
    publishers.pub_topic_name(TEST_TOPIC_NAME + "2")
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1));

    // Subscribers
    PubSubParticipant<HelloWorldType> subscribers(0, num_sub, 0, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "1")
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0));
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "2")
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    unsigned int num_assertions = 4;
    unsigned int assert_rate_ms = 10;
    for (unsigned int count = 0; count < num_assertions; count++)
    {
        publishers.assert_liveliness(0u);
        std::this_thread::sleep_for(std::chrono::milliseconds(assert_rate_ms));
    }
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 0u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 0u);

    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 1u);
}

//! Tests liveliness in the following scenario
//! A participant with two publishers with different liveliness kinds
//! A participant with two subscribers with different liveliness kinds
TEST(LivelinessQos, TwoWriters_TwoReaders)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 500;
    unsigned int announcement_period_ms = 250;

    // Publishers
    PubSubParticipant<HelloWorldType> publishers(num_pub, 0, num_sub, 0);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1));

    // Subscribers
    PubSubParticipant<HelloWorldType> subscribers(0, num_sub, 0, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    publishers.assert_liveliness(1u);
    std::this_thread::sleep_for(std::chrono::milliseconds(announcement_period_ms * 2));

    // All three subscribers are notified that liveliness was recovered
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 3u);

    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 1u);
}

//! Tests liveliness in the same scenario as above but using manual by topic liveliness
//! A participant with three publishers with different liveliness kinds
//! A participant with three subscribers with different liveliness kinds
TEST(LivelinessQos, ThreeWriters_ThreeReaders)
{
    unsigned int num_pub = 3;
    unsigned int num_sub = 3;
    unsigned int lease_duration_ms = 500;
    unsigned int announcement_period_ms = 250;

    // Publishers
    PubSubParticipant<HelloWorldType> publishers(num_pub, 0, num_sub, 0);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(2));

    // Subscribers
    PubSubParticipant<HelloWorldType> subscribers(0, num_sub, 0, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(2));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    // From the point of view of the AUTOMATIC reader, the three writers will have recovered liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(announcement_period_ms * 2));
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 3u);

    // The manual by participant writer asserts liveliness
    // The manual by participant reader will consider that both the manual by participant and manual by topic
    // writers have recovered liveliness
    publishers.assert_liveliness(1u);
    std::this_thread::sleep_for(std::chrono::milliseconds(announcement_period_ms * 2));
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 5u);

    // The manual by topic publisher asserts liveliness
    // The manual by topic reader will detect that a new writer has recovered liveliness
    publishers.assert_liveliness(2u);
    std::this_thread::sleep_for(std::chrono::milliseconds(announcement_period_ms * 2));
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 6u);

    // Wait so that the manual by participant and manual by topic writers lose liveliness
    // The manual by participant subscriber will detect that two writers lost liveliness
    // The manual by topic subscriber will detect that one writer lost liveliness
    // This means that the subscribint participant will see that liveliness was lost three times
    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 2);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 3);
}

//! Tests the case where a writer matched to two readers changes QoS and stays matched to only one reader
TEST(LivelinessQos, UnmatchedWriter)
{
    unsigned int num_pub = 1;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 500;
    unsigned int announcement_period_ms = 250;

    // Publishers
    PubSubParticipant<HelloWorldType> publishers(num_pub, 0, num_sub, 0);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .pub_deadline_period(0.15);
    ASSERT_TRUE(publishers.init_publisher(0));

    // Subscribers
    PubSubParticipant<HelloWorldType> subscribers(0, num_sub, 0, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .sub_deadline_period(0.5);
    ASSERT_TRUE(subscribers.init_subscriber(0));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .sub_deadline_period(1.5);
    ASSERT_TRUE(subscribers.init_subscriber(1));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    // Change deadline period of the first subscriber so that it no longer matches with the publisher
    subscribers.sub_update_deadline_period(0.10, 0u);

    publishers.assert_liveliness(0u);
    std::this_thread::sleep_for(std::chrono::milliseconds(announcement_period_ms * 2));
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 1u);
}
