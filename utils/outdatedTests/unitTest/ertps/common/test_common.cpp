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
 * @file test_common.hpp
 *
 *  Created on: Apr 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "fastrtps/common/CacheChange.h"

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
	EXPECT_EQ(0,payload.max_size);

	SerializedPayload_t payload2(20);
	ASSERT_FALSE(payload2.data ==NULL);
	EXPECT_EQ(20,payload2.max_size);
	EXPECT_EQ(0,payload2.length);

	payload2.length = 10;
	payload.copy(&payload2);
	EXPECT_TRUE(payload.data !=NULL);
	EXPECT_EQ(10,payload.length);
	EXPECT_EQ(10,payload.max_size);
	payload.empty();
	EXPECT_TRUE(payload.data ==NULL);
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

TEST(CommonTypes,CacheChange_t)
{
	EXPECT_NO_THROW(CacheChange_t change5);
	CacheChange_t change;
	EXPECT_EQ(ALIVE,change.kind);
	EXPECT_EQ(0,change.sequenceNumber.high);
	EXPECT_EQ(0,change.sequenceNumber.low);
	EXPECT_EQ(0,change.serializedPayload.length);
	EXPECT_EQ(0,change.serializedPayload.max_size);
	EXPECT_TRUE(change.serializedPayload.data == NULL);
	EXPECT_NO_THROW(CacheChange_t change4(30));
	CacheChange_t change2(30);
	EXPECT_EQ(30,change2.serializedPayload.max_size);
	EXPECT_FALSE(change2.serializedPayload.data==NULL);

}



