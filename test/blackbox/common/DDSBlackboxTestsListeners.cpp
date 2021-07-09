// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#define INCOMPATIBLE_TEST_TOPIC_NAME std::string( \
        std::string("incompatible_") + TEST_TOPIC_NAME)


enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSStatus : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
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
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
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
    }

};

/* This test also serves as check for the publication_matched and subscription_matched conditions,
 * as all the status conditions are processed through waitsets and we are checking if there is a match
 */
TEST_P(DDSStatus, IncompatibleQosConditions)
{
    PubSubWriterWithWaitsets<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and trigger incompatible QoS conditions on both Writer and Reader
    PubSubReaderWithWaitsets<HelloWorldType> incompatible_reliability_reader(TEST_TOPIC_NAME);
    incompatible_reliability_reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(incompatible_reliability_reader.isInitialized());

    writer.wait_incompatible_qos(1);
    incompatible_reliability_reader.wait_incompatible_qos(1);

    EXPECT_EQ(writer.times_incompatible_qos(), 1u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(),
            eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    // Another Reader on the same Topic but with incompatible QoS
    // Should not match and trigger incompatible QoS condition on both Writer and Reader
    // Total count of incompatible QoS occurrences in Writer increments, and updates the latest incompatible QoS ID,
    // but old Reader stays the same
    PubSubReaderWithWaitsets<HelloWorldType> incompatible_durability_reader(TEST_TOPIC_NAME);
    incompatible_durability_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(incompatible_durability_reader.isInitialized());

    writer.wait_incompatible_qos(2);
    incompatible_durability_reader.wait_incompatible_qos(1);

    EXPECT_EQ(writer.times_incompatible_qos(), 2u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(),
            eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_durability_reader.is_matched());

    // Create another two writers equal to the first one.
    // Incompatible readers increase incompatible QoS occurrences by two
    PubSubWriterWithWaitsets<HelloWorldType> writer2(TEST_TOPIC_NAME);
    writer2.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriterWithWaitsets<HelloWorldType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never trigger incompatible QoS conditions
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReaderWithWaitsets<HelloWorldType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
    compatible_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::requested_incompatible_qos())
            .init();
    ASSERT_TRUE(compatible_reader.isInitialized());

    writer2.wait_incompatible_qos(2);
    writer3.wait_incompatible_qos(2);
    incompatible_reliability_reader.wait_incompatible_qos(3);
    incompatible_durability_reader.wait_incompatible_qos(3);

    EXPECT_EQ(writer.times_incompatible_qos(), 2u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(writer2.times_incompatible_qos(), 2u);
    EXPECT_FALSE(writer2.is_matched());

    EXPECT_EQ(writer3.times_incompatible_qos(), 2u);
    EXPECT_FALSE(writer3.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 3u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(),
            eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 3u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_durability_reader.is_matched());

    EXPECT_EQ(compatible_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(compatible_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);
    EXPECT_FALSE(compatible_reader.is_matched());
}

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
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(),
            eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
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
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(),
            eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 1u);
    EXPECT_EQ(incompatible_durability_reader.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_durability_reader.is_matched());

    // Create another two writers equal to the first one.
    // Incompatible readers increase incompatible QoS occurrences by two
    PubSubWriter<HelloWorldType> writer2(TEST_TOPIC_NAME);
    writer2.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriter<HelloWorldType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never call incompatible QoS callbacks
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReader<HelloWorldType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
    compatible_reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(compatible_reader.isInitialized());

    writer2.wait_incompatible_qos(2);
    writer3.wait_incompatible_qos(2);
    incompatible_reliability_reader.wait_incompatible_qos(3);
    incompatible_durability_reader.wait_incompatible_qos(3);

    EXPECT_EQ(writer.times_incompatible_qos(), 2u);
    EXPECT_EQ(writer.last_incompatible_qos(), eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(writer.is_matched());

    EXPECT_EQ(writer2.times_incompatible_qos(), 2u);
    EXPECT_FALSE(writer2.is_matched());

    EXPECT_EQ(writer2.times_incompatible_qos(), 2u);
    EXPECT_FALSE(writer2.is_matched());

    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 3u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(),
            eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_FALSE(incompatible_reliability_reader.is_matched());

    EXPECT_EQ(incompatible_durability_reader.times_incompatible_qos(), 3u);
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

    wstatus = writer2.get_incompatible_qos_status();
    EXPECT_EQ(writer2.times_incompatible_qos(), wstatus.total_count);
    EXPECT_EQ(writer2.last_incompatible_qos(), wstatus.last_policy_id);

    wstatus = writer3.get_incompatible_qos_status();
    EXPECT_EQ(writer3.times_incompatible_qos(), wstatus.total_count);
    EXPECT_EQ(writer3.last_incompatible_qos(), wstatus.last_policy_id);

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

    // Create another two writers equal to the first one.
    // Incompatible readers increase incompatible QoS occurrences by two
    PubSubWriter<HelloWorldType> writer2(TEST_TOPIC_NAME);
    writer2.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriter<HelloWorldType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never increase incompatible QoS occurrences
    PubSubReader<HelloWorldType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
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

    wstatus = writer2.get_incompatible_qos_status();
    EXPECT_EQ(wstatus.total_count, 2u);
    //This is the fist time we call to get_incompatible_qos_status on this writer
    EXPECT_EQ(wstatus.total_count_change, 2u);

    //No listeners were called
    EXPECT_EQ(writer2.times_incompatible_qos(), 0u);
    EXPECT_EQ(writer2.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    wstatus = writer3.get_incompatible_qos_status();
    EXPECT_EQ(wstatus.total_count, 2u);
    //This is the fist time we call to get_incompatible_qos_status on this writer
    EXPECT_EQ(wstatus.total_count_change, 2u);

    //No listeners were called
    EXPECT_EQ(writer3.times_incompatible_qos(), 0u);
    EXPECT_EQ(writer3.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_reliability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 3u);
    //total_count_change was reset on previous call to get_incompatible_qos_status
    EXPECT_EQ(rstatus.total_count_change, 2u);
    EXPECT_EQ(rstatus.last_policy_id, eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    //No listeners were called
    EXPECT_EQ(incompatible_reliability_reader.times_incompatible_qos(), 0u);
    EXPECT_EQ(incompatible_reliability_reader.last_incompatible_qos(), eprosima::fastdds::dds::INVALID_QOS_POLICY_ID);

    rstatus = incompatible_durability_reader.get_incompatible_qos_status();
    EXPECT_EQ(rstatus.total_count, 3u);
    //total_count_change was reset on previous call to get_incompatible_qos_status
    EXPECT_EQ(rstatus.total_count_change, 2u);
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

TEST_P(DDSStatus, LivelinessConditions)
{
    PubSubReaderWithWaitsets<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriterWithWaitsets<HelloWorldType> writer(TEST_TOPIC_NAME);

    constexpr unsigned int participant_announcement_period_ms = 50000;

    writer.lease_duration(
        participant_announcement_period_ms * 3e-3, participant_announcement_period_ms * 1e-3);
    reader.lease_duration(
        participant_announcement_period_ms * 3e-3, participant_announcement_period_ms * 1e-3);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Lease duration, announcement period, and sleep time, in milliseconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

TEST_P(DDSStatus, DeadlineConditions)
{
    // This test sets a short deadline (short compared to the write rate),
    // makes the writer send a few samples and checks that the deadline was missed every time
    // Uses a topic with no key

    PubSubReaderWithWaitsets<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriterWithWaitsets<HelloWorldType> writer(TEST_TOPIC_NAME);

    // Write rate in milliseconds
    uint32_t writer_sleep_ms = 1000;
    // Number of samples written by writer
    uint32_t writer_samples = 3;
    // Deadline period in milliseconds
    uint32_t deadline_period_ms = 10;

    reader.deadline_period(deadline_period_ms * 1e-3).init();
    writer.deadline_period(deadline_period_ms * 1e-3).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(writer_samples);

    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        reader.block_for_at_least(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(writer_sleep_ms));
    }

    // All samples should have missed the deadline
    EXPECT_GE(writer.missed_deadlines(), writer_samples);
    EXPECT_GE(reader.missed_deadlines(), writer_samples);
}

TEST_P(DDSStatus, DataAvailableConditions)
{
    PubSubReaderWithWaitsets<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriterWithWaitsets<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReaderWithWaitsets<HelloWorldType> subscriber_reader(TEST_TOPIC_NAME);

    // Waitset timeout in seconds
    uint32_t timeout_s = 2;

    // This reader will receive the data notification on the reader
    reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            deactivate_status_listener(eprosima::fastdds::dds::StatusMask::data_on_readers());
    reader.waitset_timeout(timeout_s).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).init();
    ASSERT_TRUE(writer.isInitialized());

    // This reader will receive the data notification on the subscriber
    subscriber_reader.history_depth(100).
            reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
            deactivate_status_listener(eprosima::fastdds::dds::StatusMask::data_available());
    subscriber_reader.waitset_timeout(timeout_s).init();
    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery(2);
    reader.wait_discovery();
    subscriber_reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    subscriber_reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    subscriber_reader.block_for_all();

    // No timeouts until this point
    ASSERT_EQ(0u, reader.times_waitset_timeout());
    ASSERT_EQ(0u, subscriber_reader.times_waitset_timeout());

    // Now wait until at least one timeout occurs
    reader.wait_waitset_timeout();
    subscriber_reader.wait_waitset_timeout();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSStatus,
        DDSStatus,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DDSStatus::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });

