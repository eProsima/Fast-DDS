#ifndef UDPV4_TRANSPORT_H
#define UDPV4_TRANSPORT_H

#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/udp.hpp>

#include "TransportInterface.h"
#include <vector>
#include <map>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class UDPv4Transport : public TransportInterface
{
public:
   // Transport configuration
   typedef struct {
      uint32_t bufferSize;
   } TransportDescriptor;

   UDPv4Transport(const TransportDescriptor&);

   // Checks whether there are open and bound sockets for the given port.
   virtual bool IsLocatorChannelOpen(Locator_t) const;

   // Must report whether the given locator is supported by this transport (typically inspecting its "kind" value).
   virtual bool IsLocatorSupported(Locator_t) const;

   // Must the channel that maps to/from the given locator. This method must allocate, reserve and mark
   // any resources that are needed for said channel.
   virtual bool OpenLocatorChannel(Locator_t);

   // Must close the channel that maps to/from the given locator. 
   // IMPORTANT: It MUST be safe to call this method even during a Send and Receive operation. You must implement
   // any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage
   virtual bool CloseLocatorChannel(Locator_t);

   // Must report whether two locators map to the same internal channel.
   virtual bool DoLocatorsMatch(Locator_t, Locator_t) const;

   // Must execute a blocking send, through the outbound channel that maps to the localLocator, targeted to the
   // remote address defined by remoteLocator. Must be threadsafe between channels, but not necessarily
   // within the same channel.
   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator);

   // Must execute a blocking receive, on the inbound channel that maps to the localLocator, receiving from the
   // address defined by remoteLocator. Must be threadsafe between channels, but not necessarily
   // within the same channel.
   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localLocator, Locator_t remoteLocator);

private:
   TransportDescriptor mDescriptor;

   // For UDPv4, the notion of channel corresponds to a port. 
	boost::asio::io_service mSendService;
   std::map<uint16_t, boost::asio::ip::udp::socket> mPortSocketMap;
   bool OpenAndBindUnicastSocket(uint32_t port);
   bool IsAddressMulticast(Locator_t);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
