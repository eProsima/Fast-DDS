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
#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>
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

namespace {

// Accessor to DomainParticipantImpl* from DomainParticipant
using DomainParticipantImplPtr =
        eprosima::fastdds::dds::DomainParticipantImpl * eprosima::fastdds::dds::DomainParticipant::*;

DomainParticipantImplPtr get_domain_participant_impl_ptr();

template<DomainParticipantImplPtr P>
struct DomainParticipantImplAccessor
{
    friend DomainParticipantImplPtr get_domain_participant_impl_ptr()
    {
        return P;
    }

};

template struct DomainParticipantImplAccessor<&eprosima::fastdds::dds::DomainParticipant::impl_>;

// Accessor to builtin_publisher_impl_ from statistics DomainParticipantImpl
// Accessor to protected get_monitoring_status() member function
using GetMonitoringStatusFn = bool (DomainParticipantImpl::*)(
    const fastdds::rtps::GUID_t&,
    statistics::MonitorServiceData&);

GetMonitoringStatusFn get_monitoring_status_fn();

template<GetMonitoringStatusFn F>
struct GetMonitoringStatusAccessor
{
    friend GetMonitoringStatusFn get_monitoring_status_fn()
    {
        return F;
    }

};

template struct GetMonitoringStatusAccessor<&DomainParticipantImpl::get_monitoring_status>;

// Accessor to writers_ from PublisherImpl, so that the checks below can reach the DataWriters
using PublisherWritersPtr =
        std::map<std::string, std::vector<efd::DataWriterImpl*>> efd::PublisherImpl::*;

PublisherWritersPtr get_publisher_writers_ptr();

template<PublisherWritersPtr P>
struct PublisherWritersAccessor
{
    friend PublisherWritersPtr get_publisher_writers_ptr()
    {
        return P;
    }

};

template struct PublisherWritersAccessor<&efd::PublisherImpl::writers_>;

// Accessor to impl_ from Publisher
using PublisherImplPtr = efd::PublisherImpl * efd::Publisher::*;

PublisherImplPtr get_publisher_impl_ptr();

template<PublisherImplPtr P>
struct PublisherImplAccessor
{
    friend PublisherImplPtr get_publisher_impl_ptr()
    {
        return P;
    }

};

template struct PublisherImplAccessor<&efd::Publisher::impl_>;

// Accessors to the status members of DataWriterImpl
using DeadlineMissedStatusPtr = efd::OfferedDeadlineMissedStatus efd::DataWriterImpl::*;

DeadlineMissedStatusPtr get_deadline_missed_status_ptr();

template<DeadlineMissedStatusPtr P>
struct DeadlineMissedStatusAccessor
{
    friend DeadlineMissedStatusPtr get_deadline_missed_status_ptr()
    {
        return P;
    }

};

template struct DeadlineMissedStatusAccessor<&efd::DataWriterImpl::deadline_missed_status_>;

using LivelinessLostStatusPtr = efd::LivelinessLostStatus efd::DataWriterImpl::*;

LivelinessLostStatusPtr get_liveliness_lost_status_ptr();

template<LivelinessLostStatusPtr P>
struct LivelinessLostStatusAccessor
{
    friend LivelinessLostStatusPtr get_liveliness_lost_status_ptr()
    {
        return P;
    }

};

template struct LivelinessLostStatusAccessor<&efd::DataWriterImpl::liveliness_lost_status_>;

// Accessor to protected update_offered_incompatible_qos() member function of DataWriterImpl
using UpdateOfferedIncompatibleQosFn = efd::OfferedIncompatibleQosStatus& (efd::DataWriterImpl::*)(
    efd::PolicyMask);

UpdateOfferedIncompatibleQosFn update_offered_incompatible_qos_fn();

template<UpdateOfferedIncompatibleQosFn F>
struct UpdateOfferedIncompatibleQosAccessor
{
    friend UpdateOfferedIncompatibleQosFn update_offered_incompatible_qos_fn()
    {
        return F;
    }

};

template struct UpdateOfferedIncompatibleQosAccessor<&efd::DataWriterImpl::update_offered_incompatible_qos>;

//! Fakes a QoS policy violation on the DataWriter with the given GUID, updating the status it is reported in
void insert_policy_violation(
        efd::Publisher* publisher,
        const fastdds::rtps::GUID_t& guid,
        const efd::QosPolicyId_t& policy_id)
{
    efd::PublisherImpl* pub_impl = publisher->*get_publisher_impl_ptr();

    for (auto& writer_pair : pub_impl->*get_publisher_writers_ptr())
    {
        for (efd::DataWriterImpl* writer : writer_pair.second)
        {
            if (writer->guid() == guid)
            {
                if (efd::LIVELINESS_QOS_POLICY_ID == policy_id)
                {
                    (writer->*get_liveliness_lost_status_ptr()).total_count++;
                }
                else if (efd::DEADLINE_QOS_POLICY_ID == policy_id)
                {
                    efd::OfferedDeadlineMissedStatus& status = writer->*get_deadline_missed_status_ptr();
                    status.total_count++;
                    status.total_count_change++;
                }
                else
                {
                    efd::PolicyMask policy_mask;
                    policy_mask.set(policy_id);
                    (writer->*update_offered_incompatible_qos_fn())(policy_mask);
                }
            }
        }
    }
}

} // namespace

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

    DomainParticipantImpl* statistics_participant_impl_test = static_cast<DomainParticipantImpl*>(
        participant->*get_domain_participant_impl_ptr());
    ASSERT_NE(nullptr, statistics_participant_impl_test);

    fastdds::dds::TypeSupport type(new TopicDataTypeMock());
    type.register_type(statistics_participant);

    auto publisher = participant->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
    auto topic = participant->create_topic(TEST_TOPIC, "footype", fastdds::dds::TOPIC_QOS_DEFAULT);

    //! Create DataWriters
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, dw1);
    ASSERT_NE(nullptr, dw2);

    //! Insert some QoS incompatibilities
    insert_policy_violation(publisher, dw1->guid(), fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    insert_policy_violation(publisher, dw2->guid(), fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    MonitorServiceData incomp_qos_status_dw_1;
    incomp_qos_status_dw_1.incompatible_qos_status(IncompatibleQoSStatus_s{});
    MonitorServiceData incomp_qos_status_dw_2;
    incomp_qos_status_dw_2.incompatible_qos_status(IncompatibleQoSStatus_s{});
    ASSERT_TRUE((statistics_participant_impl_test->*get_monitoring_status_fn())(dw1->guid(), incomp_qos_status_dw_1));
    ASSERT_TRUE((statistics_participant_impl_test->*get_monitoring_status_fn())(dw2->guid(), incomp_qos_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1u, incomp_qos_status_dw_1.incompatible_qos_status().total_count());
    ASSERT_EQ(1u, incomp_qos_status_dw_2.incompatible_qos_status().total_count());
    ASSERT_EQ(1u,
            incomp_qos_status_dw_1.incompatible_qos_status().policies()[fastdds::dds::RELIABILITY_QOS_POLICY_ID].count());
    ASSERT_EQ(1u,
            incomp_qos_status_dw_2.incompatible_qos_status().policies()[fastdds::dds::RELIABILITY_QOS_POLICY_ID].count());

    ASSERT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->delete_contained_entities());
    ASSERT_EQ(fastdds::dds::RETCODE_OK,
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant));

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

    DomainParticipantImpl* statistics_participant_impl_test = static_cast<DomainParticipantImpl*>(
        participant->*get_domain_participant_impl_ptr());
    ASSERT_NE(nullptr, statistics_participant_impl_test);

    fastdds::dds::TypeSupport type(new TopicDataTypeMock());
    type.register_type(statistics_participant);

    auto publisher = participant->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
    auto topic = participant->create_topic(TEST_TOPIC, "footype", fastdds::dds::TOPIC_QOS_DEFAULT);

    //! Create DataWriters
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, dw1);
    ASSERT_NE(nullptr, dw2);

    //! Insert some QoS incompatibilities
    insert_policy_violation(publisher, dw1->guid(), fastdds::dds::LIVELINESS_QOS_POLICY_ID);
    insert_policy_violation(publisher, dw2->guid(), fastdds::dds::LIVELINESS_QOS_POLICY_ID);

    MonitorServiceData liv_lost_status_dw_1;
    liv_lost_status_dw_1.liveliness_lost_status(LivelinessLostStatus_s{});
    MonitorServiceData liv_lost_status_dw_2;
    liv_lost_status_dw_2.liveliness_lost_status(LivelinessLostStatus_s{});
    ASSERT_TRUE((statistics_participant_impl_test->*get_monitoring_status_fn())(dw1->guid(), liv_lost_status_dw_1));
    ASSERT_TRUE((statistics_participant_impl_test->*get_monitoring_status_fn())(dw2->guid(), liv_lost_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1u, liv_lost_status_dw_1.liveliness_lost_status().total_count());
    ASSERT_EQ(1u, liv_lost_status_dw_2.liveliness_lost_status().total_count());

    ASSERT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->delete_contained_entities());
    ASSERT_EQ(fastdds::dds::RETCODE_OK,
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant));

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

    DomainParticipantImpl* statistics_participant_impl_test = static_cast<DomainParticipantImpl*>(
        participant->*get_domain_participant_impl_ptr());
    ASSERT_NE(nullptr, statistics_participant_impl_test);

    fastdds::dds::TypeSupport type(new TopicDataTypeMock());
    type.register_type(statistics_participant);

    auto publisher = participant->create_publisher(fastdds::dds::PUBLISHER_QOS_DEFAULT);
    auto topic = participant->create_topic(TEST_TOPIC, "footype", fastdds::dds::TOPIC_QOS_DEFAULT);

    //! Create DataWriters
    auto dw1 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    auto dw2 = publisher->create_datawriter(topic, fastdds::dds::DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, dw1);
    ASSERT_NE(nullptr, dw2);

    //! Insert some QoS incompatibilities
    insert_policy_violation(publisher, dw1->guid(), fastdds::dds::DEADLINE_QOS_POLICY_ID);
    insert_policy_violation(publisher, dw2->guid(), fastdds::dds::DEADLINE_QOS_POLICY_ID);

    MonitorServiceData deadline_missed_status_dw_1;
    deadline_missed_status_dw_1.deadline_missed_status(DeadlineMissedStatus_s{});
    MonitorServiceData deadline_missed_status_dw_2;
    deadline_missed_status_dw_2.deadline_missed_status(DeadlineMissedStatus_s{});
    ASSERT_TRUE((statistics_participant_impl_test->*get_monitoring_status_fn())(dw1->guid(),
            deadline_missed_status_dw_1));
    ASSERT_TRUE((statistics_participant_impl_test->*get_monitoring_status_fn())(dw2->guid(),
            deadline_missed_status_dw_2));

    //! Expect incompatibilities
    ASSERT_EQ(1u, deadline_missed_status_dw_1.deadline_missed_status().total_count());
    ASSERT_EQ(1u, deadline_missed_status_dw_2.deadline_missed_status().total_count());

    ASSERT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->delete_contained_entities());
    ASSERT_EQ(fastdds::dds::RETCODE_OK,
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant));

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
