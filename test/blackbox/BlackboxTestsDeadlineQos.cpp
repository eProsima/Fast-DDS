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

#include <fastrtps/utils/TimeConversion.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

BLACKBOXTEST(DeadlineQos, NoKeyTopicLongDeadline)
{
    // This test sets a long deadline (long in comparison to the write rate),
    // makes the writer send a few samples and checks that the deadline was
    // not missed
    // Uses a topic with no key

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Deadline period in seconds
    eprosima::fastrtps::rtps::Duration_t deadline_s(writer_sleep_ms * 2 * 1e-3);

    reader.deadline_period(deadline_s);
    writer.deadline_period(deadline_s);
    reader.init();
    writer.init();

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

    EXPECT_EQ(writer.missed_deadlines(), 0);
    EXPECT_EQ(reader.missed_deadlines(), 0);
}

BLACKBOXTEST(DeadlineQos, NoKeyTopicShortDeadline)
{
    // This test sets a short deadline (short compared to the write rate),
    // makes the writer send a few samples and checks that the deadline was missed every time
    // Uses a topic with no key

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Deadline period in seconds
    eprosima::fastrtps::rtps::Duration_t deadline_s(writer_sleep_ms * 0.1 * 1e-3);

    reader.deadline_period(deadline_s);
    writer.deadline_period(deadline_s);
    reader.init();
    writer.init();

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
    EXPECT_EQ(writer.missed_deadlines(), writer_samples);
    EXPECT_EQ(reader.missed_deadlines(), writer_samples);
}

BLACKBOXTEST(DeadlineQos, KeyedTopicLongDeadline)
{
    // This test sets a long deadline (long in comparison to the write rate),
    // makes the writer send a few samples and checks that the deadline was met
    // Uses a topic with key

    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Number of samples written by writer
    uint32_t writer_samples = 4;
    // Deadline period in seconds
    eprosima::fastrtps::rtps::Duration_t deadline_s(writer_sleep_ms * 4 * 1e-3);

    reader.deadline_period(deadline_s);
    writer.deadline_period(deadline_s);
    reader.key(true);
    writer.key(true);
    reader.init();
    writer.init();

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
        data_sample.key(count % 2);
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.missed_deadlines(), 0);
    EXPECT_EQ(reader.missed_deadlines(), 0);
}

BLACKBOXTEST(DeadlineQos, KeyedTopicShortDeadline)
{
    // This test sets a short deadline (short compared to the write rate),
    // makes the writer send a few samples and checks that the deadline was missed every time
    // Uses a topic with key

    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    // Only one sample sent by the writer
    uint32_t writer_samples = 4;
    // Time to wait before sending the sample
    uint32_t writer_sleep_ms = 100;
    // Deadline period in ms
    eprosima::fastrtps::rtps::Duration_t deadline_s(writer_sleep_ms * 0.1 * 1e-3);

    reader.deadline_period(deadline_s);
    writer.deadline_period(deadline_s);
    reader.key(true);
    writer.key(true);
    reader.init();
    writer.init();

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
        data_sample.key(count % 2);
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    EXPECT_EQ(writer.missed_deadlines(), writer_samples);
    EXPECT_EQ(reader.missed_deadlines(), writer_samples);
}
