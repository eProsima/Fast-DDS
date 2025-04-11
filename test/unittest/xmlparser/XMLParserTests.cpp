// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "XMLParserTests.hpp"

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>
#include <tinyxml2.h>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/network/AllowedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/BlockedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <xmlparser/XMLParser.h>
#include <xmlparser/XMLProfileManager.h>
#include <xmlparser/XMLTree.h>

#include "../logging/mock/MockConsumer.h"
#include "wrapper/XMLParserTest.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using namespace ::testing;

using eprosima::fastdds::xmlparser::BaseNode;
using eprosima::fastdds::xmlparser::DataNode;
using eprosima::fastdds::xmlparser::NodeType;
using eprosima::fastdds::xmlparser::XMLP_ret;
using eprosima::fastdds::xmlparser::XMLParser;

using eprosima::fastdds::dds::Log;
using eprosima::fastdds::dds::LogConsumer;

TEST_F(XMLParserTests, regressions)
{
    std::unique_ptr<BaseNode> root;

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/12736_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/13418_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/13454_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/13513_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/14456_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/15344_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/18395_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/19354_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/19354_2_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/19851_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/simple_participant_profiles_nok.xml", root));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParser::loadXML("regressions/simple_participant_profiles_ok.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/20186_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/20187_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/20608_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/20610_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/20732_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/21153_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/21154_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/21181_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/21223_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/21334_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/21856_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/22054_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/22101_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/22535_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/22843_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/22844_profile_bin.xml", root));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("regressions/23030_profile_bin.xml", root));
    Log::Flush();
}

TEST_F(XMLParserTests, NoFile)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("missing_file.xml", root), XMLP_ret::XML_ERROR);
}

TEST_F(XMLParserTests, EmptyDefaultFile)
{
    std::ifstream inFile;
    inFile.open("DEFAULT_FASTDDS_PROFILES.xml");
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadDefaultXMLFile(root), XMLP_ret::XML_ERROR);
}

TEST_F(XMLParserTests, EmptyString)
{
    std::ifstream inFile;
    inFile.open("missing_file.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_ERROR);
}

TEST_F(XMLParserTests, WrongName)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("test_xml_profile.xml", root), XMLP_ret::XML_OK);
    xmlparser::ParticipantAttributes participant_atts;
    ASSERT_FALSE(get_participant_attributes(root, participant_atts));
}

TEST_F(XMLParserTests, WrongNameBuffer)
{
    std::ifstream inFile;
    inFile.open("test_xml_profile.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);
    xmlparser::ParticipantAttributes participant_atts;
    ASSERT_FALSE(get_participant_attributes(root, participant_atts));
}

/*
 * Checks the supported and XML validated entity hierarchy
 */
TEST_F(XMLParserTests, TypesRooted)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("test_xml_rooted_profile.xml", root), XMLP_ret::XML_OK);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    bool topic_data          = false;
    for (const auto& root_child : root->getChildren())
    {
        if (root_child->getType() == NodeType::PROFILES)
        {
            for (const auto& profile : root_child->getChildren())
            {
                if (profile->getType() == NodeType::PARTICIPANT)
                {
                    participant_profile = true;
                }
                else if (profile->getType() == NodeType::PUBLISHER)
                {
                    publisher_profile = true;
                }
                else if (profile->getType() == NodeType::SUBSCRIBER)
                {
                    subscriber_profile = true;
                }
                else if (profile->getType() == NodeType::TOPIC)
                {
                    topic_data = true;
                }
            }
        }
    }
    ASSERT_TRUE(participant_profile);
    ASSERT_TRUE(publisher_profile);
    ASSERT_TRUE(subscriber_profile);
    ASSERT_TRUE(topic_data);
}

/*
 * Checks the supported entity hierarchy
 */
TEST_F(XMLParserTests, TypesRootedDeprecated)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("test_xml_rooted_deprecated.xml", root), XMLP_ret::XML_OK);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    bool topic_data          = false;
    for (const auto& root_child : root->getChildren())
    {
        if (root_child->getType() == NodeType::PROFILES)
        {
            for (const auto& profile : root_child->getChildren())
            {
                if (profile->getType() == NodeType::PARTICIPANT)
                {
                    participant_profile = true;
                }
                else if (profile->getType() == NodeType::SUBSCRIBER)
                {
                    publisher_profile = true;
                }
                else if (profile->getType() == NodeType::PUBLISHER)
                {
                    subscriber_profile = true;
                }
            }
        }
        else if (root_child->getType() == NodeType::TOPIC)
        {
            topic_data = true;
        }
    }
    ASSERT_TRUE(participant_profile);
    ASSERT_TRUE(publisher_profile);
    ASSERT_TRUE(subscriber_profile);
    ASSERT_TRUE(topic_data);
}

/*
 * Checks the supported and XML validated entity hierarchy
 */
TEST_F(XMLParserTests, TypesRootedBuffer)
{
    std::ifstream inFile;
    inFile.open("test_xml_rooted_profile.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);
    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    bool topic_data          = false;
    for (const auto& root_child : root->getChildren())
    {
        if (root_child->getType() == NodeType::PROFILES)
        {
            for (const auto& profile : root_child->getChildren())
            {
                if (profile->getType() == NodeType::PARTICIPANT)
                {
                    participant_profile = true;
                }
                else if (profile->getType() == NodeType::PUBLISHER)
                {
                    publisher_profile = true;
                }
                else if (profile->getType() == NodeType::SUBSCRIBER)
                {
                    subscriber_profile = true;
                }
                else if (profile->getType() == NodeType::TOPIC)
                {
                    topic_data = true;
                }
            }
        }
    }
    ASSERT_TRUE(participant_profile);
    ASSERT_TRUE(publisher_profile);
    ASSERT_TRUE(subscriber_profile);
    ASSERT_TRUE(topic_data);
}

/*
 * Checks the supported entity hierarchy
 */
TEST_F(XMLParserTests, TypesRootedBufferDeprecated)
{
    std::ifstream inFile;
    inFile.open("test_xml_rooted_deprecated.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);
    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    bool topic_data          = false;
    for (const auto& root_child : root->getChildren())
    {
        if (root_child->getType() == NodeType::PROFILES)
        {
            for (const auto& profile : root_child->getChildren())
            {
                if (profile->getType() == NodeType::PARTICIPANT)
                {
                    participant_profile = true;
                }
                else if (profile->getType() == NodeType::SUBSCRIBER)
                {
                    publisher_profile = true;
                }
                else if (profile->getType() == NodeType::PUBLISHER)
                {
                    subscriber_profile = true;
                }
            }
        }
        else if (root_child->getType() == NodeType::TOPIC)
        {
            topic_data = true;
        }
    }
    ASSERT_TRUE(participant_profile);
    ASSERT_TRUE(publisher_profile);
    ASSERT_TRUE(subscriber_profile);
    ASSERT_TRUE(topic_data);
}

TEST_F(XMLParserTests, Types)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("test_xml_profile.xml", root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            participant_profile = true;
        }
        else if (profile->getType() == NodeType::SUBSCRIBER)
        {
            publisher_profile = true;
        }
        else if (profile->getType() == NodeType::PUBLISHER)
        {
            subscriber_profile = true;
        }
    }
    ASSERT_TRUE(participant_profile);
    ASSERT_TRUE(publisher_profile);
    ASSERT_TRUE(subscriber_profile);
}

TEST_F(XMLParserTests, TypesBuffer)
{
    std::ifstream inFile;
    inFile.open("test_xml_profile.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            participant_profile = true;
        }
        else if (profile->getType() == NodeType::SUBSCRIBER)
        {
            publisher_profile = true;
        }
        else if (profile->getType() == NodeType::PUBLISHER)
        {
            subscriber_profile = true;
        }
    }
    ASSERT_TRUE(participant_profile);
    ASSERT_TRUE(publisher_profile);
    ASSERT_TRUE(subscriber_profile);
}

TEST_F(XMLParserTests, DurationCheck)
{
    std::unique_ptr<BaseNode> root;
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};
    const std::string profile_name2{"test_publisher_profile"};
    const std::string profile_name3{"test_subscriber_profile"};

    ASSERT_EQ(XMLParser::loadXML("test_xml_duration_profile.xml", root), XMLP_ret::XML_OK);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    xmlparser::PublisherAttributes publisher_atts;
    bool publisher_profile = false;
    xmlparser::SubscriberAttributes subscriber_atts;
    bool subscriber_profile = false;
    for (const auto& profile : root->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::ParticipantAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
            {
                participant_atts    = *data_node->get();
                participant_profile = true;
            }
        }
        else if (profile->getType() == NodeType::PUBLISHER)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::PublisherAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name2))
            {
                publisher_atts    = *data_node->get();
                publisher_profile = true;
            }
        }
        else if (profile->getType() == NodeType::SUBSCRIBER)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::SubscriberAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name3))
            {
                subscriber_atts    = *data_node->get();
                subscriber_profile = true;
            }
        }
    }
    ASSERT_TRUE(participant_profile);
    EXPECT_EQ(participant_atts.rtps.builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(participant_atts.rtps.builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(participant_atts.rtps.builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);

    ASSERT_TRUE(publisher_profile);
    EXPECT_EQ(publisher_atts.qos.m_deadline.period.seconds, 15);
    EXPECT_EQ(publisher_atts.qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(publisher_atts.qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(publisher_atts.qos.m_liveliness.announcement_period, dds::c_TimeInfinite);
    EXPECT_EQ(publisher_atts.qos.m_latencyBudget.duration.seconds, 10);

    ASSERT_TRUE(subscriber_profile);
    EXPECT_EQ(subscriber_atts.qos.m_deadline.period, dds::c_TimeInfinite);
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.lease_duration, dds::c_TimeInfinite);
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.announcement_period.seconds, 0);
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.announcement_period.nanosec, 0u);
    EXPECT_EQ(subscriber_atts.qos.m_latencyBudget.duration.seconds, 20);
}

/*
 * Checks the XML validated data parsing
 */
TEST_F(XMLParserTests, Data)
{
    std::unique_ptr<BaseNode> root;
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};

    ASSERT_EQ(XMLParser::loadXML("test_xml_profile.xml", root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::ParticipantAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
            {
                participant_atts    = *data_node->get();
                participant_profile = true;
            }
        }
    }

    ASSERT_TRUE(participant_profile);
    EXPECT_EQ(participant_atts.domainId, 123u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin           = rtps_atts.builtin;
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
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
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
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
    EXPECT_EQ(rtps_atts.userData, std::vector<octet>({0x56, 0x30, 0x0, 0xce}));
}

/*
 * Checks the data parsing (with deprecated but supported elements)
 */
TEST_F(XMLParserTests, DataDeprecated)
{
    std::unique_ptr<BaseNode> root;
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};

    ASSERT_EQ(XMLParser::loadXML("test_xml_deprecated.xml", root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::ParticipantAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
            {
                participant_atts    = *data_node->get();
                participant_profile = true;
            }
        }
    }

    ASSERT_TRUE(participant_profile);
    EXPECT_EQ(participant_atts.domainId, 2019102u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin           = rtps_atts.builtin;
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
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
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
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
    EXPECT_EQ(rtps_atts.userData, std::vector<octet>({0x56, 0x30, 0x0, 0xce}));
}

TEST_F(XMLParserTests, DataBuffer)
{
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};
    std::ifstream inFile;
    inFile.open("test_xml_profile.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::ParticipantAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
            {
                participant_atts    = *data_node->get();
                participant_profile = true;
            }
        }
    }

    ASSERT_TRUE(participant_profile);
    EXPECT_EQ(participant_atts.domainId, 123u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin           = rtps_atts.builtin;
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
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
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
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
    EXPECT_EQ(rtps_atts.userData, std::vector<octet>({0x56, 0x30, 0x0, 0xce}));
}

TEST_F(XMLParserTests, DataBufferDeprecated)
{
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};
    std::ifstream inFile;
    inFile.open("test_xml_deprecated.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    xmlparser::ParticipantAttributes participant_atts;
    bool participant_profile = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<xmlparser::ParticipantAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
            {
                participant_atts    = *data_node->get();
                participant_profile = true;
            }
        }
    }

    ASSERT_TRUE(participant_profile);
    EXPECT_EQ(participant_atts.domainId, 2019102u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin           = rtps_atts.builtin;
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
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
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
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
    EXPECT_EQ(rtps_atts.userData, std::vector<octet>({0x56, 0x30, 0x0, 0xce}));
}

/*
 * This test checks The return of the loadXMLProfiles method when a correct xml is parsed
 */
TEST_F(XMLParserTests, loadXMLProfiles)
{

    xmlparser::up_base_node_t root_node;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    const char* xml =
            "\
            <profiles>\
                <data_writer profile_name=\"test_publisher_profile\"\
                is_default_profile=\"true\">\
                    <qos>\
                        <durability>\
                            <kind>TRANSIENT_LOCAL</kind>\
                        </durability>\
                    </qos>\
                </data_writer>\
                <data_reader profile_name=\"test_subscriber_profile\" is_default_profile=\"true\">\
                    <historyMemoryPolicy>PREALLOCATED_WITH_REALLOC</historyMemoryPolicy>\
                    <userDefinedID>13</userDefinedID>\
                    <entityID>31</entityID>\
                </data_reader>\
            </profiles>\
            ";

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::loadXMLProfiles(*titleElement, root_node));

}

/*
 * This test checks the return of the parseXMLTransportData method  and the storage of the values in the XMLProfileManager
 * xml is parsed
 * 1. Check the correct parsing of a UDP transport descriptor for both v4 and v6
 * 2. Check the correct parsing of a TCP transport descriptor for both v4 and v6
 * 3. Check the correct parsing of a SHM transport descriptor
 */
TEST_F(XMLParserTests, parseXMLTransportData)
{
    using namespace eprosima::fastdds::rtps;

    ThreadSettings modified_thread_settings;
    modified_thread_settings.scheduling_policy = 12;
    modified_thread_settings.priority = 12;
    modified_thread_settings.affinity = 12;
    modified_thread_settings.stack_size = 12;

    // Test UDPv4 and UDPv6
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        const char* xml_p =
                "\
                <transport_descriptor>\
                    <transport_id>TransportId1</transport_id>\
                    <type>UDPv%s</type>\
                    <sendBufferSize>8192</sendBufferSize>\
                    <receiveBufferSize>8192</receiveBufferSize>\
                    <TTL>250</TTL>\
                    <non_blocking_send>false</non_blocking_send>\
                    <maxMessageSize>16384</maxMessageSize>\
                    <maxInitialPeersRange>100</maxInitialPeersRange>\
                    <interfaceWhiteList>\
                        <address>192.168.1.41</address>\
                        <address>127.0.0.1</address>\
                        <interface>wlp0s20f3</interface>\
                        <interface>lo</interface>\
                    </interfaceWhiteList>\
                    <netmask_filter>ON</netmask_filter>\
                    <interfaces>\
                        <allowlist>\
                            <interface name=\"wlp59s0\" netmask_filter=\"ON\"/>\
                            <interface name=\"127.0.0.1\" netmask_filter=\"AUTO\"/>\
                        </allowlist>\
                        <blocklist>\
                            <interface name=\"docker0\"/>\
                        </blocklist>\
                    </interfaces>\
                    <wan_addr>80.80.55.44</wan_addr>\
                    <output_port>5101</output_port>\
                    <keep_alive_frequency_ms>5000</keep_alive_frequency_ms>\
                    <keep_alive_timeout_ms>25000</keep_alive_timeout_ms>\
                    <max_logical_port>9000</max_logical_port>\
                    <logical_port_range>100</logical_port_range>\
                    <logical_port_increment>2</logical_port_increment>\
                    <listening_ports>\
                        <port>5100</port>\
                        <port>5200</port>\
                    </listening_ports>\
                    <calculate_crc>false</calculate_crc>\
                    <check_crc>false</check_crc>\
                    <enable_tcp_nodelay>false</enable_tcp_nodelay>\
                    <tls><!-- TLS Section --></tls>\
                    <segment_size>262144</segment_size>\
                    <port_queue_capacity>512</port_queue_capacity>\
                    <healthy_check_timeout_ms>1000</healthy_check_timeout_ms>\
                    <rtps_dump_file>rtsp_messages.log</rtps_dump_file>\
                    <default_reception_threads>\
                        <scheduling_policy>12</scheduling_policy>\
                        <priority>12</priority>\
                        <affinity>12</affinity>\
                        <stack_size>12</stack_size>\
                    </default_reception_threads>\
                    <reception_threads>\
                        <reception_thread port=\"12345\">\
                            <scheduling_policy>12</scheduling_policy>\
                            <priority>12</priority>\
                            <affinity>12</affinity>\
                            <stack_size>12</stack_size>\
                        </reception_thread>\
                        <reception_thread port=\"12346\">\
                            <scheduling_policy>12</scheduling_policy>\
                            <priority>12</priority>\
                            <affinity>12</affinity>\
                            <stack_size>12</stack_size>\
                        </reception_thread>\
                    </reception_threads>\
                </transport_descriptor>\
                ";
        constexpr size_t xml_len {3500};
        char xml[xml_len];

        // UDPv4
        snprintf(xml, xml_len, xml_p, "4");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
        std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> pUDPv4Desc =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::UDPv4TransportDescriptor>(
            xmlparser::XMLProfileManager::getTransportById("TransportId1"));
        EXPECT_EQ(pUDPv4Desc->sendBufferSize, 8192u);
        EXPECT_EQ(pUDPv4Desc->receiveBufferSize, 8192u);
        EXPECT_EQ(pUDPv4Desc->TTL, 250u);
        EXPECT_EQ(pUDPv4Desc->non_blocking_send, false);
        EXPECT_EQ(pUDPv4Desc->max_message_size(), 16384u);
        EXPECT_EQ(pUDPv4Desc->max_initial_peers_range(), 100u);
        EXPECT_EQ(pUDPv4Desc->interfaceWhiteList[0], "192.168.1.41");
        EXPECT_EQ(pUDPv4Desc->interfaceWhiteList[1], "127.0.0.1");
        EXPECT_EQ(pUDPv4Desc->interfaceWhiteList[2], "wlp0s20f3");
        EXPECT_EQ(pUDPv4Desc->interfaceWhiteList[3], "lo");
        EXPECT_EQ(pUDPv4Desc->netmask_filter, NetmaskFilterKind::ON);
        EXPECT_EQ(pUDPv4Desc->interface_allowlist[0], AllowedNetworkInterface("wlp59s0", NetmaskFilterKind::ON));
        EXPECT_EQ(pUDPv4Desc->interface_allowlist[1], AllowedNetworkInterface("127.0.0.1", NetmaskFilterKind::AUTO));
        EXPECT_EQ(pUDPv4Desc->interface_blocklist[0], BlockedNetworkInterface("docker0"));
        EXPECT_EQ(pUDPv4Desc->m_output_udp_socket, 5101u);
        EXPECT_EQ(pUDPv4Desc->default_reception_threads(), modified_thread_settings);
        EXPECT_EQ(pUDPv4Desc->get_thread_config_for_port(12345), modified_thread_settings);
        EXPECT_EQ(pUDPv4Desc->get_thread_config_for_port(12346), modified_thread_settings);

        xmlparser::XMLProfileManager::DeleteInstance();

        // UDPv6
        snprintf(xml, xml_len, xml_p, "6");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
        std::shared_ptr<eprosima::fastdds::rtps::UDPv6TransportDescriptor> pUDPv6Desc =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::UDPv6TransportDescriptor>(
            xmlparser::XMLProfileManager::getTransportById("TransportId1"));
        EXPECT_EQ(pUDPv6Desc->sendBufferSize, 8192u);
        EXPECT_EQ(pUDPv6Desc->receiveBufferSize, 8192u);
        EXPECT_EQ(pUDPv6Desc->TTL, 250u);
        EXPECT_EQ(pUDPv6Desc->non_blocking_send, false);
        EXPECT_EQ(pUDPv6Desc->max_message_size(), 16384u);
        EXPECT_EQ(pUDPv6Desc->max_initial_peers_range(), 100u);
        EXPECT_EQ(pUDPv6Desc->interfaceWhiteList[0], "192.168.1.41");
        EXPECT_EQ(pUDPv6Desc->interfaceWhiteList[1], "127.0.0.1");
        EXPECT_EQ(pUDPv6Desc->interfaceWhiteList[2], "wlp0s20f3");
        EXPECT_EQ(pUDPv6Desc->interfaceWhiteList[3], "lo");
        EXPECT_EQ(pUDPv6Desc->netmask_filter, NetmaskFilterKind::ON);
        EXPECT_EQ(pUDPv6Desc->interface_allowlist[0], AllowedNetworkInterface("wlp59s0", NetmaskFilterKind::ON));
        EXPECT_EQ(pUDPv6Desc->interface_allowlist[1], AllowedNetworkInterface("127.0.0.1", NetmaskFilterKind::AUTO));
        EXPECT_EQ(pUDPv6Desc->interface_blocklist[0], BlockedNetworkInterface("docker0"));
        EXPECT_EQ(pUDPv6Desc->m_output_udp_socket, 5101u);
        EXPECT_EQ(pUDPv6Desc->default_reception_threads(), modified_thread_settings);
        EXPECT_EQ(pUDPv6Desc->get_thread_config_for_port(12345), modified_thread_settings);
        EXPECT_EQ(pUDPv6Desc->get_thread_config_for_port(12346), modified_thread_settings);
        xmlparser::XMLProfileManager::DeleteInstance();
    }

    // Test TCPv4 and TCPv6
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        const char* xml_p =
                "\
                <transport_descriptor>\
                    <transport_id>TransportId1</transport_id>\
                    <type>TCPv%s</type>\
                    <sendBufferSize>8192</sendBufferSize>\
                    <receiveBufferSize>8192</receiveBufferSize>\
                    <TTL>250</TTL>\
                    <maxMessageSize>16384</maxMessageSize>\
                    <maxInitialPeersRange>100</maxInitialPeersRange>\
                    <interfaceWhiteList>\
                        <address>192.168.1.41</address>\
                        <interface>lo</interface>\
                    </interfaceWhiteList>\
                    <netmask_filter>ON</netmask_filter>\
                    <interfaces>\
                        <allowlist>\
                            <interface name=\"wlp59s0\" netmask_filter=\"ON\"/>\
                            <interface name=\"127.0.0.1\" netmask_filter=\"AUTO\"/>\
                        </allowlist>\
                        <blocklist>\
                            <interface name=\"docker0\"/>\
                        </blocklist>\
                    </interfaces>\
                    <wan_addr>80.80.55.44</wan_addr>\
                    <keep_alive_frequency_ms>5000</keep_alive_frequency_ms>\
                    <keep_alive_timeout_ms>25000</keep_alive_timeout_ms>\
                    <max_logical_port>9000</max_logical_port>\
                    <logical_port_range>100</logical_port_range>\
                    <logical_port_increment>2</logical_port_increment>\
                    <listening_ports>\
                        <port>5100</port>\
                        <port>5200</port>\
                    </listening_ports>\
                    <calculate_crc>false</calculate_crc>\
                    <check_crc>false</check_crc>\
                    <enable_tcp_nodelay>false</enable_tcp_nodelay>\
                    <non_blocking_send>true</non_blocking_send>\
                    <tcp_negotiation_timeout>100</tcp_negotiation_timeout>\
                    <tls><!-- TLS Section --></tls>\
                    <keep_alive_thread>\
                        <scheduling_policy>12</scheduling_policy>\
                        <priority>12</priority>\
                        <affinity>12</affinity>\
                        <stack_size>12</stack_size>\
                    </keep_alive_thread>\
                    <accept_thread>\
                        <scheduling_policy>12</scheduling_policy>\
                        <priority>12</priority>\
                        <affinity>12</affinity>\
                        <stack_size>12</stack_size>\
                    </accept_thread>\
                    <default_reception_threads>\
                        <scheduling_policy>12</scheduling_policy>\
                        <priority>12</priority>\
                        <affinity>12</affinity>\
                        <stack_size>12</stack_size>\
                    </default_reception_threads>\
                    <reception_threads>\
                        <reception_thread port=\"12345\">\
                            <scheduling_policy>12</scheduling_policy>\
                            <priority>12</priority>\
                            <affinity>12</affinity>\
                            <stack_size>12</stack_size>\
                        </reception_thread>\
                        <reception_thread port=\"12346\">\
                            <scheduling_policy>12</scheduling_policy>\
                            <priority>12</priority>\
                            <affinity>12</affinity>\
                            <stack_size>12</stack_size>\
                        </reception_thread>\
                    </reception_threads>\
                </transport_descriptor>\
                ";
        constexpr size_t xml_len {4000};
        char xml[xml_len];

        // TCPv4
        snprintf(xml, xml_len, xml_p, "4");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
        std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> pTCPv4Desc =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPv4TransportDescriptor>(
            xmlparser::XMLProfileManager::getTransportById("TransportId1"));
        EXPECT_EQ(pTCPv4Desc->sendBufferSize, 8192u);
        EXPECT_EQ(pTCPv4Desc->receiveBufferSize, 8192u);
        EXPECT_EQ(pTCPv4Desc->TTL, 250u);
        EXPECT_EQ(pTCPv4Desc->max_message_size(), 16384u);
        EXPECT_EQ(pTCPv4Desc->max_initial_peers_range(), 100u);
        EXPECT_EQ(pTCPv4Desc->interfaceWhiteList[0], "192.168.1.41");
        EXPECT_EQ(pTCPv4Desc->interfaceWhiteList[1], "lo");
        EXPECT_EQ(pTCPv4Desc->netmask_filter, NetmaskFilterKind::ON);
        EXPECT_EQ(pTCPv4Desc->interface_allowlist[0], AllowedNetworkInterface("wlp59s0", NetmaskFilterKind::ON));
        EXPECT_EQ(pTCPv4Desc->interface_allowlist[1], AllowedNetworkInterface("127.0.0.1", NetmaskFilterKind::AUTO));
        EXPECT_EQ(pTCPv4Desc->interface_blocklist[0], BlockedNetworkInterface("docker0"));
        EXPECT_EQ(pTCPv4Desc->wan_addr[0], (octet)80);
        EXPECT_EQ(pTCPv4Desc->wan_addr[1], (octet)80);
        EXPECT_EQ(pTCPv4Desc->wan_addr[2], (octet)55);
        EXPECT_EQ(pTCPv4Desc->wan_addr[3], (octet)44);
        EXPECT_EQ(pTCPv4Desc->keep_alive_frequency_ms, 5000u);
        EXPECT_EQ(pTCPv4Desc->keep_alive_timeout_ms, 25000u);
        EXPECT_EQ(pTCPv4Desc->max_logical_port, 9000u);
        EXPECT_EQ(pTCPv4Desc->logical_port_range, 100u);
        EXPECT_EQ(pTCPv4Desc->logical_port_increment, 2u);
        EXPECT_EQ(pTCPv4Desc->logical_port_increment, 2u);
        EXPECT_EQ(pTCPv4Desc->listening_ports[0], 5100u);
        EXPECT_EQ(pTCPv4Desc->listening_ports[1], 5200u);
        EXPECT_EQ(pTCPv4Desc->keep_alive_thread, modified_thread_settings);
        EXPECT_EQ(pTCPv4Desc->non_blocking_send, true);
        EXPECT_EQ(pTCPv4Desc->accept_thread, modified_thread_settings);
        EXPECT_EQ(pTCPv4Desc->tcp_negotiation_timeout, 100u);
        EXPECT_EQ(pTCPv4Desc->default_reception_threads(), modified_thread_settings);
        EXPECT_EQ(pTCPv4Desc->get_thread_config_for_port(12345), modified_thread_settings);
        EXPECT_EQ(pTCPv4Desc->get_thread_config_for_port(12346), modified_thread_settings);
        xmlparser::XMLProfileManager::DeleteInstance();

        // TCPv6
        snprintf(xml, xml_len, xml_p, "6");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
        std::shared_ptr<eprosima::fastdds::rtps::TCPv6TransportDescriptor> pTCPv6Desc =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPv6TransportDescriptor>(
            xmlparser::XMLProfileManager::getTransportById("TransportId1"));
        EXPECT_EQ(pTCPv6Desc->sendBufferSize, 8192u);
        EXPECT_EQ(pTCPv6Desc->receiveBufferSize, 8192u);
        EXPECT_EQ(pTCPv6Desc->TTL, 250u);
        EXPECT_EQ(pTCPv6Desc->max_message_size(), 16384u);
        EXPECT_EQ(pTCPv6Desc->max_initial_peers_range(), 100u);
        EXPECT_EQ(pTCPv6Desc->interfaceWhiteList[0], "192.168.1.41");
        EXPECT_EQ(pTCPv6Desc->interfaceWhiteList[1], "lo");
        EXPECT_EQ(pTCPv6Desc->netmask_filter, NetmaskFilterKind::ON);
        EXPECT_EQ(pTCPv6Desc->interface_allowlist[0], AllowedNetworkInterface("wlp59s0", NetmaskFilterKind::ON));
        EXPECT_EQ(pTCPv6Desc->interface_allowlist[1], AllowedNetworkInterface("127.0.0.1", NetmaskFilterKind::AUTO));
        EXPECT_EQ(pTCPv6Desc->interface_blocklist[0], BlockedNetworkInterface("docker0"));
        EXPECT_EQ(pTCPv6Desc->keep_alive_frequency_ms, 5000u);
        EXPECT_EQ(pTCPv6Desc->keep_alive_timeout_ms, 25000u);
        EXPECT_EQ(pTCPv6Desc->max_logical_port, 9000u);
        EXPECT_EQ(pTCPv6Desc->logical_port_range, 100u);
        EXPECT_EQ(pTCPv6Desc->logical_port_increment, 2u);
        EXPECT_EQ(pTCPv6Desc->logical_port_increment, 2u);
        EXPECT_EQ(pTCPv6Desc->listening_ports[0], 5100u);
        EXPECT_EQ(pTCPv6Desc->listening_ports[1], 5200u);
        EXPECT_EQ(pTCPv6Desc->keep_alive_thread, modified_thread_settings);
        EXPECT_EQ(pTCPv6Desc->non_blocking_send, true);
        EXPECT_EQ(pTCPv6Desc->accept_thread, modified_thread_settings);
        EXPECT_EQ(pTCPv6Desc->tcp_negotiation_timeout, 100u);
        EXPECT_EQ(pTCPv6Desc->default_reception_threads(), modified_thread_settings);
        EXPECT_EQ(pTCPv6Desc->get_thread_config_for_port(12345), modified_thread_settings);
        EXPECT_EQ(pTCPv6Desc->get_thread_config_for_port(12346), modified_thread_settings);
        xmlparser::XMLProfileManager::DeleteInstance();
    }

    // SHM
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        const char* xml =
                "\
                <transport_descriptor>\
                    <transport_id>TransportId1</transport_id>\
                    <type>SHM</type>\
                    <segment_size>262144</segment_size>\
                    <port_queue_capacity>512</port_queue_capacity>\
                    <healthy_check_timeout_ms>1000</healthy_check_timeout_ms>\
                    <rtps_dump_file>rtsp_messages.log</rtps_dump_file>\
                    <maxMessageSize>16384</maxMessageSize>\
                    <maxInitialPeersRange>100</maxInitialPeersRange>\
                    <default_reception_threads>\
                        <scheduling_policy>12</scheduling_policy>\
                        <priority>12</priority>\
                        <affinity>12</affinity>\
                        <stack_size>12</stack_size>\
                    </default_reception_threads>\
                    <reception_threads>\
                        <reception_thread port=\"12345\">\
                            <scheduling_policy>12</scheduling_policy>\
                            <priority>12</priority>\
                            <affinity>12</affinity>\
                            <stack_size>12</stack_size>\
                        </reception_thread>\
                        <reception_thread port=\"12346\">\
                            <scheduling_policy>12</scheduling_policy>\
                            <priority>12</priority>\
                            <affinity>12</affinity>\
                            <stack_size>12</stack_size>\
                        </reception_thread>\
                    </reception_threads>\
                    <dump_thread>\
                        <scheduling_policy>12</scheduling_policy>\
                        <priority>12</priority>\
                        <affinity>12</affinity>\
                        <stack_size>12</stack_size>\
                    </dump_thread>\
                </transport_descriptor>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
        std::shared_ptr<SharedMemTransportDescriptor> pSHMDesc =
                std::dynamic_pointer_cast<SharedMemTransportDescriptor>(
            xmlparser::XMLProfileManager::getTransportById("TransportId1"));
        EXPECT_EQ(pSHMDesc->segment_size(), 262144u);
        EXPECT_EQ(pSHMDesc->port_queue_capacity(), 512u);
        EXPECT_EQ(pSHMDesc->healthy_check_timeout_ms(), 1000u);
        EXPECT_EQ(pSHMDesc->rtps_dump_file(), "rtsp_messages.log");
        EXPECT_EQ(pSHMDesc->max_message_size(), 16384u);
        EXPECT_EQ(pSHMDesc->max_initial_peers_range(), 100u);
        EXPECT_EQ(pSHMDesc->default_reception_threads(), modified_thread_settings);
        EXPECT_EQ(pSHMDesc->get_thread_config_for_port(12345), modified_thread_settings);
        EXPECT_EQ(pSHMDesc->get_thread_config_for_port(12346), modified_thread_settings);
        EXPECT_EQ(pSHMDesc->dump_thread(), modified_thread_settings);

        xmlparser::XMLProfileManager::DeleteInstance();
    }
}


/*
 * This test checks the return of the negative cases of th parseXMLTransportData method.
 * 1. Check an XMLP_ret::XML_ERROR return on an incorrectly formated parameter of every possible parameter of the
 * UDPv4, UDPv6, TCPv4, TCPv6, and SHM.
 * 2. Check the correct parsing of a TCP transport descriptor for birth v4 and v6
 * 3. Check the correct parsing of a SHM transport descriptor
 * 4. Check missing TransportID
 * 5. Check missing type
 * 6. Check wrong type
 */
TEST_F(XMLParserTests, parseXMLTransportData_NegativeClauses)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    std::string xml;
    std::vector<std::string> parameters_UDP =
    {
        "maxMessageSize",
        "maxInitialPeersRange",
        "sendBufferSize",
        "receiveBufferSize",
        "TTL",
        "non_blocking_send",
        "interfaceWhiteList",
        "netmask_filter",
        "interfaces",
        "output_port",
        "default_reception_threads",
        "reception_threads",
        "bad_element"
    };

    std::vector<std::string> parameters_TCP =
    {
        "maxMessageSize",
        "maxInitialPeersRange",
        "sendBufferSize",
        "receiveBufferSize",
        "TTL",
        "interfaceWhiteList",
        "netmask_filter",
        "interfaces",
        "keep_alive_frequency_ms",
        "keep_alive_timeout_ms",
        "max_logical_port",
        "logical_port_range",
        "logical_port_increment",
        "calculate_crc",
        "check_crc",
        "enable_tcp_nodelay",
        "non_blocking_send",
        "tls",
        "keep_alive_thread",
        "accept_thread",
        "tcp_negotiation_timeout",
        "default_reception_threads",
        "reception_threads",
        "bad_element"
    };

    std::vector<std::string> parameters_SHM =
    {
        "maxMessageSize",
        "maxInitialPeersRange",
        "segment_size",
        "port_queue_capacity",
        "healthy_check_timeout_ms",
        "rtps_dump_file",
        "default_reception_threads",
        "reception_threads",
        "dump_thread",
        "bad_element"
    };

    std::vector<std::string> parameters_TCPv4 = parameters_TCP;
    parameters_TCPv4.insert(parameters_TCPv4.end(), "wan_addr");

    std::map<std::string, std::vector<std::string>> transport_parameters =
    {
        {"UDPv4", parameters_UDP},
        {"UDPv6", parameters_UDP},
        {"TCPv4", parameters_TCPv4},
        {"TCPv6", parameters_TCP},
        {"SHM", parameters_SHM}
    };

    for (const auto& parameters : transport_parameters)
    {
        for (const auto& field : parameters.second)
        {
            xml =
                    "\
                    <transport_descriptor>\
                        <transport_id>TransportId1</transport_id>\
                        <type>" + parameters.first + "</type>\
                        <" + field + "><bad_element></bad_element></" + field +
                    ">\
                    </transport_descriptor>\
                    ";

            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
            xmlparser::XMLProfileManager::DeleteInstance();
        }

        if (parameters.first.substr(0, 3) == "TCP" )
        {
            xml =
                    "\
                    <transport_descriptor>\
                        <transport_id>TransportId1</transport_id>\
                        <type>" + parameters.first +
                    "</type>\
                        <listening_ports>\
                            <port>not_an_int</port>\
                        </listening_ports>\
                    </transport_descriptor>\
                    ";

            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
            xmlparser::XMLProfileManager::DeleteInstance();

            // Check empty pointer
            EXPECT_EQ(XMLP_ret::XML_ERROR,
                    XMLParserTest::parseXMLCommonTCPTransportData_wrapper(titleElement, nullptr));
        }
        else if (parameters.first == "SHM" )
        {
            xml = "<transport_descriptor></transport_descriptor>";
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
            titleElement = xml_doc.RootElement();
            // Check empty pointer
            EXPECT_EQ(XMLP_ret::XML_ERROR,
                    XMLParserTest::parseXMLCommonSharedMemTransportData_wrapper(titleElement, nullptr));
        }
    }

    // missing type tag
    xml =
            "\
            <transport_descriptor>\
                <transport_id>TransportId1</transport_id>\
            </transport_descriptor>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
    xmlparser::XMLProfileManager::DeleteInstance();

    // missing type value
    xml =
            "\
            <transport_descriptor>\
                <transport_id>TransportId1</transport_id>\
                <type></type>\
            </transport_descriptor>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
    xmlparser::XMLProfileManager::DeleteInstance();

    // invalid type
    xml =
            "\
            <transport_descriptor>\
                <transport_id>TransportId1</transport_id>\
                <type>bad_type</type>\
            </transport_descriptor>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
    xmlparser::XMLProfileManager::DeleteInstance();

    // missing id tag
    xml =
            "\
            <transport_descriptor>\
                <transport_id></transport_id>\
                <type>UDPv4</type>\
            </transport_descriptor>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
    xmlparser::XMLProfileManager::DeleteInstance();

    // missing id value
    xml =
            "\
            <transport_descriptor>\
                <type>UDPv4</type>\
            </transport_descriptor>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportData_wrapper(titleElement));
    xmlparser::XMLProfileManager::DeleteInstance();
}

/*
 * This test checks the return of the negative cases of th parseXMLTransportsProf method.
 * 1. Check an XMLP_ret::XML_ERROR return on an incorrect element inside the <transport_descriptor> tag
 */
TEST_F(XMLParserTests, parseXMLTransportsProfNegativeClauses)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    std::string xml =
            "\
            <transport_descriptors>\
                <transport_descriptor>\
                    <bad_element></bad_element>\
                </transport_descriptor>\
            </transport_descriptors>\
            ";

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTransportsProf_wrapper(titleElement));
}

/*
 * This test checks the return of the parseXMLConsumer method.
 * 1. Check the correct return parsing a StdoutConsumer.
 */
TEST_F(XMLParserTests, parseXMLStdoutConsumer)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // StdoutConsumer
        const char* xml =
                "\
                <consumer>\
                    <class>StdoutConsumer</class>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }
}

/*
 * This test checks the return of the parseXMLConsumer method.
 * 1. Check the correct return parsing a StdoutErrConsumer with default configuration.
 * 2. Check the correct return parsing a StdoutErrConsumer with a custom configuration.
 */
TEST_F(XMLParserTests, parseXMLStdoutErrConsumer)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    {
        // StdoutErrConsumer without properties
        const char* xml =
                "\
                <consumer>\
                    <class>StdoutErrConsumer</class>\
                </consumer>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }

    {
        // StdoutErrConsumer with properties
        const char* xml_p =
                "\
                <consumer>\
                    <class>StdoutErrConsumer</class>\
                    <property>\
                        <name>stderr_threshold</name>\
                        <value>Log::Kind::%s</value>\
                    </property>\
                </consumer>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> log_levels = {"Info", "Warning", "Error"};
        for (std::vector<std::string>::iterator it = log_levels.begin(); it != log_levels.end(); ++it)
        {
            snprintf(xml, xml_len, xml_p, (*it).c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
        }


    }
}

/*
 * This test checks the return of the parseXMLConsumer method.
 * 1. Check the correct return parsing a FileConsumer with default configuration.
 */
TEST_F(XMLParserTests, parseXMLFileConsumer)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // FileConsumer without properties
        const char* xml =
                "\
                <consumer>\
                    <class>FileConsumer</class>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }


    {
        // FileConsumer
        const char* xml =
                "\
                <consumer>\
                    <class>FileConsumer</class>\
                    <property>\
                        <name>filename</name>\
                        <value>execution.log</value>\
                    </property>\
                    <property>\
                        <name>append</name>\
                        <value>TRUE</value>\
                    </property>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }

}

/*
 * This test checks the return of the negative cases of the parseXMLConsumer method.
 * 1. Check a non-existant Consumer class.
 * 2. Check a StdoutErrConsumer with incorrect property values.
 * 3. Check a StdoutErrConsumer with std_threshold set twice.
 * 4. Check a StdoutErrConsumer with an incorrect property.
 * 5. Check a FileConsumer without a filename property.
 * 6. Check a FileConsumer without a value for the append property.
 * 7. Check a FileConsumer with an incorrect property.
 */
TEST_F(XMLParserTests, parseXMLConsumerNegativeClauses)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        Log::ClearConsumers();
        // Unknown consumer class
        const char* xml =
                "\
                <consumer>\
                    <class>UnknownConsumer</class>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }

    {
        Log::ClearConsumers();
        // StdoutErrConsumer with properties
        const char* xml =
                "\
                <consumer>\
                    <class>StdoutErrConsumer</class>\
                    <property>\
                        <name>stderr_threshold</name>\
                        <value>bad_value</value>\
                    </property>\
                </consumer>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_NOK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));

    }

    {
        Log::ClearConsumers();
        // StdoutErrConsumer with two stderr_threshold
        const char* xml =
                "\
                <consumer>\
                    <class>StdoutErrConsumer</class>\
                    <property>\
                        <name>stderr_threshold</name>\
                        <value>Log::Kind::Error</value>\
                    </property>\
                    <property>\
                        <name>stderr_threshold</name>\
                        <value>Log::Kind::Error</value>\
                    </property>\
                </consumer>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_NOK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));

    }

    {
        Log::ClearConsumers();
        // StdoutErrConsumer with wrong property name
        const char* xml =
                "\
                <consumer>\
                    <class>StdoutErrConsumer</class>\
                    <property>\
                        <name>bad_property</name>\
                    </property>\
                </consumer>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_NOK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));

    }

    {
        Log::ClearConsumers();
        // FileConsumer no filename
        const char* xml =
                "\
                <consumer>\
                    <class>FileConsumer</class>\
                    <property>\
                        <name>filename</name>\
                        <value></value>\
                    </property>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_NOK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }

    {
        Log::ClearConsumers();
        // FileConsumer no append value
        const char* xml =
                "\
                <consumer>\
                    <class>FileConsumer</class>\
                    <property>\
                        <name>append</name>\
                        <value></value>\
                    </property>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_NOK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }

    {
        Log::ClearConsumers();
        // FileConsumer bad property
        const char* xml =
                "\
                <consumer>\
                    <class>FileConsumer</class>\
                    <property>\
                        <name>bad_property</name>\
                    </property>\
                </consumer>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_NOK, XMLParserTest::parseXMLConsumer_wrapper(*titleElement));
    }

}

/*
 * This test checks the return of the parseLogConfig method.
 * 1. Check a consumer with a wrong class
 * 2. Check the use_default tag without TRUE and TRUE
 * 3. Check a wrong tag
 */
TEST_F(XMLParserTests, parseLogConfig)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // Bad parameters
        const char* xml_p =
                "\
                <log>\
                    <use_default>%s</use_default>\
                    <consumer>\
                        <class>%s</class>\
                    </consumer>\
                </log>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        // Check wrong class of consumer
        snprintf(xml, xml_len, xml_p, "FALSE", "wrong_class");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseLogConfig_wrapper(titleElement));

        // Check both values of use_default
        snprintf(xml, xml_len, xml_p, "TRUE", "StdoutConsumer");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseLogConfig_wrapper(titleElement));

        snprintf(xml, xml_len, xml_p, "FALSE", "StdoutConsumer");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseLogConfig_wrapper(titleElement));
    }

    {
        // Check bad tag
        const char* xml =
                "\
                <log>\
                    <bad_element></bad_element>\
                </log>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseLogConfig_wrapper(titleElement));

    }

}

/*
 * This test checks the return of the negative cases of the fillDataNode given a xmlparser::ParticipantAttributes DataNode
 * 1. Check passing a nullptr as if the XMLElement was wrongly parsed above
 * 2. Check missing DomainId value in tag
 * 3. Check bad values for all attributes
 * 4. Check a non existant attribute tag
 * 5. Check a non valid Easy Mode IP
 */
TEST_F(XMLParserTests, fillDataNodeParticipantNegativeClauses)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    up_participant_t participant_atts{new xmlparser::ParticipantAttributes};
    up_node_participant_t participant_node{new node_participant_t{NodeType::PARTICIPANT, std::move(participant_atts)}};

    // missing profile XMLElement
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(nullptr, *participant_node));

    {
        const char* xml_p =
                "\
                <participant profile_name=\"domainparticipant_profile_name\">\
                    %s\
                </participant>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        // Misssing DomainId Value
        snprintf(xml, xml_len, xml_p, "<domainId></domainId><rtps></rtps>");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *participant_node));
    }

    {
        // Wrong rtps child tags
        const char* xml_p =
                "\
                <participant profile_name=\"domainparticipant_profile_name\">\
                    <domainId>0</domainId>\
                    <rtps>\
                        %s\
                    </rtps>\
                </participant>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> parameters = {
            "<name></name>",
            "<prefix><bad_element></bad_element></prefix>",
            "<defaultUnicastLocatorList><bad_element></bad_element></defaultUnicastLocatorList>",
            "<defaultMulticastLocatorList><bad_element></bad_element></defaultMulticastLocatorList>",
            "<sendSocketBufferSize><bad_element></bad_element></sendSocketBufferSize>",
            "<listenSocketBufferSize><bad_element></bad_element></listenSocketBufferSize>",
            "<builtin><bad_element></bad_element></builtin>",
            "<port><bad_element></bad_element></port>",
            "<participantID><bad_element></bad_element></participantID>",
            "<easy_mode_ip><bad_element></bad_element></easy_mode_ip>",
            "<flow_controller_descriptor_list><bad_element></bad_element></flow_controller_descriptor_list>",
            "<userTransports><bad_element></bad_element></userTransports>",
            "<useBuiltinTransports><bad_element></bad_element></useBuiltinTransports>",
            "<propertiesPolicy><bad_element></bad_element></propertiesPolicy>",
            "<userData><bad_element></bad_element></userData>",
            "<allocation><bad_element></bad_element></allocation>",
            "<bad_element></bad_element>"
        };


        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it)
        {
            snprintf(xml, xml_len, xml_p, (*it).c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *participant_node));

        }

        // Check invalid easy_mode_ip value (not a valid IPv4 address)
        std::string invalid_easy_mode_ip = "<easy_mode_ip>Foo</easy_mode_ip>";
        snprintf(xml, xml_len, xml_p, invalid_easy_mode_ip.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *participant_node));
    }
}

/*
 * This test checks the correct functioning of the parseXML function when the <dds> root element does not exist.
 * 1. Check that elements <profiles>, <types> and <log> are read as root elements.
 * 2. Check that it triggers an error when reading a wrong element.
 */
TEST_F(XMLParserTests, parseXMLNoRoot)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;

    // Parametrized XML
    const char* xml_p =
            "\
            <%s>\
            </%s>\
            ";

    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Check that elements <profiles>, <types> and <log> are read as root elements.
    std::vector<std::string> elements {
        "profiles",
        "types",
        "log"
    };

    for (std::string e : elements)
    {
        snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    // Check that it triggers an error when reading a wrong element.
    snprintf(xml, xml_len, xml_p, "bad_root", "bad_root");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
}

/*
 * This test checks the correct functioning of the parseXML function for all xml child elements of <profile>
 * when the <profiles> element is an xml child element of the <dds> root element.
 * 1. Check that elements <library_settings>, <participant>, <publisher>, <data_writer>, <subscriber>, <data_reader>,
 * <topic>, <requester>, <replier>, <types>, and <log> are read as xml child elements of the <profiles> root element.
 * 2. Check that it triggers an error when reading a wrong element.
 */
TEST_F(XMLParserTests, parseXMLProfilesRoot)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;

    // Parametrized XML
    const char* xml_p =
            "\
            <dds>\
                <%s>\
                </%s>\
            </dds>\
            ";

    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Check that elements <library_settings>, <participant>, <publisher>, <data_writer>, <subscriber>, <data_reader>,
    // <topic>, <requester>, <replier>, <types>, and <log> are read as xml child elements of the <profiles>
    // root element.
    std::vector<std::string> elements_ok {
        "participant",
        "publisher",
        "data_writer",
        "subscriber",
        "data_reader",
        "topic",
        "types",
        "log"
    };
    for (std::string e : elements_ok)
    {
        snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    std::vector<std::string> elements_error {
        "library_settings",
        "requester",
        "replier"
    };
    for (std::string e : elements_error)
    {
        snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    // Check that it triggers an error when reading a wrong element.
    snprintf(xml, xml_len, xml_p, "bad_element", "bad_element");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
}

/*
 * This test checks the positive cases of the TLS configuration via XML.
 * 1. Check that parse_tls_config() return an XML_OK code for a valid configurations of <verify_paths>.
 */
TEST_F(XMLParserTests, parseTLSConfigPositiveClauses)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;
    tinyxml2::XMLElement* titleElement;
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();

    // Parametrized XML
    const char* xml =
            "\
            <tls>\
                <verify_paths>\
                    <verify_path>path_1</verify_path>\
                    <verify_path>path_2</verify_path>\
                </verify_paths>\
            </tls>\
            ";

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that parse_tls_config() return an XML_OK code for a valid configurations of <verify_paths>.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
}

/*
 * This test checks the negative cases of the TLS configuration via XML.
 * 1. Check that elements <password>, <private_key_file>, <rsa_private_key_file>, <cert_chain_file>, <tmp_dh_file>,
 * <verify_file>, <verify_depth>, <default_verify_path>, <bad_element>, <server_name>
 * return an xml error if their value is empty.
 * 2. Check all possible wrong configurations of <verify_paths>.
 * 3. Check all possible wrong configurations of <verify_mode>.
 * 4. Check all possible wrong configurations of <handshake_role>.
 * 5. Check all possible wrong configurations of <options>.
 */
TEST_F(XMLParserTests, parseTLSConfigNegativeClauses)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;
    tinyxml2::XMLElement* titleElement;
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Check that elements <password>, <private_key_file>, <rsa_private_key_file>, <cert_chain_file>, <tmp_dh_file>,
    // <verify_file>, <verify_depth>, <default_verify_path>, <bad_element>, <server_name>
    // return an xml error if their value is empty.
    {
        // Parametrized XML
        const char* xml_p =
                "\
                <tls>\
                    <%s></%s>\
                </tls>\
                ";

        std::vector<std::string> elements {
            "password",
            "private_key_file",
            "rsa_private_key_file",
            "server_name",
            "cert_chain_file",
            "tmp_dh_file",
            "verify_file",
            "verify_depth",
            "default_verify_path",
            "bad_element"
        };

        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        }
    }

    // Check all possible wrong configurations of <verify_paths>.
    {
        // Parametrized XML
        const char* xml_p_verify_paths =
                "\
                <tls>\
                    <verify_paths>\
                        <%s></%s>\
                    </verify_paths>\
                </tls>\
                ";

        std::vector<std::string> elements {"verify_path", "bad_element"};

        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p_verify_paths, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        }
    }

    // Check all possible wrong configurations of <verify_mode>.
    {
        // Parametrized XML
        const char* xml_p_verify_mode =
                "\
                <tls>\
                    <verify_mode>\
                        %s\
                    </verify_mode>\
                </tls>\
                ";

        std::vector<std::string> elements {
            "<verify></verify>",
            "<verify>bad_value</verify>",
            "<bad_element></bad_element>"
        };

        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p_verify_mode, e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        }
    }

    // Check all possible wrong configurations of <handshake_role>.
    {
        // Parametrized XML
        const char* xml_p_handshake_role =
                "\
                <tls>\
                    <handshake_role>%s</handshake_role>\
                </tls>\
                ";
        std::vector<std::string> elements {"", "bad_mode"};

        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p_handshake_role, e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        }
    }

    // Check all possible wrong configurations of <options>.
    {
        // Parametrized XML
        const char* xml_p_options =
                "\
                <tls>\
                    <options>\
                        %s\
                    </options>\
                </tls>\
                ";

        std::vector<std::string> elements {
            "<option></option>",
            "<option>bad_option</option>",
            "<bad_element></bad_element>"
        };

        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p_options, e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        }
    }
}

/*
 * This test checks all possible settings of the TLS handshake role (<handshake_role>) that return an XML OK code.
 * 1. Check that the DEFAULT setting return an xml ok code and is set correctly.
 * 2. Check that the CLIENT setting return an xml ok code and is set correctly.
 * 3. Check that the SERVER setting return an xml ok code and is set correctly.
 */
TEST_F(XMLParserTests, parseTLSConfigHandshakeRole)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;
    tinyxml2::XMLElement* titleElement;
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();

    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Parametrized XML
    const char* xml_p =
            "\
            <tls>\
                <handshake_role>%s</handshake_role>\
            </tls>\
            ";

    // Check that the DEFAULT setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p, "DEFAULT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.handshake_role,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::DEFAULT);
    }

    // Check that the CLIENT setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p, "CLIENT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.handshake_role,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::CLIENT);
    }

    // Check that the SERVER setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p, "SERVER");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.handshake_role,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::SERVER);
    }
}

/*
 * This test checks all possible settings of the TLS verify mode (<verify_mode>) that return an XML OK code.
 * 1. Check that the VERIFY_NONE setting return an xml ok code and is set correctly.
 * 2. Check that the VERIFY_PEER setting return an xml ok code and is set correctly.
 * 3. Check that the VERIFY_FAIL_IF_NO_PEER_CERT setting return an xml ok code and is set correctly.
 * 4. Check that the VERIFY_CLIENT_ONCE setting return an xml ok code and is set correctly.
 */
TEST_F(XMLParserTests, parseTLSConfigVerifyMode)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;
    tinyxml2::XMLElement* titleElement;
    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Parametrized XML
    const char* xml_p_verify_mode =
            "\
            <tls>\
                <verify_mode>\
                    <verify>%s</verify>\
                </verify_mode>\
            </tls>\
            ";

    // Check that the VERIFY_NONE setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "VERIFY_NONE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
                std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));

        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_NONE);
    }

    // Check that the VERIFY_PEER setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "VERIFY_PEER");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
                std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));

        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_PEER);
    }

    // Check that the VERIFY_FAIL_IF_NO_PEER_CERT setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "VERIFY_FAIL_IF_NO_PEER_CERT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
                std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));

        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT);
    }

    // Check that the VERIFY_CLIENT_ONCE setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "VERIFY_CLIENT_ONCE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
                std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode,
                eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_CLIENT_ONCE);
    }
}

/*
 * This test checks all possible settings of the TLS options (<options>) that return an XML OK code.
 * 1. Check that the DEFAULT_WORKAROUNDS setting return an xml ok code and is set correctly.
 * 2. Check that the NO_COMPRESSION setting return an xml ok code and is set correctly.
 * 3. Check that the NO_SSLV2 setting return an xml ok code and is set correctly.
 * 4. Check that the NO_SSLV3 setting return an xml ok code and is set correctly.
 * 5. Check that the NO_TLSV1 setting return an xml ok code and is set correctly.
 * 6. Check that the NO_TLSV1_1 setting return an xml ok code and is set correctly.
 * 7. Check that the NO_TLSV1_2 setting return an xml ok code and is set correctly.
 * 8. Check that the NO_TLSV1_3 setting return an xml ok code and is set correctly.
 * 9. Check that the SINGLE_DH_USE setting return an xml ok code and is set correctly.
 */
TEST_F(XMLParserTests, parseTLSConfigOptions)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;
    tinyxml2::XMLElement* titleElement;
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Parametrized XML
    const char* xml_p_verify_mode =
            "\
            <tls>\
                <options>\
                    <option>%s</option>\
                </options>\
            </tls>\
            ";

    // Check that the DEFAULT_WORKAROUNDS setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "DEFAULT_WORKAROUNDS");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    DEFAULT_WORKAROUNDS));
    }

    // Check that the NO_COMPRESSION setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_COMPRESSION");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_COMPRESSION));
    }

    // Check that the NO_SSLV2 setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_SSLV2");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_SSLV2));
    }

    // Check that the NO_SSLV3 setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_SSLV3");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_SSLV3));
    }

    // Check that the NO_TLSV1 setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_TLSV1");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_TLSV1));
    }

    // Check that the NO_TLSV1_1 setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_TLSV1_1");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_TLSV1_1));
    }

    // Check that the NO_TLSV1_2 setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_TLSV1_2");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_TLSV1_2));
    }

    // Check that the NO_TLSV1_3 setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "NO_TLSV1_3");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    NO_TLSV1_3));
    }

    // Check that the SINGLE_DH_USE setting return an xml ok code and is set correctly.
    {
        snprintf(xml, xml_len, xml_p_verify_mode, "SINGLE_DH_USE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
            descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions::
                    SINGLE_DH_USE));
    }
}

/*
 * This test checks the correct functioning of the parseProfile function for all xml child elements of <profile>.
 * 1. Check that elements <transport_descriptors>, <library_settings>, <participant>, <publisher>, <data_writer>,
 * <subscriber>, <data_reader>, <topic>, <requester>, <replier>, and <types> are read as xml child elements of the
 * <profiles> root element.
 * 2. Check that it triggers an error when reading a wrong element.
 */
TEST_F(XMLParserTests, parseProfiles)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    std::unique_ptr<BaseNode> profilesNode;

    // Parametrized XML
    const char* xml_p =
            "\
            <profiles>\
                <%s>\
                </%s>\
            </profiles>\
            ";

    constexpr size_t xml_len {600};
    char xml[xml_len];

    std::vector<std::string> elements_ok {
        "transport_descriptors",
        "publisher",
        "data_writer",
        "subscriber",
        "data_reader",
        "topic",
        "types"
        // "qos_profile",
        // "application",
        // "type"
    };
    for (std::string e : elements_ok)
    {
        snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        profilesNode.reset(new BaseNode{ NodeType::PROFILES });
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseProfiles_wrapper(titleElement, *profilesNode));
    }

    const char* xml_p_error =
            "\
        <profiles>\
            <%s>\
                <bad_element></bad_element>\
            </%s>\
        </profiles>\
        ";

    std::vector<std::string> elements_error {
        "library_settings",
        "participant",
        "requester",
        "replier",
        "publisher",
        "data_writer",
        "subscriber",
        "data_reader",
        "topic",
        "bad_element"
    };

    for (std::string e : elements_error)
    {
        snprintf(xml, xml_len, xml_p_error, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        profilesNode.reset(new BaseNode{ NodeType::PROFILES });
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseProfiles_wrapper(titleElement, *profilesNode));
    }
}

/*
 * This test checks the correct functioning of the parseProfile function for all unsupported xml child elements of
 * <profile>.
 * 1. Check that unsupported elements <qos_profile>, <application>, and <type> are read as xml child elements of the
 * <profiles> root element.
 * 2. Check that it outputs a Log Error when reading an unsupported element.
 */
TEST_F(XMLParserTests, parseUnsupportedProfiles)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    std::unique_ptr<BaseNode> profilesNode;

    mock_consumer = new eprosima::fastdds::dds::MockConsumer();

    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(mock_consumer));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("(XMLPARSER)"));

    // Parametrized XML
    const char* xml_p =
            "\
            <profiles>\
                <%s>\
                </%s>\
            </profiles>\
            ";

    constexpr size_t xml_len {600};
    char xml[xml_len];

    std::vector<std::string> elements {
        "qos_profile",
        "application",
        "type"
    };
    for (std::string e : elements)
    {
        snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        profilesNode.reset(new BaseNode{ NodeType::PROFILES });
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseProfiles_wrapper(titleElement, *profilesNode));
    }

    helper_block_for_at_least_entries(3);
    auto consumed_entries = mock_consumer->ConsumedEntries();
    // Expect 3 log errors.
    uint32_t num_errors = 0;
    for (const auto& entry : consumed_entries)
    {
        if (entry.kind == eprosima::fastdds::dds::Log::Kind::Error)
        {
            num_errors++;
        }
    }
    EXPECT_EQ(num_errors, 3u);
}

/*
 * This test checks the negative case in the <library_settings> xml element.
 */
TEST_F(XMLParserTests, parseXMLLibrarySettingsNegativeClauses)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <library_settings>\
                <intraprocess_delivery></intraprocess_delivery>\
            </library_settings>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that it returns an xml error when <intraprocess_delivery> is empty
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLLibrarySettings_wrapper(titleElement));
}

/*
 * This test checks an error is triggered when the xml file name is empty.
 */
TEST_F(XMLParserTests, loadXMLEmptyFileName)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParser::loadXML("", root));
}

/*
 * This test checks the negative case in the fillDataNode function when it refers to a publisher node.
 * 1. Check that an XML_ERROR is triggered when the xml element is nullptr.
 * 2. Check that an XML_ERROR is triggered when the <publisher> element does not contains any attributes.
 */
TEST_F(XMLParserTests, fillDataNodePublisherNegativeClauses)
{
    std::unique_ptr<xmlparser::PublisherAttributes> publisher_atts{new xmlparser::PublisherAttributes};
    std::unique_ptr<DataNode<xmlparser::PublisherAttributes>> publisher_node{
        new DataNode<xmlparser::PublisherAttributes>{ NodeType::PUBLISHER, std::move(publisher_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(nullptr, *publisher_node));

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <publisher>\
                <bad_element></bad_element>\
            </publisher>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that an XML_ERROR is triggered when the <publisher> element does not contains any attributes.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *publisher_node));
}

/*
 * This test checks the negative case in the fillDataNode function when it refers to a subscriber node.
 * 1. Check that an XML_ERROR is triggered when the xml element is nullptr.
 * 2. Check that an XML_ERROR is triggered when the <subscriber> element does not contains any attributes.
 */
TEST_F(XMLParserTests, fillDataNodeSubscriberNegativeClauses)
{
    std::unique_ptr<xmlparser::SubscriberAttributes> subscriber_atts{new xmlparser::SubscriberAttributes};
    std::unique_ptr<DataNode<xmlparser::SubscriberAttributes>> subscriber_node{
        new DataNode<xmlparser::SubscriberAttributes>{ NodeType::SUBSCRIBER, std::move(subscriber_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(nullptr, *subscriber_node));

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <subscriber>\
                <bad_element></bad_element>\
            </subscriber>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that an XML_ERROR is triggered when the <subscriber> element does not contains any attributes.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *subscriber_node));
}

/*
 * This test checks the negative case in the fillDataNode function when it refers to a topic node.
 * 1. Check that an XML_ERROR is triggered when the xml element is nullptr.
 * 2. Check that an XML_ERROR is triggered when the <topic> element does not contains any attributes.
 */
TEST_F(XMLParserTests, fillDataNodeTopicNegativeClauses)
{
    std::unique_ptr<TopicAttributes> topic_atts{new TopicAttributes};
    std::unique_ptr<DataNode<TopicAttributes>> topic_node{
        new DataNode<TopicAttributes>{ NodeType::TOPIC, std::move(topic_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(nullptr, *topic_node));

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <topic>\
                <bad_element></bad_element>\
            </topic>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that an XML_ERROR is triggered when the <topic> element does not contains any attributes.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *topic_node));
}

/*
 * This test checks the negative case in the fillDataNode function when it refers to a requester node.
 * 1. Check that fillDataNode() returns an XML_ERROR when the xml element is nullptr.
 * 2. Check that fillDataNode() returns an XML_ERROR when the profile_name attribute is missing.
 * 3. Check that fillDataNode() returns an XML_ERROR when the service_name attribute is missing.
 * 4. Check that fillDataNode() returns an XML_ERROR when the request_type attribute is missing.
 * 5. Check that fillDataNode() returns an XML_ERROR when the reply_type attribute is missing.
 * 5. Check that fillDataNode() returns an XML_ERROR when the <request_topic_name>, <reply_topic_name>, <publisher>, and <subscriber>
 *    does not contains a valid xml element.
 * 6. Check that fillDataNode() returns an XML_ERROR when child xml element of <requester> is not valid.
 */
TEST_F(XMLParserTests, fillDataNodeRequesterNegativeClauses)
{
    std::unique_ptr<xmlparser::RequesterAttributes> requester_atts_error{new xmlparser::RequesterAttributes};
    std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node_error{
        new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts_error) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(nullptr, *requester_node_error));

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Check that fillDataNode() returns an XML_ERROR when the profile_name attribute is missing.
    {
        const char* xml =
                "\
                <requester>\
                </requester>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::RequesterAttributes> requester_atts{new xmlparser::RequesterAttributes};
        std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node{
            new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *requester_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the service_name attribute is missing.
    {
        const char* xml =
                "\
                <requester profile_name=\"test_requester_profile\">\
                </requester>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::RequesterAttributes> requester_atts{new xmlparser::RequesterAttributes};
        std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node{
            new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *requester_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the request_type attribute is missing.
    {
        const char* xml =
                "\
                <requester profile_name=\"test_requester_profile\"\
                           service_name=\"service_name\">\
                </requester>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::RequesterAttributes> requester_atts{new xmlparser::RequesterAttributes};
        std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node{
            new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *requester_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the reply_type attribute is missing.
    {
        const char* xml =
                "\
                <requester profile_name=\"test_requester_profile\"\
                           service_name=\"service_name\"\
                           request_type=\"request_type\">\
                </requester>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::RequesterAttributes> requester_atts{new xmlparser::RequesterAttributes};
        std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node{
            new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *requester_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the <request_topic_name>, <reply_topic_name>, <publisher>,
    // and <subscriber> does not contains a valid xml element.
    // Check that fillDataNode() returns an XML_ERROR when child xml element of <requester> is not valid.
    {
        std::unique_ptr<xmlparser::RequesterAttributes> requester_atts;
        std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node;

        const char* xml_p =
                "\
                <requester profile_name=\"test_requester_profile\"\
                           service_name=\"service_name\"\
                           request_type=\"request_type\"\
                           reply_type=\"reply_type\">\
                    <%s>\
                        <bad_element></bad_element>\
                    </%s>\
                </requester>\
                ";
        constexpr size_t xml_len {800};
        char xml[xml_len];

        std::vector<std::string> elements {
            "request_topic_name",
            "reply_topic_name",
            "publisher",
            "subscriber",
            "bad_element"
        };
        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            requester_atts.reset(new xmlparser::RequesterAttributes);
            requester_node.reset(new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER,
                                                                               std::move(requester_atts) });
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *requester_node));
        }
    }
}

/*
 * This test checks the negative case in the fillDataNode function when it refers to a replier node.
 * 1. Check that fillDataNode() returns an XML_ERROR when the xml element is nullptr.
 * 2. Check that fillDataNode() returns an XML_ERROR when the profile_name attribute is missing.
 * 3. Check that fillDataNode() returns an XML_ERROR when the service_name attribute is missing.
 * 4. Check that fillDataNode() returns an XML_ERROR when the request_type attribute is missing.
 * 5. Check that fillDataNode() returns an XML_ERROR when the reply_type attribute is missing.
 * 5. Check that fillDataNode() returns an XML_ERROR when the <request_topic_name>, <reply_topic_name>, <publisher>, and <subscriber>
 *    does not contains a valid xml element.
 * 6. Check that fillDataNode() returns an XML_ERROR when child xml element of <replier> is not valid.
 */
TEST_F(XMLParserTests, fillDataNodeReplierNegativeClauses)
{
    std::unique_ptr<xmlparser::ReplierAttributes> replier_atts_error{new xmlparser::ReplierAttributes};
    std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node_error{
        new DataNode<xmlparser::ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts_error) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(nullptr, *replier_node_error));

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Check that fillDataNode() returns an XML_ERROR when the profile_name attribute is missing.
    {
        const char* xml =
                "\
                <replier>\
                </replier>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::ReplierAttributes> replier_atts{new xmlparser::ReplierAttributes};
        std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node{
            new DataNode<xmlparser::ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the service_name attribute is missing.
    {
        const char* xml =
                "\
                <replier profile_name=\"test_replier_profile\">\
                </replier>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::ReplierAttributes> replier_atts{new xmlparser::ReplierAttributes};
        std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node{
            new DataNode<xmlparser::ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the request_type attribute is missing.
    {
        const char* xml =
                "\
                <replier profile_name=\"test_replier_profile\"\
                           service_name=\"service_name\">\
                </replier>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::ReplierAttributes> replier_atts{new xmlparser::ReplierAttributes};
        std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node{
            new DataNode<xmlparser::ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the reply_type attribute is missing.
    {
        const char* xml =
                "\
                <replier profile_name=\"test_replier_profile\"\
                           service_name=\"service_name\"\
                           request_type=\"request_type\">\
                </replier>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        std::unique_ptr<xmlparser::ReplierAttributes> replier_atts{new xmlparser::ReplierAttributes};
        std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node{
            new DataNode<xmlparser::ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the <request_topic_name>, <reply_topic_name>, <publisher>,
    // and <subscriber> does not contains a valid xml element.
    // Check that fillDataNode() returns an XML_ERROR when child xml element of <replier> is not valid.
    {
        std::unique_ptr<xmlparser::ReplierAttributes> replier_atts;
        std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node;

        const char* xml_p =
                "\
                <replier profile_name=\"test_replier_profile\"\
                           service_name=\"service_name\"\
                           request_type=\"request_type\"\
                           reply_type=\"reply_type\">\
                    <%s>\
                        <bad_element></bad_element>\
                    </%s>\
                </replier>\
                ";
        constexpr size_t xml_len {800};
        char xml[xml_len];

        std::vector<std::string> elements {
            "request_topic_name",
            "reply_topic_name",
            "publisher",
            "subscriber",
            "bad_element"
        };
        for (std::string e : elements)
        {
            snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            replier_atts.reset(new xmlparser::ReplierAttributes);
            replier_node.reset(new DataNode<xmlparser::ReplierAttributes>{ NodeType::REQUESTER, std::move(
                                                                               replier_atts) });
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
        }
    }
}

/*
 * This test checks the negative clause in the parseXMLParticipantProf function. This clause is called in case the
 * the participant node could not be filled, i.e. fillDataNode returns an XML_ERROR code.
 */
TEST_F(XMLParserTests, parseXMLParticipantProfNegativeClauses)
{
    std::unique_ptr<xmlparser::ParticipantAttributes> participant_atts{new xmlparser::ParticipantAttributes};
    std::unique_ptr<DataNode<xmlparser::ParticipantAttributes>> participant_node{
        new DataNode<xmlparser::ParticipantAttributes>{ NodeType::PARTICIPANT, std::move(participant_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLParticipantProf_wrapper(nullptr, *participant_node));
}

/*
 * This test checks the negative clause in the parseXMLPublisherProf function. This clause is called in case the
 * the publisher node could not be filled, i.e. fillDataNode returns an XML_ERROR code.
 */
TEST_F(XMLParserTests, parseXMLPublisherProfNegativeClauses)
{
    std::unique_ptr<xmlparser::PublisherAttributes> publisher_atts{new xmlparser::PublisherAttributes};
    std::unique_ptr<DataNode<xmlparser::PublisherAttributes>> publisher_node{
        new DataNode<xmlparser::PublisherAttributes>{ NodeType::PUBLISHER, std::move(publisher_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLPublisherProf_wrapper(nullptr, *publisher_node));
}

/*
 * This test checks the negative clause in the parseXMLSubscriberProf function. This clause is called in case the
 * the subscriber node could not be filled, i.e. fillDataNode returns an XML_ERROR code.
 */
TEST_F(XMLParserTests, parseXMLSubscriberProfNegativeClauses)
{
    std::unique_ptr<xmlparser::SubscriberAttributes> subscriber_atts{new xmlparser::SubscriberAttributes};
    std::unique_ptr<DataNode<xmlparser::SubscriberAttributes>> subscriber_node{
        new DataNode<xmlparser::SubscriberAttributes>{ NodeType::SUBSCRIBER, std::move(subscriber_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLSubscriberProf_wrapper(nullptr, *subscriber_node));
}

/*
 * This test checks the negative clause in the parseXMLRequesterProf function. This clause is called in case the
 * the requester node could not be filled, i.e. fillDataNode returns an XML_ERROR code.
 */
TEST_F(XMLParserTests, parseXMLRequesterProfNegativeClauses)
{
    std::unique_ptr<xmlparser::RequesterAttributes> requester_atts{new xmlparser::RequesterAttributes};
    std::unique_ptr<DataNode<xmlparser::RequesterAttributes>> requester_node{
        new DataNode<xmlparser::RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLRequesterProf_wrapper(nullptr, *requester_node));
}

/*
 * This test checks the negative clause in the parseXMLReplierProf function. This clause is called in case the
 * the replier node could not be filled, i.e. fillDataNode returns an XML_ERROR code.
 */
TEST_F(XMLParserTests, parseXMLReplierProfNegativeClauses)
{
    std::unique_ptr<xmlparser::ReplierAttributes> replier_atts{new xmlparser::ReplierAttributes};
    std::unique_ptr<DataNode<xmlparser::ReplierAttributes>> replier_node{
        new DataNode<xmlparser::ReplierAttributes>{ NodeType::REPLIER, std::move(replier_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLReplierProf_wrapper(nullptr, *replier_node));
}

/*
 * This test checks the negative clause in the parseXMLTopicData function. This clause is called in case the
 * the topic node could not be filled, i.e. fillDataNode returns an XML_ERROR code.
 */
TEST_F(XMLParserTests, parseXMLTopicDataNegativeClauses)
{
    std::unique_ptr<TopicAttributes> topic_atts{new TopicAttributes};
    std::unique_ptr<DataNode<TopicAttributes>> topic_node{
        new DataNode<TopicAttributes>{ NodeType::TOPIC, std::move(topic_atts) }};

    // Check that an XML_ERROR is triggered when the xml element is nullptr.
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXMLTopicData_wrapper(nullptr, *topic_node));
}

/*
 * This test checks the behaviour of the parseXMLReceptionThreads function.
 */
TEST_F(XMLParserTests, parseXMLReceptionThreads)
{
    using namespace eprosima::fastdds::rtps;

    struct TestCase
    {
        std::string title;
        std::string xml;
        xmlparser::XMLP_ret result;
        eprosima::fastdds::rtps::PortBasedTransportDescriptor::ReceptionThreadsConfigMap threads_config;
    };

    ThreadSettings modified_thread_settings;
    modified_thread_settings.scheduling_policy = 12;
    modified_thread_settings.priority = 12;
    modified_thread_settings.affinity = 12;
    modified_thread_settings.stack_size = 12;


    std::vector<TestCase> test_cases =
    {
        {
            "reception_threads_empty",
            "<reception_threads></reception_threads>",
            xmlparser::XMLP_ret::XML_OK,
            {}
        },
        {
            "reception_threads_ok",
            R"(
                <reception_threads>
                    <reception_thread port="12345">
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </reception_thread>
                    <reception_thread port="12346">
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </reception_thread>
                </reception_threads>)",
            xmlparser::XMLP_ret::XML_OK,
            {
                {12345, {modified_thread_settings}},
                {12346, {modified_thread_settings}}
            }
        },
        {
            "reception_threads_duplicated",
            R"(
                <reception_threads>
                    <reception_thread port="12345">
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </reception_thread>
                    <reception_thread port="12345">
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </reception_thread>
                </reception_threads>)",
            xmlparser::XMLP_ret::XML_ERROR,
            {}
        },
        {
            "reception_threads_wrong_tags",
            R"(
                <reception_threads>
                    <wrong_tag port="12345">
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </wrong_tag>
                </reception_threads>)",
            xmlparser::XMLP_ret::XML_ERROR,
            {}
        },
        {
            "reception_threads_wrong_attribute",
            R"(
                <reception_threads>
                    <reception_thread wrong_attribute="12345">
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </reception_thread>
                </reception_threads>)",
            xmlparser::XMLP_ret::XML_ERROR,
            {}
        },
        {
            "reception_threads_no_attribute",
            R"(
                <reception_threads>
                    <reception_thread>
                        <scheduling_policy>12</scheduling_policy>
                        <priority>12</priority>
                        <affinity>12</affinity>
                        <stack_size>12</stack_size>
                    </reception_thread>
                </reception_threads>)",
            xmlparser::XMLP_ret::XML_ERROR,
            {}
        },
    };

    for (auto test_case : test_cases)
    {
        tinyxml2::XMLDocument xml_doc;
        std::unique_ptr<BaseNode> root;
        tinyxml2::XMLElement* titleElement;
        eprosima::fastdds::rtps::PortBasedTransportDescriptor::ReceptionThreadsConfigMap reception_threads;

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS,
                xml_doc.Parse(test_case.xml.c_str())) << "test_case = [" << test_case.title << "]";

        titleElement = xml_doc.RootElement();
        EXPECT_EQ(test_case.result, XMLParserTest::parseXMLReceptionThreads_wrapper(*titleElement, reception_threads));

        if (test_case.result == xmlparser::XMLP_ret::XML_OK)
        {
            for (auto entry : test_case.threads_config)
            {
                EXPECT_EQ(entry.second, reception_threads[entry.first]);
            }
        }
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
