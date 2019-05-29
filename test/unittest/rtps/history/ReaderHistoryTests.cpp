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

class ReaderHistoryTests : public Test
{
protected:
    HistoryAttributes history_attr;
    ReaderHistory* history;
    StatefulReader readerMock;
    std::recursive_timed_mutex mutex;

    ReaderHistoryTests() {}

    virtual ~ReaderHistoryTests() {}

    virtual void SetUp()
    {
        history_attr.memoryPolicy = MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
        history_attr.payloadMaxSize = 4;
        history_attr.initialReservedCaches = 10;
        history_attr.maximumReservedCaches = 20;

        history = new ReaderHistory(history_attr);
        history->set_reader(&readerMock, &mutex);
    }

    virtual void TearDown()
    {
        delete history;
    }

    void insert_cache_changes(int num_writers, int num_sequence_numbers)
    {
        int t=1;

        for(int i=1; i<=num_writers; i++)
        {
            GUID_t writer_guid = GUID_t(GuidPrefix_t::unknown(), i);

            for(int j=1; j<=num_sequence_numbers; j++)
            {
                CacheChange_t* ch = new CacheChange_t();
                ch->writerGUID = writer_guid;
                ch->sequenceNumber = SequenceNumber_t(0,j);;
                ch->sourceTimestamp = Time_t(0,t);
                t++;

                history->add_change(ch);
            }
        }
    }

    void insert_alternate_cache_changes(int num_writers, int num_sequence_numbers)
    {
        int t=1;

        for(int i=1; i<=num_sequence_numbers; i++)
        {
            SequenceNumber_t seq_number(0,i);

            for(int j=1; j<=num_writers; j++)
            {
                CacheChange_t* ch = new CacheChange_t();
                ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), (uint32_t)j);
                ch->sequenceNumber = seq_number;
                ch->sourceTimestamp = Time_t(0,t);
                t++;

                history->add_change(ch);
            }
        }
    }
};

TEST_F(ReaderHistoryTests, FailIfNoReaderOrMutex)
{
    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(0);

    CacheChange_t* ch = new CacheChange_t();
    GUID_t w1(GuidPrefix_t::unknown(), 1U);
    ch->writerGUID = w1;

    ASSERT_TRUE(history->add_change(ch));

    history->set_reader(nullptr, &mutex);
    ASSERT_FALSE(history->add_change(ch));
    ASSERT_FALSE(history->remove_change(ch));
    ASSERT_FALSE(history->remove_changes_with_guid(w1));

    history->set_reader(&readerMock, nullptr);
    ASSERT_FALSE(history->add_change(ch));
    ASSERT_FALSE(history->remove_change(ch));
    ASSERT_FALSE(history->remove_changes_with_guid(w1));

    history->set_reader(nullptr, nullptr);
    ASSERT_FALSE(history->add_change(ch));
    ASSERT_FALSE(history->remove_change(ch));
    ASSERT_FALSE(history->remove_changes_with_guid(w1));
}

TEST_F(ReaderHistoryTests, AddAndRemoveChanges)
{
    int num_changes = history_attr.initialReservedCaches;

    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(num_changes).
            WillRepeatedly(Return(true));

    CacheChange_t* ch = new CacheChange_t();
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    for(int i=0; i<num_changes; i++)
    {
        history->add_change(ch);
        ASSERT_EQ(history->getHistorySize(), i+1);
    }

    for(int i=0; i<num_changes; i++)
    {
        history->remove_change(ch);
        ASSERT_EQ(history->getHistorySize(), num_changes-i-1);
    }
}

TEST_F(ReaderHistoryTests, RemoveEmptyHistory)
{
    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(0);

    CacheChange_t* ch = new CacheChange_t();
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    ASSERT_FALSE(history->remove_change(ch));
}

TEST_F(ReaderHistoryTests, RemoveNullPointerCacheChange)
{
    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(0);

    CacheChange_t* ch = nullptr;
    ASSERT_FALSE(history->remove_change(ch));
}

TEST_F(ReaderHistoryTests, CacheChangePayloadMaxSize)
{
    int ch_payload_length = history_attr.payloadMaxSize + 1;
    CacheChange_t* ch = new CacheChange_t();

    ch->serializedPayload.length = ch_payload_length;
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    ASSERT_FALSE(history->add_change(ch));
    ASSERT_EQ(history->getHistorySize(), 0);
}

TEST_F(ReaderHistoryTests, GetChange)
{
    int num_writers = 2;
    int num_sequence_numbers = 2;
    int num_changes = num_writers * num_sequence_numbers;

    insert_cache_changes(num_writers, num_sequence_numbers);

    ASSERT_EQ(history->getHistorySize(), num_changes);

    for(int i=1; i<=num_writers; i++)
    {
        GUID_t writer_guid = GUID_t(GuidPrefix_t::unknown(), i);

        for(int j=1; j<=num_sequence_numbers; j++)
        {
            SequenceNumber_t seq_number = SequenceNumber_t(0,j);

            CacheChange_t* change = new CacheChange_t();
            ASSERT_TRUE(history->get_change(seq_number, writer_guid, &change));
            ASSERT_EQ(change->writerGUID, writer_guid);
            ASSERT_EQ(change->sequenceNumber, seq_number);
        }
    }
}

TEST_F(ReaderHistoryTests, GetChangeAlternatesSequences)
{
    int num_writers = 2;
    int num_sequence_numbers = 2;
    int num_changes = num_writers * num_sequence_numbers;

    insert_alternate_cache_changes(num_writers, num_sequence_numbers);

    ASSERT_EQ(history->getHistorySize(), num_changes);

    for(int i=1; i<=num_writers; i++)
    {
        GUID_t writer_guid = GUID_t(GuidPrefix_t::unknown(), i);

        for(int j=1; j<=num_sequence_numbers; j++)
        {
            SequenceNumber_t seq_number = SequenceNumber_t(0,j);

            CacheChange_t* change = new CacheChange_t();
            ASSERT_TRUE(history->get_change(seq_number, writer_guid, &change));
            ASSERT_EQ(change->writerGUID, writer_guid);
            ASSERT_EQ(change->sequenceNumber, seq_number);
        }
    }
}

TEST_F(ReaderHistoryTests, GetMinCacheFromwriter)
{
    insert_alternate_cache_changes(2, 2);

    ASSERT_EQ(history->getHistorySize(), 4);

    CacheChange_t* change = new CacheChange_t();
    GUID_t w1 = GUID_t(GuidPrefix_t::unknown(), 1U);
    ASSERT_TRUE(history->get_min_change_from(&change, w1));
    ASSERT_EQ(change->writerGUID, GUID_t(GuidPrefix_t::unknown(), 1U));
    ASSERT_EQ(change->sequenceNumber, SequenceNumber_t(0,1));

    GUID_t w2 = GUID_t(GuidPrefix_t::unknown(), 2U);
    ASSERT_TRUE(history->get_min_change_from(&change, w2));
    ASSERT_EQ(change->writerGUID, GUID_t(GuidPrefix_t::unknown(), 2U));
    ASSERT_EQ(change->sequenceNumber, SequenceNumber_t(0,1));
}

TEST_F(ReaderHistoryTests, GetMinCacheFromwriterReturnsFalse)
{
    GUID_t w1(GuidPrefix_t::unknown(), 1U);

    CacheChange_t* change = new CacheChange_t();
    ASSERT_FALSE(history->get_min_change_from(&change, w1));
}

TEST_F(ReaderHistoryTests, RemoveChangesWithGUID)
{
    insert_alternate_cache_changes(3, 2);

    EXPECT_CALL(readerMock, change_removed_by_history(_)).Times(2).
            WillRepeatedly(Return(true));

    ASSERT_EQ(history->getHistorySize(), 6);
    GUID_t w1 = GUID_t(GuidPrefix_t::unknown(), 1U);
    ASSERT_TRUE(history->remove_changes_with_guid(w1));

    ASSERT_EQ(history->getHistorySize(), 4);
}

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
