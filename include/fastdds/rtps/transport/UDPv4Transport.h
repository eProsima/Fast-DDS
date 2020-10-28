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

#ifndef _FASTDDS_UDPV4_TRANSPORT_H
#define _FASTDDS_UDPV4_TRANSPORT_H

#include <fastdds/rtps/transport/UDPTransportInterface.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This is a default UDPv4 implementation.
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
class UDPv4Transport : public UDPTransportInterface
{
public:

    RTPS_DllAPI UDPv4Transport(
            const UDPv4TransportDescriptor&);

    virtual ~UDPv4Transport() override;

    virtual const UDPTransportDescriptor* configuration() const override;

    /**
     * Starts listening on the specified port, and if the specified address is in the
     * multicast range, it joins the specified multicast group,
     */
    virtual bool OpenInputChannel(
            const fastrtps::rtps::Locator_t&,
            TransportReceiverInterface*,
            uint32_t) override;

    virtual fastrtps::rtps::LocatorList_t NormalizeLocator(
            const fastrtps::rtps::Locator_t& locator) override;

    virtual bool is_local_locator(
            const fastrtps::rtps::Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override
    {
        return &configuration_;
    }

    virtual void AddDefaultOutputLocator(
            fastrtps::rtps::LocatorList_t& defaultList) override;

    virtual bool getDefaultMetatrafficMulticastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t metatraffic_multicast_port) const override;

    virtual bool getDefaultMetatrafficUnicastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t unicast_port) const override;

protected:

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    UDPv4Transport();
    UDPv4TransportDescriptor configuration_;

    virtual bool compare_locator_ip(
            const fastrtps::rtps::Locator_t& lh,
            const fastrtps::rtps::Locator_t& rh) const override;
    virtual bool compare_locator_ip_and_port(
            const fastrtps::rtps::Locator_t& lh,
            const fastrtps::rtps::Locator_t& rh) const override;

    virtual void endpoint_to_locator(
            asio::ip::udp::endpoint& endpoint,
            fastrtps::rtps::Locator_t& locator) override;
    virtual void fill_local_ip(
            fastrtps::rtps::Locator_t& loc) const override;

    virtual asio::ip::udp::endpoint GenerateAnyAddressEndpoint(
            uint16_t port) override;
    virtual asio::ip::udp::endpoint generate_endpoint(
            uint16_t port) override;
    virtual asio::ip::udp::endpoint generate_endpoint(
            const std::string& sIp,
            uint16_t port) override;
    virtual asio::ip::udp::endpoint generate_endpoint(
            const fastrtps::rtps::Locator_t& loc,
            uint16_t port) override;
    virtual asio::ip::udp::endpoint generate_local_endpoint(
            const fastrtps::rtps::Locator_t& loc,
            uint16_t port) override;
    virtual asio::ip::udp generate_protocol() const override;
    virtual void get_ips(
            std::vector<fastrtps::rtps::IPFinder::info_IP>& locNames,
            bool return_loopback = false) override;
    virtual const std::string& localhost_name() override;
    eProsimaUDPSocket OpenAndBindInputSocket(
            const std::string& sIp,
            uint16_t port,
            bool is_multicast) override;

    //! Checks if the given interface is allowed by the white list.
    virtual bool is_interface_allowed(
            const std::string& interface) const override;

    /**
     * Method to get a list of interfaces to bind the socket associated to the given locator.
     * @return Vector of interfaces in string format.
     */
    virtual std::vector<std::string> get_binding_interfaces_list() override;

    //! Checks for whether locator is allowed.
    virtual bool is_locator_allowed(
            const fastrtps::rtps::Locator_t&) const override;

    //! Checks if the given interface is allowed by the white list.
    bool is_interface_allowed(
            const asio::ip::address_v4& ip) const;

    //! Checks if the interfaces white list is empty.
    virtual bool is_interface_whitelist_empty() const override;
    std::vector<asio::ip::address_v4> interface_whitelist_;

    virtual void set_receive_buffer_size(
            uint32_t size) override;
    virtual void set_send_buffer_size(
            uint32_t size) override;
    virtual void SetSocketOutboundInterface(
            eProsimaUDPSocket&,
            const std::string&) override;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UDPV4_TRANSPORT_H
