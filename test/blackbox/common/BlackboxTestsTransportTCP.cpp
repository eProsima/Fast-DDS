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

#include <chrono>
#include <thread>
#include <random>

#include <gtest/gtest.h>

#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>

#include "TCPReqRepHelloWorldRequester.hpp"
#include "TCPReqRepHelloWorldReplier.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT
};

class TransportTCP : public testing::TestWithParam<std::tuple<communication_type, bool>>
{
public:

    void SetUp() override
    {
        test_transport_.reset();
        use_ipv6 = std::get<1>(GetParam());
        if (use_ipv6)
        {
#ifdef __APPLE__
            // TODO: fix IPv6 issues related with zone ID
            GTEST_SKIP() << "TCPv6 tests are disabled in Mac";
#endif // ifdef __APPLE__
            test_transport_ = std::make_shared<TCPv6TransportDescriptor>();
        }
        else
        {
            test_transport_ = std::make_shared<TCPv4TransportDescriptor>();
        }
    }

    void TearDown() override
    {
        use_ipv6 = false;
    }

    std::shared_ptr<TCPTransportDescriptor> test_transport_;
};

// TCP and Domain management with logical ports tests
TEST_P(TransportTCP, TCPDomainHelloWorld_P0_P1_D0_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;
    const uint16_t nmsgs = 5;

    requester.init(0, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(1, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(requester.is_matched());
    ASSERT_TRUE(replier.is_matched());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block();
    }
}

TEST_P(TransportTCP, TCPDomainHelloWorld_P0_P1_D0_D1)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(1, 1, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());
}

TEST_P(TransportTCP, TCPDomainHelloWorld_P0_P1_D1_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 1, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(1, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());

}

TEST_P(TransportTCP, TCPDomainHelloWorld_P0_P3_D0_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;
    const uint16_t nmsgs = 5;

    requester.init(0, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block();
    }

}

TEST_P(TransportTCP, TCPDomainHelloWorld_P0_P3_D0_D1)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 1, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());
}

TEST_P(TransportTCP, TCPDomainHelloWorld_P0_P3_D1_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 1, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());

}

TEST_P(TransportTCP, TCPDomainHelloWorld_P3_P0_D0_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;
    const uint16_t nmsgs = 5;

    requester.init(3, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(0, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(requester.is_matched());
    ASSERT_TRUE(replier.is_matched());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block();
    }

}

TEST_P(TransportTCP, TCPDomainHelloWorld_P3_P0_D0_D1)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(3, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(0, 1, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());
}

TEST_P(TransportTCP, TCPDomainHelloWorld_P3_P0_D1_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(3, 1, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(0, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());

}

TEST_P(TransportTCP, TCPDomainHelloWorld_P2_P3_D0_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;
    const uint16_t nmsgs = 5;

    requester.init(2, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block();
    }

}

TEST_P(TransportTCP, TCPDomainHelloWorld_P2_P3_D0_D1)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(2, 0, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 1, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());
}

TEST_P(TransportTCP, TCPDomainHelloWorld_P2_P3_D1_D0)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(2, 1, global_port);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery. They must not discover each other.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());
}

TEST_P(TransportTCP, TCPMaxInitialPeer_P0_4_P3)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 0, global_port, 4);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(3, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(requester.is_matched());
    ASSERT_TRUE(replier.is_matched());
}

TEST_P(TransportTCP, TCPMaxInitialPeer_P0_4_P4)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 0, global_port, 4);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(4, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery(std::chrono::seconds(10));

    ASSERT_FALSE(requester.is_matched());
    ASSERT_FALSE(replier.is_matched());
}

TEST_P(TransportTCP, TCPMaxInitialPeer_P0_5_P4)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 0, global_port, 5);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(4, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(requester.is_matched());
    ASSERT_TRUE(replier.is_matched());
}

#if TLS_FOUND
TEST_P(TransportTCP, TCP_TLS)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;

    requester.init(0, 0, global_port, 5, certs_path);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(4, 0, global_port, 5, certs_path);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(requester.is_matched());
    ASSERT_TRUE(replier.is_matched());
}

// Test successful removal of client after previously matched server is removed
TEST_P(TransportTCP, TCP_TLS_client_disconnect_after_server)
{
    TCPReqRepHelloWorldRequester* requester = new TCPReqRepHelloWorldRequester();
    TCPReqRepHelloWorldReplier* replier = new TCPReqRepHelloWorldReplier();

    requester->init(0, 0, global_port, 5, certs_path);

    ASSERT_TRUE(requester->isInitialized());

    replier->init(4, 0, global_port, 5, certs_path);

    ASSERT_TRUE(replier->isInitialized());

    // Wait for discovery.
    requester->wait_discovery();
    replier->wait_discovery();

    ASSERT_TRUE(requester->is_matched());
    ASSERT_TRUE(replier->is_matched());

    // Completely remove server prior to deleting client
    delete replier;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    delete requester;
}

// Test successful removal of server after previously matched client is removed
// Issue -> https://eprosima.easyredmine.com/issues/16288
TEST_P(TransportTCP, TCP_TLS_server_disconnect_after_client)
{
    TCPReqRepHelloWorldReplier* replier = new TCPReqRepHelloWorldReplier();
    TCPReqRepHelloWorldRequester* requester = new TCPReqRepHelloWorldRequester();

    requester->init(0, 0, global_port, 5, certs_path);

    ASSERT_TRUE(requester->isInitialized());

    replier->init(4, 0, global_port, 5, certs_path);

    ASSERT_TRUE(replier->isInitialized());

    // Wait for discovery.
    requester->wait_discovery();
    replier->wait_discovery();

    ASSERT_TRUE(requester->is_matched());
    ASSERT_TRUE(replier->is_matched());

    // Completely remove client prior to deleting server
    delete requester;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    delete replier;
}

void tls_init()
{
    certs_path = std::getenv("CERTS_PATH");

    if (certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        exit(-1);
    }
}

#endif // if TLS_FOUND

// Regression test for ShrinkLocators/transform_remote_locators mechanism.
TEST_P(TransportTCP, TCPLocalhost)
{
    TCPReqRepHelloWorldRequester requester;
    TCPReqRepHelloWorldReplier replier;
    const uint16_t nmsgs = 5;

    requester.init(0, 0, global_port, 0, nullptr, true);

    ASSERT_TRUE(requester.isInitialized());

    replier.init(1, 0, global_port);

    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery.
    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(requester.is_matched());
    ASSERT_TRUE(replier.is_matched());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block();
    }
}

// Test for ==operator TCPTransportDescriptor is not required as it is an abstract class and in TCPv6 is same method
// Test for copy TCPTransportDescriptor is not required as it is an abstract class and in TCPv6 is same method

// Test == operator for TCPv4
TEST_P(TransportTCP, TCPv4_equal_operator)
{
    // TCPv4TransportDescriptor
    TCPv4TransportDescriptor tcpv4_transport_1;
    TCPv4TransportDescriptor tcpv4_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(tcpv4_transport_1, tcpv4_transport_2);

    // Modify default values in 1
    tcpv4_transport_1.set_WAN_address("80.80.99.45");

    ASSERT_FALSE(tcpv4_transport_1 == tcpv4_transport_2); // operator== != operator!=, using operator== == false instead

    // Modify default values in 2
    tcpv4_transport_2.set_WAN_address("80.80.99.45");

    ASSERT_EQ(tcpv4_transport_1, tcpv4_transport_2);
}

// Test copy constructor and copy assignment for TCPv4
TEST_P(TransportTCP, TCPv4_copy)
{
    TCPv4TransportDescriptor tcpv4_transport;
    tcpv4_transport.set_WAN_address("80.80.99.45");

    // Copy constructor
    TCPv4TransportDescriptor tcpv4_transport_copy_constructor(tcpv4_transport);
    EXPECT_EQ(tcpv4_transport, tcpv4_transport_copy_constructor);

    // Copy assignment
    TCPv4TransportDescriptor tcpv4_transport_copy = tcpv4_transport;
    EXPECT_EQ(tcpv4_transport_copy, tcpv4_transport);
}

// Test get_WAN_address member function
TEST_P(TransportTCP, TCPv4_get_WAN_address)
{
    // TCPv4TransportDescriptor
    TCPv4TransportDescriptor tcpv4_transport;
    tcpv4_transport.set_WAN_address("80.80.99.45");
    ASSERT_EQ(tcpv4_transport.get_WAN_address(), "80.80.99.45");
}

// Test == operator for TCPv6
TEST_P(TransportTCP, TCPv6_equal_operator)
{
    // TCPv6TransportDescriptor
    TCPv6TransportDescriptor tcpv6_transport_1;
    TCPv6TransportDescriptor tcpv6_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(tcpv6_transport_1, tcpv6_transport_2);

    // Modify some default values in 1
    tcpv6_transport_1.enable_tcp_nodelay = !tcpv6_transport_1.enable_tcp_nodelay; // change default value
    tcpv6_transport_1.max_logical_port = tcpv6_transport_1.max_logical_port + 10; // change default value
    tcpv6_transport_1.add_listener_port(123u * 98u);

    ASSERT_FALSE(tcpv6_transport_1 == tcpv6_transport_2); // operator== != operator!=, using operator== == false instead


    // Modify some default values in 2
    tcpv6_transport_2.enable_tcp_nodelay = !tcpv6_transport_2.enable_tcp_nodelay; // change default value
    tcpv6_transport_2.max_logical_port = tcpv6_transport_2.max_logical_port + 10; // change default value
    tcpv6_transport_2.add_listener_port(123u * 98u);

    ASSERT_EQ(tcpv6_transport_1, tcpv6_transport_2);
}

// Test copy constructor and copy assignment for TCPv6
TEST_P(TransportTCP, TCPv6_copy)
{
    // Change some varibles in order to check the non default creation
    TCPv6TransportDescriptor tcpv6_transport;
    tcpv6_transport.enable_tcp_nodelay = !tcpv6_transport.enable_tcp_nodelay; // change default value
    tcpv6_transport.max_logical_port = tcpv6_transport.max_logical_port + 10; // change default value
    tcpv6_transport.add_listener_port(123u * 98u);

    // Copy constructor
    TCPv6TransportDescriptor tcpv6_transport_copy_constructor(tcpv6_transport);
    EXPECT_EQ(tcpv6_transport, tcpv6_transport_copy_constructor);

    // Copy assignment
    TCPv6TransportDescriptor tcpv6_transport_copy = tcpv6_transport;
    EXPECT_EQ(tcpv6_transport_copy, tcpv6_transport);
}

// Test connection is successfully restablished after dropping and relaunching a TCP client (requester)
// Issue -> https://github.com/eProsima/Fast-DDS/issues/2409
TEST(TransportTCP, Client_reconnection)
{
    TCPReqRepHelloWorldReplier* replier;
    TCPReqRepHelloWorldRequester* requester;
    const uint16_t nmsgs = 5;

    replier = new TCPReqRepHelloWorldReplier;
    replier->init(1, 0, global_port);

    ASSERT_TRUE(replier->isInitialized());

    requester = new TCPReqRepHelloWorldRequester;
    requester->init(0, 0, global_port);

    ASSERT_TRUE(requester->isInitialized());

    // Wait for discovery.
    replier->wait_discovery();
    requester->wait_discovery();

    ASSERT_TRUE(replier->is_matched());
    ASSERT_TRUE(requester->is_matched());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester->send(count);
        requester->block();
    }

    // Release TCP client resources.
    delete requester;

    // Wait until unmatched.
    replier->wait_unmatched();
    ASSERT_FALSE(replier->is_matched());

    // Create new TCP client instance.
    requester = new TCPReqRepHelloWorldRequester;
    requester->init(0, 0, global_port);

    ASSERT_TRUE(requester->isInitialized());

    // Wait for discovery.
    replier->wait_discovery();
    requester->wait_discovery();

    ASSERT_TRUE(replier->is_matched());
    ASSERT_TRUE(requester->is_matched());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester->send(count);
        requester->block();
    }

    delete replier;
    delete requester;
}

// Test copy constructor and copy assignment for TCPv4
TEST_P(TransportTCP, TCPv4_autofill_port)
{
    PubSubReader<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);

    // Add TCP Transport with listening port 0
    auto p1_transport = std::make_shared<TCPv4TransportDescriptor>();
    p1_transport->add_listener_port(0);
    p1.disable_builtin_transport().add_user_transport_to_pparams(p1_transport);
    p1.init();
    ASSERT_TRUE(p1.isInitialized());

    // Add TCP Transport with listening port different from 0
    uint16_t port = 12345;
    auto p2_transport = std::make_shared<TCPv4TransportDescriptor>();
    p2_transport->add_listener_port(port);
    p2.disable_builtin_transport().add_user_transport_to_pparams(p2_transport);
    p2.init();
    ASSERT_TRUE(p2.isInitialized());

    LocatorList_t p1_locators;
    p1.get_native_reader().get_listening_locators(p1_locators);
    EXPECT_TRUE(IPLocator::getPhysicalPort(p1_locators.begin()[0]) != 0);

    LocatorList_t p2_locators;
    p2.get_native_reader().get_listening_locators(p2_locators);
    EXPECT_TRUE(IPLocator::getPhysicalPort(p2_locators.begin()[0]) == port);
}

// Test copy constructor and copy assignment for TCPv6
TEST_P(TransportTCP, TCPv6_autofill_port)
{
    PubSubReader<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);

    // Add TCP Transport with listening port 0
    auto p1_transport = std::make_shared<TCPv6TransportDescriptor>();
    p1_transport->add_listener_port(0);
    p1.disable_builtin_transport().add_user_transport_to_pparams(p1_transport);
    p1.init();
    ASSERT_TRUE(p1.isInitialized());

    // Add TCP Transport with listening port different from 0
    uint16_t port = 12345;
    auto p2_transport = std::make_shared<TCPv6TransportDescriptor>();
    p2_transport->add_listener_port(port);
    p2.disable_builtin_transport().add_user_transport_to_pparams(p2_transport);
    p2.init();
    ASSERT_TRUE(p2.isInitialized());

    LocatorList_t p1_locators;
    p1.get_native_reader().get_listening_locators(p1_locators);
    EXPECT_TRUE(IPLocator::getPhysicalPort(p1_locators.begin()[0]) != 0);

    LocatorList_t p2_locators;
    p2.get_native_reader().get_listening_locators(p2_locators);
    EXPECT_TRUE(IPLocator::getPhysicalPort(p2_locators.begin()[0]) == port);
}

// Test TCP transport on LARGE_DATA topology
TEST_P(TransportTCP, large_data_topology)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);

    // Limited to 12 readers and 12 writers so as not to exceed the system's file descriptor limit.
    uint16_t n_participants = 12;
    constexpr uint32_t samples_per_participant = 10;

    /* Test configuration */
    std::vector<std::unique_ptr<PubSubReader<KeyedHelloWorldPubSubType>>> readers;
    std::vector<std::unique_ptr<PubSubWriter<KeyedHelloWorldPubSubType>>> writers;

    for (uint16_t i = 0; i < n_participants; i++)
    {
        readers.emplace_back(new PubSubReader<KeyedHelloWorldPubSubType>(TEST_TOPIC_NAME));
        writers.emplace_back(new PubSubWriter<KeyedHelloWorldPubSubType>(TEST_TOPIC_NAME));
    }

    // Create a vector of ports and shuffle it
    std::vector<uint16_t> ports;
    for (uint16_t i = 0; i < 2 * n_participants; i++)
    {
        ports.push_back(7200 + i);
    }
    auto rng = std::default_random_engine{};
    std::shuffle(ports.begin(), ports.end(), rng);

    // Reliable Keep_all to wait for all acked as end condition
    for (uint16_t i = 0; i < n_participants; i++)
    {
        writers[i]->reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
                .lease_duration(eprosima::fastrtps::c_TimeInfinite, eprosima::fastrtps::Duration_t(3, 0))
                .resource_limits_max_instances(1)
                .resource_limits_max_samples_per_instance(samples_per_participant);

        readers[i]->reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
                .lease_duration(eprosima::fastrtps::c_TimeInfinite, eprosima::fastrtps::Duration_t(3, 0))
                .resource_limits_max_instances(n_participants)
                .resource_limits_max_samples_per_instance(samples_per_participant);

        // Force TCP EDP discovery & data communication and UDP PDP discovery (NO SHM)
        writers[i]->setup_large_data_tcp(use_ipv6, ports[i]);
        readers[i]->setup_large_data_tcp(use_ipv6, ports[n_participants + i]);
    }

    // Init participants
    for (uint16_t i = 0; i < n_participants; i++)
    {
        writers[i]->init();
        readers[i]->init();
        ASSERT_TRUE(writers[i]->isInitialized());
        ASSERT_TRUE(readers[i]->isInitialized());
    }

    // Wait for discovery
    for (uint16_t i = 0; i < n_participants; i++)
    {
        writers[i]->wait_discovery(n_participants, std::chrono::seconds(0));
        ASSERT_EQ(writers[i]->get_matched(), n_participants);
        readers[i]->wait_discovery(std::chrono::seconds(0), n_participants);
        ASSERT_EQ(readers[i]->get_matched(), n_participants);
    }

    // Send and receive data
    std::list<KeyedHelloWorld> data;
    data = default_keyedhelloworld_per_participant_data_generator(n_participants, samples_per_participant);

    for (auto& reader : readers)
    {
        reader->startReception(data);
    }

    auto validate_key = [](const std::list<KeyedHelloWorld>& data, uint16_t participant_key)
            {
                for (const auto& sample : data)
                {
                    ASSERT_EQ(sample.key(), participant_key);
                }
            };

    for (uint16_t i = 0; i < n_participants; i++)
    {
        auto start = std::next(data.begin(), i * samples_per_participant );
        auto end = std::next(start, samples_per_participant);
        auto writer_data(std::list<KeyedHelloWorld>(start, end));
        validate_key(writer_data, i);
        writers[i]->send(writer_data);
        EXPECT_TRUE(writer_data.empty());
    }

    for (auto& reader : readers)
    {
        reader->block_for_all();
    }
    for (auto& writer : writers)
    {
        EXPECT_TRUE(writer->waitForAllAcked(std::chrono::seconds(5)));
    }

    // Destroy participants
    readers.clear();
    writers.clear();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(TransportTCP,
        TransportTCP,
        testing::Combine(testing::Values(TRANSPORT), testing::Values(false, true)),
        [](const testing::TestParamInfo<TransportTCP::ParamType>& info)
        {
            bool ipv6 = std::get<1>(info.param);
            std::string suffix = ipv6 ? "TCPv6" : "TCPv4";
            switch (std::get<0>(info.param))
            {
                case TRANSPORT:
                default:
                    return "Transport" + suffix;
            }

        });
