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
#include "asio.hpp"
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static const int s_default_keep_alive_frequency = 50000; // 50 SECONDS
static const int s_default_keep_alive_timeout = 10000; // 10 SECONDS

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
    , keep_alive_frequency_ms(s_default_keep_alive_frequency)
    , keep_alive_timeout_ms(s_default_keep_alive_timeout)
{
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor(const TCPv4TransportDescriptor& t) :
    TransportDescriptorInterface(t)
    , keep_alive_frequency_ms(t.keep_alive_frequency_ms)
    , keep_alive_timeout_ms(t.keep_alive_timeout_ms)
{
}

TransportInterface* TCPv4TransportDescriptor::create_transport() const
{
    return new TCPv4Transport(*this);
}

TCPv4Transport::TCPv4Transport()
    : mSendBufferSize(0)
    , mReceiveBufferSize(0)
    , mActive(true)
    , mRTCPMessageManager(nullptr)
{
}

TCPv4Transport::~TCPv4Transport()
{
    std::vector<std::shared_ptr<TCPSocketInfo>> vDeletedSockets;

    // Collect all the existing sockets to delete them outside of the mutex.
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        mActive = false;
        mSocketsAcceptors.clear();
        for (auto it = mPendingOutputSockets.begin(); it != mPendingOutputSockets.end(); ++it)
        {
            delete(it->second);
        }
        mPendingOutputSockets.clear();

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
        ReleaseTCPSocket(*it);
        (*it) = nullptr;
    }

    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        for (auto it = mThreadPool.begin(); it != mThreadPool.end(); ++it)
        {
            (*it)->join();
            delete(*it);
        }
        mThreadPool.clear();
    }

    if (ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }
    delete mRTCPMessageManager;
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

    mRTCPMessageManager = new RTCPMessageManager(this);

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
        (mSocketsAcceptors.find(locator.get_physical_port()) != mSocketsAcceptors.end()));
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

void TCPv4Transport::UnbindInputSocket(std::shared_ptr<SocketInfo> pSocket)
{
    for (auto it = mBoundOutputSockets.begin(); it != mBoundOutputSockets.end();)
    {
        if (it->second == pSocket)
        {
            it = mBoundOutputSockets.erase(it);
        }
        else
        {
            ++it;
        }
    }
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
            success = EnqueueLogicalOutputPort(locator);
        }
    }
    else
    {
        if (!IsOutputChannelBound(locator))
        {
            BindOutputChannel(locator);
            success = EnqueueLogicalOutputPort(locator);
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
    else
    {
        success = EnqueueLogicalInputPort(locator);
    }

    return success;
}

bool TCPv4Transport::CloseOutputChannel(const Locator_t& locator)
{
    std::vector<std::shared_ptr<TCPSocketInfo>> vDeletedSockets;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (!IsOutputChannelOpen(locator))
            return false;

        mBoundOutputSockets.erase(locator);

        if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
        {
            auto& pendingSocket = mPendingOutputSockets.at(locator);
            getSocketPtr(pendingSocket->m_socket)->close();
            delete(mPendingOutputSockets[locator]);
            mPendingOutputSockets.erase(locator);
        }

        for (auto it = mOutputSockets.begin(); it != mOutputSockets.end();)
        {
            if ((*it)->m_locator == locator)
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
        ReleaseTCPSocket(*it);
        (*it) = nullptr;
    }
    vDeletedSockets.clear();

    return true;
}

bool TCPv4Transport::CloseInputChannel(const Locator_t& locator)
{
    bool bClosed = false;
    std::vector<std::shared_ptr<TCPSocketInfo>> vDeletedSockets;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);

        if (mSocketsAcceptors.find(locator.get_physical_port()) != mSocketsAcceptors.end())
        {
            mSocketsAcceptors.erase(locator.get_physical_port());
            bClosed = true;
        }

        if (mInputSockets.find(locator.get_physical_port()) != mInputSockets.end())
        {
            std::vector<std::shared_ptr<TCPSocketInfo>> sockets = mInputSockets.at(locator.get_physical_port());
            for (auto it = sockets.begin(); it != sockets.end(); ++it)
            {
                vDeletedSockets.emplace_back(*it);
            }

            mInputSockets.erase(locator.get_physical_port());
            bClosed = true;
        }
    }

    for (auto it = vDeletedSockets.begin(); it != vDeletedSockets.end(); ++it)
    {
        UnbindInputSocket(*it);
        ReleaseTCPSocket(*it);
        *it = nullptr;
    }

    return bClosed;
}

bool TCPv4Transport::IsInterfaceAllowed(const ip::address_v4& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v4::any())
        return true;

    return find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

bool TCPv4Transport::EnqueueLogicalOutputPort(Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    for (auto it = mOutputSockets.begin(); it != mOutputSockets.end(); ++it)
    {
        if ((*it)->m_locator.compare_IP4_address_and_port(locator)
                && std::find((*it)->mLogicalOutputPorts.begin(),
            (*it)->mLogicalOutputPorts.end(), locator.get_logical_port())
                == (*it)->mLogicalOutputPorts.end()
            && std::find((*it)->mPendingLogicalOutputPorts.begin(),
            (*it)->mPendingLogicalOutputPorts.end(), locator.get_logical_port())
                == (*it)->mPendingLogicalOutputPorts.end())
        {
            (*it)->mPendingLogicalOutputPorts.push_back(locator.get_logical_port());
            return true;
        }
    }
    return false;
}

bool TCPv4Transport::EnqueueLogicalInputPort(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mInputSockets.find(locator.get_physical_port()) != mInputSockets.end())
    {
        for (auto it = mInputSockets.at(locator.get_physical_port()).begin();
                it != mInputSockets.at(locator.get_physical_port()).end(); ++it)
        {
            if ((*it)->m_locator.compare_IP4_address_and_port(locator)
                    && std::find((*it)->mLogicalInputPorts.begin(),
                (*it)->mLogicalInputPorts.end(), locator.get_logical_port())
                    == (*it)->mLogicalOutputPorts.end())
            {
                (*it)->mLogicalInputPorts.push_back(locator.get_logical_port());
                return true;
            }
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
        mSocketsAcceptors.insert(std::make_pair(locator.get_physical_port(), newAcceptor));
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

void TCPv4Transport::performRTPCManagementThread(std::shared_ptr<TCPSocketInfo> pSocketInfo)
{
    std::chrono::time_point<std::chrono::system_clock> time_now;
    std::chrono::time_point<std::chrono::system_clock> next_time;
    std::chrono::time_point<std::chrono::system_clock> timeout_time;

    while (pSocketInfo->IsAlive())
    {
        time_now = std::chrono::system_clock::now();
        if (pSocketInfo->IsConnectionEstablished())
        {
            if (pSocketInfo->mPendingLogicalPort == 0 && !pSocketInfo->mPendingLogicalOutputPorts.empty())
            {
                pSocketInfo->mPendingLogicalPort = *pSocketInfo->mPendingLogicalOutputPorts.begin();
                mRTCPMessageManager->sendOpenLogicalPortRequest(pSocketInfo, pSocketInfo->mPendingLogicalPort);
            }
            else if (mConfiguration_.keep_alive_frequency_ms > 0 && mConfiguration_.keep_alive_timeout_ms > 0)
            {
                // Keep Alive Management
                if (!pSocketInfo->mWaitingForKeepAlive && time_now > next_time)
                {
                    mRTCPMessageManager->sendKeepAliveRequest(pSocketInfo);
                    pSocketInfo->mWaitingForKeepAlive = true;
                    next_time = time_now + std::chrono::milliseconds(mConfiguration_.keep_alive_frequency_ms);
                    timeout_time = time_now + std::chrono::milliseconds(mConfiguration_.keep_alive_timeout_ms);
                }
                else if (pSocketInfo->mWaitingForKeepAlive && time_now >= timeout_time)
                {
                    // Disable the socket to erase it after the reception.
                    pSocketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eDisconnected);
                    pSocketInfo->Disable();
                    continue;
                }
            }
        }
    }
    pSocketInfo = nullptr;
}

void TCPv4Transport::performListenOperation(std::shared_ptr<TCPSocketInfo> pSocketInfo)
{
    Locator_t remoteLocator;

    while (pSocketInfo->IsAlive())
    {
        // Blocking receive.
        auto& msg = pSocketInfo->GetMessageReceiver()->m_rec_msg;
        CDRMessage::initCDRMsg(&msg);
        //Locator_t remoteLocator;
        if (!Receive(pSocketInfo, msg.buffer, msg.max_size, msg.length))/*remoteLocator*/
            continue;

        // Processes the data through the CDR Message interface.
        pSocketInfo->GetMessageReceiver()->processCDRMsg(mConfiguration_.rtpsParticipantGuidPrefix,
            &pSocketInfo->m_locator, &pSocketInfo->GetMessageReceiver()->m_rec_msg);
    }
    pSocketInfo = nullptr;
}

void TCPv4Transport::OpenAndBindUnicastOutputSocket(Locator_t& locator, SenderResource* senderResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mActive)
    {
        TCPConnector* newConnector = new TCPConnector(mService, locator, senderResource, mSendBufferSize);
        if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
        {
            TCPConnector* oldConnector = mPendingOutputSockets.at(locator);
            delete oldConnector;
        }
        mPendingOutputSockets[locator] = newConnector;
        newConnector->Connect(this);
    }
}

void TCPv4Transport::OpenAndBindUnicastOutputSocket(Locator_t& locator, std::shared_ptr<MessageReceiver> messageReceiver)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mActive)
    {
        TCPConnector* newConnector = new TCPConnector(mService, locator, messageReceiver, mSendBufferSize);
        if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
        {
            TCPConnector* oldConnector = mPendingOutputSockets.at(locator);
            delete oldConnector;
        }
        mPendingOutputSockets[locator] = newConnector;
        newConnector->Connect(this);
    }
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

bool TCPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& /*localLocator*/,
    const Locator_t& remoteLocator)
{
    std::shared_ptr<TCPSocketInfo> socket = nullptr;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        if (!IsOutputChannelConnected(remoteLocator) || sendBufferSize > mConfiguration_.sendBufferSize)
            return false;

        if (mBoundOutputSockets.find(remoteLocator) != mBoundOutputSockets.end())
        {
            socket = mBoundOutputSockets.at(remoteLocator);
        }
    }

    if (socket != nullptr)
    {
        CDRMessage_t msg;
        TCPHeader tcp_header;
        tcp_header.length = sendBufferSize + static_cast<uint32_t>(TCPHeader::GetSize());
        tcp_header.logicalPort = remoteLocator.get_logical_port();
        RTCPMessageManager::CalculateCRC(tcp_header, sendBuffer, sendBufferSize);

        RTPSMessageCreator::addCustomContent(&msg, tcp_header.getAddress(), TCPHeader::GetSize());
        RTPSMessageCreator::addCustomContent(&msg, sendBuffer, sendBufferSize);

        bool success = false;
        std::unique_lock<std::recursive_mutex> sendLock(socket->GetWriteMutex());
        success |= SendThroughSocket(msg.buffer, msg.length, remoteLocator, socket);

        return success;
    }
    else
    {
        eClock::my_sleep(1000);
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
bool TCPv4Transport::Receive(std::shared_ptr<TCPSocketInfo> socketInfo, octet* receiveBuffer,
    uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize)
{
    Semaphore receiveSemaphore(0);
    bool success = false;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(socketInfo->GetReadMutex());
        if (socketInfo->IsAlive())
        {
            return false;
        }

        success = true;
        try
        {
            // Read the header
            octet header[14];
            size_t bytes_received = read(*socketInfo->getSocket(),
                asio::buffer(&header, TCPHeader::GetSize()), transfer_exactly(14));
            TCPHeader tcp_header;
            memcpy(&tcp_header, header, TCPHeader::GetSize()); // TODO Can avoid this memcpy?
            if (bytes_received != TCPHeader::GetSize())
            {
                logError(RTPS_MSG_IN, "Bad TCP header size: " << bytes_received << "(expected: : " << TCPHeader::GetSize() << ")");
            }

            size_t body_size = tcp_header.length - static_cast<uint32_t>(TCPHeader::GetSize());

            if (body_size > receiveBufferCapacity)
            {
                logError(RTPS_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                    << body_size << " vs. " << receiveBufferCapacity << ".");
                success = false;
            }
            else
            {
                success = ReadBody(receiveBuffer, receiveBufferCapacity, &receiveBufferSize, socketInfo, body_size);

                if (!RTCPMessageManager::CheckCRC(tcp_header, receiveBuffer, receiveBufferSize))
                {
                    logWarning(RTPS_MSG_IN, "Bad TCP header CRC");
                }

                if (tcp_header.logicalPort == 0)
                {
                    mRTCPMessageManager->processRTCPMessage(socketInfo, receiveBuffer);
                    success = false;
                }
            }
            receiveSemaphore.post();
        }
        catch (const asio::error_code& code)
        {
            if ((code == asio::error::eof) || (code == asio::error::connection_reset))
            {
                // Close the channel
                socketInfo->Disable();
                CloseTCPSocket(socketInfo);
            }
            receiveSemaphore.post();
        }
        catch (const asio::system_error& /*error*/)
        {
            // Close the channel
            socketInfo->Disable();
            CloseTCPSocket(socketInfo);
            receiveSemaphore.post();
        }
    }
    receiveSemaphore.wait();
    success = success && receiveBufferSize > 0;

    if (!success)
    {
        uint16_t port = socketInfo->GetPhysicalPort();
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
    else
    {
        //ip::tcp::endpoint senderEndpoint = socketInfo->getSocket()->remote_endpoint();
        //EndpointToLocator(senderEndpoint, remoteLocator);
    }

    return success;
}

bool TCPv4Transport::ReadBody(octet* receiveBuffer, uint32_t receiveBufferCapacity,
    uint32_t* bytes_received, std::shared_ptr<TCPSocketInfo> pSocketInfo,
    std::size_t body_size)
{
    *bytes_received = static_cast<uint32_t>(read(*pSocketInfo->getSocket(),
        asio::buffer(receiveBuffer, receiveBufferCapacity), transfer_exactly(body_size)));

    if (*bytes_received != body_size)
    {
        logError(RTPS_MSG_IN, "Bad TCP body size: " << bytes_received << "(expected: " << TCPHeader::GetSize() << ")");
        return false;
    }

    return true;
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
        << " FROM " << socket->getSocket()->local_endpoint());

    eSocketErrorCodes errorCode;
    bytesSent = Send(socket, sendBuffer, sendBufferSize, errorCode);
    switch(errorCode)
    {
        case eNoError:
            break;
        default:
            // Close the channel
            socket->Disable();
            Locator_t prevLocator = socket->m_locator;
            std::shared_ptr<MessageReceiver> prevMsgReceiver = socket->GetMessageReceiver();
            CloseOutputChannel(socket->m_locator);

            // Create a new connector to retry the connection.
            OpenAndBindUnicastOutputSocket(prevLocator, prevMsgReceiver);
        break;
    }

    logInfo(RTPS_MSG_OUT, "SENT " << bytesSent);
    return bytesSent > 0;
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
    if (mSocketsAcceptors.find(acceptor->m_locator.get_physical_port()) != mSocketsAcceptors.end())
    {
        if (!error.value())
        {
            // Store the new connection.
            eProsimaTCPSocket unicastSocket = eProsimaTCPSocket(std::move(socket));
            std::shared_ptr<TCPSocketInfo> socketInfo = std::make_shared<TCPSocketInfo>(unicastSocket,
                acceptor->m_locator, false, true, false);
            socketInfo->SetMessageReceiver(acceptor->m_acceptCallback());
            socketInfo->SetPhysicalPort(acceptor->m_locator.get_physical_port());
            socketInfo->SetThread(new std::thread(&TCPv4Transport::performListenOperation, this, socketInfo));
            socketInfo->SetRTCPThread(new std::thread(&TCPv4Transport::performRTPCManagementThread, this, socketInfo));
            socketInfo->ChangeStatus(TCPSocketInfo::eConnectionStatus::eWaitingForBind);
            mInputSockets[acceptor->m_locator.get_physical_port()].emplace_back(socketInfo);
        }

        // Accept new connections for the same port.
        mSocketsAcceptors.at(acceptor->m_locator.get_physical_port())->Accept(this);
    }
}

void TCPv4Transport::SocketConnected(Locator_t& locator, uint32_t /*sendBufferSize*/, const asio::error_code& error)
{
    std::string value = error.message();
    std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
    if (mPendingOutputSockets.find(locator) != mPendingOutputSockets.end())
    {
        auto& pendingConector = mPendingOutputSockets.at(locator);
        if (!error.value())
        {
            std::shared_ptr<TCPSocketInfo> outputSocket = std::make_shared<TCPSocketInfo>(pendingConector->m_socket,
                locator, true, false, false);

            if (pendingConector->m_messageReceiver != nullptr)
            {
                outputSocket->SetMessageReceiver(pendingConector->m_messageReceiver);
            }
            else
            {
                outputSocket->SetMessageReceiver(pendingConector->m_connectCallback(pendingConector->m_sendBufferSize));
            }
            outputSocket->SetThread(new std::thread(&TCPv4Transport::performListenOperation, this, outputSocket));
            outputSocket->SetRTCPThread(new std::thread(&TCPv4Transport::performRTPCManagementThread, this, outputSocket));
            outputSocket->ChangeStatus(TCPSocketInfo::eConnectionStatus::eConnected);
            mOutputSockets.push_back(outputSocket);

            // RTCP Control Message
            mRTCPMessageManager->sendConnectionRequest(outputSocket);
        }
        else
        {
            eClock::my_sleep(100);
            pendingConector->RetryConnect(mService, this);
        }
    }
}

size_t TCPv4Transport::Send(std::shared_ptr<TCPSocketInfo> socketInfo, const octet *data,
        size_t size, eSocketErrorCodes &errorCode) const
{
    size_t bytesSent = 0;
    try
    {
        std::unique_lock<std::recursive_mutex> scopedLock(socketInfo->GetWriteMutex());
        bytesSent = socketInfo->getSocket()->send(asio::buffer(data, size));
        errorCode = eSocketErrorCodes::eNoError;
    }
    catch (const asio::error_code& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.message());
        if ((asio::error::eof == error) || (asio::error::connection_reset == error))
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
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        errorCode = eSocketErrorCodes::eSystemError;
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        errorCode = eSocketErrorCodes::eException;
    }

    return bytesSent;
}

size_t TCPv4Transport::Send(std::shared_ptr<TCPSocketInfo> socketInfo, const octet *data, size_t size) const
{
    eSocketErrorCodes error;
    return Send(socketInfo, data, size, error);
}

void TCPv4Transport::CloseTCPSocket(std::shared_ptr<TCPSocketInfo> socketInfo)
{
    if (socketInfo->GetIsInputSocket())
    {
        CloseInputChannel(socketInfo->m_locator);
    }
    else
    {
        Locator_t prevLocator = socketInfo->m_locator;
        std::shared_ptr<MessageReceiver> prevMsgReceiver = socketInfo->GetMessageReceiver();
        CloseOutputChannel(socketInfo->m_locator);

        // Create a new connector to retry the connection.
        OpenAndBindUnicastOutputSocket(prevLocator, prevMsgReceiver);
    }
}

void TCPv4Transport::ReleaseTCPSocket(std::shared_ptr<TCPSocketInfo> socketInfo)
{
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        for (auto it = mThreadPool.begin(); it != mThreadPool.end(); ++it)
        {
            (*it)->join();
            delete(*it);
        }
        mThreadPool.clear();
    }

    socketInfo->Disable();
    socketInfo->getSocket()->cancel();
    socketInfo->getSocket()->close();

    {
        std::unique_lock<std::recursive_mutex> scopedLock(mSocketsMapMutex);
        mThreadPool.emplace_back(socketInfo->ReleaseThread());
        mThreadPool.emplace_back(socketInfo->ReleaseRTCPThread());
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
