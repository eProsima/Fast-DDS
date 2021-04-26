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

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastdds/dds/core/LoanableCollection.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>

#include <fastdds/statistics/topic_names.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>

#ifdef FASTDDS_STATISTICS

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::types;

struct GenericType
{
    char data[256];
};

static DataReader* enable_statistics(
        statistics::dds::DomainParticipant* participant,
        Subscriber* subscriber,
        const std::string& topic_name)
{
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable_statistics_datawriter(
                topic_name, statistics::dds::STATISTICS_DATAWRITER_QOS));

    auto topic_desc = participant->lookup_topicdescription(topic_name);
    EXPECT_NE(nullptr, topic_desc);

    return subscriber->create_datareader(topic_desc, statistics::dds::STATISTICS_DATAREADER_QOS);
}

static void disable_statistics(
        statistics::dds::DomainParticipant* participant,
        Subscriber* subscriber,
        DataReader* reader,
        const std::string& topic_name)
{
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, subscriber->delete_datareader(reader));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->disable_statistics_datawriter(topic_name));
}

static void wait_statistics(
        DataReader* reader,
        size_t num_samples,
        const char* topic_name,
        size_t num_seconds)
{
    std::cout << "Waiting for " << num_samples << " samples on " << topic_name << std::endl;

    uint32_t total_samples = 0;
    do
    {
        EXPECT_LT(0u, num_seconds);
        if (num_seconds == 0)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        --num_seconds;

        LoanableSequence<GenericType> data_seq;
        SampleInfoSeq info_seq;

        if (ReturnCode_t::RETCODE_OK == reader->take(data_seq, info_seq))
        {
            total_samples += info_seq.length();
            reader->return_loan(data_seq, info_seq);
        }
    } while (total_samples < num_samples);

    std::cout << "Received " << total_samples << " samples on " << topic_name << std::endl;
}

#endif // FASTDDS_STATISTICS

TEST(DDSStatistics, simple_statistics_datareaders)
{
#ifdef FASTDDS_STATISTICS

    PubSubReader<HelloWorldType> data_reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> data_writer(TEST_TOPIC_NAME);

    auto data = default_helloworld_data_generator();
    auto num_samples = data.size();
    auto depth = static_cast<int32_t>(num_samples);

    // Reader should be reliable so ACKNACK messages are generated (and accounted)
    data_reader.reliability(RELIABLE_RELIABILITY_QOS).history_depth(depth).init();
    // Enforce synchronous writer to force RTPS_SENT to have at least num_samples
    data_writer.asynchronously(SYNCHRONOUS_PUBLISH_MODE).history_depth(depth).init();

    // Ensure discovery traffic is not included on statistics
    data_reader.wait_discovery();
    data_writer.wait_discovery();

    auto w_participant = data_writer.getParticipant();
    ASSERT_NE(nullptr, w_participant);

    auto w_statistics_participant = statistics::dds::DomainParticipant::narrow(w_participant);
    ASSERT_NE(nullptr, w_statistics_participant);

    auto w_subscriber = w_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, w_subscriber);

    // Enable DomainParticipant related statistics
    auto rtps_stats_reader = enable_statistics(w_statistics_participant, w_subscriber, statistics::RTPS_SENT_TOPIC);
    ASSERT_NE(nullptr, rtps_stats_reader);

    // Enable DataWriter related statistics
    auto data_stats_reader = enable_statistics(w_statistics_participant, w_subscriber, statistics::DATA_COUNT_TOPIC);
    ASSERT_NE(nullptr, data_stats_reader);

    auto r_subscriber = const_cast<Subscriber*>(data_reader.get_native_reader().get_subscriber());
    ASSERT_NE(nullptr, r_subscriber);

    auto r_participant = const_cast<DomainParticipant*>(r_subscriber->get_participant());
    ASSERT_NE(nullptr, r_participant);

    auto r_statistics_participant = statistics::dds::DomainParticipant::narrow(r_participant);
    ASSERT_NE(nullptr, r_statistics_participant);

    // Enable DataReader related statistics
    auto ack_stats_reader = enable_statistics(r_statistics_participant, r_subscriber, statistics::ACKNACK_COUNT_TOPIC);
    ASSERT_NE(nullptr, ack_stats_reader);

    // Perform communication
    data_reader.startReception(data);
    data_writer.send(data);
    EXPECT_TRUE(data.empty());
    data_reader.block_for_all();
    EXPECT_TRUE(data_writer.waitForAllAcked(std::chrono::seconds(10)));

    wait_statistics(data_stats_reader, num_samples, "DATA_COUNT_TOPIC", 10u);
    wait_statistics(ack_stats_reader, 1, "ACKNACK_COUNT_TOPIC", 10u);
    wait_statistics(rtps_stats_reader, num_samples, "RTPS_SENT_TOPIC", 10u);

    disable_statistics(r_statistics_participant, r_subscriber, ack_stats_reader, statistics::ACKNACK_COUNT_TOPIC);
    disable_statistics(w_statistics_participant, w_subscriber, rtps_stats_reader, statistics::RTPS_SENT_TOPIC);
    disable_statistics(w_statistics_participant, w_subscriber, data_stats_reader, statistics::DATA_COUNT_TOPIC);

    w_participant->delete_subscriber(w_subscriber);

#endif // FASTDDS_STATISTICS
}
