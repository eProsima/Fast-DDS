#include <fastrtps/transport/test_UDPv4Transport.h>
#include <cstdlib>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

vector<vector<octet> > test_UDPv4Transport::DropLog;
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
   srand(time(NULL));
}

bool test_UDPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
   if (PacketShouldDrop(sendBuffer, sendBufferSize) &&
       IsOutputChannelOpen(localLocator) &&
       sendBufferSize <= mSendBufferSize)
   {
      LogDrop(sendBuffer, sendBufferSize);
      return true;
   }
   else   
      return UDPv4Transport::Send(sendBuffer, sendBufferSize, localLocator, remoteLocator);
}

static bool ReadSubmessageHeader(CDRMessage_t& msg, SubmessageHeader_t& smh)
{
	if(msg.length - msg.pos < 4)
		return false;

	smh.submessageId = msg.buffer[msg.pos];msg.pos++;
	smh.flags = msg.buffer[msg.pos];msg.pos++;
	msg.msg_endian = smh.flags & BIT(0) ? LITTLEEND : BIGEND;
	CDRMessage::readUInt16(&msg, &smh.submessageLength);
	return true;
}

bool test_UDPv4Transport::PacketShouldDrop(const octet* sendBuffer, uint32_t sendBufferSize)
{
   CDRMessage_t cdrMessage(sendBufferSize);;
   memcpy(cdrMessage.buffer, sendBuffer, sendBufferSize);
   cdrMessage.length = sendBufferSize;

   return ( (mDropDataMessages      && ContainsSubmessageOfID(cdrMessage, DATA))      ||
            (mDropAckNackMessages   && ContainsSubmessageOfID(cdrMessage, ACKNACK))   ||
            (mDropHeartbeatMessages && ContainsSubmessageOfID(cdrMessage, HEARTBEAT)) || 
             ContainsSequenceNumberToDrop(cdrMessage)                                 ||
             RandomChanceDrop());
}

bool test_UDPv4Transport::ContainsSubmessageOfID(CDRMessage_t& cdrMessage, octet ID)
{
	if(cdrMessage.length < RTPSMESSAGE_HEADER_SIZE)
      return false;

   cdrMessage.pos = 4 + 12; // RTPS header letters + RTPS version

   SubmessageHeader_t cdrSubMessageHeader;
	while (cdrMessage.pos < cdrMessage.length)
   {  
      ReadSubmessageHeader(cdrMessage, cdrSubMessageHeader);
		if (cdrMessage.pos + cdrSubMessageHeader.submessageLength > cdrMessage.length)
         return false;
      if (cdrSubMessageHeader.submessageId == ID)
         return true;

	   cdrMessage.pos += cdrSubMessageHeader.submessageLength;
   }

   return false;
}


bool test_UDPv4Transport::LogDrop(const octet* buffer, uint32_t size)
{
   if (size < DropLogLength)
   {
      vector<octet> message;
      message.assign(buffer, buffer + size);
      DropLog.push_back(message);
      return true;
   }

   return false;
}

bool test_UDPv4Transport::ContainsSequenceNumberToDrop(CDRMessage_t& cdrMessage)
{
	if(cdrMessage.length < RTPSMESSAGE_HEADER_SIZE)
      return false;

   cdrMessage.pos = 4 + 12; // RTPS header letters + RTPS version

   SubmessageHeader_t cdrSubMessageHeader;
	while (cdrMessage.pos < cdrMessage.length)
   {  
      ReadSubmessageHeader(cdrMessage, cdrSubMessageHeader);
		if (cdrMessage.pos + cdrSubMessageHeader.submessageLength > cdrMessage.length)
         return false;
     
      auto positionBeforeSubmessageData = cdrMessage.pos;
    
      // Skip ahead based on message fields
      switch (cdrSubMessageHeader.submessageId)
      {
      case DATA:
         cdrMessage.pos += (2+2+4+4); // Flagskip + Octets to QoS + entityID + entityID
         break;
      case GAP:
         cdrMessage.pos += (4+4); // entityID + entityID
         break;
      case HEARTBEAT:
         cdrMessage.pos += (4+4); // entityID + entityID. We read FirstSN
      default:
	      cdrMessage.pos += cdrSubMessageHeader.submessageLength;
         continue; // Messages without sequence number
      }

      SequenceNumber_t sequenceNumber;
      bool valid = CDRMessage::readInt32(&cdrMessage, &sequenceNumber.high);
      valid &= CDRMessage::readUInt32(&cdrMessage, &sequenceNumber.low);
      bool contained = find(mSequenceNumberDataMessagesToDrop.begin(),
                            mSequenceNumberDataMessagesToDrop.end(),
                            sequenceNumber) != mSequenceNumberDataMessagesToDrop.end();

      if (valid && contained)
         return true;

      // Jump to the next submessage
      cdrMessage.pos = positionBeforeSubmessageData;
	   cdrMessage.pos += cdrSubMessageHeader.submessageLength;
   }

   return false;
}

bool test_UDPv4Transport::RandomChanceDrop()
{
   return mPercentageOfMessagesToDrop > (rand()%100);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
