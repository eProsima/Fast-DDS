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

#ifndef TCP_TRANSPORT_INTERFACE_H
#define TCP_TRANSPORT_INTERFACE_H

#include <fastrtps/transport/TransportInterface.h>
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

class RTCPMessageManager;
class CleanTCPSocketsEvent;
class TCPChannelResource;

/**
 * This is a default TCP Interface implementation.
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
class TCPTransportInterface : public TransportInterface
{
public:
    friend class RTCPMessageManager;
    friend class CleanTCPSocketsEvent;

    virtual ~TCPTransportInterface() override;
    //! Stores the binding between the given locator and the given TCP socket.

    virtual void BindSocket(const Locator_t&, TCPChannelResource*) = 0;

    //! Cleans the sockets pending to delete.
    virtual void CleanDeletedSockets() = 0;

    virtual uint16_t GetLogicalPortIncrement() const  = 0;

    virtual uint16_t GetLogicalPortRange() const = 0;

    virtual uint16_t GetMaxLogicalPort() const = 0;

    virtual size_t Send(TCPChannelResource* pChannelResource, const octet* data, size_t size) const  = 0;
protected:
    TCPTransportInterface();

};

/**
* Transport configuration
* @ingroup TRANSPORT_MODULE
*/
typedef struct TCPTransportDescriptor : public TransportDescriptorInterface {
    virtual ~TCPTransportDescriptor() {}

    std::vector<uint16_t> listening_ports;
    uint32_t keep_alive_frequency_ms;
    uint32_t keep_alive_timeout_ms;
    uint16_t max_logical_port;
    uint16_t logical_port_range;
    uint16_t logical_port_increment;
    uint16_t metadata_logical_port;

    void add_listener_port(uint16_t port)
    {
        listening_ports.push_back(port);
    }

    void set_metadata_logical_port(uint16_t port)
    {
        metadata_logical_port = port;
    }

    RTPS_DllAPI TCPTransportDescriptor();

    RTPS_DllAPI TCPTransportDescriptor(const TCPTransportDescriptor& t);
} TCPTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // TCP_TRANSPORT_INTERFACE_H
