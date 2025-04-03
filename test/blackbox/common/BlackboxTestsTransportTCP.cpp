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

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>

#include "../api/dds-pim/TCPReqRepHelloWorldRequester.hpp"
#include "../api/dds-pim/TCPReqRepHelloWorldReplier.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "DatagramInjectionTransport.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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
            test_transport_ = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
        }
        else
        {
            test_transport_ = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        }
    }

    void TearDown() override
    {
        use_ipv6 = false;
    }

    std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> test_transport_;
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

// Test == operator for TCPv4/v6
TEST_P(TransportTCP, TCP_equal_operator)
{
    if (use_ipv6)
    {
        // TCPv6TransportDescriptor
        TCPv6TransportDescriptor transport1;
        TCPv6TransportDescriptor transport2;
        // Compare equal in defult values
        ASSERT_EQ(transport1, transport2);

        // Modify some default values in 1
        transport1.enable_tcp_nodelay = !transport1.enable_tcp_nodelay; // change default value
        transport1.max_logical_port = transport1.max_logical_port + 10; // change default value
        transport1.add_listener_port(123u * 98u);
        ASSERT_FALSE(transport1 == transport2); // operator== != operator!=, using operator== == false instead

        // Modify some default values in 2
        transport2.enable_tcp_nodelay = !transport2.enable_tcp_nodelay; // change default value
        transport2.max_logical_port = transport2.max_logical_port + 10; // change default value
        transport2.add_listener_port(123u * 98u);
        ASSERT_EQ(transport1, transport2);
    }
    else
    {
        // TCPv4TransportDescriptor
        TCPv4TransportDescriptor transport1;
        TCPv4TransportDescriptor transport2;
        // Compare equal in defult values
        ASSERT_EQ(transport1, transport2);

        // Modify default values in 1
        transport1.set_WAN_address("80.80.99.45");
        ASSERT_FALSE(transport1 == transport2); // operator== != operator!=, using operator== == false instead

        // Modify default values in 2
        transport2.set_WAN_address("80.80.99.45");
        ASSERT_EQ(transport1, transport2);
    }
}

// Test copy constructor and copy assignment for TCPv4/v6
TEST_P(TransportTCP, TCP_copy)
{
    if (use_ipv6)
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
    else
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
}

// Test get_WAN_address member function
TEST(TransportTCP, TCPv4_get_WAN_address)
{
    // TCPv4TransportDescriptor
    eprosima::fastdds::rtps::TCPv4TransportDescriptor tcpv4_transport;
    tcpv4_transport.set_WAN_address("80.80.99.45");
    ASSERT_EQ(tcpv4_transport.get_WAN_address(), "80.80.99.45");
}

// Test connection is successfully restablished after dropping and relaunching a TCP client (requester)
// Issue -> https://github.com/eProsima/Fast-DDS/issues/2409
TEST_P(TransportTCP, Client_reconnection)
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

// Test zero listening port for TCPv4/v6
TEST_P(TransportTCP, TCP_autofill_port)
{
    PubSubReader<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);

    std::shared_ptr<TCPTransportDescriptor> p1_transport;
    std::shared_ptr<TCPTransportDescriptor> p2_transport;
    if (use_ipv6)
    {
        // TCPv6TransportDescriptor
        p1_transport = std::make_shared<TCPv6TransportDescriptor>();
        p2_transport = std::make_shared<TCPv6TransportDescriptor>();
    }
    else
    {
        // TCPv4TransportDescriptor
        p1_transport = std::make_shared<TCPv4TransportDescriptor>();
        p2_transport = std::make_shared<TCPv4TransportDescriptor>();
    }

    // Add TCP Transport with listening port 0
    p1_transport->add_listener_port(0);
    p1.disable_builtin_transport().add_user_transport_to_pparams(p1_transport);
    p1.init();
    ASSERT_TRUE(p1.isInitialized());

    // Add TCP Transport with listening port different from 0
    uint16_t port = 12345;
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
        writers[i]->reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
                .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
                .lease_duration(eprosima::fastdds::dds::c_TimeInfinite, eprosima::fastdds::dds::Duration_t(3, 0))
                .resource_limits_max_instances(1)
                .resource_limits_max_samples_per_instance(samples_per_participant);

        readers[i]->reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
                .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
                .lease_duration(eprosima::fastdds::dds::c_TimeInfinite, eprosima::fastdds::dds::Duration_t(3, 0))
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

// This test verifies that if having a server with several listening ports, only the first one is used.
TEST_P(TransportTCP, multiple_listening_ports)
{
    // Create a server with several listening ports
    PubSubReader<HelloWorldPubSubType>* server = new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME);
    uint16_t server_port_1 = 10000;
    uint16_t server_port_2 = 10001;

    test_transport_->add_listener_port(server_port_1);
    test_transport_->add_listener_port(server_port_2);
    server->disable_builtin_transport().add_user_transport_to_pparams(test_transport_).init();
    ASSERT_TRUE(server->isInitialized());

    // Create two clients each one connecting to a different port
    PubSubWriter<HelloWorldPubSubType>* client_1 = new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType>* client_2 = new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME);
    std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> client_transport_1;
    std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> client_transport_2;
    Locator_t initialPeerLocator_1;
    Locator_t initialPeerLocator_2;
    if (use_ipv6)
    {
        client_transport_1 = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
        client_transport_2 = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
        initialPeerLocator_1.kind = LOCATOR_KIND_TCPv6;
        initialPeerLocator_2.kind = LOCATOR_KIND_TCPv6;
        IPLocator::setIPv6(initialPeerLocator_1, "::1");
        IPLocator::setIPv6(initialPeerLocator_2, "::1");
    }
    else
    {
        client_transport_1 = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        client_transport_2 = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        initialPeerLocator_1.kind = LOCATOR_KIND_TCPv4;
        initialPeerLocator_2.kind = LOCATOR_KIND_TCPv4;
        IPLocator::setIPv4(initialPeerLocator_1, 127, 0, 0, 1);
        IPLocator::setIPv4(initialPeerLocator_2, 127, 0, 0, 1);
    }
    client_1->disable_builtin_transport().add_user_transport_to_pparams(client_transport_1);
    client_2->disable_builtin_transport().add_user_transport_to_pparams(client_transport_2);
    initialPeerLocator_1.port = server_port_1;
    initialPeerLocator_2.port = server_port_2;
    LocatorList_t initial_peer_list_1;
    LocatorList_t initial_peer_list_2;
    initial_peer_list_1.push_back(initialPeerLocator_1);
    initial_peer_list_2.push_back(initialPeerLocator_2);
    client_1->initial_peers(initial_peer_list_1);
    client_2->initial_peers(initial_peer_list_2);
    client_1->init();
    client_2->init();
    ASSERT_TRUE(client_1->isInitialized());
    ASSERT_TRUE(client_2->isInitialized());

    // Wait for discovery.
    server->wait_discovery();
    client_1->wait_discovery();
    client_2->wait_discovery(std::chrono::seconds(1));
    EXPECT_EQ(server->get_matched(), 1U);
    EXPECT_EQ(client_1->get_matched(), 1U);
    EXPECT_EQ(client_2->get_matched(), 0U);

    // Send data
    auto data = default_helloworld_data_generator();
    server->startReception(data);
    client_1->send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block server until reception finished.
    server->block_for_all();
    // Wait for all data to be acked.
    EXPECT_TRUE(client_1->waitForAllAcked(std::chrono::milliseconds(100)));

    // Release TCP client and server resources.
    delete client_1;
    delete client_2;
    delete server;
}

// Test TCP send resource cleaning. This test matches a server with a client and then releases the
// client resources. After PDP unbind message, the server removes the client
// from the send resource list.
TEST_P(TransportTCP, send_resource_cleanup)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);

    using eprosima::fastdds::rtps::DatagramInjectionTransportDescriptor;

    std::unique_ptr<PubSubWriter<HelloWorldPubSubType>> client(new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME));
    std::unique_ptr<PubSubWriter<HelloWorldPubSubType>> udp_participant(new PubSubWriter<HelloWorldPubSubType>(
                TEST_TOPIC_NAME));
    std::unique_ptr<PubSubReader<HelloWorldPubSubType>> server(new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME));

    // Server
    // Create a server with two transports, one of which uses a DatagramInjectionTransportDescriptor
    // which heritates from ChainingTransportDescriptor. The low level transport of this chaining transport will be UDP.
    // This will allow us to get send_resource_list_ from the server participant when UDP transport gets its OpenOutputChannel()
    // method called. This should happen after TCP transports connection is established. We can then see how many TCP send
    // resources exist.
    // For the cleanup test we follow that same procedure. Firstly we destroy both participants and then instantiate a new
    // UDP participant. The send resource list will get updated with no TCP send resource.
    //  __________________________________________________________              _____________________
    // |                   Server                                 |            |       Client        |
    // |                                                          |            |                     |
    // |    SendResourceList                                      |            |                     |
    // |          |                                               |            |                     |
    // |        Empty                                             |            |                     |
    // |          |                                               |            |                     |
    // |          |            - TCPv4 init()                     |            |                     |
    // |          |                                               |            |                     |
    // |          |            - ChainingTransport(UDP) init()    |            |                     |
    // |          |                                               |            |                     |
    // |        1 TCP            <------------------------------------------------- TCPv4 init()     |
    // |          |                                               |            |                     |
    // |    1 TCP + 1 UDP        <------------------------------------------------- UDPv4 init()     |
    // |          |                                               |            |                     |
    // |          |             - ChainingTransport->             |            |                     |
    // | TCP SendResources == 1        get_send_resource_list()   |            |                     |
    // |          |                                               |            |                     |
    // |        Empty           <-------------------------------------------------- clean transports |
    // |          |                                               |            |                     |
    // |        1 UDP           - ChainingTransport(UDP)  <------------------------ UDPv4 init()     |
    // |          |                                               |            |                     |
    // |          |             - ChainingTransport->             |            |                     |
    // | TCP SendResources == 0        get_send_resource_list()   |            |                     |
    // |__________________________________________________________|            |_____________________|
    //
    uint16_t server_port = 10000;
    test_transport_->add_listener_port(server_port);
    auto low_level_transport = std::make_shared<UDPv4TransportDescriptor>();
    auto server_chaining_transport = std::make_shared<DatagramInjectionTransportDescriptor>(low_level_transport);
    server->disable_builtin_transport().add_user_transport_to_pparams(test_transport_).add_user_transport_to_pparams(
        server_chaining_transport).init();
    ASSERT_TRUE(server->isInitialized());

    // Client
    auto initialize_client = [&](PubSubWriter<HelloWorldPubSubType>* client)
            {
                std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> client_transport;
                Locator_t initialPeerLocator;
                if (use_ipv6)
                {
                    client_transport = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
                    initialPeerLocator.kind = LOCATOR_KIND_TCPv6;
                    IPLocator::setIPv6(initialPeerLocator, "::1");
                }
                else
                {
                    client_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
                    initialPeerLocator.kind = LOCATOR_KIND_TCPv4;
                    IPLocator::setIPv4(initialPeerLocator, 127, 0, 0, 1);
                }
                client->disable_builtin_transport().add_user_transport_to_pparams(client_transport);
                initialPeerLocator.port = server_port;
                LocatorList_t initial_peer_list;
                initial_peer_list.push_back(initialPeerLocator);
                client->initial_peers(initial_peer_list);
                client->init();
            };
    auto initialize_udp_participant = [&](PubSubWriter<HelloWorldPubSubType>* udp_participant)
            {
                auto udp_participant_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
                udp_participant->disable_builtin_transport().add_user_transport_to_pparams(udp_participant_transport);
                udp_participant->init();
            };
    initialize_client(client.get());
    ASSERT_TRUE(client->isInitialized());

    // Wait for discovery. OpenOutputChannel() is called. We create a udp participant after to guarantee
    // that the TCP participants have been mutually discovered when OpenOutputChannel() is called.
    server->wait_discovery(std::chrono::seconds(0), 1);
    client->wait_discovery(1, std::chrono::seconds(0));

    initialize_udp_participant(udp_participant.get());
    ASSERT_TRUE(udp_participant->isInitialized());
    server->wait_discovery(std::chrono::seconds(0), 2);
    udp_participant->wait_discovery(1, std::chrono::seconds(0));

    // We can only update the senders when OpenOutputChannel() is called. If the send resource
    // is deleted later, senders obtained from get_send_resource_list() won't have changed.
    auto send_resource_list = server_chaining_transport->get_send_resource_list();
    auto tcp_send_resources = [](const std::set<SenderResource*>& send_resource_list) -> size_t
            {
                size_t tcp_send_resources = 0;
                for (auto& sender_resource : send_resource_list)
                {
                    if (sender_resource->kind() == LOCATOR_KIND_TCPv4 || sender_resource->kind() == LOCATOR_KIND_TCPv6)
                    {
                        tcp_send_resources++;
                    }
                }
                return tcp_send_resources;
            };
    EXPECT_EQ(tcp_send_resources(send_resource_list), 1);

    // Release TCP client resources.
    client.reset();
    udp_participant.reset();

    // Wait for undiscovery.
    server->wait_writer_undiscovery();

    // Create new udp client.
    udp_participant.reset(new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME));

    // Wait for discovery. OpenOutputChannel() is called and we can update the senders.
    initialize_udp_participant(udp_participant.get());
    ASSERT_TRUE(udp_participant->isInitialized());
    server->wait_discovery(std::chrono::seconds(0), 1);
    udp_participant->wait_discovery(1, std::chrono::seconds(0));

    // Check that the send_resource_list has size 0. This means that the send resource
    // for the  client has been removed.
    send_resource_list = server_chaining_transport->get_send_resource_list();
    EXPECT_EQ(tcp_send_resources(send_resource_list), 0);
    send_resource_list.clear();
}

// Test TCP send resource cleaning. In this case, since the send resource has been created from an initial_peer,
// the send resource should not be removed.
TEST_P(TransportTCP, send_resource_cleanup_initial_peer)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);

    using eprosima::fastdds::rtps::DatagramInjectionTransportDescriptor;

    std::unique_ptr<PubSubWriter<HelloWorldPubSubType>> client(new PubSubWriter<HelloWorldPubSubType>(TEST_TOPIC_NAME));
    std::unique_ptr<PubSubReader<HelloWorldPubSubType>> udp_participant(new PubSubReader<HelloWorldPubSubType>(
                TEST_TOPIC_NAME));
    std::unique_ptr<PubSubReader<HelloWorldPubSubType>> server(new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME));

    // Client
    // Create a client with two transports, one of which uses a DatagramInjectionTransportDescriptor
    // which heritates from ChainingTransportDescriptor. This will allow us to get send_resource_list_
    // from the client participant when its transport gets its OpenOutputChannel() method called.

    //  __________________________________________________________              _____________________
    // |                   Server                                 |            |       Client        |
    // |                                                          |            |                     |
    // |    SendResourceList                                      |            |                     |
    // |          |                                               |            |                     |
    // |        Empty                                             |            |                     |
    // |          |                                               |            |                     |
    // |          |            - TCPv4 init()                     |            |                     |
    // |          |                                               |            |                     |
    // |          |            - ChainingTransport(UDP) init()    |            |                     |
    // |          |                                               |            |                     |
    // |        1 TCP            <------------------------------------------------- TCPv4 init()     |
    // |          |                                               |            |                     |
    // |    1 TCP + 1 UDP        <------------------------------------------------- UDPv4 init()     |
    // |          |                                               |            |                     |
    // |          |             - ChainingTransport->             |            |                     |
    // | TCP SendResources == 1        get_send_resource_list()   |            |                     |
    // |          |                                               |            |                     |
    // |  1 TCP (initial peer)  <-------------------------------------------------- clean transports |
    // |          |                                               |            |                     |
    // |    1 TCP + 1 UDP       - ChainingTransport(UDP)  <------------------------ UDPv4 init()     |
    // |          |                                               |            |                     |
    // |          |             - ChainingTransport->             |            |                     |
    // | TCP SendResources == 1        get_send_resource_list()   |            |                     |
    // |     (initial peer)                                       |            |                     |
    // |__________________________________________________________|            |_____________________|
    //

    uint16_t server_port = 10000;
    LocatorList_t initial_peer_list;
    Locator_t initialPeerLocator;
    if (use_ipv6)
    {
        initialPeerLocator.kind = LOCATOR_KIND_TCPv6;
        IPLocator::setIPv6(initialPeerLocator, "::1");
    }
    else
    {
        initialPeerLocator.kind = LOCATOR_KIND_TCPv4;
        IPLocator::setIPv4(initialPeerLocator, 127, 0, 0, 1);
    }
    initialPeerLocator.port = server_port;
    initial_peer_list.push_back(initialPeerLocator);
    client->initial_peers(initial_peer_list);

    auto low_level_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    auto client_chaining_transport = std::make_shared<DatagramInjectionTransportDescriptor>(low_level_transport);
    client->disable_builtin_transport().add_user_transport_to_pparams(test_transport_).add_user_transport_to_pparams(
        client_chaining_transport).init();
    ASSERT_TRUE(client->isInitialized());

    // Server
    auto initialize_server = [&](PubSubReader<HelloWorldPubSubType>* server)
            {
                std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> server_transport;
                if (use_ipv6)
                {
                    server_transport = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
                }
                else
                {
                    server_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
                }
                server_transport->add_listener_port(server_port);
                server->disable_builtin_transport().add_user_transport_to_pparams(server_transport);
                server->init();
            };
    auto initialize_udp_participant = [&](PubSubReader<HelloWorldPubSubType>* udp_participant)
            {
                auto udp_participant_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
                udp_participant->disable_builtin_transport().add_user_transport_to_pparams(udp_participant_transport);
                udp_participant->init();
            };
    initialize_server(server.get());
    ASSERT_TRUE(server->isInitialized());

    // Wait for discovery. OpenOutputChannel() is called. We create a udp participant after to guarantee
    // that the TCP participants have been mutually discovered when OpenOutputChannel() is called.
    client->wait_discovery(1, std::chrono::seconds(0));
    server->wait_discovery(std::chrono::seconds(0), 1);

    initialize_udp_participant(udp_participant.get());
    ASSERT_TRUE(udp_participant->isInitialized());
    client->wait_discovery(2, std::chrono::seconds(0));
    udp_participant->wait_discovery(std::chrono::seconds(0), 1);

    // We can only update the senders when OpenOutputChannel() is called. If the send resource
    // is deleted later, senders obtained from get_send_resource_list() won't have changed.
    auto send_resource_list = client_chaining_transport->get_send_resource_list();
    auto tcp_send_resources = [](const std::set<SenderResource*>& send_resource_list) -> size_t
            {
                size_t tcp_send_resources = 0;
                for (auto& sender_resource : send_resource_list)
                {
                    if (sender_resource->kind() == LOCATOR_KIND_TCPv4 || sender_resource->kind() == LOCATOR_KIND_TCPv6)
                    {
                        tcp_send_resources++;
                    }
                }
                return tcp_send_resources;
            };
    EXPECT_EQ(tcp_send_resources(send_resource_list), 1);

    // Release TCP client resources.
    server.reset();
    udp_participant.reset();

    // Wait for undiscovery.
    client->wait_reader_undiscovery();

    // Create new client instances.
    udp_participant.reset(new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME));

    // Wait for discovery. OpenOutputChannel() is called and we can update the senders.
    initialize_udp_participant(udp_participant.get());
    ASSERT_TRUE(udp_participant->isInitialized());
    client->wait_discovery(1, std::chrono::seconds(0));
    udp_participant->wait_discovery(std::chrono::seconds(0), 1);

    // Check that the send_resource_list has size 1. This means that the send resource
    // for the first client hasn't been removed because it was created from an initial_peer.
    send_resource_list = client_chaining_transport->get_send_resource_list();
    EXPECT_EQ(tcp_send_resources(send_resource_list), 1);
    send_resource_list.clear();

    // If relaunching the server, the client should connect again.
    server.reset(new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME));
    initialize_server(server.get());
    ASSERT_TRUE(server->isInitialized());
    server->wait_discovery(std::chrono::seconds(0), 1);
    client->wait_discovery(2, std::chrono::seconds(0));
}

// Test TCP transport on large message with best effort reliability
TEST_P(TransportTCP, large_message_send_receive)
{
    // Prepare data to be sent before participants discovery so it is ready to be sent as soon as possible.
    std::list<Data1mb> data;
    data = default_data300kb_data_generator(1);

    uint16_t writer_port = global_port;

    /* Test configuration */
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> writer_transport;
    std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor> reader_transport;
    Locator_t initialPeerLocator;
    if (use_ipv6)
    {
        reader_transport = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
        writer_transport = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
        initialPeerLocator.kind = LOCATOR_KIND_TCPv6;
        IPLocator::setIPv6(initialPeerLocator, "::1");
    }
    else
    {
        reader_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        writer_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        initialPeerLocator.kind = LOCATOR_KIND_TCPv4;
        IPLocator::setIPv4(initialPeerLocator, 127, 0, 0, 1);
    }
    writer_transport->tcp_negotiation_timeout = 100;
    reader_transport->tcp_negotiation_timeout = 100;

    // Add listener port to server
    writer_transport->add_listener_port(writer_port);

    // Add initial peer to client
    initialPeerLocator.port = writer_port;
    LocatorList_t initial_peer_list;
    initial_peer_list.push_back(initialPeerLocator);

    // Setup participants
    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(writer_transport);

    reader.disable_builtin_transport()
            .initial_peers(initial_peer_list)
            .add_user_transport_to_pparams(reader_transport);

    // Init participants
    writer.init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery(1, std::chrono::seconds(0));
    reader.wait_discovery(std::chrono::seconds(0), 1);

    // Send and receive data
    reader.startReception(data);

    writer.send(data);
    EXPECT_TRUE(data.empty());

    reader.block_for_all();
}

// Test TCP transport on large message with best effort reliability and LARGE_DATA mode
TEST_P(TransportTCP, large_message_large_data_send_receive)
{
    // Prepare data to be sent. before participants discovery so it is ready to be sent as soon as possible.
    // The writer might try to send the data before the reader has negotiated the connection.
    // If the negotiation timeout is too short, the writer will fail to send the data and the reader will not receive it.
    // LARGE_DATA participant discovery is tipically faster than tcp negotiation.
    std::list<Data1mb> data;
    data = default_data300kb_data_generator(1);

    /* Test configuration */
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    BuiltinTransportsOptions options;
    options.tcp_negotiation_timeout = 100;
    writer.setup_large_data_tcp(use_ipv6, 0, options);
    reader.setup_large_data_tcp(use_ipv6, 0, options);

    // Init participants
    writer.init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery(1, std::chrono::seconds(0));
    reader.wait_discovery(std::chrono::seconds(0), 1);

    // Send and receive data
    reader.startReception(data);

    writer.send(data);
    EXPECT_TRUE(data.empty());

    reader.block_for_all();
}

// Test CreateInitialConnection for TCP
TEST_P(TransportTCP, TCP_initial_peers_connection)
{
    PubSubWriter<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p3(TEST_TOPIC_NAME);

    // Add TCP Transport with listening port
    std::shared_ptr<TCPTransportDescriptor> p1_transport;
    std::shared_ptr<TCPTransportDescriptor> p2_transport;
    std::shared_ptr<TCPTransportDescriptor> p3_transport;
    if (use_ipv6)
    {
        // TCPv6TransportDescriptor
        p1_transport = std::make_shared<TCPv6TransportDescriptor>();
        p2_transport = std::make_shared<TCPv6TransportDescriptor>();
        p3_transport = std::make_shared<TCPv6TransportDescriptor>();
    }
    else
    {
        // TCPv4TransportDescriptor
        p1_transport = std::make_shared<TCPv4TransportDescriptor>();
        p2_transport = std::make_shared<TCPv4TransportDescriptor>();
        p3_transport = std::make_shared<TCPv4TransportDescriptor>();
    }
    p1_transport->add_listener_port(global_port);
    p2_transport->add_listener_port(global_port + 1);
    p3_transport->add_listener_port(global_port - 1);

    // Add initial peer to clients
    Locator_t initialPeerLocator;
    initialPeerLocator.port = global_port;
    if (use_ipv6)
    {
        initialPeerLocator.kind = LOCATOR_KIND_TCPv6;
        IPLocator::setIPv6(initialPeerLocator, "::1");
    }
    else
    {
        initialPeerLocator.kind = LOCATOR_KIND_TCPv4;
        IPLocator::setIPv4(initialPeerLocator, 127, 0, 0, 1);
    }
    LocatorList_t initial_peer_list;
    initial_peer_list.push_back(initialPeerLocator);

    // Setup participants
    p1.disable_builtin_transport()
            .add_user_transport_to_pparams(p1_transport);

    p2.disable_builtin_transport()
            .initial_peers(initial_peer_list)
            .add_user_transport_to_pparams(p2_transport);

    p3.disable_builtin_transport()
            .initial_peers(initial_peer_list)
            .add_user_transport_to_pparams(p3_transport);

    // Init participants
    p1.init();
    p2.init();
    p3.init();
    ASSERT_TRUE(p1.isInitialized());
    ASSERT_TRUE(p2.isInitialized());
    ASSERT_TRUE(p3.isInitialized());

    // Wait for discovery
    p1.wait_discovery(2, std::chrono::seconds(0));
    p2.wait_discovery(std::chrono::seconds(0), 1);
    p3.wait_discovery(std::chrono::seconds(0), 1);

    // Send and receive data
    auto data = default_helloworld_data_generator();
    p2.startReception(data);
    p3.startReception(data);

    p1.send(data);
    EXPECT_TRUE(data.empty());

    p2.block_for_all();
    p3.block_for_all();
}

TEST_P(TransportTCP, tcp_unique_network_flows_init)
{
    // TCP Writer creation should fail as feature is not implemented for writers
    {
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
        PropertyPolicy properties;
        properties.properties().emplace_back("fastdds.unique_network_flows", "");

        test_transport_->add_listener_port(global_port);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);

        writer.entity_property_policy(properties).init();

        EXPECT_FALSE(writer.isInitialized());
    }

    // Two readers on the same participant not requesting unique flows should give the same logical port and same physical port
    {
        PubSubParticipant<HelloWorldPubSubType> participant(0, 2, 0, 0);

        participant.sub_topic_name(TEST_TOPIC_NAME);

        participant.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);

        ASSERT_TRUE(participant.init_participant());
        ASSERT_TRUE(participant.init_subscriber(0));
        ASSERT_TRUE(participant.init_subscriber(1));

        LocatorList_t locators;
        LocatorList_t locators2;

        participant.get_native_reader(0).get_listening_locators(locators);
        participant.get_native_reader(1).get_listening_locators(locators2);

        EXPECT_TRUE(locators == locators2);
        // LocatorList size depends on the number of interfaces. Different address but same port.
        ASSERT_GT(locators.size(), 0);
        ASSERT_GT(locators2.size(), 0);
        auto locator1 = locators.begin();
        auto locator2 = locators2.begin();
        EXPECT_EQ(IPLocator::getPhysicalPort(*locator1), IPLocator::getPhysicalPort(*locator2));
        EXPECT_EQ(IPLocator::getLogicalPort(*locator1), IPLocator::getLogicalPort(*locator2));
    }

    // Two TCP readers on the same participant requesting unique flows should give different logical ports but same physical port
    {
        PubSubParticipant<HelloWorldPubSubType> participant(0, 2, 0, 0);

        PropertyPolicy properties;
        properties.properties().emplace_back("fastdds.unique_network_flows", "");
        participant.sub_topic_name(TEST_TOPIC_NAME).sub_property_policy(properties);

        participant.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);

        ASSERT_TRUE(participant.init_participant());
        ASSERT_TRUE(participant.init_subscriber(0));
        ASSERT_TRUE(participant.init_subscriber(1));

        LocatorList_t locators;
        LocatorList_t locators2;

        participant.get_native_reader(0).get_listening_locators(locators);
        participant.get_native_reader(1).get_listening_locators(locators2);

        EXPECT_FALSE(locators == locators2);
        // LocatorList size depends on the number of interfaces. Different address but same port.
        ASSERT_GT(locators.size(), 0);
        ASSERT_GT(locators2.size(), 0);
        auto locator1 = locators.begin();
        auto locator2 = locators2.begin();
        EXPECT_EQ(IPLocator::getPhysicalPort(*locator1), IPLocator::getPhysicalPort(*locator2));
        EXPECT_NE(IPLocator::getLogicalPort(*locator1), IPLocator::getLogicalPort(*locator2));
    }
}

TEST_P(TransportTCP, tcp_unique_network_flows_communication)
{
    PubSubParticipant<HelloWorldPubSubType> readers(0, 2, 0, 2);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy properties;
    properties.properties().emplace_back("fastdds.unique_network_flows", "");
    readers.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);

    eprosima::fastdds::rtps::Locator_t initial_peer_locator;
    if (use_ipv6)
    {
        initial_peer_locator.kind = LOCATOR_KIND_TCPv6;
        eprosima::fastdds::rtps::IPLocator::setIPv6(initial_peer_locator, "::1");
    }
    else
    {
        initial_peer_locator.kind = LOCATOR_KIND_TCPv4;
        eprosima::fastdds::rtps::IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
    }
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(initial_peer_locator, global_port);
    eprosima::fastdds::rtps::LocatorList_t initial_peer_list;
    initial_peer_list.push_back(initial_peer_locator);

    readers.sub_topic_name(TEST_TOPIC_NAME)
            .sub_property_policy(properties)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .initial_peers(initial_peer_list);

    ASSERT_TRUE(readers.init_participant());
    ASSERT_TRUE(readers.init_subscriber(0));
    ASSERT_TRUE(readers.init_subscriber(1));

    test_transport_->add_listener_port(global_port);
    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport_)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_depth(100);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    readers.sub_wait_discovery();

    // Send data
    auto data = default_helloworld_data_generator();
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block until readers have acknowledged all samples.
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(30)));
}

/**
 * This verifies that a best effort reader is capable of creating resources when a new locator
 * is received along a Data(W) in order to start communication. This will ensure the creation a new connect channel.
 * The reader must have the lowest listening port to force the participant to create the channel.
 */
TEST_P(TransportTCP, best_effort_reader_tcp_resources_creation)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Large data setup is reused to enable UDP for multicast and TCP for data.
    // However, the metatraffic unicast needs to be replaced for UDP to ensure that the TCP
    // locator is not announced in the Data(P) (In large data the metatraffic unicast is TCP).
    LocatorList metatraffic_unicast;
    eprosima::fastdds::rtps::Locator_t udp_locator;
    udp_locator.kind = LOCATOR_KIND_UDPv4;
    eprosima::fastdds::rtps::IPLocator::setIPv4(udp_locator, "127.0.0.1");
    metatraffic_unicast.push_back(udp_locator);

    // Writer with highest listening port will wait for connection
    writer.setup_large_data_tcp(use_ipv6, global_port + 1)
            .metatraffic_unicast_locator_list(metatraffic_unicast)
            .init();

    // Reader with lowest listening port to force the connection channel creation
    reader.setup_large_data_tcp(use_ipv6, global_port)
            .reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS)
            .metatraffic_unicast_locator_list(metatraffic_unicast)
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery(std::chrono::seconds(5));
    reader.wait_discovery(std::chrono::seconds(5));

    ASSERT_EQ(writer.get_matched(), 1u);
    ASSERT_EQ(reader.get_matched(), 1u);

    // Although participants have matched, the TCP connection might not be established yet.
    // This active wait ensures the connection had time to be established before sending non-reliable samples.
    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto data = default_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());

    reader.block_for_all();
}

TEST_P(TransportTCP, large_data_tcp_no_frag)
{
    /* Test configuration */
    PubSubWriter<Data100kbPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<Data100kbPubSubType> reader(TEST_TOPIC_NAME);

    // Reliable keep all to wait of all acked as end condition
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);

    // Builtin transport configuration according to test_case
    BuiltinTransportsOptions options;
    options.maxMessageSize = 200000;
    options.sockets_buffer_size = 200000;
    writer.setup_large_data_tcp(use_ipv6, 0, options);
    reader.setup_large_data_tcp(use_ipv6, 0, options);

    /* Run test */
    // Init writer
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Init reader
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    // Send data
    auto data = default_data100kb_data_generator();
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());

    // Wait for reception acknowledgement
    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(3)));
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
