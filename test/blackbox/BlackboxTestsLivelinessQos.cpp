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

//! Tests that when kind is automatic liveliness is never lost, even if the writer never sends data
BLACKBOXTEST(LivelinessQos, Liveliness_Automatic)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Liveliness lease duration and announcement period
    uint32_t liveliness_ms = 20;
    eprosima::fastrtps::Duration_t liveliness_s(liveliness_ms * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(liveliness_ms * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
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

//! Same as above in best-effort
BLACKBOXTEST(LivelinessQos, Liveliness_Automatic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Liveliness lease duration and announcement period
    uint32_t liveliness_ms = 20;
    eprosima::fastrtps::Duration_t liveliness_s(liveliness_ms * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(liveliness_ms * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
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

//! Tests MANUAL_BY_PARTICIPANT liveliness when livleliness lease duration is less than the writer write rate
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 3u);
}

//! Tests the same as above but for best-effort
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 3u);
}

//! Same as ShortLiveliness_ManualByParticipant but using assert_liveliness() instead of writing data
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant_AssertMethod)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (uint32_t count=0; count<writer_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 3u);
}

//! Tests MANUAL_BY_PARTICIPANT liveliness when livleliness lease duration is greater than the writer write rate
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2.0 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2.0 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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

    // Liveliness not lost yet
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Same as above but for best-effort
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2.0 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2.0 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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

    // Liveliness not lost yet
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Same as LongLiveliness_ManualByParticipant but using assert_liveliness() method instead of writing samples
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant_AssertMethod)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t writer_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2.0 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2.0 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (uint32_t count=0; count<writer_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    // Liveliness not lost yet
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests MANUAL_BY_TOPIC liveliness when lease duration is short in comparison with the write rate
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByTopic)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 3u);
}

//! Same as above but for best-effort
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByTopic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 3u);
}

//! Same as ShortLiveliness_ManualByTopic but using assert_liveliness() method instead of writing samples
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByTopic_AssertMehotd)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (uint32_t count=0; count<num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_lost(), 3u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 3u);
}

//! Same as ShortLiveliness_ManualByTopic_AssertMehotd but using best effort reader and writer
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByTopic_BestEffort_AssertMehotd)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (uint32_t count=0; count<num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 3u);

    // Note that in MANUAL_BY_TOPIC liveliness, when not writing data but using assert_liveliness instead,
    // the QoS relies on sending a heartbeat to let the reader know that the writer is alive
    // However best-effort writers don't send heartbeats, so the reader in this case will never get notified
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 0u);
}

//! Tests MANUAL_BY_TOPIC liveliness when lease duration is long in comparison with the write rate
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByTopic)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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

    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Same as above but for best-effort
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByTopic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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

    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Same as above LongLiveliness_ManualByTopic but using assert_liveliness() instead of writing samples
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByTopic_AssertMethod)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (size_t count=0; count < num_samples; count++)
    {
        // Send data
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Same as LongLiveliness_ManualByTopic_AssertMethod but using best effort reader and writer
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByTopic_BestEffort_AssertMethod)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (size_t count=0; count < num_samples; count++)
    {
        // Send data
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 0u);

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));
    EXPECT_EQ(writer.times_liveliness_lost(), 1u);

    // Note that in MANUAL_BY_TOPIC liveliness, when not writing data but using assert_liveliness instead,
    // the QoS relies on sending a heartbeat to let the reader know that the writer is alive
    // However best-effort writers don't send heartbeats, so the reader in this case will never get notified
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 0u);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, has long liveliness, is manual by participant, and uses write method
//! Reader is reliable, has long liveliness, is automatic
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_Reliable_Write)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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
}

//! Tests liveliness with the following parameters
//! Writer is reliable, has short liveliness, is manual by participant, and uses write method
//! Reader is reliable, has short liveliness, is automatic
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_Reliable_Write)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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
}

//! Tests liveliness with the following parameters
//! Writer is reliable, has long liveliness, is manual by participant, and uses assert method
//! Reader is reliable, has long liveliness, is automatic
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_Reliable_Assert)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (size_t count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));

    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, has short liveliness, is manual by participant, and uses assert method
//! Reader is reliable, has short liveliness, is automatic
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_Reliable_Assert)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (size_t count = 0; count<num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, has long liveliness, is manual by participant, and uses write method
//! Reader is best-effort, has long liveliness, is automatic
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_BestEffort_Write)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, has short liveliness, is manual by participant, and uses write method
//! Reader is best-effort, has short liveliness, is automatic
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_BestEffort_Write)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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
}

//! Tests liveliness with the following parameters
//! Writer is best-effort, has long liveliness, is manual by participant, and uses assert method
//! Reader is best-effort, has long liveliness, is automatic
BLACKBOXTEST(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_BestEffort_Assert)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 2 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 2 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    for (size_t count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    // Wait a bit longer
    std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms * 2));

    EXPECT_EQ(writer.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 1u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, and uses manual by topic liveliness kind
//! Reader is reliable, and uses automatic liveliness kind
//! Liveliness lease duration is short in comparison to writer write/assert rate
BLACKBOXTEST(LivelinessQos, ManualByTopic_Automatic_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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
BLACKBOXTEST(LivelinessQos, ManualByTopic_Automatic_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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
    // As best-effort readers do not process heartbeats the expected number of times liveliness was lost
    // and recovered corresponds to the bit in the test when we sent samples (not when we asserted liveliness)
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness with the following parameters
//! Writer is reliable, and uses manual by topic liveliness kind
//! Reader is reliable, and uses manual by participant liveliness kind
//! Liveliness lease duration is short in comparison to writer write/assert rate
BLACKBOXTEST(LivelinessQos, ManualByTopic_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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
BLACKBOXTEST(LivelinessQos, ManualByTopic_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds and number of samples to write
    uint32_t writer_sleep_ms = 100;
    uint32_t num_samples = 3;

    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
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
    // Note that, as best-effor readers do not proccess heartbeats and assert_liveliness() relies on sending a
    // heartbeat to assess liveliness, the expected number of times liveliness was lost and recovered
    // corresponds only to the bit in the test when the writer wrote samples
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

// TODO raquel Add tests with participants having more than one publisher/subscriber
