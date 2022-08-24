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
#include <tinyxml2.h>

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/types/TypesBase.h>
#include <statistics/types/typesPubSubTypes.h>
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "../../logging/mock/MockConsumer.h"

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

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
        m_typeSize = 4u;
        setName("footype");
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return std::function<uint32_t()>();
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

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

class StatisticsFromXMLProfileTests : public ::testing::Test
{
public:

    class TestDomainParticipant : public eprosima::fastdds::statistics::dds::DomainParticipant
    {
    public:

        DomainParticipantImpl* get_domain_participant_impl()
        {
            return static_cast<DomainParticipantImpl*>(impl_);
        }

    };

    class TestDomainParticipantImpl : public eprosima::fastdds::statistics::dds::DomainParticipantImpl
    {
    public:

        PublisherImpl*  get_publisher_impl()
        {
            return builtin_publisher_impl_;
        }

    };
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
            eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks both eprosima::fastdds::statistics::dds::DomainParticipant enable_statistics_datawriter() and
 * disable_statistics_datawriter() methods.
 * 1. If the FASTDDS_STATISTICS compilation flag is not set, check that the methods return RETCODE_UNSUPPORTED.
 * Check that this error takes precedence over other possible errors.
 * 2. Narrow DomainParticipant to the children class.
 * 3. Create TypeSupports.
 * 4. Check that the types are not registered yet.
 * 5. Check that the topics do not exist yet.
 * 6. Enable each statistics DataWriter checking that topics are created and types are registered.
 * 7. Enable an already enabled statistics DataWriter and check that it returns RETCODE_OK.
 * 8. Call enable_statistics_datawriter method with an invalid topic name and check that returns RETCODE_BAD_PARAMETER.
 * 9. Disable one statistics DataWriter and check that it is successful.
 * 10. Enable the previous statistics DataWriter with an inconsistent QoS and check that it returns
 * RETCODE_INCONSISTENT_POLICY.
 * 11. Check error code precedence: RETCODE_BAD_PARAMETER takes precedence over RETCODE_INCONSISTENT_POLICY.
 * The case where the create_datawriter fails returning RETCODE_ERROR is not checked because it only passes the error
 * upstream.
 * 12. Try to disable an already disabled statistics DataWriter and check that returns RETCODE_ERROR.
 * 13. Check that if an invalid topic name is provided to the disable_statistics_datawriter method, it returns
 * RETCODE_BAD_PARAMETER.
 * The case where the delete_datawriter fails returning RETCODE_ERROR is not checked because it only passes the error
 * upstream.
 * 14. Delete DDS entities.
 */
TEST_F(StatisticsDomainParticipantTests, EnableDisableStatisticsDataWriterTest)
{
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    eprosima::fastdds::dds::DataWriterQos inconsistent_qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    inconsistent_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    inconsistent_qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;

#ifndef FASTDDS_STATISTICS
    // 1. Compilation flag not set
    DomainParticipant* statistics_participant = static_cast<DomainParticipant*>(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, statistics_participant->enable_statistics_datawriter(
                HISTORY_LATENCY_TOPIC, STATISTICS_DATAWRITER_QOS));
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, statistics_participant->enable_statistics_datawriter("INVALID_TOPIC",
            inconsistent_qos));
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, statistics_participant->disable_statistics_datawriter(
                HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, statistics_participant->disable_statistics_datawriter(
                "INVALID_TOPIC"));
#else
    // 2. Narrow DomainParticipant to eprosima::fastdds::statistics::dds::DomainParticipant
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 3. Create TypeSupports for the different DataTypes
    eprosima::fastdds::dds::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
    eprosima::fastdds::dds::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
    eprosima::fastdds::dds::TypeSupport throughput_type(new EntityDataPubSubType);
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport discovery_type(new DiscoveryTimePubSubType);
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
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
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HISTORY_LATENCY_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_TRUE(history_latency_type == statistics_participant->find_type(history_latency_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(NETWORK_LATENCY_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_TRUE(network_latency_type == statistics_participant->find_type(network_latency_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(
                PUBLICATION_THROUGHPUT_TOPIC, STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_TRUE(throughput_type == statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(
                SUBSCRIPTION_THROUGHPUT_TOPIC, STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_TRUE(throughput_type == statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(RTPS_SENT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    EXPECT_TRUE(rtps_traffic_type == statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(RTPS_LOST_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_TRUE(rtps_traffic_type == statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(RESENT_DATAS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(ACKNACK_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(NACKFRAG_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(GAP_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(DATA_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PDP_PACKETS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(EDP_PACKETS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(DISCOVERY_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_TRUE(discovery_type == statistics_participant->find_type(discovery_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(SAMPLE_DATAS_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_TRUE(sample_identity_count_type == statistics_participant->find_type(
                sample_identity_count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PHYSICAL_DATA_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_TRUE(physical_data_type == statistics_participant->find_type(physical_data_type.get_type_name()));

    // 7. Enable an already enabled statistics DataWriter
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(SAMPLE_DATAS_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 8. Invalid topic name
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, statistics_participant->enable_statistics_datawriter("INVALID_TOPIC",
            STATISTICS_DATAWRITER_QOS));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription("INVALID_TOPIC"));

    // 9. Disable statistics DataWriter
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(history_latency_type.get_type_name()));

    // 10. Enable previous statistics DataWriter with an inconsistent QoS
    EXPECT_EQ(ReturnCode_t::RETCODE_INCONSISTENT_POLICY, statistics_participant->enable_statistics_datawriter(
                HISTORY_LATENCY_TOPIC, inconsistent_qos));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(history_latency_type.get_type_name()));

    // 11. RETCODE_BAD_PARAMETER error has precedence over RETCODE_INCONSISTENT_POLICY
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, statistics_participant->enable_statistics_datawriter("INVALID_TOPIC",
            inconsistent_qos));

    // 12. Disable already disabled DataWriter
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(HISTORY_LATENCY_TOPIC));

    // 13. Disable invalid topic name
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, statistics_participant->disable_statistics_datawriter(
                "INVALID_TOPIC"));

    // 14. Remove DDS entities
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(network_latency_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(
                PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PUBLICATION_THROUGHPUT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(throughput_type == statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(
                SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(throughput_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(RTPS_SENT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_SENT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(rtps_traffic_type == statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(RTPS_LOST_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RTPS_LOST_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(RESENT_DATAS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(RESENT_DATAS_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(ACKNACK_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(ACKNACK_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(NACKFRAG_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(NACKFRAG_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(GAP_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(GAP_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(DATA_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DATA_COUNT_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(PDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PDP_PACKETS_TOPIC));
    // The type is being used by another topic yet
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(EDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(EDP_PACKETS_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(DISCOVERY_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(DISCOVERY_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(discovery_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(SAMPLE_DATAS_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(sample_identity_count_type.get_type_name()));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(physical_data_type.get_type_name()));
#endif // FASTDDS_STATISTICS

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant));
}

/**
 * This test checks that when the topic name provided is not valid, a log error is printed.
 * 1. Create a participant with the property fastdds.statistics set to an invalid topic name
 * 2. Check that there is no topic/type registered in the participant
 * 3. Wait for the logError entry to be consumed
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
    eprosima::fastdds::dds::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
    eprosima::fastdds::dds::TypeSupport throughput_type(new EntityDataPubSubType);
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport discovery_type(new DiscoveryTimePubSubType);
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
    eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
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

    // 3. Wait until logError entries are captured
    helper_block_for_at_least_entries(2);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 2u);

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant),
            ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that enable_statistics_datawriter fails returning RETCODE_ERROR when there is already a TypeSupport
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
    invalid_type->setName(reserved_statistics_type_name);
    participant->register_type(invalid_type);
    participant->register_type(physical_data_type);

    // 2. Check call to enable_statistics_datawriter
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    eprosima::fastdds::dds::TypeSupport type = participant->find_type(count_type.get_type_name());
    EXPECT_FALSE(count_type == type);
    EXPECT_TRUE(invalid_type == type);

    // 3. Check topic creation
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    // 4. Call enable_statistics_datawriter with an already correctly registered type.
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PHYSICAL_DATA_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_TRUE(physical_data_type == participant->find_type(physical_data_type.get_type_name()));
    EXPECT_NE(nullptr, participant->lookup_topicdescription(PHYSICAL_DATA_TOPIC));

    // 5. Check log error entry
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    // delete_participant removes all builtin statistics entities
    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that enable_statistics_datawriter fails returning RETCODE_ERROR when there is already a statistics
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
                    count_type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    participant->create_topic(HEARTBEAT_COUNT_TOPIC, count_type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    participant->create_topic(PHYSICAL_DATA_TOPIC, physical_data_type->getName(),
            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    // 2. Check call to enable_statistics_datawriter
    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, statistics_participant->enable_statistics_datawriter(HISTORY_LATENCY_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 3. Check type registration
    EXPECT_EQ(null_type, participant->find_type(history_latency_type.get_type_name()));

    // 4. Call enable_statistics_datawriter with correctly created topic
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    // 5. Call enable_statistics_datawriter and check it is successful
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(PHYSICAL_DATA_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 6. Check log error entry
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    // delete_participant removes all builtin statistics entities but not others
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_topic(invalid_topic));
    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that the creation of new feature: Configuration of participant statistic QoS from XML file
 * Topic created with another type different from the one expected.
 * 1. Create XML file with the appropiate configuration.
 * 2. Read XML and enable datawrtiters from it.
 * 3. Check that Qos are correctly configured.
 */
TEST_F(StatisticsFromXMLProfileTests, XMLConfigurationForStatisticsDataWritersQoS)
{
#ifdef FASTDDS_STATISTICS
    const char* xml =
            "                                                                                                           \
        <?xml version=\"1.0\" encoding=\"utf-8\"  ?>                                                                    \
        <dds xmlns=\"http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles\">                                            \
        <profiles>                                                                                                      \
                <participant profile_name=\"statistics_participant\" is_default_profile=\"true\">                       \
                <rtps>                                                                                                  \
                        <propertiesPolicy>                                                                              \
                        <properties>                                                                                    \
                                <property>                                                                              \
                                <name>fastdds.statistics</name>                                                         \
                                <value>HISTORY_LATENCY_TOPIC;PUBLICATION_THROUGHPUT_TOPIC</value>                                                    \
                                </property>                                                                             \
                        </properties>                                                                                   \
                        </propertiesPolicy>                                                                             \
                </rtps>                                                                                                 \
                </participant>                                                                                          \
                <data_writer profile_name=\"HISTORY_LATENCY_TOPIC\">                                                    \
                        <qos>                                                                                           \
                                <reliability>                                                                           \
                                        <kind>BEST_EFFORT</kind>                                                        \
                                </reliability>                                                                          \
                                <durability>                                                                            \
                                        <kind>VOLATILE</kind>                                                           \
                                </durability>                                                                           \
                                <publishMode>                                                                           \
                                        <kind>SYNCHRONOUS</kind>                                                        \
                                </publishMode>                                                                          \
                        </qos>                                                                                          \
                </data_writer>                                                                                          \
                <data_writer profile_name=\"NETWORK_LATENCY_TOPIC\">                                                    \
                </data_writer>                                                                                          \
        </profiles>                                                                                                     \
        </dds>                                                                                                          \
    ";
    tinyxml2::XMLDocument xml_doc;
    xml_doc.Parse(xml);
    xml_doc.SaveFile("FASTRTPS_STATISTICS_PROFILES.xml");


    // 1. Set environment variable and create participant using Qos set by code
    const char* value = "FASTRTPS_STATISTICS_PROFILES.xml";
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("FASTRTPS_DEFAULT_PROFILES_FILE", value));
#else
    ASSERT_EQ(0, setenv("FASTRTPS_DEFAULT_PROFILES_FILE", value, 1));
#endif // ifdef _WIN32

    // Create a DomainParticipant (2. is implicit, as auto-enable option is true by default)
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Obtain pointer to child class
    eprosima::fastdds::statistics::dds::DomainParticipant* statistics_participant =
            eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // Static conversion to child class TestDomainParticipant
    TestDomainParticipant* test_statistics_participant = static_cast<TestDomainParticipant*>(statistics_participant);
    ASSERT_NE(test_statistics_participant, nullptr);

    // Get DomainParticipantImpl
    eprosima::fastdds::statistics::dds::DomainParticipantImpl* domain_statistics_participant_impl =
            test_statistics_participant->get_domain_participant_impl();
    ASSERT_NE(domain_statistics_participant_impl, nullptr);

    // Static conversion to child class TestDomainParticipantImpl
    TestDomainParticipantImpl* test_statistics_domain_participant_impl =
            static_cast<TestDomainParticipantImpl*>(domain_statistics_participant_impl);
    ASSERT_NE(test_statistics_domain_participant_impl, nullptr);

    // Get PublisherImpl
    eprosima::fastdds::statistics::dds::PublisherImpl* statistics_publisher_impl =
            test_statistics_domain_participant_impl->get_publisher_impl();
    ASSERT_NE(statistics_publisher_impl, nullptr);

    // 3. Get datawriters

    // HISTORY_LATENCY_TOPIC has non default qos defined in XML
    // Also is defined as data_writer profile, and in the fastdds.statistics property policy,
    // for which the non-default qos defined should prevail
    std::string history_latency_name = "_fastdds_statistics_history2history_latency";
    eprosima::fastdds::dds::DataWriter* history_latency_writer =
            statistics_publisher_impl->lookup_datawriter(history_latency_name);
    ASSERT_NE(history_latency_writer, nullptr);

    // By default, when QoS are setted in XML for an statistics DataWriter profile,
    // XMLProfileManager will use eProsima's default qos, not statistics default qos
    efd::DataWriterQos qos;
    qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind_t::VOLATILE_DURABILITY_QOS;
    qos.publish_mode().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
    ASSERT_EQ(qos, history_latency_writer->get_qos());

    // NETWORK_LATENCY_TOPIC is not defined in the fastdds.statistics property policy,
    // it is just defined as data_writer profile. Thus, should not be created
    std::string network_latency_name = "_fastdds_statistics_network_latency";
    eprosima::fastdds::dds::DataWriter* network_latency_writer =
            statistics_publisher_impl->lookup_datawriter(network_latency_name);
    ASSERT_EQ(network_latency_writer, nullptr);

    // PUBLICATION_THROUGHPUT_TOPIC should have by-default qos
    // Defined in the fastdds.statistics property policy
    std::string publication_throughput_name = "_fastdds_statistics_publication_throughput";
    eprosima::fastdds::dds::DataWriter* publication_throughput_writer =
            statistics_publisher_impl->lookup_datawriter(publication_throughput_name);
    ASSERT_NE(publication_throughput_writer, nullptr);
    ASSERT_EQ(STATISTICS_DATAWRITER_QOS, publication_throughput_writer->get_qos());

    // SUBSCRIPTION_THROUGHPUT_TOPIC is not defined. Should return nullptr
    std::string subscription_throughput_name = "_fastdds_statistics_subscription_throughput";
    eprosima::fastdds::dds::DataWriter* subscription_throughput_writer =
            statistics_publisher_impl->lookup_datawriter(subscription_throughput_name);
    ASSERT_EQ(subscription_throughput_writer, nullptr);

    // TODO: Check more datawriters

    remove("FASTRTPS_PROFILES.xml");


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
