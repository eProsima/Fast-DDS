#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, Locator_t locator)
{
   // Internal channel is open and assigned to this resource.
   transport.OpenLocatorChannel(locator);

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.CloseLocatorChannel(locator); };
   SendThroughAssociatedChannel = [&transport, locator](const vector<char>& data, Locator_t destination)-> bool
                                  { return transport.Send(data, locator, destination); };
   LocatorMapsToManagedChannel = [&transport, locator](Locator_t locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
}

bool SenderResource::Send(const std::vector<char>& data, Locator_t destinationLocator)
{
   return SendThroughAssociatedChannel(data, destinationLocator);
}

SenderResource::SenderResource(SenderResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup); 
   SendThroughAssociatedChannel.swap(rValueResource.SendThroughAssociatedChannel);
   LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
}

bool SenderResource::SupportsLocator(Locator_t localLocator)
{
   return LocatorMapsToManagedChannel(localLocator);
}

SenderResource::~SenderResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
