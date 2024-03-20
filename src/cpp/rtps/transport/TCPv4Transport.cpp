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
