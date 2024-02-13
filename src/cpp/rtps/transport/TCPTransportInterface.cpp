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

#include "TCPTransportInterface.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/socket_base.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_error.hpp>
#if TLS_FOUND
#include <asio/ssl/verify_context.hpp>
#endif // if TLS_FOUND

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/PortParameters.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/transport/SenderResource.h>
#include <fastdds/rtps/transport/SocketTransportDescriptor.h>
#include <fastdds/rtps/transport/TCPTransportDescriptor.h>
#include <fastdds/rtps/transport/TransportReceiverInterface.h>
#include <fastrtps/config.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/System.h>

#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>
#include <utils/SystemInfo.hpp>
#include <utils/thread.hpp>
#include <utils/threading.hpp>

#include "tcp/RTCPHeader.h"
#include "tcp/RTCPMessageManager.h"
#include "TCPAcceptorBasic.h"
#include "TCPChannelResourceBasic.h"
#include "TCPSenderResource.hpp"
#if TLS_FOUND
#include "TCPAcceptorSecure.h"
#include "TCPChannelResourceSecure.h"
#endif // if TLS_FOUND

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using octet = fastrtps::rtps::octet;
using IPLocator = fastrtps::rtps::IPLocator;
using SenderResource = fastrtps::rtps::SenderResource;
using CDRMessage_t = fastrtps::rtps::CDRMessage_t;
using LocatorSelector = fastrtps::rtps::LocatorSelector;
using LocatorSelectorEntry = fastrtps::rtps::LocatorSelectorEntry;
using PortParameters = fastrtps::rtps::PortParameters;
using Log = fastdds::dds::Log;

static const int s_default_keep_alive_frequency = 5000; // 5 SECONDS
static const int s_default_keep_alive_timeout = 15000; // 15 SECONDS
//static const int s_clean_deleted_sockets_pool_timeout = 100; // 100 MILLISECONDS

FASTDDS_TODO_BEFORE(3, 0,
        "Eliminate s_default_tcp_negotitation_timeout, variable used to initialize deprecate attribute.")
static const int s_default_tcp_negotitation_timeout = 5000; // 5 Seconds

TCPTransportDescriptor::TCPTransportDescriptor()
    : SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
    , keep_alive_frequency_ms(s_default_keep_alive_frequency)
    , keep_alive_timeout_ms(s_default_keep_alive_timeout)
    , max_logical_port(100)
    , logical_port_range(20)
    , logical_port_increment(2)
    , tcp_negotiation_timeout(s_default_tcp_negotitation_timeout)
    , enable_tcp_nodelay(false)
    , wait_for_tcp_negotiation(false)
    , calculate_crc(true)
    , check_crc(true)
    , apply_security(false)
{
}

TCPTransportDescriptor::TCPTransportDescriptor(
        const TCPTransportDescriptor& t)
    : SocketTransportDescriptor(t)
    , listening_ports(t.listening_ports)
    , keep_alive_frequency_ms(t.keep_alive_frequency_ms)
    , keep_alive_timeout_ms(t.keep_alive_timeout_ms)
    , max_logical_port(t.max_logical_port)
    , logical_port_range(t.logical_port_range)
    , logical_port_increment(t.logical_port_increment)
    , tcp_negotiation_timeout(t.tcp_negotiation_timeout)
    , enable_tcp_nodelay(t.enable_tcp_nodelay)
    , wait_for_tcp_negotiation(t.wait_for_tcp_negotiation)
    , calculate_crc(t.calculate_crc)
    , check_crc(t.check_crc)
    , apply_security(t.apply_security)
    , tls_config(t.tls_config)
    , keep_alive_thread(t.keep_alive_thread)
    , accept_thread(t.accept_thread)
{
}

TCPTransportDescriptor& TCPTransportDescriptor::operator =(
        const TCPTransportDescriptor& t)
{

    maxMessageSize = t.maxMessageSize;
    maxInitialPeersRange = t.maxInitialPeersRange;
    sendBufferSize = t.sendBufferSize;
    receiveBufferSize = t.receiveBufferSize;
    TTL = t.TTL;
    listening_ports = t.listening_ports;
    keep_alive_frequency_ms = t.keep_alive_frequency_ms;
    keep_alive_timeout_ms = t.keep_alive_timeout_ms;
    max_logical_port = t.max_logical_port;
    logical_port_range = t.logical_port_range;
    logical_port_increment = t.logical_port_increment;
    tcp_negotiation_timeout = t.tcp_negotiation_timeout;
    enable_tcp_nodelay = t.enable_tcp_nodelay;
    wait_for_tcp_negotiation = t.wait_for_tcp_negotiation;
    calculate_crc = t.calculate_crc;
    check_crc = t.check_crc;
    apply_security = t.apply_security;
    tls_config = t.tls_config;
    keep_alive_thread = t.keep_alive_thread;
    accept_thread = t.accept_thread;
    return *this;
}

bool TCPTransportDescriptor::operator ==(
        const TCPTransportDescriptor& t) const
{
    return (this->listening_ports == t.listening_ports &&
           this->keep_alive_frequency_ms == t.keep_alive_frequency_ms &&
           this->keep_alive_timeout_ms == t.keep_alive_timeout_ms &&
           this->max_logical_port == t.max_logical_port &&
           this->logical_port_range == t.logical_port_range &&
           this->logical_port_increment == t.logical_port_increment &&
           this->tcp_negotiation_timeout == t.tcp_negotiation_timeout &&
           this->enable_tcp_nodelay == t.enable_tcp_nodelay &&
           this->wait_for_tcp_negotiation == t.wait_for_tcp_negotiation &&
           this->calculate_crc == t.calculate_crc &&
           this->check_crc == t.check_crc &&
           this->apply_security == t.apply_security &&
           this->tls_config == t.tls_config &&
           this->keep_alive_thread == t.keep_alive_thread &&
           this->accept_thread == t.accept_thread &&
           SocketTransportDescriptor::operator ==(t));
}

TCPTransportInterface::TCPTransportInterface(
        int32_t transport_kind)
    : TransportInterface(transport_kind)
    , alive_(true)
    , non_blocking_send_(false)
#if TLS_FOUND
    , ssl_context_(asio::ssl::context::sslv23)
#endif // if TLS_FOUND
    , keep_alive_event_(io_service_timers_)
{
}

TCPTransportInterface::~TCPTransportInterface()
{
}

void TCPTransportInterface::clean()
{
    assert(receiver_resources_.size() == 0);
    alive_.store(false);

    keep_alive_event_.cancel();
    if (io_service_timers_thread_.joinable())
    {
        io_service_timers_.stop();
        io_service_timers_thread_.join();
    }

    {
        std::vector<std::shared_ptr<TCPChannelResource>> channels;

        {
            std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
            std::unique_lock<std::mutex> unbound_lock(unbound_map_mutex_);

            channels = unbound_channel_resources_;

            for (auto& channel : channel_resources_)
            {
                channels.push_back(channel.second);
            }
        }

        for (auto& channel : channels)
        {
            if (channel->connection_established())
            {
                rtcp_message_manager_->sendUnbindConnectionRequest(channel);
            }

            channel->disconnect();
            channel->clear();
        }

        std::unique_lock<std::mutex> lock(rtcp_message_manager_mutex_);
        rtcp_message_manager_cv_.wait(lock, [&]()
                {
                    return 1 >= rtcp_message_manager_.use_count();
                });

        if (rtcp_message_manager_)
        {
            rtcp_message_manager_->dispose();
            rtcp_message_manager_.reset();
        }
    }

    if (initial_peer_local_locator_socket_)
    {
        if (initial_peer_local_locator_socket_->is_open())
        {
            initial_peer_local_locator_socket_->close();
        }

        initial_peer_local_locator_socket_.reset();
    }

    if (io_service_thread_.joinable())
    {
        io_service_.stop();
        io_service_thread_.join();
    }
}

Locator TCPTransportInterface::remote_endpoint_to_locator(
        const std::shared_ptr<TCPChannelResource>& channel) const
{
    Locator locator;
    asio::error_code ec;
    endpoint_to_locator(channel->remote_endpoint(ec), locator);
    if (ec)
    {
        LOCATOR_INVALID(locator);
    }
    return locator;
}

void TCPTransportInterface::bind_socket(
        std::shared_ptr<TCPChannelResource>& channel)
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    std::unique_lock<std::mutex> unbound_lock(unbound_map_mutex_);

    auto it_remove = std::find(unbound_channel_resources_.begin(), unbound_channel_resources_.end(), channel);
    assert(it_remove != unbound_channel_resources_.end());
    unbound_channel_resources_.erase(it_remove);
    channel_resources_[channel->locator()] = channel;
}

bool TCPTransportInterface::check_crc(
        const TCPHeader& header,
        const octet* data,
        uint32_t size) const
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    return crc == header.crc;
}

void TCPTransportInterface::calculate_crc(
        TCPHeader& header,
        const octet* data,
        uint32_t size) const
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    header.crc = crc;
}

uint16_t TCPTransportInterface::create_acceptor_socket(
        const Locator& locator)
{
    uint16_t final_port = 0;
    try
    {
        if (is_interface_whitelist_empty())
        {
#if TLS_FOUND
            if (configuration()->apply_security)
            {
                std::shared_ptr<TCPAcceptorSecure> acceptor =
                        std::make_shared<TCPAcceptorSecure>(io_service_, this, locator);
                acceptors_[acceptor->locator()] = acceptor;
                acceptor->accept(this, ssl_context_);
                final_port = static_cast<uint16_t>(acceptor->locator().port);
            }
            else
#endif // if TLS_FOUND
            {
                std::shared_ptr<TCPAcceptorBasic> acceptor =
                        std::make_shared<TCPAcceptorBasic>(io_service_, this, locator);
                acceptors_[acceptor->locator()] = acceptor;
                acceptor->accept(this);
                final_port = static_cast<uint16_t>(acceptor->locator().port);
            }

            EPROSIMA_LOG_INFO(RTCP, " OpenAndBindInput (physical: " << IPLocator::getPhysicalPort(
                        locator) << "; logical: "
                                                                    << IPLocator::getLogicalPort(locator) << ")");

        }
        else
        {
            std::vector<std::string> vInterfaces = get_binding_interfaces_list();
            for (std::string& sInterface : vInterfaces)
            {
#if TLS_FOUND
                if (configuration()->apply_security)
                {
                    std::shared_ptr<TCPAcceptorSecure> acceptor =
                            std::make_shared<TCPAcceptorSecure>(io_service_, sInterface, locator);
                    acceptors_[acceptor->locator()] = acceptor;
                    acceptor->accept(this, ssl_context_);
                    final_port = static_cast<uint16_t>(acceptor->locator().port);
                }
                else
#endif // if TLS_FOUND
                {
                    std::shared_ptr<TCPAcceptorBasic> acceptor =
                            std::make_shared<TCPAcceptorBasic>(io_service_, sInterface, locator);
                    acceptors_[acceptor->locator()] = acceptor;
                    acceptor->accept(this);
                    final_port = static_cast<uint16_t>(acceptor->locator().port);
                }

                EPROSIMA_LOG_INFO(RTCP, " OpenAndBindInput (physical: " << IPLocator::getPhysicalPort(
                            locator) << "; logical: "
                                                                        << IPLocator::getLogicalPort(locator) << ")");
            }
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        EPROSIMA_LOG_ERROR(RTCP_MSG_OUT, "TCPTransport Error binding at port: (" << IPLocator::getPhysicalPort(
                    locator) << ")" << " with msg: " << e.what());
        return false;
    }
    catch (const asio::error_code& code)
    {
        (void)code;
        EPROSIMA_LOG_ERROR(RTCP, "TCPTransport Error binding at port: (" << IPLocator::getPhysicalPort(
                    locator) << ")" << " with code: " << code);
        return false;
    }

    return final_port;
}

void TCPTransportInterface::fill_rtcp_header(
        TCPHeader& header,
        const octet* send_buffer,
        uint32_t send_buffer_size,
        uint16_t logical_port) const
{
    header.length = send_buffer_size + static_cast<uint32_t>(TCPHeader::size());
    header.logical_port = logical_port;
    if (configuration()->calculate_crc)
    {
        calculate_crc(header, send_buffer, send_buffer_size);
    }
}

bool TCPTransportInterface::DoInputLocatorsMatch(
        const Locator& left,
        const Locator& right) const
{
    return IPLocator::getPhysicalPort(left) ==  IPLocator::getPhysicalPort(right);
}

bool TCPTransportInterface::init(
        const fastrtps::rtps::PropertyPolicy* properties)
{
    if (!apply_tls_config())
    {
        // TODO decide wether the Transport initialization should keep working after this error
        EPROSIMA_LOG_WARNING(TLS, "Error configuring TLS, using TCP transport without security");
    }

    /*
       Open and bind a socket to obtain a unique port. This port is assigned to PDP passed locators.
       Although real client socket local port will differ, this ensures uniqueness for server's channel
       resources mapping (uses client locators as keys).
       Open and bind a socket to obtain a unique port. This unique port is assigned to to PDP passed locators.
       This process ensures uniqueness in the server's channel resources mapping, which uses client locators as keys.
       Although differing from the real client socket local port, provides a reliable mapping mechanism.
     */
    initial_peer_local_locator_socket_ = std::unique_ptr<asio::ip::tcp::socket>(new asio::ip::tcp::socket(io_service_));
    initial_peer_local_locator_socket_->open(generate_protocol());

    // Binding to port 0 delegates the port selection to the system.
    initial_peer_local_locator_socket_->bind(asio::ip::tcp::endpoint(generate_protocol(), 0));

    ip::tcp::endpoint local_endpoint = initial_peer_local_locator_socket_->local_endpoint();
    initial_peer_local_locator_port_ = local_endpoint.port();

    // Get non_blocking_send property
    if (properties)
    {
        auto s_non_blocking_send = eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(*properties,
                        "fastdds.tcp_transport.non_blocking_send");
        non_blocking_send_ = s_non_blocking_send && *s_non_blocking_send == "true"? true : false;
    }

    // Check system buffer sizes.
    if (configuration()->sendBufferSize == 0)
    {
        socket_base::send_buffer_size option;
        initial_peer_local_locator_socket_->get_option(option);
        set_send_buffer_size(option.value());

        if (configuration()->sendBufferSize < s_minimumSocketBuffer)
        {
            set_send_buffer_size(s_minimumSocketBuffer);
        }
    }

    if (configuration()->receiveBufferSize == 0)
    {
        socket_base::receive_buffer_size option;
        initial_peer_local_locator_socket_->get_option(option);
        set_receive_buffer_size(option.value());

        if (configuration()->receiveBufferSize < s_minimumSocketBuffer)
        {
            set_receive_buffer_size(s_minimumSocketBuffer);
        }
    }

    if (configuration()->maxMessageSize > s_maximumMessageSize)
    {
        EPROSIMA_LOG_ERROR(RTCP_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (configuration()->maxMessageSize > configuration()->sendBufferSize)
    {
        EPROSIMA_LOG_ERROR(RTCP_MSG_OUT, "maxMessageSize cannot be greater than send_buffer_size");
        return false;
    }

    if (configuration()->maxMessageSize > configuration()->receiveBufferSize)
    {
        EPROSIMA_LOG_ERROR(RTCP_MSG_OUT, "maxMessageSize cannot be greater than receive_buffer_size");
        return false;
    }

    if (!rtcp_message_manager_)
    {
        rtcp_message_manager_ = std::make_shared<RTCPMessageManager>(this);
    }

    // TODO(Ricardo) Create an event that update this list.
    get_ips(current_interfaces_);

    auto ioServiceFunction = [&]()
            {
#if ASIO_VERSION >= 101200
                asio::executor_work_guard<asio::io_service::executor_type> work(io_service_.get_executor());
#else
                io_service::work work(io_service_);
#endif // if ASIO_VERSION >= 101200
                io_service_.run();
            };
    io_service_thread_ = create_thread(ioServiceFunction, configuration()->accept_thread, "dds.tcp_accept");

    if (0 < configuration()->keep_alive_frequency_ms)
    {
        auto ioServiceTimersFunction = [&]()
                {
#if ASIO_VERSION >= 101200
                    asio::executor_work_guard<asio::io_service::executor_type> work(io_service_timers_.
                                    get_executor());
#else
                    io_service::work work(io_service_timers_);
#endif // if ASIO_VERSION >= 101200
                    io_service_timers_.run();
                };
        io_service_timers_thread_ = create_thread(ioServiceTimersFunction,
                        configuration()->keep_alive_thread, "dds.tcp_keep");
    }

    return true;
}

bool TCPTransportInterface::is_input_port_open(
        uint16_t port) const
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    return receiver_resources_.find(port) != receiver_resources_.end();
}

bool TCPTransportInterface::IsInputChannelOpen(
        const Locator& locator) const
{
    return IsLocatorSupported(locator) && is_input_port_open(IPLocator::getLogicalPort(locator));
}

bool TCPTransportInterface::IsLocatorSupported(
        const Locator& locator) const
{
    return locator.kind == transport_kind_;
}

bool TCPTransportInterface::is_output_channel_open_for(
        const Locator& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);

    // Check if there is any socket opened with the given locator.
    auto channel_resource = channel_resources_.find(IPLocator::toPhysicalLocator(locator));

    if (channel_resource != channel_resources_.end())
    {
        // And it is registered as output logical port
        return channel_resource->second->is_logical_port_added(IPLocator::getLogicalPort(locator));
    }

    return false;
}

Locator TCPTransportInterface::RemoteToMainLocal(
        const Locator& remote) const
{
    if (!IsLocatorSupported(remote))
    {
        return false;
    }

    Locator mainLocal(remote);
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

bool TCPTransportInterface::transform_remote_locator(
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
                IPLocator::setPhysicalPort(result_locator, IPLocator::getPhysicalPort(remote_locator));
                IPLocator::setLogicalPort(result_locator, IPLocator::getLogicalPort(remote_locator));
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

void TCPTransportInterface::CloseOutputChannel(
        fastrtps::rtps::Locator_t& locator)
{
    locator.set_Invalid_Address();
    locator.port = 0;
}

bool TCPTransportInterface::CloseInputChannel(
        const Locator& locator)
{
    bool bClosed = false;
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);

        uint16_t logicalPort = IPLocator::getLogicalPort(locator);
        auto receiverIt = receiver_resources_.find(logicalPort);
        if (receiverIt != receiver_resources_.end())
        {
            bClosed = true;
            ReceiverInUseCV* receiver_in_use = receiverIt->second.second;
            receiver_resources_.erase(receiverIt);

            // Inform all channel resources that logical port has been closed
            for (auto channelIt : channel_resources_)
            {
                if (channelIt.second->connection_established())
                {
                    rtcp_message_manager_->sendLogicalPortIsClosedRequest(channelIt.second, logicalPort);
                }
            }

            receiver_in_use->cv.wait(scopedLock, [&]()
                    {
                        return receiver_in_use->in_use == 0;
                    });
            delete receiver_in_use;
        }
    }

    return bClosed;
}

void TCPTransportInterface::close_tcp_socket(
        std::shared_ptr<TCPChannelResource>& channel)
{
    channel->disconnect();
    // channel.reset(); lead to race conditions because TransportInterface functions used in the callbacks doesn't check validity.
}

bool TCPTransportInterface::OpenOutputChannel(
        SendResourceList& send_resource_list,
        const Locator& locator)
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    uint16_t logical_port = IPLocator::getLogicalPort(locator);
    if (0 == logical_port)
    {
        return false;
    }

    Locator physical_locator = IPLocator::toPhysicalLocator(locator);

    // We try to find a SenderResource that has this locator.
    // Note: This is done in this level because if we do in NetworkFactory level, we have to mantain what transport
    // already reuses a SenderResource.
    for (auto& sender_resource : send_resource_list)
    {
        TCPSenderResource* tcp_sender_resource = TCPSenderResource::cast(*this, sender_resource.get());

        if (tcp_sender_resource && (physical_locator == tcp_sender_resource->locator() ||
                (IPLocator::hasWan(locator) &&
                IPLocator::WanToLanLocator(physical_locator) ==
                tcp_sender_resource->locator())))
        {
            // If missing, logical port will be added in first send()
            statistics_info_.add_entry(locator);
            return true;
        }
    }

    // At this point, if there is no SenderResource to reuse, this is the first call to OpenOutputChannel for this locator.
    // Need to check if a channel already exists for this locator.

    EPROSIMA_LOG_INFO(RTCP, "Called to OpenOutputChannel (physical: " << IPLocator::getPhysicalPort(
                locator) << "; logical: "
                                                                      << IPLocator::getLogicalPort(
                locator) << ") @ " << IPLocator::to_string(locator));

    std::lock_guard<std::mutex> socketsLock(sockets_map_mutex_);
    auto channel_resource = channel_resources_.find(physical_locator);

    // Maybe as WAN?
    if (channel_resource == channel_resources_.end() && IPLocator::hasWan(locator))
    {
        Locator wan_locator = IPLocator::WanToLanLocator(locator);
        channel_resource = channel_resources_.find(IPLocator::toPhysicalLocator(wan_locator));
        if (channel_resource != channel_resources_.end())
        {
            channel_resources_[physical_locator] = channel_resource->second;     // Add alias!
        }
    }

    // (Server-Client Topology OR LARGE DATA with PDP discovery after TCP connection) - Server side
    if (channel_resource != channel_resources_.end())
    {
        std::shared_ptr<TCPChannelResource> channel;
        // There is an existing channel in channel_resources_ created for reception with the remote locator as key. Use it.
        channel = channel_resource->second;
        // Add logical port to channel if it's not there yet
        channel->add_logical_port(logical_port, rtcp_message_manager_.get());
    }
    // (Server-Client Topology - Client Side) OR LARGE DATA Topology with PDP discovery before TCP connection
    else
    {
        // Get listening port (0 if client)
        uint16_t listening_port = 0;
        const TCPTransportDescriptor* config = configuration();
        assert (config != nullptr);
        if (!config->listening_ports.empty())
        {
            listening_port = config->listening_ports.front();
        }

        // If the remote physical port is higher than our listening port, a new CONNECT channel needs to be created and connected
        // and the locator added to the send_resource_list.
        // If the remote physical port is lower than our listening port, only the locator needs to be added to the send_resource_list.
        if (IPLocator::getPhysicalPort(physical_locator) > listening_port)
        {
            // Client side (either Server-Client or LARGE_DATA)
            EPROSIMA_LOG_INFO(OpenOutputChannel, "OpenOutputChannel: [CONNECT] (physical: "
                    << IPLocator::getPhysicalPort(locator) << "; logical: "
                    << IPLocator::getLogicalPort(locator) << ") @ " << IPLocator::to_string(locator));

            // Create a TCP_CONNECT_TYPE channel
            std::shared_ptr<TCPChannelResource> channel(
#if TLS_FOUND
                (configuration()->apply_security) ?
                static_cast<TCPChannelResource*>(
                    new TCPChannelResourceSecure(this, io_service_, ssl_context_,
                    physical_locator, configuration()->maxMessageSize)) :
#endif // if TLS_FOUND
                static_cast<TCPChannelResource*>(
                    new TCPChannelResourceBasic(this, io_service_, physical_locator,
                    configuration()->maxMessageSize))
                );

            channel_resources_[physical_locator] = channel;
            channel->connect(channel_resources_[physical_locator]);
            channel->add_logical_port(logical_port, rtcp_message_manager_.get());
        }
        else
        {
            // Server side LARGE_DATA
            // Act as server and wait to the other endpoint to connect. Add locator to sender_resource_list
            EPROSIMA_LOG_INFO(OpenOutputChannel, "OpenOutputChannel: [WAIT_CONNECTION] (physical: "
                    << IPLocator::getPhysicalPort(locator) << "; logical: "
                    << IPLocator::getLogicalPort(locator) << ") @ " << IPLocator::to_string(locator));
        }
    }

    statistics_info_.add_entry(locator);
    send_resource_list.emplace_back(
        static_cast<SenderResource*>(new TCPSenderResource(*this, physical_locator)));

    return true;
}

bool TCPTransportInterface::OpenInputChannel(
        const Locator& locator,
        TransportReceiverInterface* receiver,
        uint32_t /*maxMsgSize*/)
{
    bool success = false;
    if (IsLocatorSupported(locator))
    {
        uint16_t logicalPort = IPLocator::getLogicalPort(locator);
        if (!is_input_port_open(logicalPort))
        {
            success = true;
            {
                std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
                receiver_resources_[logicalPort] = std::pair<TransportReceiverInterface*, ReceiverInUseCV*>
                            (receiver, new ReceiverInUseCV());
            }

            EPROSIMA_LOG_INFO(RTCP, " OpenInputChannel (physical: " << IPLocator::getPhysicalPort(
                        locator) << "; logical: " << \
                    IPLocator::getLogicalPort(locator) << ")");
        }
    }
    return success;
}

void TCPTransportInterface::keep_alive()
{
    std::map<Locator, std::shared_ptr<TCPChannelResource>> tmp_vec;

    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_); // Why mutex here?
        tmp_vec = channel_resources_;
    }


    for (auto& channel_resource : tmp_vec)
    {
        if (TCPChannelResource::TCPConnectionType::TCP_CONNECT_TYPE == channel_resource.second->tcp_connection_type())
        {
            rtcp_message_manager_->sendKeepAliveRequest(channel_resource.second);
        }
    }
    //TODO Check timeout.

    /*
       const TCPTransportDescriptor* config = configuration(); // Keep a copy for us.

       std::chrono::time_point<std::chrono::system_clock> time_now = std::chrono::system_clock::now();
       std::chrono::time_point<std::chrono::system_clock> next_time = time_now +
        std::chrono::milliseconds(config->keep_alive_frequency_ms);
       std::chrono::time_point<std::chrono::system_clock> timeout_time =
        time_now + std::chrono::milliseconds(config->keep_alive_timeout_ms);

       while (channel && TCPChannelResource::TCPConnectionStatus::TCP_CONNECTED == channel->tcp_connection_status())
       {
        if (channel->connection_established())
        {
            // KeepAlive
            if (config->keep_alive_frequency_ms > 0 && config->keep_alive_timeout_ms > 0)
            {
                time_now = std::chrono::system_clock::now();

                // Keep Alive Management
                if (!channel->waiting_for_keep_alive_ && time_now > next_time)
                {
                    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_); // Why mutex here?
                    rtcp_message_manager_->sendKeepAliveRequest(channel);
                    channel->waiting_for_keep_alive_ = true;
                    next_time = time_now + std::chrono::milliseconds(config->keep_alive_frequency_ms);
                    timeout_time = time_now + std::chrono::milliseconds(config->keep_alive_timeout_ms);
                }
                else if (channel->waiting_for_keep_alive_ && time_now >= timeout_time)
                {
                    // Disable the socket to erase it after the reception.
                    close_tcp_socket(channel);
                }
            }
        }
       }
       EPROSIMA_LOG_INFO(RTCP, "End perform_rtcp_management_thread " << channel->locator());
     */
}

void TCPTransportInterface::create_listening_thread(
        const std::shared_ptr<TCPChannelResource>& channel)
{
    std::weak_ptr<TCPChannelResource> channel_weak_ptr = channel;
    std::weak_ptr<RTCPMessageManager> rtcp_manager_weak_ptr = rtcp_message_manager_;
    auto fn = [this, channel_weak_ptr, rtcp_manager_weak_ptr]()
            {
                perform_listen_operation(channel_weak_ptr, rtcp_manager_weak_ptr);
            };
    uint32_t port = channel->local_endpoint().port();
    const ThreadSettings& thr_config = configuration()->get_thread_config_for_port(port);
    channel->thread(create_thread(fn, thr_config, "dds.tcp.%u", port));
}

void TCPTransportInterface::perform_listen_operation(
        std::weak_ptr<TCPChannelResource> channel_weak,
        std::weak_ptr<RTCPMessageManager> rtcp_manager)
{
    Locator remote_locator;
    uint16_t logicalPort(0);
    std::shared_ptr<RTCPMessageManager> rtcp_message_manager;
    std::shared_ptr<TCPChannelResource> channel;
    rtcp_message_manager = rtcp_manager.lock();

    // RTCP Control Message
    if (rtcp_message_manager)
    {
        channel = channel_weak.lock();

        if (channel)
        {
            remote_locator = remote_endpoint_to_locator(channel);

            uint32_t port = channel->local_endpoint().port();
            set_name_to_current_thread("dds.tcp.%u", port);

            if (channel->tcp_connection_type() == TCPChannelResource::TCPConnectionType::TCP_CONNECT_TYPE)
            {
                rtcp_message_manager->sendConnectionRequest(channel);
            }
            else
            {
                channel->change_status(TCPChannelResource::eConnectionStatus::eWaitingForBind);
            }
        }

        std::unique_lock<std::mutex> lock(rtcp_message_manager_mutex_);
        rtcp_message_manager.reset();
        rtcp_message_manager_cv_.notify_one();
    }
    else
    {
        return;
    }

    while (channel && TCPChannelResource::eConnectionStatus::eConnecting < channel->connection_status())
    {
        // Blocking receive.
        CDRMessage_t& msg = channel->message_buffer();
        fastrtps::rtps::CDRMessage::initCDRMsg(&msg);
        if (!Receive(rtcp_manager, channel, msg.buffer, msg.max_size, msg.length, msg.msg_endian, remote_locator))
        {
            continue;
        }

        if (TCPChannelResource::eConnectionStatus::eConnecting < channel->connection_status())
        {
            // Processes the data through the CDR Message interface.
            logicalPort = IPLocator::getLogicalPort(remote_locator);
            std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
            auto it = receiver_resources_.find(logicalPort);
            if (it != receiver_resources_.end())
            {
                TransportReceiverInterface* receiver = it->second.first;
                ReceiverInUseCV* receiver_in_use = it->second.second;
                receiver_in_use->in_use++;
                scopedLock.unlock();
                receiver->OnDataReceived(msg.buffer, msg.length, channel->locator(), remote_locator);
                scopedLock.lock();
                receiver_in_use->in_use--;
                receiver_in_use->cv.notify_one();
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTCP,
                        "Received Message, but no TransportReceiverInterface attached: " << logicalPort);
            }
        }
    }

    EPROSIMA_LOG_INFO(RTCP, "End PerformListenOperation " << channel->locator());
}

bool TCPTransportInterface::read_body(
        octet* receive_buffer,
        uint32_t,
        uint32_t* bytes_received,
        std::shared_ptr<TCPChannelResource>& channel,
        std::size_t body_size)
{
    asio::error_code ec;

    *bytes_received = channel->read(receive_buffer, body_size, ec);

    if (ec)
    {
        EPROSIMA_LOG_WARNING(RTCP, "Error reading RTCP body: " << ec.message());
        return false;
    }
    else if (*bytes_received != body_size)
    {
        EPROSIMA_LOG_ERROR(RTCP, "Bad RTCP body size: " << *bytes_received << " (expected: " << body_size << ")");
        return false;
    }

    return true;
}

bool receive_header(
        std::shared_ptr<TCPChannelResource>& channel,
        TCPHeader& tcp_header,
        asio::error_code& ec)
{
    // Cleanup header
    octet* ptr = tcp_header.address();
    memset(ptr, 0, sizeof(TCPHeader));

    // Prepare read position
    octet* read_pos = ptr;
    size_t bytes_needed = 4;

    // Wait for sync
    while (bytes_needed > 0)
    {
        size_t bytes_read = channel->read(read_pos, bytes_needed, ec);
        if (bytes_read > 0)
        {
            read_pos += bytes_read;
            bytes_needed -= bytes_read;
            if (0 == bytes_needed)
            {
                size_t skip =                                 // Text   Next possible match   Skip to next match
                        (tcp_header.rtcp[0] != 'R') ? 1 :     // X---   XRTCP                 1
                        (tcp_header.rtcp[1] != 'T') ? 1 :     // RX--   RRTCP                 1
                        (tcp_header.rtcp[2] != 'C') ? 2 :     // RTX-   RTRTCP                2
                        (tcp_header.rtcp[3] != 'P') ? 3 : 0;  // RTCX   RTCRTCP               3

                if (skip)
                {
                    memmove(ptr, &ptr[skip], 4 - skip);
                }

                read_pos -= skip;
                bytes_needed = skip;
            }
        }
        else if (ec)
        {
            return false;
        }
        else if (!channel->connection_status())
        {
            return false;
        }
    }

    bytes_needed = TCPHeader::size() - 4;
    while (bytes_needed > 0)
    {
        size_t bytes_read = channel->read(read_pos, bytes_needed, ec);
        if (bytes_read > 0)
        {
            read_pos += bytes_read;
            bytes_needed -= bytes_read;
        }
        else if (ec)
        {
            return false;
        }
        else if (!channel->connection_status())
        {
            return false;
        }
    }

    return true;
}

/**
 * On TCP, we must receive the header (14 Bytes) and then,
 * the rest of the message, whose length is on the header.
 * TCP Header is transparent to the caller, so receive_buffer
 * doesn't include it.
 * */
bool TCPTransportInterface::Receive(
        std::weak_ptr<RTCPMessageManager>& rtcp_manager,
        std::shared_ptr<TCPChannelResource>& channel,
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        fastrtps::rtps::Endianness_t msg_endian,
        Locator& remote_locator)
{
    bool success = false;

    try
    {
        success = true;

        // Read the header
        //octet header[TCPHEADER_SIZE];
        TCPHeader tcp_header;
        asio::error_code ec;

        bool header_found = false;

        do
        {
            header_found = receive_header(channel, tcp_header, ec);
        } while (!header_found && !ec && channel->connection_status());

        if (ec)
        {
            if (ec != asio::error::eof)
            {
                EPROSIMA_LOG_WARNING(DEBUG, "Failed to read TCP header: " << ec.message());
            }
            close_tcp_socket(channel);
            success = false;
        }
        else if (!channel->connection_status())
        {
            EPROSIMA_LOG_WARNING(DEBUG, "Failed to read TCP header: channel disconnected while reading.");
            success = false;
        }
        else
        {
            tcp_header.valid_endianness(msg_endian);

            size_t body_size = tcp_header.length - static_cast<uint32_t>(TCPHeader::size());

            if (body_size > receive_buffer_capacity)
            {
                EPROSIMA_LOG_ERROR(RTCP_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                        << static_cast<uint32_t>(body_size) << " vs. " << receive_buffer_capacity << ". "
                        << "The full message will be dropped.");
                success = false;
                // Drop the message
                size_t to_read = body_size;
                size_t read_block = receive_buffer_capacity;
                uint32_t readed;
                while (read_block > 0)
                {
                    read_body(receive_buffer, receive_buffer_capacity, &readed, channel,
                            read_block);
                    to_read -= readed;
                    read_block = (to_read >= receive_buffer_capacity) ? receive_buffer_capacity : to_read;
                }
            }
            else
            {
                EPROSIMA_LOG_INFO(RTCP_MSG_IN, "Received RTCP MSG. Logical Port " << tcp_header.logical_port);
                success = read_body(receive_buffer, receive_buffer_capacity, &receive_buffer_size,
                                channel, body_size);

                if (success)
                {
                    if (configuration()->check_crc
                            && !check_crc(tcp_header, receive_buffer, receive_buffer_size))
                    {
                        EPROSIMA_LOG_WARNING(RTCP_MSG_IN, "Bad TCP header CRC");
                    }

                    if (tcp_header.logical_port == 0)
                    {
                        std::shared_ptr<RTCPMessageManager> rtcp_message_manager;
                        if (TCPChannelResource::eConnectionStatus::eDisconnected != channel->connection_status())

                        {
                            std::unique_lock<std::mutex> lock(rtcp_message_manager_mutex_);
                            rtcp_message_manager = rtcp_manager.lock();
                        }

                        if (rtcp_message_manager)
                        {
                            // The channel is not going to be deleted because we lock it for reading.
                            ResponseCode responseCode = rtcp_message_manager->processRTCPMessage(
                                channel, receive_buffer, body_size, msg_endian);

                            if (responseCode != RETCODE_OK)
                            {
                                close_tcp_socket(channel);
                            }
                            success = false;

                            std::unique_lock<std::mutex> lock(rtcp_message_manager_mutex_);
                            rtcp_message_manager.reset();
                            rtcp_message_manager_cv_.notify_one();
                        }
                        else
                        {
                            success = false;
                            close_tcp_socket(channel);
                        }

                    }
                    else
                    {
                        if (!IsLocatorValid(remote_locator))
                        {
                            remote_locator = remote_endpoint_to_locator(channel);
                        }
                        IPLocator::setLogicalPort(remote_locator, tcp_header.logical_port);
                        EPROSIMA_LOG_INFO(RTCP_MSG_IN, "[RECEIVE] From: " << remote_locator \
                                                                          << " - " << receive_buffer_size << " bytes.");
                    }
                }
                // Error message already shown by read_body method.
            }
        }
    }
    catch (const asio::error_code& code)
    {
        if ((code == asio::error::eof) || (code == asio::error::connection_reset))
        {
            // Close the channel
            EPROSIMA_LOG_ERROR(RTCP_MSG_IN, "ASIO [RECEIVE]: " << code.message());
            //channel->ConnectionLost();
            close_tcp_socket(channel);
        }
        success = false;
    }
    catch (const asio::system_error& error)
    {
        (void)error;
        // Close the channel
        EPROSIMA_LOG_ERROR(RTCP_MSG_IN, "ASIO SYSTEM_ERROR [RECEIVE]: " << error.what());
        //channel->ConnectionLost();
        close_tcp_socket(channel);
        success = false;
    }

    success = success && receive_buffer_size > 0;

    return success;
}

bool TCPTransportInterface::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const fastrtps::rtps::Locator_t& locator,
        fastrtps::rtps::LocatorsIterator* destination_locators_begin,
        fastrtps::rtps::LocatorsIterator* destination_locators_end)
{
    fastrtps::rtps::LocatorsIterator& it = *destination_locators_begin;

    bool ret = true;

    while (it != *destination_locators_end)
    {
        if (IsLocatorSupported(*it))
        {
            ret &= send(send_buffer, send_buffer_size, locator, *it);
        }

        ++it;
    }

    return ret;
}

bool TCPTransportInterface::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const fastrtps::rtps::Locator_t& locator,
        const Locator& remote_locator)
{
    using namespace eprosima::fastdds::statistics::rtps;

    bool locator_mismatch = false;

    if (locator != IPLocator::toPhysicalLocator(remote_locator))
    {
        // Locator of sender resource does not match remote locator (from LocatorSelector)
        locator_mismatch = true;
    }

    // Maybe is WAN?
    if (locator_mismatch && IPLocator::hasWan(remote_locator))
    {
        Locator wan_locator = IPLocator::WanToLanLocator(remote_locator);
        wan_locator = IPLocator::toPhysicalLocator(wan_locator);
        if (locator == wan_locator)
        {
            locator_mismatch = false;
        }
    }

    if (locator_mismatch || send_buffer_size > configuration()->sendBufferSize)
    {
        return false;
    }

    bool success = false;

    std::lock_guard<std::mutex> scoped_lock(sockets_map_mutex_);
    auto channel_resource = channel_resources_.find(locator);
    if (channel_resource == channel_resources_.end())
    {
        // There is no channel for this locator. Skip send
        return false;
    }

    auto channel = channel_resource->second;

    /* TODO Verify when cable is removed
       if(TCPChannelResource::TCPConnectionStatus::TCP_DISCONNECTED == channel->tcp_connection_status() &&
        TCPChannelResource::TCPConnectionType::TCP_ACCEPT_TYPE == channel->tcp_connection_type())
       {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        auto channel_resource = channel_resources_.find(channel->locator());
        if (channel_resource != channel_resources_.end() && channel != channel_resource->second)
        {
            channel = channel_resource->second;
        }
       }*/

    if (channel->connection_established())
    {
        uint16_t logical_port = IPLocator::getLogicalPort(remote_locator);

        if (channel->is_logical_port_added(logical_port))
        {
            if (channel->is_logical_port_opened(logical_port))
            {
                TCPHeader tcp_header;
                statistics_info_.set_statistics_message_data(remote_locator, send_buffer, send_buffer_size);
                fill_rtcp_header(tcp_header, send_buffer, send_buffer_size, logical_port);

                {
                    asio::error_code ec;
                    size_t sent = channel->send(
                        (octet*)&tcp_header,
                        static_cast<uint32_t>(TCPHeader::size()),
                        send_buffer,
                        send_buffer_size,
                        ec);

                    if (sent != static_cast<uint32_t>(TCPHeader::size() + send_buffer_size) || ec)
                    {
                        EPROSIMA_LOG_WARNING(DEBUG, "Failed to send RTCP message (" << sent << " of " <<
                                TCPHeader::size() + send_buffer_size << " b): " << ec.message());
                        success = false;
                    }
                    else
                    {
                        success = true;
                    }
                }
            }
        }
        else
        {
            channel->add_logical_port(logical_port, rtcp_message_manager_.get());
        }
    }
    else if (TCPChannelResource::TCPConnectionType::TCP_CONNECT_TYPE == channel->tcp_connection_type() &&
            TCPChannelResource::eConnectionStatus::eDisconnected == channel->connection_status())
    {
        channel->set_all_ports_pending();
        channel->connect(channel_resources_[channel->locator()]);
    }

    return success;
}

void TCPTransportInterface::select_locators(
        LocatorSelector& selector) const
{
    fastrtps::ResourceLimitedVector<LocatorSelectorEntry*>& entries =  selector.transport_starts();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        LocatorSelectorEntry* entry = entries[i];
        if (entry->transport_should_process)
        {
            bool selected = false;
            for (size_t j = 0; j < entry->unicast.size(); ++j)
            {
                if (IsLocatorSupported(entry->unicast[j]) && !selector.is_selected(entry->unicast[j]))
                {
                    entry->state.unicast.push_back(j);
                    selected = true;
                }
            }

            if (selected)
            {
                selector.select(i);
            }
        }
    }
}

void TCPTransportInterface::SocketAccepted(
        std::shared_ptr<asio::ip::tcp::socket> socket,
        const Locator& locator,
        const asio::error_code& error)
{
    if (alive_.load())
    {
        if (!error.value())
        {
            // Always create a new channel, it might be replaced later in bind_socket()
            std::shared_ptr<TCPChannelResource> channel(new TCPChannelResourceBasic(this,
                    io_service_, socket, configuration()->maxMessageSize));

            {
                std::unique_lock<std::mutex> unbound_lock(unbound_map_mutex_);
                unbound_channel_resources_.push_back(channel);
            }

            channel->set_options(configuration());
            create_listening_thread(channel);

            EPROSIMA_LOG_INFO(RTCP, "Accepted connection (local: "
                    << channel->local_endpoint().address() << ":"
                    << channel->local_endpoint().port() << "), remote: "
                    << channel->remote_endpoint().address() << ":"
                    << channel->remote_endpoint().port() << ")");
        }
        else
        {
            EPROSIMA_LOG_INFO(RTCP, "Accepting connection (" << error.message() << ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait a little to accept again.
        }

        if (error.value() != eSocketErrorCodes::eConnectionAborted) // Operation Aborted
        {
            std::shared_ptr<TCPAcceptor> acceptor = acceptors_[locator];
            if (acceptor != nullptr)
            {
                dynamic_cast<TCPAcceptorBasic*>(acceptor.get())->accept(this);
            }
        }
    }
}

#if TLS_FOUND
void TCPTransportInterface::SecureSocketAccepted(
        std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> socket,
        const Locator& locator,
        const asio::error_code& error)
{
    if (alive_.load())
    {
        if (!error.value())
        {
            // Always create a new secure_channel, it might be replaced later in bind_socket()
            std::shared_ptr<TCPChannelResource> secure_channel(new TCPChannelResourceSecure(this,
                    io_service_, ssl_context_, socket, configuration()->maxMessageSize));

            {
                std::unique_lock<std::mutex> unbound_lock(unbound_map_mutex_);
                unbound_channel_resources_.push_back(secure_channel);
            }

            secure_channel->set_options(configuration());
            create_listening_thread(secure_channel);

            EPROSIMA_LOG_INFO(RTCP, " Accepted connection (local: "
                    << socket->lowest_layer().local_endpoint().address() << ":"
                    << socket->lowest_layer().local_endpoint().port() << "), remote: "
                    << socket->lowest_layer().remote_endpoint().address() << ":"
                    << socket->lowest_layer().remote_endpoint().port() << ")");
        }
        else
        {
            EPROSIMA_LOG_INFO(RTCP, " Accepting connection (" << error.message() << ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait a little to accept again.
        }

        if (error.value() != eSocketErrorCodes::eConnectionAborted) // Operation Aborted
        {
            std::shared_ptr<TCPAcceptor> acceptor = acceptors_[locator];
            if (acceptor != nullptr)
            {
                dynamic_cast<TCPAcceptorSecure*>(acceptor.get())->accept(this, ssl_context_);
            }
        }
    }
}

#endif // if TLS_FOUND

void TCPTransportInterface::SocketConnected(
        const std::weak_ptr<TCPChannelResource>& channel_weak_ptr,
        const asio::error_code& error)
{
    if (alive_.load())
    {
        auto channel = channel_weak_ptr.lock();

        if (channel)
        {
            if (!error)
            {
                if (TCPChannelResource::eConnectionStatus::eDisconnected < channel->connection_status())
                {
                    channel->change_status(TCPChannelResource::eConnectionStatus::eConnected);
                    channel->set_options(configuration());
                    create_listening_thread(channel);
                }
            }
            else
            {
                channel->disconnect();
            }
        }
    }
}

bool TCPTransportInterface::getDefaultMetatrafficMulticastLocators(
        LocatorList&,
        uint32_t ) const
{
    // TCP doesn't have multicast support
    return true;
}

bool TCPTransportInterface::getDefaultMetatrafficUnicastLocators(
        LocatorList& locators,
        uint32_t metatraffic_unicast_port) const
{
    Locator locator;
    locator.kind = transport_kind_;
    locator.set_Invalid_Address();
    fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);
    locators.push_back(locator);
    return true;
}

bool TCPTransportInterface::getDefaultUnicastLocators(
        LocatorList& locators,
        uint32_t unicast_port) const
{
    Locator locator;
    locator.kind = transport_kind_;
    locator.set_Invalid_Address();
    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);
    return true;
}

bool TCPTransportInterface::fillMetatrafficMulticastLocator(
        Locator&,
        uint32_t) const
{
    // TCP doesn't have multicast support
    return true;
}

bool TCPTransportInterface::fillMetatrafficUnicastLocator(
        Locator& locator,
        uint32_t metatraffic_unicast_port) const
{
    if (IPLocator::getPhysicalPort(locator.port) == 0)
    {
        fill_local_physical_port(locator);
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(metatraffic_unicast_port));
    }

    return true;
}

bool TCPTransportInterface::configureInitialPeerLocator(
        Locator& locator,
        const PortParameters& port_params,
        uint32_t domainId,
        LocatorList& list) const
{
    if (IPLocator::getPhysicalPort(locator) == 0)
    {
        for (uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
        {
            Locator auxloc(locator);
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
            for (uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
            {
                Locator auxloc(locator);
                IPLocator::setLogicalPort(auxloc, static_cast<uint16_t>(port_params.getUnicastPort(domainId, i)));
                list.push_back(auxloc);
            }
        }
        else
        {
            list.push_back(locator);
        }
    }

    return true;
}

bool TCPTransportInterface::fillUnicastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    if (IPLocator::getPhysicalPort(locator.port) == 0)
    {
        fill_local_physical_port(locator);
    }

    if (IPLocator::getLogicalPort(locator) == 0)
    {
        IPLocator::setLogicalPort(locator, static_cast<uint16_t>(well_known_port));
    }

    return true;
}

void TCPTransportInterface::shutdown()
{
}

bool TCPTransportInterface::apply_tls_config()
{
#if TLS_FOUND
    const TCPTransportDescriptor* descriptor = configuration();
    if (descriptor->apply_security)
    {
        ssl_context_.set_verify_callback([](bool preverified, ssl::verify_context&)
                {
                    return preverified;
                });

        const TCPTransportDescriptor::TLSConfig* config = &descriptor->tls_config;
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;

        if (!config->password.empty())
        {
            ssl_context_.set_password_callback(std::bind(&TCPTransportInterface::get_password, this));
        }

        if (!config->verify_file.empty())
        {
            try
            {
                ssl_context_.load_verify_file(config->verify_file);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(TLS, "Error configuring TLS trusted CA certificate: " << e.what());
                return false; // TODO check wether this should skip the rest of the configuration
            }
        }

        if (!config->cert_chain_file.empty())
        {
            try
            {
                ssl_context_.use_certificate_chain_file(config->cert_chain_file);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(TLS, "Error configuring TLS certificate: " << e.what());
                return false; // TODO check wether this should skip the rest of the configuration
            }
        }

        if (!config->private_key_file.empty())
        {
            try
            {
                ssl_context_.use_private_key_file(config->private_key_file, ssl::context::pem);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(TLS, "Error configuring TLS private key: " << e.what());
                return false; // TODO check wether this should skip the rest of the configuration
            }
        }

        if (!config->tmp_dh_file.empty())
        {
            try
            {
                ssl_context_.use_tmp_dh_file(config->tmp_dh_file);
            }
            catch (const std::exception& e)
            {
                EPROSIMA_LOG_ERROR(TLS, "Error configuring TLS dh params: " << e.what());
                return false; // TODO check wether this should skip the rest of the configuration
            }
        }

        if (!config->verify_paths.empty())
        {
            for (const std::string& path : config->verify_paths)
            {
                ssl_context_.add_verify_path(path);
            }
        }

        if (config->default_verify_path)
        {
            ssl_context_.set_default_verify_paths();
        }

        if (config->verify_depth >= 0)
        {
            ssl_context_.set_verify_depth(config->verify_depth);
        }

        if (!config->rsa_private_key_file.empty())
        {
            ssl_context_.use_private_key_file(config->rsa_private_key_file, ssl::context::pem);
        }

        if (config->options != TLSOptions::NONE)
        {
            uint32_t options = 0;

            if (config->get_option(TLSOptions::DEFAULT_WORKAROUNDS))
            {
                options |= ssl::context::default_workarounds;
            }

            if (config->get_option(TLSOptions::NO_COMPRESSION))
            {
                options |= ssl::context::no_compression;
            }

            if (config->get_option(TLSOptions::NO_SSLV2))
            {
                options |= ssl::context::no_sslv2;
            }
            else
            {
                EPROSIMA_LOG_WARNING(TLS, "Allowing SSL 2.0. This version has known vulnerabilities.");
            }

            if (config->get_option(TLSOptions::NO_SSLV3))
            {
                options |= ssl::context::no_sslv3;
            }

            if (config->get_option(TLSOptions::NO_TLSV1))
            {
                options |= ssl::context::no_tlsv1;
            }

            if (config->get_option(TLSOptions::NO_TLSV1_1))
            {
                options |= ssl::context::no_tlsv1_1;
            }

            if (config->get_option(TLSOptions::NO_TLSV1_2))
            {
                options |= ssl::context::no_tlsv1_2;
            }

#if ASIO_VERSION >= 106900 // no_tlsv1_3 added in asio 1.69
            if (config->get_option(TLSOptions::NO_TLSV1_3))
            {
                options |= ssl::context::no_tlsv1_3;
            }
#endif // if ASIO_VERSION >= 106900

            if (config->get_option(TLSOptions::SINGLE_DH_USE))
            {
                options |= ssl::context::single_dh_use;
            }

            ssl_context_.set_options(options);
        }
    }
#endif // if TLS_FOUND
    return true;
}

std::string TCPTransportInterface::get_password() const
{
    return configuration()->tls_config.password;
}

void TCPTransportInterface::update_network_interfaces()
{
    // TODO(jlbueno)
}

bool TCPTransportInterface::is_localhost_allowed() const
{
    Locator local_locator;
    fill_local_ip(local_locator);
    return is_locator_allowed(local_locator);
}

void TCPTransportInterface::fill_local_physical_port(
        Locator& locator) const
{
    if (!configuration()->listening_ports.empty())
    {
        IPLocator::setPhysicalPort(locator, *(configuration()->listening_ports.begin()));
    }
    else
    {
        IPLocator::setPhysicalPort(locator, initial_peer_local_locator_port_);
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
