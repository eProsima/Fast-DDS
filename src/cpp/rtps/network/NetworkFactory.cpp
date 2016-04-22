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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
