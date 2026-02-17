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

#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <LogCounter.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using fastlog = eprosima::fastdds::dds::Log;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DeadlineQos : public ::testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(DeadlineQos, NoKeyTopicLongDeadline)
{
    // This test sets a long deadline (long in comparison to the write rate),
    // makes the writer send a few samples and checks that the deadline was
    // not missed
    // Uses a topic with no key

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

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

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

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

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

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

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

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
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS);

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
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS);

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

/**
 * Testing Redmine issue #23289.
 * Writer-side version of ZeroDeadlinePeriod.
 * Regression test for the zero-deadline period bug.
 * Creating a DataWriter with a deadline of 0.
 * Checking if a warning is logged exactly once, the timer is cancelled without missed deadline
 * messages and a total count and count change set to max integer.
 * Checking warnings, total count and count change when changing the deadline.
 */
TEST_P(DeadlineQos, ZeroDeadlinePeriodWriter)
{
    auto observer = std::make_shared<eprosima::fastdds::testing::LogCounterObserver>(/*store=*/ false);
    std::unique_ptr<eprosima::fastdds::testing::LogCounterConsumer> consumer;
    consumer.reset(new eprosima::fastdds::testing::LogCounterConsumer(observer));

    fastlog::ClearConsumers();
    fastlog::RegisterConsumer(std::move(consumer));
    fastlog::SetVerbosity(fastlog::Kind::Warning);

    observer->set_global_needle("Deadline period is 0");

    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    writer.deadline_period(0.0).init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(1);
    writer.send_sample(data.front());

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Writer offered-deadline counters should be saturated
    EXPECT_EQ(writer.missed_deadlines(),
            std::numeric_limits<uint32_t>::max()) << "Expected the max value after a zero-deadline warning.";
    EXPECT_EQ(writer.missed_deadlines_change(), std::numeric_limits<uint32_t>::max());

    const auto prev = observer->matched_global();
    EXPECT_EQ(prev, 1u) << "Expected exactly one 'deadline=0' warning\n";

    const auto pre_total = writer.missed_deadlines();
    const auto pre_change = writer.missed_deadlines_change();

    // Wait for a period long enough to expect a new miss if the timer were still active
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    const auto post_total = writer.missed_deadlines();
    const auto post_change = writer.missed_deadlines_change();

    EXPECT_EQ(pre_total, post_total) << "The total count should not change, as the timer was canceled.";
    EXPECT_EQ(pre_change, post_change) << "The total_count_change should not change, as the timer was canceled.";
    EXPECT_EQ(observer->matched_global(), prev) << "No extra warnings after cancel.";

    auto q = writer.get_qos();
    q.deadline().period = Duration_t(0.1);

    ASSERT_TRUE(writer.set_qos(q)); // Update 0 -> finite

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_EQ(writer.missed_deadlines(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(writer.missed_deadlines_change(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(observer->matched_global(), prev) << "No new warning when moving reader from 0 -> finite";

    q.deadline().period = Duration_t(0.0);

    ASSERT_TRUE(writer.set_qos(q)); // Update finite -> 0

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_EQ(writer.missed_deadlines(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(writer.missed_deadlines_change(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(observer->matched_global(), prev + 1) << "Exactly one new warning.";

    fastlog::ClearConsumers();
}

/**
 * Testing Redmine issue #23289.
 * Regression test for the zero-deadline period bug.
 * Reader-side version of ZeroDeadlinePeriod.
 * Creating a DataReader with a deadline of 0.
 * Checking if a warning is logged exactly once, the timer is cancelled without missed deadline
 * messages and a total count and count change set to max integer.
 * Checking warnings, total count and count change when changing the deadline.
 */
TEST_P(DeadlineQos, ZeroDeadlinePeriodReader)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    // Writer deadline must also be 0 to satisfy the matching rule and ensure discovery
    writer.deadline_period(0.0).init();
    ASSERT_TRUE(writer.isInitialized());

    auto observer = std::make_shared<eprosima::fastdds::testing::LogCounterObserver>(/*store=*/ false);
    std::unique_ptr<eprosima::fastdds::testing::LogCounterConsumer> consumer;
    consumer.reset(new eprosima::fastdds::testing::LogCounterConsumer(observer));

    fastlog::ClearConsumers();
    fastlog::RegisterConsumer(std::move(consumer));
    fastlog::SetVerbosity(fastlog::Kind::Warning);

    observer->set_global_needle("Deadline period is 0");

    // Zero deadline on the READER
    reader.deadline_period(0.0).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(1);
    writer.send_sample(data.front());

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Reader requested-deadline counters should be saturated
    EXPECT_EQ(reader.missed_deadlines(),
            std::numeric_limits<uint32_t>::max()) << "Expected the max value after a zero-deadline warning.";
    EXPECT_EQ(reader.missed_deadlines_change(), std::numeric_limits<uint32_t>::max());

    const auto prev = observer->matched_global();
    EXPECT_EQ(prev, 1u) << "Expected exactly one 'deadline=0' warning\n";

    const auto pre_total  = reader.missed_deadlines();
    const auto pre_change = reader.missed_deadlines_change();

    // Wait for a period long enough to expect a new miss if the timer were still active
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    const auto post_total  = reader.missed_deadlines();
    const auto post_change = reader.missed_deadlines_change();

    EXPECT_EQ(pre_total,  post_total) << "Timer canceled on reader; total must not change.";
    EXPECT_EQ(pre_change, post_change) << "Timer canceled on reader; total_count_change must not change.";
    EXPECT_EQ(observer->matched_global(), prev) << "No extra warnings after cancel.";

    // Now change reader's deadline from 0 -> finite; still no additional warning; counters remain saturated
    auto q = reader.get_qos();
    q.deadline().period = Duration_t(0.1);
    ASSERT_TRUE(reader.set_qos(q));

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_EQ(reader.missed_deadlines(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(reader.missed_deadlines_change(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(observer->matched_global(), prev) << "No new warning when moving reader from 0 -> finite";

    q.deadline().period = Duration_t(0.0);

    ASSERT_TRUE(reader.set_qos(q)); // Update finite -> 0

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_EQ(reader.missed_deadlines(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(reader.missed_deadlines_change(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(observer->matched_global(), prev + 1) << "Exactly one new warning.";

    fastlog::ClearConsumers();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DeadlineQos,
        DeadlineQos,
        ::testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const ::testing::TestParamInfo<DeadlineQos::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }
        });
