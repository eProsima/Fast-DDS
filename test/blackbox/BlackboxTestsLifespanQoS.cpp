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

TEST(LifespanQos, LongLifespan)
{
    // This test sets a long lifespan, makes the writer send a few samples
    // and checks that those changes can be removed from the history
    // as they should not have been removed due to lifespan QoS

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 1;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Lifespan period in seconds
    eprosima::fastrtps::Duration_t lifespan_s(writer_sleep_ms * 1000 * 1e-3);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS);
    writer.lifespan_period(lifespan_s);
    writer.init();

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS);
    reader.lifespan_period(lifespan_s);
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(writer_samples);
    writer.send(data, writer_sleep_ms);
    ASSERT_TRUE(data.empty());

    // Changes should not have been removed due to lifespan
    // therefore they should still exist in the history and
    // we should be able to remove them manually
    size_t removed_pub = 0;
    writer.remove_all_changes(&removed_pub);
    EXPECT_EQ(removed_pub, writer_samples);

    // On the reader side we should be able to take the data
    HelloWorldType::type msg;
    eprosima::fastrtps::SampleInfo_t info;
    EXPECT_EQ(reader.takeNextData(&msg, &info), true);
    EXPECT_EQ(reader.takeNextData(&msg, &info), true);
    EXPECT_EQ(reader.takeNextData(&msg, &info), true);
}

TEST(LifespanQos, ShortLifespan)
{
    // This test sets a short lifespan, makes the writer send a few samples
    // and checks that those samples cannot be removed from the history as
    // they have been removed by lifespan QoS

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 100;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Lifespan period in seconds
    eprosima::fastrtps::Duration_t lifespan_s(writer_sleep_ms * 0.1 * 1e-3);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS);
    writer.lifespan_period(lifespan_s);
    writer.init();

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS);
    reader.lifespan_period(lifespan_s);
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(writer_samples);
    writer.send(data, writer_sleep_ms);
    ASSERT_TRUE(data.empty());

    // Changes should have been removed from history by lifespan QoS
    // so we should not be able to remove them anymore
    size_t removed_pub = 0;
    writer.remove_all_changes(&removed_pub);
    EXPECT_EQ(removed_pub, 0u);

    // On the reader side we should not be able to take the data
    HelloWorldType::type msg;
    eprosima::fastrtps::SampleInfo_t info;
    EXPECT_EQ(reader.takeNextData(&msg, &info), false);
    EXPECT_EQ(reader.takeNextData(&msg, &info), false);
    EXPECT_EQ(reader.takeNextData(&msg, &info), false);
}
