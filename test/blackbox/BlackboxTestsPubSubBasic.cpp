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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class BlackBox : public testing::TestWithParam<bool>
{
};

TEST_P(BlackBox, PubSubAsNonReliableHelloworld)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, AsyncPubSubAsNonReliableHelloworld)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, PubSubAsReliableHelloworld)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, AsyncPubSubAsReliableHelloworld)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, ReqRepAsReliableHelloworld)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, PubSubAsReliableData64kb)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

    PubSubReader<Data64kbType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data64kbType> writer(TEST_TOPIC_NAME);

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, PubSubMoreThan256Unacknowledged)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

TEST_P(BlackBox, PubSubAsReliableHelloworldMulticastDisabled)
{
    LibrarySettingsAttributes library_settings;
    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }

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

    if (GetParam())
    {
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(library_settings);
    }
}

INSTANTIATE_TEST_CASE_P(PubSubBasic,
        BlackBox,
        testing::Values(false, true),
        [](const testing::TestParamInfo<BlackBox::ParamType>& info)
{
    if (info.param)
    {
        return "NonIntraprocess";
    }
    return "Intraprocess";
});
