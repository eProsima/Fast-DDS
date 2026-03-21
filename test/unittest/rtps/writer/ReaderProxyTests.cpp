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

#include <fastdds/dds/core/ReturnCode.hpp>
#include <rtps/messages/RTPSGapBuilder.hpp>
#include <rtps/writer/ReaderProxy.hpp>
#include <rtps/writer/StatefulWriter.hpp>
#include <rtps/writer/StatefulWriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class Listener : public StatefulWriterListener
{
public:

    MOCK_METHOD(void, on_writer_resend_data, (
                const GUID_t&,
                const GUID_t&,
                const SequenceNumber_t&,
                uint32_t,
                const LocatorSelectorEntry&), (override));

    MOCK_METHOD(void, on_writer_data_acknowledged, (
                const GUID_t& writer_guid,
                const GUID_t& reader_guid,
                const SequenceNumber_t& sequence_number,
                uint32_t payload_length,
                const std::chrono::steady_clock::duration& ack_duration,
                const LocatorSelectorEntry& locators), (override));
};

using ListenerMock = ::testing::StrictMock<Listener>;
using testing::_;

TEST(ReaderProxyTests, find_change_test)
{
    //RemoteReaderAttributes rattr;
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ListenerMock listener;
    ReaderProxy rproxy(wTimes, alloc, &writerMock, &listener);
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

    // This would notify changes 1 and 2
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, SequenceNumber_t{0, 1}, _, _, _));
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, SequenceNumber_t{0, 2}, _, _, _));
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

    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, SequenceNumber_t{0, 3}, _, _, _));
    rproxy.acked_changes_set(SequenceNumber_t(0, 5));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 5)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 6)));
    ASSERT_FALSE(rproxy.change_is_acked(SequenceNumber_t(0, 7)));

    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, SequenceNumber_t{ 0, 6 }, _, _, _));
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, SequenceNumber_t{ 0, 7 }, _, _, _));
    rproxy.acked_changes_set(SequenceNumber_t(0, 8));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 1)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 2)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 3)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 4)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 5)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 6)));
    ASSERT_TRUE(rproxy.change_is_acked(SequenceNumber_t(0, 7)));
}

TEST(ReaderProxyTests, find_change_removed_test)
{
    //RemoteReaderAttributes rattr;
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    ListenerMock listener;
    ReaderProxy rproxy(wTimes, alloc, &writerMock, &listener);
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
    ListenerMock listener;
    ReaderProxy rproxy(wTimes, alloc, &writerMock, &listener);
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

static FragmentNumber_t mark_next_fragment_sent(
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
    ListenerMock listener;
    ReaderProxy rproxy(wTimes, alloc, &writerMock, &listener);
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
    constexpr uint32_t EXPECTED_RESENT_BYTES =
            FRAGMENT_SIZE * (TOTAL_NUMBER_OF_FRAGMENTS - NUMBER_OF_SENT_FRAGMENTS);
    EXPECT_CALL(listener, on_writer_resend_data(_, _, seq.sequenceNumber, EXPECTED_RESENT_BYTES, _));
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
    ListenerMock listener;
    ReaderProxy rproxy(wTimes, alloc, &writerMock, &listener);
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
    constexpr uint32_t EXPECTED_RESENT_BYTES =
            FRAGMENT_SIZE * (TOTAL_NUMBER_OF_FRAGMENTS - NUMBER_OF_SENT_FRAGMENTS);
    EXPECT_CALL(listener, on_writer_resend_data(_, _, seq.sequenceNumber, EXPECTED_RESENT_BYTES, _));
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
    ListenerMock listener;
    ReaderProxy rproxy(w_times, alloc, &writer_mock, &listener);

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
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, seq1.sequenceNumber, _, _, _));
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
    ListenerMock listener;
    ReaderProxy rproxy(w_times, alloc, &writer_mock, &listener);

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

static void check_fragments_resend_notification(
        ListenerMock& listener,
        ReaderProxy& rproxy,
        uint32_t& nackfrag_count,
        uint32_t fragment_size,
        const SequenceNumber_t& sequence_number,
        const std::vector<FragmentNumber_t>& fragments_to_request)
{
    if (fragments_to_request.empty())
    {
        return;
    }
    FragmentNumberSet_t fset(fragments_to_request.front());
    uint32_t n_fragments = 0;
    for (auto fragment: fragments_to_request)
    {
        n_fragments++;
        fset.add(fragment);
    }
    rproxy.process_nack_frag({}, nackfrag_count++, sequence_number, fset);
    EXPECT_CALL(listener, on_writer_resend_data(_, _, sequence_number, n_fragments * fragment_size, _));
    rproxy.perform_acknack_response(nullptr);
    bool was_last_fragment = false;
    for (auto fragment : fragments_to_request)
    {
        rproxy.mark_fragment_as_sent_for_change(sequence_number, fragment, was_last_fragment);
    }
    rproxy.from_unsent_to_status(sequence_number, UNACKNOWLEDGED, false, true);
}

TEST(ReaderProxyTests, listener_notification)
{
    RTPSMessageGroup message_group(nullptr, false);
    RTPSGapBuilder gap_builder(message_group);

    StatefulWriter writer_mock;
    WriterTimes w_times;
    RemoteLocatorsAllocationAttributes alloc;
    ListenerMock listener;
    ReaderProxy rproxy(w_times, alloc, &writer_mock, &listener);
    CacheChange_t seq1;
    seq1.sequenceNumber = {0, 1};
    seq1.serializedPayload.length = 100;

    ReaderProxyData reader_attributes(0, 0);
    reader_attributes.reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
    rproxy.start(reader_attributes);
    // Add change 1
    rproxy.add_change(ChangeForReader_t(&seq1), true, false);
    // Change 1 is sent
    rproxy.from_unsent_to_status(seq1.sequenceNumber, UNACKNOWLEDGED, false, true);

    // Change 1 is lost and requested for resend
    EXPECT_CALL(listener, on_writer_resend_data(_, _, seq1.sequenceNumber, 100, _));
    SequenceNumberSet_t set({ 0, 1 });
    set.add({ 0, 1 });
    rproxy.requested_changes_set(set, gap_builder, seq1.sequenceNumber);
    rproxy.perform_acknack_response(nullptr);

    // Change 1 is acknowledged
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, seq1.sequenceNumber, 100, _, _));
    rproxy.acked_changes_set(seq1.sequenceNumber + 1);

    // Add change 2
    seq1.sequenceNumber = {0, 2};
    rproxy.add_change(ChangeForReader_t(&seq1), true, false);
    // Change 2 is sent and automatically acknowledged
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, seq1.sequenceNumber, 100, _, _));
    rproxy.from_unsent_to_status(seq1.sequenceNumber, ACKNOWLEDGED, false, true);

    // Change 2 is removed
    rproxy.change_has_been_removed(seq1.sequenceNumber);

    // Add change 3 with big fragmented payload
    constexpr uint32_t LARGE_PAYLOAD_SIZE = 100000;
    constexpr uint16_t FRAGMENT_SIZE = 100;
    constexpr uint32_t MAX_FRAGMENTS_REQUESTED = 256;
    constexpr uint32_t TOTAL_NUMBER_OF_FRAGMENTS = LARGE_PAYLOAD_SIZE / FRAGMENT_SIZE;
    seq1.sequenceNumber = { 0, 3 };
    seq1.serializedPayload.length = LARGE_PAYLOAD_SIZE;
    seq1.setFragmentSize(FRAGMENT_SIZE);
    rproxy.add_change(ChangeForReader_t(&seq1), true, false);

    // Change 3 is sent
    for (auto i = 1u; i <= TOTAL_NUMBER_OF_FRAGMENTS; ++i)
    {
        ASSERT_EQ(mark_next_fragment_sent(rproxy, seq1.sequenceNumber, i), i);
    }
    rproxy.from_unsent_to_status(seq1.sequenceNumber, UNACKNOWLEDGED, false, true);

    // Change 3 is lost and requested for resend
    EXPECT_CALL(listener, on_writer_resend_data(_, _, seq1.sequenceNumber, MAX_FRAGMENTS_REQUESTED * FRAGMENT_SIZE, _));
    SequenceNumberSet_t set3({ 0, 3 });
    set3.add({ 0, 3 });
    rproxy.requested_changes_set(set3, gap_builder, seq1.sequenceNumber);
    rproxy.perform_acknack_response(nullptr);

    // Change 3 is sent
    for (auto i = 1u; i <= MAX_FRAGMENTS_REQUESTED; ++i)
    {
        ASSERT_EQ(mark_next_fragment_sent(rproxy, seq1.sequenceNumber, i), i);
    }
    rproxy.from_unsent_to_status(seq1.sequenceNumber, UNACKNOWLEDGED, false, true);

    // Resend all fragments individually
    uint32_t nackfrag_count = 1;
    for (FragmentNumber_t fn = 1; fn <= TOTAL_NUMBER_OF_FRAGMENTS; ++fn)
    {
        check_fragments_resend_notification(
            listener,
            rproxy,
            nackfrag_count,
            FRAGMENT_SIZE,
            seq1.sequenceNumber,
            { fn });
    }

    // Request 1 out of 33 fragments
    check_fragments_resend_notification(
        listener,
        rproxy,
        nackfrag_count,
        FRAGMENT_SIZE,
        seq1.sequenceNumber,
        { 1, 34, 67, 100, 133, 166, 199, 232 });

    // Increasing number of fragments from 1 to 32
    for (FragmentNumber_t i = 2; i <= 32; ++i)
    {
        std::vector<FragmentNumber_t> fragments;
        for (FragmentNumber_t j = 1; j <= i; ++j)
        {
            fragments.push_back(j);
        }
        check_fragments_resend_notification(
            listener,
            rproxy,
            nackfrag_count,
            FRAGMENT_SIZE,
            seq1.sequenceNumber,
            fragments);
    }

    // Change 3 is acknowledged
    EXPECT_CALL(listener, on_writer_data_acknowledged(_, _, seq1.sequenceNumber, LARGE_PAYLOAD_SIZE, _, _));
    rproxy.acked_changes_set(seq1.sequenceNumber + 1);
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
