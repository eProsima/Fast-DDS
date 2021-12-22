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

#include "mock/BlackboxMockConsumer.h"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>
#include <tuple>
#include <vector>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class PubSubBasic : public testing::TestWithParam<std::tuple<communication_type, bool>>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (std::get<0>(GetParam()))
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }

        use_pull_mode = std::get<1>(GetParam());
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (std::get<0>(GetParam()))
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }

        use_pull_mode = false;
    }

};

/**
 * @brief Test function for EnvFileWarning* tests
 *
 * Sets environment variable @c FASTDDS_ENVIRONMENT_FILE to @c env_file_name, and then initializes a DomainParticipant,
 * using a custom log consumer to check how many logWarnings matching
 * ".*does not exist. File watching not initialized." are issued, comparing that number with @c expected_logs.
 * Finally, it resets the log module
 *
 * @param env_file_name The value to assign to @c FASTDDS_ENVIRONMENT_FILE
 * @param expected_logs The number of expected logs (error or warning) matching
 *                      ".*does not exist. File watching not initialized."
 */
void env_file_warning(
        const char* env_file_name,
        size_t expected_logs)
{
    /* Set environment variable */
#ifdef _WIN32
    _putenv_s("FASTDDS_ENVIRONMENT_FILE", env_file_name);
#else
    setenv("FASTDDS_ENVIRONMENT_FILE", env_file_name, 1);
#endif // _WIN32

    using namespace eprosima::fastdds::dds;
    /* Set up log */
    BlackboxMockConsumer* helper_consumer = new BlackboxMockConsumer();
    Log::ClearConsumers();  // Remove default consumers
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(helper_consumer)); // Registering a consumer transfer ownership
    // Filter specific message
    Log::SetVerbosity(Log::Kind::Warning);
    Log::SetCategoryFilter(std::regex("RTPS_PARTICIPANT"));
    Log::SetErrorStringFilter(std::regex(".*does not exist. File watching not initialized."));

    /* Create and enable DomainParticipant */
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    /* Check logs */
    Log::Flush();
    EXPECT_EQ(helper_consumer->ConsumedEntries().size(), expected_logs);

    /* Clean-up */
    Log::Reset();  // This calls to ClearConsumers, which deletes the registered consumer
}

TEST_P(PubSubBasic, PubSubAsNonReliableHelloworld)
{
    // Best effort incompatible with best effort
    if (use_pull_mode)
    {
        return;
    }

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(PubSubBasic, AsyncPubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(PubSubBasic, PubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubBasic, AsyncPubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
            asynchronously(eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubBasic, ReqRepAsReliableHelloworld)
{
    ReqRepAsReliableHelloWorldRequester requester;
    ReqRepAsReliableHelloWorldReplier replier;
    const uint16_t nmsgs = 10;

    requester.init();

    ASSERT_TRUE(requester.isInitialized());

    replier.init();

    requester.wait_discovery();
    replier.wait_discovery();

    ASSERT_TRUE(replier.isInitialized());

    for (uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block(std::chrono::seconds(5));
    }
}

TEST_P(PubSubBasic, PubSubAsReliableData64kb)
{
    PubSubReader<Data64kbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(10).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(10).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data64kb_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubBasic, PubSubMoreThan256Unacknowledged)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator(600);
    auto expected_data(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS).
            durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.startReception(expected_data);
    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(10)));
}

TEST_P(PubSubBasic, PubSubAsReliableHelloworldMulticastDisabled)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            disable_multicast(0).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).
            disable_multicast(1).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(10)));
}

TEST_P(PubSubBasic, ReceivedDynamicDataWithNoSizeLimit)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100)
            .partition("A").partition("B").partition("C")
            .userData({'a', 'b', 'c', 'd'}).init();


    ASSERT_TRUE(writer.isInitialized());

    reader.history_depth(100)
            .partition("A")
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery(std::chrono::seconds(3));
    writer.wait_discovery(std::chrono::seconds(3));

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubBasic, ReceivedDynamicDataWithinSizeLimit)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100)
            .partition("A").partition("B").partition("C")
            .userData({'a', 'b', 'c', 'd'}).init();


    ASSERT_TRUE(writer.isInitialized());

    reader.user_data_max_size(4)
            .partitions_max_size(28)
            .history_depth(100)
            .partition("A")
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery(std::chrono::seconds(3));
    writer.wait_discovery(std::chrono::seconds(3));

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(PubSubBasic, ReceivedUserDataExceedsSizeLimit)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100)
            .userData({'a', 'b', 'c', 'd', 'e', 'f'}).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.user_data_max_size(4)
            .history_depth(100)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery(std::chrono::seconds(3));
    writer.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(reader.is_matched());
}

TEST_P(PubSubBasic, ReceivedPartitionDataExceedsSizeLimit)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.history_depth(100)
            .partition("A").partition("B").partition("C")
            .init();

    ASSERT_TRUE(writer.isInitialized());

    reader.partitions_max_size(20)
            .history_depth(100)
            .partition("A")
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery(std::chrono::seconds(3));
    writer.wait_discovery(std::chrono::seconds(3));

    ASSERT_TRUE(writer.is_matched());
    ASSERT_FALSE(reader.is_matched());
}

TEST_P(PubSubBasic, ReceivedPropertiesDataWithinSizeLimit)
{
    char* value = nullptr;
    std::string TOPIC_RANDOM_NUMBER;
    std::string W_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string R_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string MULTICAST_PORT_RANDOM_NUMBER_STR;

    // Get environment variables.
    value = std::getenv("TOPIC_RANDOM_NUMBER");
    if (value != nullptr)
    {
        TOPIC_RANDOM_NUMBER = value;
    }
    else
    {
        TOPIC_RANDOM_NUMBER = "1";
    }
    value = std::getenv("W_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = "7411";
    }
    int32_t W_UNICAST_PORT_RANDOM_NUMBER = stoi(W_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("R_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = "7421";
    }
    int32_t R_UNICAST_PORT_RANDOM_NUMBER = stoi(R_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("MULTICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = "7400";
    }
    int32_t MULTICAST_PORT_RANDOM_NUMBER = stoi(MULTICAST_PORT_RANDOM_NUMBER_STR);

    Locator_t LocatorBuffer;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterUnicastLocators;
    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = static_cast<uint16_t>(W_UNICAST_PORT_RANDOM_NUMBER);
    IPLocator::setIPv4(LocatorBuffer, 127, 0, 0, 1);
    WriterUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t WriterMulticastLocators;
    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    WriterMulticastLocators.push_back(LocatorBuffer);

    writer.static_discovery("file://PubSubWriter.xml").
            unicastLocatorList(WriterUnicastLocators).multicastLocatorList(WriterMulticastLocators).
            setPublisherIDs(1,
            2).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    LocatorList_t ReaderUnicastLocators;
    LocatorBuffer.port = static_cast<uint16_t>(R_UNICAST_PORT_RANDOM_NUMBER);
    ReaderUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t ReaderMulticastLocators;
    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    ReaderMulticastLocators.push_back(LocatorBuffer);

    //Expected properties have exactly size 92
    reader.properties_max_size(92).
            static_discovery("file://PubSubReader.xml").
            unicastLocatorList(ReaderUnicastLocators).multicastLocatorList(ReaderMulticastLocators).
            setSubscriberIDs(3,
            4).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

TEST_P(PubSubBasic, ReceivedPropertiesDataExceedsSizeLimit)
{
    char* value = nullptr;
    std::string TOPIC_RANDOM_NUMBER;
    std::string W_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string R_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string MULTICAST_PORT_RANDOM_NUMBER_STR;

    // Get environment variables.
    value = std::getenv("TOPIC_RANDOM_NUMBER");
    if (value != nullptr)
    {
        TOPIC_RANDOM_NUMBER = value;
    }
    else
    {
        TOPIC_RANDOM_NUMBER = "1";
    }
    value = std::getenv("W_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = "7411";
    }
    int32_t W_UNICAST_PORT_RANDOM_NUMBER = stoi(W_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("R_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = "7421";
    }
    int32_t R_UNICAST_PORT_RANDOM_NUMBER = stoi(R_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("MULTICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = "7400";
    }
    int32_t MULTICAST_PORT_RANDOM_NUMBER = stoi(MULTICAST_PORT_RANDOM_NUMBER_STR);

    Locator_t LocatorBuffer;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterUnicastLocators;
    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = static_cast<uint16_t>(W_UNICAST_PORT_RANDOM_NUMBER);
    IPLocator::setIPv4(LocatorBuffer, 127, 0, 0, 1);
    WriterUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t WriterMulticastLocators;
    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    WriterMulticastLocators.push_back(LocatorBuffer);

    writer.static_discovery("file://PubSubWriter.xml").
            unicastLocatorList(WriterUnicastLocators).multicastLocatorList(WriterMulticastLocators).
            setPublisherIDs(1,
            2).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    LocatorList_t ReaderUnicastLocators;
    LocatorBuffer.port = static_cast<uint16_t>(R_UNICAST_PORT_RANDOM_NUMBER);
    ReaderUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t ReaderMulticastLocators;
    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    ReaderMulticastLocators.push_back(LocatorBuffer);

    //Expected properties have size 92
    reader.properties_max_size(50)
            .static_discovery("file://PubSubReader.xml")
            .unicastLocatorList(ReaderUnicastLocators).multicastLocatorList(ReaderMulticastLocators)
            .setSubscriberIDs(3,
            4).setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER).init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(reader.is_matched());
}

TEST_P(PubSubBasic, unique_flows_one_writer_two_readers)
{
    PubSubParticipant<HelloWorldPubSubType> readers(0, 2, 0, 2);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy properties;
    properties.properties().emplace_back("fastdds.unique_network_flows", "");

    readers.sub_topic_name(TEST_TOPIC_NAME).sub_property_policy(properties).reliability(RELIABLE_RELIABILITY_QOS);

    ASSERT_TRUE(readers.init_participant());
    ASSERT_TRUE(readers.init_subscriber(0));
    ASSERT_TRUE(readers.init_subscriber(1));

    writer.history_depth(100).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    readers.sub_wait_discovery();

    // Send data
    auto data = default_helloworld_data_generator();
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block until readers have acknowledged all samples.
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(30)));
}

template<typename T>
static void two_consecutive_writers(
        PubSubReader<T>& reader,
        PubSubWriter<T>& writer,
        bool block_for_all)
{
    writer.init();
    EXPECT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto complete_data = default_helloworld_data_generator();

    reader.startReception(complete_data);

    // Send data
    writer.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    if (block_for_all)
    {
        reader.block_for_all();
    }
    else
    {
        reader.block_for_at_least(2);
    }
    reader.stopReception();

    writer.destroy();

    // Wait for undiscovery
    reader.wait_writer_undiscovery();
}

TEST_P(PubSubBasic, BestEffortTwoWritersConsecutives)
{
    // Pull mode incompatible with best effort
    if (use_pull_mode)
    {
        return;
    }

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reader.history_depth(10).init();
    EXPECT_TRUE(reader.isInitialized());

    for (int i = 0; i < 2; ++i)
    {
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
        writer.history_depth(10).reliability(BEST_EFFORT_RELIABILITY_QOS);
        two_consecutive_writers(reader, writer, false);
    }
}


TEST_P(PubSubBasic, ReliableVolatileTwoWritersConsecutives)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reader.history_depth(10).reliability(RELIABLE_RELIABILITY_QOS).init();
    EXPECT_TRUE(reader.isInitialized());

    for (int i = 0; i < 2; ++i)
    {
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
        writer.history_depth(10).durability_kind(VOLATILE_DURABILITY_QOS);
        two_consecutive_writers(reader, writer, true);
    }
}

TEST_P(PubSubBasic, ReliableTransientLocalTwoWritersConsecutives)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reader.history_depth(10).reliability(RELIABLE_RELIABILITY_QOS).durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    for (int i = 0; i < 2; ++i)
    {
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
        writer.history_depth(10).reliability(RELIABLE_RELIABILITY_QOS);
        two_consecutive_writers(reader, writer, true);
    }
}

/*
 * Check that setting FASTDDS_ENVIRONMENT_FILE to an unexisting file issues 1 logWarning
 */
TEST(PubSubBasic, EnvFileWarningWrongFile)
{
    env_file_warning("unexisting_file", 1);
}

/*
 * Check that setting FASTDDS_ENVIRONMENT_FILE to an empty string issues 0 logWarning
 */
TEST(PubSubBasic, EnvFileWarningEmpty)
{
    env_file_warning("", 0);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PubSubBasic,
        PubSubBasic,
        testing::Combine(testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING), testing::Values(false, true)),
        [](const testing::TestParamInfo<PubSubBasic::ParamType>& info)
        {
            bool pull_mode = std::get<1>(info.param);
            std::string suffix = pull_mode ? "_pull_mode" : "";
            switch (std::get<0>(info.param))
            {
                case INTRAPROCESS:
                    return "Intraprocess" + suffix;
                    break;
                case DATASHARING:
                    return "Datasharing" + suffix;
                    break;
                case TRANSPORT:
                default:
                    return "Transport" + suffix;
            }

        });
