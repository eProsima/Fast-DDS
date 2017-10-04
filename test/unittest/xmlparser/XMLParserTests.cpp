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
#include <fastrtps/log/Log.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>

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
    ASSERT_EQ(0, test_base.getNumChildren());
}

TEST_F(XMLTreeTests, RootChildren)
{
    BaseNode test_base{NodeType::ROOT};

    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(0, test_base.getNumChildren());

    BaseNode* child = new BaseNode{NodeType::APPLICATION};
    test_base.addChild(std::unique_ptr<BaseNode>(child));

    ASSERT_EQ(1, test_base.getNumChildren());
    ASSERT_EQ(child, test_base.getChild(0));
    ASSERT_EQ(&test_base, test_base.getChild(0)->getParent());
    ASSERT_EQ(NodeType::APPLICATION, test_base.getChild(0)->getType());
    ASSERT_EQ(true, test_base.removeChild(0));
    ASSERT_EQ(0, test_base.getNumChildren());
    ASSERT_EQ(false, test_base.removeChild(0));
}

TEST_F(XMLTreeTests, RootMultipleChildren)
{
    const int num_children = 10;
    BaseNode test_base{NodeType::ROOT};

    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(0, test_base.getNumChildren());

    std::vector<BaseNode*> children_backup;
    for (int i = 0; i < num_children; ++i)
    {
        children_backup.push_back(new BaseNode{NodeType::APPLICATION});
        test_base.addChild(std::unique_ptr<BaseNode>(children_backup[i]));
    }

    for (int i = 0; i < num_children; ++i)
    {
        ASSERT_EQ(children_backup[i], test_base.getChild(i));
        ASSERT_EQ(&test_base, test_base.getChild(i)->getParent());
        ASSERT_EQ(NodeType::APPLICATION, test_base.getChild(i)->getType());
    }

    for (int i = 0; i < num_children; ++i)
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
        eprosima::fastrtps::Log::Reset();
        eprosima::fastrtps::Log::KillThread();
    }

    bool get_participant_attributes(std::unique_ptr<BaseNode>& root, ParticipantAttributes& participant_atts)
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

    ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    for (const auto& profile : root->getChildren())
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

    ParticipantAttributes participant_atts;
    bool participant_profile = false;
    bool publisher_profile   = false;
    bool subscriber_profile  = false;
    for (const auto& profile : root->getChildren())
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

TEST_F(XMLParserTests, Data)
{
    std::unique_ptr<BaseNode> root;
    const std::string name_attribute{"profile_name"};
    const std::string profile_name{"test_participant_profile"};

    ASSERT_EQ(XMLParser::loadXML("test_xml_profiles.xml", root), XMLP_ret::XML_OK);

    ParticipantAttributes participant_atts;
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

    ASSERT_TRUE(participant_profile);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin           = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    locator.set_IP4_address(192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    locator.set_IP4_address(239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    locator.set_IP4_address(192, 168, 1, 1);
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

    // transport XML profile
    EXPECT_EQ(rtps_atts.userTransports.size(), 2);
    auto transport_v4 = std::dynamic_pointer_cast<UDPv4TransportDescriptor>(rtps_atts.userTransports.front());
    EXPECT_EQ(transport_v4->whiteListOutput, true);
    EXPECT_EQ(transport_v4->whiteListInput, true);
    EXPECT_EQ(transport_v4->whiteListLocators, true);
    EXPECT_EQ(transport_v4->interfaceWhiteList.size(), 1);
    EXPECT_EQ(transport_v4->interfaceWhiteList.front(), "127.0.0.1");

    auto transport_v6 = std::dynamic_pointer_cast<UDPv6TransportDescriptor>(rtps_atts.userTransports.back());
    EXPECT_EQ(transport_v6->whiteListOutput, true);
    EXPECT_EQ(transport_v6->whiteListInput, true);
    EXPECT_EQ(transport_v6->whiteListLocators, true);
    EXPECT_EQ(transport_v6->interfaceWhiteList.size(), 1);
    EXPECT_EQ(transport_v6->interfaceWhiteList.front(), "::1");
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

    ParticipantAttributes participant_atts;
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

    ASSERT_TRUE(participant_profile);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin           = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;
    locator.set_IP4_address(192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    locator.set_IP4_address(239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    locator.set_IP4_address(192, 168, 1, 1);
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

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
