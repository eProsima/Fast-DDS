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

#include <utils/SystemInfo.hpp>

#include <fastrtps/attributes/LibrarySettingsAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <statistics/types/types.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <fastdds/statistics/IListeners.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct MockListener : IListener
{
    void on_statistics_data(
            const Data& data) override
    {
        switch (data._d())
        {
            case RTPS_SENT:
                on_rtps_sent(data.entity2locator_traffic());
                break;
            case HEARTBEAT_COUNT:
                on_heartbeat_count(data.entity_count());
                break;
            case ACKNACK_COUNT:
                on_acknack_count(data.entity_count());
                break;
            case DATA_COUNT:
                on_data_count(data.entity_count());
                break;
            default:
                break;
        }
    }

    MOCK_METHOD(void, on_rtps_sent, (const eprosima::fastdds::statistics::Entity2LocatorTraffic&));
    MOCK_METHOD(void, on_heartbeat_count, (const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD(void, on_acknack_count, (const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD(void, on_data_count, (const eprosima::fastdds::statistics::EntityCount&));
};

class RTPSStatisticsTests
    : public ::testing::Test
{
protected:

    fastrtps::rtps::WriterHistory* writer_history_ = nullptr;
    fastrtps::rtps::ReaderHistory* reader_history_ = nullptr;

    fastrtps::rtps::RTPSParticipant* participant_ = nullptr;
    fastrtps::rtps::RTPSWriter* writer_ = nullptr;
    fastrtps::rtps::RTPSReader* reader_ = nullptr;

    void create_endpoints(
            uint32_t payloadMaxSize,
            fastrtps::rtps::ReliabilityKind_t reliability_qos = fastrtps::rtps::ReliabilityKind_t::RELIABLE)
    {
        using namespace fastrtps::rtps;

        HistoryAttributes history_attributes;
        history_attributes.payloadMaxSize = payloadMaxSize;
        writer_history_ = new WriterHistory(history_attributes);
        reader_history_ = new ReaderHistory(history_attributes);

        WriterAttributes w_att;
        w_att.endpoint.reliabilityKind = reliability_qos;
        writer_ = RTPSDomain::createRTPSWriter(participant_, w_att, writer_history_);

        ReaderAttributes r_att;
        r_att.endpoint.reliabilityKind = reliability_qos;
        reader_ = RTPSDomain::createRTPSReader(participant_, r_att, reader_history_);
    }

    void match_endpoints(
            bool key,
            fastrtps::string_255 data_type,
            fastrtps::string_255 topic_name)
    {
        using namespace fastrtps;
        using namespace fastrtps::rtps;

        TopicAttributes Tatt;
        Tatt.topicKind = key ? TopicKind_t::WITH_KEY : TopicKind_t::NO_KEY;
        Tatt.topicDataType = data_type;
        Tatt.topicName = topic_name;

        WriterQos Wqos;
        Wqos.m_reliability.kind =
                RELIABLE ==
                writer_->getAttributes().reliabilityKind ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;

        ReaderQos Rqos;
        Rqos.m_reliability.kind =
                RELIABLE ==
                reader_->getAttributes().reliabilityKind ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;

        participant_->registerWriter(writer_, Tatt, Wqos);
        participant_->registerReader(reader_, Tatt, Rqos);
    }

    void destroy_endpoints()
    {
        using namespace fastrtps::rtps;

        if (nullptr != writer_ )
        {
            RTPSDomain::removeRTPSWriter(writer_);
            delete writer_history_;
            writer_ = nullptr;
            writer_history_ = nullptr;
        }

        if (nullptr != reader_)
        {
            RTPSDomain::removeRTPSReader(reader_);
            delete reader_history_;
            reader_ = nullptr;
            reader_history_ = nullptr;
        }
    }

public:

    static void SetUpTestSuite()
    {
        using namespace fastrtps;

        // Intraprocess must be disable in order to receive DATA callbacks
        LibrarySettingsAttributes att;
        att.intraprocess_delivery = INTRAPROCESS_OFF;
        xmlparser::XMLProfileManager::library_settings(att);
    }

    // Sets up the test fixture.
    void SetUp() override
    {
        using namespace fastrtps::rtps;

        // create the participant
        uint32_t domain_id = SystemInfo::instance().process_id() % 100;
        RTPSParticipantAttributes p_attr;
        participant_ = RTPSDomain::createParticipant(
            domain_id, true, p_attr);
    }

    // Tears down the test fixture.
    void TearDown() override
    {
        using namespace fastrtps::rtps;

        // Remove the endpoints
        destroy_endpoints();

        // Remove the participant
        RTPSDomain::removeRTPSParticipant(participant_);
    }

};

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * Creates dummy listener objects and associates them to RTPS entities of each kind covering
 * the different possible cases: already registered, non-registered, already unregistered.
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_management)
{
    // Check API add and remove interfaces
    using namespace std;
    using namespace fastrtps::rtps;

    // Create the testing endpoints
    create_endpoints(255);

    auto listener1 = make_shared<MockListener>();
    auto listener2 = make_shared<MockListener>();
    auto nolistener = listener1;
    nolistener.reset();

    EventKind kind = EventKind::PUBLICATION_THROUGHPUT;
    EventKind another_kind = EventKind::SUBSCRIPTION_THROUGHPUT;
    EventKind yet_another_kind = EventKind::NETWORK_LATENCY;

    // test the participant apis
    // + fails to remove an empty listener
    EXPECT_FALSE(participant_->remove_statistics_listener(nolistener, kind));
    // + fails if no listener has been yet added
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1, kind));
    // + fails to add an empty listener
    EXPECT_FALSE(participant_->add_statistics_listener(nolistener, kind));
    // + succeeds to add a new listener
    ASSERT_TRUE(participant_->add_statistics_listener(listener1, kind));
    // + fails to add multiple times the same listener...
    EXPECT_FALSE(participant_->add_statistics_listener(listener1, kind));
    //   ... unless it's associated to other entity
    EXPECT_TRUE(participant_->add_statistics_listener(listener1, another_kind));
    // + fails if an unknown listener is removed
    EXPECT_FALSE(participant_->remove_statistics_listener(listener2, kind));
    // + fails if a known listener is removed with a non registered entity
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1, yet_another_kind));
    // + succeeds to remove a known listener
    EXPECT_TRUE(participant_->remove_statistics_listener(listener1, kind));
    EXPECT_TRUE(participant_->remove_statistics_listener(listener1, another_kind));
    // + fails if a listener is already removed
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1, kind));
    // + The EventKind is an actual mask that allow register multiple entities simultaneously
    EXPECT_TRUE(participant_->add_statistics_listener(listener1, static_cast<EventKind>(kind | another_kind)));
    // + When using a mask of multiple entities the return value succeeds only if all
    //   entity driven operations succeeds. The following operation must fail because one
    //   of the entities has not that registered listener
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1,
            static_cast<EventKind>(kind | another_kind | yet_another_kind)));

    // test the writer apis
    // + fails to remove an empty listener
    EXPECT_FALSE(writer_->remove_statistics_listener(nolistener));
    // + fails if no listener has been yet added
    EXPECT_FALSE(writer_->remove_statistics_listener(listener1));
    // + fails to add an empty listener
    EXPECT_FALSE(writer_->add_statistics_listener(nolistener));
    // + succeeds to add a new listener
    ASSERT_TRUE(writer_->add_statistics_listener(listener1));
    // + fails to add multiple times the same listener
    EXPECT_FALSE(writer_->add_statistics_listener(listener1));
    // + fails if an unknown listener is removed
    EXPECT_FALSE(writer_->remove_statistics_listener(listener2));
    // + succeeds to remove a known listener
    EXPECT_TRUE(writer_->remove_statistics_listener(listener1));
    // + fails if a listener is already removed
    EXPECT_FALSE(writer_->remove_statistics_listener(listener1));

    // test the reader apis
    // + fails to remove an empty listener
    EXPECT_FALSE(reader_->remove_statistics_listener(nolistener));
    // + fails if no listener has been yet added
    EXPECT_FALSE(reader_->remove_statistics_listener(listener1));
    // + fails to add an empty listener
    EXPECT_FALSE(reader_->add_statistics_listener(nolistener));
    // + succeeds to add a new listener
    ASSERT_TRUE(reader_->add_statistics_listener(listener1));
    // + fails to add multiple times the same listener
    EXPECT_FALSE(reader_->add_statistics_listener(listener1));
    // + fails if an unknown listener is removed
    EXPECT_FALSE(reader_->remove_statistics_listener(listener2));
    // + succeeds to remove a known listener
    EXPECT_TRUE(reader_->remove_statistics_listener(listener1));
    // + fails if a listener is already removed
    EXPECT_FALSE(reader_->remove_statistics_listener(listener1));
}

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * - RTPS_SENT callbacks are performed
 * - DATA_COUNT callbacks are performed for DATA submessages
 * - ACKNACK_COUNT callbacks are performed
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_callbacks)
{
    using namespace ::testing;
    using namespace fastrtps;
    using namespace fastrtps::rtps;
    using namespace std;

    // Create the testing endpoints
    uint16_t length = 255;
    create_endpoints(length);

    // participant specific callbacks
    auto participant_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_listener, EventKind::RTPS_SENT));

    // writer callbacks through participant listener
    auto participant_writer_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_writer_listener, EventKind::DATA_COUNT));

    // writer specific callbacks
    auto writer_listener = make_shared<MockListener>();
    ASSERT_TRUE(writer_->add_statistics_listener(writer_listener));

    // writer callbacks through participant listener
    auto participant_reader_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_reader_listener, EventKind::ACKNACK_COUNT));

    // reader specific callbacks
    auto reader_listener = make_shared<MockListener>();
    ASSERT_TRUE(reader_->add_statistics_listener(reader_listener));

    // We must received the RTPS_SENT notifications
    EXPECT_CALL(*participant_listener, on_rtps_sent)
            .Times(AtLeast(1));

    // Check callbacks on data exchange, at least, we must received:
    // + RTPSWriter: PUBLICATION_THROUGHPUT, RESENT_DATAS,
    //               GAP_COUNT, DATA_COUNT, SAMPLE_DATAS & PHYSICAL_DATA
    //   optionally: NACKFRAG_COUNT
    EXPECT_CALL(*writer_listener, on_heartbeat_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_data_count)
            .Times(AtLeast(1));

    EXPECT_CALL(*participant_writer_listener, on_data_count)
            .Times(AtLeast(1));

    // + RTPSReader: SUBSCRIPTION_THROUGHPUT,
    //               SAMPLE_DATAS & PHYSICAL_DATA
    //   optionally: ACKNACK_COUNT
    EXPECT_CALL(*reader_listener, on_acknack_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_reader_listener, on_acknack_count)
            .Times(AtLeast(1));

    // match writer and reader on a dummy topic
    match_endpoints(false, "string", "statisticsSmallTopic");

    // exchange data
    auto writer_change = writer_->new_change(
        [length]() -> uint32_t
        {
            return length;
        },
        ALIVE);

    ASSERT_NE(nullptr, writer_change);

    {
        string str("https://github.com/eProsima/Fast-DDS.git");
        memcpy(writer_change->serializedPayload.data, str.c_str(), str.length());
        writer_change->serializedPayload.length = (uint32_t)str.length();
    }

    ASSERT_TRUE(writer_history_->add_change(writer_change));

    // wait for reception
    EXPECT_TRUE(reader_->wait_for_unread_cache(Duration_t(5, 0)));

    // receive the sample
    CacheChange_t* reader_change = nullptr;
    ASSERT_TRUE(reader_->nextUntakenCache(&reader_change, nullptr));

    // wait for acknowledgement
    EXPECT_TRUE(writer_->wait_for_all_acked(Duration_t(5, 0)));

    reader_->releaseCache(reader_change);

    EXPECT_TRUE(writer_->remove_statistics_listener(writer_listener));
    EXPECT_TRUE(reader_->remove_statistics_listener(reader_listener));

    EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener, EventKind::RTPS_SENT));
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_writer_listener, EventKind::DATA_COUNT));
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_reader_listener, EventKind::ACKNACK_COUNT));

}

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * - participant listeners management with late joiners
 * - DATA_COUNT callbacks with DATA_FRAGS are performed
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_callbacks_fragmented)
{
    using namespace ::testing;
    using namespace fastrtps;
    using namespace fastrtps::rtps;
    using namespace std;

    // writer callbacks through participant listener
    auto participant_listener = make_shared<MockListener>();
    EventKind mask = static_cast<EventKind>(EventKind::DATA_COUNT |
            EventKind::HEARTBEAT_COUNT | EventKind::ACKNACK_COUNT);
    ASSERT_TRUE(participant_->add_statistics_listener(participant_listener, mask));

    EXPECT_CALL(*participant_listener, on_data_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_heartbeat_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_acknack_count)
            .Times(AtLeast(1));

    // Create the testing endpoints
    uint16_t length = 65000;
    create_endpoints(length);

    // match writer and reader on a dummy topic
    match_endpoints(false, "chunk", "statisticsLargeTopic");

    // exchange data
    auto writer_change = writer_->new_change(
        [length]() -> uint32_t
        {
            return length;
        },
        ALIVE);

    ASSERT_NE(nullptr, writer_change);

    {
        memset(writer_change->serializedPayload.data, 'e', length);
        writer_change->serializedPayload.length = length;
        writer_change->setFragmentSize(length);
    }

    ASSERT_TRUE(writer_history_->add_change(writer_change));

    // wait for reception
    EXPECT_TRUE(reader_->wait_for_unread_cache(Duration_t(5, 0)));

    // receive the sample
    CacheChange_t* reader_change = nullptr;
    ASSERT_TRUE(reader_->nextUntakenCache(&reader_change, nullptr));

    // wait for acknowledgement
    EXPECT_TRUE(writer_->wait_for_all_acked(Duration_t(5, 0)));

    reader_->releaseCache(reader_change);

    EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener, mask));
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::Flush();
    return ret;
}
