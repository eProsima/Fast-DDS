#include <fastrtps/rtps/network/NetworkFactory.h>
#include <utility>
using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

vector<SenderResource> NetworkFactory::BuildSenderResources(Locator_t locator)
{
   vector<SenderResource> newSenderResources; 

   for(auto& transport : mRegisteredTransports)
   {
      if ( transport->IsLocatorSupported(locator) &&
          !transport->IsLocatorChannelOpen(locator) )
      {
         SenderResource newSenderResource(*transport, locator);
         newSenderResources.push_back(move(newSenderResource));
      }
   }
   return newSenderResources;
}

vector<ReceiverResource> NetworkFactory::BuildReceiverResources(Locator_t locator)
{
   vector<ReceiverResource> newReceiverResources; 

   for(auto& transport : mRegisteredTransports)
   {
      if ( transport->IsLocatorSupported(locator) &&
          !transport->IsLocatorChannelOpen(locator) )
      {
         ReceiverResource newReceiverResource(*transport, locator);
         newReceiverResources.push_back(move(newReceiverResource));
      }
   }
   return newReceiverResources;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
