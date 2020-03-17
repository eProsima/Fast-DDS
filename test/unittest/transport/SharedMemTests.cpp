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
#include <SharedMemGlobalMock.hpp>
#include "../../../src/cpp/rtps/transport/shared_mem/SharedMemSenderResource.hpp"
#include "../../../src/cpp/rtps/transport/shared_mem/SharedMemManager.hpp"
#include "../../../src/cpp/rtps/transport/shared_mem/SharedMemGlobal.hpp"
#include "../../../src/cpp/rtps/transport/shared_mem/MultiProducerConsumerRingBuffer.hpp"

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

class SHMTransportTests: public ::testing::Test
{
    public:
        SHMTransportTests()
        {
            eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Info);
        }

        ~SHMTransportTests()
        {
            eprosima::fastdds::dds::Log::Flush();
            eprosima::fastdds::dds::Log::KillThread();
        }

        SharedMemTransportDescriptor descriptor;
};

class SHMRingBuffer : public ::testing::Test
{
protected:

    struct MyData
    {
        uint32_t thread_number;
        uint32_t counter;
    };

    std::unique_ptr<MultiProducerConsumerRingBuffer<MyData>::Cell[]> cells_;
    std::unique_ptr<MultiProducerConsumerRingBuffer<MyData> > ring_buffer_;
    uint32_t buffer_size_;

    SHMRingBuffer()
        : buffer_size_((std::max)((unsigned int) 4, std::thread::hardware_concurrency()))
    {
    }

    void SetUp() override
    {
        cells_ = std::unique_ptr<MultiProducerConsumerRingBuffer<MyData>::Cell[]>(
            new MultiProducerConsumerRingBuffer<MyData>::Cell[buffer_size_]);

        ring_buffer_ =
                std::unique_ptr<MultiProducerConsumerRingBuffer<MyData> >(new MultiProducerConsumerRingBuffer<MyData>(
                            cells_.get(), buffer_size_));
    }

    void TearDown() override
    {
        ring_buffer_.reset();
        cells_.reset();
    }
};

class SHMRingBufferMultiThread
    :   public SHMRingBuffer,
    public testing::WithParamInterface<std::tuple<uint32_t, uint32_t, uint32_t> >
{

};

TEST_F(SHMRingBuffer, test_read_write_bounds)
{
    auto listener = ring_buffer_->register_listener();

    for (uint32_t i = 0; i<buffer_size_; i++){
        ring_buffer_->push({0,i});
    }

    ASSERT_THROW(ring_buffer_->push({0,(std::numeric_limits<uint32_t>::max)()}), std::exception);

    for (uint32_t i = 0; i<buffer_size_; i++)
    {
        EXPECT_EQ(listener->head()->data().counter, i);
        listener->pop();
    }

    ASSERT_THROW(listener->pop(), std::exception);
}

TEST_F(SHMRingBuffer, circular_pointer)
{
    uint32_t r = 0;
    uint32_t w = 0;
    uint32_t i = 0;

    auto listener = ring_buffer_->register_listener();

    // Buffer full
    for (; i<buffer_size_; i++){
        ring_buffer_->push({0,w++});
    }

    i = (i % buffer_size_);
    ASSERT_EQ(i, 0u);

    // Another cicle
    for (; i<buffer_size_; i++)
    {
        ASSERT_EQ(listener->head()->data().counter, r++);
        listener->pop();
        ring_buffer_->push({0,w++});
    }

    i = (i % buffer_size_);
    ASSERT_EQ(i, 0u);

    // Flush the buffer
    for (; i<buffer_size_; i++)
    {
        ASSERT_EQ(listener->head()->data().counter, r++);
        listener->pop();
    }

    // Is empty
    ASSERT_THROW(listener->pop(), std::exception);
}

TEST_F(SHMRingBuffer, one_listener_reads_all)
{
    auto listener1 = ring_buffer_->register_listener();
    auto listener2 = ring_buffer_->register_listener();

    for (uint32_t i=0; i<buffer_size_; i++)
    {
        ring_buffer_->push({0,i});
    }

    for (uint32_t i=0; i<buffer_size_; i++)
    {
        listener1->pop();
    }

    ASSERT_EQ(listener1->head(), nullptr);
}

TEST_F(SHMRingBuffer, listeners_register_unregister)
{
    // 0 Must be discarted because no listeners
    ring_buffer_->push({0,0});

    auto listener1 = ring_buffer_->register_listener();
    // 1 Must be only read by listener1
    ring_buffer_->push({0,1});

    auto listener2 = ring_buffer_->register_listener();
    // 2 Must be read by listener1 and listener 2
    ring_buffer_->push({0,2});

    // 3
    ring_buffer_->push({0,3});

    ASSERT_EQ(listener1->head()->data().counter, 1u);
    ASSERT_EQ(listener2->head()->data().counter, 2u);

    listener1->pop(); // 1:1
    ASSERT_EQ(listener1->head()->data().counter, 2u);

    listener2->pop(); // 2:2

    // Listener 1 must decrease ref_counter of 2 and 3 in its destructor
    listener1.reset();

    // 4
    ring_buffer_->push({0,4});

    ASSERT_EQ(listener2->head()->data().counter, 3u);
    listener2->pop(); // 3

    ASSERT_EQ(listener2->head()->data().counter, 4u);
    listener2->pop();
}

TEST_P(SHMRingBufferMultiThread, multiple_writers_listeners)
{
    const uint32_t elements_to_push = buffer_size_ * std::get<1>(GetParam());
    std::vector<std::thread> threads;
    std::atomic<uint32_t> ready_listeners;
    ready_listeners.store(0);

    uint32_t num_listeners_writters = std::get<0>(GetParam());
    uint32_t num_register_unregister = std::get<2>(GetParam());

    for (uint32_t i = 0; i < num_listeners_writters; i++)
    {
        // Listeners
        threads.emplace_back(
            std::thread(
                [&]()
        {
            std::vector<uint32_t> read_counters(num_listeners_writters, (std::numeric_limits<uint32_t>::max)());
            MultiProducerConsumerRingBuffer<MyData>::Cell* cell;

            auto listener = ring_buffer_->register_listener();
            ready_listeners.fetch_add(1);

            do
            {
                // poll until there's data
                while (nullptr == (cell = listener->head()));

                ASSERT_EQ(++read_counters[cell->data().thread_number], cell->data().counter);

                listener->pop();

            } while (cell->data().counter != elements_to_push-1);

        }));
    }

    // Wait until all listeners ready
    while (ready_listeners.load(std::memory_order_relaxed) != num_listeners_writters)
    {
        std::this_thread::yield();
    }

    for (uint32_t i = 0; i < num_listeners_writters; i++)
    {
        // Writers
        threads.emplace_back(
            std::thread(
                [&](uint32_t thread_number)
        {
            for (uint32_t c = 0; c < elements_to_push; c++)
            {
                bool success = false;
                while (!success)
                {
                    try
                    {
                        ring_buffer_->push({thread_number, c});
                        success = true;
                    }
                    catch (const std::exception&)
                    {
                    }
                }
            }
        }, i));
    }

    for (uint32_t i = 0; i < num_register_unregister; i++)
    {
        // Register-Unregister
        threads.emplace_back(
            std::thread(
                [&]()
        {
            auto listener = ring_buffer_->register_listener();

            // Reads two times the size values
            for (uint32_t i=0; i<buffer_size_*2; i++)
            {
                // poll until there's data
                while (!listener->head());
                listener->pop();
            }

            // Unregister
            listener.reset();

        }));
    }

    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

TEST_F(SHMTransportTests, locators_with_kind_16_supported)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t supportedLocator;
    supportedLocator.kind = LOCATOR_KIND_SHM;
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

TEST_F(SHMTransportTests, opening_and_closing_input_channel)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t genericInputChannelLocator;
    genericInputChannelLocator.kind = LOCATOR_KIND_SHM;
    genericInputChannelLocator.port = g_input_port; // listen port

    // Then
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.OpenInputChannel(genericInputChannelLocator, nullptr, 0x00FF));
    ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_TRUE  (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(genericInputChannelLocator));
    ASSERT_FALSE (transportUnderTest.CloseInputChannel(genericInputChannelLocator));
}

TEST_F(SHMTransportTests, closing_input_channel_leaves_other_channels_unclosed)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t genericInputChannelLocator;
    genericInputChannelLocator.kind = LOCATOR_KIND_SHM;
    genericInputChannelLocator.port = g_input_port; // listen port

    Locator_t otherInputChannelLocator;
    otherInputChannelLocator.kind = LOCATOR_KIND_SHM;
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

TEST_F(SHMTransportTests, RemoteToMainLocal_returns_input_locator)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_SHM;
    remote_locator.port = g_default_port;

    // When
    Locator_t mainLocalLocator = transportUnderTest.RemoteToMainLocal(remote_locator);

    // Then
    ASSERT_EQ(mainLocalLocator, remote_locator);
}

TEST_F(SHMTransportTests, transform_remote_locator_returns_input_locator)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t remote_locator;
    remote_locator.kind = LOCATOR_KIND_SHM;
    remote_locator.port = g_default_port;

    // Then
    Locator_t otherLocator;
    ASSERT_TRUE(transportUnderTest.transform_remote_locator(remote_locator, otherLocator));
    ASSERT_EQ(otherLocator, remote_locator);
}

TEST_F(SHMTransportTests, all_shared_mem_locators_are_local)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t shared_mem_locator;
    shared_mem_locator.kind = LOCATOR_KIND_SHM;
    shared_mem_locator.port = g_default_port;

    // Then
    ASSERT_TRUE(transportUnderTest.is_local_locator(shared_mem_locator));
}

TEST_F(SHMTransportTests, match_if_port_and_address_matches)
{
    // Given
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t locatorAlpha;
    locatorAlpha.kind = LOCATOR_KIND_SHM;
    locatorAlpha.port = g_default_port;

    Locator_t locatorBeta;
    locatorBeta.kind = LOCATOR_KIND_SHM;
    locatorBeta.port = g_default_port;

    // Then
    ASSERT_TRUE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));

    locatorBeta.port = g_default_port + 1;
    // Then
    ASSERT_FALSE(transportUnderTest.DoInputLocatorsMatch(locatorAlpha, locatorBeta));
}

TEST_F(SHMTransportTests, send_and_receive_between_ports)
{
    SharedMemTransport transportUnderTest(descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHM;
    unicastLocator.port = g_default_port;

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHM;
    outputChannelLocator.port = g_default_port + 1;

    Semaphore sem;
    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    eprosima::fastrtps::rtps::SendResourceList send_resource_list;
    ASSERT_TRUE(transportUnderTest.OpenOutputChannel(send_resource_list, outputChannelLocator));
    ASSERT_FALSE(send_resource_list.empty());
    ASSERT_TRUE(transportUnderTest.IsInputChannelOpen(unicastLocator));
    octet message[5] = { 'H','e','l','l','o' };

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
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    };

    std::unique_ptr<std::thread> sender_thread;
    sender_thread.reset(new std::thread(sendThreadFunction));

    sem.wait();
    sender_thread->join();
}

TEST_F(SHMTransportTests, port_and_segment_overflow_fail)
{
    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::FAIL;
    my_descriptor.segment_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::FAIL;
    my_descriptor.segment_size = 16;
    my_descriptor.port_queue_capacity = 4;

    SharedMemTransport transportUnderTest(my_descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHM;
    unicastLocator.port = g_default_port;

    Semaphore sem;
    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    uint32_t messages_received = 0;
    std::function<void()> recCallback = [&]()
    {
        // At the second message block
        if(messages_received > 0)
        {
            messages_received++;
            sem.wait();
        }
        else
        {
            messages_received++;
        }
    };
    msg_recv->setCallback(recCallback);

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHM;
    outputChannelLocator.port = g_default_port + 1;

    eprosima::fastrtps::rtps::SendResourceList send_resource_list;
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

        ASSERT_FALSE(send_resource_list.at(0)->send(message_big, sizeof(message_big), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    }

    // At least 4 msgs of 4 bytes are allowed
    for(int i=0;i<4;i++)
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());
 
        // At least 4 msgs of 4 bytes are allowed
        ASSERT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now() + std::chrono::microseconds(100))));
    }

    // Wait until the receiver get the first message
    while(messages_received == 0)
    {
        std::this_thread::yield();
    }

    // The receiver has poped a message so now 3 messages are in the
    // port's queue

    // Push a 4th should go well
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        ASSERT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    } 

    // Push a 5th will cause port overflow
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        ASSERT_FALSE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    } 

    sem.disable(); 
}

TEST_F(SHMTransportTests, port_and_segment_overflow_discard)
{
    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::DISCARD;
    my_descriptor.segment_overflow_policy = SharedMemTransportDescriptor::OverflowPolicy::DISCARD;
    my_descriptor.segment_size = 16;
    my_descriptor.port_queue_capacity = 4;

    SharedMemTransport transportUnderTest(my_descriptor);
    ASSERT_TRUE(transportUnderTest.init());

    Locator_t unicastLocator;
    unicastLocator.kind = LOCATOR_KIND_SHM;
    unicastLocator.port = g_default_port;

    Semaphore sem;
    MockReceiverResource receiver(transportUnderTest, unicastLocator);
    MockMessageReceiver *msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

    bool is_first_message_received = false;
    std::function<void()> recCallback = [&]()
    {
        is_first_message_received = true;
        sem.wait();
    };
    msg_recv->setCallback(recCallback);

    Locator_t outputChannelLocator;
    outputChannelLocator.kind = LOCATOR_KIND_SHM;
    outputChannelLocator.port = g_default_port + 1;

    eprosima::fastrtps::rtps::SendResourceList send_resource_list;
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
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    }

    // At least 4 msgs of 4 bytes are allowed
    for(int i=0;i<4;i++)
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        // At least 4 msgs of 4 bytes are allowed
        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
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
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    } 

    // Push a 5th will not cause overflow
    {
        Locators locators_begin(locator_list.begin());
        Locators locators_end(locator_list.end());

        EXPECT_TRUE(send_resource_list.at(0)->send(message, sizeof(message), &locators_begin, &locators_end,
            (std::chrono::steady_clock::now()+ std::chrono::microseconds(100))));
    } 

    sem.disable();
}

TEST_F(SHMTransportTests, port_mutex_deadlock_recover)
{
    const std::string domain_name("SHMTransportTests");

    SharedMemGlobal shared_mem_global(domain_name);
    MockPortSharedMemGlobal port_mocker;

    port_mocker.remove_port_mutex(domain_name, 0);

    auto global_port = shared_mem_global.open_port(0, 1, 1000);

	Semaphore sem_lock_done;
	Semaphore sem_end_thread_locker;
	std::thread thread_locker([&] 
		{
			// lock has to be done in another thread because
			// boost::inteprocess_named_mutex and  interprocess_mutex are recursive in Win32
			auto port_mutex = port_mocker.get_port_mutex(domain_name, 0);
			ASSERT_TRUE(port_mutex->try_lock());
			sem_lock_done.post();
			sem_end_thread_locker.wait();
		}
	);

	sem_lock_done.wait();

	auto port_mutex = port_mocker.get_port_mutex(domain_name, 0);
	ASSERT_FALSE(port_mutex->try_lock());

    auto global_port2 = shared_mem_global.open_port(0, 1, 1000);

    ASSERT_NO_THROW(global_port2->healthy_check(1000));

	sem_end_thread_locker.post();
	thread_locker.join();
}

TEST_F(SHMTransportTests, empty_cv_mutex_deadlocked_try_push)
{
    const std::string domain_name("SHMTransportTests");

    SharedMemGlobal shared_mem_global(domain_name);
    MockPortSharedMemGlobal port_mocker;

    auto global_port = shared_mem_global.open_port(0, 1, 1000);

	Semaphore sem_lock_done;
	Semaphore sem_end_thread_locker;
	std::thread thread_locker([&]
		{
			// lock has to be done in another thread because
			// boost::inteprocess_named_mutex and  interprocess_mutex are recursive in Win32
			ASSERT_TRUE(port_mocker.lock_empty_cv_mutex(*global_port));
			sem_lock_done.post();
			sem_end_thread_locker.wait();
		}
	);

	sem_lock_done.wait();
    
    ASSERT_FALSE(port_mocker.lock_empty_cv_mutex(*global_port));

    bool listerner_active;
    SharedMemGlobal::BufferDescriptor foo;
    ASSERT_THROW(global_port->try_push(foo, &listerner_active), std::exception);

    ASSERT_THROW(global_port->healthy_check(1000), std::exception);

	sem_end_thread_locker.post();
	thread_locker.join();
}

TEST_F(SHMTransportTests, dead_listener_port_recover)
{
    const std::string domain_name("SHMTransportTests");

    SharedMemGlobal shared_mem_global(domain_name);
    auto deadlocked_port = shared_mem_global.open_port(0, 1, 1000);
    auto deadlocked_listener = deadlocked_port->create_listener();
    
    // Simulates a deadlocked wait_pop
    std::atomic_bool is_listener_closed(false);
    std::thread thread_wait_deadlock([&] 
        {
            MockPortSharedMemGlobal port_mocker;
            port_mocker.wait_pop_deadlock(*deadlocked_port, *deadlocked_listener, is_listener_closed);                
            (void)port_mocker; // Removes an inexplicable warning when compiling with VC(v140 toolset)
        });
    
    // Assert the thread is waiting
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Open the deadlocked port
    auto port = shared_mem_global.open_port(0, 1, 1000);
    auto listener = port->create_listener();
    bool listerners_active;
    SharedMemSegment::Id random_id;
    SharedMemGlobal::BufferDescriptor foo = {random_id, 0};
    ASSERT_TRUE(port->try_push(foo, &listerners_active));
    ASSERT_TRUE(listerners_active);
    ASSERT_TRUE(listener->head() != nullptr);
    ASSERT_TRUE(listener->head()->data().source_segment_id == random_id);
    ASSERT_TRUE(listener->pop());

    deadlocked_port->close_listener(&is_listener_closed);
    thread_wait_deadlock.join();
}

/*TEST_F(SHMTransportTests, simple_latency)
{
	int num_samples = 1000;
	char data[16] = { "" };

	std::thread thread_subscriber([&]
		{
			SharedMemManager shared_mem_manager("SHMTransportTests");
			auto port_pub_to_sub = shared_mem_manager.open_port(0, 64, 1000);
			auto port_sub_to_pub = shared_mem_manager.open_port(1, 64, 1000);
			auto listener_sub = port_pub_to_sub->create_listener();

			auto segment = shared_mem_manager.create_segment(sizeof(data)*64,64);
			int i = num_samples;

			do
			{
				auto recv_sample = listener_sub->pop();

				auto sample_to_send = segment->alloc_buffer(sizeof(data));
				memcpy(sample_to_send->data(), data, sizeof(data));
				ASSERT_TRUE(port_sub_to_pub->try_push(sample_to_send));
			} while (--i);
		});

	std::thread thread_publisher([&]
		{
			SharedMemManager shared_mem_manager("SHMTransportTests");
			auto port_pub_to_sub = shared_mem_manager.open_port(0, 64, 1000);
			auto port_sub_to_pub = shared_mem_manager.open_port(1, 64, 1000);
			auto listener_pub = port_sub_to_pub->create_listener();

			auto segment = shared_mem_manager.create_segment(sizeof(data) * 64, 64);
			int i = num_samples;

			std::chrono::high_resolution_clock::rep total_times = 0;
			std::chrono::high_resolution_clock::rep min_time = (std::numeric_limits<std::chrono::high_resolution_clock::rep>::max)();
			std::chrono::high_resolution_clock::rep max_time = (std::numeric_limits<std::chrono::high_resolution_clock::rep>::min)();

			while (i--)
			{
				auto t0 = std::chrono::high_resolution_clock::now();

				auto sample_to_send = segment->alloc_buffer(sizeof(data));
				memcpy(sample_to_send->data(), data, sizeof(data));
				ASSERT_TRUE(port_pub_to_sub->try_push(sample_to_send));
				sample_to_send.reset();

				auto recv_sample = listener_pub->pop();

				auto t1 = std::chrono::high_resolution_clock::now();

				auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
				if (t < min_time)
				{
					min_time = t;
				}
				if (t > max_time)
				{
					max_time = t;
				}
				total_times += t;
			}

			printf("LatencyTest for %d samples. Avg = %.3f(us) Min = %.3f(us) Max = %.3f(us)\n", num_samples, total_times / (num_samples*1000.0), min_time/1000.0, max_time/1000.0);
		});

	thread_subscriber.join();
	thread_publisher.join();
}*/

/*TEST_F(SHMTransportTests, simple_latency2)
{
	int num_samples = 1000;
	octet data[16] = { "" };

	Locator_t sub_locator;
	sub_locator.kind = LOCATOR_KIND_SHM;
	sub_locator.port = 0;

	Locator_t pub_locator;
	pub_locator.kind = LOCATOR_KIND_SHM;
	pub_locator.port = 1;

	SharedMemTransportDescriptor my_descriptor;

	std::thread thread_subscriber([&]
		{
			SharedMemTransport transport(my_descriptor);
			ASSERT_TRUE(transport.init());

            Semaphore sem;
			MockReceiverResource receiver(transport, sub_locator);
			MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

			int samples_to_receive = num_samples;

			LocatorList_t send_locators_list;
			send_locators_list.push_back(pub_locator);

            eprosima::fastrtps::rtps::SendResourceList send_resource_list;
			ASSERT_TRUE(transport.OpenOutputChannel(send_resource_list, pub_locator));

			std::function<void()> sub_callback = [&]()
			{
				Locators locators_begin(send_locators_list.begin());
				Locators locators_end(send_locators_list.end());

				EXPECT_TRUE(send_resource_list.at(0)->send(data, sizeof(data), &locators_begin, &locators_end,
					(std::chrono::steady_clock::now() + std::chrono::milliseconds(100))));

				if (--samples_to_receive == 0)
				{
					sem.post();
				}
			};

			msg_recv->setCallback(sub_callback);

			sem.wait();
		});

	std::thread thread_publisher([&]
		{
			SharedMemTransport transport(my_descriptor);
			ASSERT_TRUE(transport.init());

            Semaphore sem;
			MockReceiverResource receiver(transport, pub_locator);
			MockMessageReceiver* msg_recv = dynamic_cast<MockMessageReceiver*>(receiver.CreateMessageReceiver());

			int samples_sent = 0;

			LocatorList_t send_locators_list;
			send_locators_list.push_back(sub_locator);

            eprosima::fastrtps::rtps::SendResourceList send_resource_list;
			ASSERT_TRUE(transport.OpenOutputChannel(send_resource_list, sub_locator));

			std::chrono::high_resolution_clock::rep total_times = 0;
			std::chrono::high_resolution_clock::rep min_time = (std::numeric_limits<std::chrono::high_resolution_clock::rep>::max)();
			std::chrono::high_resolution_clock::rep max_time = (std::numeric_limits<std::chrono::high_resolution_clock::rep>::min)();

			auto t0 = std::chrono::high_resolution_clock::now();

			Locators locators_begin(send_locators_list.begin());
			Locators locators_end(send_locators_list.end());

			EXPECT_TRUE(send_resource_list.at(0)->send(data, sizeof(data), &locators_begin, &locators_end,
				(std::chrono::steady_clock::now() + std::chrono::milliseconds(100))));

			std::function<void()> pub_callback = [&]()
			{
				if (++samples_sent < num_samples)
				{
					auto t1 = std::chrono::high_resolution_clock::now();

					auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
					if (t < min_time)
					{
						min_time = t;
					}
					if (t > max_time)
					{
						max_time = t;
					}
					total_times += t;

					t0 = std::chrono::high_resolution_clock::now();

					Locators locators_begin(send_locators_list.begin());
					Locators locators_end(send_locators_list.end());

					EXPECT_TRUE(send_resource_list.at(0)->send(data, sizeof(data), &locators_begin, &locators_end,
						(std::chrono::steady_clock::now() + std::chrono::milliseconds(100))));
				}
				else
				{
					sem.post();
				}
			};

			msg_recv->setCallback(pub_callback);

			sem.wait();

			printf("LatencyTest for %d samples. Avg = %.3f(us) Min = %.3f(us) Max = %.3f(us)\n", num_samples, total_times / (num_samples * 1000.0), min_time / 1000.0, max_time / 1000.0);
		});

	thread_subscriber.join();
	thread_publisher.join();
}*/

/*TEST_F(SHMTransportTests, simple_throughput)
{
    const size_t sample_size = 1024;
    int num_samples_per_batch = 100000;

    std::atomic<int> samples_received(0);

    octet sample_data[sample_size];
    memset(sample_data, 0, sizeof(sample_data));

    Locator_t sub_locator;
    sub_locator.kind = LOCATOR_KIND_SHM;
    sub_locator.port = 0;

    Locator_t pub_locator;
    pub_locator.kind = LOCATOR_KIND_SHM;
    pub_locator.port = 1;

    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_queue_capacity = num_samples_per_batch;
    my_descriptor.segment_size = sample_size * num_samples_per_batch;

    // Subscriber

    SharedMemTransport sub_transport(my_descriptor);
    ASSERT_TRUE(sub_transport.init());

    MockReceiverResource sub_receiver(sub_transport, sub_locator);
    MockMessageReceiver* sub_msg_recv = dynamic_cast<MockMessageReceiver*>(sub_receiver.CreateMessageReceiver());

    std::function<void()> sub_callback = [&]()
        {
            samples_received.fetch_add(1);
        };

    sub_msg_recv->setCallback(sub_callback);

    // Publisher

    SharedMemTransport pub_transport(my_descriptor);
    ASSERT_TRUE(pub_transport.init());

    LocatorList_t send_locators_list;
    send_locators_list.push_back(sub_locator);

    eprosima::fastrtps::rtps::SendResourceList send_resource_list;
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
}*/

// This test is linux only
/*TEST_F(SHMTransportTests, simple_throughput_inter)
{
    const size_t sample_size = 1024;
    int num_samples_per_batch = 100000;

    std::atomic<int> samples_received(0);

    octet sample_data[sample_size];
    memset(sample_data, 0, sizeof(sample_data));

    Locator_t sub_locator;
    sub_locator.kind = LOCATOR_KIND_SHM;
    sub_locator.port = 0;

    Locator_t pub_locator;
    pub_locator.kind = LOCATOR_KIND_SHM;
    pub_locator.port = 1;

    SharedMemTransportDescriptor my_descriptor;

    my_descriptor.port_queue_capacity = num_samples_per_batch;
    my_descriptor.segment_size = sample_size * num_samples_per_batch;

    // Child
    if(fork() == 0)
    {
        Semaphore sem_end_subscriber;

        // Subscriber
        SharedMemTransport sub_transport(my_descriptor);
        ASSERT_TRUE(sub_transport.init());

        MockReceiverResource sub_receiver(sub_transport, sub_locator);
        MockMessageReceiver* sub_msg_recv = dynamic_cast<MockMessageReceiver*>(sub_receiver.CreateMessageReceiver());

        std::function<void()> sub_callback = [&]()
            {
                if(samples_received.fetch_add(1)+1 == num_samples_per_batch)
                {
                    sem_end_subscriber.post();
                }
            };

        sub_msg_recv->setCallback(sub_callback);

        sem_end_subscriber.wait();

        printf("Samples [received] [%d]\n", samples_received.load());
    }
    else
    {
        // Give time to the subscriber to listen
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Publisher
        SharedMemTransport pub_transport(my_descriptor);
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

        printf("Samples [sent] [%d] send_time_per_sample %.3f(us)\n"
            , num_samples_per_batch
            , std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count() / (num_samples_per_batch*1000.0));
    }
}*/

/*INSTANTIATE_TEST_CASE_P(
    SHMTransportTests,
    SHMRingBufferMultiThread,
    testing::Values(
        std::make_tuple(
            (std::max)((unsigned int)1, std::thread::hardware_concurrency()/2), 100000, 0),
        std::make_tuple(
            (std::max)((unsigned int)1, std::thread::hardware_concurrency())*2, 100,0),
        std::make_tuple(
            (std::max)((unsigned int)1, std::thread::hardware_concurrency()/2), 100000, std::thread::hardware_concurrency()/2),
        std::make_tuple(
            (std::max)((unsigned int)1, std::thread::hardware_concurrency())*2, 100,std::thread::hardware_concurrency())
    )
);*/

int main(int argc, char **argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
    g_default_port = get_port(4000);
    g_output_port = get_port(5000);
    g_input_port = get_port(5010);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
