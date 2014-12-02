/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file test_CacheChangePool.cpp
 *
 *  Created on: Apr 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */
#include "gtest/gtest.h"
#include "fastrtps/CacheChangePool.h"

using namespace eprosima::rtps;



class CacheChangePoolTest: public ::testing::Test
{
protected:
	CacheChangePoolTest():
		pool_size(6),
		payload_size(50),
		changePool(pool_size,payload_size)
	{

	}
	void SetUp()
	{

	}
	uint32_t pool_size;
	uint32_t payload_size;
	CacheChangePool changePool;

};

TEST_F(CacheChangePoolTest, constructor)
{
	EXPECT_EQ(pool_size,changePool.get_allCachesSize());
	EXPECT_EQ(pool_size,changePool.get_freeCachesSize());
}

TEST_F(CacheChangePoolTest, ReserveAndRelease)
{
	CacheChange_t* change = changePool.reserve_Cache();
	EXPECT_EQ(payload_size,change->serializedPayload.max_size);
	EXPECT_EQ(0,change->sequenceNumber.high);
	EXPECT_EQ(0,change->sequenceNumber.low);
	EXPECT_EQ(ALIVE,change->kind);
	ASSERT_EQ(pool_size-1,changePool.get_freeCachesSize());
	ASSERT_EQ(pool_size,changePool.get_allCachesSize());
	changePool.release_Cache(change);
	ASSERT_EQ(pool_size,changePool.get_freeCachesSize());
}


TEST_F(CacheChangePoolTest, ReserveMoreThanAllCaches)
{
	CacheChange_t* changes[pool_size+2];
	for(uint8_t i = 0;i<pool_size+2;++i)
	{
		EXPECT_NO_THROW(changes[i] = changePool.reserve_Cache());
		EXPECT_TRUE(changes[i] !=NULL);
	}
	EXPECT_TRUE(changePool.get_allCachesSize() > pool_size);
	for(uint8_t i = 0;i<pool_size+2;++i)
	{
		EXPECT_NO_THROW(changePool.release_Cache(changes[i]));
	}
	EXPECT_TRUE(changePool.get_allCachesSize()==changePool.get_freeCachesSize());
}
