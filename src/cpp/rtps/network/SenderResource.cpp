#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, const Locator_t& locator)
{
   // Internal channel is opened and assigned to this resource.
   mValid = transport.OpenOutputChannel(locator);
   if (!mValid)
      return; // Invalid resource, will be discarded by the factory.

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.CloseOutputChannel(locator); };
   SendThroughAssociatedChannel = [&transport, locator](const octet* data, uint32_t dataSize, const Locator_t& destination)-> bool
                                  { return transport.Send(data,dataSize, locator, destination); };
   LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
   ManagedChannelMapsToRemote = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, transport.RemoteToMainLocal(locatorToCheck)); };
}

bool SenderResource::Send(const octet* data, uint32_t dataLength, const Locator_t& destinationLocator)
{
   if (SendThroughAssociatedChannel)
      return SendThroughAssociatedChannel(data, dataLength, destinationLocator);
   return false;
}

SenderResource::SenderResource(SenderResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup); 
   SendThroughAssociatedChannel.swap(rValueResource.SendThroughAssociatedChannel);
   LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
   ManagedChannelMapsToRemote.swap(rValueResource.ManagedChannelMapsToRemote);
}

bool SenderResource::SupportsLocator(const Locator_t& local)
{
   if (LocatorMapsToManagedChannel)
      return LocatorMapsToManagedChannel(local);
   return false;
}

bool SenderResource::CanSendToRemoteLocator(const Locator_t& remote)
{
   if (ManagedChannelMapsToRemote)
      return ManagedChannelMapsToRemote(remote);
   return false;
}

SenderResource::~SenderResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
