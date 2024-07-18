// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/fastdds/publisher/PublisherImpl.hpp>
#include <statistics/types/typesPubSubTypes.hpp>

#include "../../logging/mock/MockConsumer.h"

constexpr const char* TEST_TOPIC = "test_topic";

class TopicDataTypeMock : public eprosima::fastdds::dds::TopicDataType
{
public:

    TopicDataTypeMock()
        : TopicDataType()
    {
        max_serialized_type_size = 4u;
        set_name("footype");
    }

    bool serialize(
            const void* const /*data*/,
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            eprosima::fastdds::dds::DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            eprosima::fastdds::dds::DataRepresentationId_t /*data_representation*/) override
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
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    void clearName()
    {
        set_name("");
    }

private:

    using eprosima::fastdds::dds::TopicDataType::calculate_serialized_size;
    using eprosima::fastdds::dds::TopicDataType::serialize;
};

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class StatisticsDomainParticipantStatusQueryableTests : public ::testing::Test
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

    bool monitoring_status(
            fastdds::rtps::GUID_t guid,
            statistics::MonitorServiceData& status)
    {
        return get_monitoring_status(guid, status);
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

TEST_F(StatisticsDomainParticipantStatusQueryableTests, istatus_queryable_get_incompatible_qos)
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
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillRepeatedly(testing::Return(false));
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);

    //! Insert some QoS incompatibilities
    auto pub_impl = publisher->get_impl();
    auto statistics_pub_impl = static_cast<fastdds::statistics::dds::PublisherImpl*>(pub_impl);

    statistics_pub_impl->insert_policy_violation(dw1->guid(), fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    statistics_pub_impl->insert_policy_violation(dw2->guid(), fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    MonitorServiceData incomp_qos_status_dw_1;
    incomp_qos_status_dw_1.incompatible_qos_status(IncompatibleQoSStatus_s{});
    MonitorServiceData incomp_qos_status_dw_2;
    incomp_qos_status_dw_2.incompatible_qos_status(IncompatibleQoSStatus_s{});
    ASSERT_TRUE(statistics_participant_impl_test->monitoring_status(dw1->guid(), incomp_qos_status_dw_1));
    ASSERT_TRUE(statistics_participant_impl_test->monitoring_status(dw2->guid(), incomp_qos_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1u, incomp_qos_status_dw_1.incompatible_qos_status().total_count());
    ASSERT_EQ(1u, incomp_qos_status_dw_2.incompatible_qos_status().total_count());
    ASSERT_EQ(1u,
            incomp_qos_status_dw_1.incompatible_qos_status().policies()[fastdds::dds::RELIABILITY_QOS_POLICY_ID].count());
    ASSERT_EQ(1u,
            incomp_qos_status_dw_2.incompatible_qos_status().policies()[fastdds::dds::RELIABILITY_QOS_POLICY_ID].count());

    statistics_pub_impl->delete_datawriters();
    topic->get_impl()->dereference();
    topic->get_impl()->dereference();
    statistics_participant->delete_topic(topic);
    statistics_participant->delete_publisher(publisher);
    statistics_participant->delete_contained_entities();

#endif // FASTDDS_STATISTICS
}

TEST_F(StatisticsDomainParticipantStatusQueryableTests, istatus_queryable_get_liveliness_lost_status)
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
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillRepeatedly(testing::Return(false));
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);

    //! Insert some QoS incompatibilities
    auto pub_impl = publisher->get_impl();
    auto statistics_pub_impl = static_cast<fastdds::statistics::dds::PublisherImpl*>(pub_impl);

    statistics_pub_impl->insert_policy_violation(dw1->guid(), fastdds::dds::LIVELINESS_QOS_POLICY_ID);
    statistics_pub_impl->insert_policy_violation(dw2->guid(), fastdds::dds::LIVELINESS_QOS_POLICY_ID);

    MonitorServiceData liv_lost_status_dw_1;
    liv_lost_status_dw_1.liveliness_lost_status(LivelinessLostStatus_s{});
    MonitorServiceData liv_lost_status_dw_2;
    liv_lost_status_dw_2.liveliness_lost_status(LivelinessLostStatus_s{});
    ASSERT_TRUE(statistics_participant_impl_test->monitoring_status(dw1->guid(), liv_lost_status_dw_1));
    ASSERT_TRUE(statistics_participant_impl_test->monitoring_status(dw2->guid(), liv_lost_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1u, liv_lost_status_dw_1.liveliness_lost_status().total_count());
    ASSERT_EQ(1u, liv_lost_status_dw_2.liveliness_lost_status().total_count());

    statistics_pub_impl->delete_datawriters();
    topic->get_impl()->dereference();
    topic->get_impl()->dereference();
    statistics_participant->delete_topic(topic);
    statistics_participant->delete_publisher(publisher);
    statistics_participant->delete_contained_entities();

#endif // FASTDDS_STATISTICS
}

TEST_F(StatisticsDomainParticipantStatusQueryableTests, istatus_queryable_get_deadline_missed_status)
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
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillRepeatedly(testing::Return(false));
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);

    //! Insert some QoS incompatibilities
    auto pub_impl = publisher->get_impl();
    auto statistics_pub_impl = static_cast<fastdds::statistics::dds::PublisherImpl*>(pub_impl);

    statistics_pub_impl->insert_policy_violation(dw1->guid(), fastdds::dds::DEADLINE_QOS_POLICY_ID);
    statistics_pub_impl->insert_policy_violation(dw2->guid(), fastdds::dds::DEADLINE_QOS_POLICY_ID);

    MonitorServiceData deadline_missed_status_dw_1;
    deadline_missed_status_dw_1.deadline_missed_status(DeadlineMissedStatus_s{});
    MonitorServiceData deadline_missed_status_dw_2;
    deadline_missed_status_dw_2.deadline_missed_status(DeadlineMissedStatus_s{});
    ASSERT_TRUE(statistics_participant_impl_test->monitoring_status(dw1->guid(), deadline_missed_status_dw_1));
    ASSERT_TRUE(statistics_participant_impl_test->monitoring_status(dw2->guid(), deadline_missed_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1u, deadline_missed_status_dw_1.deadline_missed_status().total_count());
    ASSERT_EQ(1u, deadline_missed_status_dw_2.deadline_missed_status().total_count());

    statistics_pub_impl->delete_datawriters();
    topic->get_impl()->dereference();
    topic->get_impl()->dereference();
    statistics_participant->delete_topic(topic);
    statistics_participant->delete_publisher(publisher);
    statistics_participant->delete_contained_entities();

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
