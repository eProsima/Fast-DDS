/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>

#include <rtps/transport/ethernet/EthernetPacket.hpp>
#include <rtps/transport/ethernet/InputPort.hpp>
#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool InputChannelManager::init(
        const EthernetTransportDescriptor& configuration)
{
    const std::string& interface_name = configuration.interface_name;

    std::lock_guard<std::mutex> guard(mtx_);


    if (socket_ > 0)
    {
        return true;
    }

    socket_ = socket(AF_PACKET, SOCK_RAW, static_cast<int>(htons(EthernetPacketPrefix::ETH_P_RTPS)));
    if_index_ = if_nametoindex(interface_name.c_str());

    if (socket_ < 0 || if_index_ == 0)
    {
        // Error creating socket or finding interface index
        ::close(socket_);
        socket_ = -1;
        if_index_ = -1;
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Error creating raw socket or finding interface index for " << interface_name);
        return false;
    }

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);
    if (ioctl(socket_, SIOCGIFHWADDR, &ifr) < 0)
    {
        // Error getting MAC address
        ::close(socket_);
        socket_ = -1;
        if_index_ = -1;
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT, "Error getting MAC address for interface " << interface_name);
        return false;
    }

    std::memcpy(local_address_.data(), ifr.ifr_hwaddr.sa_data, local_address_.size());

    // Allow reuse of address
    int enable = 1;
    if (0 != setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)))
    {
        ::close(socket_);
        socket_ = -1;
        if_index_ = -1;
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Could not set SO_REUSEADDR on input socket for interface " << interface_name);
        return false;
    }

    // Bind socket to the interface
    struct sockaddr_ll bind_req {};
    bind_req.sll_family = AF_PACKET;
    bind_req.sll_protocol = htons(static_cast<uint16_t>(ETH_P_ALL));
    bind_req.sll_ifindex = if_index_;

    if (0 != bind(socket_, (const sockaddr*) &bind_req, sizeof(bind_req)))
    {
        ::close(socket_);
        socket_ = -1;
        if_index_ = -1;
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT, "Could not bind input socket to interface " << interface_name);
        return false;
    }

    // Set socket options
    int recv_buffer_size = static_cast<int>(configuration.receive_buffer_size);
    if (recv_buffer_size > 0)
    {
        if (0 != setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &recv_buffer_size, sizeof(recv_buffer_size)))
        {
            EPROSIMA_LOG_WARNING(RTPS_ETHERNET_TRANSPORT,
                    "Could not set receive buffer size to " << recv_buffer_size << " bytes on interface "
                                                            << interface_name);
        }
    }

    // Print socket options
    int actual_size = 0;
    socklen_t opt_len = sizeof(actual_size);
    if (0 == getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &actual_size, &opt_len))
    {
        if (actual_size < recv_buffer_size)
        {
            EPROSIMA_LOG_INFO(RTPS_ETHERNET_TRANSPORT,
                    "Receive buffer size set to " << actual_size / 2 << " bytes on interface " << interface_name);
        }
    }

    // Store configuration
    configuration_ = configuration;

    // Start listening thread
    auto fn = [this]()
            {
                process_listening();
            };
    listening_thread_ = create_thread(fn, configuration.default_reception_threads(), "dds.eth.main");

    return true;
}

void InputChannelManager::stop()
{
    {
        std::lock_guard<std::mutex> guard(mtx_);
        // Close all open channels
        mcast_addresses_.clear();
        port_listeners_.clear();
        // Close listening socket
        if (socket_ >= 0)
        {
            ::close(socket_);
            socket_ = -1;
            if_index_ = -1;
            local_address_.fill(0);
        }
    }

    // Stop all threads
    if (listening_thread_.joinable())
    {
        listening_thread_.join();
    }
}

EthernetAddress InputChannelManager::get_interface_address() const
{
    std::lock_guard<std::mutex> guard(mtx_);
    return local_address_;
}

bool InputChannelManager::is_open(
        const EthernetAddress& address,
        uint16_t logical_port) const
{
    std::lock_guard<std::mutex> guard(mtx_);

    return is_listening_on_address(address) && is_listening_on_port(logical_port);
}

bool InputChannelManager::open(
        const EthernetAddress& address,
        uint16_t logical_port,
        TransportReceiverInterface* receiver_interface,
        uint32_t max_message_size)
{
    std::lock_guard<std::mutex> guard(mtx_);

    bool ok = false;
    ok = listen_on_address(address);
    if (ok)
    {
        ok = listen_on_port(logical_port, receiver_interface, max_message_size);
    }

    return ok;
}

bool InputChannelManager::close(
        const EthernetAddress& address,
        uint16_t logical_port)
{
    std::lock_guard<std::mutex> guard(mtx_);

    static_cast<void>(address);

    // Implementation to close an input channel
    close_port(logical_port);

    return true;
}

bool InputChannelManager::locators_match(
        const Locator& locator1,
        const Locator& locator2) const
{
    return EthernetLocator::get_logical_port(locator1) == EthernetLocator::get_logical_port(locator2);
}

bool InputChannelManager::is_listening_on_address(
        const EthernetAddress& address) const
{
    if (EthernetLocator::is_ethernet_multicast(address))
    {
        return mcast_addresses_.count(address) > 0;
    }

    if (socket_ < 0)
    {
        return false;
    }

    return local_address_ == address;
}

bool InputChannelManager::is_listening_on_port(
        uint16_t port) const
{
    return port_listeners_.find(port) != port_listeners_.end();
}

bool InputChannelManager::listen_on_address(
        const EthernetAddress& address)
{
    if (socket_ < 0)
    {
        return false;
    }

    if (EthernetLocator::is_ethernet_multicast(address))
    {
        auto result = mcast_addresses_.insert(address);
        if (result.second)
        {
            struct packet_mreq multicast_req {};
            multicast_req.mr_ifindex = if_index_;
            multicast_req.mr_alen = address.size();
            memcpy(&multicast_req.mr_address[0], address.data(), address.size());

            if (0 > setsockopt(socket_, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &multicast_req, sizeof(multicast_req)))
            {
                return false;
            }
        }
    }
    else
    {
        return address == local_address_;
    }

    return true;
}

bool InputChannelManager::listen_on_port(
        uint16_t port,
        TransportReceiverInterface* receiver_interface,
        uint32_t max_message_size)
{
    if (socket_ < 0)
    {
        return false;
    }

    if (is_listening_on_port(port))
    {
        return true;
    }

    std::unique_ptr<InputPort> listener(new InputPort(port, receiver_interface, max_message_size,
            configuration_.get_thread_config_for_port(port)));
    port_listeners_.emplace(port, std::move(listener));

    return true;
}

void InputChannelManager::close_port(
        uint16_t port)
{
    auto it = port_listeners_.find(port);
    if (it != port_listeners_.end())
    {
        port_listeners_.erase(it);
    }
}

void InputChannelManager::process_listening()
{
    int fd = -1;
    {
        std::lock_guard<std::mutex> guard(mtx_);
        fd = socket_;
    }

    while (fd > 0)
    {
        // Use select to wait for data with a timeout
        // This allows checking periodically if the socket is still valid
        fd_set read_fds, err_fs;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        FD_ZERO(&err_fs);
        FD_SET(fd, &err_fs);
        struct timeval timeout {};
        timeout.tv_sec = 1; // 1 second timeout
        int select_ret = select(fd + 1, &read_fds, nullptr, &err_fs, &timeout);
        if (select_ret <= 0 || FD_ISSET(fd, &err_fs) || !FD_ISSET(fd, &read_fds))
        {
            // Either:
            // - Error in select, possibly interrupted by a signal
            // - Timeout in select
            // - Error on the socket
            // - No data to read
            std::lock_guard<std::mutex> guard(mtx_);
            fd = socket_;
            continue;
        }

        // TODO: Get packet from pool
        auto packet = std::make_shared<EthernetPacket>();
        ssize_t received_bytes = recv(fd, packet.get(), sizeof(EthernetPacket), MSG_TRUNC);
        if (received_bytes < 0)
        {
            std::lock_guard<std::mutex> guard(mtx_);
            fd = socket_;
            continue;
        }

        // Validate received packet size
        size_t received_size = static_cast<size_t>(received_bytes);
        if (received_size < sizeof(EthernetPacketHeader) + sizeof(EthernetPacketPrefix))
        {
            // Packet too small to contain header and prefix
            continue;
        }

        uint8_t* raw_data = reinterpret_cast<uint8_t*>(packet.get()) + sizeof(EthernetPacketHeader);
        size_t payload_size = received_size - sizeof(EthernetPacketHeader);
        EthernetPacketData* data = nullptr;

        // Validate Ethertype
        if (EthernetPacketPrefix::ETH_P_RTPS == ntohs(packet->header.h_proto))
        {
            // Direct RTPS packet
            data = reinterpret_cast<EthernetPacketData*>(raw_data);
            payload_size = payload_size - 2 * sizeof(uint16_t);
        }
        else if (ETH_P_8021Q == ntohs(packet->header.h_proto))
        {
            if (EthernetPacketPrefix::ETH_P_RTPS != ntohs(packet->prefix.proto))
            {
                // Not an RTPS packet
                continue;
            }

            // VLAN tagged RTPS packet
            raw_data += 2 * sizeof(uint16_t);
            data = reinterpret_cast<EthernetPacketData*>(raw_data);
            payload_size = payload_size - sizeof(EthernetPacketPrefix);
        }
        else
        {
            // Not an RTPS packet
            continue;
        }

        // Extract source and destination ports
        assert(data != nullptr);
        uint16_t dest_port = ntohs(data->dest_port);

        std::lock_guard<std::mutex> guard(mtx_);
        auto it = port_listeners_.find(dest_port);
        if (it != port_listeners_.end())
        {
            it->second->process_data({packet, data, static_cast<uint32_t>(payload_size)});
        }
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
