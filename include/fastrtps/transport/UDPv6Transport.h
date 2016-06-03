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

RTPS_DllAPI class UDPv6Transport : public TransportInterface
{
public:
   /**
    * Transport configuration
    *
    * - bufferSize:    length of the buffers used for transmission. Passing
    *                  a buffer of different size will cause transmission to
    *                  fail.
    *
    * - granularMode:  False: Outbound channel maps to port
    *                  True:  Outbound channel maps to port + address    
    *
    * - interfaceWhiteList: Lists the allowed interfaces.
    */
   typedef struct TransportDescriptor: public TransportDescriptorInterface{
      uint32_t sendBufferSize;
      uint32_t receiveBufferSize;
      bool granularMode;
      std::vector<boost::asio::ip::address_v6> interfaceWhiteList;

      virtual ~TransportDescriptor(){}
      TransportDescriptor();
   } TransportDescriptor;

   RTPS_DllAPI UDPv6Transport(const TransportDescriptor&);
   ~UDPv6Transport();

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(const Locator_t&) const;

   /**
    * Checks whether there are open and bound sockets for the given port,
    * or in granular mode, for the given address.
    */
   virtual bool IsOutputChannelOpen(const Locator_t&) const;

   //! Checks for UDPv4 kind.
   virtual bool IsLocatorSupported(const Locator_t&) const;

   //! Reports whether Locators correspond to the same port.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const;

   /**
    * Converts a given remote locator (that is, a locator referring to a remote
    * destination) to the main local locator whose channel can write to that
    * destination. In this case it will return a 0.0.0.0 address on that port.
    */
   virtual Locator_t RemoteToMainLocal(const Locator_t&) const;

   /**
    * Starts listening on the specified port, and if the specified address is in the
    * multicast range, it joins the specified multicast group,
    */
   virtual bool OpenInputChannel(const Locator_t&);

   /**
    * Opens a socket per interface on the given port (as long as they are white listed).
    * In granular mode, it will only open a socket for the specified interface.
    */
   virtual bool OpenOutputChannel(const Locator_t&);

   //! Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(const Locator_t&);

   //! Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(const Locator_t&);

   /**
    * Blocking Send through the specified channel. In both modes, using a localLocator of 0.0.0.0 will
    * send through all whitelisted interfaces provided the channel is open.
    * @param sendBuffer Slice into the raw data to send.
    * @param sendBufferSize Size of the raw data. It will be used as a bounds check for the previous argument.
    * It must not exceed the sendBufferSize fed to this class during construction.
    * @param localLocator Locator mapping to the channel we're sending from.
    * @param remoteLocator Locator describing the remote destination we're sending to.
    */
   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator);
   /**
    * Blocking Receive from the specified channel.
    * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receiveBufferSize supplied to this class during construction.
    * @param localLocator Locator mapping to the local channel we're listening to.
    * @param[out] remoteLocator Locator describing the remote restination we received a packet from.
    */
   virtual bool Receive(std::vector<octet>& receiveBuffer, const Locator_t& localLocator, Locator_t& remoteLocator);

private:
   //! Constructor with no descriptor is necessary for implementations derived from this class.
   UDPv6Transport();
   uint32_t mSendBufferSize;
   uint32_t mReceiveBufferSize;
   bool mGranularMode;

   // For UDPv6, the notion of channel corresponds to a port + direction tuple.
	boost::asio::io_service mService;
   std::unique_ptr<boost::thread> ioServiceThread;

   mutable boost::recursive_mutex mOutputMapMutex;
   mutable boost::recursive_mutex mInputMapMutex;

   //! For non-granular UDPv4, the notion of output channel corresponds to a port.
   std::map<uint16_t, std::vector<boost::asio::ip::udp::socket> > mOutputSockets; 
   //! For granular UDPv4, the notion of output channel corresponds to an address.
   std::map<Locator_t, boost::asio::ip::udp::socket> mGranularOutputSockets;
   //! For both modes, an input channel corresponds to a port.
   std::map<uint16_t, boost::asio::ip::udp::socket> mInputSockets; 

   bool IsInterfaceAllowed(const boost::asio::ip::address_v6& ip);
   std::vector<boost::asio::ip::address_v6> mInterfaceWhiteList;


   bool OpenAndBindOutputSockets(uint16_t port);
   bool OpenAndBindGranularOutputSocket(const Locator_t& locator);
   bool OpenAndBindInputSockets(uint16_t port);

   boost::asio::ip::udp::socket OpenAndBindUnicastOutputSocket(const boost::asio::ip::address_v6&, uint32_t port);
   boost::asio::ip::udp::socket OpenAndBindInputSocket(uint32_t port);

   bool SendThroughSocket(const octet* sendBuffer,
                          uint32_t sendBufferSize,
                          const Locator_t& remoteLocator,
                          boost::asio::ip::udp::socket& socket);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
