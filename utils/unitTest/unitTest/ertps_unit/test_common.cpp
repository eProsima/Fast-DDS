/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file test_common.hpp
 *
 *  Created on: Apr 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef TEST_COMMON_HPP_
#define TEST_COMMON_HPP_

TEST(CommonTypes, CDRMessage)
{
	CDRMessage_t msg;
	ASSERT_EQ(0,msg.pos);
	ASSERT_EQ(0,msg.length);
	ASSERT_EQ(RTPSMESSAGE_MAX_SIZE,msg.max_size);
	ASSERT_TRUE(NULL != msg.buffer);
	ASSERT_EQ(EPROSIMA_ENDIAN,msg.msg_endian);
	CDRMessage_t msg2(20);
		ASSERT_EQ(0,msg2.pos);
		ASSERT_EQ(0,msg2.length);
		ASSERT_EQ(20,msg2.max_size);
		ASSERT_TRUE(NULL != msg2.buffer);
		ASSERT_EQ(EPROSIMA_ENDIAN,msg2.msg_endian);
}


TEST(CommonTypes,SequenceNumber)
{
	SequenceNumber_t seq;
	EXPECT_EQ(0,seq.high);
	EXPECT_EQ(0,seq.low);
	seq.low = 54;
	seq++;
	EXPECT_EQ(0,seq.high);
	EXPECT_EQ(55,seq.low);
	seq = seq + (pow(2.0,32));
	EXPECT_EQ(1,seq.high);
	EXPECT_EQ(55,seq.low);
	SequenceNumber_t seq2;
	seq2.high = 43;
	seq2.low = 67;
	seq = seq2;
	EXPECT_EQ(43,seq.high);
	EXPECT_EQ(67,seq.low);
	EXPECT_TRUE(seq == seq2);
}


TEST(CommonTypes,SequenceNumberSet)
{
	SequenceNumber_t seq1,seq2,seq3;
	seq1.low = 1;
	seq2.low = 235;
	seq3.low = 300;
	SequenceNumberSet_t set;
	EXPECT_TRUE(set.isSetEmpty());
	set.base = seq1;
	EXPECT_TRUE(set.add(seq2));
	EXPECT_FALSE(set.add(seq3));
	EXPECT_EQ(235,set.get_maxSeqNum().low);
	EXPECT_EQ(0,set.get_maxSeqNum().high);
}

TEST(CommonTypes,SerializedPayload)
{
	SerializedPayload_t payload;
	ASSERT_TRUE(NULL == payload.data);
	EXPECT_EQ(0,payload.length);
	SerializedPayload_t payload2(20);
	ASSERT_FALSE(NULL == payload2.data);
	payload2.length = 10;
	payload.copy(&payload2);
	EXPECT_TRUE(payload.data !=NULL);
	EXPECT_EQ(10,payload.length);
}

TEST(CommonTypes,InstanceHandle)
{
	InstanceHandle_t iH;
	for(int i=0;i<16;i++)
	{
		EXPECT_EQ(0,iH.value[i]);
		iH.value[i] = (octet)i;
	}
	InstanceHandle_t iH2 = iH;
	for(int i=0;i<16;i++)
	{
		EXPECT_EQ(i,iH2.value[i]);
	}
}



#endif /* TEST_COMMON_HPP_ */
