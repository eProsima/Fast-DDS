
#include <fastrtps/rtps/network/ReceiverResource.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ReceiverResource::ReceiverResource(TransportInterface& transport, const Locator_t& locator)
{
   // Internal channel is opened and assigned to this resource.
   mValid = transport.OpenInputChannel(locator);
   if (!mValid)
      return; // Invalid resource to be discarded by the factory.

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.CloseInputChannel(locator); };
   ReceiveFromAssociatedChannel = [&transport, locator](vector<char>& data, Locator_t& origin)-> bool
                                  { return transport.Receive(data, locator, origin); };
   LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
}

bool ReceiverResource::Receive(std::vector<char>& data, Locator_t& originLocator)
{
   if (ReceiveFromAssociatedChannel)
      return ReceiveFromAssociatedChannel(data, originLocator);
   return false;
}

ReceiverResource::ReceiverResource(ReceiverResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup); 
   ReceiveFromAssociatedChannel.swap(rValueResource.ReceiveFromAssociatedChannel);
   LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
}

bool ReceiverResource::SupportsLocator(const Locator_t& localLocator)
{
   if (LocatorMapsToManagedChannel)
      return LocatorMapsToManagedChannel(localLocator);
   return false;
}

void ReceiverResource::Abort()
{
   if (Cleanup)
      Cleanup();
}

ReceiverResource::~ReceiverResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
