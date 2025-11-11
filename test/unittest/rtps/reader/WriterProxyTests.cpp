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

#include <unordered_set>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define TEST_FRIENDS \
    FRIEND_TEST(WriterProxyTests, MissingChangesUpdate); \
    FRIEND_TEST(WriterProxyTests, LostChangesUpdate); \
    FRIEND_TEST(WriterProxyTests, ReceivedChangeSet); \
    FRIEND_TEST(WriterProxyTests, IrrelevantChangeSet);

#include <rtps/reader/WriterProxy.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/resources/TimedEvent.h>

#include <rtps/reader/WriterProxy.cpp>

// Make SequenceNumberSet_t compatible with GMock macros

namespace testing {
namespace internal {
using namespace eprosima::fastrtps::rtps;

template<>
bool AnyEq::operator ()(
        const SequenceNumberSet_t& a,
        const SequenceNumberSet_t& b) const
{
    // remember that using SequenceNumberSet_t = BitmapRange<SequenceNumber_t, SequenceNumberDiff, 256>;
    // see test\unittest\utils\BitmapRangeTests.cpp method TestResult::Check

    if (a.empty() && b.empty())
    {
        return true;
    }

    if (a.base() == b.base())
    {
        uint32_t num_bits[2];
        uint32_t num_longs[2];
        SequenceNumberSet_t::bitmap_type bitmap[2];

        a.bitmap_get(num_bits[0], bitmap[0], num_longs[0]);
        b.bitmap_get(num_bits[1], bitmap[1], num_longs[1]);

        if (num_bits[0] != num_bits[1] || num_longs[0] != num_longs[1])
        {
            return false;
        }
        return std::equal(bitmap[0].cbegin(), bitmap[0].cbegin() + num_longs[0], bitmap[1].cbegin());
    }
    else
    {
        bool equal = true;

        a.for_each([&b, &equal](const SequenceNumber_t& e)
                {
                    equal &= b.is_set(e);
                });

        if (!equal)
        {
            return false;
        }

        b.for_each([&a, &equal](const SequenceNumber_t& e)
                {
                    equal &= a.is_set(e);
                });

        return equal;
    }
}

} // namespace internal
} // namespace testing

namespace eprosima {
namespace fastrtps {
namespace rtps {

TEST(WriterProxyTests, MissingChangesUpdate)
{
    using ::testing::ReturnRef;
    using ::testing::Eq;
    using ::testing::Ref;
    using ::testing::Const;

    WriterProxyData wattr( 4u, 1u );
    StatefulReader readerMock; // avoid annoying uninteresting call warnings

    // Testing the Timed events are properly configured
    WriterProxy wproxy(&readerMock, RemoteLocatorsAllocationAttributes(), ResourceLimitedContainerConfig());
    EXPECT_CALL(*wproxy.initial_acknack_, update_interval(readerMock.getTimes().initialAcknackDelay)).Times(1u);
    EXPECT_CALL(*wproxy.heartbeat_response_, update_interval(readerMock.getTimes().heartbeatResponseDelay)).Times(1u);
    EXPECT_CALL(*wproxy.initial_acknack_, restart_timer()).Times(1u);
    wproxy.start(wattr, SequenceNumber_t());

    // 1. Simulate initial acknack
    SequenceNumberSet_t t1(SequenceNumber_t(0, 0));
    EXPECT_CALL(readerMock, simp_send_acknack(t1)).Times(1u);
    wproxy.perform_initial_ack_nack();

    // 2. Simulate Writer initial HEARTBEAT if there is a single sample in writer's history
    // According to RTPS Wire spec 8.3.7.5.3 firstSN.value and lastSN.value cannot be 0 or negative
    // and lastSN.value < firstSN.value
    bool assert_liveliness = false;
    uint32_t heartbeat_count = 1;
    int32_t current_sample_lost = 0;
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 1),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    SequenceNumberSet_t t2 (SequenceNumber_t(0, 1));
    t2.add(SequenceNumber_t(0, 1));
    ASSERT_THAT(t2, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(), wproxy.available_changes_max());
    ASSERT_EQ(0u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 1)));
    ASSERT_EQ(1u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2)));
    ASSERT_EQ(0, current_sample_lost);

    // 3. Simulate reception of a HEARTBEAT after two more samples are added to the writer's history
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 3),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    t2.add(SequenceNumber_t(0, 2));
    t2.add(SequenceNumber_t(0, 3));
    ASSERT_THAT(t2, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(), wproxy.available_changes_max());
    ASSERT_EQ(0u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 1)));
    ASSERT_EQ(1u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2)));
    ASSERT_EQ(2u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 3)));
    ASSERT_EQ(3u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)));
    ASSERT_EQ(0, current_sample_lost);

    // 4. Simulate reception of a DATA(6).
    wproxy.received_change_set(SequenceNumber_t(0, 6));

    // According to the RTPS standard, sequence numbers 4 and 5 would be unknown,
    // but henceforth we don't differentiate between unknown and missing
    t2.add(SequenceNumber_t(0, 4));
    t2.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t2, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(), wproxy.available_changes_max());
    ASSERT_EQ(4u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 5)));
    ASSERT_EQ(5u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 6)));
    ASSERT_EQ(5u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)));
    ASSERT_EQ(0, current_sample_lost);

    // 5. Simulate reception of a HEARTBEAT(1,6)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 6),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    ASSERT_THAT(t2, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(), wproxy.available_changes_max());
    ASSERT_EQ(4u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 5)));
    ASSERT_EQ(5u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 6)));
    ASSERT_EQ(5u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)));
    ASSERT_EQ(0, current_sample_lost);

    // 5. Simulate reception of a HEARTBEAT(1,7)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 7),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    t2.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t2, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(), wproxy.available_changes_max());
    ASSERT_EQ(4u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 5)));
    ASSERT_EQ(5u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 6)));
    ASSERT_EQ(5u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)));
    ASSERT_EQ(6u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 8)));
    ASSERT_EQ(0, current_sample_lost);

    // 6. Simulate reception of all missing DATA
    wproxy.received_change_set(SequenceNumber_t(0, 1));
    wproxy.received_change_set(SequenceNumber_t(0, 2));
    wproxy.received_change_set(SequenceNumber_t(0, 3));
    wproxy.received_change_set(SequenceNumber_t(0, 4));
    wproxy.received_change_set(SequenceNumber_t(0, 5));

    SequenceNumberSet_t t6(SequenceNumber_t(0, 7));
    t6.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t6, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 6), wproxy.available_changes_max());
    ASSERT_EQ(0u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)));
    ASSERT_EQ(1u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 8)));
    ASSERT_EQ(0, current_sample_lost);

    // 7. Simulate reception of a faulty HEARTBEAT with a lower last sequence number (4)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 4),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    ASSERT_THAT(t6, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 6), wproxy.available_changes_max());
    ASSERT_EQ(0, current_sample_lost);

    // 8. Simulate reception of DATA(8) and DATA(10)
    wproxy.received_change_set(SequenceNumber_t(0, 8));
    wproxy.received_change_set( SequenceNumber_t(0, 10));

    // According to the RTPS standard, sequence numbers 7 and 9 would be unknown,
    // but henceforth we don't differentiate between unknown and missing
    t6.add(SequenceNumber_t(0, 7));
    t6.add(SequenceNumber_t(0, 9));
    ASSERT_THAT(t6, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 6), wproxy.available_changes_max());
    ASSERT_EQ(4u, wproxy.number_of_changes_from_writer());
    ASSERT_EQ(1u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)));
    ASSERT_EQ(2u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 11)));
    ASSERT_EQ(0, current_sample_lost);

    // 9. Simulate reception of HEARTBEAT(1,10)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t( 0, 1 ),
        SequenceNumber_t( 0, 10 ),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    t6.add(SequenceNumber_t(0, 6));
    t6.add(SequenceNumber_t(0, 7));
    t6.add(SequenceNumber_t(0, 9));
    ASSERT_THAT(t6, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 6), wproxy.available_changes_max());
    ASSERT_EQ(0, current_sample_lost);

    // 10. Simulate reception of HEARTBEAT(11,0)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t( 0, 11 ),
        SequenceNumber_t( 0, 0 ),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    ASSERT_EQ(SequenceNumber_t(0, 10), wproxy.available_changes_max());
    ASSERT_EQ(2, current_sample_lost);

}

TEST(WriterProxyTests, LostChangesUpdate)
{
    WriterProxyData wattr(4u, 1u);
    StatefulReader readerMock;
    WriterProxy wproxy(&readerMock, RemoteLocatorsAllocationAttributes(), ResourceLimitedContainerConfig());
    EXPECT_CALL(*wproxy.initial_acknack_, update_interval(readerMock.getTimes().initialAcknackDelay)).Times(1u);
    EXPECT_CALL(*wproxy.heartbeat_response_, update_interval(readerMock.getTimes().heartbeatResponseDelay)).Times(1u);
    EXPECT_CALL(*wproxy.initial_acknack_, restart_timer()).Times(1u);
    wproxy.start(wattr, SequenceNumber_t());

    // 1. Simulate reception of a HEARTBEAT(3,3)
    uint32_t heartbeat_count = 1;
    bool assert_liveliness = false;
    int32_t current_sample_lost = 0;
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 3),
        SequenceNumber_t(0, 3),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    SequenceNumberSet_t t1(SequenceNumber_t(0, 3));
    t1.add(SequenceNumber_t(0, 3));
    ASSERT_THAT(t1, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 2), wproxy.available_changes_max());
    ASSERT_EQ(1u, wproxy.number_of_changes_from_writer());
    ASSERT_EQ(1u, wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)));
    ASSERT_EQ(0, current_sample_lost);

    // 2. Simulate reception of a DATA(5) follow by a HEARTBEAT(5,5)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.received_change_set(SequenceNumber_t(0, 5));
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 5),
        SequenceNumber_t(0, 5),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    ASSERT_THAT( SequenceNumberSet_t(SequenceNumber_t( 0, 6)), wproxy.missing_changes());
    ASSERT_EQ( SequenceNumber_t( 0, 5 ), wproxy.available_changes_max());
    ASSERT_EQ( 0u, wproxy.number_of_changes_from_writer());
    ASSERT_EQ( 0u, wproxy.unknown_missing_changes_up_to( SequenceNumber_t( 0, 5 )));
    ASSERT_EQ(2, current_sample_lost);

    // 3. Simulate reception of a faulty HEARTBEAT with a lower first sequence number (4)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 4),
        SequenceNumber_t(0, 5),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    ASSERT_THAT(SequenceNumberSet_t( SequenceNumber_t(0, 6)), wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 5), wproxy.available_changes_max());
    ASSERT_EQ(0u, wproxy.unknown_missing_changes_up_to( SequenceNumber_t(0, 5)));
    ASSERT_EQ(0, current_sample_lost);

    // 4. Simulate reception of a DATA(7)
    // According to the RTPS standard, sequence number 5 would be missing and 6 would be unknown,
    // but henceforth we don't differentiate between unknown and missing thus we add 6 to the SequenceNumberSet_t
    wproxy.received_change_set( SequenceNumber_t(0, 7));

    SequenceNumberSet_t t4(SequenceNumber_t(0, 6));
    t4.add(SequenceNumber_t(0, 6));
    ASSERT_THAT(t4, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 5), wproxy.available_changes_max());
    ASSERT_EQ(2u, wproxy.number_of_changes_from_writer());
    ASSERT_EQ(1u, wproxy.unknown_missing_changes_up_to( SequenceNumber_t(0, 8)));

    // 5. Simulate reception of a HEARTBEAT(8,8)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 8),
        SequenceNumber_t(0, 8),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    SequenceNumberSet_t t5(SequenceNumber_t(0, 8));
    t5.add(SequenceNumber_t(0, 8));
    ASSERT_THAT(t5, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 7), wproxy.available_changes_max());
    ASSERT_EQ(1, current_sample_lost);

    // 6. Simulate reception of a HEARTBEAT(10,10)
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 10),
        SequenceNumber_t(0, 10),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    SequenceNumberSet_t t6(SequenceNumber_t(0, 10));
    t6.add(SequenceNumber_t(0, 10));
    ASSERT_THAT(t6, wproxy.missing_changes());
    ASSERT_EQ(SequenceNumber_t(0, 9), wproxy.available_changes_max());
    ASSERT_EQ(2, current_sample_lost);
}

TEST(WriterProxyTests, ReceivedChangeSet)
{
    WriterProxyData wattr(4u, 1u);
    StatefulReader readerMock;
    WriterProxy wproxy(&readerMock,
            RemoteLocatorsAllocationAttributes(),
            ResourceLimitedContainerConfig());

    /// Tests that initial acknack timed event is updated with new interval
    /// Tests that heartbeat response timed event is updated with new interval
    /// Tests that initial acknack timed event is started

    EXPECT_CALL(*wproxy.initial_acknack_, update_interval(readerMock.getTimes().initialAcknackDelay)).Times(1u);
    EXPECT_CALL(*wproxy.heartbeat_response_, update_interval(readerMock.getTimes().heartbeatResponseDelay)).Times(1u);
    EXPECT_CALL(*wproxy.initial_acknack_, restart_timer()).Times(1u);
    wproxy.start(wattr, SequenceNumber_t());

    // 1. Writer proxy receives DATA with sequence number 3
    // Sequence number 1 should be UNKNOWN
    // Sequence number 2 should be UNKNOWN
    // Sequence number 3 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 3));

    // According to the RTPS standard, sequence numbers 1 and 2 would be
    // unknown, but henceforth we don't differentiate between unknown and missing
    SequenceNumberSet_t t1(SequenceNumber_t(0, 1));
    t1.add(SequenceNumber_t(0, 1));
    t1.add(SequenceNumber_t(0, 2));
    ASSERT_THAT(t1, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 3u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)), 2u);

    // 2. Writer proxy receives DATA with sequence number 6
    // Sequence number 1 should be UNKNOWN
    // Sequence number 2 should be UNKNOWN
    // Sequence number 3 should be RECEIVED
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 6));

    // According to the RTPS standard, sequence numbers 1,2, 4 and 5 would be
    // unknown, but henceforth we don't differentiate between unknown and missing
    t1.add(SequenceNumber_t(0, 4));
    t1.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t1, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 6u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 4u);

    // 3. Writer proxy receives DATA with sequence number 2
    // Sequence number 1 should be UNKNOWN
    // Sequence number 2 should be RECEIVED
    // Sequence number 3 should be RECEIVED
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 2));

    // According to the RTPS standard, sequence numbers 1, 4 and 5 would be
    // unknown, but henceforth we don't differentiate between unknown and missing
    SequenceNumberSet_t t3(SequenceNumber_t(0, 1));
    t3.add(SequenceNumber_t(0, 1));
    t3.add(SequenceNumber_t(0, 4));
    t3.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t3, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 6u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 3u);

    // 4. Writer proxy receives DATA with sequence number 1
    // Sequence number 1 should be RECEIVED
    // Sequence number 2 should be RECEIVED
    // Sequence number 3 should be RECEIVED
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 1));

    // According to the RTPS standard, sequence numbers 4 and 5 would be
    // unknown, but henceforth we don't differentiate between unknown and missing
    SequenceNumberSet_t t4(SequenceNumber_t(0, 4));
    t4.add(SequenceNumber_t(0, 4));
    t4.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t4, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 3u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 2u);

    // 5. Simulate reception of a HEARTBEAT(4,6)
    // Sequence number 1 should be LOST
    // Sequence number 2 should be LOST
    // Sequence number 3 should be LOST
    // Sequence number 4 should be MISSING
    // Sequence number 5 should be MISSING
    // Sequence number 6 should be RECEIVED
    bool assert_liveliness = false;
    uint32_t heartbeat_count = 1;
    int32_t current_sample_lost = 0;
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 4),
        SequenceNumber_t(0, 6),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    ASSERT_THAT(t4, wproxy.missing_changes());
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 3u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 2u);
    ASSERT_EQ(0, current_sample_lost);

    // 6. Writer proxy receives DATA with sequence number 8
    // Sequence number 4 should be MISSING
    // Sequence number 5 should be MISSING
    // Sequence number 6 should be RECEIVED
    // Sequence number 7 should be UNKNOWN
    // Sequence number 8 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 8));

    // According to the RTPS standard, sequence numbers 4 and 5 should be MISSING and 7 should be UNKNOWN
    // but henceforth we don't differentiate between UNKNOWN and MISSING
    t4.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t4, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 5u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 3u);

    // 7. Writer proxy receives DATA with sequence number 4
    // Sequence number 4 should be RECEIVED
    // Sequence number 5 should be MISSING
    // Sequence number 6 should be RECEIVED
    // Sequence number 7 should be UNKNOWN
    // Sequence number 8 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 4));

    // According to the RTPS standard, sequence number 5 should be MISSING and 7 should be UNKNOWN
    // but henceforth we don't differentiate between UNKNOWN and MISSING
    SequenceNumberSet_t t7(SequenceNumber_t(0, 5));
    t7.add(SequenceNumber_t(0, 5));
    t7.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t7, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 4u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 2u);

    // 8. Writer proxy receives DATA with sequence number 5
    // Sequence number 5 should be RECEIVED
    // Sequence number 6 should be RECEIVED
    // Sequence number 7 should be UNKNOWN
    // Sequence number 8 should be RECEIVED
    wproxy.received_change_set(SequenceNumber_t(0, 5));

    // According to the RTPS standard, sequence number 7 should be UNKNOWN
    // but henceforth we don't differentiate between UNKNOWN and MISSING
    SequenceNumberSet_t t8(SequenceNumber_t(0, 7));
    t8.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t8, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 1u);

    // 9. Writer proxy receives DATA with sequence sequence number 7
    // No changes from writer
    wproxy.received_change_set(SequenceNumber_t(0, 7));

    ASSERT_THAT(SequenceNumberSet_t(), wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 0u);
}

TEST(WriterProxyTests, IrrelevantChangeSet)
{
    WriterProxyData wattr(4u, 1u);
    StatefulReader readerMock;
    WriterProxy wproxy(&readerMock, RemoteLocatorsAllocationAttributes(), ResourceLimitedContainerConfig());
    EXPECT_CALL(*wproxy.initial_acknack_, update_interval(readerMock.getTimes().initialAcknackDelay)).Times(1u);
    EXPECT_CALL(*wproxy.heartbeat_response_, update_interval(readerMock.getTimes().heartbeatResponseDelay)).Times(1u);
    EXPECT_CALL(*wproxy.initial_acknack_, restart_timer()).Times(1u);
    wproxy.start(wattr, SequenceNumber_t());

    // The lambda verifies that the same sequence number is not counted twice
    std::unordered_set<SequenceNumber_t, SequenceNumberHash> counted_sequence_numbers;
    auto validate_fn = [&counted_sequence_numbers](const SequenceNumber_t& seq)
            {
                EXPECT_FALSE(counted_sequence_numbers.count(seq) > 0);
                counted_sequence_numbers.insert(seq);
            };

    // 1. Simulate reception of a GAP message for sequence number 3.
    // Sequence number 1 should be UNKNOWN
    // Sequence number 2 should be UNKNOWN
    // Sequence number 3 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 3), SequenceNumberSet_t(SequenceNumber_t(0, 4)), validate_fn);

    // According to the RTPS standard, sequence numbers 1 and 2 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    SequenceNumberSet_t t1(SequenceNumber_t(0, 1));
    t1.add(SequenceNumber_t(0, 1));
    t1.add(SequenceNumber_t(0, 2));
    ASSERT_THAT(t1, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 3u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)), 2u);

    // 2. Simulate reception of a GAP message for sequence number 6.
    // Sequence number 1 should be UNKNOWN
    // Sequence number 2 should be UNKNOWN
    // Sequence number 3 should be RECEIVED with is_relevant = false
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 6), SequenceNumberSet_t(SequenceNumber_t(0, 7)), validate_fn);

    // According to the RTPS standard, sequence numbers 1, 2, 4 and 5 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    t1.add(SequenceNumber_t(0, 4));
    t1.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t1, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 6u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 4u);

    // 3. Simulate reception of a GAP message for sequence number 2.
    // Sequence number 1 should be UNKNOWN
    // Sequence number 2 should be RECEIVED with is_relevant = false
    // Sequence number 3 should be RECEIVED with is_relevant = false
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 2), SequenceNumberSet_t(SequenceNumber_t(0, 3)), validate_fn);

    // According to the RTPS standard, sequence numbers 1, 4 and 5 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    SequenceNumberSet_t t3(SequenceNumber_t(0, 1));
    t3.add(SequenceNumber_t(0, 1));
    t3.add(SequenceNumber_t(0, 4));
    t3.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t3, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 6u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 4)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 3u);

    // 4. Simulate reception of a GAP message for sequence number 1.
    // Sequence numbers 1, 2, and 3 should be removed from writer proxy
    // Sequence number 1 should be RECEIVED with is_relevant = false
    // Sequence number 2 should be RECEIVED with is_relevant = false
    // Sequence number 3 should be RECEIVED with is_relevant = false
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 1), SequenceNumberSet_t(SequenceNumber_t(0, 2)), validate_fn);

    // According to the RTPS standard, sequence numbers 4 and 5 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    SequenceNumberSet_t t4(SequenceNumber_t(0, 4));
    t4.add(SequenceNumber_t(0, 4));
    t4.add(SequenceNumber_t(0, 5));
    ASSERT_THAT(t4, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 3u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 2u);

    // 5. Simulate reception of a GAP message for sequence number 8.
    // Sequence number 4 should be UNKNOWN
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED with is_relevant = false
    // Sequence number 7 should be UNKNOWN
    // Sequence number 8 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 8), SequenceNumberSet_t(SequenceNumber_t(0, 9)), validate_fn);

    // According to the RTPS standard, sequence numbers 4, 5 and 7 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    t4.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t4, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 5u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 3u);

    // 6. Simulate reception of a GAP message for sequence number 4.
    // Sequence number 4 should be RECEIVED with is_relevant = false
    // Sequence number 5 should be UNKNOWN
    // Sequence number 6 should be RECEIVED with is_relevant = false
    // Sequence number 7 should be UNKNOWN
    // Sequence number 8 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 4), SequenceNumberSet_t(SequenceNumber_t(0, 5)), validate_fn);

    // According to the RTPS standard, sequence numbers 5 and 7 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    SequenceNumberSet_t t6(SequenceNumber_t(0, 1));
    t6.add(SequenceNumber_t(0, 5));
    t6.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t6, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 4u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 7)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 2u);

    // 7. Simulate reception of a GAP message for sequence number 5.
    // Sequence number 5 should be RECEIVED with is_relevant = false
    // Sequence number 6 should be RECEIVED with is_relevant = false
    // Sequence number 7 should be UNKNOWN
    // Sequence number 8 should be RECEIVED with is_relevant = false
    wproxy.process_gap(SequenceNumber_t(0, 5), SequenceNumberSet_t(SequenceNumber_t(0, 6)), validate_fn);

    // According to the RTPS standard, sequence number 7 should be UNKNOWN,
    // but henceforth we don't differentiate between UNKNOWN and MISSING.
    SequenceNumberSet_t t7(SequenceNumber_t(0, 7));
    t7.add(SequenceNumber_t(0, 7));
    ASSERT_THAT(t7, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 2u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 1u);

    // 8. Simulate reception of a GAP message for sequence number 7.
    // All sequence numbers received, no changes from writer
    wproxy.process_gap(SequenceNumber_t(0, 7), SequenceNumberSet_t(SequenceNumber_t(0, 8)), validate_fn);

    ASSERT_THAT(SequenceNumberSet_t(), wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 0u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), false);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 9)), 0u);

    // 9. Simulate reception of a HEARTBEAT(1,10008) after all changes have been marked as irrelevant
    bool assert_liveliness = false;
    uint32_t heartbeat_count = 1;
    int32_t current_sample_lost = 0;
    EXPECT_CALL(*wproxy.heartbeat_response_, restart_timer()).Times(1u);
    wproxy.process_heartbeat(
        heartbeat_count++,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 10008),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    SequenceNumberSet_t t9(SequenceNumber_t(0, 9));
    t9.add_range(SequenceNumber_t(0, 9), SequenceNumber_t(0, 10008));
    ASSERT_THAT(t9, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 10000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 10000u);

    // 10. Simulate reception of a GAP message for sequence numbers 1000 to 2000. GAP should be ignored
    wproxy.process_gap(SequenceNumber_t(0, 1000), SequenceNumberSet_t(SequenceNumber_t(0, 2001)), validate_fn);

    ASSERT_THAT(t9, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 10000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 10000u);

    // 11. Simulate reception of a GAP for sequence numbers 10, 20, 30
    // Sequence numbers 10, 20, and 30 should be RECEIVED with is_relevant = false
    {
        SequenceNumberSet_t gap_set(SequenceNumber_t(0, 11));
        gap_set.add(SequenceNumber_t(0, 20));
        gap_set.add(SequenceNumber_t(0, 30));
        wproxy.process_gap(SequenceNumber_t(0, 10), gap_set, validate_fn);
    }

    SequenceNumberSet_t t11(SequenceNumber_t(0, 9));
    t11.add_range(SequenceNumber_t(0, 9), SequenceNumber_t(0, 10008));
    t11.remove(SequenceNumber_t(0, 10));
    t11.remove(SequenceNumber_t(0, 20));
    t11.remove(SequenceNumber_t(0, 30));
    ASSERT_THAT(t11, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 10000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 9997u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 11)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 20)), 10u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 21)), 10u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 30)), 19u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 31)), 19u);

    // 12. Simulate reception of a GAP for sequence numbers 100, 200, 300
    // Sequence numbers 100, and 200 should be RECEIVED with is_relevant = false
    // GAP for sequence number 300 should be ignored
    {
        SequenceNumberSet_t gap_set(SequenceNumber_t(0, 101));
        gap_set.add(SequenceNumber_t(0, 200));
        gap_set.add(SequenceNumber_t(0, 300));
        wproxy.process_gap(SequenceNumber_t(0, 100), gap_set, validate_fn);
    }

    SequenceNumberSet_t t12(t11);
    t12.remove(SequenceNumber_t(0, 100));
    t12.remove(SequenceNumber_t(0, 200));
    ASSERT_THAT(t12, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 10000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 9995u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 100)), 88u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 101)), 88u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 200)), 187u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 201)), 187u);

    // 13. Simulate reception of a GAP for sequence numbers 9 - 1008
    // All sequence numbers below 1009 considered RECEIVED, so first missing is 1009
    wproxy.process_gap(SequenceNumber_t(0, 9), SequenceNumberSet_t(SequenceNumber_t(0, 1009)), validate_fn);

    SequenceNumberSet_t t13(SequenceNumber_t(0, 1009));
    t13.add_range(SequenceNumber_t(0, 1009), SequenceNumber_t(0, 10008));
    ASSERT_THAT(t13, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 9000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 9000u);

    // 14. Simulate reception of a GAP for sequence numbers 1000 - 1008. GAP should be ignored
    wproxy.process_gap(SequenceNumber_t(0, 1000), SequenceNumberSet_t(SequenceNumber_t(0, 1009)), validate_fn);

    ASSERT_THAT(t13, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 9000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 9000u);

    // 15. Simulate reception of a GAP for sequence numbers 1009 - 2008, 2010 - 3008
    // All sequence numbers below 2009 considered RECEIVED, so first missing is 2009
    // Sequence numbers 2010 - 2265 should be RECEIVED with is_relevant = false
    {
        SequenceNumberSet_t gap_set(SequenceNumber_t(0, 2009));
        gap_set.add_range(SequenceNumber_t(0, 2010), SequenceNumber_t(0, 3009));
        wproxy.process_gap(SequenceNumber_t(0, 1009), gap_set, validate_fn);
    }

    SequenceNumberSet_t t15(SequenceNumber_t(0, 2009));
    t15.add(SequenceNumber_t(0, 2009));
    ASSERT_THAT(t15, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 8000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 8000u - 255u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009)), 0u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2010)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009 + 256)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009 + 257)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 3009)), 1000u - 255u);

    // 16. Simulate reception of a GAP for sequence 2009 + 256
    // Should be ignored because it exceeds the maximum allowed GAP limit
    {
        SequenceNumber_t gap_start(0, 2009 + 256);
        wproxy.process_gap(gap_start, SequenceNumberSet_t(gap_start + 1), validate_fn);
    }

    ASSERT_THAT(t15, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 8000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 8000u - 255u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009)), 0u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2010)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009 + 256)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009 + 257)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 3009)), 1000u - 255u);

    // 17. Simulate reception of a GAP for sequence numbers 2009 + 255
    // Should be ignored because it is exactly the maximum allowed GAP limit
    {
        SequenceNumber_t gap_start(0, 2009 + 255);
        wproxy.process_gap(gap_start, SequenceNumberSet_t(gap_start + 1), validate_fn);
    }

    ASSERT_THAT(t15, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 8000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 8000u - 255u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009)), 0u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2010)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009 + 256)), 1u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 2009 + 257)), 2u);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 3009)), 1000u - 255u);

    // 18. Simulate reception of a GAP for sequence numbers 1 - 3008
    // All sequence numbers below 3009 considered RECEIVED, so first missing is 3009
    wproxy.process_gap(SequenceNumber_t(0, 1), SequenceNumberSet_t(SequenceNumber_t(0, 3009)), validate_fn);

    SequenceNumberSet_t t18(SequenceNumber_t(0, 3009));
    t18.add_range(SequenceNumber_t(0, 3009), SequenceNumber_t(0, 10008));
    ASSERT_THAT(t18, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 7000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 7000u);

    // 19. Simulate reception of a GAP for sequence 3009 + 256
    // Should be ignored because it exceeds the maximum allowed GAP limit
    {
        SequenceNumber_t gap_start(0, 3009 + 256);
        wproxy.process_gap(gap_start, SequenceNumberSet_t(gap_start + 1), validate_fn);
    }

    ASSERT_THAT(t18, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 7000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 7000u);

    // 20. Simulate reception of a GAP for sequence 3009 + 255
    // Should be ignored because it is exactly the maximum allowed GAP limit
    {
        SequenceNumber_t gap_start(0, 3009 + 255);
        wproxy.process_gap(gap_start, SequenceNumberSet_t(gap_start + 1), validate_fn);
    }

    ASSERT_THAT(t18, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 7000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 7000u);

    // 21. Simulate reception of a GAP for sequence 3009 + 254
    // Sequence number 3009 + 254 should be RECEIVED with is_relevant = false
    {
        SequenceNumber_t gap_start(0, 3009 + 254);
        wproxy.process_gap(gap_start, SequenceNumberSet_t(gap_start + 1), validate_fn);
    }

    t18.remove(SequenceNumber_t(0, 3009 + 254));
    ASSERT_THAT(t18, wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 7000u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), true);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 6999u);

    // 22. Simulate reception of a GAP for the full range (1 - 10008)
    // All sequence numbers considered RECEIVED, no changes from writer
    wproxy.process_gap(SequenceNumber_t(0, 1), SequenceNumberSet_t(SequenceNumber_t(0, 10009)), validate_fn);
    ASSERT_THAT(SequenceNumberSet_t(), wproxy.missing_changes());
    ASSERT_EQ(wproxy.number_of_changes_from_writer(), 0u);
    ASSERT_EQ(wproxy.are_there_missing_changes(), false);
    ASSERT_EQ(wproxy.unknown_missing_changes_up_to(SequenceNumber_t(0, 10009)), 0u);
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
