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

#include <chrono>
#include <list>
#include <string>
#include <thread>
#include <tuple>

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"

#include "../types/HelloWorld.h"
#include "../types/HelloWorldType.h"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastdds/dds/core/LoanableSequence.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

#include <fastdds/statistics/topic_names.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>

#include <fastrtps/types/TypesBase.h>

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

    auto transport = std::make_shared<UDPv4TransportDescriptor>();

    PubSubReader<HelloWorldType> data_reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> data_writer(TEST_TOPIC_NAME);

    data_reader.disable_builtin_transport().add_user_transport_to_pparams(transport);
    data_writer.disable_builtin_transport().add_user_transport_to_pparams(transport);

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

    // Get Participants and Subscribers from pub and sub
    auto w_participant = data_writer.getParticipant();
    ASSERT_NE(nullptr, w_participant);

    auto w_statistics_participant = statistics::dds::DomainParticipant::narrow(w_participant);
    ASSERT_NE(nullptr, w_statistics_participant);

    auto w_subscriber = w_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, w_subscriber);

    auto r_subscriber = const_cast<Subscriber*>(data_reader.get_native_reader().get_subscriber());
    ASSERT_NE(nullptr, r_subscriber);

    auto r_participant = const_cast<DomainParticipant*>(r_subscriber->get_participant());
    ASSERT_NE(nullptr, r_participant);

    auto r_statistics_participant = statistics::dds::DomainParticipant::narrow(r_participant);
    ASSERT_NE(nullptr, r_statistics_participant);

    // TODO: some topics get stuck in infinite loop in an error (generally if they are included twice):
    // [SUBSCRIBER Error] Change not found on this key, something is wrong -> Function remove_change_sub
    // These topics are commented in test params
    // TODO: some topics could be used in both participants, but they lead to the same error

    // Create parameters to iterate over every Statistics kind
    // The test is separated between the statistics retrieved by a DataWriter or a DataReader
    std::vector<std::tuple<std::string, std::string, std::size_t>> writer_statistics_kinds = {
        {"DATA_COUNT_TOPIC",                statistics::DATA_COUNT_TOPIC,               num_samples},
        {"RTPS_SENT_TOPIC",                 statistics::RTPS_SENT_TOPIC,                num_samples},
        {"NETWORK_LATENCY_TOPIC",           statistics::NETWORK_LATENCY_TOPIC,          num_samples},
        {"PUBLICATION_THROUGHPUT_TOPIC",    statistics::PUBLICATION_THROUGHPUT_TOPIC,   num_samples},
        {"HEARTBEAT_COUNT_TOPIC",           statistics::HEARTBEAT_COUNT_TOPIC,          num_samples},
        {"SAMPLE_DATAS_TOPIC",              statistics::SAMPLE_DATAS_TOPIC,             num_samples},
        {"DISCOVERY_TOPIC",                 statistics::DISCOVERY_TOPIC,                1},
        {"PDP_PACKETS_TOPIC",               statistics::PDP_PACKETS_TOPIC,              1},
        {"EDP_PACKETS_TOPIC",               statistics::EDP_PACKETS_TOPIC,              1},
        {"PHYSICAL_DATA_TOPIC",             statistics::PHYSICAL_DATA_TOPIC,            1}
    };

    std::vector<std::tuple<std::string, std::string, std::size_t>> reader_statistics_kinds = {
        {"HISTORY_LATENCY_TOPIC",           statistics::HISTORY_LATENCY_TOPIC,          num_samples},
        {"SUBSCRIPTION_THROUGHPUT_TOPIC",   statistics::SUBSCRIPTION_THROUGHPUT_TOPIC,  num_samples},
        {"ACKNACK_COUNT_TOPIC",             statistics::ACKNACK_COUNT_TOPIC,            1},
        // {"PHYSICAL_DATA_TOPIC",             statistics::PHYSICAL_DATA_TOPIC,            1}
    };

    std::vector<DataReader*> readers_datawriter;
    std::vector<DataReader*> readers_datareader;

    // Enable Statistics Readers
    for (auto kind : writer_statistics_kinds)
    {
        auto new_reader = enable_statistics(w_statistics_participant, w_subscriber, std::get<1>(kind));
        ASSERT_NE(nullptr, new_reader);
        readers_datawriter.push_back(new_reader);
    }

    for (auto kind : reader_statistics_kinds)
    {
        auto new_reader = enable_statistics(r_statistics_participant, r_subscriber, std::get<1>(kind));
        ASSERT_NE(nullptr, new_reader);
        readers_datareader.push_back(new_reader);
    }

    // Perform communication
    data_reader.startReception(data);
    data_writer.send(data);
    EXPECT_TRUE(data.empty());
    data_reader.block_for_all();
    EXPECT_TRUE(data_writer.waitForAllAcked(std::chrono::seconds(10)));

    // Check that messages have been received
    for (std::size_t i = 0; i < readers_datawriter.size(); ++i)
    {
        wait_statistics(
            readers_datawriter[i],
            std::get<2>(writer_statistics_kinds[i]),
            std::get<0>(writer_statistics_kinds[i]).c_str(),
            10u);
        disable_statistics(
            w_statistics_participant,
            w_subscriber,
            readers_datawriter[i],
            std::get<1>(writer_statistics_kinds[i]));
    }

    for (std::size_t i = 0; i < readers_datareader.size(); ++i)
    {
        wait_statistics(
            readers_datareader[i],
            std::get<2>(reader_statistics_kinds[i]),
            std::get<0>(reader_statistics_kinds[i]).c_str(),
            10u);
        disable_statistics(
            r_statistics_participant,
            r_subscriber,
            readers_datareader[i],
            std::get<1>(reader_statistics_kinds[i]));
    }

    w_participant->delete_subscriber(w_subscriber);
    w_participant->delete_subscriber(r_subscriber);

#endif // FASTDDS_STATISTICS
}

TEST(DDSStatistics, simple_statistics_second_writer)
{
#ifdef FASTDDS_STATISTICS

    auto transport = std::make_shared<UDPv4TransportDescriptor>();
    auto domain_id = GET_PID() % 100;

    DomainParticipantQos p_qos = PARTICIPANT_QOS_DEFAULT;
    p_qos.transport().use_builtin_transports = false;
    p_qos.transport().user_transports.push_back(transport);

    auto participant_factory = DomainParticipantFactory::get_instance();
    DomainParticipant* p1 = participant_factory->create_participant(domain_id, p_qos);
    DomainParticipant* p2 = participant_factory->create_participant(domain_id, p_qos);

    ASSERT_NE(nullptr, p1);
    ASSERT_NE(nullptr, p2);

    auto statistics_p1 = statistics::dds::DomainParticipant::narrow(p1);
    auto statistics_p2 = statistics::dds::DomainParticipant::narrow(p2);
    ASSERT_NE(nullptr, statistics_p1);
    ASSERT_NE(nullptr, statistics_p2);

    auto subscriber_p1 = p1->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    auto subscriber_p2 = p2->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber_p1);
    ASSERT_NE(nullptr, subscriber_p2);

    auto physical_data_reader_1 = enable_statistics(statistics_p1, subscriber_p1, statistics::PHYSICAL_DATA_TOPIC);
    auto physical_data_reader_2 = enable_statistics(statistics_p2, subscriber_p2, statistics::PHYSICAL_DATA_TOPIC);
    ASSERT_NE(nullptr, physical_data_reader_1);
    ASSERT_NE(nullptr, physical_data_reader_2);

    wait_statistics(physical_data_reader_1, 2, "PHYSICAL_DATA_TOPIC", 10u);
    wait_statistics(physical_data_reader_2, 2, "PHYSICAL_DATA_TOPIC", 10u);

    disable_statistics(statistics_p1, subscriber_p1, physical_data_reader_1, statistics::PHYSICAL_DATA_TOPIC);
    physical_data_reader_1 = enable_statistics(statistics_p1, subscriber_p1, statistics::PHYSICAL_DATA_TOPIC);

    wait_statistics(physical_data_reader_1, 2, "PHYSICAL_DATA_TOPIC", 10u);
    wait_statistics(physical_data_reader_2, 1, "PHYSICAL_DATA_TOPIC", 10u);

    disable_statistics(statistics_p1, subscriber_p1, physical_data_reader_1, statistics::PHYSICAL_DATA_TOPIC);
    disable_statistics(statistics_p2, subscriber_p2, physical_data_reader_2, statistics::PHYSICAL_DATA_TOPIC);

    p2->delete_subscriber(subscriber_p2);
    p1->delete_subscriber(subscriber_p1);

    participant_factory->delete_participant(p2);
    participant_factory->delete_participant(p1);

#endif // FASTDDS_STATISTICS
}
