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
#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/transport/TCPv6Transport.h>
#include <gtest/gtest.h>
#include <thread>
#include <memory>
#include <fastrtps/log/Log.h>
#include <asio.hpp>
#include <MockReceiverResource.h>


using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

#ifndef __APPLE__
const uint32_t ReceiveBufferCapacity = 65536;
#endif

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
            Log::KillThread();
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
    ASSERT_EQ("0:0:0:0:0:0:0:0", locator.to_IP6_string());

    locator.get_Address()[0] = 0xff;
    ASSERT_EQ("ff00:0:0:0:0:0:0:0", locator.to_IP6_string());

    locator.get_Address()[1] = 0xaa;
    ASSERT_EQ("ffaa:0:0:0:0:0:0:0", locator.to_IP6_string());

    locator.get_Address()[2] = 0x0a;
    ASSERT_EQ("ffaa:a00:0:0:0:0:0:0", locator.to_IP6_string());

    locator.get_Address()[5] = 0x0c;
    ASSERT_EQ("ffaa:a00:c:0:0:0:0:0", locator.to_IP6_string());
}

TEST_F(TCPv6Tests, setting_ip6_values_on_locators)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_TCPv6;

    locator.set_IP6_address(0xffff,0xa, 0xaba, 0, 0, 0, 0, 0);
    ASSERT_EQ("ffff:a:aba:0:0:0:0:0", locator.to_IP6_string());
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
    genericOutputChannelLocator.set_port(g_default_port); // arbitrary

    // Then
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator, nullptr));
    ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
}

#ifndef __APPLE__
TEST_F(TCPv6Tests, opening_and_closing_input_channel)
{
    // Given
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastFilterLocator;
    multicastFilterLocator.kind = LOCATOR_KIND_TCPv6;
    multicastFilterLocator.set_port(g_default_port); // arbitrary
    multicastFilterLocator.set_IP6_address(0xff31, 0, 0, 0, 0, 0, 0x8000, 0x1234);

    NetworkFactory factory;
    factory.RegisterTransport<TCPv6Transport, TCPv6TransportDescriptor>(descriptor);
    std::vector<std::shared_ptr<ReceiverResource>> receivers;
    factory.BuildReceiverResources(multicastFilterLocator, nullptr, 0x8FFF, receivers);
    ReceiverResource* receiver = receivers.back().get();

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(multicastFilterLocator, receiver, 0x8FFF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(multicastFilterLocator));
}

TEST_F(TCPv6Tests, send_and_receive_between_ports)
{
    TCPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t localLocator;
    localLocator.set_port(g_default_port);
    localLocator.kind = LOCATOR_KIND_TCPv6;
    localLocator.set_IP6_address("::1");

    Locator_t outputChannelLocator;
    outputChannelLocator.set_port(g_default_port + 1);
    outputChannelLocator.kind = LOCATOR_KIND_TCPv6;

    MockReceiverResource receiver(transportUnderTest, localLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator, nullptr)); // Includes loopback
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(localLocator));
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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, localLocator));
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
    multicastLocator.set_IP6_address(0xff31, 0, 0, 0, 0, 0, 0, 0);

    Locator_t outputChannelLocator;
    outputChannelLocator.set_port(g_default_port + 1);
    outputChannelLocator.kind = LOCATOR_KIND_TCPv6;
    outputChannelLocator.set_IP6_address(0,0,0,0,0,0,0,1); // Loopback

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator, nullptr));
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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, multicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

#endif

void TCPv6Tests::HELPER_SetDescriptorDefaults()
{
    descriptor.maxMessageSize = 5;
    descriptor.sendBufferSize = 5;
    descriptor.receiveBufferSize = 5;
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
