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

#include <fastrtps/transport/UDPv4Transport.h>
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

static const uint32_t maximumMessageSize = 65500;
static const uint32_t minimumSocketBuffer = 65536;
static const uint8_t defaultTTL = 1;

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

UDPv4Transport::UDPv4Transport(const UDPv4TransportDescriptor& descriptor)
    : mConfiguration_(descriptor)
    , mSendBufferSize(descriptor.sendBufferSize)
    , mReceiveBufferSize(descriptor.receiveBufferSize)
    , mWhiteListOutput(descriptor.whiteListOutput)
    , mWhiteListInput(descriptor.whiteListInput)
    , mWhiteListLocators(descriptor.whiteListLocators)
{
    for (const auto& networkInterface : descriptor.interfaceWhiteList)
        mInterfaceWhiteList.emplace_back(ip::address_v4::from_string(networkInterface));
}

UDPv4TransportDescriptor::UDPv4TransportDescriptor():
    TransportDescriptorInterface(maximumMessageSize),
    sendBufferSize(0),
    receiveBufferSize(0),
    whiteListOutput(true),
    whiteListInput(false),
    whiteListLocators(false),
    TTL(defaultTTL)
{
}

UDPv4TransportDescriptor::UDPv4TransportDescriptor(const UDPv4TransportDescriptor& t) :
    TransportDescriptorInterface(t),
    sendBufferSize(t.sendBufferSize),
    receiveBufferSize(t.receiveBufferSize),
    interfaceWhiteList(t.interfaceWhiteList),
    whiteListOutput(t.whiteListOutput),
    whiteListInput(t.whiteListInput),
    whiteListLocators(t.whiteListLocators),
    TTL(t.TTL)
{
}

UDPv4Transport::UDPv4Transport() :
    mSendBufferSize(0),
    mReceiveBufferSize(0),
    mWhiteListOutput(true),
    mWhiteListInput(false),
    mWhiteListLocators(false)
    {
    }

UDPv4Transport::~UDPv4Transport()
{
  if(ioServiceThread)
  {
    mService.stop();
    ioServiceThread->join();
  }
}

bool UDPv4Transport::init()
{
    if(mConfiguration_.sendBufferSize == 0 || mConfiguration_.receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::udp::socket socket(mService);
        socket.open(ip::udp::v4());

        if(mConfiguration_.sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            mConfiguration_.sendBufferSize = option.value();

            if(mConfiguration_.sendBufferSize < minimumSocketBuffer)
            {
                mConfiguration_.sendBufferSize = minimumSocketBuffer;
                mSendBufferSize = minimumSocketBuffer;
            }
        }

        if(mConfiguration_.receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            mConfiguration_.receiveBufferSize = option.value();

            if(mConfiguration_.receiveBufferSize < minimumSocketBuffer)
            {
                mConfiguration_.receiveBufferSize = minimumSocketBuffer;
                mReceiveBufferSize = minimumSocketBuffer;
            }
        }
    }

    if(mConfiguration_.maxMessageSize > maximumMessageSize)
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

bool UDPv4Transport::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.port) != mInputSockets.end());
}

bool UDPv4Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mOutputSockets.find(locator.port) != mOutputSockets.end();
}

bool UDPv4Transport::OpenOutputChannel(Locator_t& locator)
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

bool UDPv4Transport::OpenInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;

    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator);

    if (IsMulticastAddress(locator) && IsInputChannelOpen(locator))
    {
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto& sockets = mInputSockets.at(locator.port);
        for (auto& socket : sockets)
        {
          std::vector<IPFinder::info_IP> locNames;
          GetIP4sUniqueInterfaces(locNames, true);
          for (const auto& infoIP : locNames)
          {
              auto ip = asio::ip::address_v4::from_string(infoIP.name);
              if (!mWhiteListInput || (mWhiteListInput && IsInterfaceAllowed(ip)))
              {
                 try
                 {
                    socket.set_option(ip::multicast::join_group(ip::address_v4::from_string(locator.to_IP4_string()), ip));
                 }
                 catch (std::system_error& ex)
                 {
                    (void)ex;
                    logWarning(RTPS_MSG_OUT, "Error joining multicast group on " << ip << ": "<< ex.what());
                 }
              }
          }
        }
    }

    return success;
}

bool UDPv4Transport::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(locator))
        return false;

    auto& sockets = mOutputSockets.at(locator.port);
    for (auto& socket : sockets)
    {
        socket.socket_.cancel();
        socket.socket_.close();
    }

    mOutputSockets.erase(locator.port);

    return true;
}

bool UDPv4Transport::CloseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsInputChannelOpen(locator))
    {
        return false;
    }

    mInputSockets.erase(locator.port);
    return true;
}

bool UDPv4Transport::ReleaseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsInputChannelOpen(locator))
    {
        return false;
    }

    try
    {
        ip::udp::socket socket(mService);
        socket.open(ip::udp::v4());
        auto destinationEndpoint = ip::udp::endpoint(asio::ip::address_v4(locatorToNative(locator)),
                static_cast<uint16_t>(locator.port));
        socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint);
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    return true;
}

bool UDPv4Transport::IsInterfaceAllowed(const ip::address_v4& ip) const
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v4::any())
        return true;

    return find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

bool UDPv4Transport::OpenAndBindOutputSockets(Locator_t& locator)
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
            if(mInterfaceWhiteList.empty() || !mWhiteListOutput)
            {
                asio::ip::udp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip::address_v4::any(), locator.port);
                unicastSocket.set_option(ip::multicast::enable_loopback( true ) );

                // If more than one interface, then create sockets for outbounding multicast.
                if(locNames.size() > 1)
                {
                    auto locIt = locNames.begin();

                    // Outbounding first interface with already created socket.
                    unicastSocket.set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string((*locIt).name)));
                    mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));

                    // Create other socket for outbounding rest of interfaces.
                    for(++locIt; locIt != locNames.end(); ++locIt)
                    {
                        auto ip = asio::ip::address_v4::from_string((*locIt).name);
                        uint32_t new_port = 0;
                        asio::ip::udp::socket multicastSocket = OpenAndBindUnicastOutputSocket(ip, new_port);
                        multicastSocket.set_option(ip::multicast::outbound_interface(ip));
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
                        asio::ip::udp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
                        unicastSocket.set_option(ip::multicast::outbound_interface(ip));
                        if(firstInterface)
                        {
                            unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
                            firstInterface = true;
                        }
                        mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));
                    }
                }
            }
        }
        else
        {
            auto ip = asio::ip::address_v4(locatorToNative(locator));
            asio::ip::udp::socket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.port);
            unicastSocket.set_option(ip::multicast::outbound_interface(ip));
            unicastSocket.set_option(ip::multicast::enable_loopback( true ) );
            mOutputSockets[locator.port].push_back(SocketInfo(unicastSocket));
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv4 Error binding at port: (" << locator.port << ")" << " with msg: "<<e.what());
        mOutputSockets.erase(locator.port);
        return false;
    }

    return true;
}

bool UDPv4Transport::OpenAndBindInputSockets(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    asio::ip::address_v4 ip = ip::address_v4::from_string(locator.to_IP4_string());
    uint32_t port = locator.port;
    bool is_multicast = IsMulticastAddress(locator);

    try
    {
       if (is_multicast) {
        // the locator is a multicast address, so bind to the multicast
        // as we'll restrict the interface when we join the group
        mInputSockets[port].push_back(OpenAndBindInputSocket(ip, port, is_multicast));
      }
      else {
        // no filtering, so bind to any
        mInputSockets[port].push_back(OpenAndBindInputSocket(ip::address_v4::any(), port, is_multicast));
      }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv4 Error binding at port: (" << port << ")" << " with msg: "<<e.what());
        mInputSockets.erase(port);
        return false;
    }

    return true;
}

asio::ip::udp::socket UDPv4Transport::OpenAndBindUnicastOutputSocket(const ip::address_v4& ipAddress, uint32_t& port)
{
    ip::udp::socket socket(mService);
    socket.open(ip::udp::v4());
    if(mSendBufferSize != 0)
        socket.set_option(socket_base::send_buffer_size(mSendBufferSize));
    socket.set_option(ip::multicast::hops(mConfiguration_.TTL));

    ip::udp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket.bind(endpoint);

    if(port == 0)
        port = socket.local_endpoint().port();

    return socket;
}

asio::ip::udp::socket UDPv4Transport::OpenAndBindInputSocket(const ip::address_v4& ipAddress, uint32_t port, bool is_multicast)
{
    ip::udp::socket socket(mService);
    socket.open(ip::udp::v4());
    if(mReceiveBufferSize != 0)
        socket.set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    if(is_multicast)
        socket.set_option(ip::udp::socket::reuse_address( true ) );
    ip::udp::endpoint endpoint(ipAddress, static_cast<uint16_t>(port));
    socket.bind(endpoint);

    return socket;
}

bool UDPv4Transport::DoLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.port == right.port;
}

bool UDPv4Transport::IsLocatorSupported(const Locator_t& locator) const
{
  return locator.kind == LOCATOR_KIND_UDPv4;
}

bool UDPv4Transport::IsLocatorAllowed(const Locator_t& locator) const
{
  if (!IsLocatorSupported(locator))
    return false;
  if (mInterfaceWhiteList.empty() || !mWhiteListLocators)
    return true;
  return IsInterfaceAllowed(ip::address_v4::from_string(locator.to_IP4_string()));
}

Locator_t UDPv4Transport::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    Locator_t mainLocal(remote);
    memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    return mainLocal;
}

bool UDPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
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

static void EndpointToLocator(ip::udp::endpoint& endpoint, Locator_t& locator)
{
    locator.port = endpoint.port();
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    memcpy(&locator.address[12], ipBytes.data(), sizeof(ipBytes));
}

bool UDPv4Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
        const Locator_t& localLocator, Locator_t& remoteLocator)
{
    if (!IsInputChannelOpen(localLocator))
        return false;

    bool success = false;
    ip::udp::endpoint senderEndpoint;
    Semaphore receiveSemaphore(0);

    auto handler = [&receiveBufferSize, &success, &receiveSemaphore]
        (const asio::error_code& error, std::size_t bytes_transferred)
        {
          if(error)
          {
            logError(RTPS_MSG_IN, "Error while listening to socket: " <<
                     error.category().name() << "(" <<  error.value() << ")");
            receiveBufferSize = 0;
          }
          else
          {
            receiveBufferSize = static_cast<uint32_t>(bytes_transferred);
            success = (receiveBufferSize)? true: false;
          }
          receiveSemaphore.post();
        };
    {
      // lock scope
      std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
      if (!IsInputChannelOpen(localLocator))
      {
        logError(RTPS_MSG_IN, "Local locator " << localLocator << " is not an open input channel");
        return false;
      }
      auto& sockets = mInputSockets.at(localLocator.port);
      for (auto& socket : sockets)
      {
        socket.async_receive_from(asio::buffer(receiveBuffer, receiveBufferCapacity), senderEndpoint, handler);
      }
    }
    receiveSemaphore.wait();
    if (success)
    {
      if(receiveBufferSize == 13 && memcmp(receiveBuffer, "EPRORTPSCLOSE", 13) == 0)
      {
        success = false;
      }
      else
      {
        EndpointToLocator(senderEndpoint, remoteLocator);
      }
    }
    return success;
}

bool UDPv4Transport::SendThroughSocket(const octet* sendBuffer,
        uint32_t sendBufferSize,
        const Locator_t& remoteLocator,
        asio::ip::udp::socket& socket)
{

    asio::ip::address_v4::bytes_type remoteAddress;
    memcpy(&remoteAddress, &remoteLocator.address[12], sizeof(remoteAddress));
    auto destinationEndpoint = ip::udp::endpoint(asio::ip::address_v4(remoteAddress), static_cast<uint16_t>(remoteLocator.port));
    size_t bytesSent = 0;
    logInfo(RTPS_MSG_OUT,"UDPv4: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << socket.local_endpoint());

    try
    {
        bytesSent = socket.send_to(asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
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

LocatorList_t UDPv4Transport::NormalizeLocator(const Locator_t& locator)
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

LocatorList_t UDPv4Transport::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
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
            assert((*it).kind == LOCATOR_KIND_UDPv4);

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

bool UDPv4Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv4);

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
