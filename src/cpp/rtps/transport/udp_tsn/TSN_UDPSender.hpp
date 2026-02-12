/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_TRANSPORT_UDP_TSN__TSN_UDPSENDER_HPP
#define RTPS_TRANSPORT_UDP_TSN__TSN_UDPSENDER_HPP

#include <cstdint>
#include <map>

#include <fastdds/rtps/transport/udp_tsn/UDPPriorityMappings.hpp>

#include <rtps/transport/UDPChannelResource.h>
#include <rtps/transport/UDPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief TSN_UDPSender class.
 *
 * Handles multiple UDP sockets with different configurations based on transport priority.
 */
struct TSN_UDPSender
{
    virtual ~TSN_UDPSender() = default;

    /**
     * @brief Constructor for TSN_UDPSender.
     *
     * @param mappings Mapping of transport priorities to DSCP values, source ports, and interfaces.
     */
    explicit TSN_UDPSender(
            const UDPPriorityMappings& mappings);

    /**
     * @brief Initialize the TSN_UDPSender with the given transport.
     *
     * It creates and configures UDP sockets for each transport priority defined in the mappings.
     *
     * @param transport Reference to the UDP transport interface.
     */
    bool init(
            UDPTransportInterface& transport);

    /**
     * @brief Get or create a socket based on the transport priority.
     *
     * @param[in] socket Reference to a default socket to use if no specific mapping is found.
     * @param[in] transport_priority Transport priority to select the socket configuration.
     */
    eProsimaUDPSocket& get_socket(
            eProsimaUDPSocket& default_socket,
            int32_t transport_priority);

protected:

    /**
     * @brief Set the DSCP value on the given socket.
     *
     * This method must be implemented by derived classes to handle platform-specific DSCP settings.
     *
     * @param socket Reference to the socket to configure.
     * @param dscp DSCP value to set (0-63).
     */
    virtual void set_dscp(
            eProsimaUDPSocket& socket,
            uint8_t dscp) const = 0;

private:

    //! Mapping of transport priority to sockets with the corresponding configuration.
    std::map<int32_t, eProsimaUDPSocket> priority_sockets_;
    //! Mapping of transport priority to DSCP values, source ports, and interfaces.
    const UDPPriorityMappings& priority_mappings_;

};

//! Sender for TSN over UDPv4.
struct TSN_UDPv4Sender : public TSN_UDPSender
{
    explicit TSN_UDPv4Sender(
            const UDPPriorityMappings& mappings)
        : TSN_UDPSender(mappings)
    {
    }

    ~TSN_UDPv4Sender() override = default;

protected:

    void set_dscp(
            eProsimaUDPSocket& socket,
            uint8_t dscp) const override;
};

//! Sender for TSN over UDPv6.
struct TSN_UDPv6Sender : public TSN_UDPSender
{
    explicit TSN_UDPv6Sender(
            const UDPPriorityMappings& mappings)
        : TSN_UDPSender(mappings)
    {
    }

    ~TSN_UDPv6Sender() override = default;

protected:

    void set_dscp(
            eProsimaUDPSocket& socket,
            uint8_t dscp) const override;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_TRANSPORT_UDP_TSN__TSN_UDPSENDER_HPP
