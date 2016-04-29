#include <fastrtps/transport/test_UDPv4Transport.h>

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

bool test_UDPv4Transport::Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator)
{
   UDPv4Transport::Send(sendBuffer, localLocator, remoteLocator);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
