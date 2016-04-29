#include <fastrtps/transport/test_UDPv4Transport.h>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <fastrtps/rtps/common/CDRMessage_t.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/qos/ParameterList.h>

#include <memory>
#include <string>

using namespace std;
using namespace eprosima::fastrtps::rtps;
using namespace boost::interprocess;

class test_UDPv4Tests: public ::testing::Test 
{
   public:
   test_UDPv4Tests()
   {
      HELPER_SetDescriptorDefaults();
   }

   void HELPER_SetDescriptorDefaults();
   void HELPER_WarmUpOutput(test_UDPv4Transport& transport);
   void HELPER_FillDataMessage(CDRMessage_t& message);
   void HELPER_FillAckNackMessage(CDRMessage_t& message);

   test_UDPv4Transport::TransportDescriptor descriptor;
   unique_ptr<boost::thread> senderThread;
   unique_ptr<boost::thread> receiverThread;
};

TEST_F(test_UDPv4Tests, DATA_messages_dropped)
{  
   // Given
   descriptor.dropDataMessages = true;
   test_UDPv4Transport transportUnderTest(descriptor);
   CDRMessage_t testDataMessage;
   HELPER_FillDataMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = 7400;
   locator.kind = LOCATOR_KIND_UDPv4;

   // When
   vector<char> message;
   message.resize(80);
   ASSERT_LE(testDataMessage.length, message.size());
   memcpy(message.data(), testDataMessage.buffer, testDataMessage.length);

   // Then
   ASSERT_TRUE(transportUnderTest.Send(message, locator, locator));
   ASSERT_EQ(1, test_UDPv4Transport::DropLog.size());
}

TEST_F(test_UDPv4Tests, ACKNACK_messages_dropped)
{  
   // Given
   descriptor.dropAckNackMessages = true;
   test_UDPv4Transport transportUnderTest(descriptor);
   CDRMessage_t testDataMessage;
   HELPER_FillAckNackMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = 7400;
   locator.kind = LOCATOR_KIND_UDPv4;

   // When
   vector<char> message;
   message.resize(testDataMessage.length);
   memcpy(message.data(), testDataMessage.buffer, testDataMessage.length);

   // Then
   ASSERT_TRUE(transportUnderTest.Send(message, locator, locator));
   ASSERT_EQ(1, test_UDPv4Transport::DropLog.size());
}

TEST_F(test_UDPv4Tests, No_drops_when_unrequested)
{  
   // Given
   test_UDPv4Transport transportUnderTest(descriptor); // Default, no drops
   CDRMessage_t testDataMessage;
   HELPER_FillDataMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = 7400;
   locator.kind = LOCATOR_KIND_UDPv4;

   // When
   vector<char> message;
   message.resize(80);
   ASSERT_LE(testDataMessage.length, message.size());
   memcpy(message.data(), testDataMessage.buffer, testDataMessage.length);

   // Then
   ASSERT_TRUE(transportUnderTest.Send(message, locator, locator));
   ASSERT_EQ(0, test_UDPv4Transport::DropLog.size());
}

TEST_F(test_UDPv4Tests, Send_will_still_fail_on_bad_locators_without_dropping_as_expected)
{  
   // Given
   descriptor.dropDataMessages = true;
   test_UDPv4Transport transportUnderTest(descriptor);
   CDRMessage_t testDataMessage;
   HELPER_FillDataMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t badLocator;
   badLocator.kind = LOCATOR_KIND_UDPv6; // unsupported

   // When
   vector<char> message;
   message.resize(testDataMessage.length);
   memcpy(message.data(), testDataMessage.buffer, testDataMessage.length);

   // Then
   ASSERT_FALSE(transportUnderTest.Send(message, badLocator, badLocator));
   ASSERT_EQ(0, test_UDPv4Transport::DropLog.size());
}

void test_UDPv4Tests::HELPER_SetDescriptorDefaults()
{
   descriptor.sendBufferSize = 80;
   descriptor.receiveBufferSize = 80;
   descriptor.dropDataMessages = false;
   descriptor.dropAckNackMessages = false;
   descriptor.dropHeartbeatMessages = false;
   descriptor.percentageOfMessagesToDrop = 0;
   descriptor.dropLogLength = 10;
}

void test_UDPv4Tests::HELPER_WarmUpOutput(test_UDPv4Transport& transport)
{
   Locator_t outputChannelLocator;
   outputChannelLocator.port = 7400;
   outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   ASSERT_TRUE(transport.OpenOutputChannel(outputChannelLocator));
}

void test_UDPv4Tests::HELPER_FillDataMessage(CDRMessage_t& message)
{
   GuidPrefix_t prefix;
   TopicKind_t topic = WITH_KEY;
   EntityId_t entityID;
   CacheChange_t change;
   ParameterList_t parameters;
	RTPSMessageCreator::addMessageData(&message, prefix, &change, topic, entityID, false, &parameters);
}

void test_UDPv4Tests::HELPER_FillAckNackMessage(CDRMessage_t& message)
{
   GuidPrefix_t prefix;
   EntityId_t entityID;
   SequenceNumberSet_t set;
	RTPSMessageCreator::addMessageAcknack(&message, prefix, prefix, entityID, entityID, set, 0, false);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
