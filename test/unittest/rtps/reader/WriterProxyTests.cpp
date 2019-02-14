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

#define TEST_FRIENDS \
    FRIEND_TEST(WriterProxyTests, MissingChangesUpdate); \
    FRIEND_TEST(WriterProxyTests, LostChangesUpdate); \
    FRIEND_TEST(WriterProxyTests, ReceivedChangeSet); \
    FRIEND_TEST(WriterProxyTests, IrrelevantChangeSet);

#include <fastrtps/rtps/reader/WriterProxy.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

//using namespace eprosima::fastrtps::rtps;
namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {

            TEST(WriterProxyTests, MissingChangesUpdate)
            {
                RemoteWriterAttributes wattr;
                StatefulReader readerMock;
                WriterProxy wproxy(wattr, &readerMock);

                // Update MISSING changes util sequence number 3.
                wproxy.missing_changes_update(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 0));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 3u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 1))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->getStatus(), ChangeFromWriterStatus_t::MISSING);

                // Add two UNKNOWN with sequence numberes 4 and 5.
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,4)));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,5)));

                // Update MISSING changes util sequence number 5.
                wproxy.missing_changes_update(SequenceNumber_t(0,5));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 0));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 5u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 1))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::MISSING);

                // Set all as received.
                wproxy.received_change_set(SequenceNumber_t(0, 1));
                wproxy.received_change_set(SequenceNumber_t(0, 2));
                wproxy.received_change_set(SequenceNumber_t(0, 3));
                wproxy.received_change_set(SequenceNumber_t(0, 4));
                wproxy.received_change_set(SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);

                // Try to update MISSING changes util sequence number 4.
                wproxy.missing_changes_update(SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);

                // Add three UNKNOWN changes with sequence number 6, 7 and 9.
                // Add one RECEIVED change with sequence number 8.
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 6)));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 7)));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 8)));
                wproxy.received_change_set(SequenceNumber_t(0, 8));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 9)));

                // Update MISSING changes util sequence number 8.
                wproxy.missing_changes_update(SequenceNumber_t(0, 8));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 4u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 9))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Update MISSING changes util sequence number 10.
                wproxy.missing_changes_update(SequenceNumber_t(0, 10));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 5u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 9))->getStatus(), ChangeFromWriterStatus_t::MISSING);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 10))->getStatus(), ChangeFromWriterStatus_t::MISSING);
            }

            TEST(WriterProxyTests, LostChangesUpdate)
            {
                RemoteWriterAttributes wattr;
                StatefulReader readerMock;
                WriterProxy wproxy(wattr, &readerMock);

                // Update LOST changes util sequence number 3.
                wproxy.lost_changes_update(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 2));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);

                // Add two UNKNOWN with sequence numberes 3 and 4.
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,3)));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,4)));

                // Update LOST changes util sequence number 5.
                wproxy.lost_changes_update(SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);

                // Try to update LOST changes util sequence number 4.
                wproxy.lost_changes_update(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);

                // Add two UNKNOWN changes with sequence number 5 and 8.
                // Add one MISSING change with sequence number 6.
                // Add one RECEIVED change with sequence number 7.
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 5)));
                ChangeFromWriter_t missing_aux_change_from_w(SequenceNumber_t(0, 6));
                missing_aux_change_from_w.setStatus(ChangeFromWriterStatus_t::MISSING);
                wproxy.changes_from_writer_.insert(missing_aux_change_from_w);
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 7)));
                wproxy.received_change_set(SequenceNumber_t(0, 7));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0, 8)));

                // Update LOST changes util sequence number 8.
                wproxy.lost_changes_update(SequenceNumber_t(0, 8));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 7));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 1u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Update LOST changes util sequence number 10.
                wproxy.lost_changes_update(SequenceNumber_t(0, 10));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 9));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);
            }

            TEST(WriterProxyTests, ReceivedChangeSet)
            {
                RemoteWriterAttributes wattr;
                StatefulReader readerMock;
                WriterProxy wproxy(wattr, &readerMock);

                // Set received change with sequence number 3.
                wproxy.received_change_set(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 0));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 3u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 1))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);

                // Add two UNKNOWN with sequence numberes 4 and 5.
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,4)));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,5)));

                // Set received change with sequence number 2
                wproxy.received_change_set(SequenceNumber_t(0, 2));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 0));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 5u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 1))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Set received change with sequence number 1
                wproxy.received_change_set(SequenceNumber_t(0, 1));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 2u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Try to update LOST changes util sequence number 3.
                wproxy.received_change_set(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 2u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Add received change with sequence number 6
                wproxy.received_change_set(SequenceNumber_t(0, 6));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 3u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);

                // Add received change with sequence number 8
                wproxy.received_change_set(SequenceNumber_t(0, 8));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 5u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);

                // Add received change with sequence number 4
                wproxy.received_change_set(SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 4u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);

                // Add received change with sequence number 5
                wproxy.received_change_set(SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 6));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 2u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);

                // Add received change with sequence number 7
                wproxy.received_change_set(SequenceNumber_t(0, 7));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 8));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);
            }

            TEST(WriterProxyTests, IrrelevantChangeSet)
            {
                RemoteWriterAttributes wattr;
                StatefulReader readerMock;
                WriterProxy wproxy(wattr, &readerMock);

                // Set irrelevant change with sequence number 3.
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 0));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 3u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 1))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->isRelevant(), false);

                // Add two UNKNOWN with sequence numberes 4 and 5.
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,4)));
                wproxy.changes_from_writer_.insert(ChangeFromWriter_t(SequenceNumber_t(0,5)));

                // Set irrelevant change with sequence number 2
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 2));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 0));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 5u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 1))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 2))->isRelevant(), false);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 3))->isRelevant(), false);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Set irrelevant change with sequence number 1
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 1));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 2u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Try to update LOST changes util sequence number 3.
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 2u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);

                // Add irrelevant change with sequence number 6
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 6));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 3u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->isRelevant(), false);

                // Add irrelevant change with sequence number 8
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 8));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 3));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 5u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 4))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->isRelevant(), false);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->isRelevant(), false);

                // Add irrelevant change with sequence number 4
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 4));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 4u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 5))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 6))->isRelevant(), false);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->isRelevant(), false);

                // Add irrelevant change with sequence number 5
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 5));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 6));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 2u);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 7))->getStatus(), ChangeFromWriterStatus_t::UNKNOWN);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->getStatus(), ChangeFromWriterStatus_t::RECEIVED);
                ASSERT_EQ(wproxy.changes_from_writer_.find(SequenceNumber_t(0, 8))->isRelevant(), false);

                // Add irrelevant change with sequence number 7
                wproxy.irrelevant_change_set(SequenceNumber_t(0, 7));
                ASSERT_EQ(wproxy.changes_from_writer_low_mark_, SequenceNumber_t(0, 8));
                ASSERT_EQ(wproxy.changes_from_writer_.size(), 0u);
            }

        } // namespace rtps
    } // namespace fastrtps
} // namespace eprosima

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
