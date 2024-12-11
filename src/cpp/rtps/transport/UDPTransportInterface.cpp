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

#include <rtps/transport/UDPTransportInterface.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <limits>
#include <utility>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/transport/asio_helpers.hpp>
#include <rtps/transport/UDPSenderResource.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

UDPTransportDescriptor::UDPTransportDescriptor()
    : SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
    , m_output_udp_socket(0)
{
}

bool UDPTransportDescriptor::operator ==(
        const UDPTransportDescriptor& t) const
{
    return (this->m_output_udp_socket == t.m_output_udp_socket &&
           this->non_blocking_send == t.non_blocking_send &&
           SocketTransportDescriptor::operator ==(t));
}

UDPTransportInterface::UDPTransportInterface(
        int32_t transport_kind)
    : TransportInterface(transport_kind)
    , mSendBufferSize(0)
    , mReceiveBufferSize(0)
    , first_time_open_output_channel_(true)
    , netmask_filter_(NetmaskFilterKind::AUTO)
{
}

UDPTransportInterface::~UDPTransportInterface()
{
}

void UDPTransportInterface::clean()
{
    assert(mInputSockets.size() == 0);
}

bool UDPTransportInterface::CloseInputChannel(
        const Locator& locator)
{
    std::vector<UDPChannelResource*> channel_resources;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!IsInputChannelOpen(locator))
        {
            return false;
        }

        channel_resources = std::move(mInputSockets.at(IPLocator::getPhysicalPort(locator)));
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));

    }

    // We now disable and release the channels
    for (UDPChannelResource* channel : channel_resources)
    {
        channel->disable();
        channel->release();
        channel->clear();
        delete channel;
    }

    return true;
}

void UDPTransportInterface::SenderResourceHasBeenClosed(
        eProsimaUDPSocket& socket)
{
    socket.cancel();
    socket.close();
}

bool UDPTransportInterface::DoInputLocatorsMatch(
        const Locator& left,
        const Locator& right) const
{
    return IPLocator::getPhysicalPort(left) == IPLocator::getPhysicalPort(right);
}

bool UDPTransportInterface::init(
        const fastdds::rtps::PropertyPolicy*,
        const uint32_t& max_msg_size_no_frag)
{
    uint32_t maximumMessageSize = max_msg_size_no_frag == 0 ? s_maximumMessageSize : max_msg_size_no_frag;
    uint32_t cfg_max_msg_size = configuration()->maxMessageSize;
    uint32_t cfg_send_size = configuration()->sendBufferSize;
    uint32_t cfg_recv_size = configuration()->receiveBufferSize;
    uint32_t max_int_value = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());

    if (cfg_max_msg_size > maximumMessageSize)
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "maxMessageSize cannot be greater than " << maximumMessageSize);
        return false;
    }

    if (cfg_send_size > max_int_value)
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "sendBufferSize cannot be greater than " << max_int_value);
        return false;
    }

    if (cfg_recv_size > max_int_value)
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "receiveBufferSize cannot be greater than " << max_int_value);
        return false;
    }

    if ((cfg_send_size > 0) && (cfg_max_msg_size > cfg_send_size))
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if ((cfg_recv_size > 0) && (cfg_max_msg_size > cfg_recv_size))
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "maxMessageSize cannot be greater than receiveBufferSize");
        return false;
    }

    asio::error_code ec;
    ip::udp::socket socket(io_service_);
    socket.open(generate_protocol(), ec);
    if (!!ec)
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "Error creating socket: " << ec.message());
        return false;
    }

    bool ret = asio_helpers::configure_buffer_sizes(socket, *configuration(), mSendBufferSize, mReceiveBufferSize);
    if (ret)
    {
        if (cfg_send_size > 0 && mSendBufferSize != cfg_send_size)
        {
            EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "UDPTransport sendBufferSize could not be set to the desired value. "
                    << "Using " << mSendBufferSize << " instead of " << cfg_send_size);
        }

        if (cfg_recv_size > 0 && mReceiveBufferSize != cfg_recv_size)
        {
            EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "UDPTransport receiveBufferSize could not be set to the desired value. "
                    << "Using " << mReceiveBufferSize << " instead of " << cfg_recv_size);
        }

        set_send_buffer_size(mSendBufferSize);
        set_receive_buffer_size(mReceiveBufferSize);
    }
    else
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "Couldn't set buffer sizes to minimum value: " << cfg_max_msg_size);
    }

    return ret;
}

bool UDPTransportInterface::IsInputChannelOpen(
        const Locator& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(IPLocator::getPhysicalPort(
               locator)) != mInputSockets.end());
}

bool UDPTransportInterface::IsLocatorSupported(
        const Locator& locator) const
{
    return locator.kind == transport_kind_;
}

bool UDPTransportInterface::OpenAndBindInputSockets(
        const Locator& locator,
        TransportReceiverInterface* receiver,
        bool is_multicast,
        uint32_t maxMsgSize)
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
        EPROSIMA_LOG_INFO(TRANSPORT_UDP, "UDPTransport Error binding at port: ("
                << IPLocator::getPhysicalPort(locator) << ")" << " with msg: " << e.what());
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));
        return false;
    }

    return true;
}

UDPChannelResource* UDPTransportInterface::CreateInputChannelResource(
        const std::string& sInterface,
        const Locator& locator,
        bool is_multicast,
        uint32_t maxMsgSize,
        TransportReceiverInterface* receiver)
{
    eProsimaUDPSocket unicastSocket = OpenAndBindInputSocket(sInterface,
                    IPLocator::getPhysicalPort(locator), is_multicast);
    UDPChannelResource* p_channel_resource = new UDPChannelResource(this, unicastSocket, maxMsgSize, locator,
                    sInterface, receiver, configuration()->get_thread_config_for_port(locator.port));
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
        uint32_t configured_value = 0;
        if (!asio_helpers::try_setting_buffer_size<socket_base::send_buffer_size>(
                    socket, mSendBufferSize, configuration()->maxMessageSize, configured_value))
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_UDP,
                    "Couldn't set send buffer size to minimum value: " << configuration()->maxMessageSize);
        }
        else if (configured_value != mSendBufferSize)
        {
            EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "UDPTransport sendBufferSize could not be set to the desired value. "
                    << "Using " << configured_value << " instead of " << mSendBufferSize);
        }
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

eProsimaUDPSocket UDPTransportInterface::OpenAndBindUnicastOutputSocket(
        const ip::udp::endpoint& endpoint,
        uint16_t& port,
        const LocatorWithMask& locator)
{
    eProsimaUDPSocket socket = OpenAndBindUnicastOutputSocket(endpoint, port);

    socket.locator = locator;

    auto it = std::find_if(
        allowed_interfaces_.begin(),
        allowed_interfaces_.end(),
        [&locator](const AllowedNetworkInterface& entry)
        {
            return locator == entry.locator;
        });

    if (it != allowed_interfaces_.end())
    {
        socket.netmask_filter = it->netmask_filter;
    }
    else
    {
        socket.netmask_filter = netmask_filter_;
    }

    return socket;
}

bool UDPTransportInterface::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const Locator& locator)
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    std::vector<IPFinder::info_IP> locNames;
    get_unknown_network_interfaces(sender_resource_list, locNames);

    if (locNames.empty() && !first_time_open_output_channel_)
    {
        statistics_info_.add_entry(locator);
        rescan_interfaces_.store(false);
        return true;
    }

    try
    {
        uint16_t port = configuration()->m_output_udp_socket;
        // If there is no whitelist, we can simply open a generic output socket
        // and gain efficiency.
        if (is_interface_whitelist_empty())
        {
            if (first_time_open_output_channel_)
            {
                first_time_open_output_channel_ = false;
                // We add localhost output for multicast, so in case the network cable is unplugged, local
                // participants keep receiving DATA(p) announcements
                // Also in case that no network interfaces were found
                try
                {
                    eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(GenerateAnyAddressEndpoint(
                                        port), port);
                    getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback(true));
                    SetSocketOutboundInterface(unicastSocket, localhost_name());
                    sender_resource_list.emplace_back(
                        static_cast<SenderResource*>(new UDPSenderResource(*this, unicastSocket, false, true)));
                }
                catch (asio::system_error const& e)
                {
                    (void)e;
                    EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "UDPTransport Error binding interface "
                            << localhost_name() << " (skipping) with msg: " << e.what());
                }
            }

            // Create sockets for outbounding multicast for the other found network interfaces.
            if (!locNames.empty())
            {
                // Create other socket for outbounding rest of interfaces.
                for (auto locIt = locNames.begin(); locIt != locNames.end(); ++locIt)
                {
                    uint16_t new_port = 0;
                    try
                    {
                        eProsimaUDPSocket multicastSocket =
                                OpenAndBindUnicastOutputSocket(generate_endpoint((*locIt).name, new_port), new_port,
                                        (*locIt).masked_locator);
                        SetSocketOutboundInterface(multicastSocket, (*locIt).name);
                        sender_resource_list.emplace_back(
                            static_cast<SenderResource*>(new UDPSenderResource(*this, multicastSocket, true)));
                    }
                    catch (asio::system_error const& e)
                    {
                        (void)e;
                        EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "UDPTransport Error binding interface "
                                << (*locIt).name << " (skipping) with msg: " << e.what());
                    }
                }
            }
        }
        else
        {
            get_unknown_network_interfaces(sender_resource_list, locNames, true);

            for (const auto& infoIP : locNames)
            {
                if (is_interface_allowed(infoIP.name))
                {
                    eProsimaUDPSocket unicastSocket =
                            OpenAndBindUnicastOutputSocket(generate_endpoint(infoIP.name,
                                    port), port, infoIP.masked_locator);
                    SetSocketOutboundInterface(unicastSocket, infoIP.name);
                    if (first_time_open_output_channel_)
                    {
                        getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback(true));
                        first_time_open_output_channel_ = false;
                    }
                    sender_resource_list.emplace_back(
                        static_cast<SenderResource*>(new UDPSenderResource(*this, unicastSocket, false, true)));
                }
            }
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        /* TODO Que hacer?
           EPROSIMA_LOG_ERROR(TRANSPORT_UDP, "UDPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")"
            << " with msg: " << e.what());
           for (auto& socket : mOutputSockets)
           {
            delete socket;
           }
           mOutputSockets.clear();
         */
        return false;
    }

    statistics_info_.add_entry(locator);
    rescan_interfaces_.store(false);
    return true;
}

bool UDPTransportInterface::OpenOutputChannels(
        SendResourceList& send_resource_list,
        const LocatorSelectorEntry& locator_selector_entry)
{
    bool success = false;
    for (size_t i = 0; i < locator_selector_entry.state.unicast.size(); ++i)
    {
        size_t index = locator_selector_entry.state.unicast[i];
        success |= OpenOutputChannel(send_resource_list, locator_selector_entry.unicast[index]);
    }
    return success;
}

Locator UDPTransportInterface::RemoteToMainLocal(
        const Locator& remote) const
{
    if (!IsLocatorSupported(remote))
    {
        return false;
    }

    Locator mainLocal(remote);
    //memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

bool UDPTransportInterface::transform_remote_locator(
        const Locator& remote_locator,
        Locator& result_locator,
        bool allowed_remote_localhost,
        bool allowed_local_localhost) const
{
    if (IsLocatorSupported(remote_locator))
    {
        result_locator = remote_locator;
        if (!is_local_locator(result_locator))
        {
            // is_local_locator will return false for multicast addresses as well as remote unicast ones.
            return true;
        }

        // If we get here, the locator is a local unicast address

        // Attempt conversion to localhost if remote transport listening on it allows it
        if (allowed_remote_localhost)
        {
            Locator loopbackLocator;
            fill_local_ip(loopbackLocator);
            if (is_locator_allowed(loopbackLocator))
            {
                // Locator localhost is in the whitelist, so use localhost instead of remote_locator
                fill_local_ip(result_locator);
                return true;
            }
            else if (allowed_local_localhost)
            {
                // Abort transformation if localhost not allowed by this transport, but it is by other local transport
                // and the remote one.
                return false;
            }
        }

        if (!is_locator_allowed(result_locator))
        {
            // Neither original remote locator nor localhost allowed: abort.
            return false;
        }

        return true;
    }
    return false;
}

bool UDPTransportInterface::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        eProsimaUDPSocket& socket,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        bool only_multicast_purpose,
        bool whitelisted,
        const std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    fastdds::rtps::LocatorsIterator& it = *destination_locators_begin;

    bool ret = true;

    auto time_out = std::chrono::duration_cast<std::chrono::microseconds>(
        max_blocking_time_point - std::chrono::steady_clock::now());

    while (it != *destination_locators_end)
    {
        if (IsLocatorSupported(*it))
        {
            ret &= send(buffers,
                            total_bytes,
                            socket,
                            *it,
                            only_multicast_purpose,
                            whitelisted,
                            time_out);
        }

        ++it;
    }

    return ret;
}

bool UDPTransportInterface::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        eProsimaUDPSocket& socket,
        const Locator& remote_locator,
        bool only_multicast_purpose,
        bool whitelisted,
        const std::chrono::microseconds& timeout)
{
    using namespace eprosima::fastdds::statistics::rtps;

    if (total_bytes > configuration()->sendBufferSize)
    {
        return false;
    }

    bool success = false;
    bool is_multicast_remote_address = IPLocator::isMulticast(remote_locator);

    if (is_multicast_remote_address == only_multicast_purpose || whitelisted)
    {
        if (!is_multicast_remote_address && socket.should_filter(remote_locator))
        {
            // Filter unicast remote locators according to socket conditions (e.g. netmask filtering)
            return true;
        }

        auto destinationEndpoint = generate_endpoint(remote_locator, IPLocator::getPhysicalPort(remote_locator));

        size_t bytesSent = 0;

        try
        {
            (void)timeout;
#ifndef _WIN32
            struct timeval timeStruct;
            timeStruct.tv_sec = 0;
            timeStruct.tv_usec = timeout.count() > 0 ? timeout.count() : 0;
            setsockopt(getSocketPtr(socket)->native_handle(), SOL_SOCKET, SO_SNDTIMEO,
                    reinterpret_cast<const char*>(&timeStruct), sizeof(timeStruct));
#endif // ifndef _WIN32

            asio::error_code ec;
            // Statistics submessage is always the last buffer to be added
            statistics_info_.set_statistics_message_data(remote_locator, buffers.back(), total_bytes);
            bytesSent = getSocketPtr(socket)->send_to(buffers, destinationEndpoint, 0, ec);
            if (!!ec)
            {
                if ((ec.value() == asio::error::would_block) ||
                        (ec.value() == asio::error::try_again))
                {
                    EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "UDP send would have blocked. Packet is dropped.");
                    return true;
                }

                EPROSIMA_LOG_WARNING(TRANSPORT_UDP, ec.message());
                return false;
            }

            if (bytesSent != total_bytes)
            {
                EPROSIMA_LOG_WARNING(TRANSPORT_UDP, "Boost send_to wasn't able to send all bytes");
            }
        }
        catch (const std::exception& error)
        {
            EPROSIMA_LOG_WARNING(TRANSPORT_UDP, error.what());
            return false;
        }

        (void)bytesSent;
        EPROSIMA_LOG_INFO(TRANSPORT_UDP,
                "UDPTransport: " << bytesSent << " bytes TO endpoint: " << destinationEndpoint <<
                " FROM " << getSocketPtr(socket)->local_endpoint());
        success = true;
    }

    return success;
}

/**
 * Invalidate all selector entries containing certain multicast locator.
 *
 * This function will process all entries from 'index' onwards and, if any
 * of them has 'locator' on its multicast list, will invalidate them
 * (i.e. their 'transport_should_process' flag will be changed to false).
 *
 * If this function returns true, the locator received should be selected.
 *
 * @param entries   Selector entries collection to process
 * @param index     Starting index to process
 * @param locator   Locator to be searched
 *
 * @return true when at least one entry was invalidated, false otherwise
 */
static bool check_and_invalidate(
        fastdds::ResourceLimitedVector<LocatorSelectorEntry*>& entries,
        size_t index,
        const Locator& locator)
{
    bool ret_val = false;
    for (; index < entries.size(); ++index)
    {
        LocatorSelectorEntry* entry = entries[index];
        if (entry->transport_should_process)
        {
            for (const Locator& loc : entry->multicast)
            {
                if (loc == locator)
                {
                    entry->transport_should_process = false;
                    ret_val = true;
                    break;
                }
            }
        }
    }

    return ret_val;
}

void UDPTransportInterface::select_locators(
        LocatorSelector& selector) const
{
    fastdds::ResourceLimitedVector<LocatorSelectorEntry*>& entries = selector.transport_starts();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        LocatorSelectorEntry* entry = entries[i];
        if (entry->transport_should_process)
        {
            bool selected = false;

            // First try to find a multicast locator which is at least on another list.
            for (size_t j = 0; j < entry->multicast.size() && !selected; ++j)
            {
                if (IsLocatorSupported(entry->multicast[j]))
                {
                    if (check_and_invalidate(entries, i + 1, entry->multicast[j]))
                    {
                        entry->state.multicast.push_back(j);
                        selected = true;
                    }
                    else if (entry->unicast.size() == 0)
                    {
                        entry->state.multicast.push_back(j);
                        selected = true;
                    }
                }
            }

            // If we couldn't find a multicast locator, select all unicast locators
            if (!selected)
            {
                for (size_t j = 0; j < entry->unicast.size(); ++j)
                {
                    if (IsLocatorSupported(entry->unicast[j]) && !selector.is_selected(entry->unicast[j]))
                    {
                        entry->state.unicast.push_back(j);
                        selected = true;
                    }
                }
            }

            // Select this entry if necessary
            if (selected)
            {
                selector.select(i);
            }
        }
    }
}

bool UDPTransportInterface::fillMetatrafficMulticastLocator(
        Locator& locator,
        uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_multicast_port;
    }
    return true;
}

bool UDPTransportInterface::fillMetatrafficUnicastLocator(
        Locator& locator,
        uint32_t metatraffic_unicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_unicast_port;
    }
    return true;
}

bool UDPTransportInterface::configureInitialPeerLocator(
        Locator& locator,
        const PortParameters& port_params,
        uint32_t domainId,
        LocatorList& list) const
{
    if (locator.port == 0)
    {
        if (IPLocator::isMulticast(locator))
        {
            Locator auxloc(locator);
            auxloc.port = port_params.getMulticastPort(domainId);
            list.push_back(auxloc);
        }
        else
        {
            for (uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
            {
                Locator auxloc(locator);
                auxloc.port = port_params.getUnicastPort(domainId, i);

                list.push_back(auxloc);
            }
        }
    }
    else
    {
        list.push_back(locator);
    }

    return true;
}

bool UDPTransportInterface::fillUnicastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    if (locator.port == 0)
    {
        locator.port = well_known_port;
    }
    return true;
}

void UDPTransportInterface::get_unknown_network_interfaces(
        const SendResourceList& sender_resource_list,
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback)
{
    locNames.clear();
    if (rescan_interfaces_)
    {
        get_ips(locNames, return_loopback, false);
        for (auto& sender_resource : sender_resource_list)
        {
            UDPSenderResource* udp_sender_resource = UDPSenderResource::cast(*this, sender_resource.get());
            if (nullptr != udp_sender_resource)
            {
                for (auto it = locNames.begin(); it != locNames.end();)
                {
                    if (udp_sender_resource->check_ip_address(it->locator))
                    {
                        it = locNames.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }
    }
}

void UDPTransportInterface::update_network_interfaces()
{
    rescan_interfaces_.store(true);
}

bool UDPTransportInterface::is_localhost_allowed() const
{
    Locator local_locator;
    fill_local_ip(local_locator);
    return is_locator_allowed(local_locator);
}

NetmaskFilterInfo UDPTransportInterface::netmask_filter_info() const
{
    return {netmask_filter_, allowed_interfaces_};
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
