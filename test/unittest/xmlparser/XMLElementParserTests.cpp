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

#include <cstdlib>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <tinyxml2.h>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <xmlparser/XMLParser.h>
#include <xmlparser/XMLProfileManager.h>
#include <xmlparser/XMLTree.h>

#include "../common/env_var_utils.hpp"
#include "../logging/mock/MockConsumer.h"
#include "xmlparser/XMLParserUtils.hpp"
#include "wrapper/XMLParserTest.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds;
using namespace eprosima::testing;

using xmlparser::BaseNode;
using xmlparser::DataNode;
using xmlparser::NodeType;
using xmlparser::XMLP_ret;
using xmlparser::XMLParser;
using xmlparser::up_participant_t;
using xmlparser::up_node_participant_t;
using xmlparser::node_participant_t;
using xmlparser::sp_transport_t;

using xmlparser::XMLProfileManager;

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
        mock_consumer->wait_for_at_least_entries(amount);
    }

    MockConsumer* mock_consumer;

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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLifespanQos_wrapper(titleElement, lifespan, ident));
    EXPECT_EQ(lifespan.duration.seconds, 5);
    EXPECT_EQ(lifespan.duration.nanosec, 0u);

    // Missing data
    snprintf(xml, xml_len, xml_p, "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLifespanQos_wrapper(titleElement, lifespan, ident));

    // Invalid element
    snprintf(xml, xml_len, xml_p, "5", "0", "<bad_element></bad_element>");
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
 * This test checks the proper parsing of an octet vecotr xml element, and negative cases.
 * 1. Correct parsing of a valid element with hexadecimal numbers.
 * 2. Check an bad element with a wrong separator.
 * 3. Check an bad element with a wrong number.
 * 4. Check an bad element with a number too high.
 * 5. Check an  empty xml definition.
 */
TEST_F(XMLParserTests, getXMLOctetVector)
{
    uint8_t ident = 1;
    std::vector<octet> octet_vector;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <root>\
                <value>%s</value>\
            </root>\
            ";
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML with hexadecimal numbers
    snprintf(xml, xml_len, xml_p, "10.20.30.40.50");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLOctetVector_wrapper(titleElement, octet_vector, ident));
    ASSERT_EQ(octet_vector, std::vector<octet>({0x10, 0x20, 0x30, 0x40, 0x50}));
    octet_vector.clear();

    // Invalid XML with wrong separator
    snprintf(xml, xml_len, xml_p, "1,2.3");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLOctetVector_wrapper(titleElement, octet_vector, ident));
    octet_vector.clear();

    // Invalid XML with wrong number
    snprintf(xml, xml_len, xml_p, "1.h.3");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLOctetVector_wrapper(titleElement, octet_vector, ident));
    octet_vector.clear();

    // Invalid XML with too high number
    snprintf(xml, xml_len, xml_p, "1.1F1.3");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLOctetVector_wrapper(titleElement, octet_vector, ident));
    octet_vector.clear();
}

/*
 * This test checks the proper parsing of the <disablePositiveACKs> xml element to DisablePositiveACKsQosPolicy,
 * and negative cases.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <enabled>.
 * 3. Check an empty definition of <sec> and <nanosec> in <duration> child xml element and empty <enabled>.
 * 4. Check an bad element as a child xml element.
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "true", "5", "0", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));
    EXPECT_EQ(disablePositiveACKs.enabled, true);
    EXPECT_EQ(disablePositiveACKs.enabled, true);
    EXPECT_EQ(disablePositiveACKs.duration.seconds, 5);
    EXPECT_EQ(disablePositiveACKs.duration.nanosec, 0u);

    // Missing data - enabled
    snprintf(xml, xml_len, xml_p, "", "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));

    // Missing data - duration
    snprintf(xml, xml_len, xml_p, "true", "", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));

    // Invalid element
    snprintf(xml, xml_len, xml_p, "true", "5", "0", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLDisablePositiveAcksQos_wrapper(titleElement, disablePositiveACKs, ident));
}

/*
 * This test checks the proper parsing of the <disable_heartbeat_piggyback> xml element to WriterQos,
 * and negative cases.
 * 1. Correct parsing of a valid element.
 * 2. Check an bad element as a child xml element.
 * 3. Check an  empty xml definition.
 */
TEST_F(XMLParserTests, get_xml_disable_heartbeat_piggyback)
{
    uint8_t ident = 1;
    WriterQos writer_qos;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <qos>\
                <disable_heartbeat_piggyback>%s</disable_heartbeat_piggyback>\
            </qos>\
            ";
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "true");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK,
            XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, writer_qos, ident));
    EXPECT_EQ(true, writer_qos.disable_heartbeat_piggyback);

    // Invalid element
    snprintf(xml, xml_len, xml_p, "fail");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, writer_qos, ident));

    // Missing element
    const char* miss_xml =
            "\
            <qos>\
                <disable_heartbeat_piggyback></disable_heartbeat_piggyback>\
            </qos>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(miss_xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, writer_qos, ident));
}

/*
 * This test checks the parsing of a <udpv4> element from a list of locators.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <port> .
 * 3. Check an empty definition of <address>.
 * 4. Check an bad element as a child xml element.
 */
TEST_F(XMLParserTests, getXMLLocatorUDPv4)
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
                    <udpv4>\
                        <port>%s</port>\
                        <address>%s</address>\
                        %s\
                    </udpv4>\
                </locator>\
            </unicastLocatorList>\
            ";
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "8844", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(list.begin()->port, 8844u);
    EXPECT_EQ(list.begin()->address[12], 192);
    EXPECT_EQ(list.begin()->address[13], 168);
    EXPECT_EQ(list.begin()->address[14], 1);
    EXPECT_EQ(list.begin()->address[15], 55);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_UDPv4);

    // Missing data - port
    snprintf(xml, xml_len, xml_p, "", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    snprintf(xml, xml_len, xml_p, "8844", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    snprintf(xml, xml_len, xml_p, "8844", "192.168.1.55", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <udpv4> element from a list of locators, using DNS resolution.
 * 1. Correct parsing of a valid element (with address given by domain).
 * 2. Check parsing an element with invalid domain name as address fails.
 */
TEST_F(XMLParserTests, getXMLLocatorDNSUDPv4)
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
                    <udpv4>\
                        <port>%s</port>\
                        <address>%s</address>\
                        %s\
                    </udpv4>\
                </locator>\
            </unicastLocatorList>\
            ";
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "8844", "www.acme.com.test", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(list.begin()->port, 8844u);
    EXPECT_EQ(list.begin()->address[12], 216);
    EXPECT_EQ(list.begin()->address[13], 58);
    EXPECT_EQ(list.begin()->address[14], 215);
    EXPECT_EQ(list.begin()->address[15], 164);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_UDPv4);

    // Invalid domain name address
    snprintf(xml, xml_len, xml_p, "8844", "hopefully.invalid.domain.name", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "8844", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(list.begin()->port, 8844u);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_UDPv6);

    // Missing data - port
    snprintf(xml, xml_len, xml_p, "", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    snprintf(xml, xml_len, xml_p, "8844", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    snprintf(xml, xml_len, xml_p, "8844", "::1", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <udpv6> element from a list of locators, using DNS resolution.
 * 1. Correct parsing of a valid element (with address given by domain).
 * 2. Check parsing an element with invalid domain name as address fails.
 */
TEST_F(XMLParserTests, getXMLLocatorDNSUDPv6)
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "8844", "www.acme.com.test", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    Locator_t expected_locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, "2a00:1450:400e:803::2004", 8844, expected_locator);
    EXPECT_EQ(*list.begin(), expected_locator);

    // Invalid domain name address
    snprintf(xml, xml_len, xml_p, "8844", "hopefully.invalid.domain.name", "");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");

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
    snprintf(xml, xml_len, xml_p, "", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - port
    snprintf(xml, xml_len, xml_p, "5100", "", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - unique_lan_id
    snprintf(xml, xml_len, xml_p, "5100", "8844", "", "80.80.99.45", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - wan_address
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "", "192.168.1.55", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "80.80.99.45", "192.168.1.55",
            "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <tcpv4> element from a list of locators, using DNS resolution.
 * 1. Correct parsing of a valid element (with addresses given by domain).
 * 2. Check parsing an element with invalid domain name as wan address fails.
 * 3. Check parsing an element with invalid domain name as address fails.
 */
TEST_F(XMLParserTests, getXMLLocatorDNSTCPv4)
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "www.acme.com.test", "localhost", "");

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
    EXPECT_EQ(list.begin()->address[8], 216);
    EXPECT_EQ(list.begin()->address[9], 58);
    EXPECT_EQ(list.begin()->address[10], 215);
    EXPECT_EQ(list.begin()->address[11], 164);

    // <address>
    EXPECT_EQ(list.begin()->address[12], 127);
    EXPECT_EQ(list.begin()->address[13], 0);
    EXPECT_EQ(list.begin()->address[14], 0);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_TCPv4);

    // Invalid domain name wan address
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "hopefully.invalid.domain.name", "localhost",
            "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid domain name address
    snprintf(xml, xml_len, xml_p, "5100", "8844", "192.168.1.1.1.1.2.55", "www.acme.com.test",
            "hopefully.invalid.domain.name", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <tcpv6> element from a list of locators.
 * 1. Correct parsing of a valid element.
 * 2. Check an empty definition of <physical_port> .
 * 3. Check an empty definition of <port>.
 * 4. Check an empty definition of <address>.
 * 5. Check an bad element as a child xml element.
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "5100", "8844", "::1", "");

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    EXPECT_EQ(IPLocator::getPhysicalPort(list.begin()->port), 5100u);
    EXPECT_EQ(IPLocator::getLogicalPort(list.begin()->port), 8844u);
    EXPECT_EQ(list.begin()->address[15], 1);
    EXPECT_EQ(list.begin()->kind, LOCATOR_KIND_TCPv6);

    // Missing data - physical_port
    snprintf(xml, xml_len, xml_p,  "", "8844", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - port
    snprintf(xml, xml_len, xml_p,  "5100", "", "::1", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Missing data - address
    snprintf(xml, xml_len, xml_p,  "5100", "8844", "", "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));

    // Invalid element
    snprintf(xml, xml_len, xml_p, "5100", "8844", "::1", "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
}

/*
 * This test checks the parsing of a <tcpv6> element from a list of locators, using DNS resolution.
 * 1. Correct parsing of a valid element (with address given by domain).
 * 2. Check parsing an element with invalid domain name as address fails.
 */
TEST_F(XMLParserTests, getXMLLocatorDNSTCPv6)
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "5100", "8844", "www.acme.com.test", "");

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLLocatorList_wrapper(titleElement, list, ident));
    Locator_t expected_locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, "2a00:1450:400e:803::2004", 0, expected_locator);
    IPLocator::setPhysicalPort(expected_locator, 5100u);
    IPLocator::setLogicalPort(expected_locator, 8844u);
    EXPECT_EQ(*list.begin(), expected_locator);

    // Invalid domain name address
    snprintf(xml, xml_len, xml_p, "5100", "8844", "hopefully.invalid.domain.name", "");
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
 * 4. Check an empty definition of <transport_descriptor>.
 * 5. Check an empty list of transports.
 */
TEST_F(XMLParserTests, getXMLTransports)
{
    uint8_t ident = 1;
    std::vector<std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface>> transports;
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // Valid XML
    snprintf(xml, xml_len, xml_p, "ExampleTransportId1");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLTransports_wrapper(titleElement, transports, ident));
    EXPECT_EQ(transports[0]->max_message_size(), 31416u);

    // Wrong ID
    snprintf(xml, xml_len, xml_p, "WrongTransportId");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTransports_wrapper(titleElement, transports, ident));

    // Missing data
    snprintf(xml, xml_len, xml_p, "");
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
 * This test checks the proper parsing of the <builtinTransports> xml elements and negative cases.
 * 1. Correct parsing of all valid values of BuiltinTransport.
 * 2. Check a wrong definition of <builtinTransports>.
 * 3. Check an empty definition of <builtinTransports>.
 * 4. Correct parsing arguments of LARGE_DATA.
 * 4.a-4.d. Check all possible combinations of arguments for LARGE_DATA.
 */
TEST_F(XMLParserTests, getXMLbuiltinTransports)
{
    uint8_t ident = 1;
    eprosima::fastdds::rtps::BuiltinTransports bt;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <builtinTransports>%s</builtinTransports>\
            ";
    constexpr size_t xml_len {500};
    char xml[xml_len];

    // 1. Valid XML with all possible builtin transports
    std::vector<std::string> bt_list;
    bt_list.push_back("NONE");
    bt_list.push_back("DEFAULT");
    bt_list.push_back("DEFAULTv6");
    bt_list.push_back("SHM");
    bt_list.push_back("UDPv4");
    bt_list.push_back("UDPv6");
    bt_list.push_back("LARGE_DATA");
    bt_list.push_back("LARGE_DATAv6");
    bt_list.push_back("P2P");

    for (auto test_transport : bt_list)
    {
        snprintf(xml, xml_len, xml_p, test_transport.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident));
    }

    // 2. Wrong Value
    snprintf(xml, xml_len, xml_p, "WrongBuiltinTransport");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident));

    // 3. Missing data
    snprintf(xml, xml_len, xml_p, "");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident));

    // 4. LARGE_DATA with arguments
    const char* xml_arguments =
            "\
            <builtinTransports%s>LARGE_DATA</builtinTransports>\
            ";
    eprosima::fastdds::rtps::BuiltinTransportsOptions bt_opts;
    eprosima::fastdds::rtps::BuiltinTransportsOptions bt_opts_check;

    // 4.a. Only max_msg_size
    std::string arguments = " max_msg_size=\"50KIB\"";
    bt_opts_check.maxMessageSize = 50 * 1024;
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.b. Only sockets_buffers_size
    arguments = " sockets_size=\"50KB\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check.sockets_buffer_size = 50000;
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.c. Only non_blocking_send
    arguments = " non_blocking=\"true\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check.non_blocking_send = true;
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.d. Only tcp_negotiation_timeout
    arguments = " tcp_negotiation_timeout=\"50\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check.tcp_negotiation_timeout = 50;
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.e. All options
    arguments = " max_msg_size=\"1MB\" non_blocking=\"true\" sockets_size=\"1MB\" tcp_negotiation_timeout=\"50\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check.maxMessageSize = 1000000;
    bt_opts_check.sockets_buffer_size = 1000000;
    bt_opts_check.non_blocking_send = true;
    bt_opts_check.tcp_negotiation_timeout = 50;
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.f. Wrong units for argument defaults in LARGE_DATA with default config options
    arguments = " max_msg_size=\"50TB\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.g. Exceed maximum value for argument defaults in LARGE_DATA with default config options
    arguments = " sockets_size=\"5000000000\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.h. Wrong value for argument defaults in LARGE_DATA with default config options
    arguments = " non_blocking=\"treu\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // 4.i. Wrong value for argument defaults in LARGE_DATA with default config options
    arguments = " tcp_negotiation_timeout=\"5000000000\"";
    bt_opts = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    bt_opts_check = eprosima::fastdds::rtps::BuiltinTransportsOptions();
    snprintf(xml, xml_len, xml_arguments, arguments.c_str());
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    ASSERT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinTransports_wrapper(titleElement, &bt, ident, &bt_opts));
    ASSERT_EQ(bt_opts, bt_opts_check);

    // Clean up
    xmlparser::XMLProfileManager::DeleteInstance();
}

/*
 * This test checks the proper parsing of the <property_policy> xml elements to a PropertyPolicy object, and negative
 * cases.
 * 1. Correct parsing of a valid <property_policy>.
 * 2. Check missing values for the possible elemnts of the properties.
 * 3. Check an empty list of <properties>.
 * 4. Check an empty list of <binary_properties>.
 * 5. Check an wrong descriptor for properties.
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
        "01.02.CA.FE",
        "false"};
    const std::vector<std::string> wrong_parameters {
        "",
        "",
        "",
        "",
        "ZZ",
        ""};
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
                        <value>%s</value>\
                        <propagate>%s</propagate>\
                    </property>\
                </binary_properties>\
            </propertiesPolicy>\
            ";
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    snprintf(xml, xml_len, xml_p,
            valid_parameters[0].c_str(),
            valid_parameters[1].c_str(),
            valid_parameters[2].c_str(),
            valid_parameters[3].c_str(),
            valid_parameters[4].c_str(),
            valid_parameters[5].c_str());

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, property_policy, ident));
    EXPECT_EQ(property_policy.properties()[0].name(), valid_parameters[0]);
    EXPECT_EQ(property_policy.properties()[0].value(), valid_parameters[1]);
    EXPECT_EQ(property_policy.properties()[0].propagate(), false);
    EXPECT_EQ(property_policy.binary_properties()[0].name(), valid_parameters[3]);
    EXPECT_EQ(property_policy.binary_properties()[0].value(), std::vector<uint8_t>({0x01, 0x02, 0xCA, 0xFE}));
    EXPECT_EQ(property_policy.binary_properties()[0].propagate(), false);

    for (size_t i = 0; i < valid_parameters.size(); i++)
    {
        parameters = valid_parameters;
        parameters[i] = wrong_parameters[i];

        snprintf(xml, xml_len, xml_p,
                parameters[0].c_str(),
                parameters[1].c_str(),
                parameters[2].c_str(),
                parameters[3].c_str(),
                parameters[4].c_str(),
                parameters[5].c_str());
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
            parameters.assign (8, "1");
            parameters[i] = "";
            xml =
                    "<port>"
                    "    <portBase>" + parameters[0] + "</portBase>"
                    "    <domainIDGain>" + parameters[1] + "</domainIDGain>"
                    "    <participantIDGain>" + parameters[2] + "</participantIDGain>"
                    "    <offsetd0>" + parameters[3] + "</offsetd0>"
                    "    <offsetd1>" + parameters[4] + "</offsetd1>"
                    "    <offsetd2>" + parameters[5] + "</offsetd2>"
                    "    <offsetd3>" + parameters[6] + "</offsetd3>"
                    "    <offsetd4>" + parameters[7] + "</offsetd4>"
                    "</port>";
        }
        else
        {
            xml =
                    "<port>"
                    "    <bad_element></bad_element>"
                    "</port>";
        }

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPortParameters_wrapper(titleElement, port, ident));
    }
}

/*
 * This test checks the negative cases of a <subscriber> xml profile.
 * 1. Check an incorrect for each of the possible xmlparser::SubscriberAttributes.
 * 2. Check an non existant attribute.
 */
TEST_F(XMLParserTests, getXMLSubscriberAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    xmlparser::SubscriberAttributes attr;
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
        "<expects_inline_qos><bad_element></bad_element></expects_inline_qos>",
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
 * 1. Check an incorrect for each of the possible xmlparser::PublisherAttributes.
 * 2. Check an non existant attribute.
 */
TEST_F(XMLParserTests, getXMLPublisherAttributes_NegativeClauses)
{
    uint8_t ident = 1;
    xmlparser::PublisherAttributes attr;
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
    dds::Duration_t duration;
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
    dds::Duration_t duration;
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
        EXPECT_EQ(duration, dds::c_TimeInfinite);
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
    uint64_t ui64;
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

    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLUint_wrapper(nullptr, &ui64, ident));

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<field>not_an_uint</field>"));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLUint_wrapper(titleElement, &ui64, ident));
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

    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Check an empty definition of <count> child xml element.
    snprintf(xml, xml_len, xml_p, "", "", "5", "123");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));

    // Check an empty definition of <sec> in <period> child xml element.
    snprintf(xml, xml_len, xml_p, "", "5", "", "123");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));

    // Check an empty definition of <nanosec> in <period> child xml element.
    snprintf(xml, xml_len, xml_p, "", "5", "5", "");
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
 * 9. Check an wrong definition of <userData> xml element.
 * 10. Check an wrong definition of <topicData> xml element.
 * 11. Check an wrong definition of <groupData> xml element.
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

    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Check an empty definition of <durability> xml element.
    snprintf(xml, xml_len, xml_p, "<durability></durability>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <liveliness> xml element.
    snprintf(xml, xml_len, xml_p, "<liveliness><kind></kind></liveliness>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <reliability> xml element.
    snprintf(xml, xml_len, xml_p, "<reliability><kind></kind></reliability>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <partition> xml element.
    snprintf(xml, xml_len, xml_p, "<partition></partition>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <publishMode> xml element.
    snprintf(xml, xml_len, xml_p, "<publishMode></publishMode>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));

    // Check an empty definition of <deadline> xml element.
    snprintf(xml, xml_len, xml_p, "<deadline></deadline>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <disablePositiveAcks> xml element.
    snprintf(xml, xml_len, xml_p, "<disablePositiveAcks><enabled></enabled></disablePositiveAcks>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));

    // Check an empty definition of <latencyBudget> xml element.
    snprintf(xml, xml_len, xml_p, "<latencyBudget></latencyBudget>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));

    // Check an empty definition of <lifespan> xml element.
    snprintf(xml, xml_len, xml_p, "<lifespan></lifespan>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <latencyBudget> xml element.
    snprintf(xml, xml_len, xml_p, "<latencyBudget></latencyBudget>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an empty definition of <disablePositiveAcks> xml element.
    snprintf(xml, xml_len, xml_p, "<disablePositiveAcks><enabled></enabled></disablePositiveAcks>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an wrong definition of <userData> xml element.
    snprintf(xml, xml_len, xml_p, "<userData><bad_element></bad_element></userData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));
    snprintf(xml, xml_len, xml_p, "<userData><value>1</value><value>2</value></userData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an wrong definition of <topicData> xml element.
    snprintf(xml, xml_len, xml_p, "<topicData><bad_element></bad_element></topicData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));
    snprintf(xml, xml_len, xml_p, "<topicData><value>1</value><value>2</value></topicData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check an wrong definition of <groupData> xml element.
    snprintf(xml, xml_len, xml_p, "<groupData><bad_element></bad_element></groupData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));
    snprintf(xml, xml_len, xml_p, "<groupData><value>1</value><value>2</value></groupData>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check a wrong xml element definition inside <qos>
    snprintf(xml, xml_len, xml_p, "<bad_element></bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));
}

/*
 * This test checks that there is a EPROSIMA_LOG_ERROR when setting up a non supported <data_writer>/<data_reader> qos
 * 1. Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <durabilityService> Qos.
 * 3. Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <timeBasedFilter> Qos.
 * 4. Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <ownership> Qos.
 * 5. Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <ownershipStrength> Qos.
 * 6. Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <destinationOrder> Qos.
 * 7. Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <presentation> Qos.
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

    constexpr size_t xml_len {600};
    char xml[xml_len];

    // Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <durabilityService> Qos.
    snprintf(xml, xml_len, xml_p, "<durabilityService></durabilityService>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <timeBasedFilter> Qos.
    snprintf(xml, xml_len, xml_p, "<timeBasedFilter></timeBasedFilter>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <destinationOrder> Qos.
    snprintf(xml, xml_len, xml_p, "<destinationOrder></destinationOrder>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    // Check that there is a EPROSIMA_LOG_ERROR when trying to set up the <presentation> Qos.
    snprintf(xml, xml_len, xml_p, "<presentation></presentation>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLWriterQosPolicies_wrapper(titleElement, wqos, ident));
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLReaderQosPolicies_wrapper(titleElement, rqos, ident));

    helper_block_for_at_least_entries(8);
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
    EXPECT_EQ(num_errors, 8u);
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
    eprosima::fastdds::rtps::RTPSParticipantAllocationAttributes allocation;
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
    eprosima::fastdds::rtps::DiscoverySettings settings;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // XML snippet
    const char* xml =
            "\
            <discovery_config>\
                <EDP>STATIC</EDP>\
                <static_edp_xml_config>file://my_static_edp.xml</static_edp_xml_config>\
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
    EXPECT_STREQ(settings.static_edp_xml_config(), "file://my_static_edp.xml");
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
    constexpr size_t xml_len {500};
    char xml[xml_len];
    for (const auto& policy : policies)
    {
        // Load the xml
        snprintf(xml, xml_len, xml_p, policy.first.c_str());
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
    constexpr size_t xml_len {500};
    char xml[xml_len];

    std::vector<std::string> kinds = {"VOLATILE", "TRANSIENT_LOCAL", "TRANSIENT", "PERSISTENT"};

    snprintf(xml, xml_len, xml_p, "VOLATILE");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy VOLATILE kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to VOLATILE.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS);

    snprintf(xml, xml_len, xml_p, "TRANSIENT_LOCAL");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy TRANSIENT_LOCAL kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to TRANSIENT_LOCAL.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS);

    snprintf(xml, xml_len, xml_p, "TRANSIENT");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    // Check that the XML return code is correct for the durability QoS policy TRANSIENT kind.
    EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    // Check that the durability QoS policy kind is set to TRANSIENT.
    EXPECT_EQ(durability.kind, DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS);

    snprintf(xml, xml_len, xml_p, "PERSISTENT");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

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
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinAttributes_wrapper(titleElement, builtin, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLBuiltinAttributes_wrapper(titleElement, builtin, ident));
}

/*
 * This test checks parsing of flow_controller_descriptor_list elements.
 */
TEST_F(XMLParserTests, getXMLFlowControllerDescriptorList)
{
    uint8_t ident = 1;

    /*
     * Aux mapping for FlowControllerSchedulerPolicy
     */

    auto scheduler_policy_map = std::map<std::string, FlowControllerSchedulerPolicy>
    {
        {"FIFO", FlowControllerSchedulerPolicy::FIFO},
        {"ROUND_ROBIN", FlowControllerSchedulerPolicy::ROUND_ROBIN},
        {"HIGH_PRIORITY", FlowControllerSchedulerPolicy::HIGH_PRIORITY},
        {"PRIORITY_WITH_RESERVATION", FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION}
    };

    /* Define the test cases */
    std::vector<std::pair<std::vector<std::string>, XMLP_ret>> test_cases =
    {
        /*
         * name, scheduler, max_bytes_per_period, period_ms,
         * sender_thread_scheduling_policy, sender_thread_priority,
         * sender_thread_affinity, sender_thread_stack_size, extra_xml_tag
         */
        {{"test_flow_controller", "FIFO", "120", "50", \
            "12", "12", "12", "12", "" }, XMLP_ret::XML_OK},
        {{"test_flow_controller", "ROUND_ROBIN", "2500", "100", \
            "15", "12", "12", "12", "" }, XMLP_ret::XML_OK},
        {{"test_flow_controller", "HIGH_PRIORITY", "2500", "100", \
            "15", "12", "12", "12", "" }, XMLP_ret::XML_OK},
        {{"test_flow_controller", "PRIORITY_WITH_RESERVATION", "2500", "100", \
            "15", "12", "12", "12", "" }, XMLP_ret::XML_OK},
        {{"test_flow_controller", "INVALID", "120", "50", \
            "12", "12", "12", "12", "" }, XMLP_ret::XML_ERROR},   // Invalid scheduler
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "-10", \
            "12", "12", "12", "12", "" }, XMLP_ret::XML_ERROR},   // negative period_ms
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "<bad_element></bad_element>" }, XMLP_ret::XML_ERROR},   // Invalid tag
        {{"", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "" }, XMLP_ret::XML_ERROR},   // empty name
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "<name>another_name</name>" }, XMLP_ret::XML_ERROR},   // duplicated name tag
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "<scheduler>FIFO</scheduler>" }, XMLP_ret::XML_ERROR},   // duplicated scheduler tag
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "<max_bytes_per_period>96</max_bytes_per_period>" }, XMLP_ret::XML_ERROR},   // duplicated max_bytes_per_period tag
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "<period_ms>96</period_ms>" }, XMLP_ret::XML_ERROR},   // duplicated period_ms tag
        {{"test_flow_controller", "HIGH_PRIORITY", "120", "50", \
            "12", "12", "12", "12", "<sender_thread><scheduling_policy>12</scheduling_policy></sender_thread>" },
            XMLP_ret::XML_ERROR}, // duplicated sender_thread tag
        {{"", "HIGH_PRIORITY", "120", "50", \
            "12345", "12", "12", "a", "" }, XMLP_ret::XML_ERROR},   // invalid thread settings
    };

    /* Run the tests */
    for (auto test_case : test_cases)
    {
        std::vector<std::string>& params = test_case.first;
        XMLP_ret& expectation = test_case.second;

        using namespace eprosima::fastdds::rtps;
        XMLParserTest::FlowControllerDescriptorList flow_controller_descriptor_list;
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        // Create XML snippet
        std::string xml =
                "<flow_controller_descriptor_list>"
                "   <flow_controller_descriptor>"
                "       <name>" + params[0] + "</name>"
                "       <scheduler>" + params[1] + "</scheduler>"
                "       <max_bytes_per_period>" + params[2] + "</max_bytes_per_period>"
                "       <period_ms>" + params[3] + "</period_ms>"
                "       <sender_thread>"
                "           <scheduling_policy>" + params[4] + "</scheduling_policy>"
                "           <priority>" + params[5] + "</priority>"
                "           <affinity>" + params[6] + "</affinity>"
                "           <stack_size>" + params[7] + "</stack_size>"
                "       </sender_thread>"
                + params[8] +
                "   </flow_controller_descriptor>"
                "</flow_controller_descriptor_list>";

        // Parse the XML snippet
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str())) << xml;

        // Extract FlowControllersDescriptors
        titleElement = xml_doc.RootElement();
        ASSERT_EQ(expectation,
                XMLParserTest::getXMLFlowControllerDescriptorList_wrapper(titleElement, flow_controller_descriptor_list,
                ident));

        // Validate in the OK cases
        if (expectation == XMLP_ret::XML_OK)
        {
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->name, params[0]);
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->scheduler, scheduler_policy_map[params[1]]);
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->max_bytes_per_period,
                    static_cast<int32_t>(std::stoi(params[2])));
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->period_ms, static_cast<uint64_t>(std::stoi(params[3])));
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->sender_thread.scheduling_policy,
                    static_cast<int32_t>(std::stoi(params[4])));
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->sender_thread.priority,
                    static_cast<int32_t>(std::stoi(params[5])));
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->sender_thread.affinity,
                    static_cast<uint64_t>(std::stoi(params[6])));
            ASSERT_EQ(flow_controller_descriptor_list.at(0)->sender_thread.stack_size,
                    static_cast<int32_t>(std::stoi(params[7])));
        }
    }
}

/*
 * This test checks the negative cases in the xml child element of <flow_controller_descriptor_list>
 * 1. Check an invalid tag of:
 *      <flow_controller_descriptor>
 * 2. Check invalid element
 */
TEST_F(XMLParserTests, getXMLFlowControllerDescriptorList_NegativeClauses)
{
    uint8_t ident = 1;
    XMLParserTest::FlowControllerDescriptorList flow_controller_descriptor_list;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <flow_controller_descriptor_list>\
                %s\
            </flow_controller_descriptor_list>\
            ";
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "flow_controller_descriptor"
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLFlowControllerDescriptorList_wrapper(titleElement, flow_controller_descriptor_list,
                ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR,
            XMLParserTest::getXMLFlowControllerDescriptorList_wrapper(titleElement, flow_controller_descriptor_list,
            ident));
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
    xmlparser::TopicAttributes topic;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    // Parametrized XML
    const char* xml_p =
            "\
            <topicAttributes>\
                %s\
            </topicAttributes>\
            ";
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

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
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
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
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLTopicAttributes_wrapper(titleElement, topic, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "max_samples",
        "max_instances",
        "max_samples_per_instance",
        "allocated_samples",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLResourceLimitsQos_wrapper(titleElement, resourceLimitsQos, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "initial",
        "maximum",
        "increment",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
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
        snprintf(xml, xml_len, xml_p, tag);
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
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLContainerAllocationConfig_wrapper(titleElement, allocation_config, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "kind",
        "depth",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
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
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLHistoryQosPolicy_wrapper(titleElement, historyQos, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Invalid kind
    {
        const char* tag =
                "\
                    <kind> BAD_KIND </kind>\
                ";
        snprintf(xml, xml_len, xml_p, tag);
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
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    }

    // No kind
    {
        const char* tag = "";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDurabilityQos_wrapper(titleElement, durability, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Invalid kind
    {
        const char* tag =
                "\
                    <period> BAD_PERIOD </period>\
                ";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement, deadline, ident));
    }

    // No period
    {
        const char* tag = "";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDeadlineQos_wrapper(titleElement, deadline, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Invalid duration
    {
        const char* tag =
                "\
                <duration> BAD_DURATION </duration>\
                ";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement, latencyBudget, ident));
    }

    // No duration
    {
        const char* tag = "";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLLatencyBudgetQos_wrapper(titleElement, latencyBudget, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Invalid kind
    {
        const char* tag =
                "\
                <kind> BAD_KIND </kind>\
                ";
        snprintf(xml, xml_len, xml_p, tag);
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
        snprintf(xml, xml_len, xml_p, tag);
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
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReliabilityQos_wrapper(titleElement, reliability, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Invalid names
    {
        const char* tag =
                "\
                <names>\
                    <name>  </name>\
                </names>\
                ";
        snprintf(xml, xml_len, xml_p, tag);
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
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
    }

    // Void args
    {
        const char* tag =
                "\
        ";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPartitionQos_wrapper(titleElement, partition, ident));
}

/*
 * This test checks the negative cases in the xml child element of <WriterTimes>
 * 1. Check an invalid tag of:
 *      <initial_heartbeat_delay>
 *      <heartbeat_period>
 *      <nack_response_delay>
 *      <nack_supression_duration>
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "initial_heartbeat_delay",
        "heartbeat_period",
        "nack_response_delay",
        "nack_supression_duration",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterTimes_wrapper(titleElement, times, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLWriterTimes_wrapper(titleElement, times, ident));
}

/*
 * This test checks the negative cases in the xml child element of <ReaderTimes>
 * 1. Check an invalid tag of:
 *      <initial_acknack_delay>
 *      <heartbeat_response_delay>
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "initial_acknack_delay",
        "heartbeat_response_delay",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLReaderTimes_wrapper(titleElement, times, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "port",
        "address",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLocatorUDPv4_wrapper(titleElement, locator, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Void historyMemoryPolicyType
    {
        const char* tag = "BAD POLICY";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLHistoryMemoryPolicy_wrapper(titleElement, historyMemoryPolicy, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "kind",
        "lease_duration",
        "announcement_period"
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement, liveliness, ident));
    }

    // Invalid kind
    {
        const char* tag = "<kind> BAD_KIND </kind>";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLLivelinessQos_wrapper(titleElement, liveliness, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // Invalid kind
    {
        const char* tag = "<kind> BAD_KIND </kind>";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    }

    // Empty kind
    {
        const char* tag = "<kind> </kind>";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    }

    // Empty kind
    {
        const char* tag = "";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLPublishModeQos_wrapper(titleElement, publishMode, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

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
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLParticipantAllocationAttributes_wrapper(titleElement, allocation, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
 *      <static_edp_xml_config>
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
        <%s>\
            <bad_element> </bad_element>\
        </%s>\
        ";
    constexpr size_t field_len {500};
    char field[field_len];

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
        "static_edp_xml_config"
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Bad EDP
    {
        const char* tag = "<EDP> BAD_EDP </EDP>";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Bad simpleEDP PUBWRITER_SUBREADER
    {
        const char* tag = "<simpleEDP> <PUBWRITER_SUBREADER> </PUBWRITER_SUBREADER> </simpleEDP>";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Bad simpleEDP PUBREADER_SUBWRITER
    {
        const char* tag = "<simpleEDP> <PUBREADER_SUBWRITER> </PUBREADER_SUBWRITER> </simpleEDP>";
        snprintf(xml, xml_len, xml_p, tag);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.RootElement();
    EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLDiscoverySettings_wrapper(titleElement, settings, ident));
}

/*
 * This test checks the negative cases in the xml child element of <SendBuffersAllocationAttributes>
 * 1. Check an invalid tag of:
 *      <preallocated_number>
 *      <dynamic>
 *      <network_buffers_config>
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "preallocated_number",
        "dynamic",
        "network_buffers_config"
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLSendBuffersAllocationAttributes_wrapper(titleElement, allocation, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    const char* field_p =
            "\
            <%s>\
                <bad_element> </bad_element>\
            </%s>\
            ";
    constexpr size_t field_len {500};
    char field[field_len];

    std::vector<std::string> field_vec =
    {
        "max_unicast_locators",
        "max_multicast_locators",
    };

    for (std::string tag : field_vec)
    {
        snprintf(field, field_len, field_p, tag.c_str(), tag.c_str());
        snprintf(xml, xml_len, xml_p, field);
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLRemoteLocatorsAllocationAttributes_wrapper(titleElement, allocation, ident));
    }

    // Invalid element
    snprintf(xml, xml_len, xml_p, "<bad_element> </bad_element>");
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
 * 2. Check XMLEnum with arg DiscoveryProtocol
 *      1. null input
 *      2. empty input
 *      3. invalid input
 * 3. Check XMLEnum with arg ParticipantFilteringFlags
 *      1. null input
 *      2. empty input
 *      3. invalid input
 */
TEST_F(XMLParserTests, getXMLEnum_NegativeClauses)
{
    uint8_t ident = 1;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // IntraprocessDeliveryType Enum
    {
        eprosima::fastdds::IntraprocessDeliveryType e;
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
        snprintf(xml, xml_len, enum_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));

        // Invalid argument
        snprintf(xml, xml_len, enum_p, "BAD FIELD");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
    }

    // DiscoveryProtocol Enum
    {
        DiscoveryProtocol e;
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
        snprintf(xml, xml_len, enum_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));

        // Invalid argument
        snprintf(xml, xml_len, enum_p, "BAD FIELD");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
    }

    // ParticipantFilteringFlags Enum
    {
        ParticipantFilteringFlags e;
        const char* enum_p =
                "\
                <ParticipantFilteringFlags>%s</ParticipantFilteringFlags>\
                ";

        // null input
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::getXMLEnum_wrapper(static_cast<tinyxml2::XMLElement*>(nullptr), &e, ident));

        // void tag
        snprintf(xml, xml_len, enum_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));

        // Invalid argument
        snprintf(xml, xml_len, enum_p, "BAD FIELD");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
    }
}

/*
 * This test checks the positive cases in the xml child element of <XMLEnum>
 * 1. Check XMLEnum with arg IntraprocessDeliveryType
 *      1. INTRAPROCESS_OFF
 * 2. Check XMLEnum with arg DiscoveryProtocol
 *      1. NONE
 *      2. CLIENT
 *      3. SERVER
 *      4. BACKUP
 *      5. SUPER_CLIENT
 * 3. Check XMLEnum with arg ParticipantFilteringFlags
 *      1. FILTER_DIFFERENT_PROCESS
 */
TEST_F(XMLParserTests, getXMLEnum_positive)
{
    uint8_t ident = 1;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    constexpr size_t xml_len {1000};
    char xml[xml_len];

    // IntraprocessDeliveryType Enum
    {
        eprosima::fastdds::IntraprocessDeliveryType e;
        const char* enum_p =
                "\
                <IntraprocessDelivery>OFF</IntraprocessDelivery>\
                ";

        // INTRAPROCESS_OFF case
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(enum_p));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF, e);
    }

    // IntraprocessDeliveryType Enum
    {
        eprosima::fastdds::IntraprocessDeliveryType e;
        const char* enum_p =
                "\
                <IntraprocessDelivery>USER_DATA_ONLY</IntraprocessDelivery>\
                ";

        // INTRAPROCESS_OFF case
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(enum_p));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY, e);
    }

    // DiscoveryProtocol Enum
    {
        DiscoveryProtocol e;
        const char* enum_p =
                "\
                <DiscoveryProtocol>%s</DiscoveryProtocol>\
                ";

        // NONE case
        snprintf(xml, xml_len, enum_p, "NONE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol::NONE, e);

        // CLIENT case
        snprintf(xml, xml_len, enum_p, "CLIENT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol::CLIENT, e);

        // SERVER case
        snprintf(xml, xml_len, enum_p, "SERVER");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol::SERVER, e);

        // BACKUP case
        snprintf(xml, xml_len, enum_p, "BACKUP");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol::BACKUP, e);

        // SUPER_CLIENT case
        snprintf(xml, xml_len, enum_p, "SUPER_CLIENT");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(DiscoveryProtocol::SUPER_CLIENT, e);
    }

    // ParticipantFilteringFlags Enum
    {
        ParticipantFilteringFlags e(ParticipantFilteringFlags::NO_FILTER);
        const char* enum_p =
                "\
                <ParticipantFilteringFlags>FILTER_DIFFERENT_PROCESS</ParticipantFilteringFlags>\
                ";

        // FILTER_DIFFERENT_PROCESS case
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(enum_p));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::getXMLEnum_wrapper(titleElement, &e, ident));
        EXPECT_EQ(ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS, e);
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
        constexpr size_t xml_len {1000};
        char xml[xml_len];

        snprintf(xml, xml_len, xml_p, "AUTOMATIC");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::AUTO);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        snprintf(xml, xml_len, xml_p, "ON");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::ON);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        snprintf(xml, xml_len, xml_p, "OFF");
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
        constexpr size_t xml_len {1000};
        char xml[xml_len];

        snprintf(xml, xml_len, xml_p, "AUTOMATIC");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::AUTO);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 5u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        snprintf(xml, xml_len, xml_p, "ON");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::ON);
        EXPECT_EQ(datasharing_policy.shm_directory().size(), 0u);
        EXPECT_EQ(datasharing_policy.max_domains(), 5u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 2u);
        EXPECT_EQ(datasharing_policy.domain_ids()[0], 10u);
        EXPECT_EQ(datasharing_policy.domain_ids()[1], 20u);

        snprintf(xml, xml_len, xml_p, "OFF");
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
        constexpr size_t xml_len {1000};
        char xml[xml_len];

        snprintf(xml, xml_len, xml_p, "AUTOMATIC");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::AUTO);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);

        snprintf(xml, xml_len, xml_p, "ON");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, datasharing_policy, ident));
        EXPECT_EQ(datasharing_policy.kind(), DataSharingKind::ON);
        EXPECT_EQ(datasharing_policy.shm_directory(), "shared_dir");
        EXPECT_EQ(datasharing_policy.max_domains(), 0u);
        EXPECT_EQ(datasharing_policy.domain_ids().size(), 0u);

        snprintf(xml, xml_len, xml_p, "OFF");
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

/*
 * This test checks the proper parsing of the <ownership> xml elements to a OwnershipQosPolicy object.
 * 1. Correct parsing of a valid <ownership> set to SHARED.
 * 2. Correct parsing of a valid <ownership> set to EXCLUSIVE.
 * 3. Check no kind.
 * 4. Check an invalid kind.
 */
TEST_F(XMLParserTests, getXMLOwnershipQos)
{
    uint8_t ident = 1;
    OwnershipQosPolicy ownership_policy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // Template xml
        const char* xml_p =
                "\
                <ownership>\
                    <kind>%s</kind>\
                </ownership>\
                ";
        constexpr size_t xml_len {1000};
        char xml[xml_len];

        snprintf(xml, xml_len, xml_p, "SHARED");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_policy, ident));
        EXPECT_EQ(ownership_policy.kind, OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS);

        snprintf(xml, xml_len, xml_p, "EXCLUSIVE");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK, XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_policy, ident));
        EXPECT_EQ(ownership_policy.kind, OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS);
    }

    {
        const char* xml =
                "\
                <ownership>\
                </ownership>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_policy, ident));
    }

    {
        const char* xml =
                "\
                <ownership>\
                    <kind>INVALID</kind>\
                </ownership>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_policy, ident));
    }
}

/*
 * This test checks the proper parsing of the <ownershipStrength> xml elements to a OwnershipQosPolicy object.
 * 1. Correct parsing of a valid <ownershipStrength> value set to 0.
 * 2. Correct parsing of a valid <ownershipStrength> value set to 100.
 * 3. Check no value.
 * 4. Check an invalid value.
 */
TEST_F(XMLParserTests, getXMLOwnershipStrengthQos)
{
    uint8_t ident = 1;
    OwnershipStrengthQosPolicy ownership_strength_policy;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // Template xml
        const char* xml_p =
                "\
                <ownershipStrength>\
                    <value>%s</value>\
                </ownershipStrength>\
                ";
        constexpr size_t xml_len {1000};
        char xml[xml_len];

        snprintf(xml, xml_len, xml_p, "0");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_strength_policy, ident));
        EXPECT_EQ(ownership_strength_policy.value, 0u);

        snprintf(xml, xml_len, xml_p, "100");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_OK,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_strength_policy, ident));
        EXPECT_EQ(ownership_strength_policy.value, 100u);
    }

    {
        const char* xml =
                "\
                <ownershipStrength>\
                </ownershipStrength>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_strength_policy, ident));
    }

    {
        const char* xml =
                "\
                <ownershipStrength>\
                    <value>INVALID</value>\
                </ownershipStrength>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(XMLP_ret::XML_ERROR,
                XMLParserTest::propertiesPolicy_wrapper(titleElement, ownership_strength_policy, ident));
    }
}

TEST_F(XMLParserTests, get_element_text)
{
    using namespace eprosima::fastdds::xml::detail;

    // 1. Empty content
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<elem></elem>"));
        xml_element = xml_doc.RootElement();

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "");
        EXPECT_FALSE(get_element_text(xml_element, result));
    }

    // 2. Plain content
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<elem>Content</elem>"));
        xml_element = xml_doc.RootElement();

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "Content");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Content");
    }
}

TEST_F(XMLParserTests, env_var_substitution)
{
    using namespace eprosima::fastdds::xml::detail;

    const char* const env_var_1 = "XML_PARSER_TESTS_ENV_VAR_1";
    const char* const env_var_2 = "XML_PARSER_TESTS_ENV_VAR_2";

    // 1. Empty var
    {
        clear_environment_variable(env_var_1);
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<elem>${XML_PARSER_TESTS_ENV_VAR_1}</elem>"));
        xml_element = xml_doc.RootElement();

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "");
        EXPECT_FALSE(get_element_text(xml_element, result));
    }

    // 2. Single environment var only
    {
        set_environment_variable(env_var_1, "Content");
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<elem>${XML_PARSER_TESTS_ENV_VAR_1}</elem>"));
        xml_element = xml_doc.RootElement();

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "Content");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Content");
    }

    // 3. Text plus env var
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<elem>Has ${XML_PARSER_TESTS_ENV_VAR_1}</elem>"));
        xml_element = xml_doc.RootElement();

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "Has Content");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Has Content");
    }

    // 4. Text plus empty env var
    {
        clear_environment_variable(env_var_1);

        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse("<elem>Empty ${XML_PARSER_TESTS_ENV_VAR_1}</elem>"));
        xml_element = xml_doc.RootElement();

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "Empty ");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Empty ");
    }

    // 5. Mixing vars
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS,
                xml_doc.Parse("<elem>${XML_PARSER_TESTS_ENV_VAR_1}${XML_PARSER_TESTS_ENV_VAR_2}</elem>"));
        xml_element = xml_doc.RootElement();

        // 5.1. Both empty
        clear_environment_variable(env_var_1);
        clear_environment_variable(env_var_2);

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "");
        EXPECT_FALSE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "");

        // 5.2. First with data
        set_environment_variable(env_var_1, "Content-1");
        clear_environment_variable(env_var_2);

        EXPECT_EQ(get_element_text(xml_element), "Content-1");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Content-1");

        // 5.3. Second with data
        clear_environment_variable(env_var_1);
        set_environment_variable(env_var_2, "Content-2");

        EXPECT_EQ(get_element_text(xml_element), "Content-2");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Content-2");

        // 5.4. Both with data
        set_environment_variable(env_var_1, "Content-1");
        set_environment_variable(env_var_2, "Content-2");

        EXPECT_EQ(get_element_text(xml_element), "Content-1Content-2");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "Content-1Content-2");
    }

    // 6. Mixing text and vars
    {
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* xml_element;
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS,
                xml_doc.Parse("<elem>A mix of ${XML_PARSER_TESTS_ENV_VAR_1} & ${XML_PARSER_TESTS_ENV_VAR_2}!</elem>"));
        xml_element = xml_doc.RootElement();

        // 5.1. Both empty
        clear_environment_variable(env_var_1);
        clear_environment_variable(env_var_2);

        std::string result;
        EXPECT_EQ(get_element_text(xml_element), "A mix of  & !");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "A mix of  & !");

        // 6.2. First with data
        set_environment_variable(env_var_1, "Content-1");
        clear_environment_variable(env_var_2);

        EXPECT_EQ(get_element_text(xml_element), "A mix of Content-1 & !");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "A mix of Content-1 & !");

        // 6.3. Second with data
        clear_environment_variable(env_var_1);
        set_environment_variable(env_var_2, "Content-2");

        EXPECT_EQ(get_element_text(xml_element), "A mix of  & Content-2!");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "A mix of  & Content-2!");

        // 6.4. Both with data
        set_environment_variable(env_var_1, "Content-1");
        set_environment_variable(env_var_2, "Content-2");

        EXPECT_EQ(get_element_text(xml_element), "A mix of Content-1 & Content-2!");
        EXPECT_TRUE(get_element_text(xml_element, result));
        EXPECT_EQ(result, "A mix of Content-1 & Content-2!");
    }

    // Cleanup environment variables used in this test
    clear_environment_variable(env_var_1);
    clear_environment_variable(env_var_2);
}

/*
 * This test checks parsing of thread_settings elements.
 */
TEST_F(XMLParserTests, getXMLThreadSettings)
{
    /* Define the test cases */
    std::vector<std::pair<std::vector<std::string>, XMLP_ret>> test_cases =
    {
        {{"12", "12", "12", "12", ""}, XMLP_ret::XML_OK},
        {{"-1", "12", "12", "12", ""}, XMLP_ret::XML_OK},
        {{"12", "-1", "12", "12", ""}, XMLP_ret::XML_OK},
        {{"12", "12", "12", "-1", ""}, XMLP_ret::XML_OK},
        {{"-2", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12", "12", "-2", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "-2", ""}, XMLP_ret::XML_ERROR},
        {{"a", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12", "a", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12", "12", "a", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "a", ""}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "12", "<scheduling_policy>12</scheduling_policy>"}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "12", "<priority>12</priority>"}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "12", "<affinity>12</affinity>"}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "12", "<stack_size>12</stack_size>"}, XMLP_ret::XML_ERROR},
        {{"12", "12", "12", "12", "<wrong_tag>12</wrong_tag>"}, XMLP_ret::XML_ERROR},
    };

    /* Run the tests */
    for (auto test_case : test_cases)
    {
        std::vector<std::string>& params = test_case.first;
        XMLP_ret& expectation = test_case.second;

        using namespace eprosima::fastdds::rtps;
        ThreadSettings thread_settings;
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        // Create XML snippet
        std::string xml =
                "<thread_settings>"
                "    <scheduling_policy>" + params[0] + "</scheduling_policy>"
                "    <priority>" + params[1] + "</priority>"
                "    <affinity>" + params[2] + "</affinity>"
                "    <stack_size>" + params[3] + "</stack_size>"
                + params[4] +
                "</thread_settings>";

        // Parse the XML snippet
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str())) << xml;

        // Extract ThreadSetting
        titleElement = xml_doc.RootElement();
        ASSERT_EQ(expectation, XMLParserTest::getXMLThreadSettings_wrapper(titleElement, thread_settings));

        // Validate in the OK cases
        if (expectation == XMLP_ret::XML_OK)
        {
            ASSERT_EQ(thread_settings.scheduling_policy, static_cast<int32_t>(std::stoi(params[0])));
            ASSERT_EQ(thread_settings.priority, static_cast<int32_t>(std::stoi(params[1])));
            ASSERT_EQ(thread_settings.affinity, static_cast<uint64_t>(std::stoi(params[2])));
            ASSERT_EQ(thread_settings.stack_size, static_cast<int32_t>(std::stoi(params[3])));
        }
    }
}

/*
 * This test checks parsing of thread_settings with port elements.
 */
TEST_F(XMLParserTests, getXMLThreadSettingsWithPort)
{
    /* Define the test cases */
    std::vector<std::pair<std::vector<std::string>, XMLP_ret>> test_cases =
    {
        {{"12345", "", "12", "12", "12", "12", ""}, XMLP_ret::XML_OK},
        {{"12345", "", "-1", "12", "12", "12", ""}, XMLP_ret::XML_OK},
        {{"12345", "", "12", "-1", "12", "12", ""}, XMLP_ret::XML_OK},
        {{"12345", "", "12", "12", "12", "-1", ""}, XMLP_ret::XML_OK},
        {{"12345", "", "-2", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"-1", "", "12", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"a", "", "12", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"a", "wrong_attr=\"12\"", "12", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12345", "", "12", "12", "12", "-2", ""}, XMLP_ret::XML_ERROR},
        {{"12345", "", "a", "12", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12345", "", "12", "a", "12", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12345", "", "12", "12", "a", "12", ""}, XMLP_ret::XML_ERROR},
        {{"12345", "", "12", "12", "12", "a", ""}, XMLP_ret::XML_ERROR},
        {{"12345", "", "12", "12", "12", "12", "<stack_size>12</stack_size>"}, XMLP_ret::XML_ERROR},
        {{"12345", "", "12", "12", "12", "12", "<wrong_tag>12</wrong_tag>"}, XMLP_ret::XML_ERROR},
    };

    /* Run the tests */
    for (auto test_case : test_cases)
    {
        std::vector<std::string>& params = test_case.first;
        XMLP_ret& expectation = test_case.second;

        using namespace eprosima::fastdds::rtps;
        ThreadSettings thread_settings;
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        // Create XML snippet
        std::string xml =
                "<thread_settings port=\"" + params[0] + "\" " + params[1] + ">"
                "    <scheduling_policy>" + params[2] + "</scheduling_policy>"
                "    <priority>" + params[3] + "</priority>"
                "    <affinity>" + params[4] + "</affinity>"
                "    <stack_size>" + params[5] + "</stack_size>"
                + params[6] +
                "</thread_settings>";

        // Parse the XML snippet
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str())) << xml;

        // Extract ThreadSetting
        titleElement = xml_doc.RootElement();
        uint32_t port;
        ASSERT_EQ(expectation,
                XMLParserTest::getXMLThreadSettingsWithPort_wrapper(titleElement, thread_settings, port));

        // Validate in the OK cases
        if (expectation == XMLP_ret::XML_OK)
        {
            ASSERT_EQ(port, static_cast<uint32_t>(std::stoi(params[0])));
            ASSERT_EQ(thread_settings.scheduling_policy, static_cast<int32_t>(std::stoi(params[2])));
            ASSERT_EQ(thread_settings.priority, static_cast<int32_t>(std::stoi(params[3])));
            ASSERT_EQ(thread_settings.affinity, static_cast<uint32_t>(std::stoi(params[4])));
            ASSERT_EQ(thread_settings.stack_size, static_cast<int32_t>(std::stoi(params[5])));
        }
    }
}

/*
 * This test checks parsing of entity factory qos elements.
 */
TEST_F(XMLParserTests, getXMLEntityFactoryQos)
{
    /* Define the test cases */
    std::vector<std::pair<std::vector<std::string>, XMLP_ret>> test_cases =
    {
        {{"true", ""}, XMLP_ret::XML_OK},
        {{"false", ""}, XMLP_ret::XML_OK},
        {{"0", ""}, XMLP_ret::XML_OK},
        {{"1", ""}, XMLP_ret::XML_OK},
        {{"20", ""}, XMLP_ret::XML_OK},
        {{"wrong_value", ""}, XMLP_ret::XML_ERROR}
    };

    /* Run the tests */
    for (auto test_case : test_cases)
    {
        std::vector<std::string>& params = test_case.first;
        XMLP_ret& expectation = test_case.second;

        using namespace eprosima::fastdds::dds;
        EntityFactoryQosPolicy entity_factory_qos;
        tinyxml2::XMLDocument xml_doc;
        tinyxml2::XMLElement* titleElement;

        // Create XML snippet
        std::string xml =
                "<entity_factory>"
                "    <autoenable_created_entities>" + params[0] + "</autoenable_created_entities>"
                + params[1] +
                "</entity_factory>";

        std::cout << xml << std::endl;

        // Parse the XML snippet
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));

        // Extract ThreadSetting
        titleElement = xml_doc.RootElement();
        ASSERT_EQ(expectation, XMLParserTest::getXMLEntityFactoryQos_wrapper(titleElement, entity_factory_qos));

        // Validate in the OK cases
        if (expectation == XMLP_ret::XML_OK)
        {
            bool expected_value;
            if (params[0] == "true")
            {
                expected_value = true;
            }
            else if (params[0] == "false")
            {
                expected_value = false;
            }
            else
            {
                try
                {
                    expected_value = static_cast<bool>(std::stoi(params[0]));
                }
                catch (std::invalid_argument&)
                {
                    expected_value = false;
                }
            }
            ASSERT_EQ(entity_factory_qos.autoenable_created_entities, expected_value);
        }
    }
}
