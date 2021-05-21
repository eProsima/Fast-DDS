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

enum communication_type
{
    TRANSPORT
};

class TransportUDP : public testing::TestWithParam<std::tuple<communication_type, bool>>
{
public:

    void SetUp() override
    {
        test_transport_.reset();
        use_udpv4 = std::get<1>(GetParam());
        if (use_udpv4)
        {
            test_transport_ = std::make_shared<UDPv4TransportDescriptor>();
        }
        else
        {
            test_transport_ = std::make_shared<UDPv6TransportDescriptor>();
        }
    }

    void TearDown() override
    {
        use_udpv4 = true;
    }

    void get_ip_address(
            LocatorList_t* loc)
    {
        if (use_udpv4)
        {
            eprosima::fastrtps::rtps::IPFinder::getIP4Address(loc);
        }
        else
        {
            eprosima::fastrtps::rtps::IPFinder::getIP6Address(loc);
        }
    }

    std::shared_ptr<UDPTransportDescriptor> test_transport_;
    std::string ip0;
    std::string ip1;
    std::string ip2;
};

TEST_P(TransportUDP, UDPTransportWrongConfigMaxMessageSize)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    test_transport_->maxMessageSize = 100000;

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport_).init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST_P(TransportUDP, UDPTransportWrongConfigSendBufferSize)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    test_transport_->sendBufferSize = 64000;

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport_).init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST_P(TransportUDP, UDPTransportWrongConfigReceiveBufferSize)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    test_transport_->receiveBufferSize = 64000;

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport_).init();

    ASSERT_FALSE(writer.isInitialized());
}

// TODO - GASCO: UDPMaxInitialPeer tests should use static discovery through initial peers.
TEST_P(TransportUDP, UDPMaxInitialPeer_P0_4_P3)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

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

TEST_P(TransportUDP, UDPMaxInitialPeer_P0_4_P4)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

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

TEST_P(TransportUDP, UDPMaxInitialPeer_P5_4_P4)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    reader.participant_id(5).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.max_initial_peers_range(4).participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(reader.is_matched());
}

TEST_P(TransportUDP, UDPMaxInitialPeer_P5_6_P4)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastrtps::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

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
TEST_P(TransportUDP, MulticastCommunicationBadReader)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    ip0 = use_udpv4 ? "127.0.0.1" : "::1";
    ip1 = use_udpv4 ? "239.255.1.4" : "ff1e::ffff:efff:104";
    ip2 = use_udpv4 ? "239.255.1.5" : "ff1e::ffff:efff:105";

    test_transport_->interfaceWhiteList.push_back(ip0);

    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    writer.add_to_metatraffic_multicast_locator_list(ip2, global_port);
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> readerMultiBad(TEST_TOPIC_NAME);
    readerMultiBad.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
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
TEST_P(TransportUDP, MulticastCommunicationOkReader)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    ip0 = use_udpv4 ? "127.0.0.1" : "::1";
    ip2 = use_udpv4 ? "239.255.1.5" : "ff1e::ffff:efff:105";

    // TODO(jlbueno) When announcing from localhost to multicast the RTPS packets are being sent (wireshark captures
    // them) but the packets are not received in the remote participant.
    // Using any other interface different from localhost, the test passes.
    // Disabling multicast and setting initial peers, the test also passes.
    if (use_udpv4)
    {
        test_transport_->interfaceWhiteList.push_back(ip0);

        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.add_to_metatraffic_multicast_locator_list(ip2, global_port);
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        PubSubReader<HelloWorldType> readerMultiOk(TEST_TOPIC_NAME);
        readerMultiOk.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        readerMultiOk.add_to_metatraffic_multicast_locator_list(ip2, global_port);
        readerMultiOk.init();

        ASSERT_TRUE(readerMultiOk.isInitialized());

        writer.wait_discovery();
        readerMultiOk.wait_discovery();
        ASSERT_TRUE(writer.is_matched());
        ASSERT_TRUE(readerMultiOk.is_matched());
    }
}

// #4420 Using whitelists in localhost sometimes UDP doesn't receive the release input channel message.
TEST_P(TransportUDP, whitelisting_udp_localhost_multi)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    ip0 = use_udpv4 ? "127.0.0.1" : "::1";

    // TODO(jlbueno) When announcing from localhost to multicast the RTPS packets are being sent (wireshark captures
    // them) but the packets are not received in the remote participant.
    // Using any other interface different from localhost, the test passes.
    // Disabling multicast and setting initial peers, the test also passes.
    if (use_udpv4)
    {
        test_transport_->interfaceWhiteList.push_back(ip0);

        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        for (int i = 0; i < 200; ++i)
        {
            PubSubReader<HelloWorldType> readerMultiOk(TEST_TOPIC_NAME);
            readerMultiOk.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
            readerMultiOk.init();

            ASSERT_TRUE(readerMultiOk.isInitialized());

            writer.wait_discovery();
            readerMultiOk.wait_discovery();
            ASSERT_TRUE(writer.is_matched());
            ASSERT_TRUE(readerMultiOk.is_matched());
        }
    }
}

// Checking correct copying of participant user data locators to the writers/readers
TEST_P(TransportUDP, DefaultMulticastLocatorsParticipant)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.0.1" : "ff1e::ffff:efff:1";
    if (!use_udpv4)
    {
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    writer.add_to_default_multicast_locator_list(ip1, 22222);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    reader.add_to_default_multicast_locator_list(ip1, 22222);
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
TEST_P(TransportUDP, MetatrafficMulticastLocatorsParticipant)
{
    Log::SetVerbosity(Log::Kind::Warning);

    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.1.1" : "ff1e::ffff:efff:101";

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    writer.add_to_metatraffic_multicast_locator_list(ip1, 22222);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    reader.add_to_metatraffic_multicast_locator_list(ip1, 22222);
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
TEST_P(TransportUDP, DefaultMulticastLocatorsParticipantZeroPort)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.0.1" : "ff1e::ffff:efff:1";
    if (!use_udpv4)
    {
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    writer.add_to_default_multicast_locator_list(ip1, 0);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    reader.add_to_default_multicast_locator_list(ip1, 0);
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
TEST_P(TransportUDP, MetatrafficMulticastLocatorsParticipantZeroPort)
{
    Log::SetVerbosity(Log::Kind::Warning);

    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.1.1" : "ff1e::ffff:efff:101";

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    writer.add_to_metatraffic_multicast_locator_list(ip1, 0);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    reader.add_to_metatraffic_multicast_locator_list(ip1, 0);
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
TEST_P(TransportUDP, whitelisting_udp_localhost_alone)
{
    ip0 = use_udpv4 ? "127.0.0.1" : "::1";

    test_transport_->interfaceWhiteList.push_back(ip0);

    for (int i = 0; i < 200; ++i)
    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
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

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(TransportUDP,
        TransportUDP,
        testing::Combine(testing::Values(TRANSPORT), testing::Values(false, true)),
        [](const testing::TestParamInfo<TransportUDP::ParamType>& info)
        {
            bool udpv4 = std::get<1>(info.param);
            std::string suffix = udpv4 ? "UDPv4" : "UDPv6";
            switch (std::get<0>(info.param))
            {
                case TRANSPORT:
                default:
                    return "Transport" + suffix;
            }

        });

