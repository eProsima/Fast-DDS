// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransport.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastdds/rtps/network/ReceiverResource.h>
#include <fastdds/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastdds{
namespace rtps{

using IPFinder = fastrtps::rtps::IPFinder;
using IPLocator = fastrtps::rtps::IPLocator;
using Locator_t = fastrtps::rtps::Locator_t;
using LocatorList_t = fastrtps::rtps::LocatorList_t;
using Log = dds::Log;

std::vector<std::shared_ptr<Port>> eProsimaSharedMem::global_ports_;
std::mutex eProsimaSharedMem::global_ports_mutex_;

static void get_ipv4s(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
            locNames.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;});
    locNames.erase(new_end, locNames.end());
    std::for_each(locNames.begin(), locNames.end(), [](IPFinder::info_IP& loc)
    {
        loc.locator.kind = LOCATOR_KIND_UDPv4;
    });
}

static void get_ipv4s_unique_interfaces(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false)
{
    get_ipv4s(locNames, return_loopback);
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.dev < b.dev;});
    auto new_end = std::unique(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.type != IPFinder::IP4_LOCAL && b.type != IPFinder::IP4_LOCAL && a.dev == b.dev;});
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v4::bytes_type locator_to_native(const Locator_t& locator)
{
    if (IPLocator::hasWan(locator))
    {
        return{ { IPLocator::getWan(locator)[0],
            IPLocator::getWan(locator)[1],
            IPLocator::getWan(locator)[2],
            IPLocator::getWan(locator)[3] } };
    }
    else
    {
        return{ { IPLocator::getIPv4(locator)[0],
            IPLocator::getIPv4(locator)[1],
            IPLocator::getIPv4(locator)[2],
            IPLocator::getIPv4(locator)[3] } };
    }
}

SharedMemTransport::SharedMemTransport(const SharedMemTransportDescriptor& descriptor)
    : SharedMemTransportInterface(LOCATOR_KIND_UDPv4)
    , configuration_(descriptor)
{
    mSendBufferSize = descriptor.sendBufferSize;
    mReceiveBufferSize = descriptor.receiveBufferSize;
    if (!descriptor.interfaceWhiteList.empty())
    {
        const auto white_begin = descriptor.interfaceWhiteList.begin();
        const auto white_end = descriptor.interfaceWhiteList.end();

        std::vector<IPFinder::info_IP> local_interfaces;
        get_ipv4s(local_interfaces, true);
        for (const IPFinder::info_IP& infoIP : local_interfaces)
        {
            if(std::find(white_begin, white_end, infoIP.name) != white_end)
            {
                interface_whitelist_.emplace_back(ip::address_v4::from_string(infoIP.name));
            }
        }

        if (interface_whitelist_.empty())
        {
            logError(TRANSPORT, "All whitelist interfaces where filtered out");
            interface_whitelist_.emplace_back(ip::address_v4::from_string("192.0.2.0"));
        }
    }
}

SharedMemTransport::SharedMemTransport()
    : SharedMemTransportInterface(LOCATOR_KIND_UDPv4)
{
}

SharedMemTransport::~SharedMemTransport()
{
    clean();
}


SharedMemTransportDescriptor::SharedMemTransportDescriptor()
	: SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
	, m_output_udp_socket(0)
{
}

SharedMemTransportDescriptor::SharedMemTransportDescriptor(const SharedMemTransportDescriptor& t)
	: SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
	, m_output_udp_socket(t.m_output_udp_socket)
{
}

TransportInterface* SharedMemTransportDescriptor::create_transport() const
{
    return new SharedMemTransport(*this);
}

bool SharedMemTransport::getDefaultMetatrafficMulticastLocators(
        LocatorList_t &locators,
        uint32_t metatraffic_multicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locators.push_back(locator);
    return true;
}

bool SharedMemTransport::getDefaultMetatrafficUnicastLocators(
        LocatorList_t &locators,
        uint32_t metatraffic_unicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    locator.set_Invalid_Address();
    locators.push_back(locator);

    return true;
}

bool SharedMemTransport::getDefaultUnicastLocators(
        LocatorList_t &locators,
        uint32_t unicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.set_Invalid_Address();
    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);

    return true;
}

void SharedMemTransport::AddDefaultOutputLocator(LocatorList_t &defaultList)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "239.255.0.1", configuration_.m_output_udp_socket, locator);
    defaultList.push_back(locator);
}

bool SharedMemTransport::compare_locator_ip(
        const Locator_t& lh,
        const Locator_t& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool SharedMemTransport::compare_locator_ip_and_port(
        const Locator_t& lh,
        const Locator_t& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void SharedMemTransport::endpoint_to_locator(
        ip::udp::endpoint& endpoint,
        Locator_t& locator)
{
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    IPLocator::setIPv4(locator, ipBytes.data());
}

void SharedMemTransport::fill_local_ip(Locator_t& loc) const
{
    IPLocator::setIPv4(loc, "127.0.0.1");
    loc.kind = LOCATOR_KIND_UDPv4;
}

const SharedMemTransportDescriptor* SharedMemTransport::configuration() const
{
    return &configuration_;
}

asio::ip::udp::endpoint SharedMemTransport::GenerateAnyAddressEndpoint(uint16_t port)
{
    return ip::udp::endpoint(ip::address_v4::any(), port);
}

ip::udp::endpoint SharedMemTransport::generate_endpoint(
        const Locator_t& loc,
        uint16_t port)
{
    asio::ip::address_v4::bytes_type remoteAddress;
    IPLocator::copyIPv4(loc, remoteAddress.data());
    return ip::udp::endpoint(asio::ip::address_v4(remoteAddress), port);
}

ip::udp::endpoint SharedMemTransport::generate_endpoint(
        const std::string& sIp,
        uint16_t port)
{
    return asio::ip::udp::endpoint(ip::address_v4::from_string(sIp), port);
}

ip::udp::endpoint SharedMemTransport::generate_endpoint(uint16_t port)
{
    return asio::ip::udp::endpoint(asio::ip::udp::v4(), port);
}

ip::udp::endpoint SharedMemTransport::generate_local_endpoint(
        const Locator_t& loc,
        uint16_t port)
{
    return ip::udp::endpoint(asio::ip::address_v4(locator_to_native(loc)), port);
}

asio::ip::udp SharedMemTransport::generate_protocol() const
{
    return ip::udp::v4();
}

void SharedMemTransport::get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback)
{
    get_ipv4s(locNames, return_loopback);
}

/*eProsimaSharedMem SharedMemTransport::OpenAndBindInputSocket(
        const std::string& sIp,
        uint16_t port,
        bool is_multicast)
{
	eProsimaSharedMem socket = createUDPSocket(io_service_);
    getSocketPtr(socket)->open(generate_protocol());
    if (mReceiveBufferSize != 0)
    {
        getSocketPtr(socket)->set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    }

    if (is_multicast)
    {
        getSocketPtr(socket)->set_option(ip::udp::socket::reuse_address(true));
#if defined(__QNX__)
        getSocketPtr(socket)->set_option(asio::detail::socket_option::boolean<
            ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT>(true));
#endif
    }

    getSocketPtr(socket)->bind(generate_endpoint(sIp, port));
    return socket;
}*/

bool SharedMemTransport::OpenInputChannel(
        const Locator_t& locator,
        TransportReceiverInterface* receiver,
        uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!is_locator_allowed(locator))
        return false;

	if (!IsInputChannelOpen(locator)) {
		try
		{
			auto channel_resource = CreateInputChannelResource(locator, IPLocator::isMulticast(locator), maxMsgSize, receiver);
			mInputSockets.push_back(channel_resource);
		}
		catch (std::exception& e)
		{
			logInfo(RTPS_MSG_OUT, "SharedMemTransport Error binding at port: (" << std::to_string(locator.port) << ")"
				<< " with msg: " << e.what());
			return false;
		}
	}

	return true;
}

std::vector<std::string> SharedMemTransport::get_binding_interfaces_list()
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

bool SharedMemTransport::is_interface_allowed(const std::string& interface) const
{
    return is_interface_allowed(asio::ip::address_v4::from_string(interface));
}

bool SharedMemTransport::is_interface_allowed(const ip::address_v4& ip) const
{
    if (interface_whitelist_.empty())
        return true;

    if (ip == ip::address_v4::any())
        return true;

    return find(interface_whitelist_.begin(), interface_whitelist_.end(), ip) != interface_whitelist_.end();
}

bool SharedMemTransport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool SharedMemTransport::is_locator_allowed(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (interface_whitelist_.empty() || IPLocator::isMulticast(locator))
    {
        return true;
    }
    return is_interface_allowed(IPLocator::toIPv4string(locator));
}

LocatorList_t SharedMemTransport::NormalizeLocator(const Locator_t& locator)
{
    LocatorList_t list;

    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        get_ipv4s(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v4::from_string(infoIP.name);
            if (is_interface_allowed(ip))
            {
                Locator_t newloc(locator);
                IPLocator::setIPv4(newloc, infoIP.locator);
                list.push_back(newloc);
            }
        }
        if (list.empty())
        {
            Locator_t newloc(locator);
            IPLocator::setIPv4(newloc, "127.0.0.1");
            list.push_back(newloc);
        }
    }
    else
    {
        list.push_back(locator);
    }

    return list;
}

bool SharedMemTransport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv4);

    if(IPLocator::isLocal(locator))
        return true;

    for(auto localInterface : currentInterfaces)
        if(IPLocator::compareAddress(locator, localInterface.locator))
        {
            return true;
        }

    return false;
}

void SharedMemTransport::set_receive_buffer_size(uint32_t size)
{
    configuration_.receiveBufferSize = size;
}

void SharedMemTransport::set_send_buffer_size(uint32_t size)
{
    configuration_.sendBufferSize = size;
}

/*void SharedMemTransport::SetSocketOutboundInterface(
		eProsimaSharedMem& socket,
        const std::string& sIp)
{
    getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string(sIp)));
}*/

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
