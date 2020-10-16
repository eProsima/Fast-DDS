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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/core/types.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/topic/Topic.hpp>

#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>


namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::PublisherAttributes;
using fastrtps::SubscriberAttributes;
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

    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    ASSERT_TRUE(subscriber->set_default_datareader_qos(qos) == ReturnCode_t::RETCODE_OK);

    DataReaderQos wqos;
    subscriber->get_default_datareader_qos(wqos);

    ASSERT_EQ(qos, wqos);
    ASSERT_EQ(wqos.reliability().kind, BEST_EFFORT_RELIABILITY_QOS);

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
    ASSERT_TRUE(qos.properties() == subscriber_atts.properties);
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
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
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
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);

    // Extract qos from profile
    DataReaderQos qos;
    ASSERT_EQ(
        subscriber->get_datareader_qos_from_profile("test_subscriber_profile", qos),
        ReturnCode_t::RETCODE_OK);

    //DataReader using the extracted qos
    DataReader* datareader = subscriber->create_datareader(topic, qos);

    check_datareader_with_profile(datareader, "test_subscriber_profile");

    ASSERT_EQ(
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
