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

#include "BlackboxTests.hpp"

#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <gtest/gtest.h>

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class RTPS : public testing::TestWithParam<bool>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        if (GetParam())
        {
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
            xmlparser::XMLProfileManager::library_settings(library_settings);
        }
    }
};

TEST_P(RTPS, RTPSAsNonReliableSocket)
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

TEST_P(RTPS, AsyncRTPSAsNonReliableSocket)
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

TEST_P(RTPS, AsyncRTPSAsNonReliableSocketWithWriterSpecificFlowControl)
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

TEST_P(RTPS, RTPSAsReliableSocket)
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

TEST_P(RTPS, AsyncRTPSAsReliableSocket)
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

TEST_P(RTPS, RTPSAsNonReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

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

TEST_P(RTPS, AsyncRTPSAsNonReliableWithRegistration)
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
    writer.wait_discovery();
    reader.wait_discovery();

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

TEST_P(RTPS, RTPSAsReliableWithRegistration)
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
    writer.wait_discovery();
    reader.wait_discovery();

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

TEST_P(RTPS, AsyncRTPSAsReliableWithRegistration)
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
    writer.wait_discovery();
    reader.wait_discovery();

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

// Regression test of Refs #2786, github issue #194
TEST_P(RTPS, RTPSAsReliableVolatileSocket)
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
    writer.wait_for_all_acked(std::chrono::seconds(100));

    ASSERT_TRUE(writer.is_history_empty());
}

INSTANTIATE_TEST_CASE_P(RTPS,
        RTPS,
        testing::Values(false, true),
        [](const testing::TestParamInfo<RTPS::ParamType>& info) {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });
