#include <fastrtps/rtps/network/NetworkFactory.h>
using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

std::vector<SenderResource> NetworkFactory::BuildSenderResources(Locator_t locator)
{
   vector<SenderResource> newSenderResources; 

   for(auto& transport : mRegisteredTransports)
   {
      if ( transport->IsLocatorSupported(locator) &&
          !transport->IsLocatorChannelOpen(locator) )
      {
         newSenderResources.emplace_back();
         auto& newSenderResource = newSenderResources.back();
         
         // The channel that this locator maps to is opened
         transport->OpenLocatorChannel(locator);

         // The Sender resource is bundled with the right cleanup function;
      }
   }
   return newSenderResources;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
