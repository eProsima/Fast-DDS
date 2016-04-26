#include <fastrtps/transport/UDPv4Transport.h>
#include <utility>
#include <cstring>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/RTPSLog.h>

using namespace std;
using namespace boost::asio;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const char* const CLASS_NAME = "UDPv4Transport";

UDPv4Transport::UDPv4Transport(const TransportDescriptor& descriptor):
   mDescriptor(descriptor)
{
}

bool UDPv4Transport::IsInputChannelOpen(Locator_t locator) const
{
   auto mapIterator = mInputSockets.find(locator.port);
   return mapIterator != mInputSockets.end();
}

bool UDPv4Transport::IsOutputChannelOpen(Locator_t locator) const
{
   auto mapIterator = mOutputSockets.find(locator.port);
   return mapIterator != mOutputSockets.end();
}

bool UDPv4Transport::OpenOutputChannel(Locator_t locator)
{
   if (IsOutputChannelOpen(locator))
      return false;   
   
   return OpenAndBindOutputSockets(locator.port);
}

bool UDPv4Transport::OpenInputChannel(Locator_t locator)
{
   if (IsInputChannelOpen(locator))
      return false;   
   
   return OpenAndBindInputSockets(locator.port);
}

bool UDPv4Transport::CloseOutputChannel(Locator_t locator)
{
   if (!IsOutputChannelOpen(locator))
      return false;   
  
   auto& sockets = mOutputSockets[locator.port];
   for (auto& socket : sockets)
   {
      socket.cancel();
      socket.close();
   }

   mOutputSockets.erase(locator.port);
   return true;
}

bool UDPv4Transport::CloseInputChannel(Locator_t locator)
{
   if (!IsInputChannelOpen(locator))
      return false;   
  
   auto& sockets = mInputSockets[locator.port];
   for (auto& socket : sockets)
   {
      socket.cancel();
      socket.close();
   }

   mInputSockets.erase(locator.port);
   return true;
}

bool UDPv4Transport::OpenAndBindOutputSockets(uint16_t port)
{
	const char* const METHOD_NAME = "OpenAndBindOutputSockets";

   try 
   {
      // Unicast output sockets, one per interface
      std::vector<IPFinder::info_IP> locNames;
      IPFinder::getIPs(&locNames);
      for (const auto& ip : locNames)
         mOutputSockets[port].push_back(OpenAndBindUnicastOutputSocket(boost::asio::ip::address_v4::from_string(ip.name), port));
   }
	catch (boost::system::system_error const& e)
   {
	   logInfo(RTPS_MSG_OUT, "UDPv4 Error binding at port: (" << port << ")" << " with boost msg: "<<e.what() , C_YELLOW);
      mOutputSockets.erase(port);
      return false;
   }

   return true;
}

bool UDPv4Transport::OpenAndBindInputSockets(uint16_t port)
{
	const char* const METHOD_NAME = "OpenAndBindInputSockets";

   try 
   {
      // Single multicast port
      mInputSockets[port].push_back(OpenAndBindMulticastInputSocket(port));
   }
	catch (boost::system::system_error const& e)
   {
	   logInfo(RTPS_MSG_OUT, "UDPv4 Error binding at port: (" << port << ")" << " with boost msg: "<<e.what() , C_YELLOW);
      mInputSockets.erase(port);
      return false;
   }

   // TODO needs work

   return true;
}

boost::asio::ip::udp::socket UDPv4Transport::OpenAndBindUnicastOutputSocket(ip::address_v4 ipAddress, uint32_t port)
{
   ip::udp::socket socket(mSendService);
   socket.open(ip::udp::v4());
   socket.set_option(socket_base::send_buffer_size(mDescriptor.sendBufferSize));

   ip::udp::endpoint endpoint(ipAddress, port);
   socket.bind(endpoint);

   return socket;
}

boost::asio::ip::udp::socket UDPv4Transport::OpenAndBindMulticastInputSocket(uint32_t port)
{
   ip::udp::socket socket(mSendService);
   socket.open(ip::udp::v4());
   socket.set_option(socket_base::receive_buffer_size(mDescriptor.receiveBufferSize));
	socket.set_option(ip::udp::socket::reuse_address( true ) );
	socket.set_option(ip::multicast::enable_loopback( true ) );
   auto anyIP = ip::address_v4::any();

   ip::udp::endpoint endpoint(anyIP, port);
   socket.bind(endpoint);
      
   // Join all multicast groups;
   LocatorList_t localLocatorList;
   IPFinder::getIP4Address(&locatorList);
 	for(auto& locator : locatorList)
      socket.set_option(ip::multicast::join_group(anyIP, ip::address_v4::from_string(locator.to_IP4_string())));

   return socket;
}

bool UDPv4Transport::DoLocatorsMatch(Locator_t left, Locator_t right) const
{
   return left.port == right.port;
}

bool UDPv4Transport::IsLocatorSupported(Locator_t locator) const
{
   return locator.kind == LOCATOR_KIND_UDPv4;
}

bool UDPv4Transport::Send(const std::vector<char>& sendBuffer, Locator_t localLocator, Locator_t remoteLocator)
{
   if (!IsOutputChannelOpen(localLocator) ||
       sendBuffer.size() != mDescriptor.sendBufferSize)
      return false;

   bool success = false;
   auto& sockets = mOutputSockets[localLocator.port];
   for (auto& socket : sockets)
      success |= SendThroughSocket(sendBuffer, remoteLocator, socket);

   return success;
}

bool UDPv4Transport::Receive(std::vector<char>& receiveBuffer, Locator_t localLocator, Locator_t remoteLocator)
{
   return false;
}

bool UDPv4Transport::SendThroughSocket(const std::vector<char>& sendBuffer,
                       Locator_t remoteLocator,
                       boost::asio::ip::udp::socket& socket)
{
	const char* const METHOD_NAME = "SendThroughSocket";

	boost::asio::ip::address_v4::bytes_type remoteAddress;
   memcpy(&remoteAddress, &remoteLocator.address[12], 4*sizeof(remoteAddress));
   auto destinationEndpoint = ip::udp::endpoint(boost::asio::ip::address_v4(remoteAddress), (uint16_t)remoteLocator.port);
   unsigned int bytesSent = 0;
   logInfo(RTPS_MSG_OUT,"UDPv4: " << sendBuffer.size() << " bytes TO endpoint: " << destinationEndpoint
         << " FROM " << socket.local_endpoint(), C_YELLOW);

   try 
   {
      bytesSent = socket.send_to(boost::asio::buffer(sendBuffer.data(), sendBuffer.size()), destinationEndpoint);
   }
   catch (const std::exception& error) 
   {
      logWarning(RTPS_MSG_OUT, "Error: " << error.what(), C_YELLOW);
      return false;
   }

	logInfo (RTPS_MSG_OUT,"SENT " << bytesSent,C_YELLOW);
   return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
