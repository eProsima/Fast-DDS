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

/**
 * This is a default UDPv6 implementation.
 *    - Opening an output channel by passing a locator will open a socket per interface on the given port.
 *       This collection of sockets constitute the "outbound channel". In other words, a channel corresponds
 *       to a port + a direction.
 *
 *    - In Granular mode, outbound channels correspond to a fully qualified IP address, so it is possible to
 *       write to a particular network interface. Sending to ANY will still cause a send over every 
 *       interface as expected.
 *
 *    - It is possible to provide a white list at construction, which limits the interfaces the transport 
 *       will ever be able to interact with. If left empty, all interfaces are allowed.
 *
 *    - Opening an input channel by passing a locator will open a socket listening on the given port on every
 *       whitelisted interface, and join the multicast channel specified by the locator address. Hence, any locator 
 *       that does not correspond to the multicast range will simply open the port without a subsequent join. Joining
 *       multicast groups late is supported by attempting to open the channel again with the same port + a 
 *       multicast address (the OpenInputChannel function will fail, however, because no new channel has been
 *       opened in a strict sense).
 * @ingroup TRANSPORT_MODULE
 */

class UDPv6Transport : public TransportInterface
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
      RTPS_DllAPI TransportDescriptor();
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

   //! Checks for UDPv6 kind.
   virtual bool IsLocatorSupported(const Locator_t&) const;

   //! Reports whether Locators correspond to the same port.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const;

   /**
    * Converts a given remote locator (that is, a locator referring to a remote
    * destination) to the main local locator whose channel can write to that
    * destination. In this case it will return a IP_ANY address on that port.
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
    * Blocking Send through the specified channel. In both modes, using a localLocator of ANY will
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
   virtual bool Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
                        const Locator_t& localLocator, Locator_t& remoteLocator);

private:
   uint32_t mSendBufferSize;
   uint32_t mReceiveBufferSize;
   bool mGranularMode;

   // For UDPv6, the notion of channel corresponds to a port + direction tuple.
	boost::asio::io_service mService;
   std::unique_ptr<boost::thread> ioServiceThread;

   mutable boost::recursive_mutex mOutputMapMutex;
   mutable boost::recursive_mutex mInputMapMutex;

   //! For non-granular UDPv6, the notion of output channel corresponds to a port.
   std::map<uint32_t, std::vector<boost::asio::ip::udp::socket> > mOutputSockets; 
   //! For granular UDPv6, the notion of output channel corresponds to an address.
   struct LocatorCompare{ bool operator()(const Locator_t& lhs, const Locator_t& rhs) const
                        {return (memcmp(&lhs, &rhs, sizeof(Locator_t)) < 0); } };
   //! For granular UDPv6, the notion of output channel corresponds to an address.
   std::map<Locator_t, boost::asio::ip::udp::socket, LocatorCompare> mGranularOutputSockets;
   //! For both modes, an input channel corresponds to a port.
   std::map<uint32_t, boost::asio::ip::udp::socket> mInputSockets; 

   bool IsInterfaceAllowed(const boost::asio::ip::address_v6& ip);
   std::vector<boost::asio::ip::address_v6> mInterfaceWhiteList;


   bool OpenAndBindOutputSockets(uint32_t port);
   bool OpenAndBindGranularOutputSocket(const Locator_t& locator);
   bool OpenAndBindInputSockets(uint32_t port);

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
