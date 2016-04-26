#include <MockTransport.h>
#include <algorithm>
#include <cstring>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

std::vector<MockTransport*> MockTransport::mockTransportInstances;

MockTransport::MockTransport(const TransportDescriptor& descriptor):
   mockSupportedKind(descriptor.supportedKind),
   mockMaximumChannels(descriptor.maximumChannels)
{
   mockTransportInstances.push_back(this);
}

MockTransport::MockTransport():
   mockSupportedKind(DefaultKind),
   mockMaximumChannels(DefaultMaxChannels)
{
   mockTransportInstances.push_back(this);
}

MockTransport::~MockTransport()
{
   // Remove this mock from the handle vector
   mockTransportInstances.erase(std::remove(mockTransportInstances.begin(),
                                       mockTransportInstances.end(),
                                       this),
                                mockTransportInstances.end());
}

bool MockTransport::IsOutputChannelOpen(Locator_t locator) const
{
  return (find(mockOpenOutputChannels.begin(), mockOpenOutputChannels.end(), locator.port) != mockOpenOutputChannels.end());
}

bool MockTransport::IsInputChannelOpen(Locator_t locator) const
{
  return (find(mockOpenInputChannels.begin(), mockOpenInputChannels.end(), locator.port) != mockOpenInputChannels.end());
}

bool MockTransport::IsLocatorSupported(Locator_t locator) const
{
   return locator.kind == mockSupportedKind;
}

bool MockTransport::OpenOutputChannel(Locator_t locator)
{  
   mockOpenOutputChannels.push_back(locator.port);
   return true;
}

bool MockTransport::OpenInputChannel(Locator_t locator)
{  
   mockOpenInputChannels.push_back(locator.port);
   return true;
}

bool MockTransport::DoLocatorsMatch(Locator_t left, Locator_t right) const
{
   return left.port == right.port;
}

bool MockTransport::Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress)
{
   mockMessagesSent.push_back( { remoteAddress, localChannel, sendBuffer } );
   return true;
}

bool MockTransport::Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t& remoteAddress)
{
   (void)localChannel;
   receiveBuffer = mockMessagesToReceive.back().data;
   remoteAddress = mockMessagesToReceive.back().origin;
   mockMessagesToReceive.pop_back();
   return true;
}

Locator_t MockTransport::RemoteToMainLocal(Locator_t remote) const
{
   memset(remote.address, 0x00, sizeof(remote.address));
   return remote;
}

bool MockTransport::CloseOutputChannel(Locator_t locator)
{
   mockOpenOutputChannels.erase(std::remove(mockOpenOutputChannels.begin(),
                                      mockOpenOutputChannels.end(),
                                      locator.port),
                                mockOpenOutputChannels.end());
   return true;
}

bool MockTransport::CloseInputChannel(Locator_t locator)
{
   mockOpenInputChannels.erase(std::remove(mockOpenInputChannels.begin(),
                                      mockOpenInputChannels.end(),
                                      locator.port),
                                mockOpenInputChannels.end());
   return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
