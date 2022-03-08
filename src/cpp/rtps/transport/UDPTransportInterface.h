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

#ifndef _FASTDDS_UDP_TRANSPORT_INTERFACE_H_
#define _FASTDDS_UDP_TRANSPORT_INTERFACE_H_

#include <asio.hpp>
#include <thread>

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/UDPTransportDescriptor.h>
#include <fastrtps/utils/IPFinder.h>

#include <rtps/transport/UDPChannelResource.h>
#include <statistics/rtps/messages/OutputTrafficManager.hpp>

#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

class UDPTransportInterface : public TransportInterface
{
    friend class UDPSenderResource;

public:

    ~UDPTransportInterface() override;

    void clean();

    //! Removes the listening socket for the specified port.
    bool CloseInputChannel(
            const Locator&) override;

    //! Removes all outbound sockets on the given port.
    void CloseOutputChannel(
            eProsimaUDPSocket& socket);

    //! Reports whether Locators correspond to the same port.
    bool DoInputLocatorsMatch(
            const Locator&,
            const Locator&) const override;

    virtual const UDPTransportDescriptor* configuration() const = 0;

    bool init(
            const fastrtps::rtps::PropertyPolicy* properties = nullptr) override;

    //! Checks whether there are open and bound sockets for the given port.
    bool IsInputChannelOpen(
            const Locator&) const override;

    //! Checks for TCP kinds.
    bool IsLocatorSupported(
            const Locator&) const override;

    //! Opens a socket on the given address and port (as long as they are white listed).
    bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const Locator&) override;

    /**
     * Converts a given remote locator (that is, a locator referring to a remote
     * destination) to the main local locator whose channel can write to that
     * destination. In this case it will return a 0.0.0.0 address on that port.
     */
    Locator RemoteToMainLocal(
            const Locator&) const override;

    /**
     * Transforms a remote locator into a locator optimized for local communications.
     *
     * If the remote locator corresponds to one of the local interfaces, it is converted
     * to the corresponding local address.
     *
     * @param [in]  remote_locator Locator to be converted.
     * @param [out] result_locator Converted locator.
     *
     * @return false if the input locator is not supported/allowed by this transport, true otherwise.
     */
    bool transform_remote_locator(
            const Locator& remote_locator,
            Locator& result_locator) const override;

    /**
     * Blocking Send through the specified channel. In both modes, using a localLocator of 0.0.0.0 will
     * send through all whitelisted interfaces provided the channel is open.
     * @param send_buffer Slice into the raw data to send.
     * @param send_buffer_size Size of the raw data. It will be used as a bounds check for the previous argument.
     * It must not exceed the send_buffer_size fed to this class during construction.
     * @param socket channel we're sending from.
     * @param destination_locators_begin pointer to destination locators iterator begin, the iterator can be advanced inside this fuction
     * so should not be reuse.
     * @param destination_locators_end pointer to destination locators iterator end, the iterator can be advanced inside this fuction
     * so should not be reuse.
     * @param only_multicast_purpose multicast network interface
     * @param whitelisted network interface included in the user whitelist
     * @param max_blocking_time_point maximum blocking time.
     */
    virtual bool send(
            const fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            eProsimaUDPSocket& socket,
            fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            fastrtps::rtps::LocatorsIterator* destination_locators_end,
            bool only_multicast_purpose,
            bool whitelisted,
            const std::chrono::steady_clock::time_point& max_blocking_time_point);

    /**
     * Performs the locator selection algorithm for this transport.
     *
     * It basically consists of the following steps
     *   - selector.transport_starts is called
     *   - transport handles the selection state of each locator
     *   - if a locator from an entry is selected, selector.select is called for that entry
     *
     * In the case of UDP, multicast locators are selected when present in more than one entry,
     * otherwise unicast locators are selected.
     *
     * @param [in, out] selector Locator selector.
     */
    void select_locators(
            fastrtps::rtps::LocatorSelector& selector) const override;

    bool fillMetatrafficMulticastLocator(
            Locator& locator,
            uint32_t metatraffic_multicast_port) const override;

    bool fillMetatrafficUnicastLocator(
            Locator& locator,
            uint32_t metatraffic_unicast_port) const override;

    bool configureInitialPeerLocator(
            Locator& locator,
            const fastrtps::rtps::PortParameters& port_params,
            uint32_t domainId,
            LocatorList& list) const override;

    bool fillUnicastLocator(
            Locator& locator,
            uint32_t well_known_port) const override;

    uint32_t max_recv_buffer_size() const override
    {
        return configuration()->maxMessageSize;
    }

protected:

    friend class UDPChannelResource;

    // For UDPv6, the notion of channel corresponds to a port + direction tuple.
    asio::io_service io_service_;
    std::vector<fastrtps::rtps::IPFinder::info_IP> currentInterfaces;

    mutable std::recursive_mutex mInputMapMutex;
    std::map<uint16_t, std::vector<UDPChannelResource*>> mInputSockets;

    uint32_t mSendBufferSize;
    uint32_t mReceiveBufferSize;
    eprosima::fastdds::statistics::rtps::OutputTrafficManager statistics_info_;

    //! First time open output channel flag: open the first socket with the ip::multicast::enable_loopback
    bool first_time_open_output_channel_;

    UDPTransportInterface(
            int32_t transport_kind);

    virtual bool compare_locator_ip(
            const Locator& lh,
            const Locator& rh) const = 0;
    virtual bool compare_locator_ip_and_port(
            const Locator& lh,
            const Locator& rh) const = 0;

    virtual void endpoint_to_locator(
            asio::ip::udp::endpoint& endpoint,
            Locator& locator) = 0;
    virtual void fill_local_ip(
            Locator& loc) const = 0;

    virtual asio::ip::udp::endpoint GenerateAnyAddressEndpoint(
            uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_endpoint(
            uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_endpoint(
            const std::string& sIp,
            uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_endpoint(
            const Locator& loc,
            uint16_t port) = 0;
    virtual asio::ip::udp::endpoint generate_local_endpoint(
            const Locator& loc,
            uint16_t port) = 0;
    virtual asio::ip::udp generate_protocol() const = 0;
    virtual void get_ips(
            std::vector<fastrtps::rtps::IPFinder::info_IP>& locNames,
            bool return_loopback = false) = 0;
    virtual const std::string& localhost_name() = 0;

    //! Checks if the interfaces white list is empty.
    virtual bool is_interface_whitelist_empty() const = 0;

    //! Checks if the given interface is allowed by the white list.
    virtual bool is_interface_allowed(
            const std::string& interface) const = 0;

    /**
     * Method to get a list of interfaces to bind the socket associated to the given locator.
     * @return Vector of interfaces in string format.
     */
    virtual std::vector<std::string> get_binding_interfaces_list() = 0;

    bool OpenAndBindInputSockets(
            const Locator& locator,
            TransportReceiverInterface* receiver,
            bool is_multicast,
            uint32_t maxMsgSize);
    UDPChannelResource* CreateInputChannelResource(
            const std::string& sInterface,
            const Locator& locator,
            bool is_multicast,
            uint32_t maxMsgSize,
            TransportReceiverInterface* receiver);
    virtual eProsimaUDPSocket OpenAndBindInputSocket(
            const std::string& sIp,
            uint16_t port,
            bool is_multicast) = 0;
    eProsimaUDPSocket OpenAndBindUnicastOutputSocket(
            const asio::ip::udp::endpoint& endpoint,
            uint16_t& port);

    virtual void set_receive_buffer_size(
            uint32_t size) = 0;
    virtual void set_send_buffer_size(
            uint32_t size) = 0;
    virtual void SetSocketOutboundInterface(
            eProsimaUDPSocket&,
            const std::string&) = 0;

    /**
     * Send a buffer to a destination
     */
    bool send(
            const fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            eProsimaUDPSocket& socket,
            const Locator& remote_locator,
            bool only_multicast_purpose,
            bool whitelisted,
            const std::chrono::microseconds& timeout);

    /**
     * @brief Return list of not yet open network interfaces
     *
     * @param [in] sender_resource_list List of SenderResources already registered in the transport
     * @param [out] locNames Return the list of new available network interfaces
     * @param [in] return_loopback return the lo network interface
     */
    void get_unknown_network_interfaces(
            const SendResourceList& sender_resource_list,
            std::vector<fastrtps::rtps::IPFinder::info_IP>& locNames,
            bool return_loopback = false);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UDP_TRANSPORT_INTERFACE_H_
