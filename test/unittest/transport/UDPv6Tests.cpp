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

#include <thread>
#include <memory>

#include <asio.hpp>
#include <gtest/gtest.h>
#include <MockReceiverResource.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <rtps/transport/UDPv6Transport.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;
using UDPv6Transport = eprosima::fastdds::rtps::UDPv6Transport;

#ifndef __APPLE__
const uint32_t ReceiveBufferCapacity = 65536;
#endif // ifndef __APPLE__

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

class UDPv6Tests : public ::testing::Test
{
public:

    UDPv6Tests()
    {
        HELPER_SetDescriptorDefaults();
    }

    ~UDPv6Tests()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

    void HELPER_SetDescriptorDefaults();

    UDPv6TransportDescriptor descriptor;
    std::unique_ptr<std::thread> senderThread;
    std::unique_ptr<std::thread> receiverThread;
};

TEST_F(UDPv6Tests, conversion_to_ip6_string)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;
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

TEST_F(UDPv6Tests, setting_ip6_values_on_locators)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;

    IPLocator::setIPv6(locator, 0xffff, 0xa, 0xaba, 0, 0, 0, 0, 0);
    ASSERT_EQ("ffff:a:aba::", IPLocator::toIPv6string(locator));
}

TEST_F(UDPv6Tests, locators_with_kind_2_supported)
{
    // Given
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t supportedLocator;
    supportedLocator.kind = LOCATOR_KIND_UDPv6;
    Locator_t unsupportedLocator;
    unsupportedLocator.kind = LOCATOR_KIND_UDPv4;

    // Then
    ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocator));
}

#ifndef __APPLE__
TEST_F(UDPv6Tests, opening_and_closing_input_channel)
{
    // Given
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastFilterLocator;
    multicastFilterLocator.kind = LOCATOR_KIND_UDPv6;
    multicastFilterLocator.port = g_default_port; // arbitrary
    IPLocator::setIPv6(multicastFilterLocator, 0xff31, 0, 0, 0, 0, 0, 0x8000, 0x1234);

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(multicastFilterLocator, nullptr, 0x8FFF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(multicastFilterLocator));
}

TEST_F(UDPv6Tests, send_and_receive_between_ports)
{
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    multicastLocator.port = g_default_port;
    multicastLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(multicastLocator, "ff31::8000:1234");

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator, "ff31::8000:1234");

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
    ASSERT_FALSE(send_resource_list.empty());
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
        LocatorList_t locator_list;
        locator_list.push_back(multicastLocator);

        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, 5, &locators_begin, &locators_end,
                (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
}

TEST_F(UDPv6Tests, send_to_loopback)
{
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    multicastLocator.port = g_default_port;
    multicastLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(multicastLocator, "ff31::8000:1234");

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator,0,0,0,0,0,0,0,1); // Loopback

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
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
        LocatorList_t locator_list;
        locator_list.push_back(multicastLocator);

        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, 5, &locators_begin, &locators_end,
                (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
}
#endif // ifndef __APPLE__

void UDPv6Tests::HELPER_SetDescriptorDefaults()
{
    descriptor.maxMessageSize = 5;
    descriptor.sendBufferSize = 5;
    descriptor.receiveBufferSize = 5;
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
