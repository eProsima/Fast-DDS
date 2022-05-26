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

#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

using test_UDPv4TransportDescriptor = eprosima::fastdds::rtps::test_UDPv4TransportDescriptor;

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
    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and trigger incompatible QoS conditions on both Writer and Reader
    PubSubReaderWithWaitsets<HelloWorldPubSubType> incompatible_reliability_reader(TEST_TOPIC_NAME);
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
    PubSubReaderWithWaitsets<HelloWorldPubSubType> incompatible_durability_reader(TEST_TOPIC_NAME);
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
    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    writer2.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never trigger incompatible QoS conditions
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReaderWithWaitsets<HelloWorldPubSubType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
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
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and call incompatible QoS callback on both Writer and Reader
    PubSubReader<HelloWorldPubSubType> incompatible_reliability_reader(TEST_TOPIC_NAME);
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
    PubSubReader<HelloWorldPubSubType> incompatible_durability_reader(TEST_TOPIC_NAME);
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
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    writer2.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never call incompatible QoS callbacks
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReader<HelloWorldPubSubType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
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

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and increase the incompatible QoS occurrences on both Writer and Reader
    PubSubReader<HelloWorldPubSubType> incompatible_reliability_reader(TEST_TOPIC_NAME);
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
    PubSubReader<HelloWorldPubSubType> incompatible_durability_reader(TEST_TOPIC_NAME);
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
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    writer2.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never increase incompatible QoS occurrences
    PubSubReader<HelloWorldPubSubType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
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
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

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

    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

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
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReaderWithWaitsets<HelloWorldPubSubType> subscriber_reader(TEST_TOPIC_NAME);

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

void sample_lost_test_dw_init(
        PubSubWriter<HelloWorldPubSubType>& writer)
{
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->drop_data_messages_filter_ = [](eprosima::fastrtps::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t readerID, writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                CDRMessage::readEntityId(&msg, &readerID);
                CDRMessage::readEntityId(&msg, &writerID);
                CDRMessage::readSequenceNumber(&msg, &sn);

                // restore buffer pos
                msg.pos = old_pos;

                // generate losses
                if ((writerID.value[3] & 0xC0) == 0 // only user endpoints
                        && (sn == SequenceNumber_t{0, 2} ||
                        sn == SequenceNumber_t(0, 3) ||
                        sn == SequenceNumber_t(0, 4) ||
                        sn == SequenceNumber_t(0, 6) ||
                        sn == SequenceNumber_t(0, 8) ||
                        sn == SequenceNumber_t(0, 10) ||
                        sn == SequenceNumber_t(0, 11) ||
                        sn == SequenceNumber_t(0, 13)))
                {
                    return true;
                }

                return false;
            };


    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .init();

    ASSERT_TRUE(writer.isInitialized());

}

void sample_lost_test_dr_init(
        PubSubReader<HelloWorldPubSubType>& reader,
        std::function<void(const eprosima::fastdds::dds::SampleLostStatus& status)> functor)
{
    reader.sample_lost_status_functor(functor)
            .init();

    ASSERT_TRUE(reader.isInitialized());
}

void sample_lost_test_init(
        PubSubReader<HelloWorldPubSubType>& reader,
        PubSubWriter<HelloWorldPubSubType>& writer,
        std::function<void(const eprosima::fastdds::dds::SampleLostStatus& status)> functor)
{
    sample_lost_test_dw_init(writer);
    sample_lost_test_dr_init(reader, functor);

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();
}

/*!
 * \test DDS-STS-SLS-01 Test `SampleLostStatus` in a Best-Effort DataWriter and a Best-Effort DataReader communication.
 */
TEST(DDSStatus, sample_lost_be_dw_be_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 3 == status.total_count && 3 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 4 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 7 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });


    auto data = default_helloworld_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-02 Test `SampleLostStatus` in a Best-Effort DataWriter and a late-joiner Best-Effort DataReader
 * communication.
 */
TEST(DDSStatus, sample_lost_be_dw_lj_be_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 4 == status.total_count && 4 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 6 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 8 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    data = default_helloworld_data_generator(9);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-03 Test `SampleLostStatus` in a Reliable DataWriter and a Reliable DataReader communication.
 */
TEST(DDSStatus, sample_lost_re_dw_re_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    int32_t test_count = 0;
    int32_t test_count_change_accum = 0;

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_count, &test_count_change_accum](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    test_count = status.total_count;
                    test_count_change_accum += status.total_count_change;
                }

                test_step_cv.notify_all();
            });

    auto data = default_helloworld_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
            {
                return 7 == test_count && 7 == test_count_change_accum;
            });
}

/*!
 * \test DDS-STS-SLS-04 Test `SampleLostStatus` in a Reliable DataWriter and a late-joiner Reliable DataReader
 * communication.
 */
TEST(DDSStatus, sample_lost_re_dw_lj_re_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    int32_t test_count = 0;
    int32_t test_count_change_accum = 0;

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_count, &test_count_change_accum](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    test_count = status.total_count;
                    test_count_change_accum += status.total_count_change;
                }

                test_step_cv.notify_all();
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Make sure the GAP message are received for the fourth sample.

    data = default_helloworld_data_generator(9);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
            {
                return 7 == test_count && 7 == test_count_change_accum;
            });
}

/*!
 * \test DDS-STS-SLS-05 Test `SampleLostStatus` in a Reliable DataWriter and a Best-Effort DataReader communication.
 */
TEST(DDSStatus, sample_lost_re_dw_be_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);


    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 3 == status.total_count && 3 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 4 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 7 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });

    auto data = default_helloworld_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-06 Test `SampleLostStatus` in a Reliable DataWriter and a late-joiner Best-Effort DataReader
 * communication.
 */
TEST(DDSStatus, sample_lost_re_dw_lj_be_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 4 == status.total_count && 4 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 6 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 8 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    data = default_helloworld_data_generator(9);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*
 * \test DDS-STS-SLS-07 Test `SampleLostStatus` is calculated correctly after a persistence
 * DataReader is shutting down and initiated again.
 */
TEST(DDSStatus, sample_lost_re_dw_re_persistence_dr)
{
    auto info = ::testing::UnitTest::GetInstance()->current_test_info();
    // Create DB file name from test name and PID
    std::ostringstream ss;
    std::string test_case_name(info->test_case_name());
    std::string test_name(info->name());
    ss << test_case_name << "_" << test_name << "_" << GET_PID() << ".db";
    std::string db_file_name = ss.str();

    {
        PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

        writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .make_persistent(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.74");
        reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .make_persistent(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.72");


        std::mutex test_step_mtx;
        std::condition_variable test_step_cv;
        int32_t test_count = 0;
        int32_t test_count_change_accum = 0;

        sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_count, &test_count_change_accum](
                    const eprosima::fastdds::dds::SampleLostStatus& status)
                {
                    {
                        std::unique_lock<std::mutex> lock(test_step_mtx);
                        test_count = status.total_count;
                        test_count_change_accum += status.total_count_change;
                    }

                    test_step_cv.notify_all();
                });

        auto data = default_helloworld_data_generator(6);

        reader.startReception(data);
        writer.send(data, 50);

        {
            std::unique_lock<std::mutex> lock(test_step_mtx);
            test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
                    {
                        return 3 == test_count && 3 == test_count_change_accum;
                    });
        }

        reader.destroy();
        reader.init();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Make sure the GAP message are received for the sixth sample.

        data = default_helloworld_data_generator(7);
        reader.startReception(data);
        writer.send(data, 50);

        std::unique_lock<std::mutex> lock(test_step_mtx);
        test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
                {
                    return 4 == test_count && 7 == test_count_change_accum;
                });
    }

    std::remove(db_file_name.c_str());
}

/*!
 * \test DDS-STS-SLS-08 Test `SampleLostStatus` in a Best-Effort DataWriter and a Best-Effort DataReader communication.
 */
TEST(DDSStatus, sample_lost_waitset_be_dw_be_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 3 == status.total_count && 3 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 4 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 7 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });


    auto data = default_helloworld_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-09 Test `SampleLostStatus` in a Best-Effort DataWriter and a late-joiner Best-Effort DataReader
 * communication.
 */
TEST(DDSStatus, sample_lost_waitset_be_dw_lj_be_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 4 == status.total_count && 4 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 6 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 8 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    data = default_helloworld_data_generator(9);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-10 Test `SampleLostStatus` in a Reliable DataWriter and a Reliable DataReader communication.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_re_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    int32_t test_count = 0;
    int32_t test_count_change_accum = 0;

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_count, &test_count_change_accum](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    test_count = status.total_count;
                    test_count_change_accum += status.total_count_change;
                }

                test_step_cv.notify_all();
            });

    auto data = default_helloworld_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
            {
                return 7 == test_count && 7 == test_count_change_accum;
            });
}

/*!
 * \test DDS-STS-SLS-11 Test `SampleLostStatus` in a Reliable DataWriter and a late-joiner Reliable DataReader
 * communication.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_lj_re_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    int32_t test_count = 0;
    int32_t test_count_change_accum = 0;

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_count, &test_count_change_accum](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    test_count = status.total_count;
                    test_count_change_accum += status.total_count_change;
                }

                test_step_cv.notify_all();
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Make sure the GAP message are received for the fourth sample.

    data = default_helloworld_data_generator(9);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
            {
                return 7 == test_count && 7 == test_count_change_accum;
            });
}

/*!
 * \test DDS-STS-SLS-12 Test `SampleLostStatus` in a Reliable DataWriter and a Best-Effort DataReader communication.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_be_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 3 == status.total_count && 3 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 4 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 7 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });

    auto data = default_helloworld_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-13 Test `SampleLostStatus` in a Reliable DataWriter and a late-joiner Best-Effort DataReader
 * communication.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_lj_be_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 4 == status.total_count && 4 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 6 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 8 == status.total_count && 2 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else
                    {
                        test_step = 0;
                    }
                }

                test_step_cv.notify_all();
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    data = default_helloworld_data_generator(9);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 4 == test_step;
            });
}

/*
 * \test DDS-STS-SLS-14 Test `SampleLostStatus` is calculated correctly after a persistence
 * DataReader is shutting down and initiated again.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_re_persistence_dr)
{
    // Get info about current test
    auto info = ::testing::UnitTest::GetInstance()->current_test_info();
    // Create DB file name from test name and PID
    std::ostringstream ss;
    std::string test_case_name(info->test_case_name());
    std::string test_name(info->name());
    ss << test_case_name << "_" << test_name << "_" << GET_PID() << ".db";
    std::string db_file_name = ss.str();
    {
        PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

        writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .make_persistent(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.74");
        reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .make_persistent(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.72");

        std::mutex test_step_mtx;
        std::condition_variable test_step_cv;
        int32_t test_count = 0;
        int32_t test_count_change_accum = 0;

        sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_count, &test_count_change_accum](
                    const eprosima::fastdds::dds::SampleLostStatus& status)
                {
                    {
                        std::unique_lock<std::mutex> lock(test_step_mtx);
                        test_count = status.total_count;
                        test_count_change_accum += status.total_count_change;
                    }

                    test_step_cv.notify_all();
                });

        auto data = default_helloworld_data_generator(6);

        reader.startReception(data);
        writer.send(data, 50);

        {
            std::unique_lock<std::mutex> lock(test_step_mtx);
            test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
                    {
                        return 3 == test_count && 3 == test_count_change_accum;
                    });
        }

        reader.destroy();
        reader.init();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Make sure the GAP message are received for the sixth sample.

        data = default_helloworld_data_generator(7);
        reader.startReception(data);
        writer.send(data, 50);

        std::unique_lock<std::mutex> lock(test_step_mtx);
        test_step_cv.wait(lock, [&test_count, &test_count_change_accum]()
                {
                    return 4 == test_count && 7 == test_count_change_accum;
                });
    }

    std::remove(db_file_name.c_str());
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
