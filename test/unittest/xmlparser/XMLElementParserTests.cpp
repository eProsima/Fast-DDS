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
#include <fastrtps/xmlparser/XMLProfileManager.h>
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
using eprosima::fastrtps::xmlparser::up_participant_t;
using eprosima::fastrtps::xmlparser::up_node_participant_t;
using eprosima::fastrtps::xmlparser::node_participant_t;
using eprosima::fastrtps::xmlparser::sp_transport_t;

using eprosima::fastrtps::xmlparser::XMLProfileManager;

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

};

TEST_F(XMLParserTests, getXMLLifespanQos)
{
    uint8_t ident = 1;
    WriterQos wqos;
    ReaderQos rqos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <qos>\
        <lifespan>\
            <duration>\
                <sec>%s</sec>\
                <nanosec>%s</nanosec>\
            </duration>\
            %s\
        </lifespan>\
    </qos>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement,rqos,ident));
    EXPECT_EQ(wqos.m_lifespan.duration.seconds,5);
    EXPECT_EQ(wqos.m_lifespan.duration.nanosec,0);
    EXPECT_EQ(rqos.m_lifespan.duration.seconds,5);
    EXPECT_EQ(rqos.m_lifespan.duration.nanosec,0);

    // Missing data
    sprintf(xml, xml_p, "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));

    // Invalid element
    sprintf(xml, xml_p, "5", "0", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));

}

TEST_F(XMLParserTests, getXMLDisablePositiveAcksQos)
{
    uint8_t ident = 1;
    WriterQos wqos;
    ReaderQos rqos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <qos>\
        <disablePositiveAcks>\
            <enabled>%s</enabled>\
            <duration>\
                <sec>%s</sec>\
                <nanosec>%s</nanosec>\
            </duration>\
            %s\
        </disablePositiveAcks>\
    </qos>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "true", "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement,rqos,ident));
    EXPECT_EQ(wqos.m_disablePositiveACKs.enabled,true);
    EXPECT_EQ(rqos.m_disablePositiveACKs.enabled,true);
    EXPECT_EQ(wqos.m_disablePositiveACKs.duration.seconds,5);
    EXPECT_EQ(wqos.m_disablePositiveACKs.duration.nanosec,0);
    EXPECT_EQ(rqos.m_disablePositiveACKs.duration.seconds,5);
    EXPECT_EQ(rqos.m_disablePositiveACKs.duration.nanosec,0);

    // Missing data
    sprintf(xml, xml_p, "", "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));

    // Invalid element
    sprintf(xml, xml_p, "true", "5", "0", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));

}
/*
TEST_F(XMLParserTests, unsuportedQos)
{
    uint8_t ident = 1;
    WriterQos wqos;
    ReaderQos rqos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <qos>\
        <durabilityService></durabilityService>\
        <userData></userData>\
        <timeBasedFilter></timeBasedFilter>\
        <ownership></ownership>\
        <ownershipStrength></ownershipStrength>\
        <></>\
        <></>\
        <></>\
        <></>\
        <></>\
        <></>\
        <></>\
    </qos>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "true", "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement,wqos,ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement,rqos,ident));
}
*/
TEST_F(XMLParserTests, getXMLLocatorUDPv6)
{

    uint8_t ident = 1;
    LocatorList_t list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <unicastLocatorList>\
        <locator>\
            <udpv6>\
                <port>%s</port>\
                <address>%s</address>\
                %s\
            </udpv6>\
        </locator>\
    </unicastLocatorList>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "8844", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));
    EXPECT_EQ(list.begin()->port, 8844);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_UDPv6);

    // Missing data
    sprintf(xml, xml_p, "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));

    // Invalid element
    sprintf(xml, xml_p, "8844", "::1", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));

}

TEST_F(XMLParserTests, getXMLLocatorTCPv4)
{

    uint8_t ident = 1;
    LocatorList_t list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <unicastLocatorList>\
        <locator>\
            <tcpv4>\
                <physical_port>%s</physical_port>\
                <port>%s</port>\
                <unique_lan_id>%s</unique_lan_id>\
                <wan_address>%s</wan_address>\
                <address>%s</address>\
                %s\
            </tcpv4>\
        </locator>\
    </unicastLocatorList>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));
    EXPECT_EQ(IPLocator::getPhysicalPort(list.begin()->port), 5100);
    EXPECT_EQ(IPLocator::getLogicalPort(list.begin()->port), 8844);
    EXPECT_EQ(list.begin()->address[12], 192);
    EXPECT_EQ(list.begin()->address[13], 168);
    EXPECT_EQ(list.begin()->address[14], 1);
    EXPECT_EQ(list.begin()->address[15], 55);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_TCPv4);

    // Missing data
    sprintf(xml, xml_p, "", "","", "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));

    // Invalid element
    sprintf(xml, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));

}

TEST_F(XMLParserTests, getXMLLocatorTCPv6)
{

    uint8_t ident = 1;
    LocatorList_t list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <unicastLocatorList>\
        <locator>\
            <tcpv6>\
                <physical_port>%s</physical_port>\
                <port>%s</port>\
                <address>%s</address>\
                %s\
            </tcpv6>\
        </locator>\
    </unicastLocatorList>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "5100", "8844", "::1", "");

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));
    EXPECT_EQ(IPLocator::getPhysicalPort(list.begin()->port), 5100);
    EXPECT_EQ(IPLocator::getLogicalPort(list.begin()->port), 8844);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_TCPv6);

    // Missing data
    sprintf(xml, xml_p, "", "","", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));

    // Invalid element
    sprintf(xml, xml_p, "5100", "8844", "::1", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement,list,ident));

}

TEST_F(XMLParserTests, getXMLTransports)
{

    up_participant_t participant_atts{new ParticipantAttributes};
    up_node_participant_t participant_node{new node_participant_t{NodeType::PARTICIPANT, std::move(participant_atts)}};

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    const char* xml_profile =
            "\
        <profiles>\
            <transport_descriptors>\
                <transport_descriptor>\
                    <transport_id>ExampleTransportId1</transport_id>\
                    <type>UDPv6</type>\
                </transport_descriptor>\
            </transport_descriptors>\
        </profiles>\
    ";
    tinyxml2::XMLDocument xml_profile_doc;
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_profile_doc.Parse(xml_profile));
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_profile_doc));

    // Parametrized XML
    const char* xml_p =
    "\
    <participant>\
        <rtps>\
            <userTransports>\
                <transport_id>%s</transport_id>\
                %s\
            </userTransports>\
        </rtps>\
    </participant>\
    ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "ExampleTransportId1", "");

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::fillDataNode_wrapper(titleElement,*participant_node));
    auto ret = participant_node.get()->getAttributes();


    // Missing data
    sprintf(xml, xml_p, "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement,*participant_node));

    // Clean up
    xmlparser::XMLProfileManager::DeleteInstance();
}

TEST_F(XMLParserTests, getXMLPropertiesPolicy)
{

    up_participant_t participant_atts{new ParticipantAttributes};
    up_node_participant_t participant_node{new node_participant_t{NodeType::PARTICIPANT, std::move(participant_atts)}};

    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Valid XML
    const char* xml_p =
    "\
    <participant>\
        <rtps>\
            <propertiesPolicy>\
                <properties>\
                    <property>\
                        <name>Property1Name</name>\
                        <value>Property1Value</value>\
                        <propagate>false</propagate>\
                    </property>\
                </properties>\
            </propertiesPolicy>\
        </rtps>\
    </participant>\
    ";

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_p));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::fillDataNode_wrapper(titleElement,*participant_node));
    auto ret = participant_node.get()->getAttributes();


    // Empyty property XML
    const char* xml_empty =
    "\
    <participant>\
        <rtps>\
            <propertiesPolicy>\
                <properties></properties>\
            </propertiesPolicy>\
        </rtps>\
    </participant>\
    ";

    // Missing data
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_empty));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::fillDataNode_wrapper(titleElement,*participant_node));

}

TEST_F(XMLParserTests, getXMLRemoteServer)
{
    uint8_t ident = 1;
    DiscoverySettings settings;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;


    // Parametrized XML
    const char* xml_p =
    "\
    <discovery_config>\
        <discoveryServersList>\
        <RemoteServer prefix=\"4D.49.47.55.45.4c.5f.42.41.52.52.4f\">\
                <metatrafficUnicastLocatorList>\
                    <locator>\
                        <udpv6>\
                            <port>8844</port>\
                            <address>::1</address>\
                        </udpv6>\
                    </locator>\
                </metatrafficUnicastLocatorList>\
        </RemoteServer>\
        </discoveryServersList>\
    </discovery_config>\
    ";
    char xml[600];

    // Valid XML
    sprintf(xml, xml_p, "true", "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement,settings,ident));
    EXPECT_EQ(settings.m_DiscoveryServers.begin()->metatrafficUnicastLocatorList.begin()->port, 8844);

}

// INIT NACHO SECTION


// FINISH NACHO SECTION


// INIT RAUL SECTION


// FINISH RAUL SECTION


// INIT PARIS SECTION


TEST_F(XMLParserTests, getXMLBuiltinAttributes_invalidXML)
{
    uint8_t ident = 1;
    BuiltinAttributes builtin;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <builtinAttributesType>\
        %s\
    </builtinAttributesType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "discovery_config",
        "use_WriterLivelinessProtocol",
        "metatrafficUnicastLocatorList",
        "metatrafficMulticastLocatorList",
        "initialPeersList",
        "readerHistoryMemoryPolicy",
        "writerHistoryMemoryPolicy",
        "readerPayloadSize",
        "writerPayloadSize",
        "mutation_tries",
        "avoid_builtin_multicast"
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinAttributes_wrapper(titleElement,builtin,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinAttributes_wrapper(titleElement,builtin,ident));
}

TEST_F(XMLParserTests, getXMLThroughputController_invalidXML)
{
    uint8_t ident = 1;
    ThroughputControllerDescriptor throughputController;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <throughputControllerType>\
        %s\
    </throughputControllerType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "bytesPerPeriod",
        "periodMillisecs",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLThroughputController_wrapper(titleElement,throughputController,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLThroughputController_wrapper(titleElement,throughputController,ident));
}

TEST_F(XMLParserTests, getXMLTopicAttributes_invalidXML)
{
    uint8_t ident = 1;
    TopicAttributes topic;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <topicAttributesType>\
        %s\
    </topicAttributesType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "kind",
        "name",
        "dataType",
        "kind",
        "historyQos",
        "resourceLimitsQos",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement,topic,ident));
    }

    // Invalid key in kind field
    {
        const char* tag =
        "\
        <kind>\
            BAD_KEY\
        </kind>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement,topic,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement,topic,ident));
}

TEST_F(XMLParserTests, getXMLResourceLimitsQos_invalidXML)
{
    uint8_t ident = 1;
    ResourceLimitsQosPolicy resourceLimitsQos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <topicAttributesType>\
        %s\
    </topicAttributesType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "max_samples",
        "max_instances",
        "max_samples_per_instance",
        "allocated_samples",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLResourceLimitsQos_wrapper(titleElement,resourceLimitsQos,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLResourceLimitsQos_wrapper(titleElement,resourceLimitsQos,ident));
}

TEST_F(XMLParserTests, getXMLContainerAllocationConfig_invalidXML)
{
    uint8_t ident = 1;
    ResourceLimitedContainerConfig allocation_config;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <containerAllocationConfigType>\
        %s\
    </containerAllocationConfigType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "initial",
        "maximum",
        "increment",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement,allocation_config,ident));
    }

    // Invalid tuple initial-maximum parameters
    {
        const char* tag =
        "\
            <initial> 2 </initial>\
            <maximum> 1 </maximum>\
            <increment> 1 </increment>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement,allocation_config,ident));
    }

    // Invalid increment parameters
    {
        const char* tag =
        "\
            <initial> 1 </initial>\
            <maximum> 2 </maximum>\
            <increment> 0 </increment>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement,allocation_config,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement,allocation_config,ident));
}

TEST_F(XMLParserTests, getXMLHistoryQosPolicy_invalidXML)
{
    uint8_t ident = 1;
    HistoryQosPolicy historyQos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <historyQosPolicyType>\
        %s\
    </historyQosPolicyType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "kind",
        "depth",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement,historyQos,ident));
    }

    // Invalid tuple initial-maximum parameters
    {
        const char* tag =
        "\
            <kind> KEEP_BAD </kind>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement,historyQos,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement,historyQos,ident));
}

TEST_F(XMLParserTests, getXMLDurabilityQos_invalidXML)
{
    uint8_t ident = 1;
    DurabilityQosPolicy durability;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <durabilityQosPolicyType>\
        %s\
    </durabilityQosPolicyType>\
    ";
    char xml[1000];

    // Invalid kind
    {
        const char* tag =
        "\
            <kind> BAD_KIND </kind>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement,durability,ident));
    }

    // Void kind
    {
        const char* tag =
        "\
            <kind> </kind>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement,durability,ident));
    }

    // No kind
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement,durability,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement,durability,ident));
}

TEST_F(XMLParserTests, getXMLDeadlineQos_invalidXML)
{
    uint8_t ident = 1;
    DeadlineQosPolicy deadline;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <deadlineQosPolicyType>\
        %s\
    </deadlineQosPolicyType>\
    ";
    char xml[1000];

    // Invalid kind
    {
        const char* tag =
        "\
            <period> BAD_PERIOD </period>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement,deadline,ident));
    }

    // No period
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement,deadline,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement,deadline,ident));
}

TEST_F(XMLParserTests, getXMLLatencyBudgetQos_invalidXML)
{
    uint8_t ident = 1;
    LatencyBudgetQosPolicy latencyBudget;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <latencyBudgetQosPolicyType>\
        %s\
    </latencyBudgetQosPolicyType>\
    ";
    char xml[1000];

    // Invalid duration
    {
        const char* tag =
        "\
            <duration> BAD_DURATION </duration>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement,latencyBudget,ident));
    }

    // No duration
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement,latencyBudget,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement,latencyBudget,ident));
}

TEST_F(XMLParserTests, getXMLReliabilityQos_invalidXML)
{
    uint8_t ident = 1;
    ReliabilityQosPolicy reliability;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <reliabilityQosPolicyType>\
        %s\
    </reliabilityQosPolicyType>\
    ";
    char xml[1000];

    // Invalid kind
    {
        const char* tag =
        "\
            <kind> BAD_KIND </kind>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement,reliability,ident));
    }

    // Void kind
    {
        const char* tag =
        "\
            <kind> </kind>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement,reliability,ident));
    }

    // No max_blocking_time
    {
        const char* tag =
        "\
            <max_blocking_time> BAD_MBT </max_blocking_time>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement,reliability,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement,reliability,ident));
}

TEST_F(XMLParserTests, getXMLPartitionQos_invalidXML)
{
    uint8_t ident = 1;
    PartitionQosPolicy partition;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <partitionQosPolicyType>\
        %s\
    </partitionQosPolicyType>\
    ";
    char xml[1000];

    // Invalid names
    {
        const char* tag =
        "\
            <names>\
                <name>  </name>\
            </names>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement,partition,ident));
    }

    // Void names
    {
        const char* tag =
        "\
            <names>\
            </names>\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement,partition,ident));
    }

    // Void args
    {
        const char* tag =
        "\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement,partition,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement,partition,ident));
}

TEST_F(XMLParserTests, getXMLWriterTimes_invalidXML)
{
    uint8_t ident = 1;
    WriterTimes times;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <writerTimesType>\
        %s\
    </writerTimesType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "initialHeartbeatDelay",
        "heartbeatPeriod",
        "nackResponseDelay",
        "nackSupressionDuration",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterTimes_wrapper(titleElement,times,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterTimes_wrapper(titleElement,times,ident));
}

TEST_F(XMLParserTests, getXMLReaderTimes_invalidXML)
{
    uint8_t ident = 1;
    ReaderTimes times;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <readerTimesType>\
        %s\
    </readerTimesType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "initialAcknackDelay",
        "heartbeatResponseDelay",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderTimes_wrapper(titleElement,times,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderTimes_wrapper(titleElement,times,ident));
}

TEST_F(XMLParserTests, getXMLLocatorUDPv4_invalidXML)
{
    uint8_t ident = 1;
    rtps::Locator_t locator;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <udpv4LocatorType>\
        %s\
    </udpv4LocatorType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "port",
        "address",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorUDPv4_wrapper(titleElement,locator,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorUDPv4_wrapper(titleElement,locator,ident));
}

TEST_F(XMLParserTests, getXMLHistoryMemoryPolicy_invalidXML)
{
    uint8_t ident = 1;
    MemoryManagementPolicy_t historyMemoryPolicy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <historyMemoryPolicyType>\
        %s\
    </historyMemoryPolicyType>\
    ";
    char xml[1000];

    // Void historyMemoryPolicyType
    {
        const char* tag = "BAD POLICY";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryMemoryPolicy_wrapper(titleElement,historyMemoryPolicy,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryMemoryPolicy_wrapper(titleElement,historyMemoryPolicy,ident));
}

TEST_F(XMLParserTests, getXMLLivelinessQos_invalidXML)
{
    uint8_t ident = 1;
    LivelinessQosPolicy liveliness;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <livelinessQosPolicyType>\
        %s\
    </livelinessQosPolicyType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "kind",
        "lease_duration",
        "announcement_period"
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement,liveliness,ident));
    }

    // Invalid kind
    {
        const char* tag = "<kind> BAD_KIND </kind>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement,liveliness,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement,liveliness,ident));
}

TEST_F(XMLParserTests, getXMLPublishModeQos_invalidXML)
{
    uint8_t ident = 1;
    PublishModeQosPolicy publishMode;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <publishModeQosPolicyType>\
        %s\
    </publishModeQosPolicyType>\
    ";
    char xml[1000];

    // Invalid kind
    {
        const char* tag = "<kind> BAD_KIND </kind>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement,publishMode,ident));
    }

    // Empty kind
    {
        const char* tag = "<kind> </kind>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement,publishMode,ident));
    }

    // Empty kind
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement,publishMode,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement,publishMode,ident));
}

TEST_F(XMLParserTests, getXMLParticipantAllocationAttributes_invalidXML)
{
    uint8_t ident = 1;
    rtps::RTPSParticipantAllocationAttributes allocation;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <rtpsParticipantAllocationAttributesType>\
        %s\
    </rtpsParticipantAllocationAttributesType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "remote_locators",
        "total_participants",
        "total_readers",
        "total_writers",
        "send_buffers",
        "max_properties",
        "max_user_data",
        "max_partitions"
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLParticipantAllocationAttributes_wrapper(titleElement,allocation,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLParticipantAllocationAttributes_wrapper(titleElement,allocation,ident));
}

TEST_F(XMLParserTests, getXMLDiscoverySettings_invalidXML)
{
    uint8_t ident = 1;
    rtps::DiscoverySettings settings;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
    "\
    <discoverySettingsType>\
        %s\
    </discoverySettingsType>\
    ";
    char xml[1000];

    const char* field_p =
        "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    char field[500];

    std::vector<std::string> field_vec =
    {
        "discoveryProtocol",
        "ignoreParticipantFlags",
        "EDP",
        "leaseDuration",
        "leaseAnnouncement",
        "initialAnnouncements",
        "simpleEDP",
        "clientAnnouncementPeriod",
        "discoveryServersList",
        "staticEndpointXMLFilename"
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement,settings,ident));
    }

    // Bad EDP
    {
        const char* tag = "<EDP> BAD_EDP </EDP>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement,settings,ident));
    }

    // Bad simpleEDP PUBWRITER_SUBREADER
    {
        const char* tag = "<simpleEDP> <PUBWRITER_SUBREADER> </PUBWRITER_SUBREADER> </simpleEDP>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement,settings,ident));
    }

    // Bad simpleEDP PUBREADER_SUBWRITER
    {
        const char* tag = "<simpleEDP> <PUBREADER_SUBWRITER> </PUBREADER_SUBWRITER> </simpleEDP>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement,settings,ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement,settings,ident));
}

// FINISH PARIS SECTION
