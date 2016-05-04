#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, const Locator_t& locator)
{
   // Internal channel is opened and assigned to this resource.
   transport.OpenOutputChannel(locator);

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.CloseOutputChannel(locator); };
   SendThroughAssociatedChannel = [&transport, locator](const vector<char>& data, const Locator_t& destination)-> bool
                                  { return transport.Send(data, locator, destination); };
   LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
   ManagedChannelMapsToRemote = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, transport.RemoteToMainLocal(locatorToCheck)); };
}

bool SenderResource::Send(const std::vector<char>& data, const Locator_t& destinationLocator)
{
   return SendThroughAssociatedChannel(data, destinationLocator);
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
   return LocatorMapsToManagedChannel(local);
}

bool SenderResource::CanSendToRemoteLocator(const Locator_t& remote)
{
   return ManagedChannelMapsToRemote(remote);
}

SenderResource::~SenderResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
