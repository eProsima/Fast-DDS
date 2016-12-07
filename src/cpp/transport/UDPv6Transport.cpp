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

#include <fastrtps/transport/UDPv6Transport.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const uint32_t maximumUDPSocketSize = 65536;
static const uint32_t maximumMessageSize = 65500;
static const uint8_t defaultTTL = 1;

static void GetIP6s(vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    // Controller out IP4
    auto newEnd = remove_if(locNames.begin(),
            locNames.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL;});
    locNames.erase(newEnd, locNames.end());
}

static bool IsAny(const Locator_t& locator)
{
    return locator.address[0] == 0 &&
        locator.address[1] == 0 &&
        locator.address[2] == 0 &&
        locator.address[3] == 0 &&
        locator.address[4] == 0 &&
        locator.address[5] == 0 &&
        locator.address[6] == 0 &&
        locator.address[7] == 0 &&
        locator.address[8] == 0 &&
        locator.address[9] == 0 &&
        locator.address[10] == 0 &&
        locator.address[11] == 0 &&
        locator.address[12] == 0 &&
        locator.address[13] == 0 &&
        locator.address[14] == 0 &&
        locator.address[15] == 0;
}

static asio::ip::address_v6::bytes_type locatorToNative(const Locator_t& locator)
{
    return {{locator.address[0],
        locator.address[1], locator.address[2], locator.address[3],
        locator.address[4], locator.address[5], locator.address[6],
        locator.address[7], locator.address[8], locator.address[9],
        locator.address[10], locator.address[11],locator.address[12],
        locator.address[13], locator.address[14], locator.address[15]}};
}

UDPv6Transport::UDPv6Transport(const UDPv6TransportDescriptor& descriptor):
    mMaxMessageSize(descriptor.maxMessageSize),
    mSendBufferSize(descriptor.sendBufferSize),
    mReceiveBufferSize(descriptor.receiveBufferSize),
    mTTL(descriptor.TTL)
    {
        for (const auto& interface : descriptor.interfaceWhiteList)
           mInterfaceWhiteList.emplace_back(ip::address_v6::from_string(interface));
    }

UDPv6TransportDescriptor::UDPv6TransportDescriptor():
    TransportDescriptorInterface(maximumMessageSize),
    sendBufferSize(maximumUDPSocketSize),
    receiveBufferSize(maximumUDPSocketSize),
    TTL(defaultTTL)
    {}

UDPv6Transport::~UDPv6Transport()
{
    if(ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }
}

bool UDPv6Transport::init()
{
    if(mMaxMessageSize > maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if(mMaxMessageSize > mSendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if(mMaxMessageSize > mReceiveBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than receiveBufferSize");
        return false;
    }

    auto ioServiceFunction = [&]()
    {
        io_service::work work(mService);
        mService.run();
    };
    ioServiceThread.reset(new std::thread(ioServiceFunction));

    return true;
}

bool UDPv6Transport::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.port) != mInputSockets.end());
}

bool UDPv6Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mOutputSockets.find(locator.port) != mOutputSockets.end();
}

bool UDPv6Transport::OpenOutputChannel(Locator_t& locator)
{
    if (IsOutputChannelOpen(locator) ||
            !IsLocatorSupported(locator))
        return false;

    return OpenAndBindOutputSockets(locator);
}

static bool IsMulticastAddress(const Locator_t& locator)
{
    return locator.address[0] == 0xFF;
}

bool UDPv6Transport::OpenInputChannel(const Locator_t& locator)
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
        GetIP6s(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v6::from_string(infoIP.name);
#if defined(ASIO_HAS_MOVE)
            socket.set_option(ip::multicast::join_group(ip::address_v6::from_string(locator.to_IP6_string()), ip.scope_id()));
#else
            socket->set_option(ip::multicast::join_group(ip::address_v6::from_string(locator.to_IP6_string()), ip.scope_id()));
#endif
        }
    }

    return success;
}

bool UDPv6Transport::CloseOutputChannel(const Locator_t& locator)
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

bool UDPv6Transport::CloseInputChannel(const Locator_t& locator)
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

bool UDPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v6::any())
        return true;

    return  find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}


bool UDPv6Transport::OpenAndBindOutputSockets(Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);

    try
    {
        if(IsAny(locator))
        {
            std::vector<IPFinder::info_IP> locNames;
            GetIP6s(locNames);
            // If there is no whitelist, we can simply open a generic output socket
            // and gain efficiency.
            if(mInterfaceWhiteList.empty())
            {
#if defined(ASIO_HAS_MOVE)
                asio::ip::udp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip::address_v6::any(), locator.port);
                unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
#else
                std::shared_ptr<asio::ip::udp::socket> unicastSocket = OpenAndBindUnicastOutputSocket(ip::address_v6::any(), locator.port);
                unicastSocket->set_option(ip::multicast::enable_loopback( true ) );
#endif

                // If more than one interface, then create sockets for outbounding multicast.
                if(locNames.size() > 1)
                {
                    auto locIt = locNames.begin();

                    // Outbounding first interface with already created socket.
#if defined(ASIO_HAS_MOVE)
                    unicastSocket.set_option(ip::multicast::outbound_interface(asio::ip::address_v6::from_string((*locIt).name).scope_id()));
#else
                    unicastSocket->set_option(ip::multicast::outbound_interface(asio::ip::address_v6::from_string((*locIt).name).scope_id()));
#endif
                    mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));

                    // Create other socket for outbounding rest of interfaces.
                    for(++locIt; locIt != locNames.end(); ++locIt)
                    {
                        auto ip = asio::ip::address_v6::from_string((*locIt).name);
                        uint32_t new_port = 0;
#if defined(ASIO_HAS_MOVE)
                        asio::ip::udp::socket multicastSocket = OpenAndBindUnicastOutputSocket(ip, new_port);
                        multicastSocket.set_option(ip::multicast::outbound_interface(ip.scope_id()));
#else
                        std::shared_ptr<asio::ip::udp::socket> multicastSocket = OpenAndBindUnicastOutputSocket(ip, new_port);
                        multicastSocket->set_option(ip::multicast::outbound_interface(ip.scope_id()));
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
                    auto ip = asio::ip::address_v6::from_string(infoIP.name);
                    if (IsInterfaceAllowed(ip))
                    {
#if defined(ASIO_HAS_MOVE)
                        asio::ip::udp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
                        unicastSocket.set_option(ip::multicast::outbound_interface(ip.scope_id()));
                        if(firstInterface)
                        {
                            unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
                            firstInterface = true;
                        }
#else
                        std::shared_ptr<asio::ip::udp::socket> unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
                        unicastSocket->set_option(ip::multicast::outbound_interface(ip.scope_id()));
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
            auto ip = asio::ip::address_v6(locatorToNative(locator));
#if defined(ASIO_HAS_MOVE)
            asio::ip::udp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
            unicastSocket.set_option(ip::multicast::outbound_interface(ip.scope_id()));
            unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
#else
            std::shared_ptr<asio::ip::udp::socket> unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
            unicastSocket->set_option(ip::multicast::outbound_interface(ip.scope_id()));
            unicastSocket->set_option(ip::multicast::enable_loopback( true ) );
#endif
            mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << locator.port << ")" << " with msg: "<<e.what());
        mOutputSockets.erase(locator.port);
        return false;
    }

    return true;
}

bool UDPv6Transport::OpenAndBindInputSockets(uint32_t port, bool is_multicast)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        mInputSockets.emplace(port, OpenAndBindInputSocket(port, is_multicast));
    }
    catch (asio::error_code const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << port << ")" << " with msg: "<<e.message() );
        mInputSockets.erase(port);
        return false;
    }

    return true;
}

#if defined(ASIO_HAS_MOVE)
asio::ip::udp::socket UDPv6Transport::OpenAndBindUnicastOutputSocket(const ip::address_v6& ipAddress, uint32_t& port)
{
    ip::udp::socket socket(mService);
    socket.open(ip::udp::v6());
    socket.set_option(socket_base::send_buffer_size(mSendBufferSize));
    socket.set_option(ip::multicast::hops(mTTL));

    ip::udp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket.bind(endpoint);

    if(port == 0)
        port = socket.local_endpoint().port();

    return socket;
}
#else
std::shared_ptr<asio::ip::udp::socket> UDPv6Transport::OpenAndBindUnicastOutputSocket(const ip::address_v6& ipAddress, uint32_t& port)
{
    std::shared_ptr<ip::udp::socket> socket = std::make_shared<ip::udp::socket>(mService);
    socket->open(ip::udp::v6());
    socket->set_option(socket_base::send_buffer_size(mSendBufferSize));

    ip::udp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket->bind(endpoint);

    if(port == 0)
        port = socket->local_endpoint().port();

    return socket;
}
#endif

#if defined(ASIO_HAS_MOVE)
asio::ip::udp::socket UDPv6Transport::OpenAndBindInputSocket(uint32_t port, bool is_multicast)
{
    ip::udp::socket socket(mService);
    socket.open(ip::udp::v6());
    socket.set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    if(is_multicast)
        socket.set_option(ip::udp::socket::reuse_address( true ) );
    ip::udp::endpoint endpoint(ip::address_v6::any(), static_cast<uint16_t>(port));
    socket.bind(endpoint);

    return socket;
}
#else
std::shared_ptr<asio::ip::udp::socket> UDPv6Transport::OpenAndBindInputSocket(uint32_t port, bool is_multicast)
{
    std::shared_ptr<ip::udp::socket> socket = std::make_shared<ip::udp::socket>(mService);
    socket->open(ip::udp::v6());
    socket->set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    if(is_multicast)
        socket->set_option(ip::udp::socket::reuse_address( true ) );
    ip::udp::endpoint endpoint(ip::address_v6::any(), static_cast<uint16_t>(port));
    socket->bind(endpoint);

    return socket;
}
#endif

bool UDPv6Transport::DoLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.port == right.port;
}

bool UDPv6Transport::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == LOCATOR_KIND_UDPv6;
}

Locator_t UDPv6Transport::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    // All remotes are equally mapped to from the local [0:0:0:0:0:0:0:0]:port (main output channel).
    Locator_t mainLocal(remote);
    memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    return remote;
}

bool UDPv6Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(localLocator) ||
            sendBufferSize > mSendBufferSize)
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

static Locator_t EndpointToLocator(ip::udp::endpoint& endpoint)
{
    Locator_t locator;

    locator.port = endpoint.port();
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    memcpy(&locator.address[0], ipBytes.data(), sizeof(ipBytes));

    return locator;
}

bool UDPv6Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
        const Locator_t& localLocator, Locator_t& remoteLocator)
{

    if (!IsInputChannelOpen(localLocator) ||
            receiveBufferCapacity < mReceiveBufferSize)
        return false;

    Semaphore receiveSemaphore(0);
    bool success = false;
    auto handler = [&receiveBuffer, &receiveBufferSize, &success, &receiveSemaphore]
      (const asio::error_code& error, std::size_t bytes_transferred)
    {
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

    ip::udp::endpoint senderEndpoint;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

        auto& socket = mInputSockets.at(localLocator.port);
#if defined(ASIO_HAS_MOVE)
        socket.async_receive_from(asio::buffer(receiveBuffer, receiveBufferCapacity),
                senderEndpoint,
                handler);
#else
        socket->async_receive_from(asio::buffer(receiveBuffer, receiveBufferCapacity),
                senderEndpoint,
                handler);
#endif
    }

    receiveSemaphore.wait();
    if (success)
        remoteLocator = EndpointToLocator(senderEndpoint);

    return success;
}

bool UDPv6Transport::SendThroughSocket(const octet* sendBuffer,
        uint32_t sendBufferSize,
        const Locator_t& remoteLocator,
#if defined(ASIO_HAS_MOVE)
        asio::ip::udp::socket& socket)
#else
        std::shared_ptr<asio::ip::udp::socket> socket)
#endif
{

    asio::ip::address_v6::bytes_type remoteAddress;
    memcpy(&remoteAddress, &remoteLocator.address[0], sizeof(remoteAddress));
    auto destinationEndpoint = ip::udp::endpoint(asio::ip::address_v6(remoteAddress), static_cast<uint16_t>(remoteLocator.port));
    size_t bytesSent = 0;
#if defined(ASIO_HAS_MOVE)
    logInfo(RTPS_MSG_OUT,"UDPv6: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << socket.local_endpoint());
#else
    logInfo(RTPS_MSG_OUT,"UDPv6: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << socket->local_endpoint());
#endif

    try
    {
#if defined(ASIO_HAS_MOVE)
        bytesSent = socket.send_to(asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
#else
        bytesSent = socket->send_to(asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
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

LocatorList_t UDPv6Transport::NormalizeLocator(const Locator_t& locator)
{
	LocatorList_t list;

	if (locator.address[0] == 0x0 && locator.address[1] == 0x0 &&
		locator.address[2] == 0x0 && locator.address[3] == 0x0 &&
		locator.address[4] == 0x0 && locator.address[5] == 0x0 &&
		locator.address[6] == 0x0 && locator.address[7] == 0x0 &&
		locator.address[8] == 0x0 && locator.address[9] == 0x0 &&
		locator.address[10] == 0x0 && locator.address[11] == 0x0 &&
		locator.address[12] == 0x0 && locator.address[13] == 0x0 &&
		locator.address[14] == 0x0 && locator.address[15] == 0x0)
	{
		std::vector<IPFinder::info_IP> locNames;
		GetIP6s(locNames);
		for (const auto& infoIP : locNames)
		{
			Locator_t newloc(infoIP.locator);
			newloc.kind = locator.kind;
			newloc.port = locator.port;
			list.push_back(newloc);
		}
	}
	else
		list.push_back(locator);

	return list;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
