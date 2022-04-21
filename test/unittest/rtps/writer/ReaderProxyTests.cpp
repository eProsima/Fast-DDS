// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <rtps/messages/RTPSGapBuilder.hpp>

//using namespace eprosima::fastrtps::rtps;
namespace eprosima {
namespace fastrtps {
namespace rtps {

TEST(ReaderProxyTests, find_change_test)
{
    //RemoteReaderAttributes rattr;
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq1; seq1.sequenceNumber = {0, 1};
    CacheChange_t seq2; seq2.sequenceNumber = {0, 2};
    CacheChange_t seq3; seq3.sequenceNumber = {0, 3};
    CacheChange_t seq6; seq6.sequenceNumber = {0, 6};
    CacheChange_t seq7; seq7.sequenceNumber = {0, 7};

    rproxy.add_change(ChangeForReader_t(&seq1), true, false);
    rproxy.add_change(ChangeForReader_t(&seq2), true, false);
    rproxy.add_change(ChangeForReader_t(&seq3), true, false);
    //rproxy.add_change(ChangeForReader_t(&seq4), false); // GAP
    //rproxy.add_change(ChangeForReader_t(&seq5), false); // GAP
    rproxy.add_change(ChangeForReader_t(&seq6), true, false);
    rproxy.add_change(ChangeForReader_t(&seq7), true, false);

    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 5)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 6)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 7)));

    rproxy.acked_changes_set(SequenceNumber_t(0, 3));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 5)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 6)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 7)));

    rproxy.acked_changes_set(SequenceNumber_t(0, 3)); // AGAIN SAME ACK
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 5)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 6)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 7)));

    rproxy.acked_changes_set(SequenceNumber_t(0, 5)); // AGAIN SAME ACK
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 5)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 6)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 7)));
}

TEST(ReaderProxyTests, find_change_removed_test)
{
    //RemoteReaderAttributes rattr;
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq1; seq1.sequenceNumber = {0, 1};
    CacheChange_t seq2; seq2.sequenceNumber = {0, 2};
    CacheChange_t seq3; seq3.sequenceNumber = {0, 3};
    CacheChange_t seq4; seq4.sequenceNumber = {0, 4};

    rproxy.add_change(ChangeForReader_t(&seq1), true, false);
    rproxy.add_change(ChangeForReader_t(&seq2), true, false);
    rproxy.change_has_been_removed(SequenceNumber_t(0, 1));
    rproxy.add_change(ChangeForReader_t(&seq3), true, false);
    rproxy.change_has_been_removed(SequenceNumber_t(0, 2));
    rproxy.add_change(ChangeForReader_t(&seq4), true, false);

    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));

    rproxy.acked_changes_set(SequenceNumber_t(0, 2));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
}

// Regression test for #13556 (Github #2423)
TEST(ReaderProxyTests, requested_changes_set_test)
{
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq1; seq1.sequenceNumber = {0, 1};
    CacheChange_t seq2; seq2.sequenceNumber = {0, 2};
    CacheChange_t seq3; seq3.sequenceNumber = {0, 3};
    CacheChange_t seq4; seq4.sequenceNumber = {0, 4};
    RTPSMessageGroup message_group(nullptr, false);
    RTPSGapBuilder gap_builder(message_group);

    ReaderProxyData reader_attributes(0, 0);
    reader_attributes.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    rproxy.start(reader_attributes);


    rproxy.add_change(ChangeForReader_t(&seq1), false, false);
    rproxy.add_change(ChangeForReader_t(&seq2), true, false);
    rproxy.add_change(ChangeForReader_t(&seq3), true, false);
    rproxy.add_change(ChangeForReader_t(&seq4), false, false);

    SequenceNumberSet_t set({0, 1});
    set.add({0, 1});
    set.add({0, 2});
    set.add({0, 3});
    set.add({0, 4});

    EXPECT_CALL(gap_builder, add(SequenceNumber_t(0, 1))).Times(1).WillOnce(testing::Return(true));
    EXPECT_CALL(gap_builder, add(SequenceNumber_t(0, 4))).Times(1).WillOnce(testing::Return(true));

    rproxy.requested_changes_set(set, gap_builder, {0, 1});
}

bool mark_next_fragment_sent(
        ReaderProxy& rproxy,
        SequenceNumber_t sequence_number,
        FragmentNumber_t expected_fragment)
{
    FragmentNumber_t next_fragment{expected_fragment};
    SequenceNumber_t gap_seq;
    bool need_reactivate_periodic_heartbeat;
    rproxy.change_is_unsent(sequence_number, next_fragment, gap_seq, need_reactivate_periodic_heartbeat);
    if (next_fragment != expected_fragment)
    {
        return false;
    }

    bool was_last_fragment;
    return rproxy.mark_fragment_as_sent_for_change(sequence_number, next_fragment, was_last_fragment);
}

TEST(ReaderProxyTests, process_nack_frag_single_fragment_test)
{
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq;
    seq.sequenceNumber = {0, 1};
    seq.serializedPayload.length = 100000;
    seq.setFragmentSize(100);

    RTPSMessageGroup message_group(nullptr, false);
    RTPSGapBuilder gap_builder(message_group);

    ReaderProxyData reader_attributes(0, 0);
    reader_attributes.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    rproxy.start(reader_attributes);

    ChangeForReader_t change(&seq);
    rproxy.add_change(change, true, false);

    SequenceNumberSet_t sequence_number_set({0, 1});
    sequence_number_set.add({0, 1});
    rproxy.from_unsent_to_status(seq.sequenceNumber, UNACKNOWLEDGED, false, false);
    rproxy.requested_changes_set(sequence_number_set, gap_builder, seq.sequenceNumber);

    // The number of sent fragments should be higher than the FragmentNumberSet_t size.
    constexpr FragmentNumber_t NUMBER_OF_SENT_FRAGMENTS = 259;

    for (auto i = 1u; i <= NUMBER_OF_SENT_FRAGMENTS; ++i)
    {
        ASSERT_TRUE(mark_next_fragment_sent(rproxy, seq.sequenceNumber, i));
    }

    // Set the change status to UNSENT.
    rproxy.perform_acknack_response(nullptr);

    // The difference between the latest sent fragment and an undelivered fragment should also be higher than
    // the FragmentNumberSet_t size.
    constexpr FragmentNumber_t UNDELIVERED_FRAGMENT = 3;

    FragmentNumberSet_t fragment_set(UNDELIVERED_FRAGMENT);
    fragment_set.add(UNDELIVERED_FRAGMENT);
    rproxy.process_nack_frag({}, 1, seq.sequenceNumber, fragment_set);

    ASSERT_TRUE(mark_next_fragment_sent(rproxy, seq.sequenceNumber, UNDELIVERED_FRAGMENT));
    ASSERT_TRUE(mark_next_fragment_sent(rproxy, seq.sequenceNumber, UNDELIVERED_FRAGMENT + 1u));
}

TEST(ReaderProxyTests, process_nack_frag_multiple_fragments_test)
{
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq;
    seq.sequenceNumber = {0, 1};
    seq.serializedPayload.length = 100000;
    seq.setFragmentSize(100);

    RTPSMessageGroup message_group(nullptr, false);
    RTPSGapBuilder gap_builder(message_group);

    ReaderProxyData reader_attributes(0, 0);
    reader_attributes.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    rproxy.start(reader_attributes);

    ChangeForReader_t change(&seq);
    rproxy.add_change(change, true, false);

    SequenceNumberSet_t sequence_number_set({0, 1});
    sequence_number_set.add({0, 1});
    rproxy.from_unsent_to_status(seq.sequenceNumber, UNACKNOWLEDGED, false, false);
    rproxy.requested_changes_set(sequence_number_set, gap_builder, seq.sequenceNumber);

    // The number of sent fragments should be higher than the FragmentNumberSet_t size.
    constexpr FragmentNumber_t NUMBER_OF_SENT_FRAGMENTS = 259;

    for (auto i = 1u; i <= NUMBER_OF_SENT_FRAGMENTS; ++i)
    {
        ASSERT_TRUE(mark_next_fragment_sent(rproxy, seq.sequenceNumber, i));
    }

    // Set the change status to UNSENT.
    rproxy.perform_acknack_response(nullptr);

    std::vector<FragmentNumber_t> undelivered_fragments = {3, 6, 8};

    FragmentNumberSet_t undelivered_fragment_set(undelivered_fragments.front());
    for (auto fragment : undelivered_fragments)
    {
        undelivered_fragment_set.add(fragment);
    }
    rproxy.process_nack_frag({}, 1, seq.sequenceNumber, undelivered_fragment_set);

    // Check that all undelivered fragments are chosen first for sending.
    for (auto fragment : undelivered_fragments)
    {
        ASSERT_TRUE(mark_next_fragment_sent(rproxy, seq.sequenceNumber, fragment));
    }

    // After the last undelivered fragment is sent, fragments for sent go sequentially.
    for (auto i = 1u; i < 3u; i++)
    {
        ASSERT_TRUE(mark_next_fragment_sent(rproxy, seq.sequenceNumber, undelivered_fragments.back() + i));
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
