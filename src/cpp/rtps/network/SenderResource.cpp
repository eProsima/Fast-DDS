#include <fastrtps/rtps/network/SenderResource.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, Locator_t locator)
{
   transport.OpenLocatorChannel(locator);
   Cleanup = [&transport,locator](){ transport.CloseLocatorChannel(locator); };
}

SenderResource::SenderResource(SenderResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup); 
}

SenderResource::~SenderResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
