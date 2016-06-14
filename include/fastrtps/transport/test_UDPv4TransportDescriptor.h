
#ifndef TEST_UDPV4_TRANSPORT_DESCRIPTOR 
#define TEST_UDPV4_TRANSPORT_DESCRIPTOR

#include "TransportInterface.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

typedef struct test_UDPv4TransportDescriptor : public TransportDescriptorInterface{
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

   RTPS_DllAPI test_UDPv4TransportDescriptor();
   virtual ~test_UDPv4TransportDescriptor(){}
} test_UDPv4TransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
