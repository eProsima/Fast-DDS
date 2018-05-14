#ifndef RTPS_TCP_SOCKET_LISTENER_
#define RTPS_TCP_SOCKET_LISTENER_
#include <fastrtps/rtps/common/Locator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPInputSocketListener
{
public:
    virtual void onReceive(octet* receiveBuffer, uint32_t receiveBufferSize, 
        const Locator_t& localLocator, Locator_t& remoteLocator);
    virtual void onAccept(octet* receiveBuffer, uint32_t receiveBufferSize, 
        const Locator_t& localLocator, Locator_t& remoteLocator);
};

class TCPOutputSocketListener
{
public:
    virtual void onLogicalPortReceived(const Locator_t& localLocator, 
        const Locator_t& remoteLocator);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_SOCKET_LISTENER_