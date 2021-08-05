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

#include <gtest/gtest.h>

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class DeadlineQos : public testing::TestWithParam<bool>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }

    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }
    }

};

TEST_P(DeadlineQos, NoKeyTopicLongDeadline)
{
    // This test sets a long deadline (long in comparison to the write rate),
    // makes the writer send a few samples and checks that the deadline was
    // not missed
    // Uses a topic with no key

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 10;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Deadline period in milliseconds
    uint32_t deadline_period_ms = 100000;

    reader.deadline_period(deadline_period_ms * 1e-3).init();
    writer.deadline_period(deadline_period_ms * 1e-3).init();

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
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.missed_deadlines(), 0u);
    EXPECT_EQ(reader.missed_deadlines(), 0u);
}

TEST_P(DeadlineQos, NoKeyTopicShortDeadline)
{
    // This test sets a short deadline (short compared to the write rate),
    // makes the writer send a few samples and checks that the deadline was missed every time
    // Uses a topic with no key

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 1000;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Deadline period in milliseconds
    uint32_t deadline_period_ms = 10;

    reader.deadline_period(deadline_period_ms * 1e-3).init();
    writer.deadline_period(deadline_period_ms * 1e-3).init();

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
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    // All samples should have missed the deadline
    EXPECT_GE(writer.missed_deadlines(), writer_samples);
    EXPECT_GE(reader.missed_deadlines(), writer_samples);
}

TEST_P(DeadlineQos, KeyedTopicLongDeadline)
{
    // This test sets a long deadline (long in comparison to the write rate),
    // makes the writer send a few samples and checks that the deadline was met
    // Uses a topic with key

    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 10;
    // Number of samples written by writer
    uint32_t writer_samples = 4;
    // Deadline period in milliseconds
    uint32_t deadline_period_ms = 100000;

    reader.deadline_period(deadline_period_ms * 1e-3).init();
    writer.deadline_period(deadline_period_ms * 1e-3).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(writer_samples);

    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        data_sample.key(count % 2 + 1);
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.missed_deadlines(), 0u);
    EXPECT_EQ(reader.missed_deadlines(), 0u);
}

TEST_P(DeadlineQos, KeyedTopicShortDeadline)
{
    // This test sets a short deadline (short compared to the write rate),
    // makes the writer send a few samples and checks that the deadline was missed every time
    // Uses a topic with key

    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    // Number of samples to send
    uint32_t writer_samples = 4;
    // Time to wait before sending the sample
    uint32_t writer_sleep_ms = 1000;
    // Deadline period in ms
    uint32_t deadline_period_ms = 10;

    reader.deadline_period(deadline_period_ms * 1e-3).init();
    writer.deadline_period(deadline_period_ms * 1e-3).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(writer_samples);

    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        data_sample.key(count % 2 + 1);
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_GE(writer.missed_deadlines(), writer_samples);
    EXPECT_GE(reader.missed_deadlines(), writer_samples);
}

/**
 * This test creates a volatile writer with a deadline of 10 ms and no readers.
 * The writer is used to send one sample, and after the deadline period has elapsed, a check is
 * performed to verify that one offered deadline was missed.
 */
TEST_P(DeadlineQos, KeyedTopicNoReaderVolatileWriterSetDeadline)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.durability_kind(VOLATILE_DURABILITY_QOS);

    uint32_t deadline_period_ms = 50;

    writer.deadline_period(deadline_period_ms * 1e-3).init();
    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(1);

    writer.send_sample(data.front());
    std::this_thread::sleep_for(std::chrono::milliseconds(deadline_period_ms * 2));

    EXPECT_GE(writer.missed_deadlines(), 1u);
}

/**
 * This test creates a volatile writer with a deadline of 10 ms and a best effort reader.
 * The writer is used to send one sample, and after the deadline period has elapsed, a check is
 * performed to verify that one offered deadline was missed.
 */
TEST_P(DeadlineQos, KeyedTopicBestEffortReaderVolatileWriterSetDeadline)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);

    writer.durability_kind(VOLATILE_DURABILITY_QOS);

    uint32_t deadline_period_ms = 50;

    writer.deadline_period(deadline_period_ms * 1e-3).init();
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(1);

    writer.send_sample(data.front());
    std::this_thread::sleep_for(std::chrono::milliseconds(deadline_period_ms * 2));

    EXPECT_GE(writer.missed_deadlines(), 1u);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DeadlineQos,
        DeadlineQos,
        testing::Values(false, true),
        [](const testing::TestParamInfo<DeadlineQos::ParamType>& info)
        {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });
