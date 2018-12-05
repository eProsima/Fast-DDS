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
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/rtps/network/NetworkFactory.h>
#include <gtest/gtest.h>
#include <thread>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>
//#include <fastrtps/log/Log.h>
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

uint16_t get_port()
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if(4000 > port)
    {
        port += 4000;
    }

    return port;
}

static void GetIP4s(std::vector<IPFinder::info_IP>& interfaces)
{
    IPFinder::getIPs(&interfaces, false);
    auto new_end = remove_if(interfaces.begin(),
            interfaces.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;});
    interfaces.erase(new_end, interfaces.end());
    std::for_each(interfaces.begin(), interfaces.end(), [](auto&& loc)
    {
        loc.locator.kind = LOCATOR_KIND_UDPv4;
    });
}


class UDPv4Tests: public ::testing::Test
{
    public:
        UDPv4Tests()
        {
            HELPER_SetDescriptorDefaults();
        }

        ~UDPv4Tests()
        {
            //Log::KillThread();
        }

        void HELPER_SetDescriptorDefaults();

        UDPv4TransportDescriptor descriptor;
        std::unique_ptr<std::thread> senderThread;
        std::unique_ptr<std::thread> receiverThread;
};

TEST_F(UDPv4Tests, locators_with_kind_1_supported)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t supportedLocator;
    supportedLocator.kind = LOCATOR_KIND_UDPv4;
    Locator_t unsupportedLocator;
    unsupportedLocator.kind = LOCATOR_KIND_UDPv6;

    // Then
    ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocator));
}

TEST_F(UDPv4Tests, opening_and_closing_output_channel)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    genericOutputChannelLocator.port = g_default_port; // arbitrary

    // Then
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
}

TEST_F(UDPv4Tests, opening_and_closing_input_channel)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastFilterLocator;
    multicastFilterLocator.kind = LOCATOR_KIND_UDPv4;
    multicastFilterLocator.port = g_default_port; // arbitrary
    IPLocator::setIPv4(multicastFilterLocator, 239, 255, 0, 1);

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(multicastFilterLocator, nullptr, 0x8FFF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(multicastFilterLocator));
}

#ifndef __APPLE__
TEST_F(UDPv4Tests, send_and_receive_between_ports)
{
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    multicastLocator.port = g_default_port;
    multicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(multicastLocator, 239, 255, 0, 1);

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;

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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, multicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

TEST_F(UDPv4Tests, send_to_loopback)
{
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    multicastLocator.port = g_default_port;
    multicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(multicastLocator, 239, 255, 0, 1);

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, 127,0,0,1); // Loopback

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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, multicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}
#endif

TEST_F(UDPv4Tests, send_is_rejected_if_buffer_size_is_bigger_to_size_specified_in_descriptor)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    genericOutputChannelLocator.port = g_default_port;
    transportUnderTest.OpenOutputChannel(genericOutputChannelLocator);

    Locator_t destinationLocator;
    destinationLocator.kind = LOCATOR_KIND_UDPv4;
    destinationLocator.port = g_default_port + 1;

    // Then
    std::vector<octet> receiveBufferWrongSize(descriptor.sendBufferSize + 1);
    ASSERT_FALSE(transportUnderTest.Send(receiveBufferWrongSize.data(), (uint32_t)receiveBufferWrongSize.size(), genericOutputChannelLocator, destinationLocator));

    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
}

TEST_F(UDPv4Tests, RemoteToMainLocal_simply_strips_out_address_leaving_IP_ANY)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t remoteLocator;
    remoteLocator.kind = LOCATOR_KIND_UDPv4;
    remoteLocator.port = g_default_port;
    IPLocator::setIPv4(remoteLocator, 222,222,222,222);

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remoteLocator);

    ASSERT_EQ(mainLocalLocator.port, remoteLocator.port);
    ASSERT_EQ(mainLocalLocator.kind, remoteLocator.kind);

    ASSERT_EQ(IPLocator::toIPv4string(mainLocalLocator), s_IPv4AddressAny);
}

TEST_F(UDPv4Tests, match_if_port_AND_address_matches)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
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

TEST_F(UDPv4Tests, send_to_wrong_interface)
{
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator));

    //Sending through a different IP will NOT work, except 0.0.0.0
    IPLocator::setIPv4(outputChannelLocator, 111, 111, 111, 111);
    std::vector<octet> message = { 'H','e','l','l','o' };
    ASSERT_FALSE(transportUnderTest.Send(message.data(), (uint32_t)message.size(), outputChannelLocator, Locator_t()));

    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

TEST_F(UDPv4Tests, send_to_blocked_interface)
{
    descriptor.interfaceWhiteList.emplace_back("111.111.111.111");
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator));

    // Sending through a BLOCKED IP will NOT work
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1);
    std::vector<octet> message = { 'H','e','l','l','o' };
    ASSERT_FALSE(transportUnderTest.Send(message.data(), (uint32_t)message.size(), outputChannelLocator, Locator_t()));
    ASSERT_FALSE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

TEST_F(UDPv4Tests, send_to_allowed_interface)
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
            descriptor.interfaceWhiteList.emplace_back("127.0.0.1");
            descriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv4string(locator));
            UDPv4Transport transportUnderTest(descriptor);
            transportUnderTest.init();

            Locator_t outputChannelLocator;
            outputChannelLocator.port = g_default_port;
            outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
            IPLocator::setIPv4(outputChannelLocator, IPLocator::toIPv4string(locator));
            ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator));

            Locator_t remoteMulticastLocator;
            remoteMulticastLocator.port = g_default_port;
            remoteMulticastLocator.kind = LOCATOR_KIND_UDPv4;
            IPLocator::setIPv4(remoteMulticastLocator, 239, 255, 1, 4); // Loopback

            // Sending through a ALLOWED IP will work
            IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1);
            std::vector<octet> message = { 'H','e','l','l','o' };
            ASSERT_TRUE(transportUnderTest.Send(message.data(), (uint32_t)message.size(), outputChannelLocator, remoteMulticastLocator));
            ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
        }
    }
}
#ifndef __APPLE__

TEST_F(UDPv4Tests, send_and_receive_between_allowed_sockets_using_localhost)
{
    descriptor.interfaceWhiteList.emplace_back("127.0.0.1");
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t unicastLocator;
    unicastLocator.port = g_default_port;
    unicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(unicastLocator, "127.0.0.1");

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, "127.0.0.1");

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator)); // Includes loopback
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, unicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

TEST_F(UDPv4Tests, send_and_receive_between_allowed_sockets_using_unicast)
{
    std::vector<IPFinder::info_IP> interfaces;
    GetIP4s(interfaces);

    for(const auto& interface : interfaces)
    {
        descriptor.interfaceWhiteList.push_back(interface.name);
    }
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t unicastLocator;
    unicastLocator.port = g_default_port;
    unicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(unicastLocator, interfaces.at(0).name);

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, interfaces.at(0).name);

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator)); // Includes loopback
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, unicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

TEST_F(UDPv4Tests, send_and_receive_between_allowed_sockets_using_unicast_to_multicast)
{
    std::vector<IPFinder::info_IP> interfaces;
    GetIP4s(interfaces);

    for(const auto& interface : interfaces)
    {
        descriptor.interfaceWhiteList.push_back(interface.name);
    }
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t unicastLocator;
    unicastLocator.port = g_default_port;
    unicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(unicastLocator, "239.255.1.4");

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, interfaces.at(0).name);

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator)); // Includes loopback
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
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
        EXPECT_TRUE(transportUnderTest.Send(message, 5, outputChannelLocator, unicastLocator));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
    ASSERT_TRUE(transportUnderTest.CloseOutputChannel(outputChannelLocator));
}
#endif

TEST_F(UDPv4Tests, open_a_blocked_socket)
{
    descriptor.interfaceWhiteList.emplace_back("111.111.111.111");
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    multicastLocator.port = g_default_port;
    multicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(multicastLocator, 239, 255, 0, 1);

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    ASSERT_FALSE(transportUnderTest.IsInputChannelOpen(multicastLocator));
}

TEST_F(UDPv4Tests, shrink_locator_lists)
{
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    LocatorList_t result, list1, list2, list3;
    Locator_t locator, locResult1, locResult2, locResult3;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = g_default_port;
    locResult1.kind = LOCATOR_KIND_UDPv4;
    locResult1.port = g_default_port;
    locResult2.kind = LOCATOR_KIND_UDPv4;
    locResult2.port = g_default_port;
    locResult3.kind = LOCATOR_KIND_UDPv4;
    locResult3.port = g_default_port;

    // Check shrink of only one locator list unicast.
    IPLocator::setIPv4(locator, 192,168,1,4);
    IPLocator::setIPv4(locResult1, 192,168,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,2,5);
    IPLocator::setIPv4(locResult2, 192,168,2,5);
    list1.push_back(locator);

    result = transportUnderTest.ShrinkLocatorLists({list1});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);
    list1.clear();

    // Check shrink of only one locator list with multicast.
    IPLocator::setIPv4(locator, 239,255,1,4);
    list1.push_back(locator);

    result = transportUnderTest.ShrinkLocatorLists({list1});
    ASSERT_EQ(result.size(), 1);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locator);
    list1.clear();

    // Check shrink of only one locator list with multicast and unicast.
    IPLocator::setIPv4(locator, 192,168,1,4);
    IPLocator::setIPv4(locResult1, 192,168,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,2,5);
    IPLocator::setIPv4(locResult2, 192,168,2,5);
    list1.push_back(locator);

    result = transportUnderTest.ShrinkLocatorLists({list1});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);
    list1.clear();

    // Three. Two use same multicast, the other unicast
    IPLocator::setIPv4(locator, 192,168,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list2.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,2,4);
    list2.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,3,4);
    list3.push_back(locator);
    IPLocator::setIPv4(locResult1, 239,255,1,4);
    IPLocator::setIPv4(locResult2, 192,168,3,4);

    result = transportUnderTest.ShrinkLocatorLists({list1, list2, list3});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);

    result = transportUnderTest.ShrinkLocatorLists({list3, list1, list2});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);

    result = transportUnderTest.ShrinkLocatorLists({list2, list3, list1});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);

    list1.clear();
    list2.clear();
    list3.clear();

    // Three. Two use same multicast, the other another multicast
    IPLocator::setIPv4(locator, 192,168,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list2.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,2,4);
    list2.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,3,4);
    list3.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,2,4);
    list3.push_back(locator);
    IPLocator::setIPv4(locResult1, 239,255,1,4);
    IPLocator::setIPv4(locResult2, 192,168,3,4);

    result = transportUnderTest.ShrinkLocatorLists({list1, list2, list3});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);

    result = transportUnderTest.ShrinkLocatorLists({list3, list1, list2});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);

    result = transportUnderTest.ShrinkLocatorLists({list2, list3, list1});
    ASSERT_EQ(result.size(), 2);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2);

    list1.clear();
    list2.clear();
    list3.clear();

    // Three. One uses multicast, the others unicast
    IPLocator::setIPv4(locator, 192,168,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,2,4);
    list2.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,3,4);
    list3.push_back(locator);
    IPLocator::setIPv4(locResult1, 192,168,1,4);
    IPLocator::setIPv4(locResult2, 192,168,2,4);
    IPLocator::setIPv4(locResult3, 192,168,3,4);

    result = transportUnderTest.ShrinkLocatorLists({list1, list2, list3});
    ASSERT_EQ(result.size(), 3);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2 || *it == locResult3);

    result = transportUnderTest.ShrinkLocatorLists({list3, list1, list2});
    ASSERT_EQ(result.size(), 3);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2 || *it == locResult3);

    result = transportUnderTest.ShrinkLocatorLists({list2, list3, list1});
    ASSERT_EQ(result.size(), 3);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1 || *it == locResult2 || *it == locResult3);

    list1.clear();
    list2.clear();
    list3.clear();

    // Three using same multicast
    IPLocator::setIPv4(locator, 239,255,1,4);
    list1.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list2.push_back(locator);
    IPLocator::setIPv4(locator, 239,255,1,4);
    list3.push_back(locator);
    IPLocator::setIPv4(locator, 192,168,3,4);
    list3.push_back(locator);
    IPLocator::setIPv4(locResult1, 239,255,1,4);

    result = transportUnderTest.ShrinkLocatorLists({list1, list2, list3});
    ASSERT_EQ(result.size(), 1);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1);

    result = transportUnderTest.ShrinkLocatorLists({list3, list1, list2});
    ASSERT_EQ(result.size(), 1);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1);

    result = transportUnderTest.ShrinkLocatorLists({list2, list3, list1});
    ASSERT_EQ(result.size(), 1);
    for(auto it = result.begin(); it != result.end(); ++it)
        ASSERT_TRUE(*it == locResult1);

    list1.clear();
    list2.clear();
    list3.clear();
}

void UDPv4Tests::HELPER_SetDescriptorDefaults()
{
    descriptor.maxMessageSize = 5;
    descriptor.sendBufferSize = 5;
    descriptor.receiveBufferSize = 5;
    descriptor.interfaceWhiteList.clear();
}

int main(int argc, char **argv)
{
    Log::SetVerbosity(Log::Warning);
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
