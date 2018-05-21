#ifndef RTPS_SOCKET_INFO_
#define RTPS_SOCKET_INFO_

#include <asio.hpp>
#include <memory>
#include <fastrtps/rtps/messages/MessageReceiver.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

#if defined(ASIO_HAS_MOVE)
    // Typedefs
	typedef asio::ip::udp::socket eProsimaUDPSocket;
	typedef asio::ip::tcp::socket eProsimaTCPSocket;
    // UDP
	inline eProsimaUDPSocket* getSocketPtr(eProsimaUDPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaUDPSocket moveSocket(eProsimaUDPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::udp::socket(io_service));
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket* socket)
    {
        return *socket;
    }
    // TCP
	inline eProsimaTCPSocket* getSocketPtr(eProsimaTCPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaTCPSocket moveSocket(eProsimaTCPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::tcp::socket(io_service));
    }
    inline eProsimaTCPSocket& getRefFromPtr(eProsimaTCPSocket* socket)
    {
        return *socket;
    }
#else
    // Typedefs
	typedef std::shared_ptr<asio::ip::udp::socket> eProsimaUDPSocket;
	typedef std::shared_ptr<asio::ip::tcp::socket> eProsimaTCPSocket;
    // UDP
    inline eProsimaUDPSocket getSocketPtr(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket& moveSocket(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::udp::socket>(io_service);
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    // TCP
    inline eProsimaTCPSocket getSocketPtr(eProsimaTCPSocket &socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket& moveSocket(eProsimaTCPSocket &socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::tcp::socket>(io_service);
    }
    inline eProsimaTCPSocket& getRefFromPtr(eProsimaTCPSocket &socket)
    {
        return socket;
    }
#endif

class SocketInfo
{
public:
    SocketInfo()
    : mp_receiver(nullptr)
    , m_bAlive(true)
    , m_thread(nullptr)
    {
    }

    virtual ~SocketInfo()
    {
        m_bAlive = false;
        if (m_thread != nullptr)
        {
            m_thread->join();
            delete m_thread;
        }
        mp_receiver = nullptr;
    }

    inline void SetThread(std::thread* pThread)
    {
        m_thread = pThread;
    }

    inline bool IsAlive() const
    {
        return m_bAlive;
    }

    inline void Disable()
    {
        m_bAlive = false;
    }

    inline void SetMessageReceiver(std::shared_ptr<MessageReceiver> receiver)
    {
        mp_receiver = receiver;
    }

    inline std::shared_ptr<MessageReceiver> GetMessageReceiver()
    {
        return mp_receiver;
    }
protected:
    std::shared_ptr<MessageReceiver> mp_receiver; //Associated Readers/Writers inside of MessageReceiver
    bool m_bAlive;
    std::thread* m_thread;
};

class UDPSocketInfo : public SocketInfo
{
public:
    UDPSocketInfo(eProsimaUDPSocket& socket)
        : socket_(moveSocket(socket))
        , only_multicast_purpose_(false)
    {
    }

    UDPSocketInfo(UDPSocketInfo&& socketInfo)
        : socket_(moveSocket(socketInfo.socket_))
        , only_multicast_purpose_(false)
    {
    }

    virtual ~UDPSocketInfo()
    {
    }

    UDPSocketInfo& operator=(UDPSocketInfo&& socketInfo)
    {
        socket_ = moveSocket(socketInfo.socket_);
        return *this;
    }

    void only_multicast_purpose(const bool value)
    {
        only_multicast_purpose_ = value;
    };

    bool& only_multicast_purpose()
    {
        return only_multicast_purpose_;
    }

    bool only_multicast_purpose() const
    {
        return only_multicast_purpose_;
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaUDPSocket* getSocket()
#else
    inline eProsimaUDPSocket getSocket()
#endif
    {
        return getSocketPtr(socket_);
    }

private:

    eProsimaUDPSocket socket_;
    bool only_multicast_purpose_;
    UDPSocketInfo(const UDPSocketInfo&) = delete;
    UDPSocketInfo& operator=(const UDPSocketInfo&) = delete;
};

class TCPSocketInfo : public SocketInfo
{
enum eConnectionStatus
{
    eDisconnected = 0,
    eConnected,                 // Output -> Send bind message.
    eWaitingForBind,            // Input -> Waiting for the bind message.
    eWaitingForBindResponse,    // Output -> Waiting for the bind response message.
    eEstablished,
    eUnbinding
};

public:
    TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator)
        : m_locator(locator)
        , m_physicalPort(0)
        , m_inputSocket(false)
        , socket_(moveSocket(socket))
        , mConnectionStatus(eConnectionStatus::eDisconnected)
    {
        mMutex = std::make_shared<std::recursive_mutex>();
    }

    TCPSocketInfo(TCPSocketInfo&& socketInfo)
        : m_locator(socketInfo.m_locator)
        , m_physicalPort(0)
        , m_inputSocket(false)
        , socket_(moveSocket(socketInfo.socket_))
        , mConnectionStatus(eConnectionStatus::eDisconnected)
    {
        mMutex = std::make_shared<std::recursive_mutex>();
    }

    virtual ~TCPSocketInfo()
    {
    }

    TCPSocketInfo& operator=(TCPSocketInfo&& socketInfo)
    {
        socket_ = moveSocket(socketInfo.socket_);
        return *this;
    }

    bool operator==(const TCPSocketInfo& socketInfo) const
    {
        return &socket_ == &(socketInfo.socket_);
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaTCPSocket* getSocket()
#else
    inline eProsimaTCPSocket getSocket()
#endif
    {
        return getSocketPtr(socket_);
    }

    std::recursive_mutex& GetMutex() const
    {
        return *mMutex;
    }

    inline void SetPhysicalPort(uint16_t port)
    {
        m_physicalPort = port;
    }

    inline uint16_t GetPhysicalPort() const
    {
        return m_physicalPort;
    }

    inline void SetIsInputSocket(bool bInput)
    {
        m_inputSocket = bInput;
    }
protected:
    inline void ChangeStatus(eConnectionStatus s)
    {
        mConnectionStatus = s;
    }

    friend class TCPv4Transport;

private:
    Locator_t m_locator;
    uint16_t m_physicalPort;
    bool m_inputSocket;
    std::vector<uint16_t> mPendingLogicalPorts;
    std::vector<uint16_t> mLogicalPorts;
    std::shared_ptr<std::recursive_mutex> mMutex;
    eProsimaTCPSocket socket_;
    eConnectionStatus mConnectionStatus;
    TCPSocketInfo(const TCPSocketInfo&) = delete;
    TCPSocketInfo& operator=(const TCPSocketInfo&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_PORT_MANAGER_