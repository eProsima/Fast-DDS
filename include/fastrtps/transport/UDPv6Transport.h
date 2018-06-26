// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UDPV6_TRANSPORT_H
#define UDPV6_TRANSPORT_H

#include "UDPTransportInterface.h"
#include "UDPv6TransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

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

    RTPS_DllAPI UDPv6Transport(const UDPv6TransportDescriptor&);

    virtual ~UDPv6Transport() override;

    virtual const UDPTransportDescriptor* GetConfiguration() const override;

    /**
        * Starts listening on the specified port, and if the specified address is in the
        * multicast range, it joins the specified multicast group,
        */
    virtual bool OpenInputChannel(const Locator_t&, ReceiverResource*, uint32_t) override;

    virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

    virtual bool is_local_locator(const Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &mConfiguration_; }

    virtual void AddDefaultOutputLocator(LocatorList_t &defaultList) override;

protected:

    //! Constructor with no descriptor is necessary for implementations derived from this class.
	UDPv6Transport();
    UDPv6TransportDescriptor mConfiguration_;

    virtual bool CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const override;
    virtual bool CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const override;

    virtual void EndpointToLocator(asio::ip::udp::endpoint& endpoint, Locator_t& locator) override;
    virtual void FillLocalIp(Locator_t& loc) override;

    virtual asio::ip::udp::endpoint GenerateAnyAddressEndpoint(uint16_t port) override;
    virtual asio::ip::udp::endpoint GenerateEndpoint(uint16_t port) override;
    virtual asio::ip::udp::endpoint GenerateEndpoint(const Locator_t& loc, uint16_t port) override;
    virtual asio::ip::udp::endpoint GenerateLocalEndpoint(const Locator_t& loc, uint16_t port) override;
    virtual asio::ip::udp GenerateProtocol() const override;
    virtual void GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false) override;

    bool IsInterfaceAllowed(const asio::ip::address_v6& ip);
    std::vector<asio::ip::address_v6> mInterfaceWhiteList;

    virtual bool OpenAndBindOutputSockets(const Locator_t& locator, SenderResource*) override;

    virtual void SetReceiveBufferSize(uint32_t size) override;
    virtual void SetSendBufferSize(uint32_t size) override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
