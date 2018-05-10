#include <fastrtps/transport/tcp/TCPManager.h>
#include <memory>
#include <functional>

namespace eprosima{
namespace fastrtps{
namespace rtps{

#if defined(ASIO_HAS_MOVE)
	inline TCPSocket* getSocketPtr(TCPSocket &socket)
    {
        return &socket;
    }
    inline TCPSocket&& moveSocket(TCPSocket &socket)
    {
        return std::move(socket);
    }
    inline TCPSocket&& createSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::tcp::socket(io_service));
    }
#else
    inline TCPSocket getSocketPtr(TCPSocket &socket)
    {
        return socket;
    }
    inline TCPSocket& moveSocket(TCPSocket &socket)
    {
        return socket;
    }
    inline TCPSocket&& createSocket(asio::io_service& io_service)
    {
        return std::move(std::make_shared<asio::ip::tcp::socket>(io_service));
    }
#endif

bool TCPManager::IsOpenSocket(const Locator_t &locator)
{
    if (m_LocatorInputSockets.find(locator) != m_LocatorInputSockets.end())
    {
        auto& socket = m_LocatorInputSockets.at(locator);
        return getSocketPtr(socket)->is_open();
    }
    else if (m_LocatorOutputSockets.find(locator) != m_LocatorOutputSockets.end())
    {
        auto& socket = m_LocatorOutputSockets.at(locator);
        return getSocketPtr(socket)->is_open();
    }

    return false;
}

bool TCPManager::OpenInputSocket(Locator_t &locator, const TCPSocketListener &listener)
{
    // TODO Open and accept!
    if (!IsOpenSocket(locator))
    {
        // created socket as TCPSocket
        // TCPSocket newSocket = blah blah;
        // newSocket->accept(m_myListener);
        // m_LocatorInputSockets(locator, moveSocket(newSocket));
        // return success;
    }
    return false;
}

bool TCPManager::OpenOutputSocket(Locator_t &locator)
{
    // TODO Open and connect!
    if (!IsOpenSocket(locator))
    {
        // created socket as TCPSocket
        // TCPSocket newSocket = blah blah;
        // newSocket->accept(m_myListener);
        // m_LocatorInputSockets(locator, moveSocket(newSocket));
        // return success;
    }
    return false;
}

bool TCPManager::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator)
{
    return false;
}


TCPManagerAccepter::TCPManagerAccepter(asio::io_service& io_service, uint16_t port, uint32_t receiveBufferSize)
    : m_acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    , m_port(port)
    , m_receiveBufferSize(receiveBufferSize)
    , m_socket(createSocket(io_service))
{
}

void TCPManagerAccepter::Accept(TCPManager *manager)
{
    m_acceptor.async_accept(m_socket, std::bind(&TCPManager::SocketAccepted, manager, m_port, m_receiveBufferSize,
        std::placeholders::_1));
}

void TCPManagerAccepter::RetryAccept(asio::io_service& io_service, TCPManager *manager)
{
    getSocketPtr(m_socket)->close();
    m_socket = createSocket(io_service);
    Accept(manager);
}

TCPManagerConnector::TCPManagerConnector(asio::io_service& io_service, const asio::ip::address_v4& ipAddress, uint16_t port, uint32_t id, uint32_t sendBufferSize)
    : m_port(port)
    , m_id(id)
    , m_ipAddress(ipAddress)
    , m_sendBufferSize(sendBufferSize)
    , m_socket(createSocket(io_service))
{
}

void TCPManagerConnector::Connect(TCPManager *manager, uint32_t& port)
{
    m_socket.open(asio::ip::tcp::v4());
    asio::ip::tcp::endpoint endpoint(m_ipAddress, static_cast<uint16_t>(port));
    m_socket.async_connect(endpoint, 
        std::bind(&TCPManager::SocketConnected, manager, m_port, m_id, m_sendBufferSize, std::placeholders::_1));

    if (port == 0)
        port = m_socket.local_endpoint().port();
}

void TCPManagerConnector::RetryConnect(asio::io_service& io_service, TCPManager *manager)
{
    getSocketPtr(m_socket)->close();
    m_socket = createSocket(io_service);
    Connect(manager, m_port);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
