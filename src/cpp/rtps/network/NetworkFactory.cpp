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
          !transport->AreLocatorChannelsOpen(locator) )
      {
         transport->OpenLocatorChannels(locator);
         newSenderResources.emplace_back();
      }
   }
   return newSenderResources;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
