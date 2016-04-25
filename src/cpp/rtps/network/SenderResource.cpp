#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, Locator_t locator)
{
   transport.OpenLocatorChannel(locator);
   Cleanup = [&transport,locator](){ transport.CloseLocatorChannel(locator); };
   SendThroughAssociatedChannel = [&transport, locator](const vector<char>& data, Locator_t destination)-> bool
                              { return transport.Send(data, locator, destination); };
}

bool SenderResource::Send(const std::vector<char>& data, Locator_t destinationLocator)
{
   return SendThroughAssociatedChannel(data, destinationLocator);
}

SenderResource::SenderResource(SenderResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup); 
   SendThroughAssociatedChannel.swap(rValueResource.SendThroughAssociatedChannel);
}

SenderResource::~SenderResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
