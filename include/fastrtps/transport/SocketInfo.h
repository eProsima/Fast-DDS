#ifndef RTPS_SOCKET_INFO_
#define RTPS_SOCKET_INFO_

#include <asio.hpp>
#include <memory>
#include <map>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastrtps/log/Log.h>

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
        : mAlive(true)
        , mThread(nullptr)
        , mAutoRelease(true)
    {
        logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
    }

    SocketInfo(uint32_t rec_buffer_size)
        : m_rec_msg(rec_buffer_size)
#if HAVE_SECURITY
        , m_crypto_msg(rec_buffer_size)
#endif
        , mAlive(true)
        , mThread(nullptr)
        , mAutoRelease(true)
    {
        logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
    }

    virtual ~SocketInfo()
    {
        mAlive = false;
        if (mAutoRelease)
        {
            if (mThread != nullptr)
            {
                mThread->join();
                delete mThread;
            }
        }
        else
        {
            assert(mThread == nullptr);
        }
    }

    inline void SetThread(std::thread* pThread)
    {
        mThread = pThread;
    }

    inline void SetAutoRelease(bool bRelease)
    {
        mAutoRelease = bRelease;
    }

    inline std::thread* ReleaseThread()
    {
        std::thread* outThread = mThread;
        mThread = nullptr;
        return outThread;
    }

    inline bool IsAlive() const
    {
        return mAlive;
    }

    inline void Disable()
    {
        mAlive = false;
    }

    inline CDRMessage_t& GetMessageBuffer()
    {
        return m_rec_msg;
    }

protected:
    //!Received message
    CDRMessage_t m_rec_msg;
#if HAVE_SECURITY
    CDRMessage_t m_crypto_msg;
#endif

    bool mAlive;
    std::thread* mThread;
    bool mAutoRelease;
};

class UDPSocketInfo : public SocketInfo
{
public:
    UDPSocketInfo(eProsimaUDPSocket& socket)
        : mMsgReceiver(nullptr)
        , socket_(moveSocket(socket))
        , only_multicast_purpose_(false)
    {
    }

    UDPSocketInfo(eProsimaUDPSocket& socket, uint32_t maxMsgSize)
        : SocketInfo(maxMsgSize)
        , mMsgReceiver(nullptr)
        , socket_(moveSocket(socket))
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
        mMsgReceiver = nullptr;
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

    inline void SetMessageReceiver(std::shared_ptr<MessageReceiver> receiver)
    {
        mMsgReceiver = receiver;
    }

    inline std::shared_ptr<MessageReceiver> GetMessageReceiver()
    {
        return mMsgReceiver;
    }

private:

    std::shared_ptr<MessageReceiver> mMsgReceiver; //Associated Readers/Writers inside of MessageReceiver
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
    TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
        bool autoRelease)
        : m_locator(locator)
        , m_physicalPort(0)
        , m_inputSocket(inputSocket)
        , mWaitingForKeepAlive(false)
        , mPendingLogicalPort(0)
        , mSocket(moveSocket(socket))
        , mConnectionStatus(eConnectionStatus::eDisconnected)
    {
        mAutoRelease = autoRelease;
        mReadMutex = std::make_shared<std::recursive_mutex>();
        mWriteMutex = std::make_shared<std::recursive_mutex>();
        if (outputLocator)
        {
            mPendingLogicalOutputPorts.emplace_back(locator.get_logical_port());
            std::cout << "[RTCP] Bound output locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")" << std::endl;
        }
        else
        {
            mLogicalInputPorts.emplace_back(locator.get_logical_port());
            std::cout << "[RTCP] Bound input locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")" << std::endl;
        }
    }

    TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
        bool autoRelease, uint32_t maxMsgSize)
        : SocketInfo(maxMsgSize)
        , m_locator(locator)
        , m_physicalPort(0)
        , m_inputSocket(inputSocket)
        , mWaitingForKeepAlive(false)
        , mPendingLogicalPort(0)
        , mSocket(moveSocket(socket))
        , mConnectionStatus(eConnectionStatus::eDisconnected)
    {
        mAutoRelease = autoRelease;
        mReadMutex = std::make_shared<std::recursive_mutex>();
        mWriteMutex = std::make_shared<std::recursive_mutex>();
        if (outputLocator)
        {
            mPendingLogicalOutputPorts.emplace_back(locator.get_logical_port());
            std::cout << "[RTCP] Bound output locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")" << std::endl;
        }
        else
        {
            mLogicalInputPorts.emplace_back(locator.get_logical_port());
            std::cout << "[RTCP] Bound input locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")" << std::endl;
        }
    }

    TCPSocketInfo(TCPSocketInfo&& socketInfo)
        : m_locator(socketInfo.m_locator)
        , m_physicalPort(socketInfo.m_physicalPort)
        , m_inputSocket(socketInfo.m_inputSocket)
        , mWaitingForKeepAlive(socketInfo.m_inputSocket)
        , mPendingLogicalPort(0)
        , mReadMutex(socketInfo.mReadMutex)
        , mWriteMutex(socketInfo.mWriteMutex)
        , mSocket(moveSocket(socketInfo.mSocket))
        , mConnectionStatus(socketInfo.mConnectionStatus)
    {
    }

    virtual ~TCPSocketInfo()
    {
        mAlive = false;
        if (mAutoRelease)
        {
            if (mRTCPThread != nullptr)
            {
                mRTCPThread->join();
                delete mRTCPThread;
            }
        }
        else
        {
            assert(mRTCPThread == nullptr);
        }
    }

    TCPSocketInfo& operator=(TCPSocketInfo&& socketInfo)
    {
        mSocket = moveSocket(socketInfo.mSocket);
        return *this;
    }

    bool operator==(const TCPSocketInfo& socketInfo) const
    {
        return &mSocket == &(socketInfo.mSocket);
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaTCPSocket* getSocket()
#else
    inline eProsimaTCPSocket getSocket()
#endif
    {
        return getSocketPtr(mSocket);
    }

    std::recursive_mutex& GetReadMutex() const
    {
        return *mReadMutex;
    }

    std::recursive_mutex& GetWriteMutex() const
    {
        return *mWriteMutex;
    }

    inline void SetRTCPThread(std::thread* pThread)
    {
        mRTCPThread = pThread;
    }

    inline std::thread* ReleaseRTCPThread()
    {
        std::thread* outThread = mRTCPThread;
        mRTCPThread = nullptr;
        return outThread;
    }

    inline void SetPhysicalPort(uint16_t port)
    {
        m_physicalPort = port;
    }

    inline uint16_t GetPhysicalPort() const
    {
        return m_physicalPort;
    }

    inline bool GetIsInputSocket() const
    {
        return m_inputSocket;
    }

    inline void SetIsInputSocket(bool bInput)
    {
        m_inputSocket = bInput;
    }

    bool IsConnectionEstablished()
    {
        return mConnectionStatus == eConnectionStatus::eEstablished;
    }

    inline bool AddMessageReceiver(uint16_t logicalPort, std::shared_ptr<MessageReceiver> receiver)
    {
        if (mReceiversMap.find(logicalPort) == mReceiversMap.end())
        {
            mReceiversMap[logicalPort] = receiver;
            return true;
        }
        return nullptr;
    }

    std::shared_ptr<MessageReceiver> GetMessageReceiver(uint16_t logicalPort)
    {
        if (mReceiversMap.find(logicalPort) != mReceiversMap.end())
            return mReceiversMap[logicalPort];
        return nullptr;
    }

protected:
    inline void ChangeStatus(eConnectionStatus s)
    {
        mConnectionStatus = s;
    }

    friend class TCPv4Transport;
    friend class RTCPMessageManager;

private:
    Locator_t m_locator;
    uint16_t m_physicalPort;
    bool m_inputSocket;
    bool mWaitingForKeepAlive;
    uint16_t mPendingLogicalPort;
    std::thread* mRTCPThread;
    std::vector<uint16_t> mPendingLogicalOutputPorts;
    std::vector<uint16_t> mLogicalOutputPorts;
    std::vector<uint16_t> mLogicalInputPorts;
    std::shared_ptr<std::recursive_mutex> mReadMutex;
    std::shared_ptr<std::recursive_mutex> mWriteMutex;
    std::map<uint16_t, std::shared_ptr<MessageReceiver>> mReceiversMap;  // The key is the logical port.
    eProsimaTCPSocket mSocket;
    eConnectionStatus mConnectionStatus;
    TCPSocketInfo(const TCPSocketInfo&) = delete;
    TCPSocketInfo& operator=(const TCPSocketInfo&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_PORT_MANAGER_