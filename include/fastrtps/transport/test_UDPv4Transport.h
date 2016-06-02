#ifndef TEST_UDPV4_TRANSPORT_H
#define TEST_UDPV4_TRANSPORT_H
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/rtps/messages/RTPS_messages.h>
#include <fastrtps/rtps/common/SequenceNumber.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <boost/thread.hpp>
#include <vector>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/* This transport acts as a shim over UDPv4, allowing
 * packets to be dropped under certain criteria. */

RTPS_DllAPI class test_UDPv4Transport : public UDPv4Transport
{
public:
   typedef struct TransportDescriptor : public TransportDescriptorInterface{
      // UDPv4 layer parameters
      uint32_t sendBufferSize;
      uint32_t receiveBufferSize;
      bool granularMode;

      // Test shim parameters
      uint8_t dropDataMessagesPercentage;
      uint8_t dropDataFragMessagesPercentage;
      uint8_t dropHeartbeatMessagesPercentage;
      uint8_t dropAckNackMessagesPercentage;

      // General drop percentage (indescriminate)
      uint8_t percentageOfMessagesToDrop;
      std::vector<SequenceNumber_t> sequenceNumberDataMessagesToDrop;

      uint32_t dropLogLength; // logs dropped packets.

      virtual ~TransportDescriptor(){}
   } TransportDescriptor;

   RTPS_DllAPI test_UDPv4Transport(const test_UDPv4Transport::TransportDescriptor& descriptor);

   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator);
  
   // Handle to a persistent log of dropped packets. Defaults to length 0 (no logging) to prevent wasted resources.
   RTPS_DllAPI static std::vector<std::vector<octet> > DropLog;
   RTPS_DllAPI static uint32_t DropLogLength;

private:
   uint8_t mDropDataMessagesPercentage;
   uint8_t mDropDataFragMessagesPercentage;
   uint8_t mDropHeartbeatMessagesPercentage;
   uint8_t mDropAckNackMessagesPercentage;
   std::vector<SequenceNumber_t> mSequenceNumberDataMessagesToDrop;
   uint8_t mPercentageOfMessagesToDrop;

   bool LogDrop(const octet* buffer, uint32_t size);
   bool PacketShouldDrop(const octet* sendBuffer, uint32_t sendBufferSize);
   bool ContainsSubmessageOfID(CDRMessage_t& cdrMessage, octet ID);
   bool ContainsSequenceNumberToDrop(CDRMessage_t& cdrMessage);
   bool RandomChanceDrop();
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
