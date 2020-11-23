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

#include <thread>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/rtps/common/CDRMessage_t.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/qos/ParameterTypes.h>
#include <fastrtps/utils/IPLocator.h>
#include <rtps/transport/test_UDPv4Transport.h>

#if defined(_WIN32)
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif

using IPLocator = eprosima::fastrtps::rtps::IPLocator;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;

static uint16_t g_default_port = 0;

uint16_t get_port()
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if(4000 > port)
    {
        port += 4000;
    }

    return port;
}

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class test_UDPv4Tests: public ::testing::Test
{
    public:
    test_UDPv4Tests()
    {
        HELPER_SetDescriptorDefaults();
    }

    ~test_UDPv4Tests()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

   void HELPER_SetDescriptorDefaults();
   void HELPER_WarmUpOutput(test_UDPv4Transport& transport);
   void HELPER_FillDataMessage(CDRMessage_t& message, SequenceNumber_t sequenceNumber);
   void HELPER_FillAckNackMessage(CDRMessage_t& message);
   void HELPER_FillHeartbeatMessage(CDRMessage_t& message);

   test_UDPv4TransportDescriptor descriptor;
   std::unique_ptr<std::thread> senderThread;
   std::unique_ptr<std::thread> receiverThread;
};

/*
TEST_F(test_UDPv4Tests, DATA_messages_dropped)
{
   // Given
   descriptor.dropDataMessagesPercentage = 100;
   test_UDPv4Transport transportUnderTest(descriptor);
   transportUnderTest.init();
   CDRMessage_t testDataMessage;
   HELPER_FillDataMessage(testDataMessage, SequenceNumber_t());
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = g_default_port;
   locator.kind = LOCATOR_KIND_UDPv4;

   // Then
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_EQ(1u, test_UDPv4Transport::test_UDPv4Transport_DropLog.size());

   ASSERT_TRUE(transportUnderTest.CloseOutputChannel(locator));
}

TEST_F(test_UDPv4Tests, ACKNACK_messages_dropped)
{
   // Given
   descriptor.dropAckNackMessagesPercentage = 100;
   test_UDPv4Transport transportUnderTest(descriptor);
   transportUnderTest.init();
   CDRMessage_t testDataMessage;
   HELPER_FillAckNackMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = g_default_port;
   locator.kind = LOCATOR_KIND_UDPv4;

   // Then
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_EQ(1u, test_UDPv4Transport::test_UDPv4Transport_DropLog.size());
   ASSERT_TRUE(transportUnderTest.CloseOutputChannel(locator));
}

TEST_F(test_UDPv4Tests, HEARTBEAT_messages_dropped)
{
   // Given
   descriptor.dropHeartbeatMessagesPercentage = 100;
   test_UDPv4Transport transportUnderTest(descriptor);
   transportUnderTest.init();
   CDRMessage_t testDataMessage;
   HELPER_FillHeartbeatMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = g_default_port;
   locator.kind = LOCATOR_KIND_UDPv4;

   // Then
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_EQ(1u, test_UDPv4Transport::test_UDPv4Transport_DropLog.size());
   ASSERT_TRUE(transportUnderTest.CloseOutputChannel(locator));
}

TEST_F(test_UDPv4Tests, Dropping_by_random_chance)
{
   // Given
   descriptor.percentageOfMessagesToDrop = 100; // To avoid a non-deterministic test
   test_UDPv4Transport transportUnderTest(descriptor);
   transportUnderTest.init();
   CDRMessage_t testDataMessage;
   HELPER_FillAckNackMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = g_default_port;
   locator.kind = LOCATOR_KIND_UDPv4;

   // Then
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_EQ(3u, test_UDPv4Transport::test_UDPv4Transport_DropLog.size());
   ASSERT_TRUE(transportUnderTest.CloseOutputChannel(locator));
}

TEST_F(test_UDPv4Tests, dropping_by_sequence_number)
{
   // Given
   std::vector<SequenceNumber_t> sequenceNumbersToDrop(1);
   sequenceNumbersToDrop.back().low = 1;

   descriptor.sequenceNumberDataMessagesToDrop = sequenceNumbersToDrop;
   test_UDPv4Transport transportUnderTest(descriptor);
   transportUnderTest.init();
   CDRMessage_t testDataMessage;
   HELPER_FillDataMessage(testDataMessage, sequenceNumbersToDrop.back());
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = g_default_port;
   locator.kind = LOCATOR_KIND_UDPv4;

   // Then
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_EQ(1u, test_UDPv4Transport::test_UDPv4Transport_DropLog.size());
   ASSERT_TRUE(transportUnderTest.CloseOutputChannel(locator));
}

TEST_F(test_UDPv4Tests, No_drops_when_unrequested)
{
   // Given
   descriptor.dropHeartbeatMessagesPercentage = 100;
   descriptor.dropDataMessagesPercentage = 100;
   descriptor.granularMode = false;

   test_UDPv4Transport transportUnderTest(descriptor); // Default, no drops
   transportUnderTest.init();
   CDRMessage_t testDataMessage;
   HELPER_FillAckNackMessage(testDataMessage);
   HELPER_WarmUpOutput(transportUnderTest);
   Locator_t locator;
   locator.port = g_default_port;
   locator.kind = LOCATOR_KIND_UDPv4;
   IPLocator::setIPv4(locator, 239, 255, 1, 4);

   // Then
   ASSERT_TRUE(transportUnderTest.send(testDataMessage.buffer, testDataMessage.length, locator, locator));
   ASSERT_EQ(0u, test_UDPv4Transport::test_UDPv4Transport_DropLog.size());
   ASSERT_TRUE(transportUnderTest.CloseOutputChannel(locator));
}

void test_UDPv4Tests::HELPER_SetDescriptorDefaults()
{
   descriptor.sendBufferSize = 80;
   descriptor.receiveBufferSize = 80;
   descriptor.dropDataMessagesPercentage = 0;
   descriptor.dropDataFragMessagesPercentage = 0;
   descriptor.dropAckNackMessagesPercentage = 0;
   descriptor.dropHeartbeatMessagesPercentage = 0;
   descriptor.percentageOfMessagesToDrop = 0;
   descriptor.dropLogLength = 10;
   descriptor.granularMode = false;
}

void test_UDPv4Tests::HELPER_WarmUpOutput(test_UDPv4Transport& transport)
{
   Locator_t outputChannelLocator;
   outputChannelLocator.port = g_default_port;
   outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   ASSERT_TRUE(transport.OpenOutputChannel(outputChannelLocator));
}
*/

void test_UDPv4Tests::HELPER_FillDataMessage(CDRMessage_t& message, SequenceNumber_t sequenceNumber)
{
   GuidPrefix_t prefix;
   TopicKind_t topic = WITH_KEY;
   EntityId_t entityID;
   CacheChange_t change;
   change.sequenceNumber = sequenceNumber; // Here is where the SN propagates from
   RTPSMessageCreator::addMessageData(&message, prefix, &change, topic, entityID, false, nullptr);
}

void test_UDPv4Tests::HELPER_FillAckNackMessage(CDRMessage_t& message)
{
   GuidPrefix_t prefix;
   EntityId_t entityID;
   SequenceNumberSet_t set;
	RTPSMessageCreator::addMessageAcknack(&message, prefix, prefix, entityID, entityID, set, 0, false);
}

void test_UDPv4Tests::HELPER_FillHeartbeatMessage(CDRMessage_t& message)
{
   GuidPrefix_t prefix;
   EntityId_t entityID;
   SequenceNumber_t sn1;
   SequenceNumber_t sn2;
	RTPSMessageCreator::addMessageHeartbeat(&message, prefix, entityID, entityID, sn1, sn2, 0, false, false);
}

int main(int argc, char **argv)
{
    g_default_port = get_port();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
