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
#include <fastrtps/utils/eClock.h>
#include "asio.hpp"

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

static asio::ip::address_v4::bytes_type locatorToNative(const Locator_t& locator)
{
    return{ {locator.address[12],
        locator.address[13], locator.address[14], locator.address[15]} };
}

TCPAcceptor::TCPAcceptor(asio::io_service& io_service, uint16_t port, uint32_t receiveBufferSize)
    : m_acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    , m_port(port)
    , m_receiveBufferSize(receiveBufferSize)
{
}

void TCPAcceptor::Accept(TCPv4Transport* parent)
{
    m_acceptor.async_accept(std::bind(&TCPv4Transport::SocketAccepted, parent, m_port, m_receiveBufferSize,
        std::placeholders::_1, std::placeholders::_2));
}

#if defined(ASIO_HAS_MOVE)
TCPConnector::TCPConnector(asio::io_service& io_service,
    const ip::address_v4& ipAddress,
    uint16_t tcp_port,
    uint16_t logical_port,
    uint32_t sendBufferSize)
    : m_tcp_port(tcp_port)
    , m_logical_port(logical_port)
    , m_ipAddress(ipAddress)
    , m_sendBufferSize(sendBufferSize)
    , m_socket(createTCPSocket(io_service))
{
}

void TCPConnector::Connect(TCPv4Transport* parent)
{
    getSocketPtr(m_socket)->open(ip::tcp::v4());
    ip::tcp::endpoint endpoint(m_ipAddress, static_cast<uint16_t>(m_tcp_port));
    getSocketPtr(m_socket)->async_connect(endpoint, std::bind(&TCPv4Transport::SocketConnected, parent, m_logical_port,
        m_sendBufferSize, std::placeholders::_1));
}

void TCPConnector::RetryConnect(asio::io_service& io_service, TCPv4Transport* parent)
{
    getSocketPtr(m_socket)->close();
    m_socket = createTCPSocket(io_service);
    Connect(parent);
}

#else
TCPConnector::TCPConnector(asio::io_service& io_service,
    const ip::address_v4& ipAddress,
    uint16_t tcp_port,
    uint16_t logical_port,
    uint32_t sendBufferSize)
    : m_tcp_port(tcp_port)
    , m_logical_port(logical_port)
    , m_ipAddress(ipAddress)
    , m_sendBufferSize(sendBufferSize)
    , m_socket(createTCPSocket(io_service))
{
}

void TCPConnector::Connect(TCPv4Transport* parent)
{
    getSocketPtr(m_socket)->open(ip::tcp::v4());
    ip::tcp::endpoint endpoint(m_ipAddress, static_cast<uint16_t>(m_tcp_port));
    getSocketPtr(m_socket)->async_connect(endpoint, std::bind(&TCPv4Transport::SocketConnected, parent, m_logical_port,
        m_sendBufferSize, std::placeholders::_1));
}

void TCPConnector::RetryConnect(asio::io_service& io_service, TCPv4Transport* parent)
{
    getSocketPtr(m_socket)->close();
    m_socket = createTCPSocket(io_service);
    Connect(parent);
}
#endif

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

TCPv4Transport::TCPv4Transport() :
    mSendBufferSize(0),
    mReceiveBufferSize(0)
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
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.port) != mInputSockets.end() ||
        (mPendingInputSockets.find(locator.port) != mPendingInputSockets.end()));
}

bool TCPv4Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mOutputSockets.find(locator.get_Logical_port()) != mOutputSockets.end() ||
        mPendingOutputSockets.find(locator.get_Logical_port()) != mPendingOutputSockets.end();
}

bool TCPv4Transport::OpenOutputChannel(Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;
    if (!IsOutputChannelOpen(locator))
        success = OpenAndBindOutputSockets(locator);

    return success;
}

bool TCPv4Transport::OpenInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;
    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator.get_TCP_port());

    return success;
}

bool TCPv4Transport::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(locator))
        return false;

    if (mPendingOutputSockets.find(locator.port) != mPendingOutputSockets.end())
    {
        mOutputSemaphores[locator.port]->disable();

        auto& pendingSocket = mPendingOutputSockets.at(locator.port);
        getSocketPtr(pendingSocket->m_socket)->close();

        mPendingOutputSockets.erase(locator.port);
    }

    if (mOutputSockets.find(locator.port) != mOutputSockets.end())
    {
        auto& sockets = mOutputSockets.at(locator.port);
        for (auto& socket : sockets)
        {
            socket.getSocket()->cancel();
            socket.getSocket()->close();
        }
        mOutputSockets.erase(locator.port);
    }

    return true;
}

bool TCPv4Transport::CloseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (mPendingInputSockets.find(locator.port) != mPendingInputSockets.end())
    {
        mPendingInputSockets.erase(locator.port);
        return true;
    }
    else if (IsInputChannelOpen(locator))
    {
        auto& sockets = mInputSockets.at(locator.port);
        for (auto& socket : sockets)
        {
            getSocketPtr(socket)->cancel();
            getSocketPtr(socket)->close();
        }

        mInputSockets.erase(locator.port);
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

bool TCPv4Transport::OpenAndBindOutputSockets(Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    try
    {
        auto ip = asio::ip::address_v4(locatorToNative(locator));
        OpenAndBindUnicastOutputSocket(ip, locator.get_Logical_port(), locator.get_TCP_port());
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPv4 Error binding at port: (" << locator.port << ")" << " with msg: " << e.what());
        CloseOutputChannel(locator);
        return false;
    }

    return true;
}

bool TCPv4Transport::OpenAndBindInputSockets(uint32_t tcp_port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    try
    {
        std::shared_ptr<TCPAcceptor> newAccepter = std::make_shared<TCPAcceptor>(mService, static_cast<uint16_t>(tcp_port),
            mReceiveBufferSize);
        mPendingInputSockets.insert(std::make_pair(tcp_port, newAccepter));
        newAccepter->Accept(this);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPv4 Error binding at port: (" << tcp_port << ")" << " with msg: " << e.what());
        return false;
    }

    return true;
}

void TCPv4Transport::OpenAndBindUnicastOutputSocket(const ip::address_v4& ipAddress,
    uint16_t logical_port,
    uint16_t tcp_port)
{
    mOutputSemaphores.emplace(logical_port, new Semaphore(0));

    TCPConnector* newConnector = new TCPConnector(mService, ipAddress, logical_port, tcp_port, mSendBufferSize);
    mPendingOutputSockets[logical_port] = newConnector;
    newConnector->Connect(this);
}

bool TCPv4Transport::DoLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.port == right.port;
}

bool TCPv4Transport::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == LOCATOR_KIND_TCPv4;
}

Locator_t TCPv4Transport::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    Locator_t mainLocal(remote);
    memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    return mainLocal;
}

static void fillTcpHeader(octet* header, uint32_t size, const Locator_t& loc)
{
    header[0] = 'R';
    header[1] = 'T';
    header[2] = 'P';
    header[3] = 'S';
    size += 14;
    octet* s = (octet*)&size;
    header[4] = s[0];
    header[5] = s[1];
    header[6] = s[2];
    header[7] = s[3];
    uint16_t port = loc.get_Logical_port();
    octet* p = (octet*)&port;
    header[12] = p[0];
    header[13] = p[1];
}

/*
static void showCDRMessage(CDRMessage_t* msg)
{
    std::cout << "MSG: ";
    for (uint32_t i = 0; i < msg->length; ++i)
    {
        std::cout << std::hex << static_cast<int>(msg->buffer[i]) << " ";
    }
    std::cout << std::endl;
}
*/

bool TCPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
    const Locator_t& remoteLocator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    uint16_t logicalPort = localLocator.get_Logical_port();
    if (!IsOutputChannelOpen(logicalPort) || sendBufferSize > mConfiguration_.sendBufferSize)
        return false;


    // Resources semaphore
    if (mOutputSemaphores.find(logicalPort) == mOutputSemaphores.end())
    {
        eClock::my_sleep(1000);
        return false;
    }
    mOutputSemaphores.at(logicalPort)->wait();

    if (mOutputSockets.find(logicalPort) != mOutputSockets.end())
    {
        CDRMessage_t msg;
        octet tcp_header[14];
        fillTcpHeader(tcp_header, sendBufferSize, localLocator);
        // TODO generate and fill CRC
        //uint32_t crc = ...
        // octet *crcp = (octet*)&crc;
        // tcp_header[8] = crcp[0];
        // ...
        // tcp_header[11] = crcp[3];
        for (int i = 0; i < 4; ++i)
        {
            tcp_header[8 + i] = 0x00; // CRC to 0
        }

        RTPSMessageCreator::addCustomContent(&msg, tcp_header, sizeof(tcp_header));
        RTPSMessageCreator::addCustomContent(&msg, sendBuffer, sendBufferSize);

        bool success = false;
        auto& sockets = mOutputSockets.at(localLocator.port);
        for (auto& socket : sockets)
        {
            //showCDRMessage(&msg);
            std::unique_lock<std::recursive_mutex> sendLock(socket.GetMutex());
            success |= SendThroughSocket(msg.buffer, msg.length, remoteLocator, getRefFromPtr(socket.getSocket()));
        }
        return success;
    }
    return true;
}

static void EndpointToLocator(ip::tcp::endpoint& endpoint, Locator_t& locator)
{
    locator.port = endpoint.port();
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    memcpy(&locator.address[12], ipBytes.data(), sizeof(ipBytes));
}

static uint32_t getSize(const octet *header)
{
    uint32_t size;
    octet* ps = (octet*)&size;
    ps[0] = header[4];
    ps[1] = header[5];
    ps[2] = header[6];
    ps[3] = header[7];
    return size;
}

// TODO uncomment when needed
/*
static uint32_t getCRC(const octet *header)
{
    uint32_t crc;
    octet* ps = (octet*)&crc;
    ps[0] = header[8];
    ps[1] = header[9];
    ps[2] = header[10];
    ps[3] = header[11];
    return crc;
}

static uint16_t getRTPSPort(const octet *header)
{
    uint16_t port;
    octet* ps = (octet*)&port;
    ps[0] = header[12];
    ps[1] = header[13];
    return port;
}
*/

/**
    * On TCP, we must receive the header (14 Bytes) and then,
    * the rest of the message, whose length is on the header.
    * TCP Header is transparent to the caller, so receiveBuffer
    * doesn't include it.
    * */
bool TCPv4Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
    const Locator_t& localLocator, Locator_t& remoteLocator)
{
    if (!IsInputChannelOpen(localLocator))
        return false;

    octet header[14];
    uint32_t size;// = getSize(header) - 14;

    Semaphore receiveSemaphore(0);
    bool success = false;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!IsInputChannelOpen(localLocator))
            return false;

        if (mInputSockets.find(localLocator.port) != mInputSockets.end())
        {
            auto& sockets = mInputSockets.at(localLocator.port);
            for (auto& socket : sockets)
            {
                auto handler = [&header, &success, &size, &receiveBuffer, &receiveBufferSize, &receiveBufferCapacity,
                    &receiveSemaphore, &socket]
                    (const asio::error_code& error, std::size_t bytes_transferred)
                {
                    if (error)
                    {
                        logInfo(RTPS_MSG_IN, "Error while listening to socket (tcp header)...");
                        size = 0;

                        if (error == asio::error::eof || error == asio::error::connection_reset)
                        {
                            //TODO: Delete input socket.

                        }
                    }
                    else
                    {
                        logInfo(RTPS_MSG_IN, "TCP Header processed (" << bytes_transferred << " bytes received), \
				Socket async receive put again to listen ");
                        if (bytes_transferred != 14)
                        {
                            logError(RTPS_MSG_IN, "Bad TCP header size: " << bytes_transferred);
                        }

                        size = getSize(header) - 14;

                        if (size > receiveBufferCapacity)
                        {
                            logError(RTPS_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                                << size << " vs. " << receiveBufferCapacity << ".");
                        }
                        else
                        {
                            // Read the body
                            receiveBufferSize = (uint32_t)getSocketPtr(socket)->receive(asio::buffer(receiveBuffer, size));
                            if (receiveBufferSize == size)
                            {
                                success = true;
                            }
                        }
                    }

                    receiveSemaphore.post();
                };

                // Read the header
                getSocketPtr(socket)->async_receive(asio::buffer(header, 14),
                    0,
                    handler);
                    //std::bind(&TCPv4Transport::ReceiveDataCB, this, std::placeholders::_1, std::placeholders::_2));
            }
        }
    }
    receiveSemaphore.wait();

    if (success)
    {
        // ??
        ip::tcp::endpoint senderEndpoint;
        EndpointToLocator(senderEndpoint, remoteLocator);
    }

    return success;
}

bool TCPv4Transport::ReceiveDataCB(const asio::error_code& error, std::size_t bytes_transferred)
{
    //bool success(false);

    //if (error)
    //{
    //    logInfo(RTPS_MSG_IN, "Error while listening to socket (tcp header)...");
    //    size = 0;

    //    if (error == asio::error::eof || error == asio::error::connection_reset)
    //    {
    //        //TODO: Delete input socket.

    //    }
    //}
    //else
    //{
    //    logInfo(RTPS_MSG_IN, "TCP Header processed (" << bytes_transferred << " bytes received), \
				//Socket async receive put again to listen ");
    //    if (bytes_transferred != 14)
    //    {
    //        logError(RTPS_MSG_IN, "Bad TCP header size: " << bytes_transferred);
    //    }

    //    size = getSize(header) - 14;

    //    if (size > receiveBufferCapacity)
    //    {
    //        logError(RTPS_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
    //            << size << " vs. " << receiveBufferCapacity << ".");
    //    }
    //    else
    //    {
    //        // Read the body
    //        receiveBufferSize = (uint32_t)getSocketPtr(socket)->receive(asio::buffer(receiveBuffer, size));
    //        if (receiveBufferSize == size)
    //        {
    //            success = true;
    //        }
    //    }
    //}
    //return success;
    return false;
}

bool TCPv4Transport::SendThroughSocket(const octet* sendBuffer,
    uint32_t sendBufferSize,
    const Locator_t& remoteLocator,
    eProsimaTCPSocket& socket)
{

    asio::ip::address_v4::bytes_type remoteAddress;
    memcpy(&remoteAddress, &remoteLocator.address[12], sizeof(remoteAddress));
    auto destinationEndpoint = ip::tcp::endpoint(asio::ip::address_v4(remoteAddress),
        static_cast<uint16_t>(remoteLocator.port));
    size_t bytesSent = 0;
    logInfo(RTPS_MSG_OUT, "TCPv4: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
        << " FROM " << getSocketPtr(socket)->local_endpoint());

    try
    {
        bytesSent = getSocketPtr(socket)->send(asio::buffer(sendBuffer, sendBufferSize));
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    (void)bytesSent;
    logInfo(RTPS_MSG_OUT, "SENT " << bytesSent);
    return true;
}

LocatorList_t TCPv4Transport::NormalizeLocator(const Locator_t& locator)
{
    LocatorList_t list;

    if (locator.address[12] == 0x0 && locator.address[13] == 0x0 &&
        locator.address[14] == 0x0 && locator.address[15] == 0x0)
    {
        std::vector<IPFinder::info_IP> locNames;
        GetIP4s(locNames);
        for (const auto& infoIP : locNames)
        {
            Locator_t newloc(locator);
            newloc.set_IP4_address(infoIP.locator.address[12], infoIP.locator.address[13],
                infoIP.locator.address[14], infoIP.locator.address[15]);
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
                if (memcmp(&localInterface->locator.address[12], &it->address[12], 4) == 0)
                {
                    // Loopback locator
                    Locator_t loopbackLocator;
                    loopbackLocator.set_IP4_address(127, 0, 0, 1);
                    loopbackLocator.port = it->port;
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

    if (locator.address[12] == 127 &&
        locator.address[13] == 0 &&
        locator.address[14] == 0 &&
        locator.address[15] == 1)
        return true;

    for (auto localInterface : currentInterfaces)
        if (locator.address[12] == localInterface.locator.address[12] &&
            locator.address[13] == localInterface.locator.address[13] &&
            locator.address[14] == localInterface.locator.address[14] &&
            locator.address[15] == localInterface.locator.address[15])
        {
            return true;
        }

    return false;
}

void TCPv4Transport::SocketAccepted(uint32_t port, uint32_t receiveBufferSize, const asio::error_code& error,
    asio::ip::tcp::socket socket)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (mPendingInputSockets.find(port) != mPendingInputSockets.end())
    {
        if (!error.value())
        {
            if (receiveBufferSize != 0)
                getSocketPtr(socket)->set_option(asio::socket_base::receive_buffer_size(receiveBufferSize));

            if (receiveBufferSize != 0)
            {
                socket.set_option(asio::socket_base::receive_buffer_size(receiveBufferSize));
            }

            mInputSockets[port].emplace_back(moveSocket(socket));
        }

        mPendingInputSockets.at(port)->Accept(this);
    }
}

void TCPv4Transport::SocketConnected(uint32_t logical_port, uint32_t sendBufferSize, const asio::error_code& error)
{
    std::string value = error.message();
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (mPendingOutputSockets.find(logical_port) != mPendingOutputSockets.end())
    {
        auto& pendingConector = mPendingOutputSockets.at(logical_port);
        if (!error.value())
        {
            if (sendBufferSize != 0)
            {
                getSocketPtr(pendingConector->m_socket)->set_option(socket_base::send_buffer_size(sendBufferSize));
            }

            mOutputSockets[logical_port].push_back(TCPSocketInfo(pendingConector->m_socket));
            mOutputSemaphores[logical_port]->disable();
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
