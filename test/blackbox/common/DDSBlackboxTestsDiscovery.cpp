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

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.h>
#include <rtps/transport/test_UDPv4Transport.h>
#include <utils/SystemInfo.hpp>

// Regression test for redmine issue 11857
TEST(DDSDiscovery, IgnoreParticipantFlags)
{
    // This participant is created with:
    // - ignoreParticipantFlags = FILTER_SAME_PROCESS (will avoid discovery of p2)
    // - metatrafficUnicastLocatorList = 127.0.0.1:7399, 127.0.0.1:7398 (to ensure two listening threads are created)
    PubSubReader<HelloWorldType> p1(TEST_TOPIC_NAME);
    p1.set_xml_filename("discovery_participant_flags.xml");
    p1.set_participant_profile("participant_1");
    p1.init();
    EXPECT_TRUE(p1.isInitialized());

    // This participant is created with initialPeersList = 127.0.0.1:7399
    // When the announcements of this participant arrive to p1, they will be ignored, and thus p1 will not
    // announce itself back to p2.
    PubSubReader<HelloWorldType> p2(TEST_TOPIC_NAME);
    p2.set_xml_filename("discovery_participant_flags.xml");
    p2.set_participant_profile("participant_2");
    p2.init();
    EXPECT_TRUE(p2.isInitialized());
    EXPECT_FALSE(p2.wait_participant_discovery(1, std::chrono::seconds(1)));
    EXPECT_FALSE(p1.wait_participant_discovery(1, std::chrono::seconds(1)));

    // This participant is created with:
    // - initialPeersList = 127.0.0.1:7398
    // - a custom guid prefix
    // The announcements of this participant will arrive to p1 on a different listening thread.
    // Due to the custom prefix, they should not be ignored, and mutual discovery should happen
    PubSubReader<HelloWorldType> p3(TEST_TOPIC_NAME);
    p3.set_xml_filename("discovery_participant_flags.xml");
    p3.set_participant_profile("participant_3");
    p3.init();
    EXPECT_TRUE(p1.wait_participant_discovery());
    EXPECT_TRUE(p3.wait_participant_discovery());
}

/**
 * This test checks that adding servers to the Discovery Server list results in discovering those participants.
 * It does so by:
 *    1. Creating two servers and one client that is only connected to one server. At this point check discovery
 *       state.
 *    2. Then, connect the client to the other server and check discovery again.
 *    3. Finally connect the two servers by adding one of them to the others list
 */
TEST(DDSDiscovery, AddDiscoveryServerToList)
{
    using namespace eprosima;
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastrtps::rtps;

    /* Get random port from the environment */
    std::string value;
    if (eprosima::ReturnCode_t::RETCODE_OK != SystemInfo::get_env("W_UNICAST_PORT_RANDOM_NUMBER", value))
    {
        value = std::string("11811");
    }

    /* Create first server */
    PubSubParticipant<HelloWorldType> server_1(0u, 0u, 0u, 0u);
    // Set participant as server
    WireProtocolConfigQos server_1_qos;
    server_1_qos.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SERVER;
    // Generate random GUID prefix
    srand(static_cast<unsigned>(time(nullptr)));
    GuidPrefix_t server_1_prefix;
    for (auto i = 0; i < 12; i++)
    {
        server_1_prefix.value[i] = eprosima::fastrtps::rtps::octet(rand() % 254);
    }
    server_1_qos.prefix = server_1_prefix;
    // Generate server's listening locator
    Locator_t locator_server_1;
    IPLocator::setIPv4(locator_server_1, 127, 0, 0, 1);
    uint32_t server_1_port = atol(value.c_str());
    locator_server_1.port = server_1_port;
    server_1_qos.builtin.metatrafficUnicastLocatorList.push_back(locator_server_1);
    // Init server
    ASSERT_TRUE(server_1.wire_protocol(server_1_qos).init_participant());

    /* Create second server */
    PubSubParticipant<HelloWorldType> server_2(0u, 0u, 0u, 0u);
    // Set participant as server
    WireProtocolConfigQos server_2_qos;
    server_2_qos.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SERVER;
    // Generate random GUID prefix
    GuidPrefix_t server_2_prefix = server_1_prefix;
    server_2_prefix.value[11]++;
    server_2_qos.prefix = server_2_prefix;
    // Generate server's listening locator
    Locator_t locator_server_2;
    IPLocator::setIPv4(locator_server_2, 127, 0, 0, 1);
    uint32_t server_2_port = atol(value.c_str()) + 1;
    locator_server_2.port = server_2_port;
    server_2_qos.builtin.metatrafficUnicastLocatorList.push_back(locator_server_2);
    // Init server
    ASSERT_TRUE(server_2.wire_protocol(server_2_qos).init_participant());

    /* Create a client that connects to the first server from the beginning */
    PubSubParticipant<HelloWorldType> client(0u, 0u, 0u, 0u);
    // Set participant as client
    WireProtocolConfigQos client_qos;
    client_qos.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::CLIENT;
    // Connect to first server
    RemoteServerAttributes server_1_att;
    server_1_att.guidPrefix = server_1_prefix;
    server_1_att.metatrafficUnicastLocatorList.push_back(Locator_t(locator_server_1));
    client_qos.builtin.discovery_config.m_DiscoveryServers.push_back(server_1_att);
    // Init client
    ASSERT_TRUE(client.wire_protocol(client_qos).init_participant());

    /**
     * Check that server_1 and client_1 only know each other, and that server_2 does has not
     * discovered anyone
     */
    server_1.wait_discovery(std::chrono::seconds::zero(), 1, true);
    client.wait_discovery(std::chrono::seconds::zero(), 1, true);
    server_2.wait_discovery(std::chrono::seconds::zero(), 0, true);

    /* Add server_2 to client */
    RemoteServerAttributes server_2_att;
    server_2_att.guidPrefix = server_2_prefix;
    server_2_att.metatrafficUnicastLocatorList.push_back(Locator_t(locator_server_2));
    client_qos.builtin.discovery_config.m_DiscoveryServers.push_back(server_2_att);
    // Update client's servers list
    ASSERT_TRUE(client.update_wire_protocol(client_qos));

    /* Check that the servers only know about the client, and that the client known about both servers */
    server_1.wait_discovery(std::chrono::seconds::zero(), 1, true);
    client.wait_discovery(std::chrono::seconds::zero(), 2, true);
    server_2.wait_discovery(std::chrono::seconds::zero(), 1, true);

    /* Add server_2 to server_1 */
    server_1_qos.builtin.discovery_config.m_DiscoveryServers.push_back(server_2_att);
    ASSERT_TRUE(server_1.update_wire_protocol(server_1_qos));

    /* Check that they all know each other */
    server_1.wait_discovery(std::chrono::seconds::zero(), 2, true);
    client.wait_discovery(std::chrono::seconds::zero(), 2, true);
    server_2.wait_discovery(std::chrono::seconds::zero(), 2, true);
}

/**
 * This test checks the addition of network interfaces at run-time.
 *
 * After launching the reader with the network interfaces enabled,
 * the writer is launched with the transport simulating that there
 * are no interfaces.
 * No participant discovery occurs.
 *
 * In a second step, the flag to simulate no interfaces is disabled and
 * DomainParticipant::set_qos() called to add the "new" interfaces.
 * Discovery is succesful and communication is established.
 */
TEST(DDSDiscovery, DDSNetworkInterfaceChangesAtRunTime)
{
    using namespace eprosima::fastdds::rtps;

    PubSubWriter<HelloWorldType> datawriter(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType> datareader(TEST_TOPIC_NAME);

    // datareader is initialized with all the network interfaces
    datareader.init();
    ASSERT_TRUE(datareader.isInitialized());

    // datawriter: launch without interfaces
    test_UDPv4Transport::simulate_no_interfaces = true;
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    datawriter.disable_builtin_transport().add_user_transport_to_pparams(test_transport).init();
    ASSERT_TRUE(datawriter.isInitialized());

    // no discovery
    datawriter.wait_discovery(std::chrono::seconds(3));
    datareader.wait_discovery(std::chrono::seconds(3));

    EXPECT_FALSE(datawriter.is_matched());
    EXPECT_FALSE(datareader.is_matched());

    // enable interfaces
    test_UDPv4Transport::simulate_no_interfaces = false;
    datawriter.participant_set_qos();

    // Wait for discovery
    datawriter.wait_discovery(std::chrono::seconds(3));
    datareader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(datawriter.is_matched());
    ASSERT_TRUE(datareader.is_matched());

    auto complete_data = default_helloworld_data_generator();

    datareader.startReception(complete_data);

    datawriter.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    datareader.block_for_all();

    datareader.destroy();
    datawriter.destroy();
}
