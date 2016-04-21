#ifndef NETWORK_FACTORY_HPP
#define NETWORK_FACTORY_HPP

#include <fastrtps/transport/TransportInterface.h>
#include <vector>
#include <memory>

class NetworkFactory
{
public:
   bool RegisterTransport<typename TransportType>(const TransportType::TransportDescriptor&);

   vector<SenderResource> BuildSenderResources(Locator_t);
   vector<ReceiverResource> BuildReceiverResources(Locator_t);

private:
   std::vector<std::unique_ptr<TransportInterface> > mRegisteredTransports;
};



#endif
