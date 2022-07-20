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

    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 1)), false);
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 2)), false);
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 3)), false);
    //rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 4)), false); // GAP
    //rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 5)), false); // GAP
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 6)), false);
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 7)), false);

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

    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 1)), false);
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 2)), false);
    rproxy.change_has_been_removed(SequenceNumber_t(0, 1));
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 3)), false);
    rproxy.change_has_been_removed(SequenceNumber_t(0, 2));
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 4)), false);

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

/*
 * Regression test
 * Error because async thread is not wake up for sending a GAP.
 * This is because requested_changes_set and perform_acknack_response functions only return true when some change is
 * change. Now we added are_there_gaps function to check when there are gaps in the places are needed.
 */
TEST(ReaderProxyTests, are_there_gaps)
{
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);

    ASSERT_FALSE(rproxy.are_there_gaps());
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 1)), false);
    ASSERT_FALSE(rproxy.are_there_gaps());
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 2)), false);
    ASSERT_FALSE(rproxy.are_there_gaps());
    rproxy.add_change(ChangeForReader_t(SequenceNumber_t(0, 3)), false);
    ASSERT_FALSE(rproxy.are_there_gaps());
    rproxy.change_has_been_removed(SequenceNumber_t(0, 2));
    ASSERT_TRUE(rproxy.are_there_gaps());
    rproxy.change_has_been_removed(SequenceNumber_t(0, 1));
    ASSERT_TRUE(rproxy.are_there_gaps());
    rproxy.change_has_been_removed(SequenceNumber_t(0, 3));
    ASSERT_FALSE(rproxy.are_there_gaps());
}

FragmentNumber_t mark_next_fragment_sent(
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
        return next_fragment;
    }

    bool was_last_fragment;
    rproxy.mark_fragment_as_sent_for_change(sequence_number, next_fragment, was_last_fragment);
    return next_fragment;
}

TEST(ReaderProxyTests, process_nack_frag_single_fragment_different_windows_test)
{
    constexpr FragmentNumber_t TOTAL_NUMBER_OF_FRAGMENTS = 400;
    constexpr uint16_t FRAGMENT_SIZE = 100;

    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq;
    seq.sequenceNumber = {0, 1};
    seq.serializedPayload.length = TOTAL_NUMBER_OF_FRAGMENTS * FRAGMENT_SIZE;
    seq.setFragmentSize(FRAGMENT_SIZE);

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
        ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber, i), i);
    }

    // Set the change status to UNSENT.
    rproxy.perform_acknack_response(nullptr);

    // The difference between the latest sent fragment and an undelivered fragment should also be higher than
    // the FragmentNumberSet_t size.
    constexpr FragmentNumber_t UNDELIVERED_FRAGMENT = 3;
    FragmentNumberSet_t undelivered_fragment_set(UNDELIVERED_FRAGMENT);
    undelivered_fragment_set.add(UNDELIVERED_FRAGMENT);

    rproxy.process_nack_frag({}, 1, seq.sequenceNumber, undelivered_fragment_set);

    // Nack data should be ignored: first, complete the sequential delivering of the remaining fragments.
    for (auto i = NUMBER_OF_SENT_FRAGMENTS + 1u; i <= TOTAL_NUMBER_OF_FRAGMENTS; ++i)
    {
        ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber, i), i);
    }

    // Mark the change as sent, i.e. all fragments were sent once.
    rproxy.from_unsent_to_status(seq.sequenceNumber, UNACKNOWLEDGED, false, true);

    // After the change is marked as delivered, nack data can be processed.
    rproxy.process_nack_frag({}, 2, seq.sequenceNumber, undelivered_fragment_set);

    // Now, send the fragments that were reported as undelivered.
    ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber,
            UNDELIVERED_FRAGMENT), UNDELIVERED_FRAGMENT);

    // All fragments are marked as sent.
    ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber,
            TOTAL_NUMBER_OF_FRAGMENTS + 1u), TOTAL_NUMBER_OF_FRAGMENTS + 1u);
}

TEST(ReaderProxyTests, process_nack_frag_multiple_fragments_different_windows_test)
{
    constexpr FragmentNumber_t TOTAL_NUMBER_OF_FRAGMENTS = 400;
    constexpr uint16_t FRAGMENT_SIZE = 100;

    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(wTimes, alloc, &writerMock);
    CacheChange_t seq;
    seq.sequenceNumber = {0, 1};
    seq.serializedPayload.length = TOTAL_NUMBER_OF_FRAGMENTS * FRAGMENT_SIZE;
    seq.setFragmentSize(FRAGMENT_SIZE);

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
        ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber, i), i);
    }

    // Set the change status to UNSENT.
    rproxy.perform_acknack_response(nullptr);

    // Handle the first portion of undelivered fragments.
    {
        std::vector<FragmentNumber_t> undelivered_fragments = {3, 6, 8};
        FragmentNumberSet_t undelivered_fragment_set(undelivered_fragments.front());
        for (auto fragment: undelivered_fragments)
        {
            undelivered_fragment_set.add(fragment);
        }
        rproxy.process_nack_frag({}, 1, seq.sequenceNumber, undelivered_fragment_set);

        // Nack data should be ignored: first, complete the sequential delivering of the remaining fragments.
        for (auto i = NUMBER_OF_SENT_FRAGMENTS + 1u; i <= TOTAL_NUMBER_OF_FRAGMENTS; ++i)
        {
            ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber, i), i);
        }

        // Mark the change as sent, i.e. all fragments were sent once.
        rproxy.from_unsent_to_status(seq.sequenceNumber, UNACKNOWLEDGED, false, true);

        rproxy.process_nack_frag({}, 2, seq.sequenceNumber, undelivered_fragment_set);

        // After the change is marked as delivered, nack data can be processed.
        for (auto fragment: undelivered_fragments)
        {
            ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber, fragment), fragment);
        }
    }

    // All fragments are marked as sent.
    ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber,
            TOTAL_NUMBER_OF_FRAGMENTS + 1u), TOTAL_NUMBER_OF_FRAGMENTS + 1u);

    // Handle undelivered fragments that are from a different window.
    {
        std::vector<FragmentNumber_t> undelivered_fragments = {301, 399};
        FragmentNumberSet_t undelivered_fragment_set(undelivered_fragments.front());
        for (auto fragment: undelivered_fragments)
        {
            undelivered_fragment_set.add(fragment);
        }
        rproxy.process_nack_frag({}, 3, seq.sequenceNumber, undelivered_fragment_set);

        // After the change is marked as delivered, nack data can be processed.
        for (auto fragment: undelivered_fragments)
        {
            ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber, fragment), fragment);
        }
    }

    // All fragments are marked as sent.
    ASSERT_EQ(mark_next_fragment_sent(rproxy, seq.sequenceNumber,
            TOTAL_NUMBER_OF_FRAGMENTS + 1u), TOTAL_NUMBER_OF_FRAGMENTS + 1u);
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
