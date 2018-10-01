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
#include <fastrtps/utils/Semaphore.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static void GetIP4s(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
            locNames.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;});
    locNames.erase(new_end, locNames.end());
}

static void GetIP4sUniqueInterfaces(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    GetIP4s(locNames, return_loopback);
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.dev < b.dev;});
    auto new_end = std::unique(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.type != IPFinder::IP4_LOCAL && b.type != IPFinder::IP4_LOCAL && a.dev == b.dev;});
    locNames.erase(new_end, locNames.end());
}

static bool IsAny(const Locator_t& locator)
{
    return locator.address[12] == 0 &&
        locator.address[13] == 0 &&
        locator.address[14] == 0 &&
        locator.address[15] == 0;
}

static asio::ip::address_v4::bytes_type locatorToNative(const Locator_t& locator)
{
    return {{locator.address[12],
        locator.address[13], locator.address[14], locator.address[15]}};
}

TCPv4Transport::TCPv4Transport(const TCPv4TransportDescriptor& descriptor):
    mConfiguration_(descriptor),
    mSendBufferSize(descriptor.sendBufferSize),
    mReceiveBufferSize(descriptor.receiveBufferSize)
    {
        for (const auto& interface : descriptor.interfaceWhiteList)
            mInterfaceWhiteList.emplace_back(ip::address_v4::from_string(interface));
    }

TCPv4TransportDescriptor::TCPv4TransportDescriptor():
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
    if(ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }
}

bool TCPv4Transport::init()
{
    if(mConfiguration_.sendBufferSize == 0 || mConfiguration_.receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::tcp::socket socket(mService);
        socket.open(ip::tcp::v4());

        if(mConfiguration_.sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            mConfiguration_.sendBufferSize = option.value();

            if(mConfiguration_.sendBufferSize < s_minimumSocketBuffer)
            {
                mConfiguration_.sendBufferSize = s_minimumSocketBuffer;
                mSendBufferSize = s_minimumSocketBuffer;
            }
        }

        if(mConfiguration_.receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            mConfiguration_.receiveBufferSize = option.value();

            if(mConfiguration_.receiveBufferSize < s_minimumSocketBuffer)
            {
                mConfiguration_.receiveBufferSize = s_minimumSocketBuffer;
                mReceiveBufferSize = s_minimumSocketBuffer;
            }
        }
    }

    if(mConfiguration_.maxMessageSize > s_maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if(mConfiguration_.maxMessageSize > mConfiguration_.sendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if(mConfiguration_.maxMessageSize > mConfiguration_.receiveBufferSize)
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
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.port) != mInputSockets.end());
}

bool TCPv4Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mOutputSockets.find(locator.port) != mOutputSockets.end();
}

bool TCPv4Transport::OpenOutputChannel(Locator_t& locator)
{
    if (IsOutputChannelOpen(locator) ||
            !IsLocatorSupported(locator))
        return false;

    return OpenAndBindOutputSockets(locator);
}

static bool IsMulticastAddress(const Locator_t& locator)
{
    return locator.address[12] >= 224 && locator.address[12] <= 239;
}

bool TCPv4Transport::OpenInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;

    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator.port, IsMulticastAddress(locator));

    if (IsMulticastAddress(locator) && IsInputChannelOpen(locator))
    {
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto& socket = mInputSockets.at(locator.port);

        std::vector<IPFinder::info_IP> locNames;
        GetIP4sUniqueInterfaces(locNames, true);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v4::from_string(infoIP.name);
            try
            {
#if defined(ASIO_HAS_MOVE)
                socket.set_option(ip::multicast::join_group(ip::address_v4::from_string(locator.to_IP4_string()), ip));
#else
                socket->set_option(ip::multicast::join_group(ip::address_v4::from_string(locator.to_IP4_string()), ip));
#endif
            }
            catch(std::system_error& ex)
            {
                (void)ex;
                logWarning(RTPS_MSG_OUT, "Error joining multicast group on " << ip << ": "<< ex.what());
            }
        }
    }

    return success;
}

bool TCPv4Transport::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(locator))
        return false;

    auto& sockets = mOutputSockets.at(locator.port);
    for (auto& socket : sockets)
    {
#if defined(ASIO_HAS_MOVE)
        socket.socket_.cancel();
        socket.socket_.close();
#else
        socket.socket_->cancel();
        socket.socket_->close();
#endif
    }

    mOutputSockets.erase(locator.port);

    return true;
}

bool TCPv4Transport::CloseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsInputChannelOpen(locator))
        return false;


    auto& socket = mInputSockets.at(locator.port);
#if defined(ASIO_HAS_MOVE)
    socket.cancel();
    socket.close();
#else
    socket->cancel();
    socket->close();
#endif

    mInputSockets.erase(locator.port);
    return true;
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
        if(IsAny(locator))
        {
            std::vector<IPFinder::info_IP> locNames;
            GetIP4s(locNames);
            // If there is no whitelist, we can simply open a generic output socket
            // and gain efficiency.
            if(mInterfaceWhiteList.empty())
            {
#if defined(ASIO_HAS_MOVE)
                asio::ip::tcp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip::address_v4::any(), locator.port);
                unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
#else
                std::shared_ptr<asio::ip::tcp::socket> unicastSocket = OpenAndBindUnicastOutputSocket(ip::address_v4::any(), locator.port);
                unicastSocket->set_option(ip::multicast::enable_loopback( true ) );
#endif

                // If more than one interface, then create sockets for outbounding multicast.
                if(locNames.size() > 1)
                {
                    auto locIt = locNames.begin();

                    // Outbounding first interface with already created socket.
#if defined(ASIO_HAS_MOVE)
                    unicastSocket.set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string((*locIt).name)));
#else
                    unicastSocket->set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string((*locIt).name)));
#endif
                    mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));

                    // Create other socket for outbounding rest of interfaces.
                    for(++locIt; locIt != locNames.end(); ++locIt)
                    {
                        auto ip = asio::ip::address_v4::from_string((*locIt).name);
                        uint32_t new_port = 0;
#if defined(ASIO_HAS_MOVE)
                        asio::ip::tcp::socket multicastSocket = OpenAndBindUnicastOutputSocket(ip, new_port);
                        multicastSocket.set_option(ip::multicast::outbound_interface(ip));
#else
                        std::shared_ptr<asio::ip::tcp::socket> multicastSocket = OpenAndBindUnicastOutputSocket(ip, new_port);
                        multicastSocket->set_option(ip::multicast::outbound_interface(ip));
#endif
                        SocketInfo mSocket(multicastSocket);
                        mSocket.only_multicast_purpose(true);
                        mOutputSockets[locator.port].push_back(std::move(mSocket));
                    }
                }
                else
                {
                    // Multicast data will be sent for the only one interface.
                    mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));
                }
            }
            else
            {
                bool firstInterface = false;
                for (const auto& infoIP : locNames)
                {
                    auto ip = asio::ip::address_v4::from_string(infoIP.name);
                    if (IsInterfaceAllowed(ip))
                    {
#if defined(ASIO_HAS_MOVE)
                        asio::ip::tcp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
                        unicastSocket.set_option(ip::multicast::outbound_interface(ip));
                        if(firstInterface)
                        {
                            unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
                            firstInterface = true;
                        }
#else
                        std::shared_ptr<asio::ip::tcp::socket> unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
                        unicastSocket->set_option(ip::multicast::outbound_interface(ip));
                        if(firstInterface)
                        {
                            unicastSocket->set_option(ip::multicast::enable_loopback( true ) );
                            firstInterface = true;
                        }
#endif
                        mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));
                    }
                }
            }
        }
        else
        {
            auto ip = asio::ip::address_v4(locatorToNative(locator));
#if defined(ASIO_HAS_MOVE)
            asio::ip::tcp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
            unicastSocket.set_option(ip::multicast::outbound_interface(ip));
            unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
#else
            std::shared_ptr<asio::ip::tcp::socket> unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
            unicastSocket->set_option(ip::multicast::outbound_interface(ip));
            unicastSocket->set_option(ip::multicast::enable_loopback( true ) );
#endif
            mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPv4 Error binding at port: (" << locator.port << ")" << " with msg: "<<e.what());
        mOutputSockets.erase(locator.port);
        return false;
    }

    return true;
}

bool TCPv4Transport::OpenAndBindInputSockets(uint32_t port, bool is_multicast)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        mInputSockets.emplace(port, OpenAndBindInputSocket(port, is_multicast));
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "TCPv4 Error binding at port: (" << port << ")" << " with msg: "<<e.what());
        mInputSockets.erase(port);
        return false;
    }

    return true;
}

#if defined(ASIO_HAS_MOVE)
asio::ip::tcp::socket TCPv4Transport::OpenAndBindUnicastOutputSocket(const ip::address_v4& ipAddress, uint32_t& port)
{
    ip::tcp::socket socket(mService);
    socket.open(ip::tcp::v4());
    if(mSendBufferSize != 0)
        socket.set_option(socket_base::send_buffer_size(mSendBufferSize));
    socket.set_option(ip::multicast::hops(mConfiguration_.TTL));

    ip::tcp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket.bind(endpoint);

    if(port == 0)
        port = socket.local_endpoint().port();

    return socket;
}
#else
std::shared_ptr<asio::ip::tcp::socket> TCPv4Transport::OpenAndBindUnicastOutputSocket(const ip::address_v4& ipAddress, uint32_t& port)
{
    std::shared_ptr<ip::tcp::socket> socket = std::make_shared<ip::tcp::socket>(mService);
    socket->open(ip::tcp::v4());
    if(mSendBufferSize != 0)
        socket->set_option(socket_base::send_buffer_size(mSendBufferSize));
    socket->set_option(ip::multicast::hops(mConfiguration_.TTL));

    ip::tcp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket->bind(endpoint);

    if(port == 0)
        port = socket->local_endpoint().port();

    return socket;
}
#endif

#if defined(ASIO_HAS_MOVE)
asio::ip::tcp::socket TCPv4Transport::OpenAndBindInputSocket(uint32_t port, bool is_multicast)
{
    ip::tcp::socket socket(mService);
    socket.open(ip::tcp::v4());
    if(mReceiveBufferSize != 0)
        socket.set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    if(is_multicast)
        socket.set_option(ip::tcp::socket::reuse_address( true ) );
    ip::tcp::endpoint endpoint(ip::address_v4::any(), static_cast<uint16_t>(port));
    socket.bind(endpoint);

    return socket;
}
#else
std::shared_ptr<asio::ip::tcp::socket> TCPv4Transport::OpenAndBindInputSocket(uint32_t port, bool is_multicast)
{
    std::shared_ptr<ip::tcp::socket> socket = std::make_shared<ip::tcp::socket>(mService);
    socket->open(ip::tcp::v4());
    if(mReceiveBufferSize != 0)
        socket->set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    if(is_multicast)
        socket->set_option(ip::tcp::socket::reuse_address( true ) );
    ip::tcp::endpoint endpoint(ip::address_v4::any(), static_cast<uint16_t>(port));
    socket->bind(endpoint);

    return socket;
}
#endif

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

bool TCPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(localLocator) ||
            sendBufferSize > mConfiguration_.sendBufferSize)
        return false;

    bool success = false;
    bool is_multicast_remote_address = IsMulticastAddress(remoteLocator);

    auto& sockets = mOutputSockets.at(localLocator.port);
    for (auto& socket : sockets)
    {
        if(is_multicast_remote_address || !socket.only_multicast_purpose())
            success |= SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, socket.socket_);
    }

    return success;
}

static void EndpointToLocator(ip::tcp::endpoint& endpoint, Locator_t& locator)
{
    locator.port = endpoint.port();
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    memcpy(&locator.address[12], ipBytes.data(), sizeof(ipBytes));
}

bool TCPv4Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
        const Locator_t& localLocator, Locator_t& remoteLocator)
{
    if (!IsInputChannelOpen(localLocator))
        return false;

    Semaphore receiveSemaphore(0);
    bool success = false;

    auto handler = [&receiveBuffer, &receiveBufferSize, &success, &receiveSemaphore]
        (const asio::error_code& error, std::size_t bytes_transferred)
        {
            (void)receiveBuffer;

            if(error)
            {
                logInfo(RTPS_MSG_IN, "Error while listening to socket...");
                receiveBufferSize = 0;
            }
            else
            {
                logInfo(RTPS_MSG_IN,"Msg processed (" << bytes_transferred << " bytes received), Socket async receive put again to listen ");
                receiveBufferSize = static_cast<uint32_t>(bytes_transferred);
                success = true;
            }

            receiveSemaphore.post();
        };

    ip::tcp::endpoint senderEndpoint;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!IsInputChannelOpen(localLocator))
            return false;

        auto& socket = mInputSockets.at(localLocator.port);
#if defined(ASIO_HAS_MOVE)
        socket.async_receive(asio::buffer(receiveBuffer, receiveBufferCapacity),
                0,
                handler);
#else
        socket->async_receive(asio::buffer(receiveBuffer, receiveBufferCapacity),
                0,
                handler);
#endif
    }

    receiveSemaphore.wait();
    if (success)
        EndpointToLocator(senderEndpoint, remoteLocator);

    return success;
}

bool TCPv4Transport::SendThroughSocket(const octet* sendBuffer,
        uint32_t sendBufferSize,
        const Locator_t& remoteLocator,
#if defined(ASIO_HAS_MOVE)
        asio::ip::tcp::socket& socket)
#else
        std::shared_ptr<asio::ip::tcp::socket> socket)
#endif
{

    asio::ip::address_v4::bytes_type remoteAddress;
    memcpy(&remoteAddress, &remoteLocator.address[12], sizeof(remoteAddress));
    auto destinationEndpoint = ip::tcp::endpoint(asio::ip::address_v4(remoteAddress), static_cast<uint16_t>(remoteLocator.port));
    size_t bytesSent = 0;
#if defined(ASIO_HAS_MOVE)
    logInfo(RTPS_MSG_OUT,"TCPv4: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << socket.local_endpoint());
#else
    logInfo(RTPS_MSG_OUT,"TCPv4: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << socket->local_endpoint());
#endif

    try
    {
#if defined(ASIO_HAS_MOVE)
        bytesSent = socket.send(asio::buffer(sendBuffer, sendBufferSize));
#else
        bytesSent = socket->send(asio::buffer(sendBuffer, sendBufferSize));
#endif
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    (void) bytesSent;
    logInfo (RTPS_MSG_OUT,"SENT " << bytesSent);
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

struct MultiUniLocatorsLinkage
{
    MultiUniLocatorsLinkage(LocatorList_t&& m, LocatorList_t&& u) :
        multicast(std::move(m)), unicast(std::move(u)) {}

    LocatorList_t multicast;
    LocatorList_t unicast;
};

LocatorList_t TCPv4Transport::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t multicastResult, unicastResult;
    std::vector<MultiUniLocatorsLinkage> pendingLocators;

    for(auto& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        bool multicastDefined = false;
        LocatorList_t pendingMulticast, pendingUnicast;

        while(it != locatorList.end())
        {
            assert((*it).kind == LOCATOR_KIND_TCPv4);

            if(IsMulticastAddress(*it))
            {
                // If the multicast locator is already chosen, not choose any unicast locator.
                if(multicastResult.contains(*it))
                {
                    multicastDefined = true;
                    pendingUnicast.clear();
                }
                else
                {
                    // Search the multicast locator in pending locators.
                    auto pending_it = pendingLocators.begin();
                    bool found = false;

                    while(pending_it != pendingLocators.end())
                    {
                        if((*pending_it).multicast.contains(*it))
                        {
                            // Multicast locator was found, add it to final locators.
                            multicastResult.push_back((*pending_it).multicast);
                            pendingLocators.erase(pending_it);

                            // Not choose any unicast
                            multicastDefined = true;
                            pendingUnicast.clear();
                            found = true;

                            break;
                        }

                        ++pending_it;
                    };

                    // If not found, store as pending multicast.
                    if(!found)
                        pendingMulticast.push_back(*it);
                }
            }
            else
            {
                if(!multicastDefined)
                {
                    // Check is local interface.
                    auto localInterface = currentInterfaces.begin();
                    for (; localInterface != currentInterfaces.end(); ++localInterface)
                    {
                        if(memcmp(&localInterface->locator.address[12], &it->address[12], 4) == 0)
                        {
                            // Loopback locator
                            Locator_t loopbackLocator;
                            loopbackLocator.set_IP4_address(127, 0, 0, 1);
                            loopbackLocator.port = it->port;
                            pendingUnicast.push_back(loopbackLocator);
                            break;
                        }
                    }

                    if(localInterface == currentInterfaces.end())
                        pendingUnicast.push_back(*it);
                }
            }

            ++it;
        }

        if(pendingMulticast.size() == 0)
        {
            unicastResult.push_back(pendingUnicast);
        }
        else if(pendingUnicast.size() == 0)
        {
            multicastResult.push_back(pendingMulticast);
        }
        else
        {
            pendingLocators.push_back(MultiUniLocatorsLinkage(std::move(pendingMulticast), std::move(pendingUnicast)));
        }
    }

    LocatorList_t result(std::move(unicastResult));
    result.push_back(multicastResult);

    // Store pending unicast locators
    for(auto link : pendingLocators)
        result.push_back(link.unicast);

    return result;
}

bool TCPv4Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_TCPv4);

    if(locator.address[12] == 127 &&
            locator.address[13] == 0 &&
            locator.address[14] == 0 &&
            locator.address[15] == 1)
        return true;

    for(auto localInterface : currentInterfaces)
        if(locator.address[12] == localInterface.locator.address[12] &&
            locator.address[13] == localInterface.locator.address[13] &&
            locator.address[14] == localInterface.locator.address[14] &&
            locator.address[15] == localInterface.locator.address[15])
        {
            return true;
        }

    return false;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
