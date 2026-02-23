/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../../network/asio.hpp"

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/UDPTransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/UDPPriorityMappings.hpp>
#include <fastdds/utils/IPFinder.hpp>

#include <rtps/transport/UDPChannelResource.h>
#include <rtps/transport/UDPTransportInterface.h>
#include <rtps/transport/udp_tsn/TSN_UDPSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

TSN_UDPSender::TSN_UDPSender(
        const UDPPriorityMappings& mappings)
    : priority_mappings_(mappings)
{
}

bool TSN_UDPSender::init(
        UDPTransportInterface& transport)
{
    std::vector<fastdds::rtps::IPFinder::info_IP> interfaces;
    transport.get_ips(interfaces, true, false);

    for (const auto& entry : priority_mappings_)
    {
        if (priority_sockets_.find(entry.first) != priority_sockets_.end())
        {
            // Already initialized
            EPROSIMA_LOG_WARNING(UDP, "Duplicate transport priority mapping for priority " << entry.first);
            continue;
        }

        const UDPPriorityMapping& mapping = entry.second;
        const UDPTransportDescriptor* config = transport.configuration();
        uint16_t source_port = mapping.source_port != 0 ? mapping.source_port : config->m_output_udp_socket;

        // interface
        std::string interface_ip = mapping.interface;
        if (!interface_ip.empty())
        {
            auto it = std::find_if(interfaces.begin(), interfaces.end(),
                            [&interface_ip](const fastdds::rtps::IPFinder::info_IP& info)
                            {
                                return info.name == interface_ip || info.dev == interface_ip;
                            });
            if (it == interfaces.end())
            {
                EPROSIMA_LOG_ERROR(UDP, "Interface " << mapping.interface << " not found for transport priority "
                                                     << entry.first);
                return false;
            }
            if (transport.is_interface_allowed(it->name))
            {
                interface_ip = it->name;
            }
            else
            {
                EPROSIMA_LOG_ERROR(UDP,
                        "Interface " << mapping.interface << " not allowed by transport for transport priority "
                                     << entry.first);
                return false;
            }
        }

        try
        {
            auto endpoint = transport.GenerateAnyAddressEndpoint(source_port);
            if (!interface_ip.empty())
            {
                endpoint = transport.generate_endpoint(interface_ip, source_port);
            }
            auto socket = transport.OpenAndBindUnicastOutputSocket(endpoint, source_port);
            getSocketPtr(socket)->set_option(asio::ip::multicast::enable_loopback(true));

            if (mapping.dscp > 0)
            {
                uint8_t dscp = mapping.dscp;
                if (dscp > 63)
                {
                    EPROSIMA_LOG_WARNING(UDP, "DSCP value " << static_cast<int>(dscp)
                                                            << " out of range [0..63] for transport priority "
                                                            << entry.first
                                                            << ". DSCP will not be set");
                }
                else
                {
                    set_dscp(socket, dscp);
                }
            }

            priority_sockets_.emplace(entry.first, std::move(socket));
        }
        catch (const asio::system_error& e)
        {
            EPROSIMA_LOG_ERROR(UDP, "Failed to create socket for transport priority " << entry.first
                                                                                      << " with source port "
                                                                                      << source_port << ": "
                                                                                      << e.what());
            return false;
        }
    }

    return true;
}

eProsimaUDPSocket& TSN_UDPSender::get_socket(
        eProsimaUDPSocket& default_socket,
        int32_t transport_priority)
{
    auto it = priority_sockets_.find(transport_priority);
    if (it != priority_sockets_.end())
    {
        return it->second;
    }
    return default_socket;
}

void TSN_UDPv4Sender::set_dscp(
        eProsimaUDPSocket& socket,
        uint8_t dscp) const
{
    asio::error_code ec;
    // DSCP is in the upper 6 bits of the TOS field
    asio::detail::socket_option::integer<IPPROTO_IP, IP_TOS> tos(dscp << 2);
    getSocketPtr(socket)->set_option(tos, ec);
    if (ec)
    {
        EPROSIMA_LOG_WARNING(UDP, "Failed to set DSCP value " << static_cast<int>(dscp) << ": " << ec.message());
    }
}

void TSN_UDPv6Sender::set_dscp(
        eProsimaUDPSocket& socket,
        uint8_t dscp) const
{
    asio::error_code ec;
    // DSCP is in the upper 6 bits of the Traffic Class field
    asio::detail::socket_option::integer<IPPROTO_IPV6, IPV6_TCLASS> tclass (dscp << 2);
    getSocketPtr(socket)->set_option(tclass, ec);
    if (ec)
    {
        EPROSIMA_LOG_WARNING(UDP, "Failed to set DSCP value " << static_cast<int>(dscp) << ": " << ec.message());
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
