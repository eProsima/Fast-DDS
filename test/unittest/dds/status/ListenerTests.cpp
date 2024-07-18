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
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>


using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::Mock;
using ::testing::_;

using eprosima::fastdds::rtps::RTPSDomain;

namespace eprosima {

namespace fastdds {
namespace rtps {
class RTPSDomain;

RTPSReader* RTPSDomain::reader_ = nullptr;
RTPSWriter* RTPSDomain::writer_ = nullptr;
RTPSParticipant* RTPSDomain::participant_ = nullptr;
} //namespace rtps

namespace dds {

class RTPSParticipantMock : public eprosima::fastdds::rtps::RTPSParticipant
{
public:

    RTPSParticipantMock()
    {
    }

    virtual ~RTPSParticipantMock() = default;
};

class RTPSReaderMock : public eprosima::fastdds::rtps::BaseReader
{
public:

    RTPSReaderMock()
    {
    }

    virtual ~RTPSReaderMock() = default;

    bool matched_writer_add_edp(
            const eprosima::fastdds::rtps::WriterProxyData&) override
    {
        return true;
    }

    bool matched_writer_remove(
            const eprosima::fastdds::rtps::GUID_t&,
            bool) override
    {
        return true;
    }

    bool matched_writer_is_matched(
            const eprosima::fastdds::rtps::GUID_t&) override
    {
        return true;
    }

    void assert_writer_liveliness(
            const eprosima::fastdds::rtps::GUID_t&) override
    {
    }

    bool is_in_clean_state() override
    {
        return true;
    }

};

class RTPSWriterMock : public eprosima::fastdds::rtps::RTPSWriter
{
public:

    RTPSWriterMock()
    {
    }

    virtual ~RTPSWriterMock() = default;

    virtual bool matched_reader_add(
            const eprosima::fastdds::rtps::SubscriptionBuiltinTopicData&)
    {
        return true;
    }

    virtual bool matched_reader_remove(
            const eprosima::fastdds::rtps::GUID_t&)
    {
        return true;
    }

    virtual bool matched_reader_is_matched(
            const eprosima::fastdds::rtps::GUID_t&)
    {
        return true;
    }

};

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
            const RequestedDeadlineMissedStatus&)
    {
        on_requested_deadline_missed_relay();
    }

    MOCK_METHOD0(on_requested_deadline_missed_relay, void());

    void on_liveliness_changed(
            DataReader*,
            const LivelinessChangedStatus&)
    {
        on_liveliness_changed_relay();
    }

    MOCK_METHOD0(on_liveliness_changed_relay, void());

    void on_sample_rejected(
            DataReader*,
            const SampleRejectedStatus&)
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
            const RequestedDeadlineMissedStatus&)
    {
        on_requested_deadline_missed_relay();
    }

    MOCK_METHOD0(on_requested_deadline_missed_relay, void());

    void on_liveliness_changed(
            DataReader*,
            const LivelinessChangedStatus&)
    {
        on_liveliness_changed_relay();
    }

    MOCK_METHOD0(on_liveliness_changed_relay, void());

    void on_sample_rejected(
            DataReader*,
            const SampleRejectedStatus&)
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
            const RequestedDeadlineMissedStatus&)
    {
        on_requested_deadline_missed_relay();
    }

    MOCK_METHOD0(on_requested_deadline_missed_relay, void());

    void on_liveliness_changed(
            DataReader*,
            const LivelinessChangedStatus&)
    {
        on_liveliness_changed_relay();
    }

    MOCK_METHOD0(on_liveliness_changed_relay, void());

    void on_sample_rejected(
            DataReader*,
            const SampleRejectedStatus&)
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
        max_serialized_type_size = 4u;
        set_name("footype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::dds::DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            fastdds::dds::DataRepresentationId_t /*data_representation*/) override
    {
        return 0;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

private:

    using TopicDataType::calculate_serialized_size;
    using TopicDataType::serialize;
};

class UserListeners : public ::testing::Test
{

protected:

    void SetUp() override
    {
        // Set the RTPS entity mocks on the RTPSDomain (also mocked)
        RTPSDomain::participant_ = &participant_mock_;
        RTPSDomain::writer_ = &writer_mock_;
        RTPSDomain::reader_ = &reader_mock_;

        // Create the DDS entities with the user listeners
        participant_ =
                DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                        &participant_listener_);
        ASSERT_NE(participant_, nullptr);

        TypeSupport type(new TopicDataTypeMock());
        type.register_type(participant_);
        topic_ = participant_->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
        ASSERT_NE(topic_, nullptr);

        publisher_ =
                participant_->create_publisher(PUBLISHER_QOS_DEFAULT, &publisher_listener_);
        ASSERT_NE(publisher_, nullptr);

        datawriter_ =
                publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &datawriter_listener_);
        ASSERT_NE(datawriter_, nullptr);

        subscriber_ =
                participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, &subscriber_listener_);
        ASSERT_NE(subscriber_, nullptr);

        EXPECT_CALL(participant_mock_,
                register_reader(&reader_mock_, ::testing::_, ::testing::_, nullptr)).WillRepeatedly(
            ::testing::Return(true));

        datareader_ =
                subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &datareader_listener_);
        ASSERT_NE(datareader_, nullptr);

    }

    void TearDown() override
    {
        ASSERT_EQ(publisher_->delete_datawriter(datawriter_), RETCODE_OK);
        ASSERT_EQ(participant_->delete_publisher(publisher_), RETCODE_OK);

        ASSERT_EQ(subscriber_->delete_datareader(datareader_), RETCODE_OK);
        ASSERT_EQ(participant_->delete_subscriber(subscriber_), RETCODE_OK);

        ASSERT_EQ(participant_->delete_topic(topic_), RETCODE_OK);

        ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant_), RETCODE_OK);
    }

    // RTPS entity mocks are nice, we don't want to track all calls
    NiceMock<RTPSParticipantMock> participant_mock_;
    NiceMock<RTPSWriterMock> writer_mock_;
    NiceMock<RTPSReaderMock> reader_mock_;

    // User listeners are strick, we want to track unexpected calls
    StrictMock<CustomParticipantListener> participant_listener_;
    StrictMock<CustomPublisherListener> publisher_listener_;
    StrictMock<CustomDataWriterListener> datawriter_listener_;
    StrictMock<CustomSubscriberListener> subscriber_listener_;
    StrictMock<CustomDataReaderListener> datareader_listener_;

    DomainParticipant* participant_;
    Publisher* publisher_;
    DataWriter* datawriter_;
    Subscriber* subscriber_;
    DataReader* datareader_;
    Topic* topic_;
};

void verify_expectations_on_publication_matched (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomPublisherListener>& publisher_listener_,
        StrictMock<CustomDataWriterListener>& datawriter_listener_)
{
    rtps::MatchingInfo status;

    RTPSDomain::writer_->listener_->on_writer_matched(nullptr, status);
    Mock::VerifyAndClearExpectations(&datawriter_listener_);
    Mock::VerifyAndClearExpectations(&publisher_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
}

TEST_F(UserListeners, publication_matched)
{


    datawriter_->set_listener(&datawriter_listener_, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datawriter_->set_listener(nullptr, StatusMask::all());

    publisher_->set_listener(&publisher_listener_, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(1);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    publisher_->set_listener(&publisher_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(1);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    publisher_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(1);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    publisher_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::publication_matched());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_publication_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_publication_matched_relay()).Times(0);
    verify_expectations_on_publication_matched(participant_listener_, publisher_listener_,
            datawriter_listener_);

}

void verify_expectations_on_offered_incompatible_qos (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomPublisherListener>& publisher_listener_,
        StrictMock<CustomDataWriterListener>& datawriter_listener_)
{
    PolicyMask status;

    RTPSDomain::writer_->listener_->on_offered_incompatible_qos(nullptr, status);
    Mock::VerifyAndClearExpectations(&datawriter_listener_);
    Mock::VerifyAndClearExpectations(&publisher_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
}

TEST_F(UserListeners, offered_incompatible_qos)
{
    datawriter_->set_listener(&datawriter_listener_, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datawriter_->set_listener(nullptr, StatusMask::all());

    publisher_->set_listener(&publisher_listener_, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(1);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    publisher_->set_listener(&publisher_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(1);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    publisher_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(1);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    publisher_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::offered_incompatible_qos());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_offered_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_offered_incompatible_qos_relay()).Times(0);
    verify_expectations_on_offered_incompatible_qos(participant_listener_, publisher_listener_,
            datawriter_listener_);
}

void verify_expectations_on_liveliness_lost (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomPublisherListener>& publisher_listener_,
        StrictMock<CustomDataWriterListener>& datawriter_listener_)
{
    LivelinessLostStatus status;

    RTPSDomain::writer_->listener_->on_liveliness_lost(nullptr, status);
    Mock::VerifyAndClearExpectations(&datawriter_listener_);
    Mock::VerifyAndClearExpectations(&publisher_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
}

TEST_F(UserListeners, liveliness_lost_test)
{
    datawriter_->set_listener(&datawriter_listener_, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(&datawriter_listener_, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    datawriter_->set_listener(nullptr, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datawriter_->set_listener(nullptr, StatusMask::all());

    publisher_->set_listener(&publisher_listener_, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(1);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    publisher_->set_listener(&publisher_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(1);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    publisher_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(1);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    publisher_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::liveliness_lost());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datawriter_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(publisher_listener_, on_liveliness_lost_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_lost_relay()).Times(0);
    verify_expectations_on_liveliness_lost(participant_listener_, publisher_listener_, datawriter_listener_);
}

void verify_expectations_on_subscription_matched (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomSubscriberListener>& subscriber_listener_,
        StrictMock<CustomDataReaderListener>& datareader_listener_)
{
    fastdds::rtps::MatchingInfo status;

    RTPSDomain::reader_->get_listener()->on_reader_matched(nullptr, status);
    Mock::VerifyAndClearExpectations(&datareader_listener_);
    Mock::VerifyAndClearExpectations(&subscriber_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
}

TEST_F(UserListeners, subscription_matched_test)
{
    datareader_->set_listener(&datareader_listener_, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datareader_->set_listener(nullptr, StatusMask::all());

    subscriber_->set_listener(&subscriber_listener_, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(1);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(1);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(1);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    subscriber_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::subscription_matched());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_subscription_matched_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_subscription_matched_relay()).Times(0);
    verify_expectations_on_subscription_matched(participant_listener_, subscriber_listener_,
            datareader_listener_);
}

void verify_expectations_on_liveliness_changed (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomSubscriberListener>& subscriber_listener_,
        StrictMock<CustomDataReaderListener>& datareader_listener_)
{
    LivelinessChangedStatus status;

    RTPSDomain::reader_->get_listener()->on_liveliness_changed(nullptr, status);
    Mock::VerifyAndClearExpectations(&datareader_listener_);
    Mock::VerifyAndClearExpectations(&subscriber_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
}

TEST_F(UserListeners, liveliness_changed_test)
{
    datareader_->set_listener(&datareader_listener_, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datareader_->set_listener(nullptr, StatusMask::all());

    subscriber_->set_listener(&subscriber_listener_, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(1);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(1);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(1);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    subscriber_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::liveliness_changed());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_liveliness_changed_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_liveliness_changed_relay()).Times(0);
    verify_expectations_on_liveliness_changed(participant_listener_, subscriber_listener_,
            datareader_listener_);
}

void verify_expectations_on_requested_incompatible_qos (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomSubscriberListener>& subscriber_listener_,
        StrictMock<CustomDataReaderListener>& datareader_listener_)
{
    PolicyMask status;

    RTPSDomain::reader_->get_listener()->on_requested_incompatible_qos(nullptr, status);
    Mock::VerifyAndClearExpectations(&datareader_listener_);
    Mock::VerifyAndClearExpectations(&subscriber_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
}

TEST_F(UserListeners, requested_incompatible_qos_test)
{
    // All statuses active
    datareader_->set_listener(&datareader_listener_, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datareader_->set_listener(nullptr, StatusMask::all());

    subscriber_->set_listener(&subscriber_listener_, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(1);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(1);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(1);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    subscriber_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::requested_incompatible_qos());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_requested_incompatible_qos_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_requested_incompatible_qos_relay()).Times(0);
    verify_expectations_on_requested_incompatible_qos(participant_listener_, subscriber_listener_,
            datareader_listener_);
}

void verify_expectations_on_data_available (
        StrictMock<CustomParticipantListener>& participant_listener_,
        StrictMock<CustomSubscriberListener>& subscriber_listener_,
        StrictMock<CustomDataReaderListener>& datareader_listener_)
{
    fastdds::rtps::CacheChange_t change;

    auto seq = change.sequenceNumber;
    bool notify_individual = false;

    EXPECT_CALL(*RTPSDomain::reader_->history_, get_change(_, _, _))
            .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(&change), testing::Return(true)));

    RTPSDomain::reader_->get_listener()->on_data_available(nullptr, change.writerGUID, seq, seq, notify_individual);

    Mock::VerifyAndClearExpectations(&datareader_listener_);
    Mock::VerifyAndClearExpectations(&subscriber_listener_);
    Mock::VerifyAndClearExpectations(&participant_listener_);
    Mock::VerifyAndClearExpectations(RTPSDomain::reader_->history_);
}

TEST_F(UserListeners, data_available)
{
    fastdds::rtps::CacheChange_t change;

    //data_on_readers has priority
    ////////////////////////////////////////////////////////////////////

    // Set all statuses active on the reader and participant_, see if they ever get called
    datareader_->set_listener(&datareader_listener_, StatusMask::all());
    participant_->set_listener(&participant_listener_, StatusMask::all());

    subscriber_->set_listener(&subscriber_listener_, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::all() >> StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::all() >> StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::all() >> StatusMask::data_on_readers());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(1);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    // If subscriber listener and participant_ listener are unavailable, nothing is called
    datareader_->set_listener(nullptr, StatusMask::all());
    subscriber_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);


    // If no data_on_readers, then try data_available
    ///////////////////////////////////////////////////////////////////////

    // Set all statuses active on the subscriber and participant_, except data_on_readers
    subscriber_->set_listener(&subscriber_listener_, StatusMask::all() >> StatusMask::data_on_readers());
    participant_->set_listener(&participant_listener_, StatusMask::all() >> StatusMask::data_on_readers());

    datareader_->set_listener(&datareader_listener_, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::data_available());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(&datareader_listener_, StatusMask::all() >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::data_available());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    datareader_->set_listener(nullptr, StatusMask::all() >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    // If both datareader_ and subscriber listeners are unavailable, the participant_ is called
    datareader_->set_listener(nullptr, StatusMask::all());

    subscriber_->set_listener(&subscriber_listener_, StatusMask::all()
            >> StatusMask::data_on_readers()
            >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(&subscriber_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    subscriber_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(1);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    // If participant_ listener is unavailable too, nothing gets called
    datareader_->set_listener(nullptr, StatusMask::all());
    subscriber_->set_listener(nullptr, StatusMask::all());

    participant_->set_listener(&participant_listener_, StatusMask::all()
            >> StatusMask::data_on_readers()
            >> StatusMask::data_available());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    participant_->set_listener(&participant_listener_, StatusMask::none());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);

    participant_->set_listener(nullptr, StatusMask::all());
    EXPECT_CALL(datareader_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_available_relay()).Times(0);
    EXPECT_CALL(subscriber_listener_, on_data_on_readers_relay()).Times(0);
    EXPECT_CALL(participant_listener_, on_data_on_readers_relay()).Times(0);
    verify_expectations_on_data_available(participant_listener_, subscriber_listener_, datareader_listener_);
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
