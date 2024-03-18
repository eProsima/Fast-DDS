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

#include <rtps/transport/TCPv6Transport.h>

#include <utility>
#include <cstring>
#include <algorithm>

#include <asio.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using IPLocator = fastrtps::rtps::IPLocator;
using octet = fastrtps::rtps::octet;

// TODO: move to SocketTransportInterface? not straightforward as overloaded in TCPv4Transport
static asio::ip::address_v6::bytes_type locator_to_native(
        Locator& locator)
{
    return{ { IPLocator::getIPv6(locator)[0],
        IPLocator::getIPv6(locator)[1],
        IPLocator::getIPv6(locator)[2],
        IPLocator::getIPv6(locator)[3],
        IPLocator::getIPv6(locator)[4],
        IPLocator::getIPv6(locator)[5],
        IPLocator::getIPv6(locator)[6],
        IPLocator::getIPv6(locator)[7],
        IPLocator::getIPv6(locator)[8],
        IPLocator::getIPv6(locator)[9],
        IPLocator::getIPv6(locator)[10],
        IPLocator::getIPv6(locator)[11],
        IPLocator::getIPv6(locator)[12],
        IPLocator::getIPv6(locator)[13],
        IPLocator::getIPv6(locator)[14],
        IPLocator::getIPv6(locator)[15] } };
}

TCPv6Transport::TCPv6Transport(
        const TCPv6TransportDescriptor& descriptor)
    : TCPTransportInterface(LOCATOR_KIND_TCPv6, descriptor)
    , configuration_(descriptor)
{
    fill_interface_whitelist_();

    // TODO: move to TCPTransportInterface if able to move interface_whitelist_ to SocketInterface
    if (!configuration_.listening_ports.empty())
    {
        if (configuration_.listening_ports.size() > 1)
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_TCPV6,
                    "Only one listening port is allowed for TCP transport. Only the first port will be used.");
            configuration_.listening_ports.erase(
                configuration_.listening_ports.begin() + 1, configuration_.listening_ports.end());
        }
        Locator locator(LOCATOR_KIND_TCPv6, configuration_.listening_ports.front());
        configuration_.listening_ports.front() = create_acceptor_socket(locator);
    }

#if !TLS_FOUND
    if (descriptor.apply_security)
    {
        EPROSIMA_LOG_ERROR(RTCP_TLSV6, "Trying to use TCP Transport with TLS but TLS was not found.");
    }
#endif // if !TLS_FOUND
}

TCPv6Transport::TCPv6Transport()
    : TCPTransportInterface(LOCATOR_KIND_TCPv6)
{
}

TCPv6Transport::~TCPv6Transport()
{
    clean();
}

TCPv6TransportDescriptor::TCPv6TransportDescriptor()
    : TCPTransportDescriptor()
{
}

TCPv6TransportDescriptor::TCPv6TransportDescriptor(
        const TCPv6TransportDescriptor& t)
    : TCPTransportDescriptor(t)
{
}

bool TCPv6TransportDescriptor::operator ==(
        const TCPv6TransportDescriptor& t) const
{
    return TCPTransportDescriptor::operator ==(t);
}

TransportInterface* TCPv6TransportDescriptor::create_transport() const
{
    return new TCPv6Transport(*this);
}

void TCPv6Transport::AddDefaultOutputLocator(
        LocatorList& /*defaultList*/)
{
}

const TCPTransportDescriptor* TCPv6Transport::configuration() const
{
    return &configuration_;
}

TCPTransportDescriptor* TCPv6Transport::configuration()
{
    return &configuration_;
}

uint16_t TCPv6Transport::GetLogicalPortRange() const
{
    return configuration_.logical_port_range;
}

uint16_t TCPv6Transport::GetLogicalPortIncrement() const
{
    return configuration_.logical_port_increment;
}

uint16_t TCPv6Transport::GetMaxLogicalPort() const
{
    return configuration_.max_logical_port;
}

void TCPv6Transport::fill_interface_whitelist_()
{
    if ((!configuration_.interfaceWhiteList.empty() || !configuration_.interface_allowlist.empty() ||
            !configuration_.interface_blocklist.empty()) && allowed_interfaces_.empty())
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_TCPV6, "All whitelist interfaces were filtered out");
        interface_whitelist_.emplace_back(ip::address_v6::from_string("2001:db8::"));
    }
    else
    {
        for (const auto& iface : allowed_interfaces_)
        {
            interface_whitelist_.emplace_back(ip::address_v6::from_string(iface.ip));
        }
    }
}

bool TCPv6Transport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool TCPv6Transport::is_interface_allowed(
        const std::string& iface) const
{
    return is_interface_allowed(asio::ip::address_v6::from_string(iface));
}

bool TCPv6Transport::is_interface_allowed(
        const Locator& loc) const
{
    asio::ip::address_v6 ip = asio::ip::address_v6::from_string(IPLocator::toIPv6string(loc));
    return is_interface_allowed(ip);
}

bool TCPv6Transport::is_interface_allowed(
        const ip::address_v6& ip) const
{
    if (interface_whitelist_.empty())
    {
        return true;
    }

    if (ip == ip::address_v6::any())
    {
        return true;
    }

    for (auto& whitelist : interface_whitelist_)
    {
        if (compare_ips(whitelist.to_string(), ip.to_string()))
        {
            return true;
        }
    }

    return false;
}

std::vector<std::string> TCPv6Transport::get_binding_interfaces_list()
{
    std::vector<std::string> vOutputInterfaces;
    if (is_interface_whitelist_empty())
    {
        vOutputInterfaces.push_back(s_IPv6AddressAny);
    }
    else
    {
        for (auto& ip : interface_whitelist_)
        {
            vOutputInterfaces.push_back(ip.to_string());
        }
    }

    return vOutputInterfaces;
}

bool TCPv6Transport::is_locator_allowed(
        const Locator& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (interface_whitelist_.empty())
    {
        return true;
    }
    return is_interface_allowed(IPLocator::toIPv6string(locator));
}

ip::tcp::endpoint TCPv6Transport::generate_endpoint(
        const Locator& loc,
        uint16_t port) const
{
    asio::ip::address_v6::bytes_type remoteAddress;
    IPLocator::copyIPv6(loc, remoteAddress.data());
    return ip::tcp::endpoint(asio::ip::address_v6(remoteAddress), port);
}

ip::tcp::endpoint TCPv6Transport::generate_local_endpoint(
        Locator& loc,
        uint16_t port) const
{
    return ip::tcp::endpoint(asio::ip::address_v6(locator_to_native(loc)), port);
}

ip::tcp::endpoint TCPv6Transport::generate_endpoint(
        uint16_t port) const
{
    return asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port);
}

asio::ip::tcp TCPv6Transport::generate_protocol() const
{
    return asio::ip::tcp::v6();
}


void TCPv6Transport::set_receive_buffer_size(
        uint32_t size)
{
    configuration_.receiveBufferSize = size;
}

void TCPv6Transport::set_send_buffer_size(
        uint32_t size)
{
    configuration_.sendBufferSize = size;
}

void TCPv6Transport::endpoint_to_locator(
        const ip::tcp::endpoint& endpoint,
        Locator& locator) const
{
    locator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    IPLocator::setIPv6(locator, ipBytes.data());
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
