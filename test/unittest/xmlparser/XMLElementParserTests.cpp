// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <thread>
#include <mutex>

#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLTree.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/utils/IPLocator.h>
#include "../logging/mock/MockConsumer.h"
#include "wrapper/XMLParserTest.hpp"

#include <tinyxml2.h>
#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

using namespace eprosima::fastdds::dds;
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
        Log::Reset();
        Log::KillThread();
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

    MockConsumer* mock_consumer;

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

/*
 * This test checks the proper parsing of the <lifespan> xml element to LifespanQosPolicy, and negative cases.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <sec> and <nanosec> in <duration> child xml element.
 * 3. Check an bad element as a child xml element.
 * 4. Check an  empty xml definition.
 */
TEST_F(XMLParserTests, getXMLLifespanQos)
{
    uint8_t ident = 1;
    LifespanQosPolicy lifespan;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <lifespan>\
                <duration>\
                    <sec>%s</sec>\
                    <nanosec>%s</nanosec>\
                </duration>\
                %s\
            </lifespan>\
            ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLifespanQos_wrapper(titleElement, lifespan, ident));
    EXPECT_EQ(lifespan.duration.seconds, 5);
    EXPECT_EQ(lifespan.duration.nanosec, 0u);

    // Missing data
    sprintf(xml, xml_p, "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLifespanQos_wrapper(titleElement, lifespan, ident));

    // Invalid element
    sprintf(xml, xml_p, "5", "0", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLifespanQos_wrapper(titleElement, lifespan, ident));

    // Missing element
    const char* miss_xml =
            "\
            <lifespan></lifespan>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(miss_xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLifespanQos_wrapper(titleElement, lifespan, ident));
}

/*
 * This test checks the proper parsing of the <disablePositiveACKs> xml element to DisablePositiveACKsQosPolicy,
 * and negative cases.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <sec> and <nanosec> in <duration> child xml element and empty <enabled>.
 * 3. Check an empty definition of <enabled>.
 * 4. Check an bad element as a child xml element.
 * 5. Check an  empty xml definition.
 */
TEST_F(XMLParserTests, getXMLDisablePositiveAcksQos)
{
    uint8_t ident = 1;
    DisablePositiveACKsQosPolicy disablePositiveACKs;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <disablePositiveAcks>\
                <enabled>%s</enabled>\
                <duration>\
                    <sec>%s</sec>\
                    <nanosec>%s</nanosec>\
                </duration>\
                %s\
            </disablePositiveAcks>\
            ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "true", "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));
    EXPECT_EQ(disablePositiveACKs.enabled, true);
    EXPECT_EQ(disablePositiveACKs.enabled, true);
    EXPECT_EQ(disablePositiveACKs.duration.seconds, 5);
    EXPECT_EQ(disablePositiveACKs.duration.nanosec, 0u);

    // Missing data - enabled
    sprintf(xml, xml_p, "", "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));

    // Missing data - duration
    sprintf(xml, xml_p, "true", "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));

    // Invalid element
    sprintf(xml, xml_p, "true", "5", "0", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));
}

/*
 * This test checks the parsing of a <udpv6> element from a list of locators.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <port> .
 * 3. Check an empty definition of <address>.
 * 4. Check an bad element as a child xml element.
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
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(list.begin()->port, 8844u);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_UDPv6);

    // Missing data - port
    sprintf(xml, xml_p, "", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    sprintf(xml, xml_p, "8844", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    sprintf(xml, xml_p, "8844", "::1", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <tcpv4> element from a list of locators.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <physical_port> .
 * 3. Check an empty definition of <port>.
 * 4. Check an empty definition of <unique_lan_id>.
 * 5. Check an empty definition of <wan_address>.
 * 6. Check an empty definition of <address>.
 * 7. Check an bad element as a child xml element.
 */
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
    char xml[1000];

    // Valid XML
    sprintf(xml, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(IPLocator::getPhysicalPort(list.begin()->port), 5100u);
    EXPECT_EQ(IPLocator::getLogicalPort(list.begin()->port), 8844u);

    //<unique_lan_id>
    EXPECT_EQ(list.begin()->address[0], 192);
    EXPECT_EQ(list.begin()->address[1], 168);
    EXPECT_EQ(list.begin()->address[2], 1);
    EXPECT_EQ(list.begin()->address[3], 1);
    EXPECT_EQ(list.begin()->address[4], 1);
    EXPECT_EQ(list.begin()->address[5], 1);
    EXPECT_EQ(list.begin()->address[6], 2);
    EXPECT_EQ(list.begin()->address[7], 55);

    //<wan_address>
    EXPECT_EQ(list.begin()->address[8], 80);
    EXPECT_EQ(list.begin()->address[9], 80);
    EXPECT_EQ(list.begin()->address[10], 99);
    EXPECT_EQ(list.begin()->address[11], 45);

    // <address>
    EXPECT_EQ(list.begin()->address[12], 192);
    EXPECT_EQ(list.begin()->address[13], 168);
    EXPECT_EQ(list.begin()->address[14], 1);
    EXPECT_EQ(list.begin()->address[15], 55);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_TCPv4);

    // Missing data - physical_port
    sprintf(xml, xml_p, "", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - port
    sprintf(xml, xml_p, "5100", "", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - unique_lan_id
    sprintf(xml, xml_p, "5100", "8844", "", "80.80.99.45", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - wan_address
    sprintf(xml, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    sprintf(xml, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    sprintf(xml, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55",
            "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <tcpv6> element from a list of locators.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <physical_port> .
 * 3. Check an empty definition of <port>.
 * 5. Check an empty definition of <address>.
 * 6. Check an bad element as a child xml element.
 */
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
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(IPLocator::getPhysicalPort(list.begin()->port), 5100u);
    EXPECT_EQ(IPLocator::getLogicalPort(list.begin()->port), 8844u);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_TCPv6);

    // Missing data - physical_port
    sprintf(xml, xml_p,  "", "8844", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - port
    sprintf(xml, xml_p,  "5100", "", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    sprintf(xml, xml_p,  "5100", "8844", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    sprintf(xml, xml_p, "5100", "8844", "::1", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the proper parsing of the <transport_descriptors> xml elements to in a vector of pointers to
 * TransportDescriptorInterface, and negative cases.
 * 1. Correct parsing of a valid descriptor present in the XmlProfileManager.
 * 2. Check a reference to a non existentTransportDescriptorInterface.
 * 3. Check an empty definition of <transport_id>.
 * 5. Check an empty definition of <transport_descriptor>.
 * 6. Check an empty list of transports.
 */
TEST_F(XMLParserTests, getXMLTransports)
{
    uint8_t ident = 1;
    std::vector<std::shared_ptr<TransportDescriptorInterface>> transports;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Profile describing the transport
    const char* xml_profile =
            "\
            <profiles>\
                <transport_descriptors>\
                    <transport_descriptor>\
                        <transport_id>ExampleTransportId1</transport_id>\
                        <type>UDPv6</type>\
                        <maxMessageSize>31416</maxMessageSize>\
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
            <userTransports>\
                <transport_id>%s</transport_id>\
            </userTransports>\
            ";
    char xml[500];

    // Valid XML
    sprintf(xml, xml_p, "ExampleTransportId1");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLTransports_wrapper(titleElement, transports, ident));
    EXPECT_EQ(transports[0]->max_message_size(), 31416u);

    // Wrong ID
    sprintf(xml, xml_p, "WrongTransportId");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTransports_wrapper(titleElement, transports, ident));

    // Missing data
    sprintf(xml, xml_p, "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTransports_wrapper(titleElement, transports, ident));

    // No Elements
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse( "<userTransports></userTransports>"));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTransports_wrapper(titleElement, transports, ident));

    // Clean up
    xmlparser::XMLProfileManager::DeleteInstance();
}

/*
 * This test checks the proper parsing of the <property_policy> xml elements to a PropertyPolicy object, and negative
 * cases.
 * 1. Correct parsing of a valid <property_policy>.
 * 2. Check missing values for the possible elemnts of the properties.
 * 3. Check an empty list of <properties>.
 * 5. Check an empty list of <binary_properties>.
 * 6. Check an wrong descriptor for properties.
 */
TEST_F(XMLParserTests, getXMLPropertiesPolicy)
{
    uint8_t ident = 1;
    PropertyPolicy property_policy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    const std::vector<std::string> valid_parameters {
        "Property1Name",
        "Property1Value",
        "false",
        "BinProperty1Name",
        "false"};
    std::vector<std::string> parameters(valid_parameters);

    // Template xml
    const char* xml_p =
            "\
            <propertiesPolicy>\
                <properties>\
                    <property>\
                        <name>%s</name>\
                        <value>%s</value>\
                        <propagate>%s</propagate>\
                    </property>\
                </properties>\
                <binary_properties>\
                    <property>\
                        <name>%s</name>\
                        <value></value>\
                        <propagate>%s</propagate>\
                    </property>\
                </binary_properties>\
            </propertiesPolicy>\
            ";
    char xml[1000];

    sprintf(xml, xml_p,
            valid_parameters[0].c_str(),
            valid_parameters[1].c_str(),
            valid_parameters[2].c_str(),
            valid_parameters[3].c_str(),
            valid_parameters[4].c_str());

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, property_policy, ident));
    EXPECT_EQ(property_policy.properties()[0].name(), valid_parameters[0]);
    EXPECT_EQ(property_policy.properties()[0].value(), valid_parameters[1]);
    EXPECT_EQ(property_policy.properties()[0].propagate(), false);
    EXPECT_EQ(property_policy.binary_properties()[0].name(), valid_parameters[3]);
    // TODO check when binary property values are suported
    // EXPECT_EQ(property_policy.binary_properties()[0].name(), valid_parameters[4]);
    EXPECT_EQ(property_policy.binary_properties()[0].propagate(), false);

    for (int i = 0; i < 5; i++)
    {
        parameters = valid_parameters;
        parameters[i] = "";

        sprintf(xml, xml_p,
                parameters[0].c_str(),
                parameters[1].c_str(),
                parameters[2].c_str(),
                parameters[3].c_str(),
                parameters[4].c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::propertiesPolicy_wrapper(titleElement, property_policy, ident));
    }

    // Empty property XML
    const char* xml_empty_prop =
            "\
            <propertiesPolicy>\
                <properties></properties>\
            </propertiesPolicy>\
            ";

    // Missing data
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_empty_prop));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::propertiesPolicy_wrapper(titleElement, property_policy, ident));


    // Empty binary_property XML
    const char* xml_empty_bin_prop =
            "\
            <propertiesPolicy>\
                <binary_properties></binary_properties>\
            </propertiesPolicy>\
            ";

    // Missing data
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_empty_bin_prop));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::propertiesPolicy_wrapper(titleElement, property_policy, ident));

    // wrong XML
    const char* xml_bad_prop =
            "\
            <propertiesPolicy>\
                <bad_properties></bad_properties>\
            </propertiesPolicy>\
            ";

    // Wrong property
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_bad_prop));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::propertiesPolicy_wrapper(titleElement, property_policy, ident));
}

/*
 * This test checks the proper parsing of a <RemoteServer> xml element to a RemoteServerAttributes object, and negative
 * cases.
 * 1. Check nullptr as tinyxml2::XMLElement argument.
 * 2. Check missing prefix in the <RemoteServer> tag.
 * 3. Check wrongly formated in the <RemoteServer> tag.
 * 5. Check an empty <metatrafficUnicastLocatorList> tag.
 * 6. Check an empty <metatrafficMulticastLocatorList> tag.
 * 6. Check a <RemoteServer> tag with no locators.
 */
TEST_F(XMLParserTests, getXMLRemoteServer)
{
    uint8_t ident = 1;
    RemoteServerAttributes attr;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    const std::vector<std::string> valid_parameters {
        "prefix=\"4D.49.47.55.45.4c.5f.42.41.52.52.4f\"",
        "<locator>\
            <udpv6>\
                <port>8844</port>\
                <address>::1</address>\
            </udpv6>\
        </locator>",
        "<locator>\
            <udpv6>\
                <port>8844</port>\
                <address>::1</address>\
            </udpv6>\
        </locator>", };
    std::vector<std::string> parameters(valid_parameters);

    // Parametrized XML
    const char* xml_p =
            "\
            <RemoteServer %s>\
                    <metatrafficUnicastLocatorList>%s</metatrafficUnicastLocatorList>\
                    <metatrafficMulticastLocatorList>%s</metatrafficMulticastLocatorList>\
            </RemoteServer>\
            ";
    char xml[1200];

    // Valid XML
    sprintf(xml, xml_p, valid_parameters[0].c_str(), valid_parameters[1].c_str(), valid_parameters[2].c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLRemoteServer_wrapper(titleElement, attr, ident));
    EXPECT_EQ(attr.guidPrefix.value[0], (octet)0x4d);
    EXPECT_EQ(attr.guidPrefix.value[1], (octet)0x49);
    EXPECT_EQ(attr.guidPrefix.value[2], (octet)0x47);
    EXPECT_EQ(attr.guidPrefix.value[3], (octet)0x55);
    EXPECT_EQ(attr.guidPrefix.value[4], (octet)0x45);
    EXPECT_EQ(attr.guidPrefix.value[5], (octet)0x4c);
    EXPECT_EQ(attr.guidPrefix.value[6], (octet)0x5f);
    EXPECT_EQ(attr.guidPrefix.value[7], (octet)0x42);
    EXPECT_EQ(attr.guidPrefix.value[8], (octet)0x41);
    EXPECT_EQ(attr.guidPrefix.value[9], (octet)0x52);
    EXPECT_EQ(attr.guidPrefix.value[10], (octet)0x52);
    EXPECT_EQ(attr.guidPrefix.value[11], (octet)0x4f);

    EXPECT_EQ(attr.metatrafficUnicastLocatorList.begin()->port, 8844u);
    EXPECT_EQ(attr.metatrafficUnicastLocatorList.begin()->address[15], 1);
    EXPECT_EQ(attr.metatrafficMulticastLocatorList.begin()->port, 8844u);
    EXPECT_EQ(attr.metatrafficMulticastLocatorList.begin()->address[15], 1);

    // nullptr element
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLRemoteServer_wrapper(nullptr, attr, ident));

    // No prefix
    sprintf(xml, xml_p, "", valid_parameters[1].c_str(), valid_parameters[2].c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLRemoteServer_wrapper(titleElement, attr, ident));

    // Bad prefix value
    sprintf(xml, xml_p, "prefix=\"\"", valid_parameters[1].c_str(), valid_parameters[2].c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLRemoteServer_wrapper(titleElement, attr, ident));

    // Bad unicast
    sprintf(xml, xml_p, valid_parameters[0].c_str(), "", valid_parameters[2].c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLRemoteServer_wrapper(titleElement, attr, ident));

    // Bad multicast
    sprintf(xml, xml_p, valid_parameters[0].c_str(), valid_parameters[1].c_str(), "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLRemoteServer_wrapper(titleElement, attr, ident));

    // No locators
    sprintf(xml, "<RemoteServer %s></RemoteServer>", valid_parameters[0].c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLRemoteServer_wrapper(titleElement, attr, ident));
}

/*
 * This test checks the negative cases of a <port> xml element.
 * 1. Check a missing case of each of the <port> child tags.
 * 2. Check a wrong child tag.
 */
TEST_F(XMLParserTests, getXMLPortParameters_NegativeClauses)
{
    uint8_t ident = 1;
    PortParameters port;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::string xml;
    std::vector<std::string> parameters;

    for (int i = 0; i < 8; i++)
    {
        if (i < 7)
        {
            parameters.assign (7, "1");
            parameters[i] = "";
            xml =
                    "\
                    <port>\
                        <portBase>" + parameters[0] + "</portBase>\
                        <domainIDGain>" + parameters[1] + "</domainIDGain>\
                        <participantIDGain>" + parameters[2] +
                    "</participantIDGain>\
                        <offsetd0>" + parameters[3] + "</offsetd0>\
                        <offsetd1>" + parameters[4] + "</offsetd1>\
                        <offsetd2>" + parameters[5] + "</offsetd2>\
                        <offsetd3>" + parameters[6] + "</offsetd3>\
                    </port>\
                    ";
        }
        else
        {
            xml =
                    "\
                    <port>\
                        <bad_element></bad_element>\
                    </port>\
                    ";
        }

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPortParameters_wrapper(titleElement, port, ident));
    }
}

/*
 * This test checks the negative cases of a <subscriber> xml profile.
 * 1. Check an incorrect for each of the possible SubscriberAttributes.
 * 2. Check an non existant attribute.
 */
TEST_F(XMLParserTests, getXMLSubscriberAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    SubscriberAttributes attr;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::string xml;
    std::vector<std::string> parameters {
        "<topic><bad_element></bad_element></topic>",
        "<qos><bad_element></bad_element></qos>",
        "<times><bad_element></bad_element></times>",
        "<unicastLocatorList><bad_element></bad_element></unicastLocatorList>",
        "<multicastLocatorList><bad_element></bad_element></multicastLocatorList>",
        "<remoteLocatorList><bad_element></bad_element></remoteLocatorList>",
        "<expectsInlineQos><bad_element></bad_element></expectsInlineQos>",
        "<historyMemoryPolicy><bad_element></bad_element></historyMemoryPolicy>",
        "<propertiesPolicy><bad_element></bad_element></propertiesPolicy>",
        "<userDefinedID><bad_element></bad_element></userDefinedID>",
        "<entityID><bad_element></bad_element></entityID>",
        "<matchedPublishersAllocation><bad_element></bad_element></matchedPublishersAllocation>",
        "<bad_element></bad_element>"
    };

    for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        xml =
                "\
                <subscriber profile_name=\"test_subscriber_profile\" is_default_profile=\"true\">\
                    " + *it + "\
                </subscriber>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLSubscriberAttributes_wrapper(titleElement, attr, ident));
    }

}

/*
 * This test checks the negative cases of a <publisher> xml profile.
 * 1. Check an incorrect for each of the possible PublisherAttributes.
 * 2. Check an non existant attribute.
 */
TEST_F(XMLParserTests, getXMLPublisherAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    PublisherAttributes attr;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::string xml;
    std::vector<std::string> parameters {
        "<topic><bad_element></bad_element></topic>",
        "<qos><bad_element></bad_element></qos>",
        "<times><bad_element></bad_element></times>",
        "<unicastLocatorList><bad_element></bad_element></unicastLocatorList>",
        "<multicastLocatorList><bad_element></bad_element></multicastLocatorList>",
        "<remoteLocatorList><bad_element></bad_element></remoteLocatorList>",
        "<throughputController><bad_element></bad_element></throughputController>",
        "<historyMemoryPolicy><bad_element></bad_element></historyMemoryPolicy>",
        "<propertiesPolicy><bad_element></bad_element></propertiesPolicy>",
        "<userDefinedID><bad_element></bad_element></userDefinedID>",
        "<entityID><bad_element></bad_element></entityID>",
        "<matchedSubscribersAllocation><bad_element></bad_element></matchedSubscribersAllocation>",
        "<bad_element></bad_element>"
    };

    for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        xml =
                "\
                <publisher profile_name=\"test_publisher_profile\" is_default_profile=\"true\">\
                    " + *it + "\
                </publisher>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublisherAttributes_wrapper(titleElement, attr, ident));
    }

}

/*
 * This test checks the negative cases of a locator list xml element.
 * 1. Check an incorrect for each of the possible types of <locator>.
 * 2. Check an non existant type of locator.
 */
TEST_F(XMLParserTests, getXMLLocatorList_NegativeClauses)
{

    uint8_t ident = 1;
    LocatorList_t list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::string xml;
    std::vector<std::string> parameters {
        "<locator><udpv4><bad_element></bad_element></udpv4></locator>",
        "<locator><udpv6><bad_element></bad_element></udpv6></locator>",
        "<locator><tcpv4><bad_element></bad_element></tcpv4></locator>",
        "<locator><tcpv6><bad_element></bad_element></tcpv6></locator>",
        "<locator><bad_element></bad_element></locator>",
        "<bad_element></bad_element>"
    };

    for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        xml =
                "\
                <locatorList>\
                    " + *it + "\
                </locatorList>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    }

}

/*
 * This test checks the negative cases of the XMLParser::getXMLguidPrefix method.
 * 1. Check a missing value for a <guid>.
 * 2. Check passing a nullptr as a tinyxml2::XMLElement argument.
 */
TEST_F(XMLParserTests, getXMLguidPrefix_NegativeClauses)
{

    uint8_t ident = 1;
    GuidPrefix_t prefix;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<guid></guid>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLguidPrefix_wrapper(titleElement, prefix, ident));

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLguidPrefix_wrapper(nullptr, prefix, ident));
}

/*
 * This test checks the positive cases of the XMLParser::getXMLguidPrefix method.
 * 1. Check a correct return of the method.
 * 2. Check the correct values have been passed to the prefix variable.
 */
TEST_F(XMLParserTests, getXMLguidPrefix_positive)
{

    uint8_t ident = 1;
    GuidPrefix_t prefix;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<guid>4D.49.47.55.45.4c.5f.42.41.52.52.4f</guid>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLguidPrefix_wrapper(titleElement, prefix, ident));
    EXPECT_EQ(prefix.value[0], (octet)0x4d);
    EXPECT_EQ(prefix.value[1], (octet)0x49);
    EXPECT_EQ(prefix.value[2], (octet)0x47);
    EXPECT_EQ(prefix.value[3], (octet)0x55);
    EXPECT_EQ(prefix.value[4], (octet)0x45);
    EXPECT_EQ(prefix.value[5], (octet)0x4c);
    EXPECT_EQ(prefix.value[6], (octet)0x5f);
    EXPECT_EQ(prefix.value[7], (octet)0x42);
    EXPECT_EQ(prefix.value[8], (octet)0x41);
    EXPECT_EQ(prefix.value[9], (octet)0x52);
    EXPECT_EQ(prefix.value[10], (octet)0x52);
    EXPECT_EQ(prefix.value[11], (octet)0x4f);
}

/*
 * This test checks the negative cases of the XMLParser::getXMLDuration method.
 * 1. Check passing an infinite duration and a finite duration at the same time.
 * 2. Check passing a missing value of <sec> and <nanosec>.
 * 3. Check passing a non valid value of <sec> and <nanosec>.
 * 4. Check passing an empty <duration> field.
 */
TEST_F(XMLParserTests, getXMLDuration_NegativeClauses)
{

    uint8_t ident = 1;
    Duration_t duration;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::string xml;
    std::vector<std::string> parameters {
        "DURATION_INFINITY<sec>1</sec>",
        "<sec></sec>",
        "<sec>not_an_int</sec>",
        "<nanosec></nanosec>",
        "<nanosec>not_an_int</nanosec>",
        "<bad_element></bad_element>",
        ""
    };

    for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        xml =
                "\
                <duration>\
                    " + *it + "\
                </duration>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDuration_wrapper(titleElement, duration, ident));
    }

}

/*
 * This test checks the positive cases of the XMLParser::getXMLDuration method.
 * 1. Check correct return of the method.
 * 2. Check correct parsing on DURATION_INFINITY in the <sec> field.
 * 3. Check correct parsing on DURATION_INFINITY in the <nanosec> field.
 */
TEST_F(XMLParserTests, getXMLDuration_infinite)
{

    uint8_t ident = 1;
    Duration_t duration;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::string xml;
    std::vector<std::string> parameters {
        "<sec>DURATION_INFINITY</sec>",
        "<nanosec>DURATION_INFINITY</nanosec>"
    };

    for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        xml =
                "\
                <duration>\
                    " + *it + "\
                </duration>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDuration_wrapper(titleElement, duration, ident));
        EXPECT_EQ(duration, c_TimeInfinite);
    }

}

/*
 * This test checks the correct parsing of a string field with the XMLParser::getXMLString method.
 * 1. Check passing a valid string field.
 * 2. Check passing a nullptr as a tinyxml2::XMLElement argument.
 * 4. Check passing an empty field.
 */
TEST_F(XMLParserTests, getXMLString)
{
    uint8_t ident = 1;
    std::string s;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field>field_text</field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLString_wrapper(titleElement, &s, ident));
    EXPECT_EQ(s, "field_text");

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLString_wrapper(nullptr, &s, ident));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field></field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLString_wrapper(titleElement, &s, ident));
}

/*
 * This test checks the negative cases of the XMLParser::getXMLList method.
 * 1. Check passing a nullptr as a tinyxml2::XMLElement argument.
 * 2. Check passing an empty <list> field.
 * 3. Check passing a non valid value of <RemoteServer> descriptor as an element of <list>.
 */
TEST_F(XMLParserTests, getXMLList_NegativeClauses)
{
    uint8_t ident = 1;
    RemoteServerList_t list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // empty element
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLList_wrapper(nullptr, list, ident));

    // empty list
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<list></list>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLList_wrapper(titleElement, list, ident));

    // bad remote server element
    const char* xml = "<list><RemoteServer>bad_remote_server</RemoteServer></list>";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the positive case of the XMLParser::getXMLList method.
 * 1. Check an valid return on the function.
 * 2. Check the correct element has been placed on the list.
 */
TEST_F(XMLParserTests, getXMLList_positive)
{
    uint8_t ident = 1;
    RemoteServerList_t list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // bad remote server element
    const char* xml =
            "<list>\
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
            </list>";

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLList_wrapper(titleElement, list, ident));
    EXPECT_EQ(list.begin()->metatrafficUnicastLocatorList.begin()->port, 8844u);
    EXPECT_EQ(list.begin()->metatrafficUnicastLocatorList.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->guidPrefix.value[0], 0x4d);
    EXPECT_EQ(list.begin()->guidPrefix.value[1], 0x49);
    EXPECT_EQ(list.begin()->guidPrefix.value[2], 0x47);
    EXPECT_EQ(list.begin()->guidPrefix.value[3], 0x55);
    EXPECT_EQ(list.begin()->guidPrefix.value[4], 0x45);
}

/*
 * This test checks the negative cases of the XMLParser::getXMLBool method.
 * 1. Check passing a nullptr as a tinyxml2::XMLElement argument.
 * 2. Check passing a non boolean valid inside the field.
 */
TEST_F(XMLParserTests, getXMLBool_NegativeClauses)
{
    uint8_t ident = 1;
    bool b;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBool_wrapper(nullptr, &b, ident));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field>not_a_bool</field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBool_wrapper(titleElement, &b, ident));
}

/*
 * This test checks the negative cases of the XMLParser::getXMLInt method.
 * 1. Check passing a nullptr as a tinyxml2::XMLElement argument.
 * 2. Check passing a non integer valid inside the field.
 */
TEST_F(XMLParserTests, getXMLInt_NegativeClauses)
{
    uint8_t ident = 1;
    int i;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLInt_wrapper(nullptr, &i, ident));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field>not_an_int</field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLInt_wrapper(titleElement, &i, ident));
}

/*
 * This test checks the negative cases of the two definitions of XMLParser::getXMLInt method.
 * 1. Check passing a nullptr as a tinyxml2::XMLElement argument.
 * 2. Check passing a non integer valid inside the field.
 * Both for each definition.
 */
TEST_F(XMLParserTests, getXMLUint_NegativeClauses)
{
    uint8_t ident = 1;
    unsigned int ui;
    uint16_t ui16;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLUint_wrapper(nullptr, &ui, ident));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field>not_an_uint</field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLUint_wrapper(titleElement, &ui, ident));

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLUint_wrapper(nullptr, &ui16, ident));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field>not_an_uint</field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLUint_wrapper(titleElement, &ui16, ident));
}

/*
 * This test checks the negative cases in the <initialAnnouncements> xml element.
 * 1. Check an empty definition of <count> child xml element.
 * 2. Check an empty definition of <sec> in <period> child xml element.
 * 3. Check an empty definition of <nanosec> in <period> child xml element.
 * 4. Check a wrong xml element definition inside <initialAnnouncements>
 */
TEST_F(XMLParserTests, getXMLInitialAnnouncementsConfig_NegativeClauses)
{
    uint8_t ident = 1;
    DiscoverySettings settings;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <discovery_config>\
                <initialAnnouncements>\
                    %s\
                    <count>%s</count>\
                    <period>\
                        <sec>%s</sec>\
                        <nanosec>%s</nanosec>\
                    </period>\
                </initialAnnouncements>\
            </discovery_config>\
            ";

    char xml[600];

    // Check an empty definition of <count> child xml element.
    sprintf(xml, xml_p, "", "", "5", "123");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));

    // Check an empty definition of <sec> in <period> child xml element.
    sprintf(xml, xml_p, "", "5", "", "123");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));

    // Check an empty definition of <nanosec> in <period> child xml element.
    sprintf(xml, xml_p, "", "5", "5", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));

    const char* xml_e =
            "\
            <discovery_config>\
                <initialAnnouncements>\
                    <bad_element>1</bad_element>\
                </initialAnnouncements>\
            </discovery_config>\
            ";

    // Check a wrong xml element definition inside <initialAnnouncements>
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_e));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
}

/*
 * This test checks the negative cases in the <qos> xml child element of <data_writer>/<data_reader>
 * 1. Check an empty definition of <durability> xml element.
 * 2. Check an empty definition of <liveliness> xml element.
 * 3. Check an empty definition of <reliability> xml element.
 * 4. Check an empty definition of <partition> xml element.
 * 5. Check an empty definition of <publishMode> xml element.
 * 6. Check an empty definition of <deadline> xml element.
 * 7. Check an empty definition of <disablePositiveAcks> xml element.
 * 8. Check an empty definition of <latencyBudget> xml element.
 * 9. Check a wrong xml element definition inside <qos>
 */
TEST_F(XMLParserTests, getXMLWriterReaderQosPolicies)
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
                %s\
            </qos>\
            ";

    char xml[600];

    // Check an empty definition of <durability> xml element.
    sprintf(xml, xml_p, "<durability></durability>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <liveliness> xml element.
    sprintf(xml, xml_p, "<liveliness><kind></kind></liveliness>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <reliability> xml element.
    sprintf(xml, xml_p, "<reliability><kind></kind></reliability>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <partition> xml element.
    sprintf(xml, xml_p, "<partition></partition>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <publishMode> xml element.
    sprintf(xml, xml_p, "<publishMode></publishMode>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));

    // Check an empty definition of <deadline> xml element.
    sprintf(xml, xml_p, "<deadline></deadline>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <disablePositiveAcks> xml element.
    sprintf(xml, xml_p, "<disablePositiveAcks><enabled></enabled></disablePositiveAcks>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));

    // Check an empty definition of <latencyBudget> xml element.
    sprintf(xml, xml_p, "<latencyBudget></latencyBudget>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));

    // Check an empty definition of <lifespan> xml element.
    sprintf(xml, xml_p, "<lifespan></lifespan>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <latencyBudget> xml element.
    sprintf(xml, xml_p, "<latencyBudget></latencyBudget>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <disablePositiveAcks> xml element.
    sprintf(xml, xml_p, "<disablePositiveAcks><enabled></enabled></disablePositiveAcks>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check a wrong xml element definition inside <qos>
    sprintf(xml, xml_p, "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));
}

/*
 * This test checks that there is a logError when setting up a non supported <data_writer>/<data_reader> qos
 * 1. Check that there is a logError when trying to set up the <durabilityService> Qos.
 * 2. Check that there is a logError when trying to set up the <userData> Qos.
 * 3. Check that there is a logError when trying to set up the <timeBasedFilter> Qos.
 * 4. Check that there is a logError when trying to set up the <ownership> Qos.
 * 5. Check that there is a logError when trying to set up the <ownershipStrength> Qos.
 * 6. Check that there is a logError when trying to set up the <destinationOrder> Qos.
 * 7. Check that there is a logError when trying to set up the <presentation> Qos.
 * 8. Check that there is a logError when trying to set up the <topicData> Qos.
 * 9. Check that there is a logError when trying to set up the <groupData> Qos.
 */
TEST_F(XMLParserTests, getXMLWriterReaderUnsupportedQosPolicies)
{
    uint8_t ident = 1;
    WriterQos wqos;
    ReaderQos rqos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    mock_consumer = new MockConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mock_consumer));
    Log::SetCategoryFilter(std::regex("(XMLPARSER)"));

    // Parametrized XML
    const char* xml_p =
            "\
            <qos>\
                %s\
            </qos>\
            ";

    char xml[600];

    // Check that there is a logError when trying to set up the <durabilityService> Qos.
    sprintf(xml, xml_p, "<durabilityService></durabilityService>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <userData> Qos.
    sprintf(xml, xml_p, "<userData></userData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <timeBasedFilter> Qos.
    sprintf(xml, xml_p, "<timeBasedFilter></timeBasedFilter>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <ownership> Qos.
    sprintf(xml, xml_p, "<ownership></ownership>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <ownershipStrength> Qos.
    sprintf(xml, xml_p, "<ownershipStrength></ownershipStrength>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <destinationOrder> Qos.
    sprintf(xml, xml_p, "<destinationOrder></destinationOrder>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <presentation> Qos.
    sprintf(xml, xml_p, "<presentation></presentation>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <topicData> Qos.
    sprintf(xml, xml_p, "<topicData></topicData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a logError when trying to set up the <groupData> Qos.
    sprintf(xml, xml_p, "<groupData></groupData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    helper_block_for_at_least_entries(18);
    auto consumed_entries = mock_consumer->ConsumedEntries();
    // Expect 18 log errors.
    uint32_t num_errors = 0;
    for (const auto& entry : consumed_entries)
    {
        if (entry.kind == Log::Kind::Error)
        {
            num_errors++;
        }
    }
    EXPECT_EQ(num_errors, 18u);
}

/*
 * This test checks the positive cases of configuration through XML of the data limits of the participant's allocation
 * attributes.
 * 1. Check that the XML return code is correct for the data limit settings.
 * 2. Check that the maximum number of properties attribute (max_properties) is set correctly.
 * 3. Check that the maximum user data attribute (max_user_data) is set correctly.
 * 4. Check that the maximum number of partitions attribute (max_partitions) is set correctly.
 */
TEST_F(XMLParserTests, ParticipantAllocationAttributesDataLimits)
{
    uint8_t ident = 1;
    rtps::RTPSParticipantAllocationAttributes allocation;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <rtpsParticipantAllocationAttributes>\
                <max_properties>10</max_properties>\
                <max_user_data>20</max_user_data>\
                <max_partitions>3</max_partitions>\
            </rtpsParticipantAllocationAttributes>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the data limit settings.
    EXPECT_EQ(
        XMLP_ret::XML_OK,
        XMLParserTest::getXMLParticipantAllocationAttributes_wrapper(titleElement, allocation, ident));
    // Check that the maximum number of properties attribute (max_properties) is set correctly.
    EXPECT_EQ(allocation.data_limits.max_properties, 10ul);
    // Check that the maximum user data attribute (max_user_data) is set correctly.
    EXPECT_EQ(allocation.data_limits.max_user_data, 20ul);
    // Check that the maximum number of partitions attribute (max_partitions) is set correctly.
    EXPECT_EQ(allocation.data_limits.max_partitions, 3ul);
}

/*
 * This test checks the positive cases of configuration through XML of the STATIC EDP.
 * 1. Check that the XML return code is correct for the STATIC EDP settings.
 * 2. Check that the SIMPLE discovery protocol is set to false.
 * 3. Check that the STATIC discovery protocol is set to true.
 * 4. Check that the static endpoint XML filename is set correctly.
 */
TEST_F(XMLParserTests, getXMLDiscoverySettingsStaticEDP)
{
    uint8_t ident = 1;
    rtps::DiscoverySettings settings;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <discovery_config>\
                <EDP>STATIC</EDP>\
                <staticEndpointXMLFilename>my_static_edp.xml</staticEndpointXMLFilename>\
            </discovery_config>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the STATIC EDP settings.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    // Check that the SIMPLE discovery protocol is set to false.
    EXPECT_FALSE(settings.use_SIMPLE_EndpointDiscoveryProtocol);
    // Check that the STATIC discovery protocol is set to true.
    EXPECT_TRUE(settings.use_STATIC_EndpointDiscoveryProtocol);
    // Check that the static endpoint XML filename is set correctly.
    EXPECT_STREQ(settings.getStaticEndpointXMLFilename(), "my_static_edp.xml");
}

/*
 * This test checks the positive case of configuration via XML of the livelines automatic kind.
 * 1. Check that the XML return code is correct for the liveliness kind setting.
 * 2. Check that the liveliness kind is set to AUTOMATIC.
 */
TEST_F(XMLParserTests, getXMLLivelinessQosAutomaticKind)
{
    uint8_t ident = 1;
    LivelinessQosPolicy liveliness;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <liveliness>\
                <kind>AUTOMATIC</kind>\
            </liveliness>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the liveliness kind setting.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement, liveliness, ident));
    // Check that the liveliness kind is set to AUTOMATIC.
    EXPECT_EQ(liveliness.kind, LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS);
}

/*
 * This test checks the positive case of configuration via XML of the publish mode synchronous kind.
 * 1. Check that the XML return code is correct for the publish mode kind setting.
 * 2. Check that the publish mode kind is set to SYNCHRONOUS_PUBLISH_MODE.
 */
TEST_F(XMLParserTests, getXMLPublishModeQosSynchronousKind)
{
    uint8_t ident = 1;
    PublishModeQosPolicy publishMode;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <publishMode>\
                <kind>SYNCHRONOUS</kind>\
            </publishMode>\
            ";

    // Load the xml
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the publish mode kind setting.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    // Check that the publish mode kind is set to SYNCHRONOUS_PUBLISH_MODE.
    EXPECT_EQ(publishMode.kind, PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE);
}

/*
 * This test checks the positive case of configuration via XML of the history memory policy.
 * 1. Check that the XML return code is correct for the history memory policy setting.
 * 2. Check that the history memory policy mode is set to PREALLOCATED_MEMORY_MODE.
 * 3. Check that the history memory policy mode is set to PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
 * 4. Check that the history memory policy mode is set to DYNAMIC_RESERVE_MEMORY_MODE.
 * 5. Check that the history memory policy mode is set to DYNAMIC_REUSABLE_MEMORY_MODE.
 */
TEST_F(XMLParserTests, getXMLHistoryMemoryPolicy)
{
    uint8_t ident = 1;
    MemoryManagementPolicy_t historyMemoryPolicy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    std::map<std::string, MemoryManagementPolicy> policies =
    {
        {"PREALLOCATED", MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE},
        {"PREALLOCATED_WITH_REALLOC", MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE},
        {"DYNAMIC", MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE},
        {"DYNAMIC_REUSABLE", MemoryManagementPolicy::DYNAMIC_REUSABLE_MEMORY_MODE}
    };

    // Parametrized XML
    const char* xml_p =
            "\
            <historyMemoryPolicyType>%s</historyMemoryPolicyType>\
            ";
    char xml[500];
    for (const auto& policy : policies)
    {
        // Load the xml
        sprintf(xml, xml_p, policy.first.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(
            XMLP_ret::XML_OK,
            XMLParserTest::getXMLHistoryMemoryPolicy_wrapper(titleElement, historyMemoryPolicy, ident));
        EXPECT_EQ(historyMemoryPolicy, policy.second);
    }
}

/*
 * This test checks the positive case of configuration via XML of the durability QoS policy kind.
 * 1. Check that the XML return code is correct for all the durability QoS policy kind values.
 * 2. Check that the durability QoS policy kind is set to VOLATILE.
 * 3. Check that the durability QoS policy kind is set to TRANSIENT_LOCAL.
 * 4. Check that the durability QoS policy kind is set to TRANSIENT.
 * 5. Check that the durability QoS policy kind is set to PERSISTENT.
 */
TEST_F(XMLParserTests, getXMLDurabilityQosKind)
{
    uint8_t ident = 1;
    DurabilityQosPolicy durability;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <durability>\
                <kind>%s</kind>\
            </durability>\
            ";
    char xml[500];

    std::vector<std::string> kinds = {"VOLATILE", "TRANSIENT_LOCAL", "TRANSIENT", "PERSISTENT"};

    sprintf(xml, xml_p, "VOLATILE");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy VOLATILE kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to VOLATILE.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS);

    sprintf(xml, xml_p, "TRANSIENT_LOCAL");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy TRANSIENT_LOCAL kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to TRANSIENT_LOCAL.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS);

    sprintf(xml, xml_p, "TRANSIENT");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy TRANSIENT kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to TRANSIENT.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS);

    sprintf(xml, xml_p, "PERSISTENT");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy PERSISTENT kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to PERSISTENT.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS);
}

/*
 * This test checks the negative cases in the xml child element of <BuiltinAttributes>
 * 1. Check an invalid tag of:
 *      <discovery_config>
 *      <use_WriterLivelinessProtocol>
 *      <metatrafficUnicastLocatorList>
 *      <metatrafficMulticastLocatorList>
 *      <initialPeersList>
 *      <readerHistoryMemoryPolicy>
 *      <writerHistoryMemoryPolicy>
 *      <readerPayloadSize>
 *      <writerPayloadSize>
 *      <mutation_tries>
 *      <avoid_builtin_multicast>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLBuiltinAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    BuiltinAttributes builtin;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <builtinAttributes>\
                %s\
            </builtinAttributes>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinAttributes_wrapper(titleElement, builtin, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinAttributes_wrapper(titleElement, builtin, ident));
}

/*
 * This test checks the negative cases in the xml child element of <ThroughputController>
 * 1. Check an invalid tag of:
 *      <dbytesPerPeriod>
 *      <periodMillisecs>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLThroughputController_NegativeClauses)
{
    uint8_t ident = 1;
    ThroughputControllerDescriptor throughputController;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <throughputController>\
                %s\
            </throughputController>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLThroughputController_wrapper(titleElement, throughputController, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLThroughputController_wrapper(titleElement, throughputController, ident));
}

/*
 * This test checks the negative cases in the xml child element of <TopicAttributes>
 * 1. Check an invalid tag of:
 *      <kind>
 *      <name>
 *      <data>
 *      <kind>
 *      <historyQos>
 *      <resourceLimitsQos>
 * 2. Check invalid <kind> type
 * 3. Check invalid element
 */
TEST_F(XMLParserTests, getXMLTopicAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    TopicAttributes topic;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <topicAttributes>\
                %s\
            </topicAttributes>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement, topic, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement, topic, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement, topic, ident));
}

/*
 * This test checks the negative cases in the xml child element of <ResourceLimitsQos>
 * 1. Check an invalid tag of:
 *      <max_samples>
 *      <max_instances>
 *      <max_samples_per_instance>
 *      <allocated_samples>
 * 2. Check invalid <kind> type
 * 3. Check invalid element
 */
TEST_F(XMLParserTests, getXMLResourceLimitsQos_NegativeClauses)
{
    uint8_t ident = 1;
    ResourceLimitsQosPolicy resourceLimitsQos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <topicAttributes>\
                %s\
            </topicAttributes>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLResourceLimitsQos_wrapper(titleElement, resourceLimitsQos, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLResourceLimitsQos_wrapper(titleElement, resourceLimitsQos,
            ident));
}

/*
 * This test checks the negative cases in the xml child element of <ContainerAllocationConfig>
 * 1. Check an invalid tag of:
 *      <initial>
 *      <maximum>
 *      <increment>
 * 2. Check invalid config <initial> > <maximum>
 * 3. Check incalid config <increment> = 0
 * 4. Check invalid element
 */
TEST_F(XMLParserTests, getXMLContainerAllocationConfig_NegativeClauses)
{
    uint8_t ident = 1;
    ResourceLimitedContainerConfig allocation_config;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <containerAllocationConfig>\
                %s\
            </containerAllocationConfig>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement, allocation_config, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement, allocation_config, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement, allocation_config, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement, allocation_config, ident));
}

/*
 * This test checks the negative cases in the xml child element of <HistoryQosPolicy>
 * 1. Check an invalid tag of:
 *      <kind>
 *      <depth>
 * 2. Check invalid <kind> element
 * 3. Check invalid element
 */
TEST_F(XMLParserTests, getXMLHistoryQosPolicy_NegativeClauses)
{
    uint8_t ident = 1;
    HistoryQosPolicy historyQos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <historyQosPolicy>\
                %s\
            </historyQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement, historyQos, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement, historyQos, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement, historyQos, ident));
}

/*
 * This test checks the negative cases in the xml child element of <DurabilityQos>
 * 1. Check invalid <kind> element
 * 2. Check empty <kind> element
 * 3. Check no <kind> element
 * 4. Check invalid element
 */
TEST_F(XMLParserTests, getXMLDurabilityQos_NegativeClauses)
{
    uint8_t ident = 1;
    DurabilityQosPolicy durability;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <durabilityQosPolicy>\
                %s\
            </durabilityQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    }

    // No kind
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
}

/*
 * This test checks the negative cases in the xml child element of <DeadlineQos>
 * 1. Check invalid <period> element
 * 2. Check no <period> element
 * 3. Check invalid element
 */
TEST_F(XMLParserTests, getXMLDeadlineQos_NegativeClauses)
{
    uint8_t ident = 1;
    DeadlineQosPolicy deadline;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <deadlineQosPolicy>\
                %s\
            </deadlineQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement, deadline, ident));
    }

    // No period
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement, deadline, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement, deadline, ident));
}

/*
 * This test checks the negative cases in the xml child element of <LatencyBudgetQos>
 * 1. Check invalid <duration> element
 * 2. Check no <duration> element
 * 3. Check invalid element
 */
TEST_F(XMLParserTests, getXMLLatencyBudgetQos_NegativeClauses)
{
    uint8_t ident = 1;
    LatencyBudgetQosPolicy latencyBudget;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <latencyBudgetQosPolicy>\
                %s\
            </latencyBudgetQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement, latencyBudget, ident));
    }

    // No duration
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement, latencyBudget, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement, latencyBudget, ident));
}

/*
 * This test checks the negative cases in the xml child element of <ReliabilityQos>
 * 1. Check invalid <kind> element
 * 2. Check empty <kind> element
 * 3. Check no <max_blocking_time> element
 * 4. Check invalid element
 */
TEST_F(XMLParserTests, getXMLReliabilityQos_NegativeClauses)
{
    uint8_t ident = 1;
    ReliabilityQosPolicy reliability;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <reliabilityQosPolicy>\
                %s\
            </reliabilityQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement, reliability, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement, reliability, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement, reliability, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement, reliability, ident));
}

/*
 * This test checks the negative cases in the xml child element of <PartitionQos>
 * 1. Check invalid <names> element
 * 2. Check empty <names> element
 * 3. Check no <names> element
 * 4. Check invalid element
 */
TEST_F(XMLParserTests, getXMLPartitionQos_NegativeClauses)
{
    uint8_t ident = 1;
    PartitionQosPolicy partition;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <partitionQosPolicy>\
                %s\
            </partitionQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
    }

    // Void args
    {
        const char* tag =
                "\
        ";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
}

/*
 * This test checks the negative cases in the xml child element of <WriterTimes>
 * 1. Check an invalid tag of:
 *      <initialHeartbeatDelay>
 *      <heartbeatPeriod>
 *      <nackResponseDelay>
 *      <nackSupressionDuration>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLWriterTimes_NegativeClauses)
{
    uint8_t ident = 1;
    WriterTimes times;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <writerTimes>\
                %s\
            </writerTimes>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterTimes_wrapper(titleElement, times, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterTimes_wrapper(titleElement, times, ident));
}

/*
 * This test checks the negative cases in the xml child element of <ReaderTimes>
 * 1. Check an invalid tag of:
 *      <initialAcknackDelay>
 *      <heartbeatResponseDelay>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLReaderTimes_NegativeClauses)
{
    uint8_t ident = 1;
    ReaderTimes times;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <readerTimes>\
                %s\
            </readerTimes>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderTimes_wrapper(titleElement, times, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderTimes_wrapper(titleElement, times, ident));
}

/*
 * This test checks the negative cases in the xml child element of <LocatorUDPv4>
 * 1. Check an invalid tag of:
 *      <port>
 *      <address>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLLocatorUDPv4_NegativeClauses)
{
    uint8_t ident = 1;
    Locator_t locator;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <udpv4Locator>\
                %s\
            </udpv4Locator>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorUDPv4_wrapper(titleElement, locator, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorUDPv4_wrapper(titleElement, locator, ident));
}

/*
 * This test checks the negative cases in the xml child element of <HistoryMemoryPolicy>
 * 1. Check no elements
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLHistoryMemoryPolicy_NegativeClauses)
{
    uint8_t ident = 1;
    MemoryManagementPolicy_t historyMemoryPolicy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <historyMemoryPolicy>\
                %s\
            </historyMemoryPolicy>\
            ";
    char xml[1000];

    // Void historyMemoryPolicyType
    {
        const char* tag = "BAD POLICY";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLHistoryMemoryPolicy_wrapper(titleElement, historyMemoryPolicy, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLHistoryMemoryPolicy_wrapper(titleElement, historyMemoryPolicy, ident));
}

/*
 * This test checks the negative cases in the xml child element of <LivelinessQos>
 * 1. Check an invalid tag of:
 *      <kind>
 *      <lease_duration>
 *      <announcement_period>
 * 2. Check invalid <kind> element
 * 3. Check invalid element
 */
TEST_F(XMLParserTests, getXMLLivelinessQos_NegativeClauses)
{
    uint8_t ident = 1;
    LivelinessQosPolicy liveliness;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <livelinessQosPolicy>\
                %s\
            </livelinessQosPolicy>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement, liveliness, ident));
    }

    // Invalid kind
    {
        const char* tag = "<kind> BAD_KIND </kind>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement, liveliness, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement, liveliness, ident));
}

/*
 * This test checks the negative cases in the xml child element of <PublishModeQos>
 * 1. Check invalid <kind> element
 * 2. Check empty <kind> element
 * 3. Check no <kind> element
 * 4. Check invalid element
 */
TEST_F(XMLParserTests, getXMLPublishModeQos_NegativeClauses)
{
    uint8_t ident = 1;
    PublishModeQosPolicy publishMode;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <publishModeQosPolicy>\
                %s\
            </publishModeQosPolicy>\
            ";
    char xml[1000];

    // Invalid kind
    {
        const char* tag = "<kind> BAD_KIND </kind>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    }

    // Empty kind
    {
        const char* tag = "<kind> </kind>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    }

    // Empty kind
    {
        const char* tag = "";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
}

/*
 * This test checks the negative cases in the xml child element of <ParticipantAllocationAttributes>
 * 1. Check an invalid tag of:
 *      <remote_locators>
 *      <total_participants>
 *      <total_readers>
 *      <total_writers>
 *      <send_buffers>
 *      <max_properties>
 *      <max_user_data>
 *      <max_partitions>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLParticipantAllocationAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    RTPSParticipantAllocationAttributes allocation;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <rtpsParticipantAllocationAttributes>\
                %s\
            </rtpsParticipantAllocationAttributes>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLParticipantAllocationAttributes_wrapper(titleElement, allocation, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLParticipantAllocationAttributes_wrapper(titleElement, allocation, ident));
}

/*
 * This test checks the negative cases in the xml child element of <DiscoverySettings>
 * 1. Check an invalid tag of:
 *      <discoveryProtocol>
 *      <ignoreParticipantFlags>
 *      <EDP>
 *      <leaseDuration>
 *      <leaseAnnouncement>
 *      <initialAnnouncements>
 *      <simpleEDP>
 *      <clientAnnouncementPeriod>
 *      <discoveryServersList>
 *      <staticEndpointXMLFilename>
 * 2. Check invalid <EDP> element
 * 3. Check invalid <SimpleEDP <PUBWRITER_SUBREADER>> element
 * 4. Check invalid <SimpleEDP <PUBREADER_SUBWRITER>> element
 * 5. Check invalid element
 */
TEST_F(XMLParserTests, getXMLDiscoverySettings_NegativeClauses)
{
    uint8_t ident = 1;
    DiscoverySettings settings;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <discoverySettings>\
                %s\
            </discoverySettings>\
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
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Bad EDP
    {
        const char* tag = "<EDP> BAD_EDP </EDP>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Bad simpleEDP PUBWRITER_SUBREADER
    {
        const char* tag = "<simpleEDP> <PUBWRITER_SUBREADER> </PUBWRITER_SUBREADER> </simpleEDP>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Bad simpleEDP PUBREADER_SUBWRITER
    {
        const char* tag = "<simpleEDP> <PUBREADER_SUBWRITER> </PUBREADER_SUBWRITER> </simpleEDP>";
        sprintf(xml, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
}

/*
 * This test checks the negative cases in the xml child element of <SendBuffersAllocationAttributes>
 * 1. Check an invalid tag of:
 *      <preallocated_number>
 *      <dynamic>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLSendBuffersAllocationAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    SendBuffersAllocationAttributes allocation;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <sendBuffersAllocationConfig>\
                %s\
            </sendBuffersAllocationConfig>\
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
        "preallocated_number",
        "dynamic",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLSendBuffersAllocationAttributes_wrapper(titleElement, allocation, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLSendBuffersAllocationAttributes_wrapper(titleElement, allocation, ident));
}

/*
 * This test checks the negative cases in the xml child element of <RemoteLocatorsAllocationAttributes>
 * 1. Check an invalid tag of:
 *      <max_unicast_locators>
 *      <max_multicast_locators>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLRemoteLocatorsAllocationAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    RemoteLocatorsAllocationAttributes allocation;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <remoteLocatorsAllocationConfig>\
                %s\
            </remoteLocatorsAllocationConfig>\
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
        "max_unicast_locators",
        "max_multicast_locators",
    };

    for (std::string tag : field_vec)
    {
        sprintf(field, field_p, tag.c_str(), tag.c_str());
        sprintf(xml, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLRemoteLocatorsAllocationAttributes_wrapper(titleElement, allocation, ident));
    }

    // Invalid element
    sprintf(xml, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLRemoteLocatorsAllocationAttributes_wrapper(titleElement, allocation, ident));
}

/*
 * This test checks the negative cases of parsing an enum type field with getXMLEnum
 * 1. Check XMLEnum with arg IntraprocessDeliveryType
 *      1. null input
 *      2. empty input
 *      3. invalid input
 * 2. Check XMLEnum with arg DiscoveryProtocol_t
 *      1. null input
 *      2. empty input
 *      3. invalid input
 * 3. Check XMLEnum with arg ParticipantFilteringFlags_t
 *      1. null input
 *      2. empty input
 *      3. invalid input
 */
TEST_F(XMLParserTests, getXMLEnum_NegativeClauses)
{
    uint8_t ident = 1;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    char xml[1000];

    // IntraprocessDeliveryType Enum
    {
        IntraprocessDeliveryType e;
        const char* enum_p =
                "\
                <IntraprocessDelivery>\
                    %s\
                </IntraprocessDelivery>\
                ";

        // null input
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLEnum_wrapper(static_cast<tinyxml2::XMLElement*>(nullptr), &e, ident));

        // void tag
        sprintf(xml, enum_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));

        // Invalid argument
        sprintf(xml, enum_p, "BAD FIELD");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
    }

    // DiscoveryProtocol Enum
    {
        DiscoveryProtocol_t e;
        const char* enum_p =
                "\
                <DiscoveryProtocol>\
                    %s\
                </DiscoveryProtocol>\
                ";

        // null input
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLEnum_wrapper(static_cast<tinyxml2::XMLElement*>(nullptr), &e, ident));

        // void tag
        sprintf(xml, enum_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));

        // Invalid argument
        sprintf(xml, enum_p, "BAD FIELD");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
    }

    // ParticipantFilteringFlags_t Enum
    {
        ParticipantFilteringFlags_t e;
        const char* enum_p =
                "\
                <ParticipantFilteringFlags>%s</ParticipantFilteringFlags>\
                ";

        // null input
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLEnum_wrapper(static_cast<tinyxml2::XMLElement*>(nullptr), &e, ident));

        // void tag
        sprintf(xml, enum_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));

        // Invalid argument
        sprintf(xml, enum_p, "BAD FIELD");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
    }
}

/*
 * This function is not implemented, so this test checks fulfillment of the XMLElementParser coverage
 * 1. Check an error message is received
 */
TEST_F(XMLParserTests, getXMLOctetVector_NegativeClauses)
{
    uint8_t indent = 1;
    std::vector<octet> v;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    const char* xml = "</void>";

    mock_consumer = new MockConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mock_consumer));
    Log::SetCategoryFilter(std::regex("(XMLPARSER)"));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLOctetVector_wrapper(titleElement, v, indent));

    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer->ConsumedEntries();
    // Expect 1 log error.
    uint32_t num_errors = 0;
    for (const auto& entry : consumed_entries)
    {
        if (entry.kind == Log::Kind::Error)
        {
            num_errors++;
        }
    }
    EXPECT_EQ(num_errors, 1u);
}

/*
 * This test checks the positive cases in the xml child element of <XMLEnum>
 * 1. Check XMLEnum with arg IntraprocessDeliveryType
 *      1. INTRAPROCESS_OFF
 * 2. Check XMLEnum with arg DiscoveryProtocol_t
 *      1. NONE
 *      2. CLIENT
 *      3. SERVER
 *      4. BACKUP
 * 3. Check XMLEnum with arg ParticipantFilteringFlags_t
 *      1. FILTER_DIFFERENT_PROCESS
 */
TEST_F(XMLParserTests, getXMLEnum_positive)
{
    uint8_t ident = 1;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    char xml[1000];

    // IntraprocessDeliveryType Enum
    {
        IntraprocessDeliveryType e;
        const char* enum_p =
                "\
                <IntraprocessDelivery>OFF</IntraprocessDelivery>\
                ";

        // INTRAPROCESS_OFF case
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(enum_p));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(IntraprocessDeliveryType::INTRAPROCESS_OFF, e);
    }

    // IntraprocessDeliveryType Enum
    {
        IntraprocessDeliveryType e;
        const char* enum_p =
                "\
                <IntraprocessDelivery>USER_DATA_ONLY</IntraprocessDelivery>\
                ";

        // INTRAPROCESS_OFF case
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(enum_p));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY, e);
    }

    // DiscoveryProtocol Enum
    {
        DiscoveryProtocol_t e;
        const char* enum_p =
                "\
                <DiscoveryProtocol>%s</DiscoveryProtocol>\
                ";

        // NONE case
        sprintf(xml, enum_p, "NONE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol_t::NONE, e);

        // CLIENT case
        sprintf(xml, enum_p, "CLIENT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol_t::CLIENT, e);

        // SERVER case
        sprintf(xml, enum_p, "SERVER");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol_t::SERVER, e);

        // BACKUP case
        sprintf(xml, enum_p, "BACKUP");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol_t::BACKUP, e);
    }

    // ParticipantFilteringFlags_t Enum
    {
        ParticipantFilteringFlags_t e(ParticipantFilteringFlags_t::NO_FILTER);
        const char* enum_p =
                "\
                <ParticipantFilteringFlags>FILTER_DIFFERENT_PROCESS</ParticipantFilteringFlags>\
                ";

        // FILTER_DIFFERENT_PROCESS case
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(enum_p));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(ParticipantFilteringFlags_t::FILTER_DIFFERENT_PROCESS, e);
    }
}


/*
 * This test checks the proper parsing of the <data_sharing> xml elements to a DataSharingQosPolicy object.
 * 1. Correct parsing of a valid <data_sharing> set to AUTO with domain IDs and shared memory directory.
 * 2. Correct parsing of a valid <data_sharing> set to ON with domain IDs and shared memory directory.
 * 3. Correct parsing of a valid <data_sharing> set to OFF with domain IDs and shared memory directory.
 * 4. Correct parsing of a valid <data_sharing> set to AUTO with domain IDs.
 * 5. Correct parsing of a valid <data_sharing> set to ON with domain IDs.
 * 6. Correct parsing of a valid <data_sharing> set to OFF with domain IDs.
 * 7. Correct parsing of a valid <data_sharing> set to AUTO with shared memory directory.
 * 8. Correct parsing of a valid <data_sharing> set to ON with shared memory directory.
 * 9. Correct parsing of a valid <data_sharing> set to OFF with shared memory directory.
 */
TEST_F(XMLParserTests, getXMLDataSharingQos)
{
    uint8_t ident = 1;
    DataSharingQosPolicy datasharing_policy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // Template xml
        const char* xml_p =
                "\
                <data_sharing>\
                    <kind>%s</kind>\
                    <shared_dir>shared_dir</shared_dir>\
                    <domain_ids>\
                        <domainId>10</domainId>\
                        <domainId>20</domainId>\
                    </domain_ids>\
                </data_sharing>\
                ";
        char xml[1000];

        sprintf(xml, xml_p, "AUTOMATIC");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::AUTO);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        sprintf(xml, xml_p, "ON");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::ON);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        sprintf(xml, xml_p, "OFF");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::OFF);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);
    }

    {
        // Template xml
        const char* xml_p =
                "\
                <data_sharing>\
                    <kind>%s</kind>\
                    <max_domains>5</max_domains>\
                    <domain_ids>\
                        <domainId>10</domainId>\
                        <domainId>20</domainId>\
                    </domain_ids>\
                </data_sharing>\
                ";
        char xml[1000];

        sprintf(xml, xml_p, "AUTOMATIC");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::AUTO);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 5u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        sprintf(xml, xml_p, "ON");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::ON);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 5u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        sprintf(xml, xml_p, "OFF");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::OFF);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 5u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);
    }

    {
        // Template xml
        const char* xml_p =
                "\
                <data_sharing>\
                    <kind>%s</kind>\
                    <shared_dir>shared_dir</shared_dir>\
                </data_sharing>\
                ";
        char xml[1000];

        sprintf(xml, xml_p, "AUTOMATIC");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::AUTO);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);

        sprintf(xml, xml_p, "ON");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::ON);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);

        sprintf(xml, xml_p, "OFF");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::OFF);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);
    }
}

/*
 * This test checks the negative cases on parsing of the <data_sharing> xml elements to a DataSharingQosPolicy object.
 * 1. Check an empty list of <domain_ids>.
 * 2. Check a list of <domain_ids> with more domains than the maximum.
 * 3. Check no kind.
 * 4. Check an invalid kind.
 * 5. Check a negative max_domains.
 * 6. Check empty shared_dir.
 * 7. Check invalid tags (at different levels)
 */
TEST_F(XMLParserTests, getXMLDataSharingQos_negativeCases)
{
    uint8_t ident = 1;
    DataSharingQosPolicy datasharing_policy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>AUTOMATIC</kind>\
                    <domain_ids>\
                    </domain_ids>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }

    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>AUTOMATIC</kind>\
                    <max_domains>1</max_domains>\
                    <domain_ids>\
                        <domainId>10</domainId>\
                        <domainId>20</domainId>\
                    </domain_ids>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }

    {
        const char* xml =
                "\
                <data_sharing>\
                    <domain_ids>\
                        <domainId>10</domainId>\
                        <domainId>20</domainId>\
                    </domain_ids>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }

    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>INVALID</kind>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }


    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>AUTOMATIC</kind>\
                    <max_domains>-1</max_domains>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }

    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>AUTOMATIC</kind>\
                    <shared_dir></shared_dir>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }

    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>AUTOMATIC</kind>\
                    <invalid_tag>value</invalid_tag>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }

    {
        const char* xml =
                "\
                <data_sharing>\
                    <kind>AUTOMATIC</kind>\
                    <domain_ids>\
                        <domainId>10</domainId>\
                        <domainId>20</domainId>\
                        <invalid_tag>value</invalid_tag>\
                    </domain_ids>\
                </data_sharing>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
    }
}
