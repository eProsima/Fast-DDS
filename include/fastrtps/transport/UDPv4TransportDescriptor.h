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
 */
typedef struct UDPv4TransportDescriptor: public TransportDescriptorInterface {
   uint32_t sendBufferSize;
   uint32_t receiveBufferSize;
   bool granularMode;
   std::vector<std::string> interfaceWhiteList;

   virtual ~UDPv4TransportDescriptor(){}
   RTPS_DllAPI UDPv4TransportDescriptor();
} UDPv4TransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
