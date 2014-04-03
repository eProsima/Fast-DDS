/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file test_HistoryCache.cpp
 *
 *  Created on: Apr 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */
#include "gtest/gtest.h"
#include "eprosimartps/HistoryCache.h"


using namespace eprosima::rtps;
class HistoryCacheTest:public ::testing::Test
{
protected:
	HistoryCacheTest():
		historySize(4),
		payloadSize(50),
		history(historySize,payloadSize)
		{

		}

	void SetUp(){

	}
	uint16_t historySize;
	uint16_t payloadSize;
	HistoryCache history;
};


TEST_F(HistoryCacheTest,Creation)
{
	EXPECT_EQ(0,history.m_changes.size());
	EXPECT_TRUE(history.mp_rtpswriter == NULL);
	EXPECT_TRUE(history.mp_rtpsreader == NULL);
	EXPECT_EQ(UDEF,history.m_historyKind);
}

TEST_F(HistoryCacheTest,ReserveAndRelease)
{
	CacheChange_t* change = NULL;
	EXPECT_NO_THROW(change = history.reserve_Cache());
	EXPECT_NO_THROW(history.release_Cache(change));
}

TEST_F(HistoryCacheTest,AddChangeUNDEF)
{
	CacheChange_t* change = history.reserve_Cache();
	change->sequenceNumber.low = 11;
	EXPECT_FALSE(history.add_change(change));
}
TEST_F(HistoryCacheTest,AddChangeWriter)
{
	history.m_historyKind = WRITER;
	for(uint i = 0;i<historySize;i++)
	{
		CacheChange_t* change = history.reserve_Cache();
		ASSERT_TRUE(change!=NULL);
		change->sequenceNumber.low = i+1;
		EXPECT_TRUE(history.add_change(change));
	}
	CacheChange_t* change = history.reserve_Cache();
	change->sequenceNumber.low = historySize+1;
	EXPECT_FALSE(history.add_change(change));
	EXPECT_EQ(historySize,history.m_changes.size());
	EXPECT_TRUE(history.isFull());
}

TEST_F(HistoryCacheTest,IsFull)
{
	EXPECT_FALSE(history.isFull());
}


TEST_F(HistoryCacheTest,AddChangeReader)
{
	history.m_historyKind = READER;
	CacheChange_t* change = history.reserve_Cache();
	change->sequenceNumber.low = 1;
	EXPECT_TRUE(history.add_change(change));
	EXPECT_FALSE(history.add_change(change));
}

TEST_F(HistoryCacheTest,RemoveChange)
{
	history.m_historyKind = WRITER;
	for(uint i = 0;i<historySize;i++)
	{
		CacheChange_t* change = history.reserve_Cache();
		ASSERT_TRUE(change!=NULL);
		EXPECT_TRUE(history.add_change(change));
	}
	SequenceNumber_t seq;
	seq.low = 2;
	GUID_t guid;
	GUID_UNKNOWN(guid);
	size_t siz = history.m_changes.size();
	EXPECT_TRUE(history.remove_change(seq,guid));
	EXPECT_FALSE(history.remove_change(seq,guid));
	EXPECT_EQ(siz-1,history.m_changes.size());
	std::vector<CacheChange_t*>::iterator it = history.m_changes.begin();
	siz = history.m_changes.size();
	EXPECT_TRUE(history.remove_change(it));
	EXPECT_EQ(siz-1,history.m_changes.size());
}


