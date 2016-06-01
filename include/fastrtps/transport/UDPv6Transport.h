#ifndef UDPV6_TRANSPORT_H
#define UDPV6_TRANSPORT_H

#include <boost/asio.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread.hpp>

#include "TransportInterface.h"
#include <vector>
#include <memory>
#include <map>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/* This is a non-granular UDPv6 implementation. It means the following:
 *    -> Opening an output channel by passing a locator will open a socket per interface on the given port.
 *       This collection of sockets constitute the "outbound channel". In other words, a channel corresponds
 *       to a port + a direction.
 *
 *    -> Opening an input channel by passing a locator will open a socket listening on the given port on every
 *       interface, and join the multicast channel specified by the locator address. Hence, any locator that
 *       does not correspond to the multicast range will be rejected when opening an input channel. Joining
 *       multicast groups late is not currently supported. Listening on particular unicast ports is not yet 
 *       supported (both will be part of a future granular UDPv6 implementation). Again, channel = port + direction.
 */

class UDPv6Transport : public TransportInterface
{
public:
   /*Transport configuration
    * - bufferSize: length of the buffers used for transmission. Passing
    *               a buffer of different size will cause transmission to
    *               fail.
    * */
   typedef struct TransportDescriptor: public TransportDescriptorInterface{
      uint32_t sendBufferSize;
      uint32_t receiveBufferSize;

      virtual ~TransportDescriptor(){}
   } TransportDescriptor;

   UDPv6Transport(const TransportDescriptor&);
   ~UDPv6Transport();

   // Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(const Locator_t&)         const;
   virtual bool IsOutputChannelOpen(const Locator_t&)        const;

   // Checks for UDPv6 kind.
   virtual bool IsLocatorSupported(const Locator_t&)         const;

   // Reports whether Locators correspond to the same port.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const;
   virtual Locator_t RemoteToMainLocal(const Locator_t& remote) const;

   // Starts listening on the specified port and joins the specified multicast group.
   virtual bool OpenInputChannel(const Locator_t&);

   // Opens a socket per interface on the given port.
   virtual bool OpenOutputChannel(const Locator_t&);

   // Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(const Locator_t&);

   // Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(const Locator_t&);

   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator);
   virtual bool Receive(std::vector<octet>& receiveBuffer, const Locator_t& localLocator, Locator_t& remoteLocator);

private:
   TransportDescriptor mDescriptor;

   // For UDPv6, the notion of channel corresponds to a port + direction tuple.
	boost::asio::io_service mService;
   std::unique_ptr<boost::thread> ioServiceThread;

   mutable boost::recursive_mutex mOutputMapMutex;
   mutable boost::recursive_mutex mInputMapMutex;
   std::map<uint16_t, std::vector<boost::asio::ip::udp::socket> > mOutputSockets; // Maps port to socket collection.
   std::map<uint16_t, boost::asio::ip::udp::socket> mInputSockets;  // Maps port to socket

   bool OpenAndBindOutputSockets(uint16_t port);
   bool OpenAndBindInputSockets(uint16_t port, boost::asio::ip::address_v6 multicastFilterAddress);
   boost::asio::ip::udp::socket OpenAndBindUnicastOutputSocket(boost::asio::ip::address_v6, uint32_t port);
   boost::asio::ip::udp::socket OpenAndBindMulticastInputSocket(uint32_t port, boost::asio::ip::address_v6 multicastFilterAddress);

   bool SendThroughSocket(const octet* sendBuffer,
                          uint32_t sendBufferSize,
                          const Locator_t& remoteLocator,
                          boost::asio::ip::udp::socket& socket);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
