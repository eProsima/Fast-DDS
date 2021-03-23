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
#include <fastrtps/attributes/TopicAttributes.h>

#include <statistics/types/types.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct MockListener : IListener
{
    MOCK_METHOD(void, on_statistics_data, (const Data& ), (override));
};

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 *  1. Creates dummy listener objects and associates them to RTPS entities of each kind covering
 *     the different possible cases: already registered, non-registered, already unregistered.
 *  2. Verify the different kinds of rtps layer callbacks are performed as expected.
 */
TEST(RTPSStatisticsTests, statistics_rpts_listener_management)
{
    using namespace std;
    using namespace fastrtps::rtps;

    logError(RTPS_STATISTICS, "Test fails because statistics api implementation is missing.");

    // create the entities
    uint32_t domain_id = 0;
    RTPSParticipantAttributes p_attr;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);
    ASSERT_NE(participant, nullptr);

    HistoryAttributes h_attr;
    h_attr.payloadMaxSize = 255;
    unique_ptr<WriterHistory> w_history(new WriterHistory(h_attr));
    unique_ptr<ReaderHistory> r_history(new ReaderHistory(h_attr));

    WriterAttributes w_attr;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, w_history.get());
    ASSERT_NE(nullptr, writer);

    ReaderAttributes r_att;
    RTPSReader* reader = RTPSDomain::createRTPSReader(participant, r_att, r_history.get());
    ASSERT_NE(nullptr, reader);

    // Check API add and remove interfaces
    {
        auto listener1 = make_shared<MockListener>();
        auto listener2 = make_shared<MockListener>();
        auto nolistener = listener1;
        nolistener.reset();

        EventKind kind = EventKind::PUBLICATION_THROUGHPUT;
        EventKind another_kind = EventKind::SUBSCRIPTION_THROUGHPUT;
        EventKind yet_another_kind = EventKind::NETWORK_LATENCY;

        // test the participant apis
        // + fails if no listener has been yet added
        EXPECT_FALSE(participant->remove_statistics_listener(listener1, kind));
        // + fails to add an empty listener
        EXPECT_FALSE(participant->add_statistics_listener(nolistener, kind));
        // + succeeds to add a new listener
        ASSERT_TRUE(participant->add_statistics_listener(listener1, kind));
        // + fails to add multiple times the same listener...
        EXPECT_FALSE(participant->add_statistics_listener(listener1, kind));
        //   ... unless it's associated to other entity
        EXPECT_TRUE(participant->add_statistics_listener(listener1, another_kind));
        // + fails if an unknown listener is removed
        EXPECT_FALSE(participant->remove_statistics_listener(listener2, kind));
        // + fails if a known listener is removed with a non registered entity
        EXPECT_FALSE(participant->remove_statistics_listener(listener1, yet_another_kind));
        // + succeeds to remove a known listener
        EXPECT_TRUE(participant->remove_statistics_listener(listener1, kind));
        EXPECT_TRUE(participant->remove_statistics_listener(listener1, another_kind));
        // + fails if a listener is already removed
        EXPECT_FALSE(participant->remove_statistics_listener(listener1, kind));
        // + The EventKind is an actual mask that allow register multiple entities simultaneously
        EXPECT_TRUE(participant->add_statistics_listener(listener1, static_cast<EventKind>(kind | another_kind)));
        // + When using a mask of multiple entities the return value succeeds only if all
        //   entity driven operations succeeds. The following operation must fail because one
        //   of the entities has not that registered listener
        EXPECT_FALSE(participant->remove_statistics_listener(listener1,
                static_cast<EventKind>(kind | another_kind | yet_another_kind)));

        // test the writer apis
        // + fails if no listener has been yet added
        EXPECT_FALSE(writer->remove_statistics_listener(listener1));
        // + fails to add an empty listener
        EXPECT_FALSE(writer->add_statistics_listener(nolistener));
        // + succeeds to add a new listener
        ASSERT_TRUE(writer->add_statistics_listener(listener1));
        // + fails to add multiple times the same listener
        EXPECT_FALSE(writer->add_statistics_listener(listener1));
        // + fails if an unknown listener is removed
        EXPECT_FALSE(writer->remove_statistics_listener(listener2));
        // + succeeds to remove a known listener
        EXPECT_TRUE(writer->remove_statistics_listener(listener1));
        // + fails if a listener is already removed
        EXPECT_FALSE(writer->remove_statistics_listener(listener1));

        // test the reader apis
        // + fails if no listener has been yet added
        EXPECT_FALSE(reader->remove_statistics_listener(listener1));
        // + fails to add an empty listener
        EXPECT_FALSE(reader->add_statistics_listener(nolistener));
        // + succeeds to add a new listener
        ASSERT_TRUE(reader->add_statistics_listener(listener1));
        // + fails to add multiple times the same listener
        EXPECT_FALSE(reader->add_statistics_listener(listener1));
        // + fails if an unknown listener is removed
        EXPECT_FALSE(reader->remove_statistics_listener(listener2));
        // + succeeds to remove a known listener
        EXPECT_TRUE(reader->remove_statistics_listener(listener1));
        // + fails if a listener is already removed
        EXPECT_FALSE(reader->remove_statistics_listener(listener1));
    }

    // Check PUBLICATION_THROUGHPUT and SUBSCRIPTION_THROUGHPUT callbacks are performed
    {
        using namespace ::testing;
        using namespace fastrtps;

        auto participant_listener = make_shared<MockListener>();
        auto writer_listener = make_shared<MockListener>();
        auto reader_listener = make_shared<MockListener>();
        ASSERT_TRUE(participant->add_statistics_listener(participant_listener, EventKind::DISCOVERED_ENTITY));
        ASSERT_TRUE(writer->add_statistics_listener(writer_listener));
        ASSERT_TRUE(reader->add_statistics_listener(reader_listener));

        // We must received the discovery time of each endpoint
        EXPECT_CALL(*writer_listener, on_statistics_data)
                .Times(2);

        // match writer and reader on a dummy topic
        TopicAttributes Tatt;
        Tatt.topicKind = NO_KEY;
        Tatt.topicDataType = "string";
        Tatt.topicName = "statisticsTopic";
        WriterQos Wqos;
        ReaderQos Rqos;
        participant->registerWriter(writer, Tatt, Wqos);
        participant->registerReader(reader, Tatt, Rqos);

        // Check callbacks on data exchange, at least, we must received:
        // + RTPSWriter: PUBLICATION_THROUGHPUT, RTPS_SENT, RESENT_DATAS,
        //               GAP_COUNT, DATA_COUNT, SAMPLE_DATAS & PHYSICAL_DATA
        //   optionally: ACKNACK_COUNT & NACKFRAG_COUNT
        EXPECT_CALL(*writer_listener, on_statistics_data)
                .Times(AtMost(7));

        // + RTPSReader: SUBSCRIPTION_THROUGHPUT, RTPS_LOST, DATA_COUNT,
        //               SAMPLE_DATAS & PHYSICAL_DATA
        //   optionally: HEARTBEAT_COUNT
        EXPECT_CALL(*reader_listener, on_statistics_data)
                .Times(AtMost(5));

        // exchange data
        uint32_t payloadMaxSize = h_attr.payloadMaxSize;
        auto writer_change = writer->new_change(
            [payloadMaxSize]() -> uint32_t
            {
                return payloadMaxSize;
            },
            ALIVE);
        ASSERT_NE(nullptr, writer_change);
        ASSERT_TRUE(w_history->add_change(writer_change));

        // wait for reception
        EXPECT_TRUE(reader->wait_for_unread_cache(Duration_t(5, 0)));

        // receive the sample
        CacheChange_t* reader_change = nullptr;
        ASSERT_TRUE(reader->nextUntakenCache(&reader_change, nullptr));

        EXPECT_TRUE(writer->release_change(writer_change));
        reader->releaseCache(reader_change);
        EXPECT_TRUE(writer->remove_statistics_listener(writer_listener));
        EXPECT_TRUE(reader->remove_statistics_listener(reader_listener));
    }

    // Remove the entities
    RTPSDomain::removeRTPSWriter(writer);
    RTPSDomain::removeRTPSReader(reader);
    RTPSDomain::removeRTPSParticipant(participant);
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
