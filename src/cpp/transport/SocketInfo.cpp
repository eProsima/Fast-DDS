#include <fastrtps/transport/SocketInfo.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

SocketInfo::SocketInfo()
    : mAlive(true)
    , mThread(nullptr)
    , mAutoRelease(true)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

SocketInfo::SocketInfo(uint32_t rec_buffer_size)
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

SocketInfo::~SocketInfo()
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

std::thread* SocketInfo::ReleaseThread()
{
    std::thread* outThread = mThread;
    mThread = nullptr;
    return outThread;
}

UDPSocketInfo::UDPSocketInfo(eProsimaUDPSocket& socket)
    : mMsgReceiver(nullptr)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
{
}

UDPSocketInfo::UDPSocketInfo(eProsimaUDPSocket& socket, uint32_t maxMsgSize)
    : SocketInfo(maxMsgSize)
    , mMsgReceiver(nullptr)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
{
}

UDPSocketInfo::UDPSocketInfo(UDPSocketInfo&& socketInfo)
    : socket_(moveSocket(socketInfo.socket_))
    , only_multicast_purpose_(false)
{
}

UDPSocketInfo::~UDPSocketInfo()
{
    mMsgReceiver = nullptr;
}


TCPSocketInfo::TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
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

TCPSocketInfo::TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
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

TCPSocketInfo::TCPSocketInfo(TCPSocketInfo&& socketInfo)
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

TCPSocketInfo::~TCPSocketInfo()
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

std::thread* TCPSocketInfo::ReleaseRTCPThread()
{
    std::thread* outThread = mRTCPThread;
    mRTCPThread = nullptr;
    return outThread;
}

bool TCPSocketInfo::AddMessageReceiver(uint16_t logicalPort, std::shared_ptr<MessageReceiver> receiver)
{
    if (mReceiversMap.find(logicalPort) == mReceiversMap.end())
    {
        mReceiversMap[logicalPort] = receiver;
        return true;
    }
    return nullptr;
}

std::shared_ptr<MessageReceiver> TCPSocketInfo::GetMessageReceiver(uint16_t logicalPort)
{
    if (mReceiversMap.find(logicalPort) != mReceiversMap.end())
        return mReceiversMap[logicalPort];
    return nullptr;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
