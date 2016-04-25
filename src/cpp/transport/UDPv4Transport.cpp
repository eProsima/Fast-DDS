#include <fastrtps/transport/UDPv4Transport.h>
#include <utility>
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

bool UDPv4Transport::IsLocatorChannelOpen(Locator_t locator) const
{
   auto mapIterator = mPortSocketMap.find(locator.port);
   return mapIterator != mPortSocketMap.end();
}

bool UDPv4Transport::OpenLocatorChannel(Locator_t locator)
{
   if (IsLocatorChannelOpen(locator))
      return false;   
   
   return OpenAndBindSockets(locator.port);
}

bool UDPv4Transport::OpenAndBindSockets(uint32_t port)
{
	const char* const METHOD_NAME = "OpenAndBindSockets";

   try 
   {
      // Multicast first, in vector index 0
      mPortSocketMap[port].push_back(OpenAndBindSocket(ip::address_v4::any(), port));

      // Unicast output sockets, one per interface
      std::vector<IPFinder::info_IP> locNames;
      IPFinder::getIPs(&locNames);
      for (const auto& ip : locNames)
         mPortSocketMap[port].push_back(OpenAndBindSocket(boost::asio::ip::address_v4::from_string(ip.name), port));
   }
	catch (boost::system::system_error const& e)
   {
	   logInfo(RTPS_MSG_OUT, "UDPv4 Error binding at port: (" << port << ")" << " with boost msg: "<<e.what() , C_YELLOW);
      mPortSocketMap.erase(port);
      return false;
   }

   return true;
}

boost::asio::ip::udp::socket UDPv4Transport::OpenAndBindSocket(ip::address_v4 ipAddress, uint32_t port)
{
   ip::udp::socket socket(mSendService);
   socket.open(ip::udp::v4());
   socket.set_option(socket_base::send_buffer_size(mDescriptor.bufferSize));

   ip::udp::endpoint endpoint(ipAddress, port);
   socket.bind(endpoint);

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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
