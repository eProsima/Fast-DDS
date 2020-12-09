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
#include <fastrtps/utils/IPFinder.h>

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

