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
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/flowcontrol/ThroughputControllerDescriptor.h>
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/xmlparser/XMLParser.h>


#include <thread>
#include <memory>
#include <cstdlib>
#include <string>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
#define MEMORY_MODE_STRING ReallocMem
#define MEMORY_MODE_BYTE 1
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
#define MEMORY_MODE_STRING DynMem
#define MEMORY_MODE_BYTE 2
#else
#define MEMORY_MODE_STRING PreallocMem
#define MEMORY_MODE_BYTE 3
#endif

#define PASTER(x, y) x ## _ ## y
#define EVALUATOR(x, y) PASTER(x, y)
#define BLACKBOXTEST(test_case_name, test_name) TEST(EVALUATOR(test_case_name, MEMORY_MODE_STRING), test_name)
#define BLACKBOXTEST_F(test_case_name, test_name) TEST_F(EVALUATOR(test_case_name, MEMORY_MODE_STRING), test_name)
#define TEST_TOPIC_NAME std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())

uint16_t global_port = 0;

#if HAVE_SECURITY
static const char* certs_path = nullptr;
#endif

uint16_t get_port()
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if(5000 > port)
    {
        port += 5000;
    }

    std::cout << "Generating port " << port << std::endl;
    return port;
}

class BlackboxEnvironment : public ::testing::Environment
{
    public:

        void SetUp()
        {
            global_port = get_port();
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
            {
                data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
            }
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
            {
                data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
            }
            ++index;
            return data;
            });

    return returnedValue;
}

std::list<Data1mb> default_data300kb_mix_data_generator(size_t max = 0)
{
    unsigned char index = 1;
    size_t maximum = max ? max : 10;
    std::list<Data1mb> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index] {
            Data1mb data;
            size_t length = index % 2 != 0 ? data300kb_length : 30000;
            data.data().resize(length);
            data.data()[0] = index;
            for(size_t i = 1; i < length; ++i)
            {
                data.data()[i] = static_cast<unsigned char>(i + data.data()[0]);
            }
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

class EVALUATOR(BlackBoxPersistence, MEMORY_MODE_STRING) : public ::testing::Test
{
public:
    const std::string& db_file_name() const { return db_file_name_; }
    const eprosima::fastrtps::rtps::GuidPrefix_t& guid_prefix() const { return guid_prefix_; }
    std::list<HelloWorld> not_received_data;

    void run_one_send_recv_test(RTPSWithRegistrationReader<HelloWorldType>& reader, RTPSWithRegistrationWriter<HelloWorldType>& writer, uint32_t seq_check = 0, bool reliable = false)
    {
        // Wait for discovery.
        writer.waitDiscovery();
        reader.waitDiscovery();

        auto data = default_helloworld_data_generator();
        not_received_data.insert(not_received_data.end(), data.begin(), data.end());

        reader.expected_data(not_received_data);
        reader.startReception();

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());

        // Block reader until reception finished or timeout.
        if (seq_check > 0)
        {
            reader.block_until_seq_number_greater_or_equal({ 0,seq_check });
        }
        else
        {
            if (reliable)
            {
                reader.block_for_all();
            }
            else
            {
                reader.block_for_at_least(2);
            }
        }

        reader.destroy();
        writer.destroy();

        data = reader.not_received_data();
        print_non_received_messages(data, default_helloworld_print);
        not_received_data = data;
    }

protected:
    std::string db_file_name_;
    eprosima::fastrtps::rtps::GuidPrefix_t guid_prefix_;

    virtual void SetUp()
    {
        // Get info about current test
        auto info = ::testing::UnitTest::GetInstance()->current_test_info();

        // Create DB file name from test name and PID
        std::ostringstream ss;
        ss << info->test_case_name() << "_" << info->name() << "_" << GET_PID() << ".db";
        db_file_name_ = ss.str();

        // Fill guid prefix
        int32_t* p_value = (int32_t*)guid_prefix_.value;
        *p_value++ = info->line();
        *p_value = GET_PID();
        guid_prefix_.value[8] = HAVE_SECURITY;
        guid_prefix_.value[9] = MEMORY_MODE_BYTE;
        eprosima::fastrtps::rtps::LocatorList_t loc;
        eprosima::fastrtps::rtps::IPFinder::getIP4Address(&loc);
        if (loc.size()>0)
        {
            guid_prefix_.value[10] = loc.begin()->address[14];
            guid_prefix_.value[11] = loc.begin()->address[15];
        }
        else
        {
            guid_prefix_.value[10] = 127;
            guid_prefix_.value[11] = 1;
        }
    }

    virtual void TearDown()
    {
        std::remove(db_file_name_.c_str());
    }
};

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

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
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

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
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

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
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
    reader.block_for_all();
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
    reader.block_for_all();
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

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
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

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
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
    reader.block_for_all();
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
    reader.block_for_all();
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

    requester.waitDiscovery();
    replier.waitDiscovery();

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
    reader.wait_participant_undiscovery();
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

BLACKBOXTEST_F(BlackBoxPersistence, RTPSAsNonReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_persistent(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_persistent(db_file_name(), guid_prefix()).reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, false);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 13, false);
    reader.destroy();
    writer.destroy();

    std::cout << "Second round finished." << std::endl;
}

BLACKBOXTEST_F(BlackBoxPersistence, AsyncRTPSAsNonReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_persistent(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_persistent(db_file_name(), guid_prefix()).reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, false);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 13, false);

    std::cout << "Second round finished." << std::endl;
}

BLACKBOXTEST_F(BlackBoxPersistence, RTPSAsReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_persistent(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).
        reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_persistent(db_file_name(), guid_prefix()).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, true);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 20, true);

    std::cout << "Second round finished." << std::endl;
}

BLACKBOXTEST_F(BlackBoxPersistence, AsyncRTPSAsReliableWithPersistence)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.make_persistent(db_file_name(), guid_prefix()).add_to_multicast_locator_list(ip, global_port).
        reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.make_persistent(db_file_name(), guid_prefix()).asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 0, true);

    // Stop and start reader and writer
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "First round finished." << std::endl;

    reader.init();
    writer.init();

    // Discover, send and receive
    run_one_send_recv_test(reader, writer, 20, true);
    reader.destroy();
    writer.destroy();

    std::cout << "Second round finished." << std::endl;
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
        max_blocking_time({0, 0}).
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
        max_blocking_time({0, 0}).
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
    char* value = nullptr;
    std::string TOPIC_RANDOM_NUMBER;
    std::string W_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string R_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string MULTICAST_PORT_RANDOM_NUMBER_STR;
    // Get environment variables.
    value = std::getenv("TOPIC_RANDOM_NUMBER");
    if(value != nullptr)
    {
        TOPIC_RANDOM_NUMBER = value;
    }
    else
    {
        TOPIC_RANDOM_NUMBER = "1";
    }
    value = std::getenv("W_UNICAST_PORT_RANDOM_NUMBER");
    if(value != nullptr)
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = "7411";
    }
    int32_t W_UNICAST_PORT_RANDOM_NUMBER = stoi(W_UNICAST_PORT_RANDOM_NUMBER_STR);
    value =std::getenv("R_UNICAST_PORT_RANDOM_NUMBER");
    if(value != nullptr)
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = "7421";
    }
    int32_t R_UNICAST_PORT_RANDOM_NUMBER = stoi(R_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("MULTICAST_PORT_RANDOM_NUMBER");
    if(value != nullptr)
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = "7400";
    }
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

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

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

    writer.history_depth(100).
        userData({'a','b','c','d'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.setOnDiscoveryFunction([&writer](const ParticipantDiscoveryInfo& info) -> bool{
            if(info.rtps.m_guid == writer.participant_guid())
            {
                std::cout << "Received USER_DATA from the writer: ";
                for (auto i: info.rtps.m_userData) std::cout << i << ' ';
                return info.rtps.m_userData == std::vector<octet>({'a','b','c','d'});
            }

            return false;
        });

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());


    reader.waitDiscovery();
    writer.waitDiscovery();

    reader.wait_discovery_result();
}

BLACKBOXTEST(BlackBox, PubSubAsReliableHelloworldParticipantDiscovery)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    int count = 0;
    reader.setOnDiscoveryFunction([&writer, &count](const ParticipantDiscoveryInfo& info) -> bool{
            if(info.rtps.m_guid == writer.participant_guid())
            {
                if(info.rtps.m_status == DISCOVERED_RTPSPARTICIPANT)
                {
                    std::cout << "Discovered participant " << info.rtps.m_guid << std::endl;
                    ++count;
                }
                else if(info.rtps.m_status == REMOVED_RTPSPARTICIPANT ||
                        info.rtps.m_status == DROPPED_RTPSPARTICIPANT)
                {
                    std::cout << "Removed participant " << info.rtps.m_guid << std::endl;
                    return ++count == 2;
                }
            }

            return false;
        });

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.waitDiscovery();
    writer.waitDiscovery();

    writer.destroy();

    reader.wait_participant_undiscovery();

    reader.wait_discovery_result();
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

    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    delete reader;
    delete writer;

    checker.block_until_discover_topic(checker.topic_name(), 1);
    checker.block_until_discover_partition("test", 0);
    checker.block_until_discover_partition("othertest", 0);
}

// Used to detect Github issue #155
BLACKBOXTEST(BlackBox, EndpointRediscovery)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    reader.disable_builtin_transport();
    reader.add_user_transport_to_pparams(testTransport);

    reader.lease_duration({3, 0}, {1, 0}).reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    // We drop 20% of all data frags
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    writer.lease_duration({6, 0}, {2, 0}).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    // Wait heartbeat period of builtin endpoints
    std::this_thread::sleep_for(std::chrono::seconds(4));

    test_UDPv4Transport::ShutdownAllNetwork = true;

    writer.wait_reader_undiscovery();

    test_UDPv4Transport::ShutdownAllNetwork = false;

    writer.waitDiscovery();
}

// Used to detect Github issue #154
BLACKBOXTEST(BlackBox, LocalInitialPeers)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    uint32_t port = get_port();

    Locator_t loc_initial_peer, loc_default_unicast;
    LocatorList_t reader_initial_peers;
    loc_initial_peer.set_IP4_address(127, 0, 0, 1);
    loc_initial_peer.port = port;
    reader_initial_peers.push_back(loc_initial_peer);
    LocatorList_t reader_default_unicast_locator;
    loc_default_unicast.port = port + 1;
    reader_default_unicast_locator.push_back(loc_default_unicast);

    reader.metatraffic_unicast_locator_list(reader_default_unicast_locator).
        initial_peers(reader_initial_peers).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    LocatorList_t writer_initial_peers;
    loc_initial_peer.port = port + 1;
    writer_initial_peers.push_back(loc_initial_peer);
    LocatorList_t writer_default_unicast_locator;
    loc_default_unicast.port = port;
    writer_default_unicast_locator.push_back(loc_default_unicast);

    writer.metatraffic_unicast_locator_list(writer_default_unicast_locator).
        initial_peers(writer_initial_peers).init();

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

// Regression test of Refs #2535, github micro-RTPS #1
BLACKBOXTEST(BlackBox, PubXmlLoadedPartition)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.partition("A").init();

    ASSERT_TRUE(reader.isInitialized());

    const std::string xml = R"(<profiles>
  <publisher profile_name="partition_publisher_profile">
    <topic>
      <name>)" + writer.topic_name() + R"(</name>
      <dataType>HelloWorldType</dataType>
    </topic>
    <qos>
      <partition>
        <names>
          <name>A</name>
        </names>
      </partition>
    </qos>
    </publisher>
</profiles>)";

    writer.load_publisher_attr(xml).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.waitDiscovery();
    writer.waitDiscovery();
}

// Regression test of Refs #2786, github issue #194
BLACKBOXTEST(BlackBox, RTPSAsReliableVolatileSocket)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
        durability(eprosima::fastrtps::rtps::DurabilityKind_t::VOLATILE).
        add_to_multicast_locator_list(ip, global_port).
        auto_remove_on_volatile().init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Wait for acks to be sent and check writer history is empty
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_TRUE(writer.is_history_empty());
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_rtps_ok)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_rtps_ok)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_large_string)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_rtps_large_string)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_rtps_data300kb)
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
    uint32_t periodInMs = 500;

    writer.history_depth(5).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_rtps_data300kb)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_submessage_ok)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_submessage_ok)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_submessage_large_string)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_submessage_large_string)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_submessage_data300kb)
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
    uint32_t periodInMs = 500;

    writer.history_depth(5).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_submessage_data300kb)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_payload_ok)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_payload_ok)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_payload_large_string)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_payload_large_string)
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
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_payload_data300kb)
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
    uint32_t periodInMs = 500;

    writer.history_depth(5).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_payload_data300kb)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_all_ok)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_all_ok)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_all_large_string)
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
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_all_large_string)
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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_besteffort_all_data300kb)
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
    uint32_t periodInMs = 1000;

    writer.history_depth(5).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
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
    reader.block_for_at_least(2);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_all_data300kb)
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

// Regression test of Refs #2457
BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_reliable_all_data300kb_mix)
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

    writer.history_depth(2).resource_limits_max_samples(2).resource_limits_allocated_samples(2).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_mix_data_generator(10);

    reader.startReception(data);

    size_t count = 0;
    for(auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        if(count % 2 == 0)
        {
            // Block reader until reception finished or timeout.
            reader.block_for_at_least(count);
        }
    }
}

// Regression test of Refs #2457, Github ros2 #438.
BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndCryptoPlugin_user_data)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
                   pub_property_policy, sub_property_policy;

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

    writer.history_depth(100).
        userData({'a','b','c','d','e'}).
        property_policy(pub_part_property_policy).
        entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

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

    reader.setOnDiscoveryFunction([&writer](const ParticipantDiscoveryInfo& info) -> bool{
            if(info.rtps.m_guid == writer.participant_guid())
            {
                std::cout << "Received USER_DATA from the writer: ";
                for (auto i: info.rtps.m_userData) std::cout << i << ' ';
                return info.rtps.m_userData == std::vector<octet>({'a','b','c','d','e'});
            }

            return false;
        });

    reader.history_depth(100).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        property_policy(sub_part_property_policy).
        entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    reader.waitDiscovery();
    writer.waitDiscovery();

    reader.wait_discovery_result();
}

static void BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(PubSubReader<HelloWorldType>& reader,
        PubSubWriter<HelloWorldType>& writer, const std::string& governance_file)
{
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
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                    "builtin.Access-Permissions"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                    "file://" + std::string(certs_path) + "/" + governance_file));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                    "file://" + std::string(certs_path) + "/permissions.smime"));

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
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                    "builtin.Access-Permissions"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                    "file://" + std::string(certs_path) + "/" + governance_file));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                    "file://" + std::string(certs_path) + "/permissions.smime"));

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

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


BLACKBOXTEST(BlackBox, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_none)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
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

// Test created to check bug #3020 (Github ros2/demos #238)
BLACKBOXTEST(BlackBox, PubSubAsReliableVolatilePubRemoveWithoutSubs)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.history_depth(10).
        durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    size_t number_of_changes_removed = 0;
    ASSERT_FALSE(writer.remove_all_changes(&number_of_changes_removed));
}

// Test created to check bug #3087 (Github #230)
BLACKBOXTEST(BlackBox, AsyncPubSubAsNonReliableVolatileHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
        durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
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

// Test created to check a bug with writers that use BEST_EFFORT WITH VOLATILE that don't remove messages from history.
BLACKBOXTEST(BlackBox, AsyncPubSubAsNonReliableVolatileKeepAllHelloworld)
{
    RTPSAsSocketReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
        durability(eprosima::fastrtps::rtps::DurabilityKind_t::VOLATILE).
        add_to_multicast_locator_list(ip, global_port).
        auto_remove_on_volatile().init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Wait for acks to be sent and check writer history is empty
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_TRUE(writer.is_history_empty());
}

// Test created to check bug #3290 (ROS2 #539)
BLACKBOXTEST(BlackBox, AsyncVolatileKeepAllPubReliableSubNonReliable300Kb)
{
    PubSubReader<Data1mbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbType> writer(TEST_TOPIC_NAME);

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
        init();

    ASSERT_TRUE(reader.isInitialized());

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
        resource_limits_allocated_samples(9).
        resource_limits_max_samples(9).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodInMs).
        init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data,300);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

// Test created to check bug #3290 (ROS2 #539)
BLACKBOXTEST(BlackBox, VolatileKeepAllPubReliableSubNonReliableHelloWorld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
        init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
        resource_limits_allocated_samples(9).
        resource_limits_max_samples(9).
        init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

// Test created to check bug #3290 (ROS2 #539)
BLACKBOXTEST(BlackBox, AsyncVolatileKeepAllPubReliableSubNonReliableHelloWorld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
        init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
        reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
        durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).
        resource_limits_allocated_samples(9).
        resource_limits_max_samples(9).
        asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).
        init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.waitDiscovery();
    reader.waitDiscovery();

    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    // Send data with some interval, to let async writer thread send samples
    writer.send(data,300);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

// Regression test of Refs #3376, github ros2/rmw_fastrtps #226
BLACKBOXTEST(BlackBox, ReqRepVolatileHelloworldRequesterCheckWriteParams)
{
    ReqRepAsReliableHelloWorldRequester requester;

    requester.durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();

    ASSERT_TRUE(requester.isInitialized());

    requester.send(1);
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
