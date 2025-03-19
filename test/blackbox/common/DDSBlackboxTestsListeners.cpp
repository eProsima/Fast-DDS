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

#include <set>
#include <thread>
#include <vector>

#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds::rtps;

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
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery =
                        eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
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
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
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
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and trigger incompatible QoS conditions on both Writer and Reader
    PubSubReaderWithWaitsets<HelloWorldPubSubType> incompatible_reliability_reader(TEST_TOPIC_NAME);
    incompatible_reliability_reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
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
    incompatible_durability_reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
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
    writer2.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriterWithWaitsets<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never trigger incompatible QoS conditions
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReaderWithWaitsets<HelloWorldPubSubType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
    compatible_reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
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
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and call incompatible QoS callback on both Writer and Reader
    PubSubReader<HelloWorldPubSubType> incompatible_reliability_reader(TEST_TOPIC_NAME);
    incompatible_reliability_reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
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
    incompatible_durability_reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS).init();
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
    writer2.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS).init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never call incompatible QoS callbacks
    // Total count of incompatible QoS occurrences and latest incompatible QoS ID stay the same
    PubSubReader<HelloWorldPubSubType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
    compatible_reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS).init();
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
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer.isInitialized());

    // A Reader on the same Topic but with incompatible QoS
    // Should not match and increase the incompatible QoS occurrences on both Writer and Reader
    PubSubReader<HelloWorldPubSubType> incompatible_reliability_reader(TEST_TOPIC_NAME);
    incompatible_reliability_reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
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
    incompatible_durability_reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
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
    writer2.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer2.isInitialized());

    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    writer3.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .deactivate_status_listener(eprosima::fastdds::dds::StatusMask::offered_incompatible_qos()).init();
    ASSERT_TRUE(writer3.isInitialized());

    // A compatible Reader on another Topic
    // Should not match but never increase incompatible QoS occurrences
    PubSubReader<HelloWorldPubSubType> compatible_reader(INCOMPATIBLE_TEST_TOPIC_NAME);
    compatible_reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
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

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
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
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            deactivate_status_listener(eprosima::fastdds::dds::StatusMask::data_on_readers());
    reader.waitset_timeout(timeout_s).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100).init();
    ASSERT_TRUE(writer.isInitialized());

    // This reader will receive the data notification on the subscriber
    subscriber_reader.history_depth(100).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
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

// We want to ensure that samples are only lost due to the custom filter we have set in sample_lost_test_dw_init.
// Since we are going to send 300KB samples in the test for fragments, let's increase the buffer size to avoid any
// other possible loss.
static constexpr uint32_t SAMPLE_LOST_TEST_BUFFER_SIZE =
        300ul * 1024ul // sample size
        * 13ul         // number of samples
        * 2ul;         // 2x to avoid any possible loss

template<typename T>
void sample_lost_test_dw_init(
        PubSubWriter<T>& writer)
{
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = SAMPLE_LOST_TEST_BUFFER_SIZE;
    testTransport->receiveBufferSize = SAMPLE_LOST_TEST_BUFFER_SIZE;

    testTransport->drop_data_messages_filter_ = [](eprosima::fastdds::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t readerID;
                EntityId_t writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                readerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                writerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);

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
    testTransport->drop_data_frag_messages_filter_ = [](eprosima::fastdds::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.4 DataFrag Submessage
                EntityId_t readerID;
                EntityId_t writerID;
                SequenceNumber_t sn;
                uint32_t first_fragment = 0;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                readerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                writerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;

                first_fragment = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);

                // restore buffer pos
                msg.pos = old_pos;

                // generate losses
                if ((writerID.value[3] & 0xC0) == 0 // only user endpoints
                        && (1 == first_fragment)    // only first fragment
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

template<typename T>
void sample_lost_test_dr_init(
        PubSubReader<T>& reader,
        std::function<void(const eprosima::fastdds::dds::SampleLostStatus& status)> functor)
{
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    udp_transport->sendBufferSize = SAMPLE_LOST_TEST_BUFFER_SIZE;
    udp_transport->receiveBufferSize = SAMPLE_LOST_TEST_BUFFER_SIZE;

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(udp_transport)
            .sample_lost_status_functor(functor)
            .init();

    ASSERT_TRUE(reader.isInitialized());
}

template<typename T>
void sample_lost_test_init(
        PubSubReader<T>& reader,
        PubSubWriter<T>& writer,
        std::function<void(const eprosima::fastdds::dds::SampleLostStatus& status)> functor)
{
    reader.socket_buffer_size(SAMPLE_LOST_TEST_BUFFER_SIZE);
    writer.socket_buffer_size(SAMPLE_LOST_TEST_BUFFER_SIZE);

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

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);

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
 * \test DDS-STS-SLS-01 Test `SampleLostStatus` in a Best-Effort DataWriter and a Best-Effort DataReader communication.
 * This is also a regression test for bug redmine 20162
 */
TEST(DDSStatus, sample_lost_be_dw_be_dr_fragments)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);

    sample_lost_test_init(reader, writer, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    std::cout << status.total_count << " " << status.total_count_change << std::endl;
                    if (0 == test_step && 1 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 2 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 3 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (3 == test_step && 4 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (4 == test_step && 5 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (5 == test_step && 6 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (6 == test_step && 7 == status.total_count && 1 == status.total_count_change)
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


    auto data = default_data300kb_data_generator(13);

    reader.startReception(data);
    writer.send(data, 100);

    std::unique_lock<std::mutex> lock(test_step_mtx);
    test_step_cv.wait(lock, [&test_step]()
            {
                return 7 == test_step;
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

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 1 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 2 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 4 == status.total_count && 2 == status.total_count_change)
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
                return 3 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-03 Test `SampleLostStatus` in a Reliable DataWriter and a Reliable DataReader communication.
 */
TEST(DDSStatus, sample_lost_re_dw_re_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

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

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    int32_t test_count = 0;
    int32_t test_count_change_accum = 0;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
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
                return 4 == test_count && 4 == test_count_change_accum;
            });
}

/*!
 * \test DDS-STS-SLS-05 Test `SampleLostStatus` in a Reliable DataWriter and a Best-Effort DataReader communication.
 */
TEST(DDSStatus, sample_lost_re_dw_be_dr)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);


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

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 1 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 2 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 4 == status.total_count && 2 == status.total_count_change)
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
                return 3 == test_step;
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

        writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .make_transient(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.74");
        reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .make_transient(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.72");


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

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);

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

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 1 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 2 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 4 == status.total_count && 2 == status.total_count_change)
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
                return 3 == test_step;
            });
}

/*!
 * \test DDS-STS-SLS-10 Test `SampleLostStatus` in a Reliable DataWriter and a Reliable DataReader communication.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_re_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

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

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    int32_t test_count = 0;
    int32_t test_count_change_accum = 0;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
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
                return 4 == test_count && 4 == test_count_change_accum;
            });
}

/*!
 * \test DDS-STS-SLS-12 Test `SampleLostStatus` in a Reliable DataWriter and a Best-Effort DataReader communication.
 */
TEST(DDSStatus, sample_lost_waitset_re_dw_be_dr)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);

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

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    sample_lost_test_dw_init(writer);

    auto data = default_helloworld_data_generator(4);
    writer.send(data, 50);

    std::mutex test_step_mtx;
    std::condition_variable test_step_cv;
    uint8_t test_step = 0;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    sample_lost_test_dr_init(reader, [&test_step_mtx, &test_step_cv, &test_step](
                const eprosima::fastdds::dds::SampleLostStatus& status)
            {
                {
                    std::unique_lock<std::mutex> lock(test_step_mtx);
                    if (0 == test_step && 1 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (1 == test_step && 2 == status.total_count && 1 == status.total_count_change)
                    {
                        ++test_step;
                    }
                    else if (2 == test_step && 4 == status.total_count && 2 == status.total_count_change)
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
                return 3 == test_step;
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

        writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .make_transient(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.74");
        reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .make_transient(db_file_name, "67.62.79.64.75.62.5f.60.75.72.73.5f|76.65.79.72");

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

template<typename T>
void sample_rejected_test_dw_init(
        PubSubWriter<T>& writer)
{
    static std::set<SequenceNumber_t> samples_to_lost_only_one_time;
    static bool received_nien = false;
    samples_to_lost_only_one_time.clear();
    received_nien = false;

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->drop_data_messages_filter_ =
            [](eprosima::fastdds::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t readerID, writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                readerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                writerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);

                // restore buffer pos
                msg.pos = old_pos;

                // generate losses
                if ((writerID.value[3] & 0xC0) == 0 // only user endpoints
                        && (sn == SequenceNumber_t{0, 2} ||
                        sn == SequenceNumber_t(0, 3) ||
                        sn == SequenceNumber_t(0, 7) ||
                        sn == SequenceNumber_t(0, 8)))
                {
                    if (!received_nien || samples_to_lost_only_one_time.end() ==
                            std::find(samples_to_lost_only_one_time.begin(), samples_to_lost_only_one_time.end(), sn))
                    {
                        samples_to_lost_only_one_time.insert(sn);
                        return true;
                    }
                }
                else if (SequenceNumber_t(0, 9) == sn)
                {
                    received_nien = true;
                }

                return false;
            };
    testTransport->drop_data_frag_messages_filter_ = testTransport->drop_data_messages_filter_;

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .disable_heartbeat_piggyback(true)
            .init();

    ASSERT_TRUE(writer.isInitialized());

}

template<typename T>
void sample_rejected_test_dr_init(
        PubSubReader<T>& reader,
        std::function<void(const eprosima::fastdds::dds::SampleRejectedStatus& status)> functor)
{
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sample_rejected_status_functor(functor)
            .init();

    ASSERT_TRUE(reader.isInitialized());
}

template<typename T>
void sample_rejected_test_init(
        PubSubReader<T>& reader,
        PubSubWriter<T>& writer,
        std::function<void(const eprosima::fastdds::dds::SampleRejectedStatus& status)> functor)
{
    sample_rejected_test_dw_init(writer);
    sample_rejected_test_dr_init(reader, functor);

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();
}

/*!
 * \test DDS-STS-SRS-01 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication when reader is configured with `KEEP_ALL_HISTORY_QOS` policy and
 * `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_nokey_re_dw_re_dr_keep_all_max_samples_2)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(2);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });


    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_EQ(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-02 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication when reader is configured with `KEEP_ALL_HISTORY_QOS` policy and
 * `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_key_re_dw_re_dr_keep_all_max_samples_2)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(2)
            .resource_limits_max_instances(2)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyedhelloworld_data_generator(10);

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_NE(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-03 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication using a large type when reader is configured with
 * `KEEP_ALL_HISTORY_QOS` policy and `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_nokey_large_re_dw_re_dr_keep_all_max_samples_2)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(2);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });


    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_EQ(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-04 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication using a large type when reader is configured with `KEEP_ALL_HISTORY_QOS`
 * policy and `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_key_large_re_dw_re_dr_keep_all_max_samples_2)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(2)
            .resource_limits_max_instances(2)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyeddata300kb_data_generator(10);

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_NE(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-05 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication when reader is configured with `KEEP_LAST_HISTORY_QOS` policy and
 * `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_nokey_re_dw_re_dr_keep_last_max_samples_2)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples(2);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });


    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_EQ(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-06 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication when reader is configured with `KEEP_LAST_HISTORY_QOS` policy and
 * `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_key_re_dw_re_dr_keep_last_max_samples_2)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples(2)
            .resource_limits_max_instances(2)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyedhelloworld_data_generator(10);

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_NE(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-07 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication using a large type when reader is configured with
 * `KEEP_LAST_HISTORY_QOS` policy and `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_nokey_large_re_dw_re_dr_keep_last_max_samples_2)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples(2);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });


    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_EQ(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-08 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication using a large type when reader is configured with `KEEP_LAST_HISTORY_QOS`
 * policy and `max_samples = 2`.
 */
TEST(DDSStatus, sample_rejected_key_large_re_dw_re_dr_keep_last_max_samples_2)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples(2)
            .resource_limits_max_instances(2)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyeddata300kb_data_generator(10);

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_NE(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-09 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication when reader is configured with `KEEP_ALL_HISTORY_QOS` policy and
 * `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_re_dw_re_dr_keep_all_max_samples_per_instance_1)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-10 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication when reader is configured with `KEEP_ALL_HISTORY_QOS` policy and
 * `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_key_re_dw_re_dr_keep_all_max_samples_per_instance_1)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyedhelloworld_data_generator(10);
    auto instance_1 = writer.register_instance(*data.begin());
    auto instance_2 = writer.register_instance(*std::next(data.begin()));

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status, &instance_1, &instance_2](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                if ((1 == status.total_count && instance_2 == status.last_instance_handle) ||
                (2 == status.total_count && instance_1 == status.last_instance_handle) ||
                (3 == status.total_count && instance_2 == status.last_instance_handle) ||
                (4 == status.total_count && instance_1 == status.last_instance_handle) ||
                (5 == status.total_count && instance_2 == status.last_instance_handle))
                {
                    test_status.total_count = status.total_count;
                    test_status.total_count_change += status.total_count_change;
                    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT, status.last_reason);
                    test_status.last_reason = status.last_reason;
                    test_status.last_instance_handle = status.last_instance_handle;
                }
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT, test_status.last_reason);
    ASSERT_EQ(instance_2, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-11 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication using a large type when reader is configured with
 * `KEEP_ALL_HISTORY_QOS` policy and `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_large_re_dw_re_dr_keep_all_max_samples_per_instance_1)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-12 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication using a large type when reader is configured with `KEEP_ALL_HISTORY_QOS`
 * policy and `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_key_large_re_dw_re_dr_keep_all_max_samples_per_instance_1)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyeddata300kb_data_generator(10);
    auto instance_1 = writer.register_instance(*data.begin());
    auto instance_2 = writer.register_instance(*std::next(data.begin()));

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status, instance_1, instance_2](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                if ((1 == status.total_count && instance_2 == status.last_instance_handle) ||
                (2 == status.total_count && instance_1 == status.last_instance_handle) ||
                (3 == status.total_count && instance_2 == status.last_instance_handle) ||
                (4 == status.total_count && instance_1 == status.last_instance_handle) ||
                (5 == status.total_count && instance_2 == status.last_instance_handle))
                {
                    test_status.total_count = status.total_count;
                    test_status.total_count_change += status.total_count_change;
                    test_status.last_instance_handle = status.last_instance_handle;
                    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT, status.last_reason);
                    test_status.last_reason = status.last_reason;
                    test_status.last_instance_handle = status.last_instance_handle;
                }
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(5u, test_status.total_count);
    ASSERT_EQ(5u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT, test_status.last_reason);
    ASSERT_EQ(instance_2, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-13 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication when reader is configured with `KEEP_LAST_HISTORY_QOS` policy and
 * `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_re_dw_re_dr_keep_last_max_samples_per_instance_1)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-14 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication when reader is configured with `KEEP_LAST_HISTORY_QOS` policy and
 * `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_key_re_dw_re_dr_keep_last_max_samples_per_instance_1)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyedhelloworld_data_generator(10);

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-15 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication using a large type when reader is configured with
 * `KEEP_LAST_HISTORY_QOS` policy and `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_large_re_dw_re_dr_keep_last_max_samples_per_instance_1)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-16 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication using a large type when reader is configured with `KEEP_LAST_HISTORY_QOS`
 * policy and `max_samples_per_instance = 1`.
 */
TEST(DDSStatus, sample_rejected_key_large_re_dw_re_dr_keep_last_max_samples_per_instance_1)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_samples_per_instance(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyeddata300kb_data_generator(10);

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-17 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication when reader is configured with `KEEP_ALL_HISTORY_QOS` policy and
 * `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_re_dw_re_dr_keep_all_max_instances_1)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-18 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication when reader is configured with `KEEP_ALL_HISTORY_QOS` policy and
 * `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_key_re_dw_re_dr_keep_all_max_instances_1)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyedhelloworld_data_generator(9);
    auto instance_2 = writer.register_instance(*std::next(data.begin()));

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status, instance_2](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                ASSERT_EQ(instance_2, status.last_instance_handle);
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 9});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(4u, test_status.total_count);
    ASSERT_EQ(4u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, test_status.last_reason);
    ASSERT_EQ(instance_2, test_status.last_instance_handle);
}


/*!
 * \test DDS-STS-SRS-19 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication using a large type when reader is configured with
 * `KEEP_ALL_HISTORY_QOS` policy and `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_large_re_dw_re_dr_keep_all_max_instances_1)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-20 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication using a large type when reader is configured with `KEEP_ALL_HISTORY_QOS`
 * policy and `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_key_large_re_dw_re_dr_keep_all_max_instances_1)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyeddata300kb_data_generator(9);
    auto instance_2 = writer.register_instance(*std::next(data.begin()));

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status, instance_2](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                ASSERT_EQ(instance_2, status.last_instance_handle);
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 9});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(4u, test_status.total_count);
    ASSERT_EQ(4u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, test_status.last_reason);
    ASSERT_EQ(instance_2, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-21 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication when reader is configured with `KEEP_LAST_HISTORY_QOS` policy and
 * `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_re_dw_re_dr_keep_last_max_instances_1)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_helloworld_data_generator(10);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}


/*!
 * \test DDS-STS-SRS-22 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication when reader is configured with `KEEP_LAST_HISTORY_QOS` policy and
 * `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_key_re_dw_re_dr_keep_last_max_instances_1)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyedhelloworld_data_generator(9);
    auto instance_2 = writer.register_instance(*std::next(data.begin()));

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status, instance_2](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                ASSERT_EQ(instance_2, status.last_instance_handle);
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data);

    reader.block_for_seq({0, 9});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(4u, test_status.total_count);
    ASSERT_EQ(4u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, test_status.last_reason);
    ASSERT_EQ(instance_2, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-23 Test `SampleRejectedStatus` in a Reliable Non-keyed DataWriter and
 * a Reliable Non-keyed DataReader communication using a large type when reader is configured with
 * `KEEP_LAST_HISTORY_QOS` policy and `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_nokey_large_re_dw_re_dr_keep_last_max_instances_1)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_init(reader, writer, [&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
            });


    auto data = default_data300kb_data_generator(10);

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 10});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(0u, test_status.total_count);
}

/*!
 * \test DDS-STS-SRS-24 Test `SampleRejectedStatus` in a Reliable Keyed DataWriter and
 * a Reliable Keyed DataReader communication using a large type when reader is configured with `KEEP_LAST_HISTORY_QOS`
 * policy and `max_instances = 1`.
 */
TEST(DDSStatus, sample_rejected_key_large_re_dw_re_dr_keep_last_max_instances_1)
{
    PubSubReader<KeyedData1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedData1mbPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Avoid losing more frangments
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 132000, 50);
    reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .resource_limits_max_instances(1);

    sample_rejected_test_dw_init(writer);

    auto data = default_keyeddata300kb_data_generator(9);
    auto instance_2 = writer.register_instance(*std::next(data.begin()));

    sample_rejected_test_dr_init(reader, [&test_mtx, &test_status, instance_2](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                test_status.last_instance_handle = status.last_instance_handle;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                ASSERT_EQ(instance_2, status.last_instance_handle);
                test_status.last_instance_handle = status.last_instance_handle;
            });

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(data);
    writer.send(data, 50);

    reader.block_for_seq({0, 9});

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(4u, test_status.total_count);
    ASSERT_EQ(4u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT, test_status.last_reason);
    ASSERT_EQ(instance_2, test_status.last_instance_handle);
}

/*!
 * \test DDS-STS-SRS-25 Test `SampleRejectedStatus` and Waitsets
 */
TEST(DDSStatus, sample_rejected_waitset)
{
    PubSubReaderWithWaitsets<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    std::mutex test_mtx;
    eprosima::fastdds::dds::SampleRejectedStatus test_status;

    int skip_step = 0;
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->drop_data_messages_filter_ =
            [&skip_step](eprosima::fastdds::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t readerID, writerID;
                SequenceNumber_t sn;
                bool ret = false;

                msg.pos += 2; // flags
                msg.pos += 2; // octets to inline quos
                readerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                writerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);

                // restore buffer pos
                msg.pos = old_pos;

                // generate losses
                if ((writerID.value[3] & 0xC0) == 0) // only user endpoints
                {
                    if (SequenceNumber_t{0, 1} == sn)
                    {
                        if (0 == skip_step)
                        {
                            return true;
                        }
                        else
                        {
                            if (1 == skip_step)
                            {
                                ++skip_step;
                            }
                        }
                    }
                    else if (SequenceNumber_t{0, 2} == sn)
                    {
                        if (0 == skip_step)
                        {
                            ++skip_step;
                        }
                        else if (2 <= skip_step && 12 > skip_step) // Could be several network interfaces.
                        {
                            ret =  true;
                            ++skip_step;
                        }
                    }
                }

                return ret;
            };
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .disable_heartbeat_piggyback(true)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams( // Be sure are sent in separate submessage each DATA.
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 300, 300) // Be sure the first message is processed before sending the second.
            .init();

    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .resource_limits_max_samples(1)
            .sample_rejected_status_functor([&test_mtx, &test_status](
                const eprosima::fastdds::dds::SampleRejectedStatus& status)
            {
                std::unique_lock<std::mutex> lock(test_mtx);
                test_status.total_count = status.total_count;
                test_status.total_count_change += status.total_count_change;
                ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, status.last_reason);
                test_status.last_reason = status.last_reason;
                test_status.last_instance_handle = status.last_instance_handle;
            })
            .init();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(2);

    reader.startReception(data);
    writer.send(data);

    reader.block_for_all();

    std::unique_lock<std::mutex> lock(test_mtx);
    ASSERT_EQ(1u, test_status.total_count);
    ASSERT_EQ(1u, test_status.total_count_change);
    ASSERT_EQ(eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT, test_status.last_reason);
    ASSERT_EQ(c_InstanceHandle_Unknown, test_status.last_instance_handle);
}

template<typename T>
void best_effort_on_unack_test_init(
        PubSubWriter<T>& writer,
        PubSubReader<T>& reader)
{
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 1, 1000)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    reader.init();
    ASSERT_TRUE(reader.isInitialized());
}

/*!
 * \test DDS-USR-01 test: `on_unacknowledged_sample_removed` callback with Best Effort DataWriter
 * Constrained History with Flow Controller configured preventing the sending of samples.
 */
TEST(DDSStatus, best_effort_on_unack_sample_removed)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    best_effort_on_unack_test_init(writer, reader);

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    writer.send(data);

    EXPECT_EQ(reader.getReceivedCount(), 0u);
    EXPECT_EQ(writer.times_unack_sample_removed(), 9u);

    auto instances = writer.instances_removed_unack();
    for (auto instance : instances)
    {
        EXPECT_EQ(instance, eprosima::fastdds::dds::HANDLE_NIL);
    }
}

/*!
 * \test DDS-USR-02 test: `on_unacknowledged_sample_removed` callback with Best Effort Keyed DataWriter
 */
TEST(DDSStatus, keyed_best_effort_on_unack_sample_removed)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    best_effort_on_unack_test_init(writer, reader);

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator();

    auto dummy_data = new KeyedHelloWorldPubSubType();
    eprosima::fastdds::dds::InstanceHandle_t handle_odd;
    eprosima::fastdds::dds::InstanceHandle_t handle_even;
    dummy_data->compute_key(&data.front(), handle_even);
    dummy_data->compute_key(&data.back(), handle_odd);

    reader.startReception(data);
    writer.send(data);

    EXPECT_EQ(reader.getReceivedCount(), 0u);
    EXPECT_EQ(writer.times_unack_sample_removed(), 8u);

    auto instances = writer.instances_removed_unack();
    uint32_t index = 0;
    for (auto instance : instances)
    {
        EXPECT_EQ(instance, (index % 2) == 0 ? handle_even : handle_odd);
        index++;
    }
    delete dummy_data;
}

// Auxiliary method to initialize Reliable DataWriter configuring sample drops in the transport
template<typename T>
void reliable_on_unack_test_init(
        PubSubWriter<T>& writer,
        PubSubReader<T>& reader)
{
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->drop_data_messages_filter_ = [](eprosima::fastdds::rtps::CDRMessage_t& msg)-> bool
            {
                uint32_t old_pos = msg.pos;

                // see RTPS DDS 9.4.5.3 Data Submessage
                EntityId_t writerID;
                SequenceNumber_t sn;

                msg.pos += 2; // flags
                msg.pos += 2; // inline QoS
                msg.pos += 4; // readerID
                writerID = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32(
                    (char*)&msg.buffer[msg.pos]);

                // restore buffer pos
                msg.pos = old_pos;

                // generate losses
                if ((writerID.value[3] & 0xC0) == 0 // only user endpoints
                        && (sn == SequenceNumber_t{0, 2} ||
                        sn == SequenceNumber_t{0, 4}))
                {
                    return true;
                }

                return false;
            };


    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());
}

/*!
 * \test DDS-USR-03 test: `on_unacknowledged_sample_removed` callback with Reliable DataWriter
 * Drop samples using test_UDPv4Transport
 */
TEST(DDSStatus, reliable_on_unack_sample_removed)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reliable_on_unack_test_init(writer, reader);

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Ensure that the ACK has been received (except for the samples dropped by the transport that timeout)
    for (auto sample : data)
    {
        writer.send_sample(sample);
        writer.waitForAllAcked(std::chrono::milliseconds(250));
    }

    reader.block_for_at_least(8);
    EXPECT_EQ(reader.getReceivedCount(), 8u);
    EXPECT_EQ(writer.times_unack_sample_removed(), 2u);

    auto instances = writer.instances_removed_unack();
    for (auto instance : instances)
    {
        EXPECT_EQ(instance, eprosima::fastdds::dds::HANDLE_NIL);
    }
}

/*!
 * DDS-USR-04 test: `on_unacknowledged_sample_removed` callback with Reliable Keyed DataWriter
 */
TEST(DDSStatus, keyed_reliable_on_unack_sample_removed)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reliable_on_unack_test_init(writer, reader);

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator();

    auto dummy_data = new KeyedHelloWorldPubSubType();
    eprosima::fastdds::dds::InstanceHandle_t handle;
    dummy_data->compute_key(&data.back(), handle);

    reader.startReception(data);
    // To avoid race condition receiving ACK, wait some time between samples
    // Ensure that the ACK has been received (except for the samples dropped by the transport that timeout)
    for (auto sample : data)
    {
        writer.send_sample(sample);
        writer.waitForAllAcked(std::chrono::milliseconds(250));
    }

    reader.block_for_at_least(8);
    EXPECT_EQ(reader.getReceivedCount(), 8u);
    EXPECT_EQ(writer.times_unack_sample_removed(), 2u);

    auto instances = writer.instances_removed_unack();
    for (auto instance : instances)
    {
        EXPECT_EQ(instance, handle);
    }
    delete dummy_data;
}

class CustomDataWriterListener : public eprosima::fastdds::dds::DataWriterListener
{
public:

    CustomDataWriterListener()
        : times_unack_sample_removed_(0)
    {
    }

    ~CustomDataWriterListener() = default;

    void on_unacknowledged_sample_removed(
            eprosima::fastdds::dds::DataWriter* datawriter,
            const eprosima::fastdds::dds::InstanceHandle_t& handle) override
    {
        notified_writer_ = datawriter;
        if (writer_ == datawriter)
        {
            times_unack_sample_removed_++;
        }
        instances_removed_unack_.push_back(handle);
    }

    void assign_writer(
            eprosima::fastdds::dds::DataWriter* writer)
    {
        writer_ = writer;
    }

    eprosima::fastdds::dds::DataWriter* notified_writer() const
    {
        return notified_writer_;
    }

    unsigned int times_unack_sample_removed() const
    {
        return times_unack_sample_removed_;
    }

    std::vector<eprosima::fastdds::dds::InstanceHandle_t> instances_removed_unack() const
    {
        return instances_removed_unack_;
    }

private:

    CustomDataWriterListener& operator =(
            const CustomDataWriterListener&) = delete;

    //! DataWriter attached to the listener
    eprosima::fastdds::dds::DataWriter* writer_;
    //! DataWriter notified by the callback
    eprosima::fastdds::dds::DataWriter* notified_writer_;
    //! Number of times a sample has been removed unacknowledged
    unsigned int times_unack_sample_removed_;
    //! Instances notified in the callback
    std::vector<eprosima::fastdds::dds::InstanceHandle_t> instances_removed_unack_;
};

template<typename T>
void reliable_disable_acks_on_unack_test_init(
        PubSubWriter<T>& writer_1,
        PubSubWriter<T>& writer_2,
        PubSubReader<T>& reader)
{
    writer_1.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 1, 1000)
            .keep_duration(eprosima::fastdds::dds::c_TimeInfinite)
            .init();
    ASSERT_TRUE(writer_1.isInitialized());

    writer_2.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .keep_duration(eprosima::fastdds::dds::c_TimeInfinite)
            .init();
    ASSERT_TRUE(writer_2.isInitialized());

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(reader.isInitialized());
}

/*!
 * DDS-USR-05 test: `on_unacknowledged_sample_removed` callback with reliable DataWriter and disable positive ACKs QoS
 * enabled.
 * Check that the callback returns the proper DataWriter.
 */
TEST(DDSStatus, reliable_positive_acks_disabled_on_unack_sample_removed)
{
    PubSubWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reliable_disable_acks_on_unack_test_init(writer_1, writer_2, reader);

    writer_1.wait_discovery();
    writer_2.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    auto data_2 = data;

    reader.startReception(data);

    CustomDataWriterListener listener;
    listener.assign_writer(&writer_1.get_native_writer());
    writer_1.get_native_writer().set_listener(&listener);
    writer_2.get_native_writer().set_listener(&listener);

    writer_1.send(data);

    EXPECT_EQ(reader.getReceivedCount(), 0u);
    EXPECT_EQ(listener.times_unack_sample_removed(), 9u);
    EXPECT_EQ(listener.notified_writer(), &(writer_1.get_native_writer()));

    listener.assign_writer(&writer_2.get_native_writer());

    writer_2.send(data_2);

    reader.block_for_at_least(10);
    EXPECT_EQ(reader.getReceivedCount(), 10u);
    EXPECT_EQ(listener.times_unack_sample_removed(), 9u);
    EXPECT_EQ(listener.notified_writer(), &(writer_1.get_native_writer()));

    auto instances = listener.instances_removed_unack();
    for (auto instance : instances)
    {
        EXPECT_EQ(instance, eprosima::fastdds::dds::HANDLE_NIL);
    }
}

/*!
 * DDS-USR-06 test: `on_unacknowledged_sample_removed` callback with reliable keyed DataWriter and disable positive ACKs
 * QoS enabled.
 */
TEST(DDSStatus, keyed_reliable_positive_acks_disabled_on_unack_sample_removed)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reliable_disable_acks_on_unack_test_init(writer_1, writer_2, reader);

    writer_1.wait_discovery();
    writer_2.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator();
    auto data_2 = data;

    auto dummy_data = new KeyedHelloWorldPubSubType();
    eprosima::fastdds::dds::InstanceHandle_t handle_odd;
    eprosima::fastdds::dds::InstanceHandle_t handle_even;
    dummy_data->compute_key(&data.front(), handle_even);
    dummy_data->compute_key(&data.back(), handle_odd);

    reader.startReception(data);

    CustomDataWriterListener listener;
    listener.assign_writer(&writer_1.get_native_writer());
    writer_1.get_native_writer().set_listener(&listener);
    writer_2.get_native_writer().set_listener(&listener);

    writer_1.send(data);

    EXPECT_EQ(reader.getReceivedCount(), 0u);
    EXPECT_EQ(listener.times_unack_sample_removed(), 8u);
    EXPECT_EQ(listener.notified_writer(), &(writer_1.get_native_writer()));

    auto instances = listener.instances_removed_unack();
    uint32_t index = 0;
    for (auto instance : instances)
    {
        EXPECT_EQ(instance, (index % 2) == 0 ? handle_even : handle_odd);
        index++;
    }

    listener.assign_writer(&writer_2.get_native_writer());

    writer_2.send(data_2);

    reader.block_for_at_least(10);
    EXPECT_EQ(reader.getReceivedCount(), 10u);
    EXPECT_EQ(listener.times_unack_sample_removed(), 8u);
    EXPECT_EQ(listener.notified_writer(), &(writer_1.get_native_writer()));
    delete dummy_data;
}

/*!
 * Regression Test for 22658: when the entire history is acked in volatile, given that the entries are deleted from the
 * history, check_acked_status satisfies min_low_mark >= get_seq_num_min() because seq_num_min is unknown. This makes
 * try_remove to fail, because it tries to remove changes but there were none. This causes prepare_change to not
 * perform the changes, since the history was full and could not delete any changes.
 */

TEST(DDSStatus, entire_history_acked_volatile_unknown_pointer)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS, eprosima::fastdds::dds::Duration_t (200, 0))
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_instances(1)
            .resource_limits_max_samples(1)
            .resource_limits_max_samples_per_instance(1)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(2);
    for (auto sample : data)
    {
        // A value of true means that the sample was sent successfully.
        // This aligns with the expected behaviour of having the history
        // acknowledged and emptied before the next message.
        EXPECT_TRUE(writer.send_sample(sample));
    }
}

/*¡
 * Regression Test for 22648: on_unacknowledged_sample_removed callback is called when writer with keep all
 * history is used, when the history was full but before max_blocking_time a sample was acknowledged, as is_acked was
 * checked before the waiting time, and is not re-checked. This should not happen.
 */
TEST(DDSStatus, reliable_keep_all_unack_sample_removed_call)
{
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->drop_data_messages_filter_ = [](eprosima::fastdds::rtps::CDRMessage_t& msg) -> bool
            {
                static std::vector<std::pair<eprosima::fastdds::rtps::SequenceNumber_t,
                        std::chrono::steady_clock::time_point>> delayed_messages;

                uint32_t old_pos = msg.pos;

                // Parse writer ID and sequence number
                msg.pos += 2; // flags
                msg.pos += 2; // inline QoS
                msg.pos += 4; // reader ID
                auto writerID = eprosima::fastdds::helpers::cdr_parse_entity_id((char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                eprosima::fastdds::rtps::SequenceNumber_t sn;
                sn.high = (int32_t)eprosima::fastdds::helpers::cdr_parse_u32((char*)&msg.buffer[msg.pos]);
                msg.pos += 4;
                sn.low = eprosima::fastdds::helpers::cdr_parse_u32((char*)&msg.buffer[msg.pos]);

                // Restore buffer position
                msg.pos = old_pos;

                // Delay logic for user endpoints only
                if ((writerID.value[3] & 0xC0) == 0) // only user endpoints
                {
                    auto now = std::chrono::steady_clock::now();
                    auto it = std::find_if(delayed_messages.begin(), delayed_messages.end(),
                                    [&sn](const auto& pair)
                                    {
                                        return pair.first == sn;
                                    });

                    if (it == delayed_messages.end())
                    {
                        // If the sequence number is encountered for the first time, start the delay
                        delayed_messages.emplace_back(sn, now + std::chrono::milliseconds(750)); // Add delay
                        return true; // Start dropping this message
                    }
                    else if (now < it->second)
                    {
                        // If the delay period has not elapsed, keep dropping the message
                        return true;
                    }
                    else
                    {
                        // Once the delay has elapsed, allow the message to proceed
                        delayed_messages.erase(it);
                    }
                }
                return false; // Allow message to proceed
            };

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS, eprosima::fastdds::dds::Duration_t (200, 0))
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_instances(1)
            .resource_limits_max_samples(1)
            .resource_limits_max_samples_per_instance(1)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(2);

    for (auto sample : data)
    {
        writer.send_sample(sample);
    }

    EXPECT_EQ(writer.times_unack_sample_removed(), 0u);
}

/*!
 * Test that checks with a writer of each type that having the same listener attached, the notified writer in the
 * callback is the corresponding writer that has removed a sample unacknowledged.
 */
TEST(DDSStatus, several_writers_on_unack_sample_removed)
{
    PubSubWriter<HelloWorldPubSubType> best_effort_writer(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> reliable_writer(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> ack_disabled_writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> best_effort_reader(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reliable_reader(TEST_TOPIC_NAME);

    best_effort_writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 1, 1000)
            .init();
    ASSERT_TRUE(best_effort_writer.isInitialized());

    best_effort_reader.init();
    ASSERT_TRUE(best_effort_reader.isInitialized());

    reliable_on_unack_test_init(reliable_writer, reliable_reader);

    ack_disabled_writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
            .history_depth(1)
            .asynchronously(eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE)
            .add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, 1, 1000)
            .keep_duration(eprosima::fastdds::dds::c_TimeInfinite)
            .init();
    ASSERT_TRUE(ack_disabled_writer.isInitialized());

    // The only non matching case is the best effort writer with the reliable reader
    best_effort_writer.wait_discovery(1u, std::chrono::seconds::zero());
    reliable_writer.wait_discovery(2u, std::chrono::seconds::zero());
    ack_disabled_writer.wait_discovery(2u, std::chrono::seconds::zero());
    best_effort_reader.wait_discovery(std::chrono::seconds::zero(), 3u);
    reliable_reader.wait_discovery(std::chrono::seconds::zero(), 2u);

    auto best_effort_data = default_helloworld_data_generator();
    auto reliable_data = best_effort_data;
    auto ack_disabled_data = best_effort_data;

    best_effort_reader.startReception(best_effort_data);
    reliable_reader.startReception(reliable_data);

    CustomDataWriterListener listener;
    best_effort_writer.get_native_writer().set_listener(&listener);
    reliable_writer.get_native_writer().set_listener(&listener);
    ack_disabled_writer.get_native_writer().set_listener(&listener);

    // Send two samples from each writer (overwrite first sample)
    best_effort_writer.send_sample(best_effort_data.front());
    best_effort_data.pop_front();
    best_effort_writer.send_sample(best_effort_data.front());
    EXPECT_EQ(listener.notified_writer(), &(best_effort_writer.get_native_writer()));
    reliable_writer.send_sample(reliable_data.front());
    reliable_writer.waitForAllAcked(std::chrono::milliseconds(150));
    EXPECT_EQ(listener.notified_writer(), &(best_effort_writer.get_native_writer()));
    ack_disabled_writer.send_sample(ack_disabled_data.front());
    ack_disabled_data.pop_front();
    ack_disabled_writer.send_sample(ack_disabled_data.front());
    EXPECT_EQ(listener.notified_writer(), &(ack_disabled_writer.get_native_writer()));
    reliable_data.pop_front();
    reliable_writer.send_sample(reliable_data.front());
    reliable_data.pop_front();
    reliable_writer.send_sample(reliable_data.front());
    reliable_writer.waitForAllAcked(std::chrono::milliseconds(150));
    EXPECT_EQ(listener.notified_writer(), &(reliable_writer.get_native_writer()));
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
