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

#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/transport/UDPTransportInterface.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include "UDPSenderResource.hpp"
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

struct MultiUniLocatorsLinkage
{
    MultiUniLocatorsLinkage(LocatorList_t&& m, LocatorList_t&& u)
        : multicast(std::move(m))
        , unicast(std::move(u))
    {
    }

    LocatorList_t multicast;
    LocatorList_t unicast;
};

UDPTransportDescriptor::UDPTransportDescriptor()
    : SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
    , m_output_udp_socket(0)
{
}

UDPTransportDescriptor::UDPTransportDescriptor(const UDPTransportDescriptor& t)
    : SocketTransportDescriptor(t)
    , m_output_udp_socket(t.m_output_udp_socket)
{
}

UDPTransportInterface::UDPTransportInterface(int32_t transport_kind)
    : TransportInterface(transport_kind)
    , mSendBufferSize(0)
    , mReceiveBufferSize(0)
{
}

UDPTransportInterface::~UDPTransportInterface()
{
}

void UDPTransportInterface::clean()
{
    assert(mInputSockets.size() == 0);
}

bool UDPTransportInterface::CloseInputChannel(const Locator_t& locator)
{
    std::vector<UDPChannelResource*> channel_resources;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!IsInputChannelOpen(locator))
            return false;

        channel_resources = std::move(mInputSockets.at(IPLocator::getPhysicalPort(locator)));
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));

    }

    std::map<UDPChannelResource*, asio::ip::address> addresses;
    // It may sound redundant, but we must mark all the related channel to be killed first.
    // Mostly in Windows, but in Linux can happen too, if we access to the endpoint
    // of an already closed socket we get an exception. So we store the interface address to
    // be used in the ReleaseInputChannel call later.
    for (UDPChannelResource* channel_resource : channel_resources)
    {
        if (channel_resource->alive())
        {
            addresses[channel_resource] = channel_resource->socket()->local_endpoint().address();
        }
        else
        {
            addresses[channel_resource] = asio::ip::address();
        }
        channel_resource->disable();
    }

    // Then we release the channels
    for (UDPChannelResource* channel : channel_resources)
    {
        ReleaseInputChannel(locator, addresses[channel]);
        channel->socket()->cancel();
        channel->socket()->close();
        delete channel;
    }

    return true;
}

void UDPTransportInterface::CloseOutputChannel(eProsimaUDPSocket& socket)
{
    socket.cancel();
    socket.close();
}

bool UDPTransportInterface::DoInputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return IPLocator::getPhysicalPort(left) == IPLocator::getPhysicalPort(right);
}

bool UDPTransportInterface::init()
{
    if (configuration()->sendBufferSize == 0 || configuration()->receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::udp::socket socket(io_service_);
        socket.open(generate_protocol());

        if (configuration()->sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            set_send_buffer_size(option.value());

            if (configuration()->sendBufferSize < s_minimumSocketBuffer)
            {
                set_send_buffer_size(s_minimumSocketBuffer);
                mSendBufferSize = s_minimumSocketBuffer;
            }
        }

        if (configuration()->receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            set_receive_buffer_size(option.value());

            if (configuration()->receiveBufferSize < s_minimumSocketBuffer)
            {
                set_receive_buffer_size(s_minimumSocketBuffer);
                mReceiveBufferSize = s_minimumSocketBuffer;
            }
        }
    }

    if (configuration()->maxMessageSize > s_maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (configuration()->maxMessageSize > configuration()->sendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than send_buffer_size");
        return false;
    }

    if (configuration()->maxMessageSize > configuration()->receiveBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than receive_buffer_size");
        return false;
    }

    // TODO(Ricardo) Create an event that update this list.
    get_ips(currentInterfaces);

    return true;
}

bool UDPTransportInterface::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(IPLocator::getPhysicalPort(locator)) != mInputSockets.end());
}

bool UDPTransportInterface::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == transport_kind_;
}

bool UDPTransportInterface::OpenAndBindInputSockets(const Locator_t& locator, TransportReceiverInterface* receiver,
    bool is_multicast, uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        std::vector<std::string> vInterfaces = get_binding_interfaces_list();
        for (std::string sInterface : vInterfaces)
        {
            UDPChannelResource* p_channel_resource;
            p_channel_resource = CreateInputChannelResource(sInterface, locator, is_multicast, maxMsgSize, receiver);
            mInputSockets[IPLocator::getPhysicalPort(locator)].push_back(p_channel_resource);
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")"
            << " with msg: " << e.what());
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));
        return false;
    }

    return true;
}

UDPChannelResource* UDPTransportInterface::CreateInputChannelResource(const std::string& sInterface, const Locator_t& locator,
    bool is_multicast, uint32_t maxMsgSize, TransportReceiverInterface* receiver)
{
    eProsimaUDPSocket unicastSocket = OpenAndBindInputSocket(sInterface, IPLocator::getPhysicalPort(locator), is_multicast);
    UDPChannelResource* p_channel_resource = new UDPChannelResource(unicastSocket, maxMsgSize);
    p_channel_resource->message_receiver(receiver);
    p_channel_resource->interface(sInterface);
    p_channel_resource->thread(std::thread(&UDPTransportInterface::perform_listen_operation, this,
        p_channel_resource, locator));
    return p_channel_resource;
}

eProsimaUDPSocket UDPTransportInterface::OpenAndBindUnicastOutputSocket(
        const ip::udp::endpoint& endpoint,
        uint16_t& port)
{
    eProsimaUDPSocket socket = createUDPSocket(io_service_);
    getSocketPtr(socket)->open(generate_protocol());
    if (mSendBufferSize != 0)
    {
        getSocketPtr(socket)->set_option(socket_base::send_buffer_size(mSendBufferSize));
    }
    getSocketPtr(socket)->set_option(ip::multicast::hops(configuration()->TTL));
    getSocketPtr(socket)->bind(endpoint);
    getSocketPtr(socket)->non_blocking(configuration()->non_blocking_send);

    if (port == 0)
    {
        port = getSocketPtr(socket)->local_endpoint().port();
    }

    return socket;
}

bool UDPTransportInterface::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const Locator_t& locator)
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // We try to find a SenderResource that can be reuse to this locator.
    // Note: This is done in this level because if we do in NetworkFactory level, we have to mantain what transport
    // already reuses a SenderResource.
    for(auto& sender_resource : sender_resource_list)
    {
        UDPSenderResource* udp_sender_resource = UDPSenderResource::cast(*this, sender_resource.get());

        if(udp_sender_resource)
        {
            return true;
        }
    }

    try
    {
        uint16_t port = configuration()->m_output_udp_socket;
        std::vector<IPFinder::info_IP> locNames;
        get_ips(locNames);
        // If there is no whitelist, we can simply open a generic output socket
        // and gain efficiency.
        if (is_interface_whitelist_empty())
        {
            eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(GenerateAnyAddressEndpoint(port), port);
            getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback(true));

            // Outbounding first interface with already created socket.
            if(!locNames.empty())
            {
                SetSocketOutboundInterface(unicastSocket, (*locNames.begin()).name);
            }

            // If more than one interface, then create sockets for outbounding multicast.
            if (locNames.size() > 1)
            {
                auto locIt = locNames.begin();
                sender_resource_list.emplace_back(
                        static_cast<SenderResource*>(new UDPSenderResource(*this, unicastSocket)));

                // Create other socket for outbounding rest of interfaces.
                for (++locIt; locIt != locNames.end(); ++locIt)
                {
                    uint16_t new_port = 0;
                    try
                    {
                        eProsimaUDPSocket multicastSocket =
                            OpenAndBindUnicastOutputSocket(generate_endpoint((*locIt).name, new_port), new_port);
                        SetSocketOutboundInterface(multicastSocket, (*locIt).name);

                        sender_resource_list.emplace_back(
                                static_cast<SenderResource*>(new UDPSenderResource(*this, multicastSocket, true)));
                    }
                    catch(asio::system_error const& e)
                    {
                        (void)e;
                        logWarning(RTPS_MSG_OUT, "UDPTransport Error binding interface "
                            << (*locIt).name << " (skipping) with msg: " << e.what());
                    }
                }
            }
            else
            {
                // Multicast data will be sent for the only one interface.
                sender_resource_list.emplace_back(
                        static_cast<SenderResource*>(new UDPSenderResource(*this, unicastSocket)));
            }
        }
        else
        {
            locNames.clear();
            get_ips(locNames, true);

            bool firstInterface = false;
            for (const auto& infoIP : locNames)
            {
                if (is_interface_allowed(infoIP.name))
                {
                    eProsimaUDPSocket unicastSocket =
                        OpenAndBindUnicastOutputSocket(generate_endpoint(infoIP.name, port), port);
                    SetSocketOutboundInterface(unicastSocket, infoIP.name);
                    if (!firstInterface)
                    {
                        getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback(true));
                        firstInterface = true;
                    }
                    sender_resource_list.emplace_back(
                            static_cast<SenderResource*>(new UDPSenderResource(*this, unicastSocket)));
                }
            }
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        /* TODO Que hacer?
        logError(RTPS_MSG_OUT, "UDPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")"
            << " with msg: " << e.what());
        for (auto& socket : mOutputSockets)
        {
            delete socket;
        }
        mOutputSockets.clear();
        */
        return false;
    }

    return true;
}

void UDPTransportInterface::perform_listen_operation(UDPChannelResource* p_channel_resource, Locator_t input_locator)
{
    Locator_t remote_locator;

    while (p_channel_resource->alive())
    {
        // Blocking receive.
        auto& msg = p_channel_resource->message_buffer();
        if (!Receive(p_channel_resource, msg.buffer, msg.max_size, msg.length, remote_locator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        auto receiver = p_channel_resource->message_receiver();
        if (receiver != nullptr)
        {
            receiver->OnDataReceived(msg.buffer, msg.length, input_locator, remote_locator);
        }
        else
        {
            logWarning(RTPS_MSG_IN, "Received Message, but no receiver attached");
        }
    }
}

bool UDPTransportInterface::Receive(UDPChannelResource* p_channel_resource, octet* receive_buffer,
    uint32_t receive_buffer_capacity, uint32_t& receive_buffer_size, Locator_t& remote_locator)
{
    try
    {
        ip::udp::endpoint senderEndpoint;

        size_t bytes = p_channel_resource->socket()->receive_from(asio::buffer(receive_buffer, receive_buffer_capacity), senderEndpoint);
        receive_buffer_size = static_cast<uint32_t>(bytes);
        if (receive_buffer_size > 0)
        {
            if (receive_buffer_size == 13 && memcmp(receive_buffer, "EPRORTPSCLOSE", 13) == 0)
            {
                return false;
            }
            endpoint_to_locator(senderEndpoint, remote_locator);
        }
        return (receive_buffer_size > 0);
    }
    catch (const std::exception& error)
    {
        (void)error;
        logWarning(RTPS_MSG_OUT, "Error receiving data: " << error.what());
        return false;
    }
}

bool UDPTransportInterface::ReleaseInputChannel(const Locator_t& locator, const asio::ip::address& interface_address)
{
    try
    {
        uint16_t port = IPLocator::getPhysicalPort(locator);

        if(is_interface_whitelist_empty())
        {
            Locator_t localLocator;
            fill_local_ip(localLocator);

            ip::udp::socket socket(io_service_);
            socket.open(generate_protocol());
            socket.bind(generate_local_endpoint(localLocator, 0));

            // We first send directly to localhost, in case all network interfaces are disabled
            // (which would mean that multicast traffic may not be sent)
            auto localEndpoint = generate_local_endpoint(localLocator, port);
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), localEndpoint);

            // We then send to the address of the input locator
            auto destinationEndpoint = generate_local_endpoint(locator, port);

            asio::error_code ec;
            socket_base::message_flags flags = 0;

            // We ignore the error message because some OS don't allow this functionality like Windows (WSAENETUNREACH) or Mac (EADDRNOTAVAIL)
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint,flags, ec);

            socket.close();
        }
        else
        {
            ip::udp::socket socket(io_service_);
            socket.open(generate_protocol());
            socket.bind(asio::ip::udp::endpoint(interface_address, 0));

            auto localEndpoint = ip::udp::endpoint(interface_address, port);
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), localEndpoint);

            // We then send to the address of the input locator
            auto destinationEndpoint = generate_local_endpoint(locator, port);

            asio::error_code ec;
            socket_base::message_flags flags = 0;

            // We ignore the error message because some OS don't allow this functionality like Windows (WSAENETUNREACH) or Mac (EADDRNOTAVAIL)
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint, flags, ec);

            socket.close();
        }
    }
    catch (const std::exception& error)
    {
        logError(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    return true;
}

Locator_t UDPTransportInterface::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
    {
        return false;
    }

    Locator_t mainLocal(remote);
    //memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

bool UDPTransportInterface::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        eProsimaUDPSocket& socket,
        const Locator_t& remote_locator,
        bool only_multicast_purpose)
{
    if (!IsLocatorSupported(remote_locator) || send_buffer_size > configuration()->sendBufferSize)
    {
        return false;
    }

    bool success = false;
    bool is_multicast_remote_address = IPLocator::isMulticast(remote_locator);

    if (is_multicast_remote_address || !only_multicast_purpose)
    {
        auto destinationEndpoint = generate_endpoint(remote_locator, IPLocator::getPhysicalPort(remote_locator));

        size_t bytesSent = 0;

        try
        {
            asio::error_code ec;
            bytesSent = getSocketPtr(socket)->send_to(asio::buffer(send_buffer, send_buffer_size), destinationEndpoint, 0, ec);
            if(!!ec)
            {
                if ((ec.value() == asio::error::would_block) ||
                    (ec.value() == asio::error::try_again))
                {
                    logWarning(RTPS_MSG_OUT, "UDP send would have blocked. Packet is dropped.");
                    return true;
                }

                logWarning(RTPS_MSG_OUT, ec.message());
                return false;
            }
        }
        catch (const std::exception& error)
        {
            logWarning(RTPS_MSG_OUT, error.what());
            return false;
        }

        (void)bytesSent;
        logInfo(RTPS_MSG_OUT, "UDPTransport: " << bytesSent << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << getSocketPtr(socket)->local_endpoint());
        success = true;
    }

    return success;
}

LocatorList_t UDPTransportInterface::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t multicastResult, unicastResult;
    std::vector<MultiUniLocatorsLinkage> pendingLocators;

    for (auto& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        bool multicastDefined = false;
        LocatorList_t pendingMulticast, pendingUnicast;

        while (it != locatorList.end())
        {
            assert((*it).kind == transport_kind_);

            if (IPLocator::isMulticast(*it))
            {
                // If the multicast locator is already chosen, not choose any unicast locator.
                if (multicastResult.contains(*it))
                {
                    multicastDefined = true;
                    pendingUnicast.clear();
                }
                else
                {
                    // Search the multicast locator in pending locators.
                    auto pending_it = pendingLocators.begin();
                    bool found = false;

                    while (pending_it != pendingLocators.end())
                    {
                        if ((*pending_it).multicast.contains(*it))
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
                    if (!found)
                        pendingMulticast.push_back(*it);
                }
            }
            else
            {
                if (!multicastDefined)
                {
                    // Check is local interface.
                    auto localInterface = currentInterfaces.begin();
                    for (; localInterface != currentInterfaces.end(); ++localInterface)
                    {
                        if (compare_locator_ip(localInterface->locator, *it))
                        {
                            // Check 127.0.0.1 in the whitelist
                            Locator_t loopbackLocator;
                            fill_local_ip(loopbackLocator);
                            if (is_locator_allowed(loopbackLocator))
                            {
                                // Loopback locator
                                IPLocator::setPhysicalPort(loopbackLocator, IPLocator::getPhysicalPort(*it));
                                pendingUnicast.push_back(loopbackLocator);
                            }
                            else
                            {
                                // Check interface in whitelist
                                if (is_locator_allowed(*it))
                                {
                                    // Custom Loopback locator
                                    pendingUnicast.push_back(*it);
                                }
                            }
                            break;
                        }
                    }

                    if (localInterface == currentInterfaces.end())
                    {
                        pendingUnicast.push_back(*it);
                    }
                }
            }

            ++it;
        }

        if (pendingMulticast.size() == 0)
        {
            unicastResult.push_back(pendingUnicast);
        }
        else if (pendingUnicast.size() == 0)
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
    for (auto link : pendingLocators)
        result.push_back(link.unicast);

    return result;
}

bool UDPTransportInterface::fillMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_multicast_port;
    }
    return true;
}

bool UDPTransportInterface::fillMetatrafficUnicastLocator(Locator_t &locator,
        uint32_t metatraffic_unicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_unicast_port;
    }
    return true;
}

bool UDPTransportInterface::configureInitialPeerLocator(Locator_t &locator, const PortParameters &port_params,
        uint32_t domainId, LocatorList_t& list) const
{
    if(locator.port == 0)
    {
        for(uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
        {
            Locator_t auxloc(locator);
            auxloc.port = port_params.getUnicastPort(domainId, i);

            list.push_back(auxloc);
        }
    }
    else
        list.push_back(locator);

    return true;
}

bool UDPTransportInterface::fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const
{
    if (locator.port == 0)
    {
        locator.port = well_known_port;
    }
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
