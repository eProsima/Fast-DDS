#ifndef TEST_UDPV4_TRANSPORT_H
#define TEST_UDPV4_TRANSPORT_H
#include <fastrtps/transport/UDPv4Transport.h>
#include <boost/thread.hpp>
#include <vector>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/* This transport acts as a shim over UDPv4, allowing
 * packets to be dropped under certain criteria.
 */

class test_UDPv4Transport : public UDPv4Transport
{
public:
   typedef struct {
      // UDPv4 layer parameters
      uint32_t sendBufferSize;
      uint32_t receiveBufferSize;

      // Test shim parameters
      bool dropDataMessages;
      bool dropHeartbeatMessages;
      bool dropAckNackMessages;
      std::vector<uint32_t> sequenceNumberDataMessagesToDrop;
      uint8_t percentageOfMessagesToDrop;

      uint32_t dropLogLength; // logs dropped packets.
   } TransportDescriptor;

   test_UDPv4Transport(const test_UDPv4Transport::TransportDescriptor& descriptor);

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator);
  
   // Handle to a persistent log of dropped packets. Defaults to length 0 (no logging) to prevent wasted resources.
   static std::vector<std::vector<char> > DropLog;
   static uint32_t DropLogLength;

private:
   bool mDropDataMessages;
   bool mDropHeartbeatMessages;
   bool mDropAckNackMessages;
   std::vector<uint32_t> mSequenceNumberDataMessagesToDrop;
   uint8_t mPercentageOfMessagesToDrop;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
