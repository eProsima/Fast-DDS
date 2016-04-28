#ifndef UDPV4_TRANSPORT_H
#define UDPV4_TRANSPORT_H

#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
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

/* This is a non-granular UDPv4 implementation. It means the following:
 *    -> Opening an output channel by passing a locator will open a socket per interface on the given port.
 *       This collection of sockets constitute the "outbound channel". In other words, a channel corresponds
 *       to a port + a direction.
 *
 *    -> Opening an input channel by passing a locator will open a socket listening on the given port on every
 *       interface, and join the multicast channel specified by the locator address. Hence, any locator that
 *       does not correspond to the multicast range will be rejected when opening an input channel. Joining
 *       multicast groups late is not currently supported. Listening on particular unicast ports is not yet 
 *       supported (both will be part of a future granular UDPv4 implementation).
 */

class UDPv4Transport : public TransportInterface
{
public:
   /*Transport configuration
    * - bufferSize: length of the buffers used for transmission. Passing
    *               a buffer of different size will cause transmission to
    *               fail.
    * */
   typedef struct {
      uint32_t sendBufferSize;
      uint32_t receiveBufferSize;
   } TransportDescriptor;

   UDPv4Transport(const TransportDescriptor&);
   ~UDPv4Transport();

   // Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(Locator_t)         const;
   virtual bool IsOutputChannelOpen(Locator_t)        const;

   // Checks for UDPv4 kind.
   virtual bool IsLocatorSupported(Locator_t)         const;

   // Reports whether Locators correspond to the same port.
   virtual bool DoLocatorsMatch(Locator_t, Locator_t) const;
   virtual Locator_t RemoteToMainLocal(Locator_t remote) const;

   // Starts listening on the specified port and joins the specified multicast group.
   virtual bool OpenInputChannel(Locator_t multicastFilter);

   // Opens a socket per interface on the given port.
   virtual bool OpenOutputChannel(Locator_t);

   // Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(Locator_t);

   // Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(Locator_t);

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator);
   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localLocator, Locator_t & remoteLocator);

protected:
   // Constructor with no descriptor is necessary for implementations derived from this class.
   UDPv4Transport();
   uint32_t mSendBufferSize;
   uint32_t mReceiveBufferSize;

   // For non-granular UDPv4, the notion of channel corresponds to a port + direction tuple.
	boost::asio::io_service mService;
   std::unique_ptr<boost::thread> ioServiceThread;

   mutable boost::recursive_mutex mOutputMapMutex;
   mutable boost::recursive_mutex mInputMapMutex;
   std::map<uint16_t, std::vector<boost::asio::ip::udp::socket> > mOutputSockets; // Maps port to socket collection.
   std::map<uint16_t, boost::asio::ip::udp::socket> mInputSockets;  // Maps port to socket collection.

   bool OpenAndBindOutputSockets(uint16_t port);
   bool OpenAndBindInputSockets(uint16_t port, boost::asio::ip::address_v4 multicastFilterAddress);
   boost::asio::ip::udp::socket OpenAndBindUnicastOutputSocket(boost::asio::ip::address_v4, uint32_t port);
   boost::asio::ip::udp::socket OpenAndBindMulticastInputSocket(uint32_t port, boost::asio::ip::address_v4 multicastFilterAddress);

   bool SendThroughSocket(const std::vector<char>& sendBuffer,
                          Locator_t remoteLocator,
                          boost::asio::ip::udp::socket& socket);

   void StartAsyncListen(std::vector<char>& receiveBuffer,
                         boost::asio::ip::udp::socket& socket, 
                         Locator_t remoteLocator, 
                         boost::interprocess::interprocess_semaphore& receiveSemaphore);

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
