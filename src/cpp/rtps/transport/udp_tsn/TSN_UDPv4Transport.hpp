/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_TRANSPORT_UDP_TSN__TSN_UDPV4TRANSPORT_HPP
#define RTPS_TRANSPORT_UDP_TSN__TSN_UDPV4TRANSPORT_HPP

#include <chrono>
#include <cstdint>
#include <vector>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/common/LocatorsIterator.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv4TransportDescriptor.hpp>

#include <rtps/transport/UDPChannelResource.h>
#include <rtps/transport/UDPv4Transport.h>
#include <rtps/transport/udp_tsn/TSN_UDPSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct TSN_UDPv4Transport : public UDPv4Transport
{
    explicit TSN_UDPv4Transport(
            const TSN_UDPv4TransportDescriptor& descriptor);

    ~TSN_UDPv4Transport() override = default;

    bool init(
            const PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) override;

    bool send(
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes,
            eProsimaUDPSocket& socket,
            LocatorsIterator* destination_locators_begin,
            LocatorsIterator* destination_locators_end,
            bool only_multicast_purpose,
            bool whitelisted,
            const std::chrono::steady_clock::time_point& max_blocking_time_point,
            const int32_t transport_priority) override;

protected:

    /**
     * Set socket options before binding the output socket.
     *
     * @param socket Reference to the socket to set options on.
     */
    void set_output_pre_bind_options(
            eProsimaUDPSocket& socket) const override;

private:

    //! Sender handling TSN priorities.
    TSN_UDPv4Sender tsn_sender_;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_TRANSPORT_UDP_TSN__TSN_UDPV4TRANSPORT_HPP
