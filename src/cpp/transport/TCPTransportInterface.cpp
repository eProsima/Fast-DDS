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

    if (mCleanSocketsPoolTimer != nullptr)
    {
        mCleanSocketsPoolTimer->cancel_timer();
        delete mCleanSocketsPoolTimer;
        mCleanSocketsPoolTimer = nullptr;
    }

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
            auto delIt = std::find(vDeletedSockets.begin(), vDeletedSockets.end(), it->second);
            if (delIt == vDeletedSockets.end())
            {
                vDeletedSockets.push_back(it->second);
            }
            if (it->second->IsConnectionEstablished())
            {
                mRTCPMessageManager->sendUnbindConnectionRequest(it->second);
            }
        }
        mChannelResources.clear();

        vDeletedSockets.insert(vDeletedSockets.end(), mUnboundChannelResources.begin(),
            mUnboundChannelResources.end());
        mUnboundChannelResources.clear();
    }

    std::for_each(vDeletedSockets.begin(), vDeletedSockets.end(), [](auto it){
        it->Disable(); // Disable all added TCPChannelResources
    });

    {
        std::unique_lock<std::recursive_mutex> scopedLock(mDeletedSocketsPoolMutex);
        std::for_each(vDeletedSockets.begin(), vDeletedSockets.end(), [&](auto socket){
            auto it = std::find(mDeletedSocketsPool.begin(), mDeletedSocketsPool.end(), socket);
            if (it == mDeletedSocketsPool.end())
            {
                mDeletedSocketsPool.emplace_back(socket);
            }
        });
    }

    CleanDeletedSockets();

    if (ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }

    delete mRTCPMessageManager;
}

TCPChannelResource* TCPTransportInterface::BindSocket(const Locator_t& locator, TCPChannelResource *pChannelResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (IsLocatorSupported(locator))
    {
        std::remove(mUnboundChannelResources.begin(), mUnboundChannelResources.end(), pChannelResource);
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto it = mChannelResources.find(physicalLocator);
        if (it == mChannelResources.end())
        {
            mChannelResources[physicalLocator] = pChannelResource;
            return nullptr;
        }

        TCPChannelResource* oldChannel = it->second;
        if (oldChannel->IsConnectionEstablished())
        {
            logWarning(RTCP, "Trying to restablish connection on already connected locator." << locator);
        }
        else
        {
            mChannelResources[physicalLocator] = pChannelResource;
            return oldChannel;
        }
    }
    return nullptr;
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

bool TCPTransportInterface::CheckCRC(const TCPHeader &header, const octet *data, uint32_t size) const
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    return crc == header.crc;
}

void TCPTransportInterface::CalculateCRC(TCPHeader &header, const octet *data, uint32_t size) const
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
        logInfo(RTCP_MSG_OUT, "TCPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")" << " with msg: " << e.what());
        return false;
    }

    return true;
}

bool TCPTransportInterface::EnqueueLogicalOutputPort(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    auto socketIt = mChannelResources.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != mChannelResources.end())
    {
        socketIt->second->AddLogicalPort(IPLocator::getLogicalPort(locator));
        return true;
    }
    return false;
}

void TCPTransportInterface::FillTCPHeader(TCPHeader& header, const octet* sendBuffer, uint32_t sendBufferSize,
        uint16_t logicalPort) const
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
        logError(RTCP_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (GetConfiguration()->maxMessageSize > GetConfiguration()->sendBufferSize)
    {
        logError(RTCP_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if (GetConfiguration()->maxMessageSize > GetConfiguration()->receiveBufferSize)
    {
        logError(RTCP_MSG_OUT, "maxMessageSize cannot be greater than receiveBufferSize");
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

bool TCPTransportInterface::IsInputPortOpen(uint16_t port) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    return mReceiverResources.find(port) != mReceiverResources.end();
}

bool TCPTransportInterface::IsInputChannelOpen(const Locator_t& locator) const
{
    return IsLocatorSupported(locator) && IsInputPortOpen(IPLocator::getLogicalPort(locator));
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
        socketIt->second->RemoveLogicalPort(IPLocator::getLogicalPort(locator));
        return true;
    }
    return false;
}

bool TCPTransportInterface::CloseInputChannel(const Locator_t& locator)
{
    bool bClosed = false;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);

        auto logicalPort = IPLocator::getLogicalPort(locator);
        auto receiverIt = mReceiverResources.find(logicalPort);
        if (receiverIt != mReceiverResources.end())
        {
            bClosed = true;
            mReceiverResources.erase(receiverIt);

            // Inform all channel resources that logical port has been closed
            for (auto channelIt : mChannelResources)
            {
                channelIt.second->InputPortClosed(logicalPort);
            }
        }
    }

    return bClosed;
}

void TCPTransportInterface::CloseTCPSocket(TCPChannelResource *pChannelResource)
{
    if (pChannelResource->IsAlive())
    {
        TCPChannelResource *newChannel = nullptr;
        auto physicalLocator = IPLocator::toPhysicalLocator(pChannelResource->GetLocator());
        if (!pChannelResource->GetIsInputSocket())
        {
            newChannel = new TCPChannelResource(this, mRTCPMessageManager, mService, physicalLocator);
            pChannelResource->SetAllPortsAsPending();
            newChannel->CopyPendingPortsFrom(pChannelResource);
        }

        {
            std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
            pChannelResource->Disable();
            auto it = mChannelResources.find(physicalLocator);
            if (it != mChannelResources.end())
            {
                mChannelResources.erase(it);
            }
        }

        {
            std::unique_lock<std::recursive_mutex> scopedPoolLock(mDeletedSocketsPoolMutex);
            auto it = std::find(mDeletedSocketsPool.begin(), mDeletedSocketsPool.end(), pChannelResource);
            if (it == mDeletedSocketsPool.end())
            {
                mDeletedSocketsPool.emplace_back(pChannelResource);
            }
        }

        if (newChannel != nullptr)
        {
            std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
            mChannelResources[physicalLocator] = newChannel;
            newChannel->Connect();
        }
    }
}


bool TCPTransportInterface::OpenOutputChannel(const Locator_t& locator, SenderResource* /*senderResource*/,
        uint32_t /*msgSize*/)
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
            // Create output channel
            auto socket = createTCPSocket(mService);
            channel = new TCPChannelResource(this, mRTCPMessageManager, mService, physicalLocator);
            mChannelResources[physicalLocator] = channel;
            mUnboundChannelResources.push_back(channel);
            channel->Connect();
        }

        success = true;
        channel->AddLogicalPort(logicalPort);
    }

    return success;
}

bool TCPTransportInterface::OpenExtraOutputChannel(const Locator_t& locator, SenderResource* senderResource,
        uint32_t msgSize)
{
    return OpenOutputChannel(locator, senderResource, msgSize);
}

bool TCPTransportInterface::OpenInputChannel(const Locator_t& locator, TransportReceiverInterface* receiver,
    uint32_t /*maxMsgSize*/)
{
    bool success = false;
    if (IsLocatorSupported(locator))
    {
        auto logicalPort = IPLocator::getLogicalPort(locator);
        if (!IsInputPortOpen(logicalPort))
        {
            success = true;
            {
                std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
                mReceiverResources[logicalPort] = receiver;
            }

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

    //*
    logInfo(RTCP, "START performRTPCManagementThread " << IPLocator::toIPv4string(pChannelResource->GetLocator()) \
        << ":" << IPLocator::getPhysicalPort(pChannelResource->GetLocator()) << " (" \
        << pChannelResource->getSocket()->local_endpoint().address() << ":" \
        << pChannelResource->getSocket()->local_endpoint().port() << "->" \
        << pChannelResource->getSocket()->remote_endpoint().address() << ":" \
        << pChannelResource->getSocket()->remote_endpoint().port() << ")");
    //*/

    while (pChannelResource->IsAlive())
    {
        if (pChannelResource->IsConnectionEstablished())
        {
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
                    CloseTCPSocket(pChannelResource);
                    break;
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
    /*
    logInfo(RTCP, "START PerformListenOperation " << pChannelResource->GetLocator() << " (" << \
        pChannelResource->getSocket()->local_endpoint().address() << "->" << \
        pChannelResource->getSocket()->remote_endpoint().address() << ")");
    */
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
        if (!pChannelResource->IsAlive())
        {
            success = false;
        }
        else
        {
            success = true;
            try
            {
                std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->GetReadMutex());
                // Read the header
                //octet header[TCPHEADER_SIZE];
                TCPHeader tcp_header;
                size_t bytes_received = read(*pChannelResource->getSocket(),
                    asio::buffer(&tcp_header, TCPHeader::getSize()),
                    transfer_exactly(TCPHeader::getSize()));

                remoteLocator = pChannelResource->GetLocator();

                // EndpointToLocator(pChannelResource->getSocket()->remote_endpoint(), remoteLocator);

                if (bytes_received != TCPHeader::getSize())
                {
                    logError(RTCP_MSG_IN, "Bad TCP header size: " << bytes_received << "(expected: : " << TCPHeader::getSize() << ")");
                    success = false;
                }
                else
                {
                    //TCPHeader tcp_header;
                    //memcpy(&tcp_header, header, TCPHeader::getSize());
                    size_t body_size = tcp_header.length - static_cast<uint32_t>(TCPHeader::getSize());
                    //logInfo(RTCP_MSG_IN, " Received [TCPHeader] (CRC=" << tcp_header.crc << ")");

                    if (body_size > receiveBufferCapacity)
                    {
                        logError(RTCP_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                            << static_cast<uint32_t>(body_size) << " vs. " << receiveBufferCapacity << ".");
                        success = false;
                    }
                    else
                    {
                        logInfo(RTCP_MSG_IN, "Received RTCP MSG. Logical Port " << tcp_header.logicalPort);
                        success = ReadBody(receiveBuffer, receiveBufferCapacity, &receiveBufferSize, pChannelResource, body_size);
                        //logInfo(RTCP_MSG_IN, " Received [ReadBody]");

                        if (!CheckCRC(tcp_header, receiveBuffer, receiveBufferSize))
                        {
                            logWarning(RTCP_MSG_IN, "Bad TCP header CRC");
                        }

                        if (tcp_header.logicalPort == 0)
                        {
                            //logInfo(RTCP_MSG_IN, " Receive [RTCP Control]  (" << receiveBufferSize+bytes_received
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
                            logInfo(RTCP_MSG_IN, "[RECEIVE] From: " << remoteLocator \
                                << " - " << receiveBufferSize << " bytes.");
                        }
                    }
                }
            }
            catch (const asio::error_code& code)
            {
                if ((code == asio::error::eof) || (code == asio::error::connection_reset))
                {
                    // Close the channel
                    logInfo(RTCP_MSG_IN, "ASIO [RECEIVE]: " << code.message());
                    //pChannelResource->ConnectionLost();
                    CloseTCPSocket(pChannelResource);
                }
                success = false;
            }
            catch (const asio::system_error& error)
            {
                (void)error;
                // Close the channel
                logInfo(RTCP_MSG_IN, "ASIO [RECEIVE]: " << error.what());
                //pChannelResource->ConnectionLost();
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

size_t TCPTransportInterface::Send(TCPChannelResource *pChannelResource, const octet *data,
    size_t size, eSocketErrorCodes &errorCode) const
{
    size_t bytesSent = 0;
    try
    {
        std::unique_lock<std::recursive_mutex> scopedLock(pChannelResource->GetWriteMutex());
        bytesSent = pChannelResource->getSocket()->send(asio::buffer(data, size));
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
    /*
    logInfo(RTCP, " SEND [RTPS Data] to locator " << IPLocator::getPhysicalPort(remoteLocator) << ":" << \
        IPLocator::getLogicalPort(remoteLocator));
    */

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
        uint16_t logicalPort = IPLocator::getLogicalPort(remoteLocator);

        if (tcpChannelResource->IsLogicalPortAdded(logicalPort))
        {
            bool bShouldWait = GetConfiguration()->wait_for_tcp_negotiation;
            bool bConnected = tcpChannelResource->IsAlive() && tcpChannelResource->IsConnectionEstablished();
            while (bShouldWait && bConnected && !tcpChannelResource->IsLogicalPortOpened(logicalPort))
            {
                bConnected = tcpChannelResource->WaitUntilPortIsOpenOrConnectionIsClosed(logicalPort);
            }

            if (bConnected && tcpChannelResource->IsLogicalPortOpened(logicalPort))
            {
                TCPHeader tcp_header;
                FillTCPHeader(tcp_header, sendBuffer, sendBufferSize, logicalPort);

                {
                    std::unique_lock<std::recursive_mutex> sendLock(tcpChannelResource->GetWriteMutex());
                    success = SendThroughSocket((octet*)&tcp_header, static_cast<uint32_t>(TCPHeader::getSize()), remoteLocator, tcpChannelResource);

                    if (success)
                    {
                        success = SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, tcpChannelResource);
                    }
                }
            }
        }
        return success;
    }
    else
    {
        logWarning(RTCP, " SEND [RTPS] Failed: Connection not established " \
            << IPLocator::getLogicalPort(remoteLocator));
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

    //logInfo(RTCP, "SOCKET SEND to physical port " << socket->getSocket()->remote_endpoint().port());

    eSocketErrorCodes errorCode;
    bytesSent = Send(socket, sendBuffer, sendBufferSize, errorCode);
    switch (errorCode)
    {
    case eNoError:
        //logInfo(RTCP, " Sent [OK]: " << sendBufferSize << " bytes to locator " << IPLocator::getLogicalPort(remoteLocator));
        break;
    default:
        // Inform that connection has been lost
        logInfo(RTCP, " Sent [FAILED]: " << sendBufferSize << " bytes to locator " << IPLocator::getLogicalPort(remoteLocator) << " ERROR=" << errorCode);
        //socket->ConnectionLost();
        CloseTCPSocket(socket);
        break;
    }

    logInfo(RTCP_MSG_OUT, "[SENT] TO " << remoteLocator << " - " << sendBufferSize << " (" << bytesSent << ").");
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
            TCPChannelResource *pChannelResource = new TCPChannelResource(this, mRTCPMessageManager, mService,
                unicastSocket);

            mUnboundChannelResources.push_back(pChannelResource);
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

void TCPTransportInterface::SocketConnected(TCPChannelResource *outputSocket)
{
    try
    {
        outputSocket->SetThread(
            new std::thread(&TCPTransportInterface::performListenOperation, this, outputSocket));
        outputSocket->SetRTCPThread(
            new std::thread(&TCPTransportInterface::performRTPCManagementThread, this, outputSocket));

        {
            std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
            auto it = std::find(mUnboundChannelResources.begin(), mUnboundChannelResources.end(), outputSocket);
            if (it != mUnboundChannelResources.end())
            {
                mUnboundChannelResources.erase(it);
            }
        }

        // RTCP Control Message
        mRTCPMessageManager->sendConnectionRequest(outputSocket);
    }
    catch (asio::system_error const& /*e*/)
    {
        /*
        (void)e;
        logInfo(RTCP_MSG_OUT, "TCPTransport Error establishing the connection at port:(" << IPLocator::getPhysicalPort(locator) << ")" << " with msg:" << e.what());
        CloseOutputChannel(locator);
        */
    }
}

void TCPTransportInterface::UnbindSocket(TCPChannelResource *pSocket)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    auto it = mChannelResources.find(IPLocator::toPhysicalLocator(pSocket->mLocator));
    if (it != mChannelResources.end())
    {
        mChannelResources.erase(it);
    }
}

bool TCPTransportInterface::fillMetatrafficMulticastLocator(Locator_t &, uint32_t) const
{
    // TCP doesn't have multicast support
    return true;
}

bool TCPTransportInterface::fillMetatrafficUnicastLocator(Locator_t &locator,
        uint32_t metatraffic_unicast_port)
{
    if (IPLocator::getPhysicalPort(locator.port) == 0)
    {
        auto config = GetConfiguration();
        if (config != nullptr)
        {
            if (!config->listening_ports.empty())
            {
                IPLocator::setPhysicalPort(locator, *(config->listening_ports.begin()));
            }
            else
            {
                // TODO Think about a "virtual physical port" to avoid send the real one
                IPLocator::setPhysicalPort(locator, 0);
            }
        }
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(metatraffic_unicast_port));
    }

    // TODO: Add WAN address

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
    if (IPLocator::getPhysicalPort(locator.port) == 0)
    {
        auto config = GetConfiguration();
        if (config != nullptr)
        {
            if (!config->listening_ports.empty())
            {
                IPLocator::setPhysicalPort(locator, *(config->listening_ports.begin()));
            }
            else
            {
                // TODO Think about a "virtual physical port" to avoid send the real one
                IPLocator::setPhysicalPort(locator, 0);
            }
        }
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(well_known_port));
    }

    // TODO: Add WAN address

    return true;
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
