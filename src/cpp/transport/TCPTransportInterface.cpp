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
#include <fastrtps/transport/TCPChannelResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static const int s_default_keep_alive_frequency = 50000; // 50 SECONDS
static const int s_default_keep_alive_timeout = 10000; // 10 SECONDS
static const int s_clean_deleted_sockets_pool_timeout = 100; // 100 MILLISECONDS
static const int s_default_tcp_negotitation_timeout = 5000; // 5 Seconds

TCPAcceptor::TCPAcceptor(asio::io_service& io_service, TCPTransportInterface* parent, const Locator_t& locator, uint32_t maxMsgSize)
    : mAcceptor(io_service, parent->GenerateEndpoint(locator.get_physical_port()))
    , mLocator(locator)
    , mMaxMsgSize(maxMsgSize)
{
#ifndef ASIO_HAS_MOVE
    mSocket = std::make_shared<asio::ip::tcp::socket>(io_service);
    mEndPoint = asio::ip::tcp::endpoint(parent->GenerateProtocol(), locator.get_physical_port());
#endif
}


#ifdef ASIO_HAS_MOVE
void TCPAcceptor::Accept(TCPTransportInterface* parent, asio::io_service&)
{
    mAcceptor.async_accept(std::bind(&TCPTransportInterface::SocketAccepted, parent, this, std::placeholders::_1,
        std::placeholders::_2));
}
#else
void TCPAcceptor::Accept(TCPTransportInterface* parent, asio::io_service& io_service)
{
    if (mSocket == nullptr)
    {
        mSocket = std::make_shared<asio::ip::tcp::socket>(io_service);
    }
	mAcceptor.async_accept(*mSocket.get(), mEndPoint, std::bind(&TCPTransportInterface::SocketAccepted, parent, this, std::placeholders::_1));
}
#endif

TCPConnector::TCPConnector(asio::io_service& io_service, const Locator_t& locator, uint32_t msgSize)
    : m_locator(locator)
    , m_socket(createTCPSocket(io_service))
    , m_msgSize(msgSize)
{
}

TCPConnector::~TCPConnector()
{
    getSocketPtr(m_socket)->close();
}

void TCPConnector::Connect(TCPTransportInterface* parent, SenderResource *senderResource)
{
    getSocketPtr(m_socket)->open(parent->GetProtocolType());
    ip::tcp::endpoint endpoint = parent->GenerateLocalEndpoint(m_locator, m_locator.get_physical_port());
    getSocketPtr(m_socket)->async_connect(endpoint, std::bind(&TCPTransportInterface::SocketConnected, parent,
        m_locator, senderResource, std::placeholders::_1));
}

void TCPConnector::RetryConnect(asio::io_service& io_service, TCPTransportInterface* parent, SenderResource *senderResource)
{
    getSocketPtr(m_socket)->close();
    m_socket = createTCPSocket(io_service);
    Connect(parent, senderResource);
}

TCPTransportDescriptor::TCPTransportDescriptor()
    : TransportDescriptorInterface(s_maximumMessageSize)
    , keep_alive_frequency_ms(s_default_keep_alive_frequency)
    , keep_alive_timeout_ms(s_default_keep_alive_timeout)
    , max_logical_port(100)
    , logical_port_range(20)
    , logical_port_increment(2)
    , metadata_logical_port(0)
	, tcp_negotiation_timeout(s_default_tcp_negotitation_timeout)
	, wait_for_tcp_negotiation(true)
{
}

TCPTransportDescriptor::TCPTransportDescriptor(const TCPTransportDescriptor& t)
    : TransportDescriptorInterface(t)
    , listening_ports(t.listening_ports)
    , keep_alive_frequency_ms(t.keep_alive_frequency_ms)
    , keep_alive_timeout_ms(t.keep_alive_timeout_ms)
    , max_logical_port(t.max_logical_port)
    , logical_port_range(t.logical_port_range)
    , logical_port_increment(t.logical_port_increment)
    , metadata_logical_port(t.metadata_logical_port)
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

        for (auto it = mSocketConnectors.begin(); it != mSocketConnectors.end(); ++it)
        {
            delete(it->second);
        }
        mSocketConnectors.clear();

        for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
        {
            vDeletedSockets.push_back(*it);
        }
        mOutputSockets.clear();

        for (auto it = mInputSockets.begin(); it != mInputSockets.end(); ++it)
        {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                vDeletedSockets.push_back(*it2);
            }
        }
        mInputSockets.clear();
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
    }

    CleanDeletedSockets();

    if (ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }

    delete mRTCPMessageManager;
}

void TCPTransportInterface::AssociateSenderToSocket(TCPChannelResource *socket, SenderResource *sender) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    auto it = mSocketToSenders.find(socket);
    if (it == mSocketToSenders.end())
    {
        mSocketToSenders[socket].emplace_back(sender);
        socket->AddLogicalConnection();
    }
    else if (std::find(it->second.begin(), it->second.end(), sender) == it->second.end())
    {
        it->second.emplace_back(sender);
    }

    //sender->SetChannelResource(socket);
}

void TCPTransportInterface::BindOutputChannel(const Locator_t& locator, SenderResource *senderResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
    {
        return;
    }

    for (auto it = mSocketConnectors.begin(); it != mSocketConnectors.end(); ++it)
    {
        if (CompareLocatorIPAndPort(it->first, locator) && std::find(it->second->m_PendingLocators.begin(),
            it->second->m_PendingLocators.end(), locator) == it->second->m_PendingLocators.end())
        {
            it->second->m_PendingLocators.emplace_back(locator);
        }
    }

    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if (CompareLocatorIPAndPort((*it)->GetLocator(), locator))
        {
            BindSocket(locator, *it);
            if (senderResource != nullptr)
            {
                AssociateSenderToSocket(*it, senderResource);
            }
        }
    }
}

void TCPTransportInterface::BindSocket(const Locator_t& locator, TCPChannelResource *pChannelResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
    {
        return;
    }

    auto it = mBoundOutputSockets.find(locator);
    if (it == mBoundOutputSockets.end())
    {
        mBoundOutputSockets[locator].push_back(pChannelResource); // First element, just add it
    }
    else
    {
        // There are more, check isn't already added
        auto found = std::find(mBoundOutputSockets[locator].begin(), mBoundOutputSockets[locator].end(), pChannelResource);
        if (found == mBoundOutputSockets[locator].end())
        {
            mBoundOutputSockets[locator].push_back(pChannelResource);
        }
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


bool TCPTransportInterface::CreateAcceptorSocket(const Locator_t& locator, uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    try
    {
        TCPAcceptor* newAcceptor = new TCPAcceptor(mService, this, locator, maxMsgSize);
        mSocketAcceptors.insert(std::make_pair(locator.get_physical_port(), newAcceptor));
        logInfo(RTCP, " OpenAndBindInput (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")");
        newAcceptor->Accept(this, mService);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPTransport Error binding at port: (" << locator.get_physical_port() << ")" << " with msg: " << e.what());
        return false;
    }

    return true;
}

void TCPTransportInterface::CreateConnectorSocket(const Locator_t& locator, SenderResource *senderResource,
    std::vector<Locator_t>& pendingLocators, uint32_t msgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (IsInterfaceAllowed(locator))
    {
        TCPConnector* newConnector = new TCPConnector(mService, locator, msgSize);
        newConnector->m_PendingLocators = pendingLocators;
        if (mSocketConnectors.find(locator) != mSocketConnectors.end())
        {
            TCPConnector* oldConnector = mSocketConnectors.at(locator);
            delete oldConnector;
        }
        mSocketConnectors[locator] = newConnector;
        logInfo(RTCP, " OpenAndBindOutput (physical: " << locator.get_physical_port() <<
            "; logical: " << locator.get_logical_port() << ")");
        newConnector->Connect(this, senderResource);
    }
}

bool TCPTransportInterface::EnqueueLogicalOutputPort(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (IsTCPInputSocket(locator))
    {
        auto socketIt = mInputSockets.find(locator.get_physical_port());
        if (socketIt != mInputSockets.end())
        {
            for (auto it = socketIt->second.begin(); it != socketIt->second.end(); ++it)
            {
                (*it)->EnqueueLogicalPort(locator.get_logical_port());
            }
            return true;
        }
        return false;
    }
    else
    {
        for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
        {
            // Checks that the given logical port matches with the IP and port,
            // and checks that the logical port ins't opened or pending to open.
            if (CompareLocatorIPAndPort((*it)->GetLocator(), locator))
            {
                (*it)->EnqueueLogicalPort(locator.get_logical_port());
                return true;
            }
        }
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

    auto socket = mBoundOutputSockets.find(locator);
    if (socket != mBoundOutputSockets.end())
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

    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if ((*it)->GetLocator().compare_IP6_address_and_port(locator))
        {
            return true;
        }
    }

    return IsOutputChannelBound(locator);
}

bool TCPTransportInterface::IsTCPInputSocket(const Locator_t& locator) const
{
    if (is_local_locator(locator))
    {
        for (auto it = GetConfiguration()->listening_ports.begin(); it != GetConfiguration()->listening_ports.end(); ++it)
        {
            if (locator.get_physical_port() == *it)
            {
                return true;
            }
        }
    }
    return false;
}

bool TCPTransportInterface::DoInputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.get_port() == right.get_port();
}

bool TCPTransportInterface::DoOutputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.get_port() == right.get_port();
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
    return IsLocatorSupported(locator) && mReceiverResources.find(locator) != mReceiverResources.end();
}

bool TCPTransportInterface::IsInputSocketOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.get_physical_port()) != mInputSockets.end() ||
        (mSocketAcceptors.find(locator.get_physical_port()) != mSocketAcceptors.end()));
}

bool TCPTransportInterface::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == mTransportKind;
}

bool TCPTransportInterface::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // Check if there is any connector to the given locator.
    for (auto it = mSocketConnectors.begin(); it != mSocketConnectors.end(); ++it)
    {
        if (CompareLocatorIPAndPort(it->first, locator))
        {
            return true;
        }
    }

    // Check if there is any socket opened with the given locator.
    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if (CompareLocatorIPAndPort((*it)->GetLocator(), locator))
        {
            return true;
        }
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
    std::vector<TCPChannelResource*> vDeletedSockets;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (!IsOutputChannelOpen(locator))
            return false;

        if (mSocketConnectors.find(locator) != mSocketConnectors.end())
        {
            auto& pendingSocket = mSocketConnectors.at(locator);
            getSocketPtr(pendingSocket->m_socket)->close();
            delete(mSocketConnectors[locator]);
            mSocketConnectors.erase(locator);
        }

        if (IsTCPInputSocket(locator))
        {
            for (auto it = mPendingOutputPorts.begin(); it != mPendingOutputPorts.end();)
            {
                if (it->first == locator)
                {
                    it = mPendingOutputPorts.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        mBoundOutputSockets.erase(locator);

        for (auto it = mOutputSockets.begin(); it != mOutputSockets.end();)
        {
            if ((*it)->GetLocator() == locator)
            {
                vDeletedSockets.emplace_back(*it);
                it = mOutputSockets.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    for (auto it = vDeletedSockets.begin(); it != vDeletedSockets.end(); ++it)
    {
        ReleaseTCPSocket(*it, false);
        (*it) = nullptr;
    }
    vDeletedSockets.clear();

    return true;
}

void TCPTransportInterface::CloseInputSocket(TCPChannelResource *pChannelResource)
{
    if (pChannelResource->IsAlive())
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eDisconnected);
        auto inputSocketIt = mInputSockets.find(pChannelResource->GetLocator().get_physical_port());
        if (inputSocketIt != mInputSockets.end())
        {
            auto& vector = mInputSockets[pChannelResource->GetLocator().get_physical_port()];
            auto it = std::find(vector.begin(), vector.end(), pChannelResource);
            vector.erase(it);

            if (vector.size() == 0)
            {
                mInputSockets.erase(inputSocketIt);
            }
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

        if (mReceiverResources.find(locator) != mReceiverResources.end())
        {
            mReceiverResources.erase(locator);
            bClosed = true;
        }

        auto acceptorIt = mSocketAcceptors.find(locator.get_physical_port());
        if (acceptorIt != mSocketAcceptors.end())
        {
            delete acceptorIt->second;
            mSocketAcceptors.erase(acceptorIt);
            bClosed = true;
        }

        auto InputSocketIt = mInputSockets.find(locator.get_physical_port());
        if (InputSocketIt != mInputSockets.end())
        {
            std::vector<TCPChannelResource*> sockets = InputSocketIt->second;
            for (auto it = sockets.begin(); it != sockets.end(); ++it)
            {
                vDeletedSockets.emplace_back(*it);
            }

            mInputSockets.erase(InputSocketIt);
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

            // Create a new connector to retry the connection.
            CreateConnectorSocket(prevLocator, nullptr, logicalLocators, msgSize);
        }
    }
}


bool TCPTransportInterface::OpenOutputChannel(const Locator_t& locator, SenderResource* senderResource, uint32_t msgSize)
{
    bool success = false;
    if (IsLocatorSupported(locator))
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (!IsTCPInputSocket(locator))
        {
            if (!IsOutputChannelConnected(locator))
            {
                if (!IsOutputChannelOpen(locator))
                {
                    success = OpenOutputSockets(locator, senderResource, msgSize);
                }
                else
                {
                    BindOutputChannel(locator, senderResource);
                    success = EnqueueLogicalOutputPort(locator);
                }
            }
            else
            {
                if (!IsOutputChannelBound(locator))
                {
                    BindOutputChannel(locator, senderResource);
                    success = EnqueueLogicalOutputPort(locator);
                }
                else
                    success = true;
            }
        }
        else
        {
            if (mBoundOutputSockets.find(locator) == mBoundOutputSockets.end())
            {
                if (std::find(mPendingOutputPorts[locator].begin(), mPendingOutputPorts[locator].end(),
                    senderResource) == mPendingOutputPorts[locator].end())
                {
                    mPendingOutputPorts[locator].push_back(senderResource);
                }

                auto socketIt = mInputSockets.find(locator.get_physical_port());
                if (socketIt != mInputSockets.end())
                {
                    for (auto it = socketIt->second.begin(); it != socketIt->second.end(); ++it)
                    {
                        BindSocket(locator, *it);
                        success = EnqueueLogicalOutputPort(locator);
                    }
                }
            }
            success = true;
        }
    }
    else
    {
        success = true;
    }
    return success;
}

bool TCPTransportInterface::OpenExtraOutputChannel(const Locator_t& locator, SenderResource* senderResource, uint32_t msgSize)
{
    return OpenOutputChannel(locator, senderResource, msgSize);
}

bool TCPTransportInterface::OpenInputChannel(const Locator_t& locator, ReceiverResource* receiverResource,
    uint32_t maxMsgSize)
{
    bool success = false;
    if (IsLocatorSupported(locator))
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (mReceiverResources.find(locator) == mReceiverResources.end())
        {
            mReceiverResources[locator] = receiverResource;

            logInfo(RTCP, " OpenInputChannel (physical: " << locator.get_physical_port() << "; logical: " << locator.get_logical_port() << ")");
            if (IsTCPInputSocket(locator))
            {
                success = true;
                if (!IsInputSocketOpen(locator))
                {
                    success = CreateAcceptorSocket(locator, maxMsgSize);
                }
            }
            else
            {
                success = true;
                if (IsOutputChannelConnected(locator))
                {
                    TCPChannelResource* pSocket = nullptr;
                    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
                    {
                        if ((*it)->GetLocator().compare_IP6_address_and_port(locator))
                        {
                            pSocket = *it;
                            break;
                        }
                    }

                    if (pSocket != nullptr)
                    {
                        RegisterReceiverResources(pSocket, locator);
                    }
                }
            }
        }
        else
        {
            success = false;
        }

    }
    return success;
}

bool TCPTransportInterface::OpenOutputSockets(const Locator_t& locator, SenderResource *senderResource, uint32_t msgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    try
    {
        std::vector<Locator_t> emptyLocators;
        CreateConnectorSocket(locator, senderResource, emptyLocators, msgSize);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCP Error binding at port:(" << locator.get_port() << ")" << " with msg:" << e.what());
        CloseOutputChannel(locator);
        return false;
    }

    return true;
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
    logInfo(RTCP, "START performRTPCManagementThread " << pChannelResource->GetLocator() << " (" << pChannelResource->getSocket()->local_endpoint().address() << "->" << pChannelResource->getSocket()->remote_endpoint().address() << ")");
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
    logInfo(RTCP, "START PerformListenOperation " << pChannelResource->GetLocator() << " (" << pChannelResource->getSocket()->local_endpoint().address() << "->" << pChannelResource->getSocket()->remote_endpoint().address() << ")");
    while (pChannelResource->IsAlive())
    {
        // Blocking receive.
        auto msg = pChannelResource->GetMessageBuffer();
        CDRMessage::initCDRMsg(&msg);
        if (!Receive(pChannelResource, msg.buffer, msg.max_size, msg.length, logicalPort))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        MessageReceiver* receiver = pChannelResource->GetMessageReceiver(logicalPort);
        if (receiver != nullptr)
        {
            receiver->processCDRMsg(rtpsParticipantGuidPrefix, &pChannelResource->mLocator, &msg);
        }
        else
        {
            logWarning(RTCP, "Received Message, but no MessageReceiver attached: " << logicalPort);
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
    uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize, uint16_t& logicalPort)
{
    bool success = false;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(*pChannelResource->GetReadMutex());
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

                logInfo(RTPS_MSG_IN, "[RECEIVE] From: " << pChannelResource->getSocket()->remote_endpoint().address() << " to " << pChannelResource->getSocket()->local_endpoint().address());

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
                            logicalPort = tcp_header.logicalPort;
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
        if (mInputSockets.find(pChannelResource->GetLocator().get_physical_port()) != mInputSockets.end())
        {
            auto& sockets = mInputSockets.at(pChannelResource->GetLocator().get_physical_port());
            for (auto it = sockets.begin(); it != sockets.end();)
            {
                if (!(*it)->IsAlive())
                {
                    it = sockets.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    return success;
}

void TCPTransportInterface::RegisterReceiverResources(TCPChannelResource* pChannelResource, const Locator_t& locator)
{
    // Create one message receiver for each registered receiver resources.
    for (auto it = mReceiverResources.begin(); it != mReceiverResources.end(); ++it)
    {
        if (CompareLocatorIPAndPort(it->first, locator) &&
            pChannelResource->GetMessageReceiver(it->first.get_logical_port()) == nullptr)
        {
            pChannelResource->AddMessageReceiver(it->first.get_logical_port(), it->second->CreateMessageReceiver());
            pChannelResource->mLogicalInputPorts.emplace_back(it->first.get_logical_port());
            pChannelResource->AddLogicalConnection();
        }
    }
}

void TCPTransportInterface::ReleaseTCPSocket(TCPChannelResource *pChannelResource, bool force)
{
    if (pChannelResource != nullptr && pChannelResource->IsAlive())
    {
        pChannelResource->RemoveLogicalConnection();
        if (force || pChannelResource->HasLogicalConnections())
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
            catch (std::exception)
            {
                // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
            }
            pChannelResource->getSocket()->close();

            // Remove from senders
            auto it = mSocketToSenders.find(pChannelResource);
            if (it != mSocketToSenders.end())
            {
                /*
                auto& senders = mSocketToSenders.at(pChannelResource);
                for (auto& sender : senders)
                {
                sender->SetChannelResource(nullptr);
                }
                */
                mSocketToSenders.erase(it);
            }

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
        std::unique_lock<std::recursive_mutex> scopedLock(*pChannelResource->GetWriteMutex());
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
    //    logInfo(RTCP, " SEND [RTPS Data] to locator " << remoteLocator.get_physical_port() << ":" << remoteLocator.get_logical_port());
    std::vector<TCPChannelResource* > sendVector;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (!IsOutputChannelConnected(remoteLocator) || sendBufferSize > GetConfiguration()->sendBufferSize)
        {
            logWarning(RTCP, "SEND [RTPS] Failed: Not connect: " << remoteLocator.get_logical_port());
            return false;
        }

        std::map<Locator_t, std::vector<TCPChannelResource*>>::iterator it = mBoundOutputSockets.find(remoteLocator);
        if (it == mBoundOutputSockets.end())
        {
            logWarning(RTCP, "SEND [RTPS] Failed: Not bound: " << remoteLocator.get_logical_port());
            return false;
        }
        else
        {
            sendVector = it->second;
        }
    }

    bool result = true;
    for (TCPChannelResource* socket : sendVector)
    {
        result = result && Send(sendBuffer, sendBufferSize, localLocator, remoteLocator, socket);
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
        uint16_t logicalPort = tcpChannelResource->mLogicalPortRouting.find(remoteLocator.get_logical_port())
            != tcpChannelResource->mLogicalPortRouting.end()
            ? tcpChannelResource->mLogicalPortRouting.at(remoteLocator.get_logical_port())
            : remoteLocator.get_logical_port();

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
                std::unique_lock<std::recursive_mutex> sendLock(*tcpChannelResource->GetWriteMutex());
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
        logWarning(RTCP, " SEND [RTPS] Failed: Connection not established" << remoteLocator.get_logical_port());
        eClock::my_sleep(100);
        return false;
    }
}

bool TCPTransportInterface::SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize,
    const Locator_t& remoteLocator, TCPChannelResource *socket)
{
    auto destinationEndpoint = GenerateEndpoint(remoteLocator, remoteLocator.get_physical_port());

    size_t bytesSent = 0;
    (void)destinationEndpoint;
    logInfo(RTPS_MSG_OUT, "TCP: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint << " FROM " << socket->getSocket()->local_endpoint());

    //logInfo(RTCP, "SOCKET SEND to physical port " << socket->getSocket()->remote_endpoint().port());

    eSocketErrorCodes errorCode;
    bytesSent = Send(socket, sendBuffer, sendBufferSize, errorCode);
    switch (errorCode)
    {
    case eNoError:
        //logInfo(RTCP, " Sent [OK]: " << sendBufferSize << " bytes to locator " << remoteLocator.get_logical_port());
        break;
    default:
        // Close the channel
        logInfo(RTCP, " Sent [FAILED]: " << sendBufferSize << " bytes to locator " << remoteLocator.get_logical_port() << " ERROR=" << errorCode);
        Locator_t prevLocator = socket->GetLocator();
        uint32_t msgSize = socket->GetMsgSize();

        std::vector<Locator_t> logicalLocators;
        socket->fillLogicalPorts(logicalLocators);
        CloseOutputChannel(socket->GetLocator());

        // Create a new connector to retry the connection.
        CreateConnectorSocket(prevLocator, nullptr, logicalLocators, msgSize);
        break;
    }

    logInfo(RTPS_MSG_OUT, "SENT " << bytesSent);
    return bytesSent > 0;
}

void TCPTransportInterface::SetParticipantGUIDPrefix(const GuidPrefix_t& prefix)
{
    rtpsParticipantGuidPrefix = prefix;
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
                    loopbackLocator.set_port(it->get_physical_port());
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

#ifdef ASIO_HAS_MOVE
void TCPTransportInterface::SocketAccepted(TCPAcceptor* acceptor, const asio::error_code& error, asio::ip::tcp::socket socket)
{
    if (!error.value())
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (mSocketAcceptors.find(acceptor->mLocator.get_physical_port()) != mSocketAcceptors.end())
        {
            // Store the new connection.
            eProsimaTCPSocket unicastSocket = eProsimaTCPSocket(std::move(socket));
            TCPChannelResource *pChannelResource = new TCPChannelResource(unicastSocket, acceptor->mLocator, false, true,
                acceptor->mMaxMsgSize);
            pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eWaitingForBind);
            RegisterReceiverResources(pChannelResource, acceptor->mLocator);

            for (auto it = mPendingOutputPorts.begin(); it != mPendingOutputPorts.end(); ++it)
            {
                if (CompareLocatorIPAndPort(acceptor->mLocator, it->first))
                {
                    BindSocket(it->first, pChannelResource);
                    for (auto senderIt : it->second)
                    {
                        AssociateSenderToSocket(pChannelResource, senderIt);
                    }
                }
            }
            pChannelResource->SetThread(new std::thread(&TCPTransportInterface::performListenOperation, this, pChannelResource));
            pChannelResource->SetRTCPThread(new std::thread(&TCPTransportInterface::performRTPCManagementThread, this, pChannelResource));

            mInputSockets[acceptor->mLocator.get_physical_port()].emplace_back(pChannelResource);
            logInfo(RTCP, " Accepted connection (physical local: " << acceptor->mLocator.get_physical_port()
                << ", remote: " << pChannelResource->getSocket()->remote_endpoint().port()
                << ") IP: " << pChannelResource->getSocket()->remote_endpoint().address());
        }
        else
        {
            logError(RTPC, "Incomming connection from unknown Acceptor: " << acceptor->mLocator.get_physical_port());
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
        if (mSocketAcceptors.find(acceptor->mLocator.get_physical_port()) != mSocketAcceptors.end())
        {
            mSocketAcceptors.at(acceptor->mLocator.get_physical_port())->Accept(this, mService);
        }
    }
}
#else

void TCPTransportInterface::SocketAccepted(TCPAcceptor* acceptor, const asio::error_code& error)
{
    if (!error.value())
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (mSocketAcceptors.find(acceptor->mLocator.get_physical_port()) != mSocketAcceptors.end())
        {
            // Store the new connection and release the pointer to the acceptor to create a new one for the next waiting.
            eProsimaTCPSocket unicastSocket = eProsimaTCPSocket(acceptor->mSocket);
            acceptor->mSocket = nullptr;

            TCPChannelResource *pChannelResource = new TCPChannelResource(unicastSocket,
                acceptor->mLocator, false, true, acceptor->mMaxMsgSize);
            pChannelResource->SetThread(new std::thread(&TCPTransportInterface::performListenOperation, this, pChannelResource));
            pChannelResource->SetRTCPThread(new std::thread(&TCPTransportInterface::performRTPCManagementThread, this, pChannelResource));
            pChannelResource->ChangeStatus(TCPChannelResource::eConnectionStatus::eWaitingForBind);

            // Create one message receiver for each registered receiver resources.
            RegisterReceiverResources(pChannelResource, acceptor->mLocator);
            mInputSockets[acceptor->mLocator.get_physical_port()].emplace_back(pChannelResource);

            for (auto it = mPendingOutputPorts.begin(); it != mPendingOutputPorts.end(); ++it)
            {
                if (CompareLocatorIPAndPort(acceptor->mLocator, it->first))
                {
                    BindSocket(it->first, pChannelResource);
                    for (auto senderIt : it->second)
                    {
                        AssociateSenderToSocket(pChannelResource, senderIt);
                    }
                }
            }

            logInfo(RTCP, " Accepted connection (physical local: " << acceptor->mLocator.get_physical_port() << ", remote: " << pChannelResource->getSocket()->remote_endpoint().port() << ") IP: " << pChannelResource->getSocket()->remote_endpoint().address());
        }
        else
        {
            logError(RTPC, "Incomming connection from unknown Acceptor: " << acceptor->mLocator.get_physical_port());
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
        if (mSocketAcceptors.find(acceptor->mLocator.get_physical_port()) != mSocketAcceptors.end())
        {
            mSocketAcceptors.at(acceptor->mLocator.get_physical_port())->Accept(this, mService);
        }
    }
}
#endif

void TCPTransportInterface::SocketConnected(Locator_t& locator, SenderResource *senderResource, const asio::error_code& error)
{
    std::string value = error.message();
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mSocketConnectors.find(locator) != mSocketConnectors.end())
    {
        auto pendingConector = mSocketConnectors.at(locator);
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

                // Create one message receiver for each registered receiver resources.
                RegisterReceiverResources(outputSocket, locator);
                outputSocket->ChangeStatus(TCPChannelResource::eConnectionStatus::eConnected);
                outputSocket->SetThread(new std::thread(&TCPTransportInterface::performListenOperation, this, outputSocket));
                outputSocket->SetRTCPThread(new std::thread(&TCPTransportInterface::performRTPCManagementThread, this, outputSocket));
                mOutputSockets.push_back(outputSocket);
                BindOutputChannel(locator);

                for (auto& it : pendingConector->m_PendingLocators)
                {
                    EnqueueLogicalOutputPort(it);
                }

                logInfo(RTCP, " Socket Connected (physical remote: " << locator.get_physical_port()
                    << ", local: " << outputSocket->getSocket()->local_endpoint().port()
                    << ") IP: " << outputSocket->getSocket()->remote_endpoint().address());

                // RTCP Control Message
                mRTCPMessageManager->sendConnectionRequest(outputSocket, GetConfiguration()->metadata_logical_port);
                if (senderResource != nullptr)
                {
                    AssociateSenderToSocket(outputSocket, senderResource);
                }
            }
            catch (asio::system_error const& e)
            {
                (void)e;
                logInfo(RTPS_MSG_OUT, "TCPTransport Error establishing the connection at port:(" << locator.get_port() << ")" << " with msg:" << e.what());
                CloseOutputChannel(locator);
            }
        }
        else
        {
            eClock::my_sleep(100);
            std::cout << "Reconnect..." << this << std::endl;
            pendingConector->RetryConnect(mService, this, senderResource);
        }
    }
}

void TCPTransportInterface::UnbindSocket(TCPChannelResource *pSocket)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    for (auto it = mBoundOutputSockets.begin(); it != mBoundOutputSockets.end();)
    {
        auto& vector = it->second;
        auto toRemove = std::find(vector.begin(), vector.end(), pSocket);
        if (toRemove != vector.end())
        {
            vector.erase(toRemove);
        }

        if (vector.empty())
        {
            it = mBoundOutputSockets.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
