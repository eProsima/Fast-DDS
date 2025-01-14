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
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include "../types/HelloWorld.hpp"
#include "../types/HelloWorldPubSubTypes.hpp"
#include "../types/statistics/types.hpp"
#include "../types/statistics/typesPubSubTypes.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#ifdef FASTDDS_STATISTICS

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

struct GenericType
{
    char data[256];
};

static DataReader* enable_statistics(
        statistics::dds::DomainParticipant* participant,
        Subscriber* subscriber,
        const std::string& topic_name)
{
    auto qos = statistics::dds::STATISTICS_DATAWRITER_QOS;
    qos.history().depth = 10;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, participant->enable_statistics_datawriter(
                topic_name, qos));

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
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, subscriber->delete_datareader(reader));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, participant->disable_statistics_datawriter(topic_name));
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

        if (eprosima::fastdds::dds::RETCODE_OK == reader->take(data_seq, info_seq))
        {
            total_samples += info_seq.length();
            reader->return_loan(data_seq, info_seq);
        }
    } while (total_samples < num_samples);

    std::cout << "Received " << total_samples << " samples on " << topic_name << std::endl;
}

enum class DiscoveryTopicPhysicalDataTest : uint8_t
{
    AUTO_PHYSICAL_DATA = 1,
    USER_DEFINED_PHYSICAL_DATA = 2,
    USER_DEFINED_PHYSICAL_DATA_XML = 3,
    NO_PHYSICAL_DATA = 4
};

void test_discovery_topic_physical_data(
        DiscoveryTopicPhysicalDataTest test_kind)
{
    auto domain_id = GET_PID() % 100;
    DomainParticipantFactory* participant_factory = DomainParticipantFactory::get_instance();

    /* Prepare DomainParticipantQos */
    std::string user_defined_host = "test_host";
    std::string user_defined_user = "test_user";
    std::string user_defined_process = "test_process";

    if (test_kind == DiscoveryTopicPhysicalDataTest::USER_DEFINED_PHYSICAL_DATA_XML)
    {
        std::string xml_profile =
                "\
            <?xml version=\"1.0\" encoding=\"utf-8\"?>\
            <dds xmlns=\"http://www.eprosima.com\">\
                <profiles>\
                    <participant profile_name=\"statistics_participant\" is_default_profile=\"true\">\
                        <rtps>\
                            <propertiesPolicy>\
                                <properties>\
                                    <property>\
                                        <name>" + std::string(parameter_policy_physical_data_host) +
                "</name>\
                                        <value>" + user_defined_host +
                "</value>\
                                    </property>\
                                    <property>\
                                        <name>" + std::string(parameter_policy_physical_data_user) +
                "</name>\
                                        <value>" + user_defined_user +
                "</value>\
                                    </property>\
                                    <property>\
                                        <name>" + std::string(parameter_policy_physical_data_process) +
                "</name>\
                                        <value>" + user_defined_process +
                "</value>\
                                    </property>\
                                </properties>\
                            </propertiesPolicy>\
                        </rtps>\
                    </participant>\
                </profiles>\
            </dds>\
            ";

        participant_factory->load_XML_profiles_string(xml_profile.c_str(), xml_profile.length());
        participant_factory->load_profiles();
    }

    DomainParticipantQos pqos = participant_factory->get_default_participant_qos();

    // Avoid discovery of participants external to the test
    pqos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
            static_cast<eprosima::fastdds::rtps::ParticipantFilteringFlags>(
        eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST |
        eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS);

    // Configure physical properties according to test case
    switch (test_kind)
    {
        case DiscoveryTopicPhysicalDataTest::USER_DEFINED_PHYSICAL_DATA:
        {
            pqos.properties().properties().clear();
            pqos.properties().properties().emplace_back(parameter_policy_physical_data_host, user_defined_host);
            pqos.properties().properties().emplace_back(parameter_policy_physical_data_user, user_defined_user);
            pqos.properties().properties().emplace_back(parameter_policy_physical_data_process, user_defined_process);
            break;
        }
        case DiscoveryTopicPhysicalDataTest::NO_PHYSICAL_DATA:
        {
            pqos.properties().properties().clear();
        }
        default:
        {
            break;
        }
    }

    /* Enable DISCOVERY_TOPIC DataWriter in the first DomainParticipant */
    DomainParticipant* p1 = participant_factory->create_participant(domain_id, pqos);
    ASSERT_NE(nullptr, p1);
    auto statistics_p1 = statistics::dds::DomainParticipant::narrow(p1);
    ASSERT_NE(nullptr, statistics_p1);
    Publisher* publisher_p1 = p1->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher_p1);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            statistics_p1->enable_statistics_datawriter(statistics::DISCOVERY_TOPIC,
            statistics::dds::STATISTICS_DATAWRITER_QOS));

    /* Create DISCOVERY_TOPIC DataReader in the second DomainParticipant */
    DomainParticipant* p2 = participant_factory->create_participant(domain_id, pqos);
    ASSERT_NE(nullptr, p2);
    TypeSupport discovery_type(new statistics::DiscoveryTimePubSubType);
    ASSERT_NE(nullptr, discovery_type);
    discovery_type.register_type(p2);
    Topic* topic = p2->create_topic(statistics::DISCOVERY_TOPIC,
                    discovery_type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);
    Subscriber* subscriber_p2 = p2->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber_p2);
    DataReader* discovery_data_reader = subscriber_p2->create_datareader(topic,
                    statistics::dds::STATISTICS_DATAREADER_QOS);
    EXPECT_NE(nullptr, discovery_data_reader);

    // Get the second participant's physical properties
    const std::string* p2_host = eprosima::fastdds::rtps::PropertyPolicyHelper::find_property(
        p2->get_qos().properties(), parameter_policy_physical_data_host);
    const std::string* p2_user = eprosima::fastdds::rtps::PropertyPolicyHelper::find_property(
        p2->get_qos().properties(), parameter_policy_physical_data_user);
    const std::string* p2_process = eprosima::fastdds::rtps::PropertyPolicyHelper::find_property(
        p2->get_qos().properties(), parameter_policy_physical_data_process);

    // Verify that the second participant's physical properties are set according to specification
    switch (test_kind)
    {
        case DiscoveryTopicPhysicalDataTest::AUTO_PHYSICAL_DATA:
        {
            EXPECT_NE("", *p2_host);
            EXPECT_NE("", *p2_user);
            EXPECT_NE("", *p2_process);
            break;
        }
        case DiscoveryTopicPhysicalDataTest::USER_DEFINED_PHYSICAL_DATA:
        case DiscoveryTopicPhysicalDataTest::USER_DEFINED_PHYSICAL_DATA_XML:
        {
            ASSERT_NE(nullptr, p2_host);
            ASSERT_NE(nullptr, p2_user);
            ASSERT_NE(nullptr, p2_process);
            EXPECT_EQ(user_defined_host, *p2_host);
            EXPECT_EQ(user_defined_user, *p2_user);
            EXPECT_EQ(user_defined_process, *p2_process);
            break;
        }
        case DiscoveryTopicPhysicalDataTest::NO_PHYSICAL_DATA:
        {
            EXPECT_EQ(nullptr, p2_host);
            EXPECT_EQ(nullptr, p2_user);
            EXPECT_EQ(nullptr, p2_process);
            break;
        }
        default:
        {
            FAIL() << "Test kind not supported";
            break;
        }
    }

    /* Create a DataWriter in the second participant */
    TypeSupport helloworld_type(new HelloWorldPubSubType());
    ASSERT_NE(nullptr, helloworld_type);
    helloworld_type.register_type(p2);
    Topic* helloworld_topic = p2->create_topic("helloworld",
                    helloworld_type.get_type_name(), TOPIC_QOS_DEFAULT);
    Publisher* publisher_p2 = p2->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher_p2);
    DataWriter* datawriter_p2 = publisher_p2->create_datawriter(helloworld_topic, DATAWRITER_QOS_DEFAULT);

    /* Create waitset for the DataReader */
    WaitSet waitset;
    StatusCondition& condition = discovery_data_reader->get_statuscondition();
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, condition.set_enabled_statuses(StatusMask::data_available()));
    ASSERT_EQ(false, condition.get_trigger_value());
    waitset.attach_condition(condition);

    ConditionSeq triggered_conditions;
    waitset.wait(triggered_conditions, eprosima::fastdds::dds::c_TimeInfinite);

    auto to_guid_prefix = [](const statistics::detail::GuidPrefix_s& prefix)
            {
                eprosima::fastdds::rtps::GuidPrefix_t guid_prefix;
                for (size_t i = 0; i < prefix.value().size(); i++)
                {
                    guid_prefix.value[i] = prefix.value()[i];
                }
                return guid_prefix;
            };

    auto to_entity_id = [](const statistics::detail::EntityId_s& id)
            {
                eprosima::fastdds::rtps::EntityId_t entity_id;
                for (size_t i = 0; i < id.value().size(); i++)
                {
                    entity_id.value[i] = id.value()[i];
                }
                return entity_id;
            };

    LoanableSequence<statistics::DiscoveryTime> discovery_time_seq;
    SampleInfoSeq info_seq;

    while (eprosima::fastdds::dds::RETCODE_OK == discovery_data_reader->take(discovery_time_seq, info_seq))
    {
        for (LoanableSequence<statistics::DiscoveryTime>::size_type n = 0; n < info_seq.length(); n++)
        {
            if (info_seq[n].valid_data)
            {
                /* Get discovery information from the sample */
                eprosima::fastdds::rtps::GuidPrefix_t local_prefix = to_guid_prefix(
                    discovery_time_seq[n].local_participant_guid().guidPrefix());
                eprosima::fastdds::rtps::GuidPrefix_t remote_prefix = to_guid_prefix(
                    discovery_time_seq[n].remote_entity_guid().guidPrefix());
                eprosima::fastdds::rtps::EntityId_t remote_entity_id = to_entity_id(
                    discovery_time_seq[n].remote_entity_guid().entityId());

                /* Validate discovery sample */
                EXPECT_EQ(statistics_p1->guid().guidPrefix, local_prefix);
                EXPECT_EQ(p2->guid().guidPrefix, remote_prefix);

                // Check expectations depending of the test case
                switch (test_kind)
                {
                    case DiscoveryTopicPhysicalDataTest::NO_PHYSICAL_DATA:
                    {
                        EXPECT_EQ(discovery_time_seq[n].host(), "");
                        EXPECT_EQ(discovery_time_seq[n].user(), "");
                        EXPECT_EQ(discovery_time_seq[n].process(), "");
                        break;
                    }
                    default:
                    {
                        // If the remote entity is a participant, then host, user, and process should have the values
                        // of that participant
                        if (remote_entity_id == ENTITYID_RTPSParticipant)
                        {
                            EXPECT_EQ(*p2_host, discovery_time_seq[n].host());
                            EXPECT_EQ(*p2_user, discovery_time_seq[n].user());
                            EXPECT_EQ(*p2_process, discovery_time_seq[n].process());
                        }
                        // If the remote entity is not a participant, then host, user, and process should be empty
                        else
                        {
                            EXPECT_TRUE(
                                remote_entity_id == discovery_data_reader->guid().entityId ||
                                remote_entity_id == datawriter_p2->guid().entityId);
                            EXPECT_EQ(discovery_time_seq[n].host(), "");
                            EXPECT_EQ(discovery_time_seq[n].user(), "");
                            EXPECT_EQ(discovery_time_seq[n].process(), "");
                        }
                        break;
                    }
                }
            }
        }
        discovery_data_reader->return_loan(discovery_time_seq, info_seq);
    }

    /* Delete first DomainParticipant */
    statistics_p1->disable_statistics_datawriter(statistics::DISCOVERY_TOPIC);
    statistics_p1->delete_contained_entities();
    participant_factory->delete_participant(p1);

    /* Delete second DomainParticipant */
    p2->delete_contained_entities();
    participant_factory->delete_participant(p2);
}

#endif // FASTDDS_STATISTICS

TEST(DDSStatistics, simple_statistics_datareaders)
{
#ifdef FASTDDS_STATISTICS

    auto transport = std::make_shared<UDPv4TransportDescriptor>();

    PubSubReader<HelloWorldPubSubType> data_reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> data_writer(TEST_TOPIC_NAME);

    data_reader.disable_builtin_transport().add_user_transport_to_pparams(transport);
    data_writer.disable_builtin_transport().add_user_transport_to_pparams(transport);

    auto data = default_helloworld_data_generator();
    auto num_samples = data.size();
    auto depth = static_cast<int32_t>(num_samples);

    // Reader should be reliable so ACKNACK messages are generated (and accounted)
    data_reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).history_depth(depth).init();
    // Enforce synchronous writer to force RTPS_SENT to have at least num_samples
    data_writer.asynchronously(eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE).history_depth(depth).init();

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
        {"HEARTBEAT_COUNT_TOPIC",           statistics::HEARTBEAT_COUNT_TOPIC,          1},
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

// Regression test for #12390. Mostly a replication of test simple_statistics_second_writer but creating
// and additional publisher with partitions before the creation of the builtin publisher
TEST(DDSStatistics, statistics_with_partition_on_user)
{
#ifdef FASTDDS_STATISTICS
    auto domain_id = GET_PID() % 100;

    DomainParticipantQos p_qos = PARTICIPANT_QOS_DEFAULT;
    DomainParticipantFactory* participant_factory = DomainParticipantFactory::get_instance();

    // We disable the auto-enabling so the builtin entities do not get created.
    DomainParticipantFactoryQos factory_qos;
    participant_factory->get_qos(factory_qos);
    factory_qos.entity_factory().autoenable_created_entities = false;
    participant_factory->set_qos(factory_qos);

    DomainParticipant* p1 = participant_factory->create_participant(domain_id, p_qos);
    DomainParticipant* p2 = participant_factory->create_participant(domain_id, p_qos);

    ASSERT_NE(nullptr, p1);
    ASSERT_NE(nullptr, p2);

    // Now we create a Publisher with a partition
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    pub_qos.partition().push_back("partition_a");
    auto user_pub_1 = p1->create_publisher(pub_qos);

    // We enable the participants
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, p1->enable());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, p2->enable());

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
    disable_statistics(statistics_p2, subscriber_p2, physical_data_reader_2, statistics::PHYSICAL_DATA_TOPIC);

    p1->delete_publisher(user_pub_1);
    p2->delete_subscriber(subscriber_p2);
    p1->delete_subscriber(subscriber_p1);

    participant_factory->delete_participant(p2);
    participant_factory->delete_participant(p1);

#endif // FASTDDS_STATISTICS
}

/*
 * The following tests check that the DISCOVERY_TOPIC carries the correct physical data, i.e.:
 *
 * 1. When FASTDDS_STATISTICS is defined and the user does not configure anything, the DISCOVERY_TOPIC carries
 *    physical information for participants, and nothing for writers and readers.
 * 2. When FASTDDS_STATISTICS is defined and the user does configure the physical properties using API, the
 *    DISCOVERY_TOPIC carries physical information for participants, and nothing for writers and readers.
 * 3. When FASTDDS_STATISTICS is defined and the user does configure the physical properties using XML, the
 *    DISCOVERY_TOPIC carries physical information for participants, and nothing for writers and readers.
 * 4. When FASTDDS_STATISTICS is defined and the user removes the physical properties, the DISCOVERY_TOPIC
 *    carries no physical information for participants, writers, or readers.
 */
TEST(DDSStatistics, discovery_topic_physical_data_auto)
{
#ifdef FASTDDS_STATISTICS
    test_discovery_topic_physical_data(DiscoveryTopicPhysicalDataTest::AUTO_PHYSICAL_DATA);
#endif // FASTDDS_STATISTICS
}

TEST(DDSStatistics, discovery_topic_physical_data_user_defined)
{
#ifdef FASTDDS_STATISTICS
    test_discovery_topic_physical_data(DiscoveryTopicPhysicalDataTest::USER_DEFINED_PHYSICAL_DATA);
#endif // FASTDDS_STATISTICS
}

TEST(DDSStatistics, discovery_topic_physical_data_user_defined_xml)
{
#ifdef FASTDDS_STATISTICS
    test_discovery_topic_physical_data(DiscoveryTopicPhysicalDataTest::USER_DEFINED_PHYSICAL_DATA_XML);
#endif // FASTDDS_STATISTICS
}

TEST(DDSStatistics, discovery_topic_physical_data_delete_physical_properties)
{
#ifdef FASTDDS_STATISTICS
    test_discovery_topic_physical_data(DiscoveryTopicPhysicalDataTest::NO_PHYSICAL_DATA);
#endif // FASTDDS_STATISTICS
}

class CustomStatisticsParticipantSubscriber : public PubSubReader<HelloWorldPubSubType>
{
public:

    CustomStatisticsParticipantSubscriber(
            const std::string& topic_name)
        : PubSubReader<HelloWorldPubSubType>(topic_name)
    {
    }

    void destroy() override
    {
        participant_->delete_contained_entities();
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }

};

// Regression test for #20816. When an application is terminated with delete_contained_entities()
// it has to properly finish. The test creates a number of participants with some of them sharing the same topic.
// Each participant asynchronously sends and receive a number of samples. In the readers, when a minumm number of samples
// is received the destroy() method is called (abruptly). The test checks that the application finishes successfully
TEST(DDSStatistics, correct_deletion_upon_delete_contained_entities)
{
#ifdef FASTDDS_STATISTICS

    //! Set environment variable and create participant using Qos set by code
    const char* value = "HISTORY_LATENCY_TOPIC;NETWORK_LATENCY_TOPIC;"
            "PUBLICATION_THROUGHPUT_TOPIC;SUBSCRIPTION_THROUGHPUT_TOPIC;RTPS_SENT_TOPIC;"
            "RTPS_LOST_TOPIC;HEARTBEAT_COUNT_TOPIC;ACKNACK_COUNT_TOPIC;NACKFRAG_COUNT_TOPIC;"
            "GAP_COUNT_TOPIC;DATA_COUNT_TOPIC;RESENT_DATAS_TOPIC;SAMPLE_DATAS_TOPIC;"
            "PDP_PACKETS_TOPIC;EDP_PACKETS_TOPIC;DISCOVERY_TOPIC;PHYSICAL_DATA_TOPIC;";

    #ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("FASTDDS_STATISTICS", value));
    #else
    ASSERT_EQ(0, setenv("FASTDDS_STATISTICS", value, 1));
    #endif // ifdef _WIN32

    size_t n_participants = 5;
    size_t n_participants_same_topic = 2;

    std::vector<std::shared_ptr<PubSubWriter<HelloWorldPubSubType>>> writers;
    std::vector<std::shared_ptr<CustomStatisticsParticipantSubscriber>> readers;

    readers.reserve(n_participants);
    writers.reserve(n_participants);

    std::vector<std::shared_ptr<std::thread>> threads;
    threads.reserve(2 * n_participants);

    for (size_t i = 0; i < n_participants; ++i)
    {
        size_t topic_number = (i < n_participants_same_topic) ? 0 : i;

        auto writer = std::make_shared<PubSubWriter<HelloWorldPubSubType>>(TEST_TOPIC_NAME + std::to_string(
                            topic_number));
        auto reader =
                std::make_shared<CustomStatisticsParticipantSubscriber>(TEST_TOPIC_NAME + std::to_string(topic_number));

        std::shared_ptr<std::list<HelloWorld>> data = std::make_shared<std::list<HelloWorld>>(default_helloworld_data_generator(
                            10));

        threads.emplace_back(std::make_shared<std::thread>([reader, data]()
                {
                    reader->init();
                    ASSERT_TRUE(reader->isInitialized());
                    reader->startReception(data->size());
                    reader->block_for_at_least(3);
                    reader->destroy();
                }));

        threads.emplace_back(std::make_shared<std::thread>([writer, data]()
                {
                    writer->init();
                    ASSERT_TRUE(writer->isInitialized());
                    writer->wait_discovery();
                    writer->send(*data, 200);
                    writer->destroy();
                }));

        writers.push_back(writer);
        readers.push_back(reader);
    }

    for (auto& thread : threads)
    {
        thread->join();
    }

#endif // FASTDDS_STATISTICS
}
