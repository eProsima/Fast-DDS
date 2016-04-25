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
   // Sends to a destination locator, through the channel used to create the resource.
   bool Send(const std::vector<char>& data, Locator_t destinationLocator);

   // Resources can only be transfered through move semantics. Copy, assignment, and 
   // construction outside of the factory are forbidden.
   SenderResource(SenderResource&&);
   ~SenderResource();

private:
   SenderResource()                                 = delete;
   SenderResource(const SenderResource&)            = delete;
   SenderResource& operator=(const SenderResource&) = delete;

   SenderResource(TransportInterface&, Locator_t);
   std::function<void()> Cleanup;
   std::function<bool(const std::vector<char>&, Locator_t)> SendThroughAssociatedChannel;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
