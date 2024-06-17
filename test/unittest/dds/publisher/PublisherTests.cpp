// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.h>

namespace eprosima {
namespace fastdds {
namespace dds {

using fastdds::rtps::PropertyPolicyHelper;

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
            eprosima::fastdds::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    bool serialize(
            void* /*data*/,
            fastdds::rtps::SerializedPayload_t* /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t* /*payload*/,
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
            fastdds::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

};

class LoanableTopicDataTypeMock : public TopicDataType
{
public:

    LoanableTopicDataTypeMock()
        : TopicDataType()
    {
        m_typeSize = 4u;
        setName("loanablefootype");
    }

    bool serialize(
            void* data,
            eprosima::fastdds::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    bool serialize(
            void* /*data*/,
            fastdds::rtps::SerializedPayload_t* /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t* /*payload*/,
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

    inline bool is_bounded() const override
    {
        return true;
    }

    inline bool is_plain() const override
    {
        return true;
    }

    inline bool is_plain(
            DataRepresentationId_t) const override
    {
        return true;
    }

    bool getKey(
            void* /*data*/,
            fastdds::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

private:

    using TopicDataType::is_plain;
};



TEST(PublisherTests, GetPublisherParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    ASSERT_EQ(publisher->get_participant(), participant);

    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(PublisherTests, ChangeDefaultDataWriterQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);
    DataWriterQos qos;
    publisher->get_default_datawriter_qos(qos);
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    // .durability
    qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS;
    // .durability_service
    qos.durability_service().history_kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    qos.durability_service().history_depth = 10;
    qos.durability_service().max_samples = 5;
    qos.durability_service().max_instances = 20;
    qos.durability_service().max_samples_per_instance = 30;
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
    qos.reliability().max_blocking_time.seconds = 100;
    qos.reliability().max_blocking_time.nanosec = eprosima::fastdds::Time_t::INFINITE_NANOSECONDS;
    // .destination_order
    qos.destination_order().kind = eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    // .history
    qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    qos.history().depth = 1000;
    // .resource_limits
    qos.resource_limits().max_samples = 3000;
    qos.resource_limits().max_instances = 100;
    qos.resource_limits().max_samples_per_instance = 500;
    qos.resource_limits().allocated_samples = 50;
    qos.resource_limits().extra_samples = 2;
    // .transport_priority
    qos.transport_priority().value = 10;
    // .lifespan
    qos.lifespan().duration.seconds = 10;
    qos.lifespan().duration.nanosec = 33u;
    // .user_data
    qos.user_data().push_back(0);
    qos.user_data().push_back(1);
    qos.user_data().push_back(2);
    qos.user_data().push_back(3);
    // .ownership
    qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    // .ownership_strength
    qos.ownership_strength().value = 30;
    // .writer_data_lifecycle
    qos.writer_data_lifecycle().autodispose_unregistered_instances = false;
    // .publish_mode
    qos.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    qos.publish_mode().flow_controller_name = "Prueba";

    // .properties
    eprosima::fastdds::rtps::Property property;
    property.name("Property1");
    property.value("Value1");
    qos.properties().properties().push_back(property);
    property.name("Property2");
    property.value("Value2");
    qos.properties().properties().push_back(property);
    // .reliable_writer_qos
    qos.reliable_writer_qos().times.initialHeartbeatDelay.seconds = 2;
    qos.reliable_writer_qos().times.initialHeartbeatDelay.nanosec = 15u;
    qos.reliable_writer_qos().times.heartbeatPeriod.seconds = 3;
    qos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 16u;
    qos.reliable_writer_qos().times.nackResponseDelay.seconds = 4;
    qos.reliable_writer_qos().times.nackResponseDelay.nanosec = 17u;
    qos.reliable_writer_qos().times.nackSupressionDuration.seconds = 5;
    qos.reliable_writer_qos().times.nackSupressionDuration.nanosec = 18u;
    qos.reliable_writer_qos().disable_positive_acks.enabled = true;
    qos.reliable_writer_qos().disable_positive_acks.duration.seconds = 13;
    qos.reliable_writer_qos().disable_positive_acks.duration.nanosec = 320u;
    qos.reliable_writer_qos().disable_heartbeat_piggyback = true;
    // .endpoint
    qos.endpoint().user_defined_id = 1;
    qos.endpoint().entity_id = 2;
    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    // . writer_resource_limits
    qos.writer_resource_limits().matched_subscriber_allocation.initial = 30;
    qos.writer_resource_limits().matched_subscriber_allocation.maximum = 300;
    qos.writer_resource_limits().matched_subscriber_allocation.increment = 400;
    // . data_sharing
    qos.data_sharing().on("/");

    ASSERT_TRUE(publisher->set_default_datawriter_qos(qos) == RETCODE_OK);
    DataWriterQos wqos;
    publisher->get_default_datawriter_qos(wqos);

    // .durability
    EXPECT_EQ(eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS, wqos.durability().kind);
    // .durability_service
    EXPECT_EQ(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS, wqos.durability_service().history_kind);
    EXPECT_EQ(10, wqos.durability_service().history_depth);
    EXPECT_EQ(5, wqos.durability_service().max_samples);
    EXPECT_EQ(20, wqos.durability_service().max_instances);
    EXPECT_EQ(30, wqos.durability_service().max_samples_per_instance);
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
    EXPECT_EQ(100, wqos.reliability().max_blocking_time.seconds);
    EXPECT_EQ(eprosima::fastdds::Time_t::INFINITE_NANOSECONDS, wqos.reliability().max_blocking_time.nanosec);
    // .destination_order
    EXPECT_EQ(eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, wqos.destination_order().kind);
    // .history
    EXPECT_EQ(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS, wqos.history().kind);
    EXPECT_EQ(1000, wqos.history().depth);
    // .resource_limits
    EXPECT_EQ(3000, wqos.resource_limits().max_samples);
    EXPECT_EQ(100, wqos.resource_limits().max_instances);
    EXPECT_EQ(500, wqos.resource_limits().max_samples_per_instance);
    EXPECT_EQ(50, wqos.resource_limits().allocated_samples);
    EXPECT_EQ(2, wqos.resource_limits().extra_samples);
    // .transport_priority
    EXPECT_EQ(10u, wqos.transport_priority().value);
    // .lifespan
    EXPECT_EQ(10, wqos.lifespan().duration.seconds);
    EXPECT_EQ(33u, wqos.lifespan().duration.nanosec);
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
    // .ownership_strength
    EXPECT_EQ(30u, wqos.ownership_strength().value);
    // .writer_data_lifecycle
    EXPECT_FALSE(wqos.writer_data_lifecycle().autodispose_unregistered_instances);
    // .publish_mode
    EXPECT_EQ(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE, wqos.publish_mode().kind);
    EXPECT_EQ(true, wqos.publish_mode().flow_controller_name == "Prueba");
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
    // .reliable_writer_qos
    EXPECT_EQ(2, wqos.reliable_writer_qos().times.initialHeartbeatDelay.seconds);
    EXPECT_EQ(15u, wqos.reliable_writer_qos().times.initialHeartbeatDelay.nanosec);
    EXPECT_EQ(3, wqos.reliable_writer_qos().times.heartbeatPeriod.seconds);
    EXPECT_EQ(16u, wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec);
    EXPECT_EQ(4, wqos.reliable_writer_qos().times.nackResponseDelay.seconds);
    EXPECT_EQ(17u, wqos.reliable_writer_qos().times.nackResponseDelay.nanosec);
    EXPECT_EQ(5, wqos.reliable_writer_qos().times.nackSupressionDuration.seconds);
    EXPECT_EQ(18u, wqos.reliable_writer_qos().times.nackSupressionDuration.nanosec);
    EXPECT_TRUE(wqos.reliable_writer_qos().disable_positive_acks.enabled);
    EXPECT_EQ(13, wqos.reliable_writer_qos().disable_positive_acks.duration.seconds);
    EXPECT_EQ(320u, wqos.reliable_writer_qos().disable_positive_acks.duration.nanosec);
    EXPECT_TRUE(wqos.reliable_writer_qos().disable_heartbeat_piggyback);
    // .endpoint
    EXPECT_EQ(1, wqos.endpoint().user_defined_id);
    EXPECT_EQ(2, wqos.endpoint().entity_id);
    EXPECT_EQ(eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE, wqos.endpoint().history_memory_policy);
    // . writer_resource_limits
    EXPECT_EQ(30, wqos.writer_resource_limits().matched_subscriber_allocation.initial);
    EXPECT_EQ(300, wqos.writer_resource_limits().matched_subscriber_allocation.maximum);
    EXPECT_EQ(400, wqos.writer_resource_limits().matched_subscriber_allocation.increment);
    // . data_sharing
    EXPECT_EQ(eprosima::fastdds::dds::ON, wqos.data_sharing().kind());
    EXPECT_EQ(0, wqos.data_sharing().shm_directory().compare("/"));

    EXPECT_TRUE(qos == wqos);

    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}


TEST(PublisherTests, ChangePublisherQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    PublisherQos qos;
    publisher->get_qos(qos);

    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    publisher->set_qos(qos);
    PublisherQos pqos;
    publisher->get_qos(pqos);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);

}

TEST(PublisherTests, CreateDataWriter)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    ASSERT_EQ(publisher->delete_datawriter(datawriter), RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

void check_datawriter_with_profile (
        DataWriter* datawriter,
        const std::string& profile_name)
{
    DataWriterQos qos;
    datawriter->get_qos(qos);

    DataWriterQos profile_qos;
    EXPECT_EQ(datawriter->get_publisher()->get_datawriter_qos_from_profile(profile_name, profile_qos),
            RETCODE_OK);
    EXPECT_EQ(qos, profile_qos);
}

TEST(PublisherTests, CreateDataWriterWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profile.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);

    //Datawriter using the default profile
    DataWriter* default_datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(default_datawriter, nullptr);
    check_datawriter_with_profile(default_datawriter, "test_default_publisher_profile");
    ASSERT_TRUE(publisher->delete_datawriter(default_datawriter) == RETCODE_OK);

    //participant using non-default profile
    DataWriter* datawriter = publisher->create_datawriter_with_profile(topic, "test_publisher_profile");
    ASSERT_NE(datawriter, nullptr);
    check_datawriter_with_profile(datawriter, "test_publisher_profile");
    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);

    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(PublisherTests, CreateDataWriterWithProfileFromString)
{

    std::ifstream t("test_xml_for_string_profile.xml");
    std::stringstream buffer;
    buffer << t.rdbuf();

    DomainParticipantFactory::get_instance()->load_XML_profiles_string(buffer.str().c_str(), buffer.str().length());
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);

    //Datawriter using the default profile
    DataWriter* default_datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(default_datawriter, nullptr);
    check_datawriter_with_profile(default_datawriter, "test_default_publisher_profile_string");
    ASSERT_TRUE(publisher->delete_datawriter(default_datawriter) == RETCODE_OK);

    //participant using non-default profile
    DataWriter* datawriter = publisher->create_datawriter_with_profile(topic, "test_publisher_profile_string");
    ASSERT_NE(datawriter, nullptr);
    check_datawriter_with_profile(datawriter, "test_publisher_profile_string");
    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);

    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(PublisherTests, GetDataWriterProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profile.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Extract qos from profile
    DataWriterQos qos;
    EXPECT_EQ(
        publisher->get_datawriter_qos_from_profile("test_publisher_profile", qos),
        RETCODE_OK);

    //Datawriter using the extracted qos
    DataWriter* datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_NE(datawriter, nullptr);

    check_datawriter_with_profile(datawriter, "test_publisher_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        publisher->get_datawriter_qos_from_profile("incorrect_profile_name", qos),
        RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(publisher->delete_datawriter(datawriter), RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(PublisherTests, DeletePublisherWithWriters)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(publisher->delete_datawriter(datawriter), RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}


void set_listener_test (
        Publisher* publisher,
        PublisherListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(publisher->set_listener(listener, mask), RETCODE_OK);
    ASSERT_EQ(publisher->get_status_mask(), mask);
}

class CustomListener : public PublisherListener
{

};

TEST(PublisherTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, &listener);
    ASSERT_NE(publisher, nullptr);
    ASSERT_EQ(publisher->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<Publisher*, PublisherListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { publisher, &listener, StatusMask::liveliness_lost() },
        { publisher, &listener, StatusMask::offered_deadline_missed() },
        { publisher, &listener, StatusMask::offered_incompatible_qos() },
        { publisher, &listener, StatusMask::publication_matched() },
        //all except one
        { publisher, &listener, StatusMask::all() >> StatusMask::liveliness_lost() },
        { publisher, &listener, StatusMask::all() >> StatusMask::offered_deadline_missed() },
        { publisher, &listener, StatusMask::all() >> StatusMask::offered_incompatible_qos() },
        { publisher, &listener, StatusMask::all() >> StatusMask::publication_matched() },
        //all and none
        { publisher, &listener, StatusMask::all() },
        { publisher, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

// Delete contained entities test
TEST(Publisher, DeleteContainedEntities)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new LoanableTopicDataTypeMock());
    type.register_type(participant);

    Topic* topic_foo = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic_foo, nullptr);
    Topic* topic_bar = participant->create_topic("bartopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic_bar, nullptr);

    DataWriter* data_writer_foo = publisher->create_datawriter(topic_foo, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer_foo, nullptr);
    DataWriter* data_writer_bar = publisher->create_datawriter(topic_bar, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer_bar, nullptr);

    std::vector<DataWriter*> data_writer_list;
    publisher->get_datawriters(data_writer_list);
    ASSERT_EQ(data_writer_list.size(), 2u);

    data_writer_list.clear();
    void* loan_data;
    ASSERT_EQ(data_writer_bar->loan_sample(loan_data), RETCODE_OK);

    ASSERT_EQ(publisher->delete_contained_entities(), RETCODE_PRECONDITION_NOT_MET);
    publisher->get_datawriters(data_writer_list);
    ASSERT_EQ(data_writer_list.size(), 2u);

    data_writer_list.clear();
    data_writer_bar->discard_loan(loan_data);

    ASSERT_EQ(publisher->delete_contained_entities(), RETCODE_OK);
    publisher->get_datawriters(data_writer_list);
    ASSERT_FALSE(publisher->has_datawriters());
}

/*
 * This test checks that the Publisher methods defined in the standard not yet implemented in FastDDS return
 * RETCODE_UNSUPPORTED. The following methods are checked:
 * 1. suspend_publications
 * 2. resume_publications
 * 3. begin_coherent_changes
 * 4. end_coherent_changes
 */
TEST(PublisherTests, UnsupportedPublisherMethods)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    fastdds::dds::DataWriterQos writer_qos;
    fastdds::dds::TopicQos topic_qos;
    EXPECT_EQ(RETCODE_UNSUPPORTED, publisher->suspend_publications());
    EXPECT_EQ(RETCODE_UNSUPPORTED, publisher->resume_publications());
    EXPECT_EQ(RETCODE_UNSUPPORTED, publisher->begin_coherent_changes());
    EXPECT_EQ(RETCODE_UNSUPPORTED, publisher->end_coherent_changes());

    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

/**
 * Utility class to set some values other than default to those Qos common to Topic and DataWriter.
 *
 * This is a class instead of a free function to avoid linking with its TestsSubscriber counterpart.
 */
class TestsPublisherQosCommonUtils
{
public:

    template<typename T>
    static void set_common_qos(
            T& qos)
    {
        qos.durability_service().history_kind = KEEP_ALL_HISTORY_QOS;
        qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        qos.durability().kind = VOLATILE_DURABILITY_QOS;
        qos.deadline().period = {0, 500000000};
        qos.latency_budget().duration = 0;
        qos.liveliness().kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        qos.resource_limits().max_samples = 1000;
        qos.transport_priority().value = 1;
        qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
        qos.representation().m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
        qos.destination_order().kind = eprosima::fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        qos.history().kind = KEEP_ALL_HISTORY_QOS;
        qos.lifespan().duration = {5, 0};
    }

};

/*
 * This test:
 *   1. Creates a Topic with custom Qos
 *   2. Creates a control DataWriterQos in which the non-common Qos are set to a value different from the default
 *   3. Creates a test DataWriterQos and assigns it the value of the control Qos
 *   4. Updates the control Qos' common Qos with the same values used in the Topic Qos
 *   5. Calls Publisher::copy_from_topic_qos() with the test Qos and the Topic Qos
 *   6. Checks that the resulting test Qos has the same values as the control Qos
 */
TEST(PublisherTests, datawriter_copy_from_topic_qos)
{
    /* Set Topic Qos different from default */
    TopicQos topic_qos;
    TestsPublisherQosCommonUtils::set_common_qos(topic_qos);

    /* Create the publisher under test */
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    /* Create control and test Qos instances */
    // Override non-common Qos with values different from the default on the control Qos
    DataWriterQos control_qos;
    control_qos.ownership_strength().value = 1;
    control_qos.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    control_qos.writer_data_lifecycle().autodispose_unregistered_instances = false;
    control_qos.user_data().push_back(0);
    control_qos.endpoint().entity_id = 1;
    control_qos.writer_resource_limits().matched_subscriber_allocation =
            eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1u);
    control_qos.data_sharing().off();

    // Copy control Qos to test Qos. At this point, test_qos has non-default values for the non-common Qos,
    // and default values for the common Qos
    DataWriterQos test_qos = control_qos;

    // Set common Qos to the control Qos with the same values used in the Topic Qos
    TestsPublisherQosCommonUtils::set_common_qos(control_qos);

    /* Function under test call */
    publisher->copy_from_topic_qos(test_qos, topic_qos);

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
