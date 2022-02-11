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

#ifndef __APPLE__
TEST_F(UDPv6Tests, send_and_receive_between_ports)
{
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t multicastLocator;
    std::string address("ff31::8000:1234");
    multicastLocator.port = g_default_port;
    multicastLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(multicastLocator, address);

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator, address);

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(multicastLocator));
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
                LocatorList_t locator_list;
                locator_list.push_back(multicastLocator);

                bool sent = false;
                for (auto& send_resource : send_resource_list)
                {
                    Locators locators_begin(locator_list.begin());
                    Locators locators_end(locator_list.end());
                    sent |= send_resource->send(message, 5, &locators_begin, &locators_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    if (sent)
                    {
                        break;
                    }
                }
                EXPECT_TRUE(sent);
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
    IPLocator::setIPv6(multicastLocator, std::string("ff31::8000:1234"));

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator, 0, 0, 0, 0, 0, 0, 0, 1); // Loopback

    MockReceiverResource receiver(transportUnderTest, multicastLocator);
    MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(multicastLocator));
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
                LocatorList_t locator_list;
                locator_list.push_back(multicastLocator);

                bool sent = false;
                for (auto& send_resource : send_resource_list)
                {
                    Locators locators_begin(locator_list.begin());
                    Locators locators_end(locator_list.end());
                    sent |= send_resource->send(message, 5, &locators_begin, &locators_end,
                                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100)));
                    if (sent)
                    {
                        break;
                    }
                }
                EXPECT_TRUE(sent);
            };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
}
#endif // ifndef __APPLE__

TEST_F(UDPv6Tests, send_is_rejected_if_buffer_size_is_bigger_to_size_specified_in_descriptor)
{
    // Given
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    SendResourceList send_resource_list;
    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    genericOutputChannelLocator.port = g_default_port;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, genericOutputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());

    Locator_t destinationLocator;
    destinationLocator.kind = LOCATOR_KIND_UDPv6;
    destinationLocator.port = g_default_port + 1;

    LocatorList_t locator_list;
    locator_list.push_back(destinationLocator);
    Locators locators_begin(locator_list.begin());
    Locators locators_end(locator_list.end());

    // Then
    std::vector<octet> receiveBufferWrongSize(descriptor.sendBufferSize + 1);
    ASSERT_FALSE(send_resource_list.at(0)->send(receiveBufferWrongSize.data(), (uint32_t)receiveBufferWrongSize.size(),
            &locators_begin, &locators_end, (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
}

TEST_F(UDPv6Tests, RemoteToMainLocal_simply_strips_out_address_leaving_IP_ANY)
{
    // Given
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_UDPv6;
    remote_locator.port = g_default_port;
    IPLocator::setIPv6(remote_locator, std::string("fe80::ffff:dede:dede"));

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remote_locator);

    ASSERT_EQ(mainLocalLocator.port, remote_locator.port);
    ASSERT_EQ(mainLocalLocator.kind, remote_locator.kind);

    ASSERT_EQ(IPLocator::toIPv6string(mainLocalLocator), s_IPv6AddressAny);
}

TEST_F(UDPv6Tests, match_if_port_AND_address_matches)
{
    // Given
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t locatorAlpha;
    locatorAlpha.kind = LOCATOR_KIND_UDPv6;
    locatorAlpha.port = g_default_port;
    IPLocator::setIPv6(locatorAlpha, std::string("ff1e::ffff:efff:1"));
    Locator_t locatorBeta = locatorAlpha;

    // Then
    ASSERT_TRUE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));

    IPLocator::setIPv6(locatorBeta, std::string("fe80::ffff:6464:6464"));
    // Then
    ASSERT_TRUE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));
}

TEST_F(UDPv6Tests, send_to_wrong_interface)
{
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    SendResourceList send_resource_list;
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator, std::string("::1")); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());

    LocatorList_t locator_list;
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;
    locator_list.push_back(locator);
    Locators locators_begin(locator_list.begin());
    Locators locators_end(locator_list.end());

    //Sending through a different IP will NOT work, except 0.0.0.0
    IPLocator::setIPv6(outputChannelLocator, std::string("fe80::ffff:6f6f:6f6f"));
    std::vector<octet> message = { 'H', 'e', 'l', 'l', 'o' };
    ASSERT_FALSE(send_resource_list.at(0)->send(message.data(), (uint32_t)message.size(), &locators_begin,
            &locators_end,
            (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
}

TEST_F(UDPv6Tests, send_to_blocked_interface)
{
    descriptor.interfaceWhiteList.emplace_back("fe80::ffff:6f6f:6f6f");
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    SendResourceList send_resource_list;
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator, std::string("::1")); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_TRUE(send_resource_list.empty());
}

TEST_F(UDPv6Tests, send_to_allowed_interface)
{
    LocatorList_t interfaces;
    if (IPFinder::getAllIPAddress(&interfaces))
    {
        Locator_t locator;
        for (auto& tmpLocator : interfaces)
        {
            if (tmpLocator.kind == LOCATOR_KIND_UDPv6 && IPLocator::toIPv6string(tmpLocator) != "::1")
            {
                locator = tmpLocator;
                break;
            }
        }

        if (IsAddressDefined(locator))
        {
            descriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv6string(locator));
            UDPv6Transport transportUnderTest(descriptor);
            transportUnderTest.init();

            SendResourceList send_resource_list;
            Locator_t outputChannelLocator;
            outputChannelLocator.port = g_default_port;
            outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
            IPLocator::setIPv6(outputChannelLocator, IPLocator::toIPv6string(locator));
            ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
            ASSERT_FALSE(send_resource_list.empty());

            Locator_t remoteMulticastLocator;
            remoteMulticastLocator.port = g_default_port;
            remoteMulticastLocator.kind = LOCATOR_KIND_UDPv6;
            IPLocator::setIPv6(remoteMulticastLocator, std::string("ff1e::ffff:efff:104"));

            LocatorList_t locator_list;
            locator_list.push_back(remoteMulticastLocator);
            Locators locators_begin(locator_list.begin());
            Locators locators_end(locator_list.end());

            // Sending through a ALLOWED IP will work
            std::vector<octet> message = { 'H', 'e', 'l', 'l', 'o' };
            ASSERT_TRUE(send_resource_list.at(0)->send(message.data(), (uint32_t)message.size(),
                    &locators_begin, &locators_end,
                    (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
        }
    }
}

#ifndef __APPLE__
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
                loc.locator.kind = LOCATOR_KIND_UDPv6;
            });
}

TEST_F(UDPv6Tests, send_and_receive_between_allowed_sockets_using_localhost)
{
    descriptor.interfaceWhiteList.emplace_back("::1");
    UDPv6Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t unicastLocator;
    unicastLocator.port = g_default_port;
    unicastLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(unicastLocator, std::string("::1"));

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);

    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
    IPLocator::setIPv6(outputChannelLocator, std::string("::1"));

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
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

TEST_F(UDPv6Tests, send_and_receive_between_allowed_sockets_using_unicast)
{
    std::vector<IPFinder::info_IP> interfaces;
    GetIP6s(interfaces);

    if (interfaces.size() > 0)
    {
        for (const auto& interface : interfaces)
        {
            descriptor.interfaceWhiteList.push_back(interface.name);
        }
        UDPv6Transport transportUnderTest(descriptor);
        transportUnderTest.init();

        Locator_t unicastLocator;
        unicastLocator.port = g_default_port;
        unicastLocator.kind = LOCATOR_KIND_UDPv6;
        IPLocator::setIPv6(unicastLocator, interfaces.at(0).name);

        LocatorList_t locator_list;
        locator_list.push_back(unicastLocator);

        Locator_t outputChannelLocator;
        outputChannelLocator.port = g_default_port + 1;
        outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
        IPLocator::setIPv6(outputChannelLocator, interfaces.at(0).name);

        MockReceiverResource receiver(transportUnderTest, unicastLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

        SendResourceList send_resource_list;
        ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
        ASSERT_FALSE(send_resource_list.empty());
        ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
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
}

TEST_F(UDPv6Tests, send_and_receive_between_allowed_sockets_using_unicast_to_multicast)
{
    std::vector<IPFinder::info_IP> interfaces;
    GetIP6s(interfaces);

    if (interfaces.size() > 0)
    {
        for (const auto& interface : interfaces)
        {
            descriptor.interfaceWhiteList.push_back(interface.name);
        }
        UDPv6Transport transportUnderTest(descriptor);
        transportUnderTest.init();

        Locator_t unicastLocator;
        unicastLocator.port = g_default_port;
        unicastLocator.kind = LOCATOR_KIND_UDPv6;
        IPLocator::setIPv6(unicastLocator, std::string("ff1e::ffff:efff:104"));

        LocatorList_t locator_list;
        locator_list.push_back(unicastLocator);

        Locator_t outputChannelLocator;
        outputChannelLocator.port = g_default_port + 1;
        outputChannelLocator.kind = LOCATOR_KIND_UDPv6;
        IPLocator::setIPv6(outputChannelLocator, interfaces.at(0).name);

        MockReceiverResource receiver(transportUnderTest, unicastLocator);
        MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

        SendResourceList send_resource_list;
        ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
        ASSERT_FALSE(send_resource_list.empty());
        ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
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
}

TEST_F(UDPv6Tests, open_and_close_two_multicast_transports_with_whitelist)
{
    std::vector<IPFinder::info_IP> interfaces;
    GetIP6s(interfaces);

    if (interfaces.size() > 0)
    {
        descriptor.interfaceWhiteList.push_back(interfaces.at(0).name);

        UDPv6Transport transport1(descriptor);
        UDPv6Transport transport2(descriptor);
        transport1.init();
        transport2.init();

        Locator_t multicastLocator;
        multicastLocator.port = g_default_port;
        multicastLocator.kind = LOCATOR_KIND_UDPv6;
        IPLocator::setIPv6(multicastLocator, std::string("ff00::efff:0104"));

        std::cout << "Opening input channels" << std::endl;
        ASSERT_TRUE(transport1.OpenInputChannel(multicastLocator, nullptr, 65500));
        ASSERT_TRUE(transport2.OpenInputChannel(multicastLocator, nullptr, 65500));
        std::cout << "Closing input channel on transport 1" << std::endl;
        ASSERT_TRUE(transport1.CloseInputChannel(multicastLocator));
        std::cout << "Closing input channel on transport 2" << std::endl;
        ASSERT_TRUE(transport2.CloseInputChannel(multicastLocator));
    }
}
#endif // ifndef __APPLE__

TEST_F(UDPv6Tests, open_a_blocked_socket)
{
    std::vector<IPFinder::info_IP> ip_list;
    IPFinder::getIPs(&ip_list);

    for (const IPFinder::info_IP& ip : ip_list)
    {
        if (IPFinder::IP6 == ip.type)
        {
            descriptor.interfaceWhiteList.emplace_back("::1");
            UDPv6Transport transportUnderTest(descriptor);
            transportUnderTest.init();

            Locator_t multicastLocator;
            multicastLocator.port = g_default_port;
            multicastLocator.kind = LOCATOR_KIND_UDPv6;
            IPLocator::setIPv6(multicastLocator, ip.name);

            MockReceiverResource receiver(transportUnderTest, multicastLocator);
            ASSERT_FALSE(transportUnderTest.IsInputChannelOpen(multicastLocator));
            break;
        }
    }
}

TEST_F(UDPv6Tests, simple_throughput)
{
    const size_t sample_size = 1024;
    int num_samples_per_batch = 100000;

    std::atomic<int> samples_received(0);

    Semaphore sem_end_subscriber;

    octet sample_data[sample_size];
    memset(sample_data, 0, sizeof(sample_data));

    Locator_t sub_locator;
    sub_locator.kind = LOCATOR_KIND_UDPv6;
    sub_locator.port = 50000;
    IPLocator::setIPv6(sub_locator, std::string("::1"));

    UDPv6TransportDescriptor my_descriptor;

    // Subscriber

    UDPv6Transport sub_transport(my_descriptor);
    ASSERT_TRUE(sub_transport.init());

    MockReceiverResource sub_receiver(sub_transport, sub_locator);
    MockMessageReceiver* sub_msg_recv = dynamic_cast<MockMessageReceiver*>(sub_receiver.CreateMessageReceiver());

    std::function<void()> sub_callback = [&]()
            {
                samples_received.fetch_add(1);
            };

    sub_msg_recv->setCallback(sub_callback);

    // Publisher

    UDPv6Transport pub_transport(my_descriptor);
    ASSERT_TRUE(pub_transport.init());

    LocatorList_t send_locators_list;
    send_locators_list.push_back(sub_locator);

    SendResourceList send_resource_list;
    ASSERT_TRUE(pub_transport.OpenOutputChannel(send_resource_list, sub_locator));

    auto t0 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_samples_per_batch; i++)
    {
        Locators locators_begin(send_locators_list.begin());
        Locators locators_end(send_locators_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(sample_data, sizeof(sample_data), &locators_begin, &locators_end,
                (std::chrono::steady_clock::now() + std::chrono::milliseconds(100))));
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    auto real_samples_received = samples_received.load();
    printf("Samples [sent,received] [%d,%d] send_time_per_sample %.3f(us)\n"
            , num_samples_per_batch
            , real_samples_received
            , std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / (num_samples_per_batch * 1000.0));
}

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
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
