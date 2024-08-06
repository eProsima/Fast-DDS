// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <limits>
#include <memory>
#include <thread>

#include <asio.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <rtps/network/NetworkFactory.hpp>
#include <rtps/transport/TCPv6Transport.h>
#include <utils/Semaphore.hpp>

#include "mock/MockTCPv6Transport.h"
#include <MockReceiverResource.h>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds;

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

static uint16_t g_default_port = 0;

uint16_t get_port()
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if (4000 > port)
    {
        port += 4000;
    }

    return port;
}

class TCPv6Tests : public ::testing::Test
{
public:

    void SetUp() override
    {
#ifdef __APPLE__
        // TODO: fix IPv6 issues related with zone ID
        GTEST_SKIP() << "TCPv6 tests are disabled in Mac";
#endif // ifdef __APPLE__
    }

    TCPv6Tests()
    {
        HELPER_SetDescriptorDefaults();
    }

    ~TCPv6Tests()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

    void HELPER_SetDescriptorDefaults();

    TCPv6TransportDescriptor descriptor;
    std::unique_ptr<std::thread> senderThread;
    std::unique_ptr<std::thread> receiverThread;
};

TEST_F(TCPv6Tests, wrong_configuration_values)
{
    // Too big sendBufferSize
    {
        auto wrong_descriptor = descriptor;
        wrong_descriptor.sendBufferSize = std::numeric_limits<uint32_t>::max();
        TCPv6Transport transportUnderTest(wrong_descriptor);
        ASSERT_FALSE(transportUnderTest.init());
        eprosima::fastdds::dds::Log::Flush();
    }

    // Too big receiveBufferSize
    {
        auto wrong_descriptor = descriptor;
        wrong_descriptor.receiveBufferSize = std::numeric_limits<uint32_t>::max();
        TCPv6Transport transportUnderTest(wrong_descriptor);
        ASSERT_FALSE(transportUnderTest.init());
        eprosima::fastdds::dds::Log::Flush();
    }

    // Too big maxMessageSize
    {
        auto wrong_descriptor = descriptor;
        wrong_descriptor.maxMessageSize = std::numeric_limits<uint32_t>::max();
        TCPv6Transport transportUnderTest(wrong_descriptor);
        ASSERT_FALSE(transportUnderTest.init());
        eprosima::fastdds::dds::Log::Flush();
    }

    // maxMessageSize bigger than receiveBufferSize
    {
        auto wrong_descriptor = descriptor;
        wrong_descriptor.maxMessageSize = 10;
        wrong_descriptor.receiveBufferSize = 5;
        TCPv6Transport transportUnderTest(wrong_descriptor);
        ASSERT_FALSE(transportUnderTest.init());
        eprosima::fastdds::dds::Log::Flush();
    }

    // maxMessageSize bigger than sendBufferSize
    {
        auto wrong_descriptor = descriptor;
        wrong_descriptor.maxMessageSize = 10;
        wrong_descriptor.sendBufferSize = 5;
        TCPv6Transport transportUnderTest(wrong_descriptor);
        ASSERT_FALSE(transportUnderTest.init());
        eprosima::fastdds::dds::Log::Flush();
    }

    // Buffer sizes automatically decrease
    {
        auto wrong_descriptor = descriptor;
        wrong_descriptor.sendBufferSize = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
        wrong_descriptor.receiveBufferSize = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
        wrong_descriptor.maxMessageSize = 1470;
        TCPv6Transport transportUnderTest(wrong_descriptor);
        ASSERT_TRUE(transportUnderTest.init());
        auto* final_cfg = transportUnderTest.configuration();
        EXPECT_GE(final_cfg->sendBufferSize, final_cfg->maxMessageSize);
        // The system could allow for the send buffer to be MAX_INT, so we cannot check it to be strictly lower
        EXPECT_LE(final_cfg->sendBufferSize, wrong_descriptor.sendBufferSize);
        EXPECT_GE(final_cfg->receiveBufferSize, final_cfg->maxMessageSize);
        // The system could allow for the receive buffer to be MAX_INT, so we cannot check it to be strictly lower
        EXPECT_LE(final_cfg->receiveBufferSize, wrong_descriptor.receiveBufferSize);
        eprosima::fastdds::dds::Log::Flush();
    }
}

TEST_F(TCPv6Tests, conversion_to_ip6_string)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_TCPv6;
    ASSERT_EQ("::", IPLocator::toIPv6string(locator));

    locator.address[0] = 0xff;
    ASSERT_EQ("ff00::", IPLocator::toIPv6string(locator));

    locator.address[1] = 0xaa;
    ASSERT_EQ("ffaa::", IPLocator::toIPv6string(locator));

    locator.address[2] = 0x0a;
    ASSERT_EQ("ffaa:a00::", IPLocator::toIPv6string(locator));

    locator.address[5] = 0x0c;
    ASSERT_EQ("ffaa:a00:c::", IPLocator::toIPv6string(locator));
}

TEST_F(TCPv6Tests, setting_ip6_values_on_locators)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_TCPv6;

    IPLocator::setIPv6(locator, 0xffff, 0xa, 0xaba, 0, 0, 0, 0, 0);
    ASSERT_EQ("ffff:a:aba::", IPLocator::toIPv6string(locator));
}

TEST_F(TCPv6Tests, locators_with_kind_2_supported)
{
    // Given
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t supportedLocator;
    supportedLocator.kind = LOCATOR_KIND_TCPv6;
    Locator_t unsupportedLocator;
    unsupportedLocator.kind = LOCATOR_KIND_UDPv4;

    // Then
    ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocator));
}

TEST_F(TCPv6Tests, opening_and_closing_output_channel)
{
    // Given
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_TCPv6;
    genericOutputChannelLocator.port = g_default_port; // arbitrary
    IPLocator::setLogicalPort(genericOutputChannelLocator, g_default_port);
    IPLocator::setIPv6(genericOutputChannelLocator, "::1");

    // Then
    /*
       ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
       ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator));
       ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
       ASSERT_TRUE  (transportUnderTest.SenderResourceHasBeenClosed(genericOutputChannelLocator));
       ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
       ASSERT_FALSE (transportUnderTest.SenderResourceHasBeenClosed(genericOutputChannelLocator));
     */
}

#ifndef __APPLE__
TEST_F(TCPv6Tests, opening_and_closing_input_channel)
{
    // Given
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastFilterLocator;
    multicastFilterLocator.kind = LOCATOR_KIND_TCPv6;
    multicastFilterLocator.port = g_default_port; // arbitrary
    IPLocator::setIPv6(multicastFilterLocator, 0xff31, 0, 0, 0, 0, 0, 0x8000, 0x1234);

    RTPSParticipantAttributes p_attr{};
    NetworkFactory factory{p_attr};
    factory.RegisterTransport<TCPv6Transport, TCPv6TransportDescriptor>(descriptor);
    std::vector<std::shared_ptr<ReceiverResource>> receivers;
    factory.BuildReceiverResources(multicastFilterLocator, receivers, 0x8FFF);
    ReceiverResource* receiver = receivers.back().get();

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(multicastFilterLocator, receiver, 0x8FFF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(multicastFilterLocator));
}

// This test verifies that the autofill port feature correctly sets an automatic port when
// the descriptors's port is set to 0.
TEST_F(TCPv6Tests, autofill_port)
{
    // Check normal port assignation
    TCPv6TransportDescriptor test_descriptor;
    test_descriptor.add_listener_port(g_default_port);
    TCPv6Transport transportUnderTest(test_descriptor);
    transportUnderTest.init();

    EXPECT_TRUE(transportUnderTest.configuration()->listening_ports[0] == g_default_port);

    // Check default port assignation
    TCPv6TransportDescriptor test_descriptor_autofill;
    test_descriptor_autofill.add_listener_port(0);
    TCPv6Transport transportUnderTest_autofill(test_descriptor_autofill);
    transportUnderTest_autofill.init();

    EXPECT_TRUE(transportUnderTest_autofill.configuration()->listening_ports[0] != 0);
    EXPECT_TRUE(transportUnderTest_autofill.configuration()->listening_ports.size() == 1u);
}

static void GetIP6s(
        std::vector<IPFinder::info_IP>& interfaces)
{
    IPFinder::getIPs(&interfaces, false);
    auto new_end = remove_if(interfaces.begin(),
                    interfaces.end(),
                    [](IPFinder::info_IP ip)
                    {
                        return ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL;
                    });
    interfaces.erase(new_end, interfaces.end());
    std::for_each(interfaces.begin(), interfaces.end(), [](IPFinder::info_IP& loc)
            {
                loc.locator.kind = LOCATOR_KIND_TCPv6;
            });
}

TEST_F(TCPv6Tests, check_TCPv6_interface_whitelist_initialization)
{
    std::vector<IPFinder::info_IP> interfaces;

    GetIP6s(interfaces);

    // asio::ip::addres_v6 appends the interface name to the IP address, but the locator does not
    // Create two different vectors to compare them
    std::vector<std::string> asio_interfaces;
    std::vector<std::string> locator_interfaces;
    for (auto& ip : interfaces)
    {
        asio_interfaces.push_back(ip.name);
        locator_interfaces.push_back(IPLocator::toIPv6string(ip.locator));
    }
    // Add manually localhost to test adding multiple interfaces
    asio_interfaces.push_back("::1");
    locator_interfaces.push_back("::1");

    for (auto& ip : locator_interfaces)
    {
        descriptor.interfaceWhiteList.emplace_back(ip);
    }
    descriptor.add_listener_port(g_default_port);
    MockTCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    // Check that the transport whitelist and the acceptors map is the same size as the locator_interfaces
    ASSERT_EQ(transportUnderTest.get_interface_whitelist().size(), descriptor.interfaceWhiteList.size());
    ASSERT_EQ(transportUnderTest.get_acceptors_map().size(), descriptor.interfaceWhiteList.size());

    // Check that every interface is in the whitelist
    auto check_whitelist = transportUnderTest.get_interface_whitelist();
    for (auto& ip : asio_interfaces)
    {
        ASSERT_NE(std::find(check_whitelist.begin(), check_whitelist.end(), asio::ip::address_v6::from_string(
                    ip)), check_whitelist.end());
    }

    // Check that every interface is in the acceptors map
    for (const auto& test : transportUnderTest.get_acceptors_map())
    {
        ASSERT_NE(std::find(locator_interfaces.begin(), locator_interfaces.end(), IPLocator::toIPv6string(
                    test.first)), locator_interfaces.end());
    }
}

// This test verifies server's channel resources mapping keys uniqueness, where keys are clients locators.
// Clients typically communicated its PID as its locator port. When having several clients in the same
// process this lead to overwriting server's channel resources map elements.
TEST_F(TCPv6Tests, client_announced_local_port_uniqueness)
{
    TCPv6TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    MockTCPv6Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv6TransportDescriptor sendDescriptor_1;
    TCPv6Transport sendTransportUnderTest_1(sendDescriptor_1);
    sendTransportUnderTest_1.init();

    TCPv6TransportDescriptor sendDescriptor_2;
    TCPv6Transport sendTransportUnderTest_2(sendDescriptor_2);
    sendTransportUnderTest_2.init();

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(outputLocator, "::1");
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7610);

    SendResourceList send_resource_list_1;
    ASSERT_TRUE(sendTransportUnderTest_1.OpenOutputChannel(send_resource_list_1, outputLocator));
    ASSERT_FALSE(send_resource_list_1.empty());

    SendResourceList send_resource_list_2;
    ASSERT_TRUE(sendTransportUnderTest_2.OpenOutputChannel(send_resource_list_2, outputLocator));
    ASSERT_FALSE(send_resource_list_2.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(receiveTransportUnderTest.get_channel_resources().size(), 2u);
}

#ifndef _WIN32
// The primary purpose of this test is to check the non-blocking behavior of a socket sending data to a
// destination that does not read or does it so slowly.
TEST_F(TCPv6Tests, non_blocking_send)
{
    uint16_t port = g_default_port;
    uint32_t msg_size = 64ul * 1024ul;
    // Create a TCP Server transport
    TCPv6TransportDescriptor senderDescriptor;
    senderDescriptor.add_listener_port(port);
    senderDescriptor.non_blocking_send = true;
    senderDescriptor.sendBufferSize = msg_size;
    MockTCPv6Transport senderTransportUnderTest(senderDescriptor);
    senderTransportUnderTest.init();

    // Create a TCP Client socket.
    // The creation of a reception transport for testing this functionality is not
    // feasible. For the saturation of the sending socket, it's necessary first to
    // saturate the reception socket of the datareader. This saturation requires
    // preventing the datareader from reading from the socket, what inevitably
    // happens continuously if instantiating and connecting the receiver transport.
    // Hence, a raw socket is opened and connected to the server. Read calls on that
    // socket are controlled.
    Locator_t serverLoc;
    serverLoc.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(serverLoc, "::1");
    serverLoc.port = g_default_port;
    IPLocator::setLogicalPort(serverLoc, 7610);

    // TCPChannelResourceBasic::connect() like connection
    asio::io_service io_service;
    asio::ip::tcp::resolver resolver(io_service);
    auto endpoints = resolver.resolve(
        IPLocator::ip_to_string(serverLoc),
        std::to_string(IPLocator::getPhysicalPort(serverLoc)));

    asio::ip::tcp::socket socket = asio::ip::tcp::socket (io_service);
    asio::async_connect(
        socket,
        endpoints,
        [](std::error_code ec
#if ASIO_VERSION >= 101200
        , asio::ip::tcp::endpoint
#else
        , asio::ip::tcp::resolver::iterator
#endif // if ASIO_VERSION >= 101200
        )
        {
            ASSERT_TRUE(!ec);
        }
        );

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /*
       Get server's accepted channel. This is retrieved from the unbound_channel_resources_,
       which is a vector where client channels are pushed immediately after the server accepts
       a connection. This channel will not be present in the server's channel_resources_ map
       as communication lacks most of the discovery messages using a raw socket as participant.
     */
    auto sender_unbound_channel_resources = senderTransportUnderTest.get_unbound_channel_resources();
    ASSERT_TRUE(sender_unbound_channel_resources.size() == 1u);
    auto sender_channel_resource =
            std::static_pointer_cast<TCPChannelResourceBasic>(sender_unbound_channel_resources[0]);

    // Prepare the message
    asio::error_code ec;
    std::vector<octet> message(msg_size * 2, 0);
    const octet* data = message.data();
    size_t size = message.size();
    NetworkBuffer buffers(data, size);
    std::vector<NetworkBuffer> buffer_list;
    buffer_list.push_back(buffers);

    // Send the message with no header. Since TCP actually allocates twice the size of the buffer requested
    // it should be able to send a message of msg_size*2.
    size_t bytes_sent = sender_channel_resource->send(nullptr, 0, buffer_list, size, ec);
    ASSERT_EQ(bytes_sent, size);

    // Now wait until the receive buffer is flushed (send buffer will be empty too)
    std::vector<octet> buffer(size, 0);
    size_t bytes_read = asio::read(socket, asio::buffer(buffer, size), asio::transfer_exactly(size), ec);
    ASSERT_EQ(bytes_read, size);

    // Now try to send a message that is bigger than the buffer size: (msg_size*2 + 1) + bytes_in_send_buffer(0) > 2*sendBufferSize
    message.resize(msg_size * 2 + 1);
    data = message.data();
    size = message.size();
    buffer_list.clear();
    buffer_list.emplace_back(data, size);
    bytes_sent = sender_channel_resource->send(nullptr, 0, buffer_list, size, ec);
    ASSERT_EQ(bytes_sent, 0u);

    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.cancel();
    socket.close();
}
#endif // ifndef _WIN32

// This test verifies that a server can reconnect to a client after the client has once failed in a
// openLogicalPort request
TEST_F(TCPv6Tests, reconnect_after_open_port_failure)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);
    uint16_t port = g_default_port;
    // Create a TCP Server transport
    TCPv6TransportDescriptor serverDescriptor;
    serverDescriptor.add_listener_port(port);
    std::unique_ptr<TCPv6Transport> serverTransportUnderTest(new TCPv6Transport(serverDescriptor));
    serverTransportUnderTest->init();

    // Create a TCP Client transport
    TCPv6TransportDescriptor clientDescriptor;
    std::unique_ptr<MockTCPv6Transport> clientTransportUnderTest(new MockTCPv6Transport(clientDescriptor));
    clientTransportUnderTest->init();

    // Add initial peer to the client
    Locator_t initialPeerLocator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", port, initialPeerLocator);
    IPLocator::setLogicalPort(initialPeerLocator, 7410);

    // Connect client to server
    EXPECT_TRUE(serverTransportUnderTest->OpenInputChannel(initialPeerLocator, nullptr, 0x00FF));
    SendResourceList client_resource_list;
    ASSERT_TRUE(clientTransportUnderTest->OpenOutputChannel(client_resource_list, initialPeerLocator));
    ASSERT_FALSE(client_resource_list.empty());
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto channel = clientTransportUnderTest->get_channel_resources().begin()->second;

    // Logical port is opened
    ASSERT_TRUE(channel->is_logical_port_opened(7410));

    // Disconnect server
    EXPECT_TRUE(serverTransportUnderTest->CloseInputChannel(initialPeerLocator));
    serverTransportUnderTest.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // Client should have passed logical port to pending list
    ASSERT_FALSE(channel->is_logical_port_opened(7410));
    ASSERT_TRUE(channel->is_logical_port_added(7410));

    // Now try normal reconnection
    serverTransportUnderTest.reset(new TCPv6Transport(serverDescriptor));
    serverTransportUnderTest->init();
    ASSERT_TRUE(serverTransportUnderTest->OpenInputChannel(initialPeerLocator, nullptr, 0x00FF));
    clientTransportUnderTest->send(nullptr, 0, channel->locator(), initialPeerLocator); // connect()

    // Logical port is opened (moved from pending list)
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    ASSERT_TRUE(channel->is_logical_port_opened(7410));

    // Disconnect server
    EXPECT_TRUE(serverTransportUnderTest->CloseInputChannel(initialPeerLocator));
    serverTransportUnderTest.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // Client should have passed logical port to pending list
    ASSERT_FALSE(channel->is_logical_port_opened(7410));
    ASSERT_TRUE(channel->is_logical_port_added(7410));

    // Now try reconnect the server and close server's input channel before client's open logical
    // port request, and then delete server and reconnect
    serverTransportUnderTest.reset(new TCPv6Transport(serverDescriptor));
    serverTransportUnderTest->init();
    ASSERT_TRUE(serverTransportUnderTest->OpenInputChannel(initialPeerLocator, nullptr, 0x00FF));
    EXPECT_TRUE(serverTransportUnderTest->CloseInputChannel(initialPeerLocator));
    clientTransportUnderTest->send(nullptr, 0, channel->locator(), initialPeerLocator); // connect()
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    serverTransportUnderTest.reset();
    ASSERT_FALSE(channel->is_logical_port_opened(7410));
    ASSERT_TRUE(channel->is_logical_port_added(7410));

    // Now try normal reconnection
    serverTransportUnderTest.reset(new TCPv6Transport(serverDescriptor));
    serverTransportUnderTest->init();
    ASSERT_TRUE(serverTransportUnderTest->OpenInputChannel(initialPeerLocator, nullptr, 0x00FF));
    clientTransportUnderTest->send(nullptr, 0, channel->locator(), initialPeerLocator); // connect()

    // Logical port is opened (moved from pending list)
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    ASSERT_TRUE(channel->is_logical_port_opened(7410));

    // Clear test
    EXPECT_TRUE(serverTransportUnderTest->CloseInputChannel(initialPeerLocator));
    client_resource_list.clear();
}

TEST_F(TCPv6Tests, opening_output_channel_with_same_locator_as_local_listening_port)
{
    descriptor.add_listener_port(g_default_port);
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    // Two locators with the same port as the local listening port, but different addresses
    Locator_t lowerOutputChannelLocator;
    lowerOutputChannelLocator.kind = LOCATOR_KIND_TCPv6;
    lowerOutputChannelLocator.port = g_default_port;
    IPLocator::setLogicalPort(lowerOutputChannelLocator, g_default_port);
    Locator_t higherOutputChannelLocator = lowerOutputChannelLocator;
    IPLocator::setIPv6(lowerOutputChannelLocator, "::");
    IPLocator::setIPv6(higherOutputChannelLocator, "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

    SendResourceList send_resource_list;

    // If the remote address is lower than the local one, no channel must be created but it must be added to the send_resource_list
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, lowerOutputChannelLocator));
    ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(lowerOutputChannelLocator));
    ASSERT_EQ(send_resource_list.size(), 1);
    // If the remote address is higher than the local one, a CONNECT channel must be created and added to the send_resource_list
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, higherOutputChannelLocator));
    ASSERT_TRUE(transportUnderTest.is_output_channel_open_for(higherOutputChannelLocator));
    ASSERT_EQ(send_resource_list.size(), 2u);
}

// This test verifies that the send resource list is correctly cleaned and the channel resource is removed
// from the channel_resources_map.
TEST_F(TCPv6Tests, remove_from_send_resource_list)
{
    TCPv6TransportDescriptor send_descriptor;
    MockTCPv6Transport send_transport_under_test(send_descriptor);
    send_transport_under_test.init();

    Locator_t output_locator_1;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", g_default_port, output_locator_1);
    IPLocator::setLogicalPort(output_locator_1, 7410);

    Locator_t output_locator_2;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", g_default_port + 1, output_locator_2);
    IPLocator::setLogicalPort(output_locator_2, 7410);

    LocatorList_t initial_peer_list;
    initial_peer_list.push_back(output_locator_2);

    SendResourceList send_resource_list;
    ASSERT_TRUE(send_transport_under_test.OpenOutputChannel(send_resource_list, output_locator_1));
    ASSERT_TRUE(send_transport_under_test.OpenOutputChannel(send_resource_list, output_locator_2));
    ASSERT_EQ(send_resource_list.size(), 2u);

    // Using a wrong locator should not remove the channel resource
    LocatorList_t wrong_remote_participant_physical_locators;
    Locator_t wrong_output_locator;
    IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", g_default_port + 2, wrong_output_locator);
    IPLocator::setLogicalPort(wrong_output_locator, 7410);
    wrong_remote_participant_physical_locators.push_back(wrong_output_locator);
    send_transport_under_test.cleanup_sender_resources(
        send_resource_list,
        wrong_remote_participant_physical_locators,
        initial_peer_list);
    ASSERT_EQ(send_resource_list.size(), 2u);

    // Using the correct locator should remove the channel resource
    LocatorList_t remote_participant_physical_locators;
    remote_participant_physical_locators.push_back(output_locator_1);
    send_transport_under_test.cleanup_sender_resources(
        send_resource_list,
        remote_participant_physical_locators,
        initial_peer_list);
    ASSERT_EQ(send_resource_list.size(), 1u);

    // Using the initial peer locator should not remove the channel resource
    remote_participant_physical_locators.clear();
    remote_participant_physical_locators.push_back(output_locator_2);
    send_transport_under_test.cleanup_sender_resources(
        send_resource_list,
        remote_participant_physical_locators,
        initial_peer_list);
    ASSERT_EQ(send_resource_list.size(), 1u);
}

// This test verifies the logical port passed to OpenOutputChannel is correctly added to the channel pending list or the
// trasnport's pending channel logical ports map.
TEST_F(TCPv6Tests, add_logical_port_on_send_resource_creation)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);

    // TCP Client
    {
        uint16_t port = 12345;
        TCPv6TransportDescriptor clientDescriptor;
        std::unique_ptr<MockTCPv6Transport> clientTransportUnderTest(new MockTCPv6Transport(clientDescriptor));
        clientTransportUnderTest->init();

        // Add initial peer to the client
        Locator_t initialPeerLocator;
        IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", port, initialPeerLocator);
        IPLocator::setLogicalPort(initialPeerLocator, 7410);

        // OpenOutputChannel
        SendResourceList client_resource_list;
        ASSERT_TRUE(clientTransportUnderTest->OpenOutputChannel(client_resource_list, initialPeerLocator));
        IPLocator::setLogicalPort(initialPeerLocator, 7411);
        ASSERT_TRUE(clientTransportUnderTest->OpenOutputChannel(client_resource_list, initialPeerLocator));
        ASSERT_FALSE(client_resource_list.empty());
        auto channel = clientTransportUnderTest->get_channel_resources().begin()->second;
        ASSERT_TRUE(channel->is_logical_port_added(7410));
        ASSERT_TRUE(channel->is_logical_port_added(7411));
        auto channel_pending_logical_ports = clientTransportUnderTest->get_channel_pending_logical_ports();
        ASSERT_TRUE(channel_pending_logical_ports.empty());

        client_resource_list.clear();
    }

    // TCP Server - LARGE_DATA
    {
        uint16_t port = 12345;
        // Discovered participant physical port has to have a lower value than the listening port to behave as a server
        uint16_t participantPhysicalLocator = 12344;
        // Create a TCP Server transport
        TCPv6TransportDescriptor serverDescriptor;
        serverDescriptor.add_listener_port(port);
        std::unique_ptr<MockTCPv6Transport> serverTransportUnderTest(new MockTCPv6Transport(serverDescriptor));
        serverTransportUnderTest->init();

        // Add participant discovered (from UDP discovery for example)
        Locator_t discoveredParticipantLocator;
        IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", participantPhysicalLocator, discoveredParticipantLocator);
        IPLocator::setLogicalPort(discoveredParticipantLocator, 7410);

        // OpenOutputChannel
        SendResourceList server_resource_list;
        ASSERT_TRUE(serverTransportUnderTest->OpenOutputChannel(server_resource_list, discoveredParticipantLocator));
        IPLocator::setLogicalPort(discoveredParticipantLocator, 7411);
        ASSERT_TRUE(serverTransportUnderTest->OpenOutputChannel(server_resource_list, discoveredParticipantLocator));
        ASSERT_FALSE(server_resource_list.empty());
        ASSERT_TRUE(serverTransportUnderTest->get_channel_resources().empty());
        auto channel_pending_logical_ports = serverTransportUnderTest->get_channel_pending_logical_ports();
        ASSERT_EQ(channel_pending_logical_ports.size(), 1);
        ASSERT_EQ(channel_pending_logical_ports.begin()->second.size(), 2);
        ASSERT_TRUE(channel_pending_logical_ports.begin()->second.find(
                    7410) != channel_pending_logical_ports.begin()->second.end());
        ASSERT_TRUE(channel_pending_logical_ports.begin()->second.find(
                    7411) != channel_pending_logical_ports.begin()->second.end());

        server_resource_list.clear();
    }

    // TCP Client - LARGE_DATA
    {
        uint16_t port = 12345;
        // Discovered participant physical port has to have a larger value than the listening port to behave as a client
        uint16_t participantPhysicalLocator = 12346;
        // Create a TCP Client transport
        TCPv6TransportDescriptor clientDescriptor;
        clientDescriptor.add_listener_port(port);
        std::unique_ptr<MockTCPv6Transport> clientTransportUnderTest(new MockTCPv6Transport(clientDescriptor));
        clientTransportUnderTest->init();

        // Add participant discovered (from UDP discovery for example)
        Locator_t discoveredParticipantLocator;
        IPLocator::createLocator(LOCATOR_KIND_TCPv6, "::1", participantPhysicalLocator, discoveredParticipantLocator);
        IPLocator::setLogicalPort(discoveredParticipantLocator, 7410);

        // OpenOutputChannel
        SendResourceList client_resource_list;
        ASSERT_TRUE(clientTransportUnderTest->OpenOutputChannel(client_resource_list, discoveredParticipantLocator));
        IPLocator::setLogicalPort(discoveredParticipantLocator, 7411);
        ASSERT_TRUE(clientTransportUnderTest->OpenOutputChannel(client_resource_list, discoveredParticipantLocator));
        ASSERT_FALSE(client_resource_list.empty());
        auto channel = clientTransportUnderTest->get_channel_resources().begin()->second;
        ASSERT_TRUE(channel->is_logical_port_added(7410));
        ASSERT_TRUE(channel->is_logical_port_added(7411));
        auto channel_pending_logical_ports = clientTransportUnderTest->get_channel_pending_logical_ports();
        ASSERT_TRUE(channel_pending_logical_ports.empty());

        client_resource_list.clear();
    }
}

// TODO: TEST_F(TCPv6Tests, send_and_receive_between_both_secure_ports)
// TODO: TEST_F(TCPv6Tests, send_and_receive_between_ports)

#endif // ifndef __APPLE__

void TCPv6Tests::HELPER_SetDescriptorDefaults()
{
}

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
