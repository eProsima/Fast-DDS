
#include <fastrtps/rtps/network/ReceiverResource.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ReceiverResource::ReceiverResource(TransportInterface& transport, Locator_t locator)
{
   // Internal channel is opened and assigned to this resource.
   transport.OpenInputChannel(locator);

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.CloseInputChannel(locator); };
   ReceiveFromAssociatedChannel = [&transport, locator](vector<char>& data, Locator_t& origin)-> bool
                                  { return transport.Receive(data, locator, origin); };
   LocatorMapsToManagedChannel = [&transport, locator](Locator_t locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
}

bool ReceiverResource::Receive(std::vector<char>& data, Locator_t& originLocator)
{
   return ReceiveFromAssociatedChannel(data, originLocator);
}

ReceiverResource::ReceiverResource(ReceiverResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup); 
   ReceiveFromAssociatedChannel.swap(rValueResource.ReceiveFromAssociatedChannel);
   LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
}

bool ReceiverResource::SupportsLocator(Locator_t localLocator)
{
   return LocatorMapsToManagedChannel(localLocator);
}

ReceiverResource::~ReceiverResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
