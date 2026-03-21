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

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <vector>

#include "BlackboxTests.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class DDSFindTopicTest : public testing::Test
{
    /**
     * A dummy type support class.
     */
    class TestType : public TopicDataType
    {
    public:

        TestType()
            : TopicDataType()
        {
            is_compute_key_provided = false;
            max_serialized_type_size = 16;
        }

        bool serialize(
                const void* const,
                fastdds::rtps::SerializedPayload_t&,
                fastdds::dds::DataRepresentationId_t) override
        {
            return true;
        }

        bool deserialize(
                fastdds::rtps::SerializedPayload_t&,
                void*) override
        {
            return true;
        }

        uint32_t calculate_serialized_size(
                const void* const,
                fastdds::dds::DataRepresentationId_t) override
        {
            return 0u;
        }

        void* create_data() override
        {
            return nullptr;
        }

        void delete_data(
                void*) override
        {
        }

        bool compute_key(
                fastdds::rtps::SerializedPayload_t&,
                fastdds::rtps::InstanceHandle_t&,
                bool) override
        {
            return true;
        }

        bool compute_key(
                const void* const,
                fastdds::rtps::InstanceHandle_t&,
                bool) override
        {
            return false;
        }

    private:

        using TopicDataType::calculate_serialized_size;
        using TopicDataType::serialize;
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
        type_->set_name(c_type_name);
        participant_->register_type(type_);
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
    TopicListener listener_;

    /**
     * Create a topic with parameters different to the default ones, in order to check that the topics returned by
     * find_topic are not default created.
     */
    Topic* create_test_topic()
    {
        TopicQos qos;
        qos.transport_priority().value = 10;
        return participant_->create_topic(TEST_TOPIC_NAME, c_type_name, qos, &listener_, StatusMask::none());
    }

    void check_topics(
            const Topic* lhs,
            const Topic* rhs)
    {
        EXPECT_NE(lhs, rhs);
        EXPECT_EQ(lhs->get_name(), rhs->get_name());
        EXPECT_EQ(lhs->get_type_name(), rhs->get_type_name());
        EXPECT_EQ(lhs->get_qos(), rhs->get_qos());
        EXPECT_EQ(lhs->get_listener(), rhs->get_listener());
        EXPECT_EQ(lhs->get_status_mask(), rhs->get_status_mask());
    }

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
    eprosima::fastdds::dds::Duration_t timeout{ 0, 50 * 1000 * 1000 };
    auto max_tp = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
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
    Topic* topic0 = create_test_topic();
    ASSERT_NE(nullptr, topic0);

    // Procedure:
    // 1. Call DomainParticipant::find_topic with the same topic name as the input Topic, and infinite timeout.
    auto topic = participant_->find_topic(TEST_TOPIC_NAME, fastdds::dds::c_TimeInfinite);

    // Output:
    // - The call returns something different from nullptr.
    // - The returned object has the same topic name, type name, qos, listener, and mask as the original Topic.
    ASSERT_NE(nullptr, topic);
    check_topics(topic0, topic);
}

/**
 * Implements test DDS-DP-FT-03 from the test plan.
 *
 * Check unblocking behavior of DomainParticipant::find_topic.
 */
TEST_F(DDSFindTopicTest, find_topic_unblock)
{
    static constexpr size_t num_threads = 4;

    // Input:
    // - A DomainParticipant correctly initialized (on test setup)
    // - A type registered for c_type_name (on test setup)

    // Procedure:
    // 1. Create several threads, which call DomainParticipant::find_topic with certain topic name,
    //    and infinite timeout.
    auto exec_fn = [this]()
            {
                return participant_->find_topic(TEST_TOPIC_NAME, fastdds::dds::c_TimeInfinite);
            };
    std::vector<std::future<Topic*>> threads;
    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::async(std::launch::async, exec_fn));
    }

    // 2. On the main thread, call DomainParticipant::create_topic with the same topic name, and test_type_name.
    Topic* topic0 = create_test_topic();
    ASSERT_NE(nullptr, topic0);

    // Output:
    // - All threads return from the call to find_topic.
    // - All calls to find_topic return something different from nullptr.
    std::array<Topic*, num_threads> topics;
    topics.fill(nullptr);
    for (size_t i = 0; i < num_threads; ++i)
    {
        topics[i] = threads[i].get();
        ASSERT_NE(nullptr, topics[i]);
    }

    // - All returned values are different.
    for (size_t i = 0; i < num_threads; ++i)
    {
        for (size_t j = i + 1; j < num_threads; ++j)
        {
            EXPECT_NE(topics[i], topics[j]);
        }
    }

    // - All returned values are different from the one returned by create_topic.
    // - All find_topic returned objects have the same topic name, type name, qos, listener, and mask as the object
    //   returned by create_topic.
    for (Topic* topic : topics)
    {
        check_topics(topic0, topic);
    }
}

/**
 * Implements test DDS-DP-FT-04 from the test plan.
 *
 * Check proxy behavior of the objects returned by DomainParticipant::find_topic.
 */
TEST_F(DDSFindTopicTest, find_topic_is_proxy)
{
    // Input:
    // - A DomainParticipant correctly initialized (on test setup)
    // - A Topic object topic_1 correctly created with DomainParticipant::create_topic.
    // - A Topic object topic_2 obtained by calling DomainParticipant::find_topic with the same topic name.
    Topic* topic_1 = create_test_topic();
    ASSERT_NE(nullptr, topic_1);
    Topic* topic_2 = participant_->find_topic(TEST_TOPIC_NAME, fastdds::dds::c_TimeInfinite);
    ASSERT_NE(nullptr, topic_2);
    check_topics(topic_1, topic_2);

    // Procedure:
    // 1. Call set_qos and set_listener on topic_2.
    // 2. Call get_qos, get_listener, and get_status_mask on topic_1.
    // 3. Call set_qosand set_listener on topic_1.
    // 4. Call get_qos, get_listener, and get_status_mask on topic_2.1. Call DomainParticipant::find_topic with the same topic name as the input Topic, and infinite timeout.
    // Output:
    // - Values returned from the getters should match the ones written by the setters.

    // Steps 1, 2.
    {
        TopicListener other_listener;
        TopicQos other_qos;
        topic_2->get_qos(other_qos);
        other_qos.transport_priority().value *= 10;

        topic_2->set_qos(other_qos);
        topic_2->set_listener(&other_listener, StatusMask::inconsistent_topic());
        EXPECT_EQ(other_qos, topic_1->get_qos());
        EXPECT_EQ(&other_listener, topic_1->get_listener());
        EXPECT_EQ(StatusMask::inconsistent_topic(), topic_1->get_status_mask());
    }

    // Steps 3, 4.
    {
        TopicListener other_listener;
        TopicQos other_qos;
        topic_1->get_qos(other_qos);
        other_qos.transport_priority().value /= 10;

        topic_1->set_qos(other_qos);
        topic_1->set_listener(&other_listener, StatusMask::none());
        EXPECT_EQ(other_qos, topic_2->get_qos());
        EXPECT_EQ(&other_listener, topic_2->get_listener());
        EXPECT_EQ(StatusMask::none(), topic_2->get_status_mask());
    }
}

/**
 * Implements test DDS-DP-FT-05 from the test plan.
 *
 * Check create_topic and delete_topic interactions with find_topic.
 */
TEST_F(DDSFindTopicTest, find_topic_delete_topic)
{
    // Input:
    // - A DomainParticipant correctly initialized (on test setup)
    // - A Topic object topic_1 correctly created with DomainParticipant::create_topic.
    // - Two Topic objects topic_2 and topic_3, obtained by calling DomainParticipant::find_topic with the same
    //   topic name.
    // - A DataWriter created on topic_1.
    // - A DataReader created on topic_2.
    Topic* topic_1 = create_test_topic();
    ASSERT_NE(nullptr, topic_1);
    Topic* topic_2 = participant_->find_topic(TEST_TOPIC_NAME, fastdds::dds::c_TimeInfinite);
    ASSERT_NE(nullptr, topic_2);
    Topic* topic_3 = participant_->find_topic(TEST_TOPIC_NAME, fastdds::dds::c_TimeInfinite);
    ASSERT_NE(nullptr, topic_3);
    auto publisher = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    auto data_writer = publisher->create_datawriter(topic_1, DATAWRITER_QOS_DEFAULT);
    auto subscriber = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    auto data_reader = subscriber->create_datareader(topic_2, DATAREADER_QOS_DEFAULT);

    // Procedure:
    // 1. Call DomainParticipant::delete_topic for topic_3.
    // 2. Call DomainParticipant::delete_topic for topic_2.
    // 3. Call DomainParticipant::delete_topic for topic_1.
    // 4. Call DomainParticipant::create_topic for the same topic name.
    // 5. Delete the DataWriter.
    // 6. Call DomainParticipant::delete_topic for topic_3.
    // 7. Call DomainParticipant::delete_topic for topic_2.
    // 8. Call DomainParticipant::delete_topic for topic_1.
    // 9. Call DomainParticipant::create_topic for the same topic name.
    // 10. Delete the DataReader.
    // 11. Call DomainParticipant::delete_topic for topic_3.
    // 12. Call DomainParticipant::delete_topic for topic_2.
    // 13. Call DomainParticipant::delete_topic for topic_1.
    // 14. Call DomainParticipant::create_topic for the same topic name.
    // Output:
    // - The calls to delete_topic return the following codes:
    //   - OK for calls 1, 8, and 12.
    //   - PRECONDITION_NOT_MET for the rest.
    // - Only the last call to create_topic succeeds.

    // Steps 1-4.
    {
        EXPECT_EQ(RETCODE_OK, participant_->delete_topic(topic_3));
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, participant_->delete_topic(topic_2));
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, participant_->delete_topic(topic_1));
        EXPECT_EQ(nullptr, create_test_topic());
    }

    // Steps 5-9.
    {
        EXPECT_EQ(RETCODE_OK, publisher->delete_datawriter(data_writer));
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, participant_->delete_topic(topic_3));
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, participant_->delete_topic(topic_2));
        EXPECT_EQ(RETCODE_OK, participant_->delete_topic(topic_1));
        EXPECT_EQ(nullptr, create_test_topic());
    }

    // Steps 10-14.
    {
        EXPECT_EQ(RETCODE_OK, subscriber->delete_datareader(data_reader));
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, participant_->delete_topic(topic_3));
        EXPECT_EQ(RETCODE_OK, participant_->delete_topic(topic_2));
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, participant_->delete_topic(topic_1));
        EXPECT_NE(nullptr, create_test_topic());
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
