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
/**
 * Provides the FastRTPS library with abstract resources, which
 * in turn manage the SEND and RECEIVE operations over some transport.
 * Once a transport is registered, it becomes invisible to the library
 * and is abstracted away for good.
 * @ingroup NETWORK_MODULE.
 */
class NetworkFactory
{
public:
   /**
    * Allows registration of a transport known at compile time. This method is preferable
    * for user-defined transports.
    * @param descriptor Structure that defines all initial configuration for a given transport.
    */
   template<typename TransportType> void RegisterTransport(const typename TransportType::TransportDescriptor& descriptor)
   {
      mRegisteredTransports.emplace_back(new TransportType(descriptor));
   }

   /**
    * Allows registration of a transport dynamically. Only the transports built into FastRTPS
    * are supported here (although it can be easily extended at NetworkFactory.cpp)
    * @param descriptor Structure that defines all initial configuration for a given transport.
    */
   void RegisterTransport(const TransportDescriptorInterface* descriptor);

   /**
    * Walks over the list of transports, opening every possible channel that can send through 
    * the given locator and returning a vector of Sender Resources associated with it.
    * @param local Locator through which to send.
    */
   std::vector<SenderResource>   BuildSenderResources                 (const Locator_t& local);
   /**
    * Walks over the list of transports, opening every possible channel that can send to the 
    * given remote locator and returning a vector of Sender Resources associated with it.
    * @param local Destination locator that we intend to send to.
    */
   std::vector<SenderResource>   BuildSenderResourcesForRemoteLocator (const Locator_t& remote);
   /**
    * Walks over the list of transports, opening every possible channel that we can listen to
    * from the given locator, and returns a vector of Receiver Resources for this goal.
    * @param local Locator from which to listen.
    */
   std::vector<ReceiverResource> BuildReceiverResources               (const Locator_t& local);

  

private:
   std::vector<std::unique_ptr<TransportInterface> > mRegisteredTransports;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
