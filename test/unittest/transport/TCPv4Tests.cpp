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

#include <fastrtps/transport/TCPv4Transport.h>
#include <gtest/gtest.h>
#include <thread>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/log/Log.h>
#include <memory>
#include <asio.hpp>
#include <MockReceiverResource.h>


using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#ifndef __APPLE__
const uint32_t ReceiveBufferCapacity = 65536;
#endif

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif

static uint16_t g_default_port = 0;
static uint16_t g_output_port = 0;
static uint16_t g_input_port = 0;

uint16_t get_port(uint16_t offset)
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if(offset > port)
    {
        port += offset;
    }

    return port;
}

class TCPv4Tests: public ::testing::Test
{
    public:
        TCPv4Tests()
        {
            HELPER_SetDescriptorDefaults();            
        }

        ~TCPv4Tests()
        {
            Log::KillThread();
        }

        void HELPER_SetDescriptorDefaults();

        TCPv4TransportDescriptor descriptor;
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
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericOutputChannelLocator.set_port(g_output_port); // arbitrary

    // Then
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator, nullptr));
    ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
}

TEST_F(TCPv4Tests, opening_and_closing_input_channel)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericInputChannelLocator;
    genericInputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericInputChannelLocator.set_port(g_input_port); // listen port
    genericInputChannelLocator.set_IP4_address(127, 0, 0, 1);

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    //ASSERT_TRUE  (transportUnderTest.OpenInputChannel(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
}

#ifndef __APPLE__

TEST_F(TCPv4Tests, send_and_receive_between_ports)
{
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t inputLocator;
    inputLocator.set_port(g_input_port);
    inputLocator.kind = LOCATOR_KIND_TCPv4;
    inputLocator.set_IP4_address(127, 0, 0, 1);

    Locator_t outputLocator;
    outputLocator.set_port(g_input_port);
    outputLocator.kind = LOCATOR_KIND_TCPv4;

    MockReceiverResource receiver(transportUnderTest, inputLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenInputChannel(inputLocator, &receiver, 0x8FFF));
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputLocator, nullptr));
    octet message[5] = { 'H','e','l','l','o' };

    auto sendThreadFunction = [&]()
    {
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputLocator, inputLocator));
    };

    std::function<void()> recCallback = [&]()
    {
        EXPECT_EQ(memcmp(message,msg_recv->data,5), 0);
    };

    msg_recv->setCallback(recCallback);

    senderThread.reset(new std::thread(sendThreadFunction));
    senderThread->join();
}

#endif

TEST_F(TCPv4Tests, send_is_rejected_if_buffer_size_is_bigger_to_size_specified_in_descriptor)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    genericOutputChannelLocator.set_port(g_output_port);
    transportUnderTest.OpenOutputChannel(genericOutputChannelLocator, nullptr);

    Locator_t destinationLocator;
    destinationLocator.kind = LOCATOR_KIND_TCPv4;
    destinationLocator.set_port(g_output_port + 1);

    // Then
    std::vector<octet> receiveBufferWrongSize(descriptor.sendBufferSize + 1);
    ASSERT_FALSE(transportUnderTest.Send(receiveBufferWrongSize.data(), (uint32_t)receiveBufferWrongSize.size(), genericOutputChannelLocator, destinationLocator));
}

TEST_F(TCPv4Tests, RemoteToMainLocal_simply_strips_out_address_leaving_IP_ANY)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t remoteLocator;
    remoteLocator.kind = LOCATOR_KIND_TCPv4;
    remoteLocator.set_port(g_default_port);
    remoteLocator.set_IP4_address(222,222,222,222);

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remoteLocator);

    ASSERT_EQ(mainLocalLocator.get_port(), remoteLocator.get_port());
    ASSERT_EQ(mainLocalLocator.kind, remoteLocator.kind);
    ASSERT_EQ(mainLocalLocator.to_IP4_string(), "0.0.0.0");
}

TEST_F(TCPv4Tests, match_if_port_AND_address_matches)
{
    // Given
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t locatorAlpha;
    locatorAlpha.set_port(g_default_port);
    locatorAlpha.set_IP4_address(239, 255, 0, 1);
    Locator_t locatorBeta = locatorAlpha;

    // Then
    ASSERT_TRUE(transportUnderTest.DoLocatorsMatch(locatorAlpha, locatorBeta));

    locatorBeta.set_IP4_address(100, 100, 100, 100);
    // Then
    ASSERT_TRUE(transportUnderTest.DoLocatorsMatch(locatorAlpha, locatorBeta));
}

TEST_F(TCPv4Tests, send_to_wrong_interface)
{
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t outputChannelLocator;
    outputChannelLocator.set_port(g_output_port);
    outputChannelLocator.kind = LOCATOR_KIND_TCPv4;
    outputChannelLocator.set_IP4_address(127,0,0,1); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator, nullptr));

    //Sending through a different IP will NOT work, except 0.0.0.0
    outputChannelLocator.set_IP4_address(111,111,111,111);
    std::vector<octet> message = { 'H','e','l','l','o' };
    ASSERT_FALSE(transportUnderTest.Send(message.data(), (uint32_t)message.size(), outputChannelLocator, Locator_t()));
}

TEST_F(TCPv4Tests, shrink_locator_lists)
{
    TCPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    LocatorList_t result, list1, list2, list3;
    Locator_t locator, locResult1, locResult2, locResult3;
    locator.kind = LOCATOR_KIND_TCPv4;
    locator.set_port(g_default_port);
    locResult1.kind = LOCATOR_KIND_TCPv4;
    locResult1.set_port(g_default_port);
    locResult2.kind = LOCATOR_KIND_TCPv4;
    locResult2.set_port(g_default_port);
    locResult3.kind = LOCATOR_KIND_TCPv4;
    locResult3.set_port(g_default_port);

    // Check shrink of only one locator list unicast.
    locator.set_IP4_address(192,168,1,4);
    locResult1.set_IP4_address(192,168,1,4);
    list1.push_back(locator);
    locator.set_IP4_address(192,168,2,5);
    locResult2.set_IP4_address(192,168,2,5);
    list1.push_back(locator);

    result = transportUnderTest.ShrinkLocatorLists({list1});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);
    list1.clear();
}

void TCPv4Tests::HELPER_SetDescriptorDefaults()
{
    descriptor.maxMessageSize = 5;
    descriptor.sendBufferSize = 5;
    descriptor.receiveBufferSize = 5;
    descriptor.add_listener_port(5100);
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Info);
    g_default_port = get_port(4000);
    g_output_port = get_port(5000);
    g_input_port = get_port(5010);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
