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

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <statistics/types/types.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace statistics {

class TestListener : public fastdds::statistics::IListener
{
    public:

    void on_statistics_data(
            const fastdds::statistics::Data& ) override
    {
    }
};

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * Creates dummy listener objects and associates them to RTPS entities of each kind covering
 * the different possible cases: already registered, non-registered, already unregistered.
 */
TEST(RTPSStatisticsTests, statistics_rpts_listener_management)
{
    using namespace std;

    logError(RTPS_STATISTICS , "Test fails because statistics api implementation is missing.");

    // create the entities
    uint32_t domain_id = 0;
    RTPSParticipantAttributes p_attr;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);
    ASSERT_NE(participant, nullptr);

    HistoryAttributes h_attr;
    h_attr.payloadMaxSize = 255;
    WriterHistory* w_history = new WriterHistory(h_attr);
    ReaderHistory* r_history = new ReaderHistory(h_attr);

    WriterAttributes w_attr;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, w_history);
    ASSERT_NE(nullptr, writer);

    ReaderAttributes r_att;
    RTPSReader* reader = RTPSDomain::createRTPSReader(participant, r_att, r_history);
    ASSERT_NE(nullptr, reader);

    {
        auto listener1 = make_shared<TestListener>();
        auto listener2 = make_shared<TestListener>();
        auto nolistener = listener1;
        nolistener.reset();

        fastdds::statistics::EventKind kind =
            fastdds::statistics::EventKind::PUBLICATION_THROUGHPUT;
        fastdds::statistics::EventKind another_kind =
            fastdds::statistics::EventKind::SUBSCRIPTION_THROUGHPUT;
        fastdds::statistics::EventKind yet_another_kind =
            fastdds::statistics::EventKind::NETWORK_LATENCY;

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
        // + fails if a listener is already removed
        EXPECT_FALSE(participant->remove_statistics_listener(listener1, kind));

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

    // Remove the entities
    RTPSDomain::removeRTPSWriter(writer);
    RTPSDomain::removeRTPSReader(reader);
    RTPSDomain::removeRTPSParticipant(participant);
}

} // namespace statistics
} // namespace rtps
} // namespace fastrtps
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
