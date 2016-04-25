#include <MockTransport.h>
#include <algorithm>
using namespace std;


namespace eprosima{
namespace fastrtps{
namespace rtps{

std::vector<MockTransport*> MockTransport::mockTransportInstances;

MockTransport::MockTransport(const TransportDescriptor& descriptor):
   mockSupportedKind(descriptor.supportedKind),
   mockMaximumChannels(descriptor.maximumChannels)
{
   MockTransport::mockTransportInstances.push_back(this);
}

MockTransport::MockTransport():
   mockSupportedKind(DefaultKind),
   mockMaximumChannels(DefaultMaxChannels)
{
   MockTransport::mockTransportInstances.push_back(this);
}

MockTransport::~MockTransport()
{
   // Remove this mock from the handle vector
   mockTransportInstances.erase(std::remove(mockTransportInstances.begin(),
                                       mockTransportInstances.end(),
                                       this),
                                mockTransportInstances.end());
}

bool MockTransport::IsLocatorChannelOpen(Locator_t locator) const
{
  return (find(mockOpenChannels.begin(), mockOpenChannels.end(), locator.port) != mockOpenChannels.end());
}

bool MockTransport::IsLocatorSupported(Locator_t locator) const
{
   return locator.kind == mockSupportedKind;
}

bool MockTransport::OpenLocatorChannel(Locator_t locator)
{  
   mockOpenChannels.push_back(locator.port);
   return true;
}

bool MockTransport::DoLocatorsMatch(Locator_t left, Locator_t right) const
{
   return left.port == right.port;
}

bool MockTransport::Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress)
{
   mockMessagesSent.push_back( {localChannel, remoteAddress, sendBuffer} );
}

bool MockTransport::Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t remoteAddress)
{
   receiveBuffer = mockMessagesToReceive.back().data;
   mockMessagesToReceive.pop_back();
}

bool MockTransport::CloseLocatorChannel(Locator_t locator)
{
   mockOpenChannels.erase(std::remove(mockOpenChannels.begin(),
                                      mockOpenChannels.end(),
                                      locator.port),
                                mockOpenChannels.end());
   return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
