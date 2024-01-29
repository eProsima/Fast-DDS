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

#include <memory>
#include <thread>

#include <asio.hpp>
#include <gtest/gtest.h>
#include <MockReceiverResource.h>
#include "mock/MockTCPChannelResource.h"
#include "mock/MockTCPv4Transport.h"
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>
#include <rtps/transport/TCPv4Transport.h>
#include <rtps/transport/tcp/RTCPHeader.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using TCPv4Transport = eprosima::fastdds::rtps::TCPv4Transport;
using TCPHeader = eprosima::fastdds::rtps::TCPHeader;

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

static uint16_t g_default_port = 0;
static uint16_t g_output_port = 0;
static uint16_t g_input_port = 0;
static std::string g_test_wan_address = "88.88.88.88";

uint16_t get_port(
        uint16_t offset)
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if (offset > port)
    {
        port += offset;
    }

    return port;
}

class TCPv4Tests : public ::testing::Test
{
public:

    TCPv4Tests()
    {
        eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
        HELPER_SetDescriptorDefaults();
    }

    ~TCPv4Tests()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

    void HELPER_SetDescriptorDefaults();

    TCPv4TransportDescriptor descriptor;
    TCPv4TransportDescriptor descriptorOnlyOutput;
    std::unique_ptr<std::thread> senderThread;
    std::unique_ptr<std::thread> receiverThread;
};

TEST_F(TCPv4Tests, locators_with_kind_1_supported)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t supportedLocator;
    supportedLocator.kind = LOCATOR_KIND_TCPv4;
    Locator_t unsupportedLocatorv4;
    unsupportedLocatorv4.kind = LOCATOR_KIND_UDPv4;
    Locator_t unsupportedLocatorv6;
    unsupportedLocatorv6.kind = LOCATOR_KIND_UDPv6;

    // Then
    ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorv4));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorv6));
}

TEST_F(TCPv4Tests, opening_and_closing_output_channel)
{
    // Given
    TCPv4Transport transportUnderTest(descriptorOnlyOutput);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericOutputChannelLocator.port = g_output_port; // arbitrary
    IPLocator::setLogicalPort(genericOutputChannelLocator, g_output_port);
    SendResourceList send_resource_list;

    // Then
    ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, genericOutputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
    send_resource_list.clear();
    //ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
}

// This test checks that opening a listening port, never bound by an input channel,
// is correctly closed without valgrind errors. It should show a warning message
// in the log about called on deleted.
TEST_F(TCPv4Tests, opening_and_closing_output_channel_with_listener)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericOutputChannelLocator.port = g_output_port; // arbitrary
    IPLocator::setLogicalPort(genericOutputChannelLocator, g_output_port);
    SendResourceList send_resource_list;

    // Then
    ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, genericOutputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
    send_resource_list.clear();
    //ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
}

TEST_F(TCPv4Tests, opening_and_closing_input_channel)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericInputChannelLocator;
    genericInputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericInputChannelLocator.port = g_input_port; // listen port
    IPLocator::setIPv4(genericInputChannelLocator, 127, 0, 0, 1);

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(genericInputChannelLocator, nullptr, 0x00FF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
}

#ifndef __APPLE__
TEST_F(TCPv4Tests, send_and_receive_between_ports)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
    std::regex filter("RTCP(?!_SEQ)");
    eprosima::fastdds::dds::Log::SetCategoryFilter(filter);
    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
    MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
    ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

    SendResourceList send_resource_list;
    ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
    ASSERT_FALSE(send_resource_list.empty());
    octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

    Semaphore sem;
    std::function<void()> recCallback = [&]()
            {
                EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                sem.post();
            };

    msg_recv->setCallback(recCallback);

    auto sendThreadFunction = [&]()
            {
                bool sent = false;
                while (!sent)
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                EXPECT_TRUE(sent);
            };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
}
#endif // ifndef __APPLE__

TEST_F(TCPv4Tests, send_is_rejected_if_buffer_size_is_bigger_to_size_specified_in_descriptor)
{
    // Given
    TCPv4Transport transportUnderTest(descriptorOnlyOutput);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericOutputChannelLocator.port = g_output_port;
    IPLocator::setLogicalPort(genericOutputChannelLocator, 7400);
    SendResourceList send_resource_list;
    transportUnderTest.OpenOutputChannel(send_resource_list, genericOutputChannelLocator);
    ASSERT_FALSE(send_resource_list.empty());

    Locator_t destinationLocator;
    destinationLocator.kind = LOCATOR_KIND_TCPv4;
    destinationLocator.port = g_output_port + 1;
    IPLocator::setLogicalPort(destinationLocator, 7400);

    LocatorList_t locator_list;
    locator_list.push_back(destinationLocator);
    Locators destination_begin(locator_list.begin());
    Locators destination_end(locator_list.end());

    // Then
    std::vector<octet> receiveBufferWrongSize(descriptor.sendBufferSize + 1);
    ASSERT_FALSE(send_resource_list.at(0)->send(receiveBufferWrongSize.data(), (uint32_t)receiveBufferWrongSize.size(),
            &destination_begin, &destination_end, (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
}

TEST_F(TCPv4Tests, RemoteToMainLocal_simply_strips_out_address_leaving_IP_ANY)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_TCPv4;
    remote_locator.port = g_default_port;
    IPLocator::setIPv4(remote_locator, 222, 222, 222, 222);

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remote_locator);

    ASSERT_EQ(mainLocalLocator.port, remote_locator.port);
    ASSERT_EQ(mainLocalLocator.kind, remote_locator.kind);
    ASSERT_EQ(IPLocator::toIPv4string(mainLocalLocator), s_IPv4AddressAny);
}

TEST_F(TCPv4Tests, match_if_port_AND_address_matches)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t locatorAlpha;
    locatorAlpha.port = g_default_port;
    IPLocator::setIPv4(locatorAlpha, 239, 255, 0, 1);
    Locator_t locatorBeta = locatorAlpha;

    // Then
    ASSERT_TRUE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));

    IPLocator::setIPv4(locatorBeta, 100, 100, 100, 100);
    // Then
    ASSERT_TRUE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));
}

TEST_F(TCPv4Tests, send_to_wrong_interface)
{
    TCPv4Transport transportUnderTest(descriptorOnlyOutput);
    transportUnderTest.init();

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_output_port;
    outputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setLogicalPort(outputChannelLocator, 7400);
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1); // Loopback
    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());

    //Sending through a different IP will NOT work, except 0.0.0.0
    Locator_t wrongLocator(outputChannelLocator);
    IPLocator::setIPv4(wrongLocator, 111, 111, 111, 111);

    LocatorList_t locator_list;
    locator_list.push_back(wrongLocator);
    Locators wrong_begin(locator_list.begin());
    Locators wrong_end(locator_list.end());

    std::vector<octet> message = { 'H', 'e', 'l', 'l', 'o' };
    ASSERT_FALSE(send_resource_list.at(0)->send(message.data(), (uint32_t)message.size(), &wrong_begin, &wrong_end,
            (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
}

TEST_F(TCPv4Tests, send_to_blocked_interface)
{
    descriptor.interfaceWhiteList.emplace_back("111.111.111.111");
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_output_port;
    outputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setLogicalPort(outputChannelLocator, 7400);
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1); // Loopback
    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());

    //Sending through a different IP will NOT work, except 0.0.0.0
    Locator_t wrongLocator(outputChannelLocator);
    IPLocator::setIPv4(wrongLocator, 111, 111
            , 111, 111);

    LocatorList_t locator_list;
    locator_list.push_back(wrongLocator);
    Locators wrong_begin(locator_list.begin());
    Locators wrong_end(locator_list.end());

    std::vector<octet> message = { 'H', 'e', 'l', 'l', 'o' };
    ASSERT_FALSE(send_resource_list.at(0)->send(message.data(), (uint32_t)message.size(), &wrong_begin, &wrong_end,
            (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
}

#ifndef __APPLE__
TEST_F(TCPv4Tests, send_and_receive_between_allowed_interfaces_ports)
{
    LocatorList_t interfaces;
    if (IPFinder::getAllIPAddress(&interfaces))
    {
        Locator_t locator;
        for (auto& tmpLocator : interfaces)
        {
            if (tmpLocator.kind == LOCATOR_KIND_UDPv4 && IPLocator::toIPv4string(tmpLocator) != "127.0.0.1")
            {
                locator = tmpLocator;
                break;
            }
        }

        if (IsAddressDefined(locator))
        {
            eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
            std::regex filter("RTCP(?!_SEQ)");
            eprosima::fastdds::dds::Log::SetCategoryFilter(filter);
            TCPv4TransportDescriptor recvDescriptor;
            recvDescriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv4string(locator));
            recvDescriptor.add_listener_port(g_default_port);
            TCPv4Transport receiveTransportUnderTest(recvDescriptor);
            receiveTransportUnderTest.init();

            TCPv4TransportDescriptor sendDescriptor;
            sendDescriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv4string(locator));
            TCPv4Transport sendTransportUnderTest(sendDescriptor);
            sendTransportUnderTest.init();

            Locator_t inputLocator;
            inputLocator.kind = LOCATOR_KIND_TCPv4;
            inputLocator.port = g_default_port;
            inputLocator.set_address(locator);
            IPLocator::setLogicalPort(inputLocator, 7410);

            LocatorList_t locator_list;
            locator_list.push_back(inputLocator);

            Locator_t outputLocator;
            outputLocator.kind = LOCATOR_KIND_TCPv4;
            outputLocator.set_address(locator);
            outputLocator.port = g_default_port;
            IPLocator::setLogicalPort(outputLocator, 7410);

            {
                MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
                MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
                ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

                SendResourceList send_resource_list;
                ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
                ASSERT_FALSE(send_resource_list.empty());
                octet message[5] = { 'H', 'e', 'l', 'l', 'o' };
                bool bOk = false;
                std::function<void()> recCallback = [&]()
                        {
                            EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                            bOk = true;
                        };

                msg_recv->setCallback(recCallback);

                bool bFinish(false);
                auto sendThreadFunction = [&]()
                        {
                            Locators input_begin(locator_list.begin());
                            Locators input_end(locator_list.end());

                            bool sent =
                                    send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                            (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                            while (!bFinish && !sent)
                            {
                                Locators input_begin2(locator_list.begin());
                                Locators input_end2(locator_list.end());

                                sent =
                                        send_resource_list.at(0)->send(message, 5, &input_begin2, &input_end2,
                                                (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                            EXPECT_TRUE(sent);
                            //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                        };

                senderThread.reset(new std::thread(sendThreadFunction));
                std::this_thread::sleep_for(std::chrono::seconds(10));
                bFinish = true;
                senderThread->join();
                ASSERT_TRUE(bOk);
            }
        }
    }
}

#if TLS_FOUND
TEST_F(TCPv4Tests, send_and_receive_between_secure_ports_client_verifies)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.password = "fastddspwd";
    recvDescriptor.tls_config.cert_chain_file = "fastdds.crt";
    recvDescriptor.tls_config.private_key_file = "fastdds.key";
    recvDescriptor.tls_config.tmp_dh_file = "dh_params.pem";
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    recvDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.apply_security = true;
    sendDescriptor.tls_config.verify_file = "ca.crt";
    sendDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    sendDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
                {
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    sem.post();
                };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    while (!sent)
                    {
                        Locators l_input_begin(locator_list.begin());
                        Locators l_input_end(locator_list.end());

                        sent =
                                send_resource_list.at(0)->send(message, 5, &l_input_begin, &l_input_end,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    EXPECT_TRUE(sent);
                    //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
}

TEST_F(TCPv4Tests, send_and_receive_between_secure_ports_server_verifies)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.handshake_role = TLSHSRole::CLIENT;
    recvDescriptor.tls_config.verify_file = "ca.crt";
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    recvDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.apply_security = true;
    sendDescriptor.tls_config.handshake_role = TLSHSRole::SERVER;
    sendDescriptor.tls_config.password = "fastddspwd";
    sendDescriptor.tls_config.cert_chain_file = "fastdds.crt";
    sendDescriptor.tls_config.private_key_file = "fastdds.key";
    sendDescriptor.tls_config.tmp_dh_file = "dh_params.pem";
    sendDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER | TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT;
    sendDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    sendDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
                {
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    sem.post();
                };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    while (!sent)
                    {
                        Locators l_input_begin(locator_list.begin());
                        Locators l_input_end(locator_list.end());

                        sent =
                                send_resource_list.at(0)->send(message, 5,  &l_input_begin, &l_input_end,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    EXPECT_TRUE(sent);
                    //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
}

TEST_F(TCPv4Tests, send_and_receive_between_both_secure_ports)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.password = "testkey";
    recvDescriptor.tls_config.cert_chain_file = "mainpubcert.pem";
    recvDescriptor.tls_config.private_key_file = "mainpubkey.pem";
    recvDescriptor.tls_config.verify_file = "maincacert.pem";
    // Server doesn't accept clients without certs
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER | TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    recvDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV3);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.apply_security = true;
    sendDescriptor.tls_config.password = "testkey";
    sendDescriptor.tls_config.cert_chain_file = "mainsubcert.pem";
    sendDescriptor.tls_config.private_key_file = "mainsubkey.pem";
    sendDescriptor.tls_config.verify_file = "maincacert.pem";
    sendDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    sendDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    sendDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV3);
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
                {
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    sem.post();
                };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    while (!sent)
                    {
                        Locators l_input_begin(locator_list.begin());
                        Locators l_input_end(locator_list.end());

                        sent =
                                send_resource_list.at(0)->send(message, 5, &l_input_begin, &l_input_end,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    EXPECT_TRUE(sent);
                    //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
}

TEST_F(TCPv4Tests, send_and_receive_between_both_secure_ports_untrusted)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.password = "testkey";
    recvDescriptor.tls_config.cert_chain_file = "mainpubcert.pem";
    recvDescriptor.tls_config.private_key_file = "mainpubkey.pem";
    recvDescriptor.tls_config.verify_file = "ca.pem"; // This CA doesn't know about these certificates
    // Server doesn't accept clients without certs
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    recvDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV3);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.apply_security = true;
    sendDescriptor.tls_config.password = "testkey";
    sendDescriptor.tls_config.cert_chain_file = "mainsubcert.pem";
    sendDescriptor.tls_config.private_key_file = "mainsubkey.pem";
    sendDescriptor.tls_config.verify_file = "ca.pem"; // This CA doesn't know about these certificates
    sendDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    sendDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    sendDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV3);
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
                {
                    ASSERT_TRUE(false);
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    sem.post();
                };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    int count = 0;
                    while (!sent && count < 30)
                    {
                        Locators l_input_begin(locator_list.begin());
                        Locators l_input_end(locator_list.end());

                        sent =
                                send_resource_list.at(0)->send(message, 5, &l_input_begin, &l_input_end,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        ++count;
                    }
                    EXPECT_FALSE(sent);
                    sem.post();
                    //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
}

TEST_F(TCPv4Tests, send_and_receive_between_secure_clients_1)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.handshake_role = TLSHSRole::CLIENT;
    //recvDescriptor.tls_config.password = "testkey";
    //recvDescriptor.tls_config.password = "test";
    //recvDescriptor.tls_config.cert_chain_file = "mainpubcert.pem";
    //recvDescriptor.tls_config.private_key_file = "mainpubkey.pem";
    recvDescriptor.tls_config.verify_file = "maincacert.pem"; // This CA only know about mainsub certificates
    //recvDescriptor.tls_config.verify_file = "ca.pem";
    // Server doesn't accept clients without certs
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.apply_security = true;
    sendDescriptor.tls_config.handshake_role = TLSHSRole::SERVER;
    sendDescriptor.tls_config.password = "testkey";
    sendDescriptor.tls_config.cert_chain_file = "mainsubcert.pem";
    sendDescriptor.tls_config.private_key_file = "mainsubkey.pem";
    //sendDescriptor.tls_config.password = "test";
    //sendDescriptor.tls_config.cert_chain_file = "server.pem";
    //sendDescriptor.tls_config.private_key_file = "server.pem";
    //sendDescriptor.tls_config.verify_file = "maincacert.pem";
    sendDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    sendDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
                {
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    sem.post();
                };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    while (!sent)
                    {
                        Locators l_input_begin(locator_list.begin());
                        Locators l_input_end(locator_list.end());

                        sent =
                                send_resource_list.at(0)->send(message, 5, &l_input_begin, &l_input_end,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    EXPECT_TRUE(sent);
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
}
/*
   TEST_F(TCPv4Tests, send_and_receive_between_secure_clients_2)
   {
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port + 1);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.handshake_role = TLSHSRole::CLIENT;
    //recvDescriptor.tls_config.password = "testkey";
    //recvDescriptor.tls_config.password = "test";
    //recvDescriptor.tls_config.cert_chain_file = "mainpubcert.pem";
    //recvDescriptor.tls_config.private_key_file = "mainpubkey.pem";
    recvDescriptor.tls_config.verify_file = "maincacert.pem"; // This CA only know about mainsub certificates
    //recvDescriptor.tls_config.verify_file = "ca.pem";
    // Server doesn't accept clients without certs
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT | TLSVerifyMode::VERIFY_PEER;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port + 1;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port + 1;
    IPLocator::setLogicalPort(outputLocator, 7410);

    TCPv4TransportDescriptor sendDescriptor2;
    sendDescriptor2.apply_security = true;
    sendDescriptor2.tls_config.handshake_role = TLSHSRole::SERVER;
    sendDescriptor2.tls_config.password = "test";
    sendDescriptor2.tls_config.cert_chain_file = "server.pem";
    sendDescriptor2.tls_config.private_key_file = "server.pem";
    //sendDescriptor2.tls_config.password = "testkey";
    //sendDescriptor2.tls_config.cert_chain_file = "mainsubcert.pem";
    //sendDescriptor2.tls_config.private_key_file = "mainsubkey.pem";
    sendDescriptor2.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    sendDescriptor2.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    TCPv4Transport sendTransportUnderTest2(sendDescriptor2);
    sendTransportUnderTest2.init();

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        ASSERT_TRUE(sendTransportUnderTest2.OpenOutputChannel(outputLocator));
        octet message[5] = { 'H','e','l','l','o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
        {
            EXPECT_FALSE(true); // Should not receive
            sem.post();
        };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
        {
            bool sent = sendTransportUnderTest2.send(message, 5, outputLocator, inputLocator);
            int count = 0;
            while (!sent && count < 30)
            {
                sent = sendTransportUnderTest2.send(message, 5, outputLocator, inputLocator);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
 ++count;
            }
            EXPECT_FALSE(sent);
            sem.post();
        };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
    ASSERT_TRUE(sendTransportUnderTest2.CloseOutputChannel(outputLocator));
   }
 */

TEST_F(TCPv4Tests, send_and_receive_between_secure_ports_untrusted_server)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;

    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    recvDescriptor.apply_security = true;
    recvDescriptor.tls_config.password = "testkey";
    recvDescriptor.tls_config.cert_chain_file = "mainpubcert.pem";
    recvDescriptor.tls_config.private_key_file = "mainpubkey.pem";
    // Server doesn't accept clients without certs
    recvDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    recvDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    recvDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    recvDescriptor.tls_config.add_option(TLSOptions::NO_SSLV3);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.apply_security = true;
    sendDescriptor.tls_config.verify_file = "ca.pem"; // This CA doesn't know about these certificates
    sendDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    sendDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    sendDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    sendDescriptor.tls_config.add_option(TLSOptions::NO_SSLV3);
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
                {
                    ASSERT_TRUE(false);
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    sem.post();
                };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    int count = 0;
                    while (!sent && count < 30)
                    {
                        Locators l_input_begin(locator_list.begin());
                        Locators l_input_end(locator_list.end());
                        sent =
                                send_resource_list.at(0)->send(message, 5, &l_input_begin, &l_input_end,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        ++count;
                    }
                    EXPECT_FALSE(sent);
                    sem.post();
                    //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        senderThread->join();
        sem.wait();
    }
}

#ifndef _WIN32
// The primary purpose of this test is to check the non-blocking behavior of a secure socket sending data to a
// destination that does not read or does it so slowly.
TEST_F(TCPv4Tests, secure_non_blocking_send)
{
    uint16_t port = g_default_port;
    uint32_t msg_size = eprosima::fastdds::rtps::s_minimumSocketBuffer;
    // Create a TCP Server transport
    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;
    TCPv4TransportDescriptor senderDescriptor;
    senderDescriptor.add_listener_port(port);
    senderDescriptor.sendBufferSize = msg_size;
    senderDescriptor.tls_config.handshake_role = TLSHSRole::CLIENT;
    senderDescriptor.tls_config.verify_file = "ca.crt";
    senderDescriptor.tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
    senderDescriptor.tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    senderDescriptor.tls_config.add_option(TLSOptions::SINGLE_DH_USE);
    senderDescriptor.tls_config.add_option(TLSOptions::NO_SSLV2);
    senderDescriptor.tls_config.add_option(TLSOptions::NO_COMPRESSION);
    MockTCPv4Transport senderTransportUnderTest(senderDescriptor);
    eprosima::fastrtps::rtps::RTPSParticipantAttributes att;
    att.properties.properties().emplace_back("fastdds.tcp_transport.non_blocking_send", "true");
    senderTransportUnderTest.init(&att.properties);

    // Create a TCP Client socket.
    // The creation of a reception transport for testing this functionality is not
    // feasible. For the saturation of the sending socket, it's necessary first to
    // saturate the reception socket of the datareader. This saturation requires
    // preventing the datareader from reading from the socket, what inevitably
    // happens continuously if instantiating and connecting the receiver transport.
    // Hence, a raw socket is opened and connected to the server. There won't be read
    // calls on that socket.
    Locator_t serverLoc;
    serverLoc.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(serverLoc, 127, 0, 0, 1);
    serverLoc.port = port;
    IPLocator::setLogicalPort(serverLoc, 7410);

    // Socket TLS config
    asio::ssl::context ssl_context(asio::ssl::context::sslv23);
    ssl_context.set_verify_callback([](bool preverified, asio::ssl::verify_context&)
            {
                return preverified;
            });
    ssl_context.set_password_callback([](std::size_t, asio::ssl::context_base::password_purpose)
            {
                return "fastddspwd";
            });
    ssl_context.use_certificate_chain_file("fastdds.crt");
    ssl_context.use_private_key_file("fastdds.key", asio::ssl::context::pem);
    ssl_context.use_tmp_dh_file("dh_params.pem");

    uint32_t options = 0;
    options |= asio::ssl::context::default_workarounds;
    options |= asio::ssl::context::single_dh_use;
    options |= asio::ssl::context::no_sslv2;
    options |= asio::ssl::context::no_compression;
    ssl_context.set_options(options);

    // TCPChannelResourceSecure::connect() like connection
    asio::io_service io_service;
    asio::ip::tcp::resolver resolver(io_service);
    auto endpoints = resolver.resolve(
        IPLocator::ip_to_string(serverLoc),
        std::to_string(IPLocator::getPhysicalPort(serverLoc)));

    auto secure_socket = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(io_service, ssl_context);
    asio::ssl::verify_mode vm = 0x00;
    vm |= asio::ssl::verify_peer;
    secure_socket->set_verify_mode(vm);

    asio::async_connect(secure_socket->lowest_layer(), endpoints,
            [secure_socket](const std::error_code& ec
#if ASIO_VERSION >= 101200
            , asio::ip::tcp::endpoint
#else
            , const tcp::resolver::iterator&     /*endpoint*/
#endif // if ASIO_VERSION >= 101200
            )
            {
                ASSERT_TRUE(!ec);
                asio::ssl::stream_base::handshake_type role = asio::ssl::stream_base::server;
                secure_socket->async_handshake(role,
                [](const std::error_code& ec)
                {
                    ASSERT_TRUE(!ec);
                });
            });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    /*
       Get server's accepted channel. This is retrieved from the unbound_channel_resources_,
       which is a vector where client channels are pushed immediately after the server accepts
       a connection. This channel will not be present in the server's channel_resources_ map
       as communication lacks most of the discovery messages using a raw socket as participant.
     */
    auto sender_unbound_channel_resources = senderTransportUnderTest.get_unbound_channel_resources();
    ASSERT_TRUE(sender_unbound_channel_resources.size() == 1);
    auto sender_channel_resource =
            std::static_pointer_cast<TCPChannelResourceBasic>(sender_unbound_channel_resources[0]);

    // Prepare the message
    asio::error_code ec;
    std::vector<octet> message(msg_size, 0);
    const octet* data = message.data();
    size_t size = message.size();

    // Send the message with no header
    for (int i = 0; i < 5; i++)
    {
        sender_channel_resource->send(nullptr, 0, data, size, ec);
    }

    secure_socket->lowest_layer().close(ec);
}
#endif // ifndef _WIN32

#endif //TLS_FOUND

TEST_F(TCPv4Tests, send_and_receive_between_allowed_localhost_interfaces_ports)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
    std::regex filter("RTCP(?!_SEQ)");
    eprosima::fastdds::dds::Log::SetCategoryFilter(filter);
    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.interfaceWhiteList.emplace_back("127.0.0.1");
    recvDescriptor.add_listener_port(g_default_port);
    TCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor;
    sendDescriptor.interfaceWhiteList.emplace_back("127.0.0.1");
    TCPv4Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(inputLocator, 7410);

    LocatorList_t locator_list;
    locator_list.push_back(inputLocator);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        SendResourceList send_resource_list;
        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
        ASSERT_FALSE(send_resource_list.empty());
        octet message[5] = { 'H', 'e', 'l', 'l', 'o' };
        bool bOk = false;
        std::function<void()> recCallback = [&]()
                {
                    EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                    bOk = true;
                };

        msg_recv->setCallback(recCallback);

        bool bFinish(false);
        auto sendThreadFunction = [&]()
                {
                    Locators input_begin(locator_list.begin());
                    Locators input_end(locator_list.end());

                    bool sent =
                            send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    while (!bFinish && !sent)
                    {
                        Locators input_begin2(locator_list.begin());
                        Locators input_end2(locator_list.end());

                        sent =
                                send_resource_list.at(0)->send(message, 5, &input_begin2, &input_end2,
                                        (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    EXPECT_TRUE(sent);
                    //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                };

        senderThread.reset(new std::thread(sendThreadFunction));
        std::this_thread::sleep_for(std::chrono::seconds(10));
        bFinish = true;
        senderThread->join();
        ASSERT_TRUE(bOk);
    }
}

TEST_F(TCPv4Tests, send_and_receive_between_blocked_interfaces_ports)
{
    LocatorList_t interfaces;
    if (IPFinder::getAllIPAddress(&interfaces))
    {
        Locator_t locator;
        for (auto& tmpLocator : interfaces)
        {
            if (tmpLocator.kind == LOCATOR_KIND_UDPv4 && IPLocator::toIPv4string(tmpLocator) != "127.0.0.1")
            {
                locator = tmpLocator;
                break;
            }
        }

        if (IsAddressDefined(locator))
        {
            eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
            std::regex filter("RTCP(?!_SEQ)");
            eprosima::fastdds::dds::Log::SetCategoryFilter(filter);
            TCPv4TransportDescriptor recvDescriptor;
            recvDescriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv4string(locator));
            recvDescriptor.add_listener_port(g_default_port);
            TCPv4Transport receiveTransportUnderTest(recvDescriptor);
            receiveTransportUnderTest.init();

            TCPv4TransportDescriptor sendDescriptor;
            sendDescriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv4string(locator));
            TCPv4Transport sendTransportUnderTest(sendDescriptor);
            sendTransportUnderTest.init();

            Locator_t inputLocator;
            inputLocator.kind = LOCATOR_KIND_TCPv4;
            inputLocator.port = g_default_port;
            IPLocator::setIPv4(inputLocator, 127, 0, 0, 1);
            IPLocator::setLogicalPort(inputLocator, 7410);

            LocatorList_t locator_list;
            locator_list.push_back(inputLocator);

            Locator_t outputLocator;
            outputLocator.kind = LOCATOR_KIND_TCPv4;
            IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
            outputLocator.port = g_default_port;
            IPLocator::setLogicalPort(outputLocator, 7410);

            {
                MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
                MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
                ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

                SendResourceList send_resource_list;
                ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(send_resource_list, outputLocator));
                ASSERT_FALSE(send_resource_list.empty());
                octet message[5] = { 'H', 'e', 'l', 'l', 'o' };
                bool bOk = false;
                std::function<void()> recCallback = [&]()
                        {
                            EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
                            bOk = true;
                        };

                msg_recv->setCallback(recCallback);

                bool bFinished(false);
                auto sendThreadFunction = [&]()
                        {
                            Locators input_begin(locator_list.begin());
                            Locators input_end(locator_list.end());

                            bool sent =
                                    send_resource_list.at(0)->send(message, 5, &input_begin, &input_end,
                                            (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                            while (!bFinished && !sent)
                            {
                                Locators input_begin2(locator_list.begin());
                                Locators input_end2(locator_list.end());

                                sent =
                                        send_resource_list.at(0)->send(message, 5, &input_begin2, &input_end2,
                                                (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                            EXPECT_FALSE(sent);
                            //EXPECT_TRUE(transportUnderTest.send(message, 5, outputLocator, inputLocator));
                        };

                senderThread.reset(new std::thread(sendThreadFunction));
                std::this_thread::sleep_for(std::chrono::seconds(10));
                bFinished = true;
                senderThread->join();
                ASSERT_FALSE(bOk);
            }
        }
    }
}

#endif // ifndef __APPLE__

TEST_F(TCPv4Tests, receive_unordered_data)
{
    constexpr uint16_t logical_port = 7410;
    constexpr uint32_t num_bytes_1 = 3;
    constexpr uint32_t num_bytes_2 = 13;
    const char* bad_headers[] =
    {
        "-RTC", "-RT", "-R",
        "-RRTC", "-RRT", "-RR",
        "-RTRTC", "-RTRT", "-RTR",
        "-RTCRTC", "-RTCRT", "-RTCR"
    };

    struct Receiver : public TransportReceiverInterface
    {
        std::array<std::size_t, 3> num_received{ 0, 0, 0 };

        void OnDataReceived(
                const octet* data,
                const uint32_t size,
                const Locator_t& local_locator,
                const Locator_t& remote_locator) override
        {
            static_cast<void>(data);
            static_cast<void>(local_locator);
            static_cast<void>(remote_locator);

            std::cout << "Received " << size << " bytes: " << std::hex << uint32_t(data[0]) << std::dec << std::endl;

            switch (size)
            {
                case num_bytes_1:
                    num_received[0]++;
                    break;
                case num_bytes_2:
                    num_received[1]++;
                    break;
                default:
                    num_received[2]++;
                    break;
            }
        }

    };

    Receiver receiver;

    TCPv4TransportDescriptor test_descriptor = descriptor;
    test_descriptor.check_crc = false;
    TCPv4Transport uut(test_descriptor);
    ASSERT_TRUE(uut.init()) << "Failed to initialize transport. Port " << g_default_port << " may be in use";

    Locator_t input_locator;
    input_locator.kind = LOCATOR_KIND_TCPv4;
    input_locator.port = g_default_port;
    IPLocator::setIPv4(input_locator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(input_locator, logical_port);

    EXPECT_TRUE(uut.OpenInputChannel(input_locator, &receiver, 0xFFFF));

    // Let acceptor to be open
    std::this_thread::sleep_for(std::chrono::seconds(1));

    asio::error_code ec;
    asio::io_context ctx;

    asio::ip::tcp::socket sender(ctx);
    asio::ip::tcp::endpoint destination;
    destination.port(g_default_port);
    destination.address(asio::ip::address::from_string("127.0.0.1"));
    sender.connect(destination, ec);
    ASSERT_TRUE(!ec) << ec;

    std::array<octet, num_bytes_1> bytes_1{ 0 };
    std::array<octet, num_bytes_2> bytes_2{ 0 };

    TCPHeader h1;
    h1.logical_port = logical_port;
    h1.length += num_bytes_1;

    TCPHeader h2;
    h2.logical_port = logical_port;
    h2.length += num_bytes_2;

    std::array<std::size_t, 3> expected_number{ 0, 0, 0 };

    auto send_first = [&]()
            {
                expected_number[0]++;
                bytes_1[0]++;
                EXPECT_EQ(TCPHeader::size(), asio::write(sender, asio::buffer(&h1, TCPHeader::size()), ec));
                EXPECT_EQ(num_bytes_1, asio::write(sender, asio::buffer(bytes_1.data(), bytes_1.size()), ec));
            };

    // Send first synchronized
    send_first();

    // Send non-matching RTCP headers
    for (const char* header : bad_headers)
    {
        asio::write(sender, asio::buffer(header, strlen(header) - 1), ec);
    }

    // Send first prepended with bad headers
    for (const char* header : bad_headers)
    {
        asio::write(sender, asio::buffer(header, strlen(header) - 1), ec);
        send_first();
    }

    // Interleave headers and data (only first will arrive)
    expected_number[0]++;
    EXPECT_EQ(TCPHeader::size(), asio::write(sender, asio::buffer(&h1, TCPHeader::size()), ec));
    EXPECT_EQ(TCPHeader::size(), asio::write(sender, asio::buffer(&h2, TCPHeader::size()), ec));
    EXPECT_EQ(num_bytes_1, asio::write(sender, asio::buffer(bytes_1.data(), bytes_1.size()), ec));
    EXPECT_EQ(num_bytes_2, asio::write(sender, asio::buffer(bytes_2.data(), bytes_2.size()), ec));

    // Send second without interleaving
    expected_number[1]++;
    EXPECT_EQ(TCPHeader::size(), asio::write(sender, asio::buffer(&h2, TCPHeader::size()), ec));
    EXPECT_EQ(num_bytes_2, asio::write(sender, asio::buffer(bytes_2.data(), bytes_2.size()), ec));

    // Wait for data to be received
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_TRUE(!sender.close(ec));

    EXPECT_EQ(expected_number, receiver.num_received);

    EXPECT_TRUE(uut.CloseInputChannel(input_locator));
}

// This test verifies that disabling a TCPChannelResource in the middle of a Receive call (invoked in
// perform_listen_operation) does not result in a hungup state [13721].
TEST_F(TCPv4Tests, header_read_interrumption)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
    std::regex filter("RTCP(?!_SEQ)");
    eprosima::fastdds::dds::Log::SetCategoryFilter(filter);

    TCPv4TransportDescriptor test_descriptor;
    test_descriptor.add_listener_port(g_default_port);
    TCPv4Transport transportUnderTest(test_descriptor);
    transportUnderTest.init();

    Locator_t locator;
    locator.kind = LOCATOR_KIND_TCPv4;
    locator.port = g_default_port;
    IPLocator::setIPv4(locator, 127, 0, 0, 1);
    IPLocator::setLogicalPort(locator, 7410);

    std::weak_ptr<eprosima::fastdds::rtps::RTCPMessageManager> rtcp_manager =
            std::make_shared<eprosima::fastdds::rtps::RTCPMessageManager>(&transportUnderTest);
    std::shared_ptr<TCPChannelResource> channel = std::make_shared<MockTCPChannelResource>(&transportUnderTest, locator,
                    32767);
    octet* buffer = {};
    uint32_t receive_buffer_capacity = 65500;
    uint32_t receive_buffer_size = 0;

    // Simulate channel connection
    channel->connect(nullptr);

    // Simulate channel disconnection after a second
    std::thread thread([&channel]
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                channel->disable();
            });

    // Start TCP segment reception
    // Should get stuck in receive_header until channel is disabled
    transportUnderTest.Receive(rtcp_manager, channel, buffer, receive_buffer_capacity, receive_buffer_size, locator);
    thread.join();
}

// This test verifies that the autofill port feature correctly sets an automatic port when
// the descriptors's port is set to 0.
TEST_F(TCPv4Tests, autofill_port)
{
    // Check normal port assignation
    TCPv4TransportDescriptor test_descriptor;
    test_descriptor.add_listener_port(g_default_port);
    TCPv4Transport transportUnderTest(test_descriptor);
    transportUnderTest.init();

    EXPECT_TRUE(transportUnderTest.configuration()->listening_ports[0] == g_default_port);

    // Check default port assignation
    TCPv4TransportDescriptor test_descriptor_autofill;
    test_descriptor_autofill.add_listener_port(0);
    TCPv4Transport transportUnderTest_autofill(test_descriptor_autofill);
    transportUnderTest_autofill.init();

    EXPECT_TRUE(transportUnderTest_autofill.configuration()->listening_ports[0] != 0);
    EXPECT_TRUE(transportUnderTest_autofill.configuration()->listening_ports.size() == 1);

    uint16_t port = 12345;
    TCPv4TransportDescriptor test_descriptor_multiple_autofill;
    test_descriptor_multiple_autofill.add_listener_port(0);
    test_descriptor_multiple_autofill.add_listener_port(port);
    test_descriptor_multiple_autofill.add_listener_port(0);
    TCPv4Transport transportUnderTest_multiple_autofill(test_descriptor_multiple_autofill);
    transportUnderTest_multiple_autofill.init();

    EXPECT_TRUE(transportUnderTest_multiple_autofill.configuration()->listening_ports[0] != 0);
    EXPECT_TRUE(transportUnderTest_multiple_autofill.configuration()->listening_ports[1] == port);
    EXPECT_TRUE(transportUnderTest_multiple_autofill.configuration()->listening_ports[2] != 0);
    EXPECT_TRUE(
        transportUnderTest_multiple_autofill.configuration()->listening_ports[0] !=
        transportUnderTest_multiple_autofill.configuration()->listening_ports[2]);
    EXPECT_TRUE(transportUnderTest_multiple_autofill.configuration()->listening_ports.size() == 3);
}

// This test verifies server's channel resources mapping keys uniqueness, where keys are clients locators.
// Clients typically communicated its PID as its locator port. When having several clients in the same
// process this lead to overwriting server's channel resources map elements.
TEST_F(TCPv4Tests, client_announced_local_port_uniqueness)
{
    TCPv4TransportDescriptor recvDescriptor;
    recvDescriptor.add_listener_port(g_default_port);
    MockTCPv4Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv4TransportDescriptor sendDescriptor_1;
    TCPv4Transport sendTransportUnderTest_1(sendDescriptor_1);
    sendTransportUnderTest_1.init();

    TCPv4TransportDescriptor sendDescriptor_2;
    TCPv4Transport sendTransportUnderTest_2(sendDescriptor_2);
    sendTransportUnderTest_2.init();

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(outputLocator, 127, 0, 0, 1);
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    SendResourceList send_resource_list_1;
    ASSERT_TRUE(sendTransportUnderTest_1.OpenOutputChannel(send_resource_list_1, outputLocator));
    ASSERT_FALSE(send_resource_list_1.empty());

    SendResourceList send_resource_list_2;
    ASSERT_TRUE(sendTransportUnderTest_2.OpenOutputChannel(send_resource_list_2, outputLocator));
    ASSERT_FALSE(send_resource_list_2.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(receiveTransportUnderTest.get_channel_resources().size(), 2);
}

#ifndef _WIN32
// The primary purpose of this test is to check the non-blocking behavior of a secure socket sending data to a
// destination that does not read or does it so slowly.
TEST_F(TCPv4Tests, non_blocking_send)
{
    uint16_t port = g_default_port;
    uint32_t msg_size = eprosima::fastdds::rtps::s_minimumSocketBuffer;
    // Create a TCP Server transport
    TCPv4TransportDescriptor senderDescriptor;
    senderDescriptor.add_listener_port(port);
    senderDescriptor.sendBufferSize = msg_size;
    MockTCPv4Transport senderTransportUnderTest(senderDescriptor);
    eprosima::fastrtps::rtps::RTPSParticipantAttributes att;
    att.properties.properties().emplace_back("fastdds.tcp_transport.non_blocking_send", "true");
    senderTransportUnderTest.init(&att.properties);

    // Create a TCP Client socket.
    // The creation of a reception transport for testing this functionality is not
    // feasible. For the saturation of the sending socket, it's necessary first to
    // saturate the reception socket of the datareader. This saturation requires
    // preventing the datareader from reading from the socket, what inevitably
    // happens continuously if instantiating and connecting the receiver transport.
    // Hence, a raw socket is opened and connected to the server. There won't be read
    // calls on that socket.
    Locator_t serverLoc;
    serverLoc.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setIPv4(serverLoc, 127, 0, 0, 1);
    serverLoc.port = port;
    IPLocator::setLogicalPort(serverLoc, 7410);

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
    ASSERT_TRUE(sender_unbound_channel_resources.size() == 1);
    auto sender_channel_resource =
            std::static_pointer_cast<TCPChannelResourceBasic>(sender_unbound_channel_resources[0]);

    // Prepare the message
    asio::error_code ec;
    std::vector<octet> message(msg_size, 0);
    const octet* data = message.data();
    size_t size = message.size();

    // Send the message with no header
    for (int i = 0; i < 5; i++)
    {
        sender_channel_resource->send(nullptr, 0, data, size, ec);
    }

    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.cancel();
    socket.close();
}
#endif // ifndef _WIN32

void TCPv4Tests::HELPER_SetDescriptorDefaults()
{
    descriptor.add_listener_port(g_default_port);
    descriptor.set_WAN_address(g_test_wan_address);
}

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
    g_default_port = get_port(4000);
    g_output_port = get_port(5000);
    g_input_port = get_port(5010);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
