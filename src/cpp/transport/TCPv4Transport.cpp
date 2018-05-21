// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/transport/TCPv4Transport.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/rtps/messages/TCPMessageReceiver.h>
#include <fastrtps/utils/eClock.h>
#include "asio.hpp"
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static void GetIP4s(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
        locNames.end(),
        [](IPFinder::info_IP ip) {return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL; });
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v4::bytes_type locatorToNative(Locator_t& locator)
{
    return{ {locator.get_IP4_address()[0],
        locator.get_IP4_address()[1], locator.get_IP4_address()[2], locator.get_IP4_address()[3]} };
}

TCPAcceptor::TCPAcceptor(asio::io_service& io_service, const Locator_t& locator, ReceiverResource* receiverResource,
    uint32_t receiveBufferSize)
    : m_acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), locator.get_physical_port()))
    , m_locator(locator)
    , m_receiveBufferSize(receiveBufferSize)
{
    m_acceptCallback = std::bind(&ReceiverResource::CreateMessageReceiver, receiverResource);
}

void TCPAcceptor::Accept(TCPv4Transport* parent)
{
    m_acceptor.async_accept(std::bind(&TCPv4Transport::SocketAccepted, parent, this, std::placeholders::_1,
        std::placeholders::_2));
}

TCPConnector::TCPConnector(asio::io_service& io_service,
    Locator_t& locator,
    SenderResource* senderResource,
    uint32_t sendBufferSize)
    : m_locator(locator)
    , m_sendBufferSize(sendBufferSize)
    , m_socket(createTCPSocket(io_service))
    , m_senderResource(senderResource)
    , m_messageReceiver(nullptr)
{
    m_connectCallback = std::bind(&SenderResource::CreateMessageReceiver, senderResource, std::placeholders::_1);
}

TCPConnector::TCPConnector(asio::io_service& io_service,
    Locator_t& locator,
    std::shared_ptr<MessageReceiver> messageReceiver,
    uint32_t sendBufferSize)
    : m_locator(locator)
    , m_sendBufferSize(sendBufferSize)
    , m_socket(createTCPSocket(io_service))
    , m_senderResource(nullptr)
    , m_messageReceiver(messageReceiver)
{
    m_connectCallback = nullptr;
}

void TCPConnector::Connect(TCPv4Transport* parent)
{
    getSocketPtr(m_socket)->open(ip::tcp::v4());
    auto ipAddress = asio::ip::address_v4(locatorToNative(m_locator));
    ip::tcp::endpoint endpoint(ipAddress, static_cast<uint16_t>(m_locator.get_physical_port()));
    getSocketPtr(m_socket)->async_connect(endpoint, std::bind(&TCPv4Transport::SocketConnected, parent,
        m_locator, m_sendBufferSize, std::placeholders::_1));
}

void TCPConnector::RetryConnect(asio::io_service& io_service, TCPv4Transport* parent)
{
    getSocketPtr(m_socket)->close();
    m_socket = createTCPSocket(io_service);
    Connect(parent);
}

TCPv4Transport::TCPv4Transport(const TCPv4TransportDescriptor& descriptor) :
    mConfiguration_(descriptor),
    mSendBufferSize(descriptor.sendBufferSize),
    mReceiveBufferSize(descriptor.receiveBufferSize)
{
    for (const auto& interface : descriptor.interfaceWhiteList)
        mInterfaceWhiteList.emplace_back(ip::address_v4::from_string(interface));
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor() :
    TransportDescriptorInterface(s_maximumMessageSize)
{
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor(const TCPv4TransportDescriptor& t) :
    TransportDescriptorInterface(t)
{
}

TransportInterface* TCPv4TransportDescriptor::create_transport() const
{
    return new TCPv4Transport(*this);
}

TCPv4Transport::TCPv4Transport()
    : mSendBufferSize(0)
    , mReceiveBufferSize(0)
    , mTCPMessageReceiver(nullptr)
{
}

TCPv4Transport::~TCPv4Transport()
{
    if (ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }
}

bool TCPv4Transport::init()
{
    if (mConfiguration_.sendBufferSize == 0 || mConfiguration_.receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::tcp::socket socket(mService);
        socket.open(ip::tcp::v4());

        if (mConfiguration_.sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            mConfiguration_.sendBufferSize = option.value();

            if (mConfiguration_.sendBufferSize < s_minimumSocketBuffer)
            {
                mConfiguration_.sendBufferSize = s_minimumSocketBuffer;
                mSendBufferSize = s_minimumSocketBuffer;
            }
        }

        if (mConfiguration_.receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            mConfiguration_.receiveBufferSize = option.value();

            if (mConfiguration_.receiveBufferSize < s_minimumSocketBuffer)
            {
                mConfiguration_.receiveBufferSize = s_minimumSocketBuffer;
                mReceiveBufferSize = s_minimumSocketBuffer;
            }
        }

        socket.close();
    }

    if (mConfiguration_.maxMessageSize > s_maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (mConfiguration_.maxMessageSize > mConfiguration_.sendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if (mConfiguration_.maxMessageSize > mConfiguration_.receiveBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than receiveBufferSize");
        return false;
    }

    mTCPMessageReceiver = new TCPMessageReceiver();

    // TODO(Ricardo) Create an event that update this list.
    GetIP4s(currentInterfaces);

    auto ioServiceFunction = [&]()
    {
        io_service::work work(mService);
        mService.run();
    };
    ioServiceThread.reset(new std::thread(ioServiceFunction));

    return true;
}

bool TCPv4Transport::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.get_physical_port()) != mInputSockets.end() ||
        (mPendingInputSockets.find(locator.get_physical_port()) != mPendingInputSockets.end()));
}

bool TCPv4Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
        return true;
    else
    {
        for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
        {
            if ((*it)->m_locator.compare_IP4_address_and_port(locator))
            {
                return true;
            }
        }
    }

    return false;
}

void TCPv4Transport::BindOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
    {
        return;
    }

    assert(mBoundOutputSockets.find(locator) == mBoundOutputSockets.end());
    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if ((*it)->m_locator.compare_IP4_address_and_port(locator))
        {
            mBoundOutputSockets[locator] = (*it);
        }
    }
}

bool TCPv4Transport::IsOutputChannelBound(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mBoundOutputSockets.find(locator) != mBoundOutputSockets.end();
}

bool TCPv4Transport::IsOutputChannelConnected(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if ((*it)->m_locator.compare_IP4_address_and_port(locator))
        {
            return true;
        }
    }
    return false;
}

bool TCPv4Transport::OpenOutputChannel(Locator_t& locator, SenderResource* senderResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;
    if (!IsOutputChannelConnected(locator))
    {
        if (!IsOutputChannelOpen(locator))
        {
            success = OpenAndBindOutputSockets(locator, senderResource);
        }
        else
        {
            BindOutputChannel(locator);
            success = EnqueueLogicalPort(locator);
        }
    }
    else
    {
        if (!IsOutputChannelBound(locator))
        {
            BindOutputChannel(locator);
            success = EnqueueLogicalPort(locator);
        }
        else
            success = true;
    }

    return success;
}

bool TCPv4Transport::OpenInputChannel(const Locator_t& locator, ReceiverResource* receiverResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    bool success = false;
    if (!IsInputChannelOpen(locator))
    {
        success = OpenAndBindInputSockets(locator, receiverResource);
    }

    return success;
}

bool TCPv4Transport::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (!IsOutputChannelOpen(locator))
        return false;

    mBoundOutputSockets.erase(locator);

    if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
    {
        mOutputSemaphores[locator.get_logical_port()]->disable();

        auto& pendingSocket = mPendingOutputSockets.at(locator);
        getSocketPtr(pendingSocket->m_socket)->close();

        mPendingOutputSockets.erase(locator);
    }

    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end();)
    {
        if ((*it)->m_locator == locator)
        {
            (*it)->getSocket()->cancel();
            (*it)->getSocket()->close();
            it = mOutputSockets.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return true;
}

bool TCPv4Transport::CloseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);

    if (IsOutputChannelBound(locator))
    {
        mBoundOutputSockets.erase(locator);
    }

    if (mPendingInputSockets.find(locator.get_physical_port()) != mPendingInputSockets.end())
    {
        mPendingInputSockets.erase(locator.get_physical_port());
        return true;
    }
    else if (IsInputChannelOpen(locator))
    {
        std::vector<std::shared_ptr<TCPSocketInfo>> sockets = mInputSockets.at(locator.get_physical_port());
        for (auto it = sockets.begin(); it != sockets.end(); ++it)
        {
            (*it)->getSocket()->cancel();
            (*it)->getSocket()->close();
        }

        mInputSockets.erase(locator.get_physical_port());
        return true;
    }
    return false;
}

bool TCPv4Transport::IsInterfaceAllowed(const ip::address_v4& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v4::any())
        return true;

    return find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

bool TCPv4Transport::EnqueueLogicalPort(Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if ((*it)->m_locator.compare_IP4_address_and_port(locator) && std::find((*it)->mLogicalPorts.begin(),
            (*it)->mLogicalPorts.end(), locator.get_logical_port()) == (*it)->mLogicalPorts.end()
            && std::find((*it)->mPendingLogicalPorts.begin(),
            (*it)->mPendingLogicalPorts.end(), locator.get_logical_port()) == (*it)->mPendingLogicalPorts.end())
        {
            (*it)->mPendingLogicalPorts.push_back(locator.get_logical_port());
            return true;
        }
    }
    return false;
}

bool TCPv4Transport::OpenAndBindOutputSockets(Locator_t& locator, SenderResource* senderResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    try
    {
        OpenAndBindUnicastOutputSocket(locator, senderResource);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPv4 Error binding at port: (" << locator.get_port() << ")" << " with msg: " << e.what());
        CloseOutputChannel(locator);
        return false;
    }

    return true;
}

bool TCPv4Transport::OpenAndBindInputSockets(const Locator_t& locator, ReceiverResource* receiverResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    try
    {
        std::shared_ptr<TCPAcceptor> newAcceptor = std::make_shared<TCPAcceptor>(mService,
            locator, receiverResource, mReceiveBufferSize);
        mPendingInputSockets.insert(std::make_pair(locator.get_physical_port(), newAcceptor));
        newAcceptor->Accept(this);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPv4 Error binding at port: (" << locator.get_physical_port() << ")" << " with msg: " << e.what());
        return false;
    }

    return true;
}

void TCPv4Transport::performListenOperation(std::shared_ptr<TCPSocketInfo> pSocketInfo)
{
    Locator_t remoteLocator;
    while (pSocketInfo->IsAlive())
    {
        if (pSocketInfo->mConnectionStatus == TCPSocketInfo::eConnectionStatus::eEstablished)
        {
            if (pSocketInfo->mPendingLogicalPorts.size() > 0)
            {
                //TODO: Send OpenLogicalPort Message
            }

            // Blocking receive.
            auto& msg = pSocketInfo->GetMessageReceiver()->m_rec_msg;
            CDRMessage::initCDRMsg(&msg);
            if (!Receive(msg.buffer, msg.max_size, msg.length, pSocketInfo.get(), remoteLocator))
                continue;

            if (mTCPMessageReceiver->CheckTCPControlMessage(pSocketInfo.get(), msg.buffer, msg.length))
            {
            }
            else
            {
                // Processes the data through the CDR Message interface.
                pSocketInfo->GetMessageReceiver()->processCDRMsg(mConfiguration_.rtpsParticipantGuidPrefix,
                    &pSocketInfo->m_locator, &pSocketInfo->GetMessageReceiver()->m_rec_msg);
            }
        }
        else
        {
            switch (pSocketInfo->mConnectionStatus)
            {
            default:
            case TCPSocketInfo::eConnectionStatus::eWaitingForBind:
                // DO NOTHING
                break;
            case TCPSocketInfo::eConnectionStatus::eConnected:
                pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eWaitingForBindResponse);
                //ARCE: Send Bind Message.
                break;
            }
        }
    }
}

void TCPv4Transport::OpenAndBindUnicastOutputSocket(Locator_t& locator, SenderResource* senderResource)
{
    mOutputSemaphores.emplace(locator.get_logical_port(), new Semaphore(0));

    TCPConnector* newConnector = new TCPConnector(mService, locator, senderResource, mSendBufferSize);
    mPendingOutputSockets[locator] = newConnector;
    newConnector->Connect(this);
}

void TCPv4Transport::OpenAndBindUnicastOutputSocket(Locator_t& locator, std::shared_ptr<MessageReceiver> messageReceiver)
{
    mOutputSemaphores.emplace(locator.get_logical_port(), new Semaphore(0));

    TCPConnector* newConnector = new TCPConnector(mService, locator, messageReceiver, mSendBufferSize);
    mPendingOutputSockets[locator] = newConnector;
    newConnector->Connect(this);
}

bool TCPv4Transport::DoLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.get_port() == right.get_port();
}

bool TCPv4Transport::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == LOCATOR_KIND_TCPv4;
}

void TCPv4Transport::AddDefaultLocator(LocatorList_t &/*defaultList*/)
{
    // On TCP, no default send locators.
    //defaultList.emplace_back(LOCATOR_KIND_TCPv4, 0);
}

Locator_t TCPv4Transport::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    Locator_t mainLocal(remote);
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

void TCPv4Transport::SetParticipantGUIDPrefix(const GuidPrefix_t& prefix)
{
    mConfiguration_.rtpsParticipantGuidPrefix = prefix;
}

bool TCPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
    const Locator_t& remoteLocator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    uint16_t logicalPort = remoteLocator.get_logical_port();
    if (!IsOutputChannelConnected(remoteLocator) || sendBufferSize > mConfiguration_.sendBufferSize)
        return false;


    // Resources semaphore
    if (mOutputSemaphores.find(logicalPort) == mOutputSemaphores.end())
    {
        eClock::my_sleep(1000);
        return false;
    }
    mOutputSemaphores.at(logicalPort)->wait();

    if (mBoundOutputSockets.find(remoteLocator) != mBoundOutputSockets.end())
    {
        CDRMessage_t msg;
        TCPHeader tcp_header;
        tcp_header.length = sendBufferSize + static_cast<uint32_t>(TCPHeader::GetSize());
        tcp_header.logicalPort = localLocator.get_logical_port();
        tcp_header.crc = 0; // TODO generate and fill CRC

        RTPSMessageCreator::addCustomContent(&msg, tcp_header.getAddress(), TCPHeader::GetSize());
        RTPSMessageCreator::addCustomContent(&msg, sendBuffer, sendBufferSize);

        bool success = false;
        std::shared_ptr<TCPSocketInfo> socket = mBoundOutputSockets.at(remoteLocator);
        std::unique_lock<std::recursive_mutex> sendLock(socket->GetMutex());
        success |= SendThroughSocket(msg.buffer, msg.length, remoteLocator, socket);

        return success;
    }
    return true;
}

static void EndpointToLocator(ip::tcp::endpoint& endpoint, Locator_t& locator)
{
    locator.kind = LOCATOR_KIND_TCPv4;
    locator.set_port(endpoint.port());
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    locator.set_IP4_address(ipBytes.data());
}

/**
    * On TCP, we must receive the header (14 Bytes) and then,
    * the rest of the message, whose length is on the header.
    * TCP Header is transparent to the caller, so receiveBuffer
    * doesn't include it.
    * */
bool TCPv4Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
    SocketInfo* socketInfo, Locator_t& remoteLocator)
{
    Semaphore receiveSemaphore(0);
    bool success = false;
    TCPSocketInfo* tcpSocketInfo = reinterpret_cast<TCPSocketInfo*>(socketInfo);

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (!socketInfo->IsAlive())
            return false;

        success = true;

        // Read the header
        tcpSocketInfo->getSocket()->set_option(asio::socket_base::receive_low_watermark(
            static_cast<int>(TCPHeader::GetSize())));

        //TCPHeader header;
        octet header[14];
        async_read(*tcpSocketInfo->getSocket(), asio::buffer(&header, TCPHeader::GetSize()),
            std::bind(&TCPv4Transport::DataReceived, this, header, receiveBuffer, receiveBufferCapacity,
                &receiveBufferSize, &receiveSemaphore, tcpSocketInfo, success, std::placeholders::_1,
                std::placeholders::_2));
    }
    receiveSemaphore.wait();
    success = success && receiveBufferSize > 0;

    if (success)
    {
        // ??
        ip::tcp::endpoint senderEndpoint;
        EndpointToLocator(senderEndpoint, remoteLocator);
    }
    else
    {
        uint16_t port = tcpSocketInfo->GetPhysicalPort();
        if (mInputSockets.find(port) != mInputSockets.end())
        {
            auto& sockets = mInputSockets.at(port);
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

bool TCPv4Transport::DataReceived(const octet* header, octet* receiveBuffer, uint32_t receiveBufferCapacity,
    uint32_t* receiveBufferSize, Semaphore* semaphore, TCPSocketInfo* pSocketInfo, bool& bSuccess,
    const asio::error_code& error, std::size_t bytes_transferred)
{
    uint32_t size(0);

    TCPHeader tcp_header;
    memcpy(&tcp_header, header, TCPHeader::GetSize()); // TODO Can avoid this memcpy?
    if (error)
    {
        logInfo(RTPS_MSG_IN, "Error while listening to socket (tcp header)...");
        size = 0;
        if (error == asio::error::eof || error == asio::error::connection_reset)
        {
            // Disable the socket to erase it after the reception.
            pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eDisconnected);
            pSocketInfo->Disable();
            bSuccess = false;
        }
    }
    else
    {
        logInfo(RTPS_MSG_IN, "TCP Header processed (" << bytes_transferred << " bytes received), \
				Socket async receive put again to listen ");
        if (bytes_transferred != TCPHeader::GetSize())
        {
            logError(RTPS_MSG_IN, "Bad TCP header size: " << bytes_transferred << "(expected: : " << TCPHeader::GetSize() << ")");
        }

        size = tcp_header.length - static_cast<uint32_t>(TCPHeader::GetSize());

        if (size > receiveBufferCapacity)
        {
            logError(RTPS_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                << size << " vs. " << receiveBufferCapacity << ".");
            bSuccess = false;
        }
        else
        {
            // Read the body
            pSocketInfo->getSocket()->set_option(asio::socket_base::receive_low_watermark(size));
            *receiveBufferSize = (uint32_t) pSocketInfo->getSocket()->read_some(asio::buffer(receiveBuffer, size));
            if (*receiveBufferSize != size)
            {
                bSuccess = false;
            }
        }
    }

    semaphore->post();
    return bSuccess;
}

bool TCPv4Transport::SendThroughSocket(const octet* sendBuffer,
    uint32_t sendBufferSize,
    const Locator_t& remoteLocator,
    std::shared_ptr<TCPSocketInfo> socket)
{

    asio::ip::address_v4::bytes_type remoteAddress;
    remoteLocator.copy_IP4_address(remoteAddress.data());
    auto destinationEndpoint = ip::tcp::endpoint(asio::ip::address_v4(remoteAddress),
        static_cast<uint16_t>(remoteLocator.get_port()));
    size_t bytesSent = 0;
    logInfo(RTPS_MSG_OUT, "TCPv4: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
        << " FROM " << socket.getSocket()->local_endpoint());

    try
    {
        bytesSent = socket->getSocket()->send(asio::buffer(sendBuffer, sendBufferSize));
    }
    catch (const asio::error_code& error)
    {
        if ((asio::error::eof == error) || (asio::error::connection_reset == error))
        {
            // Close the channel
            Locator_t prevLocator = socket->m_locator;
            std::shared_ptr<MessageReceiver> prevMsgReceiver = socket->GetMessageReceiver();
            CloseOutputChannel(socket->m_locator);

            // Create a new connector to retry the connection.
            OpenAndBindUnicastOutputSocket(prevLocator, prevMsgReceiver);
        }
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        // Close the channel
        Locator_t prevLocator = socket->m_locator;
        std::shared_ptr<MessageReceiver> prevMsgReceiver = socket->GetMessageReceiver();
        CloseOutputChannel(socket->m_locator);

        // Create a new connector to retry the connection.
        OpenAndBindUnicastOutputSocket(prevLocator, prevMsgReceiver);
        return false;
    }

    (void)bytesSent;
    logInfo(RTPS_MSG_OUT, "SENT " << bytesSent);
    return true;
}

LocatorList_t TCPv4Transport::NormalizeLocator(const Locator_t& locator)
{
    LocatorList_t list;

    if (locator.is_Any())
    {
        std::vector<IPFinder::info_IP> locNames;
        GetIP4s(locNames);
        for (const auto& infoIP : locNames)
        {
            Locator_t newloc(locator);
            newloc.set_IP4_address(infoIP.locator);
            list.push_back(newloc);
        }
    }
    else
        list.push_back(locator);

    return list;
}

LocatorList_t TCPv4Transport::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t unicastResult;
    for (auto& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        LocatorList_t pendingUnicast;

        while (it != locatorList.end())
        {
            assert((*it).kind == LOCATOR_KIND_TCPv4);

            // Check is local interface.
            auto localInterface = currentInterfaces.begin();
            for (; localInterface != currentInterfaces.end(); ++localInterface)
            {
                //if (memcmp(&localInterface->locator.address[12], &it->address[12], 4) == 0)
                if (localInterface->locator.compare_IP4_address(*it))
                {
                    // Loopback locator
                    Locator_t loopbackLocator;
                    loopbackLocator.set_IP4_address(127, 0, 0, 1);
                    loopbackLocator.set_port(it->get_physical_port());
                    pendingUnicast.push_back(loopbackLocator);
                    break;
                }
            }

            if (localInterface == currentInterfaces.end())
                pendingUnicast.push_back(*it);

            ++it;
        }

        unicastResult.push_back(pendingUnicast);
    }

    LocatorList_t result(std::move(unicastResult));
    return result;
}

bool TCPv4Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_TCPv4);

    if (locator.is_IP4_Local())
        return true;

    for (auto localInterface : currentInterfaces)
        if (locator.compare_IP4_address(localInterface.locator))
        {
            return true;
        }

    return false;
}

void TCPv4Transport::SocketAccepted(TCPAcceptor* acceptor, const asio::error_code& error, asio::ip::tcp::socket socket)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mPendingInputSockets.find(acceptor->m_locator.get_physical_port()) != mPendingInputSockets.end())
    {
        if (!error.value())
        {
            if (acceptor->m_receiveBufferSize != 0)
            {
                socket.set_option(asio::socket_base::receive_buffer_size(acceptor->m_receiveBufferSize));
            }

            // Store the new connection.
            eProsimaTCPSocket unicastSocket = eProsimaTCPSocket(std::move(socket));
            std::shared_ptr<TCPSocketInfo> socketInfo = std::make_shared<TCPSocketInfo>(unicastSocket, acceptor->m_locator);
            socketInfo->SetIsInputSocket(true);
            socketInfo->SetMessageReceiver(acceptor->m_acceptCallback());
            socketInfo->SetPhysicalPort(acceptor->m_locator.get_physical_port());
            std::thread* newThread = new std::thread(&TCPv4Transport::performListenOperation, this, socketInfo);
            socketInfo->SetThread(newThread);
            socketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eWaitingForBind);
            mInputSockets[acceptor->m_locator.get_physical_port()].emplace_back(socketInfo);
        }

        // Accept new connections for the same port.
        mPendingInputSockets.at(acceptor->m_locator.get_physical_port())->Accept(this);
    }
}

void TCPv4Transport::SocketConnected(Locator_t& locator, uint32_t sendBufferSize, const asio::error_code& error)
{
    std::string value = error.message();
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
    {
        auto& pendingConector = mPendingOutputSockets.at(locator);
        if (!error.value())
        {
            if (sendBufferSize != 0)
            {
                getSocketPtr(pendingConector->m_socket)->set_option(socket_base::send_buffer_size(sendBufferSize));
            }

            std::shared_ptr<TCPSocketInfo> outputSocket = std::make_shared<TCPSocketInfo>(pendingConector->m_socket, locator);
            outputSocket->SetIsInputSocket(false);
            if (pendingConector->m_messageReceiver != nullptr)
            {
                outputSocket->SetMessageReceiver(pendingConector->m_messageReceiver);
            }
            else
            {
                outputSocket->SetMessageReceiver(pendingConector->m_connectCallback(pendingConector->m_sendBufferSize));
            }
            std::thread* newThread = new std::thread(&TCPv4Transport::performListenOperation, this, outputSocket);
            outputSocket->SetThread(newThread);
            outputSocket->ChangeStatus(TCPSocketInfo::eConnectionStatus::eConnected);

            mOutputSockets.push_back(outputSocket);
            mOutputSemaphores[locator.get_logical_port()]->disable();
        }
        else
        {
            pendingConector->RetryConnect(mService, this);
        }
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
