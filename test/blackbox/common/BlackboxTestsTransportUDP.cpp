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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <gtest/gtest.h>

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TEST(BlackBox, UDPv4TransportWrongConfig)
{
    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 100000;

        writer.disable_builtin_transport().
                add_user_transport_to_pparams(testTransport).init();

        ASSERT_FALSE(writer.isInitialized());
    }

    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->sendBufferSize = 64000;

        writer.disable_builtin_transport().
                add_user_transport_to_pparams(testTransport).init();

        ASSERT_FALSE(writer.isInitialized());
    }

    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->receiveBufferSize = 64000;

        writer.disable_builtin_transport().
                add_user_transport_to_pparams(testTransport).init();

        ASSERT_FALSE(writer.isInitialized());
    }
}

// TODO - GASCO: UDPMaxInitialPeer tests should use static discovery through initial peers.
TEST(BlackBox, UDPMaxInitialPeer_P0_4_P3)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    eprosima::fastrtps::rtps::IPFinder::getIP4Address(&loc);

    reader.participant_id(0).max_initial_peers_range(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.participant_id(3).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

TEST(BlackBox, UDPMaxInitialPeer_P0_4_P4)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    eprosima::fastrtps::rtps::IPFinder::getIP4Address(&loc);

    reader.participant_id(0).max_initial_peers_range(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

TEST(BlackBox, UDPMaxInitialPeer_P5_4_P4)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    eprosima::fastrtps::rtps::IPFinder::getIP4Address(&loc);

    reader.participant_id(5).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.max_initial_peers_range(4).participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(reader.is_matched());
}

TEST(BlackBox, UDPMaxInitialPeer_P5_6_P4)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    eprosima::fastrtps::rtps::IPFinder::getIP4Address(&loc);

    reader.participant_id(5).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.max_initial_peers_range(6).participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

// Used to reproduce VPN environment issue with multicast.
TEST(BlackBox, MulticastCommunicationBadReader)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto transport = std::make_shared<UDPv4TransportDescriptor>();
    std::string ip0("127.0.0.1");
    std::string ip1("239.255.1.4");
    std::string ip2("239.255.1.5");

    transport->interfaceWhiteList.push_back(ip0);

    writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
    writer.add_to_metatraffic_multicast_locator_list(ip2, global_port);
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> readerMultiBad(TEST_TOPIC_NAME);
    readerMultiBad.disable_builtin_transport().add_user_transport_to_pparams(transport);
    readerMultiBad.add_to_metatraffic_multicast_locator_list(ip1, global_port);
    readerMultiBad.init();

    ASSERT_TRUE(readerMultiBad.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    readerMultiBad.wait_discovery(std::chrono::seconds(3));
    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(readerMultiBad.is_matched());
}

// Used to reproduce VPN environment issue with multicast.
TEST(BlackBox, MulticastCommunicationOkReader)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto transport = std::make_shared<UDPv4TransportDescriptor>();
    std::string ip0("127.0.0.1");
    std::string ip2("239.255.1.5");

    transport->interfaceWhiteList.push_back(ip0);

    writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
    writer.add_to_metatraffic_multicast_locator_list(ip2, global_port);
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> readerMultiOk(TEST_TOPIC_NAME);
    readerMultiOk.disable_builtin_transport().add_user_transport_to_pparams(transport);
    readerMultiOk.add_to_metatraffic_multicast_locator_list(ip2, global_port);
    readerMultiOk.init();

    ASSERT_TRUE(readerMultiOk.isInitialized());

    writer.wait_discovery();
    readerMultiOk.wait_discovery();
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(readerMultiOk.is_matched());
}

// #4420 Using whitelists in localhost sometimes UDP doesn't receive the release input channel message.
TEST(BlackBox, whitelisting_udp_localhost_multi)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto transport = std::make_shared<UDPv4TransportDescriptor>();
    std::string ip0("127.0.0.1");

    transport->interfaceWhiteList.push_back(ip0);

    writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    for (int i = 0; i < 200; ++i)
    {
        PubSubReader<HelloWorldType> readerMultiOk(TEST_TOPIC_NAME);
        readerMultiOk.disable_builtin_transport().add_user_transport_to_pparams(transport);
        readerMultiOk.init();

        ASSERT_TRUE(readerMultiOk.isInitialized());

        writer.wait_discovery();
        readerMultiOk.wait_discovery();
        ASSERT_TRUE(writer.is_matched());
        ASSERT_TRUE(readerMultiOk.is_matched());
    }
}

// Checking correct copying of participant user data locators to the writers/readers
TEST(BlackBox, DefaultMulticastLocatorsParticipant)
{
    size_t writer_samples = 5;

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.add_to_default_multicast_locator_list("239.255.0.1", 22222);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.add_to_default_multicast_locator_list("239.255.0.1", 22222);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Checking correct copying of participant metatraffic locators to the datawriters/datatreaders
TEST(BlackBox, MetatraficMulticastLocatorsParticipant)
{
    Log::SetVerbosity(Log::Kind::Warning);

    size_t writer_samples = 5;

    auto transport = std::make_shared<UDPv4TransportDescriptor>();

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
    writer.add_to_metatraffic_multicast_locator_list("239.255.1.1", 22222);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.disable_builtin_transport().add_user_transport_to_pparams(transport);
    reader.add_to_metatraffic_multicast_locator_list("239.255.1.1", 22222);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Checking correct copying of participant user data locators to the writers/readers
TEST(BlackBox, DefaultMulticastLocatorsParticipantZeroPort)
{
    size_t writer_samples = 5;

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.add_to_default_multicast_locator_list("239.255.0.1", 0);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.add_to_default_multicast_locator_list("239.255.0.1", 0);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Checking correct copying of participant metatraffic locators to the datawriters/datatreaders
TEST(BlackBox, MetatraficMulticastLocatorsParticipantZeroPort)
{
    Log::SetVerbosity(Log::Kind::Warning);

    size_t writer_samples = 5;

    auto transport = std::make_shared<UDPv4TransportDescriptor>();

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
    writer.add_to_metatraffic_multicast_locator_list("239.255.1.1", 0);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.disable_builtin_transport().add_user_transport_to_pparams(transport);
    reader.add_to_metatraffic_multicast_locator_list("239.255.1.1", 0);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// #4420 Using whitelists in localhost sometimes UDP doesn't receive the release input channel message.
TEST(BlackBox, whitelisting_udp_localhost_alone)
{
    auto transport = std::make_shared<UDPv4TransportDescriptor>();
    std::string ip0("127.0.0.1");

    transport->interfaceWhiteList.push_back(ip0);

    for (int i = 0; i < 200; ++i)
    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
        writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
        writer.init();
        ASSERT_TRUE(writer.isInitialized());
    }
}

// Test for ==operator UDPTransportDescriptor is not required as it is an abstract class and in UDPv4 is same method
// Test for copy UDPTransportDescriptor is not required as it is an abstract class and in UDPv4 is same method

// Test == operator for UDPv4
TEST(BlackBox, UDPv4_equal_operator)
{
    // UDPv4TransportDescriptor
    UDPv4TransportDescriptor udpv4_transport_1;
    UDPv4TransportDescriptor udpv4_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(udpv4_transport_1, udpv4_transport_2);

    // Modify some default values in 1
    udpv4_transport_1.non_blocking_send = !udpv4_transport_1.non_blocking_send; // change default value
    udpv4_transport_1.m_output_udp_socket = udpv4_transport_1.m_output_udp_socket + 10; // change default value

    ASSERT_FALSE(udpv4_transport_1 == udpv4_transport_2); // operator== != operator!=, using operator== == false instead

    // Modify default values in 2
    udpv4_transport_2.non_blocking_send = !udpv4_transport_2.non_blocking_send; // change default value
    udpv4_transport_2.m_output_udp_socket = udpv4_transport_2.m_output_udp_socket + 10; // change default value

    ASSERT_EQ(udpv4_transport_1, udpv4_transport_2);
}

// Test copy constructor and copy assignment for UDPv4
TEST(BlackBox, UDPv4_copy)
{
    UDPv4TransportDescriptor udpv4_transport;
    udpv4_transport.non_blocking_send = !udpv4_transport.non_blocking_send; // change default value
    udpv4_transport.m_output_udp_socket = udpv4_transport.m_output_udp_socket + 10; // change default value

    // Copy constructor
    UDPv4TransportDescriptor udpv4_transport_copy_constructor(udpv4_transport);
    EXPECT_EQ(udpv4_transport, udpv4_transport_copy_constructor);

    // Copy assignment
    UDPv4TransportDescriptor udpv4_transport_copy = udpv4_transport;
    EXPECT_EQ(udpv4_transport_copy, udpv4_transport);
}

// Test == operator for UDPv6
TEST(BlackBox, UDPv6_equal_operator)
{
    // UDPv6TransportDescriptor
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_1;
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(udpv6_transport_1, udpv6_transport_2);

    // Modify some default values in 1
    udpv6_transport_1.non_blocking_send = !udpv6_transport_1.non_blocking_send; // change default value
    udpv6_transport_1.m_output_udp_socket = udpv6_transport_1.m_output_udp_socket + 10; // change default value

    ASSERT_FALSE(udpv6_transport_1 == udpv6_transport_2); // operator== != operator!=, using operator== == false instead


    // Modify some default values in 2
    udpv6_transport_2.non_blocking_send = !udpv6_transport_2.non_blocking_send; // change default value
    udpv6_transport_2.m_output_udp_socket = udpv6_transport_2.m_output_udp_socket + 10; // change default value

    ASSERT_EQ(udpv6_transport_1, udpv6_transport_2);
}

// Test copy constructor and copy assignment for UDPv6
TEST(BlackBox, UDPv6_copy)
{
    // Change some varibles in order to check the non default cretion
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport;
    udpv6_transport.non_blocking_send = !udpv6_transport.non_blocking_send; // change default value
    udpv6_transport.m_output_udp_socket = udpv6_transport.m_output_udp_socket + 10; // change default value

    // Copy constructor
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_copy_constructor(udpv6_transport);
    EXPECT_EQ(udpv6_transport, udpv6_transport_copy_constructor);

    // Copy assignment
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_copy = udpv6_transport;
    EXPECT_EQ(udpv6_transport_copy, udpv6_transport);
}
