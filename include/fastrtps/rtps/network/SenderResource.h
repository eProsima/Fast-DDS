#ifndef SENDER_RESOURCE_H
#define SENDER_RESOURCE_H

#include <functional>
#include <fastrtps/transport/TransportInterface.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class SenderResource 
{
   friend class NetworkFactory;

public:
   bool Send();

   // Resources can only be transfered through move semantics. Copy, assignment, and 
   // construction outside of the factory are forbidden.
   SenderResource(SenderResource&&);
   ~SenderResource();

   SenderResource()                                 = delete;
   SenderResource(const SenderResource&)            = delete;
   SenderResource& operator=(const SenderResource&) = delete;
   
private:
   SenderResource(TransportInterface&, Locator_t);
   std::function<void()> Cleanup;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
