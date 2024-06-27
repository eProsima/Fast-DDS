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

#include <algorithm>
#include <random>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <rtps/common/ChangeComparison.hpp>
#include <rtps/reader/StatefulReader.hpp>


using namespace eprosima::fastdds;
using namespace ::rtps;
using namespace ::testing;
using namespace std;

class ReaderHistoryTests : public Test
{
protected:

    HistoryAttributes history_attr;
    ReaderHistory* history;
    StatefulReader* readerMock;
    RecursiveTimedMutex mutex;

    uint32_t num_writers = 2;
    uint32_t num_sequence_numbers = 2;
    uint32_t num_changes = num_writers * num_sequence_numbers;
    vector<CacheChange_t*> changes_list;

    ReaderHistoryTests()
    {
    }

    virtual ~ReaderHistoryTests()
    {
    }

    virtual void SetUp()
    {
        history_attr.memoryPolicy = MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
        history_attr.payloadMaxSize = 4;
        history_attr.initialReservedCaches = 10;
        history_attr.maximumReservedCaches = 20;

        history = new ReaderHistory(history_attr);
        readerMock = new StatefulReader(history, &mutex);

        //Create changes list
        uint32_t t = 0;
        for (uint32_t i = 1; i <= num_writers; i++)
        {
            GUID_t writer_guid = GUID_t(GuidPrefix_t::unknown(), i);

            for (uint32_t j = 1; j <= num_sequence_numbers; j++)
            {
                CacheChange_t* ch = new CacheChange_t(0);
                ch->writerGUID = writer_guid;
                ch->sequenceNumber = SequenceNumber_t(0, j);
                ch->sourceTimestamp = rtps::Time_t(0, t);

                changes_list.push_back(ch);

                t++;
            }
        }
    }

    virtual void TearDown()
    {
        for (CacheChange_t* ch : changes_list)
        {
            delete ch;
        }

        delete history;
        delete readerMock;
    }

};

TEST_F(ReaderHistoryTests, add_and_remove_changes)
{
    EXPECT_CALL(*readerMock, change_removed_by_history(_)).Times(num_changes).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*readerMock, release_cache(_)).Times(num_changes);

    for (uint32_t i = 0; i < num_changes; i++)
    {
        history->add_change(changes_list[i]);
        ASSERT_EQ(history->getHistorySize(), i + 1U);
    }

    for (uint32_t i = 0; i < num_changes; i++)
    {
        history->remove_change(changes_list[i]);
        ASSERT_EQ(history->getHistorySize(), num_changes - i - 1U);
    }
}

TEST_F(ReaderHistoryTests, remove_empty_history)
{
    EXPECT_CALL(*readerMock, change_removed_by_history(_)).Times(0);
    EXPECT_CALL(*readerMock, release_cache(_)).Times(0);

    CacheChange_t* ch = new CacheChange_t();
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    ASSERT_FALSE(history->remove_change(ch));

    delete ch;
}

TEST_F(ReaderHistoryTests, remove_null_cache_change)
{
    EXPECT_CALL(*readerMock, change_removed_by_history(_)).Times(0);
    EXPECT_CALL(*readerMock, release_cache(_)).Times(0);

    CacheChange_t* ch = nullptr;
    ASSERT_FALSE(history->remove_change(ch));
}

TEST_F(ReaderHistoryTests, cache_change_payload_max_size)
{
    uint32_t ch_payload_length = history_attr.payloadMaxSize + 1U;
    CacheChange_t* ch = new CacheChange_t();

    ch->serializedPayload.length = ch_payload_length;
    ch->writerGUID = GUID_t(GuidPrefix_t::unknown(), 1U);

    ASSERT_FALSE(history->add_change(ch));
    ASSERT_EQ(history->getHistorySize(), 0U);

    delete ch;
}

TEST_F(ReaderHistoryTests, get_change)
{
    for (uint32_t i = 0; i < num_changes; i++)
    {
        history->add_change(changes_list[i]);
    }

    ASSERT_EQ(history->getHistorySize(), num_changes);

    for (uint32_t i = 1; i <= num_writers; i++)
    {
        GUID_t writer_guid = GUID_t(GuidPrefix_t::unknown(), i);

        for (uint32_t j = 1; j <= num_sequence_numbers; j++)
        {
            SequenceNumber_t seq_number = SequenceNumber_t(0, j);

            CacheChange_t* ch = nullptr;
            ASSERT_TRUE(history->get_change(seq_number, writer_guid, &ch));
            ASSERT_EQ(ch->writerGUID, writer_guid);
            ASSERT_EQ(ch->sequenceNumber, seq_number);
        }
    }
}

TEST_F(ReaderHistoryTests, change_order)
{
    constexpr uint32_t n_writers = 4;
    constexpr uint32_t changes_per_writer = 5;
    std::vector<CacheChange_t*> changes;

    // Create changes list
    for (uint32_t i = 1; i <= n_writers; i++)
    {
        uint32_t ts = 0;
        GUID_t writer_guid = GUID_t(GuidPrefix_t::unknown(), i);

        for (uint32_t j = 1; j <= changes_per_writer; j++)
        {
            CacheChange_t* ch = new CacheChange_t(0);
            ch->writerGUID = writer_guid;
            ch->sequenceNumber = SequenceNumber_t(0, j);
            ch->sourceTimestamp = rtps::Time_t(0, ts);

            changes.push_back(ch);
            ts += i;
        }
    }

    size_t total_changes = changes.size();
    ASSERT_EQ(total_changes, n_writers * changes_per_writer);
    int num_removes = static_cast<int>(total_changes * total_changes);
    EXPECT_CALL(*readerMock, change_removed_by_history(_)).Times(num_removes).WillRepeatedly(Return(true));
    EXPECT_CALL(*readerMock, release_cache(_)).Times(num_removes);

    auto rng = std::default_random_engine{};
    for (size_t n = 0; n < changes.size(); ++n)
    {
        // Shuffle the generated samples
        std::shuffle(std::begin(changes), std::end(changes), rng);

        // Add samples to history
        for (CacheChange_t* ch : changes)
        {
            history->add_change(ch);
        }

        // Check full history order
        ASSERT_EQ(history->getHistorySize(), total_changes);
        for (auto it = history->changesBegin(); it != history->changesEnd(); ++it)
        {
            for (auto it2 = it + 1; it2 != history->changesEnd(); ++it2)
            {
                EXPECT_FALSE(eprosima::fastdds::rtps::history_order_cmp(*it2, *it));
            }
        }

        // Prepare for next iteration
        history->remove_all_changes();
        ASSERT_EQ(history->getHistorySize(), 0);
    }

    // Clean-up
    for (CacheChange_t* ch : changes)
    {
        delete ch;
    }
}

TEST_F(ReaderHistoryTests, get_min_change_from_writer)
{
    for (uint32_t i = 0; i < num_changes; i++)
    {
        history->add_change(changes_list[i]);
    }

    ASSERT_EQ(history->getHistorySize(), num_changes);

    CacheChange_t* ch = nullptr;
    GUID_t w1 = GUID_t(GuidPrefix_t::unknown(), 1U);
    ASSERT_TRUE(history->get_min_change_from(&ch, w1));
    ASSERT_EQ(ch->writerGUID, GUID_t(GuidPrefix_t::unknown(), 1U));
    ASSERT_EQ(ch->sequenceNumber, SequenceNumber_t(0, 1U));

    GUID_t w2 = GUID_t(GuidPrefix_t::unknown(), 2U);
    ASSERT_TRUE(history->get_min_change_from(&ch, w2));
    ASSERT_EQ(ch->writerGUID, GUID_t(GuidPrefix_t::unknown(), 2U));
    ASSERT_EQ(ch->sequenceNumber, SequenceNumber_t(0, 1U));
}

TEST_F(ReaderHistoryTests, get_min_change_from_writer_non_existent)
{
    GUID_t w1(GuidPrefix_t::unknown(), 1U);

    CacheChange_t* ch = nullptr;
    ASSERT_FALSE(history->get_min_change_from(&ch, w1));
}

TEST_F(ReaderHistoryTests, remove_changes_with_guid)
{
    for (uint32_t i = 0; i < num_changes; i++)
    {
        history->add_change(changes_list[i]);
    }

    EXPECT_CALL(*readerMock, change_removed_by_history(_)).Times(2).
            WillRepeatedly(Return(true));
    EXPECT_CALL(*readerMock, release_cache(_)).Times(2);

    ASSERT_EQ(history->getHistorySize(), num_changes);
    GUID_t w1 = GUID_t(GuidPrefix_t::unknown(), 1U);
    ASSERT_TRUE(history->remove_changes_with_guid(w1));

    ASSERT_EQ(history->getHistorySize(), num_changes - num_sequence_numbers);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
