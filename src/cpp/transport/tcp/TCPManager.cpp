#include <fastrtps/transport/tcp/TCPManager.h>
#include <fastrtps/log/Log.h>
#include <memory>
#include <functional>

namespace eprosima{
namespace fastrtps{
namespace rtps{

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
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;
    
    if (!IsOpenSocket(locator))
    {
        return OpenAndBindInputSockets(locator, listener);
    }
    return false;
}


        // created socket as TCPSocket
        // TCPSocket newSocket = blah blah;
        // newSocket->accept(m_myListener);
        // m_LocatorInputSockets(locator, moveSocket(newSocket));
        // return success;

bool TCPManager::OpenAndBindInputSockets(Locator_t &locator, const TCPSocketListener &listener)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        OpenAndBindInputSocket(locator, listener);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, (locator.kind == LOCATOR_KIND_TCPv4 ? "TCPv4" : "TCPv6") << " Error binding at port: (" << locator.get_port() << ")" << " with msg: " << e.what());
        return false;
    }

    return true;
}

void TCPManager::OpenAndBindInputSocket(Locator_t &locator, const TCPSocketListener &listener)
{
    mInputSemaphores.emplace(locator, new Semaphore(0));
    TCPManagerAccepter* newAccepter = new TCPManagerAccepter(mService, static_cast<uint16_t>(port), mReceiveBufferSize);
    mPendingInputSockets.insert(std::make_pair(port, newAccepter));
    newAccepter->Accept(this);
}

bool TCPManager::OpenOutputSocket(Locator_t &locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
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

bool TCPManager::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6;
}

bool TCPManager::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator)
{
    return false;
}

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

static asio::ip::address_v6::bytes_type locatorToNativev6(const Locator_t& locator)
{
    return{ {locator.address[0], locator.address[1], locator.address[2], locator.address[3],
        locator.address[4], locator.address[5], locator.address[6], locator.address[7],
        locator.address[8], locator.address[9], locator.address[10], locator.address[11],
        locator.address[12], locator.address[13], locator.address[14], locator.address[15]} };
}

TCPManagerAccepter::TCPManagerAccepter(asio::io_service& io_service, Locator_t &locator, uint32_t receiveBufferSize)
    : m_acceptor(io_service, asio::ip::tcp::endpoint(getInternetProtocol(locator), locator.get_TCP_port()))
    , m_Locator(locator)
    , m_receiveBufferSize(receiveBufferSize)
    , m_socket(createSocket(io_service))
{
}

void TCPManagerAccepter::Accept(TCPManager *manager)
{
    m_acceptor.async_accept(m_socket, std::bind(&TCPManager::SocketAccepted, manager, m_Locator, m_receiveBufferSize,
        std::placeholders::_1));
}

void TCPManagerAccepter::RetryAccept(asio::io_service& io_service, TCPManager *manager)
{
    getSocketPtr(m_socket)->close();
    m_socket = createSocket(io_service);
    Accept(manager);
}

TCPManagerConnector::TCPManagerConnector(asio::io_service& io_service, Locator_t &locator, uint32_t id, uint32_t sendBufferSize)
    : m_Locator(&locator)
    , m_id(id)
    , m_sendBufferSize(sendBufferSize)
    , m_socket(createSocket(io_service))
{
}

Locator_t& TCPManagerConnector::Connect(TCPManager *manager, Locator_t &locator)
{
    m_socket.open(getInternetProtocol(locator));
    asio::ip::tcp::endpoint endpoint;
    if (locator.kind == LOCATOR_KIND_TCPv4)
    {
        endpoint = asio::ip::tcp::endpoint(locatorToNativev4(locator), locator.get_TCP_port());
    }
    else
    {
        endpoint = asio::ip::tcp::endpoint(locatorToNativev6(locator), locator.get_TCP_port());
    }
    //asio::ip::tcp::endpoint endpoint(m_ipAddress, static_cast<uint16_t>(port));
    m_socket.async_connect(endpoint, 
        std::bind(&TCPManager::SocketConnected, manager, m_Locator, m_id, m_sendBufferSize, std::placeholders::_1));

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
