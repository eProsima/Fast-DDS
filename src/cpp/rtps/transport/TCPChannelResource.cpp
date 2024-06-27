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

#include <rtps/transport/TCPChannelResource.h>

#include <chrono>
#include <thread>

#include <fastdds/utils/IPLocator.hpp>

#include <rtps/transport/asio_helpers.hpp>
#include <rtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

/**
 * Search for the base port in the current domain without taking account the participant
 */
static uint16_t GetBaseAutoPort(
        uint16_t currentPort)
{
    if (currentPort < 7411)
    {
        return currentPort;
    }
    uint16_t aux = currentPort - 7411; // base + offset3
    uint16_t domain = static_cast<uint16_t>(aux / 250.);
    uint16_t part = static_cast<uint16_t>(aux % 250);
    part = part / 2;

    return 7411 + (domain * 250); // And participant 0
}

TCPChannelResource::TCPChannelResource(
        TCPTransportInterface* parent,
        const Locator& locator,
        uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , parent_ (parent)
    , locator_(locator)
    , waiting_for_keep_alive_(false)
    , connection_status_(eConnectionStatus::eDisconnected)
    , tcp_connection_type_(TCPConnectionType::TCP_CONNECT_TYPE)
{
}

TCPChannelResource::TCPChannelResource(
        TCPTransportInterface* parent,
        uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , parent_(parent)
    , locator_()
    , waiting_for_keep_alive_(false)
    , connection_status_(eConnectionStatus::eConnected)
    , tcp_connection_type_(TCPConnectionType::TCP_ACCEPT_TYPE)
{
}

void TCPChannelResource::disable()
{
    ChannelResource::disable(); // prevent asio callback workings on this channel.

    disconnect();
}

ResponseCode TCPChannelResource::process_bind_request(
        const Locator& locator)
{
    eConnectionStatus expected = TCPChannelResource::eConnectionStatus::eWaitingForBind;
    if (connection_status_.compare_exchange_strong(expected, eConnectionStatus::eEstablished))
    {
        locator_ = IPLocator::toPhysicalLocator(locator);
        EPROSIMA_LOG_INFO(RTCP_MSG, "Connection Established");
        return RETCODE_OK;
    }
    else if (expected == eConnectionStatus::eEstablished)
    {
        return RETCODE_EXISTING_CONNECTION;
    }

    return RETCODE_SERVER_ERROR;
}

void TCPChannelResource::set_all_ports_pending()
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    pending_logical_output_ports_.insert(pending_logical_output_ports_.end(),
            logical_output_ports_.begin(),
            logical_output_ports_.end());
    logical_output_ports_.clear();
}

bool TCPChannelResource::is_logical_port_opened(
        uint16_t port)
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    return is_logical_port_opened_nts(port);
}

bool TCPChannelResource::is_logical_port_opened_nts(
        uint16_t port)
{
    return std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port) != logical_output_ports_.end();
}

bool TCPChannelResource::is_logical_port_added(
        uint16_t port)
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    return std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port) != logical_output_ports_.end()
           || std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port)
           != pending_logical_output_ports_.end();
}

bool TCPChannelResource::wait_logical_port_under_negotiation(
        uint16_t port,
        const std::chrono::milliseconds& timeout)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);

    // Early return if the port is already opened.
    if (is_logical_port_opened_nts(port))
    {
        return true;
    }

    // Early return if the timeout is 0.
    if (timeout == std::chrono::milliseconds(0))
    {
        return false;
    }

    // The port is under negotiation if it's in the pending list and in the negotiation list.
    bool found_in_negotiating_list = negotiating_logical_ports_.end() != std::find_if(
        negotiating_logical_ports_.begin(),
        negotiating_logical_ports_.end(),
        [port](const decltype(negotiating_logical_ports_)::value_type& item)
        {
            return item.second == port;
        });

    if (found_in_negotiating_list &&
            pending_logical_output_ports_.end() != std::find(
                pending_logical_output_ports_.begin(),
                pending_logical_output_ports_.end(),
                port))
    {
        // Wait for the negotiation to finish. The condition variable might get notified if other logical port is opened. In such case,
        // it should wait again with the respective remaining time.
        auto wait_predicate = [this, port]() -> bool
                {
                    return is_logical_port_opened_nts(port);
                };
        logical_output_ports_updated_cv.wait_for(scopedLock, timeout, wait_predicate);
    }

    return is_logical_port_opened_nts(port);
}

void TCPChannelResource::add_logical_port(
        uint16_t port,
        RTCPMessageManager* rtcp_manager)
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    // Already opened?
    if (std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port) == logical_output_ports_.end())
    {
        if (port == 0)
        {
            EPROSIMA_LOG_ERROR(RTPS, "Trying to open logical port 0.");
        } // But let's continue...

        if (std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port)
                == pending_logical_output_ports_.end()) // Check isn't enqueued already
        {
            pending_logical_output_ports_.emplace_back(port);
            if (connection_established())
            {
                TCPTransactionId id = rtcp_manager->sendOpenLogicalPortRequest(this, port);
                negotiating_logical_ports_[id] = port;
            }
        }
    }

}

void TCPChannelResource::send_pending_open_logical_ports(
        RTCPMessageManager* rtcp_manager)
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    if (!pending_logical_output_ports_.empty())
    {
        for (uint16_t port : pending_logical_output_ports_)
        {
            TCPTransactionId id = rtcp_manager->sendOpenLogicalPortRequest(this, port);
            negotiating_logical_ports_[id] = port;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void TCPChannelResource::add_logical_port_response(
        const TCPTransactionId& id,
        bool success,
        RTCPMessageManager* rtcp_manager)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    auto it = negotiating_logical_ports_.find(id);
    if (it != negotiating_logical_ports_.end())
    {
        uint16_t port = it->second;
        auto portIt = std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port);
        negotiating_logical_ports_.erase(it);
        if (portIt != pending_logical_output_ports_.end())
        {
            if (success)
            {
                pending_logical_output_ports_.erase(portIt);
                logical_output_ports_.push_back(port);
                logical_output_ports_updated_cv.notify_all();
                EPROSIMA_LOG_INFO(RTCP, "OpenedLogicalPort: " << port);
            }
            else
            {
                scopedLock.unlock();
                prepare_send_check_logical_ports_req(port, rtcp_manager);
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTCP, "Received add_logical_port_response for port "
                    << port << ", but it wasn't found in pending list.");
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTCP, "Received add_logical_port_response, but the transaction id wasn't registered " <<
                "(maybe removed" << " while negotiating?).");
    }
}

void TCPChannelResource::prepare_send_check_logical_ports_req(
        uint16_t closedPort,
        RTCPMessageManager* rtcp_manager)
{
    std::vector<uint16_t> candidatePorts;
    uint16_t base_port = GetBaseAutoPort(closedPort); // The first failed port
    uint16_t max_port = closedPort + parent_->GetMaxLogicalPort();

    for (uint16_t p = base_port;
            p <= closedPort + (parent_->GetLogicalPortRange()
            * parent_->GetLogicalPortIncrement());
            p += parent_->GetLogicalPortIncrement())
    {
        // Don't add ports just tested and already pendings
        if (p <= max_port && p != closedPort)
        {
            std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
            auto pendingIt = std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), p);
            if (pendingIt == pending_logical_output_ports_.end())
            {
                candidatePorts.emplace_back(p);
            }
        }
    }

    if (candidatePorts.empty()) // No more available ports!
    {
        EPROSIMA_LOG_ERROR(RTCP, "Cannot find an available logical port.");
    }
    else
    {
        TCPTransactionId id = rtcp_manager->sendCheckLogicalPortsRequest(this, candidatePorts);
        std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
        last_checked_logical_port_[id] = candidatePorts.back();
    }
}

void TCPChannelResource::process_check_logical_ports_response(
        const TCPTransactionId& transactionId,
        const std::vector<uint16_t>& availablePorts,
        RTCPMessageManager* rtcp_manager)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    auto it = last_checked_logical_port_.find(transactionId);
    if (it != last_checked_logical_port_.end())
    {
        uint16_t lastPort = it->second;
        last_checked_logical_port_.erase(it);
        scopedLock.unlock();
        if (availablePorts.empty())
        {
            prepare_send_check_logical_ports_req(lastPort, rtcp_manager);
        }
        else
        {
            add_logical_port(availablePorts.front(), rtcp_manager);
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTCP, "Received process_check_logical_ports_response without sending a Request.");
    }
}

void TCPChannelResource::set_logical_port_pending(
        uint16_t port)
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    auto it = std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port);
    if (it != logical_output_ports_.end())
    {
        pending_logical_output_ports_.push_back(port);
        logical_output_ports_.erase(it);
    }
}

bool TCPChannelResource::remove_logical_port(
        uint16_t port)
{
    std::lock_guard<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    if (!is_logical_port_added(port))
    {
        return false;
    }

    auto it = std::remove(logical_output_ports_.begin(), logical_output_ports_.end(), port);
    logical_output_ports_.erase(it, logical_output_ports_.end());
    it = std::remove(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port);
    pending_logical_output_ports_.erase(it, pending_logical_output_ports_.end());
    return true;
}

bool TCPChannelResource::check_socket_send_buffer(
        const size_t& msg_size,
        const asio::ip::tcp::socket::native_handle_type& socket_native_handle)
{
    int bytesInSendQueue = 0;

#ifndef _WIN32
    if (ioctl(socket_native_handle, TIOCOUTQ, &bytesInSendQueue) == -1)
    {
        bytesInSendQueue = 0;
    }
#else // ifdef _WIN32
    static_cast<void>(socket_native_handle);
#endif // ifndef _WIN32


    size_t future_queue_size = size_t(bytesInSendQueue) + msg_size;
    // TCP actually allocates twice the size of the buffer requested.
    if (future_queue_size > size_t(2 * parent_->configuration()->sendBufferSize))
    {
        return false;
    }
    return true;
}

void TCPChannelResource::set_socket_options(
        asio::basic_socket<asio::ip::tcp>& socket,
        const TCPTransportDescriptor* options)
{
    uint32_t minimum_value = options->maxMessageSize;

    // Set the send buffer size
    {
        uint32_t desired_value = options->sendBufferSize;
        uint32_t configured_value = 0;
        if (!asio_helpers::try_setting_buffer_size<asio::socket_base::send_buffer_size>(
                    socket, desired_value, minimum_value, configured_value))
        {
            EPROSIMA_LOG_ERROR(TCP_TRANSPORT,
                    "Couldn't set send buffer size to minimum value: " << minimum_value);
        }
        else if (desired_value != configured_value)
        {
            EPROSIMA_LOG_WARNING(TCP_TRANSPORT,
                    "Couldn't set send buffer size to desired value. "
                    << "Using " << configured_value << " instead of " << desired_value);
        }
    }

    // Set the receive buffer size
    {
        uint32_t desired_value = options->receiveBufferSize;
        uint32_t configured_value = 0;
        if (!asio_helpers::try_setting_buffer_size<asio::socket_base::receive_buffer_size>(
                    socket, desired_value, minimum_value, configured_value))
        {
            EPROSIMA_LOG_ERROR(TCP_TRANSPORT,
                    "Couldn't set receive buffer size to minimum value: " << minimum_value);
        }
        else if (desired_value != configured_value)
        {
            EPROSIMA_LOG_WARNING(TCP_TRANSPORT,
                    "Couldn't set receive buffer size to desired value. "
                    << "Using " << configured_value << " instead of " << desired_value);
        }
    }

    // Set the TCP_NODELAY option
    socket.set_option(asio::ip::tcp::no_delay(options->enable_tcp_nodelay));
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
