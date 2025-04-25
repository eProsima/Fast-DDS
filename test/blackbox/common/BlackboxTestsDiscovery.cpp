// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <atomic>
#include <thread>

#ifndef _WIN32
#include <stdlib.h>
#endif // _WIN32

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/messages/RTPS_messages.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include <rtps/attributes/ServerAttributes.hpp>

#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "DatagramInjectionTransport.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"
#include "PubSubParticipant.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class Discovery : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(Discovery, ParticipantRemoval)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // Reader will not be reading, so datasharing needs some extra samples in pool
    writer.resource_limits_extra_samples(10).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Send some data.
    auto data = default_helloworld_data_generator();
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.wait_participant_undiscovery();
}

void static_discovery_test(
        const std::string& reader_property_value,
        const std::string& writer_property_value,
        bool discovery_will_be_success = true)
{
    char* value = nullptr;
    std::string TOPIC_RANDOM_NUMBER;
    std::string W_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string R_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string MULTICAST_PORT_RANDOM_NUMBER_STR;
    // Get environment variables.
    value = std::getenv("TOPIC_RANDOM_NUMBER");
    if (value != nullptr)
    {
        TOPIC_RANDOM_NUMBER = value;
    }
    else
    {
        TOPIC_RANDOM_NUMBER = "1";
    }
    value = std::getenv("W_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = "7411";
    }
    int32_t W_UNICAST_PORT_RANDOM_NUMBER = stoi(W_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("R_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = "7421";
    }
    int32_t R_UNICAST_PORT_RANDOM_NUMBER = stoi(R_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("MULTICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = "7400";
    }
    int32_t MULTICAST_PORT_RANDOM_NUMBER = stoi(MULTICAST_PORT_RANDOM_NUMBER_STR);

    PropertyPolicy writer_property_policy;
    writer_property_policy.properties().push_back({"dds.discovery.static_edp.exchange_format", writer_property_value});

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterUnicastLocators;
    Locator_t LocatorBuffer;

    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = static_cast<uint16_t>(W_UNICAST_PORT_RANDOM_NUMBER);
    IPLocator::setIPv4(LocatorBuffer, 127, 0, 0, 1);
    WriterUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t WriterMulticastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    WriterMulticastLocators.push_back(LocatorBuffer);

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .property_policy(writer_property_policy);
    writer.static_discovery("file://PubSubWriter_static_disc.xml").reliability(
        eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            unicastLocatorList(WriterUnicastLocators).multicastLocatorList(WriterMulticastLocators).
            setPublisherIDs(1,
            2).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();


    if (discovery_will_be_success)
    {
        ASSERT_TRUE(writer.isInitialized());
    }
    else
    {
        ASSERT_FALSE(writer.isInitialized());
    }

    PropertyPolicy reader_property_policy;
    reader_property_policy.properties().push_back({"dds.discovery.static_edp.exchange_format", reader_property_value});

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    LocatorList_t ReaderUnicastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(R_UNICAST_PORT_RANDOM_NUMBER);
    ReaderUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t ReaderMulticastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    ReaderMulticastLocators.push_back(LocatorBuffer);


    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .property_policy(reader_property_policy);
    reader.static_discovery("file://PubSubReader_static_disc.xml").
            unicastLocatorList(ReaderUnicastLocators).multicastLocatorList(ReaderMulticastLocators).
            setSubscriberIDs(3,
            4).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

    if (discovery_will_be_success)
    {
        ASSERT_TRUE(reader.isInitialized());

        // Because its volatile the durability
        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();
        auto expected_data(data);

        writer.send(data);
        ASSERT_TRUE(data.empty());

        reader.startReception(expected_data);
        reader.block_for_all();
    }
    else
    {
        ASSERT_FALSE(reader.isInitialized());
    }
}

TEST(Discovery, StaticDiscovery_v1)
{
    static_discovery_test("v1", "v1");
}

TEST(Discovery, StaticDiscovery_v1_Reduced)
{
    static_discovery_test("v1_Reduced", "v1_Reduced");
}

TEST(Discovery, StaticDiscovery_v1_Mixed)
{
    static_discovery_test("v1", "v1_Reduced");
}

TEST(Discovery, StaticDiscovery_wrong_exchange_format)
{
    static_discovery_test("wrong", "wrong", false);
}

/*!
 * Test Static EDP discovery configured via a XML content in a raw string.
 *
 * Currently Fast DDS API supports configure Static EDP discovery in two ways: setting the file containing the XML
 * configuration or passing directly the XML content. This test tests the second way.
 *
 * Steps:
 *
 * 1. Configure a writer. Static EDP Discovery is enable and XML configuration is passed directly using
 * static_edp_xml_config() API funcion.
 *
 * 2. Initialize writer.
 *
 * 3. Configure a reader. Static EDP Discovery is enable and XML configuration is passed directly using
 * static_edp_xml_config() API funcion.
 *
 * 4. Initialize writer.
 *
 * 5. Wait both entities discover between them. If the Static EDP Discovery was configured correctly, they should
 * discover each other.
 *
 * 6. Writer send a batch of samples.
 *
 * 7. Wait to receive all them. If the Static EDP Discovery was configured correctly, the communication should work
 * successfully.
 *
 */
TEST(Discovery, StaticDiscoveryFromString)
{
    char* value = std::getenv("TOPIC_RANDOM_NUMBER");
    std::string TOPIC_RANDOM_NUMBER;
    if (value != nullptr)
    {
        TOPIC_RANDOM_NUMBER = value;
    }
    else
    {
        TOPIC_RANDOM_NUMBER = "1";
    }

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    std::string writer_xml = "data://<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
            "<staticdiscovery>" \
            "<participant>" \
            "<name>RTPSParticipant</name>" \
            "<reader>" \
            "<userId>3</userId>" \
            "<entityID>4</entityID>" \
            "<topicName>BlackBox_StaticDiscoveryFromString_" +
            TOPIC_RANDOM_NUMBER +
            std::string("</topicName>" \
                    "<topicDataType>HelloWorld</topicDataType>" \
                    "<topicKind>NO_KEY</topicKind>" \
                    "<reliabilityQos>RELIABLE_RELIABILITY_QOS</reliabilityQos>" \
                    "<durabilityQos>TRANSIENT_LOCAL_DURABILITY_QOS</durabilityQos>" \
                    "</reader>" \
                    "</participant>" \
                    "</staticdiscovery>");
    writer.static_discovery(writer_xml.c_str()).setPublisherIDs(1, 2).
            setManualTopicName(std::string("BlackBox_StaticDiscoveryFromString_") + TOPIC_RANDOM_NUMBER).
            init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);


    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    std::string reader_xml = "data://<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
            "<staticdiscovery>" \
            "<participant>" \
            "<name>RTPSParticipant</name>" \
            "<writer>" \
            "<userId>1</userId>" \
            "<entityID>2</entityID>" \
            "<topicName>BlackBox_StaticDiscoveryFromString_" +
            TOPIC_RANDOM_NUMBER +
            std::string(
        "</topicName>" \
        "<topicDataType>HelloWorld</topicDataType>" \
        "<topicKind>NO_KEY</topicKind>" \
        "<reliabilityQos>RELIABLE_RELIABILITY_QOS</reliabilityQos>" \
        "<durabilityQos>TRANSIENT_LOCAL_DURABILITY_QOS</durabilityQos>" \
        "</writer>" \
        "</participant>" \
        "</staticdiscovery>");
    reader.static_discovery(reader_xml.c_str()).setSubscriberIDs(3, 4).
            setManualTopicName(std::string("BlackBox_StaticDiscoveryFromString_") + TOPIC_RANDOM_NUMBER).
            init();

    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    reader.startReception(expected_data);
    reader.block_for_all();
}

TEST_P(Discovery, EDPSlaveReaderAttachment)
{
    PubSubWriter<HelloWorldPubSubType> checker(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType>* reader = new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType>* writer = new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME);

    checker.init();

    ASSERT_TRUE(checker.isInitialized());

    reader->partition("test").partition("othertest").init();

    ASSERT_TRUE(reader->isInitialized());

    writer->partition("test").init();

    ASSERT_TRUE(writer->isInitialized());

    checker.block_until_discover_topic(checker.topic_name(), 3);
    checker.block_until_discover_partition("test", 2);
    checker.block_until_discover_partition("othertest", 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    delete reader;
    delete writer;

    checker.block_until_discover_topic(checker.topic_name(), 1);
    checker.block_until_discover_partition("test", 0);
    checker.block_until_discover_partition("othertest", 0);
}

// Used to detect Github issue #155
TEST(Discovery, EndpointRediscovery)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto test_transport_reader = std::make_shared<test_UDPv4TransportDescriptor>();
    reader.disable_builtin_transport();
    reader.add_user_transport_to_pparams(test_transport_reader);

    reader.lease_duration({ 3, 0 }, { 1, 0 }).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto test_transport_writer = std::make_shared<test_UDPv4TransportDescriptor>();
    // We drop 20% of all data frags
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(test_transport_writer);

    writer.lease_duration({ 6, 0 }, { 2, 0 }).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    test_transport_writer->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;
    test_transport_reader->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    writer.wait_reader_undiscovery();

    test_transport_writer->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;
    test_transport_reader->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    writer.wait_discovery();
}

// Used to detect Github issue #457
TEST(Discovery, EndpointRediscovery_2)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    reader.lease_duration({ 120, 0 }, { 1, 0 }).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(test_transport);

    writer.lease_duration({ 2, 0 }, { 1, 0 }).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    reader.wait_participant_undiscovery();

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    reader.wait_discovery();
}

// Regression for bug #9629
TEST(Discovery, EndpointRediscoveryWithTransientLocalData)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    reader
            .lease_duration({ 120, 0 }, { 1, 0 })
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(test_transport);

    writer
            .lease_duration({ 2, 0 }, { 1, 0 })
            .history_depth(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator(10);

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(1)));

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    reader.wait_participant_undiscovery();

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    reader.wait_discovery();

    // The bug made the last sample to be sent again, producing a test failure inside
    // PubSubReader::receive_one
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/*!
 * @test Checks that although DATA(p) are not received, but other kind of RTPS submessages, it keeps the participant
 * liveliness from the remote participant.
 *
 * **Behaviour:** Two participants are enabled, one publisher and other one subscriber. Both will use the
 * test_UDPv4Transport and be configured to use a short participant liveliness lease duration. After the discovery the
 * test starts dropping builtin topics. After lease duration, both participants still known each other.
 * **Input:**
 * **Output:** Execution should end successfully without any assertion being launched.
 * **Associated requirements:**
 * **Other requirements:**
 */
TEST(Discovery, ParticipantLivelinessAssertion)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport).
            lease_duration({ 0, 800000000 },
            { 0, 500000000 }).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport).
            lease_duration({ 0, 800000000 }, { 0, 500000000 }).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    test_transport->test_transport_options->always_drop_participant_builtin_topic_data = true;

    std::thread thread([&writer]()
            {
                HelloWorld msg;
                for (int count = 0; count < 20; ++count)
                {
                    writer.send_sample(msg);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });

    EXPECT_FALSE(reader.wait_participant_undiscovery(std::chrono::seconds(1)));
    EXPECT_FALSE(writer.wait_participant_undiscovery(std::chrono::seconds(1)));

    test_transport->test_transport_options->always_drop_participant_builtin_topic_data = false;

    thread.join();
}

// Regression test of Refs #2535, github micro-RTPS #1
TEST(Discovery, PubXmlLoadedPartition)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.partition("A").init();

    ASSERT_TRUE(reader.isInitialized());

    const std::string xml =
            R"(<profiles>
  <publisher profile_name="partition_publisher_profile">
    <topic>
      <name>)" + writer.topic_name() +
            R"(</name>
      <dataType>HelloWorld</dataType>
    </topic>
    <qos>
      <partition>
        <names>
          <name>A</name>
        </names>
      </partition>
    </qos>
    </publisher>
</profiles>)";

    writer.load_publisher_attr(xml).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.wait_discovery();
    writer.wait_discovery();
}

TEST(Discovery, LocalInitialPeers)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    Locator_t loc_initial_peer, loc_default_unicast;
    LocatorList_t reader_initial_peers;
    IPLocator::setIPv4(loc_initial_peer, 127, 0, 0, 1);
    loc_initial_peer.port = static_cast<uint16_t>(global_port);
    reader_initial_peers.push_back(loc_initial_peer);
    LocatorList_t reader_default_unicast_locator;
    loc_default_unicast.port = static_cast<uint16_t>(global_port + 1);
    reader_default_unicast_locator.push_back(loc_default_unicast);

    reader.metatraffic_unicast_locator_list(reader_default_unicast_locator).
            initial_peers(reader_initial_peers).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    LocatorList_t writer_initial_peers;
    loc_initial_peer.port = static_cast<uint16_t>(global_port + 1);
    writer_initial_peers.push_back(loc_initial_peer);
    LocatorList_t writer_default_unicast_locator;
    loc_default_unicast.port = static_cast<uint16_t>(global_port);
    writer_default_unicast_locator.push_back(loc_default_unicast);

    writer.metatraffic_unicast_locator_list(writer_default_unicast_locator).
            initial_peers(writer_initial_peers).history_depth(10).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Test created to check bug #2010 (Github #90).
// It also checks https://github.com/eProsima/Fast-DDS/issues/2107
TEST_P(Discovery, PubSubAsReliableHelloworldPartitions)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(10).
            partition("PartitionTests").
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(10).
            partition("PartitionTe*").init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Change reader to different partition to check un-matching
    ASSERT_TRUE(reader.update_partition("OtherPartition"));

    reader.wait_writer_undiscovery();
    writer.wait_reader_undiscovery();

    // Reset partition and wait for discovery to check that emptying the list triggers un-matching.
    // This is to check Github #2107
    ASSERT_TRUE(reader.update_partition("PartitionTests"));

    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(reader.clear_partitions());

    reader.wait_writer_undiscovery();
    writer.wait_reader_undiscovery();

    // Set reader and writer in compatible partitions
    ASSERT_TRUE(reader.update_partition("OtherPartition"));
    ASSERT_TRUE(writer.update_partition("OtherPart*"));

    writer.wait_discovery();
    reader.wait_discovery();

    data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

/*!
 * @test: Regression test for redmine issue #15839
 *
 * This test creates one writer and two readers, listening for metatraffic on different ports.
 *
 */
TEST(Discovery, LocalInitialPeersDiferrentLocators)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> readers[2]{ {TEST_TOPIC_NAME}, {TEST_TOPIC_NAME} };

    static const uint32_t writer_port = global_port;
    static const uint32_t reader_ports[] = { global_port + 1u, global_port + 2u };

    // Checks that the wrong locator is only accessed when necessary
    struct Checker
    {
        // Maximum number of times the locator of the first reader is expected when the second one is initiated.
        // We allow for one DATA(p) to be sent.
        const size_t max_allowed_times = 1;
        // Flag to indicate whether the locator of the first reader is expected.
        bool first_reader_locator_allowed = true;
        // Counts the number of times the locator of the first reader is used after the second one is initiated
        size_t wrong_times = 0;

        void check(
                const eprosima::fastdds::rtps::Locator& destination)
        {
            if (!first_reader_locator_allowed && destination.port == reader_ports[0])
            {
                ++wrong_times;
                EXPECT_LE(wrong_times, max_allowed_times);
            }
        }

    };

    // Install hook on the test transport to check for destination locators on the writer participant
    Checker checker;
    auto locator_printer = [&checker](const eprosima::fastdds::rtps::Locator& destination)
            {
                checker.check(destination);
                return false;
            };

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->test_transport_options->locator_filter = locator_printer;

    // Configure writer participant:
    // - Uses the test transport, to check destination behavior
    // - Listens for metatraffic on `writer_port`
    // - Has no automatic announcements
    {
        LocatorList_t writer_metatraffic_unicast;
        Locator_t locator;
        locator.port = static_cast<uint16_t>(writer_port);
        writer_metatraffic_unicast.push_back(locator);

        writer.disable_builtin_transport().
                add_user_transport_to_pparams(test_transport).
                metatraffic_unicast_locator_list(writer_metatraffic_unicast).
                lease_duration(eprosima::fastdds::dds::c_TimeInfinite, { 3600, 0 }).
                initial_announcements(0, {}).
                reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    }

    // Configure reader participants:
    // - Use (non-testing) UDP transport only
    // - Listen on different ports
    // - Announce only once to the port of the writer only
    //   (i.e. no communication between reader participants will happen)
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    for (uint16_t i = 0; i < 2; ++i)
    {
        LocatorList_t reader_metatraffic_unicast;
        Locator_t locator;
        locator.port = static_cast<uint16_t>(reader_ports[i]);
        reader_metatraffic_unicast.push_back(locator);

        LocatorList_t reader_initial_peers;
        Locator_t loc_initial_peer;
        IPLocator::setIPv4(loc_initial_peer, 127, 0, 0, 1);
        loc_initial_peer.port = static_cast<uint16_t>(writer_port);
        reader_initial_peers.push_back(loc_initial_peer);

        readers[i].disable_builtin_transport().
                add_user_transport_to_pparams(udp_transport).
                lease_duration(eprosima::fastdds::dds::c_TimeInfinite, {3600, 0}).
                initial_announcements(1, {0, 100 * 1000 * 1000}).
                metatraffic_unicast_locator_list(reader_metatraffic_unicast).
                initial_peers(reader_initial_peers).
                reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    }

    // Start writer and first reader, and wait for them to discover
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    readers[0].init();
    ASSERT_TRUE(readers[0].isInitialized());

    writer.wait_discovery();
    readers[0].wait_discovery();

    // Wait a bit (in case some additional ACKNACK / DATA(p) is exchanged after discovery)
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Check that, when initializing the second reader, the writer does not communicate with the first reader,
    // except for a single DATA(p)
    checker.first_reader_locator_allowed = false;

    readers[1].init();
    ASSERT_TRUE(readers[1].isInitialized());
    readers[1].wait_discovery();
}

TEST_P(Discovery, PubSubAsReliableHelloworldParticipantDiscovery)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    int count = 0;
    reader.set_on_discovery_function([&writer, &count](const ParticipantBuiltinTopicData& info,
            ParticipantDiscoveryStatus status) -> bool
            {
                if (info.guid == writer.participant_guid())
                {
                    if (status == ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
                    {
                        std::cout << "Discovered participant " << info.guid << std::endl;
                        ++count;
                    }
                    else if (status == ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
                    status == ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
                    {
                        std::cout << "Removed participant " << info.guid << std::endl;
                        return ++count == 2;
                    }
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery();
    writer.wait_discovery();

    writer.destroy();

    reader.wait_participant_undiscovery();

    reader.wait_discovery_result();
}

TEST_P(Discovery, PubSubAsReliableHelloworldUserData)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).
            user_data({'a', 'b', 'c', 'd'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.set_on_discovery_function([&writer](const ParticipantBuiltinTopicData& info,
            ParticipantDiscoveryStatus /*status*/) -> bool
            {
                if (info.guid == writer.participant_guid())
                {
                    std::cout << "Received USER_DATA from the writer: ";
                    for (auto i: info.user_data)
                    {
                        std::cout << i << ' ';
                    }
                    return info.user_data == std::vector<octet>({'a', 'b', 'c', 'd'});
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());


    reader.wait_discovery();
    writer.wait_discovery();

    reader.wait_discovery_result();
}

// Regression test for #8690.
TEST_P(Discovery, PubSubAsReliableHelloworldEndpointUserData)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).
            endpoint_userData({'a', 'b', 'c', 'd'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.setOnEndpointDiscoveryFunction([&writer](WriterDiscoveryStatus /*reason*/,
            const PublicationBuiltinTopicData& info) -> bool
            {
                if (info.guid == writer.datawriter_guid())
                {
                    std::cout << "Received USER_DATA from the writer: ";
                    for (auto i: info.user_data)
                    {
                        std::cout << i << ' ';
                    }
                    return info.user_data == std::vector<octet>({'a', 'b', 'c', 'd'});
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());


    reader.wait_discovery();
    writer.wait_discovery();

    reader.wait_discovery_result();
}

//! Auxiliar method for discovering participants tests
template <typename ParticipantConfigurator>
static void discoverParticipantsTest(
        bool avoid_multicast,
        size_t n_participants,
        uint32_t wait_ms,
        const std::string& topic_name,
        ParticipantConfigurator participant_configurator)
{
    std::vector<std::shared_ptr<PubSubWriterReader<HelloWorldPubSubType>>> pubsub;
    pubsub.reserve(n_participants);

    for (size_t i = 0; i < n_participants; ++i)
    {
        pubsub.emplace_back(std::make_shared<PubSubWriterReader<HelloWorldPubSubType>>(topic_name));
    }

    // Initialization of all the participants
    std::cout << "Initializing PubSubs for topic " << topic_name << std::endl;
    uint32_t idx = 1;
    for (auto& ps : pubsub)
    {
        std::cout << "\rParticipant " << idx++ << " of " << n_participants << std::flush;
        participant_configurator(ps);
        ps->init(avoid_multicast);
        ASSERT_EQ(ps->isInitialized(), true);
    }

    bool all_discovered = false;
    std::cout << std::endl << "Waiting discovery between " << n_participants << " participants." << std::endl;
    while (!all_discovered)
    {
        all_discovered = true;

        for (auto& ps : pubsub)
        {
            if ((ps->get_num_discovered_participants() < n_participants - 1) ||
                    (ps->get_num_discovered_publishers() < n_participants) ||
                    (ps->get_num_discovered_subscribers() < n_participants) ||
                    (ps->get_publication_matched() < n_participants) ||
                    (ps->get_subscription_matched() < n_participants))
            {
                all_discovered = false;
                break;
            }
        }

        if (!all_discovered)
        {
            // Give some time so that participants can discover each other
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        }
    }

    // Test will fail by timeout if a discovery error happens
    std::cout << "All discovered. Closing participants..." << std::endl;
    for (auto& ps : pubsub)
    {
        ps->destroy();
    }
}

static void discoverParticipantsTest(
        bool avoid_multicast,
        size_t n_participants,
        uint32_t wait_ms,
        const std::string& topic_name)
{
    auto no_op = [](const std::shared_ptr<PubSubWriterReader<HelloWorldPubSubType>>&)
            {
            };
    discoverParticipantsTest(avoid_multicast, n_participants, wait_ms, topic_name, no_op);
}

//! Tests discovery of 20 participants, having one publisher and one subscriber each, using multicast
TEST(Discovery, TwentyParticipantsMulticast)
{
    discoverParticipantsTest(false, 20, 20, TEST_TOPIC_NAME);
}

//! Regresion test for ros2/ros2#1052
TEST(Discovery, TwentyParticipantsMulticastLocalhostOnly)
{
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    auto participant_config = [&test_transport](const std::shared_ptr<PubSubWriterReader<HelloWorldPubSubType>>& part)
            {
                part->disable_builtin_transport().add_user_transport_to_pparams(test_transport);
            };

    test_transport->test_transport_options->simulate_no_interfaces = true;
    discoverParticipantsTest(false, 20, 20, TEST_TOPIC_NAME, participant_config);
}

//! Tests discovery of 20 participants, having one publisher and one subscriber each, using unicast
TEST_P(Discovery, TwentyParticipantsUnicast)
{
    discoverParticipantsTest(true, 20, 20, TEST_TOPIC_NAME);
}

//! Auxiliar method for discovering participants tests
static void discoverParticipantsSeveralEndpointsTest(
        bool avoid_multicast,
        size_t n_participants,
        size_t n_topics,
        uint32_t wait_ms,
        const std::string& topic_name)
{
    // Total number of discovered endpoints
    size_t n_total_endpoints = n_participants * n_topics;

    std::vector<std::shared_ptr<PubSubWriterReader<HelloWorldPubSubType>>> pubsub;
    pubsub.reserve(n_participants);

    for (unsigned int i = 0; i < n_participants; i++)
    {
        pubsub.emplace_back(std::make_shared<PubSubWriterReader<HelloWorldPubSubType>>(topic_name));
    }

    // Initialization of all the participants
    std::cout << "Initializing PubSubs for topic " << topic_name << std::endl;
    uint32_t idx = 1;
    for (auto& ps : pubsub)
    {
        std::cout << "\rParticipant " << idx++ << " of " << n_participants << std::flush;
        ps->init(avoid_multicast);
        ASSERT_EQ(ps->isInitialized(), true);
        ASSERT_TRUE(ps->create_additional_topics(n_topics - 1, "/"));
    }

    bool all_discovered = false;
    std::cout << std::endl << "Waiting discovery between " << n_participants << " participants." << std::endl;
    while (!all_discovered)
    {
        all_discovered = true;

        for (auto& ps : pubsub)
        {
            if ((ps->get_num_discovered_participants() < n_participants - 1) ||
                    (ps->get_num_discovered_publishers() < n_total_endpoints) ||
                    (ps->get_num_discovered_subscribers() < n_total_endpoints) ||
                    (ps->get_publication_matched() < n_total_endpoints) ||
                    (ps->get_subscription_matched() < n_total_endpoints))
            {
                all_discovered = false;
                break;
            }
        }

        if (!all_discovered)
        {
            // Give some time so that participants can discover each other
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        }
    }

    // Test will fail by timeout if a discovery error happens
    std::cout << "All discovered. Closing participants..." << std::endl;
    for (auto& ps : pubsub)
    {
        ps->destroy();
    }
}

//! Regression for ROS2 #280 and #281, using multicat
TEST(Discovery, TwentyParticipantsSeveralEndpointsMulticast)
{
    discoverParticipantsSeveralEndpointsTest(false, 20, 20, 20, TEST_TOPIC_NAME);
}

//! Regression for ROS2 #280 and #281, using unicast
TEST_P(Discovery, TwentyParticipantsSeveralEndpointsUnicast)
{
#if defined(__APPLE__)
    discoverParticipantsSeveralEndpointsTest(true, 10, 10, 10, TEST_TOPIC_NAME);
#else
    discoverParticipantsSeveralEndpointsTest(true, 20, 20, 20, TEST_TOPIC_NAME);
#endif // if defined(__APPLE__)
}

//! Regression test for support case 7552 (CRM #353)
TEST_P(Discovery, RepeatPubGuid)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);

    reader
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .participant_id(2)
            .init();

    writer
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .participant_id(1)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    writer.destroy();
    reader.wait_writer_undiscovery();
    reader.wait_participant_undiscovery();

    writer2
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .participant_id(1)
            .init();

    ASSERT_TRUE(writer2.isInitialized());

    writer2.wait_discovery();
    reader.wait_discovery();

    data = default_helloworld_data_generator();
    reader.startReception(data);

    // Send data
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

//! Regression test for bug 8547: intraprocess segfaults
TEST_P(Discovery, EndpointCreationMultithreaded)
{
    constexpr std::chrono::milliseconds creation_sleep = std::chrono::milliseconds(10);

    std::atomic_bool stop(false);
    PubSubWriterReader<HelloWorldPubSubType> participant_1(TEST_TOPIC_NAME);

    // First participant is initialized
    participant_1.init();

    auto endpoint_creation_process = [&creation_sleep, &stop, &participant_1]()
            {
                while (!stop)
                {
                    std::this_thread::sleep_for(creation_sleep);
                    EXPECT_NO_THROW(participant_1.create_additional_topics(1, "/"));
                }
            };

    // Start thread creating endpoints every 250ms
    std::thread endpoint_thr(endpoint_creation_process);

    // Create another participant that will receive endpoint creation messages.
    // When this participant is removed, the first one should stop sending
    // intraprocess delivery messages to the builtin endpoints of the second one.
    auto second_participant_process = [&participant_1]()
            {
                {
                    PubSubWriterReader<HelloWorldPubSubType> participant_2(TEST_TOPIC_NAME);
                    participant_2.init();

                    // Ensure first participant has discovered the second one
                    participant_1.wait_discovery();
                }

                // Additional endpoints created just after the second participant.
                // This gives the first participant very few time to receive the undiscovery,
                // and makes the intraprocess delivery on a deleted builtin reader.
                participant_1.create_additional_topics(1, "_");
            };

    EXPECT_NO_THROW(second_participant_process());

    // Stop endpoint creation thread
    stop = true;
    endpoint_thr.join();
}

//! Regression test for redmine issue 16253
TEST_P(Discovery, AsymmeticIgnoreParticipantFlags)
{
    if (INTRAPROCESS != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on INTRAPROCESS";
        return;
    }

    // This participant is created with flags to ignore participants which are not on the same process.
    // When the announcements of this participant arrive to p2, a single DATA(p) should be sent back.
    // No other traffic is expected, since it will take place through intraprocess.
    PubSubReader<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    p1.ignore_participant_flags(static_cast<eprosima::fastdds::rtps::ParticipantFilteringFlags>(
                eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST |
                eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS));
    p1.init();
    EXPECT_TRUE(p1.isInitialized());

    // This participant is created with the test transport to check that nothing unexpected is sent to the
    // multicast metatraffic locators.
    // Setting localhost in the interface whitelist ensures that the traffic will not leave the host, and also
    // that multicast datagrams are sent only once.
    // A very long period for the participant announcement is set, along with 0 initial announcements, so we can
    // have a exact expectation on the number of datagrams sent to multicast.
    PubSubWriter<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);

    // This will hold the multicast port. Since the test is not always run in the same domain, we'll need to set
    // its value when the first multicast datagram is sent.
    std::atomic<uint32_t> multicast_port{ 0 };
    // Only one multicast datagram is allowed: the initial DATA(p)
    constexpr uint32_t allowed_messages_on_port = 1;

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    std::atomic<uint32_t> messages_on_port{ 0 };
    test_transport->interfaceWhiteList.push_back("127.0.0.1");
    test_transport->locator_filter_ = [&multicast_port, &messages_on_port](
        const eprosima::fastdds::rtps::Locator& destination)
            {
                if (IPLocator::isMulticast(destination))
                {
                    uint32_t port = 0;
                    multicast_port.compare_exchange_strong(port, destination.port);
                    if (destination.port == multicast_port)
                    {
                        ++messages_on_port;
                    }
                }
                return false;
            };

    p2.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport).
            lease_duration({ 60 * 60, 0 }, { 50 * 60, 0 }).
            initial_announcements(0, {});
    p2.init();
    EXPECT_TRUE(p2.isInitialized());

    // Wait for participants and endpoints to discover each other
    p1.wait_discovery();
    p2.wait_discovery();

    // Check expectation on the number of multicast datagrams sent by p2
    EXPECT_EQ(messages_on_port, allowed_messages_on_port);
}

//! Regression test for redmine issue 22506
TEST_P(Discovery, single_unicast_pdp_response)
{
    // Leverage intraprocess so transport is only used for participant discovery
    if (INTRAPROCESS != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on INTRAPROCESS";
        return;
    }

    using namespace eprosima::fastdds::dds;

    // All participants would restrict communication to UDP localhost.
    // The main participant should send a single initial announcement, and have a big announcement period.
    // This is to ensure that we only check the datagrams sent in response to the participant discovery,
    // and not the ones sent in the periodic announcements.
    // The main participant will use the test transport to count the number of unicast messages sent.

    // This will hold the multicast port. Since the test is not always run in the same domain, we'll need to set
    // its value when the first multicast datagram is sent.
    std::atomic<uint32_t> multicast_port{ 0 };
    // Declare a test transport that will count the number of unicast messages sent
    std::atomic<size_t> num_unicast_sends{ 0 };
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->interfaceWhiteList.push_back("127.0.0.1");
    test_transport->locator_filter_ = [&num_unicast_sends, &multicast_port](
        const eprosima::fastdds::rtps::Locator& destination)
            {
                if (IPLocator::isMulticast(destination))
                {
                    uint32_t port = 0;
                    multicast_port.compare_exchange_strong(port, destination.port);
                }
                else
                {
                    num_unicast_sends.fetch_add(1u, std::memory_order_seq_cst);
                }

                // Do not discard any message
                return false;
            };

    // Create the main participant
    auto main_participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
    WireProtocolConfigQos main_wire_protocol;
    main_wire_protocol.builtin.avoid_builtin_multicast = true;
    main_wire_protocol.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    main_wire_protocol.builtin.discovery_config.leaseDuration_announcementperiod = { 3600, 0 };
    main_wire_protocol.builtin.discovery_config.initial_announcements.count = 1;
    main_wire_protocol.builtin.discovery_config.initial_announcements.period = { 0, 100000000 };

    // The main participant will use the test transport and a specific announcements configuration
    main_participant->disable_builtin_transport().add_user_transport_to_pparams(test_transport)
            .wire_protocol(main_wire_protocol);

    // Start the main participant
    ASSERT_TRUE(main_participant->init_participant());

    // Wait for the initial announcements to be sent
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // This would have set the multicast port
    EXPECT_NE(multicast_port, 0u);

    // The rest of the participants only send announcements to the main participant
    // Calculate the metatraffic unicast port of the main participant
    uint32_t port = multicast_port + main_wire_protocol.port.offsetd1 - main_wire_protocol.port.offsetd0;

    // The rest of the participants only send announcements to the main participant
    auto udp_localhost_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    udp_localhost_transport->interfaceWhiteList.push_back("127.0.0.1");
    Locator peer_locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "127.0.0.1", port, peer_locator);
    WireProtocolConfigQos wire_protocol;
    wire_protocol.builtin.avoid_builtin_multicast = true;
    wire_protocol.builtin.initialPeersList.push_back(peer_locator);

    std::vector<std::shared_ptr<PubSubParticipant<HelloWorldPubSubType>>> participants;
    for (size_t i = 0; i < 5; ++i)
    {
        auto participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
        // All participants use the same transport
        participant->disable_builtin_transport().add_user_transport_to_pparams(udp_localhost_transport)
                .wire_protocol(wire_protocol);
        participants.push_back(participant);
    }

    // Start the rest of the participants
    for (auto& participant : participants)
    {
        ASSERT_TRUE(participant->init_participant());
        participant->wait_discovery();
    }

    // Destroy main participant
    main_participant.reset();
    for (auto& participant : participants)
    {
        participant->wait_discovery(std::chrono::seconds::zero(), 0, true);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Check that only two unicast messages per participant were sent
    EXPECT_EQ(num_unicast_sends.load(std::memory_order::memory_order_seq_cst),
            participants.size() + participants.size());

    // Clean up
    participants.clear();
}

//! Regression test for redmine issue 22506
//! Test using a user's flowcontroller limiting the bandwidth and 5 remote participants waiting for the PDP sample.
TEST_P(Discovery, single_unicast_pdp_response_flowcontroller)
{
    // Leverage intraprocess so transport is only used for participant discovery
    if (INTRAPROCESS != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on INTRAPROCESS";
        return;
    }

    using namespace eprosima::fastdds::dds;

    // All participants would restrict communication to UDP localhost.
    // The main participant should send a single initial announcement, and have a big announcement period.
    // This is to ensure that we only check the datagrams sent in response to the participant discovery,
    // and not the ones sent in the periodic announcements.
    // The main participant will use the test transport to count the number of unicast messages sent.

    // This will hold the multicast port. Since the test is not always run in the same domain, we'll need to set
    // its value when the first multicast datagram is sent.
    std::atomic<uint32_t> multicast_port{ 0 };
    // Declare a test transport that will count the number of unicast messages sent
    std::atomic<size_t> num_unicast_sends{ 0 };
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->interfaceWhiteList.push_back("127.0.0.1");
    test_transport->locator_filter_ = [&num_unicast_sends, &multicast_port](
        const eprosima::fastdds::rtps::Locator& destination)
            {
                if (IPLocator::isMulticast(destination))
                {
                    uint32_t port = 0;
                    multicast_port.compare_exchange_strong(port, destination.port);
                }
                else
                {
                    num_unicast_sends.fetch_add(1u, std::memory_order_seq_cst);
                }

                // Do not discard any message
                return false;
            };

    // Create the main participant
    auto main_participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
    WireProtocolConfigQos main_wire_protocol;
    main_wire_protocol.builtin.avoid_builtin_multicast = true;
    main_wire_protocol.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    main_wire_protocol.builtin.discovery_config.leaseDuration_announcementperiod = { 3600, 0 };
    main_wire_protocol.builtin.discovery_config.initial_announcements.count = 1;
    main_wire_protocol.builtin.discovery_config.initial_announcements.period = { 0, 100000000u };
    main_wire_protocol.builtin.flow_controller_name = "TestFlowController";

    // Flowcontroller to limit the bandwidth
    auto test_flow_controller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
    test_flow_controller->name = "TestFlowController";
    test_flow_controller->max_bytes_per_period = 3700;
    test_flow_controller->period_ms = static_cast<uint64_t>(100);

    // The main participant will use the test transport, specific announcements configuration and a flowcontroller
    main_participant->disable_builtin_transport().add_user_transport_to_pparams(test_transport)
            .wire_protocol(main_wire_protocol)
            .flow_controller(test_flow_controller);

    // Start the main participant
    ASSERT_TRUE(main_participant->init_participant());

    // Wait for the initial announcements to be sent
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // This would have set the multicast port
    EXPECT_NE(multicast_port, 0u);

    // The rest of the participants only send announcements to the main participant
    // Calculate the metatraffic unicast port of the main participant
    uint32_t port = multicast_port + main_wire_protocol.port.offsetd1 - main_wire_protocol.port.offsetd0;

    // The rest of the participants only send announcements to the main participant
    auto udp_localhost_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    udp_localhost_transport->interfaceWhiteList.push_back("127.0.0.1");
    Locator peer_locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "127.0.0.1", port, peer_locator);
    WireProtocolConfigQos wire_protocol;
    wire_protocol.builtin.avoid_builtin_multicast = true;
    wire_protocol.builtin.initialPeersList.push_back(peer_locator);
    wire_protocol.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    wire_protocol.builtin.discovery_config.leaseDuration_announcementperiod = { 3600, 0 };
    wire_protocol.builtin.discovery_config.initial_announcements.count = 1;
    wire_protocol.builtin.discovery_config.initial_announcements.period = { 0, 100000000u };

    std::vector<std::shared_ptr<PubSubParticipant<HelloWorldPubSubType>>> participants;
    for (size_t i = 0; i < 5; ++i)
    {
        auto participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
        // All participants use the same transport
        participant->disable_builtin_transport().add_user_transport_to_pparams(udp_localhost_transport)
                .wire_protocol(wire_protocol);
        participants.push_back(participant);
    }

    // Start the rest of the participants
    for (auto& participant : participants)
    {
        ASSERT_TRUE(participant->init_participant());
        participant->wait_discovery(std::chrono::seconds::zero(), 1, true);
    }

    main_participant->wait_discovery(std::chrono::seconds::zero(), 5, true);

    // When in single threaded application, give some time for the builtin endpoints matching
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Destroy main participant
    main_participant.reset();
    for (auto& participant : participants)
    {
        participant->wait_discovery(std::chrono::seconds::zero(), 0, true);
    }

    // Check that the main participant sends two unicast messages to every other participant.
    // One Data[P] and one Data[uP].
    // Note that in a single core system, the number of unicast messages sent may be one
    // per participant since the main participant's destruction races with
    // the asynchronous Data[uP] in the locator selector (the unicast locator of the remote may not be there by the time)
    // using the multicast instead.
    EXPECT_GE(num_unicast_sends.load(std::memory_order::memory_order_seq_cst),
            participants.size());

    // Clean up
    participants.clear();
}

//! Regression test for redmine issue 22506
//! Same test as single_unicast_pdp_response_flowcontroller but the main participant's builtin controller is so limited
//! that it will not be able to send all the initial announcements.
TEST_P(Discovery, single_unicast_pdp_response_flowcontroller_limited)
{
    // Leverage intraprocess so transport is only used for participant discovery
    if (INTRAPROCESS != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on INTRAPROCESS";
        return;
    }

    using namespace eprosima::fastdds::dds;

    // All participants would restrict communication to UDP localhost.
    // The main participant should send a single initial announcement, and have a big announcement period.
    // This is to ensure that we only check the datagrams sent in response to the participant discovery,
    // and not the ones sent in the periodic announcements.
    // The main participant will use the test transport to count the number of unicast messages sent.

    // This will hold the multicast port. Since the test is not always run in the same domain, we'll need to set
    // its value when the first multicast datagram is sent.
    std::atomic<uint32_t> multicast_port{ 0 };
    // Declare a test transport that will count the number of unicast messages sent
    std::atomic<size_t> num_unicast_sends{ 0 };
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->interfaceWhiteList.push_back("127.0.0.1");
    test_transport->locator_filter_ = [&num_unicast_sends, &multicast_port](
        const eprosima::fastdds::rtps::Locator& destination)
            {
                if (IPLocator::isMulticast(destination))
                {
                    uint32_t port = 0;
                    multicast_port.compare_exchange_strong(port, destination.port);
                }
                else
                {
                    num_unicast_sends.fetch_add(1u, std::memory_order_seq_cst);
                }

                // Do not discard any message
                return false;
            };

    // Create the main participant
    auto main_participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
    WireProtocolConfigQos main_wire_protocol;
    main_wire_protocol.builtin.avoid_builtin_multicast = true;
    main_wire_protocol.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    main_wire_protocol.builtin.discovery_config.leaseDuration_announcementperiod = { 3600, 0 };
    main_wire_protocol.builtin.discovery_config.initial_announcements.count = 1;
    main_wire_protocol.builtin.discovery_config.initial_announcements.period = { 0, 100000000u };
    main_wire_protocol.builtin.flow_controller_name = "TestFlowController";

    // Flowcontroller to limit the bandwidth
    auto test_flow_controller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
    test_flow_controller->name = "TestFlowController";
    test_flow_controller->max_bytes_per_period = 3700;
    test_flow_controller->period_ms = static_cast<uint64_t>(100000);

    // The main participant will use the test transport, specific announcements configuration and a flowcontroller
    main_participant->disable_builtin_transport().add_user_transport_to_pparams(test_transport)
            .wire_protocol(main_wire_protocol)
            .flow_controller(test_flow_controller);

    // Start the main participant
    ASSERT_TRUE(main_participant->init_participant());

    // Wait for the initial announcements to be sent
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // This would have set the multicast port
    EXPECT_NE(multicast_port, 0u);

    // The rest of the participants only send announcements to the main participant
    // Calculate the metatraffic unicast port of the main participant
    uint32_t port = multicast_port + main_wire_protocol.port.offsetd1 - main_wire_protocol.port.offsetd0;

    // The rest of the participants only send announcements to the main participant
    auto udp_localhost_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    udp_localhost_transport->interfaceWhiteList.push_back("127.0.0.1");
    Locator peer_locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "127.0.0.1", port, peer_locator);
    WireProtocolConfigQos wire_protocol;
    wire_protocol.builtin.avoid_builtin_multicast = true;
    wire_protocol.builtin.initialPeersList.push_back(peer_locator);
    wire_protocol.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    wire_protocol.builtin.discovery_config.leaseDuration_announcementperiod = { 3600, 0 };
    wire_protocol.builtin.discovery_config.initial_announcements.count = 1;
    wire_protocol.builtin.discovery_config.initial_announcements.period = { 0, 100000000u };

    std::vector<std::shared_ptr<PubSubParticipant<HelloWorldPubSubType>>> participants;
    for (size_t i = 0; i < 10; ++i)
    {
        auto participant = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);
        // All participants use the same transport
        participant->disable_builtin_transport().add_user_transport_to_pparams(udp_localhost_transport)
                .wire_protocol(wire_protocol);
        participants.push_back(participant);
    }

    // Start the rest of the participants
    for (auto& participant : participants)
    {
        ASSERT_TRUE(participant->init_participant());
        participant->wait_discovery(std::chrono::seconds(1), 1, true);
    }

    // The builtin flowcontroller of the main participant will not be able to send all the initial announcements as the max byter per period has already
    // been reached. In fact no more messages will be sent from the builtin writers of the main participant.
    EXPECT_LT(num_unicast_sends.load(std::memory_order::memory_order_seq_cst), participants.size());
    auto num_unicast_sends_limit = num_unicast_sends.load(std::memory_order::memory_order_seq_cst);

    // Destroy main participant
    main_participant.reset();
    for (auto& participant : participants)
    {
        participant->wait_discovery(std::chrono::seconds(1), 0, true);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // No more messages have been sent sin the limit was reached
    EXPECT_EQ(num_unicast_sends.load(std::memory_order::memory_order_seq_cst), num_unicast_sends_limit);

    // Clean up
    participants.clear();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(Discovery,
        Discovery,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<Discovery::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });

TEST(Discovery, RemoteBuiltinEndpointHonoring)
{

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto reader_test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    auto writer_test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    uint32_t num_wlp_reader_heartbeat = 0;
    uint32_t num_wlp_reader_acknack = 0;

    reader_test_transport->drop_heartbeat_messages_filter_ = [&num_wlp_reader_heartbeat](CDRMessage_t& msg)
            {
                // Go back to submsgkind
                auto submsgkind_pos = msg.pos - 4;
                auto hb_submsg = eprosima::fastdds::helpers::cdr_parse_heartbeat_submsg(
                    (char*)&msg.buffer[submsgkind_pos],
                    msg.length - submsgkind_pos);

                assert(hb_submsg.submsgHeader().submessageId() == HEARTBEAT);

                if (eprosima::fastdds::rtps::c_EntityId_WriterLiveliness ==
                        *reinterpret_cast<EntityId_t*>(&hb_submsg.writerId()))
                {
                    num_wlp_reader_heartbeat++;
                }
                return false;
            };

    reader_test_transport->drop_ack_nack_messages_filter_ = [&num_wlp_reader_acknack](CDRMessage_t& msg)
            {
                // Go back to submsgkind
                auto submsgkind_pos = msg.pos - 4;
                auto acknack_submsg = eprosima::fastdds::helpers::cdr_parse_acknack_submsg(
                    (char*)&msg.buffer[submsgkind_pos],
                    msg.length - submsgkind_pos);

                assert(acknack_submsg.submsgHeader().submessageId() == ACKNACK);

                if (eprosima::fastdds::rtps::c_EntityId_WriterLiveliness ==
                        *reinterpret_cast<EntityId_t*>(&acknack_submsg.writerId()))
                {
                    num_wlp_reader_acknack++;
                }
                return false;
            };

    reader_test_transport->interfaceWhiteList.push_back("127.0.0.1");

    uint32_t num_wlp_writer_heartbeat = 0;
    uint32_t num_wlp_writer_acknack = 0;

    writer_test_transport->drop_heartbeat_messages_filter_ = [&](CDRMessage_t& msg)
            {
                // Go back to submsgkind
                auto submsgkind_pos = msg.pos - 4;
                auto hb_submsg = eprosima::fastdds::helpers::cdr_parse_heartbeat_submsg(
                    (char*)&msg.buffer[submsgkind_pos],
                    msg.length - submsgkind_pos);

                assert(hb_submsg.submsgHeader().submessageId() == HEARTBEAT);

                if (eprosima::fastdds::rtps::c_EntityId_WriterLiveliness ==
                        *reinterpret_cast<EntityId_t*>(&hb_submsg.writerId()))
                {
                    num_wlp_writer_heartbeat++;
                }
                return false;
            };

    writer_test_transport->drop_ack_nack_messages_filter_ = [&num_wlp_writer_acknack](CDRMessage_t& msg)
            {
                // Go back to submsgkind
                auto submsgkind_pos = msg.pos - 4;
                auto acknack_submsg = eprosima::fastdds::helpers::cdr_parse_acknack_submsg(
                    (char*)&msg.buffer[submsgkind_pos],
                    msg.length - submsgkind_pos);

                assert(acknack_submsg.submsgHeader().submessageId() == ACKNACK);

                if (eprosima::fastdds::rtps::c_EntityId_WriterLiveliness ==
                        *reinterpret_cast<EntityId_t*>(&acknack_submsg.writerId()))
                {
                    num_wlp_writer_acknack++;
                }
                return false;
            };

    writer_test_transport->interfaceWhiteList.push_back("127.0.0.1");

    reader.disable_builtin_transport().add_user_transport_to_pparams(reader_test_transport).
            use_writer_liveliness_protocol(false);
    writer.disable_builtin_transport().add_user_transport_to_pparams(writer_test_transport);

    reader.init();
    writer.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));

    std::this_thread::sleep_for(std::chrono::seconds(5));

    ASSERT_EQ(num_wlp_reader_heartbeat, 0u);
    ASSERT_EQ(num_wlp_reader_acknack, 0u);
    ASSERT_EQ(num_wlp_writer_heartbeat, 0u);
    ASSERT_EQ(num_wlp_writer_acknack, 0u);
}

//! Regression test for redmine issue 10674
TEST(Discovery, MulticastInitialPeer)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::LocatorList peers;
    eprosima::fastdds::rtps::Locator loc{};
    loc.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(loc, "239.255.0.1");
    peers.push_back(loc);

    reader.participant_id(100).initial_peers(peers).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.participant_id(101).initial_peers(peers).init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery (times out before the fix).
    writer.wait_discovery();
    reader.wait_discovery();
}

//! Regression test for redmine issue 17162
TEST(Discovery, MultipleXMLProfileLoad)
{
    // These test may fail because one of the participants disappear before the other has found it.
    // Thus, use condition variable so threads only finish once the discovery has taken place.
    std::condition_variable cv;
    std::mutex cv_mtx;
    std::atomic<int> n_discoveries(0);

    auto participant_creation_reader = [&]()
            {
                PubSubReader<HelloWorldPubSubType> participant(TEST_TOPIC_NAME);
                participant.init();
                participant.wait_discovery();

                // Notify discovery has happen
                {
                    std::unique_lock<std::mutex> lock(cv_mtx);
                    n_discoveries++;
                }
                cv.notify_all();

                std::unique_lock<std::mutex> lock(cv_mtx);
                cv.wait(
                    lock,
                    [&]()
                    {
                        return n_discoveries >= 2;
                    }
                    );
            };

    auto participant_creation_writer = [&]()
            {
                PubSubWriter<HelloWorldPubSubType> participant(TEST_TOPIC_NAME);
                participant.init();
                participant.wait_discovery();

                // Notify discovery has happen
                {
                    std::unique_lock<std::mutex> lock(cv_mtx);
                    n_discoveries++;
                }
                cv.notify_all();

                std::unique_lock<std::mutex> lock(cv_mtx);
                cv.wait(
                    lock,
                    [&]()
                    {
                        return n_discoveries >= 2;
                    }
                    );
            };

    // Start thread creating second participant
    std::thread thr_reader(participant_creation_reader);
    std::thread thr_writer(participant_creation_writer);

    thr_reader.join();
    thr_writer.join();
}

//! Regression test for redmine issue 20641
TEST(Discovery, discovery_cyclone_participant_with_custom_pid)
{
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastdds::rtps;

    /* Custom participant listener to count number of discovered participants */
    class DiscoveryListener : public DomainParticipantListener
    {
    public:

        void on_participant_discovery(
                DomainParticipant*,
                ParticipantDiscoveryStatus status,
                const ParticipantBuiltinTopicData& /*info*/,
                bool& should_be_ignored) override
        {
            should_be_ignored = false;
            if (ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT == status)
            {
                discovered_participants_++;
            }
            else if (ParticipantDiscoveryStatus::REMOVED_PARTICIPANT == status)
            {
                discovered_participants_--;
            }
        }

        uint8_t discovered_participants() const
        {
            return discovered_participants_;
        }

    private:

        using DomainParticipantListener::on_participant_discovery;

        std::atomic<uint8_t> discovered_participants_{0};
    };

    /* Create a datagram injection transport */
    using eprosima::fastdds::rtps::DatagramInjectionTransportDescriptor;
    using eprosima::fastdds::rtps::DatagramInjectionTransport;
    auto low_level_transport = std::make_shared<UDPv4TransportDescriptor>();
    auto transport = std::make_shared<DatagramInjectionTransportDescriptor>(low_level_transport);

    /* Disable builtin transport and add custom one */
    DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.clear();
    participant_qos.transport().user_transports.push_back(transport);

    /* Create participant with custom transport and listener */
    DiscoveryListener listener;
    /* We need to match the domain id in the datagram */
    uint32_t domain_id = 0;
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipant* participant = factory->create_participant(domain_id, participant_qos, &listener);
    ASSERT_NE(nullptr, participant);

    /* Inject a Cyclone DDS Data(p) with a custom PID that we also use */
    auto receivers = transport->get_receivers();
    ASSERT_FALSE(receivers.empty());
    DatagramInjectionTransport::deliver_datagram_from_file(receivers, "datagrams/20641.bin");

    /* Assert that the participant is discovered */
    ASSERT_EQ(listener.discovered_participants(), 1u);

    /* Clean up */
    factory->delete_participant(participant);
}

// This test checks that a Discover Server does not send duplicated PDP messages of itself when new clients
// are discovered
TEST_P(Discovery, discovery_server_pdp_messages_sent)
{
    // Skip test in intraprocess and datasharing mode
    if (TRANSPORT != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on TRANSPORT";
        return;
    }

    using namespace eprosima::fastdds::dds;

    // One discovery server will be created, with multiple direct clients connected to it.
    // Initial announcements will be disabled and lease announcements will be configured to control discovery sequence.
    // The main participant will use the test transport to count the number of Data(p) sent.

    // Look for the PID_DOMAIN_ID in the message as it is only present in Data(p) messages
    auto builtin_msg_is_data_p = [](CDRMessage_t& msg, std::atomic<size_t>& num_data_p)
            {
                uint32_t qos_size = 0;
                uint32_t original_pos = msg.pos;
                bool is_sentinel = false;
                bool inline_qos_msg = false;

                while (!is_sentinel)
                {
                    msg.pos = original_pos + qos_size;

                    uint16_t pid = eprosima::fastdds::helpers::cdr_parse_u16(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos += 2;
                    uint16_t plength = eprosima::fastdds::helpers::cdr_parse_u16(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos += 2;
                    bool valid = true;

                    // If inline_qos submessage is found we will have an additional Sentinel
                    if (pid == eprosima::fastdds::dds::PID_RELATED_SAMPLE_IDENTITY)
                    {
                        inline_qos_msg = true;
                    }
                    else if (pid == eprosima::fastdds::dds::PID_SENTINEL)
                    {
                        // PID_SENTINEL is always considered of length 0
                        plength = 0;
                        if (!inline_qos_msg)
                        {
                            // If the PID is not inline qos, then we need to set the sentinel
                            // to true, as it is the last PID
                            is_sentinel = true;
                        }
                    }

                    qos_size += (4 + plength);

                    // Align to 4 byte boundary and prepare for next iteration
                    qos_size = (qos_size + 3) & ~3;

                    if (!valid || ((msg.pos + plength) > msg.length))
                    {
                        return false;
                    }
                    else if (!is_sentinel)
                    {
                        if (pid == eprosima::fastdds::dds::PID_DOMAIN_ID)
                        {
                            std::cout << "Data(p) sent by the server" << std::endl;
                            inline_qos_msg = false;
                            num_data_p.fetch_add(1u, std::memory_order_seq_cst);
                            break;
                        }
                    }
                }

                // Do not drop the packet in any case
                return false;
            };

    // Declare a test transport that will count the number of Data(p) messages sent
    std::atomic<size_t> num_data_p_sends{ 0 };
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->drop_builtin_data_messages_filter_ = [&](CDRMessage_t& msg)
            {
                return builtin_msg_is_data_p(msg, num_data_p_sends);
            };

    // Create the main participant
    auto server = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);

    Locator_t locator_server;  // UDPv4 locator by default
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator_server, 127, 0, 0, 1);
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(locator_server, global_port);

    WireProtocolConfigQos server_wp_qos;
    server_wp_qos.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SERVER;
    server_wp_qos.builtin.metatrafficUnicastLocatorList.push_back(locator_server);

    server_wp_qos.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    server_wp_qos.builtin.discovery_config.leaseDuration_announcementperiod = c_TimeInfinite;
    server_wp_qos.builtin.discovery_config.initial_announcements.count = 0;

    // The main participant will use the test transport and a specific announcements configuration
    server->disable_builtin_transport().add_user_transport_to_pparams(test_transport)
            .wire_protocol(server_wp_qos);

    // Start the main participant
    ASSERT_TRUE(server->init_participant());

    // Create a client that connects to the first server
    PubSubParticipant<HelloWorldPubSubType> client_1(0u, 0u, 0u, 0u);
    PubSubParticipant<HelloWorldPubSubType> client_2(0u, 0u, 0u, 0u);
    PubSubParticipant<HelloWorldPubSubType> client_3(0u, 0u, 0u, 0u);
    // Set participant as client
    WireProtocolConfigQos client_qos;
    client_qos.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::CLIENT;
    client_qos.builtin.discovery_config.m_DiscoveryServers.push_back(locator_server);
    client_qos.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    client_qos.builtin.discovery_config.leaseDuration_announcementperiod = c_TimeInfinite;
    client_qos.builtin.discovery_config.initial_announcements.count = 1;
    // Init client 1
    ASSERT_TRUE(client_1.wire_protocol(client_qos)
                    .setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4)
                    .init_participant());

    // Wait for the initial announcements to be sent
    server->wait_discovery(std::chrono::seconds(5), 1, true);
    // Let some time for the server to run the internal routine and check if it sent Data(p)
    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_EQ(num_data_p_sends.load(std::memory_order::memory_order_seq_cst), 1u);

    // Init client 2
    ASSERT_TRUE(client_2.wire_protocol(client_qos)
                    .setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4)
                    .init_participant());


    // Wait for the initial announcements to be sent
    server->wait_discovery(std::chrono::seconds(5), 2, true);
    // Let some time for the server to run the internal routine and check if it sent Data(p)
    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_EQ(num_data_p_sends.load(std::memory_order::memory_order_seq_cst), 2u);

    // Init client 3
    ASSERT_TRUE(client_3.wire_protocol(client_qos)
                    .setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4)
                    .init_participant());


    // Wait for the initial announcements to be sent
    server->wait_discovery(std::chrono::seconds(5), 3, true);
    // Let some time for the server to run the internal routine and check if it sent Data(p)
    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_EQ(num_data_p_sends.load(std::memory_order::memory_order_seq_cst), 3u);
}

TEST_P(Discovery, discovery_server_edp_messages_sent)
{
    // Skip test in intraprocess and datasharing mode
    if (TRANSPORT != GetParam())
    {
        GTEST_SKIP() << "Only makes sense on TRANSPORT";
        return;
    }

    using namespace eprosima::fastdds::dds;

    // Two discovery servers will be created, each with a direct client connected to them.
    // Initial announcements will be disabled and lease announcements will be configured to control discovery sequence.
    // The main participant will use the test transport to count the number of Data(r/w) sent.

    // Look for the PID_ENDPOINT_GUID in the message as it is only present in Data(r/w) messages
    auto builtin_msg_is_data_r_w = [](CDRMessage_t& msg, std::atomic<size_t>& num_data_r_w)
            {
                uint32_t qos_size = 0;
                uint32_t original_pos = msg.pos;
                bool is_sentinel = false;
                bool inline_qos_msg = false;

                while (!is_sentinel)
                {
                    msg.pos = original_pos + qos_size;

                    uint16_t pid = eprosima::fastdds::helpers::cdr_parse_u16(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos += 2;
                    uint16_t plength = eprosima::fastdds::helpers::cdr_parse_u16(
                        (char*)&msg.buffer[msg.pos]);
                    msg.pos += 2;
                    bool valid = true;

                    if (pid == eprosima::fastdds::dds::PID_RELATED_SAMPLE_IDENTITY)
                    {
                        inline_qos_msg = true;
                    }
                    else if (pid == eprosima::fastdds::dds::PID_SENTINEL)
                    {
                        // PID_SENTINEL is always considered of length 0
                        plength = 0;
                        if (!inline_qos_msg)
                        {
                            // If the PID is not inline qos, then we need to set the sentinel
                            // to true, as it is the last PID
                            is_sentinel = true;
                        }
                    }

                    qos_size += (4 + plength);

                    // Align to 4 byte boundary and prepare for next iteration
                    qos_size = (qos_size + 3) & ~3;

                    if (!valid || ((msg.pos + plength) > msg.length))
                    {
                        return false;
                    }
                    else if (!is_sentinel)
                    {
                        if (pid == eprosima::fastdds::dds::PID_ENDPOINT_GUID)
                        {
                            std::cout << "Data (r/w) sent by the server" << std::endl;
                            num_data_r_w.fetch_add(1u, std::memory_order_seq_cst);
                            break;
                        }
                        else if (pid == eprosima::fastdds::dds::PID_VENDORID)
                        {
                            // Vendor ID is present in both Data(p) and Data(r/w) messages
                            inline_qos_msg = false;
                        }
                    }
                }

                // Do not drop the packet in any case
                return false;
            };

    // Declare a test transport that will count the number of Data(r/w) messages sent
    std::atomic<size_t> num_data_r_w_sends_s1{ 0 };
    std::atomic<size_t> num_data_r_w_sends_s2{ 0 };
    auto test_transport_s1 = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport_s1->drop_builtin_data_messages_filter_ = [&](CDRMessage_t& msg)
            {
                return builtin_msg_is_data_r_w(msg, num_data_r_w_sends_s1);
            };

    auto test_transport_s2 = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport_s2->drop_builtin_data_messages_filter_ = [&](CDRMessage_t& msg)
            {
                return builtin_msg_is_data_r_w(msg, num_data_r_w_sends_s2);
            };

    // Create server 1
    auto server_1 = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);

    Locator_t locator_server_1;  // UDPv4 locator by default
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator_server_1, 127, 0, 0, 1);
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(locator_server_1, global_port);

    WireProtocolConfigQos server_wp_qos_1;
    server_wp_qos_1.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SERVER;
    server_wp_qos_1.builtin.metatrafficUnicastLocatorList.push_back(locator_server_1);

    server_wp_qos_1.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    server_wp_qos_1.builtin.discovery_config.leaseDuration_announcementperiod = c_TimeInfinite;
    server_wp_qos_1.builtin.discovery_config.initial_announcements.count = 0;

    // The main participant will use the test transport and a specific announcements configuration
    server_1->disable_builtin_transport().add_user_transport_to_pparams(test_transport_s1)
            .wire_protocol(server_wp_qos_1);

    // Start the main participant
    ASSERT_TRUE(server_1->init_participant());

    // Create server 2
    auto server_2 = std::make_shared<PubSubParticipant<HelloWorldPubSubType>>(0, 0, 0, 0);

    Locator_t locator_server_2 = locator_server_1;  // UDPv4 locator by default
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(locator_server_2, global_port + 1);

    WireProtocolConfigQos server_wp_qos_2 = server_wp_qos_1;
    server_wp_qos_2.builtin.metatrafficUnicastLocatorList.clear();
    server_wp_qos_2.builtin.metatrafficUnicastLocatorList.push_back(locator_server_2);
    // Configure 1 initial announcement as this Server will connect to the first one
    server_wp_qos_2.builtin.discovery_config.initial_announcements.count = 1;
    server_wp_qos_2.builtin.discovery_config.m_DiscoveryServers.push_back(locator_server_1);

    // The main participant will use the test transport and a specific announcements configuration
    server_2->disable_builtin_transport().add_user_transport_to_pparams(test_transport_s2)
            .wire_protocol(server_wp_qos_2);

    // Start the main participant
    ASSERT_TRUE(server_2->init_participant());

    // Both servers match
    server_1->wait_discovery(std::chrono::seconds(5), 1, true);
    server_2->wait_discovery(std::chrono::seconds(5), 1, true);
    // Let some time for the server to run the internal routine and match virtual endpoints
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Create a client that connects to their corresponding server
    PubSubWriter<HelloWorldPubSubType> client_1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> client_2(TEST_TOPIC_NAME);
    // Set participant as client
    WireProtocolConfigQos client_qos;
    client_qos.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::CLIENT;
    client_qos.builtin.discovery_config.m_DiscoveryServers.push_back(locator_server_1);
    client_qos.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    client_qos.builtin.discovery_config.leaseDuration_announcementperiod = { 15, 0 };
    client_qos.builtin.discovery_config.initial_announcements.count = 0;

    // Init client 1
    client_1.set_wire_protocol_qos(client_qos)
            .setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4)
            .init();

    // Init client 2
    client_qos.builtin.discovery_config.m_DiscoveryServers.clear();
    client_qos.builtin.discovery_config.m_DiscoveryServers.push_back(locator_server_2);
    client_2.set_wire_protocol_qos(client_qos)
            .setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4)
            .init();

    ASSERT_TRUE(client_1.isInitialized());
    ASSERT_TRUE(client_2.isInitialized());

    // Wait the lease announcement period to discover endpoints
    server_1->wait_discovery(std::chrono::seconds(5), 2, true);
    server_2->wait_discovery(std::chrono::seconds(5), 2, true);

    // Ensure that no additional Data(r/w) messages are sent by DS routine
    std::this_thread::sleep_for(std::chrono::seconds(15));

    EXPECT_EQ(num_data_r_w_sends_s1.load(std::memory_order::memory_order_seq_cst), 2u);
    EXPECT_EQ(num_data_r_w_sends_s2.load(std::memory_order::memory_order_seq_cst), 2u);
}
