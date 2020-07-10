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

#include <EntityMocks.hpp>

using ::testing::StrictMock;
using ::testing::Mock;

namespace eprosima {
namespace fastdds {
namespace dds {
class CustomDataReaderListener : public DataReaderListener
{
public:

    void on_data_available(
            DataReader*)
    {
        on_data_available_relay();
    }

    MOCK_METHOD0(on_data_available_relay, void());

    void on_subscription_matched(
            DataReader*,
            const fastdds::dds::SubscriptionMatchedStatus&)
    {
        on_subscription_matched_relay();
    }

    MOCK_METHOD0(on_subscription_matched_relay, void());

    void on_requested_deadline_missed(
            DataReader*,
            const fastrtps::RequestedDeadlineMissedStatus&)
    {
        on_requested_deadline_missed_relay();
    }

    MOCK_METHOD0(on_requested_deadline_missed_relay, void());

    void on_liveliness_changed(
            DataReader*,
            const fastrtps::LivelinessChangedStatus&)
    {
        on_liveliness_changed_relay();
    }

    MOCK_METHOD0(on_liveliness_changed_relay, void());

    void on_sample_rejected(
            DataReader*,
            const fastrtps::SampleRejectedStatus&)
    {
        on_sample_rejected_relay();
    }

    MOCK_METHOD0(on_sample_rejected_relay, void());

    void on_requested_incompatible_qos(
            DataReader*,
            const RequestedIncompatibleQosStatus&)
    {
        on_requested_incompatible_qos_relay();
    }

    MOCK_METHOD0(on_requested_incompatible_qos_relay, void());

    void on_sample_lost(
            DataReader*,
            const SampleLostStatus&)
    {
        on_sample_lost_relay();
    }

    MOCK_METHOD0(on_sample_lost_relay, void());
};

class CustomSubscriberListener : public SubscriberListener
{
public:

    void on_data_on_readers(
            Subscriber*)
    {
        on_data_on_readers_relay();
    }

    MOCK_METHOD0(on_data_on_readers_relay, void());

    void on_data_available(
            DataReader*)
    {
        on_data_available_relay();
    }

    MOCK_METHOD0(on_data_available_relay, void());

    void on_subscription_matched(
            DataReader*,
            const fastdds::dds::SubscriptionMatchedStatus&)
    {
        on_subscription_matched_relay();
    }

    MOCK_METHOD0(on_subscription_matched_relay, void());

    void on_requested_deadline_missed(
            DataReader*,
            const fastrtps::RequestedDeadlineMissedStatus&)
    {
        on_requested_deadline_missed_relay();
    }

    MOCK_METHOD0(on_requested_deadline_missed_relay, void());

    void on_liveliness_changed(
            DataReader*,
            const fastrtps::LivelinessChangedStatus&)
    {
        on_liveliness_changed_relay();
    }

    MOCK_METHOD0(on_liveliness_changed_relay, void());

    void on_sample_rejected(
            DataReader*,
            const fastrtps::SampleRejectedStatus&)
    {
        on_sample_rejected_relay();
    }

    MOCK_METHOD0(on_sample_rejected_relay, void());

    void on_requested_incompatible_qos(
            DataReader*,
            const RequestedIncompatibleQosStatus&)
    {
        on_requested_incompatible_qos_relay();
    }

    MOCK_METHOD0(on_requested_incompatible_qos_relay, void());

    void on_sample_lost(
            DataReader*,
            const SampleLostStatus&)
    {
        on_sample_lost_relay();
    }

    MOCK_METHOD0(on_sample_lost_relay, void());
};

class CustomDataWriterListener : public DataWriterListener
{
public:

    void on_publication_matched(
            DataWriter*,
            const PublicationMatchedStatus&)
    {
        on_publication_matched_relay();
    }

    MOCK_METHOD0(on_publication_matched_relay, void());

    void on_offered_deadline_missed(
            DataWriter*,
            const OfferedDeadlineMissedStatus&)
    {
        on_offered_deadline_missed_relay();
    }

    MOCK_METHOD0(on_offered_deadline_missed_relay, void());

    void on_offered_incompatible_qos(
            DataWriter*,
            const OfferedIncompatibleQosStatus&)
    {
        on_offered_incompatible_qos_relay();
    }

    MOCK_METHOD0(on_offered_incompatible_qos_relay, void());

    void on_liveliness_lost(
            DataWriter*,
            const LivelinessLostStatus&)
    {
        on_liveliness_lost_relay();
    }

    MOCK_METHOD0(on_liveliness_lost_relay, void());
};

class CustomPublisherListener : public PublisherListener
{
public:

    void on_publication_matched(
            DataWriter*,
            const PublicationMatchedStatus&)
    {
        on_publication_matched_relay();
    }

    MOCK_METHOD0(on_publication_matched_relay, void());

    void on_offered_deadline_missed(
            DataWriter*,
            const OfferedDeadlineMissedStatus&)
    {
        on_offered_deadline_missed_relay();
    }

    MOCK_METHOD0(on_offered_deadline_missed_relay, void());

    void on_offered_incompatible_qos(
            DataWriter*,
            const OfferedIncompatibleQosStatus&)
    {
        on_offered_incompatible_qos_relay();
    }

    MOCK_METHOD0(on_offered_incompatible_qos_relay, void());

    void on_liveliness_lost(
            DataWriter*,
            const LivelinessLostStatus&)
    {
        on_liveliness_lost_relay();
    }

    MOCK_METHOD0(on_liveliness_lost_relay, void());
};

class CustomParticipantListener : public DomainParticipantListener
{
public:

    void on_publication_matched(
            DataWriter*,
            const PublicationMatchedStatus&)
    {
        on_publication_matched_relay();
    }

    MOCK_METHOD0(on_publication_matched_relay, void());

    void on_offered_deadline_missed(
            DataWriter*,
            const OfferedDeadlineMissedStatus&)
    {
        on_offered_deadline_missed_relay();
    }

    MOCK_METHOD0(on_offered_deadline_missed_relay, void());

    void on_offered_incompatible_qos(
            DataWriter*,
            const OfferedIncompatibleQosStatus&)
    {
        on_offered_incompatible_qos_relay();
    }

    MOCK_METHOD0(on_offered_incompatible_qos_relay, void());

    void on_liveliness_lost(
            DataWriter*,
            const LivelinessLostStatus&)
    {
        on_liveliness_lost_relay();
    }

    MOCK_METHOD0(on_liveliness_lost_relay, void());

    void on_data_on_readers(
            Subscriber*)
    {
        on_data_on_readers_relay();
    }

    MOCK_METHOD0(on_data_on_readers_relay, void());

    void on_data_available(
            DataReader*)
    {
        on_data_available_relay();
    }

    MOCK_METHOD0(on_data_available_relay, void());

    void on_subscription_matched(
            DataReader*,
            const fastdds::dds::SubscriptionMatchedStatus&)
    {
        on_subscription_matched_relay();
    }

    MOCK_METHOD0(on_subscription_matched_relay, void());

    void on_requested_deadline_missed(
            DataReader*,
            const fastrtps::RequestedDeadlineMissedStatus&)
    {
        on_requested_deadline_missed_relay();
    }

    MOCK_METHOD0(on_requested_deadline_missed_relay, void());

    void on_liveliness_changed(
            DataReader*,
            const fastrtps::LivelinessChangedStatus&)
    {
        on_liveliness_changed_relay();
    }

    MOCK_METHOD0(on_liveliness_changed_relay, void());

    void on_sample_rejected(
            DataReader*,
            const fastrtps::SampleRejectedStatus&)
    {
        on_sample_rejected_relay();
    }

    MOCK_METHOD0(on_sample_rejected_relay, void());

    void on_requested_incompatible_qos(
            DataReader*,
            const RequestedIncompatibleQosStatus&)
    {
        on_requested_incompatible_qos_relay();
    }

    MOCK_METHOD0(on_requested_incompatible_qos_relay, void());

    void on_sample_lost(
            DataReader*,
            const SampleLostStatus&)
    {
        on_sample_lost_relay();
    }

    MOCK_METHOD0(on_sample_lost_relay, void());

    void on_inconsistent_topic(
            Topic*,
            InconsistentTopicStatus)
    {
        on_inconsistent_topic_relay();
    }

    MOCK_METHOD0(on_inconsistent_topic_relay, void());
};

// Mocked TopicDataType for Topic creation tests
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

void verify_expectations_on_publication_matched (
        DataWriterMock* writer,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomPublisherListener>& publisher_listener,
        StrictMock<CustomDataWriterListener>& datawriter_listener)
{
    fastdds::dds::PublicationMatchedStatus status;

    writer->get_implementation()->get_inner_listener()->onWriterMatched(nullptr, status);
    Mock::VerifyAndClearExpectations(&datawriter_listener);
    Mock::VerifyAndClearExpectations(&publisher_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, publication_matched)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomPublisherListener> publisher_listener;
    PublisherMock* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, &publisher_listener);
    ASSERT_NE(publisher, nullptr);

    StrictMock<CustomDataWriterListener> datawriter_listener;
    DataWriterMock* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, &datawriter_listener);
    ASSERT_NE(datawriter, nullptr);


    datawriter->set_listener(&datawriter_listener, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datawriter->set_listener(nullptr, StatusMask::all());

    publisher->set_listener(&publisher_listener, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(1);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    publisher->set_listener(&publisher_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(1);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    publisher->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(1);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    // If participant listener is unavailable too, nothing gets called
    publisher->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void verify_expectations_on_offered_incompatible_qos (
        DataWriterMock* writer,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomPublisherListener>& publisher_listener,
        StrictMock<CustomDataWriterListener>& datawriter_listener)
{
    PolicyMask status;

    writer->get_implementation()->get_inner_listener()->on_offered_incompatible_qos(nullptr, status);
    Mock::VerifyAndClearExpectations(&datawriter_listener);
    Mock::VerifyAndClearExpectations(&publisher_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, offered_incompatible_qos)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomPublisherListener> publisher_listener;
    PublisherMock* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, &publisher_listener);
    ASSERT_NE(publisher, nullptr);

    StrictMock<CustomDataWriterListener> datawriter_listener;
    DataWriterMock* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, &datawriter_listener);
    ASSERT_NE(datawriter, nullptr);

    datawriter->set_listener(&datawriter_listener, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datawriter->set_listener(nullptr, StatusMask::all());

    publisher->set_listener(&publisher_listener, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(1);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    publisher->set_listener(&publisher_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(1);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    publisher->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(1);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    // If participant listener is unavailable too, nothing gets called
    publisher->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(datawriter, participant_listener, publisher_listener,
            datawriter_listener);

    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void verify_expectations_on_liveliness_lost (
        DataWriterMock* writer,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomPublisherListener>& publisher_listener,
        StrictMock<CustomDataWriterListener>& datawriter_listener)
{
    LivelinessLostStatus status;

    writer->get_implementation()->get_inner_listener()->on_liveliness_lost(nullptr, status);
    Mock::VerifyAndClearExpectations(&datawriter_listener);
    Mock::VerifyAndClearExpectations(&publisher_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, liveliness_lost_test)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomPublisherListener> publisher_listener;
    PublisherMock* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, &publisher_listener);
    ASSERT_NE(publisher, nullptr);

    StrictMock<CustomDataWriterListener> datawriter_listener;
    DataWriterMock* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, &datawriter_listener);
    ASSERT_NE(datawriter, nullptr);

    datawriter->set_listener(&datawriter_listener, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(&datawriter_listener, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    datawriter->set_listener(nullptr, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datawriter->set_listener(nullptr, StatusMask::all());

    publisher->set_listener(&publisher_listener, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(1);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    publisher->set_listener(&publisher_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(1);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    publisher->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(1);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    // If participant listener is unavailable too, nothing gets called
    publisher->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(datawriter, participant_listener, publisher_listener, datawriter_listener);

    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void verify_expectations_on_subscription_matched (
        DataReaderMock* reader,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomSubscriberListener>& subscriber_listener,
        StrictMock<CustomDataReaderListener>& datareader_listener)
{
    SubscriptionMatchedStatus status;

    reader->get_implementation()->get_inner_listener()->onReaderMatched(nullptr, status);
    Mock::VerifyAndClearExpectations(&datareader_listener);
    Mock::VerifyAndClearExpectations(&subscriber_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, subscription_matched_test)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomSubscriberListener> subscriber_listener;
    SubscriberMock* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &subscriber_listener);
    ASSERT_NE(subscriber, nullptr);

    StrictMock<CustomDataReaderListener> datareader_listener;
    DataReaderMock* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT, &datareader_listener);
    ASSERT_NE(datareader, nullptr);

    datareader->set_listener(&datareader_listener, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datareader->set_listener(nullptr, StatusMask::all());

    subscriber->set_listener(&subscriber_listener, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(1);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(1);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(1);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    // If participant listener is unavailable too, nothing gets called
    subscriber->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    ASSERT_EQ(subscriber->delete_datareader(datareader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void verify_expectations_on_liveliness_changed (
        DataReaderMock* reader,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomSubscriberListener>& subscriber_listener,
        StrictMock<CustomDataReaderListener>& datareader_listener)
{
    LivelinessChangedStatus status;

    reader->get_implementation()->get_inner_listener()->on_liveliness_changed(nullptr, status);
    Mock::VerifyAndClearExpectations(&datareader_listener);
    Mock::VerifyAndClearExpectations(&subscriber_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, liveliness_changed_test)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomSubscriberListener> subscriber_listener;
    SubscriberMock* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &subscriber_listener);
    ASSERT_NE(subscriber, nullptr);

    StrictMock<CustomDataReaderListener> datareader_listener;
    DataReaderMock* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT, &datareader_listener);
    ASSERT_NE(datareader, nullptr);

    datareader->set_listener(&datareader_listener, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datareader->set_listener(nullptr, StatusMask::all());

    subscriber->set_listener(&subscriber_listener, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(1);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(1);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(1);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    // If participant listener is unavailable too, nothing gets called
    subscriber->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    ASSERT_EQ(subscriber->delete_datareader(datareader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void verify_expectations_on_requested_incompatible_qos (
        DataReaderMock* reader,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomSubscriberListener>& subscriber_listener,
        StrictMock<CustomDataReaderListener>& datareader_listener)
{
    PolicyMask status;

    reader->get_implementation()->get_inner_listener()->on_requested_incompatible_qos(nullptr, status);
    Mock::VerifyAndClearExpectations(&datareader_listener);
    Mock::VerifyAndClearExpectations(&subscriber_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, requested_incompatible_qos_test)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomSubscriberListener> subscriber_listener;
    SubscriberMock* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &subscriber_listener);
    ASSERT_NE(subscriber, nullptr);

    StrictMock<CustomDataReaderListener> datareader_listener;
    DataReaderMock* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT, &datareader_listener);
    ASSERT_NE(datareader, nullptr);

    // All statuses active
    datareader->set_listener(&datareader_listener, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datareader->set_listener(nullptr, StatusMask::all());

    subscriber->set_listener(&subscriber_listener, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(1);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(1);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(1);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    // If participant listener is unavailable too, nothing gets called
    subscriber->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(datareader, participant_listener, subscriber_listener,
            datareader_listener);

    ASSERT_EQ(subscriber->delete_datareader(datareader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

void verify_expectations_on_data_available (
        DataReaderMock* reader,
        StrictMock<CustomParticipantListener>& participant_listener,
        StrictMock<CustomSubscriberListener>& subscriber_listener,
        StrictMock<CustomDataReaderListener>& datareader_listener)
{
    fastrtps::rtps::CacheChange_t change;

    reader->get_implementation()->get_inner_listener()->onNewCacheChangeAdded(nullptr, &change);
    Mock::VerifyAndClearExpectations(&datareader_listener);
    Mock::VerifyAndClearExpectations(&subscriber_listener);
    Mock::VerifyAndClearExpectations(&participant_listener);
}

TEST(UserListeners, data_available)
{
    StrictMock<CustomParticipantListener> participant_listener;
    DomainParticipantMock* participant =
            DomainParticipantFactoryMock::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                    &participant_listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    StrictMock<CustomSubscriberListener> subscriber_listener;
    SubscriberMock* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &subscriber_listener);
    ASSERT_NE(subscriber, nullptr);

    StrictMock<CustomDataReaderListener> datareader_listener;
    DataReaderMock* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT, &datareader_listener);
    ASSERT_NE(datareader, nullptr);

    fastrtps::rtps::CacheChange_t change;

    //data_on_readers has priority
    ////////////////////////////////////////////////////////////////////

    // Set all statuses active on the reader and participant, see if they ever get called
    datareader->set_listener(&datareader_listener, StatusMask::all());
    participant->set_listener(&participant_listener, StatusMask::all());

    subscriber->set_listener(&subscriber_listener, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::all() >> StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::all() >> StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::all() >> StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    // If subscriber listener and participant listener are unavailable, nothing is called
    datareader->set_listener(nullptr, StatusMask::all());
    subscriber->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);


    // If no data_on_readers, then try data_available
    ///////////////////////////////////////////////////////////////////////

    // Set all statuses active on the subscriber and participant, except data_on_readers
    subscriber->set_listener(&subscriber_listener, StatusMask::all() >> StatusMask::data_on_readers());
    participant->set_listener(&participant_listener, StatusMask::all() >> StatusMask::data_on_readers());

    datareader->set_listener(&datareader_listener, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::data_available());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(&datareader_listener, StatusMask::all() >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(nullptr, StatusMask::data_available());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    datareader->set_listener(nullptr, StatusMask::all() >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    // If both datareader and subscriber listeners are unavailable, the participant is called
    datareader->set_listener(nullptr, StatusMask::all());

    subscriber->set_listener(&subscriber_listener, StatusMask::all()
            >> StatusMask::data_on_readers()
            >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(&subscriber_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    subscriber->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    // If participant listener is unavailable too, nothing gets called
    datareader->set_listener(nullptr, StatusMask::all());
    subscriber->set_listener(nullptr, StatusMask::all());

    participant->set_listener(&participant_listener, StatusMask::all()
            >> StatusMask::data_on_readers()
            >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    participant->set_listener(&participant_listener, StatusMask::none());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);

    participant->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(datareader, participant_listener, subscriber_listener, datareader_listener);


    ASSERT_EQ(subscriber->delete_datareader(datareader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(DomainParticipantFactoryMock::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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
