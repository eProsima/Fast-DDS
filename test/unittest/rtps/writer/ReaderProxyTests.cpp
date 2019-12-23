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
namespace eprosima
{
namespace fastrtps
{
namespace rtps
{

TEST(ReaderProxyTests, find_change_test)
{
    //RemoteReaderAttributes rattr;
    StatefulWriter writerMock;
    WriterTimes wTimes;
    RemoteLocatorsAllocationAttributes alloc;
    VariableLengthDataLimits limits;
    ReaderProxy rproxy(wTimes, alloc, limits, &writerMock);

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
    VariableLengthDataLimits limits;
    ReaderProxy rproxy(wTimes, alloc, limits, &writerMock);

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
    VariableLengthDataLimits limits;
    ReaderProxy rproxy(wTimes, alloc, limits, &writerMock);

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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
