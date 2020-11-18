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
