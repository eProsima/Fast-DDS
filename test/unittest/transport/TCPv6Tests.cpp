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

#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/transport/TCPv6Transport.h>
#include <gtest/gtest.h>
#include <thread>
#include <memory>
#include <fastdds/dds/log/Log.hpp>
#include <asio.hpp>
#include <MockReceiverResource.h>


using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif

static uint16_t g_default_port = 0;

uint16_t get_port()
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if(4000 > port)
    {
        port += 4000;
    }

    return port;
}

class TCPv6Tests: public ::testing::Test
{
    public:
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

TEST_F(TCPv6Tests, conversion_to_ip6_string)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_TCPv6;
    ASSERT_EQ("0:0:0:0:0:0:0:0", IPLocator::toIPv6string(locator));

    locator.address[0] = 0xff;
    ASSERT_EQ("ff00:0:0:0:0:0:0:0", IPLocator::toIPv6string(locator));

    locator.address[1] = 0xaa;
    ASSERT_EQ("ffaa:0:0:0:0:0:0:0", IPLocator::toIPv6string(locator));

    locator.address[2] = 0x0a;
    ASSERT_EQ("ffaa:a00:0:0:0:0:0:0", IPLocator::toIPv6string(locator));

    locator.address[5] = 0x0c;
    ASSERT_EQ("ffaa:a00:c:0:0:0:0:0", IPLocator::toIPv6string(locator));
}

TEST_F(TCPv6Tests, setting_ip6_values_on_locators)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_TCPv6;

    IPLocator::setIPv6(locator, 0xffff,0xa, 0xaba, 0, 0, 0, 0, 0);
    ASSERT_EQ("ffff:a:aba:0:0:0:0:0", IPLocator::toIPv6string(locator));
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
    ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
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

    NetworkFactory factory;
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
/*
TEST_F(TCPv6Tests, send_and_receive_between_both_secure_ports)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);

    using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;

    TCPv6TransportDescriptor recvDescriptor;
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
    TCPv6Transport receiveTransportUnderTest(recvDescriptor);
    receiveTransportUnderTest.init();

    TCPv6TransportDescriptor sendDescriptor;
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
    TCPv6Transport sendTransportUnderTest(sendDescriptor);
    sendTransportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.kind = LOCATOR_KIND_TCPv6;
    inputLocator.port = g_default_port;
    IPLocator::setIPv4(inputLocator, "::1");
    IPLocator::setLogicalPort(inputLocator, 7410);

    Locator_t outputLocator;
    outputLocator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv4(outputLocator, "::1");
    outputLocator.port = g_default_port;
    IPLocator::setLogicalPort(outputLocator, 7410);

    {
        MockReceiverResource receiver(receiveTransportUnderTest, inputLocator);
        MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());
        ASSERT_TRUE(receiveTransportUnderTest.IsInputChannelOpen(inputLocator));

        ASSERT_TRUE(sendTransportUnderTest.OpenOutputChannel(outputLocator));
        octet message[5] = { 'H','e','l','l','o' };

        Semaphore sem;
        std::function<void()> recCallback = [&]()
        {
            EXPECT_EQ(memcmp(message, msg_recv->data, 5), 0);
            sem.post();
        };

        msg_recv->setCallback(recCallback);

        auto sendThreadFunction = [&]()
        {
            bool sent = sendTransportUnderTest.send(message, 5, outputLocator, inputLocator);
            while (!sent)
            {
                sent = sendTransportUnderTest.send(message, 5, outputLocator, inputLocator);
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
    ASSERT_TRUE(sendTransportUnderTest.CloseOutputChannel(outputLocator));
}
*/
// TODO SKIP AT THIS MOMENT
/*
TEST_F(TCPv6Tests, send_and_receive_between_ports)
{
    descriptor.listening_ports.push_back(g_default_port);
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t localLocator;
    localLocator.port = g_default_port;
    localLocator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(localLocator, "::1");

    Locator_t outputChannelLocator;
    outputChannelLocator = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(outputChannelLocator, "::1");

    MockReceiverResource receiver(transportUnderTest, localLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator)); // Includes loopback
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(localLocator));
    octet message[5] = { 'H','e','l','l','o' };

    std::this_thread::sleep_for(std::chrono::seconds(5));

    Semaphore sem;
    std::function<void()> recCallback = [&]()
    {
        EXPECT_EQ(memcmp(message,msg_recv->data,5), 0);
        sem.post();
    };

    msg_recv->setCallback(recCallback);

    auto sendThreadFunction = [&]()
    {
        EXPECT_TRUE(transportUnderTest.send(message, 5, outputChannelLocator, localLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

TEST_F(TCPv6Tests, send_to_loopback)
{
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    multicastLocator.set_port(g_default_port);
    multicastLocator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(multicastLocator, 0xff31, 0, 0, 0, 0, 0, 0, 0);

    Locator_t outputChannelLocator;
    outputChannelLocator.set_port(g_default_port + 1);
    outputChannelLocator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setIPv6(outputChannelLocator, 0,0,0,0,0,0,0,1); // Loopback

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator));
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(multicastLocator));
    octet message[5] = { 'H','e','l','l','o' };

    Semaphore sem;
    std::function<void()> recCallback = [&]()
    {
        EXPECT_EQ(memcmp(message,msg_recv->data,5), 0);
        sem.post();
    };

    msg_recv->setCallback(recCallback);

    auto sendThreadFunction = [&]()
    {
        EXPECT_TRUE(transportUnderTest.send(message, 5, outputChannelLocator, multicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}
*/
#endif

void TCPv6Tests::HELPER_SetDescriptorDefaults()
{
}

int main(int argc, char **argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
