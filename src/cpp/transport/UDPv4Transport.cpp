#include <fastrtps/transport/UDPv4Transport.h>
#include <utility>
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

   if (IsAddressMulticast(locator))
      return false;
   else
      return OpenAndBindUnicastSocket(locator.port);
}

bool UDPv4Transport::OpenAndBindUnicastSocket(uint32_t port)
{
	const char* const METHOD_NAME = "OpenAndBindUnicastSocket";

   ip::udp::socket socket(mSendService);
   socket.open(ip::udp::v4());
   socket.set_option(socket_base::send_buffer_size(mDescriptor.bufferSize));

   auto anyIPAddress = ip::address_v4::any();
   ip::udp::endpoint endpointSinglePortAnyIP(anyIPAddress, port);

   try 
   {
      socket.bind(endpointSinglePortAnyIP);
   }
	catch (boost::system::system_error const& e)
   {
	   logInfo(RTPS_MSG_OUT, "UDPv4 Error binding endpoint: (" << endpointSinglePortAnyIP << ")" << " with boost msg: "<<e.what() , C_YELLOW);
      return false;
   }
  
   mPortSocketMap.emplace(port, move(socket));
   return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
