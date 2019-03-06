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

#ifndef TCPV4_TRANSPORT_H
#define TCPV4_TRANSPORT_H

#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/transport/tcp/RTCPHeader.h>


#include <asio.hpp>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * This is a default TCPv4 implementation.
 *    - Opening an output channel by passing a remote locator will try to open a TCP conection with the endpoint.
 *       If there is created a connection with the same endpoint, the transport will use the same one.
 *
 *    - It is possible to provide a white list at construction, which limits the interfaces the transport
 *       will ever be able to interact with. If left empty, all interfaces are allowed.
 *
 *    - Opening an input channel by passing a locator will open a socket listening on the given physical port on every
 *       whitelisted interface, it will wait for incomming connections until the receiver closes the channel.
 *       Several endpoints can connect to other to the same physical port, because the OS creates a connection socket
 *       after each establishment.
 * @ingroup TRANSPORT_MODULE
 */
class TCPv4Transport : public TCPTransportInterface
{
protected:
    TCPv4TransportDescriptor configuration_;
    std::vector<asio::ip::address_v4> interface_whitelist_;

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    TCPv4Transport();

    virtual bool compare_locator_ip(
        const Locator_t& lh,
        const Locator_t& rh) const override;

    virtual bool compare_locator_ip_and_port(
        const Locator_t& lh,
        const Locator_t& rh) const override;

    virtual void fill_local_ip(Locator_t& loc) const override;

    virtual asio::ip::tcp::endpoint generate_endpoint(uint16_t port) const override;

    virtual asio::ip::tcp::endpoint generate_endpoint(
        const Locator_t& loc,
        uint16_t port) const override;

    virtual asio::ip::tcp::endpoint generate_local_endpoint(
        Locator_t& loc,
        uint16_t port) const override;

    virtual asio::ip::tcp generate_protocol() const override;

    virtual asio::ip::tcp get_protocol_type() const override { return asio::ip::tcp::v4(); }

    virtual void get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false) const override;

    /**
    * Method to get a list of interfaces to bind the socket associated to the given locator.
    * @return Vector of interfaces in string format.
    */
    virtual std::vector<std::string> get_binding_interfaces_list() override;

    bool is_locator_allowed(const Locator_t& locator) const override;

    //! Checks if the given ip has been included in the white list to use it.
    virtual bool is_interface_allowed(const std::string& interface) const override;

    //! Checks if the given interface is allowed by the white list.
    bool is_interface_allowed(const asio::ip::address_v4& ip) const;

    //! Checks if the given interface is allowed by the white list.
    virtual bool is_interface_allowed(const Locator_t& loc) const override;

    //! Checks if the interfaces white list is empty.
    virtual bool is_interface_whitelist_empty() const override;

    virtual void set_receive_buffer_size(uint32_t size) override;
    virtual void set_send_buffer_size(uint32_t size) override;

    virtual void endpoint_to_locator(
        const asio::ip::tcp::endpoint& endpoint,
        Locator_t& locator) const override;

public:
    RTPS_DllAPI TCPv4Transport(const TCPv4TransportDescriptor&);

    virtual ~TCPv4Transport() override;

    virtual const TCPTransportDescriptor* configuration() const override;

    virtual TCPTransportDescriptor* configuration() override;

    virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

    virtual bool is_local_locator(const Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &configuration_; }

    virtual void AddDefaultOutputLocator(LocatorList_t&) override;

    virtual uint16_t GetLogicalPortIncrement() const override;

    virtual uint16_t GetLogicalPortRange() const override;

    virtual uint16_t GetMaxLogicalPort() const override;

    virtual bool fillMetatrafficUnicastLocator(
        Locator_t &locator,
        uint32_t metatraffic_unicast_port) const override;

    virtual bool fillUnicastLocator(
        Locator_t &locator,
        uint32_t well_known_port) const override;

    /**
     * Performs the locator selection algorithm for this transport.
     *
     * It basically constist of the following steps
     *   - selector.transport_starts is called
     *   - transport handles the selection state of each locator
     *   - if a locator from an entry is selected, selector.select is called for that entry
     *
     * For TCPv4, only unicast locators on the same WAN as the transport or with the WAN address
     * of a connected channel are selected.
     *
     * @param [in, out] selector Locator selector.
     */
    virtual void select_locators(LocatorSelector& selector) const override;

    virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
