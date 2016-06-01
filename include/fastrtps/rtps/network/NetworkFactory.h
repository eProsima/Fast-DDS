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

   // Dynamic version
   void RegisterTransport(const TransportDescriptorInterface* descriptor);

   std::vector<SenderResource>   BuildSenderResources                 (const Locator_t& local);
   std::vector<SenderResource>   BuildSenderResourcesForRemoteLocator (const Locator_t& remote);
   std::vector<ReceiverResource> BuildReceiverResources               (const Locator_t& local);

private:
   std::vector<std::unique_ptr<TransportInterface> > mRegisteredTransports;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
