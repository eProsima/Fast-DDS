#ifndef NETWORK_FACTORY_HPP
#define NETWORK_FACTORY_HPP

#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <vector>
#include <memory>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class NetworkFactory
{
public:
   template<typename TransportType> void RegisterTransport(const typename TransportType::TransportDescriptor& descriptor)
   {
      mRegisteredTransports.emplace_back(new TransportType(descriptor));
   }

   std::vector<SenderResource>   BuildSenderResources                   (Locator_t local);
   std::vector<SenderResource>   BuildSenderResourcesForRemoteLocator   (Locator_t remote);
   std::vector<ReceiverResource> BuildReceiverResources                 (Locator_t local);

private:
   std::vector<std::unique_ptr<TransportInterface> > mRegisteredTransports;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
