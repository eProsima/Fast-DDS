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
#include <fastrtps/transport/UDPv6Transport.h>
#include <gtest/gtest.h>
#include <thread>
#include <memory>
#include <fastdds/dds/log/Log.hpp>
#include <asio.hpp>
#include <MockReceiverResource.h>


using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

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

void reset_locator_address(
        Locator_t& locator)
{
    for (size_t i = 0; i < 16; ++i)
    {
        locator.address[i] = 0;
    }
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
    // IPv6 Addresses adhere to RFC4291 section 2.2
    // https://tools.ietf.org/html/rfc4291#section-2.2
    // Do not worry about 2.2.3, mixed IPv4/v6 as it
    // is optional

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

    //Zero Compression of 2001:DB8:0:0:8:800:200C:417A
    reset_locator_address(locator);
    locator.address[0] = 0x20;
    locator.address[1] = 0x01;
    locator.address[2] = 0x0d;
    locator.address[3] = 0xb8;
    locator.address[9] = 0x08;
    locator.address[10] = 0x08;
    locator.address[12] = 0x20;
    locator.address[13] = 0x0c;
    locator.address[14] = 0x41;
    locator.address[15] = 0x7a;
    ASSERT_EQ("2001:db8::8:800:200c:417a", IPLocator::toIPv6string(locator));

    //Zero Compression of FF01:0:0:0:0:0:0:101
    reset_locator_address(locator);
    locator.address[0] = 0xff;
    locator.address[1] = 0x01;
    locator.address[14] = 0x01;
    locator.address[15] = 0x01;
    ASSERT_EQ("ff01::101", IPLocator::toIPv6string(locator));

    //Zero Compression of 0:0:0:0:0:0:0:1
    reset_locator_address(locator);
    locator.address[15] = 0x01;
    ASSERT_EQ("::1", IPLocator::toIPv6string(locator));

    //Zero Compression of 0:0:0:0:0:0:0:0
    reset_locator_address(locator);
    locator.address[15] = 0x00;
    ASSERT_EQ("::", IPLocator::toIPv6string(locator));

    //Trailing Zeros
    reset_locator_address(locator);
    locator.address[14] = 0x10;
    ASSERT_EQ("::1000", IPLocator::toIPv6string(locator));
    locator.address[15] = 0x20;
    ASSERT_EQ("::1020", IPLocator::toIPv6string(locator));

    //Embedded Zeros in 2001:DB8:a::
    reset_locator_address(locator);
    locator.address[0] = 0x20;
    locator.address[1] = 0x01;
    locator.address[2] = 0x0d;
    locator.address[3] = 0xb8;
    locator.address[5] = 0xa;
    ASSERT_EQ("2001:db8:a::", IPLocator::toIPv6string(locator));

    locator.address[15] = 0x01;
    ASSERT_EQ("2001:db8:a::1", IPLocator::toIPv6string(locator));
    locator.address[15] = 0x10;
    ASSERT_EQ("2001:db8:a::10", IPLocator::toIPv6string(locator));
    locator.address[15] = 0;
    locator.address[14] = 0x01;
    ASSERT_EQ("2001:db8:a::100", IPLocator::toIPv6string(locator));
    locator.address[14] = 0x10;
    ASSERT_EQ("2001:db8:a::1000", IPLocator::toIPv6string(locator));
    locator.address[14] = 0;

    locator.address[13] = 0x01;
    ASSERT_EQ("2001:db8:a::1:0", IPLocator::toIPv6string(locator));
    locator.address[13] = 0x10;
    ASSERT_EQ("2001:db8:a::10:0", IPLocator::toIPv6string(locator));
    locator.address[13] = 0;

    // 2001:db8:a:0:0:1:0:0 special case for two equal compressible blocks
    // When this occurs, it's recommended to collapse the left
    locator.address[11] = 0x01;
    ASSERT_EQ("2001:db8:a::1:0:0", IPLocator::toIPv6string(locator));
    locator.address[11] = 0;

    locator.address[9] = 0x01;
    ASSERT_EQ("2001:db8:a:0:1::", IPLocator::toIPv6string(locator));
    locator.address[9] = 0;

    locator.address[7] = 0x01;
    ASSERT_EQ("2001:db8:a:1::", IPLocator::toIPv6string(locator));
    locator.address[7] = 0;
}


TEST_F(UDPv6Tests, setting_ip6_values_on_locators)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;
    std::string ex;

    ex = "ffff:a:abc::";
    IPLocator::setIPv6(locator, 0xffff, 0xa, 0xabc, 0, 0, 0, 0, 0);
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));
    IPLocator::setIPv6(locator, "FFFF:000A:0ABC:0:0:0:0:0");
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));
    IPLocator::setIPv6(locator, "FFFF:000A:0abc:0:0:0:0:0");
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));
    IPLocator::setIPv6(locator, "FFFF:000A:0abc::");
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));

    // Zero Compression
    ex = "2001:db8:a::";
    IPLocator::setIPv6(locator, 0x2001, 0xdb8, 0xa, 0, 0, 0, 0, 0);
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));
    IPLocator::setIPv6(locator, "2001:db8:a:0:000:0:0000");
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));

    IPLocator::setIPv6(locator, 0x2001, 0xdb8, 0xa, 0, 0, 0, 0, 0xf1);
    ASSERT_EQ("2001:db8:a::f1", IPLocator::toIPv6string(locator));

    IPLocator::setIPv6(locator, 0x2001, 0xdb8, 0xa, 0, 0, 0, 0xf1, 0);
    ASSERT_EQ("2001:db8:a::f1:0", IPLocator::toIPv6string(locator));

    //Special equal compression case
    ex = "2001:db8:a::f1:0:0";
    IPLocator::setIPv6(locator, 0x2001, 0xdb8, 0xa, 0, 0, 0xf1, 0, 0);
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));
    IPLocator::setIPv6(locator, "2001:db8:a:0:0:f1:0:0");
    ASSERT_EQ(ex, IPLocator::toIPv6string(locator));

    IPLocator::setIPv6(locator, 0x2001, 0xdb8, 0xa, 0, 0xf1, 0, 0, 0);
    ASSERT_EQ("2001:db8:a:0:f1::", IPLocator::toIPv6string(locator));

    IPLocator::setIPv6(locator, 0x2001, 0xdb8, 0xa, 0xf1, 0, 0, 0, 0);
    ASSERT_EQ("2001:db8:a:f1::", IPLocator::toIPv6string(locator));
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

TEST_F(UDPv6Tests, opening_and_closing_output_channel)
{
    // Given
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    genericOutputChannelLocator.port = g_default_port; // arbitrary

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
/*
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

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator)); // Includes loopback
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
