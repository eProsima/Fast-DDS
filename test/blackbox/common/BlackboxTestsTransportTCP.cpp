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

#include "TCPReqRepHelloWorldRequester.hpp"
#include "TCPReqRepHelloWorldReplier.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <chrono>
#include <thread>
#include <random>

#include <gtest/gtest.h>

#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>

#include "DatagramInjectionTransport.hpp"

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

// Test zero listening port for TCPv4
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

// Test zero listening port for TCPv6
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

    std::shared_ptr<TCPTransportDescriptor> writer_transport;
    std::shared_ptr<TCPTransportDescriptor> reader_transport;
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

    uint32_t tcp_negotiation_timeout = 100;
    writer.setup_large_data_tcp(use_ipv6, 0, tcp_negotiation_timeout);
    reader.setup_large_data_tcp(use_ipv6, 0, tcp_negotiation_timeout);

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

// Test TCP send resource cleaning. This test matches a server with a client and then releases the
// client resources. After PDP unbind message, the server removes the client
// from the send resource list.
TEST_P(TransportTCP, send_resource_cleanup)
{

#if defined(__APPLE__)
    if (use_ipv6)
    {
        GTEST_SKIP() << "macOS TCPv6 transport skipped";
        return;
    }
#endif // if defined(__APPLE__)

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
                std::shared_ptr<TCPTransportDescriptor> client_transport;
                Locator_t initialPeerLocator;
                if (use_ipv6)
                {
                    client_transport = std::make_shared<TCPv6TransportDescriptor>();
                    initialPeerLocator.kind = LOCATOR_KIND_TCPv6;
                    IPLocator::setIPv6(initialPeerLocator, "::1");
                }
                else
                {
                    client_transport = std::make_shared<TCPv4TransportDescriptor>();
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
                auto udp_participant_transport = std::make_shared<UDPv4TransportDescriptor>();
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
#if defined(__APPLE__)
    if (use_ipv6)
    {
        GTEST_SKIP() << "macOS TCPv6 transport skipped";
        return;
    }
#endif // if defined(__APPLE__)

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

    auto low_level_transport = std::make_shared<UDPv4TransportDescriptor>();
    auto client_chaining_transport = std::make_shared<DatagramInjectionTransportDescriptor>(low_level_transport);
    client->disable_builtin_transport().add_user_transport_to_pparams(test_transport_).add_user_transport_to_pparams(
        client_chaining_transport).init();
    ASSERT_TRUE(client->isInitialized());

    // Server
    auto initialize_server = [&](PubSubReader<HelloWorldPubSubType>* server)
            {
                std::shared_ptr<TCPTransportDescriptor> server_transport;
                if (use_ipv6)
                {
                    server_transport = std::make_shared<TCPv6TransportDescriptor>();
                }
                else
                {
                    server_transport = std::make_shared<TCPv4TransportDescriptor>();
                }
                server_transport->add_listener_port(server_port);
                server->disable_builtin_transport().add_user_transport_to_pparams(server_transport);
                server->init();
            };
    auto initialize_udp_participant = [&](PubSubReader<HelloWorldPubSubType>* udp_participant)
            {
                auto udp_participant_transport = std::make_shared<UDPv4TransportDescriptor>();
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

// Test CreateInitialConnection for TCP
TEST_P(TransportTCP, TCP_initial_peers_connection)
{
    PubSubWriter<HelloWorldPubSubType> p1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p2(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> p3(TEST_TOPIC_NAME);

    // Add TCP Transport with listening port
    auto p1_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    p1_transport->add_listener_port(global_port);
    auto p2_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    p2_transport->add_listener_port(global_port + 1);
    auto p3_transport = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    p3_transport->add_listener_port(global_port - 1);

    // Add initial peer to client
    Locator_t initialPeerLocator;
    initialPeerLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(initialPeerLocator, 127, 0, 0, 1);
    initialPeerLocator.port = global_port;
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
