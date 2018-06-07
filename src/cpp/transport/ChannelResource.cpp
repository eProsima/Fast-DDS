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

#include <fastrtps/transport/ChannelResource.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

ChannelResource::ChannelResource()
    : mAlive(true)
    , mThread(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

ChannelResource::ChannelResource(ChannelResource&& channelResource)
    : m_rec_msg(std::move(channelResource.m_rec_msg))
    , mThread(channelResource.mThread)
{
    bool b = channelResource.mAlive;
    mAlive = b;
    channelResource.mThread = nullptr;
    //logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
    //m_rec_msg = std::move(channelResource.m_rec_msg);
#if HAVE_SECURITY
    m_crypto_msg = std::move(channelResource.m_crypto_msg);
#endif
}

ChannelResource::ChannelResource(uint32_t rec_buffer_size)
    : m_rec_msg(rec_buffer_size)
#if HAVE_SECURITY
    , m_crypto_msg(rec_buffer_size)
#endif
    , mAlive(true)
    , mThread(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

ChannelResource::~ChannelResource()
{
    mAlive = false;
    if (mThread != nullptr)
    {
        mThread->join();
        delete mThread;
        mThread = nullptr;
    }
}

std::thread* ChannelResource::ReleaseThread()
{
    std::thread* outThread = mThread;
    mThread = nullptr;
    return outThread;
}

UDPChannelResource::UDPChannelResource(eProsimaUDPSocket& socket)
    : mMsgReceiver(nullptr)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
{
}

UDPChannelResource::UDPChannelResource(eProsimaUDPSocket& socket, uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , mMsgReceiver(nullptr)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
{
}

UDPChannelResource::UDPChannelResource(UDPChannelResource&& channelResource)
    : mMsgReceiver(channelResource.mMsgReceiver)
    , socket_(moveSocket(channelResource.socket_))
    , only_multicast_purpose_(channelResource.only_multicast_purpose_)
{
    channelResource.mMsgReceiver = nullptr;
}

UDPChannelResource::~UDPChannelResource()
{
    if (mMsgReceiver != nullptr)
        delete mMsgReceiver;
    mMsgReceiver = nullptr;
}

TCPChannelResource::TCPChannelResource(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket)
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

TCPChannelResource::TCPChannelResource(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
    uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
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
TCPChannelResource::TCPChannelResource(TCPChannelResource&& channelResource)
    : ChannelResource(std::move(channelResource))
    , mLocator(channelResource.mLocator)
    , m_inputSocket(channelResource.m_inputSocket)
    , mWaitingForKeepAlive(channelResource.m_inputSocket)
    , mPendingLogicalPort(channelResource.mPendingLogicalPort)
    , mNegotiatingLogicalPort(channelResource.mNegotiatingLogicalPort)
    , mCheckingLogicalPort(channelResource.mCheckingLogicalPort)
    , mRTCPThread(channelResource.mRTCPThread)
    , mReadMutex(channelResource.mReadMutex)
    , mWriteMutex(channelResource.mWriteMutex)
    , mSocket(moveSocket(channelResource.mSocket))
    , mConnectionStatus(channelResource.mConnectionStatus)
{
    channelResource.mReadMutex = nullptr;
    channelResource.mWriteMutex = nullptr;
    channelResource.mRTCPThread = nullptr;
    std::cout << "############ MOVE CTOR ###########" << std::endl;
}
*/
TCPChannelResource::~TCPChannelResource()
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

std::thread* TCPChannelResource::ReleaseRTCPThread()
{
    std::thread* outThread = mRTCPThread;
    mRTCPThread = nullptr;
    return outThread;
}

bool TCPChannelResource::AddMessageReceiver(uint16_t logicalPort, MessageReceiver* receiver)
{
    if (mReceiversMap.find(logicalPort) == mReceiversMap.end())
    {
        mReceiversMap[logicalPort] = receiver;
        return true;
    }
    return false;
}

MessageReceiver* TCPChannelResource::GetMessageReceiver(uint16_t logicalPort)
{
    if (mReceiversMap.find(logicalPort) != mReceiversMap.end())
        return mReceiversMap[logicalPort];
    return nullptr;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
