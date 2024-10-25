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

#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/reader/WriterProxy.h>
#include <rtps/reader/WriterProxy.cpp>
#include <rtps/resources/TimedEvent.h>

#include "../../common/operators.hpp"

namespace eprosima {
namespace fastdds {
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
            readerMock.getTimes().initial_acknack_delay.to_ns() / 1000000);
    wproxy.perform_initial_ack_nack();
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initial_acknack_delay.to_ns() * 2 / 1000000);
    wproxy.perform_initial_ack_nack();
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initial_acknack_delay.to_ns() * 4 / 1000000);

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
            readerMock.getTimes().initial_acknack_delay.to_ns() * 4 / 1000000);
    wproxy.perform_initial_ack_nack();
    EXPECT_EQ ( wproxy.initial_acknack_->getIntervalMilliSec(),
            readerMock.getTimes().initial_acknack_delay.to_ns() * 4 / 1000000);

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
