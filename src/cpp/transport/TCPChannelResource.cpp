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
        asio::io_service& service, const Locator_t& locator, uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , mParent (parent)
    , mRTCPManager(rtcpManager)
    , mLocator(locator)
    , m_inputSocket(false)
    , mWaitingForKeepAlive(false)
    , mRTCPThread(nullptr)
    , mService(service)
    , mSocket(createTCPSocket(service))
    , mConnectionStatus(eConnectionStatus::eDisconnected)
{
}

TCPChannelResource::TCPChannelResource(TCPTransportInterface* parent, RTCPMessageManager* rtcpManager,
        asio::io_service& service, eProsimaTCPSocketRef socket, uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , mParent(parent)
    , mRTCPManager(rtcpManager)
    , mLocator()
    , m_inputSocket(true)
    , mWaitingForKeepAlive(false)
    , mRTCPThread(nullptr)
    , mService(service)
    , mSocket(moveSocket(socket))
    , mConnectionStatus(eConnectionStatus::eWaitingForBind)
{
}

TCPChannelResource::~TCPChannelResource()
{
    if (mRTCPThread != nullptr)
    {
        mRTCPThread->join();
        delete(mRTCPThread);
        mRTCPThread = nullptr;
    }
}

void TCPChannelResource::Disable()
{
    ChannelResource::Disable();

    Disconnect();
}

void TCPChannelResource::Connect()
{
    std::unique_lock<std::mutex> scoped(mStatusMutex);
    if (mConnectionStatus == eConnectionStatus::eDisconnected)
    {
        mConnectionStatus = eConnectionStatus::eConnecting;
        auto type = mParent->GetProtocolType();
        auto endpoint = mParent->GenerateLocalEndpoint(mLocator, IPLocator::getPhysicalPort(mLocator));
        try
        {
            mSocket.open(type);
            mSocket.async_connect(endpoint, std::bind(&TCPTransportInterface::SocketConnected, mParent,
                mLocator, std::placeholders::_1));
        }
        catch(const std::system_error &error)
        {
            logError(RTCP, "Openning socket " << error.what());
        }
    }
}

ResponseCode TCPChannelResource::ProcessBindRequest(const Locator_t& locator)
{
    std::unique_lock<std::mutex> scoped(mStatusMutex);
    if (mConnectionStatus == TCPChannelResource::eConnectionStatus::eWaitingForBind)
    {
        mLocator = locator;
        TCPChannelResource* oldChannel = mParent->BindSocket(mLocator, this);
        if (oldChannel != nullptr)
        {
            CopyPendingPortsFrom(oldChannel);
            mParent->DeleteSocket(oldChannel);
            //delete oldChannel;
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

void TCPChannelResource::InputPortClosed(uint16_t port)
{
    if (IsConnectionEstablished())
    {
        mRTCPManager->sendLogicalPortIsClosedRequest(this, port);
    }
}

void TCPChannelResource::SetAllPortsAsPending()
{
    std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    mPendingLogicalOutputPorts.insert(mPendingLogicalOutputPorts.end(),
        mLogicalOutputPorts.begin(),
        mLogicalOutputPorts.end());
    mLogicalOutputPorts.clear();
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
            asio::error_code ec;
            mSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            mSocket.cancel();

          // This method was added on the version 1.12.0
#if ASIO_VERSION >= 101200 && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0603)
            mSocket.release();
#endif
        }
        catch (std::exception&)
        {
            // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
        }
        mSocket.close();
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

bool TCPChannelResource::WaitUntilPortIsOpenOrConnectionIsClosed(uint16_t port)
{
    std::unique_lock<std::mutex> scoped(mStatusMutex);
    bool bConnected = mAlive && mConnectionStatus == eConnectionStatus::eEstablished;
    while (bConnected && !IsLogicalPortOpened(port))
    {
        mNegotiationCondition.wait(scoped);
        bConnected = mAlive && mConnectionStatus == eConnectionStatus::eEstablished;
    }
    return bConnected;
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

        if (std::find(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port)
                == mPendingLogicalOutputPorts.end()) // Check isn't enqueued already
        {
            mPendingLogicalOutputPorts.emplace_back(port);
            if (IsConnectionEstablished())
            {
                TCPTransactionId id = mRTCPManager->sendOpenLogicalPortRequest(this, port);
                mNegotiatingLogicalPorts[id] = port;
            }
        }
    }
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

void TCPChannelResource::AddLogicalPortResponse(const TCPTransactionId &id, bool success)
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
                mNegotiationCondition.notify_all();
                logInfo(RTCP, "OpenedLogicalPort " << port);
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

void TCPChannelResource::SetLogicalPortPending(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    auto it = std::find(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port);
    if (it != mLogicalOutputPorts.end())
    {
        mPendingLogicalOutputPorts.push_back(port);
        mLogicalOutputPorts.erase(it);
    }
}

bool TCPChannelResource::RemoveLogicalPort(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mPendingLogicalMutex);
    if (!IsLogicalPortAdded(port))
        return false;

    auto it = std::remove(mLogicalOutputPorts.begin(), mLogicalOutputPorts.end(), port);
    mLogicalOutputPorts.erase(it, mLogicalOutputPorts.end());
    it = std::remove(mPendingLogicalOutputPorts.begin(), mPendingLogicalOutputPorts.end(), port);
    mPendingLogicalOutputPorts.erase(it, mPendingLogicalOutputPorts.end());
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
