// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define TEST_FRIENDS \
    FRIEND_TEST(WriterProxyAcknackTests, AcknackBackoff);

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/reader/WriterProxy.h>

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

TEST(WriterProxyAcknackTests, AcknackBackoff)
{
    WriterProxyData wattr( 4u, 1u );
    StatefulReader readerMock; // avoid annoying uninteresting call warnings

    // Testing the Timed events are properly configured
    EXPECT_CALL(readerMock, getEventResource()).Times(1u);
    WriterProxy wproxy(&readerMock, RemoteLocatorsAllocationAttributes(), ResourceLimitedContainerConfig());
    wproxy.start(wattr, SequenceNumber_t());

    // Simulate initial acknack and check that the current acknack timer is increased from the default
    SequenceNumberSet_t t1(SequenceNumber_t(0, 0));
    EXPECT_CALL(readerMock, simp_send_acknack(t1)).Times(2u);
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initialAcknackDelay.to_ns() / 1000000);
    wproxy.perform_initial_ack_nack();
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initialAcknackDelay.to_ns() * 2 / 1000000);
    wproxy.perform_initial_ack_nack();
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initialAcknackDelay.to_ns() * 4 / 1000000);

    // Simulate heartbeat reception and check if the delay cannot be updated again
    bool assert_liveliness = false;
    uint32_t heartbeat_count = 1;
    int32_t current_sample_lost = 0;

    wproxy.process_heartbeat(
        heartbeat_count,
        SequenceNumber_t(0, 1),
        SequenceNumber_t(0, 1),
        false,
        false,
        false,
        assert_liveliness,
        current_sample_lost);

    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initialAcknackDelay.to_ns() * 4 / 1000000);
    wproxy.perform_initial_ack_nack();
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initialAcknackDelay.to_ns() * 4 / 1000000);

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
