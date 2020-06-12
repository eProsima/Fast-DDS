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
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class DDSStatus : public testing::TestWithParam<bool>
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


TEST_P(DDSStatus, IncompatibleQosListeners)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
          .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and call incompatible QoS callback on both Writer and Reader
    PubSubReader<HelloWorldType> incompatible_reliability_reader(TEST_TOPIC_NAME);
    incompatible_reliability_reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(incompatible_reliability_reader.isInitialized());

    writer.wait_incompatible_qos(1);
    incompatible_reliability_reader.wait_incompatible_qos(1);
    EXPECT_EQ(writer.times_incompatible_qos(), 1u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    // Another Reader on the same Topic but with incompatible QoS
    // Should not match and call incompatible QoS callback on both Writer and Reader
    // Total count of incompatible QoS occurrences in Writer increments, and updates the latest incompatible QoS ID,
    // but old Reader stays the same
    PubSubReader<HelloWorldType> incompatible_durability_reader(TEST_TOPIC_NAME);
    incompatible_durability_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS).init();
    ASSERT_TRUE(incompatible_durability_reader.isInitialized());

    writer.wait_incompatible_qos(2);
    incompatible_durability_reader.wait_incompatible_qos(1);
    EXPECT_EQ(writer.times_incompatible_qos(), 2u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_durability_reader.is_matched());

    // A compatible Reader on another Topic
    // Should not match but never call incompatible QoS callbacks
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReader<HelloWorldType> compatible_reader("IncompatibleTopic");
    compatible_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(compatible_reader.isInitialized());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(writer.times_incompatible_qos(), 2u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_durability_reader.is_matched());

    EXPECT_EQ(compatible_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(compatible_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);
    EXPECT_FALSE(compatible_reader.is_matched());

    // Information on the inner status must match the one above
    eprosima::fastdds::dds::RequestedIncompatibleQosStatus wstatus;
    eprosima::fastdds::dds::RequestedIncompatibleQosStatus rstatus;

    wstatus = writer.get_incompatible_qos_status();
    EXPECT_EQ(writer.times_incompatible_qos(), wstatus.total_count);
    EXPECT_EQ(writer.last_incompatible_qos(), wstatus.last_policy_id);

    rstatus = incompatible_reliability_reader.get_incompatible_qos_status();
    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), rstatus.total_count);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), rstatus.last_policy_id);

    rstatus = incompatible_durability_reader.get_incompatible_qos_status();
    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), rstatus.total_count);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), rstatus.last_policy_id);

    rstatus = compatible_reader.get_incompatible_qos_status();
    EXPECT_EQ(compatible_reader.times_incompatible_qos(), rstatus.total_count);
    EXPECT_EQ(compatible_reader.last_incompatible_qos(), rstatus.last_policy_id);
}

TEST_P(DDSStatus, IncompatibleQosGetters)
{
    eprosima::fastdds::dds::RequestedIncompatibleQosStatus wstatus;
    eprosima::fastdds::dds::RequestedIncompatibleQosStatus rstatus;

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
          .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
          .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and increase the incompatible QoS occurrences on both Writer and Reader
    PubSubReader<HelloWorldType> incompatible_reliability_reader(TEST_TOPIC_NAME);
    incompatible_reliability_reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                  .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::requested_incompatible_qos()).init();
    ASSERT_TRUE(incompatible_reliability_reader.isInitialized());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_FALSE(writer.is_matched());
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    wstatus = writer.get_incompatible_qos_status();
    EXPECT_EQ(wstatus.total_count, 1u);
    EXPECT_EQ(wstatus.total_count_change, 1u);
    EXPECT_EQ(wstatus.last_policy_id, eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(writer.times_incompatible_qos(), 0u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_reliability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 1u);
    EXPECT_EQ(rstatus.total_count_change, 1u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    // Another Reader on the same Topic but with incompatible QoS
    // Should not match and increase incompatible QoS occurrences on both Writer and Reader
    // Total count of incompatible QoS occurrences in Writer increments, and updates the latest incompatible QoS ID,
    // but old Reader stays the same
    PubSubReader<HelloWorldType> incompatible_durability_reader(TEST_TOPIC_NAME);
    incompatible_durability_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::requested_incompatible_qos()).init();
    ASSERT_TRUE(incompatible_durability_reader.isInitialized());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_FALSE(writer.is_matched());
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());
    EXPECT_FALSE(incompatible_durability_reader.is_matched());

    wstatus = writer.get_incompatible_qos_status();
    EXPECT_EQ(wstatus.total_count, 2u);
    EXPECT_EQ(wstatus.total_count_change, 1u);
    EXPECT_EQ(wstatus.last_policy_id, eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(writer.times_incompatible_qos(), 0u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_reliability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 1u);
    //total_count_change was reset on previous call to get_incompatible_qos_status
    EXPECT_EQ(rstatus.total_count_change, 0u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_durability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 1u);
    EXPECT_EQ(rstatus.total_count_change, 1u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    // A compatible Reader on another Topic
    // Should not match but never increase incompatible QoS occurrences
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReader<HelloWorldType> compatible_reader("IncompatibleTopic");
    compatible_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::requested_incompatible_qos()).init();
    ASSERT_TRUE(compatible_reader.isInitialized());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_FALSE(writer.is_matched());
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());
    EXPECT_FALSE(incompatible_durability_reader.is_matched());
    EXPECT_FALSE(compatible_reader.is_matched());

    wstatus = writer.get_incompatible_qos_status();
    EXPECT_EQ(wstatus.total_count, 2u);
    //total_count_change was reset on previous call to get_incompatible_qos_status
    EXPECT_EQ(wstatus.total_count_change, 0u);
    EXPECT_EQ(wstatus.last_policy_id, eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(writer.times_incompatible_qos(), 0u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_reliability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 1u);
    //total_count_change was reset on previous call to get_incompatible_qos_status
    EXPECT_EQ(rstatus.total_count_change, 0u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_durability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 1u);
    //total_count_change was reset on previous call to get_incompatible_qos_status
    EXPECT_EQ(rstatus.total_count_change, 0u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = compatible_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 0u);
    EXPECT_EQ(rstatus.total_count_change, 0u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(compatible_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(compatible_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);
}

INSTANTIATE_TEST_CASE_P(DDSStatus,
        DDSStatus,
        testing::Values(false, true),
        [](const testing::TestParamInfo<DDSStatus::ParamType>& info) {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });

