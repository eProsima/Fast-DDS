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

#include <fastrtps/xmlparser/XMLEndpointParser.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include "mock/XMLMockConsumer.h"

#include <tinyxml2.h>
#include <gtest/gtest.h>

#include <string>
#include <fstream>
#include <sstream>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace ::testing;

using eprosima::fastrtps::xmlparser::XMLP_ret;
using eprosima::fastrtps::xmlparser::XMLEndpointParser;
using eprosima::fastrtps::xmlparser::StaticRTPSParticipantInfo;
using eprosima::fastrtps::rtps::ReaderProxyData;
using eprosima::fastrtps::rtps::WriterProxyData;

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
        std::unique_lock<std::mutex> lck(*xml_mutex_);
        mock_consumer->cv().wait(lck, [this, amount]
                {
                    return mock_consumer->ConsumedEntriesSize_nts() >= amount;
                });
    }

    eprosima::fastdds::dds::XMLMockConsumer* mock_consumer;

    mutable std::mutex* xml_mutex_;
    XMLEndpointParser* mp_edpXML;

protected:

    void SetUp() override
    {
        xml_mutex_ = new std::mutex();
        mp_edpXML = new xmlparser::XMLEndpointParser();
    }

    void TearDown() override
    {
        delete xml_mutex_;
        delete mp_edpXML;
        xml_mutex_ = nullptr;
        mp_edpXML = nullptr;
    }

};

/*
 * This test checks the negative cases of the XMLEndpointParser::loadXMLFileNegativeClauses method.
 * 1. Check passing non existant file.
 * 2. Check passing an empty file.
 */
TEST_F(XMLEndpointParserTests, loadXMLFileNegativeClauses)
{
    std::string filename;

    filename = "bad_filename";
    EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLFile(filename));


    filename = "empty.xml";
    const char* empty_string = "";
    std::ofstream out(filename);
    out << empty_string;
    out.close();
    EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLFile(filename));
    remove(filename.c_str());
}

/*
 * This test checks the negative cases of the XMLEndpointParser::loadXMLNode method.
 */
TEST_F(XMLEndpointParserTests, loadXMLNodeNegativeClauses)
{
    uint8_t ident = 1;
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // Wrong XML
        const char* xml ="<bad_xml></bad_xml>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_ERROR, mp_edpXML->loadXMLNode(xml_doc));
    }

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
                            <expectsInlineQos>true</expectsInlineQos>\
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
                        </writer>\
                    </participant>\
                </staticdiscovery>\
                ";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(XMLP_ret::XML_OK, mp_edpXML->loadXMLNode(xml_doc));
    }

}




//
// This test checks the negative cases of the XMLEndpointParser::loadXMLParticipantEndpoint method.
//
TEST_F(XMLEndpointParserTests, loadXMLParticipantEndpoint)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();

    // Parametrized XML
    const char* xml =
            "\
            <participant>\
                <name>HelloWorldSubscriber</name>\
                <reader>\
                    <userId>3</userId>\
                    <entityID>4</entityID>\
                    <expectsInlineQos>true</expectsInlineQos>\
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
                </writer>\
            </participant>\
            ";

    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.FirstChildElement();
    mp_edpXML->loadXMLParticipantEndpoint(titleElement, pdata);
}


//
// This test checks the negative cases of the XMLEndpointParser::loadXMLReaderEndpoint method.
//
TEST_F(XMLEndpointParserTests, loadXMLReaderEndpoint)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();

    // Parametrized XML
    // Parametrized XML
    const char* xml =
            "\
            <reader>\
                <userId>3</userId>\
                <entityID>4</entityID>\
                <expectsInlineQos>true</expectsInlineQos>\
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
            </reader>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.FirstChildElement();
    mp_edpXML->loadXMLReaderEndpoint(titleElement, pdata);
}

//
// This test checks the negative cases of the XMLEndpointParser::loadXMLWriterEndpoint method.
//
TEST_F(XMLEndpointParserTests, loadXMLWriterEndpoint)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;
    StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();

    // Parametrized XML
    // Parametrized XML
    const char* xml =
            "\
            <writer>\
                <userId>3</userId>\
                <entityID>4</entityID>\
                <expectsInlineQos>true</expectsInlineQos>\
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
            </writer>\
            ";
    ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
    titleElement = xml_doc.FirstChildElement();
    mp_edpXML->loadXMLWriterEndpoint(titleElement, pdata);
}

/* WIP

//
// This test checks the negative cases of the XMLEndpointParser::lookforReader method.
//
TEST_F(XMLEndpointParserTests, lookforReader)
{
    const char* partname = "";
    uint16_t id;
    ReaderProxyData** rdataptr;

    mp_edpXML->lookforReader(partname, id, rdataptr);
}

//
// This test checks the negative cases of the XMLEndpointParser::lookforWriter method.
//
TEST_F(XMLEndpointParserTests, lookforWriter)
{
    const char* partname = "";
    uint16_t id;
    WriterProxyData** wdataptr;

    mp_edpXML->lookforWriter(partname, id, wdataptr);
}
*/

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

