#include <fastrtps/transport/tcp/TCPManager.h>
#include <fastrtps/transport/tcp/TCPManagerLocator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

TCPManager* TCPManagerLocator::m_Manager = nullptr;

bool TCPManagerLocator::registerTCPManager(TCPManager* service)
{
    if (m_Manager == nullptr)
    {
        m_Manager = service;
    }
    return m_Manager == service;
}

TCPManager* TCPManagerLocator::getTCPManager()
{
    if (m_Manager == nullptr)
    {
        m_Manager = new TCPManager();
    }
    return m_Manager;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima