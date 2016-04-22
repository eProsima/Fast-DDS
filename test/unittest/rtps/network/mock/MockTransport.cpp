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

bool MockTransport::AreLocatorChannelsOpen(Locator_t locator) const
{
   return false;
};

bool MockTransport::IsLocatorSupported(Locator_t locator) const
{
   return locator.kind == mockSupportedKind;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
