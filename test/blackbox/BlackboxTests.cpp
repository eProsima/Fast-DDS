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

#include "types/HelloWorld.h"
#include "types/Data64kbType.h"
#include "types/Data1mbType.h"

#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/flowcontrol/ThroughputController.h>
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/common/Locator.h>

#include <thread>
#include <memory>
#include <gtest/gtest.h>

#define TEST_TOPIC_NAME std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())

uint32_t global_port = 0;

class BlackboxEnvironment : public ::testing::Environment
{
    public:

        void SetUp()
        {
            global_port = boost::interprocess::ipcdetail::get_current_process_id();

            if(global_port + 7400 > global_port)
                global_port += 7400;
        }

        void TearDown()
        {
            Log::Reset();
            eprosima::fastrtps::rtps::RTPSDomain::stopAll();
        }
};

/****** Auxiliary data generators *******/
std::list<HelloWorld> default_helloword_data_generator(size_t max = 0)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 100;
    std::list<HelloWorld> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index] {
            HelloWorld hello;
            hello.index(index);
            std::stringstream ss;
            ss << "HelloWorld " << index;
            hello.message(ss.str());
            ++index;
            return hello;
            });

    return returnedValue;
}

const size_t data64kb_length = 63996;
std::list<Data64kb> default_data64kb_data_generator(size_t max = 0)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 100;
    std::list<Data64kb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index] {
            Data64kb data;
            data.data().resize(data64kb_length);
            data.data()[0] = index;
            for(size_t i = 1; i < data64kb_length; ++i)
                data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
            ++index;
            return data;
            });

    return returnedValue;
}

const size_t data300kb_length = 307201;
std::list<Data1mb> default_data300kb_data_generator(size_t max = 0)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 100;
    std::list<Data1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index] {
            Data1mb data;
            data.data().resize(data300kb_length);
            data.data()[0] = index;
            for(size_t i = 1; i < data300kb_length; ++i)
                data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
            ++index;
            return data;
            });

    return returnedValue;
}

/****** Auxiliary lambda functions  ******/
const std::function<void(const HelloWorld&)>  default_helloworld_print = [](const HelloWorld& hello)
{
    std::cout << hello.index() << " ";
};

const std::function<void(const Data64kb&)>  default_data64kb_print = [](const Data64kb& data)
{
    std::cout << (uint16_t)data.data()[0] << " ";
};

const std::function<void(const Data1mb&)>  default_data300kb_print = [](const Data1mb& data)
{
    std::cout << (uint16_t)data.data()[0] << " ";
};

template<typename T>
void print_non_received_messages(const std::list<T>& data, const std::function<void(const T&)>& printer)
{
    if(data.size() != 0)
    {
        std::cout << "Samples not received: ";
        std::for_each(data.begin(), data.end(), printer);
        std::cout << std::endl;
    }
}
/***** End auxiliary lambda function *****/

TEST(BlackBox, RTPSAsNonReliableSocket)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(3));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, AsyncRTPSAsNonReliableSocket)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
   
    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        add_to_multicast_locator_list(ip, global_port).
        asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(3));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, AsyncRTPSAsNonReliableSocketWithWriterSpecificFlowControl)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t sizeToClear = 440; // Roughly ten times the size of the payload being sent
    uint32_t refreshTimeMS = 300;
    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        add_to_multicast_locator_list(ip, global_port).
        asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).
        add_throughput_controller_descriptor_to_pparams(sizeToClear, refreshTimeMS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(20));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, RTPSAsReliableSocket)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloword_data_generator();
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(5));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, AsyncRTPSAsReliableSocket)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
        add_to_multicast_locator_list(ip, global_port).
        asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloword_data_generator();
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(5));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, RTPSAsNonReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(3));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, AsyncRTPSAsNonReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(3));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, RTPSAsReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.add_to_multicast_locator_list(ip, global_port).
        reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(5));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, AsyncRTPSAsReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");
    
    reader.add_to_multicast_locator_list(ip, global_port).
        reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(5));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, PubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();
    
    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(3));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, AsyncPubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    size_t data_length = data.size();
    
    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(3));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, PubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(5));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, AsyncPubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(30));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, ReqRepAsReliableHelloworld)
{
    ReqRepAsReliableHelloWorldRequester requester;
    ReqRepAsReliableHelloWorldReplier replier;
    const uint16_t nmsgs = 100;

    requester.init();

    ASSERT_TRUE(requester.isInitialized());

    replier.init();

    ASSERT_TRUE(replier.isInitialized());

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block(std::chrono::seconds(5));
    }
}

TEST(BlackBox, ParticipantRemoval)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    // Send some data.
    auto data = default_helloword_data_generator();
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.waitRemoval();
}

TEST(BlackBox, PubSubAsReliableData64kb)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 500).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator(30);
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(30));

    print_non_received_messages(data, default_data64kb_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, AsyncPubSubAsReliableData64kbWithParticipantFlowControl)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t sizeToClear = 68000;
    uint32_t periodInMs = 500;
    writer.add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs);

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 500).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator(30);
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(30));

    print_non_received_messages(data, default_data64kb_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, AsyncPubSubAsReliableData64kbWithParticipantFlowControlAndUserTransport)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t sizeToClear = 300000;
    uint32_t periodInMs = 500;
    writer.add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs);

    auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
    testTransport->granularMode = true;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 500).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator(30);
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(50));

    print_non_received_messages(data, default_data64kb_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, PubSubAsNonReliableData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);
    
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST(BlackBox, PubSubAsReliableData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);
    
    writer.init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST(BlackBox, AsyncPubSubAsNonReliableData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);
    
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
	
    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t sizeToClear = 65536;
    uint32_t periodInMs = 50;

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 500).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(30);
    size_t data_length = data.size();
    
    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(20));

    print_non_received_messages(data, default_data300kb_print);
    ASSERT_LE(data.size(), data_length - 2);
}

TEST(BlackBox, AsyncPubSubAsReliableData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

	// When doing fragmentation, it is necessary to have some degree of
	// flow control not to overrun the receive buffer.
	uint32_t sizeToClear = 65536;
	uint32_t periodInMs = 50;

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 500).
        add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(30);
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(30));

    print_non_received_messages(data, default_data300kb_print);
    ASSERT_EQ(data.size(), 0);
}

TEST(BlackBox, AsyncPubSubAsReliableData300kbInLossyConditions)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

	// When doing fragmentation, it is necessary to have some degree of
	// flow control not to overrun the receive buffer.
	uint32_t sizeToClear = 300000;
	uint32_t periodInMs = 200;
	writer.add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs);

   // To simulate lossy conditions, we are going to remove the default
   // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    testTransport->granularMode = false;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 10;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 500).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(30);
    
    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(30));

    print_non_received_messages(data, default_data300kb_print);
    ASSERT_EQ(data.size(), 0);

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(test_UDPv4Transport::DropLog.size(), testTransport->dropLogLength);
}

// Test created to check bug #1568 (Github #34)
TEST(BlackBox, PubSubAsNonReliableKeepLastReaderSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator(10);
    
    reader.expected_data(data);

    unsigned int tries = 0;
    for(; tries < 6 && !data.empty(); ++tries)
    {
        // Store previous data vector size.
        size_t previous_size = data.size();
        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        std::this_thread::sleep_for(std::chrono::seconds(2));
        reader.startReception();
        // Block reader until reception finished or timeout.
        data = reader.block(std::chrono::seconds(1));
        reader.stopReception();
        // Should be received only two samples.
        ASSERT_EQ(previous_size - data.size(), 2);
    }
    // To send 10 samples needs at least five tries.
    ASSERT_GE(tries, 5u);

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}
//Test created to deal with Issue 39 on Github
TEST(BlackBox, CacheChangeReleaseTest)
{
	PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
	PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

	//Reader Config
	reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
	reader.history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS);
	reader.history_depth(1);
	reader.resource_limits_max_samples(1);
	reader.allocated_samples(5);
	reader.heartbeatPeriod(0,4294967 * 50);
	reader.init();
	ASSERT_TRUE(reader.isInitialized());

	writer.heartbeat_period_seconds(0).heartbeat_period_fraction(4294967*100);
	writer.resource_limits_max_samples(1);
	writer.history_kind(KEEP_LAST_HISTORY_QOS);
	writer.history_depth(1);
	writer.reliability(BEST_EFFORT_RELIABILITY_QOS);
	writer.allocated_samples(50);	
	writer.init();
	ASSERT_TRUE(writer.isInitialized());


	// Because its volatile the durability
	// Wait for discovery.
	writer.waitDiscovery();
	reader.waitDiscovery();

	auto data = default_helloword_data_generator(60);
    
	reader.expected_data(data);
	reader.startReception();

    writer.send(data);
    ASSERT_TRUE(data.empty());
    data = reader.block(std::chrono::seconds(10));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), static_cast<size_t>(9));
}
// Test created to check bug #1555 (Github #31)
TEST(BlackBox, PubSubAsReliableKeepLastReaderSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967*100).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator(10);

    reader.expected_data(data);

    unsigned int tries = 0;
    for(; tries < 5 && !data.empty(); ++tries)
    {
        // Store previous data vector size.
        size_t previous_size = data.size();
        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        std::this_thread::sleep_for(std::chrono::seconds(2));
        reader.startReception();
        // Block reader until reception finished or timeout.
        data = reader.block(std::chrono::seconds(1));
        reader.stopReception();
        // Should be received only two samples.
        ASSERT_EQ(previous_size - data.size(), 2);
        if(data.size() > 0)
            ASSERT_EQ(data.back().index(), previous_size - 2);
    }

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

// Test created to check bug #1558 (Github #33)
TEST(BlackBox, PubSubKeepAll)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_max_samples(20).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 100).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();

    reader.expected_data(data);

    unsigned int tries = 0;
    for(; tries < 5 && !data.empty(); ++tries)
    {
        // Backup data vector size.
        size_t previous_size = data.size();
        // Send data
        writer.send(data);
        // Store number samples sent.
        size_t sent_size = previous_size - data.size();
        // In this test the history has 20 max_samples.
        ASSERT_LE(sent_size, 20u);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        reader.startReception(sent_size);
        // Block reader until reception finished or timeout.
        data = reader.block(std::chrono::seconds(20));
        reader.stopReception();
        // Should be received the data was sent.
        ASSERT_EQ(previous_size - data.size(), sent_size);
        if(data.size() > 0)
            ASSERT_EQ(data.front().index(), (sent_size * (tries + 1)) + 1);
        //Wait for acknowledge, because then the history could be entirely again.
        ASSERT_TRUE(writer.waitForAllAcked(std::chrono::seconds(20)));
    }
    // To send 100 samples needs at least five tries.
    ASSERT_EQ(tries, 5);

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

// Test created to check bug #1558 (Github #33)
TEST(BlackBox, PubSubKeepAllTransient)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
        resource_limits_max_samples(20).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 100).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloword_data_generator();

    reader.expected_data(data);

    unsigned int tries = 0;
    for(; tries < 5 && !data.empty(); ++tries)
    {
        // Backup data vector size.
        size_t previous_size = data.size();
        // Send data
        writer.send(data);
        // Store number samples sent.
        size_t sent_size = previous_size - data.size();
        // In this test the history has 20 max_samples.
        ASSERT_LE(sent_size, 20u);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        reader.startReception(sent_size);
        // Block reader until reception finished or timeout.
        data = reader.block(std::chrono::seconds(20 ));
        reader.stopReception();
        // Should be received the data was sent.
        ASSERT_EQ(previous_size - data.size(), sent_size);
        if(data.size() > 0)
            ASSERT_EQ(data.front().index(), (sent_size * (tries + 1)) + 1);
        //Wait for acknowledge, because then the history could be entirely again.
        ASSERT_TRUE(writer.waitForAllAcked(std::chrono::seconds(20)));
    }
    // To send 100 samples needs at least five tries.
    ASSERT_EQ(tries, 5);

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

//Verify that outLocatorList is used to select the desired output channel
TEST(BlackBox, PubSubOutLocatorSelection){
   
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    
    LocatorList_t WriterOutLocators;
    Locator_t LocatorBuffer;
    
    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = 31337;

    WriterOutLocators.push_back(LocatorBuffer);

	
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
        resource_limits_max_samples(20).
        heartbeat_period_seconds(0).
        heartbeat_period_fraction(4294967 * 100).
	outLocatorList(WriterOutLocators).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    	auto data = default_helloword_data_generator(10);
    
	reader.expected_data(data);
	reader.startReception();

    writer.send(data);
    ASSERT_TRUE(data.empty());
    data = reader.block(std::chrono::seconds(10));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), static_cast<size_t>(0));
}

//Verify that Cachechanges are removed from History when the a Writer unmatches
TEST(BlackBox, StatefulReaderCacheChangeRelease){
	PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
	PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

	reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
	ASSERT_TRUE(reader.isInitialized());
	writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
	ASSERT_TRUE(writer.isInitialized());

	writer.waitDiscovery();
	reader.waitDiscovery();

	auto data = default_helloword_data_generator(2);
	reader.expected_data(data);

	writer.send(data);
	ASSERT_TRUE(data.empty());
	ASSERT_EQ(data.size(), static_cast<size_t>(0));
	writer.destroy();
	reader.startReception();
	data = reader.block(std::chrono::seconds(2));
	
	ASSERT_EQ(data.size(), static_cast<size_t>(2));
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);
    return RUN_ALL_TESTS();
}
