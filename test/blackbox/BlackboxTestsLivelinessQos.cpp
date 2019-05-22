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

//! Tests that when liveliness is short (in comparison to write rate) listener callbacks are invoked
//! and liveliness status structs are updated correctly. For automatic kind
BLACKBOXTEST(LivelinessQos, ShortLiveliness_Automatic)
{
    // This test sets a short lease duration (short in comparison to the write rate),
    // makes the writer send a few samples and checks that liveliness was lost

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // No need to write samples for automatic liveliness
    // Number of samples to write
    size_t writer_samples = 3;
    for (size_t i=0; i<writer_samples; i++)
    {
        i++;
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    // When using automatic kind, liveliness should not be lost as it is managed by message writer and reader
    // Liveliness would only be lost if the publishing application crashed, which can't be reproduced in the test
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_changed(), 0u);
}

//! Tests that when liveliness is short (in comparison to write rate) listener callbacks are invoked
//! and liveliness status structs are updated correctly. For manual by topic kind
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByTopic)
{
    // This test sets a short lease duration (short in comparison to the write rate),
    // makes the writer send a few samples and checks that liveliness was lost

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Number of samples to write
    uint32_t num_samples = 3;
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

//    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_changed(), 6u); // Liveliness was lost 3 times and re-asserted 3 times
}

//! Tests that when liveliness is short (in comparison to write rate) listener callbacks are invoked
//! and liveliness status structs are updated correctly. For manual by participant kind
BLACKBOXTEST(LivelinessQos, ShortLiveliness_ManualByParticipant)
{
    // This test sets a short lease duration (short in comparison to the write rate),
    // makes the writer send a few samples and checks that liveliness was lost

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Liveliness lease duration and announcement period, in seconds
    eprosima::fastrtps::Duration_t liveliness_s(writer_sleep_ms * 0.1 * 1e-3);
    eprosima::fastrtps::Duration_t announcement_period(writer_sleep_ms * 0.1 * 1e-3 * 0.9);

    reader.liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(liveliness_s)
            .init();
    writer.liveliness_kind(eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period)
            .liveliness_lease_duration(liveliness_s)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Number of samples to write
    uint32_t writer_samples = 3;
    auto data = default_helloworld_data_generator(writer_samples);

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

//    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_changed(), 6u); // Liveliness was lost 3 times and re-asserted 3 times
}
// TODO raquel add tests mixing different liveliness kinds (i.e. different subscribers with different kinds):
// AUTOMATIC             AUTOMATIC
// MANUAL_BY_PARTICIPANT MANUAL_BY_PARTICIPANT
// MANUAL_BY_TOPIC       MANUAL_BY_TOPIC
// MANUAL_BY_PARTICIPANT AUTOMATIC
// MANUAL_BY_TOPIC       AUTOMATIC
// MANUAL_BY_TOPIC       MANUAL_BY_PARTICIPANT
// TODO raquel also test best-effort
// TODO raquel test at least one case with HAVE_SECURITY
