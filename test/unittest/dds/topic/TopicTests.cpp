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
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <dds/domain/DomainParticipant.hpp>
#include <dds/core/types.hpp>
#include <dds/topic/Topic.hpp>

#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>


namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::TopicAttributes;
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

};

TEST(TopicTests, ChangeTopicQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    TopicQos qos;
    ASSERT_EQ(topic->get_qos(qos), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    ASSERT_EQ(topic->set_qos(qos), ReturnCode_t::RETCODE_OK);
    TopicQos tqos;
    ASSERT_EQ(topic->get_qos(tqos), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == tqos);
    ASSERT_EQ(tqos.reliability().kind, RELIABLE_RELIABILITY_QOS);

    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

TEST(TopicTests, GetTopicParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    ASSERT_EQ(topic->get_participant(), participant);

    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

void set_listener_test (
        Topic* topic,
        TopicListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(topic->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(topic->get_status_mask(), mask);
}

class CustomListener : public TopicListener
{

};

TEST(TopicTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    ASSERT_EQ(topic->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<Topic*, TopicListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { topic, &listener, StatusMask::data_available() },
        //all except one
        { topic, &listener, StatusMask::all() >> StatusMask::sample_lost() },
        //all and none
        { topic, &listener, StatusMask::all() },
        { topic, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

/*
 * This test checks the allocation consistency when NOT using instances.
 * If the topic is keyed,
 * max_samples should be greater or equal than max_instances * max_samples_per_instance.
 * If that condition is not met, the endpoint creation should fail.
 * If not keyed (not using instances), the only property that is used is max_samples,
 * thus, should not fail with the previously mentioned configuration.
 * The following method is checked:
 * 1. create_topic
 */
TEST(TopicTests, InstancePolicyAllocationConsistencyNotKeyed)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // Next QoS config checks that if user sets max_instances to inf and leaves max_samples by default,
    // create_topic() should NOT return nullptr.
    // By not using instances, this does not make any change.
    TopicQos qos = TOPIC_QOS_DEFAULT;
    qos.resource_limits().max_instances = 0;

    Topic* topic1 = participant->create_topic("footopic1", type.get_type_name(), qos);
    ASSERT_NE(topic1, nullptr);

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // By not using instances, this does not make any change.
    qos.resource_limits().max_instances = -1;

    Topic* topic2 = participant->create_topic("footopic1", type.get_type_name(), qos);
    ASSERT_NE(topic2, nullptr);

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // create_topic() should NOT return nullptr.
    // By not using instances, this does not make any change.
    qos.resource_limits().max_samples = 4999;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    Topic* topic3 = participant->create_topic("footopic2", type.get_type_name(), qos);
    ASSERT_NE(topic3, nullptr);
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
