#include "eprosimartps/CDRMessage.h"

TEST(CDRMessage, constructor)
{
	CDRMessage_t msg;
	ASSERT_EQ(0,msg.pos);
	ASSERT_EQ(0,msg.length);
	ASSERT_EQ(RTPSMESSAGE_MAX_SIZE,msg.max_size);
	ASSERT_TRUE(NULL != msg.buffer);
	ASSERT_EQ(EPROSIMA_ENDIAN,msg.msg_endian);
}

TEST(CDRMessage, constructor_size)
{
	CDRMessage_t msg2(20);
	ASSERT_EQ(0,msg2.pos);
	ASSERT_EQ(0,msg2.length);
	ASSERT_EQ(20,msg2.max_size);
	ASSERT_TRUE(NULL != msg2.buffer);
	ASSERT_EQ(EPROSIMA_ENDIAN,msg2.msg_endian);
}

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

TEST_F(CDRMessageTest, add_octet)
{
	octet o = 35;
	saveLengthPos();
	CDRMessage::addOctet(&t_msg,o);
	EXPECT_TRUE(checkLengthIncrement(1));
	EXPECT_TRUE(checkPosIncrement(1));
	ASSERT_EQ(35,t_msg.buffer[t_msg.pos-1]) << "Octet not added correctly";
}

TEST_F(CDRMessageTest, add_uint16)
{
	uint16_t
	saveLengthPos();
	CDRMessage::addOctet(&t_msg,o);
	EXPECT_TRUE(checkLengthIncrement(1));
	EXPECT_TRUE(checkPosIncrement(1));
	ASSERT_EQ(35,t_msg.buffer[t_msg.pos-1]) << "Octet not added correctly";
}


