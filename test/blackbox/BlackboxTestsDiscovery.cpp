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

#include <fastrtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TEST(BlackBox, ParticipantRemoval)
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

TEST(BlackBox, StaticDiscovery)
{
    //Log::SetVerbosity(Log::Info);
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

TEST(BlackBox, EDPSlaveReaderAttachment)
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
TEST(BlackBox, EndpointRediscovery)
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
TEST(BlackBox, EndpointRediscovery_2)
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

// Regression test of Refs #2535, github micro-RTPS #1
TEST(BlackBox, PubXmlLoadedPartition)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.partition("A").init();

    ASSERT_TRUE(reader.isInitialized());

    const std::string xml = R"(<profiles>
  <publisher profile_name="partition_publisher_profile">
    <topic>
      <name>)" + writer.topic_name() + R"(</name>
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
TEST(BlackBox, LocalInitialPeers)
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
TEST(BlackBox, PubSubAsReliableHelloworldPartitions)
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

TEST(BlackBox, PubSubAsReliableHelloworldParticipantDiscovery)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    int count = 0;
    reader.setOnDiscoveryFunction([&writer, &count](const ParticipantDiscoveryInfo& info) -> bool{
            if(info.info.m_guid == writer.participant_guid())
            {
                if(info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
                {
                    std::cout << "Discovered participant " << info.info.m_guid << std::endl;
                    ++count;
                }
                else if(info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT ||
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

TEST(BlackBox, PubSubAsReliableHelloworldUserData)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).
        userData({'a','b','c','d'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.setOnDiscoveryFunction([&writer](const ParticipantDiscoveryInfo& info) -> bool{
            if(info.info.m_guid == writer.participant_guid())
            {
                std::cout << "Received USER_DATA from the writer: ";
                for (auto i: info.info.m_userData) std::cout << i << ' ';
                return info.info.m_userData == std::vector<octet>({'a','b','c','d'});
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

//! Tests discovery of 20 participants, having one publisher and one subscriber each
TEST(Discovery, TwentyParticipants)
{
    // Number of participants
    constexpr size_t n_participants = 20;
    // Wait time for discovery
    constexpr unsigned int wait_ms = 20;

    std::vector<std::shared_ptr<PubSubWriterReader<HelloWorldType>>> pubsub;
    pubsub.reserve(n_participants);

    for (unsigned int i=0; i<n_participants; i++)
    {
        pubsub.emplace_back(std::make_shared<PubSubWriterReader<HelloWorldType>>(TEST_TOPIC_NAME));
    }

    // Initialization of all the participants
    for (auto& ps : pubsub)
    {
        ps->init();
        ASSERT_EQ(ps->isInitialized(), true);
    }

    bool all_discovered = false;
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

//! Regression for ROS2 #280 and #281
TEST(Discovery, TwentyParticipantsSeveralEndpoints)
{
    // Number of participants
    constexpr size_t n_participants = 20;
    // Number of endpoints
    constexpr size_t n_topics = 10;
    // Total number of discovered endpoints
    constexpr size_t n_total_endpoints = n_participants * n_topics;
    // Wait time for discovery
    constexpr unsigned int wait_ms = 20;

    std::vector<std::shared_ptr<PubSubWriterReader<HelloWorldType>>> pubsub;
    pubsub.reserve(n_participants);

    for (unsigned int i = 0; i < n_participants; i++)
    {
        pubsub.emplace_back(std::make_shared<PubSubWriterReader<HelloWorldType>>(TEST_TOPIC_NAME));
    }

    // Initialization of all the participants
    for (auto& ps : pubsub)
    {
        ps->init();
        ASSERT_EQ(ps->isInitialized(), true);
        ASSERT_TRUE(ps->create_additional_topics(n_topics - 1));
    }

    bool all_discovered = false;
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
