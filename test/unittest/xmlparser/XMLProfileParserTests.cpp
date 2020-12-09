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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/transport/TCPTransportDescriptor.h>
#include <fastrtps/transport/UDPTransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <tinyxml2.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <stdlib.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace ::testing;

using eprosima::fastdds::dds::Log;
using eprosima::fastdds::dds::LogMock;
using eprosima::fastdds::dds::LogConsumer;

LogMock* log_mock = nullptr;

// Initialize Log mock
void TestRegisterConsumerFunc(
        std::unique_ptr<LogConsumer>&& c)
{
    log_mock->RegisterConsumer(std::move(c));
}

void TestClearConsumersFunc()
{
    log_mock->ClearConsumers();
}

std::function<void(std::unique_ptr<LogConsumer>&&)> Log::RegisterConsumerFunc = TestRegisterConsumerFunc;
std::function<void()> Log::ClearConsumersFunc = TestClearConsumersFunc;

class XMLProfileParserTests : public ::testing::Test
{
public:

    XMLProfileParserTests()
    {
        log_mock = new LogMock();
    }

    ~XMLProfileParserTests()
    {
        delete log_mock;
    }

protected:

    void SetUp() override
    {
        xmlparser::XMLProfileManager::DeleteInstance();
    }

};

TEST_F(XMLProfileParserTests, XMLParserRootLibrarySettings)
{
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_root_library_settings.xml"));

    const LibrarySettingsAttributes& library_settings = xmlparser::XMLProfileManager::library_settings();
    EXPECT_EQ(library_settings.intraprocess_delivery, IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY);
}

TEST_F(XMLProfileParserTests, XMLoadProfiles)
{
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_security_profiles.xml"));
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

/*
 * This test checks the loadXMLProfiles function```
 * 1. Check correct parsing of an XMLElement
 * 2. that an error is returned when trying to parse an XMLElement with an unknown tag.
 * 3. Check return on parsing of an XMLElement with an empty profiles tag
 * 4. Check return on parsing of an XMLElement with a root element different from <profiles>
 */
TEST_F(XMLProfileParserTests, loadXMLProfiles)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // minimal profile xml
        std::string xml_profile =
                "\
                <profiles>\
                    <publisher profile_name=\"test_publisher_profile\"\
                    is_default_profile=\"true\">\
                        <qos>\
                            <durability>\
                                <kind>TRANSIENT_LOCAL</kind>\
                            </durability>\
                        </qos>\
                    </publisher>\
                </profiles>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_profile.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLProfiles(*titleElement));
        xmlparser::XMLProfileManager::DeleteInstance();

    }

    {
        std::vector<std::string> errors =
        {
            "<profiles><bad_tag></bad_tag></profiles>", // wrong profile xml
            "<profiles></profiles>", // empty profile xml
            "<dds></dds>" // not a profile tag as root
        };

        for (std::string& xml_profile : errors)
        {
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_profile.c_str()));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLProfiles(*titleElement));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }
}

/*
 * This test checks the return of the loadXMLDynamicTypes method
 * 1. Check correct return
 * 1. Check error return
 */
TEST_F(XMLProfileParserTests, loadXMLDynamicTypes)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // minimal types xml
        std::string xml =
                "\
                <types>\
                    <type>\
                        <enum name=\"MyAloneEnumType\">\
                            <enumerator name=\"A\" value=\"0\"/>\
                            <enumerator name=\"B\" value=\"1\"/>\
                        </enum>\
                    </type>\
                </types>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLDynamicTypes(*titleElement));
    }

    {
        // wrong xml
        std::string xml =
                "\
                <types>\
                    <type>\
                        <bad_tag></bad_tag>\
                    </type>\
                </types>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLDynamicTypes(*titleElement));
    }
}

TEST_F(XMLProfileParserTests, XMLParserLibrarySettings)
{
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));

    const LibrarySettingsAttributes& library_settings = xmlparser::XMLProfileManager::library_settings();
    EXPECT_EQ(library_settings.intraprocess_delivery, IntraprocessDeliveryType::INTRAPROCESS_FULL);
}

TEST_F(XMLProfileParserTests, XMLParserParcipant)
{
    std::string participant_profile = std::string("test_participant_profile");
    ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile, participant_atts));

    EXPECT_EQ(participant_atts.domainId, 2019102u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    EXPECT_EQ(rtps_atts.allocation.locators.max_unicast_locators, 4u);
    EXPECT_EQ(rtps_atts.allocation.locators.max_multicast_locators, 1u);
    EXPECT_EQ(rtps_atts.allocation.participants.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.participants.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.participants.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.readers.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.readers.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.readers.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.writers.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.writers.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.writers.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.preallocated_number, 127u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.dynamic, true);

    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 1);
    locator.port = 1979;
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32u);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000u);
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.discovery_config.ignoreParticipantFlags,
            eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_SAME_PROCESS |
            eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_HOST);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.count, 2u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.seconds, 1);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.nanosec, 827u);
    EXPECT_FALSE(builtin.avoid_builtin_multicast);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    IPLocator::setIPv4(locator, 192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(builtin.readerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.writerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.readerPayloadSize, 1000u);
    EXPECT_EQ(builtin.writerPayloadSize, 2000u);
    EXPECT_EQ(builtin.mutation_tries, 55u);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.throughputController.bytesPerPeriod, 2048u);
    EXPECT_EQ(rtps_atts.throughputController.periodMillisecs, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

TEST_F(XMLProfileParserTests, XMLParserDefaultParcipantProfile)
{
    std::string participant_profile = std::string("test_participant_profile");
    ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts);

    EXPECT_EQ(participant_atts.domainId, 2019102u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 1);
    locator.port = 1979;
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32u);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000u);
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.discovery_config.ignoreParticipantFlags,
            eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_SAME_PROCESS |
            eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_HOST);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.count, 2u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.seconds, 1);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.nanosec, 827u);
    EXPECT_FALSE(builtin.avoid_builtin_multicast);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    IPLocator::setIPv4(locator, 192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(builtin.readerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.writerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.readerPayloadSize, 1000u);
    EXPECT_EQ(builtin.writerPayloadSize, 2000u);
    EXPECT_EQ(builtin.mutation_tries, 55u);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.throughputController.bytesPerPeriod, 2048u);
    EXPECT_EQ(rtps_atts.throughputController.periodMillisecs, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
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

    TopicAttributes& pub_topic = publisher_atts.topic;
    WriterQos& pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes& pub_times = publisher_atts.times;

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
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.names()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.names()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_times.initialHeartbeatDelay, c_TimeZero);
    EXPECT_EQ(pub_times.heartbeatPeriod.seconds, 11);
    EXPECT_EQ(pub_times.heartbeatPeriod.nanosec, 32u);
    EXPECT_EQ(pub_times.nackResponseDelay, c_TimeZero);
    EXPECT_EQ(pub_times.nackSupressionDuration.seconds, 121);
    EXPECT_EQ(pub_times.nackSupressionDuration.nanosec, 332u);
    IPLocator::setIPv4(locator, 192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    //locator.port = 2021;
    //EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    //EXPECT_EQ(loc_list_it->get_port(), 2021);
    EXPECT_EQ(publisher_atts.throughputController.bytesPerPeriod, 9236u);
    EXPECT_EQ(publisher_atts.throughputController.periodMillisecs, 234u);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
    EXPECT_EQ(publisher_atts.matched_subscriber_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

TEST_F(XMLProfileParserTests, XMLParserDefaultPublisherProfile)
{
    std::string publisher_profile = std::string("test_publisher_profile");
    PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    xmlparser::XMLProfileManager::getDefaultPublisherAttributes(publisher_atts);

    TopicAttributes& pub_topic = publisher_atts.topic;
    WriterQos& pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes& pub_times = publisher_atts.times;

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
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.names()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.names()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_times.initialHeartbeatDelay, c_TimeZero);
    EXPECT_EQ(pub_times.heartbeatPeriod.seconds, 11);
    EXPECT_EQ(pub_times.heartbeatPeriod.nanosec, 32u);
    EXPECT_EQ(pub_times.nackResponseDelay, c_TimeZero);
    EXPECT_EQ(pub_times.nackSupressionDuration.seconds, 121);
    EXPECT_EQ(pub_times.nackSupressionDuration.nanosec, 332u);
    IPLocator::setIPv4(locator, 192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    //locator.port = 2021;
    //EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    //EXPECT_EQ(loc_list_it->get_port(), 2021);
    EXPECT_EQ(publisher_atts.throughputController.bytesPerPeriod, 9236u);
    EXPECT_EQ(publisher_atts.throughputController.periodMillisecs, 234u);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
    EXPECT_EQ(publisher_atts.matched_subscriber_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

TEST_F(XMLProfileParserTests, XMLParserSubscriber)
{
    std::string subscriber_profile = std::string("test_subscriber_profile");
    SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_atts));

    TopicAttributes& sub_topic = subscriber_atts.topic;
    ReaderQos& sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes& sub_times = subscriber_atts.times;

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
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.nanosec, 22u);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.names()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.names()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.names()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.names()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initialAcknackDelay, c_TimeZero);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.nanosec, 81u);
    IPLocator::setIPv4(locator, 192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2079;
    //EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
    EXPECT_EQ(subscriber_atts.matched_publisher_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

TEST_F(XMLProfileParserTests, XMLParserDefaultSubscriberProfile)
{
    std::string subscriber_profile = std::string("test_subscriber_profile");
    SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));
    xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(subscriber_atts);

    TopicAttributes& sub_topic = subscriber_atts.topic;
    ReaderQos& sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes& sub_times = subscriber_atts.times;

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
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.nanosec, 22u);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.names()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.names()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.names()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.names()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initialAcknackDelay, c_TimeZero);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeatResponseDelay.nanosec, 81u);
    IPLocator::setIPv4(locator, 192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2079;
    //EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
    EXPECT_EQ(subscriber_atts.matched_publisher_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Tests the correct parsing of a requester XML profile
 * 1. Check if attributes were filled correctly
 * 2. Check error return on a missing requester profile name
 */
TEST_F(XMLProfileParserTests, XMLParserRequesterProfile)
{
    std::string requester_profile = std::string("test_requester_profile");
    RequesterAttributes requester_atts;

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::fillRequesterAttributes(requester_profile, requester_atts));

    PublisherAttributes& publisher_atts = requester_atts.publisher;
    SubscriberAttributes& subscriber_atts = requester_atts.subscriber;

    EXPECT_EQ(publisher_atts.topic.topicDataType, "request_type");
    EXPECT_EQ(publisher_atts.topic.topicName, "service_name_Request");
    EXPECT_EQ(publisher_atts.qos.m_durability.kind, PERSISTENT_DURABILITY_QOS);

    EXPECT_EQ(subscriber_atts.topic.topicDataType, "reply_type");
    EXPECT_EQ(subscriber_atts.topic.topicName, "service_name_Reply");
    EXPECT_EQ(subscriber_atts.qos.m_durability.kind, PERSISTENT_DURABILITY_QOS);

    // Wrong profile name
    std::string missing_profile = std::string("missing_profile");
    EXPECT_EQ(
        xmlparser::XMLP_ret::XML_ERROR,
        xmlparser::XMLProfileManager::fillRequesterAttributes(missing_profile, requester_atts)
        );
}

/*
 * Tests the correct parsing of a replier XML profile
 * 1. Check if attributes were filled correctly
 * 2. Check error return on a missing replier profile name
 */
TEST_F(XMLProfileParserTests, XMLParserReplierProfile)
{
    std::string replier_profile = std::string("test_replier_profile");
    ReplierAttributes replier_atts;

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::loadXMLFile("test_xml_profiles.xml"));

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::fillReplierAttributes(replier_profile, replier_atts));

    PublisherAttributes& publisher_atts = replier_atts.publisher;
    SubscriberAttributes& subscriber_atts = replier_atts.subscriber;

    EXPECT_EQ(publisher_atts.topic.topicDataType, "reply_type");
    EXPECT_EQ(publisher_atts.topic.topicName, "reply_topic_name");
    EXPECT_EQ(publisher_atts.qos.m_liveliness.kind, MANUAL_BY_TOPIC_LIVELINESS_QOS);

    EXPECT_EQ(subscriber_atts.topic.topicDataType, "request_type");
    EXPECT_EQ(subscriber_atts.topic.topicName, "request_topic_name");
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.kind, MANUAL_BY_TOPIC_LIVELINESS_QOS);

    // Wrong profile name
    std::string missing_profile = std::string("missing_profile");
    EXPECT_EQ(
        xmlparser::XMLP_ret::XML_ERROR,
        xmlparser::XMLProfileManager::fillReplierAttributes(missing_profile, replier_atts)
        );
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

    PropertySeq& part_props = participant_atts.rtps.properties.properties();
    BinaryPropertySeq& part_bin_props = participant_atts.rtps.properties.binary_properties();

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

    PropertySeq& pub_props = publisher_atts.properties.properties();
    BinaryPropertySeq& pub_bin_props = publisher_atts.properties.binary_properties();

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

    PropertySeq& sub_props = subscriber_atts.properties.properties();
    BinaryPropertySeq& sub_bin_props = subscriber_atts.properties.binary_properties();

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

#endif // if HAVE_SECURITY

TEST_F(XMLProfileParserTests, file_xml_consumer_append)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsFileConsumer())).Times(1);
    xmlparser::XMLProfileManager::loadXMLFile("log_node_file_append.xml");
}

TEST_F(XMLProfileParserTests, log_inactive)
{
    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    xmlparser::XMLProfileManager::loadXMLFile("log_inactive.xml");
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it sets
 * a `stderr_threshold` to`Log::Kind::Error` using a property `stderr_threshold`. The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_OK`, meaning that the property was correctly set.
 */
TEST_F(XMLProfileParserTests, log_register_stdouterr)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastrtps::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile("log_stdouterr.xml");
    ASSERT_EQ(eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK, ret);
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it
 * attempts to set a `stderr_threshold` to`Log::Kind::Error` using a property `threshold` (which is not the correct
 * property name). The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_ERROR`, meaning that the property was NOT correctly set.
 */
TEST_F(XMLProfileParserTests, log_register_stdouterr_wrong_property_name)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastrtps::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile(
        "log_stdouterr_wrong_property_name.xml");
    ASSERT_EQ(eprosima::fastrtps::xmlparser::XMLP_ret::XML_ERROR, ret);
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it
 * attempts to set a `stderr_threshold` to`Log::Kind::Error` using a property `stderr_threshold` with value `Error`
 * (which is not a correct property value). The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_ERROR`, meaning that the property was NOT correctly set.
 */
TEST_F(XMLProfileParserTests, log_register_stdouterr_wrong_property_value)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastrtps::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile(
        "log_stdouterr_wrong_property_value.xml");
    ASSERT_EQ(eprosima::fastrtps::xmlparser::XMLP_ret::XML_ERROR, ret);
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it
 * attempts to set a `stderr_threshold` to`Log::Kind::Error` using two properties `stderr_threshold` with different
 * values. However, this operation is not permited, since only one property with name `` can be present.
 * The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_ERROR`, meaning that the property was NOT correctly set. In
 *       this case, the first `stderr_threshold` property will be used, but the function will warn of an incorrect XML
 *       configuration.
 */
TEST_F(XMLProfileParserTests, log_register_stdouterr_two_thresholds)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastrtps::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile(
        "log_stdouterr_two_thresholds.xml");
    ASSERT_EQ(eprosima::fastrtps::xmlparser::XMLP_ret::XML_ERROR, ret);
}

TEST_F(XMLProfileParserTests, file_and_default)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, RegisterConsumer(IsFileConsumer())).Times(1);
    xmlparser::XMLProfileManager::loadXMLFile("log_def_file.xml");
}

TEST_F(XMLProfileParserTests, tls_config)
{
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("tls_config.xml"));

    xmlparser::sp_transport_t transport = xmlparser::XMLProfileManager::getTransportById("Test");

    using TCPDescriptor = std::shared_ptr<TCPTransportDescriptor>;
    TCPDescriptor descriptor = std::dynamic_pointer_cast<TCPTransportDescriptor>(transport);

    /*
       <tls>
        <password>Password</password>
        <private_key_file>Key_file.pem</private_key_file>
        <cert_chain_file>Chain.pem</cert_chain_file>
        <tmp_dh_file>DH.pem</tmp_dh_file>
        <verify_file>verify.pem</verify_file>
        <verify_mode>VERIFY_PEER</verify_mode>
        <options>
            <option>NO_TLSV1</option>
            <option>NO_TLSV1_1</option>
        </options>
       </tls>
     */

    EXPECT_EQ("Password", descriptor->tls_config.password);
    EXPECT_EQ("Key_file.pem", descriptor->tls_config.private_key_file);
    EXPECT_EQ("RSA_file.pem", descriptor->tls_config.rsa_private_key_file);
    EXPECT_EQ("Chain.pem", descriptor->tls_config.cert_chain_file);
    EXPECT_EQ("DH.pem", descriptor->tls_config.tmp_dh_file);
    EXPECT_EQ("verify.pem", descriptor->tls_config.verify_file);
    EXPECT_EQ(TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_PEER, descriptor->tls_config.verify_mode);
    EXPECT_TRUE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1));
    EXPECT_TRUE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1_1));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_SSLV2));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_SSLV3));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1_2));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1_3));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::DEFAULT_WORKAROUNDS));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_COMPRESSION));
    EXPECT_FALSE(descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::SINGLE_DH_USE));

    EXPECT_EQ(descriptor->tls_config.verify_paths.size(), static_cast<size_t>(3));
    EXPECT_EQ(descriptor->tls_config.verify_paths[0], "Path1");
    EXPECT_EQ(descriptor->tls_config.verify_paths[1], "Path2");
    EXPECT_EQ(descriptor->tls_config.verify_paths[2], "Path3");
    EXPECT_EQ(descriptor->tls_config.verify_depth, static_cast<int32_t>(55));
    EXPECT_TRUE(descriptor->tls_config.default_verify_path);

    EXPECT_EQ(descriptor->tls_config.handshake_role, TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::SERVER);
}

TEST_F(XMLProfileParserTests, UDP_transport_descriptors_config)
{
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("UDP_transport_descriptors_config.xml"));

    xmlparser::sp_transport_t transport = xmlparser::XMLProfileManager::getTransportById("Test");

    using UDPDescriptor = std::shared_ptr<UDPTransportDescriptor>;
    UDPDescriptor descriptor = std::dynamic_pointer_cast<UDPTransportDescriptor>(transport);

    ASSERT_NE(descriptor, nullptr);
    EXPECT_EQ(descriptor->sendBufferSize, 8192u);
    EXPECT_EQ(descriptor->receiveBufferSize, 8192u);
    EXPECT_EQ(descriptor->TTL, 250u);
    EXPECT_EQ(descriptor->non_blocking_send, true);
    EXPECT_EQ(descriptor->maxMessageSize, 16384u);
    EXPECT_EQ(descriptor->maxInitialPeersRange, 100u);
    EXPECT_EQ(descriptor->interfaceWhiteList.size(), 2u);
    EXPECT_EQ(descriptor->interfaceWhiteList[0], "192.168.1.41");
    EXPECT_EQ(descriptor->interfaceWhiteList[1], "127.0.0.1");
    EXPECT_EQ(descriptor->m_output_udp_socket, 5101u);
}

TEST_F(XMLProfileParserTests, SHM_transport_descriptors_config)
{
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("SHM_transport_descriptors_config.xml"));

    xmlparser::sp_transport_t transport = xmlparser::XMLProfileManager::getTransportById("Test");

    using SHMDescriptor = std::shared_ptr<eprosima::fastdds::rtps::SharedMemTransportDescriptor>;
    SHMDescriptor descriptor = std::dynamic_pointer_cast<eprosima::fastdds::rtps::SharedMemTransportDescriptor>(
        transport);

    ASSERT_NE(descriptor, nullptr);
    ASSERT_EQ(descriptor->segment_size(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(descriptor->port_queue_capacity(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(descriptor->healthy_check_timeout_ms(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(descriptor->rtps_dump_file(), "test_file.dump");
    ASSERT_EQ(descriptor->maxMessageSize, 128000u);
    ASSERT_EQ(descriptor->max_message_size(), 128000u);
}

/*
 * Test return code of the insertTransportById method when trying to insert two transports with the same id
 */
TEST_F(XMLProfileParserTests, insertTransportByIdNegativeClauses)
{
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> transport;
    EXPECT_EQ(true, xmlparser::XMLProfileManager::insertTransportById("duplicated_id", transport));
    EXPECT_EQ(false, xmlparser::XMLProfileManager::insertTransportById("duplicated_id", transport));
}

/*
 * Test return code of the getDynamicTypeByName method when trying to retrieve a type which has not been parsed
 */
TEST_F(XMLProfileParserTests, getDynamicTypeByNameNegativeClauses)
{
    EXPECT_EQ(nullptr, xmlparser::XMLProfileManager::getDynamicTypeByName("wrong_type"));
}

/*
 * Test return code of the getTransportById method when trying to retrieve a transport descriptor which has not been
 * parsed
 */
TEST_F(XMLProfileParserTests, getTransportByIdNegativeClauses)
{
    EXPECT_EQ(nullptr, xmlparser::XMLProfileManager::getTransportById("wrong_type"));
}

/*
 * Tests whether the extraction of XML profiles succeeds when all profiles are correct.
 * XMLProfileManager::loadXMLNode returns XMLProfileManager::extractProfiles.
 * The following cases are tested:
 *  1. A corrrect standalone profile element is parsed, return value of XMLP_ret::XML_OK is expected
 *  1. A corrrect rooted profile element is parsed, return value of XMLP_ret::XML_OK is expected
 *  1. An incorrect profile element is parsed, return value of XMLP_ret::XML_ERROR is expected
 */
TEST_F(XMLProfileParserTests, extract_profiles_ok)
{
    tinyxml2::XMLDocument xml_doc;
    {
        const char* xml =
                "                                                                                                            \n\
        <profiles>                                                                                                   \n\
            <participant profile_name=\"participant_prof\">                                                          \n\
                <rtps></rtps>                                                                                        \n\
            </participant>                                                                                           \n\
            <publisher profile_name=\"publisher_prof\"></publisher>                                                  \n\
            <subscriber profile_name=\"subscriber_prof\"></subscriber>                                               \n\
            <topic profile_name=\"topic_prof\"></topic>                                                              \n\
            <requester profile_name=\"req_prof\" service_name=\"serv\" request_type=\"req\" reply_type=\"reply\">    \n\
            </requester>                                                                                             \n\
            <replier profile_name=\"replier_prof\" service_name=\"serv\" request_type=\"req\" reply_type=\"reply\">  \n\
            </replier>                                                                                               \n\
        </profiles>                                                                                                  \n\
        ";

        // Standalone
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();

        // Rooted
        std::string xml_dds = "<dds>" + std::string(xml) + "</dds>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_dds.c_str()));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();
    }

    {
        // wrong xml
        std::string xml = "<bad_tag></bad_tag>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();
    }


}

/*
 * Tests whether the extraction of XML profiles succeeds when some profiles are correct and some are not.
 * XMLProfileManager::loadXMLNode returns XMLProfileManager::extractProfiles.
 * The expected return value is XMLP_ret::XML_NOK.
 * Checks a combination of a correct profile with an incorrectprofile of each profyle type
 */
TEST_F(XMLProfileParserTests, extract_profiles_nok)
{
    tinyxml2::XMLDocument xml_doc;

    const char* xml_p =
            "\
                <profiles>\
                    <!-- OK PROFILE -->\
                    <participant profile_name=\"participant_prof\">\
                        <rtps></rtps>\
                    </participant>\
                    \
                    <!-- NOK PROFILE --> \
                    <%s></%s>\
                </profiles>\
            ";
    char xml[500];

    std::vector<std::string> elements {
        "participant",
        "publisher",
        "subscriber",
        "topic",
        "requester",
        "replier",
    };

    for (const std::string& e : elements)
    {
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_NOK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

/*
 * Tests whether the extraction of XML profiles succeeds when all profiles are wrong.
 * XMLProfileManager::loadXMLNode returns XMLProfileManager::extractProfiles.
 * The expected return value is XMLP_ret::XML_ERROR.
 * Checks an incorrec profile of each type as the only present in the profiles tag
 */
TEST_F(XMLProfileParserTests, extract_profiles_error)
{
    tinyxml2::XMLDocument xml_doc;

    {
        // XML Without a root
        const char* xml = "<a/>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
    }

    {
        const char* xml_p =
                "\
                    <profiles>\
                        <%s>\
                        </%s>\
                    </profiles>\
                ";
        char xml[500];

        std::vector<std::string> elements {
            "participant",
            "publisher",
            "subscriber",
            "topic",
            "requester",
            "replier",
        };

        for (const std::string& e : elements)
        {
            sprintf(xml, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        // XMLNode with no profile element
        const char* xml = "<log></log>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
    }
}

/*
 * This test check the possible errors in the parsing of the profile of each type and the loading of a profile as
 * the default profile.
 * Errors tested:
 *  1. Empty profile name
 *  2. Loading the same profile twice
 */
TEST_F(XMLProfileParserTests, extract_type_profiles)
{
    tinyxml2::XMLDocument xml_doc;

    // Participant
    {
        const char* xml_p =
                "<profiles>\
            <participant profile_name=\"%s\">\
                <rtps></rtps>\
            </participant>\
        </profiles>";
        char xml[500];

        // empty name
        sprintf(xml, xml_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

        // same profile name twice
        sprintf(xml, xml_p, "test_name");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

        xmlparser::XMLProfileManager::DeleteInstance();
    }

    {
        const char* xml_p =
                "<profiles>\
            <%s profile_name=\"%s\" %s>\
            </%s>\
        </profiles>";
        char xml[500];

        std::vector<std::string> elements {
            "publisher",
            "subscriber",
            "topic",
        };

        for (const std::string& e : elements)
        {
            // empty profile name
            sprintf(xml, xml_p, e.c_str(), "", "", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

            // default profile
            sprintf(xml, xml_p, e.c_str(), "default_name_1", "is_default_profile=\"true\"", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

            // same profile name twice
            sprintf(xml, xml_p, e.c_str(), "test_name", "", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        const char* xml_p =
                "<profiles>\
            <%s profile_name=\"%s\" service_name=\"serv\" request_type=\"req\" reply_type=\"reply\">\
            </%s>\
        </profiles>";
        char xml[500];

        std::vector<std::string> elements {
            "requester",
            "replier"
        };

        for (const std::string& e : elements)
        {
            // empty profile name
            sprintf(xml, xml_p, e.c_str(), "", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

            // same profile name twice
            sprintf(xml, xml_p, e.c_str(), "test_name", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

}


/*
 * Tests whether the SKIP_DEFAULT_XML_FILE variable prevents the xmlparser from loading the default XML file.
 * participant_atts_none skips the default and obtains the values from the constructors.
 * participant_atts_default contains the attributes in the default file in this folder which should be different.
 */
TEST_F(XMLProfileParserTests, skip_default_xml)
{
    const char* xml =
            "                                                                                                          \
        <profiles>                                                                                                     \
            <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">                        \
                <domainId>2020268</domainId>                                                                           \
                <rtps></rtps>                                                                                          \
            </participant>                                                                                             \
        </profiles>                                                                                                    \
    ";
    tinyxml2::XMLDocument xml_doc;
    xml_doc.Parse(xml);
    xml_doc.SaveFile("DEFAULT_FASTRTPS_PROFILES.xml");

#ifdef _WIN32
    _putenv_s("SKIP_DEFAULT_XML_FILE", "1");
#else
    setenv("SKIP_DEFAULT_XML_FILE", "1", 1);
#endif // ifdef _WIN32
    ParticipantAttributes participant_atts_none;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_none);

#ifdef _WIN32
    _putenv_s("SKIP_DEFAULT_XML_FILE", "");
#else
    unsetenv("SKIP_DEFAULT_XML_FILE");
#endif // ifdef _WIN32
    ParticipantAttributes participant_atts_default;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_default);

    remove("DEFAULT_FASTRTPS_PROFILES.xml");

    EXPECT_NE(participant_atts_none.domainId, participant_atts_default.domainId);
}

/*
 * Tests whether the FASTRTPS_DEFAULT_PROFILES_FILE environment file correctly loads the selected file as default.
 * - participant_atts_default contains the attributes in the default file in this folder.
 * - participant_atts_file contains the attributes in the default file created by the test.
 */
TEST_F(XMLProfileParserTests, default_env_variable)
{
    const char* xml =
            "                                                                                                                  \
        <profiles>                                                                                                     \
            <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">                        \
                <domainId>20203011</domainId>                                                                          \
                <rtps></rtps>                                                                                          \
            </participant>                                                                                             \
        </profiles>                                                                                                    \
    ";
    tinyxml2::XMLDocument xml_doc;
    xml_doc.Parse(xml);
    xml_doc.SaveFile("FASTRTPS_PROFILES.xml");

    ParticipantAttributes participant_atts_default;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_default);

#ifdef _WIN32
    _putenv_s("FASTRTPS_DEFAULT_PROFILES_FILE", "FASTRTPS_PROFILES.xml");
#else
    setenv("FASTRTPS_DEFAULT_PROFILES_FILE", "FASTRTPS_PROFILES.xml", 1);
#endif // ifdef _WIN32
    ParticipantAttributes participant_atts_file;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_file);
    remove("FASTRTPS_PROFILES.xml");

    EXPECT_NE(participant_atts_file.domainId, participant_atts_default.domainId);
}

/*
 * Tests the return codes of the loadXMLFile method.
 *  1. On parsing a correctly parsed standalone profiles file, expect XMLP_ret::XML_OK.
 *  2. On parsing a correctly parsed rooted profiles file, expect XMLP_ret::XML_OK.
 *  3. On parsing an incorrect element inside profiles element, expect XMLP_ret::XML_ERROR.
 *  4. On giving an empty string as a filename, expect XMLP_ret::XML_ERROR.
 *  5. On parsing the same file twice, expect XMLP_ret::XML_OK.
 *  6. On parsing an incorrect element inside profiles element, expect XMLP_ret::XML_ERROR.
 */
TEST_F(XMLProfileParserTests, loadXMLFile)
{
    const char* filename = "minimal_file.xml";
    tinyxml2::XMLDocument xml_doc;

    {
        std::vector<std::string> correct_xmls =
        {
            "<profiles>\
            <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">\
                <domainId>20203011</domainId>\
                <rtps></rtps>\
            </participant>\
        </profiles>",
            "<types>\
            <type>\
                <enum name=\"MyAloneEnumType\">\
                    <enumerator name=\"A\" value=\"0\"/>\
                    <enumerator name=\"B\" value=\"1\"/>\
                </enum>\
            </type>\
        </types>",
            "<log>\
            <use_default>FALSE</use_default>\
            <consumer>\
                <class>StdoutErrConsumer</class>\
            </consumer>\
        </log>",
            "<library_settings>\
            <intraprocess_delivery>FULL</intraprocess_delivery>\
        </library_settings>"
        };

        // Standalone
        for (std::string& xml : correct_xmls)
        {
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse((xml).c_str()));
            xml_doc.SaveFile(filename);
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
            remove(filename);
            xmlparser::XMLProfileManager::DeleteInstance();
        }

        // Rooted
        for (std::string& xml : correct_xmls)
        {
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(("<dds>" + xml + "</dds>").c_str()));
            xml_doc.SaveFile(filename);
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
            remove(filename);
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        // Negatve clauses
        const char* xml_p =
                "\
                <%s>\
                    <bad_tag></bad_tag>\
                </%s>\
                ";
        char xml[500];

        std::vector<std::string> elements {
            "profiles",
            "types",
            "log",
            "library_settings",
            "bad_profile"
        };

        for (const std::string& e : elements)
        {
            sprintf(xml, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            xml_doc.SaveFile(filename);
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLFile(filename));
            remove(filename);
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        const char* xml =
                "<profiles>\
                <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">\
                    <domainId>20203011</domainId>\
                    <rtps></rtps>\
                </participant>\
            </profiles>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        xml_doc.SaveFile(filename);

        // passing an empty string as a filename
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLFile(""));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
        remove(filename);
        xmlparser::XMLProfileManager::DeleteInstance();
    }

}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
