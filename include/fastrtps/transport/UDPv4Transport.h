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

   virtual bool IsLocatorSupported(Locator_t) const;

   virtual bool OpenLocatorChannel(Locator_t);

   virtual bool CloseLocatorChannel(Locator_t);

   virtual bool DoLocatorsMatch(Locator_t, Locator_t) const;

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator);

   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localLocator, Locator_t remoteLocator);

private:
   TransportDescriptor mDescriptor;

   // For UDPv4, the notion of channel corresponds to a port. 
   // Requesting a port with any IP  will trigger the binding of a socket per interface, plus
   // a multicast receiver socket.
	boost::asio::io_service mSendService;
   std::map<uint16_t, std::vector<boost::asio::ip::udp::socket> > mPortSocketMap;

   bool OpenAndBindSockets(uint32_t port);
   boost::asio::ip::udp::socket OpenAndBindSocket(boost::asio::ip::address_v4, uint32_t port);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
