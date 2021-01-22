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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <dds/core/types.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/topic/Topic.hpp>
#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/TopicBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>


namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::ParticipantAttributes;
using fastrtps::PublisherAttributes;
using fastrtps::SubscriberAttributes;
using fastrtps::types::DynamicData_ptr;
using fastrtps::types::DynamicDataFactory;
using fastrtps::types::DynamicType_ptr;
using fastrtps::types::DynamicTypeBuilder_ptr;
using fastrtps::types::DynamicTypeBuilderFactory;
using fastrtps::types::TypeDescriptor;
using fastrtps::xmlparser::XMLP_ret;
using fastrtps::xmlparser::XMLProfileManager;


// Mocked TopicDataType for Topic creation tests
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

    void clearName()
    {
        setName("");
    }

};

TEST(ParticipantTests, DomainParticipantFactoryGetInstance)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    ASSERT_NE(factory, nullptr);
    ASSERT_EQ(factory, DomainParticipantFactory::get_instance());
}

TEST(ParticipantTests, ChangeDomainParticipantFactoryQos)
{
    DomainParticipantFactoryQos qos;
    DomainParticipantFactory::get_instance()->get_qos(qos);

    ASSERT_EQ(qos.entity_factory().autoenable_created_entities, true);

    EntityFactoryQosPolicy entity_factory = qos.entity_factory();
    entity_factory.autoenable_created_entities = false;
    qos.entity_factory(entity_factory);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    DomainParticipantFactoryQos fqos;
    DomainParticipantFactory::get_instance()->get_qos(fqos);

    ASSERT_EQ(qos, fqos);
    ASSERT_EQ(fqos.entity_factory().autoenable_created_entities, false);
}

TEST(ParticipantTests, CreateDomainParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_NE(participant, nullptr);
    EXPECT_EQ(participant->get_listener(), nullptr);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

void check_participant_with_profile (
        DomainParticipant* participant,
        const std::string& profile_name)
{
    DomainParticipantQos qos;
    participant->get_qos(qos);

    ParticipantAttributes participant_atts;
    XMLProfileManager::fillParticipantAttributes(profile_name, participant_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.allocation() == participant_atts.rtps.allocation);
    ASSERT_TRUE(qos.properties() == participant_atts.rtps.properties);
    ASSERT_TRUE(qos.name().to_string() == participant_atts.rtps.getName());
    ASSERT_TRUE(qos.wire_protocol().prefix == participant_atts.rtps.prefix);
    ASSERT_TRUE(qos.wire_protocol().participant_id == participant_atts.rtps.participantID);
    ASSERT_TRUE(qos.wire_protocol().builtin == participant_atts.rtps.builtin);
    ASSERT_TRUE(qos.wire_protocol().port == participant_atts.rtps.port);
    ASSERT_TRUE(qos.wire_protocol().throughput_controller == participant_atts.rtps.throughputController);
    ASSERT_TRUE(qos.wire_protocol().default_unicast_locator_list ==
            participant_atts.rtps.defaultUnicastLocatorList);
    ASSERT_TRUE(qos.wire_protocol().default_multicast_locator_list ==
            participant_atts.rtps.defaultMulticastLocatorList);
    ASSERT_TRUE(qos.transport().user_transports == participant_atts.rtps.userTransports);
    ASSERT_TRUE(qos.transport().use_builtin_transports == participant_atts.rtps.useBuiltinTransports);
    ASSERT_TRUE(qos.transport().send_socket_buffer_size == participant_atts.rtps.sendSocketBufferSize);
    ASSERT_TRUE(qos.transport().listen_socket_buffer_size == participant_atts.rtps.listenSocketBufferSize);
    ASSERT_TRUE(qos.user_data().data_vec() == participant_atts.rtps.userData);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.entity_factory() == PARTICIPANT_QOS_DEFAULT.entity_factory());
}

TEST(ParticipantTests, CreateDomainParticipantWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");

    //participant using the default profile
    DomainParticipant* default_participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(default_participant, nullptr);
    ASSERT_EQ(default_participant->get_domain_id(), 0u); //Keep the DID given to the method, not the one on the profile
    check_participant_with_profile(default_participant, "test_default_participant_profile");
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(
                default_participant) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant_with_profile(0, "test_participant_profile");
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_domain_id(), 0u); //Keep the DID given to the method, not the one on the profile
    check_participant_with_profile(participant, "test_participant_profile");
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetParticipantProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipantQos qos;
    EXPECT_EQ(
        DomainParticipantFactory::get_instance()->get_participant_qos_from_profile("test_participant_profile", qos),
        ReturnCode_t::RETCODE_OK);

    // Extract ParticipantQos from profile
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, qos);
    ASSERT_NE(participant, nullptr);

    check_participant_with_profile(participant, "test_participant_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        DomainParticipantFactory::get_instance()->get_participant_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


TEST(ParticipantTests, CreatePSMDomainParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::core::null;
    participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_NE(participant, ::dds::core::null);
}

TEST(ParticipantTests, DeleteDomainParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteDomainParticipantWithEntities)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(
                participant), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);

    participant = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(
                participant), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);

    participant = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(
                participant), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangeDefaultParticipantQos)
{
    DomainParticipantQos qos;
    DomainParticipantFactory::get_instance()->get_default_participant_qos(qos);

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    EntityFactoryQosPolicy entity_factory = qos.entity_factory();
    entity_factory.autoenable_created_entities = false;
    qos.entity_factory(entity_factory);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_default_participant_qos(qos) == ReturnCode_t::RETCODE_OK);
    DomainParticipantQos pqos;
    DomainParticipantFactory::get_instance()->get_default_participant_qos(pqos);

    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_default_participant_qos(
                PARTICIPANT_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultParticipantQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ::dds::domain::qos::DomainParticipantQos qos = participant.default_participant_qos();

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    EntityFactoryQosPolicy entity_factory = qos.entity_factory();
    entity_factory.autoenable_created_entities = false;
    qos.entity_factory(entity_factory);

    ASSERT_NO_THROW(participant.default_participant_qos(qos));
    ::dds::domain::qos::DomainParticipantQos pqos = participant.default_participant_qos();

    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_NO_THROW(participant.default_participant_qos(PARTICIPANT_QOS_DEFAULT));
}

TEST(ParticipantTests, ChangeDomainParticipantQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipantQos qos;
    participant->get_qos(qos);

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_TRUE(participant->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    DomainParticipantQos pqos;
    participant->get_qos(pqos);

    ASSERT_FALSE(pqos == PARTICIPANT_QOS_DEFAULT);
    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(qos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

TEST(ParticipantTests, ChangePSMDomainParticipantQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::core::null;
    participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::domain::qos::DomainParticipantQos qos = participant.qos();

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_NO_THROW(participant.qos(qos));
    ::dds::domain::qos::DomainParticipantQos pqos;
    pqos = participant.qos();

    ASSERT_FALSE(pqos == PARTICIPANT_QOS_DEFAULT);
    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(qos.entity_factory().autoenable_created_entities, false);

}

TEST(ParticipantTests, EntityFactoryBehavior)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    {
        DomainParticipantFactoryQos qos;
        qos.entity_factory().autoenable_created_entities = false;

        ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    }

    // Ensure that participant is created disabled.
    DomainParticipant* participant = factory->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    // Participant is disabled. This means we can change an inmutable qos.
    DomainParticipantQos qos = PARTICIPANT_QOS_DEFAULT;
    qos.wire_protocol().builtin.avoid_builtin_multicast = !qos.wire_protocol().builtin.avoid_builtin_multicast;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->set_qos(qos));

    // Creating lower entities should create them disabled
    Publisher* pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, pub);
    EXPECT_FALSE(pub->is_enabled());

    Subscriber* sub = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, sub);
    EXPECT_FALSE(sub->is_enabled());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    EXPECT_FALSE(topic->is_enabled());

    // Enabling should fail on lower entities until participant is enabled
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, pub->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, sub->enable());

    // Enable participant and check idempotency of enable
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());

    // As the participant was created with the default value for ENTITY_FACTORY,
    // lower entities should have been automatically enabled.
    EXPECT_TRUE(pub->is_enabled());
    EXPECT_TRUE(sub->is_enabled());

    // Now that participant is enabled, we should not be able change an inmutable qos.
    ASSERT_EQ(ReturnCode_t::RETCODE_IMMUTABLE_POLICY, participant->set_qos(PARTICIPANT_QOS_DEFAULT));

    // Created entities should now be automatically enabled
    Subscriber* sub2 = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, sub2);
    EXPECT_TRUE(sub2->is_enabled());

    // We can change ENTITY_FACTORY on the participant
    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->set_qos(qos));

    // Lower entities should now be created disabled
    Publisher* pub2 = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, pub2);
    EXPECT_FALSE(pub2->is_enabled());

    // But could be enabled afterwards
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, pub2->enable());
    EXPECT_TRUE(pub2->is_enabled());

    // Check idempotency of enable on entities
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, pub->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, pub2->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, sub->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, sub2->enable());

    // Delete lower entities
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(sub2));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(pub2));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(sub));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(pub));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_topic(topic));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant));
}

TEST(ParticipantTests, CreatePublisher)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    ASSERT_NE(publisher, nullptr);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

void check_publisher_with_profile (
        Publisher* publisher,
        const std::string& profile_name)
{
    PublisherQos qos;
    publisher->get_qos(qos);

    PublisherAttributes publisher_atts;
    XMLProfileManager::fillPublisherAttributes(profile_name, publisher_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.group_data().dataVec() == publisher_atts.qos.m_groupData.dataVec());
    ASSERT_TRUE(qos.partition() == publisher_atts.qos.m_partition);
    ASSERT_TRUE(qos.presentation() == publisher_atts.qos.m_presentation);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.entity_factory() == PUBLISHER_QOS_DEFAULT.entity_factory());
}

TEST(ParticipantTests, CreatePublisherWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    //publisher using the default profile
    Publisher* default_publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(default_publisher, nullptr);
    check_publisher_with_profile(default_publisher, "test_default_publisher_profile");
    ASSERT_TRUE(participant->delete_publisher(default_publisher) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    Publisher* publisher = participant->create_publisher_with_profile("test_publisher_profile");
    ASSERT_NE(publisher, nullptr);
    check_publisher_with_profile(publisher, "test_publisher_profile");
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreatePSMPublisher)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::core::null;
    publisher = ::dds::pub::Publisher(participant);

    ASSERT_NE(publisher, ::dds::core::null);
}

TEST(ParticipantTests, ChangeDefaultPublisherQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_TRUE(participant->set_default_publisher_qos(PUBLISHER_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);

    PublisherQos qos;
    ASSERT_TRUE(participant->get_default_publisher_qos(qos) == ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_TRUE(participant->set_default_publisher_qos(qos) == ReturnCode_t::RETCODE_OK);

    PublisherQos pqos;
    ASSERT_TRUE(participant->get_default_publisher_qos(pqos) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultPublisherQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::qos::PublisherQos qos = participant.default_publisher_qos();
    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_NO_THROW(participant.default_publisher_qos(qos));

    ::dds::pub::qos::PublisherQos pqos = participant.default_publisher_qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
}

TEST(ParticipantTests, CreateSubscriber)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

void check_subscriber_with_profile (
        Subscriber* subscriber,
        const std::string& profile_name)
{
    SubscriberQos qos;
    subscriber->get_qos(qos);

    SubscriberAttributes subscriber_atts;
    XMLProfileManager::fillSubscriberAttributes(profile_name, subscriber_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.group_data().dataVec() == subscriber_atts.qos.m_groupData.dataVec());
    ASSERT_TRUE(qos.partition() == subscriber_atts.qos.m_partition);
    ASSERT_TRUE(qos.presentation() == subscriber_atts.qos.m_presentation);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.entity_factory() == SUBSCRIBER_QOS_DEFAULT.entity_factory());
}

TEST(ParticipantTests, GetSubscriberProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Extract qos from profile
    SubscriberQos qos;
    EXPECT_EQ(
        participant->get_subscriber_qos_from_profile("test_subscriber_profile", qos),
        ReturnCode_t::RETCODE_OK);

    Subscriber* subscriber = participant->create_subscriber(qos);
    ASSERT_NE(subscriber, nullptr);

    check_subscriber_with_profile(subscriber, "test_subscriber_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        participant->get_subscriber_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreateSubscriberWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    //subscriber using the default profile
    Subscriber* default_subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(default_subscriber, nullptr);
    check_subscriber_with_profile(default_subscriber, "test_default_subscriber_profile");
    ASSERT_TRUE(participant->delete_subscriber(default_subscriber) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    Subscriber* subscriber = participant->create_subscriber_with_profile("test_subscriber_profile");
    ASSERT_NE(subscriber, nullptr);
    check_subscriber_with_profile(subscriber, "test_subscriber_profile");
    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetPublisherProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Extract qos from profile
    PublisherQos qos;
    EXPECT_EQ(
        participant->get_publisher_qos_from_profile("test_publisher_profile", qos),
        ReturnCode_t::RETCODE_OK);

    Publisher* publisher = participant->create_publisher(qos);
    ASSERT_NE(publisher, nullptr);

    check_publisher_with_profile(publisher, "test_publisher_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        participant->get_publisher_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


TEST(ParticipantTests, CreatePSMSubscriber)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::Subscriber subscriber = ::dds::core::null;
    subscriber = ::dds::sub::Subscriber(participant, SUBSCRIBER_QOS_DEFAULT);

    ASSERT_NE(subscriber, ::dds::core::null);
}

TEST(ParticipantTests, DeletePublisher)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteSubscriber)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangeDefaultSubscriberQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_EQ(participant->set_default_subscriber_qos(SUBSCRIBER_QOS_DEFAULT), ReturnCode_t::RETCODE_OK);

    SubscriberQos qos;
    ASSERT_EQ(participant->get_default_subscriber_qos(qos), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_EQ(participant->set_default_subscriber_qos(qos), ReturnCode_t::RETCODE_OK);

    SubscriberQos pqos;
    ASSERT_EQ(participant->get_default_subscriber_qos(pqos), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(pqos == qos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultSubscriberQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::qos::SubscriberQos qos = participant.default_subscriber_qos();
    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_NO_THROW(participant.default_subscriber_qos(qos));

    ::dds::sub::qos::SubscriberQos pqos = participant.default_subscriber_qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
}

TEST(ParticipantTests, ChangeDefaultTopicQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_TRUE(participant->set_default_topic_qos(TOPIC_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);

    TopicQos qos;
    participant->get_default_topic_qos(qos);

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    ASSERT_TRUE(participant->set_default_topic_qos(qos) == ReturnCode_t::RETCODE_OK);

    TopicQos tqos;
    participant->get_default_topic_qos(tqos);

    ASSERT_EQ(qos, tqos);
    ASSERT_EQ(tqos.reliability().kind, BEST_EFFORT_RELIABILITY_QOS);

    qos.durability().kind = PERSISTENT_DURABILITY_QOS;
    ASSERT_FALSE(participant->set_default_topic_qos(qos) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultTopicQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::topic::qos::TopicQos qos = participant.default_topic_qos();

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;

    ASSERT_NO_THROW(participant.default_topic_qos(qos));

    ::dds::topic::qos::TopicQos tqos = participant.default_topic_qos();
    ASSERT_EQ(qos, tqos);
    ASSERT_EQ(tqos.ownership().kind, EXCLUSIVE_OWNERSHIP_QOS);
}

void check_topic_with_profile (
        Topic* topic,
        const std::string& profile_name)
{
    TopicQos qos;
    topic->get_qos(qos);

    TopicAttributesQos topic_atts;
    XMLProfileManager::fillTopicAttributes(profile_name, topic_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.history() == topic_atts.historyQos);
    ASSERT_TRUE(qos.resource_limits() == topic_atts.resourceLimitsQos);
}

TEST(ParticipantTests, GetTopicProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // Extract qos from profile
    TopicQos qos;
    EXPECT_EQ(
        participant->get_topic_qos_from_profile("test_topic_profile", qos),
        ReturnCode_t::RETCODE_OK);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), qos);
    ASSERT_NE(topic, nullptr);


    check_topic_with_profile(topic, "test_topic_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        participant->get_topic_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreateTopic)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");

    // Topic using the default profile
    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Try to create the same topic twice
    Topic* topic_duplicated = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);
    ASSERT_EQ(topic_duplicated, nullptr);

    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);

    // Topic using non-default profile
    Topic* topic_profile = participant->create_topic_with_profile("footopic", "footype", "test_topic_profile");
    ASSERT_NE(topic_profile, nullptr);
    check_topic_with_profile(topic_profile, "test_topic_profile");
    ASSERT_TRUE(participant->delete_topic(topic_profile) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, PSMCreateTopic)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant.delegate().get(), "footype");

    ::dds::topic::Topic topic = ::dds::core::null;
    topic = ::dds::topic::Topic(participant, "footopic", "footype", TOPIC_QOS_DEFAULT);

    ASSERT_NE(topic, ::dds::core::null);
}

TEST(ParticipantTests, DeleteTopic)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipant* participant2 =
            DomainParticipantFactory::get_instance()->create_participant(1, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");

    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    ASSERT_TRUE(participant->delete_topic(nullptr) == ReturnCode_t::RETCODE_BAD_PARAMETER);
    ASSERT_TRUE(participant2->delete_topic(topic) == ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant2) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, LookupTopicDescription)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    const std::string topic_name("footopic");

    // Topic not created yet. Should return nil
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), nullptr);

    // After topic creation ...
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");
    Topic* topic = participant->create_topic(topic_name, "footype", TOPIC_QOS_DEFAULT);
    EXPECT_NE(topic, nullptr);

    // ... the topic should be returned.
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), topic);

    // After topic deletion, should return nil
    EXPECT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), nullptr);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteTopicInUse)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");

    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    DataWriter* data_writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(publisher->delete_datawriter(data_writer), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


void set_listener_test (
        DomainParticipant* participant,
        DomainParticipantListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(participant->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->get_status_mask(), mask);
}

class CustomListener : public DomainParticipantListener
{

};

TEST(ParticipantTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT, &listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<DomainParticipant*, DomainParticipantListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { participant, &listener, StatusMask::liveliness_lost() },
        { participant, &listener, StatusMask::offered_deadline_missed() },
        { participant, &listener, StatusMask::offered_incompatible_qos() },
        { participant, &listener, StatusMask::publication_matched() },
        { participant, &listener, StatusMask::data_on_readers() },
        { participant, &listener, StatusMask::data_available() },
        { participant, &listener, StatusMask::sample_rejected() },
        { participant, &listener, StatusMask::liveliness_changed() },
        { participant, &listener, StatusMask::requested_deadline_missed() },
        { participant, &listener, StatusMask::requested_incompatible_qos() },
        { participant, &listener, StatusMask::subscription_matched() },
        { participant, &listener, StatusMask::sample_lost() },
        //all except one
        { participant, &listener, StatusMask::all() >> StatusMask::liveliness_lost() },
        { participant, &listener, StatusMask::all() >> StatusMask::offered_deadline_missed() },
        { participant, &listener, StatusMask::all() >> StatusMask::offered_incompatible_qos() },
        { participant, &listener, StatusMask::all() >> StatusMask::publication_matched() },
        { participant, &listener, StatusMask::all() >> StatusMask::data_on_readers() },
        { participant, &listener, StatusMask::all() >> StatusMask::data_available() },
        { participant, &listener, StatusMask::all() >> StatusMask::sample_rejected() },
        { participant, &listener, StatusMask::all() >> StatusMask::liveliness_changed() },
        { participant, &listener, StatusMask::all() >> StatusMask::requested_deadline_missed() },
        { participant, &listener, StatusMask::all() >> StatusMask::requested_incompatible_qos() },
        { participant, &listener, StatusMask::all() >> StatusMask::subscription_matched() },
        { participant, &listener, StatusMask::all() >> StatusMask::sample_lost() },
        //all and none
        { participant, &listener, StatusMask::all() },
        { participant, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the negative cases of the check_qos() function.
 * 1. User data is set to be a 5-element size octet vector.
 * 2. The participant's qos are set to save these the user data.
 * 3. Change the ParticipantResourceLimitsQos to a maximum user data value less than the current user data size.
 * 4. Check that the previous operation returns an Inconsistent Policy error code
 */
TEST(ParticipantTests, CheckDomainParticipantQos)
{
    // Create the participant factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    // Disable automatic entity enablement on the participant
    {
        DomainParticipantFactoryQos qos;
        qos.entity_factory().autoenable_created_entities = false;

        ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    }

    // Create the participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Get the participant qos
    DomainParticipantQos qos;
    ASSERT_TRUE(participant->get_qos(qos) == ReturnCode_t::RETCODE_OK);

    // Change the user data
    qos.user_data().set_max_size(5);
    std::vector<eprosima::fastrtps::rtps::octet> my_data {0, 1, 2, 3, 4};
    qos.user_data().setValue(my_data);
    ASSERT_TRUE(participant->set_qos(qos) == ReturnCode_t::RETCODE_OK);

    // Change the ParticipantResourceLimitsQos to a maximum user data value less than the current user data size
    // This should return an Inconsistent Policy error code
    qos.allocation().data_limits.max_user_data = 1;
    ASSERT_EQ(qos.allocation().data_limits.max_user_data, 1ul);
    ASSERT_TRUE(participant->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    // Enable the participant
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());

    // Remove the participant
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the cases in which the allocation QoS is modified.
 * 1. Check that the qos is modified if the participant is not enabled.
 * 2. Check that the qos is not changed and it generates an error code if the participant is already enabled.
 */
TEST(ParticipantTests, ChangeAllocationDomainParticipantQos)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipantFactoryQos pfqos;
    pfqos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(factory->set_qos(pfqos), ReturnCode_t::RETCODE_OK);

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_FALSE(participant->is_enabled());
    DomainParticipantQos qos;
    participant->get_qos(qos);

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    qos.allocation().data_limits.max_properties = 10;
    ASSERT_EQ(participant->set_qos(qos), ReturnCode_t::RETCODE_OK);
    DomainParticipantQos pqos;
    participant->get_qos(pqos);

    ASSERT_FALSE(pqos == PARTICIPANT_QOS_DEFAULT);
    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(pqos.allocation().data_limits.max_properties, 10ul);

    participant->enable();
    ASSERT_TRUE(participant->is_enabled());
    participant->get_qos(pqos);
    pqos.allocation().data_limits.max_properties = 20;
    ASSERT_EQ(participant->set_qos(pqos), ReturnCode_t::RETCODE_IMMUTABLE_POLICY);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the cases in which the participant name is modified.
 * 1. Check that the name is modified if the participant is not enabled.
 * 2. Check that the name is not changed and it generates an error code if the participant is already enabled.
 */
TEST(ParticipantTests, ChangeDomainParcipantName)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipantFactoryQos pfqos;
    pfqos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(factory->set_qos(pfqos), ReturnCode_t::RETCODE_OK);

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_FALSE(participant->is_enabled());
    DomainParticipantQos qos;
    participant->get_qos(qos);

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    qos.name() = "part1";
    ASSERT_EQ(participant->set_qos(qos), ReturnCode_t::RETCODE_OK);
    DomainParticipantQos pqos;
    participant->get_qos(pqos);

    ASSERT_FALSE(pqos == PARTICIPANT_QOS_DEFAULT);
    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(pqos.name(), "part1");

    participant->enable();
    ASSERT_TRUE(participant->is_enabled());
    participant->get_qos(pqos);
    pqos.name() = "new_part1";
    ASSERT_EQ(participant->set_qos(pqos), ReturnCode_t::RETCODE_IMMUTABLE_POLICY);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the scenarios in which an error is given at trying to delete the publisher and subscriber entites.
 */
TEST(ParticipantTests, DeleteEntitiesNegativeClauses)
{
    // Create two participants
    DomainParticipant* participant_1 =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipant* participant_2 =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create a subscriber in the first participant
    Subscriber* subscriber_1 = participant_1->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber_1, nullptr);
    // Try to delete this subscriber using the second partipant. This should return a RETCODE_PRECONDITION_NOT_MET
    // error code as this subscriber does not belong to the second participant
    ASSERT_EQ(participant_2->delete_subscriber(subscriber_1), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant_1->delete_subscriber(subscriber_1), ReturnCode_t::RETCODE_OK);

    // Create a publisher in the first participant
    Publisher* publisher_1 = participant_1->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher_1, nullptr);
    // Try to delete this publisher using the second partipant. This should return a RETCODE_PRECONDITION_NOT_MET
    // error code as this publisher does not belong to the second participant
    ASSERT_EQ(participant_2->delete_publisher(publisher_1), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant_1->delete_publisher(publisher_1), ReturnCode_t::RETCODE_OK);

    // Remove both participants
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant_1), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant_2), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that the participant's child entities are not created if an empty profile if provided for these
 * entities.
 */
TEST(ParticipantTests, CreateEntitiesWithProfileNegativeClauses)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create publisher with an empty profile should return nullptr
    Publisher* publisher = participant->create_publisher_with_profile("");
    ASSERT_EQ(publisher, nullptr);

    // Create subscriber with an empty profile should return nullptr
    Subscriber* subscriber = participant->create_subscriber_with_profile("");
    ASSERT_EQ(subscriber, nullptr);

    // Create topic with an empty profile should return nullptr
    Topic* topic = participant->create_topic_with_profile("footopic", "footype", "");
    ASSERT_EQ(topic, nullptr);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that an error is given when registering a TypeSupport with an empty name in the TopicDataType.
 */
TEST(ParticipantTests, RegisterTypeNegativeClauses)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the TopicDataType and delete the topic data type name
    TopicDataTypeMock* data_type = new TopicDataTypeMock();
    data_type->clearName();

    // Create the TypeSupport with the TopicDataType with an empty name
    TypeSupport type(data_type);
    // Register the type shoul return a RETCODE_BAD_PARAMETER return code
    EXPECT_EQ(type.register_type(participant), ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that an error is given when trying to assert the liveliness.
 * 1. Check that an error is given at trying to assert the livelines from a non enabled participant.
 * 2. Check that an error is given at trying to assert the livelines from a participant with a disabled
 *    Writer Liveliness Protocol (WLP writer is not defined).
 */
TEST(ParticipantTests, AssertLivelinesNegativeClauses)
{
    // Do not enable entities on creation
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipantFactoryQos qos;
    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(factory->set_qos(qos), ReturnCode_t::RETCODE_OK);

    // Create a disabled participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    // Assert liveliness from a disabled participant should return a RETCODE_NOT_ENABLED return code.
    ASSERT_EQ(participant->assert_liveliness(), ReturnCode_t::RETCODE_NOT_ENABLED);

    // Change the participant QoS to disable the Writer Liveliness Protocol
    DomainParticipantQos pqos;
    ASSERT_EQ(participant->get_qos(pqos), ReturnCode_t::RETCODE_OK);
    pqos.wire_protocol().builtin.use_WriterLivelinessProtocol = false;
    ASSERT_EQ(participant->set_qos(pqos), ReturnCode_t::RETCODE_OK);

    // Enable the participant
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());
    // Check that an error is given at trying to assert the livelines from a participant with a disabled
    // Writer Liveliness Protocol (WLP writer is not defined).
    ASSERT_EQ(participant->assert_liveliness(), ReturnCode_t::RETCODE_ERROR);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test check the get_current_time public member function of the DomainParticipant.
 */
TEST(ParticipantTests, GetCurrentTime)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    eprosima::fastrtps::Time_t now;
    ASSERT_EQ(participant->get_current_time(now), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that a constant pointer to the DomainParticipant is returned when calling the get_participant()
 * function from a publisher of this participant.
 */
TEST(ParticipantTests, GetParticipantConst)
{
    // Create the participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the publisher
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    // Call the get_participant() Publisher member function
    const DomainParticipant* participant_pub = publisher->get_participant();

    // Check that the GUIDs of the created DomainParticipant and the returned one match.
    ASSERT_EQ(participant_pub->guid(), participant->guid());

    // Remove the publisher and the participant
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


/*
 * This test checks the get_participant_names() DomainParticipant member function.
 * 1. Check that the participant name is empty if the participant is not enabled.
 * 2. Check that the participant name is filled when the participant is enabled.
 */
TEST(ParticipantTests, GetParticipantNames)
{
    // Do not enable entities on creation
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipantFactoryQos qos;
    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(factory->set_qos(qos), ReturnCode_t::RETCODE_OK);

    // Create a disabled participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    // Check that the participant name is empty if the participant is not enabled
    std::vector<std::string> participant_names = participant->get_participant_names();
    ASSERT_TRUE(participant_names.empty());

    // Enable the participant
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());

    // Check that the participant name is filled when the participant is enabled
    participant_names = participant->get_participant_names();
    ASSERT_FALSE(participant_names.empty());

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that a topic is not created with a wrong settings.
 * 1. Check that the topic is not created if a wrong type name is provided.
 * 2. Check that the topic is not created if a non supported durability QoS is provided.
 */
TEST(ParticipantTests, CreateTopicNegativeClauses)
{
    // Create the participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Register the type
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // Check that the topic is not created if a wrong type name is provided
    Topic* topic;
    topic = participant->create_topic("footopic", "fake_type_name", TOPIC_QOS_DEFAULT);
    ASSERT_EQ(topic, nullptr);

    // Check that the topic is not created if a non supported durability QoS is provided
    TopicQos tqos;
    participant->get_default_topic_qos(tqos);
    tqos.durability().kind = PERSISTENT_DURABILITY_QOS;
    topic = participant->create_topic("footopic", type.get_type_name(), tqos);
    ASSERT_EQ(topic, nullptr);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the contais_entity() DomainParticipant member function.
 * 1. Check that the participant contains an already created topic in this participant.
 * 2. Check that the participant contains an already created publisher in this participant.
 * 3. Check that the participant contains an already created subscriber in this participant.
 * 4. Check that the participant contains an already created data_writer in this participant.
 * 5. Check that the participant contains an already created data_reader in this participant.
 * 6. Check that the participant does not contains a removed publisher.
 */
TEST(ParticipantTests, ContainsEntity)
{
    // Create the participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // Create the topic
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t topic_ihandle = topic->get_instance_handle();
    // Check that the participant contains an already created topic in this participant
    ASSERT_TRUE(participant->contains_entity(topic_ihandle, false));

    // Create the publisher
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t pub_ihandle = publisher->get_instance_handle();
    // Check that the participant contains an already created publisher in this participant
    ASSERT_TRUE(participant->contains_entity(pub_ihandle, false));

    // Create the subscriber
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t sub_ihandle = subscriber->get_instance_handle();
    // Check that the participant contains an already created subscriber in this participant
    ASSERT_TRUE(participant->contains_entity(sub_ihandle, false));

    // Create the data_writer
    DataWriter* data_writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t data_writer_ihandle = data_writer->get_instance_handle();
    // Check that the participant contains an already created data_writer in this participant
    ASSERT_TRUE(participant->contains_entity(data_writer_ihandle, true));

    // Create the data_reader
    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t data_reader_ihandle = data_reader->get_instance_handle();
    // Check that the participant contains an already created data_reader in this participant
    ASSERT_TRUE(participant->contains_entity(data_reader_ihandle, true));

    // Remove data_writer, data_reader, publisher, subscriber and topic entities.
    ASSERT_EQ(publisher->delete_datawriter(data_writer), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    // Check that the participant does not contains a removed publisher
    ASSERT_FALSE(participant->contains_entity(pub_ihandle, false));

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the unregister_type() DomainParticipant member function.
 * 1. Check that an error is given at trying to unregister a type with an empty name.
 * 2. Check that no error is given at trying to unregister a non registered type.
 * 3. Check that an error is given at trying to unregister a type that is been used by a data_reader/data_writer.
 * 4. Check that no errors result when an unused topic is unregistered.
 */
TEST(ParticipantTests, UnregisterType)
{
    // Create the participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Check that an error is given at trying to unregister a type with an empty name
    ASSERT_EQ(participant->unregister_type(""), ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Check that no error is given at trying to unregister a non registered type
    ASSERT_EQ(participant->unregister_type("missing_type"), ReturnCode_t::RETCODE_OK);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Create the subscriber and a data_reader that use the above topic
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);
    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);
    // Check that an error is given at trying to unregister a type that is been used by a data_reader
    ASSERT_EQ(participant->unregister_type(type.get_type_name()), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    // Create the publisher and a data_writer that use the above topic
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);
    DataWriter* data_writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer, nullptr);
    // Check that an error is given at trying to unregister a type that is been used by a data_writer
    ASSERT_EQ(participant->unregister_type(type.get_type_name()), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(publisher->delete_datawriter(data_writer), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    // At this point, the type is not been used by any entity.
    // Check that no errors result when an unused topic is unregistered
    ASSERT_EQ(participant->unregister_type(type.get_type_name()), ReturnCode_t::RETCODE_OK);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the negative clauses of new_remote_endpoint_discovered() DomainParticipant memeber function
 * used in the STATIC discovery.
 * 1. Check that the remote endpoint is not registered in a disabled participant.
 * 2. Check that a remote WRITER endpoint is not registered in an enabled participant if the discovery protocol is
 *    SIMPLE.
 * 3. Check that a remote READER endpoint is not registered in an enabled participant if the discovery protocol is
 *    SIMPLE.
 */
TEST(ParticipantTests, NewRemoteEndpointDiscovered)
{
    // Do not enable entities on creation
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipantFactoryQos qos;
    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);

    // Create a disabled participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    eprosima::fastrtps::rtps::GUID_t remote_endpoint_guid;
    std::istringstream("72.61.75.6c.5f.73.61.6e.63.68.65.7a") >> remote_endpoint_guid;

    // Check that the remote endpoint is not registered in a disabled participant
    ASSERT_FALSE(participant->new_remote_endpoint_discovered(
                remote_endpoint_guid, 1, eprosima::fastrtps::rtps::EndpointKind_t::WRITER));

    // Enable the participant
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    ASSERT_TRUE(participant->is_enabled());

    // Check that a WRITER remote endpoint is not registered in an enabled participant
    ASSERT_FALSE(participant->new_remote_endpoint_discovered(
                remote_endpoint_guid, 1, eprosima::fastrtps::rtps::EndpointKind_t::WRITER));
    // Check that a READER remote endpoint is not registered in an enabled participant
    ASSERT_FALSE(participant->new_remote_endpoint_discovered(
                remote_endpoint_guid, 1, eprosima::fastrtps::rtps::EndpointKind_t::READER));

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the set_qos() DomainParticipant member function for the PropertyPolicyQos and TransportConfigQos.
 */
TEST(ParticipantTests, SetDomainParticipantQos)
{
    // Create the DomainParticipantQos object
    DomainParticipantQos pqos;
    // Change in the DomainParticipantQos object the persistence guid property
    pqos.properties().properties().emplace_back("dds.persistence.guid", "72.61.75.6c.5f.73.61.6e.63.68.65.7a");
    // Change in the DomainParticipantQos object the listening socket buffer size setting of the transport qos
    pqos.transport().listen_socket_buffer_size = 262144;
    // Set the modified participant qos as the default qos
    ASSERT_EQ(DomainParticipantFactory::get_instance()->set_default_participant_qos(pqos), ReturnCode_t::RETCODE_OK);

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    DomainParticipantQos qos;
    participant->get_qos(qos);

    // Check that the participant QoS are the modified qos
    const std::string* persistence_property_old =
            eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(pqos.properties(), "dds.persistence.guid");
    ASSERT_NE(persistence_property_old, nullptr);
    eprosima::fastrtps::rtps::GUID_t persistence_guid_old;
    std::istringstream(persistence_property_old->c_str()) >> persistence_guid_old;
    const std::string* persistence_property_new =
            eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(qos.properties(), "dds.persistence.guid");
    ASSERT_NE(persistence_property_new, nullptr);
    eprosima::fastrtps::rtps::GUID_t persistence_guid_new;
    std::istringstream(persistence_property_old->c_str()) >> persistence_guid_new;
    ASSERT_EQ(persistence_guid_new, persistence_guid_old);

    ASSERT_EQ(qos.transport().listen_socket_buffer_size, pqos.transport().listen_socket_buffer_size);

    // Remove the participant
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that the PropertyPolicyQos and TransportConfigQos are immutable policy qos, i.e. these can not be
 * changed in an enabled participant
 */
TEST(ParticipantTests, UpdatableDomainParticipantQos)
{

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    DomainParticipantQos pqos;

    // Check that the PropertyPolicyQos can not be changed in an enabled participant
    participant->get_qos(pqos);
    pqos.properties().properties().emplace_back("dds.persistence.guid", "72.61.75.6c.5f.73.61.6e.63.68.65.7a");
    ASSERT_EQ(participant->set_qos(pqos), ReturnCode_t::RETCODE_IMMUTABLE_POLICY);

    // Check that the TransportConfigQos can not be changed in an enabled participant
    participant->get_qos(pqos);
    pqos.transport().listen_socket_buffer_size = 262144;
    ASSERT_EQ(participant->set_qos(pqos), ReturnCode_t::RETCODE_IMMUTABLE_POLICY);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test adds a complete dynamic type to the participant dynamic type factories
 */
TEST(ParticipantTests, RegisterDynamicTypeToFactories)
{
    // Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    // Create a disabled participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the dynamic type builder
    DynamicType_ptr base_type = DynamicTypeBuilderFactory::get_instance()->create_uint32_type();
    DynamicTypeBuilder_ptr builder = DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
    builder->add_member(0, "uint", base_type);
    // Build the complete dynamic type
    DynamicType_ptr dyn_type = builder->build();
    // Create the data instance
    DynamicData_ptr data(DynamicDataFactory::get_instance()->create_data(dyn_type));
    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    // Activating the auto_fill_type_information or the auto_fill_type_object settings, the participant will try to
    // add the type dynamic type factories
    type->auto_fill_type_information(true);
    type->auto_fill_type_object(true);
    ASSERT_EQ(type.register_type(participant), ReturnCode_t::RETCODE_OK);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test adds a complete dynamic type to the participant dynamic type factories without enabling the
 * auto_fill_type_information setting
 */
TEST(ParticipantTests, RegisterDynamicTypeToFactoriesNotFillTypeInfo)
{
    // Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    // Create a disabled participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the dynamic type builder
    DynamicType_ptr base_type = DynamicTypeBuilderFactory::get_instance()->create_uint32_type();
    DynamicTypeBuilder_ptr builder = DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
    builder->add_member(0, "uint", base_type);

    // Build the complete dynamic type
    DynamicType_ptr dyn_type = builder->build();
    // Create the data instance
    DynamicData_ptr data(DynamicDataFactory::get_instance()->create_data(dyn_type));

    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    type->auto_fill_type_information(false);
    type->auto_fill_type_object(true);
    ASSERT_EQ(type.register_type(participant), ReturnCode_t::RETCODE_OK);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

// Mocked DynamicType for DynamicType creation tests
class DynamicTypeMock : public eprosima::fastrtps::types::DynamicType
{
public:

    DynamicTypeMock(
            const eprosima::fastrtps::types::TypeDescriptor* descriptor)
        : eprosima::fastrtps::types::DynamicType(descriptor)
    {
    }

    DynamicType_ptr get_base_type_wrapper() const
    {
        return get_base_type();
    }

};

/*
 * This test attempts to add a non supported custom dynamic type to the participant dynamic type factories. The type
 * should be registered in the participant but not added to the dynamic types factories.
 */
TEST(ParticipantTests, RegisterDynamicTypeToFactoriesNotTypeIdentifier)
{
    // Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    // Create a disabled participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create a not supported TypeDescriptor
    const TypeDescriptor* myDescriptor = new TypeDescriptor("my_descriptor", 0x11);
    // Create the base type for the dynamic type
    DynamicType_ptr base_type(new DynamicTypeMock(myDescriptor));
    // Create a custom dynamic type builder using the wrong TypeDescriptor
    DynamicTypeBuilder_ptr builder =
            DynamicTypeBuilderFactory::get_instance()->create_custom_builder(myDescriptor, "my_dynamic_type");
    builder->add_member(0, "uint", base_type);
    // Create the dynamic type
    DynamicType_ptr dyn_type = builder->build();
    // Create the data instance
    DynamicData_ptr data(DynamicDataFactory::get_instance()->create_data(dyn_type));

    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    type->auto_fill_type_information(true);
    type->auto_fill_type_object(true);
    type.register_type(participant);

    TypeSupport ret_type = participant->find_type("my_dynamic_type");

    // The type is registered in the participant but not in the dynamic types factories
    ASSERT_FALSE(ret_type.empty());

    // Remove TypeDescriptor before closing
    delete myDescriptor;

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test create a sequence of TypeIdentifiers to call the get_types() DomainParticipant function. It should return
 * the TypeObjects associated with the TypeIdentifiers. Finally, the test checks that the writer guid prefix given by
 * the TypeObject is the same as the DomainPartipant guid prefix.
 */
TEST(ParticipantTests, GetTypes)
{
    // Create the participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_client = true;
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    // Create the dynamic type builder
    DynamicTypeBuilder_ptr builder_string = DynamicTypeBuilderFactory::get_instance()->create_string_builder(100);
    // Create the dynamic type
    DynamicType_ptr dyn_type_string = builder_string->build();
    TypeSupport type_string(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type_string));
    // Create the data instance
    DynamicData_ptr data_string(DynamicDataFactory::get_instance()->create_data(dyn_type_string));
    data_string->set_string_value("Dynamic String");

    // Register the type
    type_string->auto_fill_type_information(true);
    type_string->auto_fill_type_object(true);
    type_string.register_type(participant);

    // Create the sequence of TypeIdentifiers
    const fastrtps::types::TypeIdentifier* indentifier_string =
            fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
        type_string.get_type_name());

    fastrtps::types::TypeIdentifierSeq types;
    types.push_back(*indentifier_string);

    // Checks that the writer guid prefix given by the TypeObject is the same as the DomainPartipant guid prefix
    ASSERT_EQ(participant->guid().guidPrefix, participant->get_types(types).writer_guid().guidPrefix);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test create a sequence of TypeIdentifiers to call the get_type_dependencies() DomainParticipant function.
 * Finally, the test checks that the writer guid prefix given by the TypeObject is the same as the DomainPartipant
 * guid prefix.
 */
TEST(ParticipantTests, GetTypeDependencies)
{
    // Create the participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.typelookup_config.use_client = true;
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    // Create the dynamic type builder
    DynamicTypeBuilder_ptr builder_string = DynamicTypeBuilderFactory::get_instance()->create_string_builder(100);
    // Create the dynamic type
    DynamicType_ptr dyn_type_string = builder_string->build();
    TypeSupport type_string(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type_string));
    // Create the data instance
    DynamicData_ptr data_string(DynamicDataFactory::get_instance()->create_data(dyn_type_string));
    data_string->set_string_value("Dynamic String");

    // Register the type
    type_string->auto_fill_type_information(true);
    type_string->auto_fill_type_object(true);
    type_string.register_type(participant);

    // Create the sequence of TypeIdentifiers
    const fastrtps::types::TypeIdentifier* indentifier_string =
            fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
        type_string.get_type_name());

    fastrtps::types::TypeIdentifierSeq types;
    types.push_back(*indentifier_string);

    // Checks that the writer guid prefix given by the TypeObject is the same as the DomainPartipant guid prefix
    ASSERT_EQ(participant->guid().guidPrefix, participant->get_type_dependencies(types).writer_guid().guidPrefix);

    // Remove the participant
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test create two participants which will share a complete dynamic type.
 * 1. The remote participant registers a dynamic type
 * 2. The local participant register the dynamic type of the remote participant using the TypeInformation and the type
 *    name
 * 3. Check that the type is not registered if the local participant is disabled
 * 4. Check that the type is registered if the local participant is enabled
 */
TEST(ParticipantTests, RegisterRemoteTypeComplete)
{
    // Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    // Create the remote participant and enable it
    DomainParticipant* remote_participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, remote_participant->enable());
    EXPECT_TRUE(remote_participant->is_enabled());

    // Create the local participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the complete dynamic type builder
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr seqLong_builder =
            DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(int32_builder.get());
    DynamicTypeBuilder_ptr mySequenceLong_builder =
            DynamicTypeBuilderFactory::get_instance()->create_alias_builder(seqLong_builder.get(), "MySequenceLong");
    // Build the dynamic type
    DynamicType_ptr dyn_type = mySequenceLong_builder->build();

    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    type.register_type(remote_participant);

    // Retrieve the Typeidentifier, the type name and the TypeInformation from the TypeObjectFactory
    const fastrtps::types::TypeIdentifier* identifier =
            fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
        type.get_type_name());

    std::string type_name = fastrtps::types::TypeObjectFactory::get_instance()->get_type_name(identifier);

    const fastrtps::types::TypeInformation* type_information =
            fastrtps::types::TypeObjectFactory::get_instance()->get_type_information(type_name);

    Topic* topic = remote_participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Create the functor for the remote type registration
    std::string topic_name = "footopic";
    std::function<void(const std::string&, const fastrtps::types::DynamicType_ptr)> callback =
            [topic_name](const std::string&, const fastrtps::types::DynamicType_ptr type)
            {
                std::cout << "Callback for type: " << type->get_name() << " on topic: " << topic_name << std::endl;
            };

    // Register the remote type in the disabled local participant. This should return a RETCODE_NOT_ENABLED return code
    ASSERT_EQ(participant->register_remote_type(*type_information, type.get_type_name(), callback),
            ReturnCode_t::RETCODE_NOT_ENABLED);

    // Enable the local participant
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());

    // Register the remote type in the disabled local participant
    ASSERT_EQ(participant->register_remote_type(*type_information, type_name, callback),
            ReturnCode_t::RETCODE_OK);

    // Remove the topic and both participants
    ASSERT_EQ(remote_participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(remote_participant),
            ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test create two participants which will share a minimal dynamic type.
 * 1. The remote participant registers a dynamic type
 * 2. The local participant register the dynamic type of the remote participant using the TypeInformation and the type
 *    name
 * 3. Check that the type is not registered if the local participant is disabled
 * 4. Check that the type is registered if the local participant is enabled
 */
TEST(ParticipantTests, RegisterRemoteTypeMinimal)
{
    // Do not enable entities on creation
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    // Create the remote participant and enable it
    DomainParticipant* remote_participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, remote_participant->enable());
    EXPECT_TRUE(remote_participant->is_enabled());

    // Create the local participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the minimal dynamic type builder
    DynamicTypeBuilder_ptr builder = DynamicTypeBuilderFactory::get_instance()->create_char16_builder();
    // Build the dynamic type
    DynamicType_ptr dyn_type = DynamicTypeBuilderFactory::get_instance()->create_type(builder.get());
    DynamicData_ptr data(DynamicDataFactory::get_instance()->create_data(dyn_type));
    data->set_string_value("Dynamic Char16");

    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    type.register_type(remote_participant);

    // Retrieve the Typeidentifier, the type name and the TypeInformation from the TypeObjectFactory
    const fastrtps::types::TypeIdentifier* identifier =
            fastrtps::types::TypeObjectFactory::get_instance()->get_type_identifier_trying_complete(
        type.get_type_name());

    std::string type_name = fastrtps::types::TypeObjectFactory::get_instance()->get_type_name(identifier);

    const fastrtps::types::TypeInformation* type_information =
            fastrtps::types::TypeObjectFactory::get_instance()->get_type_information(type_name);

    Topic* topic = remote_participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Create the functor for the remote type registration
    std::string topic_name = "footopic";
    std::function<void(const std::string&, const fastrtps::types::DynamicType_ptr)> callback =
            [topic_name](const std::string&, const fastrtps::types::DynamicType_ptr type)
            {
                std::cout << "Callback for type: " << type->get_name() << " on topic: " << topic_name << std::endl;
            };

    // Register the remote type in the disabled local participant. This should return a RETCODE_NOT_ENABLED return code
    ASSERT_EQ(participant->register_remote_type(*type_information, type.get_type_name(), callback),
            ReturnCode_t::RETCODE_NOT_ENABLED);

    // Enable the local participant
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());

    // Register the remote type in the disabled local participant
    ASSERT_EQ(participant->register_remote_type(*type_information, type_name, callback),
            ReturnCode_t::RETCODE_OK);

    // Remove the topic and both participants
    ASSERT_EQ(remote_participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(remote_participant),
            ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks that a RETCODE_PRECONDITION_NOT_MET error code is returned when registering a dynamic remote type
 * with an empty TypeInformation
 */
TEST(ParticipantTests, RegisterRemoteTypePreconditionNotMet)
{
    // Create the remote participant
    DomainParticipant* remote_participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the local participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create the type builder
    DynamicTypeBuilder_ptr int32_builder = DynamicTypeBuilderFactory::get_instance()->create_int32_builder();
    DynamicTypeBuilder_ptr seqLong_builder =
            DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(int32_builder.get());
    DynamicTypeBuilder_ptr mySequenceLong_builder =
            DynamicTypeBuilderFactory::get_instance()->create_alias_builder(seqLong_builder.get(), "MySequenceLong");
    // Build the dynamic type
    DynamicType_ptr dyn_type = mySequenceLong_builder->build();

    // Register the type
    TypeSupport type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    type.register_type(remote_participant);

    Topic* topic = remote_participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Create the functor for the remote type registration
    std::string topic_name = "footopic";
    std::function<void(const std::string&, const fastrtps::types::DynamicType_ptr)> callback =
            [topic_name](const std::string&, const fastrtps::types::DynamicType_ptr type)
            {
                std::cout << "Callback for type: " << type->get_name() << " on topic: " << topic_name << std::endl;
            };

    // Create an empty TypeInformation
    fastrtps::types::TypeInformation info = fastrtps::types::TypeInformation();
    // Check that register_remote_type() returns RETCODE_PRECONDITION_NOT_MET if the TypeInformation is empty
    ASSERT_EQ(participant->register_remote_type(info, type.get_type_name(), callback),
            ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    // Remove the topic and both participants
    ASSERT_EQ(remote_participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(remote_participant),
            ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


/*
 * This test checks that the following methods are not implemented and returns an error
 *  create_contentfilteredtopic
 *  delete_contentfilteredtopic
 *  create_multitopic
 *  delete_multitopic
 *  find_topic
 *  get_builtin_subscriber
 *  ignore_participant
 *  ignore_topic
 *  ignore_publictaion
 *  ignore_subscription
 *  delete_contained_entities
 *  get_discovered_participants
 *  get_discovered_topics
 *
 * Tests missing: get_discovered_participant_data & get_discovered_topic_data
 * These methods cannot be tested because there are no implementation of their parameter classes
 */
TEST(ParticipantTests, UnsupportedMethods)
{
    // Create the participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Create a type and a topic
    TypeSupport type(new TopicDataTypeMock());
    ASSERT_EQ(type.register_type(participant), ReturnCode_t::RETCODE_OK);

    Topic* topic = participant->create_topic("topic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    ASSERT_EQ(
        participant->create_contentfilteredtopic(
            "contentfilteredtopic",
            topic,
            "filter_expression",
            std::vector<std::string>({"a", "b"})),
        nullptr);

    // nullptr use as there are not such a class
    ASSERT_EQ(participant->delete_contentfilteredtopic(nullptr), ReturnCode_t::RETCODE_UNSUPPORTED);

    ASSERT_EQ(
        participant->create_multitopic(
            "multitopic",
            "type",
            "subscription_expression",
            std::vector<std::string>({"a", "b"})),
        nullptr);

    // nullptr use as there are not such a class
    ASSERT_EQ(participant->delete_multitopic(nullptr), ReturnCode_t::RETCODE_UNSUPPORTED);

    ASSERT_EQ(participant->find_topic("topic", Duration_t(1, 0)), nullptr);

    ASSERT_EQ(participant->get_builtin_subscriber(), nullptr);

    ASSERT_EQ(participant->ignore_participant(InstanceHandle_t()), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(participant->ignore_topic(InstanceHandle_t()), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(participant->ignore_publictaion(InstanceHandle_t()), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(participant->ignore_subscription(InstanceHandle_t()), ReturnCode_t::RETCODE_UNSUPPORTED);

    ASSERT_EQ(participant->delete_contained_entities(), ReturnCode_t::RETCODE_UNSUPPORTED);

    // Discovery methods
    std::vector<InstanceHandle_t> handle_vector({InstanceHandle_t()});
    builtin::ParticipantBuiltinTopicData pbtd;
    builtin::TopicBuiltinTopicData tbtd;

    ASSERT_EQ(participant->get_discovered_participants(handle_vector), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(
        participant->get_discovered_participant_data(pbtd, InstanceHandle_t()), ReturnCode_t::RETCODE_UNSUPPORTED);

    ASSERT_EQ(participant->get_discovered_topics(handle_vector), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(
        participant->get_discovered_topic_data(tbtd, InstanceHandle_t()), ReturnCode_t::RETCODE_UNSUPPORTED);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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
