// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <fastrtps/log/Log.h>
#include <MockReceiverResource.h>
#include "../../../src/cpp/rtps/transport/shared_mem/SharedMemSenderResource.hpp"
#include "../../../src/cpp/rtps/transport/shared_mem/SharedMemManager.hpp"

#include <memory>
#include <gtest/gtest.h>
#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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

class SharedMemTests: public ::testing::Test
{
    public:
        SharedMemTests()
        {
            Log::SetVerbosity(Log::Kind::Info);
            HELPER_SetDescriptorDefaults();
        }

        ~SharedMemTests()
        {
            Log::KillThread();
        }

        void HELPER_SetDescriptorDefaults();

        SharedMemTransportDescriptor descriptor;
        std::unique_ptr<std::thread> senderThread;
        std::unique_ptr<std::thread> receiverThread;
        std::unique_ptr<std::thread> timeoutThread;
};

TEST_F(SharedMemTests, locators_with_kind_16_supported)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t supportedLocator;
    supportedLocator.kind = LOCATOR_KIND_SHMEM;
    Locator_t unsupportedLocatorTcpv4;
    unsupportedLocatorTcpv4.kind = LOCATOR_KIND_TCPv4;
    Locator_t unsupportedLocatorTcpv6;
    unsupportedLocatorTcpv6.kind = LOCATOR_KIND_TCPv6;
    Locator_t unsupportedLocatorUdpv4;
    unsupportedLocatorUdpv4.kind = LOCATOR_KIND_UDPv4;
    Locator_t unsupportedLocatorUdpv6;
    unsupportedLocatorUdpv6.kind = LOCATOR_KIND_UDPv6;

    // Then
    ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorTcpv4));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorTcpv6));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorUdpv4));
    ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocatorUdpv6));
}
/*
TEST_F(SharedMemTests, opening_and_closing_output_channel)
{
    // Given
    SharedMemTransport transportUnderTest(descriptorOnlyOutput);
    transportUnderTest.init();

    Locator_t genericOutputChannelLocator;
    genericOutputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    genericOutputChannelLocator.port = g_output_port; // arbitrary
    SendResourceList send_resource_list;

    // Then
    ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, genericOutputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
    send_resource_list.clear();
    ASSERT_FALSE(transportUnderTest.is_output_channel_open_for(genericOutputChannelLocator));
}
*/
TEST_F(SharedMemTests, opening_and_closing_input_channel)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t genericInputChannelLocator;
    genericInputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    genericInputChannelLocator.port = g_input_port; // listen port

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(genericInputChannelLocator, nullptr, 0x00FF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
}

TEST_F(SharedMemTests, closing_input_channel_leaves_other_channels_unclosed)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t genericInputChannelLocator;
    genericInputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    genericInputChannelLocator.port = g_input_port; // listen port

    Locator_t otherInputChannelLocator;
    otherInputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    otherInputChannelLocator.port = g_input_port + 1; // listen port

    // Then
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(genericInputChannelLocator, nullptr, 0x00FF));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(otherInputChannelLocator, nullptr, 0x00FF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(otherInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(otherInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(otherInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(otherInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(otherInputChannelLocator));
}

TEST_F(SharedMemTests, RemoteToMainLocal_returns_input_locator)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_SHMEM;
    remote_locator.port = g_default_port;

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remote_locator);

    // Then
    ASSERT_EQ(mainLocalLocator, remote_locator);
}

TEST_F(SharedMemTests, transform_remote_locator_returns_input_locator)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_SHMEM;
    remote_locator.port = g_default_port;

    // Then
    Locator_t otherLocator;
    ASSERT_TRUE(transportUnderTest.transform_remote_locator(remote_locator, otherLocator));
    ASSERT_EQ(otherLocator, remote_locator);
}

TEST_F(SharedMemTests, all_shared_mem_locators_are_local)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t shared_mem_locator;
    shared_mem_locator.kind = LOCATOR_KIND_SHMEM;
    shared_mem_locator.port = g_default_port;

    // Then
    ASSERT_TRUE(transportUnderTest.is_local_locator(shared_mem_locator));
}

TEST_F(SharedMemTests, match_if_port_AND_address_matches)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t locatorAlpha;
    locatorAlpha.kind = LOCATOR_KIND_SHMEM;
    locatorAlpha.port = g_default_port;

    Locator_t locatorBeta;
    locatorBeta.kind = LOCATOR_KIND_SHMEM;
    locatorBeta.port = g_default_port;

    // Then
    ASSERT_TRUE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));

    locatorBeta.port = g_default_port + 1;
    // Then
    ASSERT_FALSE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));
}

TEST_F(SharedMemTests, send_and_receive_between_ports)
{
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHMEM;
    unicastLocator.port = g_default_port;

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    outputChannelLocator.port = g_default_port + 1;

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
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

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);

    auto sendThreadFunction = [&]()
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, 5, &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    };
    senderThread.reset(new std::thread(sendThreadFunction));

    sem.wait();
    senderThread->join();
}

TEST_F(SharedMemTests, port_and_segment_overflow_fail)
{
    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::FAIL;
    my_descriptor.segment_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::FAIL;
    my_descriptor.segment_size = 16;
    my_descriptor.port_queue_capacity = 4;

    SharedMemTransport transportUnderTest(my_descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHMEM;
    unicastLocator.port = g_default_port;

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    Semaphore sem;
    bool is_first_message_received = false;
    std::function<void()> recCallback = [&]()
    {
        is_first_message_received = true;
        sem.wait();
    };
    msg_recv->setCallback(recCallback);

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    outputChannelLocator.port = g_default_port + 1;

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    octet message[4] = { 'H','e','l','l'};

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);

    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());
        // Internally the segment is bigger than "my_descriptor.segment_size" so a bigger buffer is tried 
        // to cause segment overflow
        octet message_big[4096] = { 'H','e','l','l'};

        EXPECT_FALSE(send_resource_list.at(0)->send(message_big, sizeof(message_big), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    }

    // At least 4 msgs of 4 bytes are allowed
    for(int i=0;i<4;i++)
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        // At least 4 msgs of 4 bytes are allowed
        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    }

    // Wait until the receiver get the first message
    while(!is_first_message_received)
    {
        std::this_thread::yield();
    }

    // The receiver has poped a message so now 3 messages are in the
    // port's queue

    // Push a 4th should go well
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    } 

    // Push a 5th will cause port overflow
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_FALSE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    } 

    sem.disable();
        
}

TEST_F(SharedMemTests, port_and_segment_overflow_discard)
{
    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::DISCARD;
    my_descriptor.segment_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::DISCARD;
    my_descriptor.segment_size = 16;
    my_descriptor.port_queue_capacity = 4;

    SharedMemTransport transportUnderTest(my_descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHMEM;
    unicastLocator.port = g_default_port;

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    Semaphore sem;
    bool is_first_message_received = false;
    std::function<void()> recCallback = [&]()
    {
        is_first_message_received = true;
        sem.wait();
    };
    msg_recv->setCallback(recCallback);

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    outputChannelLocator.port = g_default_port + 1;

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    octet message[4] = { 'H','e','l','l'};

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);

    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());
        // Internally the segment is bigger than "my_descriptor.segment_size" so a bigger buffer is tried 
        // to cause segment overflow
        octet message_big[4096] = { 'H','e','l','l'};

        EXPECT_TRUE(send_resource_list.at(0)->send(message_big, sizeof(message_big), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    }

    // At least 4 msgs of 4 bytes are allowed
    for(int i=0;i<4;i++)
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        // At least 4 msgs of 4 bytes are allowed
        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    }

    // Wait until the receiver get the first message
    while(!is_first_message_received)
    {
        std::this_thread::yield();
    }

    // The receiver has poped a message so now 3 messages are in the
    // port's queue

    // Push a 4th should go well
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    } 

    // Push a 5th will not cause overflow
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    } 

    sem.disable();
}

TEST_F(SharedMemTests, port_overflow_wait)
{
    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::WAIT;
    // segment overflow WAIT is not supported
    my_descriptor.segment_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::FAIL;
    my_descriptor.segment_size = 16;
    my_descriptor.port_queue_capacity = 4;

    SharedMemTransport transportUnderTest(my_descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHMEM;
    unicastLocator.port = g_default_port;

    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    Semaphore sem;
    bool is_first_message_received = false;
    std::function<void()> recCallback = [&]()
    {
        is_first_message_received = true;
        sem.wait();
    };
    msg_recv->setCallback(recCallback);

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHMEM;
    outputChannelLocator.port = g_default_port + 1;

    SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    octet message[4] = { 'H','e','l','l'};

    LocatorList_t locator_list;
    locator_list.push_back(unicastLocator);

    // At least 4 msgs of 4 bytes are allowed
    for(int i=0;i<4;i++)
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        // At least 4 msgs of 4 bytes are allowed
        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    }

    // Wait until the receiver get the first message
    while(!is_first_message_received)
    {
        std::this_thread::yield();
    }

    // The receiver has poped a message so now 3 messages are in the
    // port's queue

    // Push a 4th should go well
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::milliseconds(100))));
    } 

    // Push a 5th will wait 1(s) and then fail
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        auto t1 = std::chrono::high_resolution_clock::now();

        EXPECT_FALSE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::seconds(1))));

        EXPECT_TRUE(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - t1).count() >= 1000);
    } 

    sem.disable();
}

void SharedMemTests::HELPER_SetDescriptorDefaults()
{
    
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
