#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/rtps/messages/CDRMessage.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

vector<vector<char> > test_UDPv4Transport::DropLog;
uint32_t test_UDPv4Transport::DropLogLength = 0;

test_UDPv4Transport::test_UDPv4Transport(const test_UDPv4Transport::TransportDescriptor& descriptor):
   mDropDataMessages(descriptor.dropDataMessages),
   mDropHeartbeatMessages(descriptor.dropHeartbeatMessages),
   mDropAckNackMessages(descriptor.dropAckNackMessages),
   mSequenceNumberDataMessagesToDrop(descriptor.sequenceNumberDataMessagesToDrop),
   mPercentageOfMessagesToDrop(descriptor.percentageOfMessagesToDrop)
{
   UDPv4Transport::mSendBufferSize = descriptor.sendBufferSize;
   UDPv4Transport::mReceiveBufferSize = descriptor.receiveBufferSize;
   DropLog.clear();
   DropLogLength = descriptor.dropLogLength;
}

bool test_UDPv4Transport::Send(const vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator)
{
   if (PacketShouldDrop(sendBuffer))
      LogDrop(sendBuffer);
   else
      UDPv4Transport::Send(sendBuffer, localLocator, remoteLocator);
}

static bool ReadSubmessageHeader(CDRMessage_t* msg, SubmessageHeader_t* smh)
{
	if(msg->length - msg->pos < 4)
		return false;

	smh->submessageId = msg->buffer[msg->pos];msg->pos++;
	smh->flags = msg->buffer[msg->pos];msg->pos++;
	msg->msg_endian = smh->flags & BIT(0) ? LITTLEEND : BIGEND;
	CDRMessage::readUInt16(msg,&smh->submessageLength);
	return true;
}

bool test_UDPv4Transport::PacketShouldDrop(const std::vector<char>& message)
{
   return (mDropDataMessages && ContainsDataSubmessage(message));
}

bool test_UDPv4Transport::ContainsDataSubmessage(const std::vector<char>& message)
{
   CDRMessage_t cdrMessage(message.size());;
   memcpy(cdrMessage.buffer, message.data(), message.size());
   cdrMessage.length = message.size();
   SubmessageHeader_t cdrSubMessageHeader;

   while(ReadSubmessageHeader(&cdrMessage,&cdrSubMessageHeader))
   {
      if (cdrSubMessageHeader.submessageId == DATA)
      {
         cdrMessage.pos = 0;
         return true;
      }
   }
   return false;
}

bool test_UDPv4Transport::LogDrop(const std::vector<char>& message)
{
   if (DropLog.size() < DropLogLength)
   {
      DropLog.push_back(message);
      return true;
   }

   return false;
}

uint32_t test_UDPv4Transport::ParseSequenceNumber(const vector<char>& message)
{
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
