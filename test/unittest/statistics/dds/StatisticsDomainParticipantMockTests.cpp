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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/types/TypesBase.h>
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/fastdds/publisher/PublisherImpl.hpp>
#include <statistics/types/typesPubSubTypes.h>

#include "../../logging/mock/MockConsumer.h"

using eprosima::fastrtps::types::ReturnCode_t;

constexpr const char* TEST_TOPIC = "test_topic";

class TopicDataTypeMock : public eprosima::fastdds::dds::TopicDataType
{
public:

    TopicDataTypeMock()
        : TopicDataType()
    {
        m_typeSize = 4u;
        setName("footype");
    }

    bool serialize(
            void* /*data*/,
            eprosima::fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return []()->uint32_t
               {
                   return 0;
               };
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
            eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    void clearName()
    {
        setName("");
    }

};

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class StatisticsDomainParticipantMockTests : public ::testing::Test
{
public:

    void helper_block_for_at_least_entries(
            uint32_t amount)
    {
        mock_consumer_->wait_for_at_least_entries(amount);
    }

    eprosima::fastdds::dds::MockConsumer* mock_consumer_;

};

class DomainParticipantImplTest : public DomainParticipantImpl
{
public:

    eprosima::fastdds::dds::Publisher* get_builtin_publisher() const
    {
        return builtin_publisher_;
    }

    PublisherImpl* get_builtin_publisher_impl() const
    {
        return builtin_publisher_impl_;
    }

    bool incompatible_qos_status(fastrtps::rtps::GUID_t guid, fastdds::dds::IncompatibleQosStatus status)
    {
        return get_incompatible_qos_status(guid, status);
    }

    bool inconsistent_topic_status(fastrtps::rtps::GUID_t guid, fastdds::dds::InconsistentTopicStatus status)
    {
        return get_inconsistent_topic_status(guid, status);
    }

    bool liveliness_lost_status(fastrtps::rtps::GUID_t guid, fastdds::dds::LivelinessLostStatus status)
    {
        return get_liveliness_lost_status(guid, status);
    }

    bool liveliness_changed_status(fastrtps::rtps::GUID_t guid, fastdds::dds::LivelinessChangedStatus status)
    {
        return get_liveliness_changed_status(guid, status);
    }

    bool deadline_missed_status(fastrtps::rtps::GUID_t guid, fastdds::dds::DeadlineMissedStatus status)
    {
        return get_deadline_missed_status(guid, status);
    }

    bool sample_lost_status(fastrtps::rtps::GUID_t guid, fastdds::dds::SampleLostStatus status)
    {
        return get_sample_lost_status(guid, status);
    }

};

class DomainParticipantTest : public eprosima::fastdds::dds::DomainParticipant
{
public:

    eprosima::fastdds::dds::DomainParticipantImpl* get_impl() const
    {
        return impl_;
    }

};

/**
 * This test checks that enable_statistics_datawriter fails returning RETCODE_ERROR when create_datawriter fails
 * returning a nullptr.
 * 1. Create participant
 * 2. Mock create_datawriter so it returns nullptr
 * 3. Call enable_statistics_datawriter and check it fails.
 * 4. Capture log error entry
 */
TEST_F(StatisticsDomainParticipantMockTests, EnableStatisticsDataWriterFailureCreateDataWriter)
{
#ifdef FASTDDS_STATISTICS
    mock_consumer_ = new eprosima::fastdds::dds::MockConsumer();

    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(mock_consumer_));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("(STATISTICS_DOMAIN_PARTICIPANT)"));
    eprosima::fastdds::dds::Log::SetErrorStringFilter(std::regex("(DataWriter creation has failed)"));

    eprosima::fastdds::dds::TypeSupport null_type(nullptr);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 2. Mock create_datawriter
    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());
    ASSERT_NE(nullptr, statistics_participant_impl_test);
    PublisherImpl* builtin_pub_impl = statistics_participant_impl_test->get_builtin_publisher_impl();
    EXPECT_CALL(*builtin_pub_impl, create_datawriter_mock()).WillOnce(testing::Return(true));

    // 3. enable_statistics_datawriter
    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));

    // 4. Check log error entry
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(participant), ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that disable_statistics_datawriter fails returning RETCODE_ERROR when delete_datawriter fails.
 * 1. Create a participant
 * 2. Mock delete_datawriter
 * 3. Enable a statistics datawriter
 * 4. Call disable_statistics_datawriter and check return code
 */
TEST_F(StatisticsDomainParticipantMockTests, DisableStatisticsDataWriterFailureDeleteDataWriter)
{
#ifdef FASTDDS_STATISTICS
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 2. Mock delete_datawriter
    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());
    ASSERT_NE(nullptr, statistics_participant_impl_test);
    eprosima::fastdds::dds::Publisher* builtin_pub = statistics_participant_impl_test->get_builtin_publisher();
    ASSERT_NE(nullptr, builtin_pub);
    PublisherImpl* builtin_pub_impl = statistics_participant_impl_test->get_builtin_publisher_impl();
    ASSERT_NE(nullptr, builtin_pub_impl);
    EXPECT_CALL(*builtin_pub, delete_datawriter_mock()).WillOnce(testing::Return(true));
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(false));

    // 3. enable_statistics_datawriter
    EXPECT_CALL(*builtin_pub_impl, create_datawriter_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 4. disable_statistics_datawriter
    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, statistics_participant->disable_statistics_datawriter(
                HEARTBEAT_COUNT_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_CALL(*builtin_pub, delete_datawriter_mock()).WillOnce(testing::Return(false));
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->disable_statistics_datawriter(
                HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that disable_statistics_datawriter fails returning RETCODE_ERROR when delete_topic fails.
 * 1. Create a participant
 * 2. Mock delete_topic
 * 3. Enable a statistics datawriter
 * 4. Call disable_statistics_datawriter and check return code
 */
TEST_F(StatisticsDomainParticipantMockTests, DisableStatisticsDataWriterFailureDeleteTopic)
{
#ifdef FASTDDS_STATISTICS
    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 2. Mock delete_topic
    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());
    ASSERT_NE(nullptr, statistics_participant_impl_test);
    eprosima::fastdds::dds::Publisher* builtin_pub = statistics_participant_impl_test->get_builtin_publisher();
    ASSERT_NE(nullptr, builtin_pub);
    PublisherImpl* builtin_pub_impl = statistics_participant_impl_test->get_builtin_publisher_impl();
    ASSERT_NE(nullptr, builtin_pub_impl);
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(true));
    EXPECT_CALL(*builtin_pub, delete_datawriter_mock()).WillOnce(testing::Return(false));

    // 3. enable_statistics_datawriter
    EXPECT_CALL(*builtin_pub_impl, create_datawriter_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    // 4. disable_statistics_datawriter
    EXPECT_EQ(ReturnCode_t::RETCODE_ERROR, statistics_participant->disable_statistics_datawriter(
                HEARTBEAT_COUNT_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    // As the DataWriter has been deleted, the topic has to be removed manually
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, statistics_participant->delete_topic(
                dynamic_cast<eprosima::fastdds::dds::Topic*>(statistics_participant->lookup_topicdescription(
                    HEARTBEAT_COUNT_TOPIC))));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), ReturnCode_t::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

TEST_F(StatisticsDomainParticipantMockTests, istatus_queryable_get_incompatible_qos)
{
#ifdef FASTDDS_STATISTICS
    // Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());

    ASSERT_NE(nullptr, statistics_participant_impl_test);

    fastdds::dds::TypeSupport type(new TopicDataTypeMock());
    type.register_type(statistics_participant);

    auto publisher = participant_test->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
    auto topic = participant_test->create_topic(TEST_TOPIC, "footype", fastdds::dds::TOPIC_QOS_DEFAULT);

    //! Create DataWriters
    EXPECT_CALL(*publisher, create_datawriter_mock()).WillRepeatedly(testing::Return(false));
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);

    //! Insert some QoS incompatibilities
    auto pub_impl = publisher->get_impl();
    auto statistics_pub_impl = static_cast<fastdds::statistics::dds::PublisherImpl*>(pub_impl);

    statistics_pub_impl->insert_policy_violation(dw1->guid(), fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    statistics_pub_impl->insert_policy_violation(dw2->guid(), fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    fastdds::dds::IncompatibleQosStatus incomp_qos_status_dw_1, incomp_qos_status_dw_2;
    ASSERT_TRUE(statistics_participant_impl_test->incompatible_qos_status(dw1->guid(), incomp_qos_status_dw_1));
    ASSERT_TRUE(statistics_participant_impl_test->incompatible_qos_status(dw2->guid(), incomp_qos_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1, incomp_qos_status_dw_1.total_count);
    ASSERT_EQ(1, incomp_qos_status_dw_2.total_count);
    ASSERT_EQ(1, incomp_qos_status_dw_1.policies[fastdds::dds::RELIABILITY_QOS_POLICY_ID].count);
    ASSERT_EQ(1, incomp_qos_status_dw_2.policies[fastdds::dds::RELIABILITY_QOS_POLICY_ID].count);

#endif // FASTDDS_STATISTICS
}

TEST_F(StatisticsDomainParticipantMockTests, istatus_queryable_get_liveliness_lost_status)
{
#ifdef FASTDDS_STATISTICS
    // Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());

    ASSERT_NE(nullptr, statistics_participant_impl_test);

    fastdds::dds::TypeSupport type(new TopicDataTypeMock());
    type.register_type(statistics_participant);

    auto publisher = participant_test->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
    auto topic = participant_test->create_topic(TEST_TOPIC, "footype", fastdds::dds::TOPIC_QOS_DEFAULT);

    //! Create DataWriters
    EXPECT_CALL(*publisher, create_datawriter_mock()).WillRepeatedly(testing::Return(false));
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);

    //! Insert some QoS incompatibilities
    auto pub_impl = publisher->get_impl();
    auto statistics_pub_impl = static_cast<fastdds::statistics::dds::PublisherImpl*>(pub_impl);

    statistics_pub_impl->insert_policy_violation(dw1->guid(), fastdds::dds::LIVELINESS_QOS_POLICY_ID);
    statistics_pub_impl->insert_policy_violation(dw2->guid(), fastdds::dds::LIVELINESS_QOS_POLICY_ID);

    fastdds::dds::LivelinessLostStatus liv_lost_status_dw_1, liv_lost_status_dw_2;
    ASSERT_TRUE(statistics_participant_impl_test->liveliness_lost_status(dw1->guid(), liv_lost_status_dw_1));
    ASSERT_TRUE(statistics_participant_impl_test->liveliness_lost_status(dw2->guid(), liv_lost_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1, liv_lost_status_dw_1.total_count);
    ASSERT_EQ(1, liv_lost_status_dw_2.total_count);

#endif // FASTDDS_STATISTICS
}

TEST_F(StatisticsDomainParticipantMockTests, istatus_queryable_get_deadline_missed_status)
{
#ifdef FASTDDS_STATISTICS
    // Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());

    ASSERT_NE(nullptr, statistics_participant_impl_test);

    fastdds::dds::TypeSupport type(new TopicDataTypeMock());
    type.register_type(statistics_participant);

    auto publisher = participant_test->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
    auto topic = participant_test->create_topic(TEST_TOPIC, "footype", fastdds::dds::TOPIC_QOS_DEFAULT);

    //! Create DataWriters
    EXPECT_CALL(*publisher, create_datawriter_mock()).WillRepeatedly(testing::Return(false));
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);

    //! Insert some QoS incompatibilities
    auto pub_impl = publisher->get_impl();
    auto statistics_pub_impl = static_cast<fastdds::statistics::dds::PublisherImpl*>(pub_impl);

    statistics_pub_impl->insert_policy_violation(dw1->guid(), fastdds::dds::DEADLINE_QOS_POLICY_ID);
    statistics_pub_impl->insert_policy_violation(dw2->guid(), fastdds::dds::DEADLINE_QOS_POLICY_ID);

    fastdds::dds::DeadlineMissedStatus deadline_missed_status_dw_1, deadline_missed_status_dw_2;
    ASSERT_TRUE(statistics_participant_impl_test->deadline_missed_status(dw1->guid(), deadline_missed_status_dw_1));
    ASSERT_TRUE(statistics_participant_impl_test->deadline_missed_status(dw2->guid(), deadline_missed_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1, deadline_missed_status_dw_1.total_count);
    ASSERT_EQ(1, deadline_missed_status_dw_2.total_count);

#endif // FASTDDS_STATISTICS
}

} // dds
} // statistics
} // fastdds
} // eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
