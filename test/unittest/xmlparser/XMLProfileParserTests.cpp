// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class XMLProfileParserTests: public ::testing::Test
{
    public:
        XMLProfileParserTests()
        {
        }

        ~XMLProfileParserTests()
        {
            Log::KillThread();
        }
};

TEST_F(XMLProfileParserTests, XMLoadProfiles)
{
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_security_profiles.xml"));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_ERROR,
                xmlparser::XMLProfileManager::loadXMLFile("missing_file.xml"));

    ParticipantAttributes participant_atts;
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillParticipantAttributes("test_participant_profile", participant_atts));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_ERROR,
                xmlparser::XMLProfileManager::fillParticipantAttributes("bad_name", participant_atts));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_ERROR,
                xmlparser::XMLProfileManager::fillParticipantAttributes("test_publisher_profile", participant_atts));
}

TEST_F(XMLProfileParserTests, XMLParserParcipant)
{
    std::string participant_profile = std::string("test_participant_profile");
    ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile, participant_atts));

    RTPSParticipantAttributes &rtps_atts = participant_atts.rtps;
    BuiltinAttributes &builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters &port = rtps_atts.port;

    locator.set_IP4_address(192, 168, 1 , 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    locator.set_IP4_address(239, 255, 0 , 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    locator.set_IP4_address(192, 168, 1 , 1);
    locator.port = 1979;
    EXPECT_EQ(*rtps_atts.defaultOutLocatorList.begin(), locator);
    EXPECT_EQ(rtps_atts.defaultSendPort, 80);
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000);
    EXPECT_EQ(builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol, true);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.domainId, 2019102);
    EXPECT_EQ(builtin.leaseDuration, c_TimeInfinite);
    EXPECT_EQ(builtin.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.leaseDuration_announcementperiod.fraction, 333);
    EXPECT_EQ(builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    locator.set_IP4_address(192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    locator.set_IP4_address(192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    locator.set_IP4_address(239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.use_IP4_to_send, true);
    EXPECT_EQ(rtps_atts.use_IP6_to_send, false);
    EXPECT_EQ(rtps_atts.throughputController.bytesPerPeriod, 2048);
    EXPECT_EQ(rtps_atts.throughputController.periodMillisecs, 45);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, false);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

TEST_F(XMLProfileParserTests, XMLParserDefaultParcipantProfile)
{
    std::string participant_profile = std::string("test_participant_profile");
    ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts);

    RTPSParticipantAttributes &rtps_atts = participant_atts.rtps;
    BuiltinAttributes &builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters &port = rtps_atts.port;

    locator.set_IP4_address(192, 168, 1 , 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    locator.set_IP4_address(239, 255, 0 , 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    locator.set_IP4_address(192, 168, 1 , 1);
    locator.port = 1979;
    EXPECT_EQ(*rtps_atts.defaultOutLocatorList.begin(), locator);
    EXPECT_EQ(rtps_atts.defaultSendPort, 80);
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000);
    EXPECT_EQ(builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol, true);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.domainId, 2019102);
    EXPECT_EQ(builtin.leaseDuration, c_TimeInfinite);
    EXPECT_EQ(builtin.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.leaseDuration_announcementperiod.fraction, 333);
    EXPECT_EQ(builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    locator.set_IP4_address(192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    locator.set_IP4_address(192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    locator.set_IP4_address(239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.use_IP4_to_send, true);
    EXPECT_EQ(rtps_atts.use_IP6_to_send, false);
    EXPECT_EQ(rtps_atts.throughputController.bytesPerPeriod, 2048);
    EXPECT_EQ(rtps_atts.throughputController.periodMillisecs, 45);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, false);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

TEST_F(XMLProfileParserTests, XMLParserPublisher)
{
    std::string publisher_profile = std::string("test_publisher_profile");
    PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillPublisherAttributes(publisher_profile, publisher_atts));

    TopicAttributes &pub_topic = publisher_atts.topic;
    WriterQos &pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes &pub_times = publisher_atts.times;

    EXPECT_EQ(pub_topic.topicKind, NO_KEY);
    EXPECT_EQ(pub_topic.topicName, "samplePubSubTopic");
    EXPECT_EQ(pub_topic.topicDataType, "samplePubSubTopicType");
    EXPECT_EQ(pub_topic.historyQos.kind, KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(pub_topic.historyQos.depth, 50);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples, 432);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_instances, 1);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples_per_instance, 100);
    EXPECT_EQ(pub_topic.resourceLimitsQos.allocated_samples, 123);
    EXPECT_EQ(pub_qos.m_durability.kind, TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.kind, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.fraction, 2);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.getNames()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.getNames()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_times.initialHeartbeatDelay, c_TimeZero);
    EXPECT_EQ(pub_times.heartbeatPeriod.seconds, 11);
    EXPECT_EQ(pub_times.heartbeatPeriod.fraction, 32);
    EXPECT_EQ(pub_times.nackResponseDelay, c_TimeInvalid);
    EXPECT_EQ(pub_times.nackSupressionDuration.seconds, 121);
    EXPECT_EQ(pub_times.nackSupressionDuration.fraction, 332);
    locator.set_IP4_address(192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    locator.set_IP4_address(192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    locator.set_IP4_address(0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 2021;
    EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(loc_list_it->port, 2021);
    EXPECT_EQ(publisher_atts.throughputController.bytesPerPeriod, 9236);
    EXPECT_EQ(publisher_atts.throughputController.periodMillisecs, 234);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
}

TEST_F(XMLProfileParserTests, XMLParserDefaultPublisherProfile)
{
    std::string publisher_profile = std::string("test_publisher_profile");
    PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    xmlparser::XMLProfileManager::getDefaultPublisherAttributes(publisher_atts);

    TopicAttributes &pub_topic = publisher_atts.topic;
    WriterQos &pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes &pub_times = publisher_atts.times;

    EXPECT_EQ(pub_topic.topicKind, NO_KEY);
    EXPECT_EQ(pub_topic.topicName, "samplePubSubTopic");
    EXPECT_EQ(pub_topic.topicDataType, "samplePubSubTopicType");
    EXPECT_EQ(pub_topic.historyQos.kind, KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(pub_topic.historyQos.depth, 50);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples, 432);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_instances, 1);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples_per_instance, 100);
    EXPECT_EQ(pub_topic.resourceLimitsQos.allocated_samples, 123);
    EXPECT_EQ(pub_qos.m_durability.kind, TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.kind, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.fraction, 2);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.getNames()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.getNames()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_times.initialHeartbeatDelay, c_TimeZero);
    EXPECT_EQ(pub_times.heartbeatPeriod.seconds, 11);
    EXPECT_EQ(pub_times.heartbeatPeriod.fraction, 32);
    EXPECT_EQ(pub_times.nackResponseDelay, c_TimeInvalid);
    EXPECT_EQ(pub_times.nackSupressionDuration.seconds, 121);
    EXPECT_EQ(pub_times.nackSupressionDuration.fraction, 332);
    locator.set_IP4_address(192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    locator.set_IP4_address(192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    locator.set_IP4_address(0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 2021;
    EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(loc_list_it->port, 2021);
    EXPECT_EQ(publisher_atts.throughputController.bytesPerPeriod, 9236);
    EXPECT_EQ(publisher_atts.throughputController.periodMillisecs, 234);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
}

TEST_F(XMLProfileParserTests, XMLParserSubscriber)
{
    std::string subscriber_profile = std::string("test_subscriber_profile");
    SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_atts));

    TopicAttributes &sub_topic = subscriber_atts.topic;
    ReaderQos &sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes &sub_times = subscriber_atts.times;

    EXPECT_EQ(sub_topic.topicKind, WITH_KEY);
    EXPECT_EQ(sub_topic.topicName, "otherSamplePubSubTopic");
    EXPECT_EQ(sub_topic.topicDataType, "otherSamplePubSubTopicType");
    EXPECT_EQ(sub_topic.historyQos.kind, KEEP_ALL_HISTORY_QOS);
    EXPECT_EQ(sub_topic.historyQos.depth, 1001);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples, 52);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_instances, 25);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples_per_instance, 32);
    EXPECT_EQ(sub_topic.resourceLimitsQos.allocated_samples, 37);
    EXPECT_EQ(sub_qos.m_durability.kind, PERSISTENT_DURABILITY_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.kind, MANUAL_BY_TOPIC_LIVELINESS_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.seconds, 11);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.fraction, 22);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.getNames()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.getNames()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.getNames()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.getNames()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initialAcknackDelay, c_TimeZero);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.fraction, 81);
    locator.set_IP4_address(192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    locator.set_IP4_address(0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    locator.set_IP4_address(0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(192, 168, 1, 2);
    locator.port = 2079;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
}

TEST_F(XMLProfileParserTests, XMLParserDefaultSubscriberProfile)
{
    std::string subscriber_profile = std::string("test_subscriber_profile");
    SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(subscriber_atts);

    TopicAttributes &sub_topic = subscriber_atts.topic;
    ReaderQos &sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes &sub_times = subscriber_atts.times;

    EXPECT_EQ(sub_topic.topicKind, WITH_KEY);
    EXPECT_EQ(sub_topic.topicName, "otherSamplePubSubTopic");
    EXPECT_EQ(sub_topic.topicDataType, "otherSamplePubSubTopicType");
    EXPECT_EQ(sub_topic.historyQos.kind, KEEP_ALL_HISTORY_QOS);
    EXPECT_EQ(sub_topic.historyQos.depth, 1001);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples, 52);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_instances, 25);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples_per_instance, 32);
    EXPECT_EQ(sub_topic.resourceLimitsQos.allocated_samples, 37);
    EXPECT_EQ(sub_qos.m_durability.kind, PERSISTENT_DURABILITY_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.kind, MANUAL_BY_TOPIC_LIVELINESS_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.seconds, 11);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.fraction, 22);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.getNames()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.getNames()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.getNames()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.getNames()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initialAcknackDelay, c_TimeZero);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.fraction, 81);
    locator.set_IP4_address(192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    locator.set_IP4_address(0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    locator.set_IP4_address(0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.set_IP4_address(192, 168, 1, 2);
    locator.port = 2079;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
}

#if HAVE_SECURITY

TEST_F(XMLProfileParserTests, XMLParserSecurity)
{
    std::string participant_profile = std::string("test_participant_security_profile");
    ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::loadXMLFile("test_xml_security_profiles.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile, participant_atts));

    PropertySeq &part_props = participant_atts.rtps.properties.properties();
    BinaryPropertySeq &part_bin_props = participant_atts.rtps.properties.binary_properties();

    EXPECT_EQ(part_props[0].name(), "dds.sec.auth.builtin.PKI-DH.identity_ca");
    EXPECT_EQ(part_props[0].value(), "maincacert.pem");
    EXPECT_EQ(part_props[0].propagate(), false);
    EXPECT_EQ(part_props[1].name(), "dds.sec.auth.builtin.PKI-DH.identity_certificate");
    EXPECT_EQ(part_props[1].value(), "appcert.pem");
    EXPECT_EQ(part_props[1].propagate(), true);
    EXPECT_EQ(part_bin_props[0].name(), "binary_prop_a");
    EXPECT_EQ(part_bin_props[0].propagate(), false);
    EXPECT_EQ(part_bin_props[1].name(), "binary_prop_b");
    EXPECT_EQ(part_bin_props[1].propagate(), true);


    std::string publisher_profile = std::string("test_publisher_security_profile");
    PublisherAttributes publisher_atts;
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillPublisherAttributes(publisher_profile, publisher_atts));

    PropertySeq &pub_props = publisher_atts.properties.properties();
    BinaryPropertySeq &pub_bin_props = publisher_atts.properties.binary_properties();

    EXPECT_EQ(pub_props[0].name(), "rtps.endpoint.submessage_protection_kind");
    EXPECT_EQ(pub_props[0].value(), "ENCRYPT");
    EXPECT_EQ(pub_props[0].propagate(), true);
    EXPECT_EQ(pub_props[1].name(), "rtps.endpoint.payload_protection_kind");
    EXPECT_EQ(pub_props[1].value(), "ENCRYPT");
    EXPECT_EQ(pub_props[1].propagate(), true);
    EXPECT_EQ(pub_bin_props[0].name(), "binary_prop_c");
    EXPECT_EQ(pub_bin_props[0].propagate(), true);
    EXPECT_EQ(pub_bin_props[1].name(), "binary_prop_d");
    EXPECT_EQ(pub_bin_props[1].propagate(), false);


    std::string subscriber_profile = std::string("test_subscriber_security_profile");
    SubscriberAttributes subscriber_atts;

    EXPECT_EQ(xmlparser::XMLP_ret::XML_OK,
                xmlparser::XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_atts));

    PropertySeq &sub_props = subscriber_atts.properties.properties();
    BinaryPropertySeq &sub_bin_props = subscriber_atts.properties.binary_properties();

    EXPECT_EQ(sub_props[0].name(), "rtps.endpoint.submessage_protection_kind");
    EXPECT_EQ(pub_props[0].value(), "ENCRYPT");
    EXPECT_EQ(pub_props[0].propagate(), true);
    EXPECT_EQ(sub_props[1].name(), "rtps.endpoint.payload_protection_kind");
    EXPECT_EQ(sub_props[1].value(), "ENCRYPT");
    EXPECT_EQ(sub_props[1].propagate(), true);
    EXPECT_EQ(sub_bin_props[0].name(), "binary_prop_e");
    EXPECT_EQ(sub_bin_props[0].propagate(), true);
    EXPECT_EQ(sub_bin_props[1].name(), "binary_prop_f");
    EXPECT_EQ(sub_bin_props[1].propagate(), false);
}

#endif

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);
    Log::SetCategoryFilter(std::regex("(XMLPARSER)"));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
