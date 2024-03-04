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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <dds/core/types.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/topic/Topic.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>

#include <fastdds/rtps/attributes/PropertyPolicy.h>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::PublisherAttributes;
using fastrtps::SubscriberAttributes;
using fastrtps::rtps::PropertyPolicyHelper;
using fastrtps::xmlparser::XMLProfileManager;
using fastrtps::xmlparser::XMLP_ret;


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

class BarType
{
public:

    inline uint32_t index() const
    {
        return index_;
    }

    inline uint32_t& index()
    {
        return index_;
    }

    inline void index(
            uint32_t value)
    {
        index_ = value;
    }

    inline const std::array<char, 256>& message() const
    {
        return message_;
    }

    inline std::array<char, 256>& message()
    {
        return message_;
    }

    inline void message(
            const std::array<char, 256>& value)
    {
        message_ = value;
    }

    inline void serialize(
            eprosima::fastcdr::Cdr& scdr) const
    {
        scdr << index_;
        scdr << message_;
    }

    inline void deserialize(
            eprosima::fastcdr::Cdr& dcdr)
    {
        dcdr >> index_;
        dcdr >> message_;
    }

    inline bool isKeyDefined()
    {
        return true;
    }

    inline void serializeKey(
            eprosima::fastcdr::Cdr& scdr) const
    {
        scdr << index_;
    }

    inline bool operator ==(
            const BarType& other) const
    {
        return (index_ == other.index_) && (message_ == other.message_);
    }

private:

    uint32_t index_ = 0;
    std::array<char, 256> message_;
};

class TopicDataTypeMock : public TopicDataType
{
public:

    TopicDataTypeMock()
        : TopicDataType()
    {
        m_typeSize = 4u;
        setName("footype");
    }

    bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
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
            void* data) override
    {
        return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/,
            DataRepresentationId_t /*data_representation*/) override
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
            fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

};

TEST(SubscriberTests, ChangeSubscriberQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    SubscriberQos qos;
    ASSERT_EQ(subscriber->get_qos(qos), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_EQ(subscriber->set_qos(qos), ReturnCode_t::RETCODE_OK);
    SubscriberQos pqos;
    ASSERT_EQ(subscriber->get_qos(pqos), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

TEST(SubscriberTests, ChangePSMSubscriberQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::Subscriber subscriber = ::dds::sub::Subscriber(participant);

    ::dds::sub::qos::SubscriberQos qos = subscriber.qos();
    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_NO_THROW(subscriber.qos(qos));
    ::dds::sub::qos::SubscriberQos pqos = subscriber.qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
}

TEST(SubscriberTests, ChangeDefaultDataReaderQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    DataReaderQos qos;
    subscriber->get_default_datareader_qos(qos);
    ASSERT_EQ(qos, DATAREADER_QOS_DEFAULT);

    // .durability
    qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS;
    // .deadline
    qos.deadline().period.seconds = 10;
    qos.deadline().period.nanosec = 20u;
    // .latency_budget
    qos.latency_budget().duration.seconds = 20;
    qos.latency_budget().duration.nanosec = 30u;
    // .liveliness
    qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    qos.liveliness().lease_duration.seconds = 40;
    qos.liveliness().lease_duration.nanosec = 61u;
    qos.liveliness().announcement_period.seconds = 30;
    qos.liveliness().announcement_period.nanosec = 50u;
    // .reliability
    qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    // .destination_order
    qos.destination_order().kind = eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    // . history
    qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    qos.history().depth = 1000;
    // .resource_limits
    qos.resource_limits().max_samples = 3000;
    qos.resource_limits().max_instances = 100;
    qos.resource_limits().max_samples_per_instance = 500;
    qos.resource_limits().allocated_samples = 50;
    qos.resource_limits().extra_samples = 2;
    // .user_data
    qos.user_data().push_back(0);
    qos.user_data().push_back(1);
    qos.user_data().push_back(2);
    qos.user_data().push_back(3);
    // .ownership
    qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    // .time_based_filter
    qos.time_based_filter().minimum_separation.seconds = eprosima::fastrtps::Time_t::INFINITE_SECONDS;
    qos.time_based_filter().minimum_separation.nanosec = eprosima::fastrtps::Time_t::INFINITE_NANOSECONDS;
    // .reader_data_lifecycle
    qos.reader_data_lifecycle().autopurge_disposed_samples_delay.seconds = 100;
    qos.reader_data_lifecycle().autopurge_disposed_samples_delay.nanosec = 30000u;
    qos.reader_data_lifecycle().autopurge_no_writer_samples_delay.seconds = 30000;
    qos.reader_data_lifecycle().autopurge_no_writer_samples_delay.nanosec = 100u;
    // .lifespan
    qos.lifespan().duration.seconds = 10;
    qos.lifespan().duration.nanosec = 33u;
    // .durability_service
    qos.durability_service().history_kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    qos.durability_service().history_depth = 10;
    qos.durability_service().max_samples = 5;
    qos.durability_service().max_instances = 20;
    qos.durability_service().max_samples_per_instance = 30;
    // .reliable_reader_qos
    qos.reliable_reader_qos().times.initialAcknackDelay.seconds = 34;
    qos.reliable_reader_qos().times.initialAcknackDelay.nanosec = 32u;
    qos.reliable_reader_qos().times.heartbeatResponseDelay.seconds = 432;
    qos.reliable_reader_qos().times.heartbeatResponseDelay.nanosec = 43u;
    qos.reliable_reader_qos().disable_positive_ACKs.enabled = true;
    qos.reliable_reader_qos().disable_positive_ACKs.duration.seconds = 13;
    qos.reliable_reader_qos().disable_positive_ACKs.duration.nanosec = 320u;
    // .type_consistency
    qos.type_consistency().representation.m_value.push_back(XML_DATA_REPRESENTATION);
    qos.type_consistency().representation.m_value.push_back(XCDR_DATA_REPRESENTATION);
    qos.type_consistency().type_consistency.m_ignore_sequence_bounds = false;
    qos.type_consistency().type_consistency.m_ignore_string_bounds = false;
    qos.type_consistency().type_consistency.m_ignore_member_names = true;
    qos.type_consistency().type_consistency.m_prevent_type_widening = true;
    qos.type_consistency().type_consistency.m_force_type_validation = true;
    // .expects_inline_qos
    qos.expects_inline_qos(true);
    // .properties
    eprosima::fastrtps::rtps::Property property;
    property.name("Property1");
    property.value("Value1");
    qos.properties().properties().push_back(property);
    property.name("Property2");
    property.value("Value2");
    qos.properties().properties().push_back(property);

    // .endpoint
    qos.endpoint().user_defined_id = 1;
    qos.endpoint().entity_id = 2;
    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    // .reader_resource_limits
    qos.reader_resource_limits().matched_publisher_allocation.initial = 30;
    qos.reader_resource_limits().matched_publisher_allocation.maximum = 300;
    qos.reader_resource_limits().matched_publisher_allocation.increment = 4;
    qos.reader_resource_limits().sample_infos_allocation.initial = 40;
    qos.reader_resource_limits().sample_infos_allocation.maximum = 400;
    qos.reader_resource_limits().sample_infos_allocation.increment = 5;
    qos.reader_resource_limits().outstanding_reads_allocation.initial = 50;
    qos.reader_resource_limits().outstanding_reads_allocation.maximum = 500;
    qos.reader_resource_limits().outstanding_reads_allocation.increment = 6;
    qos.reader_resource_limits().max_samples_per_read = 33;

    // .data_sharing
    qos.data_sharing().on("/");

    ASSERT_TRUE(subscriber->set_default_datareader_qos(qos) == ReturnCode_t::RETCODE_OK);

    DataReaderQos wqos;
    subscriber->get_default_datareader_qos(wqos);

    // .durability
    EXPECT_EQ(eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS, wqos.durability().kind);
    // .deadline
    EXPECT_EQ(10, wqos.deadline().period.seconds);
    EXPECT_EQ(20u, wqos.deadline().period.nanosec);
    // .latency_budget
    EXPECT_EQ(20, wqos.latency_budget().duration.seconds);
    EXPECT_EQ(30u, wqos.latency_budget().duration.nanosec);
    // .liveliness
    EXPECT_EQ(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, wqos.liveliness().kind);
    EXPECT_EQ(40, wqos.liveliness().lease_duration.seconds);
    EXPECT_EQ(61u, wqos.liveliness().lease_duration.nanosec);
    EXPECT_EQ(30, wqos.liveliness().announcement_period.seconds);
    EXPECT_EQ(50u, wqos.liveliness().announcement_period.nanosec);
    // .reliability
    EXPECT_EQ(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS, wqos.reliability().kind);
    // .destination_order
    EXPECT_EQ(eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, wqos.destination_order().kind);
    // . history
    EXPECT_EQ(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS, wqos.history().kind);
    EXPECT_EQ(1000, wqos.history().depth);
    // .resource_limits
    EXPECT_EQ(3000, wqos.resource_limits().max_samples);
    EXPECT_EQ(100, wqos.resource_limits().max_instances);
    EXPECT_EQ(500, wqos.resource_limits().max_samples_per_instance);
    EXPECT_EQ(50, wqos.resource_limits().allocated_samples);
    EXPECT_EQ(2, wqos.resource_limits().extra_samples);
    // .user_data
    size_t count = 1;
    for (auto user_value : wqos.user_data())
    {
        switch (count)
        {
            case 1:
                EXPECT_EQ(0, user_value);
                break;
            case 2:
                EXPECT_EQ(1, user_value);
                break;
            case 3:
                EXPECT_EQ(2, user_value);
                break;
            case 4:
                EXPECT_EQ(3, user_value);
                break;
            default:
                EXPECT_TRUE(false);
        }
        ++count;
    }
    // .ownership
    EXPECT_EQ(eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS, wqos.ownership().kind);
    // .time_based_filter
    EXPECT_EQ(eprosima::fastrtps::Time_t::INFINITE_SECONDS, wqos.time_based_filter().minimum_separation.seconds);
    EXPECT_EQ(eprosima::fastrtps::Time_t::INFINITE_NANOSECONDS, wqos.time_based_filter().minimum_separation.nanosec);
    // .reader_data_lifecycle
    EXPECT_EQ(100, wqos.reader_data_lifecycle().autopurge_disposed_samples_delay.seconds);
    EXPECT_EQ(30000u, wqos.reader_data_lifecycle().autopurge_disposed_samples_delay.nanosec);
    EXPECT_EQ(30000, wqos.reader_data_lifecycle().autopurge_no_writer_samples_delay.seconds);
    EXPECT_EQ(100u, wqos.reader_data_lifecycle().autopurge_no_writer_samples_delay.nanosec);
    // .lifespan
    EXPECT_EQ(10, wqos.lifespan().duration.seconds);
    EXPECT_EQ(33u, wqos.lifespan().duration.nanosec);
    // .durability_service
    EXPECT_EQ(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS, wqos.durability_service().history_kind);
    EXPECT_EQ(10, wqos.durability_service().history_depth);
    EXPECT_EQ(5, wqos.durability_service().max_samples);
    EXPECT_EQ(20, wqos.durability_service().max_instances);
    EXPECT_EQ(30, wqos.durability_service().max_samples_per_instance);
    // .reliable_reader_qos
    EXPECT_EQ(34, wqos.reliable_reader_qos().times.initialAcknackDelay.seconds);
    EXPECT_EQ(32u, wqos.reliable_reader_qos().times.initialAcknackDelay.nanosec);
    EXPECT_EQ(432, wqos.reliable_reader_qos().times.heartbeatResponseDelay.seconds);
    EXPECT_EQ(43u, wqos.reliable_reader_qos().times.heartbeatResponseDelay.nanosec);
    EXPECT_TRUE(wqos.reliable_reader_qos().disable_positive_ACKs.enabled);
    EXPECT_EQ(13, wqos.reliable_reader_qos().disable_positive_ACKs.duration.seconds);
    EXPECT_EQ(320u, wqos.reliable_reader_qos().disable_positive_ACKs.duration.nanosec);
    // .type_consistency
    EXPECT_EQ(XML_DATA_REPRESENTATION, wqos.type_consistency().representation.m_value.at(0));
    EXPECT_EQ(XCDR_DATA_REPRESENTATION, wqos.type_consistency().representation.m_value.at(1));
    EXPECT_FALSE(wqos.type_consistency().type_consistency.m_ignore_sequence_bounds);
    EXPECT_FALSE(wqos.type_consistency().type_consistency.m_ignore_string_bounds);
    EXPECT_TRUE(wqos.type_consistency().type_consistency.m_ignore_member_names);
    EXPECT_TRUE(wqos.type_consistency().type_consistency.m_prevent_type_widening);
    EXPECT_TRUE(wqos.type_consistency().type_consistency.m_force_type_validation);
    // .expects_inline_qos
    EXPECT_TRUE(wqos.expects_inline_qos());
    // .properties
    count = 1;
    for (auto prop : wqos.properties().properties())
    {
        switch (count)
        {
            case 1:
                EXPECT_EQ(0, prop.name().compare("Property1"));
                EXPECT_EQ(0, prop.value().compare("Value1"));
                break;
            case 2:
                EXPECT_EQ(0, prop.name().compare("Property2"));
                EXPECT_EQ(0, prop.value().compare("Value2"));
                break;
            default:
                EXPECT_TRUE(false);
        }
        ++count;
    }
    // .endpoint
    EXPECT_EQ(1, wqos.endpoint().user_defined_id);
    EXPECT_EQ(2, wqos.endpoint().entity_id);
    EXPECT_EQ(eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE, wqos.endpoint().history_memory_policy);
    // .reader_resource_limits
    EXPECT_EQ(30u, wqos.reader_resource_limits().matched_publisher_allocation.initial);
    EXPECT_EQ(300u, wqos.reader_resource_limits().matched_publisher_allocation.maximum);
    EXPECT_EQ(4u, wqos.reader_resource_limits().matched_publisher_allocation.increment);
    EXPECT_EQ(40u, wqos.reader_resource_limits().sample_infos_allocation.initial);
    EXPECT_EQ(400u, wqos.reader_resource_limits().sample_infos_allocation.maximum);
    EXPECT_EQ(5u, wqos.reader_resource_limits().sample_infos_allocation.increment);
    EXPECT_EQ(50u, wqos.reader_resource_limits().outstanding_reads_allocation.initial);
    EXPECT_EQ(500u, wqos.reader_resource_limits().outstanding_reads_allocation.maximum);
    EXPECT_EQ(6u, wqos.reader_resource_limits().outstanding_reads_allocation.increment);
    EXPECT_EQ(33, wqos.reader_resource_limits().max_samples_per_read);
    // .data_sharing
    EXPECT_EQ(eprosima::fastdds::dds::ON, wqos.data_sharing().kind());
    EXPECT_EQ(0, wqos.data_sharing().shm_directory().compare("/"));

    EXPECT_EQ(qos, wqos);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(SubscriberTests, ChangePSMDefaultDataReaderQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::Subscriber subscriber = ::dds::sub::Subscriber(participant, SUBSCRIBER_QOS_DEFAULT);

    ::dds::sub::qos::DataReaderQos qos = subscriber.default_datareader_qos();
    ASSERT_EQ(qos, DATAREADER_QOS_DEFAULT);

    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    ASSERT_NO_THROW(subscriber.default_datareader_qos(qos));

    ::dds::sub::qos::DataReaderQos rqos = subscriber.default_datareader_qos();

    ASSERT_EQ(qos, rqos);
    ASSERT_EQ(rqos.reliability().kind, BEST_EFFORT_RELIABILITY_QOS);
}

TEST(SubscriberTests, GetSubscriberParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_EQ(subscriber->get_participant(), participant);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(SubscriberTests, GetPSMSubscriberParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::Subscriber subscriber = ::dds::sub::Subscriber(participant, SUBSCRIBER_QOS_DEFAULT);

    ASSERT_EQ(subscriber.participant().delegate().get(), participant.delegate().get());
}

TEST(SubscriberTests, CreateDataReader)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void check_datareader_with_profile (
        DataReader* datareader,
        const std::string& profile_name)
{
    DataReaderQos qos;
    datareader->get_qos(qos);

    SubscriberAttributes subscriber_atts;
    XMLProfileManager::fillSubscriberAttributes(profile_name, subscriber_atts);

    //Values taken from profile
    ASSERT_TRUE(
        qos.reader_resource_limits().matched_publisher_allocation ==
        subscriber_atts.matched_publisher_allocation);
    if (subscriber_atts.qos.m_partition.names().empty())
    {
        ASSERT_TRUE(qos.properties() == subscriber_atts.properties);
    }
    else
    {
        ASSERT_NE(PropertyPolicyHelper::find_property(qos.properties(), "partitions"), nullptr);
        for (auto partition: subscriber_atts.qos.m_partition.names())
        {
            ASSERT_NE(PropertyPolicyHelper::find_property(qos.properties(), "partitions")->find(
                        partition), std::string::npos);
        }
    }
    ASSERT_TRUE(qos.expects_inline_qos() == subscriber_atts.expectsInlineQos);
    ASSERT_TRUE(qos.endpoint().unicast_locator_list == subscriber_atts.unicastLocatorList);
    ASSERT_TRUE(qos.endpoint().multicast_locator_list == subscriber_atts.multicastLocatorList);
    ASSERT_TRUE(qos.endpoint().remote_locator_list == subscriber_atts.remoteLocatorList);
    ASSERT_TRUE(qos.endpoint().history_memory_policy == subscriber_atts.historyMemoryPolicy);
    ASSERT_TRUE(qos.endpoint().user_defined_id == subscriber_atts.getUserDefinedID());
    ASSERT_TRUE(qos.endpoint().entity_id == subscriber_atts.getEntityID());
    ASSERT_TRUE(qos.reliable_reader_qos().times == subscriber_atts.times);
    ASSERT_TRUE(qos.reliable_reader_qos().disable_positive_ACKs == subscriber_atts.qos.m_disablePositiveACKs);
    ASSERT_TRUE(qos.durability() == subscriber_atts.qos.m_durability);
    ASSERT_TRUE(qos.durability_service() == subscriber_atts.qos.m_durabilityService);
    ASSERT_TRUE(qos.deadline() == subscriber_atts.qos.m_deadline);
    ASSERT_TRUE(qos.latency_budget() == subscriber_atts.qos.m_latencyBudget);
    ASSERT_TRUE(qos.liveliness() == subscriber_atts.qos.m_liveliness);
    ASSERT_TRUE(qos.reliability() == subscriber_atts.qos.m_reliability);
    ASSERT_TRUE(qos.lifespan() == subscriber_atts.qos.m_lifespan);
    ASSERT_TRUE(qos.user_data().data_vec() == subscriber_atts.qos.m_userData.data_vec());
    ASSERT_TRUE(qos.ownership() == subscriber_atts.qos.m_ownership);
    ASSERT_TRUE(qos.destination_order() == subscriber_atts.qos.m_destinationOrder);
    ASSERT_TRUE(qos.type_consistency().type_consistency == subscriber_atts.qos.type_consistency);
    ASSERT_TRUE(qos.type_consistency().representation == subscriber_atts.qos.representation);
    ASSERT_TRUE(qos.time_based_filter() == subscriber_atts.qos.m_timeBasedFilter);
    ASSERT_TRUE(qos.history() == subscriber_atts.topic.historyQos);
    ASSERT_TRUE(qos.resource_limits() == subscriber_atts.topic.resourceLimitsQos);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.reader_data_lifecycle() == DATAREADER_QOS_DEFAULT.reader_data_lifecycle());
}

TEST(SubscriberTests, CreateDataReaderWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profile.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);

    //Datareader using the default profile
    DataReader* default_datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(default_datareader, nullptr);
    check_datareader_with_profile(default_datareader, "test_default_subscriber_profile");
    ASSERT_TRUE(subscriber->delete_datareader(default_datareader) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    DataReader* datareader = subscriber->create_datareader_with_profile(topic, "test_subscriber_profile");
    ASSERT_NE(datareader, nullptr);
    check_datareader_with_profile(datareader, "test_subscriber_profile");
    ASSERT_TRUE(subscriber->delete_datareader(datareader) == ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(SubscriberTests, GetDataReaderProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profile.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Extract qos from profile
    DataReaderQos qos;
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_profile("test_subscriber_profile", qos),
        ReturnCode_t::RETCODE_OK);

    //DataReader using the extracted qos
    DataReader* datareader = subscriber->create_datareader(topic, qos);
    ASSERT_NE(datareader, nullptr);

    check_datareader_with_profile(datareader, "test_subscriber_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(subscriber->delete_datareader(datareader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(SubscriberTests, DeleteSubscriberWithReaders)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

//TODO: [ILG] Activate the test once PSM API for DataReader is in place
/*
   TEST(SubscriberTests, CreatePSMDataReader)
   {
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ::dds::sub::Subscriber subscriber = ::dds::core::null;
    subscriber = ::dds::sub::Subscriber(participant);

    ASSERT_NE(subscriber, ::dds::core::null);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant.delegate().get());

    ::dds::topic::Topic topic = ::dds::core::null;
    topic = ::dds::topic::Topic(participant, "footopic", type_->getName(), TOPIC_QOS_DEFAULT);

    ASSERT_NE(topic, ::dds::core::null);

    ::dds::sub::DataReader data_reader = ::dds::core::null;
    data_reader = ::dds::sub::DataReader(subscriber, topic);

    ASSERT_NE(data_reader, ::dds::core::null);
   }
 */

void set_listener_test (
        Subscriber* subscriber,
        SubscriberListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(subscriber->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(subscriber->get_status_mask(), mask);
}

class CustomListener : public SubscriberListener
{

};

TEST(SubscriberTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &listener);
    ASSERT_NE(subscriber, nullptr);
    ASSERT_EQ(subscriber->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<Subscriber*, SubscriberListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { subscriber, &listener, StatusMask::data_on_readers() },
        { subscriber, &listener, StatusMask::data_available() },
        { subscriber, &listener, StatusMask::sample_rejected() },
        { subscriber, &listener, StatusMask::liveliness_changed() },
        { subscriber, &listener, StatusMask::requested_deadline_missed() },
        { subscriber, &listener, StatusMask::requested_incompatible_qos() },
        { subscriber, &listener, StatusMask::subscription_matched() },
        { subscriber, &listener, StatusMask::sample_lost() },
        //all except one
        { subscriber, &listener, StatusMask::all() >> StatusMask::data_on_readers() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::data_available() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::sample_rejected() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::liveliness_changed() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::requested_deadline_missed() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::requested_incompatible_qos() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::subscription_matched() },
        { subscriber, &listener, StatusMask::all() >> StatusMask::sample_lost() },
        //all and none
        { subscriber, &listener, StatusMask::all() },
        { subscriber, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that the Subscriber methods defined in the standard not yet implemented in FastDDS return
 * ReturnCode_t::RETCODE_UNSUPPORTED. The following methods are checked:
 * 1. copy_from_topic_qos
 * 2. begin_access
 * 3. end_access
 * 4. get_datareaders (all parameters)
 */
TEST(SubscriberTests, UnsupportedPublisherMethods)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    std::vector<DataReader*> readers;
    std::vector<SampleStateKind> sample_states;
    std::vector<ViewStateKind> view_states;
    std::vector<InstanceStateKind> instance_states;

    fastdds::dds::DataReaderQos reader_qos;
    fastdds::dds::TopicQos topic_qos;
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, subscriber->copy_from_topic_qos(reader_qos, topic_qos));
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, subscriber->begin_access());
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, subscriber->end_access());
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, subscriber->get_datareaders(
                readers,
                sample_states,
                view_states,
                instance_states));

    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/**
 * This test checks that changing the PartitionQosPolicy on a subscriber takes effect on changing the actual QoS.
 * It was discovered in https://github.com/eProsima/Fast-DDS/issues/2107 that this was not correctly handled when
 * setting an empty partitions list on a subscriber that already had some partitions. The test does the following:
 *
 *    1. Create a subscriber with default QoS
 *    2. Add a partition
 *    3. Add three more partitions
 *    4. Remove 1 partition
 *    5. Remove 2 more partition
 *    6. Remove all partitions
 */
TEST(SubscriberTests, UpdatePartitions)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);
    ASSERT_EQ(subscriber->get_qos().partition().size(), 0u);

    // Add 1 partition to subscriber
    SubscriberQos sub_qos;
    PartitionQosPolicy partitions;
    partitions.push_back("partition_1");
    sub_qos.partition() = partitions;
    subscriber->set_qos(sub_qos);
    ASSERT_EQ(subscriber->get_qos().partition().size(), 1u);
    ASSERT_EQ(partitions, subscriber->get_qos().partition());

    // Add 3 more partitions to subscriber
    partitions.push_back("partition_2");
    partitions.push_back("partition_3");
    partitions.push_back("partition_4");
    sub_qos.partition() = partitions;
    subscriber->set_qos(sub_qos);
    ASSERT_EQ(subscriber->get_qos().partition().size(), 4u);
    ASSERT_EQ(partitions, subscriber->get_qos().partition());

    // Remove 1 partition from subscriber
    partitions.clear();
    ASSERT_TRUE(static_cast<bool>(partitions.empty()));
    partitions.push_back("partition_1");
    partitions.push_back("partition_2");
    partitions.push_back("partition_3");
    sub_qos.partition() = partitions;
    subscriber->set_qos(sub_qos);
    ASSERT_EQ(subscriber->get_qos().partition().size(), 3u);
    ASSERT_EQ(partitions, subscriber->get_qos().partition());

    // Remove 2 more partitions from the subscriber
    partitions.clear();
    ASSERT_TRUE(partitions.empty());
    partitions.push_back("partition_1");
    sub_qos.partition() = partitions;
    subscriber->set_qos(sub_qos);
    ASSERT_EQ(subscriber->get_qos().partition().size(), 1u);
    ASSERT_EQ(partitions, subscriber->get_qos().partition());

    // Remove all partitions from the subscriber
    partitions.clear();
    ASSERT_TRUE(partitions.empty());
    sub_qos.partition() = partitions;
    subscriber->set_qos(sub_qos);
    ASSERT_EQ(subscriber->get_qos().partition().size(), 0u);
    ASSERT_EQ(partitions, subscriber->get_qos().partition());
}

// Delete contained entities test
TEST(SubscriberTests, DeleteContainedEntities)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);



    TypeSupport type(new TopicDataTypeMock());
    //TypeSupport type;
    type.register_type(participant);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);


    Topic* topic_foo = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic_foo, nullptr);
    Topic* topic_bar = participant->create_topic("bartopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic_bar, nullptr);

    DataReader* data_reader_foo = subscriber->create_datareader(topic_foo, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader_foo, nullptr);
    DataReader* data_reader_bar = subscriber->create_datareader(topic_bar, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader_bar, nullptr);

    DataWriter* data_writer_foo = publisher->create_datawriter(topic_foo, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_reader_bar, nullptr);

    InstanceHandle_t handle_nil = HANDLE_NIL;
    BarType data;
    data.index(1);
    type.get_key(&data, &handle_nil);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, data_writer_foo->write(&data, HANDLE_NIL));

    // Wait for data to arrive and check OK should be returned
    Duration_t wait_time(1, 0);
    EXPECT_TRUE(data_reader_foo->wait_for_unread_message(wait_time));

    LoanableSequence<BarType> mock_coll;
    SampleInfoSeq mock_seq;

    ASSERT_EQ(data_reader_foo->take(mock_coll, mock_seq), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(subscriber->delete_contained_entities(), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(data_reader_foo->return_loan(mock_coll, mock_seq), ReturnCode_t::RETCODE_OK);

    SampleStateMask mock_sample_state_kind = ANY_SAMPLE_STATE;
    ViewStateMask mock_view_state_kind = ANY_VIEW_STATE;
    InstanceStateMask mock_instance_states = ANY_INSTANCE_STATE;
    const std::string mock_query_expression;
    const std::vector<std::string> mock_query_parameters;

    QueryCondition* query_condition = data_reader_foo->create_querycondition(
        mock_sample_state_kind,
        mock_view_state_kind,
        mock_instance_states,
        mock_query_expression,
        mock_query_parameters
        );

    // To be updated when Query Conditions are available
    ASSERT_EQ(query_condition, nullptr);

    std::vector<DataReader*> data_reader_list;
    subscriber->get_datareaders(data_reader_list);
    ASSERT_TRUE(data_reader_list.size() == 2);

    data_reader_list.clear();
    ASSERT_EQ(subscriber->delete_contained_entities(), ReturnCode_t::RETCODE_OK);

    subscriber->get_datareaders(data_reader_list);
    ASSERT_TRUE(data_reader_list.size() == 0);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
