// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fastrtps/transport/SocketInfo.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

SocketInfo::SocketInfo()
    : mAlive(true)
    , mThread(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

SocketInfo::SocketInfo(SocketInfo&& socketInfo)
    : m_rec_msg(std::move(socketInfo.m_rec_msg))
    , mThread(socketInfo.mThread)
{
    bool b = socketInfo.mAlive;
    mAlive = b;
    socketInfo.mThread = nullptr;
    //logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
    //m_rec_msg = std::move(socketInfo.m_rec_msg);
#if HAVE_SECURITY
    m_crypto_msg = std::move(socketInfo.m_crypto_msg);
#endif
}

SocketInfo::SocketInfo(uint32_t rec_buffer_size)
    : m_rec_msg(rec_buffer_size)
#if HAVE_SECURITY
    , m_crypto_msg(rec_buffer_size)
#endif
    , mAlive(true)
    , mThread(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

SocketInfo::~SocketInfo()
{
    mAlive = false;
    if (mThread != nullptr)
    {
        mThread->join();
        delete mThread;
        mThread = nullptr;
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
    : mMsgReceiver(socketInfo.mMsgReceiver)
    , socket_(moveSocket(socketInfo.socket_))
    , only_multicast_purpose_(socketInfo.only_multicast_purpose_)
{
    socketInfo.mMsgReceiver = nullptr;
}

UDPSocketInfo::~UDPSocketInfo()
{
    if (mMsgReceiver != nullptr)
        delete mMsgReceiver;
    mMsgReceiver = nullptr;
}

TCPSocketInfo::TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket)
    : mLocator(locator)
    , m_inputSocket(inputSocket)
    , mWaitingForKeepAlive(false)
    , mPendingLogicalPort(0)
    , mNegotiatingLogicalPort(0)
    , mCheckingLogicalPort(0)
    , mSocket(moveSocket(socket))
    , mConnectionStatus(eConnectionStatus::eDisconnected)
{
    mReadMutex = new std::recursive_mutex();
    mWriteMutex = new std::recursive_mutex();
    if (outputLocator)
    {
        mPendingLogicalOutputPorts.emplace_back(locator.get_logical_port());
        logInfo(RTCP, "Bound output locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")");
    }
    else
    {
        mLogicalInputPorts.emplace_back(locator.get_logical_port());
        logInfo(RTCP, "Bound input locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")");
    }
}

TCPSocketInfo::TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
    uint32_t maxMsgSize)
    : SocketInfo(maxMsgSize)
    , mLocator(locator)
    , m_inputSocket(inputSocket)
    , mWaitingForKeepAlive(false)
    , mPendingLogicalPort(0)
    , mNegotiatingLogicalPort(0)
    , mCheckingLogicalPort(0)
    , mSocket(moveSocket(socket))
    , mConnectionStatus(eConnectionStatus::eDisconnected)
{
    mReadMutex = new std::recursive_mutex();
    mWriteMutex = new std::recursive_mutex();
    if (outputLocator)
    {
        mPendingLogicalOutputPorts.emplace_back(locator.get_logical_port());
        logInfo(RTCP, "Bound output locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")");
    }
    else
    {
        mLogicalInputPorts.emplace_back(locator.get_logical_port());
        logInfo(RTCP, "Bound input locator (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")");
    }
}
/*
TCPSocketInfo::TCPSocketInfo(TCPSocketInfo&& socketInfo)
    : SocketInfo(std::move(socketInfo))
    , mLocator(socketInfo.mLocator)
    , m_inputSocket(socketInfo.m_inputSocket)
    , mWaitingForKeepAlive(socketInfo.m_inputSocket)
    , mPendingLogicalPort(socketInfo.mPendingLogicalPort)
    , mNegotiatingLogicalPort(socketInfo.mNegotiatingLogicalPort)
    , mCheckingLogicalPort(socketInfo.mCheckingLogicalPort)
    , mRTCPThread(socketInfo.mRTCPThread)
    , mReadMutex(socketInfo.mReadMutex)
    , mWriteMutex(socketInfo.mWriteMutex)
    , mSocket(moveSocket(socketInfo.mSocket))
    , mConnectionStatus(socketInfo.mConnectionStatus)
{
    socketInfo.mReadMutex = nullptr;
    socketInfo.mWriteMutex = nullptr;
    socketInfo.mRTCPThread = nullptr;
    std::cout << "############ MOVE CTOR ###########" << std::endl;
}
*/
TCPSocketInfo::~TCPSocketInfo()
{
    mAlive = false;
    for (auto it = mReceiversMap.begin(); it != mReceiversMap.end(); ++it)
    {
        if (it->second != nullptr)
        {
            delete(it->second);
        }
    }
    mReceiversMap.clear();

    if (mReadMutex != nullptr)
    {
        delete mReadMutex;
    }
    if (mWriteMutex != nullptr)
    {
        delete mWriteMutex;
    }

    if (mRTCPThread != nullptr)
    {
        mRTCPThread->join();
        delete(mRTCPThread);
        mRTCPThread = nullptr;
    }
}

std::thread* TCPSocketInfo::ReleaseRTCPThread()
{
    std::thread* outThread = mRTCPThread;
    mRTCPThread = nullptr;
    return outThread;
}

bool TCPSocketInfo::AddMessageReceiver(uint16_t logicalPort, MessageReceiver* receiver)
{
    if (mReceiversMap.find(logicalPort) == mReceiversMap.end())
    {
        mReceiversMap[logicalPort] = receiver;
        return true;
    }
    return false;
}

MessageReceiver* TCPSocketInfo::GetMessageReceiver(uint16_t logicalPort)
{
    if (mReceiversMap.find(logicalPort) != mReceiversMap.end())
        return mReceiversMap[logicalPort];
    return nullptr;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
