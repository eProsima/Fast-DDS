#ifndef SENDER_RESOURCE_H
#define SENDER_RESOURCE_H

#include <functional>
#include <vector>
#include <fastrtps/transport/TransportInterface.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class SenderResource 
{
   friend class NetworkFactory;

public:
   // Sends to a destination locator, through the channel managed by this resource.
   bool Send(const std::vector<char>& data, const Locator_t& destinationLocator);

   // Reports whether this resource supports the given local locator (i.e., said locator
   // maps to the transport channel managed by this resource).
   bool SupportsLocator(const Locator_t& local);
   // Reports whether this resource can write to a remote locator.
   bool CanSendToRemoteLocator(const Locator_t& remote);

   // Resources can only be transfered through move semantics. Copy, assignment, and 
   // construction outside of the factory are forbidden.
   SenderResource(SenderResource&&);
   ~SenderResource();

private:
   SenderResource()                                 = delete;
   SenderResource(const SenderResource&)            = delete;
   SenderResource& operator=(const SenderResource&) = delete;

   SenderResource(TransportInterface&, const Locator_t&);
   std::function<void()> Cleanup;
   std::function<bool(const std::vector<char>&, const Locator_t&)> SendThroughAssociatedChannel;
   std::function<bool(const Locator_t&)> LocatorMapsToManagedChannel;
   std::function<bool(const Locator_t&)> ManagedChannelMapsToRemote;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
