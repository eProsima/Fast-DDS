#ifndef EPROSIMA_FASTRTPS_TCP_LOCATOR_
#define EPROSIMA_FASTRTPS_TCP_LOCATOR_

#include "TCPManager.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPManagerLocator
{
public:
    /** Returns if service was registered. Only first service provided will be registered. 
     */
    static bool registerTCPManager(TCPManager* service);
    static TCPManager* getTCPManager();
private:
    TCPManagerLocator() = delete;
    static TCPManager* m_Manager;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // EPROSIMA_FASTRTPS_TCP_LOCATOR_