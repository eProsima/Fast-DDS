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

#include <rtps/transport/TCPv4Transport.h>

#include <utility>
#include <cstring>
#include <algorithm>

#include <asio.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using octet = fastrtps::rtps::octet;
using IPLocator = fastrtps::rtps::IPLocator;

// TODO: move to SocketTransportInterface? not straightforward as overloaded in TCPv4Transport
static asio::ip::address_v4::bytes_type locator_to_native(
        Locator& locator,
        const octet* local_wan)
{
    const octet* wan = IPLocator::getWan(locator);
    if (IPLocator::hasWan(locator) && (memcmp(local_wan, wan, 4) != 0))
    {
        return{ { wan[0], wan[1], wan[2], wan[3]} };
    }
    else
    {
        return{ { IPLocator::getIPv4(locator)[0],
            IPLocator::getIPv4(locator)[1],
            IPLocator::getIPv4(locator)[2],
            IPLocator::getIPv4(locator)[3]} };
    }
}

TCPv4Transport::TCPv4Transport(
        const TCPv4TransportDescriptor& descriptor)
    : TCPTransportInterface(LOCATOR_KIND_TCPv4, descriptor)
    , configuration_(descriptor)
{
    fill_interface_whitelist_();

    // TODO: move to TCPTransportInterface if able to move interface_whitelist_ to SocketInterface
    if (!configuration_.listening_ports.empty())
    {
        if (configuration_.listening_ports.size() > 1)
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_TCPV4,
                    "Only one listening port is allowed for TCP transport. Only the first port will be used.");
            configuration_.listening_ports.erase(
                configuration_.listening_ports.begin() + 1, configuration_.listening_ports.end());
        }
        Locator locator(LOCATOR_KIND_TCPv4, configuration_.listening_ports.front());
        configuration_.listening_ports.front() = create_acceptor_socket(locator);
    }

#if !TLS_FOUND
    if (descriptor.apply_security)
    {
        EPROSIMA_LOG_ERROR(RTCP_TLSV4, "Trying to use TCP Transport with TLS but TLS was not found.");
    }
#endif // if !TLS_FOUND
}

TCPv4Transport::TCPv4Transport()
    : TCPTransportInterface(LOCATOR_KIND_TCPv4)
{
}

TCPv4Transport::~TCPv4Transport()
{
    clean();
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor()
    : TCPTransportDescriptor()
{
    memset(wan_addr, 0, 4);
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor(
        const TCPv4TransportDescriptor& t)
    : TCPTransportDescriptor(t)
{
    memcpy(wan_addr, t.wan_addr, 4);
}

TCPv4TransportDescriptor& TCPv4TransportDescriptor::operator =(
        const TCPv4TransportDescriptor& t)
{
    *static_cast<TCPTransportDescriptor*>(this) = t;
    memcpy(wan_addr, t.wan_addr, 4);

    return *this;
}

bool TCPv4TransportDescriptor::operator ==(
        const TCPv4TransportDescriptor& t) const
{
    return (this->wan_addr[0] == t.wan_addr[0] &&
           this->wan_addr[1] == t.wan_addr[1] &&
           this->wan_addr[2] == t.wan_addr[2] &&
           this->wan_addr[3] == t.wan_addr[3] &&
           TCPTransportDescriptor::operator ==(t));
}

TransportInterface* TCPv4TransportDescriptor::create_transport() const
{
    return new TCPv4Transport(*this);
}

void TCPv4Transport::AddDefaultOutputLocator(
        LocatorList&)
{
}

const TCPTransportDescriptor* TCPv4Transport::configuration() const
{
    return &configuration_;
}

TCPTransportDescriptor* TCPv4Transport::configuration()
{
    return &configuration_;
}

uint16_t TCPv4Transport::GetLogicalPortIncrement() const
{
    return configuration_.logical_port_increment;
}

uint16_t TCPv4Transport::GetLogicalPortRange() const
{
    return configuration_.logical_port_range;
}

uint16_t TCPv4Transport::GetMaxLogicalPort() const
{
    return configuration_.max_logical_port;
}

void TCPv4Transport::fill_interface_whitelist_()
{
    if ((!configuration_.interfaceWhiteList.empty() || !configuration_.interface_allowlist.empty() ||
            !configuration_.interface_blocklist.empty()) && allowed_interfaces_.empty())
    {
        EPROSIMA_LOG_ERROR(TRANSPORT_TCPV4, "All whitelist interfaces were filtered out");
        interface_whitelist_.emplace_back(ip::address_v4::from_string("192.0.2.0"));
    }
    else
    {
        for (const auto& iface : allowed_interfaces_)
        {
            interface_whitelist_.emplace_back(ip::address_v4::from_string(iface.ip));
        }
    }
}

bool TCPv4Transport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool TCPv4Transport::is_interface_allowed(
        const std::string& iface) const
{
    return is_interface_allowed(asio::ip::address_v4::from_string(iface));
}

bool TCPv4Transport::is_interface_allowed(
        const Locator& loc) const
{
    asio::ip::address_v4 ip = asio::ip::address_v4::from_string(IPLocator::toIPv4string(loc));
    return is_interface_allowed(ip);
}

bool TCPv4Transport::is_interface_allowed(
        const ip::address_v4& ip) const
{
    if (interface_whitelist_.empty())
    {
        return true;
    }

    if (ip == ip::address_v4::any())
    {
        return true;
    }

    return find(interface_whitelist_.begin(), interface_whitelist_.end(), ip) != interface_whitelist_.end();
}

std::vector<std::string> TCPv4Transport::get_binding_interfaces_list()
{
    std::vector<std::string> vOutputInterfaces;
    if (is_interface_whitelist_empty())
    {
        vOutputInterfaces.push_back(s_IPv4AddressAny);
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

bool TCPv4Transport::is_locator_allowed(
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
    return is_interface_allowed(IPLocator::toIPv4string(locator));
}

ip::tcp::endpoint TCPv4Transport::generate_endpoint(
        const Locator& loc,
        uint16_t port) const
{
    asio::ip::address_v4::bytes_type remoteAddress;
    IPLocator::copyIPv4(loc, remoteAddress.data());
    return ip::tcp::endpoint(asio::ip::address_v4(remoteAddress), port);
}

ip::tcp::endpoint TCPv4Transport::generate_local_endpoint(
        Locator& loc,
        uint16_t port) const
{
    return ip::tcp::endpoint(asio::ip::address_v4(locator_to_native(loc, configuration_.wan_addr)), port);
}

ip::tcp::endpoint TCPv4Transport::generate_endpoint(
        uint16_t port) const
{
    return asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
}

asio::ip::tcp TCPv4Transport::generate_protocol() const
{
    return asio::ip::tcp::v4();
}


void TCPv4Transport::set_receive_buffer_size(
        uint32_t size)
{
    configuration_.receiveBufferSize = size;
}

void TCPv4Transport::set_send_buffer_size(
        uint32_t size)
{
    configuration_.sendBufferSize = size;
}

void TCPv4Transport::endpoint_to_locator(
        const ip::tcp::endpoint& endpoint,
        Locator& locator) const
{
    locator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    IPLocator::setIPv4(locator, ipBytes.data());
}

bool TCPv4Transport::fillMetatrafficUnicastLocator(
        Locator& locator,
        uint32_t metatraffic_unicast_port) const
{
    bool result = TCPTransportInterface::fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);

    IPLocator::setWan(locator,
            configuration_.wan_addr[0], configuration_.wan_addr[1],
            configuration_.wan_addr[2], configuration_.wan_addr[3]);

    return result;
}

bool TCPv4Transport::fillUnicastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    bool result = TCPTransportInterface::fillUnicastLocator(locator, well_known_port);

    IPLocator::setWan(locator,
            configuration_.wan_addr[0], configuration_.wan_addr[1],
            configuration_.wan_addr[2], configuration_.wan_addr[3]);

    return result;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
