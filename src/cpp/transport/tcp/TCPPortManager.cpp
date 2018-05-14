#include <fastrtps/transport/tcp/TCPPortManager.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const asio::ip::tcp& getInternetProtocol(const Locator_t &locator)
{
    if (locator.kind == LOCATOR_KIND_TCPv4)
    {
        return asio::ip::tcp::v4();
    }
    else if (locator.kind == LOCATOR_KIND_TCPv6)
    {
        return asio::ip::tcp::v6();
    }
    return asio::ip::tcp::v4(); // Default?
}

static asio::ip::address_v4::bytes_type locatorToNativev4(const Locator_t& locator)
{
    return{ {locator.address[12], locator.address[13], locator.address[14], locator.address[15]} };
}

bool TCPPortManager::IsOpenInput()
{
    return m_socket.getSocket()->is_open();
}

bool TCPPortManager::IsOpenOutput(const Locator_t& remoteLocator)
{
    if (m_OutputConnections.find(remoteLocator) != m_OutputConnections.end())
    {
        return m_OutputConnections.at(remoteLocator)->getSocket()->is_open();
    }
    return false;
}

bool TCPPortManager::OpenInput(Locator_t &locator, const TCPInputSocketListener* listener)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;
    
    if (!IsOpenInput())
    {
        return OpenAndBindInputSockets(locator, listener);
    }
    else
    {
        RegisterListener(listener);
        return true;
    }
}

bool TCPPortManager::OpenOutput(Locator_t &locator, const TCPOutputSocketListener* listener)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    if (!IsOpenOutput(locator))
    {
        // TODO Connect, assign logical port to my Locator_t and call listener
    }
    else
    {
        // TODO OpenLogicalPort, set logical port and call listener
    }
}

bool TCPPortManager::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator)
{

}

void TCPPortManager::SocketAccepted(const Locator_t& locator, 
    uint32_t receiveBufferSize, const asio::error_code& error)
{

}

void TCPPortManager::SocketConnected(const Locator_t& locator, 
    uint32_t id, uint32_t sendBufferSize, const asio::error_code& error)
{

}

void TCPPortManager::Connect(Locator_t &locator)
{

}

void TCPPortManager::RetryConnect(asio::io_service& io_service)
{

}

bool TCPPortManager::IsLocatorSupported(const Locator_t&) const
{

}

bool TCPPortManager::OpenAndBindInputSockets(Locator_t &locator, const TCPInputSocketListener *listener)
{

}

void TCPPortManager::OpenAndBindInputSocket(Locator_t &locator, const TCPInputSocketListener *listener)
{

}

void TCPPortManager::RegisterListener(const TCPInputSocketListener *listener)
{
    m_AcceptListeners.emplace_back(listener);
}

TCPPortManager::TCPAccepter::TCPAccepter(asio::io_service& io_service, uint16_t port, uint32_t receiveBufferSize)
{

}

void TCPPortManager::TCPAccepter::Accept()
{

}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima