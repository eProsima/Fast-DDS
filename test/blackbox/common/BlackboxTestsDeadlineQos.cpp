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

#include <memory>
#include <sstream>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using eprosima::fastdds::dds::Duration_t;
using eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
using eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
using eprosima::fastdds::dds::RETCODE_OK;
using eprosima::fastdds::dds::StatusMask;

// ---- Fast DDS logging API (new) OR Fast RTPS (old) ----
#if __has_include(<fastdds/dds/log/Log.hpp>)
  #include <fastdds/dds/log/Log.hpp>
  #include <fastdds/dds/log/OStreamConsumer.hpp>
  namespace fastlog = eprosima::fastdds::dds;
#elif __has_include(<fastrtps/log/Log.h>)
  #include <fastrtps/log/Log.h>
  #include <fastrtps/log/OStreamConsumer.h>
  namespace fastlog = eprosima::fastrtps;
#else
  #error "No Fast DDS/RTPS log header found"
#endif

// Consumer: counts matching warnings and mirrors them to an ostringstream
class TestStringConsumer : public fastlog::LogConsumer
{
public:
    TestStringConsumer(std::ostringstream& out, std::string needle)
        : out_(out), needle_(std::move(needle))
    {}

    void Consume(const fastlog::Log::Entry& e) override
    {
        // Count exactly the warning we expect
        if (e.kind == fastlog::Log::Kind::Warning &&
            e.message.find(needle_) != std::string::npos)
        {
            count_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    size_t count() const { return count_.load(std::memory_order_relaxed); }

private:
    std::ostringstream& out_;
    std::string needle_;
    std::atomic<size_t> count_{0};
};

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DeadlineQos : public testing::TestWithParam<communication_type>
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
 * Regression test for the zero-deadline period bug.
 * Creating a DataWriter with a deadline of 0.
 * Checking if a warning is logged exactly once, the timer is cancelled without missed deadline
 * messages and a total count set to max integer.
 */
TEST_P(DeadlineQos, DeadlineZeroWithoutDeadlineMissedMessages)
{
    std::ostringstream logbuf;
    const char* needle = "Deadline period is 0";
    auto *consumer_ptr = new TestStringConsumer(logbuf, needle);
    fastlog::Log::ClearConsumers();
    fastlog::Log::RegisterConsumer(std::unique_ptr<fastlog::LogConsumer>(consumer_ptr));
    fastlog::Log::SetVerbosity(fastlog::Log::Kind::Warning);

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

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // To directly read the status struct
    eprosima::fastdds::dds::OfferedDeadlineMissedStatus st{};
    ASSERT_EQ(writer.get_offered_deadline_missed_status(st), eprosima::fastdds::dds::RETCODE_OK);

    EXPECT_EQ(st.total_count, std::numeric_limits<int32_t>::max()) << "Expected the max value after a zero-deadline warning.";
    EXPECT_EQ(st.total_count_change, 0);

    EXPECT_EQ(consumer_ptr->count(), 1u) << "Expected exactly one 'deadline=0' warning\n" << logbuf.str();

    eprosima::fastdds::dds::OfferedDeadlineMissedStatus st_pre_wait{};
    ASSERT_EQ(writer.get_offered_deadline_missed_status(st_pre_wait), eprosima::fastdds::dds::RETCODE_OK);

    // Wait for a period long enough to expect a new miss if the timer were still active
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    eprosima::fastdds::dds::OfferedDeadlineMissedStatus st_post_wait{};
    ASSERT_EQ(writer.get_offered_deadline_missed_status(st_post_wait), eprosima::fastdds::dds::RETCODE_OK);

    EXPECT_EQ(st_pre_wait.total_count, st_post_wait.total_count) << "The total count should not change, as the timer was canceled.";

    EXPECT_EQ(st_post_wait.total_count_change, 0u) << "The total_count_change should be 0, as the timer was canceled.";

    auto prev = consumer_ptr->count();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(consumer_ptr->count(), prev) << "Timer should be canceled; no more warnings";

    fastlog::Log::ClearConsumers();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DeadlineQos,
        DeadlineQos,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DeadlineQos::ParamType>& info)
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
