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

#include "BlackboxTests.hpp"

#include "PubSubWriterReader.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <gtest/gtest.h>

#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastdds/rtps/attributes/ServerAttributes.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class Discovery : public testing::TestWithParam<bool>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }

    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }
    }

};

TEST_P(Discovery, ParticipantRemoval)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

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

TEST(Discovery, StaticDiscovery)
{
    //Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
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

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterUnicastLocators;
    Locator_t LocatorBuffer;

    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = static_cast<uint16_t>(W_UNICAST_PORT_RANDOM_NUMBER);
    IPLocator::setIPv4(LocatorBuffer, 127, 0, 0, 1);
    WriterUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t WriterMulticastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    WriterMulticastLocators.push_back(LocatorBuffer);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS);
    writer.static_discovery("PubSubWriter.xml").reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    unicastLocatorList(WriterUnicastLocators).multicastLocatorList(WriterMulticastLocators).
    setPublisherIDs(1, 2).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();


    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);

    LocatorList_t ReaderUnicastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(R_UNICAST_PORT_RANDOM_NUMBER);
    ReaderUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t ReaderMulticastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    ReaderMulticastLocators.push_back(LocatorBuffer);


    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS);
    reader.static_discovery("PubSubReader.xml").
    unicastLocatorList(ReaderUnicastLocators).multicastLocatorList(ReaderMulticastLocators).
    setSubscriberIDs(3, 4).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

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
    PubSubWriter<HelloWorldType> checker(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType>* reader = new PubSubReader<HelloWorldType>(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType>* writer = new PubSubWriter<HelloWorldType>(TEST_TOPIC_NAME);

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
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport).
    lease_duration({ 0, 800000000 }, { 0, 500000000 }).reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

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
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.partition("A").init();

    ASSERT_TRUE(reader.isInitialized());

    const std::string xml =
            R"(<profiles>
  <publisher profile_name="partition_publisher_profile">
    <topic>
      <name>)" + writer.topic_name() +
            R"(</name>
      <dataType>HelloWorldType</dataType>
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

// Used to detect Github issue #154
TEST(Discovery, LocalInitialPeers)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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
    initial_peers(writer_initial_peers).init();

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

// Test created to check bug #2010 (Github #90)
TEST_P(Discovery, PubSubAsReliableHelloworldPartitions)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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

    ASSERT_TRUE(reader.update_partition("OtherPartition"));

    reader.wait_writer_undiscovery();
    writer.wait_reader_undiscovery();

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

TEST_P(Discovery, PubSubAsReliableHelloworldParticipantDiscovery)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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

//! Auxiliar method for discovering participants tests
static void discoverParticipantsTest(
        bool avoid_multicast,
        size_t n_participants,
        uint32_t wait_ms,
        const std::string& topic_name)
{
    std::vector<std::shared_ptr<PubSubWriterReader<HelloWorldType> > > pubsub;
    pubsub.reserve(n_participants);

    for (size_t i = 0; i < n_participants; ++i)
    {
        pubsub.emplace_back(std::make_shared<PubSubWriterReader<HelloWorldType> >(topic_name));
    }

    // Initialization of all the participants
    std::cout << "Initializing PubSubs for topic " << topic_name << std::endl;
    uint32_t idx = 1;
    for (auto& ps : pubsub)
    {
        std::cout << "\rParticipant " << idx++ << " of " << n_participants << std::flush;
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

//! Tests discovery of 20 participants, having one publisher and one subscriber each, using multicast
TEST(Discovery, TwentyParticipantsMulticast)
{
    discoverParticipantsTest(false, 20, 20, TEST_TOPIC_NAME);
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

    std::vector<std::shared_ptr<PubSubWriterReader<HelloWorldType> > > pubsub;
    pubsub.reserve(n_participants);

    for (unsigned int i = 0; i < n_participants; i++)
    {
        pubsub.emplace_back(std::make_shared<PubSubWriterReader<HelloWorldType> >(topic_name));
    }

    // Initialization of all the participants
    std::cout << "Initializing PubSubs for topic " << topic_name << std::endl;
    uint32_t idx = 1;
    for (auto& ps : pubsub)
    {
        std::cout << "\rParticipant " << idx++ << " of " << n_participants << std::flush;
        ps->init(avoid_multicast);
        ASSERT_EQ(ps->isInitialized(), true);
        ASSERT_TRUE(ps->create_additional_topics(n_topics - 1));
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
    discoverParticipantsSeveralEndpointsTest(true, 20, 20, 20, TEST_TOPIC_NAME);
}

//! Regression test for support case 7552 (CRM #353)
TEST_P(Discovery, RepeatPubGuid)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer2(TEST_TOPIC_NAME);

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
    PubSubWriterReader<HelloWorldType> participant_1(TEST_TOPIC_NAME);

    // First participant is initialized
    participant_1.init();

    auto endpoint_creation_process = [&creation_sleep, &stop, &participant_1]()
            {
                while (!stop)
                {
                    std::this_thread::sleep_for(creation_sleep);
                    EXPECT_NO_THROW(participant_1.create_additional_topics(1));
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
                    PubSubWriterReader<HelloWorldType> participant_2(TEST_TOPIC_NAME);
                    participant_2.init();

                    // Ensure first participant has discovered the second one
                    participant_1.wait_discovery();
                }

                // Additional endpoints created just after the second participant.
                // This gives the first participant very few time to receive the undiscovery,
                // and makes the intraprocess delivery on a deleted builtin reader.
                participant_1.create_additional_topics(1);
            };

    EXPECT_NO_THROW(second_participant_process());

    // Stop endpoint creation thread
    stop = true;
    endpoint_thr.join();
}

INSTANTIATE_TEST_CASE_P(Discovery,
        Discovery,
        testing::Values(false, true),
        [](const testing::TestParamInfo<Discovery::ParamType>& info)
        {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });

//! Tests the server-client setup using environment variable works fine
TEST(Discovery, ServerClientEnvironmentSetUp)
{
    using namespace std;

    RemoteServerList_t output, standard;
    RemoteServerAttributes att;
    Locator_t loc;

    // We are going to use several test string and check they are properly parsed and turn into RemoteServerList_t
    // 1. single server address without specific port provided
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

    // 2. single server address specifying a custom listening port
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

    // 3. check any locator is turned into localhost
    text = "0.0.0.0:14520";

    att.clear();
    output.clear();
    standard.clear();
    IPLocator::setIPv4(loc, string("127.0.0.1"));
    IPLocator::setPhysicalPort(loc, 14520);
    att.metatrafficUnicastLocatorList.push_back(loc);
    get_server_client_default_guidPrefix(0, att.guidPrefix);
    standard.push_back(att);

    ASSERT_TRUE(load_environment_server_info(text, output));
    ASSERT_EQ(output, standard);

    // 4. check empty string scenario is handled
    text = "";
    output.clear();

    ASSERT_FALSE(load_environment_server_info(text, output));

    // 5. check at least one server be present scenario is hadled
    text = ";;;;";
    output.clear();

    ASSERT_FALSE(load_environment_server_info(text, output));

    // 6. check several server scenario
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

    // 7. check ignore some servers scenario
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

    // 7. check ignore some servers scenario
    text = ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;"
            ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;"
            ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;192.168.36.34:14520";
    output.clear();

    ASSERT_FALSE(load_environment_server_info(text, output));

    // 8. check non-consistent addresses scenario
    text = "192.168.36.34:14520;localhost:12345;172.30.80.1:31090;";

    output.clear();
    ASSERT_FALSE(load_environment_server_info(text, output));

}
