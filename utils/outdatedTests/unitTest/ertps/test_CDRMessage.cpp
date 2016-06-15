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

#include "fastrtps/CDRMessage.h"



class CDRMessageTest: public ::testing::Test
{
protected:
	void SetUp()
	{

	}
	CDRMessage_t t_msg;
	uint16_t t_length;
	uint16_t t_pos;
	void saveLengthPos()
	{
		t_length = t_msg.length;
		t_pos = t_msg.pos;
	}
	bool checkLengthIncrement(uint16_t inc)
	{
		if(t_length+inc == t_msg.length)
			return true;
		else
			return false;
	}
	bool checkPosIncrement(uint16_t inc)
	{
		if(t_pos+inc == t_msg.pos)
			return true;
		else
			return false;
	}


};

TEST_F(CDRMessageTest, octet)
{
	octet o = 35;
	saveLengthPos();
	CDRMessage::addOctet(&t_msg,o);
	EXPECT_TRUE(checkLengthIncrement(1));
	EXPECT_TRUE(checkPosIncrement(1));
	ASSERT_EQ(35,t_msg.buffer[t_msg.pos-1]) << "Octet not added correctly";
	t_msg.pos--;
	saveLengthPos();
	CDRMessage::readOctet(&t_msg,&o);
	EXPECT_FALSE(checkLengthIncrement(1));
	EXPECT_TRUE(checkPosIncrement(1));
	ASSERT_EQ(35,o) << "Octet not readed correctly";
}

TEST_F(CDRMessageTest, uint16)
{
	uint16_t n = 645;
	saveLengthPos();
	CDRMessage::addUInt16(&t_msg,n);
	EXPECT_TRUE(checkLengthIncrement(2));
	EXPECT_TRUE(checkPosIncrement(2));
	t_msg.pos-=2;
	CDRMessage::readUInt16(&t_msg,&n);
	ASSERT_EQ(645,n);
}

TEST_F(CDRMessageTest, uint32)
{
	uint32_t n = 123456;
	saveLengthPos();
	CDRMessage::addUInt32(&t_msg,n);
	EXPECT_TRUE(checkLengthIncrement(4));
	EXPECT_TRUE(checkPosIncrement(4));
	t_msg.pos-=4;
	CDRMessage::readUInt32(&t_msg,&n);
	ASSERT_EQ(123456,n);
}


TEST_F(CDRMessageTest, int32)
{
	int32_t n = 123456;
	saveLengthPos();
	EXPECT_TRUE(CDRMessage::addInt32(&t_msg,n));
	EXPECT_TRUE(checkLengthIncrement(4));
	EXPECT_TRUE(checkPosIncrement(4));
	t_msg.pos-=4;
	EXPECT_TRUE(CDRMessage::readInt32(&t_msg,&n));
	ASSERT_EQ(123456,n);
	n = -123456;
	saveLengthPos();
	EXPECT_TRUE(CDRMessage::addInt32(&t_msg,n));
	EXPECT_TRUE(checkLengthIncrement(4));
	EXPECT_TRUE(checkPosIncrement(4));
	t_msg.pos-=4;
	EXPECT_TRUE(CDRMessage::readInt32(&t_msg,&n));
	ASSERT_EQ(-123456,n);
}

TEST_F(CDRMessageTest, SequenceNumber)
{
	SequenceNumber_t seq;
	seq.high = 10;
	seq.low = 25;
	saveLengthPos();
	EXPECT_TRUE(CDRMessage::addSequenceNumber(&t_msg,&seq));
	EXPECT_TRUE(checkLengthIncrement(8));
	EXPECT_TRUE(checkPosIncrement(8));
	t_msg.pos-=8;
	CDRMessage::readSequenceNumber(&t_msg,&seq);
	ASSERT_EQ(10,seq.high);
	ASSERT_EQ(25,seq.low);

}

