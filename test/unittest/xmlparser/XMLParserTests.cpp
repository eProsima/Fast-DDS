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
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
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

    void helper_block_for_at_least_entries(
            uint32_t amount)
    {
        std::unique_lock<std::mutex> lck(*xml_mutex_);
        mock_consumer->cv().wait(lck, [this, amount]
                {
                    return mock_consumer->ConsumedEntriesSize_nts() >= amount;
                });
    }

    eprosima::fastdds::dds::XMLMockConsumer* mock_consumer;

    mutable std::mutex* xml_mutex_;

protected:

    void SetUp() override
    {
        xml_mutex_ = new std::mutex();
    }

    void TearDown() override
    {
        delete xml_mutex_;
        xml_mutex_ = nullptr;
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
 * 1. Check that elements <library_settings>, <participant>, <publisher>, <data_writer>, <subscriber>, <data_reader>,
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

    // Check that elements <library_settings>, <participant>, <publisher>, <data_writer>, <subscriber>, <data_reader>,
    // <topic>, <requester>, <replier>, <types>, and <log> are read as xml child elements of the <profiles>
    // root element.
    std::vector<std::string> elements_ok {
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
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    std::vector<std::string> elements_error {
        "library_settings",
        "participant",
        "requester",
        "replier"
    };
    for (std::string e : elements_error)
    {
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
    }

    // Check that it triggers an error when reading an wrong element.
    sprintf(xml, xml_p, "bad_element", "bad_element");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::parseXML_wrapper(xml_doc, root));
}

/*
 * This test checks the postive cases of the TLS configuration via XML.
 * 1. Check that parse_tls_config() return an XML_OK code for a valid configurations of <verify_paths>.
 */
TEST_F(XMLParserTests, parseTLSConfigPositiveClauses)
{
    tinyxml2::XMLDocument xml_doc;
    std::unique_ptr<BaseNode> root;
    tinyxml2::XMLElement* titleElement;
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<rtps::TCPv4TransportDescriptor>();

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
 * <verify_file>, <verify_depth>, <default_verify_path>, and <bad_element> return an xml error if their value is empty.
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
            std::make_shared<rtps::TCPv4TransportDescriptor>();
    char xml[600];

    // Check that elements <password>, <private_key_file>, <rsa_private_key_file>, <cert_chain_file>, <tmp_dh_file>,
    // <verify_file>, <verify_depth>, <default_verify_path>, and <bad_element> return an xml error if their value is empty.
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
            "cert_chain_file",
            "tmp_dh_file",
            "verify_file",
            "verify_depth",
            "default_verify_path",
            "bad_element"
        };

        for (std::string e : elements)
        {
            sprintf(xml, xml_p, e.c_str(), e.c_str());
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
            sprintf(xml, xml_p_verify_paths, e.c_str(), e.c_str());
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
            sprintf(xml, xml_p_verify_mode, e.c_str());
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
            sprintf(xml, xml_p_handshake_role, e.c_str());
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
            sprintf(xml, xml_p_options, e.c_str());
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
            std::make_shared<rtps::TCPv4TransportDescriptor>();

    char xml[600];

    // Parametrized XML
    const char* xml_p =
            "\
            <tls>\
                <handshake_role>%s</handshake_role>\
            </tls>\
            ";

    // Check that the DEFAULT setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p, "DEFAULT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.handshake_role,
                TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::DEFAULT);
    }

    // Check that the CLIENT setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p, "CLIENT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.handshake_role,
                TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::CLIENT);
    }

    // Check that the SERVER setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p, "SERVER");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.handshake_role,
                TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::SERVER);
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
    char xml[600];

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
        sprintf(xml, xml_p_verify_mode, "VERIFY_NONE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));

        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode, TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_NONE);
    }

    // Check that the VERIFY_PEER setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "VERIFY_PEER");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));

        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode, TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_PEER);
    }

    // Check that the VERIFY_FAIL_IF_NO_PEER_CERT setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "VERIFY_FAIL_IF_NO_PEER_CERT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));

        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode,
                TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT);
    }

    // Check that the VERIFY_CLIENT_ONCE setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "VERIFY_CLIENT_ONCE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();

        std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> tcp_transport =
            std::make_shared<rtps::TCPv4TransportDescriptor>();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_EQ(descriptor->tls_config.verify_mode,
                TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_CLIENT_ONCE);
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
            std::make_shared<rtps::TCPv4TransportDescriptor>();
    char xml[600];

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
        sprintf(xml, xml_p_verify_mode, "DEFAULT_WORKAROUNDS");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::DEFAULT_WORKAROUNDS));
    }

    // Check that the NO_COMPRESSION setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_COMPRESSION");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_COMPRESSION));
    }

    // Check that the NO_SSLV2 setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_SSLV2");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_SSLV2));
    }

    // Check that the NO_SSLV3 setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_SSLV3");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_SSLV3));
    }

    // Check that the NO_TLSV1 setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_TLSV1");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1));
    }

    // Check that the NO_TLSV1_1 setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_TLSV1_1");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1_1));
    }

    // Check that the NO_TLSV1_2 setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_TLSV1_2");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1_2));
    }

    // Check that the NO_TLSV1_3 setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "NO_TLSV1_3");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::NO_TLSV1_3));
    }

    // Check that the SINGLE_DH_USE setting return an xml ok code and is set correctly.
    {
        sprintf(xml, xml_p_verify_mode, "SINGLE_DH_USE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parse_tls_config_wrapper(titleElement, tcp_transport));
        std::shared_ptr<TCPTransportDescriptor> descriptor =
                std::dynamic_pointer_cast<TCPTransportDescriptor>(tcp_transport);
        EXPECT_TRUE(
                descriptor->tls_config.get_option(TCPTransportDescriptor::TLSConfig::TLSOptions::SINGLE_DH_USE));
    }
}

/*
 * This test checks the correct functioning of the parseProfile function for all xml child elements of <profile>.
 * 1. Check that elements <transport_descriptors>, <library_settings>, <participant>, <publisher>, <data_writer>,
 * <subscriber>, <data_reader>, <topic>, <requester>, <replier>, and <types> are read as xml child elements of the
 * <profiles> root element.
 * 2. Check that it triggers an error when reading an wrong element.
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

    char xml[600];

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
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        profilesNode.reset(new BaseNode{ NodeType::PROFILES });
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseProfiles_wrapper(titleElement, *profilesNode));
    }

    std::vector<std::string> elements_error {
        "library_settings",
        "participant",
        "requester",
        "replier",
        "bad_element"
    };
    for (std::string e : elements_error)
    {
        sprintf(xml, xml_p, e.c_str(), e.c_str());
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

    mock_consumer = new eprosima::fastdds::dds::XMLMockConsumer(xml_mutex_);

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

    char xml[600];

    std::vector<std::string> elements {
        "qos_profile",
        "application",
        "type"
    };
    for (std::string e : elements)
    {
        sprintf(xml, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        profilesNode.reset(new BaseNode{ NodeType::PROFILES });
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::parseProfiles_wrapper(titleElement, *profilesNode));
    }

    helper_block_for_at_least_entries(3);
    auto consumed_entries = mock_consumer->ConsumedEntries();
    // Expect 1 log error.
    uint32_t num_errors = 0;
    for (const auto& entry : consumed_entries)
    {
        if (entry.kind == eprosima::fastdds::dds::Log::Kind::Error)
        {
            num_errors++;
        }
    }
    EXPECT_EQ(num_errors, 3);
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
    std::unique_ptr<PublisherAttributes> publisher_atts{new PublisherAttributes};
    std::unique_ptr<DataNode<PublisherAttributes>> publisher_node{
            new DataNode<PublisherAttributes>{ NodeType::PUBLISHER, std::move(publisher_atts) }};

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
    std::unique_ptr<SubscriberAttributes> subscriber_atts{new SubscriberAttributes};
    std::unique_ptr<DataNode<SubscriberAttributes>> subscriber_node{
            new DataNode<SubscriberAttributes>{ NodeType::SUBSCRIBER, std::move(subscriber_atts) }};

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
    std::unique_ptr<RequesterAttributes> requester_atts_error{new RequesterAttributes};
    std::unique_ptr<DataNode<RequesterAttributes>> requester_node_error{
            new DataNode<RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts_error) }};

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
        std::unique_ptr<RequesterAttributes> requester_atts{new RequesterAttributes};
        std::unique_ptr<DataNode<RequesterAttributes>> requester_node{
            new DataNode<RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
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
        std::unique_ptr<RequesterAttributes> requester_atts{new RequesterAttributes};
        std::unique_ptr<DataNode<RequesterAttributes>> requester_node{
            new DataNode<RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
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
        std::unique_ptr<RequesterAttributes> requester_atts{new RequesterAttributes};
        std::unique_ptr<DataNode<RequesterAttributes>> requester_node{
            new DataNode<RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
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
        std::unique_ptr<RequesterAttributes> requester_atts{new RequesterAttributes};
        std::unique_ptr<DataNode<RequesterAttributes>> requester_node{
            new DataNode<RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *requester_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the <request_topic_name>, <reply_topic_name>, <publisher>,
    // and <subscriber> does not contains a valid xml element.
    // Check that fillDataNode() returns an XML_ERROR when child xml element of <requester> is not valid.
    {
        std::unique_ptr<RequesterAttributes> requester_atts;
        std::unique_ptr<DataNode<RequesterAttributes>> requester_node;

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
        char xml[800];

        std::vector<std::string> elements {
            "request_topic_name",
            "reply_topic_name",
            "publisher",
            "subscriber",
            "bad_element"
        };
        for (std::string e : elements)
        {
            sprintf(xml, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            requester_atts.reset(new RequesterAttributes);
            requester_node.reset(new DataNode<RequesterAttributes>{ NodeType::REQUESTER, std::move(requester_atts) });
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
    std::unique_ptr<ReplierAttributes> replier_atts_error{new ReplierAttributes};
    std::unique_ptr<DataNode<ReplierAttributes>> replier_node_error{
            new DataNode<ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts_error) }};

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
        std::unique_ptr<ReplierAttributes> replier_atts{new ReplierAttributes};
        std::unique_ptr<DataNode<ReplierAttributes>> replier_node{
            new DataNode<ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
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
        std::unique_ptr<ReplierAttributes> replier_atts{new ReplierAttributes};
        std::unique_ptr<DataNode<ReplierAttributes>> replier_node{
            new DataNode<ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
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
        std::unique_ptr<ReplierAttributes> replier_atts{new ReplierAttributes};
        std::unique_ptr<DataNode<ReplierAttributes>> replier_node{
            new DataNode<ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
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
        std::unique_ptr<ReplierAttributes> replier_atts{new ReplierAttributes};
        std::unique_ptr<DataNode<ReplierAttributes>> replier_node{
            new DataNode<ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) }};
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
    }

    // Check that fillDataNode() returns an XML_ERROR when the <request_topic_name>, <reply_topic_name>, <publisher>,
    // and <subscriber> does not contains a valid xml element.
    // Check that fillDataNode() returns an XML_ERROR when child xml element of <replier> is not valid.
    {
        std::unique_ptr<ReplierAttributes> replier_atts;
        std::unique_ptr<DataNode<ReplierAttributes>> replier_node;

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
        char xml[800];

        std::vector<std::string> elements {
            "request_topic_name",
            "reply_topic_name",
            "publisher",
            "subscriber",
            "bad_element"
        };
        for (std::string e : elements)
        {
            sprintf(xml, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.RootElement();
            replier_atts.reset(new ReplierAttributes);
            replier_node.reset(new DataNode<ReplierAttributes>{ NodeType::REQUESTER, std::move(replier_atts) });
            EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement, *replier_node));
        }
    }
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
