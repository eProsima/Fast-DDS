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
#include "types/StringType.h"
#include "types/Data64kbType.h"
#include "types/Data1mbType.h"

#include <thread>

/****** Auxiliary print functions  ******/
template<class Type>
void default_receive_print(const Type&)
{
    std::cout << "Received data" << std::endl;
}

template<>
void default_receive_print(const HelloWorld& hello)
{
    std::cout << "Received HelloWorld " << hello.index() << std::endl;
}

template<>
void default_receive_print(const String& str)
{
    std::cout << "Received String " <<str.message()[str.message().size() - 2]
        << str.message()[str.message().size() - 1] << std::endl;
}

template<>
void default_receive_print(const Data64kb& data)
{
    std::cout << "Received Data64kb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_receive_print(const Data1mb& data)
{
    std::cout << "Received Data1mb " << (uint16_t)data.data()[0] << std::endl;;
}

template<class Type>
void default_send_print(const Type&)
{
    std::cout << "Sent data" << std::endl;
}

template<>
void default_send_print(const HelloWorld& hello)
{
    std::cout << "Sent HelloWorld " << hello.index() << std::endl;
}

template<>
void default_send_print(const String& str)
{
    std::cout << "Sent String " <<str.message()[str.message().size() - 2]
        << str.message()[str.message().size() - 1] << std::endl;
}

template<>
void default_send_print(const Data64kb& data)
{
    std::cout << "Sent Data64kb " << (uint16_t)data.data()[0] << std::endl;
}

template<>
void default_send_print(const Data1mb& data)
{
    std::cout << "Sent Data1mb " << (uint16_t)data.data()[0] << std::endl;;
}

#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.h>
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/xmlparser/XMLProfileParser.h>


#include <thread>
#include <memory>
#include <cstdlib>
#include <string>
#include <gtest/gtest.h>

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
#define MEMORY_MODE_STRING ReallocMem
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
#define MEMORY_MODE_STRING DynMem
#else
#define MEMORY_MODE_STRING PreallocMem
#endif

#define PASTER(x, y) x ## _ ## y
#define EVALUATOR(x, y) PASTER(x, y)
#define BLACKBOXTEST(test_case_name, test_name) TEST(EVALUATOR(test_case_name, MEMORY_MODE_STRING), test_name)
#define TEST_TOPIC_NAME std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())

uint32_t global_port = 0;

#if HAVE_SECURITY
static const char* certs_path = nullptr;
#endif

class BlackboxEnvironment : public ::testing::Environment
{
    public:

        void SetUp()
        {
            global_port = GET_PID();

            if(global_port + 7400 > global_port)
                global_port += 7400;
            //Log::SetVerbosity(Log::Info);
            //Log::SetCategoryFilter(std::regex("(SECURITY)"));
        }

        void TearDown()
        {
            Log::Reset();
            eprosima::fastrtps::rtps::RTPSDomain::stopAll();
        }
};

/****** Auxiliary data generators *******/
std::list<HelloWorld> default_helloworld_data_generator(size_t max = 0)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
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

std::list<String> default_large_string_data_generator(size_t max = 0)
{
    uint16_t index = 1;
    size_t maximum = max ? max : 10;
    std::list<String> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index] {
            String str;
            std::stringstream ss;
            ss << std::string(998, 'a') << std::setw(2) << std::setfill('0') << index;
            str.message(ss.str());
            ++index;
            return str;
            });

    return returnedValue;
}

const size_t data64kb_length = 63996;
std::list<Data64kb> default_data64kb_data_generator(size_t max = 0)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
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
    size_t maximum = max ? max : 10;
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

const std::function<void(const String&)>  default_string_print = [](const String& str)
{
    std::cout << str.message()[str.message().size() - 2]
        << str.message()[str.message().size() - 1] << " ";
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

BLACKBOXTEST(BlackBox, RTPSAsNonReliableSocket)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(1));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

BLACKBOXTEST(BlackBox, AsyncRTPSAsNonReliableSocket)
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

    auto data = default_helloworld_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(1));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

BLACKBOXTEST(BlackBox, AsyncRTPSAsNonReliableSocketWithWriterSpecificFlowControl)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 440; // Roughly ten times the size of the payload being sent
    uint32_t periodMillisecs = 300;
    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        add_to_multicast_locator_list(ip, global_port).
        asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodMillisecs).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(1));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

BLACKBOXTEST(BlackBox, RTPSAsReliableSocket)
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

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(2));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

BLACKBOXTEST(BlackBox, AsyncRTPSAsReliableSocket)
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

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(2));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

BLACKBOXTEST(BlackBox, RTPSAsNonReliableWithRegistration)
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

    auto data = default_helloworld_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(1));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

BLACKBOXTEST(BlackBox, AsyncRTPSAsNonReliableWithRegistration)
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

    auto data = default_helloworld_data_generator();
    size_t data_length = data.size();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(1));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_LE(data.size(), data_length - 2);
}

BLACKBOXTEST(BlackBox, RTPSAsReliableWithRegistration)
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

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(2));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

BLACKBOXTEST(BlackBox, AsyncRTPSAsReliableWithRegistration)
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

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    data = reader.block(std::chrono::seconds(2));

    print_non_received_messages(data, default_helloworld_print);
    ASSERT_EQ(data.size(), 0);
}

BLACKBOXTEST(BlackBox, PubSubAsNonReliableHelloworld)
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

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, PubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, ReqRepAsReliableHelloworld)
{
    ReqRepAsReliableHelloWorldRequester requester;
    ReqRepAsReliableHelloWorldReplier replier;
    const uint16_t nmsgs = 10;

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

BLACKBOXTEST(BlackBox, ParticipantRemoval)
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
    auto data = default_helloworld_data_generator();
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.waitRemoval();
}

BLACKBOXTEST(BlackBox, PubSubAsReliableData64kb)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(10).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsReliableData64kbWithParticipantFlowControl)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 68000;
    uint32_t periodInMs = 500;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    writer.history_depth(3).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsReliableData64kbWithParticipantFlowControlAndUserTransport)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(3).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 65000;
    uint32_t periodInMs = 500;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(3).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator(3);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, PubSubAsNonReliableData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

BLACKBOXTEST(BlackBox, PubSubAsReliableData300kb)
{
    // Mutes an expected error
    Log::SetErrorStringFilter(std::regex("^((?!Big data).)*$"));

    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data300kb_data_generator(1);
    // Send data
    writer.send(data);
    // In this test data is not sent.
    ASSERT_FALSE(data.empty());
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsNonReliableData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(10).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsReliableData300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, AsyncPubSubWithFlowController64kb)
{
    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> slowWriter(TEST_TOPIC_NAME);

    reader.history_depth(2).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    uint32_t sizeToClear = 68000; //68kb
    uint32_t periodInMs = 1000; //1sec

    slowWriter.history_depth(2).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(sizeToClear, periodInMs).init();
    ASSERT_TRUE(slowWriter.isInitialized());

    slowWriter.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data64kb_data_generator(2);

    reader.startReception(data);
    slowWriter.send(data);
    // In 1 second only one of the messages has time to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(reader.getReceivedCount(), 1);
}

BLACKBOXTEST(BlackBox, AsyncPubSubAsReliableData300kbInLossyConditions)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_depth(5).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 300000;
    uint32_t periodInMs = 200;
    writer.add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs);

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataFragMessagesPercentage = 20;
    testTransport->dropLogLength = 1;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.history_depth(5).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Sanity check. Make sure we have dropped a few packets
    ASSERT_EQ(test_UDPv4Transport::DropLog.size(), testTransport->dropLogLength);
}


BLACKBOXTEST(BlackBox, AsyncFragmentSizeTest)
{
    // ThroghputController size large than maxMessageSize.
    {
        PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
        PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

        reader.history_depth(10).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        // When doing fragmentation, it is necessary to have some degree of
        // flow control not to overrun the receive buffer.
        uint32_t size = 32536;
        uint32_t periodInMs = 500;
        writer.add_throughput_controller_descriptor_to_pparams(size, periodInMs);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 32000;
        testTransport->sendBufferSize = 65536;
        testTransport->receiveBufferSize = 65536;
        writer.disable_builtin_transport();
        writer.add_user_transport_to_pparams(testTransport);
        writer.history_depth(10).asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

        ASSERT_TRUE(writer.isInitialized());

        // Because its volatile the durability
        // Wait for discovery.
        writer.waitDiscovery();
        reader.waitDiscovery();

        auto data = default_data64kb_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        std::this_thread::sleep_for(std::chrono::seconds(3));
        size_t current_received = reader.getReceivedCount();
        ASSERT_GE(current_received, static_cast<size_t>(1));
        ASSERT_LE(current_received, static_cast<size_t>(3));

    }
    // ThroghputController size smaller than maxMessageSize.
    {
        PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
        PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

        reader.history_depth(10).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        // When doing fragmentation, it is necessary to have some degree of
        // flow control not to overrun the receive buffer.
        uint32_t size = 32000;
        uint32_t periodInMs = 500;
        writer.add_throughput_controller_descriptor_to_pparams(size, periodInMs);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 32536;
        testTransport->sendBufferSize = 65536;
        testTransport->receiveBufferSize = 65536;
        writer.disable_builtin_transport();
        writer.add_user_transport_to_pparams(testTransport);
        writer.history_depth(10).
            asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

        ASSERT_TRUE(writer.isInitialized());

        // Because its volatile the durability
        // Wait for discovery.
        writer.waitDiscovery();
        reader.waitDiscovery();

        auto data = default_data64kb_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        std::this_thread::sleep_for(std::chrono::seconds(3));
        size_t current_received = reader.getReceivedCount();
        ASSERT_GE(current_received, static_cast<size_t>(1));
        ASSERT_LE(current_received, static_cast<size_t>(3));
    }
}

BLACKBOXTEST(BlackBox, FlowControllerIfNotAsync)
{
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

    uint32_t size = 10000;
    uint32_t periodInMs = 1000;
    writer.add_throughput_controller_descriptor_to_pparams(size, periodInMs).init();
    ASSERT_FALSE(writer.isInitialized());
}

BLACKBOXTEST(BlackBox, UDPv4TransportWrongConfig)
{
    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->maxMessageSize = 100000;

        writer.disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

        ASSERT_FALSE(writer.isInitialized());
    }

    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->sendBufferSize = 64000;

        writer.disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

        ASSERT_FALSE(writer.isInitialized());
    }

    {
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->receiveBufferSize = 64000;

        writer.disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

        ASSERT_FALSE(writer.isInitialized());
    }
}

// Test created to check bug #1568 (Github #34)
BLACKBOXTEST(BlackBox, PubSubAsNonReliableKeepLastReaderSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    while(data.size() > 1)
    {
        auto expected_data(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        size_t current_received = reader.block_for_at_least(2);
        reader.stopReception();
        // Should be received only two samples.
        ASSERT_EQ(current_received, 2);
        data = reader.data_not_received();
    }
}

//Test created to deal with Issue 39 on Github
BLACKBOXTEST(BlackBox, CacheChangeReleaseTest)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    //Reader Config
    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    reader.history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS);
    reader.history_depth(1);
    reader.resource_limits_allocated_samples(1);
    reader.resource_limits_max_samples(1);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    writer.resource_limits_allocated_samples(1);
    writer.resource_limits_max_samples(1);
    writer.history_kind(KEEP_LAST_HISTORY_QOS);
    writer.history_depth(1);
    writer.reliability(BEST_EFFORT_RELIABILITY_QOS);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());


    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    size_t current_received = reader.block_for_all(std::chrono::seconds(3));

    ASSERT_GE(current_received, static_cast<size_t>(1));
}

// Test created to check bug #1555 (Github #31)
BLACKBOXTEST(BlackBox, PubSubAsReliableKeepLastReaderSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    while(data.size() > 1)
    {
        auto data_backup(data);
        decltype(data) expected_data;
        expected_data.push_back(data_backup.back()); data_backup.pop_back();
        expected_data.push_back(data_backup.back()); data_backup.pop_back();

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        writer.waitForAllAcked(std::chrono::seconds(300));
        // Should be received only two samples.
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        reader.block_for_all();
        reader.stopReception();

        data = data_backup;
    }
}

// Test created to check bug #1738 (Github #54)
BLACKBOXTEST(BlackBox, PubSubAsReliableKeepLastWriterSmallDepth)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    reader.reliability(RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.
        history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS).
        history_depth(2).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

// Test created to check bug #1558 (Github #33)
BLACKBOXTEST(BlackBox, PubSubKeepAll)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    while(!data.empty())
    {
        auto expected_data(data);

        // Send data
        writer.send(data);

        for(auto& value : data)
            expected_data.remove(value);

        // In this test the history has 20 max_samples.
        ASSERT_LE(expected_data.size(), 2u);
        writer.waitForAllAcked(std::chrono::seconds(300));
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        reader.block_for_all();
        reader.stopReception();
    }
}

// Test created to check bug #1558 (Github #33)
BLACKBOXTEST(BlackBox, PubSubKeepAllTransient)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    while(!data.empty())
    {
        auto expected_data(data);

        // Send data
        writer.send(data);

        for(auto& value : data)
            expected_data.remove(value);

        // In this test the history has 20 max_samples.
        ASSERT_LE(expected_data.size(), 2u);
        writer.waitForAllAcked(std::chrono::seconds(300));
        reader.startReception(expected_data);
        // Block reader until reception finished or timeout.
        reader.block_for_all();
        reader.stopReception();
    }
}

BLACKBOXTEST(BlackBox, PubReliableKeepAllSubNonReliable)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(1).
        resource_limits_max_samples(1).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

//Verify that outLocatorList is used to select the desired output channel
BLACKBOXTEST(BlackBox, PubSubOutLocatorSelection){

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterOutLocators;
    Locator_t LocatorBuffer;

    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = 31337;

    WriterOutLocators.push_back(LocatorBuffer);


    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        resource_limits_allocated_samples(2).
        resource_limits_max_samples(2).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).
        resource_limits_allocated_samples(20).
        resource_limits_max_samples(20).
        outLocatorList(WriterOutLocators).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}

//Verify that Cachechanges are removed from History when the a Writer unmatches
BLACKBOXTEST(BlackBox, StatefulReaderCacheChangeRelease){
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(2).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());
    writer.history_depth(2).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(writer.isInitialized());

    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator(2);
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());
    writer.waitForAllAcked(std::chrono::seconds(300));
    writer.destroy();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    reader.startReception(expected_data);

    ASSERT_EQ(reader.getReceivedCount(), 0);
}

BLACKBOXTEST(BlackBox, PubSubMoreThan256Unacknowledged)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator(600);
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.startReception(expected_data);
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, StaticDiscovery)
{
    // Get environment variables.
    std::string TOPIC_RANDOM_NUMBER(std::getenv("TOPIC_RANDOM_NUMBER"));
    ASSERT_FALSE(TOPIC_RANDOM_NUMBER.empty());
    std::string W_UNICAST_PORT_RANDOM_NUMBER_STR(std::getenv("W_UNICAST_PORT_RANDOM_NUMBER"));
    ASSERT_FALSE(W_UNICAST_PORT_RANDOM_NUMBER_STR.empty());
    int32_t W_UNICAST_PORT_RANDOM_NUMBER = stoi(W_UNICAST_PORT_RANDOM_NUMBER_STR);
    std::string R_UNICAST_PORT_RANDOM_NUMBER_STR(std::getenv("R_UNICAST_PORT_RANDOM_NUMBER"));
    ASSERT_FALSE(R_UNICAST_PORT_RANDOM_NUMBER_STR.empty());
    int32_t R_UNICAST_PORT_RANDOM_NUMBER = stoi(R_UNICAST_PORT_RANDOM_NUMBER_STR);
    std::string MULTICAST_PORT_RANDOM_NUMBER_STR(std::getenv("MULTICAST_PORT_RANDOM_NUMBER"));
    ASSERT_FALSE(MULTICAST_PORT_RANDOM_NUMBER_STR.empty());
    int32_t MULTICAST_PORT_RANDOM_NUMBER = stoi(MULTICAST_PORT_RANDOM_NUMBER_STR);

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterUnicastLocators;
    Locator_t LocatorBuffer;

    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = W_UNICAST_PORT_RANDOM_NUMBER;
    LocatorBuffer.set_IP4_address(127,0,0,1);
    WriterUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t WriterMulticastLocators;

    LocatorBuffer.port = MULTICAST_PORT_RANDOM_NUMBER;
    WriterMulticastLocators.push_back(LocatorBuffer);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS);
    writer.static_discovery("PubSubWriter.xml").reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        unicastLocatorList(WriterUnicastLocators).multicastLocatorList(WriterMulticastLocators).
        setPublisherIDs(1, 2).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();


    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);

    LocatorList_t ReaderUnicastLocators;

    LocatorBuffer.port = R_UNICAST_PORT_RANDOM_NUMBER;
    ReaderUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t ReaderMulticastLocators;

    LocatorBuffer.port = MULTICAST_PORT_RANDOM_NUMBER;
    ReaderMulticastLocators.push_back(LocatorBuffer);


    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
    durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS);
    reader.static_discovery("PubSubReader.xml").
        unicastLocatorList(ReaderUnicastLocators).multicastLocatorList(ReaderMulticastLocators).
        setSubscriberIDs(3, 4).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

    ASSERT_TRUE(reader.isInitialized());

    auto data = default_helloworld_data_generator();
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    reader.startReception(expected_data);
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, PubSubAsReliableHelloworldMulticastDisabled)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        disable_multicast(0).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        disable_multicast(1).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Test created to check bug #2010 (Github #90)
BLACKBOXTEST(BlackBox, PubSubAsReliableHelloworldPartitions)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
        partition("PartitionTests").
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        partition("PartitionTe*").init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, PubSubAsReliableHelloworldUserData)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    std::vector<octet> received_user_data;
    reader.setOnDiscoveryFunction([](const ParticipantDiscoveryInfo& info) -> bool{
            std::cout << "Received USER_DATA from the writer: ";
            for (auto i: info.rtps.m_userData) std::cout << i << ' ';
            return info.rtps.m_userData == std::vector<octet>({'a','b','c','d'});
        });

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        userData({'a','b','c','d'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.waitDiscovery();
    writer.waitDiscovery();

    ASSERT_TRUE(reader.getDiscoveryResult());
}

BLACKBOXTEST(BlackBox, EDPSlaveReaderAttachment)
{
    PubSubWriter<HelloWorldType> checker(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType>* reader = new PubSubReader<HelloWorldType>(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType>* writer = new PubSubWriter<HelloWorldType>(TEST_TOPIC_NAME);

    checker.attach_edp_listeners().init();

    ASSERT_TRUE(checker.isInitialized());

    reader->partition("test").partition("othertest").init();

    ASSERT_TRUE(reader->isInitialized());

    writer->partition("test").init();

    ASSERT_TRUE(writer->isInitialized());

    checker.block_until_discover_topic(checker.topic_name(), 3);
    checker.block_until_discover_partition("test", 2);
    checker.block_until_discover_partition("othertest", 1);

    delete reader;
    delete writer;

    checker.block_until_discover_topic(checker.topic_name(), 1);
    checker.block_until_discover_partition("test", 0);
    checker.block_until_discover_partition("othertest", 0);
}

#if HAVE_SECURITY

BLACKBOXTEST(BlackBox, BuiltinAuthenticationPlugin_PKIDH_validation_ok)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));

    writer.history_depth(10).
        property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
BLACKBOXTEST(BlackBox, BuiltinAuthenticationPlugin_PKIDH_validation_ok_same_participant)
{
    PubSubWriterReader<HelloWorldType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy property_policy;

    property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));

    wreader.property_policy(property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationPlugin_PKIDH_validation_fail)
{
    {
        PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        PropertyPolicy pub_property_policy;

        reader.history_depth(10).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));

        writer.history_depth(10).
            property_policy(pub_property_policy).init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        writer.waitUnauthorized();
    }
    {
        PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
        PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

        PropertyPolicy sub_property_policy;

        sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));

        reader.history_depth(10).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

        ASSERT_TRUE(reader.isInitialized());

        writer.history_depth(10).init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitUnauthorized();
    }
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationPlugin_PKIDH_lossy_conditions)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_property_policy).init();

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataMessagesPercentage = 40;
    testTransport->dropLogLength = 10;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));

    writer.history_depth(10).
        property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_rtps_ok)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_rtps_ok_same_participant)
{
    PubSubWriterReader<HelloWorldType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy,
                   property_policy;

    property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    wreader.property_policy(property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_rtps_large_string)
{
    PubSubReader<StringType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_rtps_data300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(5).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).
        property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_submessage_ok)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_submessage_ok_same_participant)
{
    PubSubWriterReader<HelloWorldType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy,
                   property_policy;

    property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    wreader.property_policy(property_policy).
        pub_property_policy(pub_property_policy).
        sub_property_policy(sub_property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_submessage_large_string)
{
    PubSubReader<StringType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_submessage_data300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(5).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_payload_ok)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_payload_ok_same_participant)
{
    PubSubWriterReader<HelloWorldType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy,
                   property_policy;

    property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    wreader.property_policy(property_policy).
        pub_property_policy(pub_property_policy).
        sub_property_policy(sub_property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_payload_large_string)
{
    PubSubReader<StringType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    ASSERT_EQ(data.size(), 0);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_payload_data300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_all_ok)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_all_large_string)
{
    PubSubReader<StringType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_all_data300kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    sub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
                    "builtin.PKI-DH"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
                    "file://" + std::string(certs_path) + "/mainpubkey.pem"));
    pub_part_property_policy.properties().emplace_back(Property("dds.sec.crypto.plugin",
                    "builtin.AES-GCM-GMAC"));
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

#endif

template<typename T>
void send_async_data(PubSubWriter<T>& writer, std::list<typename T::type> data_to_send)
{
    // Send data
    writer.send(data_to_send);
    // In this test all data should be sent.
    ASSERT_TRUE(data_to_send.empty());
}

BLACKBOXTEST(BlackBox, PubSubAsReliableMultithreadKeepLast1)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_depth(1).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(1).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator(300);

    reader.startReception(data);

    std::thread thr1(&send_async_data<HelloWorldType>, std::ref(writer),
            std::list<HelloWorld>(data.begin(), std::next(data.begin(), 100)));
    std::thread thr2(&send_async_data<HelloWorldType>, std::ref(writer),
            std::list<HelloWorld>(std::next(data.begin(), 100), std::next(data.begin(), 200)));
    std::thread thr3(&send_async_data<HelloWorldType>, std::ref(writer),
            std::list<HelloWorld>(std::next(data.begin(), 200), data.end()));

    thr1.join();
    thr2.join();
    thr3.join();

    // Block reader until reception finished or timeout.
    reader.block_for_at_least(105);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);

#if HAVE_SECURITY
    certs_path = std::getenv("CERTS_PATH");

    if(certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        exit(-1);
    }
#endif

    return RUN_ALL_TESTS();
}
