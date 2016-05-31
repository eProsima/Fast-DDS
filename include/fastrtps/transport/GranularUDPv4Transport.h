#ifndef GRANULAR_UDPV4_TRANSPORT_H
#define GRANULAR_UDPV4_TRANSPORT_H

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

class GranularUDPv4Transport : public TransportInterface
{
public:
   typedef struct {
      uint32_t sendBufferSize;
      uint32_t receiveBufferSize;
   } TransportDescriptor;

   GranularUDPv4Transport(const TransportDescriptor&);
   ~GranularUDPv4Transport();

   virtual bool IsInputChannelOpen(const Locator_t&)         const;
   virtual bool IsOutputChannelOpen(const Locator_t&)        const;

   virtual bool IsLocatorSupported(const Locator_t&)         const;

   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const;
   virtual Locator_t RemoteToMainLocal(const Locator_t& remote) const;

   virtual bool OpenInputChannel(const Locator_t& multicastFilter);

   virtual bool OpenOutputChannel(const Locator_t&);

   virtual bool CloseInputChannel(const Locator_t&);

   virtual bool CloseOutputChannel(const Locator_t&);

   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator);
   virtual bool Receive(std::vector<octet>& receiveBuffer, const Locator_t& localLocator, Locator_t& remoteLocator);

protected:
   // Constructor with no descriptor is necessary for implementations derived from this class.
   GranularUDPv4Transport();
   uint32_t mSendBufferSize;
   uint32_t mReceiveBufferSize;

	boost::asio::io_service mService;
   std::unique_ptr<boost::thread> ioServiceThread;

   mutable boost::recursive_mutex mOutputMapMutex;
   mutable boost::recursive_mutex mInputMapMutex;

   // For granular UDPv4, the notion of channel corresponds to a fully qualified address.
   std::map<Locator_t, boost::asio::ip::udp::socket> mOutputSockets; // Maps address to socket.
   std::map<Locator_t, boost::asio::ip::udp::socket> mInputSockets;  // Maps address to socket.

   bool OpenAndBindOutputSockets(const Locator_t& locator);
   boost::asio::ip::udp::socket OpenAndBindUnicastOutputSocket(boost::asio::ip::address_v4, uint32_t port);

   bool OpenAndBindInputSockets(uint16_t port, boost::asio::ip::address_v4 multicastFilterAddress);
   boost::asio::ip::udp::socket OpenAndBindMulticastInputSocket(uint32_t port, boost::asio::ip::address_v4 multicastFilterAddress);

   bool SendThroughSocket(const octet* sendBuffer,
                          uint32_t sendBufferSize,
                          const Locator_t& remoteLocator,
                          boost::asio::ip::udp::socket& socket);
};

// Arbitrary, meaningless comparison Required for map and set indexing.
inline bool operator<(const Locator_t& lhs, const Locator_t& rhs)
{
	if(lhs.port < rhs.port)
		return true;
	for(uint8_t i =0; i<16 ;i++)
   {
		if(lhs.address[i] < rhs.address[i])
			return true;
	}
	return false;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
