#include <fastrtps/rtps/network/SenderResource.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, Locator_t locator):
   mOriginalGeneratingLocator(locator),
   mAssociatedTransport(transport)
{
   transport.OpenLocatorChannel(locator);
}

SenderResource::SenderResource(SenderResource&& rValueResource):
   mOriginalGeneratingLocator(rValueResource.mOriginalGeneratingLocator),
   mAssociatedTransport(rValueResource.mAssociatedTransport)
{
}

SenderResource::~SenderResource()
{
   mAssociatedTransport.CloseLocatorChannel(mOriginalGeneratingLocator);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
