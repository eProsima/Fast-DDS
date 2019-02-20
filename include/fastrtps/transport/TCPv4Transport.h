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
public:
    RTPS_DllAPI TCPv4Transport(const TCPv4TransportDescriptor&);

    virtual ~TCPv4Transport() override;

    virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

    virtual bool is_local_locator(const Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &mConfiguration_; }

    virtual void AddDefaultOutputLocator(LocatorList_t&) override;

    virtual uint16_t GetLogicalPortIncrement() const override;

    virtual uint16_t GetLogicalPortRange() const override;

    virtual uint16_t GetMaxLogicalPort() const override;

    virtual bool fillMetatrafficUnicastLocator(Locator_t &locator, uint32_t metatraffic_unicast_port) const override;

    virtual bool fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const override;

    virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

protected:

    TCPv4TransportDescriptor mConfiguration_;
    std::vector<asio::ip::address_v4> mInterfaceWhiteList;

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    TCPv4Transport();

    virtual bool CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const override;
    virtual bool CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const override;

    virtual void FillLocalIp(Locator_t& loc) const override;

    virtual const TCPTransportDescriptor* GetConfiguration() const override;
    virtual TCPTransportDescriptor* GetConfiguration() override;

    virtual asio::ip::tcp::endpoint GenerateEndpoint(uint16_t port) const override;
    virtual asio::ip::tcp::endpoint GenerateEndpoint(const Locator_t& loc, uint16_t port) const override;
    virtual asio::ip::tcp::endpoint GenerateLocalEndpoint(Locator_t& loc, uint16_t port) const override;
    virtual asio::ip::tcp GenerateProtocol() const override;

    virtual asio::ip::tcp GetProtocolType() const override { return asio::ip::tcp::v4(); }

    virtual void GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false) const override;

    /**
    * Method to get a list of interfaces to bind the socket associated to the given locator.
    * @param locator Input locator.
    * @return Vector of interfaces in string format.
    */
    virtual std::vector<std::string> GetBindingInterfacesList() override;

    bool IsLocatorAllowed(const Locator_t& locator) const override;

    //! Checks if the given ip has been included in the white list to use it.
    virtual bool IsInterfaceAllowed(const std::string& interface) const override;

    //! Checks if the given interface is allowed by the white list.
    bool IsInterfaceAllowed(const asio::ip::address_v4& ip) const;

    //! Checks if the interfaces white list is empty.
    virtual bool IsInterfaceWhiteListEmpty() const override;

    //! Checks if the given interface is allowed by the white list.
    virtual bool IsInterfaceAllowed(const Locator_t& loc) const override;

    virtual void SetReceiveBufferSize(uint32_t size) override;
    virtual void SetSendBufferSize(uint32_t size) override;

    virtual void EndpointToLocator(const asio::ip::tcp::endpoint& endpoint, Locator_t& locator) const override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
