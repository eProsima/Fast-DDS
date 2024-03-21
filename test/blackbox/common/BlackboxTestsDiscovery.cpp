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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <rtps/transport/test_UDPv4Transport.h>
#include <utils/SystemInfo.hpp>

#include "BlackboxTests.hpp"
#include "DatagramInjectionTransport.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;
using test_UDPv4TransportDescriptor = eprosima::fastdds::rtps::test_UDPv4TransportDescriptor;

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
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
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
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
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

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .property_policy(writer_property_policy);
    writer.static_discovery("file://PubSubWriter_static_disc.xml").reliability(
        eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
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


    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
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

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS);
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


    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS);
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

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    reader.disable_builtin_transport();
    reader.add_user_transport_to_pparams(testTransport);

    reader.lease_duration({ 3, 0 }, { 1, 0 }).reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    // We drop 20% of all data frags
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.lease_duration({ 6, 0 }, { 2, 0 }).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = true;

    writer.wait_reader_undiscovery();

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = false;

    writer.wait_discovery();
}

// Used to detect Github issue #457
TEST(Discovery, EndpointRediscovery_2)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();

    reader.lease_duration({ 120, 0 }, { 1, 0 }).reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.lease_duration({ 2, 0 }, { 1, 0 }).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = true;

    reader.wait_participant_undiscovery();

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = false;

    reader.wait_discovery();
}

// Regression for bug #9629
TEST(Discovery, EndpointRediscoveryWithTransientLocalData)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();

    reader
            .lease_duration({ 120, 0 }, { 1, 0 })
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer
            .lease_duration({ 2, 0 }, { 1, 0 })
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
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

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = true;

    reader.wait_participant_undiscovery();

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = false;

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
            { 0, 500000000 }).reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport).
            lease_duration({ 0, 800000000 }, { 0, 500000000 }).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    test_UDPv4Transport::always_drop_participant_builtin_topic_data = true;

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

    test_UDPv4Transport::always_drop_participant_builtin_topic_data = false;

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
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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

    auto old_locator_filter = test_UDPv4Transport::locator_filter;
    test_UDPv4Transport::locator_filter = locator_printer;

    // Configure writer participant:
    // - Uses the test transport, to check destination behavior
    // - Listens for metatraffic on `writer_port`
    // - Has no automatic announcements
    {
        auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

        LocatorList_t writer_metatraffic_unicast;
        Locator_t locator;
        locator.port = static_cast<uint16_t>(writer_port);
        writer_metatraffic_unicast.push_back(locator);

        writer.disable_builtin_transport().
                add_user_transport_to_pparams(test_transport).
                metatraffic_unicast_locator_list(writer_metatraffic_unicast).
                lease_duration(c_TimeInfinite, { 3600, 0 }).
                initial_announcements(0, {}).
                reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
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
                lease_duration(c_TimeInfinite, {3600, 0}).
                initial_announcements(1, {0, 100 * 1000 * 1000}).
                metatraffic_unicast_locator_list(reader_metatraffic_unicast).
                initial_peers(reader_initial_peers).
                reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
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

    // Restore filter before deleting the participants
    test_UDPv4Transport::locator_filter = old_locator_filter;
}

TEST_P(Discovery, PubSubAsReliableHelloworldParticipantDiscovery)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    int count = 0;
    reader.setOnDiscoveryFunction([&writer, &count](const ParticipantDiscoveryInfo& info) -> bool
            {
                if (info.info.m_guid == writer.participant_guid())
                {
                    if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
                    {
                        std::cout << "Discovered participant " << info.info.m_guid << std::endl;
                        ++count;
                    }
                    else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT ||
                    info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
                    {
                        std::cout << "Removed participant " << info.info.m_guid << std::endl;
                        return ++count == 2;
                    }
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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
            userData({'a', 'b', 'c', 'd'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.setOnDiscoveryFunction([&writer](const ParticipantDiscoveryInfo& info) -> bool
            {
                if (info.info.m_guid == writer.participant_guid())
                {
                    std::cout << "Received USER_DATA from the writer: ";
                    for (auto i: info.info.m_userData)
                    {
                        std::cout << i << ' ';
                    }
                    return info.info.m_userData == std::vector<octet>({'a', 'b', 'c', 'd'});
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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

    reader.setOnEndpointDiscoveryFunction([&writer](const WriterDiscoveryInfo& info) -> bool
            {
                if (info.info.guid() == writer.datawriter_guid())
                {
                    std::cout << "Received USER_DATA from the writer: ";
                    for (auto i: info.info.m_qos.m_userData)
                    {
                        std::cout << i << ' ';
                    }
                    return info.info.m_qos.m_userData == std::vector<octet>({'a', 'b', 'c', 'd'});
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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

    test_UDPv4Transport::simulate_no_interfaces = true;
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
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .participant_id(2)
            .init();

    writer
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
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
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
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

// Regression test for redmine issue 16253
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
    p1.ignore_participant_flags(static_cast<eprosima::fastrtps::rtps::ParticipantFilteringFlags_t>(
                eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_HOST |
                eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_PROCESS));
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
    // Only two multicast datagrams are allowed: the initial DATA(p) and the DATA(p) sent in response of the discovery
    // of p1.
    constexpr uint32_t allowed_messages_on_port = 2;

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

//! Tests the server-client setup using environment variable works fine
TEST(Discovery, ServerClientEnvironmentSetUp)
{
    using namespace std;
    using namespace eprosima::fastdds::rtps;

    RemoteServerList_t output, standard;
    RemoteServerAttributes att;
    Locator_t loc, loc6(LOCATOR_KIND_UDPv6, 0);

    // We are going to use several test string and check they are properly parsed and turn into RemoteServerList_t
    // 1. Single server address without specific port provided
    string text = "192.168.36.34";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, text);
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 2. Single server IPv6 address without specific port provided
    text = "2a02:26f0:dd:499::356e";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, text);
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 3. Single server address specifying a custom listening port
    text = "192.168.36.34:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, text);
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 4. Single server IPv6 address specifying a custom listening port
    text = "[2001:470:142:5::116]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2001:470:142:5::116");
    IPLocator::setPhysicalPort(loc6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 5. Check any locator is turned into localhost
    text = "0.0.0.0:14520;[::]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, "127.0.0.1");
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv6(loc6, "::1");
    IPLocator::setPhysicalPort(loc6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 6. Check empty string scenario is handled
    text = "";
    output.clear();

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_TRUE(output.empty());

    // 7. Check at least one server be present scenario is hadled
    text = ";;;;";
    output.clear();

    ASSERT_FALSE(load_environment_server_info(text, output));

    // 8. Check several server scenario
    text = "192.168.36.34:14520;172.29.55.77:8783;172.30.80.1:31090";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.29.55.77"));
    IPLocator::setPhysicalPort(loc, 8783);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(2, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 9. Check several server scenario with IPv6 addresses too
    text = "192.168.36.34:14520;[2a02:ec80:600:ed1a::3]:8783;172.30.80.1:31090";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, "192.168.36.34");
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv6(loc6, "2a02:ec80:600:ed1a::3");
    IPLocator::setPhysicalPort(loc6, 8783);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(2, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 10. Check multicast addresses are identified as such
    text = "239.255.0.1;ff1e::ffff:efff:1";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, "239.255.0.1");
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficMulticastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv6(loc6, "ff1e::ffff:efff:1");
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficMulticastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 11. Check ignore some servers scenario
    text = ";192.168.36.34:14520;;172.29.55.77:8783;172.30.80.1:31090;";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.29.55.77"));
    IPLocator::setPhysicalPort(loc, 8783);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(3, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(4, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 12. Check that env var cannot specify more than 256 servers
    text = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;"
            ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;"
            ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;192.168.36.34:14520";
    output.clear();

    ASSERT_FALSE(load_environment_server_info(text, output));

    // 13. Check addresses as dns name (test domain urls are checked on a specific test)
    text = "localhost:12345";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, "127.0.0.1");
    IPLocator::setPhysicalPort(loc, 12345);
    att.metatrafficUnicastLocatorList.push_back(loc);
    IPLocator::setIPv6(loc6, "::1");
    IPLocator::setPhysicalPort(loc6, 12345);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 14. Check mixed scenario with addresses and dns
    text = "192.168.36.34:14520;localhost:12345;172.30.80.1:31090;";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("127.0.0.1"));
    IPLocator::setPhysicalPort(loc, 12345);
    att.metatrafficUnicastLocatorList.push_back(loc);
    IPLocator::setIPv6(loc6, string("::1"));
    IPLocator::setPhysicalPort(loc6, 12345);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(2, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCP transport

    Locator_t loc_tcp(LOCATOR_KIND_TCPv4, 0);
    Locator_t loc_tcp_6(LOCATOR_KIND_TCPv6, 0);

    // 15. Single TCPv4 address without specifying a custom listening port

    text = "TCPv4:[192.168.36.34]";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "192.168.36.34");
    IPLocator::setPhysicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 16. Single TCPv6 address without specifying a custom listening port

    text = "TCPv6:[2a02:26f0:dd:499::356e]";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a02:26f0:dd:499::356e");
    IPLocator::setPhysicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp_6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 17. Single TCPv4 address specifying a custom listening port

    text = "TCPv4:[192.168.36.34]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "192.168.36.34");
    IPLocator::setPhysicalPort(loc_tcp, 14520);
    IPLocator::setLogicalPort(loc_tcp, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 18. Single TCPv6 address specifying a custom listening port

    text = "TCPv6:[2a02:26f0:dd:499::356e]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a02:26f0:dd:499::356e");
    IPLocator::setPhysicalPort(loc_tcp_6, 14520);
    IPLocator::setLogicalPort(loc_tcp_6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp_6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);
}

//! Tests the server-client setup using environment variable works fine using DNS
TEST(Discovery, ServerClientEnvironmentSetUpDNS)
{
    using namespace std;
    using namespace eprosima::fastdds::rtps;

    RemoteServerList_t output, standard;
    RemoteServerAttributes att;
    Locator_t loc, loc6(LOCATOR_KIND_UDPv6, 0);

    Locator_t loc_tcp(LOCATOR_KIND_TCPv4, 0);
    Locator_t loc_tcp_6(LOCATOR_KIND_TCPv6, 0);

    // 1. single server DNS address resolution without specific port provided
    std::string text = "www.acme.com.test";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 2. single server DNS address specifying a custom listening port
    text = "www.acme.com.test:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 3. single server DNS address specifying a custom locator type
    // UDPv4
    text = "UDPv4:[www.acme.com.test]";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // UDPv6
    text = "UDPv6:[www.acme.com.test]";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, DEFAULT_ROS2_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv4
    text = "TCPv4:[www.acme.com.test]";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "216.58.215.164");
    IPLocator::setPhysicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp, DEFAULT_TCP_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv6
    text = "TCPv6:[www.acme.com.test]";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    IPLocator::setLogicalPort(loc_tcp_6, DEFAULT_TCP_SERVER_PORT);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp_6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 4. single server DNS address specifying a custom locator type and listening port
    // UDPv4
    text = "UDPv4:[www.acme.com.test]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, "216.58.215.164");
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // UDPv6
    text = "UDPv6:[www.acme.com.test]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv4
    text = "TCPv4:[www.acme.com.test]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc_tcp, "216.58.215.164");
    IPLocator::setPhysicalPort(loc_tcp, 14520);
    IPLocator::setLogicalPort(loc_tcp, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // TCPv6
    text = "TCPv6:[www.acme.com.test]:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv6(loc_tcp_6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc_tcp_6, 14520);
    IPLocator::setLogicalPort(loc_tcp_6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc_tcp_6);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // SHM Locator kind should fail
    text = "SHM:[www.acme.com.test]";

    output.clear();
    ASSERT_FALSE(load_environment_server_info(text, output));

    // 5. Check mixed scenario with addresses and dns
    text = "192.168.36.34:14520;UDPv6:[www.acme.com.test]:14520;172.30.80.1:31090;";

    output.clear();
    standard.clear();

    att.clear();
    IPLocator::setIPv4(loc, string("192.168.36.34"));
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv6(loc6, "2a00:1450:400e:803::2004");
    IPLocator::setPhysicalPort(loc6, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc6);
    get_server_client_default_guidPrefix(1, att.guidPrefix);
    standard.push_back(att);

    att.clear();
    IPLocator::setIPv4(loc, string("172.30.80.1"));
    IPLocator::setPhysicalPort(loc, 31090);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(2, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);
}

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
                auto old_pos = msg.pos;
                msg.pos += 4;
                eprosima::fastrtps::rtps::EntityId_t writer_entity_id;
                eprosima::fastrtps::rtps::CDRMessage::readEntityId(&msg, &writer_entity_id);
                msg.pos = old_pos;

                if (eprosima::fastrtps::rtps::c_EntityId_WriterLiveliness == writer_entity_id)
                {
                    num_wlp_reader_heartbeat++;
                }
                return false;
            };

    reader_test_transport->drop_ack_nack_messages_filter_ = [&num_wlp_reader_acknack](CDRMessage_t& msg)
            {
                auto old_pos = msg.pos;
                msg.pos += 4;
                eprosima::fastrtps::rtps::EntityId_t writer_entity_id;
                eprosima::fastrtps::rtps::CDRMessage::readEntityId(&msg, &writer_entity_id);
                msg.pos = old_pos;

                if (eprosima::fastrtps::rtps::c_EntityId_WriterLiveliness == writer_entity_id)
                {
                    num_wlp_reader_acknack++;
                }
                return false;
            };

    reader_test_transport->interfaceWhiteList.push_back("127.0.0.1");

    uint32_t num_wlp_writer_heartbeat = 0;
    uint32_t num_wlp_writer_acknack = 0;

    writer_test_transport->drop_heartbeat_messages_filter_ = [&num_wlp_writer_heartbeat](CDRMessage_t& msg)
            {
                auto old_pos = msg.pos;
                msg.pos += 4;
                eprosima::fastrtps::rtps::EntityId_t writer_entity_id;
                eprosima::fastrtps::rtps::CDRMessage::readEntityId(&msg, &writer_entity_id);
                msg.pos = old_pos;

                if (eprosima::fastrtps::rtps::c_EntityId_WriterLiveliness == writer_entity_id)
                {
                    num_wlp_writer_heartbeat++;
                }
                return false;
            };

    writer_test_transport->drop_ack_nack_messages_filter_ = [&num_wlp_writer_acknack](CDRMessage_t& msg)
            {
                auto old_pos = msg.pos;
                msg.pos += 4;
                eprosima::fastrtps::rtps::EntityId_t writer_entity_id;
                eprosima::fastrtps::rtps::CDRMessage::readEntityId(&msg, &writer_entity_id);
                msg.pos = old_pos;

                if (eprosima::fastrtps::rtps::c_EntityId_WriterLiveliness == writer_entity_id)
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
    using namespace eprosima::fastrtps::rtps;

    /* Custom participant listener to count number of discovered participants */
    class DiscoveryListener : public DomainParticipantListener
    {
    public:

        void on_participant_discovery(
                DomainParticipant*,
                ParticipantDiscoveryInfo&& info) override
        {
            if (ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT == info.status)
            {
                discovered_participants_++;
            }
            else if (ParticipantDiscoveryInfo::REMOVED_PARTICIPANT == info.status)
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
    uint32_t domain_id = static_cast<uint32_t>(GET_PID()) % 230;
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
