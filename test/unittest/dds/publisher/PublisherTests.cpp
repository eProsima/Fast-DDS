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
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/pub/DataWriter.hpp>
#include <dds/topic/Topic.hpp>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>


namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::PublisherAttributes;
using fastrtps::xmlparser::XMLProfileManager;
using fastrtps::xmlparser::XMLP_ret;

class TopicDataTypeMock : public TopicDataType
{
public:

    TopicDataTypeMock()
        : TopicDataType()
    {
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

TEST(PublisherTests, GetPublisherParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    ASSERT_EQ(publisher->get_participant(), participant);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(PublisherTests, GetPSMPublisherParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant, PUBLISHER_QOS_DEFAULT);

    ASSERT_EQ(publisher.participant().delegate().get(), participant.delegate().get());

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

    qos.deadline().period = 260;

    ASSERT_TRUE(publisher->set_default_datawriter_qos(qos) == ReturnCode_t::RETCODE_OK);
    DataWriterQos wqos;
    publisher->get_default_datawriter_qos(wqos);

    ASSERT_TRUE(qos == wqos);
    ASSERT_EQ(wqos.deadline().period, 260);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}


TEST(PublisherTests, ChangePSMDefaultDataWriterQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);

    ::dds::pub::qos::DataWriterQos qos = publisher.default_datawriter_qos();
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.deadline().period = 540;

    ASSERT_NO_THROW(publisher.default_datawriter_qos(qos));
    ::dds::pub::qos::DataWriterQos wqos = publisher.default_datawriter_qos();

    ASSERT_TRUE(qos == wqos);
    ASSERT_EQ(wqos.deadline().period, 540);
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

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

TEST(PublisherTests, ChangePSMPublisherQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);

    ::dds::pub::qos::PublisherQos qos = publisher.qos();
    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    publisher.qos(qos);
    ::dds::pub::qos::PublisherQos pqos = publisher.qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
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

    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


void check_datawriter_with_profile (
        DataWriter* datawriter,
        const std::string& profile_name)
{
    DataWriterQos qos;
    datawriter->get_qos(qos);

    PublisherAttributes publisher_atts;
    XMLProfileManager::fillPublisherAttributes(profile_name, publisher_atts);

    //Values taken from profile
    ASSERT_TRUE(
        qos.writer_resource_limits().matched_subscriber_allocation ==
        publisher_atts.matched_subscriber_allocation);
    ASSERT_TRUE(qos.properties() == publisher_atts.properties);
    ASSERT_TRUE(qos.throughput_controller() == publisher_atts.throughputController);
    ASSERT_TRUE(qos.endpoint().unicast_locator_list == publisher_atts.unicastLocatorList);
    ASSERT_TRUE(qos.endpoint().multicast_locator_list == publisher_atts.multicastLocatorList);
    ASSERT_TRUE(qos.endpoint().remote_locator_list == publisher_atts.remoteLocatorList);
    ASSERT_TRUE(qos.endpoint().history_memory_policy == publisher_atts.historyMemoryPolicy);
    ASSERT_TRUE(qos.endpoint().user_defined_id == publisher_atts.getUserDefinedID());
    ASSERT_TRUE(qos.endpoint().entity_id == publisher_atts.getEntityID());
    ASSERT_TRUE(qos.reliable_writer_qos().times == publisher_atts.times);
    ASSERT_TRUE(qos.reliable_writer_qos().disable_positive_acks == publisher_atts.qos.m_disablePositiveACKs);
    ASSERT_TRUE(qos.durability() == publisher_atts.qos.m_durability);
    ASSERT_TRUE(qos.durability_service() == publisher_atts.qos.m_durabilityService);
    ASSERT_TRUE(qos.deadline() == publisher_atts.qos.m_deadline);
    ASSERT_TRUE(qos.latency_budget() == publisher_atts.qos.m_latencyBudget);
    ASSERT_TRUE(qos.liveliness() == publisher_atts.qos.m_liveliness);
    ASSERT_TRUE(qos.reliability() == publisher_atts.qos.m_reliability);
    ASSERT_TRUE(qos.lifespan() == publisher_atts.qos.m_lifespan);
    ASSERT_TRUE(qos.user_data().data_vec() == publisher_atts.qos.m_userData.data_vec());
    ASSERT_TRUE(qos.ownership() == publisher_atts.qos.m_ownership);
    ASSERT_TRUE(qos.ownership_strength() == publisher_atts.qos.m_ownershipStrength);
    ASSERT_TRUE(qos.destination_order() == publisher_atts.qos.m_destinationOrder);
    ASSERT_TRUE(qos.representation() == publisher_atts.qos.representation);
    ASSERT_TRUE(qos.publish_mode() == publisher_atts.qos.m_publishMode);
    ASSERT_TRUE(qos.history() == publisher_atts.topic.historyQos);
    ASSERT_TRUE(qos.resource_limits() == publisher_atts.topic.resourceLimitsQos);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.transport_priority() == DATAWRITER_QOS_DEFAULT.transport_priority());
    ASSERT_TRUE(qos.writer_data_lifecycle() == DATAWRITER_QOS_DEFAULT.writer_data_lifecycle());
}

TEST(PublisherTests, CreateDataWriterWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
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
    ASSERT_TRUE(publisher->delete_datawriter(default_datawriter) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    DataWriter* datawriter = publisher->create_datawriter_with_profile(topic, "test_publisher_profile");
    ASSERT_NE(datawriter, nullptr);
    check_datawriter_with_profile(datawriter, "test_publisher_profile");
    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(PublisherTests, GetDataWriterProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);

    // Extract qos from profile
    DataWriterQos qos;
    ASSERT_EQ(
        publisher->get_datawriter_qos_from_profile("test_publisher_profile", qos),
        ReturnCode_t::RETCODE_OK);

    //Datawriter using the extracted qos
    DataWriter* datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_NE(datawriter, nullptr);
    check_datawriter_with_profile(datawriter, "test_publisher_profile");

    ASSERT_EQ(
        publisher->get_datawriter_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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

    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


void set_listener_test (
        Publisher* publisher,
        PublisherListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(publisher->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
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

    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
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
