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

#include <gtest/gtest.h>

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/xmlparser//XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class LifespanQos : public testing::TestWithParam<bool>
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


TEST_P(LifespanQos, LongLifespan)
{
    // This test sets a long lifespan, makes the writer send a few samples
    // and checks that those changes can be removed from the history
    // as they should not have been removed due to lifespan QoS

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME, false);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 1;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Lifespan period in milliseconds
    uint32_t lifespan_ms = 10000;

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
    .lifespan_period(lifespan_ms * 1e-3)
    .init();
    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
    .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
    .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(writer_samples);
    auto expected_data = data;
    writer.send(data, writer_sleep_ms);
    ASSERT_TRUE(data.empty());

    reader.startReception(expected_data);
    reader.block_for_at_least(writer_samples);

    // Changes should not have been removed due to lifespan
    // therefore they should still exist in the history and
    // we should be able to remove them manually
    size_t removed_pub = 0;
    writer.remove_all_changes(&removed_pub);
    EXPECT_EQ(removed_pub, writer_samples);

    // On the reader side we should be able to take the data
    HelloWorldType::type msg;
    EXPECT_EQ(reader.takeNextData(&msg), true);
    EXPECT_EQ(reader.takeNextData(&msg), true);
    EXPECT_EQ(reader.takeNextData(&msg), true);
}

TEST_P(LifespanQos, ShortLifespan)
{
    // This test sets a short lifespan, makes the writer send a few samples
    // and checks that those samples cannot be removed from the history as
    // they have been removed by lifespan QoS

    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 200;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Lifespan period in milliseconds
    uint32_t lifespan_ms = 1;

    writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
    .lifespan_period(lifespan_ms * 1e-3)
    .init();
    reader.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
    .lifespan_period(lifespan_ms * 1e-3)
    .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(writer_samples);
    writer.send(data, writer_sleep_ms);
    ASSERT_TRUE(data.empty());

    // Changes should have been removed from history by lifespan QoS
    // so we should not be able to remove them anymore
    size_t removed_pub = 0;
    writer.remove_all_changes(&removed_pub);
    EXPECT_EQ(removed_pub, 0u);

    // On the reader side we should not be able to take the data
    HelloWorldType::type msg;
    EXPECT_EQ(reader.takeNextData(&msg), false);
    EXPECT_EQ(reader.takeNextData(&msg), false);
    EXPECT_EQ(reader.takeNextData(&msg), false);
}

INSTANTIATE_TEST_CASE_P(LifespanQos,
        LifespanQos,
        testing::Values(false, true),
        [](const testing::TestParamInfo<LifespanQos::ParamType>& info)
        {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });

