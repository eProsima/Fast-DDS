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

#include <fastrtps/transport/TCPChannelResource.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/eClock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Search for the base port in the current domain without taking account the participant
 */
static uint16_t GetBaseAutoPort(uint16_t currentPort)
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
        RTCPMessageManager* rtcpManager,
        const Locator_t& locator,
        uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , parent_ (parent)
    , rtcp_manager_(rtcpManager)
    , locator_(locator)
    , input_socket_(false)
    , waiting_for_keep_alive_(false)
    , rtcp_thread_(nullptr)
    , connection_status_(eConnectionStatus::eDisconnected)
{
}

TCPChannelResource::TCPChannelResource(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , parent_(parent)
    , rtcp_manager_(rtcpManager)
    , locator_()
    , input_socket_(true)
    , waiting_for_keep_alive_(false)
    , rtcp_thread_(nullptr)
    , connection_status_(eConnectionStatus::eWaitingForBind)
{
}

TCPChannelResource::~TCPChannelResource()
{
    if (rtcp_thread_ != nullptr)
    {
        rtcp_thread_->join();
        delete(rtcp_thread_);
        rtcp_thread_ = nullptr;
    }
}

void TCPChannelResource::disable()
{
    ChannelResource::disable();
    disconnect();
}

ResponseCode TCPChannelResource::process_bind_request(const Locator_t& locator)
{
    std::unique_lock<std::mutex> scoped(status_mutex_);
    if (connection_status_ == TCPChannelResource::eConnectionStatus::eWaitingForBind)
    {
        locator_ = locator;
        TCPChannelResource* oldChannel = parent_->BindSocket(locator_, this);
        if (oldChannel != nullptr)
        {
            copy_pending_ports_from(oldChannel);
            parent_->DeleteSocket(oldChannel);
            //delete oldChannel;
        }

        connection_status_ = eConnectionStatus::eEstablished;
        logInfo(RTPC_MSG, "Connection Stablished");
        return RETCODE_OK;
    }
    else if (connection_status_ == eConnectionStatus::eEstablished)
    {
        return RETCODE_EXISTING_CONNECTION;
    }

    return RETCODE_SERVER_ERROR;
}

void TCPChannelResource::input_port_closed(uint16_t port)
{
    if (connection_established())
    {
        rtcp_manager_->sendLogicalPortIsClosedRequest(this, port);
    }
}

void TCPChannelResource::set_all_ports_pending()
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    pending_logical_output_ports_.insert(pending_logical_output_ports_.end(),
        logical_output_ports_.begin(),
        logical_output_ports_.end());
    logical_output_ports_.clear();
}

void TCPChannelResource::copy_pending_ports_from(TCPChannelResource* from)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    std::unique_lock<std::recursive_mutex> fromLock(from->pending_logical_mutex_);
    pending_logical_output_ports_.insert(pending_logical_output_ports_.end(),
        from->pending_logical_output_ports_.begin(),
        from->pending_logical_output_ports_.end());
}

bool TCPChannelResource::is_logical_port_opened(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    return std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port) != logical_output_ports_.end();
}

bool TCPChannelResource::is_logical_port_added(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    return std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port) != logical_output_ports_.end()
        || std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port)
        != pending_logical_output_ports_.end();
}

bool TCPChannelResource::wait_until_port_is_open_or_connection_is_closed(uint16_t port)
{
    std::unique_lock<std::mutex> scoped(status_mutex_);
    bool bConnected = alive_ && connection_status_ == eConnectionStatus::eEstablished;
    while (bConnected && !is_logical_port_opened(port))
    {
        negotiation_condition_.wait(scoped);
        bConnected = alive_ && connection_status_ == eConnectionStatus::eEstablished;
    }
    return bConnected;
}

std::thread* TCPChannelResource::release_rtcp_thread()
{
    std::thread* outThread = rtcp_thread_;
    rtcp_thread_ = nullptr;
    return outThread;
}

void TCPChannelResource::add_logical_port(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    // Already opened?
    if (std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port) == logical_output_ports_.end())
    {
        if (port == 0)
        {
            logError(RTPS, "Trying to open logical port 0.");
        } // But let's continue...

        if (std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port)
                == pending_logical_output_ports_.end()) // Check isn't enqueued already
        {
            pending_logical_output_ports_.emplace_back(port);
            if (connection_established())
            {
                TCPTransactionId id = rtcp_manager_->sendOpenLogicalPortRequest(this, port);
                negotiating_logical_ports_[id] = port;
            }
        }
    }
}

void TCPChannelResource::send_pending_open_logical_ports()
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    if (!pending_logical_output_ports_.empty())
    {
        for (uint16_t port : pending_logical_output_ports_)
        {
            TCPTransactionId id = rtcp_manager_->sendOpenLogicalPortRequest(this, port);
            negotiating_logical_ports_[id] = port;
            eClock::my_sleep(100);
        }
    }
}

void TCPChannelResource::add_logical_port_response(
        const TCPTransactionId &id,
        bool success)
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
            pending_logical_output_ports_.erase(portIt);
            if (success)
            {
                logical_output_ports_.push_back(port);
                negotiation_condition_.notify_all();
                logInfo(RTCP, "OpenedLogicalPort " << port);
            }
            else
            {
                prepare_send_check_logical_ports_req(port);
            }
        }
        else
        {
            logWarning(RTCP, "Received add_logical_port_response for port " << port
                << ", but it wasn't found in pending list.");
        }
    }
    else
    {
        logWarning(RTCP, "Received add_logical_port_response, but the transaction id wasn't registered " <<
            "(maybe removed" << " while negotiating?).");
    }
}

void TCPChannelResource::prepare_send_check_logical_ports_req(uint16_t closedPort)
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
            std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
            auto pendingIt = std::find(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), p);
            if (pendingIt == pending_logical_output_ports_.end())
            {
                candidatePorts.emplace_back(p);
            }
        }
    }

    if (candidatePorts.empty()) // No more available ports!
    {
        logError(RTCP, "Cannot find an available logical port.");
    }
    else
    {
        TCPTransactionId id = rtcp_manager_->sendCheckLogicalPortsRequest(this, candidatePorts);
        last_checked_logical_port_[id] = candidatePorts.back();
    }
}

void TCPChannelResource::process_check_logical_ports_response(
        const TCPTransactionId &transactionId,
        const std::vector<uint16_t> &availablePorts)
{
    auto it = last_checked_logical_port_.find(transactionId);
    if (it != last_checked_logical_port_.end())
    {
        uint16_t lastPort = it->second;
        last_checked_logical_port_.erase(it);
        if (availablePorts.empty())
        {
            prepare_send_check_logical_ports_req(lastPort);
        }
        else
        {
            add_logical_port(availablePorts.front());
        }
    }
    else
    {
        logWarning(RTCP, "Received process_check_logical_ports_response without sending a Request.");
    }
}

void TCPChannelResource::set_logical_port_pending(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    auto it = std::find(logical_output_ports_.begin(), logical_output_ports_.end(), port);
    if (it != logical_output_ports_.end())
    {
        pending_logical_output_ports_.push_back(port);
        logical_output_ports_.erase(it);
    }
}

bool TCPChannelResource::remove_logical_port(uint16_t port)
{
    std::unique_lock<std::recursive_mutex> scopedLock(pending_logical_mutex_);
    if (!is_logical_port_added(port))
        return false;

    auto it = std::remove(logical_output_ports_.begin(), logical_output_ports_.end(), port);
    logical_output_ports_.erase(it, logical_output_ports_.end());
    it = std::remove(pending_logical_output_ports_.begin(), pending_logical_output_ports_.end(), port);
    pending_logical_output_ports_.erase(it, pending_logical_output_ports_.end());
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
