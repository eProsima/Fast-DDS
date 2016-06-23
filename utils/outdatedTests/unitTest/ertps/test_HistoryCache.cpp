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

/**
 * @file test_HistoryCache.cpp
 *
 *  Created on: Apr 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */
#include "gtest/gtest.h"
#include "fastrtps/HistoryCache.h"


using namespace eprosima::rtps;
class HistoryCacheTest:public ::testing::Test
{
protected:
	HistoryCacheTest():
		historySize(4),
		payloadSize(50),
		history(NULL)
		{

		}

	void SetUp(){

	}
	void createHistory(HistoryKind_t kind)
	{
		history = new HistoryCache(historySize,payloadSize,kind,NULL);
	}

	uint16_t historySize;
	uint16_t payloadSize;
	HistoryCache* history;
	void TearDown()
	{
		delete(history);
	}
};


TEST_F(HistoryCacheTest,Creation)
{
	EXPECT_NO_THROW(createHistory(READER));
	EXPECT_EQ(0,history->getHistorySize());
	EXPECT_FALSE(history->isFull());
}

TEST_F(HistoryCacheTest,ReserveAndRelease)
{
	createHistory(WRITER);
	CacheChange_t* change = NULL;
	EXPECT_NO_THROW(change = history->reserve_Cache());
	EXPECT_NO_THROW(history->release_Cache(change));
}


TEST_F(HistoryCacheTest,AddChangeWriter)
{
	createHistory(WRITER);
	for(uint i = 0;i<historySize;i++)
	{
		CacheChange_t* change = history->reserve_Cache();
		ASSERT_TRUE(change!=NULL);
		change->sequenceNumber.low = i+1;
		EXPECT_TRUE(history->add_change(change));
	}
	CacheChange_t* change = history->reserve_Cache();
	change->sequenceNumber.low = historySize+1;
	EXPECT_FALSE(history->add_change(change));
	EXPECT_EQ(historySize,history->getHistorySize());
	EXPECT_TRUE(history->isFull());
}


TEST_F(HistoryCacheTest,AddChangeReader)
{
	createHistory(READER);
	CacheChange_t* change = history->reserve_Cache();
	change->sequenceNumber.low = 1;
	EXPECT_TRUE(history->add_change(change));
	EXPECT_FALSE(history->add_change(change));
}

TEST_F(HistoryCacheTest,RemoveChange)
{
	createHistory(WRITER);
	for(uint i = 0;i<historySize;i++)
	{
		CacheChange_t* change = history->reserve_Cache();
		ASSERT_TRUE(change!=NULL);
		EXPECT_TRUE(history->add_change(change));
	}
	SequenceNumber_t seq;
	seq.low = 2;
	GUID_t guid;
	GUID_UNKNOWN(guid);
	size_t siz = history->getHistorySize();
	EXPECT_TRUE(history->remove_change(seq,guid));
	EXPECT_FALSE(history->remove_change(seq,guid));
	EXPECT_EQ(siz-1,history->getHistorySize());
	std::vector<CacheChange_t*>::iterator it = history->m_changes.begin();
	siz = history->getHistorySize();
	EXPECT_TRUE(history->remove_change(it));
	EXPECT_EQ(siz-1,history->getHistorySize());
}

TEST_F(HistoryCacheTest,GetChange)
{
	createHistory(WRITER);
	for(uint i = 0;i<historySize;i++)
	{
		CacheChange_t* change = history->reserve_Cache();
		ASSERT_TRUE(change!=NULL);
		EXPECT_TRUE(history->add_change(change));
	}
	CacheChange_t* change2;
	SequenceNumber_t seq;
	GUID_t guid;
	seq.low = 2;
	ASSERT_TRUE(history->get_change(seq,guid,&change2));
	EXPECT_EQ(2,change2->sequenceNumber.to64long());
	ASSERT_TRUE(history->get_last_added_cache(&change2));
	EXPECT_EQ(historySize,change2->sequenceNumber.to64long());
}

TEST_F(HistoryCacheTest,GetMaxMinSeqNum)
{
	createHistory(WRITER);
	for(uint i = 0;i<historySize;i++)
	{
		CacheChange_t* change = history->reserve_Cache();
		ASSERT_TRUE(change!=NULL);
		EXPECT_TRUE(history->add_change(change));
	}
	SequenceNumber_t seq;
	GUID_t guid;
	EXPECT_TRUE(history->get_seq_num_min(&seq,&guid));
	EXPECT_EQ(1,seq.to64long());
	EXPECT_TRUE(history->get_seq_num_max(&seq,&guid));
	EXPECT_EQ(historySize,seq.to64long());
	EXPECT_TRUE(history->remove_change(seq,guid));
	EXPECT_TRUE(history->get_seq_num_max(&seq,&guid));
		EXPECT_EQ(historySize-1,seq.to64long());
}




