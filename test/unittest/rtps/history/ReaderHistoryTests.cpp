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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

using namespace eprosima::fastrtps::rtps;
using namespace ::testing;

TEST(ReaderHistoryTests, AddChange)
{
    HistoryAttributes history_attr;
    ReaderHistory* history = new ReaderHistory(history_attr);

    StatefulReader readerMock;
    std::recursive_timed_mutex mutex;
    history->set_reader(&readerMock, &mutex);

    CacheChange_t* ch = new CacheChange_t();
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    history->add_change(ch);

    ASSERT_TRUE(history->getHistorySize() == 1);
}

TEST(ReaderHistoryTests, RemoveChange)
{
    HistoryAttributes history_attr;
    ReaderHistory* history = new ReaderHistory(history_attr);

    StatefulReader readerMock;
    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(1).
            WillOnce(Return(false));

    std::recursive_timed_mutex mutex;
    history->set_reader(&readerMock, &mutex);

    CacheChange_t* ch = new CacheChange_t();
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    history->add_change(ch);
    history->remove_change(ch);

    ASSERT_TRUE(history->getHistorySize() == 0);
}

TEST(ReaderHistoryTests, GetRepeatedSequenceNumber)
{
    HistoryAttributes history_attr;
    ReaderHistory* history = new ReaderHistory(history_attr);

    StatefulReader readerMock;
//    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(1).
//            WillOnce(Return(false));

    std::recursive_timed_mutex mutex;
    history->set_reader(&readerMock, &mutex);

    GUID_t w1(GuidPrefix_t::unknown(), 1U);
    GUID_t w2(GuidPrefix_t::unknown(), 2U);

    SequenceNumber_t s1(0,1);
    SequenceNumber_t s2(0,2);

    CacheChange_t* w1_ch1 = new CacheChange_t();
    w1_ch1->writerGUID = w1;
    w1_ch1->sequenceNumber = s1;
    w1_ch1->sourceTimestamp = Time_t(0,1);

    CacheChange_t* w1_ch2 = new CacheChange_t();
    w1_ch2->writerGUID = w1;
    w1_ch2->sequenceNumber = s2;
    w1_ch2->sourceTimestamp = Time_t(0,2);

    // Same sequence number and later timestamp
    CacheChange_t* w2_ch1 = new CacheChange_t();
    w2_ch1->writerGUID = w2;
    w2_ch1->sequenceNumber = s1;
    w2_ch1->sourceTimestamp = Time_t(0,3);

    history->add_change(w1_ch1);
    history->add_change(w1_ch2);
    history->add_change(w2_ch1);

    CacheChange_t** change = nullptr;
    ASSERT_TRUE(history->get_change(s1, w2, change));
}

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
