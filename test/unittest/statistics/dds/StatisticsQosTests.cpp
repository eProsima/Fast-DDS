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

#include <string>

#include <gtest/gtest.h>
#include <tinyxml2.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerConsts.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/statistics/topic_names.hpp>

#ifdef FASTDDS_STATISTICS
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#endif // ifdef FASTDDS_STATISTICS

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class StatisticsFromXMLProfileTests : public ::testing::Test
{
public:

#ifdef FASTDDS_STATISTICS
    class TestDomainParticipant : public eprosima::fastdds::statistics::dds::DomainParticipant
    {
    public:

        DomainParticipantImpl* get_domain_participant_impl()
        {
            return static_cast<DomainParticipantImpl*>(impl_);
        }

    };

    class TestDomainParticipantImpl : public eprosima::fastdds::statistics::dds::DomainParticipantImpl
    {
    public:

        efd::Publisher*  get_publisher()
        {
            return builtin_publisher_;
        }

    };

    eprosima::fastdds::statistics::dds::DomainParticipant* statistics_participant = nullptr;
    eprosima::fastdds::dds::Publisher* statistics_publisher = nullptr;

    void test_setup_XMLConfigurationForStatisticsDataWritersQoS(
            eprosima::fastdds::statistics::dds::DomainParticipant*& _statistics_participant,
            eprosima::fastdds::dds::Publisher*& statistics_pub,
            const std::string& xml)
    {

        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_string(xml.c_str(),
                xml.size());

        // Create DomainParticipant. Builtin statistics DataWriters are also created, as auto-enable option is true by default.
        eprosima::fastdds::dds::DomainParticipant* participant =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                        create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
        ASSERT_NE(participant, nullptr);

        // Obtain pointer to child class
        _statistics_participant =
                eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
        ASSERT_NE(_statistics_participant, nullptr);

        // Static conversion to child class TestDomainParticipant
        TestDomainParticipant* test_statistics_participant =
                static_cast<TestDomainParticipant*>(_statistics_participant);
        ASSERT_NE(test_statistics_participant, nullptr);

        // Get DomainParticipantImpl
        eprosima::fastdds::statistics::dds::DomainParticipantImpl* domain_statistics_participant_impl =
                test_statistics_participant->get_domain_participant_impl();
        ASSERT_NE(domain_statistics_participant_impl, nullptr);

        // Static conversion to child class TestDomainParticipantImpl
        TestDomainParticipantImpl* test_statistics_domain_participant_impl =
                static_cast<TestDomainParticipantImpl*>(domain_statistics_participant_impl);
        ASSERT_NE(test_statistics_domain_participant_impl, nullptr);

        // Get Publisher
        statistics_pub =
                test_statistics_domain_participant_impl->get_publisher();
        ASSERT_NE(statistics_pub, nullptr);
    }

#endif // ifdef FASTDDS_STATISTICS
};

/*
 * This test checks that STATISTICS_DATAWRITER_QOS correctly sets the expected QoS.
 * 1. Reliability RELIABLE
 * 2. Durability TRANSIENT LOCAL
 * 3. Pull mode enabled
 * 4. Publication mode ASYNCHRONOUS with custom flow controller
 * 5. History kind KEEP LAST
 * 6. History depth 10
 */
TEST(StatisticsQosTests, StatisticsDataWriterQosTest)
{
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.reliability().kind, eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.durability().kind, eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    const std::string* pushMode_property = eprosima::fastdds::rtps::PropertyPolicyHelper::find_property(
        STATISTICS_DATAWRITER_QOS.properties(), "fastdds.push_mode");
    ASSERT_NE(pushMode_property, nullptr);
    EXPECT_EQ(pushMode_property->compare("false"), 0);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.publish_mode().kind, eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.publish_mode().flow_controller_name,
            eprosima::fastdds::rtps::FASTDDS_STATISTICS_FLOW_CONTROLLER_DEFAULT);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.history().kind, eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.history().depth, 10);
}

/*
 * This test checks that STATISTICS_DATAREADER_QOS correctly sets the expected QoS.
 * 1. Reliability RELIABLE
 * 2. Durability TRANSIENT LOCAL
 * 3. History kind KEEP LAST
 * 4. History depth 100
 */
TEST(StatisticsQosTests, StatisticsDataReaderQosTest)
{
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.reliability().kind == eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.durability().kind == eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().kind == eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().depth == 100);
}

/**
 * This test checks the configuration of datawriter statistics QoS from XML file.
 * First an XML file with the appropiate configuration is created.
 * With DomainParticipant creation, some datawriters are enabled in a built-in manner.
 * Then, DataWriters creation is verified, as well as the effective configuration of their QoS.
 * Finally, the `enable_statistics_datawriter_with_profile()` public method is verified,
 * which enables datawriters that are not enabled in a built-in manner.
 */
TEST_F(StatisticsFromXMLProfileTests, XMLConfigurationForStatisticsDataWritersQoS)
{
#ifdef FASTDDS_STATISTICS

    const std::string xml =
            "                                                                                                                 \
            <?xml version=\"1.0\" encoding=\"utf-8\"  ?>                                                                      \
                <dds xmlns=\"http://www.eprosima.com\">                                           \
                    <profiles>                                                                                                \
                        <participant profile_name=\"statistics_participant\" is_default_profile=\"true\">                     \
                            <rtps>                                                                                            \
                                <propertiesPolicy>                                                                            \
                                    <properties>                                                                              \
                                        <property>                                                                            \
                                            <name>fastdds.statistics</name>                                                   \
                                            <value>HISTORY_LATENCY_TOPIC;PUBLICATION_THROUGHPUT_TOPIC;DATA_COUNT_TOPIC</value>\
                                        </property>                                                                           \
                                    </properties>                                                                             \
                                </propertiesPolicy>                                                                           \
                            </rtps>                                                                                           \
                        </participant>                                                                                        \
                        <data_writer profile_name=\"HISTORY_LATENCY_TOPIC\">                                                  \
                            <qos>                                                                                             \
                                <reliability>                                                                                 \
                                    <kind>BEST_EFFORT</kind>                                                                  \
                                </reliability>                                                                                \
                                <durability>                                                                                  \
                                    <kind>VOLATILE</kind>                                                                     \
                                </durability>                                                                                 \
                                <publishMode>                                                                                 \
                                    <kind>SYNCHRONOUS</kind>                                                                  \
                                </publishMode>                                                                                \
                            </qos>                                                                                            \
                        </data_writer>                                                                                        \
                        <data_writer profile_name=\"NETWORK_LATENCY_TOPIC\">                                                  \
                        </data_writer>                                                                                        \
                        <data_writer profile_name=\"SUBSCRIPTION_THROUGHPUT_TOPIC\">                                          \
                            <qos>                                                                                             \
                                <reliability>                                                                                 \
                                    <kind>BEST_EFFORT</kind>                                                                  \
                                </reliability>                                                                                \
                                <partition>                                                                                   \
                                    <names>                                                                                   \
                                        <name>part1</name>                                                                    \
                                        <name>part2</name>                                                                    \
                                    </names>                                                                                  \
                                </partition>                                                                                  \
                                <deadline>                                                                                    \
                                    <period>                                                                                  \
                                        <sec>3</sec>                                                                          \
                                    </period>                                                                                 \
                                </deadline>                                                                                   \
                                <latencyBudget>                                                                               \
                                    <duration>                                                                                \
                                        <sec>2</sec>                                                                          \
                                    </duration>                                                                               \
                                </latencyBudget>                                                                              \
                                <disable_heartbeat_piggyback>true</disable_heartbeat_piggyback>                               \
                            </qos>                                                                                            \
                        </data_writer>                                                                                        \
                        <data_writer profile_name=\"DATA_COUNT_TOPIC\">                                                       \
                            <qos>                                                                                             \
                                <liveliness>                                                                                  \
                                    <kind>AUTOMATIC</kind>                                                                    \
                                    <lease_duration>                                                                          \
                                        <sec>1</sec>                                                                          \
                                        <nanosec>856000</nanosec>                                                             \
                                    </lease_duration>                                                                         \
                                    <announcement_period>                                                                     \
                                        <sec>1</sec>                                                                          \
                                        <nanosec>856000</nanosec>                                                             \
                                    </announcement_period>                                                                    \
                                </liveliness>                                                                                 \
                            </qos>                                                                                            \
                        </data_writer>                                                                                        \
                        <data_writer profile_name=\"HEARTBEAT_COUNT_TOPIC\">                                                  \
                            <qos>                                                                                             \
                                <liveliness>                                                                                  \
                                    <kind>AUTOMATIC</kind>                                                                    \
                                    <lease_duration>                                                                          \
                                        <sec>1</sec>                                                                          \
                                        <nanosec>856000</nanosec>                                                             \
                                    </lease_duration>                                                                         \
                                    <announcement_period>                                                                     \
                                        <sec>1</sec>                                                                          \
                                        <nanosec>856000</nanosec>                                                             \
                                    </announcement_period>                                                                    \
                                </liveliness>                                                                                 \
                            </qos>                                                                                            \
                        </data_writer>                                                                                        \
                        <data_writer profile_name=\"OTHER_NAME_FOR_PROFILE\">                                                 \
                            <qos>                                                                                             \
                                <reliability>                                                                                 \
                                    <kind>BEST_EFFORT</kind>                                                                  \
                                </reliability>                                                                                \
                            </qos>                                                                                            \
                        </data_writer>                                                                                        \
                    </profiles>                                                                                               \
                </dds>                                                                                                        \
            ";

    test_setup_XMLConfigurationForStatisticsDataWritersQoS(
        statistics_participant,
        statistics_publisher,
        xml);

    // Get and check built-in DataWriters:

    // HISTORY_LATENCY_TOPIC has a QoS profile defined in the XML file,
    // and it has been created automatically within the participant
    // (because it has been included in the corresponding property)
    std::string history_latency_name = HISTORY_LATENCY_TOPIC;
    eprosima::fastdds::dds::DataWriter* history_latency_writer =
            statistics_publisher->lookup_datawriter(history_latency_name);
    ASSERT_NE(history_latency_writer, nullptr);

    // By default, when QoS are set in XML for an statistics DataWriter profile,
    // Fast DDS default QoS are used for those QoS that are not explictly set, not the statistics recommended QoS
    efd::DataWriterQos qos;
    qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS;
    qos.publish_mode().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
    ASSERT_EQ(qos, history_latency_writer->get_qos());

    // Check that the Statistics recommended QoS are not being used
    eprosima::fastdds::statistics::dds::DataWriterQos qos2;
    qos2.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    qos2.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS;
    qos2.publish_mode().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
    ASSERT_EQ(false, qos2 == history_latency_writer->get_qos());

    // PUBLICATION_THROUGHPUT_TOPIC should have the statistics recommended QoS
    // Also created automatically
    std::string publication_throughput_name = PUBLICATION_THROUGHPUT_TOPIC;
    eprosima::fastdds::dds::DataWriter* publication_throughput_writer =
            statistics_publisher->lookup_datawriter(publication_throughput_name);
    ASSERT_NE(publication_throughput_writer, nullptr);
    ASSERT_EQ(STATISTICS_DATAWRITER_QOS, publication_throughput_writer->get_qos());

    // SUBSCRIPTION_THROUGHPUT_TOPIC has not been created at initialization
    std::string subscription_throughput_name = SUBSCRIPTION_THROUGHPUT_TOPIC;
    eprosima::fastdds::dds::DataWriter* subscription_throughput_writer =
            statistics_publisher->lookup_datawriter(subscription_throughput_name);
    ASSERT_EQ(subscription_throughput_writer, nullptr);

    // Test public method: enable_statistics_datawriter_with_profile()

    // NETWORK_LATENCY_TOPIC is not defined in the fastdds.statistics property policy,
    // it is just defined as data_writer profile. Thus, should not be created at initialization
    std::string network_latency_name = NETWORK_LATENCY_TOPIC;
    eprosima::fastdds::dds::DataWriter* network_latency_writer =
            statistics_publisher->lookup_datawriter(network_latency_name);
    ASSERT_EQ(network_latency_writer, nullptr);

    // But user can enable it manually through enable_statistics_datawriter_with_profile()
    fastdds::dds::ReturnCode_t ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "NETWORK_LATENCY_TOPIC",
        "NETWORK_LATENCY_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_OK, ret);
    network_latency_writer =
            statistics_publisher->lookup_datawriter(network_latency_name);
    ASSERT_NE(network_latency_writer, nullptr);

    efd::DataWriterQos qos3;
    ASSERT_EQ(qos3, network_latency_writer->get_qos());

    // SUBSCRIPTION_THROUGHPUT_TOPIC is not defined in the fastdds.statistics property policy,
    // it is just defined as data_writer profile. Thus, should not be created
    eprosima::fastdds::dds::DataWriter* subscription_througput_writer =
            statistics_publisher->lookup_datawriter(subscription_throughput_name);
    ASSERT_EQ(subscription_througput_writer, nullptr);

    // But user can enable it manually through enable_statistics_datawriter_with_profile()
    ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "SUBSCRIPTION_THROUGHPUT_TOPIC",
        "SUBSCRIPTION_THROUGHPUT_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_OK, ret);
    subscription_througput_writer =
            statistics_publisher->lookup_datawriter(subscription_throughput_name);
    ASSERT_NE(subscription_througput_writer, nullptr);

    // Expected QoS construction for Subscription_Throughput topic:
    efd::DataWriterQos qos4;
    qos4.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    eprosima::fastdds::rtps::Property property;
    property.name("partitions");
    std::string partitions = "part1;part2";
    property.value(std::move(partitions));
    qos4.properties().properties().push_back(std::move(property));
    qos4.deadline().period.seconds = 3;
    qos4.deadline().period.nanosec = 0; // If seconds are set in XML, nanoseconds are set to 0 (while default value is max value)
    qos4.latency_budget().duration.seconds = 2;
    qos4.reliable_writer_qos().disable_heartbeat_piggyback = true;
    ASSERT_EQ(qos4, subscription_througput_writer->get_qos());

    // Calling enable_statistics_datawriter_with_profile with a profile that does not exist,
    // fastdds::dds::RETCODE_ERROR must be returned.
    ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "FAKE_TOPIC",
        "FAKE_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_ERROR, ret);

    // DATA_COUNT_TOPIC is defined with inconsistent QoS policies,
    // and configured to be enabled automatically at initialization
    // It would have been created in a built-in manner with the creation of the DomainParticipant,
    // but as the QoS are inconsistent, the DataWriter is not created, and lookup_datawriter
    // must  return nullptr.
    std::string data_count_name = DATA_COUNT_TOPIC;
    eprosima::fastdds::dds::DataWriter* data_count_writer =
            statistics_publisher->lookup_datawriter(data_count_name);
    ASSERT_EQ(data_count_writer, nullptr);

    // Calling enable_statistics_datawriter_with_profile with a profile defined with inconsistent QoS configuration,
    // fastdds::dds::RETCODE_INCONSISTENT_POLICY must be returned.
    ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "HEARTBEAT_COUNT_TOPIC",
        "HEARTBEAT_COUNT_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_INCONSISTENT_POLICY, ret);

    // There is the possibility to enable a statistics topic with a profile defined with different name:
    ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "OTHER_NAME_FOR_PROFILE",
        "NACKFRAG_COUNT_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_OK, ret);
    std::string nackfrag_count_name = NACKFRAG_COUNT_TOPIC;
    eprosima::fastdds::dds::DataWriter* nackfrag_count_writer =
            statistics_publisher->lookup_datawriter(nackfrag_count_name);
    ASSERT_NE(nackfrag_count_writer, nullptr);

    efd::DataWriterQos qos5;
    qos5.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    ASSERT_EQ(qos5, nackfrag_count_writer->get_qos());

#else // FASTDDS_STATISTICS not set

    // Create DomainParticipant.
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Try to obtain pointer to child class
    eprosima::fastdds::statistics::dds::DomainParticipant* statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
    ASSERT_EQ(statistics_participant, nullptr);

    // Obtain pointer to child class with static_cast instead of narrow()
    // because FASTDDS_STATISTICS CMake option has not been set
    statistics_participant = static_cast<DomainParticipant*>(participant);
    ASSERT_NE(statistics_participant, nullptr);

    fastdds::dds::ReturnCode_t ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "HISTORY_LATENCY_TOPIC",
        "HISTORY_LATENCY_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, ret);

    ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "NETWORK_LATENCY_TOPIC",
        "NETWORK_LATENCY_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, ret);

    ret = statistics_participant->enable_statistics_datawriter_with_profile(
        "SUBSCRIPTION_THROUGHPUT_TOPIC",
        "SUBSCRIPTION_THROUGHPUT_TOPIC");
    ASSERT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, ret);

#endif // FASTDDS_STATISTICS

}

/**
 * This test checks the configuration of datawriter statistics QoS from XML file using generic profile.
 * Generic profile is used for each statistics topic to be enabled that has no specific profile defined
 * in the XML file.
 * Instead, if a specific profile is defined for a topic, it is enabled with that QoS configuration.
 */
TEST_F(StatisticsFromXMLProfileTests, XMLConfigurationForStatisticsDataWritersQoSGenericProfile)
{
#ifdef FASTDDS_STATISTICS

    const std::string xml =
            "                                                                                                                                       \
            <?xml version=\"1.0\" encoding=\"utf-8\"  ?>                                                                                            \
                <dds xmlns=\"http://www.eprosima.com\">                                                                 \
                    <profiles>                                                                                                                      \
                        <participant profile_name=\"statistics_participant\" is_default_profile=\"true\">                                           \
                            <rtps>                                                                                                                  \
                                <propertiesPolicy>                                                                                                  \
                                    <properties>                                                                                                    \
                                        <property>                                                                                                  \
                                            <name>fastdds.statistics</name>                                                                         \
                                            <value>HISTORY_LATENCY_TOPIC;PUBLICATION_THROUGHPUT_TOPIC;_fastdds_statistics_discovered_entity;</value>\
                                            </property>                                                                                             \
                                    </properties>                                                                                                   \
                                </propertiesPolicy>                                                                                                 \
                            </rtps>                                                                                                                 \
                        </participant>                                                                                                              \
                        <data_writer profile_name=\"GENERIC_STATISTICS_PROFILE\">                                                                   \
                            <qos>                                                                                                                   \
                                <reliability>                                                                                                       \
                                    <kind>BEST_EFFORT</kind>                                                                                        \
                                </reliability>                                                                                                      \
                                <durability>                                                                                                        \
                                    <kind>VOLATILE</kind>                                                                                           \
                                </durability>                                                                                                       \
                                <publishMode>                                                                                                       \
                                    <kind>SYNCHRONOUS</kind>                                                                                        \
                                </publishMode>                                                                                                      \
                            </qos>                                                                                                                  \
                        </data_writer>                                                                                                              \
                        <data_writer profile_name=\"NETWORK_LATENCY_TOPIC\">                                                                        \
                        </data_writer>                                                                                                              \
                        <data_writer profile_name=\"SUBSCRIPTION_THROUGHPUT_TOPIC\">                                                                \
                            <qos>                                                                                                                   \
                                <reliability>                                                                                                       \
                                    <kind>BEST_EFFORT</kind>                                                                                        \
                                </reliability>                                                                                                      \
                                <partition>                                                                                                         \
                                    <names>                                                                                                         \
                                        <name>part1</name>                                                                                          \
                                        <name>part2</name>                                                                                          \
                                    </names>                                                                                                        \
                                </partition>                                                                                                        \
                                <deadline>                                                                                                          \
                                    <period>                                                                                                        \
                                        <sec>3</sec>                                                                                                \
                                    </period>                                                                                                       \
                                </deadline>                                                                                                         \
                                <latencyBudget>                                                                                                     \
                                    <duration>                                                                                                      \
                                        <sec>2</sec>                                                                                                \
                                    </duration>                                                                                                     \
                                </latencyBudget>                                                                                                    \
                                <disable_heartbeat_piggyback>true</disable_heartbeat_piggyback>                                                     \
                            </qos>                                                                                                                  \
                        </data_writer>                                                                                                              \
                        <data_writer profile_name=\"_fastdds_statistics_discovered_entity\">                                                        \
                            <qos>                                                                                                                   \
                                <reliability>                                                                                                       \
                                    <kind>RELIABLE</kind>                                                                                           \
                                </reliability>                                                                                                      \
                            </qos>                                                                                                                  \
                        </data_writer>                                                                                                              \
                    </profiles>                                                                                                                     \
                </dds>                                                                                                                              \
            ";

    test_setup_XMLConfigurationForStatisticsDataWritersQoS(
        statistics_participant,
        statistics_publisher,
        xml);

    // Get and check built-in DataWriters:

    // HISTORY_LATENCY_TOPIC has been created automatically within the participant
    // (because it has been included in the corresponding property)
    std::string history_latency_name = HISTORY_LATENCY_TOPIC;
    eprosima::fastdds::dds::DataWriter* history_latency_writer =
            statistics_publisher->lookup_datawriter(history_latency_name);
    ASSERT_NE(history_latency_writer, nullptr);

    // In this test, this topic doesn't have a specific profile.
    // Instead, a generic profile has been defined for default QoS when no specific profile exists.
    // The generic profile is defined under "GENERIC_STATISTICS_PROFILE" name.
    efd::DataWriterQos qos;
    qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS;
    qos.publish_mode().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
    ASSERT_EQ(qos, history_latency_writer->get_qos());

    // SUBSCRIPTION_THROUGHPUT_TOPIC has not been created at initialization
    std::string subscription_throughput_name = SUBSCRIPTION_THROUGHPUT_TOPIC;
    eprosima::fastdds::dds::DataWriter* subscription_throughput_writer =
            statistics_publisher->lookup_datawriter(subscription_throughput_name);
    ASSERT_EQ(subscription_throughput_writer, nullptr);

    // DISCOVERY_TOPIC has been created automatically within the participant
    // and specific QoS has been set by specifying the profile with
    // name of the statistics topic name "_fastdds_statistics_discovered_entity"
    std::string discovery_name = DISCOVERY_TOPIC;
    eprosima::fastdds::dds::DataWriter* discovery_writer =
            statistics_publisher->lookup_datawriter(discovery_name);
    ASSERT_NE(discovery_writer, nullptr);

    efd::DataWriterQos qos2;
    qos2.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    ASSERT_EQ(qos2, discovery_writer->get_qos());


#endif // FASTDDS_STATISTICS
}

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::KillThread();
    return ret;
}
