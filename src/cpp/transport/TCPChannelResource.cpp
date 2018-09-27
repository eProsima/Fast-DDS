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
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/eClock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Search for the base port in the current domain without taking account the participant
 */
static uint16_t GetBaseAutoPort(uint16_t currentPort)
{
    if (currentPort < 7411)
    {
        return currentPort;
    }
    uint16_t aux = currentPort - 7411; // base + offset3
    uint16_t domain = static_cast<uint16_t>(aux / 250.);
    uint16_t part = static_cast<uint16_t>(aux % 250);
    part = part / 2;

    //std::cout << "GetBaseAutoPort(" << currentPort << "): Domain = " << domain << " & Part = " << part << std::endl;
    return 7411 + (domain * 250); // And participant 0
}

TCPChannelResource::TCPChannelResource(TCPTransportInterface* parent, RTCPMessageManager* rtcpManager,
        eProsimaTCPSocketRef socket, const Locator_t& locator)
    : ChannelResource()
    , mParent (parent)
    , mRTCPManager(rtcpManager)
    , mLocator(locator)
    , m_inputSocket(false)
    , mWaitingForKeepAlive(false)
    , mNegotiationSemaphore(0)
    , mSocket(moveSocket(socket))
    , mConnectionStatus(eConnectionStatus::eDisconnected)
{
}

TCPChannelResource::TCPChannelResource(TCPTransportInterface* parent, RTCPMessageManager* rtcpManager,
        eProsimaTCPSocketRef socket)
    : ChannelResource()
    , mParent(parent)
    , mRTCPManager(rtcpManager)
    , mLocator()
    , m_inputSocket(true)
    , mWaitingForKeepAlive(false)
	, mNegotiationSemaphore(0)
	, mSocket(moveSocket(socket))
    , mConnectionStatus(eConnectionStatus::eWaitingForBind)
{
}

TCPChannelResource::~TCPChannelResource()
{
	mNegotiationSemaphore.disable();

    Clear();

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

    Disconnect();
}

void TCPChannelResource::Connect()
{
    if (mConnectionStatus == eConnectionStatus::eDisconnected)
    {
        mConnectionStatus = eConnectionStatus::eConnecting;
        auto type = mParent->GetProtocolType();
        auto endpoint = mParent->GenerateLocalEndpoint(mLocator, IPLocator::getPhysicalPort(mLocator));
        mSocket.open(type);
        mSocket.async_connect(endpoint, std::bind(&TCPChannelResource::SocketConnected, this,
            std::placeholders::_1));
    }
}

ResponseCode TCPChannelResource::ProcessBindRequest(const Locator_t& locator)
{
    
    if (mConnectionStatus == TCPChannelResource::eConnectionStatus::eWaitingForBind)
    {
        mLocator = locator;
        TCPChannelResource* oldChannel = mParent->BindSocket(mLocator, this);
        if (oldChannel != nullptr)
        {
            CopyPendingPortsFrom(oldChannel);
            delete oldChannel;
        }

        mConnectionStatus = eConnectionStatus::eEstablished;
        logInfo(RTPC_MSG, "Connection Stablished");
        return RETCODE_OK;
    }
    else if (mConnectionStatus == eConnectionStatus::eEstablished)
    {
        return RETCODE_EXISTING_CONNECTION;
    }

    return RETCODE_SERVER_ERROR;
}

void TCPChannelResource::ConnectionLost()
{
    if (mConnectionStatus != eConnectionStatus::eConnecting)
    {
        { // Mark all logical ports as pending
            std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
            mPendingLogicalOutputPorts.insert(mPendingLogicalOutputPorts.end(),
                mLogicalOutputPorts.begin(),
                mLogicalOutputPorts.end());
            mLogicalOutputPorts.clear();
        }
        Disconnect();
        if (mAlive && !m_inputSocket)
        {
            eClock::my_sleep(100);
            Connect();
        }
    }
}

void TCPChannelResource::CopyPendingPortsFrom(TCPChannelResource* from)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    std::unique_lock<std::recursive_mutex> fromLock(from->mPendingLogicalMutex);
    mPendingLogicalOutputPorts.insert(mPendingLogicalOutputPorts.end(),
        from->mPendingLogicalOutputPorts.begin(),
        from->mPendingLogicalOutputPorts.end());
}

void TCPChannelResource::Disconnect()
{
    if (ChangeStatus(eConnectionStatus::eDisconnected))
    {
        try
        {
            mSocket.cancel();
            mSocket.shutdown(asio::ip::tcp::socket::shutdown_both);
        }
        catch (std::exception&)
        {
            // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
        }
        mSocket.close();
    }
}

void TCPChannelResource::SocketConnected(const asio::error_code& error)
{
    // If we were disabled while trying to connect, this method will be
    // called with a 'canceled' error value. In that case we return directly
    if (!mAlive)
        return;

    if (error.value())
    {
        Disconnect();
        eClock::my_sleep(100);
        Connect();
    }
    else
    {
        if (mConnectionStatus == eConnectionStatus::eConnecting)
        {
            mConnectionStatus = eConnectionStatus::eConnected;
            mParent->SocketConnected(this);
        }
    }
}

bool TCPChannelResource::IsLogicalPortOpened(uint16_t port)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
	return std::find(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port) != mLogicalOutputPorts.end();
}

bool TCPChannelResource::IsLogicalPortAdded(uint16_t port)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
	return std::find(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port) != mLogicalOutputPorts.end()
           || std::find(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port)
                != mPendingLogicalOutputPorts.end();
}

std::thread* TCPChannelResource::ReleaseRTCPThread()
{
    std::thread* outThread = mRTCPThread;
    mRTCPThread = nullptr;
    return outThread;
}

void TCPChannelResource::fillLogicalPorts(std::vector<Locator_t>& outVector)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    Locator_t temp = mLocator;
    for (uint16_t port : mPendingLogicalOutputPorts)
    {
        IPLocator::setLogicalPort(temp, port);
        outVector.emplace_back(temp);
    }
    for (uint16_t port : mLogicalOutputPorts)
    {
        IPLocator::setLogicalPort(temp, port);
        outVector.emplace_back(temp);
    }
}

uint32_t TCPChannelResource::GetMsgSize() const
{
	return m_rec_msg.max_size;
}

void TCPChannelResource::AddLogicalPort(uint16_t port)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    // Already opened?
    if (std::find(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port) == mLogicalOutputPorts.end())
    {
        if (port == 0)
        {
            logError(RTPS, "Trying to open logical port 0.");
        } // But let's continue...

        if (mPendingLogicalOutputPorts.size() == 0) // Empty pending, let's try to open directly
        {
            TCPTransactionId id = mRTCPManager->sendOpenLogicalPortRequest(this, port);
            mNegotiatingLogicalPorts[id] = port;
            mPendingLogicalOutputPorts.emplace_back(port);
        }
        else if (std::find(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port)
                 == mPendingLogicalOutputPorts.end()) // Check isn't enqueued already
        {
            mPendingLogicalOutputPorts.emplace_back(port);
        }
    }



    /*
    { // Logical Ports
        if (pChannelResource->mPendingLogicalPort == 0 && !pChannelResource->mPendingLogicalOutputPorts.empty())
        {
            pChannelResource->mPendingLogicalPort = *pChannelResource->mPendingLogicalOutputPorts.begin();
            bSendOpenLogicalPort = true;
        }
        else if (pChannelResource->mPendingLogicalPort == 0)
        {
            if (GetConfiguration()->wait_for_tcp_negotiation)
            {
                pChannelResource->mNegotiationSemaphore.post();
            }
        }
        else if (std::chrono::system_clock::now() > negotiation_time)
        {
            pChannelResource->mNegotiationSemaphore.post();
        }
    }

    if (bSendOpenLogicalPort)
    {
        mRTCPMessageManager->sendOpenLogicalPortRequest(pChannelResource,
            pChannelResource->mPendingLogicalPort);
        negotiation_time = time_now + std::chrono::milliseconds(GetConfiguration()->tcp_negotiation_timeout);
        bSendOpenLogicalPort = false;
    }
    */
}

void TCPChannelResource::SendPendingOpenLogicalPorts()
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    if (!mPendingLogicalOutputPorts.empty())
    {
        for (uint16_t port : mPendingLogicalOutputPorts)
        {
            TCPTransactionId id = mRTCPManager->sendOpenLogicalPortRequest(this, port);
            mNegotiatingLogicalPorts[id] = port;
            eClock::my_sleep(100);
        }
    }
}

void TCPChannelResource::AddLogicalPortResponse(const TCPTransactionId &id, bool success, Locator_t &remote)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    auto it = mNegotiatingLogicalPorts.find(id);
    if (it != mNegotiatingLogicalPorts.end())
    {
        uint16_t port = it->second;
        auto portIt = std::find(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port);
        mNegotiatingLogicalPorts.erase(it);
        if (portIt != mPendingLogicalOutputPorts.end())
        {
            mPendingLogicalOutputPorts.erase(portIt);
            if (success)
            {
                mLogicalOutputPorts.push_back(port);
                IPLocator::setLogicalPort(remote, port);
                logInfo(RTCP, "OpenedLogicalPort " << port);
                mParent->BindSocket(remote, this);
            }
            else
            {
                PrepareAndSendCheckLogicalPortsRequest(port);
            }
        }
        else
        {
            logWarning(RTCP, "Received AddLogicalPortResponse for port " << port \
                << ", but it wasn't found in pending list.");
        }
    }
    else
    {
        logWarning(RTCP, "Received AddLogicalPortResponse, but the transaction id wasn't registered (maybe removed" <<\
            " while negotiating?).");
    }
}

void TCPChannelResource::PrepareAndSendCheckLogicalPortsRequest(uint16_t closedPort)
{
    std::vector<uint16_t> candidatePorts;
    uint16_t base_port = GetBaseAutoPort(closedPort); // The first failed port
    uint16_t max_port = closedPort + mParent->GetMaxLogicalPort();

    for (uint16_t p = base_port;
        p <= closedPort + (mParent->GetLogicalPortRange()
            * mParent->GetLogicalPortIncrement());
        p += mParent->GetLogicalPortIncrement())
    {
        // Don't add ports just tested and already pendings
        if (p <= max_port && p != closedPort)
        {
	        std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
            auto pendingIt = std::find(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), p);
            if (pendingIt == mPendingLogicalOutputPorts.end())
            {
                candidatePorts.emplace_back(p);
            }
        }
    }

    if (candidatePorts.empty()) // No more available ports!
    {
        logError(RTCP, "Cannot find an available logical port.");
    }
	else
	{
		TCPTransactionId id = mRTCPManager->sendCheckLogicalPortsRequest(this, candidatePorts);
        mLastCheckedLogicalPort[id] = candidatePorts.back();
	}
}

void TCPChannelResource::ProcessCheckLogicalPortsResponse(const TCPTransactionId &transactionId,
        const std::vector<uint16_t> &availablePorts)
{
    auto it = mLastCheckedLogicalPort.find(transactionId);
    if (it != mLastCheckedLogicalPort.end())
    {
        uint16_t lastPort = it->second;
        mLastCheckedLogicalPort.erase(it);
        if (availablePorts.empty())
        {
            PrepareAndSendCheckLogicalPortsRequest(lastPort);
        }
        else
        {
            AddLogicalPort(availablePorts.front());
        }
    }
    else
    {
        logWarning(RTCP, "Received ProcessCheckLogicalPortsResponse without sending a Request.");
    }
}

void TCPChannelResource::RemoveLogicalPort(uint16_t port)
{
	std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    std::remove(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port);
    std::remove(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
