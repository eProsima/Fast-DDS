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

#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <tinyxml2.h>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <xmlparser/XMLEndpointParser.h>

#include "../logging/mock/MockConsumer.h"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using namespace ::testing;

using eprosima::fastdds::xmlparser::XMLP_ret;
using eprosima::fastdds::xmlparser::XMLEndpointParser;
using eprosima::fastdds::xmlparser::StaticRTPSParticipantInfo;
using eprosima::fastdds::rtps::ReaderProxyData;
using eprosima::fastdds::rtps::WriterProxyData;

using eprosima::fastdds::dds::Log;
using eprosima::fastdds::dds::LogConsumer;

class XMLEndpointParserTests : public ::testing::Test
{
public:

    XMLEndpointParserTests()
    {
    }

    ~XMLEndpointParserTests()
    {
        eprosima::fastdds::dds::Log::Reset();
        eprosima::fastdds::dds::Log::KillThread();
    }

    void helper_block_for_at_least_entries(
            uint32_t amount)
    {
        mock_consumer->wait_for_at_least_entries(amount);
    }

    eprosima::fastdds::dds::MockConsumer* mock_consumer;

    XMLEndpointParser* mp_edpXML;

protected:

    void SetUp() override
    {
        mp_edpXML = new xmlparser::XMLEndpointParser();
    }

    void TearDown() override
    {
        delete mp_edpXML;
        mp_edpXML = nullptr;
    }

};

/*
 * This test checks the negative cases of the XMLEndpointParser::loadXMLFileNegativeClauses method.
 *     1. Check passing non existant file.
 *     2. Check passing an empty file.
 */
TEST_F(XMLEndpointParserTests, loadXMLFileNegativeClauses)
{
    std::string filename;

    filename = "bad_filename";
    EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLFile(filename));


    filename = "wrong.xml";
    const char* content = "<bad_element></bad_element>";
    std::ofstream out(filename);
    out << content;
    out.close();
    EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLFile(filename));
    remove(filename.c_str());
}

/*
 * This test checks the method XMLEndpointParser::loadXMLNode.
 *     1. Check the return for a correct XML
 *     2. Check the return for an incorrect XML
 */
TEST_F(XMLEndpointParserTests, loadXMLNode)
{
    tinyxml2::XMLDocument xml_doc;

    {
        // Correct XML
        const char* xml =
                "\
                <staticdiscovery>\
                    <participant>\
                        <name>HelloWorldSubscriber</name>\
                        <reader>\
                            <userId>3</userId>\
                            <entityID>4</entityID>\
                            <expects_inline_qos>true</expects_inline_qos>\
                            <topicName>HelloWorldTopic</topicName>\
                            <topicDataType>HelloWorld</topicDataType>\
                            <topicKind>WITH_KEY</topicKind>\
                            <partitionQos>HelloPartition</partitionQos>\
                            <partitionQos>WorldPartition</partitionQos>\
                            <unicastLocator address=\"192.168.0.128\" port=\"5000\"/>\
                            <unicastLocator address=\"10.47.8.30\" port=\"6000\"/>\
                            <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                            <reliabilityQos>BEST_EFFORT_RELIABILITY_QOS</reliabilityQos>\
                            <durabilityQos>VOLATILE_DURABILITY_QOS</durabilityQos>\
                            <ownershipQos kind=\"SHARED_OWNERSHIP_QOS\"/>\
                            <livelinessQos kind=\"AUTOMATIC_LIVELINESS_QOS\" leaseDuration_ms=\"1000\"/>\
                            <disablePositiveAcks>\
                                <enabled>true</enabled>\
                            </disablePositiveAcks>\
                        </reader>\
                    </participant>\
                    <participant>\
                        <name>HelloWorldPublisher</name>\
                        <writer>\
                            <unicastLocator address=\"192.168.0.120\" port=\"9000\"/>\
                            <unicastLocator address=\"10.47.8.31\" port=\"8000\"/>\
                            <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                            <userId>5</userId>\
                            <entityID>6</entityID>\
                            <topicName>HelloWorldTopic</topicName>\
                            <topicDataType>HelloWorld</topicDataType>\
                            <topicKind>WITH_KEY</topicKind>\
                            <partitionQos>HelloPartition</partitionQos>\
                            <partitionQos>WorldPartition</partitionQos>\
                            <reliabilityQos>BEST_EFFORT_RELIABILITY_QOS</reliabilityQos>\
                            <durabilityQos>VOLATILE_DURABILITY_QOS</durabilityQos>\
                            <ownershipQos kind=\"SHARED_OWNERSHIP_QOS\" strength=\"50\"/>\
                            <livelinessQos kind=\"AUTOMATIC_LIVELINESS_QOS\" leaseDuration_ms=\"1000\"/>\
                            <disablePositiveAcks>\
                                <enabled>true</enabled>\
                                <duration>\
                                <sec>300</sec>\
                                </duration>\
                            </disablePositiveAcks>\
                        </writer>\
                    </participant>\
                </staticdiscovery>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLNode(xml_doc));
    }

    {
        // Wrong XML
        const char* xml = "<bad_xml></bad_xml>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLNode(xml_doc));
    }

}

/*
 * This test checks the XMLEndpointParser::loadXMLReaderEndpoint method.
 *     1. Check incorrect values for the writer tag
 *     2. Check incorrect values for the reader tag
 *     3. Check an incorrect tag
 */
TEST_F(XMLEndpointParserTests, loadXMLParticipantEndpoint)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    mock_consumer = new eprosima::fastdds::dds::MockConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mock_consumer));

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml =
                "\
                <participant>\
                    <name>HelloWorldSubscriber</name>\
                    <reader>\
                        <userId>3</userId>\
                        <entityID>4</entityID>\
                        <expects_inline_qos>true</expects_inline_qos>\
                        <topicName>HelloWorldTopic</topicName>\
                        <topicDataType>HelloWorld</topicDataType>\
                        <topicKind>WITH_KEY</topicKind>\
                        <topic name=\"HelloWorldTopic\" dataType=\"HelloWorld\" kind=\"WITH_KEY\"/>\
                        <partitionQos>HelloPartition</partitionQos>\
                        <partitionQos>WorldPartition</partitionQos>\
                        <unicastLocator address=\"192.168.0.128\" port=\"5000\"/>\
                        <unicastLocator address=\"10.47.8.30\" port=\"6000\"/>\
                        <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                        <reliabilityQos>BEST_EFFORT_RELIABILITY_QOS</reliabilityQos>\
                        <durabilityQos>VOLATILE_DURABILITY_QOS</durabilityQos>\
                        <ownershipQos kind=\"SHARED_OWNERSHIP_QOS\"/>\
                        <livelinessQos kind=\"AUTOMATIC_LIVELINESS_QOS\" leaseDuration_ms=\"1000\"/>\
                    </reader>\
                </participant>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        mp_edpXML->loadXMLParticipantEndpoint(titleElement, pdata);
        EXPECT_EQ(pdata->m_RTPSParticipantName, "HelloWorldSubscriber");
        EXPECT_EQ(pdata->m_readers.size(), (size_t)1);

        // Delete the ReaderProxyData created inside loadXMLParticipantEndpoint
        delete pdata->m_readers[0];

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml_p =
                "\
                <participant>\
                    <%s>bad_value</%s>\
                </participant>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> test_tags = {"reader", "writer", "bad_element"};

        for (const std::string& tag : test_tags)
        {
            snprintf(xml, xml_len, xml_p, tag.c_str(), tag.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.FirstChildElement();
            mp_edpXML->loadXMLParticipantEndpoint(titleElement, pdata);
        }

        helper_block_for_at_least_entries(5);
        auto consumed_entries = mock_consumer->ConsumedEntries();
        // Expect 3 log error.
        uint32_t num_errors = 0;
        for (const auto& entry : consumed_entries)
        {
            if (entry.kind == Log::Kind::Error)
            {
                num_errors++;
            }
        }
        EXPECT_EQ(num_errors, 5u);
        delete pdata;
    }
}

/*
 * This test checks the XMLEndpointParser::loadXMLReaderEndpoint method.
 *     1. Check correct parsing of the XML int ReaderProxyData
 *     2. Check incorrect values for the livelinesQos
 *     3. Check incorrect values for the ownershipQos
 *     4. Check an incorrect value for tags with parsable content
 *     5. Check an incorrect value for tags with parsable attributes
 */
TEST_F(XMLEndpointParserTests, loadXMLReaderEndpoint)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    int user_id = 3;
    int entity_id = 4;

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml =
                "\
                <reader>\
                    <userId>3</userId>\
                    <entityID>4</entityID>\
                    <expects_inline_qos>true</expects_inline_qos>\
                    <topicName>HelloWorldTopic</topicName>\
                    <topicDataType>HelloWorld</topicDataType>\
                    <topicKind>WITH_KEY</topicKind>\
                    <topic name=\"HelloWorldTopic\" dataType=\"HelloWorld\" kind=\"WITH_KEY\"/>\
                    <partitionQos>HelloPartition</partitionQos>\
                    <partitionQos>WorldPartition</partitionQos>\
                    <unicastLocator address=\"192.168.0.128\" port=\"5000\"/>\
                    <unicastLocator address=\"10.47.8.30\" port=\"6000\"/>\
                    <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                    <reliabilityQos>BEST_EFFORT_RELIABILITY_QOS</reliabilityQos>\
                    <durabilityQos>VOLATILE_DURABILITY_QOS</durabilityQos>\
                    <ownershipQos kind=\"SHARED_OWNERSHIP_QOS\"/>\
                    <livelinessQos kind=\"AUTOMATIC_LIVELINESS_QOS\" leaseDuration_ms=\"1000\"/>\
                    <disablePositiveAcks>\
                        <enabled>true</enabled>\
                    </disablePositiveAcks>\
                </reader>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();

        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLReaderEndpoint(titleElement, pdata));

        // Topic attributes
        EXPECT_EQ(pdata->m_readers[0]->topic_name, "HelloWorldTopic");
        EXPECT_EQ(pdata->m_readers[0]->topic_kind, TopicKind_t::WITH_KEY);
        EXPECT_EQ(pdata->m_readers[0]->type_name, "HelloWorld");
        EXPECT_EQ(pdata->m_readers[0]->has_locators(), true);

        // Locators
        Locator_t uni_loc;
        IPLocator::setIPv4(uni_loc, "192.168.0.128");
        uni_loc.port = static_cast<uint16_t>(5000);
        EXPECT_EQ(pdata->m_readers[0]->remote_locators.unicast[0],  uni_loc);

        Locator_t multi_loc;
        IPLocator::setIPv4(multi_loc, "239.255.1.1");
        multi_loc.port = static_cast<uint16_t>(7000);
        EXPECT_EQ(pdata->m_readers[0]->remote_locators.multicast[0],  multi_loc);

        // qos
        EXPECT_EQ(pdata->m_readers[0]->reliability.kind,  dds::BEST_EFFORT_RELIABILITY_QOS);
        EXPECT_EQ(pdata->m_readers[0]->durability.kind,  dds::VOLATILE_DURABILITY_QOS);
        EXPECT_EQ(pdata->m_readers[0]->ownership.kind,  dds::SHARED_OWNERSHIP_QOS);
        EXPECT_EQ(pdata->m_readers[0]->liveliness.kind,  dds::AUTOMATIC_LIVELINESS_QOS);
        EXPECT_TRUE(pdata->m_readers[0]->disable_positive_acks.enabled);

        // Delete the ReaderProxyData created inside loadXMLParticipantEndpoint
        delete pdata->m_readers[0];

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml_p =
                "\
                <reader>\
                    <userId>%d</userId>\
                    <entityID>%d</entityID>\
                    <livelinessQos kind=\"%s\" leaseDuration_ms=\"%s\"/>\
                </reader>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "AUTOMATIC_LIVELINESS_QOS", "1000");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS", "1000");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_TOPIC_LIVELINESS_QOS", "1000");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_TOPIC_LIVELINESS_QOS", "INF");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_TOPIC_LIVELINESS_QOS", "0");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        // Delete the WriterProxyData created inside loadXMLWriterEndpoint
        for (auto wdata : pdata->m_writers)
        {
            delete wdata;
        }

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml_p =
                "\
                <reader>\
                    <userId>%d</userId>\
                    <entityID>%d</entityID>\
                    <ownershipQos kind=\"%s\"/>\
                </reader>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id,  "SHARED_OWNERSHIP_QOS");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLReaderEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "EXCLUSIVE_OWNERSHIP_QOS");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLReaderEndpoint(titleElement, pdata));

        // Delete the ReaderProxyData created inside loadXMLReaderEndpoint
        for (auto rdata : pdata->m_readers)
        {
            delete rdata;
        }

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Tags with child tags
        const char* xml_content =
                "\
                <reader>\
                    <%s>bad_value</%s>\
                </reader>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> content_tags =
        {
            "userId",
            "entityID",
            "expects_inline_qos",
            "topicName",
            "topicDataType",
            "topicKind",
            "partitionQos",
            "partitionQos",
            "reliabilityQos",
            "durabilityQos",
            "bad_element"
        };

        for (const std::string& tag : content_tags)
        {
            snprintf(xml, xml_len, xml_content, tag.c_str(), tag.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.FirstChildElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLReaderEndpoint(titleElement, pdata));
        }
        delete pdata;
    }

    {
        // Tags with attributes
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        const char* xml_attribute =
                "\
                <reader>\
                    <%s bad_attribute=\"bad_value\"/>\
                </reader>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> attribute_tags =
        {
            "unicastLocator",
            "unicastLocator",
            "multicastLocator",
            "ownershipQos",
            "livelinessQos"
        };

        for (const std::string& tag : attribute_tags)
        {
            snprintf(xml, xml_len, xml_attribute, tag.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.FirstChildElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLReaderEndpoint(titleElement, pdata));
        }
        delete pdata;
    }

}

/*
 * This test checks the XMLEndpointParser::loadXMLWriterEndpoint method.
 *     1. Check correct parsing of the XML int WriterProxyData
 *     2. Check incorrect values for the livelinesQos
 *     3. Check incorrect values for the ownershipQos
 *     4. Check an incorrect value for tags with parsable content
 *     5. Check an incorrect value for tags with parsable attributes
 */
TEST_F(XMLEndpointParserTests, loadXMLWriterEndpoint)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    int user_id = 3;
    int entity_id = 4;

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();

        // Parametrized XML
        const char* xml =
                "\
                <writer>\
                    <userId>3</userId>\
                    <entityID>4</entityID>\
                    <expects_inline_qos>true</expects_inline_qos>\
                    <topicName>HelloWorldTopic</topicName>\
                    <topicDataType>HelloWorld</topicDataType>\
                    <topicKind>NO_KEY</topicKind>\
                    <topic name=\"HelloWorldTopic\" dataType=\"HelloWorld\" kind=\"NO_KEY\"/>\
                    <partitionQos>HelloPartition</partitionQos>\
                    <partitionQos>WorldPartition</partitionQos>\
                    <unicastLocator address=\"192.168.0.128\" port=\"5000\"/>\
                    <unicastLocator address=\"10.47.8.30\" port=\"6000\"/>\
                    <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                    <reliabilityQos>BEST_EFFORT_RELIABILITY_QOS</reliabilityQos>\
                    <durabilityQos>VOLATILE_DURABILITY_QOS</durabilityQos>\
                    <ownershipQos kind=\"SHARED_OWNERSHIP_QOS\"/>\
                    <livelinessQos kind=\"AUTOMATIC_LIVELINESS_QOS\" leaseDuration_ms=\"1000\"/>\
                    <disablePositiveAcks>\
                        <enabled>true</enabled>\
                        <duration>\
                            <sec>300</sec>\
                        </duration>\
                    </disablePositiveAcks>\
                </writer>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        // Topic attributes
        EXPECT_EQ(pdata->m_writers[0]->topic_name, "HelloWorldTopic");
        EXPECT_EQ(pdata->m_writers[0]->topic_kind, TopicKind_t::NO_KEY);
        EXPECT_EQ(pdata->m_writers[0]->type_name, "HelloWorld");
        EXPECT_EQ(pdata->m_writers[0]->has_locators(), true);

        // Locators
        Locator_t uni_loc;
        IPLocator::setIPv4(uni_loc, "192.168.0.128");
        uni_loc.port = static_cast<uint16_t>(5000);
        EXPECT_EQ(pdata->m_writers[0]->remote_locators.unicast[0],  uni_loc);

        Locator_t multi_loc;
        IPLocator::setIPv4(multi_loc, "239.255.1.1");
        multi_loc.port = static_cast<uint16_t>(7000);
        EXPECT_EQ(pdata->m_writers[0]->remote_locators.multicast[0],  multi_loc);

        // qos
        EXPECT_EQ(pdata->m_writers[0]->reliability.kind,  dds::BEST_EFFORT_RELIABILITY_QOS);
        EXPECT_EQ(pdata->m_writers[0]->durability.kind,  dds::VOLATILE_DURABILITY_QOS);
        EXPECT_EQ(pdata->m_writers[0]->ownership.kind,  dds::SHARED_OWNERSHIP_QOS);
        EXPECT_EQ(pdata->m_writers[0]->liveliness.kind,  dds::AUTOMATIC_LIVELINESS_QOS);
        EXPECT_TRUE(pdata->m_writers[0]->disable_positive_acks.enabled);
        EXPECT_EQ(pdata->m_writers[0]->disable_positive_acks.duration.seconds, 300);
        EXPECT_EQ(pdata->m_writers[0]->disable_positive_acks.duration.nanosec, 0u);

        // Delete the WriterProxyData created inside loadXMLWriterEndpoint
        delete pdata->m_writers[0];

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml_p =
                "\
                <writer>\
                    <userId>%d</userId>\
                    <entityID>%d</entityID>\
                    <livelinessQos kind=\"%s\" leaseDuration_ms=\"%s\"/>\
                </writer>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "AUTOMATIC_LIVELINESS_QOS", "1000");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS", "1000");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_TOPIC_LIVELINESS_QOS", "1000");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_TOPIC_LIVELINESS_QOS", "INF");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "MANUAL_BY_TOPIC_LIVELINESS_QOS", "0");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        // Delete the WriterProxyData created inside loadXMLWriterEndpoint
        for (auto wdata : pdata->m_writers)
        {
            delete wdata;
        }

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Parametrized XML
        const char* xml_p =
                "\
                <writer>\
                    <userId>%d</userId>\
                    <entityID>%d</entityID>\
                    <ownershipQos kind=\"%s\"/>\
                </writer>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id,  "SHARED_OWNERSHIP_QOS");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        user_id += 2;
        entity_id += 2;
        snprintf(xml, xml_len, xml_p, user_id, entity_id, "EXCLUSIVE_OWNERSHIP_QOS");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        titleElement = xml_doc.FirstChildElement();
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));

        // Delete the WriterProxyData created inside loadXMLWriterEndpoint
        for (auto wdata : pdata->m_writers)
        {
            delete wdata;
        }

        // Then delete StaticRTPSParticipantInfo
        delete pdata;
    }

    {
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        // Tags with child tags
        const char* xml_content =
                "\
                <writer>\
                    <%s>bad_value</%s>\
                </writer>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> content_tags =
        {
            "userId",
            "entityID",
            "expects_inline_qos",
            "topicName",
            "topicDataType",
            "topicKind",
            "partitionQos",
            "partitionQos",
            "reliabilityQos",
            "durabilityQos",
            "bad_element"
        };



        for (const std::string& tag : content_tags)
        {
            snprintf(xml, xml_len, xml_content, tag.c_str(), tag.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.FirstChildElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));
        }

        delete pdata;
    }

    {
        // Tags with attributes
        StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
        const char* xml_attribute =
                "\
                <writer>\
                    <%s bad_attribute=\"bad_value\"/>\
                </writer>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> attribute_tags =
        {
            "unicastLocator",
            "unicastLocator",
            "multicastLocator",
            "ownershipQos",
            "livelinessQos"
        };

        for (const std::string& tag : attribute_tags)
        {
            snprintf(xml, xml_len, xml_attribute, tag.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            titleElement = xml_doc.FirstChildElement();
            EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata));
        }
        delete pdata;
    }
}

/*
 * This test checks the XMLEndpointParser::lookforWriter method. First load a writer to the parser with loadXMLNode,
 * then retrieve it with its id. Then check the returned writer data is correct.
 */
TEST_F(XMLEndpointParserTests, lookforReader)
{
    tinyxml2::XMLDocument xml_doc;
    ReaderProxyData* rdataptr = nullptr;

    const char* xml =
            "\
            <staticdiscovery>\
                <participant>\
                    <name>HelloWorldPublisher</name>\
                    <reader>\
                        <userId>3</userId>\
                        <entityID>4</entityID>\
                        <topicName>HelloWorldTopic</topicName>\
                        <topicDataType>HelloWorld</topicDataType>\
                        <topicKind>WITH_KEY</topicKind>\
                        <unicastLocator address=\"192.168.0.128\" port=\"5000\"/>\
                        <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                    </reader>\
                </participant>\
            </staticdiscovery>\
            ";

    // Load writer with known properties
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLNode(xml_doc));

    ASSERT_EQ(XMLP_ret::XML_OK, mp_edpXML->lookforReader("HelloWorldPublisher", 3, &rdataptr));
    ASSERT_NE(rdataptr, nullptr);
    EXPECT_EQ(rdataptr->topic_name, "HelloWorldTopic");
    EXPECT_EQ(rdataptr->topic_kind, TopicKind_t::WITH_KEY);
    EXPECT_EQ(rdataptr->type_name, "HelloWorld");
    EXPECT_EQ(rdataptr->has_locators(), true);

    // Locators
    Locator_t uni_loc;
    IPLocator::setIPv4(uni_loc, "192.168.0.128");
    uni_loc.port = static_cast<uint16_t>(5000);
    EXPECT_EQ(rdataptr->remote_locators.unicast[0],  uni_loc);

    Locator_t multi_loc;
    IPLocator::setIPv4(multi_loc, "239.255.1.1");
    multi_loc.port = static_cast<uint16_t>(7000);
    EXPECT_EQ(rdataptr->remote_locators.multicast[0],  multi_loc);

    ASSERT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->lookforReader("WrongName", 15, &rdataptr));
}

/*
 * This test checks the XMLEndpointParser::lookforWriter method. First load a writer to the parser with loadXMLNode,
 * then retrieve it with its id. Then check the returned writer data is correct.
 */
TEST_F(XMLEndpointParserTests, lookforWriter)
{
    tinyxml2::XMLDocument xml_doc;
    WriterProxyData* wdataptr = nullptr;

    const char* xml =
            "\
            <staticdiscovery>\
                <participant>\
                    <name>HelloWorldPublisher</name>\
                    <writer>\
                        <userId>3</userId>\
                        <entityID>4</entityID>\
                        <topicName>HelloWorldTopic</topicName>\
                        <topicDataType>HelloWorld</topicDataType>\
                        <topicKind>WITH_KEY</topicKind>\
                        <unicastLocator address=\"192.168.0.128\" port=\"5000\"/>\
                        <multicastLocator address=\"239.255.1.1\" port=\"7000\"/>\
                    </writer>\
                </participant>\
            </staticdiscovery>\
            ";

    // Load writer with known properties
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLNode(xml_doc));

    ASSERT_EQ(XMLP_ret::XML_OK, mp_edpXML->lookforWriter("HelloWorldPublisher", 3, &wdataptr));
    EXPECT_EQ(wdataptr->topic_name, "HelloWorldTopic");
    EXPECT_EQ(wdataptr->topic_kind, TopicKind_t::WITH_KEY);
    EXPECT_EQ(wdataptr->type_name, "HelloWorld");
    EXPECT_EQ(wdataptr->has_locators(), true);

    // Locators
    Locator_t uni_loc;
    IPLocator::setIPv4(uni_loc, "192.168.0.128");
    uni_loc.port = static_cast<uint16_t>(5000);
    EXPECT_EQ(wdataptr->remote_locators.unicast[0],  uni_loc);

    Locator_t multi_loc;
    IPLocator::setIPv4(multi_loc, "239.255.1.1");
    multi_loc.port = static_cast<uint16_t>(7000);
    EXPECT_EQ(wdataptr->remote_locators.multicast[0],  multi_loc);

    ASSERT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->lookforWriter("WrongName", 15, &wdataptr));
}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
