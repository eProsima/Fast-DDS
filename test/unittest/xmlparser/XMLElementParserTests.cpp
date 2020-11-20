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

// Class to test protected methods
class XMLParserTest : public XMLParser{
    public:
    static XMLP_ret getXMLWriterQosPolicies_wrapper(
        tinyxml2::XMLElement* elem,
        WriterQos& qos,
        uint8_t ident)
    {
        return getXMLWriterQosPolicies(elem, qos, ident);
    }
    static XMLP_ret getXMLReaderQosPolicies_wrapper(
        tinyxml2::XMLElement* elem,
        ReaderQos& qos,
        uint8_t ident)
    {
        return getXMLReaderQosPolicies(elem, qos, ident);
    }
    static XMLP_ret getXMLLocatorList_wrapper(
        tinyxml2::XMLElement* elem,
        LocatorList_t& locatorList,
        uint8_t ident)
    {
        return getXMLLocatorList(elem, locatorList, ident);
    }

    static XMLP_ret fillDataNode_wrapper(
        tinyxml2::XMLElement* p_profile,
        DataNode<ParticipantAttributes>& participant_node)
    {
        return fillDataNode(p_profile, participant_node);
    }
    
    static XMLP_ret getXMLDiscoverySettings_wrapper(
        tinyxml2::XMLElement* elem,
        rtps::DiscoverySettings& settings,
        uint8_t ident)
    {
        return getXMLDiscoverySettings(elem,settings,ident);
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
    auto a = settings.m_DiscoveryServers.begin()->metatrafficUnicastLocatorList.begin()->port;
    EXPECT_EQ(settings.m_DiscoveryServers.begin()->metatrafficUnicastLocatorList.begin()->port, 8844);

}
