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

#ifndef UDP_TRANSPORT_INTERFACE_H
#define UDP_TRANSPORT_INTERFACE_H

#include <asio.hpp>
#include <thread>

#include "TransportInterface.h"
#include "UDPChannelResource.h"
#include "UDPTransportDescriptor.h"
#include "../utils/IPFinder.h"

#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class UDPTransportInterface : public TransportInterface
{
public:

   virtual ~UDPTransportInterface() override;

   void clean();

   //! Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(const Locator_t&) override;

   //! Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(const Locator_t&) override;

   //! Reports whether Locators correspond to the same port.
   virtual bool DoInputLocatorsMatch(const Locator_t&, const Locator_t&) const override;
   virtual bool DoOutputLocatorsMatch(const Locator_t&, const Locator_t&) const override;

   virtual const UDPTransportDescriptor* configuration() const = 0;

   bool init() override;

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(const Locator_t&) const override;

   //! Checks for TCP kinds.
   virtual bool IsLocatorSupported(const Locator_t&) const override;

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsOutputChannelOpen(const Locator_t&) const override;

   //! Opens a socket on the given address and port (as long as they are white listed).
   virtual bool OpenOutputChannel(const Locator_t&) override;
   virtual bool OpenExtraOutputChannel(const Locator_t&) override;

   /**
   * Blocking Receive from the specified channel.
   * @param p_channel_resource Pointer to the channer resource that stores the socket.
   * @param receive_buffer vector with enough capacity (not size) to accomodate a full receive buffer. That
   * capacity must not be less than the receive_buffer_size supplied to this class during construction.
   * @param receive_buffer_capacity Maximum size of the receive_buffer.
   * @param[out] receive_buffer_size Size of the received buffer.
   * @param[out] remote_locator Locator describing the remote restination we received a packet from.
   */
   bool Receive(UDPChannelResource* p_channel_resource, octet* receive_buffer,
       uint32_t receive_buffer_capacity, uint32_t& receive_buffer_size, Locator_t& remote_locator);

   //! Release the listening socket for the specified port.
   bool ReleaseInputChannel(const Locator_t& locator, const asio::ip::address& interface_address);

   /**
   * Converts a given remote locator (that is, a locator referring to a remote
   * destination) to the main local locator whose channel can write to that
   * destination. In this case it will return a 0.0.0.0 address on that port.
   */
   virtual Locator_t RemoteToMainLocal(const Locator_t&) const override;

   /**
   * Blocking Send through the specified channel. In both modes, using a localLocator of 0.0.0.0 will
   * send through all whitelisted interfaces provided the channel is open.
   * @param send_buffer Slice into the raw data to send.
   * @param send_buffer_size Size of the raw data. It will be used as a bounds check for the previous argument.
   * It must not exceed the send_buffer_size fed to this class during construction.
   * @param localLocator Locator mapping to the channel we're sending from.
   * @param remote_locator Locator describing the remote destination we're sending to.
   */
   virtual bool send(const octet* send_buffer, uint32_t send_buffer_size, const Locator_t& localLocator,
       const Locator_t& remote_locator) override;

   virtual bool send(const octet* send_buffer, uint32_t send_buffer_size, const Locator_t& localLocator,
       const Locator_t& remote_locator, ChannelResource* p_channel_resource) override;

    /**
     * Performs the locator selection algorithm for this transport.
     *
     * It basically constist of the following steps
     *   - selector.transport_starts is called
     *   - transport handles the selection state of each locator
     *   - if a locator from an entry is selected, selector.select is called for that entry
     *
     * In the case of UDP, multicast locators are selected when present in more than one entry,
     * otherwise unicast locators are selected.
     * 
     * @param [in, out] selector Locator selector.
     */
   virtual void select_locators(LocatorSelector& selector) const override;

   virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

    virtual bool fillMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool fillMetatrafficUnicastLocator(Locator_t &locator, uint32_t metatraffic_unicast_port) const override;

    virtual bool configureInitialPeerLocator(Locator_t &locator, const PortParameters &port_params, uint32_t domainId,
        LocatorList_t& list) const override;

    virtual bool fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const override;

protected:

    int32_t transport_kind_;

    // For UDPv6, the notion of channel corresponds to a port + direction tuple.
    asio::io_service io_service_;
    std::vector<IPFinder::info_IP> currentInterfaces;

    mutable std::recursive_mutex mOutputMapMutex;
    mutable std::recursive_mutex mInputMapMutex;
    std::map<uint16_t, std::vector<UDPChannelResource*>> mInputSockets;
    std::vector<UDPChannelResource*> mOutputSockets;

    uint32_t mSendBufferSize;
    uint32_t mReceiveBufferSize;

    UDPTransportInterface();

    virtual bool compare_locator_ip(const Locator_t& lh, const Locator_t& rh) const = 0;
    virtual bool compare_locator_ip_and_port(const Locator_t& lh, const Locator_t& rh) const = 0;

    virtual void endpoint_to_locator(asio::ip::udp::endpoint& endpoint, Locator_t& locator) = 0;
    virtual void fill_local_ip(Locator_t& loc) = 0;

    virtual asio::ip::udp::endpoint GenerateAnyAddressEndpoint(uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_endpoint(uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_endpoint(const std::string& sIp, uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_endpoint(const Locator_t& loc, uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_local_endpoint(const Locator_t& loc, uint16_t port) = 0;
    virtual asio::ip::udp generate_protocol() const = 0;
    virtual void get_ips(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false) = 0;

    //! Checks if the interfaces white list is empty.
    virtual bool is_interface_whitelist_empty() const = 0;

    //! Checks if the given interface is allowed by the white list.
    virtual bool is_interface_allowed(const std::string& interface) const = 0;

    /**
    * Method to get a list of interfaces to bind the socket associated to the given locator.
    * @return Vector of interfaces in string format.
    */
    virtual std::vector<std::string> get_binding_interfaces_list() = 0;

    bool OpenAndBindInputSockets(const Locator_t& locator, TransportReceiverInterface* receiver, bool is_multicast,
        uint32_t maxMsgSize);
    UDPChannelResource* CreateInputChannelResource(const std::string& sInterface, const Locator_t& locator,
        bool is_multicast, uint32_t maxMsgSize, TransportReceiverInterface* receiver);
    virtual eProsimaUDPSocket OpenAndBindInputSocket(const std::string& sIp, uint16_t port, bool is_multicast) = 0;
    bool OpenAndBindOutputSockets(const Locator_t& locator);
    eProsimaUDPSocket OpenAndBindUnicastOutputSocket(const asio::ip::udp::endpoint& endpoint, uint16_t& port);
    /** Function to be called from a new thread, which takes cares of performing a blocking receive
    operation on the ReceiveResource
    @param input_locator - Locator that triggered the creation of the resource
    */
    void perform_listen_operation(UDPChannelResource* p_channel_resource, Locator_t input_locator);

    bool send_through_socket(const octet* send_buffer, uint32_t send_buffer_size, const Locator_t& remote_locator,
        eProsimaUDPSocketRef socket);

    virtual void set_receive_buffer_size(uint32_t size) = 0;
    virtual void set_send_buffer_size(uint32_t size) = 0;
    virtual void SetSocketOutboundInterface(eProsimaUDPSocket&, const std::string&) = 0;

    /*
        struct LocatorCompare {
        bool operator()(const Locator_t& lhs, const Locator_t& rhs) const
        {
            return (memcmp(&lhs, &rhs, sizeof(Locator_t)) < 0);
        }
    };
    */
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // UDP_TRANSPORT_INTERFACE_H
