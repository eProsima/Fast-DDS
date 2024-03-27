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

#ifndef _FASTDDS_TCPV4_TRANSPORT_H_
#define _FASTDDS_TCPV4_TRANSPORT_H_

#include <vector>
#include <map>
#include <mutex>

#include <asio.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <rtps/transport/TCPTransportInterface.h>
#include <rtps/transport/tcp/RTCPHeader.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This is a default TCPv4 implementation.
 *    - Opening an output channel by passing a remote locator will try to open a TCP conection with the endpoint.
 *       If there is created a connection with the same endpoint, the transport will use the same one.
 *
 *    - It is possible to provide a white list at construction, which limits the interfaces the transport
 *       will ever be able to interact with. If left empty, all interfaces are allowed.
 *
 *    - Opening an input channel by passing a locator will open a socket listening on the given physical port on every
 *       whitelisted interface, it will wait for incoming connections until the receiver closes the channel.
 *       Several endpoints can connect to other to the same physical port, because the OS creates a connection socket
 *       after each establishment.
 * @ingroup TRANSPORT_MODULE
 */
class TCPv4Transport : public TCPTransportInterface
{
protected:

    TCPv4TransportDescriptor configuration_;

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    TCPv4Transport();

    virtual asio::ip::tcp::endpoint generate_endpoint(
            uint16_t port) const override;

    virtual asio::ip::tcp::endpoint generate_endpoint(
            const Locator& loc,
            uint16_t port) const override;

    virtual asio::ip::tcp::endpoint generate_local_endpoint(
            Locator& loc,
            uint16_t port) const override;

    virtual asio::ip::tcp generate_protocol() const override;

    virtual asio::ip::tcp get_protocol_type() const override
    {
        return asio::ip::tcp::v4();
    }

    virtual void set_receive_buffer_size(
            uint32_t size) override;
    virtual void set_send_buffer_size(
            uint32_t size) override;

    virtual void endpoint_to_locator(
            const asio::ip::tcp::endpoint& endpoint,
            Locator& locator) const override;

public:

    RTPS_DllAPI TCPv4Transport(
            const TCPv4TransportDescriptor&);

    virtual ~TCPv4Transport() override;

    virtual const TCPTransportDescriptor* configuration() const override;

    virtual TCPTransportDescriptor* configuration() override;

    TransportDescriptorInterface* get_configuration() override
    {
        return &configuration_;
    }

    virtual void AddDefaultOutputLocator(
            LocatorList&) override;

    virtual uint16_t GetLogicalPortIncrement() const override;

    virtual uint16_t GetLogicalPortRange() const override;

    virtual uint16_t GetMaxLogicalPort() const override;

    virtual bool fillMetatrafficUnicastLocator(
            Locator& locator,
            uint32_t metatraffic_unicast_port) const override;

    virtual bool fillUnicastLocator(
            Locator& locator,
            uint32_t well_known_port) const override;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TCPV4_TRANSPORT_H_
