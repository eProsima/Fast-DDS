// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>

#include <algorithm>
#include <memory>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/BuiltinTransports.hpp>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <rtps/transport/shared_mem/SHMLocator.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

static bool is_intraprocess_only(
        const RTPSParticipantAttributes& att)
{
    return
        xmlparser::XMLProfileManager::library_settings().intraprocess_delivery == INTRAPROCESS_FULL &&
        att.builtin.discovery_config.ignoreParticipantFlags ==
        (ParticipantFilteringFlags::FILTER_DIFFERENT_HOST | ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS);
}

static std::shared_ptr<fastdds::rtps::SharedMemTransportDescriptor> create_shm_transport(
        const RTPSParticipantAttributes& att,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = std::make_shared<fastdds::rtps::SharedMemTransportDescriptor>();

    // We assume (Linux) UDP doubles the user socket buffer size in kernel, so
    // the equivalent segment size in SHM would be socket buffer size x 2
    auto segment_size_udp_equivalent =
            std::max(att.sendSocketBufferSize, att.listenSocketBufferSize) * 2;
    descriptor->segment_size(segment_size_udp_equivalent);
    // Needed for the maxMessageSizeBetweenTransports
    descriptor->maxMessageSize = options.maxMessageSize;
    descriptor->default_reception_threads(att.builtin_transports_reception_threads);
    return descriptor;
}

static std::shared_ptr<fastdds::rtps::UDPv4TransportDescriptor> create_udpv4_transport(
        const RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = std::make_shared<fastdds::rtps::UDPv4TransportDescriptor>();
    descriptor->maxMessageSize = options.maxMessageSize;
    descriptor->sendBufferSize = att.sendSocketBufferSize;
    descriptor->receiveBufferSize = att.listenSocketBufferSize;
    descriptor->non_blocking_send = options.non_blocking_send;
    descriptor->default_reception_threads(att.builtin_transports_reception_threads);
    if (intraprocess_only)
    {
        // Avoid multicast leaving the host for intraprocess-only participants
        descriptor->TTL = 0;
    }
    return descriptor;
}

static std::shared_ptr<fastdds::rtps::UDPv6TransportDescriptor> create_udpv6_transport(
        const RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
    descriptor->maxMessageSize = options.maxMessageSize;
    descriptor->sendBufferSize = att.sendSocketBufferSize;
    descriptor->receiveBufferSize = att.listenSocketBufferSize;
    descriptor->non_blocking_send = options.non_blocking_send;
    descriptor->default_reception_threads(att.builtin_transports_reception_threads);
    if (intraprocess_only)
    {
        // Avoid multicast leaving the host for intraprocess-only participants
        descriptor->TTL = 0;
    }
    return descriptor;
}

static std::shared_ptr<fastdds::rtps::TCPv4TransportDescriptor> create_tcpv4_transport(
        const RTPSParticipantAttributes& att,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = std::make_shared<fastdds::rtps::TCPv4TransportDescriptor>();
    descriptor->add_listener_port(0);
    descriptor->maxMessageSize = options.maxMessageSize;
    descriptor->sendBufferSize = att.sendSocketBufferSize;
    descriptor->receiveBufferSize = att.listenSocketBufferSize;
    descriptor->non_blocking_send = options.non_blocking_send;

    descriptor->calculate_crc = false;
    descriptor->check_crc = false;
    descriptor->apply_security = false;
    descriptor->enable_tcp_nodelay = true;
    descriptor->tcp_negotiation_timeout = options.tcp_negotiation_timeout;

    descriptor->default_reception_threads(att.builtin_transports_reception_threads);
    descriptor->accept_thread = att.builtin_transports_reception_threads;
    descriptor->keep_alive_thread = att.builtin_transports_reception_threads;
    return descriptor;
}

static std::shared_ptr<fastdds::rtps::TCPv6TransportDescriptor> create_tcpv6_transport(
        const RTPSParticipantAttributes& att,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = std::make_shared<fastdds::rtps::TCPv6TransportDescriptor>();
    descriptor->add_listener_port(0);
    descriptor->maxMessageSize = options.maxMessageSize;
    descriptor->sendBufferSize = att.sendSocketBufferSize;
    descriptor->receiveBufferSize = att.listenSocketBufferSize;
    descriptor->non_blocking_send = options.non_blocking_send;

    descriptor->calculate_crc = false;
    descriptor->check_crc = false;
    descriptor->apply_security = false;
    descriptor->enable_tcp_nodelay = true;
    descriptor->tcp_negotiation_timeout = options.tcp_negotiation_timeout;

    descriptor->default_reception_threads(att.builtin_transports_reception_threads);
    descriptor->accept_thread = att.builtin_transports_reception_threads;
    descriptor->keep_alive_thread = att.builtin_transports_reception_threads;
    return descriptor;
}

static void setup_transports_default(
        RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = create_udpv4_transport(att, intraprocess_only, options);

#ifdef SHM_TRANSPORT_BUILTIN
    if (!intraprocess_only)
    {
        auto shm_transport = create_shm_transport(att, options);
        // Use same default max_message_size on both UDP and SHM
        shm_transport->max_message_size(descriptor->max_message_size());
        att.userTransports.push_back(shm_transport);
    }
#endif // ifdef SHM_TRANSPORT_BUILTIN

    att.userTransports.push_back(descriptor);
}

static void setup_transports_defaultv6(
        RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = create_udpv6_transport(att, intraprocess_only, options);

#ifdef SHM_TRANSPORT_BUILTIN
    if (!intraprocess_only)
    {
        auto shm_transport = create_shm_transport(att, options);
        // Use same default max_message_size on both UDP and SHM
        shm_transport->max_message_size(descriptor->max_message_size());
        att.userTransports.push_back(shm_transport);
    }
#endif // ifdef SHM_TRANSPORT_BUILTIN

    att.userTransports.push_back(descriptor);
}

static void setup_transports_shm(
        RTPSParticipantAttributes& att,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
#ifdef FASTDDS_SHM_TRANSPORT_DISABLED
    static_cast<void>(att);
    EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Trying to configure SHM transport only, " <<
            "but Fast DDS was built without SHM transport support.");
#else
    auto descriptor = create_shm_transport(att, options);
    att.userTransports.push_back(descriptor);
#endif  // FASTDDS_SHM_TRANSPORT_DISABLED
}

static void setup_transports_udpv4(
        RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = create_udpv4_transport(att, intraprocess_only, options);
    att.userTransports.push_back(descriptor);
}

static void setup_transports_udpv6(
        RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    auto descriptor = create_udpv6_transport(att, intraprocess_only, options);
    att.userTransports.push_back(descriptor);
}

static void setup_transports_large_data(
        RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    if (!intraprocess_only)
    {
        auto shm_transport = create_shm_transport(att, options);
        att.userTransports.push_back(shm_transport);

        auto shm_loc = fastdds::rtps::SHMLocator::create_locator(0, fastdds::rtps::SHMLocator::Type::UNICAST);
        att.defaultUnicastLocatorList.push_back(shm_loc);

        auto tcp_transport = create_tcpv4_transport(att, options);
        att.userTransports.push_back(tcp_transport);

        Locator_t tcp_loc;
        tcp_loc.kind = LOCATOR_KIND_TCPv4;
        IPLocator::setIPv4(tcp_loc, "0.0.0.0");
        IPLocator::setPhysicalPort(tcp_loc, 0);
        IPLocator::setLogicalPort(tcp_loc, 0);
        att.builtin.metatrafficUnicastLocatorList.push_back(tcp_loc);
        att.defaultUnicastLocatorList.push_back(tcp_loc);
    }

    auto udp_descriptor = create_udpv4_transport(att, intraprocess_only, options);
    att.userTransports.push_back(udp_descriptor);

    if (!intraprocess_only)
    {
        Locator_t pdp_locator;
        pdp_locator.kind = LOCATOR_KIND_UDPv4;
        IPLocator::setIPv4(pdp_locator, "239.255.0.1");
        att.builtin.metatrafficMulticastLocatorList.push_back(pdp_locator);
    }
}

static void setup_transports_large_datav6(
        RTPSParticipantAttributes& att,
        bool intraprocess_only,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    if (!intraprocess_only)
    {
        auto shm_transport = create_shm_transport(att, options);
        att.userTransports.push_back(shm_transport);

        auto shm_loc = fastdds::rtps::SHMLocator::create_locator(0, fastdds::rtps::SHMLocator::Type::UNICAST);
        att.defaultUnicastLocatorList.push_back(shm_loc);

        auto tcp_transport = create_tcpv6_transport(att, options);
        att.userTransports.push_back(tcp_transport);

        Locator_t tcp_loc;
        tcp_loc.kind = LOCATOR_KIND_TCPv6;
        IPLocator::setIPv6(tcp_loc, "::");
        IPLocator::setPhysicalPort(tcp_loc, 0);
        IPLocator::setLogicalPort(tcp_loc, 0);
        att.builtin.metatrafficUnicastLocatorList.push_back(tcp_loc);
        att.defaultUnicastLocatorList.push_back(tcp_loc);
    }

    auto udp_descriptor = create_udpv6_transport(att, intraprocess_only, options);
    att.userTransports.push_back(udp_descriptor);

    if (!intraprocess_only)
    {
        Locator_t pdp_locator;
        pdp_locator.kind = LOCATOR_KIND_UDPv6;
        IPLocator::setIPv6(pdp_locator, "ff1e::ffff:efff:1");
        att.builtin.metatrafficMulticastLocatorList.push_back(pdp_locator);
    }
}

void RTPSParticipantAttributes::setup_transports(
        fastdds::rtps::BuiltinTransports transports,
        const fastdds::rtps::BuiltinTransportsOptions& options)
{
    if (options.maxMessageSize > fastdds::rtps::s_maximumMessageSize &&
            (transports != fastdds::rtps::BuiltinTransports::NONE &&
            transports != fastdds::rtps::BuiltinTransports::SHM &&
            transports != fastdds::rtps::BuiltinTransports::LARGE_DATA &&
            transports != fastdds::rtps::BuiltinTransports::LARGE_DATAv6))
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "Max message size of UDP cannot be greater than " << std::to_string(
                    fastdds::rtps::s_maximumMessageSize) << ".");
        return;
    }
    bool intraprocess_only = is_intraprocess_only(*this);

    sendSocketBufferSize = options.sockets_buffer_size;
    listenSocketBufferSize = options.sockets_buffer_size;

    switch (transports)
    {
        case fastdds::rtps::BuiltinTransports::NONE:
            break;

        case fastdds::rtps::BuiltinTransports::DEFAULT:
            setup_transports_default(*this, intraprocess_only, options);
            break;

        case fastdds::rtps::BuiltinTransports::DEFAULTv6:
            setup_transports_defaultv6(*this, intraprocess_only, options);
            break;

        case fastdds::rtps::BuiltinTransports::SHM:
            setup_transports_shm(*this, options);
            break;

        case fastdds::rtps::BuiltinTransports::UDPv4:
            setup_transports_udpv4(*this, intraprocess_only, options);
            break;

        case fastdds::rtps::BuiltinTransports::UDPv6:
            setup_transports_udpv6(*this, intraprocess_only, options);
            break;

        case fastdds::rtps::BuiltinTransports::LARGE_DATA:
            // This parameter will allow allow the initialization of UDP transports with maxMessageSize > 65500 KB (s_maximumMessageSize)
            max_msg_size_no_frag = options.maxMessageSize;
            setup_transports_large_data(*this, intraprocess_only, options);
            break;

        case fastdds::rtps::BuiltinTransports::LARGE_DATAv6:
            // This parameter will allow allow the initialization of UDP transports with maxMessageSize > 65500 KB (s_maximumMessageSize)
            max_msg_size_no_frag = options.maxMessageSize;
            setup_transports_large_datav6(*this, intraprocess_only, options);
            break;

        default:
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Setup for '" << transports << "' transport configuration not yet supported.");
            return;
    }

    useBuiltinTransports = false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
