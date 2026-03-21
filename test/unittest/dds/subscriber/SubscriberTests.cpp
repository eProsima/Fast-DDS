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

#include <fastcdr/Cdr.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>

#include <FileUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using rtps::PropertyPolicyHelper;

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
        max_serialized_type_size = 4u;
        set_name("footype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
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
            DataRepresentationId_t /*data_representation*/) override
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

};

TEST(SubscriberTests, ChangeSubscriberQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    SubscriberQos qos;
    ASSERT_EQ(subscriber->get_qos(qos), RETCODE_OK);

    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_EQ(subscriber->set_qos(qos), RETCODE_OK);
    SubscriberQos pqos;
    ASSERT_EQ(subscriber->get_qos(pqos), RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);

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
    qos.time_based_filter().minimum_separation.seconds = eprosima::fastdds::dds::Time_t::INFINITE_SECONDS;
    qos.time_based_filter().minimum_separation.nanosec = eprosima::fastdds::dds::Time_t::INFINITE_NANOSECONDS;
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
    qos.reliable_reader_qos().times.initial_acknack_delay.seconds = 34;
    qos.reliable_reader_qos().times.initial_acknack_delay.nanosec = 32u;
    qos.reliable_reader_qos().times.heartbeat_response_delay.seconds = 432;
    qos.reliable_reader_qos().times.heartbeat_response_delay.nanosec = 43u;
    qos.reliable_reader_qos().disable_positive_acks.enabled = true;
    qos.reliable_reader_qos().disable_positive_acks.duration.seconds = 13;
    qos.reliable_reader_qos().disable_positive_acks.duration.nanosec = 320u;
    // .type_consistency
    qos.representation().m_value.push_back(XML_DATA_REPRESENTATION);
    qos.representation().m_value.push_back(XCDR_DATA_REPRESENTATION);
    qos.type_consistency().m_ignore_sequence_bounds = false;
    qos.type_consistency().m_ignore_string_bounds = false;
    qos.type_consistency().m_ignore_member_names = true;
    qos.type_consistency().m_prevent_type_widening = true;
    qos.type_consistency().m_force_type_validation = true;
    // .expects_inline_qos
    qos.expects_inline_qos(true);
    // .properties
    eprosima::fastdds::rtps::Property property;
    property.name("Property1");
    property.value("Value1");
    qos.properties().properties().push_back(property);
    property.name("Property2");
    property.value("Value2");
    qos.properties().properties().push_back(property);

    // .endpoint
    qos.endpoint().user_defined_id = 1;
    qos.endpoint().entity_id = 2;
    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

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

    ASSERT_TRUE(subscriber->set_default_datareader_qos(qos) == RETCODE_OK);

    // Obtain already modified qos
    DataReaderQos rqos;
    subscriber->get_default_datareader_qos(rqos);

    // .durability
    EXPECT_EQ(eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS, rqos.durability().kind);
    // .deadline
    EXPECT_EQ(10, rqos.deadline().period.seconds);
    EXPECT_EQ(20u, rqos.deadline().period.nanosec);
    // .latency_budget
    EXPECT_EQ(20, rqos.latency_budget().duration.seconds);
    EXPECT_EQ(30u, rqos.latency_budget().duration.nanosec);
    // .liveliness
    EXPECT_EQ(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, rqos.liveliness().kind);
    EXPECT_EQ(40, rqos.liveliness().lease_duration.seconds);
    EXPECT_EQ(61u, rqos.liveliness().lease_duration.nanosec);
    EXPECT_EQ(30, rqos.liveliness().announcement_period.seconds);
    EXPECT_EQ(50u, rqos.liveliness().announcement_period.nanosec);
    // .reliability
    EXPECT_EQ(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS, rqos.reliability().kind);
    // .destination_order
    EXPECT_EQ(eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, rqos.destination_order().kind);
    // . history
    EXPECT_EQ(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS, rqos.history().kind);
    EXPECT_EQ(1000, rqos.history().depth);
    // .resource_limits
    EXPECT_EQ(3000, rqos.resource_limits().max_samples);
    EXPECT_EQ(100, rqos.resource_limits().max_instances);
    EXPECT_EQ(500, rqos.resource_limits().max_samples_per_instance);
    EXPECT_EQ(50, rqos.resource_limits().allocated_samples);
    EXPECT_EQ(2, rqos.resource_limits().extra_samples);
    // .user_data
    size_t count = 1;
    for (auto user_value : rqos.user_data())
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
    EXPECT_EQ(eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS, rqos.ownership().kind);
    // .time_based_filter
    EXPECT_EQ(eprosima::fastdds::dds::Time_t::INFINITE_SECONDS, rqos.time_based_filter().minimum_separation.seconds);
    EXPECT_EQ(eprosima::fastdds::dds::Time_t::INFINITE_NANOSECONDS,
            rqos.time_based_filter().minimum_separation.nanosec);
    // .reader_data_lifecycle
    EXPECT_EQ(100, rqos.reader_data_lifecycle().autopurge_disposed_samples_delay.seconds);
    EXPECT_EQ(30000u, rqos.reader_data_lifecycle().autopurge_disposed_samples_delay.nanosec);
    EXPECT_EQ(30000, rqos.reader_data_lifecycle().autopurge_no_writer_samples_delay.seconds);
    EXPECT_EQ(100u, rqos.reader_data_lifecycle().autopurge_no_writer_samples_delay.nanosec);
    // .lifespan
    EXPECT_EQ(10, rqos.lifespan().duration.seconds);
    EXPECT_EQ(33u, rqos.lifespan().duration.nanosec);
    // .durability_service
    EXPECT_EQ(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS, rqos.durability_service().history_kind);
    EXPECT_EQ(10, rqos.durability_service().history_depth);
    EXPECT_EQ(5, rqos.durability_service().max_samples);
    EXPECT_EQ(20, rqos.durability_service().max_instances);
    EXPECT_EQ(30, rqos.durability_service().max_samples_per_instance);
    // .reliable_reader_qos
    EXPECT_EQ(34, rqos.reliable_reader_qos().times.initial_acknack_delay.seconds);
    EXPECT_EQ(32u, rqos.reliable_reader_qos().times.initial_acknack_delay.nanosec);
    EXPECT_EQ(432, rqos.reliable_reader_qos().times.heartbeat_response_delay.seconds);
    EXPECT_EQ(43u, rqos.reliable_reader_qos().times.heartbeat_response_delay.nanosec);
    EXPECT_TRUE(rqos.reliable_reader_qos().disable_positive_acks.enabled);
    EXPECT_EQ(13, rqos.reliable_reader_qos().disable_positive_acks.duration.seconds);
    EXPECT_EQ(320u, rqos.reliable_reader_qos().disable_positive_acks.duration.nanosec);
    // .type_consistency
    EXPECT_EQ(XML_DATA_REPRESENTATION, rqos.representation().m_value.at(0));
    EXPECT_EQ(XCDR_DATA_REPRESENTATION, rqos.representation().m_value.at(1));
    EXPECT_FALSE(rqos.type_consistency().m_ignore_sequence_bounds);
    EXPECT_FALSE(rqos.type_consistency().m_ignore_string_bounds);
    EXPECT_TRUE(rqos.type_consistency().m_ignore_member_names);
    EXPECT_TRUE(rqos.type_consistency().m_prevent_type_widening);
    EXPECT_TRUE(rqos.type_consistency().m_force_type_validation);
    // .expects_inline_qos
    EXPECT_TRUE(rqos.expects_inline_qos());
    // .properties
    count = 1;
    for (auto prop : rqos.properties().properties())
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
    EXPECT_EQ(1, rqos.endpoint().user_defined_id);
    EXPECT_EQ(2, rqos.endpoint().entity_id);
    EXPECT_EQ(eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE, rqos.endpoint().history_memory_policy);
    // .reader_resource_limits
    EXPECT_EQ(30u, rqos.reader_resource_limits().matched_publisher_allocation.initial);
    EXPECT_EQ(300u, rqos.reader_resource_limits().matched_publisher_allocation.maximum);
    EXPECT_EQ(4u, rqos.reader_resource_limits().matched_publisher_allocation.increment);
    EXPECT_EQ(40u, rqos.reader_resource_limits().sample_infos_allocation.initial);
    EXPECT_EQ(400u, rqos.reader_resource_limits().sample_infos_allocation.maximum);
    EXPECT_EQ(5u, rqos.reader_resource_limits().sample_infos_allocation.increment);
    EXPECT_EQ(50u, rqos.reader_resource_limits().outstanding_reads_allocation.initial);
    EXPECT_EQ(500u, rqos.reader_resource_limits().outstanding_reads_allocation.maximum);
    EXPECT_EQ(6u, rqos.reader_resource_limits().outstanding_reads_allocation.increment);
    EXPECT_EQ(33, rqos.reader_resource_limits().max_samples_per_read);
    // .data_sharing
    EXPECT_EQ(eprosima::fastdds::dds::ON, rqos.data_sharing().kind());
    EXPECT_EQ(0, rqos.data_sharing().shm_directory().compare("/"));

    EXPECT_EQ(qos, rqos);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(SubscriberTests, GetSubscriberParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_EQ(subscriber->get_participant(), participant);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
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

    ASSERT_EQ(subscriber->delete_datareader(data_reader), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

void check_datareader_with_profile (
        DataReader* datareader,
        const std::string& profile_name)
{
    DataReaderQos qos;
    datareader->get_qos(qos);

    DataReaderQos profile_qos;
    EXPECT_EQ(datareader->get_subscriber()->get_datareader_qos_from_profile(profile_name, profile_qos),
            RETCODE_OK);
    EXPECT_EQ(qos, profile_qos);
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
    ASSERT_TRUE(subscriber->delete_datareader(default_datareader) == RETCODE_OK);

    //participant using non-default profile
    DataReader* datareader = subscriber->create_datareader_with_profile(topic, "test_subscriber_profile");
    ASSERT_NE(datareader, nullptr);
    check_datareader_with_profile(datareader, "test_subscriber_profile");
    ASSERT_TRUE(subscriber->delete_datareader(datareader) == RETCODE_OK);

    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
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
        RETCODE_OK);

    //DataReader using the extracted qos
    DataReader* datareader = subscriber->create_datareader(topic, qos);
    ASSERT_NE(datareader, nullptr);

    check_datareader_with_profile(datareader, "test_subscriber_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_profile("incorrect_profile_name", qos),
        RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(subscriber->delete_datareader(datareader), RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(SubscriberTests, GetDataReaderQosFromXml)
{
    const std::string xml_filename("test_xml_profile.xml");
    const std::string profile_name("test_subscriber_profile");

    std::string complete_xml = testing::load_file(xml_filename);

    // Disable created auxiliar entities to avoid polluting traffic
    DomainParticipantFactoryQos factory_qos;
    DomainParticipantFactory::get_instance()->get_qos(factory_qos);
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    // Get QoS given profile name
    DataReaderQos qos;
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_xml(complete_xml, qos, profile_name),
        RETCODE_OK);

    // Get QoS without providing profile name (gets first one found)
    DataReaderQos qos_empty_profile;
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_xml(complete_xml, qos_empty_profile),
        RETCODE_OK);

    // Check they correspond to the same profile
    // NOTE: test_subscriber_profile is assumed to be the first subscriber profile in the XML file
    EXPECT_EQ(qos, qos_empty_profile);

    // Load profiles from XML file and get QoS given profile name
    DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_filename);
    DataReaderQos qos_from_profile;
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_profile(profile_name, qos_from_profile),
        RETCODE_OK);

    // Check they correspond to the same profile
    EXPECT_EQ(qos, qos_from_profile);

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        subscriber->get_datareader_qos_from_xml(complete_xml, qos, "incorrect_profile_name"),
        RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(PublisherTests, GetDefaultDataReaderQosFromXml)
{
    const std::string xml_filename("test_xml_profile.xml");

    std::string complete_xml = testing::load_file(xml_filename);

    // Disable created auxiliar entities to avoid polluting traffic
    DomainParticipantFactoryQos factory_qos;
    DomainParticipantFactory::get_instance()->get_qos(factory_qos);
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    // Get default QoS from XML
    DataReaderQos default_qos;
    EXPECT_EQ(
        subscriber->get_default_datareader_qos_from_xml(complete_xml, default_qos),
        RETCODE_OK);

    // Load profiles from XML file and get default QoS after resetting its value
    DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_filename);
    // NOTE: At the time of this writing, the only way to reset the default qos after loading an XML is to do as follows
    DomainParticipantFactory::get_instance()->load_profiles();
    subscriber->set_default_datareader_qos(DATAREADER_QOS_DEFAULT);
    DataReaderQos default_qos_from_profile;
    EXPECT_EQ(
        subscriber->get_default_datareader_qos(default_qos_from_profile),
        RETCODE_OK);

    // Check they correspond to the same profile
    EXPECT_EQ(default_qos, default_qos_from_profile);

    // Clean up
    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
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

    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

void set_listener_test (
        Subscriber* subscriber,
        SubscriberListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(subscriber->set_listener(listener, mask), RETCODE_OK);
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

    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

/*
 * This test checks that the Subscriber methods defined in the standard not yet implemented in FastDDS return
 * RETCODE_UNSUPPORTED. The following methods are checked:
 * 1. begin_access
 * 2. end_access
 * 3. get_datareaders (all parameters)
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
    EXPECT_EQ(RETCODE_UNSUPPORTED, subscriber->begin_access());
    EXPECT_EQ(RETCODE_UNSUPPORTED, subscriber->end_access());
    EXPECT_EQ(RETCODE_UNSUPPORTED, subscriber->get_datareaders(
                readers,
                sample_states,
                view_states,
                instance_states));

    ASSERT_EQ(participant->delete_subscriber(subscriber), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
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
    type.compute_key(&data, handle_nil);
    EXPECT_EQ(RETCODE_OK, data_writer_foo->write(&data, HANDLE_NIL));

    // Wait for data to arrive and check OK should be returned
    dds::Duration_t wait_time(1, 0);
    EXPECT_TRUE(data_reader_foo->wait_for_unread_message(wait_time));

    LoanableSequence<BarType> mock_coll;
    fastdds::dds::SampleInfoSeq mock_seq;

    ASSERT_EQ(data_reader_foo->take(mock_coll, mock_seq), RETCODE_OK);

    ASSERT_EQ(subscriber->delete_contained_entities(), RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(data_reader_foo->return_loan(mock_coll, mock_seq), RETCODE_OK);

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
    ASSERT_EQ(subscriber->delete_contained_entities(), RETCODE_OK);

    subscriber->get_datareaders(data_reader_list);
    ASSERT_TRUE(data_reader_list.size() == 0);
}

/**
 * Utility class to set some values other than default to those Qos common to Topic and DataReader.
 *
 * This is a class instead of a free function to avoid linking with its TestsPublisher counterpart.
 */
class TestsSubscriberQosCommonUtils
{
public:

    // Set common Qos values to both TopicQos and DataReaderQos
    template<typename T>
    static void set_common_qos(
            T& qos)
    {
        qos.durability_service().history_kind = KEEP_ALL_HISTORY_QOS;
        qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        qos.deadline().period = {0, 500000000};
        qos.latency_budget().duration = 0;
        qos.liveliness().kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        qos.resource_limits().max_samples = 1000;
        qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
        // Representation is not on the same place in DataReaderQos and TopicQos
        set_representation_qos(qos);
        qos.destination_order().kind = eprosima::fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        qos.history().kind = KEEP_ALL_HISTORY_QOS;
    }

private:

    // Set representation Qos (as it is not in the same place in DataReaderQos and TopicQos)
    template<typename T>
    static void set_representation_qos(
            T& qos);
};

// Specialization for DataReaderQos
template<>
void TestsSubscriberQosCommonUtils::set_representation_qos(
        eprosima::fastdds::dds::DataReaderQos& qos)
{
    qos.representation().m_value.push_back(
        eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
}

// Specialization for TopicQos
template<>
void TestsSubscriberQosCommonUtils::set_representation_qos(
        eprosima::fastdds::dds::TopicQos& qos)
{
    qos.representation().m_value.push_back(eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
}

/*
 * This test:
 *   1. Creates a Topic with custom Qos
 *   2. Creates a control DataReaderQos in which the non-common Qos are set to a value different from the default
 *   3. Creates a test DataReaderQos and assigns it the value of the control Qos
 *   4. Updates the control Qos' common Qos with the same values used in the Topic Qos
 *   5. Calls Subscriber::copy_from_topic_qos() with the test Qos and the Topic Qos
 *   6. Checks that the resulting test Qos has the same values as the control Qos
 */
TEST(SubscriberTests, datareader_copy_from_topic_qos)
{
    /* Set Topic Qos different from default */
    TopicQos topic_qos;
    TestsSubscriberQosCommonUtils::set_common_qos(topic_qos);

    /* Create the subscriber under test */
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    /* Create control and test Qos instances */
    // Override non-common Qos with values different from the default on the control Qos
    DataReaderQos control_qos;
    control_qos.reader_data_lifecycle().autopurge_no_writer_samples_delay = {3, 0};
    control_qos.user_data().push_back(0);
    control_qos.endpoint().entity_id = 1;
    control_qos.reader_resource_limits().matched_publisher_allocation =
            eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1u);
    control_qos.data_sharing().off();

    // Copy control Qos to test Qos. At this point, test_qos has non-default values for the non-common Qos,
    // and default values for the common Qos
    DataReaderQos test_qos = control_qos;

    // Set common Qos to the control Qos with the same values used in the Topic Qos
    TestsSubscriberQosCommonUtils::set_common_qos(control_qos);

    /* Function under test call */
    subscriber->copy_from_topic_qos(test_qos, topic_qos);

    /* Check that the test Qos has the same values as the control Qos */
    ASSERT_EQ(control_qos, test_qos);
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
