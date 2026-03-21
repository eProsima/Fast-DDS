// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/fastdds/publisher/PublisherImpl.hpp>
#include <statistics/types/typesPubSubTypes.hpp>

#include "../../logging/mock/MockConsumer.h"

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class StatisticsDomainParticipantMockTests : public ::testing::Test
{
public:

    void helper_block_for_at_least_entries(
            uint32_t amount)
    {
        mock_consumer_->wait_for_at_least_entries(amount);
    }

    eprosima::fastdds::dds::MockConsumer* mock_consumer_;

};

class DomainParticipantImplTest : public DomainParticipantImpl
{
public:

    eprosima::fastdds::dds::Publisher* get_builtin_publisher() const
    {
        return builtin_publisher_;
    }

    PublisherImpl* get_builtin_publisher_impl() const
    {
        return builtin_publisher_impl_;
    }

};

class DomainParticipantTest : public eprosima::fastdds::dds::DomainParticipant
{
public:

    eprosima::fastdds::dds::DomainParticipantImpl* get_impl() const
    {
        return impl_;
    }

};

/**
 * This test checks that enable_statistics_datawriter fails returning eprosima::fastdds::dds::RETCODE_ERROR when create_datawriter fails
 * returning a nullptr.
 * 1. Create participant
 * 2. Mock create_datawriter so it returns nullptr
 * 3. Call enable_statistics_datawriter and check it fails.
 * 4. Capture log error entry
 */
TEST_F(StatisticsDomainParticipantMockTests, EnableStatisticsDataWriterFailureCreateDataWriter)
{
#ifdef FASTDDS_STATISTICS
    mock_consumer_ = new eprosima::fastdds::dds::MockConsumer();

    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(mock_consumer_));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("(STATISTICS_DOMAIN_PARTICIPANT)"));
    eprosima::fastdds::dds::Log::SetErrorStringFilter(std::regex("(DataWriter creation has failed)"));

    eprosima::fastdds::dds::TypeSupport null_type(nullptr);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 2. Mock create_datawriter
    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());
    ASSERT_NE(nullptr, statistics_participant_impl_test);
    PublisherImpl* builtin_pub_impl = statistics_participant_impl_test->get_builtin_publisher_impl();
    EXPECT_CALL(*builtin_pub_impl, create_datawriter_mock()).WillOnce(testing::Return(true));

    // 3. enable_statistics_datawriter
    EXPECT_EQ(fastdds::dds::RETCODE_ERROR,
            statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));

    // 4. Check log error entry
    helper_block_for_at_least_entries(1);
    auto consumed_entries = mock_consumer_->ConsumedEntries();
    EXPECT_EQ(consumed_entries.size(), 1u);

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(participant), fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that disable_statistics_datawriter fails returning eprosima::fastdds::dds::RETCODE_ERROR when delete_datawriter fails.
 * 1. Create a participant
 * 2. Mock delete_datawriter
 * 3. Enable a statistics datawriter
 * 4. Call disable_statistics_datawriter and check return code
 */
TEST_F(StatisticsDomainParticipantMockTests, DisableStatisticsDataWriterFailureDeleteDataWriter)
{
#ifdef FASTDDS_STATISTICS
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);
    eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
    count_type->register_type_object_representation();

    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 2. Mock delete_datawriter
    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());
    ASSERT_NE(nullptr, statistics_participant_impl_test);
    eprosima::fastdds::dds::Publisher* builtin_pub = statistics_participant_impl_test->get_builtin_publisher();
    ASSERT_NE(nullptr, builtin_pub);
    PublisherImpl* builtin_pub_impl = statistics_participant_impl_test->get_builtin_publisher_impl();
    ASSERT_NE(nullptr, builtin_pub_impl);
    EXPECT_CALL(*builtin_pub, delete_datawriter_mock()).WillOnce(testing::Return(true));
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(false));

    // 3. enable_statistics_datawriter
    EXPECT_CALL(*builtin_pub_impl, create_datawriter_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(fastdds::dds::RETCODE_OK,
            statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));

    // 4. disable_statistics_datawriter
    EXPECT_EQ(fastdds::dds::RETCODE_ERROR, statistics_participant->disable_statistics_datawriter(
                HEARTBEAT_COUNT_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_TRUE(count_type == statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_CALL(*builtin_pub, delete_datawriter_mock()).WillOnce(testing::Return(false));
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->disable_statistics_datawriter(
                HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(null_type, statistics_participant->find_type(count_type.get_type_name()));

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/**
 * This test checks that disable_statistics_datawriter fails returning eprosima::fastdds::dds::RETCODE_ERROR when delete_topic fails.
 * 1. Create a participant
 * 2. Mock delete_topic
 * 3. Enable a statistics datawriter
 * 4. Call disable_statistics_datawriter and check return code
 */
TEST_F(StatisticsDomainParticipantMockTests, DisableStatisticsDataWriterFailureDeleteTopic)
{
#ifdef FASTDDS_STATISTICS
    // 1. Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DomainParticipant* statistics_participant = DomainParticipant::narrow(participant);
    ASSERT_NE(statistics_participant, nullptr);

    // 2. Mock delete_topic
    DomainParticipantTest* participant_test = static_cast<DomainParticipantTest*>(participant);
    ASSERT_NE(nullptr, participant_test);
    DomainParticipantImplTest* statistics_participant_impl_test = static_cast<DomainParticipantImplTest*>(
        participant_test->get_impl());
    ASSERT_NE(nullptr, statistics_participant_impl_test);
    eprosima::fastdds::dds::Publisher* builtin_pub = statistics_participant_impl_test->get_builtin_publisher();
    ASSERT_NE(nullptr, builtin_pub);
    PublisherImpl* builtin_pub_impl = statistics_participant_impl_test->get_builtin_publisher_impl();
    ASSERT_NE(nullptr, builtin_pub_impl);
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(true));
    EXPECT_CALL(*builtin_pub, delete_datawriter_mock()).WillOnce(testing::Return(false));

    // 3. enable_statistics_datawriter
    EXPECT_CALL(*builtin_pub_impl, create_datawriter_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(fastdds::dds::RETCODE_OK,
            statistics_participant->enable_statistics_datawriter(HEARTBEAT_COUNT_TOPIC,
            STATISTICS_DATAWRITER_QOS));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    // 4. disable_statistics_datawriter
    EXPECT_EQ(fastdds::dds::RETCODE_ERROR, statistics_participant->disable_statistics_datawriter(
                HEARTBEAT_COUNT_TOPIC));
    EXPECT_NE(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    // As the DataWriter has been deleted, the topic has to be removed manually
    EXPECT_CALL(*statistics_participant_impl_test, delete_topic_mock()).WillOnce(testing::Return(false));
    EXPECT_EQ(fastdds::dds::RETCODE_OK, statistics_participant->delete_topic(
                dynamic_cast<eprosima::fastdds::dds::Topic*>(statistics_participant->lookup_topicdescription(
                    HEARTBEAT_COUNT_TOPIC))));
    EXPECT_EQ(nullptr, statistics_participant->lookup_topicdescription(HEARTBEAT_COUNT_TOPIC));

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    delete_participant(statistics_participant), fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

} // dds
} // statistics
} // fastdds
} // eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
