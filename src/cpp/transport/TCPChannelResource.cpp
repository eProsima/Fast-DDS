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

#include <asio.hpp>
#include <fastrtps/transport/TCPChannelResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

TCPChannelResource::TCPChannelResource(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket)
    : ChannelResource()
    , mLocator(locator)
    , m_inputSocket(inputSocket)
    , mWaitingForKeepAlive(false)
    , mPendingLogicalPort(0)
    , mNegotiatingLogicalPort(0)
    , mCheckingLogicalPort(0)
	, mNegotiationSemaphore(0)
	, mSocket(moveSocket(socket))
    , mConnectionStatus(eConnectionStatus::eDisconnected)
    //, mLogicalConnections(0)
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
    //, mLogicalConnections(0)
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
	mNegotiationSemaphore.disable();

    Clear();

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

void TCPChannelResource::Disable()
{
	mNegotiationSemaphore.disable();

	ChannelResource::Disable();
}

bool TCPChannelResource::IsLogicalPortOpened(uint16_t port)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
	return std::find(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port) != mLogicalOutputPorts.end();
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

void TCPChannelResource::fillLogicalPorts(std::vector<Locator_t>& outVector)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    Locator_t temp = mLocator;
    for (uint16_t port : mPendingLogicalOutputPorts)
    {
        temp.set_logical_port(port);
        outVector.emplace_back(temp);
    }
    for (uint16_t port : mLogicalOutputPorts)
    {
        temp.set_logical_port(port);
        outVector.emplace_back(temp);
    }
}

uint32_t TCPChannelResource::GetMsgSize() const
{
	return m_rec_msg.max_size;
}

void TCPChannelResource::EnqueueLogicalPort(uint16_t port)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    if (std::find(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port)
        == mPendingLogicalOutputPorts.end()
        && std::find(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port) == mLogicalOutputPorts.end())
    {
        mPendingLogicalOutputPorts.emplace_back(port);
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
