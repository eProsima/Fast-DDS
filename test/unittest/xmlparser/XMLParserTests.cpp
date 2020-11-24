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

#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLTree.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/IPLocator.h>
#include "mock/XMLMockConsumer.h"
#include "wrapper/XMLParserTest.hpp"

#include <tinyxml2.h>
#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

using eprosima::fastrtps::xmlparser::BaseNode;
using eprosima::fastrtps::xmlparser::DataNode;
using eprosima::fastrtps::xmlparser::NodeType;
using eprosima::fastrtps::xmlparser::XMLP_ret;
using eprosima::fastrtps::xmlparser::XMLParser;

class XMLTreeTests : public ::testing::Test
{
public:

    XMLTreeTests()
    {
    }

    ~XMLTreeTests()
    {
    }

};

TEST_F(XMLTreeTests, OnlyRoot)
{
    BaseNode test_base{NodeType::ROOT};
    ASSERT_EQ(NodeType::ROOT, test_base.getType());
    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(nullptr, test_base.getParent());
    ASSERT_EQ(0u, test_base.getNumChildren());
}

TEST_F(XMLTreeTests, RootChildren)
{
    BaseNode test_base{NodeType::ROOT};

    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(0u, test_base.getNumChildren());

    BaseNode* child = new BaseNode{NodeType::APPLICATION};
    test_base.addChild(std::unique_ptr<BaseNode>(child));

    ASSERT_EQ(1u, test_base.getNumChildren());
    ASSERT_EQ(child, test_base.getChild(0));
    ASSERT_EQ(&test_base, test_base.getChild(0)->getParent());
    ASSERT_EQ(NodeType::APPLICATION, test_base.getChild(0)->getType());
    ASSERT_EQ(true, test_base.removeChild(0));
    ASSERT_EQ(0u, test_base.getNumChildren());
    ASSERT_EQ(false, test_base.removeChild(0));
}

TEST_F(XMLTreeTests, RootMultipleChildren)
{
    const unsigned int num_children = 10;
    BaseNode test_base{NodeType::ROOT};

    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(0u, test_base.getNumChildren());

    std::vector<BaseNode*> children_backup;
    for (unsigned int i = 0; i < num_children; ++i)
    {
        children_backup.push_back(new BaseNode{NodeType::APPLICATION});
        test_base.addChild(std::unique_ptr<BaseNode>(children_backup[i]));
    }

    for (unsigned int i = 0; i < num_children; ++i)
    {
        ASSERT_EQ(children_backup[i], test_base.getChild(i));
        ASSERT_EQ(&test_base, test_base.getChild(i)->getParent());
        ASSERT_EQ(NodeType::APPLICATION, test_base.getChild(i)->getType());
    }

    for (unsigned int i = 0; i < num_children; ++i)
    {
        ASSERT_EQ(num_children - i, test_base.getNumChildren());
        ASSERT_EQ(true, test_base.removeChild(0));
        ASSERT_EQ(num_children - i - 1, test_base.getNumChildren());
        ASSERT_EQ(false, test_base.removeChild(num_children - i));
    }
}

TEST_F(XMLTreeTests, DataNode)
{
    const std::string attribute_name0{"Attribute0"};
    const std::string attribute_name1{"Attribute1"};
    const std::string attribute_value0{"TESTATTRIBUTE"};
    const std::string attribute_value1{"TESTATTRIBUTE1"};

    DataNode<std::string> data_node{NodeType::PUBLISHER};
    std::string* data = new std::string("TESTDATA");

    data_node.setData(std::unique_ptr<std::string>(data));
    data_node.addAttribute(attribute_name0, attribute_value0);
    data_node.addAttribute(attribute_name1, attribute_value1);

    ASSERT_EQ(NodeType::PUBLISHER, data_node.getType());
    ASSERT_EQ(data, data_node.getData().get());
    ASSERT_STREQ(attribute_value0.data(), data_node.getAttributes().at(attribute_name0).data());
    ASSERT_STREQ(attribute_value1.data(), data_node.getAttributes().at(attribute_name1).data());
}

class XMLParserTests : public ::testing::Test
{
public:

    XMLParserTests()
    {
    }

    ~XMLParserTests()
    {
        eprosima::fastdds::dds::Log::Reset();
        eprosima::fastdds::dds::Log::KillThread();
    }

    bool get_participant_attributes(
            std::unique_ptr<BaseNode>& root,
            ParticipantAttributes& participant_atts)
    {
        const std::string name_attribute{"profile_name"};
        const std::string profile_name{"missing_profile"};
        bool participant_profile = false;
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::PARTICIPANT)
            {
                auto data_node = dynamic_cast<DataNode<ParticipantAttributes>*>(profile.get());
                auto search    = data_node->getAttributes().find(name_attribute);
                if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
                {
                    participant_atts    = *data_node->get();
                    participant_profile = true;
                }
            }
        }
        return participant_profile;
    }

};

TEST_F(XMLParserTests, NoFIle)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("missing_file.xml", root), XMLP_ret::XML_ERROR);
}

TEST_F(XMLParserTests, EmptyDefaultFile)
{
    std::ifstream inFile;
    inFile.open("DEFAULT_FASTRTPS_PROFILES.xml");
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
    ASSERT_EQ(XMLParser::loadXML("test_xml_profiles.xml", root), XMLP_ret::XML_OK);
    ParticipantAttributes participant_atts;
    ASSERT_FALSE(get_participant_attributes(root, participant_atts));
}

TEST_F(XMLParserTests, WrongNameBuffer)
{
    std::ifstream inFile;
    inFile.open("test_xml_profiles.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);
    ParticipantAttributes participant_atts;
    ASSERT_FALSE(get_participant_attributes(root, participant_atts));
}

TEST_F(XMLParserTests, TypesRooted)
{
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML("test_xml_profiles_rooted.xml", root), XMLP_ret::XML_OK);

    ParticipantAttributes participant_atts;
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

TEST_F(XMLParserTests, TypesRootedBuffer)
{
    std::ifstream inFile;
    inFile.open("test_xml_profiles_rooted.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);
    ParticipantAttributes participant_atts;
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
    ASSERT_EQ(XMLParser::loadXML("test_xml_profiles.xml", root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    ParticipantAttributes participant_atts;
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
    inFile.open("test_xml_profiles.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    ParticipantAttributes participant_atts;
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

    ASSERT_EQ(XMLParser::loadXML("test_xml_duration.xml", root), XMLP_ret::XML_OK);

    ParticipantAttributes participant_atts;
    bool participant_profile = false;
    PublisherAttributes publisher_atts;
    bool publisher_profile = false;
    SubscriberAttributes subscriber_atts;
    bool subscriber_profile = false;
    for (const auto& profile : root->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<ParticipantAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name))
            {
                participant_atts    = *data_node->get();
                participant_profile = true;
            }
        }
        else if (profile->getType() == NodeType::PUBLISHER)
        {
            auto data_node = dynamic_cast<DataNode<PublisherAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name2))
            {
                publisher_atts    = *data_node->get();
                publisher_profile = true;
            }
        }
        else if (profile->getType() == NodeType::SUBSCRIBER)
        {
            auto data_node = dynamic_cast<DataNode<SubscriberAttributes>*>(profile.get());
            auto search    = data_node->getAttributes().find(name_attribute);
            if ((search != data_node->getAttributes().end()) && (search->second == profile_name3))
            {
                subscriber_atts    = *data_node->get();
                subscriber_profile = true;
            }
        }
    }
    ASSERT_TRUE(participant_profile);
    EXPECT_EQ(participant_atts.rtps.builtin.discovery_config.leaseDuration, c_TimeInfinite);
    EXPECT_EQ(participant_atts.rtps.builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(participant_atts.rtps.builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);

    ASSERT_TRUE(publisher_profile);
    EXPECT_EQ(publisher_atts.qos.m_deadline.period.seconds, 15);
    EXPECT_EQ(publisher_atts.qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(publisher_atts.qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(publisher_atts.qos.m_liveliness.announcement_period, c_TimeInfinite);
    EXPECT_EQ(publisher_atts.qos.m_latencyBudget.duration.seconds, 10);

    ASSERT_TRUE(subscriber_profile);
    EXPECT_EQ(subscriber_atts.qos.m_deadline.period, c_TimeInfinite);
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.lease_duration, c_TimeInfinite);
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.announcement_period.seconds, 0);
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.announcement_period.nanosec, 0u);
    EXPECT_EQ(subscriber_atts.qos.m_latencyBudget.duration.seconds, 20);
}

TEST_F(XMLParserTests, Data)
{
    std::unique_ptr<BaseNode> root;
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};

    ASSERT_EQ(XMLParser::loadXML("test_xml_profiles.xml", root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    ParticipantAttributes participant_atts;
    bool participant_profile = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<ParticipantAttributes>*>(profile.get());
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
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, c_TimeInfinite);
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
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.throughputController.bytesPerPeriod, 2048u);
    EXPECT_EQ(rtps_atts.throughputController.periodMillisecs, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

TEST_F(XMLParserTests, DataBuffer)
{
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};
    std::ifstream inFile;
    inFile.open("test_xml_profiles.xml");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::unique_ptr<BaseNode> root;
    ASSERT_EQ(XMLParser::loadXML(strStream.str().data(), strStream.str().size(), root), XMLP_ret::XML_OK);

    BaseNode* profiles(root->getChild(0));
    ASSERT_TRUE(profiles);
    ASSERT_EQ(profiles->getType(), xmlparser::NodeType::PROFILES);

    ParticipantAttributes participant_atts;
    bool participant_profile = false;
    for (const auto& profile : profiles->getChildren())
    {
        if (profile->getType() == NodeType::PARTICIPANT)
        {
            auto data_node = dynamic_cast<DataNode<ParticipantAttributes>*>(profile.get());
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
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, c_TimeInfinite);
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
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.throughputController.bytesPerPeriod, 2048u);
    EXPECT_EQ(rtps_atts.throughputController.periodMillisecs, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

// INIT NACHO SECTION

// FINISH NACHO SECTION

// INIT RAUL SECTION
/*
 * This test checks the correct functioning of the parseXML function when the <dds> root element does not exist.
 * 1. Check that elements <profiles>, <types> and <log> are read as root elements.
 * 2. Check that it triggers an error when reading an wrong element.
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

    char xml[600];

    // Check that elements <profiles>, <types> and <log> are read as root elements.
    std::vector<std::string> elements {
        "profiles",
        "types",
        "log"
    };

    for (std::string e : elements)
    {
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    // Check that it triggers an error when reading an wrong element.
    sprintf(xml, xml_p, "bad_root", "bad_root");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
}

/*
 * This test checks the correct functioning of the parseXML function for all xml child elements of <profile>
 * when the <profiles> element is an xml child element of the <dds> root element.
 * 1. Check that elements library_settings <participant>, <publisher>, <data_writer>, <subscriber>, <data_reader>,
 * <topic>, <requester>, <replier>, <types>, and <log> are read as xml child elements of the <profiles> root element.
 * 2. Check that it triggers an error when reading an wrong element.
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

    char xml[600];

    // Check that elements library_settings <participant>, <publisher>, <data_writer>, <subscriber>, <data_reader>,
    // <topic>, <requester>, <replier>, <types>, and <log> are read as xml child elements of the <profiles> root element.
    std::vector<std::string> elements {
        "library_settings",
        "participant",
        "publisher",
        "data_writer",
        "subscriber",
        "data_reader",
        "topic",
        "requester",
        "replier",
        "types",
        "log"
    };

    for (std::string e : elements)
    {
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    // Check that it triggers an error when reading an wrong element.
    sprintf(xml, xml_p, "bad_element", "bad_element");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
}
// FINISH RAUL SECTION

// INIT PARIS SECTION

// FINISH PARIS SECTION

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
