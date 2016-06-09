#ifndef RECEIVER_RESOURCE_H
#define RECEIVER_RESOURCE_H

#include <functional>
#include <vector>
#include <fastrtps/transport/TransportInterface.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class ReceiverResource 
{
friend class NetworkFactory;

public:
   // Performs a blocking receive through the channel managed by this resource,
   // notifying about the origin locator.
   bool Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
                Locator_t& originLocator);

   // Reports whether this resource supports the given local locator (i.e., said locator
   // maps to the transport channel managed by this resource).
   bool SupportsLocator(const Locator_t& localLocator);

   // Aborts a blocking receive (thread safe).
   void Abort();

   // Resources can only be transfered through move semantics. Copy, assignment, and 
   // construction outside of the factory are forbidden.
   ReceiverResource(ReceiverResource&&);
   ~ReceiverResource();

private:
   ReceiverResource()                                   = delete;
   ReceiverResource(const ReceiverResource&)            = delete;
   ReceiverResource& operator=(const ReceiverResource&) = delete;

   ReceiverResource(TransportInterface&, const Locator_t&);
   std::function<void()> Cleanup;
   std::function<bool(octet*, uint32_t, uint32_t&, Locator_t&)> ReceiveFromAssociatedChannel;
   std::function<bool(const Locator_t&)> LocatorMapsToManagedChannel;
   bool mValid; // Post-construction validity check for the NetworkFactory
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
