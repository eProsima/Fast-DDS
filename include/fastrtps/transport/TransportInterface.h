#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <vector>
#include <fastrtps/rtps/common/Locator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/* TransportInterface expects the user to implement a logical equivalence between Locators and protocol-specific "channels".
 * This equivalence need not be univocal: For instance in UDP/IP, a port would take the role of channel, and several different
 * locators can map to the same port. The converse is also possible: a single locator may in turn map to several channels.
 * */

class TransportInterface
{
public:
   /* Aside from the API defined here, an user-defined Transport must define a descriptor data type and a constructor that
    * expects a constant reference to such descriptor. e.g:
    *
    * class MyTransport:
    * public:
    *    typedef struct { ... } MyTransportDescriptor;
    *    MyTransport(const MyTransportDescriptor&);
    *    ...
    * */
   virtual ~TransportInterface(){};

   // Must report whether all channels associated to the given locator are open.
   virtual bool AreLocatorChannelsOpen(Locator_t) const = 0;

   // Must report whether the given locator is supported by this transport (typically inspecting it's "kind" value).
   virtual bool IsLocatorSupported(Locator_t)     const = 0;

   // Must open all closed channels that map to/from the given locator.
   virtual bool OpenLocatorChannels(Locator_t)          = 0;

   // Must close all open channels that map to/from the given locator.
   virtual bool CloseLocatorChannels(Locator_t)         = 0;

   // Must execute a blocking send, using all outbound channels that map to the localLocator, targeted to all
   // remote channels that map to the remoteLocator.
   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator) = 0;

   // Must execute a blocking receive, on all inbound channels that map to the localLocator, receiving from all
   // remote channels that map to the remoteLocator.
   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localLocator, Locator_t remoteLocator) = 0;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
