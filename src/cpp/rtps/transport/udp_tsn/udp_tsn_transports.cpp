/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <chrono>
#include <cstdint>
#include <vector>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/common/LocatorsIterator.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv6TransportDescriptor.hpp>

#include <rtps/transport/UDPChannelResource.h>
#include <rtps/transport/UDPv4Transport.h>
#include <rtps/transport/UDPv6Transport.h>
#include <rtps/transport/udp_tsn/TSN_UDPv4Transport.hpp>
#include <rtps/transport/udp_tsn/TSN_UDPv6Transport.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

TSN_UDPv4Transport::TSN_UDPv4Transport(
        const TSN_UDPv4TransportDescriptor& descriptor)
    : UDPv4Transport(descriptor)
    , tsn_sender_(descriptor.priority_mapping)
{
}

bool TSN_UDPv4Transport::init(
        const PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    bool ok = UDPv4Transport::init(properties, max_msg_size_no_frag);
    if (ok)
    {
        ok = tsn_sender_.init(*this);
    }
    return ok;
}

bool TSN_UDPv4Transport::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        eProsimaUDPSocket& socket,
        LocatorsIterator* destination_locators_begin,
        LocatorsIterator* destination_locators_end,
        bool only_multicast_purpose,
        bool whitelisted,
        const std::chrono::steady_clock::time_point& max_blocking_time_point,
        const int32_t transport_priority)
{
    eProsimaUDPSocket& tsn_socket = tsn_sender_.get_socket(socket, transport_priority);
    return UDPv4Transport::send(buffers, total_bytes, tsn_socket, destination_locators_begin,
                   destination_locators_end, only_multicast_purpose, whitelisted,
                   max_blocking_time_point, transport_priority);
}

void TSN_UDPv4Transport::set_output_pre_bind_options(
        eProsimaUDPSocket& socket) const
{
    // Call base implementation
    UDPv4Transport::set_output_pre_bind_options(socket);

    // Allow address reuse
    socket.set_option(asio::ip::udp::socket::reuse_address(true));
}

TSN_UDPv6Transport::TSN_UDPv6Transport(
        const TSN_UDPv6TransportDescriptor& descriptor)
    : UDPv6Transport(descriptor)
    , tsn_sender_(descriptor.priority_mapping)
{
}

bool TSN_UDPv6Transport::init(
        const PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    bool ok = UDPv6Transport::init(properties, max_msg_size_no_frag);
    if (ok)
    {
        ok = tsn_sender_.init(*this);
    }
    return ok;
}

bool TSN_UDPv6Transport::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        eProsimaUDPSocket& socket,
        LocatorsIterator* destination_locators_begin,
        LocatorsIterator* destination_locators_end,
        bool only_multicast_purpose,
        bool whitelisted,
        const std::chrono::steady_clock::time_point& max_blocking_time_point,
        const int32_t transport_priority)
{
    eProsimaUDPSocket& tsn_socket = tsn_sender_.get_socket(socket, transport_priority);
    return UDPv6Transport::send(buffers, total_bytes, tsn_socket, destination_locators_begin,
                   destination_locators_end, only_multicast_purpose, whitelisted,
                   max_blocking_time_point, transport_priority);
}

void TSN_UDPv6Transport::set_output_pre_bind_options(
        eProsimaUDPSocket& socket) const
{
    // Call base implementation
    UDPv6Transport::set_output_pre_bind_options(socket);

    // Allow address reuse
    socket.set_option(asio::ip::udp::socket::reuse_address(true));
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
