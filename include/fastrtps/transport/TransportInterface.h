#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <vector>
#include <fastrtps/rtps/common/Locator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/* TransportInterface expects the user to implement a logical equivalence between Locators and protocol-specific "channels".
 * This equivalence can be narrowing: For instance in UDP/IP, a port would take the role of channel, and several different
 * locators can map to the same port, and hence the same channel. 
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

   // Must report whether the channel associated to this locator is open. Channels must either be fully closed or fully 
   // open, so that "open" and "close" operations are whole and definitive.
   virtual bool IsOutputChannelOpen(const Locator_t&) const = 0;
   virtual bool IsInputChannelOpen(const Locator_t&) const = 0;

   // Must report whether the given locator is supported by this transport (typically inspecting it's "kind" value).
   virtual bool IsLocatorSupported(const Locator_t&) const = 0;

   // Returns the locator describing the main (most general) channel that can write to the provided remote locator.
   virtual Locator_t RemoteToMainLocal(const Locator_t& remote) const = 0;

   // Must the channel that maps to/from the given locator. This method must allocate, reserve and mark
   // any resources that are needed for said channel.
   virtual bool OpenOutputChannel(const Locator_t&) = 0;
   virtual bool OpenInputChannel(const Locator_t&) = 0;

   // Must close the channel that maps to/from the given locator. 
   // IMPORTANT: It MUST be safe to call this method even during a Send and Receive operation. You must implement
   // any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage
   virtual bool CloseOutputChannel(const Locator_t&) = 0;
   virtual bool CloseInputChannel(const Locator_t&) = 0;

   // Must report whether two locators map to the same internal channel.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const = 0;

   // Must execute a blocking send, through the outbound channel that maps to the localLocator, targeted to the
   // remote address defined by remoteLocator. Must be threadsafe between channels, but not necessarily
   // within the same channel.
   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator) = 0;

   // Must execute a blocking receive, on the inbound channel that maps to the localLocator, receiving from the
   // address that gets written to remoteLocator. Must be threadsafe between channels, but not necessarily
   // within the same channel.
   virtual bool Receive(std::vector<octet>& receiveBuffer, const Locator_t& localLocator, Locator_t& remoteLocator) = 0;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
