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
#include <asio/ssl.hpp>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/System.h>
#include <fastrtps/transport/TCPChannelResourceBasic.h>
#include <fastrtps/transport/TCPChannelResourceSecure.h>
#include <fastrtps/transport/TCPAcceptorBasic.h>
#include <fastrtps/transport/TCPAcceptorSecure.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static const int s_default_keep_alive_frequency = 50000; // 50 SECONDS
static const int s_default_keep_alive_timeout = 10000; // 10 SECONDS
static const int s_clean_deleted_sockets_pool_timeout = 100; // 100 MILLISECONDS
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

TCPTransportDescriptor::TCPTransportDescriptor(const TCPTransportDescriptor& t)
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
{
}

TCPTransportDescriptor& TCPTransportDescriptor::operator=(const TCPTransportDescriptor& t)
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
    return *this;
}

TCPTransportInterface::TCPTransportInterface()
    : ssl_context_(asio::ssl::context::sslv23)
    , rtcp_message_manager_(nullptr)
    , send_retry_active_(true)
    , clean_sockets_pool_timer_(nullptr)
{
}

TCPTransportInterface::~TCPTransportInterface()
{
}

void TCPTransportInterface::clean()
{
    std::vector<TCPChannelResource*> vDeletedSockets;

    if (clean_sockets_pool_timer_ != nullptr)
    {
        clean_sockets_pool_timer_->cancel_timer();
        delete clean_sockets_pool_timer_;
        clean_sockets_pool_timer_ = nullptr;
    }

    // Collect all the existing sockets to delete them outside of the mutex.
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        for (auto it = socket_Acceptors_.begin(); it != socket_Acceptors_.end(); ++it)
        {
            for (TCPAcceptor* acceptorIt : it->second)
            {
                deleted_acceptors_.push_back(acceptorIt);
                delete acceptorIt;
            }
        }
        socket_Acceptors_.clear();

        for (auto it = channel_resources_.begin(); it != channel_resources_.end(); ++it)
        {
            auto delIt = std::find(vDeletedSockets.begin(), vDeletedSockets.end(), it->second);
            if (delIt == vDeletedSockets.end())
            {
                vDeletedSockets.push_back(it->second);
            }
            if (it->second->connection_established())
            {
                rtcp_message_manager_->sendUnbindConnectionRequest(it->second);
            }
        }
        channel_resources_.clear();

        vDeletedSockets.insert(vDeletedSockets.end(), unbound_channel_resources_.begin(),
            unbound_channel_resources_.end());
        unbound_channel_resources_.clear();
    }

    std::for_each(vDeletedSockets.begin(), vDeletedSockets.end(), [this](TCPChannelResource* it)
    {
        this->DeleteSocket(it); // Disable all added TCPChannelResources
    });

    clean_deleted_sockets();

    if (io_service_thread_)
    {
        io_service_.stop();
        io_service_thread_->join();
    }

    delete rtcp_message_manager_;
}

TCPChannelResource* TCPTransportInterface::BindSocket(const Locator_t& locator, TCPChannelResource *p_channel_resource)
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    if (IsLocatorSupported(locator))
    {
        auto it_remove = std::find(unbound_channel_resources_.begin(), unbound_channel_resources_.end(), p_channel_resource);
        if (it_remove != unbound_channel_resources_.end())
        {
            unbound_channel_resources_.erase(it_remove);
        }

        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto it = channel_resources_.find(physicalLocator);
        if (it == channel_resources_.end())
        {
            channel_resources_[physicalLocator] = p_channel_resource;
            return nullptr;
        }

        TCPChannelResource* oldChannel = it->second;
        if (oldChannel->connection_established())
        {
            logWarning(RTCP, "Trying to restablish connection on already connected locator." << locator);
        }
        else
        {
            channel_resources_[physicalLocator] = p_channel_resource;
            return oldChannel;
        }
    }
    return nullptr;
}

void TCPTransportInterface::clean_deleted_sockets()
{
    std::vector<TCPChannelResource*> deleteList;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(deleted_sockets_pool_mutex_);
        deleteList = std::move(deleted_sockets_pool_);
    }

    for (auto it = deleteList.begin(); it != deleteList.end(); ++it)
    {
        delete(*it);
    }
}

void TCPTransportInterface::DeleteSocket(TCPChannelResource *channelResource)
{
    if (channelResource != nullptr && channelResource->alive())
    {
        channelResource->disable();
        {
            std::unique_lock<std::recursive_mutex> scopedPoolLock(deleted_sockets_pool_mutex_);
            auto it = std::find(deleted_sockets_pool_.begin(), deleted_sockets_pool_.end(), channelResource);
            if (it == deleted_sockets_pool_.end())
            {
                deleted_sockets_pool_.emplace_back(channelResource);
            }
        }
    }
}

bool TCPTransportInterface::check_crc(const TCPHeader &header, const octet *data, uint32_t size) const
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    return crc == header.crc;
}

void TCPTransportInterface::calculate_crc(TCPHeader &header, const octet *data, uint32_t size) const
{
    uint32_t crc(0);
    for (uint32_t i = 0; i < size; ++i)
    {
        crc = RTCPMessageManager::addToCRC(crc, data[i]);
    }
    header.crc = crc;
}


bool TCPTransportInterface::create_acceptor_socket(const Locator_t& locator)
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    try
    {
        TCPAcceptor* newAcceptor(nullptr);

        if (is_interface_whitelist_empty())
        {
            if (configuration()->apply_security)
            {
                newAcceptor = new TCPAcceptorSecure(io_service_, ssl_context_, this, locator);
            }
            else
            {
                newAcceptor = new TCPAcceptorBasic(io_service_, this, locator);
            }

            uint16_t port = IPLocator::getPhysicalPort(locator);
            if (socket_Acceptors_.find(port) != socket_Acceptors_.end())
            {
                std::vector<TCPAcceptor*> vAcceptors{ newAcceptor };
                socket_Acceptors_.insert(std::make_pair(port, vAcceptors));
            }
            else if (std::find(socket_Acceptors_[port].begin(), socket_Acceptors_[port].end(),
                    newAcceptor) == socket_Acceptors_[port].end())
            {
                socket_Acceptors_[port].push_back(newAcceptor);
            }

            logInfo(RTCP, " OpenAndBindInput (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: "
                << IPLocator::getLogicalPort(locator) << ")");

            if (configuration()->apply_security)
            {
                static_cast<TCPAcceptorSecure*>(newAcceptor)->Accept(this, io_service_, ssl_context_);
            }
            else
            {
                static_cast<TCPAcceptorBasic*>(newAcceptor)->Accept(this, io_service_);
            }
        }
        else
        {
            std::vector<std::string> vInterfaces = get_binding_interfaces_list();
            for (std::string& sInterface : vInterfaces)
            {
                if (configuration()->apply_security)
                {
                    newAcceptor = new TCPAcceptorSecure(io_service_, ssl_context_, sInterface, locator);
                }
                else
                {
                    newAcceptor = new TCPAcceptorBasic(io_service_, sInterface, locator);
                }

                uint16_t port = IPLocator::getPhysicalPort(locator);
                if (socket_Acceptors_.find(port) != socket_Acceptors_.end())
                {
                    std::vector<TCPAcceptor*> vAcceptors{ newAcceptor };
                    socket_Acceptors_.insert(std::make_pair(port, vAcceptors));
                }
                else if (std::find(socket_Acceptors_[port].begin(), socket_Acceptors_[port].end(),
                        newAcceptor) == socket_Acceptors_[port].end())
                {
                    socket_Acceptors_[port].push_back(newAcceptor);
                }

                logInfo(RTCP, " OpenAndBindInput (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: "
                    << IPLocator::getLogicalPort(locator) << ")");

                if (configuration()->apply_security)
                {
                    static_cast<TCPAcceptorSecure*>(newAcceptor)->Accept(this, io_service_, ssl_context_);
                }
                else
                {
                    static_cast<TCPAcceptorBasic*>(newAcceptor)->Accept(this, io_service_);
                }
            }
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logError(RTCP_MSG_OUT, "TCPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")" << " with msg: " << e.what());
        return false;
    }
    catch (const asio::error_code& code)
    {
        (void)code;
        logError(RTCP, "TCPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")" << " with code: " << code);
        return false;
    }

    return true;
}

bool TCPTransportInterface::enqueue_logical_output_port(const Locator_t& locator)
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    auto socketIt = channel_resources_.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != channel_resources_.end())
    {
        socketIt->second->add_logical_port(IPLocator::getLogicalPort(locator));
        return true;
    }
    return false;
}

void TCPTransportInterface::fill_rtcp_header(TCPHeader& header, const octet* send_buffer, uint32_t send_buffer_size,
        uint16_t logical_port) const
{
    header.length = send_buffer_size + static_cast<uint32_t>(TCPHeader::size());
    header.logical_port = logical_port;
    if (configuration()->calculate_crc)
    {
        calculate_crc(header, send_buffer, send_buffer_size);
    }
}


bool TCPTransportInterface::IsOutputChannelBound(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
        return false;

    auto socket = channel_resources_.find(IPLocator::toPhysicalLocator(locator));
    if (socket != channel_resources_.end())
    {
        return true;
    }

    return false;
}

bool TCPTransportInterface::IsOutputChannelConnected(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
        return false;

    auto socketIt = channel_resources_.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != channel_resources_.end())
    {
        return true;
    }

    return IsOutputChannelBound(locator);
}

bool TCPTransportInterface::is_tcp_input_socket(const Locator_t& locator) const
{
    if (is_local_locator(locator))
    {
        for (auto it = configuration()->listening_ports.begin();
                it != configuration()->listening_ports.end(); ++it)
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
    return compare_locator_ip_and_port(left, right);
}

bool TCPTransportInterface::init()
{
    if (configuration()->sendBufferSize == 0 || configuration()->receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::tcp::socket socket(io_service_);
        socket.open(generate_protocol());

        if (configuration()->sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            set_send_buffer_size(option.value());

            if (configuration()->sendBufferSize < s_minimumSocketBuffer)
            {
                set_send_buffer_size(s_minimumSocketBuffer);
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
            }
        }

        socket.close();
    }

    if (configuration()->maxMessageSize > s_maximumMessageSize)
    {
        logError(RTCP_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (configuration()->maxMessageSize > configuration()->sendBufferSize)
    {
        logError(RTCP_MSG_OUT, "maxMessageSize cannot be greater than send_buffer_size");
        return false;
    }

    if (configuration()->maxMessageSize > configuration()->receiveBufferSize)
    {
        logError(RTCP_MSG_OUT, "maxMessageSize cannot be greater than receive_buffer_size");
        return false;
    }

    if (rtcp_message_manager_ == nullptr)
    {
        rtcp_message_manager_ = new RTCPMessageManager(this);
    }

    // TODO(Ricardo) Create an event that update this list.
    get_ips(current_interfaces_);

    auto ioServiceFunction = [&]()
    {
        io_service::work work(io_service_);
        io_service_.run();
    };
    io_service_thread_.reset(new std::thread(ioServiceFunction));

    clean_sockets_pool_timer_ = new CleanTCPSocketsEvent(this, io_service_, *io_service_thread_.get(),
        s_clean_deleted_sockets_pool_timeout);

    return true;
}

bool TCPTransportInterface::is_input_port_open(uint16_t port) const
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    return receiver_resources_.find(port) != receiver_resources_.end();
}

bool TCPTransportInterface::IsInputChannelOpen(const Locator_t& locator) const
{
    return IsLocatorSupported(locator) && is_input_port_open(IPLocator::getLogicalPort(locator));
}

bool TCPTransportInterface::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == transport_kind_;
}

bool TCPTransportInterface::IsOutputChannelOpen(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);

    // Check if there is any socket opened with the given locator.
    auto socketIt = channel_resources_.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != channel_resources_.end())
    {
        // And it is registered as output logical port
        return socketIt->second->is_logical_port_added(IPLocator::getLogicalPort(locator));
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
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    auto socketIt = channel_resources_.find(IPLocator::toPhysicalLocator(locator));
    if (socketIt != channel_resources_.end())
    {
        return socketIt->second->remove_logical_port(IPLocator::getLogicalPort(locator));
    }
    return false;
}

bool TCPTransportInterface::CloseInputChannel(const Locator_t& locator)
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
                channelIt.second->input_port_closed(logicalPort);
            }

            receiver_in_use->cv.wait(scopedLock, [&]() { return receiver_in_use->in_use == false; });
            delete receiver_in_use;
        }
    }

    return bClosed;
}

void TCPTransportInterface::close_tcp_socket(TCPChannelResource *p_channel_resource)
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);

    // This check has been added because ASIO sends callbacks sometimes when the channel resource has been deleted.
    auto searchIt = std::find_if(channel_resources_.begin(), channel_resources_.end(), [p_channel_resource](const std::pair<Locator_t, TCPChannelResource*>& p)
    {
        return p.second == p_channel_resource;
    });

    if (searchIt != channel_resources_.end() && p_channel_resource->alive() && send_retry_active_)
    {
        TCPChannelResource *newChannel = nullptr;
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(p_channel_resource->locator());
        {
            auto it = channel_resources_.find(physicalLocator);
            if (it != channel_resources_.end())
            {
                if (!p_channel_resource->input_socket())
                {
                    newChannel = new TCPChannelResourceBasic(this, rtcp_message_manager_, io_service_, physicalLocator,
                        configuration()->maxMessageSize);
                    p_channel_resource->set_all_ports_pending();
                    newChannel->copy_pending_ports_from(p_channel_resource);
                }
                channel_resources_.erase(it);
            }
        }

        DeleteSocket(p_channel_resource);

        if (newChannel != nullptr)
        {
            channel_resources_[physicalLocator] = newChannel;
            newChannel->connect();
        }
    }
}


bool TCPTransportInterface::OpenOutputChannel(const Locator_t& locator)
{
    bool success = false;
    uint16_t logicalPort = IPLocator::getLogicalPort(locator);
    if (IsLocatorSupported(locator) && (logicalPort != 0))
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        logInfo(RTCP, "OpenOutputChannel (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: " \
            << IPLocator::getLogicalPort(locator) << ") @ IP: " << IPLocator::toIPv4string(locator));

        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto socketIt = channel_resources_.find(physicalLocator);
        TCPChannelResource* channel = nullptr;
        if (socketIt != channel_resources_.end())
        {
            channel = socketIt->second;
        }
        else
        {
            // Create output channel
            // TODO - Puede eliminarse directamente?
            tcp_basic::eProsimaTCPSocket socket = tcp_basic::createTCPSocket(io_service_);
            channel = new TCPChannelResourceBasic(this, rtcp_message_manager_, io_service_, physicalLocator,
                configuration()->maxMessageSize);
            channel_resources_[physicalLocator] = channel;
            channel->connect();
        }

        success = true;
        channel->add_logical_port(logicalPort);
    }

    return success;
}

bool TCPTransportInterface::OpenExtraOutputChannel(const Locator_t& locator)
{
    return OpenOutputChannel(locator);
}

bool TCPTransportInterface::OpenInputChannel(const Locator_t& locator, TransportReceiverInterface* receiver,
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

            logInfo(RTCP, " OpenInputChannel (physical: " << IPLocator::getPhysicalPort(locator) << "; logical: " << \
                IPLocator::getLogicalPort(locator) << ")");
        }
    }
    return success;
}

void TCPTransportInterface::perform_rtcp_management_thread(TCPChannelResource *p_channel_resource)
{
    std::chrono::time_point<std::chrono::system_clock> time_now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> next_time = time_now +
        std::chrono::milliseconds(configuration()->keep_alive_frequency_ms);
    std::chrono::time_point<std::chrono::system_clock> timeout_time =
        time_now + std::chrono::milliseconds(configuration()->keep_alive_timeout_ms);

/*
    logInfo(RTCP, "START perform_rtcp_management_thread " << IPLocator::toIPv4string(p_channel_resource->locator()) \
            << ":" << IPLocator::getPhysicalPort(p_channel_resource->locator()) << " (" \
            << p_channel_resource->socket()->local_endpoint().address() << ":" \
            << p_channel_resource->socket()->local_endpoint().port() << "->" \
            << p_channel_resource->socket()->remote_endpoint().address() << ":" \
            << p_channel_resource->socket()->remote_endpoint().port() << ")");
*/
    while (p_channel_resource->alive())
    {
        if (p_channel_resource->connection_established())
        {
            // KeepAlive
            if (configuration()->keep_alive_frequency_ms > 0 && configuration()->keep_alive_timeout_ms > 0)
            {
                time_now = std::chrono::system_clock::now();

                // Keep Alive Management
                if (!p_channel_resource->waiting_for_keep_alive_ && time_now > next_time)
                {
                    rtcp_message_manager_->sendKeepAliveRequest(p_channel_resource);
                    p_channel_resource->waiting_for_keep_alive_ = true;
                    next_time = time_now + std::chrono::milliseconds(configuration()->keep_alive_frequency_ms);
                    timeout_time = time_now + std::chrono::milliseconds(configuration()->keep_alive_timeout_ms);
                }
                else if (p_channel_resource->waiting_for_keep_alive_ && time_now >= timeout_time)
                {
                    // Disable the socket to erase it after the reception.
                    close_tcp_socket(p_channel_resource);
                    break;
                }
            }
        }
        eClock::my_sleep(100);
    }
    logInfo(RTCP, "End perform_rtcp_management_thread " << p_channel_resource->locator());
}

void TCPTransportInterface::perform_listen_operation(TCPChannelResource *p_channel_resource)
{
    Locator_t remote_locator;
    uint16_t logicalPort(0);

    while (p_channel_resource->alive())
    {
        // Blocking receive.
        CDRMessage_t& msg = p_channel_resource->message_buffer();
        CDRMessage::initCDRMsg(&msg);
        if (!Receive(p_channel_resource, msg.buffer, msg.max_size, msg.length, remote_locator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        logicalPort = IPLocator::getLogicalPort(remote_locator);
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        auto it = receiver_resources_.find(logicalPort);
        //TransportReceiverInterface* receiver = p_channel_resource->GetMessageReceiver(logicalPort);
        if (it != receiver_resources_.end())
        {
            TransportReceiverInterface* receiver = it->second.first;
            ReceiverInUseCV* receiver_in_use = it->second.second;
            receiver_in_use->in_use = true;
            scopedLock.unlock();
            receiver->OnDataReceived(msg.buffer, msg.length, p_channel_resource->locator(), remote_locator);
            scopedLock.lock();
            receiver_in_use->in_use = false;
            receiver_in_use->cv.notify_one();
        }
        else
        {
            logWarning(RTCP, "Received Message, but no TransportReceiverInterface attached: " << logicalPort);
        }
    }

    logInfo(RTCP, "End PerformListenOperation " << p_channel_resource->locator());
}

bool TCPTransportInterface::read_body(octet* receive_buffer, uint32_t receive_buffer_capacity,
    uint32_t* bytes_received, TCPChannelResource *p_channel_resource, std::size_t body_size)
{
    //*bytes_received = static_cast<uint32_t>(read(*p_channel_resource->socket(),
    //    asio::buffer(receive_buffer, receive_buffer_capacity), transfer_exactly(body_size)));

    *bytes_received = p_channel_resource->read(receive_buffer, receive_buffer_capacity, body_size);

    if (*bytes_received != body_size)
    {
        logError(RTCP, "Bad TCP body size: " << bytes_received << "(expected: " << TCPHeader::size() << ")");
        return false;
    }

    return true;
}

/**
* On TCP, we must receive the header (14 Bytes) and then,
* the rest of the message, whose length is on the header.
* TCP Header is transparent to the caller, so receive_buffer
* doesn't include it.
* */
bool TCPTransportInterface::Receive(TCPChannelResource *p_channel_resource, octet* receive_buffer,
    uint32_t receive_buffer_capacity, uint32_t& receive_buffer_size, Locator_t& remote_locator)
{
    bool success = false;

    if (!p_channel_resource->alive())
    {
        success = false; // Quick return
    }
    else
    {
        try
        {
            std::unique_lock<std::recursive_mutex> scopedLock(p_channel_resource->read_mutex());
            // Once mutex is optained, check again, just in case we took it in the little window
            // between disabling and destructor.
            if (p_channel_resource->alive())
            {
                success = true;

                // Read the header
                //octet header[TCPHEADER_SIZE];
                TCPHeader tcp_header;
                asio::error_code ec;
                //size_t bytes_received = read(*p_channel_resource->socket(),
                //    asio::buffer(&tcp_header, TCPHeader::getSize()),
                //    transfer_exactly(TCPHeader::getSize()), ec);

                size_t bytes_received = p_channel_resource->read(tcp_header.address(),
                    TCPHeader::size(), TCPHeader::size(), ec);

                remote_locator = p_channel_resource->locator();

                if (bytes_received != TCPHeader::size())
                {
                    if (bytes_received > 0)
                    {
                        logError(RTCP_MSG_IN, "Bad TCP header size: " << bytes_received << "(expected: : "
                            << TCPHeader::size() << ")" << ec);
                    }
                    close_tcp_socket(p_channel_resource);
                    success = false;
                }
                else
                {
                    size_t body_size = tcp_header.length - static_cast<uint32_t>(TCPHeader::size());

                    if (body_size > receive_buffer_capacity)
                    {
                        logError(RTCP_MSG_IN, "Size of incoming TCP message is bigger than buffer capacity: "
                            << static_cast<uint32_t>(body_size) << " vs. " << receive_buffer_capacity << ". " <<
                            "The full message will be dropped.");
                        success = false;
                        // Drop the message
                        size_t to_read = body_size;
                        size_t read_block = receive_buffer_capacity;
                        uint32_t readed;
                        while (read_block > 0)
                        {
                            read_body(receive_buffer, receive_buffer_capacity, &readed, p_channel_resource, read_block);
                            to_read -= readed;
                            read_block = (to_read >= receive_buffer_capacity) ? receive_buffer_capacity : to_read;
                        }
                    }
                    else
                    {
                        logInfo(RTCP_MSG_IN, "Received RTCP MSG. Logical Port " << tcp_header.logical_port);
                        success = read_body(receive_buffer, receive_buffer_capacity, &receive_buffer_size, p_channel_resource,
                            body_size);
                        //logInfo(RTCP_MSG_IN, " Received [read_body]");

                        if (configuration()->check_crc && !check_crc(tcp_header, receive_buffer, receive_buffer_size))
                        {
                            logWarning(RTCP_MSG_IN, "Bad TCP header CRC");
                        }

                        if (tcp_header.logical_port == 0)
                        {
                            //logInfo(RTCP_MSG_IN, " Receive [RTCP Control]  (" << receive_buffer_size+bytes_received
                            // << " bytes): " << receive_buffer_size << " bytes.");
                            ResponseCode responseCode =
                                rtcp_message_manager_->processRTCPMessage(p_channel_resource, receive_buffer, body_size);
                            if (responseCode != RETCODE_OK)
                            {
                                switch (responseCode)
                                {
                                    case RETCODE_INCOMPATIBLE_VERSION:
                                        {
                                            CloseOutputChannel(p_channel_resource->locator());
                                            break;
                                        }
                                    default: // Ignore
                                        {
                                            close_tcp_socket(p_channel_resource);
                                            break;
                                        }
                                }
                            }
                            success = false;
                        }
                        else
                        {
                            IPLocator::setLogicalPort(remote_locator, tcp_header.logical_port);
                            logInfo(RTCP_MSG_IN, "[RECEIVE] From: " << remote_locator \
                                << " - " << receive_buffer_size << " bytes.");
                        }
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
                //p_channel_resource->ConnectionLost();
                close_tcp_socket(p_channel_resource);
            }
            success = false;
        }
        catch (const asio::system_error& error)
        {
            (void)error;
            // Close the channel
            logInfo(RTCP_MSG_IN, "ASIO [RECEIVE]: " << error.what());
            //p_channel_resource->ConnectionLost();
            close_tcp_socket(p_channel_resource);
            success = false;
        }
    }

    success = success && receive_buffer_size > 0;

    return success;
}

size_t TCPTransportInterface::send(TCPChannelResource* p_channel_resource, const octet* data,
    size_t size, eSocketErrorCodes& errorCode) const
{
    size_t bytesSent = 0;
    try
    {
        asio::error_code ec;
        std::unique_lock<std::recursive_mutex> scopedLock(p_channel_resource->write_mutex());
        //bytesSent = p_channel_resource->socket()->send(asio::buffer(data, size), 0, ec);
        if (p_channel_resource->alive())
        {
            bytesSent = p_channel_resource->send(data, size, ec);
            errorCode = eSocketErrorCodes::eNoError;
        }
        else
        {
            errorCode = eSocketErrorCodes::eBrokenPipe;
        }

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

size_t TCPTransportInterface::send(TCPChannelResource *p_channel_resource, const octet *data, size_t size) const
{
    eSocketErrorCodes error;
    return send(p_channel_resource, data, size, error);
}


bool TCPTransportInterface::send(const octet* send_buffer, uint32_t send_buffer_size, const Locator_t& localLocator,
    const Locator_t& remote_locator)
{
    /*
    logInfo(RTCP, " SEND [RTPS Data] to locator " << IPLocator::getPhysicalPort(remote_locator) << ":" << \
        IPLocator::getLogicalPort(remote_locator));
    */

    TCPChannelResource* channelResource = nullptr;
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (!IsOutputChannelConnected(remote_locator) || send_buffer_size > configuration()->sendBufferSize)
        {
            logWarning(RTCP, "SEND [RTPS] Failed: Not connect: " << IPLocator::getLogicalPort(remote_locator) \
                << " @ IP: " << IPLocator::toIPv4string(remote_locator));
            return false;
        }

        auto it = channel_resources_.find(IPLocator::toPhysicalLocator(remote_locator));
        if (it == channel_resources_.end())
        {
            enqueue_logical_output_port(remote_locator);
            logInfo(RTCP, "SEND [RTPS] Failed: Not yet bound: " << IPLocator::getLogicalPort(remote_locator) \
                << " @ IP: " << IPLocator::toIPv4string(remote_locator) << " will be bound.");
            return false;
        }
        else
        {
            channelResource = it->second;
        }
    }

    bool result = true;
    if (channelResource != nullptr)
    {
        result = result && send(send_buffer, send_buffer_size, localLocator, remote_locator, channelResource);
    }
    return result;
}

bool TCPTransportInterface::send(const octet* send_buffer, uint32_t send_buffer_size, const Locator_t& /*localLocator*/,
    const Locator_t& remote_locator, ChannelResource *p_channel_resource)
{
    TCPChannelResource* tcpChannelResource = dynamic_cast<TCPChannelResource*>(p_channel_resource);
    if (tcpChannelResource != nullptr && tcpChannelResource->connection_established())
    {
        bool success = false;
        uint16_t logicalPort = IPLocator::getLogicalPort(remote_locator);

        if (tcpChannelResource->is_logical_port_added(logicalPort))
        {
            bool bShouldWait = configuration()->wait_for_tcp_negotiation;
            bool bConnected = tcpChannelResource->alive() && tcpChannelResource->connection_established();
            while (bShouldWait && bConnected && !tcpChannelResource->is_logical_port_opened(logicalPort))
            {
                bConnected = tcpChannelResource->wait_until_port_is_open_or_connection_is_closed(logicalPort);
            }

            if (bConnected && tcpChannelResource->is_logical_port_opened(logicalPort))
            {
                TCPHeader tcp_header;
                fill_rtcp_header(tcp_header, send_buffer, send_buffer_size, logicalPort);

                {
                    //std::unique_lock<std::recursive_mutex> sendLock(tcpChannelResource->write_mutex());
                    success = send_through_socket((octet*)&tcp_header, static_cast<uint32_t>(TCPHeader::size()),
                        remote_locator, tcpChannelResource);

                    if (success)
                    {
                        success = send_through_socket(send_buffer, send_buffer_size, remote_locator, tcpChannelResource);
                    }
                }
            }
        }
        else
        {
            tcpChannelResource->add_logical_port(logicalPort);
        }

        return success;
    }
    else if (send_retry_active_)
    {
        logWarning(RTCP, " SEND [RTPS] Failed: Connection not established " \
            << IPLocator::getLogicalPort(remote_locator));
        return false;
    }
    else
    {
        // With the retry disabled, the messages are discarded
        return true;
    }
}

bool TCPTransportInterface::send_through_socket(const octet* send_buffer, uint32_t send_buffer_size,
    const Locator_t& remote_locator, TCPChannelResource *socket)
{
    auto destinationEndpoint = generate_endpoint(remote_locator, IPLocator::getPhysicalPort(remote_locator));

    size_t bytesSent = 0;
    (void)destinationEndpoint;

    //logInfo(RTCP, "SOCKET SEND to physical port " << socket->socket()->remote_endpoint().port());

    eSocketErrorCodes errorCode;
    bytesSent = send(socket, send_buffer, send_buffer_size, errorCode);
    switch (errorCode)
    {
    case eNoError:
        //logInfo(RTCP, " Sent [OK]: " << send_buffer_size << " bytes to locator " << IPLocator::getLogicalPort(remote_locator));
        break;
    default:
        // Inform that connection has been lost
        logInfo(RTCP, " Sent [FAILED]: " << send_buffer_size << " bytes to locator " << IPLocator::getLogicalPort(remote_locator) << " ERROR=" << errorCode);
        //socket->ConnectionLost();
        close_tcp_socket(socket);
        break;
    }

    logInfo(RTCP_MSG_OUT, "[SENT] TO " << remote_locator << " - " << send_buffer_size << " (" << bytesSent << ").");
    return bytesSent > 0;
}

LocatorList_t TCPTransportInterface::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t unicastResult;
    for (const LocatorList_t& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        LocatorList_t pendingUnicast;

        while (it != locatorList.end())
        {
            assert((*it).kind == transport_kind_);

            // Check is local interface.
            auto localInterface = current_interfaces_.begin();
            for (; localInterface != current_interfaces_.end(); ++localInterface)
            {
                if (compare_locator_ip(localInterface->locator, *it))
                {
                    // Loopback locator
                    Locator_t loopbackLocator;
                    fill_local_ip(loopbackLocator);
                    IPLocator::setPhysicalPort(loopbackLocator, IPLocator::getPhysicalPort(*it));
                    IPLocator::setLogicalPort(loopbackLocator, IPLocator::getLogicalPort(*it));
                    pendingUnicast.push_back(loopbackLocator);
                    break;
                }
            }

            if (localInterface == current_interfaces_.end())
                pendingUnicast.push_back(*it);

            ++it;
        }

        unicastResult.push_back(pendingUnicast);
    }

    LocatorList_t result(std::move(unicastResult));
    return result;
}

void TCPTransportInterface::SocketAccepted(TCPAcceptorBasic* acceptor, const asio::error_code& error)
{
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (std::find(deleted_acceptors_.begin(), deleted_acceptors_.end(), acceptor) != deleted_acceptors_.end())
        {
            //std::cout << "Acceptor called on delete" << std::endl;
            // SocketAccepted was called by asio after the acceptor was deleted. By must abort any operation.
            logWarning(RTCP, "Acceptor called on delete");
            return;
        }
    }

    if (!error.value())
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (socket_Acceptors_.find(IPLocator::getPhysicalPort(acceptor->locator)) != socket_Acceptors_.end())
        {
#if defined(ASIO_HAS_MOVE)
            tcp_basic::eProsimaTCPSocket unicastSocket = tcp_basic::eProsimaTCPSocket(std::move(acceptor->socket));
#else
            tcp_basic::eProsimaTCPSocket unicastSocket = tcp_basic::eProsimaTCPSocket(acceptor->socket);
            acceptor->socket = nullptr;
#endif

            //tcp_basic::getSocketPtr(unicastSocket)->set_option(
            //    socket_base::receive_buffer_size(configuration()->receiveBufferSize));
            //tcp_basic::getSocketPtr(unicastSocket)->set_option(
            //    socket_base::send_buffer_size(configuration()->sendBufferSize));
            //tcp_basic::getSocketPtr(unicastSocket)->set_option(
            //    ip::tcp::no_delay(configuration()->enable_tcp_nodelay));


            // Store the new connection.
            TCPChannelResource *p_channel_resource = new TCPChannelResourceBasic(this, rtcp_message_manager_, io_service_,
                unicastSocket, configuration()->maxMessageSize);

            p_channel_resource->set_options(configuration());

            unbound_channel_resources_.push_back(p_channel_resource);
            p_channel_resource->thread(new std::thread(&TCPTransportInterface::perform_listen_operation, this,
                p_channel_resource));
            p_channel_resource->rtcp_thread(new std::thread(&TCPTransportInterface::perform_rtcp_management_thread,
                this, p_channel_resource));


            logInfo(RTCP, " Accepted connection (physical local: " << IPLocator::getPhysicalPort(acceptor->locator)
                << ", remote: " << p_channel_resource->remote_endpoint().port()
                << ") IP: " << p_channel_resource->remote_endpoint().address());


            // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            // std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            // std::cout << std::put_time(std::localtime(&now_c), "%F %T")
            //     <<  "--> Accepted connection (physical local: " << acceptor->locator.get_physical_port()
            //     << ", remote: " << p_channel_resource->socket()->remote_endpoint().port()
            //     << ") IP: " << p_channel_resource->socket()->remote_endpoint().address() << std::endl;
        }
        else
        {
            logError(RTPC, "Incomming connection from unknown Acceptor: "
                << IPLocator::getPhysicalPort(acceptor->locator));
            return;
        }
    }
    else
    {
        logInfo(RTCP, " Accepting connection failed (error: " << error.message() << ")");
        eClock::my_sleep(200); // Wait a little to accept again.
    }

    if (error.value() != eSocketErrorCodes::eConnectionAborted) // Operation Aborted
    {
        // Accept new connections for the same port. Could be not found when exiting.
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (socket_Acceptors_.find(IPLocator::getPhysicalPort(acceptor->locator)) != socket_Acceptors_.end())
        {
            acceptor->Accept(this, io_service_);
        }
    }
}

void TCPTransportInterface::SecureSocketAccepted(
    TCPAcceptorSecure* acceptor,
    asio::ip::tcp::socket&& socket,
    const asio::error_code& error)
{
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (std::find(deleted_acceptors_.begin(), deleted_acceptors_.end(), acceptor) != deleted_acceptors_.end())
        {
            //std::cout << "Acceptor called on delete" << std::endl;
            // SocketAccepted was called by asio after the acceptor was deleted. By must abort any operation.
            logWarning(RTCP, "Acceptor called on delete");
            return;
        }
    }

    if (!error.value())
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (socket_Acceptors_.find(IPLocator::getPhysicalPort(acceptor->locator)) != socket_Acceptors_.end())
        {
            // Store the new connection.
            TCPChannelResource *p_channel_resource = new TCPChannelResourceSecure(this, rtcp_message_manager_, io_service_,
                ssl_context_, std::move(socket), configuration()->maxMessageSize);

            p_channel_resource->set_options(configuration());

            unbound_channel_resources_.push_back(p_channel_resource);
            p_channel_resource->thread(new std::thread(&TCPTransportInterface::perform_listen_operation, this,
                p_channel_resource));
            p_channel_resource->rtcp_thread(new std::thread(&TCPTransportInterface::perform_rtcp_management_thread,
                this, p_channel_resource));


            logInfo(RTCP, " Accepted connection (physical local: " << IPLocator::getPhysicalPort(acceptor->locator)
                << ", remote: " << p_channel_resource->remote_endpoint().port()
                << ") IP: " << p_channel_resource->remote_endpoint().address());


            // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            // std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            // std::cout << std::put_time(std::localtime(&now_c), "%F %T")
            //     <<  "--> Accepted connection (physical local: " << acceptor->locator.get_physical_port()
            //     << ", remote: " << p_channel_resource->socket()->remote_endpoint().port()
            //     << ") IP: " << p_channel_resource->socket()->remote_endpoint().address() << std::endl;
        }
        else
        {
            logError(RTPC, "Incomming connection from unknown Acceptor: "
                << IPLocator::getPhysicalPort(acceptor->locator));
            return;
        }
    }
    else
    {
        logInfo(RTCP, " Accepting connection failed (error: " << error.message() << ")");
        eClock::my_sleep(200); // Wait a little to accept again.
    }

    if (error.value() != eSocketErrorCodes::eConnectionAborted) // Operation Aborted
    {
        // Accept new connections for the same port. Could be not found when exiting.
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        if (socket_Acceptors_.find(IPLocator::getPhysicalPort(acceptor->locator)) != socket_Acceptors_.end())
        {
            acceptor->Accept(this, io_service_, ssl_context_);
        }
    }
}

void TCPTransportInterface::SocketConnected(Locator_t locator, const asio::error_code& error)
{
    TCPChannelResource* outputSocket = nullptr;
    {
        std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
        auto it = channel_resources_.find(IPLocator::toPhysicalLocator(locator));
        if (it != channel_resources_.end())
        {
            outputSocket = it->second;
        }
    }

    if(outputSocket != nullptr)
    {
        if(error.value() == 0)
        {
            try
            {
                //outputSocket->socket()->set_option(
                //    socket_base::receive_buffer_size(configuration()->receiveBufferSize));
                //outputSocket->socket()->set_option(
                //    socket_base::send_buffer_size(configuration()->sendBufferSize));
                //outputSocket->socket()->set_option(
                //    ip::tcp::no_delay(configuration()->enable_tcp_nodelay));
                outputSocket->set_options(configuration());

                outputSocket->thread(
                    new std::thread(&TCPTransportInterface::perform_listen_operation, this, outputSocket));
                outputSocket->rtcp_thread(
                    new std::thread(&TCPTransportInterface::perform_rtcp_management_thread, this, outputSocket));

                // RTCP Control Message
                rtcp_message_manager_->sendConnectionRequest(outputSocket);
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
        else
        {
            if (error.value() == asio::error::basic_errors::connection_refused)
            {
                // Wait a little before try again to avoid exhaust file descriptors in some systems
                eClock::my_sleep(200);
            }
            close_tcp_socket(outputSocket);
        }
    }
}

void TCPTransportInterface::UnbindSocket(TCPChannelResource *pSocket)
{
    std::unique_lock<std::mutex> scopedLock(sockets_map_mutex_);
    auto it = channel_resources_.find(IPLocator::toPhysicalLocator(pSocket->locator()));
    if (it != channel_resources_.end())
    {
        channel_resources_.erase(it);
    }
}

bool TCPTransportInterface::getDefaultMetatrafficMulticastLocators(LocatorList_t &, uint32_t ) const
{
    // TCP doesn't have multicast support
    return true;
}

bool TCPTransportInterface::getDefaultMetatrafficUnicastLocators(LocatorList_t &locators,
    uint32_t metatraffic_unicast_port) const
{
    Locator_t locator;
    locator.kind = transport_kind_;
    locator.set_Invalid_Address();
    fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);
    locators.push_back(locator);
    return true;
}

bool TCPTransportInterface::getDefaultUnicastLocators(LocatorList_t &locators, uint32_t unicast_port) const
{
    Locator_t locator;
    locator.kind = transport_kind_;
    locator.set_Invalid_Address();
    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);
    return true;
}

bool TCPTransportInterface::fillMetatrafficMulticastLocator(Locator_t &, uint32_t) const
{
    // TCP doesn't have multicast support
    return true;
}

bool TCPTransportInterface::fillMetatrafficUnicastLocator(Locator_t &locator,
        uint32_t metatraffic_unicast_port) const
{
    if (IPLocator::getPhysicalPort(locator.port) == 0)
    {
        const TCPTransportDescriptor* config = configuration();
        if (config != nullptr)
        {
            if (!config->listening_ports.empty())
            {
                IPLocator::setPhysicalPort(locator, *(config->listening_ports.begin()));
            }
            else
            {
                IPLocator::setPhysicalPort(locator, static_cast<uint16_t>(System::GetPID()));
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
        for(uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
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
            for(uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
            {
                Locator_t auxloc(locator);
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

bool TCPTransportInterface::fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const
{
    if (IPLocator::getPhysicalPort(locator.port) == 0)
    {
        const TCPTransportDescriptor* config = configuration();
        if (config != nullptr)
        {
            if (!config->listening_ports.empty())
            {
                IPLocator::setPhysicalPort(locator, *(config->listening_ports.begin()));
            }
            else
            {
                IPLocator::setPhysicalPort(locator, static_cast<uint16_t>(System::GetPID()));
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

void TCPTransportInterface::shutdown()
{
    send_retry_active_ = false;
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
