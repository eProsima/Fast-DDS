#ifndef UDPV4_TRANSPORT_DESCRIPTOR 
#define UDPV4_TRANSPORT_DESCRIPTOR

#include <fastrtps/transport/TransportInterface.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

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
 * @ingroup TRANSPORT_MODULE
 */
typedef struct UDPv4TransportDescriptor: public TransportDescriptorInterface {
   //! Length of the send buffer.
   uint32_t sendBufferSize;
   //! Length of the receive buffer.
   uint32_t receiveBufferSize;
   //! Whether a channel maps to a port (true) or to a port + address (false).
   bool granularMode;
   //! Allowed interfaces in an IP string format.
   std::vector<std::string> interfaceWhiteList;

   virtual ~UDPv4TransportDescriptor(){}
   RTPS_DllAPI UDPv4TransportDescriptor();
} UDPv4TransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
