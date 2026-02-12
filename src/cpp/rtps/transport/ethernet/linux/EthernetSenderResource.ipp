/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <cstdint>
#include <chrono>
#include <cstring>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorsIterator.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>

#include <rtps/transport/ethernet/EthernetPacket.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct SocketAddress_t : public sockaddr_ll {};
struct SocketSendmsgMessage_t : public msghdr {};

EthernetSenderResource::EthernetSenderResource(
        const EthernetTransportDescriptor& configuration)
    : SenderResource(LOCATOR_KIND_ETHERNET)
    , priority_mapping_(configuration.priority_mapping)
{
    const std::string& interface_name = configuration.interface_name;
    const uint16_t default_source_port = configuration.default_source_port;

    // Initialize packet header and prefix using the provided interface name.
    socket_ = socket(AF_PACKET, SOCK_RAW, static_cast<int>(htons(EthernetPacketPrefix::ETH_P_RTPS)));
    if_index_ = if_nametoindex(interface_name.c_str());

    if (socket_ < 0 || if_index_ == 0)
    {
        // Error creating socket or finding interface index
        close(socket_);
        socket_ = -1;
        if_index_ = -1;
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Error creating raw socket or finding interface index for " << interface_name);
        return;
    }

    // Set socket options
    int send_buffer_size = static_cast<int>(configuration.send_buffer_size);
    if (send_buffer_size > 0)
    {
        if (0 != setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)))
        {
            EPROSIMA_LOG_WARNING(RTPS_ETHERNET_TRANSPORT,
                    "Could not set send buffer size to " << send_buffer_size << " bytes on interface "
                                                         << interface_name);
        }
    }

    // Print socket options
    int actual_size = 0;
    socklen_t opt_len = sizeof(actual_size);
    if (0 == getsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &actual_size, &opt_len))
    {
        if (actual_size < send_buffer_size)
        {
            EPROSIMA_LOG_INFO(RTPS_ETHERNET_TRANSPORT,
                    "Send buffer size set to " << actual_size / 2 << " bytes on interface " << interface_name);
        }
    }

    // Set source MAC address in the Ethernet header
    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);
    if (ioctl(socket_, SIOCGIFHWADDR, &ifr) < 0)
    {
        // Error getting MAC address
        close(socket_);
        socket_ = -1;
        if_index_ = -1;
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT, "Error getting MAC address for interface " << interface_name);
        return;
    }

    std::memcpy(eth_header_.h_source, ifr.ifr_hwaddr.sa_data, EthernetPacketHeader::ETH_ADDR_LEN);
    eth_header_.h_proto = htons(ETH_P_8021Q);
    eth_prefix_.proto = htons(EthernetPacketPrefix::ETH_P_RTPS);
    eth_prefix_.pcp_dei_vid = 0; // Default VLAN tag
    eth_prefix_.source_port = htons(default_source_port);
    eth_prefix_.dest_port = 0;   // To be set per packet

    send_lambda_ = [this](const std::vector<NetworkBuffer>& buffers, uint32_t total_bytes,
            LocatorsIterator* destination_locators_begin, LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& max_blocking_time, int32_t transport_priority)
            {
                constexpr size_t MAX_USER_BUFFERS = 64;

                if (total_bytes > EthernetPacket::MAX_RTPS_PAYLOAD_SIZE)
                {
                    // Message too large for Ethernet transport
                    return false;
                }

                if (total_bytes == 0 || buffers.empty())
                {
                    // Nothing to send
                    return true;
                }

                if (buffers.size() > MAX_USER_BUFFERS)
                {
                    // Too many buffers
                    return false;
                }

                EthernetPacketHeader header = eth_header_;
                EthernetPacketPrefix prefix = eth_prefix_;

                bool use_destination_vlan_config = true;

                if (transport_priority != 0)
                {
                    if (fill_prefix_from_priority(transport_priority, prefix))
                    {
                        use_destination_vlan_config = false;
                    }
                }

                // Prepare buffers for sendmsg call
                uint32_t total_size = 0;

                struct iovec iovecs[MAX_USER_BUFFERS + 3] = {};
                size_t iovec_count = 0;
                // First buffer is the Ethernet header
                iovecs[iovec_count].iov_base = &header;
                iovecs[iovec_count].iov_len = sizeof(EthernetPacketHeader);
                total_size += iovecs[iovec_count].iov_len;
                iovec_count++;
                // Second buffer is the Ethernet prefix
                iovecs[iovec_count].iov_base = &prefix;
                iovecs[iovec_count].iov_len = sizeof(EthernetPacketPrefix);
                total_size += iovecs[iovec_count].iov_len;
                iovec_count++;
                // Following buffers are the user data
                uint32_t payload_size = 0;
                for (const auto& buffer : buffers)
                {
                    iovecs[iovec_count].iov_base = const_cast<void*>(buffer.buffer);
                    iovecs[iovec_count].iov_len = buffer.size;
                    total_size += iovecs[iovec_count].iov_len;
                    payload_size += buffer.size;
                    iovec_count++;
                }

                if (total_bytes != payload_size)
                {
                    // Mismatch in total bytes calculation
                    return false;
                }

                if (payload_size < EthernetPacket::MIN_RTPS_PAYLOAD_SIZE)
                {
                    // Pad the payload to meet minimum Ethernet frame size
                    static uint8_t padding[EthernetPacket::MIN_RTPS_PAYLOAD_SIZE] = {};
                    iovecs[iovec_count].iov_base = padding;
                    iovecs[iovec_count].iov_len = EthernetPacket::MIN_RTPS_PAYLOAD_SIZE - payload_size;
                    total_size += iovecs[iovec_count].iov_len;
                    iovec_count++;
                }
                else if (payload_size > EthernetPacket::MAX_RTPS_PAYLOAD_SIZE)
                {
                    // Payload exceeds maximum Ethernet frame size
                    return false;
                }

                // Send to each destination locator
                LocatorsIterator& it = *destination_locators_begin;
                while (it != *destination_locators_end)
                {
                    const Locator& locator = *it;
                    ++it;
                    if (locator.kind == LOCATOR_KIND_ETHERNET)
                    {
                        statistics_info_.add_entry(locator);
                        statistics_info_.set_statistics_message_data(locator, buffers.back(), total_bytes);

                        // Set destination MAC address, port and pcp_vlan from locator
                        const EthernetLocator& eth_locator = reinterpret_cast<const EthernetLocator&>(locator);
                        memcpy(header.h_dest, eth_locator.address.data(), eth_locator.address.size());
                        prefix.dest_port = htons(EthernetLocator::get_logical_port(locator));
                        if (use_destination_vlan_config)
                        {
                            prefix.pcp_dei_vid = htons(eth_locator.get_pcp_dei_vid());
                        }

                        // Send the packet using sendmsg
                        SocketAddress_t locator_addr{};
                        locator_addr.sll_ifindex = if_index_;
                        locator_addr.sll_halen = ETH_ALEN;
                        memcpy(&locator_addr.sll_addr[0], eth_locator.address.data(), eth_locator.address.size());

                        // Fill net message structure
                        SocketSendmsgMessage_t net_msg{};
                        net_msg.msg_name = &locator_addr;
                        net_msg.msg_namelen = sizeof(locator_addr);
                        net_msg.msg_iov = &iovecs[0];
                        net_msg.msg_iovlen = static_cast<decltype(net_msg.msg_iovlen)>(iovec_count);

                        ssize_t send_ret = sendmsg(socket_, &net_msg, MSG_DONTWAIT);
                        if ((send_ret < 0) || (static_cast<uint64_t>(send_ret) != static_cast<uint64_t>(total_size)))
                        {
                            EPROSIMA_LOG_WARNING(RTPS_ETHERNET_TRANSPORT,
                                    "Error sending message to locator " << locator);
                        }

                    }
                }

                static_cast<void>(max_blocking_time);

                return true;
            };

    clean_up = [this]()
            {
                // Close output socket
                close(socket_);
            };
}

void EthernetSenderResource::add_locators_to_list(
        LocatorList& locators) const
{
    EthernetAddress my_address{};
    memcpy(my_address.data(), eth_header_.h_source, my_address.max_size());
    locators.push_back(EthernetLocator::create_locator(my_address, 0, 0, 0));
}

bool EthernetSenderResource::fill_prefix_from_priority(
        int32_t transport_priority,
        EthernetPacketPrefix& prefix)
{
    // Map transport priority to source port and PCP/DEI/VLAN ID
    auto it = priority_mapping_.find(transport_priority);
    if (it != priority_mapping_.end())
    {
        prefix.source_port = htons(it->second.source_port);

        uint16_t pcp = static_cast<uint16_t>(it->second.pcp) & 0x07u; // PCP is 3 bits
        uint16_t vlan_id = static_cast<uint16_t>(it->second.vlan_id) & 0x0FFFu; // VLAN ID is 12 bits
        prefix.pcp_dei_vid = htons((pcp << 13) | vlan_id);

        return true;
    }

    return false;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
