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

#include <rtps/messages/RTPSGapBuilder.hpp>
#include <rtps/writer/ReaderProxy.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
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
#ifndef __QNXNTO__
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
    reader_attributes.reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
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
#endif // __QNXNTO__

FragmentNumber_t mark_next_fragment_sent(
        ReaderProxy& rproxy,
        SequenceNumber_t sequence_number,
        FragmentNumber_t expected_fragment)
{
    FragmentNumber_t next_fragment{expected_fragment};
    SequenceNumber_t gap_seq;
    SequenceNumber_t min_seq;
    bool need_reactivate_periodic_heartbeat;
    rproxy.change_is_unsent(sequence_number, next_fragment, gap_seq, min_seq, need_reactivate_periodic_heartbeat);
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
    reader_attributes.reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
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
    reader_attributes.reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
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

TEST(ReaderProxyTests, has_been_delivered_test)
{
    StatefulWriter writer_mock;
    WriterTimes w_times;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(w_times, alloc, &writer_mock);

    CacheChange_t seq1;
    CacheChange_t seq2;
    seq1.sequenceNumber = {0, 1};
    seq2.sequenceNumber = {0, 2};

    ReaderProxyData reader_attributes(0, 0);
    reader_attributes.reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
    rproxy.start(reader_attributes);

    auto expect_result = [&rproxy](SequenceNumber_t seq, bool delivered, bool should_be_found)
            {
                bool found = false;
                EXPECT_EQ(delivered, rproxy.has_been_delivered(seq, found));
                EXPECT_EQ(should_be_found, found);
            };

    // Add changes 1 and 2
    rproxy.add_change(ChangeForReader_t(&seq1), true, false);
    rproxy.add_change(ChangeForReader_t(&seq2), true, false);

    // None of them has been delivered
    expect_result(seq1.sequenceNumber, false, true);
    expect_result(seq2.sequenceNumber, false, true);

    // Change 1 is sent
    rproxy.from_unsent_to_status(seq1.sequenceNumber, UNACKNOWLEDGED, false, true);

    // Only change 1 has been delivered. Both are found
    expect_result(seq1.sequenceNumber, true, true);
    expect_result(seq2.sequenceNumber, false, true);

    // Change 1 is acknowledged
    rproxy.acked_changes_set(seq1.sequenceNumber + 1);

    // Only change 1 has been delivered. Only change 2 is found
    expect_result(seq1.sequenceNumber, true, false);
    expect_result(seq2.sequenceNumber, false, true);

    // Change in the future should return not delivered and not found
    expect_result({0, 3}, false, false);
}

// Test expectations regarding acknack count.
// Serves as a regression test for redmine issue #20729.
TEST(ReaderProxyTests, acknack_count)
{
    StatefulWriter writer_mock;
    WriterTimes w_times;
    RemoteLocatorsAllocationAttributes alloc;
    ReaderProxy rproxy(w_times, alloc, &writer_mock);

    ReaderProxyData reader_attributes(0, 0);
    reader_attributes.reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    rproxy.start(reader_attributes);

    // Check that the initial acknack count is 0.
    EXPECT_TRUE(rproxy.check_and_set_acknack_count(0u));
    // Check that it is not accepted twice.
    EXPECT_FALSE(rproxy.check_and_set_acknack_count(0u));
    // Check that it is accepted if it is incremented.
    EXPECT_TRUE(rproxy.check_and_set_acknack_count(1u));
    // Check that it is not accepted twice.
    EXPECT_FALSE(rproxy.check_and_set_acknack_count(1u));
    // Check that it is not accepted if it is decremented.
    EXPECT_FALSE(rproxy.check_and_set_acknack_count(0u));
    // Check that it is accepted if it has a big increment.
    EXPECT_TRUE(rproxy.check_and_set_acknack_count(100u));
    // Check that previous values are rejected.
    for (uint32_t i = 0; i <= 100u; ++i)
    {
        EXPECT_FALSE(rproxy.check_and_set_acknack_count(i));
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
