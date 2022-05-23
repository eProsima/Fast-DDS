// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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



#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <gtest/gtest.h>

#include <chrono>

#include "BlackboxTests.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

class DDSFindTopicTest : public testing::Test
{
    /**
     * A mock type support class.
     */
    struct TestType : public TopicDataType
    {
        bool serialize(
                void*,
                fastrtps::rtps::SerializedPayload_t*) override
        {
            return true;
        }

        bool deserialize(
                fastrtps::rtps::SerializedPayload_t*,
                void*) override
        {
            return true;
        }

        std::function<uint32_t()> getSerializedSizeProvider(
                void*) override
        {
            return {};
        }

        void* createData() override
        {
            return nullptr;
        }

        void deleteData(
                void*) override
        {
        }

        bool getKey(
                void*,
                fastrtps::rtps::InstanceHandle_t*,
                bool) override
        {
            return false;
        }

    };

public:

    void SetUp() override
    {
        // No need to enable the participant, as it does not need to transmit information.
        DomainParticipantFactoryQos factory_qos;
        factory_qos.entity_factory().autoenable_created_entities = false;
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        ASSERT_NE(nullptr, participant_);

        type_.reset(new TestType);
        type_->setName(c_type_name);
        type_.register_type(participant_);
    }

    void TearDown() override
    {
        if (nullptr != participant_)
        {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

protected:

    static constexpr const char* const c_type_name = "testing::find_topic::test_type";

    DomainParticipant* participant_ = nullptr;
    TypeSupport type_;
};

/**
 * Implements test DDS-DP-FT-01 from the test plan.
 *
 * Check timeout behavior of DomainParticipant::find_topic.
 */
TEST_F(DDSFindTopicTest, find_topic_timeout)
{
    // Input:
    // - A DomainParticipant correctly initialized (on test setup)

    // Procedure:
    // 1. Call DomainParticipant::find_topic with a valid topic name and certain, non-infinite, timeout.
    eprosima::fastrtps::Duration_t timeout{ 1, 0 };
    auto max_tp = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    auto topic = participant_->find_topic(TEST_TOPIC_NAME, timeout);

    // Output:
    // - The call returns nullptr.
    // - At least timeout time has elapsed.
    EXPECT_EQ(nullptr, topic);
    EXPECT_GE(std::chrono::steady_clock::now(), max_tp);
}

/**
 * Implements test DDS-DP-FT-02 from the test plan.
 *
 * Check non-timeout behavior of DomainParticipant::find_topic.
 */
TEST_F(DDSFindTopicTest, find_topic_no_timeout)
{
    // Input:
    // - A DomainParticipant correctly initialized (on test setup)
    // - A Topic correctly created with DomainParticipant::create_topic.
    TopicListener listener;
    TopicQos qos;
    qos.transport_priority().value = 10;
    Topic* topic0 = participant_->create_topic(TEST_TOPIC_NAME, c_type_name, qos, &listener, StatusMask::none());
    ASSERT_NE(nullptr, topic0);

    // Procedure:
    // 1. Call DomainParticipant::find_topic with the same topic name as the input Topic, and infinite timeout.
    auto topic = participant_->find_topic(TEST_TOPIC_NAME, fastrtps::c_TimeInfinite);

    // Output:
    // - The call returns something different from nullptr.
    // - The returned object has the same topic name, type name, qos, listener, and mask as the original Topic.
    ASSERT_NE(nullptr, topic);
    EXPECT_EQ(topic0->get_name(), topic->get_name());
    EXPECT_EQ(topic0->get_type_name(), topic->get_type_name());
    EXPECT_EQ(topic0->get_qos(), topic->get_qos());
    EXPECT_EQ(topic0->get_listener(), topic->get_listener());
    EXPECT_EQ(topic0->get_status_mask(), topic->get_status_mask());
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
