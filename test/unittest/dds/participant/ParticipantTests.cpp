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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/core/types.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/topic/Topic.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

// Mocked TopicDataType for Topic creation tests
class TopicDataTypeMock : public TopicDataType
{
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

    void * createData() override
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


TEST(ParticipantTest, DomainParticipantFactoryGetInstance)
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
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    ASSERT_NE(participant, nullptr);

}

TEST(ParticipantTests, CreatePSMDomainParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::core::null;
    participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_NE(participant, ::dds::core::null);

}

TEST(ParticipantTests, DeleteDomainParticipant)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

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

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_default_participant_qos(PARTICIPANT_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);

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
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
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

TEST(ParticipantTests, CreatePublisher)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    ASSERT_NE(publisher, nullptr);
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
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    PublisherQos qos;
    ASSERT_TRUE(participant->get_default_publisher_qos(qos) == ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_TRUE(participant->set_default_publisher_qos(qos) == ReturnCode_t::RETCODE_OK);

    PublisherQos pqos;
    ASSERT_TRUE(participant->get_default_publisher_qos(pqos) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
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
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    ASSERT_NE(subscriber, nullptr);
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
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteSubscriber)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangeDefaultSubscriberQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    SubscriberQos qos;
    ASSERT_EQ(participant->get_default_subscriber_qos(qos), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_EQ(participant->set_default_subscriber_qos(qos), ReturnCode_t::RETCODE_OK);

    SubscriberQos pqos;
    ASSERT_EQ(participant->get_default_subscriber_qos(pqos), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(pqos == qos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
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
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    TopicQos qos;
    participant->get_default_topic_qos(qos);

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    ASSERT_TRUE(participant->set_default_topic_qos(qos) == ReturnCode_t::RETCODE_OK);

    TopicQos tqos;
    participant->get_default_topic_qos(tqos);

    ASSERT_EQ(qos, tqos);
    ASSERT_EQ(tqos.reliability().kind, BEST_EFFORT_RELIABILITY_QOS);
}

TEST(ParticipantTests, ChangePSMDefaultTopicQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0);
    ::dds::topic::qos::TopicQos qos = participant.default_topic_qos();

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;

    ASSERT_NO_THROW(participant.default_topic_qos(qos));

    ::dds::topic::qos::TopicQos tqos = participant.default_topic_qos();
    ASSERT_EQ(qos, tqos);
    ASSERT_EQ(tqos.ownership().kind, EXCLUSIVE_OWNERSHIP_QOS);
}

TEST(ParticipantTests, CreateTopic)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    TypeSupport type_(new TopicDataTypeMock());
    participant->register_type(type_, "footype");

    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
}

TEST(ParticipantTests, PSMCreateTopic)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0);

    TypeSupport type_(new TopicDataTypeMock());
    participant->register_type(type_, "footype");

    ::dds::topic::Topic topic = ::dds::core::null;
    topic = ::dds::topic::Topic(participant, "footopic", "footype", TOPIC_QOS_DEFAULT);

    ASSERT_NE(topic, ::dds::core::null);
}

TEST(ParticipantTests, DeleteTopic)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    DomainParticipant* participant2 = DomainParticipantFactory::get_instance()->create_participant(1);

    TypeSupport type_(new TopicDataTypeMock());
    participant->register_type(type_, "footype");

    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    ASSERT_TRUE(participant->delete_topic(nullptr) == ReturnCode_t::RETCODE_BAD_PARAMETER);
    ASSERT_TRUE(participant2->delete_topic(topic) == ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, LookupTopicDescription)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    const std::string topic_name("footopic");

    // Topic not created yet. Should return nil
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), nullptr);

    // After topic creation ...
    TypeSupport type_(new TopicDataTypeMock());
    participant->register_type(type_, "footype");
    Topic* topic = participant->create_topic(topic_name, "footype", TOPIC_QOS_DEFAULT);
    EXPECT_NE(topic, nullptr);

    // ... the topic should be returned.
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), topic);

    // After topic deletion, should return nil
    EXPECT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), nullptr);
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
