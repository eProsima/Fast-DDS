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
//#include <fastdds/dds/log/Log.hpp>
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
    /*
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
    */
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
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
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
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
}
#endif

TEST_F(UDPv4Tests, send_is_rejected_if_buffer_size_is_bigger_to_size_specified_in_descriptor)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    SendResourceList send_resource_list;
    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    genericOutputChannelLocator.port = g_default_port;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, genericOutputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());

    Locator_t destinationLocator;
    destinationLocator.kind = LOCATOR_KIND_UDPv4;
    destinationLocator.port = g_default_port + 1;

    LocatorList_t locator_list;
    locator_list.push_back(destinationLocator);
    Locators locators_begin(locator_list.begin());
    Locators locators_end(locator_list.end());

    // Then
    std::vector<octet> receiveBufferWrongSize(descriptor.sendBufferSize + 1);
    ASSERT_FALSE(send_resource_list.at(0)->send(receiveBufferWrongSize.data(), (uint32_t)receiveBufferWrongSize.size(),
                &locators_begin, &locators_end, (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
}

TEST_F(UDPv4Tests, RemoteToMainLocal_simply_strips_out_address_leaving_IP_ANY)
{
    // Given
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_UDPv4;
    remote_locator.port = g_default_port;
    IPLocator::setIPv4(remote_locator, 222,222,222,222);

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remote_locator);

    ASSERT_EQ(mainLocalLocator.port, remote_locator.port);
    ASSERT_EQ(mainLocalLocator.kind, remote_locator.kind);

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

    SendResourceList send_resource_list;
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());

    LocatorList_t locator_list;
    locator_list.push_back(Locator_t());
    Locators locators_begin(locator_list.begin());
    Locators locators_end(locator_list.end());

    //Sending through a different IP will NOT work, except 0.0.0.0
    IPLocator::setIPv4(outputChannelLocator, 111, 111, 111, 111);
    std::vector<octet> message = { 'H','e','l','l','o' };
    ASSERT_FALSE(send_resource_list.at(0)->send(message.data(), (uint32_t)message.size(), &locators_begin, &locators_end, 
        (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
}

TEST_F(UDPv4Tests, send_to_blocked_interface)
{
    descriptor.interfaceWhiteList.emplace_back("111.111.111.111");
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    SendResourceList send_resource_list;
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, 127, 0, 0, 1); // Loopback
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_TRUE(send_resource_list.empty());
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
            descriptor.interfaceWhiteList.emplace_back(IPLocator::toIPv4string(locator));
            UDPv4Transport transportUnderTest(descriptor);
            transportUnderTest.init();

            SendResourceList send_resource_list;
            Locator_t outputChannelLocator;
            outputChannelLocator.port = g_default_port;
            outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
            IPLocator::setIPv4(outputChannelLocator, IPLocator::toIPv4string(locator));
            ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
            ASSERT_FALSE(send_resource_list.empty());

            Locator_t remoteMulticastLocator;
            remoteMulticastLocator.port = g_default_port;
            remoteMulticastLocator.kind = LOCATOR_KIND_UDPv4;
            IPLocator::setIPv4(remoteMulticastLocator, 239, 255, 1, 4); // Loopback

            LocatorList_t locator_list;
            locator_list.push_back(remoteMulticastLocator);
            Locators locators_begin(locator_list.begin());
            Locators locators_end(locator_list.end());

            // Sending through a ALLOWED IP will work
            std::vector<octet> message = { 'H','e','l','l','o' };
            ASSERT_TRUE(send_resource_list.at(0)->send(message.data(), (uint32_t)message.size(),
                        &locators_begin, &locators_end, (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
        }
    }
}
#ifndef __APPLE__
static void GetIP4s(std::vector<IPFinder::info_IP>& interfaces)
{
    IPFinder::getIPs(&interfaces, false);
    auto new_end = remove_if(interfaces.begin(),
        interfaces.end(),
        [](IPFinder::info_IP ip)
    {
        return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;
    });
    interfaces.erase(new_end, interfaces.end());
    std::for_each(interfaces.begin(), interfaces.end(), [](IPFinder::info_IP& loc)
    {
        loc.locator.kind = LOCATOR_KIND_UDPv4;
    });
}

TEST_F(UDPv4Tests, send_and_receive_between_allowed_sockets_using_localhost)
{
    descriptor.interfaceWhiteList.emplace_back("127.0.0.1");
    UDPv4Transport transportUnderTest(descriptor);
    transportUnderTest.init();

    Locator_t unicastLocator;
    unicastLocator.port = g_default_port;
    unicastLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(unicastLocator, "127.0.0.1");

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);
    
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, "127.0.0.1");

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
    ASSERT_FALSE(send_resource_list.empty());
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
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, 5, &locators_begin, &locators_end, 
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
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

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);
    
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, interfaces.at(0).name);

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
    ASSERT_FALSE(send_resource_list.empty());
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
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, 5, &locators_begin, &locators_end, 
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
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

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);
    
    Locator_t outputChannelLocator;
    outputChannelLocator.port = g_default_port + 1;
    outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
    IPLocator::setIPv4(outputChannelLocator, interfaces.at(0).name);

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator)); // Includes loopback
    ASSERT_FALSE(send_resource_list.empty());
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
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, 5, &locators_begin, &locators_end, 
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    };

    senderThread.reset(new std::thread(sendThreadFunction));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    senderThread->join();
    sem.wait();
}

TEST_F(UDPv4Tests, open_and_close_two_multicast_transports_with_whitelist)
{
    std::vector<IPFinder::info_IP> interfaces;
    GetIP4s(interfaces);

    if (interfaces.size() > 0)
    {
        descriptor.interfaceWhiteList.push_back(interfaces.at(0).name);

        UDPv4Transport transport1(descriptor);
        UDPv4Transport transport2(descriptor);
        transport1.init();
        transport2.init();

        Locator_t multicastLocator;
        multicastLocator.port = g_default_port;
        multicastLocator.kind = LOCATOR_KIND_UDPv4;
        IPLocator::setIPv4(multicastLocator, "239.255.1.4");

        std::cout << "Opening input channels" << std::endl;
        ASSERT_TRUE(transport1.OpenInputChannel(multicastLocator, nullptr, 65500));
        ASSERT_TRUE(transport2.OpenInputChannel(multicastLocator, nullptr, 65500));
        std::cout << "Closing input channel on transport 1" << std::endl;
        ASSERT_TRUE(transport1.CloseInputChannel(multicastLocator));
        std::cout << "Closing input channel on transport 2" << std::endl;
        ASSERT_TRUE(transport2.CloseInputChannel(multicastLocator));
    }
}
#endif

TEST_F(UDPv4Tests, open_a_blocked_socket)
{
    std::vector<IPFinder::info_IP> ip_list;
    IPFinder::getIPs(&ip_list);

    for (const IPFinder::info_IP& ip : ip_list)
    {
        if (IPFinder::IP4 == ip.type)
        {
            descriptor.interfaceWhiteList.emplace_back("127.0.0.1");
            UDPv4Transport transportUnderTest(descriptor);
            transportUnderTest.init();

            Locator_t multicastLocator;
            multicastLocator.port = g_default_port;
            multicastLocator.kind = LOCATOR_KIND_UDPv4;
            IPLocator::setIPv4(multicastLocator, ip.name);

            MockReceiverResource receiver(transportUnderTest, multicastLocator);
            ASSERT_FALSE(transportUnderTest.IsInputChannelOpen(multicastLocator));
            break;
        }
    }
}

TEST_F(UDPv4Tests, simple_throughput)
{
    const size_t sample_size = 1024;
    int num_samples_per_batch = 100000;

    std::atomic<int> samples_received(0);

    Semaphore sem_end_subscriber;

    octet sample_data[sample_size];
    memset(sample_data, 0, sizeof(sample_data));

    Locator_t sub_locator;
    sub_locator.kind = LOCATOR_KIND_UDPv4;
    sub_locator.port = 50000;
    IPLocator::setIPv4(sub_locator, 127, 0, 0, 1);

    UDPv4TransportDescriptor my_descriptor;

    // Subscriber

    UDPv4Transport sub_transport(my_descriptor);
    ASSERT_TRUE(sub_transport.init());

    MockReceiverResource sub_receiver(sub_transport, sub_locator);
    MockMessageReceiver* sub_msg_recv = dynamic_cast<MockMessageReceiver*>(sub_receiver.CreateMessageReceiver());

    std::function<void()> sub_callback = [&]()
        {
            samples_received.fetch_add(1);
        };

    sub_msg_recv->setCallback(sub_callback);

    // Publisher

    UDPv4Transport pub_transport(my_descriptor);
    ASSERT_TRUE(pub_transport.init());

    LocatorList_t send_locators_list;
    send_locators_list.push_back(sub_locator);

    SendResourceList send_resource_list;
    ASSERT_TRUE(pub_transport.OpenOutputChannel(send_resource_list, sub_locator));

    auto t0 = std::chrono::high_resolution_clock::now();

    for (int i=0; i<num_samples_per_batch; i++)
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
        , std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count() / (num_samples_per_batch*1000.0));
}

TEST_F(UDPv4Tests, simple_latency)
{
    /*int num_samples = 10000;
    std::vector<uint32_t> data_sizes = {16, 1024, 1024*64, 1024*512, 1024*1024};*/

    int num_samples = 10000;
    std::vector<uint32_t> data_sizes = {8*1024, 16*1024, 32*1024, 64*1024, 128*1024, 256*1024, 512*1024, 1024*1024};

    struct results
    {
        uint32_t sample_size;
        std::chrono::high_resolution_clock::rep total_times;
        std::chrono::high_resolution_clock::rep min_time;
        std::chrono::high_resolution_clock::rep max_time;
    };

    std::vector<results> data_results;

    for (auto data_size : data_sizes)
    {    
        auto data = std::unique_ptr<octet[]>(new octet[data_size]);

        printf("Starting %u(bytes) measure...\n", data_size);

        Locator_t sub_locator;
        sub_locator.kind = LOCATOR_KIND_UDPv4;
        sub_locator.port = 50000;
        IPLocator::setIPv4(sub_locator, 127, 0, 0, 1);

        Locator_t pub_locator;
        pub_locator.kind = LOCATOR_KIND_UDPv4;
        pub_locator.port = 50001;
        IPLocator::setIPv4(pub_locator, 127, 0, 0, 1);

        UDPv4TransportDescriptor my_descriptor;
        my_descriptor.interfaceWhiteList.push_back("127.0.0.1");
        my_descriptor.receiveBufferSize = (std::max)(data_sizes.back() * 2, my_descriptor.max_message_size());
        my_descriptor.sendBufferSize = (std::max)(data_sizes.back() * 2, my_descriptor.max_message_size());

        uint32_t full_fragments = 1;
        uint32_t fragment_size = data_size;
        uint32_t last_fragment_bytes = 0;

        uint32_t max_message_size = my_descriptor.max_message_size();

        if(data_size > max_message_size)
        {
            fragment_size = max_message_size;
            full_fragments = data_size / max_message_size;
            last_fragment_bytes = data_size % max_message_size;
        }

        uint32_t total_sends = ( full_fragments + ((last_fragment_bytes > 0) ? 1 : 0) ) * num_samples;

        std::thread thread_subscriber([&]
            {
                UDPv4Transport transport(my_descriptor);
                ASSERT_TRUE(transport.init());

                Semaphore sem;
                MockReceiverResource receiver(transport, sub_locator);
                MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

                int buffers_to_receive = total_sends;

                LocatorList_t send_locators_list;
                send_locators_list.push_back(pub_locator);

                eprosima::fastrtps::rtps::SendResourceList send_resource_list;
                ASSERT_TRUE(transport.OpenOutputChannel(send_resource_list, pub_locator));

                std::function<void()> sub_callback = [&]()
                {
                    Locators locators_begin(send_locators_list.begin());
                    Locators locators_end(send_locators_list.end());

                    ASSERT_TRUE(send_resource_list.at(0)->send(msg_recv->data, msg_recv->data_size, &locators_begin, &locators_end,
                        (std::chrono::steady_clock::now() + std::chrono::milliseconds(1000))));

                    if (--buffers_to_receive == 0)
                    {
                        sem.post();
                    }
                };

                msg_recv->setCallback(sub_callback);

                sem.wait();
            });

        std::thread thread_publisher([&]
            {
                UDPv4Transport transport(my_descriptor);
                ASSERT_TRUE(transport.init());

                Semaphore sem;
                MockReceiverResource receiver(transport, pub_locator);
                MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

                int samples_sent = 0;

                LocatorList_t send_locators_list;
                send_locators_list.push_back(sub_locator);

                eprosima::fastrtps::rtps::SendResourceList send_resource_list;
                ASSERT_TRUE(transport.OpenOutputChannel(send_resource_list, sub_locator));

                data_results.push_back(
                    {
                        data_size,
                        0, 
                        (std::numeric_limits<std::chrono::high_resolution_clock::rep>::max)(),
                        (std::numeric_limits<std::chrono::high_resolution_clock::rep>::min)()
                    });

                auto& results = data_results.back();
                
                auto t0 = std::chrono::high_resolution_clock::now();

                Locators locators_begin(send_locators_list.begin());
                Locators locators_end(send_locators_list.end());

                auto my_full_fragments = full_fragments;
                auto my_last_fragment_bytes = last_fragment_bytes;
                
                ASSERT_TRUE(send_resource_list.at(0)->send(data.get(), fragment_size, &locators_begin, &locators_end,
                    (std::chrono::steady_clock::now() + std::chrono::milliseconds(1000))));

                my_full_fragments--;

                std::function<void()> pub_callback = [&]()
                {
                    Locators locators_begin(send_locators_list.begin());
                    Locators locators_end(send_locators_list.end());

                    if(my_full_fragments > 0)
                    {
                        ASSERT_TRUE(send_resource_list.at(0)->send(data.get(), fragment_size, &locators_begin, &locators_end,
                            (std::chrono::steady_clock::now() + std::chrono::milliseconds(1000))));
                        my_full_fragments--;
                    }
                    else
                    {
                        if(my_last_fragment_bytes > 0)
                        {
                            ASSERT_TRUE(send_resource_list.at(0)->send(data.get(), last_fragment_bytes, &locators_begin, &locators_end,
                            (std::chrono::steady_clock::now() + std::chrono::milliseconds(1000))));

                            my_last_fragment_bytes = 0;
                        }
                        else
                        {
                            auto t1 = std::chrono::high_resolution_clock::now();

                            auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 2;
                            if (t < results.min_time)
                            {
                                results.min_time = t;
                            }
                            if (t > results.max_time)
                            {
                                results.max_time = t;
                            }
                            results.total_times += t;

                            samples_sent++;
                            t0 = std::chrono::high_resolution_clock::now();

                            my_full_fragments = full_fragments;
                            my_last_fragment_bytes = last_fragment_bytes;

                            if(samples_sent < num_samples)
                            {
                                ASSERT_TRUE(send_resource_list.at(0)->send(data.get(), fragment_size, &locators_begin, &locators_end,
                                (std::chrono::steady_clock::now() + std::chrono::milliseconds(1000))));
                                my_full_fragments--;
                                
                            }
                            else
                            {
                                sem.post();
                            }
                        }
                    }
                };

                msg_recv->setCallback(pub_callback);

                sem.wait();
            });

        thread_subscriber.join();
        thread_publisher.join();
    }

    for(const auto& results : data_results)
    {
        printf("LatencyTest for %d samples of %08d(bytes). Avg = %.3f(us) Min = %.3f(us) Max = %.3f(us)\n", num_samples, results.sample_size, results.total_times / (num_samples * 1000.0), results.min_time / 1000.0, results.max_time / 1000.0);
    }
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
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Warning);
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

