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

#ifndef TCPV6_TRANSPORT_H
#define TCPV6_TRANSPORT_H

#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
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
 * This is a default TCPv6 implementation.
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
class TCPv6Transport : public TCPTransportInterface
{
public:
    RTPS_DllAPI TCPv6Transport(const TCPv6TransportDescriptor&);

    virtual ~TCPv6Transport() override;

    virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

    virtual bool is_local_locator(const Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &mConfiguration_; }

    virtual void AddDefaultOutputLocator(LocatorList_t&) override;

    virtual uint16_t GetLogicalPortIncrement() const override;

    virtual uint16_t GetLogicalPortRange() const override;

    virtual uint16_t GetMaxLogicalPort() const override;

protected:

    TCPv6TransportDescriptor mConfiguration_;
    std::vector<asio::ip::address_v6> mInterfaceWhiteList;


    //! Constructor with no descriptor is necessary for implementations derived from this class.
    TCPv6Transport();

    virtual bool CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const override;
    virtual bool CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const override;

    virtual void FillLocalIp(Locator_t& loc) const override;

    virtual const TCPTransportDescriptor* GetConfiguration() const override;
    virtual TCPTransportDescriptor* GetConfiguration() override;

    virtual asio::ip::tcp::endpoint GenerateEndpoint(uint16_t port) const override;
    virtual asio::ip::tcp::endpoint GenerateEndpoint(const Locator_t& loc, uint16_t port) const override;
    virtual asio::ip::tcp::endpoint GenerateLocalEndpoint(Locator_t& loc, uint16_t port) const override;
    virtual asio::ip::tcp GenerateProtocol() const override;

    virtual asio::ip::tcp GetProtocolType() const override { return asio::ip::tcp::v6(); }

    virtual void GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false) const override;

    bool IsLocatorAllowed(const Locator_t& locator) const;

    //! Checks if the given ip has been included in the white list to use it.
    virtual bool IsInterfaceAllowed(const std::string& interface) const override;
    bool IsInterfaceAllowed(const asio::ip::address_v6& ip) const;

    virtual bool IsInterfaceAllowed(const Locator_t& loc) const override;

    virtual void SetReceiveBufferSize(uint32_t size) override;
    virtual void SetSendBufferSize(uint32_t size) override;

    virtual void EndpointToLocator(const asio::ip::tcp::endpoint& endpoint, Locator_t& locator) const override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
