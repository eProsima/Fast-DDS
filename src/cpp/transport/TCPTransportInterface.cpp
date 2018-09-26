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

#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/transport/timedevent/CleanTCPSocketsEvent.h>
#include <utility>
#include <asio.hpp>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include "asio.hpp"
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/transport/TCPChannelResource.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static const int s_default_keep_alive_frequency = 50000; // 50 SECONDS
static const int s_default_keep_alive_timeout = 10000; // 10 SECONDS
static const int s_clean_deleted_sockets_pool_timeout = 100; // 100 MILLISECONDS
static const int s_default_tcp_negotitation_timeout = 5000; // 5 Seconds

TCPAcceptor::TCPAcceptor(asio::io_service& io_service, TCPTransportInterface* parent, const Locator_t& locator)
    : mAcceptor(io_service, parent->GenerateEndpoint(IPLocator::getPhysicalPort(locator)))
    , mLocator(locator)
    , mSocket(createTCPSocket(io_service))
{
    mEndPoint = asio::ip::tcp::endpoint(parent->GenerateProtocol(), IPLocator::getPhysicalPort(locator));
}

void TCPAcceptor::Accept(TCPTransportInterface* parent, asio::io_service& io_service)
{
    mSocket = createTCPSocket(io_service);

    // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    // std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    // std::cout << std::put_time(std::localtime(&now_c), "%F %T") << "--> Async_Accept" << std::endl;

	mAcceptor.async_accept(getTCPSocketRef(mSocket), mEndPoint, std::bind(&TCPTransportInterface::SocketAccepted,
        parent, this, std::placeholders::_1));
}

TCPConnector::TCPConnector(asio::io_service& io_service, asio::ip::tcp type, asio::ip::tcp::endpoint endpoint)
    : m_socket(createTCPSocket(io_service))
    , m_service(io_service)
    , m_type(type)
    , m_endpoint(endpoint)
{
}

TCPConnector::~TCPConnector()
{
    getSocketPtr(m_socket)->close();
}

void TCPConnector::Connect(TCPChannelResource* channel)
{
    getSocketPtr(m_socket)->open(m_type);
    getSocketPtr(m_socket)->async_connect(m_endpoint, std::bind(&TCPConnector::SocketConnected, this,
        channel, std::placeholders::_1));
}

void TCPConnector::RetryConnect(TCPChannelResource* channel)
{
    getSocketPtr(m_socket)->close();
    Connect(channel);
}

void TCPConnector::SocketConnected(TCPChannelResource* channel, const asio::error_code& error)
{
    if (error.value())
    {
        eClock::my_sleep(100);
        RetryConnect(channel);
    }
    else
    {
        channel->Connected();
    }
}

TCPTransportDescriptor::TCPTransportDescriptor()
    : SocketTransportDescriptor(s_maximumMessageSize)
    , keep_alive_frequency_ms(s_default_keep_alive_frequency)
    , keep_alive_timeout_ms(s_default_keep_alive_timeout)
    , max_logical_port(100)
    , logical_port_range(20)
    , logical_port_increment(2)
	, tcp_negotiation_timeout(s_default_tcp_negotitation_timeout)
	, wait_for_tcp_negotiation(true)
{
}

TCPTransportDescriptor::TCPTransportDescriptor(const TCPTransportDescriptor& t)
    : SocketTransportDescriptor(t)
    , listening_ports(t.listening_ports)
    , keep_alive_frequency_ms(t.keep_alive_frequency_ms)
    , keep_alive_timeout_ms(t.keep_alive_timeout_ms)
    , max_logical_port(t.max_logical_port)
    , logical_port_range(t.logical_port_range)
    , logical_port_increment(t.logical_port_increment)
	, tcp_negotiation_timeout(t.tcp_negotiation_timeout)
	, wait_for_tcp_negotiation(t.wait_for_tcp_negotiation)
{
}

TCPTransportInterface::TCPTransportInterface()
    : mRTCPMessageManager(nullptr)
    , mCleanSocketsPoolTimer(nullptr)
{
}

TCPTransportInterface::~TCPTransportInterface()
{
}

void TCPTransportInterface::Clean()
{
    std::vector<TCPChannelResource*> vDeletedSockets;

    // Collect all the existing sockets to delete them outside of the mutex.
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        for (auto it = mSocketAcceptors.begin(); it != mSocketAcceptors.end(); ++it)
        {
            delete it->second;
        }
        mSocketAcceptors.clear();

        for (auto it = mChannelResources.begin(); it != mChannelResources.end(); ++it)
        {
            vDeletedSockets.push_back(it->second);
        }
        mChannelResources.clear();
    }

    for (auto it = vDeletedSockets.begin(); it != vDeletedSockets.end(); ++it)
    {
        ReleaseTCPSocket(*it, false);
        (*it) = nullptr;
    }

    if (mCleanSocketsPoolTimer != nullptr)
    {
        mCleanSocketsPoolTimer->cancel_timer();
        delete mCleanSocketsPoolTimer;
        mCleanSocketsPoolTimer = nullptr;
    }

    CleanDeletedSockets();

    if (ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }

    delete mRTCPMessageManager;
}

void TCPTransportInterface::BindOutputChannel(const Locator_t& locator, SenderResource* /* senderResource*/)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
    {
        return;
    }

    auto it = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (it != mChannelResources.end())
    {
        BindSocket(locator, itc->second);
    }
}

void TCPTransportInterface::BindSocket(const Locator_t& locator, TCPChannelResource *pChannelResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (IsLocatorSupported(locator))
    {
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto it = mChannelResources.find(physicalLocator);
        if (it == mChannelResources.end())
        {
            mChannelResources[physicalLocator] = pChannelResource; // First element, just add it
        }
        /*
        else
        {
            // There are more, check isn't already added
            auto found = std::find(mBoundOutputSockets[locator].begin(), mBoundOutputSockets[locator].end(), pChannelResource);
            if (found == mBoundOutputSockets[locator].end())
            {
                mBoundOutputSockets[locator].push_back(pChannelResource);
            }
        }
        */
    }
}

void TCPTransportInterface::CleanDeletedSockets()
{
    std::unique_lock<std::recursive_mutex> scopedLock(mDeletedSocketsPoolMutex);
    for (auto it = mDeletedSocketsPool.begin(); it != mDeletedSocketsPool.end(); ++it)
    {
        std::thread* rtcpThread = (*it)->ReleaseRTCPThread();
        std::thread* thread = (*it)->ReleaseThread();
        if (rtcpThread != nullptr)
        {
            rtcpThread->join();
            delete(rtcpThread);
        }
        if (thread != nullptr)
        {
            thread->join();
            delete(thread);
        }

        delete(*it);
    }
    mDeletedSocketsPool.clear();
}

bool TCPTransportInterface::CheckCRC(const TCPHeader &header, const octet *data, uint32_t size)
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    return crc == header.crc;
}

void TCPTransportInterface::CalculateCRC(TCPHeader &header, const octet *data, uint32_t size)
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    header.crc = crc;
}


bool TCPTransportInterface::CreateAcceptorSocket(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    try
    {
        TCPAcceptor* newAcceptor = new TCPAcceptor(mService, this, locator);
        mSocketAcceptors.insert(std::make_pair(IPLocator::getPhysicalPort(locator), newAcceptor));
        logInfo(RTCP, " OpenAndBindInput (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: " << IPLocator::getLogicalPort(locator) << ")");
        newAcceptor->Accept(this, mService);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")" << " with msg: " << e.what());
        return false;
    }

    return true;
}

/*
void TCPTransportInterface::CreateConnectorSocket(const Locator_t& locator, SenderResource *senderResource,
    std::vector<Locator_t>& pendingLocators, uint32_t msgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (IsInterfaceAllowed(locator))
    {
        TCPConnector* newConnector = new TCPConnector(mService, locator, msgSize);
        newConnector->m_PendingLocators = pendingLocators;
        const Locator_t &physicalLocator = IPLocator::toPhysicalLocator(locator);
        if (mSocketConnectors.find(physicalLocator) != mSocketConnectors.end())
        {
            TCPConnector* oldConnector = mSocketConnectors.at(physicalLocator);
            delete oldConnector;
        }
        mSocketConnectors[physicalLocator] = newConnector;
        logInfo(RTCP, " OpenAndBindOutput (physical: " << IPLocator::getPhysicalPort(locator) <<
            "; logical: " << IPLocator::getLogicalPort(locator) << ")");
        newConnector->Connect(this, senderResource);
    }
}
*/

bool TCPTransportInterface::EnqueueLogicalOutputPort(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    auto socketIt = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != mChannelResources.end())
    {
        socketIt->second->EnqueueLogicalPort(IPLocator::getLogicalPort(locator));
        return true;
    }
    return false;
}

void TCPTransportInterface::FillTCPHeader(TCPHeader& header, const octet* sendBuffer, uint32_t sendBufferSize,
    uint16_t logicalPort)
{
    header.length = sendBufferSize + static_cast<uint32_t>(TCPHeader::getSize());
    header.logicalPort = logicalPort;
    CalculateCRC(header, sendBuffer, sendBufferSize);
}


bool TCPTransportInterface::IsOutputChannelBound(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    auto socket = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (socket != mChannelResources.end())
    {
        return true;
    }

    return false;
}

bool TCPTransportInterface::IsOutputChannelConnected(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    auto socketIt = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != mChannelResources.end())
    {
        return true;
    }

    return IsOutputChannelBound(locator);
}

bool TCPTransportInterface::IsTCPInputSocket(const Locator_t& locator) const
{
    if (is_local_locator(locator))
    {
        for (auto it = GetConfiguration()->listening_ports.begin(); it != GetConfiguration()->listening_ports.end(); ++it)
        {
            if (IPLocator::getPhysicalPort(locator) == *it)
            {
                return true;
            }
        }
    }
    return false;
}

bool TCPTransportInterface::DoInputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return IPLocator::getPhysicalPort(left) ==  IPLocator::getPhysicalPort(right);
}

bool TCPTransportInterface::DoOutputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return CompareLocatorIPAndPort(left, right);
}

bool TCPTransportInterface::init()
{
    if (GetConfiguration()->sendBufferSize == 0 || GetConfiguration()->receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::tcp::socket socket(mService);
        socket.open(GenerateProtocol());

        if (GetConfiguration()->sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            SetSendBufferSize(option.value());

            if (GetConfiguration()->sendBufferSize < s_minimumSocketBuffer)
            {
                SetSendBufferSize(s_minimumSocketBuffer);
            }
        }

        if (GetConfiguration()->receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            SetReceiveBufferSize(option.value());

            if (GetConfiguration()->receiveBufferSize < s_minimumSocketBuffer)
            {
                SetReceiveBufferSize(s_minimumSocketBuffer);
            }
        }

        socket.close();
    }

    if (GetConfiguration()->maxMessageSize > s_maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (GetConfiguration()->maxMessageSize > GetConfiguration()->sendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if (GetConfiguration()->maxMessageSize > GetConfiguration()->receiveBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than receiveBufferSize");
        return false;
    }

    if (mRTCPMessageManager == nullptr)
    {
        mRTCPMessageManager = new RTCPMessageManager(this);
    }

    // TODO(Ricardo) Create an event that update this list.
    GetIPs(mCurrentInterfaces);

    auto ioServiceFunction = [&]()
    {
        io_service::work work(mService);
        mService.run();
    };
    ioServiceThread.reset(new std::thread(ioServiceFunction));

    mCleanSocketsPoolTimer = new CleanTCPSocketsEvent(this, mService, *ioServiceThread.get(),
        s_clean_deleted_sockets_pool_timeout);

    return true;
}

bool TCPTransportInterface::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    return IsLocatorSupported(locator)
           && mReceiverResources.find(IPLocator::getLogicalPort(locator)) != mReceiverResources.end();
}

bool TCPTransportInterface::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == mTransportKind;
}

bool TCPTransportInterface::IsOutputChannelOpen(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);

    // Check if there is any socket opened with the given locator.
    auto socketIt = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != mChannelResources.end())
    {
        // And it is registered as output logical port
        return socketIt->second->IsLogicalPortAdded(IPLocator::getLogicalPort(locator));
    }

    return false;
}

Locator_t TCPTransportInterface::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    Locator_t mainLocal(remote);
    mainLocal.set_Invalid_Address();
    return mainLocal;
}


bool TCPTransportInterface::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    auto socketIt = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != mChannelResources.end())
    {
        // TODO: socketIt->RemoveLogicalPort(locator.logical_port)
        return true;
    }
    return false;
}

void TCPTransportInterface::CloseInputSocket(TCPChannelResource *pChannelResource)
{
    if (pChannelResource->IsAlive())
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eDisconnected);
        auto inputSocketIt = mChannelResources.find(IPLocator::toPhysicalLocator(pChannelResource->GetLocator()));
        if (inputSocketIt != mChannelResources.end())
        {
            mChannelResources.erase(inputSocketIt);
        }
        ReleaseTCPSocket(pChannelResource, false);
    }
}

bool TCPTransportInterface::CloseInputChannel(const Locator_t& locator)
{
    bool bClosed = false;
    std::vector<TCPChannelResource*> vDeletedSockets;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);

        if (mReceiverResources.find(IPLocator::getLogicalPort(locator)) != mReceiverResources.end())
        {
            mReceiverResources.erase(IPLocator::getLogicalPort(locator));
            bClosed = true;
        }

        auto acceptorIt = mSocketAcceptors.find(IPLocator::getPhysicalPort(locator));
        if (acceptorIt != mSocketAcceptors.end())
        {
            delete acceptorIt->second;
            mSocketAcceptors.erase(acceptorIt);
            bClosed = true;
        }

        auto InputSocketIt = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
        if (InputSocketIt != mChannelResources.end())
        {
            mChannelResources.erase(InputSocketIt);
            vDeletedSockets.emplace_back(InputSocketIt->second);
            bClosed = true;
        }
    }

    for (auto it = vDeletedSockets.begin(); it != vDeletedSockets.end(); ++it)
    {
        ReleaseTCPSocket(*it, false);
        *it = nullptr;
    }

    return bClosed;
}

void TCPTransportInterface::CloseTCPSocket(TCPChannelResource *pChannelResource)
{
    if (pChannelResource->IsAlive())
    {
        if (pChannelResource->GetIsInputSocket())
        {
            CloseInputSocket(pChannelResource);
        }
        else
        {
            Locator_t prevLocator = pChannelResource->GetLocator();
            std::vector<Locator_t> logicalLocators;
            pChannelResource->fillLogicalPorts(logicalLocators);
            uint32_t msgSize = pChannelResource->GetMsgSize();

            CloseOutputChannel(prevLocator);
        }
    }
}


bool TCPTransportInterface::OpenOutputChannel(const Locator_t& locator, SenderResource* senderResource,
        uint32_t msgSize)
{
    bool success = false;
    auto logicalPort = IPLocator::getLogicalPort(locator);
    if (IsLocatorSupported(locator) && (logicalPort != 0))
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        logInfo(RTCP, "OpenOutputChannel (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: " \
            << IPLocator::getLogicalPort(locator) << ") @ IP: " << IPLocator::toIPv4string(locator));

        auto physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto socketIt = mChannelResources.find(physicalLocator);
        TCPChannelResource* channel = nullptr;
        if (socketIt != mChannelResources.end())
        {
            channel = socketIt->second;
        }
        else
        {
            // Create connector
            auto connector = new TCPConnector(mService, GetProtocolType(),
                GenerateLocalEndpoint(physicalLocator, IPLocator::getPhysicalPort(physicalLocator)));
            channel = new TCPChannelResource(this, connector, physicalLocator);
            mChannelResources[physicalLocator] = channel;
            /*
            // There's an already opened connection?
            success = mChannelResources.find(IPLocator::toPhysicalLocator(locator)) != mChannelResources.end();
            if (!success)
            {
                // Maybe, there's a waiting acceptor?
                auto it = mSocketAcceptors.find(IPLocator::getPhysicalPort(locator));
                if (it != mSocketAcceptors.end())
                {
                    it->second->mPendingOutLocators.push_back(locator);
                    BindOutputChannel(locator, senderResource);
                    success = EnqueueLogicalOutputPort(locator);
                }
            }
            */
        }
        }

        // TODO: success = channel->AddLogicalPort(logicalPort);
        success = true;
        channel->EnqueueLogicalPort(logicalPort);
    }

    return success;
}

bool TCPTransportInterface::OpenExtraOutputChannel(const Locator_t& locator, SenderResource* senderResource,
        uint32_t msgSize)
{
    return OpenOutputChannel(locator, senderResource, msgSize);
}

bool TCPTransportInterface::OpenInputChannel(const Locator_t& locator, TransportReceiverInterface* receiver,
    uint32_t maxMsgSize)
{
    bool success = false;
    if (IsLocatorSupported(locator))
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        auto logicalPort = IPLocator::getLogicalPort(locator);
        if (mReceiverResources.find(logicalPort) == mReceiverResources.end())
        {
            success = true;
            mReceiverResources[logicalPort] = receiver;

            logInfo(RTCP, " OpenInputChannel (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: " << \
                IPLocator::getLogicalPort(locator) << ")");
        }
    }
    return success;
}

void TCPTransportInterface::performRTPCManagementThread(TCPChannelResource *pChannelResource)
{
    std::chrono::time_point<std::chrono::system_clock> time_now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> next_time = time_now +
        std::chrono::milliseconds(GetConfiguration()->keep_alive_frequency_ms);
    std::chrono::time_point<std::chrono::system_clock> timeout_time =
        time_now + std::chrono::milliseconds(GetConfiguration()->keep_alive_timeout_ms);
    std::chrono::time_point<std::chrono::system_clock> negotiation_time = time_now;

    bool bSendOpenLogicalPort = false;
    /*
    logInfo(RTCP, "START performRTPCManagementThread " << IPLocator::toIPv4string(pChannelResource->GetLocator()) \
        << ":" << IPLocator::getPhysicalPort(pChannelResource->GetLocator()) << " (" \
        << pChannelResource->getSocket()->local_endpoint().address() << ":" \
        << pChannelResource->getSocket()->local_endpoint().port() << "->" \
        << pChannelResource->getSocket()->remote_endpoint().address() << ":" \
        << pChannelResource->getSocket()->remote_endpoint().port() << ")");
        */
    while (pChannelResource->IsAlive())
    {
        if (pChannelResource->IsConnectionEstablished())
        {
            { // Logical Ports
                std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->mPendingLogicalMutex);
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
                mRTCPMessageManager->sendOpenLogicalPortRequest(pChannelResource, pChannelResource->mPendingLogicalPort);
                negotiation_time = time_now + std::chrono::milliseconds(GetConfiguration()->tcp_negotiation_timeout);
                bSendOpenLogicalPort = false;
            }

            // KeepAlive
            if (GetConfiguration()->keep_alive_frequency_ms > 0 && GetConfiguration()->keep_alive_timeout_ms > 0)
            {
                time_now = std::chrono::system_clock::now();

                // Keep Alive Management
                if (!pChannelResource->mWaitingForKeepAlive && time_now > next_time)
                {
                    mRTCPMessageManager->sendKeepAliveRequest(pChannelResource);
                    pChannelResource->mWaitingForKeepAlive = true;
                    next_time = time_now + std::chrono::milliseconds(GetConfiguration()->keep_alive_frequency_ms);
                    timeout_time = time_now + std::chrono::milliseconds(GetConfiguration()->keep_alive_timeout_ms);
                }
                else if (pChannelResource->mWaitingForKeepAlive && time_now >= timeout_time)
                {
                    // Disable the socket to erase it after the reception.
                    mRTCPMessageManager->sendUnbindConnectionRequest(pChannelResource);
                    ReleaseTCPSocket(pChannelResource, true);
                    continue;
                }
            }
        }
        eClock::my_sleep(100);
    }
    logInfo(RTCP, "End performRTPCManagementThread " << pChannelResource->GetLocator());
}

void TCPTransportInterface::performListenOperation(TCPChannelResource *pChannelResource)
{
    Locator_t remoteLocator;
    uint16_t logicalPort(0);
    logInfo(RTCP, "START PerformListenOperation " << pChannelResource->GetLocator() << " (" << \
        pChannelResource->getSocket()->local_endpoint().address() << "->" << \
        pChannelResource->getSocket()->remote_endpoint().address() << ")");
    while (pChannelResource->IsAlive())
    {
        // Blocking receive.
        auto msg = pChannelResource->GetMessageBuffer();
        CDRMessage::initCDRMsg(&msg);
        if (!Receive(pChannelResource, msg.buffer, msg.max_size, msg.length, remoteLocator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        logicalPort = IPLocator::getLogicalPort(remoteLocator);
        auto it = mReceiverResources.find(logicalPort);
        //TransportReceiverInterface* receiver = pChannelResource->GetMessageReceiver(logicalPort);
        if (it != mReceiverResources.end())
        {
            it->second->OnDataReceived(msg.buffer, msg.length, pChannelResource->GetLocator(), remoteLocator);
        }
        else
        {
            logWarning(RTCP, "Received Message, but no TransportReceiverInterface attached: " << logicalPort);
        }
    }

    logInfo(RTCP, "End PerformListenOperation " << pChannelResource->GetLocator());
}

bool TCPTransportInterface::ReadBody(octet* receiveBuffer, uint32_t receiveBufferCapacity,
    uint32_t* bytes_received, TCPChannelResource *pChannelResource, std::size_t body_size)
{
    *bytes_received = static_cast<uint32_t>(read(*pChannelResource->getSocket(),
        asio::buffer(receiveBuffer, receiveBufferCapacity), transfer_exactly(body_size)));

    if (*bytes_received != body_size)
    {
        logError(RTCP, "Bad TCP body size: " << bytes_received << "(expected: " << TCPHeader::getSize() << ")");
        return false;
    }

    return true;
}

/**
* On TCP, we must receive the header (14 Bytes) and then,
* the rest of the message, whose length is on the header.
* TCP Header is transparent to the caller, so receiveBuffer
* doesn't include it.
* */
bool TCPTransportInterface::Receive(TCPChannelResource *pChannelResource, octet* receiveBuffer,
    uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize, Locator_t& remoteLocator)
{
    bool success = false;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->GetReadMutex());
        if (!pChannelResource->IsAlive())
        {
            success = false;
        }
        else
        {
            success = true;
            try
            {
                // Read the header
                //octet header[TCPHEADER_SIZE];
                TCPHeader tcp_header;
                size_t bytes_received = read(*pChannelResource->getSocket(),
                    asio::buffer(&tcp_header, TCPHeader::getSize()),
                    transfer_exactly(TCPHeader::getSize()));

                logInfo(RTPS_MSG_IN, "[RECEIVE] From: " << pChannelResource->getSocket()->remote_endpoint().address() \
                    << " to " << pChannelResource->getSocket()->local_endpoint().address());

                EndpointToLocator(pChannelResource->getSocket()->remote_endpoint(), remoteLocator);

                if (bytes_received != TCPHeader::getSize())
                {
                    logError(RTPS_MSG_IN, "Bad TCP header size: " << bytes_received << "(expected: : " << TCPHeader::getSize() << ")");
                    success = false;
                }
                else
                {
                    //TCPHeader tcp_header;
                    //memcpy(&tcp_header, header, TCPHeader::getSize());
                    size_t body_size = tcp_header.length - static_cast<uint32_t>(TCPHeader::getSize());
                    //logInfo(RTPS_MSG_IN, " Received [TCPHeader] (CRC=" << tcp_header.crc << ")");

                    if (body_size > receiveBufferCapacity)
                    {
                        logError(RTPS_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                            << static_cast<uint32_t>(body_size) << " vs. " << receiveBufferCapacity << ".");
                        success = false;
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN, "Received RTCP MSG. Logical Port " << tcp_header.logicalPort);
                        success = ReadBody(receiveBuffer, receiveBufferCapacity, &receiveBufferSize, pChannelResource, body_size);
                        //logInfo(RTPS_MSG_IN, " Received [ReadBody]");

                        if (!CheckCRC(tcp_header, receiveBuffer, receiveBufferSize))
                        {
                            logWarning(RTPS_MSG_IN, "Bad TCP header CRC");
                        }

                        if (tcp_header.logicalPort == 0)
                        {
                            //logInfo(RTPS_MSG_IN, " Receive [RTCP Control]  (" << receiveBufferSize+bytes_received
                            // << " bytes): " << receiveBufferSize << " bytes.");
                            if (!mRTCPMessageManager->processRTCPMessage(pChannelResource, receiveBuffer, body_size))
                            {
                                CloseTCPSocket(pChannelResource);
                            }
                            success = false;
                        }
                        else
                        {
                            IPLocator::setLogicalPort(remoteLocator, tcp_header.logicalPort);
                            //logInfo(RTPS_MSG_IN, " Receive [RTPS Data]: " << receiveBufferSize << " bytes.");
                        }
                    }
                }
            }
            catch (const asio::error_code& code)
            {
                if ((code == asio::error::eof) || (code == asio::error::connection_reset))
                {
                    // Close the channel
                    logInfo(RTPS_MSG_IN, "ASIO [RECEIVE]: " << code.message());
                    CloseTCPSocket(pChannelResource);
                }
                success = false;
            }
            catch (const asio::system_error& error)
            {
                (void)error;
                // Close the channel
                logInfo(RTPS_MSG_IN, "ASIO [RECEIVE]: " << error.what());
                CloseTCPSocket(pChannelResource);
                success = false;
            }
        }
    }
    success = success && receiveBufferSize > 0;

    if (!success)
    {
        // TODO Manage errors, but don't remove the Channel Resource wihtout checks
        /*
        auto it = mChannelResources.find(IPLocator::toPhysicalLocator(pChannelResource->GetLocator()));
        if (it != mChannelResources.end())
        {
            mChannelResources.erase(it);
        }
        */
    }
    return success;
}

void TCPTransportInterface::ReleaseTCPSocket(TCPChannelResource *pChannelResource, bool /*force*/)
{
    if (pChannelResource != nullptr && pChannelResource->IsAlive())
    {
        //if (pChannelResource->HasLogicalConnections())
        //{
        //    pChannelResource->RemoveLogicalConnection();
        //}

        //if (force || !pChannelResource->HasLogicalConnections())
        {
            // Pauses the timer to clean the deleted sockets pool.
            if (mCleanSocketsPoolTimer != nullptr)
            {
                mCleanSocketsPoolTimer->cancel_timer();
            }

            // Remove the all bindings with this socket.
            UnbindSocket(pChannelResource);

            pChannelResource->Disable();
            pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eDisconnected);
            try
            {
                pChannelResource->getSocket()->cancel();
                pChannelResource->getSocket()->shutdown(ip::tcp::socket::shutdown_both);
            }
            catch (std::exception&)
            {
                // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
            }
            pChannelResource->getSocket()->close();

            {
                std::unique_lock<std::recursive_mutex> scopedLock(mDeletedSocketsPoolMutex);
                mDeletedSocketsPool.emplace_back(pChannelResource);
            }
            pChannelResource = nullptr;

            // Starts a timer to clean the deleted sockets pool.
            if (mCleanSocketsPoolTimer != nullptr)
            {
                mCleanSocketsPoolTimer->update_interval_millisec(s_clean_deleted_sockets_pool_timeout);
                mCleanSocketsPoolTimer->restart_timer();
            }
        }
    }
}

size_t TCPTransportInterface::Send(TCPChannelResource *pChannelResource, const octet *data,
    size_t size, eSocketErrorCodes &errorCode) const
{
    size_t bytesSent = 0;
    try
    {
        std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->GetWriteMutex());
        bytesSent = pChannelResource->getSocket()->send(asio::buffer(data, size));
        logInfo(RTCP, "[SENT] From: " << pChannelResource->getSocket()->local_endpoint().address()
            << " to " << pChannelResource->getSocket()->remote_endpoint().address());
        errorCode = eSocketErrorCodes::eNoError;
    }
    catch (const asio::error_code& error)
    {
        logInfo(RTCP, "ASIO [SEND]: " << error.message());
        if ((asio::error::eof == error.value()) || (asio::error::connection_reset == error.value()))
        {
            errorCode = eSocketErrorCodes::eBrokenPipe;
        }
        else
        {
            errorCode = eSocketErrorCodes::eAsioError;
        }
    }
    catch (const asio::system_error& error)
    {
        (void)error;
        logInfo(RTCP, "ASIO [SEND]: " << error.what());
        errorCode = eSocketErrorCodes::eSystemError;
    }
    catch (const std::exception& error)
    {
        (void)error;
        logInfo(RTCP, "ASIO [SEND]: " << error.what());
        errorCode = eSocketErrorCodes::eException;
    }

    return bytesSent;
}

size_t TCPTransportInterface::Send(TCPChannelResource *pChannelResource, const octet *data, size_t size) const
{
    eSocketErrorCodes error;
    return Send(pChannelResource, data, size, error);
}


bool TCPTransportInterface::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
    const Locator_t& remoteLocator)
{
    //    logInfo(RTCP, " SEND [RTPS Data] to locator " << IPLocator::getPhysicalPort(remoteLocator) << ":" << \
            IPLocator::getLogicalPort(remoteLocator));

    TCPChannelResource* channelResource = nullptr;
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsOutputChannelConnected(remoteLocator) || sendBufferSize > GetConfiguration()->sendBufferSize)
    {
        logWarning(RTCP, "SEND [RTPS] Failed: Not connect: " << IPLocator::getLogicalPort(remoteLocator) \
            << " @ IP: " << IPLocator::toIPv4string(remoteLocator));
        return false;
    }

    auto it = mChannelResources.find(IPLocator::toPhysicalLocator(remoteLocator));
    if (it == mChannelResources.end())
    {
        EnqueueLogicalOutputPort(remoteLocator);
        logInfo(RTCP, "SEND [RTPS] Failed: Not yet bound: " << IPLocator::getLogicalPort(remoteLocator) \
            << " @ IP: " << IPLocator::toIPv4string(remoteLocator) << " will be bound.");
        return false;
    }
    else
    {
        channelResource = it->second;
    }

    bool result = true;
    if (channelResource != nullptr)
    {
        result = result && Send(sendBuffer, sendBufferSize, localLocator, remoteLocator, channelResource);
    }
    return result;
}

bool TCPTransportInterface::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& /*localLocator*/,
    const Locator_t& remoteLocator, ChannelResource *pChannelResource)
{
    TCPChannelResource* tcpChannelResource = dynamic_cast<TCPChannelResource*>(pChannelResource);
    if (tcpChannelResource != nullptr && tcpChannelResource->IsConnectionEstablished())
    {
        bool success = false;
        uint16_t logicalPort = tcpChannelResource->mLogicalPortRouting.find(IPLocator::getLogicalPort(remoteLocator))
            != tcpChannelResource->mLogicalPortRouting.end()
            ? tcpChannelResource->mLogicalPortRouting.at(IPLocator::getLogicalPort(remoteLocator))
            : IPLocator::getLogicalPort(remoteLocator);

        bool bSendMsg = true;
        if (!tcpChannelResource->IsLogicalPortOpened(logicalPort))
        {
            tcpChannelResource->EnqueueLogicalPort(logicalPort);
            if (GetConfiguration()->wait_for_tcp_negotiation)
            {
                tcpChannelResource->mNegotiationSemaphore.wait();
            }
            else
            {
                bSendMsg = false;
            }
        }

        if (bSendMsg && tcpChannelResource->IsAlive())
        {
            //CDRMessage_t msg(static_cast<uint32_t>(sendBufferSize + TCPHeader::getSize()));
            TCPHeader tcp_header;
            FillTCPHeader(tcp_header, sendBuffer, sendBufferSize, logicalPort);

            //RTPSMessageCreator::addCustomContent(&msg, (octet*)&tcp_header, TCPHeader::getSize());
            //RTPSMessageCreator::addCustomContent(&msg, sendBuffer, sendBufferSize);

            {
                std::unique_lock<std::recursive_mutex> sendLock(tcpChannelResource->GetWriteMutex());
                success = SendThroughSocket((octet*)&tcp_header, static_cast<uint32_t>(TCPHeader::getSize()), remoteLocator, tcpChannelResource);

                if (success)
                {
                    success = SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, tcpChannelResource);
                }
            }
        }
        return success;
    }
    else
    {
        logWarning(RTCP, " SEND [RTPS] Failed: Connection not established" << IPLocator::getLogicalPort(remoteLocator));
        eClock::my_sleep(100);
        return false;
    }
}

bool TCPTransportInterface::SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize,
    const Locator_t& remoteLocator, TCPChannelResource *socket)
{
    auto destinationEndpoint = GenerateEndpoint(remoteLocator, IPLocator::getPhysicalPort(remoteLocator));

    size_t bytesSent = 0;
    (void)destinationEndpoint;
    logInfo(RTPS_MSG_OUT, "TCP: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint << " FROM " << socket->getSocket()->local_endpoint());

    //logInfo(RTCP, "SOCKET SEND to physical port " << socket->getSocket()->remote_endpoint().port());

    eSocketErrorCodes errorCode;
    bytesSent = Send(socket, sendBuffer, sendBufferSize, errorCode);
    switch (errorCode)
    {
    case eNoError:
        //logInfo(RTCP, " Sent [OK]: " << sendBufferSize << " bytes to locator " << IPLocator::getLogicalPort(remoteLocator));
        break;
    default:
        // Close the channel
        logInfo(RTCP, " Sent [FAILED]: " << sendBufferSize << " bytes to locator " << IPLocator::getLogicalPort(remoteLocator) << " ERROR=" << errorCode);
        Locator_t prevLocator = socket->GetLocator();
        uint32_t msgSize = socket->GetMsgSize();

        std::vector<Locator_t> logicalLocators;
        socket->fillLogicalPorts(logicalLocators);
        CloseOutputChannel(socket->GetLocator());

        // Create a new connector to retry the connection.
        // CreateConnectorSocket(prevLocator, nullptr, logicalLocators, msgSize);
        break;
    }

    logInfo(RTPS_MSG_OUT, "SENT " << bytesSent);
    return bytesSent > 0;
}

LocatorList_t TCPTransportInterface::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t unicastResult;
    for (auto& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        LocatorList_t pendingUnicast;

        while (it != locatorList.end())
        {
            assert((*it).kind == mTransportKind);

            // Check is local interface.
            auto localInterface = mCurrentInterfaces.begin();
            for (; localInterface != mCurrentInterfaces.end(); ++localInterface)
            {
                if (CompareLocatorIP(localInterface->locator, *it))
                {
                    // Loopback locator
                    Locator_t loopbackLocator;
                    FillLocalIp(loopbackLocator);
                    IPLocator::setPhysicalPort(loopbackLocator, IPLocator::getPhysicalPort(*it));
                    pendingUnicast.push_back(loopbackLocator);
                    break;
                }
            }

            if (localInterface == mCurrentInterfaces.end())
                pendingUnicast.push_back(*it);

            ++it;
        }

        unicastResult.push_back(pendingUnicast);
    }

    LocatorList_t result(std::move(unicastResult));
    return result;
}

void TCPTransportInterface::SocketAccepted(TCPAcceptor* acceptor, const asio::error_code& error)
{
    if (!error.value())
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (mSocketAcceptors.find(IPLocator::getPhysicalPort(acceptor->mLocator)) != mSocketAcceptors.end())
        {
#if defined(ASIO_HAS_MOVE)
            eProsimaTCPSocket unicastSocket = eProsimaTCPSocket(std::move(acceptor->mSocket));
#else
            eProsimaTCPSocket unicastSocket = eProsimaTCPSocket(acceptor->mSocket);
            acceptor->mSocket = nullptr;
#endif
            // Store the new connection.
            TCPChannelResource *pChannelResource = new TCPChannelResource(this, unicastSocket);
            pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eWaitingForBind);

            //BindSocket(acceptor->mLocator, pChannelResource);
            Locator_t remoteLocator;
            EndpointToLocator(pChannelResource->getSocket()->remote_endpoint(), remoteLocator);
            BindSocket(remoteLocator, pChannelResource);
            IPLocator::setPhysicalPort(remoteLocator, IPLocator::getPhysicalPort(acceptor->mLocator));
            BindSocket(remoteLocator, pChannelResource);

            for (const Locator_t& loc : acceptor->mPendingOutLocators)
            {
                BindSocket(loc, pChannelResource);
            }

            pChannelResource->SetThread(new std::thread(&TCPTransportInterface::performListenOperation, this,
                pChannelResource));
            pChannelResource->SetRTCPThread(new std::thread(&TCPTransportInterface::performRTPCManagementThread,
                this, pChannelResource));

            logInfo(RTCP, " Accepted connection (physical local: " << IPLocator::getPhysicalPort(acceptor->mLocator)
                << ", remote: " << pChannelResource->getSocket()->remote_endpoint().port()
                << ") IP: " << pChannelResource->getSocket()->remote_endpoint().address());


            // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            // std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            // std::cout << std::put_time(std::localtime(&now_c), "%F %T")
            //     <<  "--> Accepted connection (physical local: " << acceptor->mLocator.get_physical_port()
            //     << ", remote: " << pChannelResource->getSocket()->remote_endpoint().port()
            //     << ") IP: " << pChannelResource->getSocket()->remote_endpoint().address() << std::endl;
        }
        else
        {
            logError(RTPC, "Incomming connection from unknown Acceptor: " << IPLocator::getPhysicalPort(acceptor->mLocator));
            return;
        }
    }
    else
    {
        logInfo(RTCP, " Accepting connection failed (error: " << error.message() << ")");
    }

    if (error.value() != eSocketErrorCodes::eConnectionAborted) // Operation Aborted
    {
        // Accept new connections for the same port. Could be not found when exiting.
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (mSocketAcceptors.find(IPLocator::getPhysicalPort(acceptor->mLocator)) != mSocketAcceptors.end())
        {
            mSocketAcceptors.at(IPLocator::getPhysicalPort(acceptor->mLocator))->Accept(this, mService);
        }
    }
}

/*
void TCPTransportInterface::SocketConnected(Locator_t& locator, SenderResource *senderResource, const asio::error_code& error)
{
    std::string value = error.message();
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    const Locator_t &physicalLocator = IPLocator::toPhysicalLocator(locator);
    if (mSocketConnectors.find(physicalLocator) != mSocketConnectors.end())
    {
        auto pendingConector = mSocketConnectors.at(physicalLocator);
        if (!error.value())
        {
            try
            {
                TCPChannelResource *outputSocket(nullptr);
                if (pendingConector->m_msgSize != 0)
                {
                    outputSocket = new TCPChannelResource(pendingConector->m_socket,
                        locator, true, false, pendingConector->m_msgSize);
                }
                else
                {
                    outputSocket = new TCPChannelResource(pendingConector->m_socket, locator, true, false);
                }

                outputSocket->ChangeStatus(TCPChannelResource::eConnectionStatus::eConnected);
                outputSocket->SetThread(
                    new std::thread(&TCPTransportInterface::performListenOperation, this, outputSocket));
                outputSocket->SetRTCPThread(
                    new std::thread(&TCPTransportInterface::performRTPCManagementThread, this, outputSocket));

                BindOutputChannel(locator);

                const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
                auto it = mChannelResources.find(physicalLocator);
                if (it == mChannelResources.end())
                {
                    mChannelResources[physicalLocator] = outputSocket;
                }

                for (auto& it : pendingConector->m_PendingLocators)
                {
                    EnqueueLogicalOutputPort(it);
                }

                Locator_t remoteLocator;
                EndpointToLocator(outputSocket->getSocket()->local_endpoint(), remoteLocator);
                BindSocket(remoteLocator, outputSocket);

                logInfo(RTCP, " Socket Connected (physical remote: " << IPLocator::getPhysicalPort(locator)
                    << ", local: " << outputSocket->getSocket()->local_endpoint().port()
                    << ") IP: " << outputSocket->getSocket()->remote_endpoint().address());

                // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                // std::time_t now_c = std::chrono::system_clock::to_time_t(now);
                // std::cout << std::put_time(std::localtime(&now_c), "%F %T") << "--> Socket Connected (physical remote: " << locator.get_physical_port()
                //     << ", local: " << outputSocket->getSocket()->local_endpoint().port()
                //     << ") IP: " << outputSocket->getSocket()->remote_endpoint().address() << std::endl;

                // RTCP Control Message
                mRTCPMessageManager->sendConnectionRequest(outputSocket);
            }
            catch (asio::system_error const& e)
            {
                (void)e;
                logInfo(RTPS_MSG_OUT, "TCPTransport Error establishing the connection at port:(" << IPLocator::getPhysicalPort(locator) << ")" << " with msg:" << e.what());
                CloseOutputChannel(locator);
            }
        }
        else
        {
            eClock::my_sleep(100);
            //std::cout << "Reconnect..." << this << std::endl;
            pendingConector->RetryConnect(mService, this, senderResource);
        }
    }
}
*/

void TCPTransportInterface::UnbindSocket(TCPChannelResource *pSocket)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    auto it = mChannelResources.find(IPLocator::toPhysicalLocator(pSocket->mLocator));
    if (it != mChannelResources.end())
    {
        mChannelResources.erase(it);
    }
}

bool TCPTransportInterface::fillMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(metatraffic_multicast_port));
    }

    return true;
}

bool TCPTransportInterface::fillMetatrafficUnicastLocator(Locator_t &locator,
        uint32_t metatraffic_unicast_port)
{
    if (locator.port == 0)
    {
        locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(metatraffic_unicast_port));
    }

    if (GetConfiguration() != nullptr)
    {
        // TODO: Add physical port and WAN address
    }

    return true;
}

bool TCPTransportInterface::configureInitialPeerLocator(Locator_t &locator, const PortParameters &port_params,
        uint32_t domainId, LocatorList_t& list) const
{
    if(IPLocator::getPhysicalPort(locator) == 0)
    {
        // TODO(Ricardo) Make configurable.
        for(int32_t i = 0; i < 4; ++i)
        {
            Locator_t auxloc(locator);
            auxloc.port = static_cast<uint16_t>(port_params.getUnicastPort(domainId, i));

            if (IPLocator::getLogicalPort(locator) == 0)
            {
                IPLocator::setLogicalPort(auxloc, static_cast<uint16_t>(port_params.getUnicastPort(domainId, i)));
            }

            list.push_back(auxloc);
        }
    }
    else
    {
        if (IPLocator::getLogicalPort(locator) == 0)
        {
            // TODO(Ricardo) Make configurable.
            /*for(int32_t i = 0; i < 4; ++i)
            {
                Locator_t auxloc(locator);
                IPLocator::setLogicalPort(auxloc, static_cast<uint16_t>(port_params.getUnicastPort(domainId, i)));
                list.push_back(auxloc);
            }*/
            Locator_t auxloc(locator);
            IPLocator::setLogicalPort(auxloc, static_cast<uint16_t>(port_params.getUnicastPort(domainId, 0)));
            list.push_back(auxloc);
        }
        else
        {
            list.push_back(locator);
        }
    }

    return true;
}

bool TCPTransportInterface::fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const
{
    if (locator.port == 0)
    {
        locator.port = well_known_port;
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(well_known_port));
    }

    return true;
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
