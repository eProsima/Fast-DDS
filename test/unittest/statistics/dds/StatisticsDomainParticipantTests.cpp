// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <statistics/types/typesPubSubTypes.hpp>

#include "../../logging/mock/MockConsumer.h"

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class FooType
{
public:

    FooType()
    {
    }

    ~FooType()
    {
    }

    inline std::string& message()
    {
        return message_;
    }

    inline void message(
            const std::string& message)
    {
        message_ = message;
    }

    bool isKeyDefined()
    {
        return false;
    }

private:

    std::string message_;
};

class TopicDataTypeMock : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef FooType type;

    TopicDataTypeMock()
        : eprosima::fastdds::dds::TopicDataType()
    {
        max_serialized_type_size = 4u;
        set_name("footype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::dds::DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            fastdds::dds::DataRepresentationId_t /*data_representation*/) override
    {
        return 0;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

private:

    using eprosima::fastdds::dds::TopicDataType::calculate_serialized_size;
    using eprosima::fastdds::dds::TopicDataType::serialize;
};

class StatisticsDomainParticipantTests : public ::testing::Test
{
public:

    void helper_block_for_at_least_entries(
            uint32_t amount)
    {
        mock_consumer_->wait_for_at_least_entries(amount);
    }

    eprosima::fastdds::dds::MockConsumer* mock_consumer_;

};

/*
 * This test checks eprosima::fastdds::statistics::dds::DomainParticipant narrow methods.
 * 1. Create a eprosima::fastdds::dds::DomainParticipant
 * 2. Use both narrow methods to obtain the pointer to the children class.
 * If FASTDDS_STATISTICS option is not set, nullptr is expected.
 * Otherwise, a valid pointer is expected.
 * 3. Call both narrow methods with an invalid pointer and check that it returns nullptr
 * 4. Delete DDS entities
 */
TEST_F(StatisticsDomainParticipantTests, NarrowDomainParticipantTest)
{
    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    const eprosima::fastdds::dds::DomainParticipant* const_participant = participant;

    // 2. Call to both narrow methods
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    const DomainParticipant* const_statistics_participant = DomainParticipant::narrow(const_participant);
#ifndef FASTDDS_STATISTICS
    EXPECT_EQ(statistics_participant, nullptr);
    EXPECT_EQ(const_statistics_participant, nullptr);
#else
    EXPECT_NE(statistics_participant, nullptr);
    EXPECT_NE(const_statistics_participant, nullptr);
#endif // FASTDDS_STATISTICS

    // 3. Call narrow methods with invalid parameter
    eprosima::fastdds::dds::DomainParticipant* null_participant = nullptr;
    statistics_participant = DomainParticipant::narrow(null_participant);
    EXPECT_EQ(statistics_participant, nullptr);

    const_participant = nullptr;
    const_statistics_participant = DomainParticipant::narrow(const_participant);
    EXPECT_EQ(const_statistics_participant, nullptr);

    // 4. Delete DDS entities
    ASSERT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant),
            fastdds::dds::RETCODE_OK);
}

/*
 * This test checks both eprosima::fastdds::statistics::dds::DomainParticipant enable_statistics_datawriter() and
 * disable_statistics_datawriter() methods.
 * 1. If the FASTDDS_STATISTICS compilation flag is not set, check that the methods return fastdds::dds::RETCODE_UNSUPPORTED.
 * Check that this error takes precedence over other possible errors.
 * 2. Narrow DomainParticipant to the children class.
 * 3. Create TypeSupports.
 * 4. Check that the types are not registered yet.
 * 5. Check that the topics do not exist yet.
 * 6. Enable each statistics DataWriter checking that topics are created and types are registered.
 * 7. Enable an already enabled statistics DataWriter and check that it returns fastdds::dds::RETCODE_OK.
 * 8. Call enable_statistics_datawriter method with an invalid topic name and check that returns fastdds::dds::RETCODE_BAD_PARAMETER.
 * 9. Disable one statistics DataWriter and check that it is successful.
 * 10. Enable the previous statistics DataWriter with an inconsistent QoS and check that it returns
 * fastdds::dds::RETCODE_INCONSISTENT_POLICY.
 * 11. Check error code precedence: fastdds::dds::RETCODE_BAD_PARAMETER takes precedence over fastdds::dds::RETCODE_INCONSISTENT_POLICY.
 * The case where the create_datawriter fails returning fastdds::dds::RETCODE_ERROR is not checked because it only passes the error
 * upstream.
 * 12. Try to disable an already disabled statistics DataWriter and check that returns fastdds::dds::RETCODE_ERROR.
 * 13. Check that if an invalid topic name is provided to the disable_statistics_datawriter method, it returns
 * fastdds::dds::RETCODE_BAD_PARAMETER.
 * The case where the delete_datawriter fails returning fastdds::dds::RETCODE_ERROR is not checked because it only passes the error
 * upstream.
 * 14. Delete DDS entities.
 */
TEST_F(StatisticsDomainParticipantTests, EnableDisableStatisticsDataWriterTest)
{
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    eprosima::fastdds::dds::DataWriterQos inconsistent_qos = STATISTICS_DATAWRITER_QOS;
    inconsistent_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    inconsistent_qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;

#ifndef FASTDDS_STATISTICS
    // 1. Compilation flag not set
    DomainParticipant* statistics_participant = static_cast<DomainParticipant*>(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, statistics_participant->enable_statistics_datawriter(
                HISTORY_LATENCY_TOPIC, STATISTICS_DATAWRITER_QOS));
    EXPECT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, statistics_participant->enable_statistics_datawriter("INVALID_TOPIC",
            inconsistent_qos));
    EXPECT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, statistics_participant->disable_statistics_datawriter(
                HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(fastdds::dds::RETCODE_UNSUPPORTED, statistics_participant->disable_statistics_datawriter(
                "INVALID_TOPIC"));
#else
    // 2. Narrow DomainParticipant to eprosima::fastdds::statistics::dds::DomainParticipant
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 3. Create TypeSupports for the different DataTypes
    eprosima::fastdds::dds::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
    history_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
    network_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport throughput_type(new EntityDataPubSubType);
    throughput_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
    rtps_traffic_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
    count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport discovery_type(new DiscoveryTimePubSubType);
    discovery_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
    sample_identity_count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
    physical_data_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);

    // 4. Check that the types are not registered yet
    EXPECT_EQ(null_type, statistics_participant->find_type(history_latency_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(network_latency_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(throughput_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(rtps_traffic_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(discovery_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(sample_identity_count_type.get_type_name()));
    EXPECT_EQ(null_type, statistics_participant->find_type(physical_data_type.get_type_name()));

    // 5. Check that the topics do not exist
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));

    // 6. Enable each statistics DataWriter checking that topics are created and types are registered.
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HISTORY_LATENCY_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_TRUE(history_latency_type == statistics_participant->find_type(history_latency_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(NETWORK_LATENCY_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_TRUE(network_latency_type == statistics_participant->find_type(network_latency_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(
                PUBLICATION_THROUGHPUT_TOPIC, STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_TRUE(throughput_type == statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(
                SUBSCRIPTION_THROUGHPUT_TOPIC, STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_TRUE(throughput_type == statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(RTPS_SENT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    EXPECT_TRUE(rtps_traffic_type == statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(RTPS_LOST_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_TRUE(rtps_traffic_type == statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(RESENT_DATAS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(ACKNACK_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(NACKFRAG_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(GAP_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(DATA_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PDP_PACKETS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(EDP_PACKETS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(DISCOVERY_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_TRUE(discovery_type == statistics_participant->find_type(discovery_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(SAMPLE_DATAS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_TRUE(sample_identity_count_type == statistics_participant->find_type(
                sample_identity_count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PHYSICAL_DATA_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_TRUE(physical_data_type == statistics_participant->find_type(physical_data_type.get_type_name()));

    // 7. Enable an already enabled statistics DataWriter
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(SAMPLE_DATAS_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 8. Invalid topic name
    EXPECT_EQ(fastdds::dds::RETCODE_BAD_PARAMETER, statistics_participant->enable_statistics_datawriter("INVALID_TOPIC",
            STATISTICS_DATAWRITER_QOS));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription("INVALID_TOPIC"));

    // 9. Disable statistics DataWriter
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(history_latency_type.get_type_name()));

    // 10. Enable previous statistics DataWriter with an inconsistent QoS
    EXPECT_EQ(fastdds::dds::RETCODE_INCONSISTENT_POLICY, statistics_participant->enable_statistics_datawriter(
                HISTORY_LATENCY_TOPIC, inconsistent_qos));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(history_latency_type.get_type_name()));

    // 11. fastdds::dds::RETCODE_BAD_PARAMETER error has precedence over fastdds::dds::RETCODE_INCONSISTENT_POLICY
    EXPECT_EQ(fastdds::dds::RETCODE_BAD_PARAMETER, statistics_participant->enable_statistics_datawriter("INVALID_TOPIC",
            inconsistent_qos));

    // 12. Disable already disabled DataWriter
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(HISTORY_LATENCY_TOPIC));

    // 13. Disable invalid topic name
    EXPECT_EQ(fastdds::dds::RETCODE_BAD_PARAMETER, statistics_participant->disable_statistics_datawriter(
                "INVALID_TOPIC"));

    // 14. Remove DDS entities
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(network_latency_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(
                PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(throughput_type == statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(
                SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(RTPS_SENT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(rtps_traffic_type == statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(RTPS_LOST_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(RESENT_DATAS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(ACKNACK_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(NACKFRAG_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(GAP_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(DATA_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(PDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(EDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(DISCOVERY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(discovery_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(sample_identity_count_type.get_type_name()));

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(physical_data_type.get_type_name()));
#endif // FASTDDS_STATISTICS

    EXPECT_EQ(fastdds::dds::RETCODE_OK, eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant));
}

/**
 * This test checks that when the topic name provided is not valid, a log error is printed.
 * 1. Create a participant with the property fastdds.statistics set to an invalid topic name
 * 2. Check that there is no topic/type registered in the participant
 * 3. Wait for the EPROSIMA_LOG_ERROR entry to be consumed
 */
TEST_F(StatisticsDomainParticipantTests, CreateParticipantWithInvalidTopicName)
{
#ifdef FASTDDS_STATISTICS
    mock_consumer_ = new eprosima::fastdds::dds::MockConsumer();

    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(mock_consumer_));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("(STATISTICS_DOMAIN_PARTICIPANT)"));
    eprosima::fastdds::dds::Log::SetErrorStringFilter(std::regex("(not a valid statistics topic name/alias)"));

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.properties().properties().emplace_back("fastdds.statistics",
            "INVALID_TOPIC_NAME1; INVALID_TOPIC_NAME2");

    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, pqos);
    ASSERT_NE(participant, nullptr);

    // 2. Check topics/types
    // Create TypeSupports
    eprosima::fastdds::dds::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
    history_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
    network_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport throughput_type(new EntityDataPubSubType);
    throughput_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
    rtps_traffic_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
    count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport discovery_type(new DiscoveryTimePubSubType);
    discovery_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
    sample_identity_count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
    physical_data_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);

    EXPECT_EQ(null_type, participant->find_type(history_latency_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(network_latency_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(throughput_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(rtps_traffic_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(count_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(discovery_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(sample_identity_count_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(physical_data_type.get_type_name()));

    EXPECT_EQ(nullptr, participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));

    // 3. Wait until EPROSIMA_LOG_ERROR entries are captured
    helper_block_for_at_least_entries(2);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 2u);

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant),
            fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that enable_statistics_datawriter fails returning fastdds::dds::RETCODE_ERROR when there is already a TypeSupport
 * using a statistics reserved name registered in the participant.
 * 1. Create a participant and register a TypeSupport using one of the statistics reserved type names.
 * 2. Call enable_statistics_datawriter and check that it fails.
 * 3. Check that the topic has not been created.
 * 4. Call enable_statistics_datawriter with a type correctly registered and check that it suceeds.
 * 5. Check log error entry generated in DomainParticipantImpl::register_type
 */
TEST_F(StatisticsDomainParticipantTests, EnableStatisticsDataWriterFailureIncompatibleType)
{
#ifdef FASTDDS_STATISTICS
    mock_consumer_ = new eprosima::fastdds::dds::MockConsumer();

    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(mock_consumer_));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("(PARTICIPANT)"));
    eprosima::fastdds::dds::Log::SetErrorStringFilter(std::regex("(already registered)"));

    const char* reserved_statistics_type_name = "eprosima::fastdds::statistics::EntityCount";

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Register TypeSupport
    eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);
    eprosima::fastdds::dds::TypeSupport invalid_type(new TopicDataTypeMock);
    invalid_type->set_name(reserved_statistics_type_name);
    participant->register_type(invalid_type);
    participant->register_type(physical_data_type);

    // 2. Check call to enable_statistics_datawriter
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(fastdds::dds::RETCODE_ERROR, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    eprosima::fastdds::dds::TypeSupport type = participant->find_type(count_type.get_type_name());
    EXPECT_FALSE(count_type == type);
    EXPECT_TRUE(invalid_type == type);

    // 3. Check topic creation
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    // 4. Call enable_statistics_datawriter with an already correctly registered type.
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PHYSICAL_DATA_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_TRUE(physical_data_type == participant->find_type(physical_data_type.get_type_name()));
    EXPECT_NE(nullptr, participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));

    // 5. Check log error entry
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    // delete_participant removes all builtin statistics entities
    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that enable_statistics_datawriter fails returning fastdds::dds::RETCODE_ERROR when there is already a statistics
 * Topic created with another type different from the one expected.
 * 1. Create a participant and register a Topic using one of the statistics reserved topic names and with another type
 * different from the one expected. Register another Topic correctly.
 * 2. Call enable_statistics_datawriter and check that it fails.
 * 3. Check that the type has not been registered.
 * 4. Call enable_statistics_datawriter with correct Topic and check it works correctly.
 * 5. Call enable_statistics_datawriter with Topic and Type previously created in the participant. Check it is
 * successful.
 * 6. Check log error entry generated in DomainParticipant::check_statistics_topic_and_type
 */
TEST_F(StatisticsDomainParticipantTests, EnableStatisticsDataWriterFailureIncompatibleTopic)
{
#ifdef FASTDDS_STATISTICS
    mock_consumer_ = new eprosima::fastdds::dds::MockConsumer();

    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(mock_consumer_));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("(STATISTICS_DOMAIN_PARTICIPANT)"));
    eprosima::fastdds::dds::Log::SetErrorStringFilter(std::regex("(not using expected type)"));

    eprosima::fastdds::dds::TypeSupport null_type(nullptr);
    eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
    eprosima::fastdds::dds::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Register types
    participant->register_type(count_type);
    participant->register_type(physical_data_type);

    // Create topic
    eprosima::fastdds::dds::Topic* invalid_topic = participant->create_topic(HISTORY_LATENCY_TOPIC,
                    count_type->get_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    participant->create_topic(HEARTBEAT_COUNT_TOPIC, count_type->get_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    participant->create_topic(PHYSICAL_DATA_TOPIC, physical_data_type->get_name(),
            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    // 2. Check call to enable_statistics_datawriter
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(fastdds::dds::RETCODE_ERROR, statistics_participant->enable_statistics_datawriter(HISTORY_LATENCY_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 3. Check type registration
    EXPECT_EQ(null_type, participant->find_type(history_latency_type.get_type_name()));

    // 4. Call enable_statistics_datawriter with correctly created topic
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    // 5. Call enable_statistics_datawriter and check it is successful
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PHYSICAL_DATA_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 6. Check log error entry
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    // delete_participant removes all builtin statistics entities but not others
    EXPECT_EQ(fastdds::dds::RETCODE_OK, participant->delete_topic(invalid_topic));
    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**

 * This test checks that a participant is correctly deleted by the factory
 * after calling delete_contained_entities() method
 * 1. Create a statistics participant, register type
 * 2. Create a sample topic
 * 3. Perform a delete_contained_entities() in the statistics participant
 * 4. Delete the participant
 */
TEST_F(StatisticsDomainParticipantTests, DeleteParticipantAfterDeleteContainedEntitiesFailure)
{
#ifdef FASTDDS_STATISTICS

    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);

    // 1. Create DomainParticipant with statistics
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Register type
    participant->register_type(count_type);

    // 2. Create a sample topic
    participant->create_topic(HEARTBEAT_COUNT_TOPIC,
            count_type->get_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 3. Perform a delete_contained_entities() in the statistics participant
    EXPECT_EQ(participant->delete_contained_entities(), fastdds::dds::RETCODE_OK);

    // 4. Delete the participant
    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(participant), fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::KillThread();
    return ret;
}
