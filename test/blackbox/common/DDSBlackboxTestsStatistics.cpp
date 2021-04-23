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

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

struct GenericType
{
    char data[256];
};

TEST(DDSStatistics, simple_statistics_datareader)
{
#ifdef FASTDDS_STATISTICS

    PubSubReader<HelloWorldType> data_reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> data_writer(TEST_TOPIC_NAME);

    data_reader.init();
    data_writer.init();

    auto participant = data_writer.getParticipant();
    ASSERT_NE(nullptr, participant);

    auto statistics_participant = statistics::dds::DomainParticipant::narrow(participant);
    ASSERT_NE(nullptr, statistics_participant);

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(
        statistics::DATA_COUNT_TOPIC, statistics::dds::STATISTICS_DATAWRITER_QOS));

    auto topic_desc = participant->lookup_topicdescription(statistics::DATA_COUNT_TOPIC);
    ASSERT_NE(nullptr, topic_desc);

    auto subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    auto stats_reader = subscriber->create_datareader(topic_desc, statistics::dds::STATISTICS_DATAREADER_QOS);
    ASSERT_NE(nullptr, stats_reader);

    data_reader.wait_discovery();
    data_writer.wait_discovery();

    auto data = default_helloworld_data_generator();
    auto num_samples = data.size();
    data_writer.send(data);
    EXPECT_TRUE(data.empty());
    EXPECT_TRUE(data_writer.waitForAllAcked(std::chrono::seconds(10)));

    LoanableSequence<GenericType> data_seq;
    SampleInfoSeq info_seq;

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, stats_reader->take(data_seq, info_seq));
    EXPECT_EQ(info_seq.length(), num_samples);

    subscriber->delete_datareader(stats_reader);
    participant->delete_subscriber(subscriber);

#endif // FASTDDS_STATISTICS

}
