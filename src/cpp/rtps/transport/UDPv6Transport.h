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

#ifndef _FASTDDS_UDPV6_TRANSPORT_H_
#define _FASTDDS_UDPV6_TRANSPORT_H_

#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <rtps/transport/UDPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This is a default UDPv6 implementation.
 *    - Opening an output channel by passing a locator will open a socket per interface on the given port.
 *       This collection of sockets constitute the "outbound channel". In other words, a channel corresponds
 *       to a port + a direction.
 *
 *    - It is possible to provide a white list at construction, which limits the interfaces the transport
 *       will ever be able to interact with. If left empty, all interfaces are allowed.
 *
 *    - Opening an input channel by passing a locator will open a socket listening on the given port on every
 *       whitelisted interface, and join the multicast channel specified by the locator address. Hence, any locator
 *       that does not correspond to the multicast range will simply open the port without a subsequent join. Joining
 *       multicast groups late is supported by attempting to open the channel again with the same port + a
 *       multicast address (the OpenInputChannel function will fail, however, because no new channel has been
 *       opened in a strict sense).
 * @ingroup TRANSPORT_MODULE
 */
class UDPv6Transport : public UDPTransportInterface
{
public:

    RTPS_DllAPI UDPv6Transport(
            const UDPv6TransportDescriptor&);

    ~UDPv6Transport() override;

    const UDPTransportDescriptor* configuration() const override;

    /**
     * Starts listening on the specified port, and if the specified address is in the
     * multicast range, it joins the specified multicast group,
     */
    bool OpenInputChannel(
            const Locator&,
            TransportReceiverInterface*,
            uint32_t) override;

    TransportDescriptorInterface* get_configuration() override
    {
        return &configuration_;
    }

    bool getDefaultMetatrafficMulticastLocators(
            LocatorList& locators,
            uint32_t metatraffic_multicast_port) const override;

    bool getDefaultMetatrafficUnicastLocators(
            LocatorList& locators,
            uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
            LocatorList& locators,
            uint32_t unicast_port) const override;

    void AddDefaultOutputLocator(
            LocatorList& defaultList) override;

    void update_network_interfaces() override;

protected:

    UDPv6TransportDescriptor configuration_;

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    UDPv6Transport();

    void endpoint_to_locator(
            asio::ip::udp::endpoint& endpoint,
            Locator& locator) override;

    asio::ip::udp::endpoint GenerateAnyAddressEndpoint(
            uint16_t port) override;
    asio::ip::udp::endpoint generate_endpoint(
            uint16_t port) override;
    asio::ip::udp::endpoint generate_endpoint(
            const std::string& sIp,
            uint16_t port) override;
    asio::ip::udp::endpoint generate_endpoint(
            const Locator& loc,
            uint16_t port) override;
    asio::ip::udp::endpoint generate_local_endpoint(
            const Locator& loc,
            uint16_t port) override;
    asio::ip::udp generate_protocol() const override;
    eProsimaUDPSocket OpenAndBindInputSocket(
            const std::string& sIp,
            uint16_t port,
            bool is_multicast) override;

    void set_receive_buffer_size(
            uint32_t size) override;
    void set_send_buffer_size(
            uint32_t size) override;
    void SetSocketOutboundInterface(
            eProsimaUDPSocket&,
            const std::string&) override;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UDPV6_TRANSPORT_H_
