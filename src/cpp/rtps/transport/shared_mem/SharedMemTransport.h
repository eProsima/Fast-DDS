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

#ifndef _FASTDDS_SHAREDMEM_TRANSPORT_H_
#define _FASTDDS_SHAREDMEM_TRANSPORT_H_

#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

#include <rtps/transport/shared_mem/SharedMemManager.hpp>
#include <rtps/transport/shared_mem/SharedMemLog.hpp>

#include <map>
#include <asio.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class SharedMemChannelResource;

/**
 * Shared memory transport implementation.
 *
 *    - Opening an output channel by passing a locator will prepare the output channel.
 *      The opening of the shared memory port will be done when a send operation is issued, based on
 *      the port indicated in the destination locator.
 *
 *    - Opening an input channel by passing a locator will open a shared memory port and a listener attached
 *      to the port.
 *
 * @ingroup TRANSPORT_MODULE
 */
class SharedMemTransport : public TransportInterface
{
public:

    FASTDDS_EXPORTED_API SharedMemTransport(
            const SharedMemTransportDescriptor&);

    const SharedMemTransportDescriptor* configuration() const;

    bool init(
            const PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) override;

    ~SharedMemTransport() override;

    /**
     * Starts listening on the specified port, and if the specified address is in the
     * multicast range, it joins the specified multicast group,
     */
    bool OpenInputChannel(
            const Locator&,
            TransportReceiverInterface*,
            uint32_t) override;

    //! Removes the listening socket for the specified port.
    bool CloseInputChannel(
            const Locator&) override;

    //! Checks whether there are open and bound sockets for the given port.
    bool IsInputChannelOpen(
            const Locator&) const override;

    //! Reports whether Locators correspond to the same port.
    bool DoInputLocatorsMatch(
            const Locator&,
            const Locator&) const override;

    //! Checks for TCP kinds.
    bool IsLocatorSupported(
            const Locator&) const override;

    //! Opens a socket on the given address and port (as long as they are white listed).
    bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const Locator&) override;

    /**
     * Opens a socket on the locators provided by the given locator_selector_entry.
     *
     * @param sender_resource_list Participant's send resource list.
     * @param locator_selector_entry Locator selector entry with the remote entity locators.
     *
     * @return true if the socket was correctly opened or if finding an already opened one.
     */
    bool OpenOutputChannels(
            SendResourceList& sender_resource_list,
            const LocatorSelectorEntry& locator_selector_entry) override;

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
     * to the corresponding local address if allowed by both local and remote transports.
     *
     * @param [in]  remote_locator Locator to be converted.
     * @param [out] result_locator Converted locator.
     * @param [in]  allowed_remote_localhost Whether localhost is allowed (and hence used) in the remote transport.
     * @param [in]  allowed_local_localhost Whether localhost is allowed locally (by this or other transport).
     *
     * @return false if the input locator is not supported/allowed by this transport, true otherwise.
     */
    bool transform_remote_locator(
            const Locator& remote_locator,
            Locator& result_locator,
            bool allowed_remote_localhost,
            bool allowed_local_localhost) const override;

    LocatorList NormalizeLocator(
            const Locator& locator) override;

    bool is_local_locator(
            const Locator& locator) const override;

    bool is_localhost_allowed() const override;

    TransportDescriptorInterface* get_configuration() override
    {
        return &configuration_;
    }

    void AddDefaultOutputLocator(
            LocatorList& defaultList) override;

    bool getDefaultMetatrafficMulticastLocators(
            LocatorList& locators,
            uint32_t metatraffic_multicast_port) const override;

    bool getDefaultMetatrafficUnicastLocators(
            LocatorList& locators,
            uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
            LocatorList& locators,
            uint32_t unicast_port) const override;

    /**
     * Blocking Send through the specified channel. In both modes, using a localLocator of 0.0.0.0 will
     * send through all whitelisted interfaces provided the channel is open.
     * @param buffers Vector of buffers to send.
     * @param send_buffer_size Total amount of bytes to send. It will be used as a bounds check for the previous
     * argument. It must not exceed the send_buffer_size fed to this class during construction.
     * @param socket channel we're sending from.
     * @param remote_locator Locator describing the remote destination we're sending to.
     * @param only_multicast_purpose
     * @param timeout Maximum time this function will block
     */
    virtual bool send(
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes,
            LocatorsIterator* destination_locators_begin,
            LocatorsIterator* destination_locators_end,
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
            LocatorSelector& selector) const override;

    bool fillMetatrafficMulticastLocator(
            Locator& locator,
            uint32_t metatraffic_multicast_port) const override;

    bool fillMetatrafficUnicastLocator(
            Locator& locator,
            uint32_t metatraffic_unicast_port) const override;

    bool configureInitialPeerLocator(
            Locator& locator,
            const PortParameters& port_params,
            uint32_t domainId,
            LocatorList& list) const override;

    bool fillUnicastLocator(
            Locator& locator,
            uint32_t well_known_port) const override;

    uint32_t max_recv_buffer_size() const override
    {
        return (std::numeric_limits<uint32_t>::max)();
    }

private:

    using TransportInterface::transform_remote_locator;

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    SharedMemTransport();

    SharedMemTransportDescriptor configuration_;

    //! Checks for whether locator is allowed.
    bool is_locator_allowed(
            const Locator&) const override;

    //! Checks for whether locator is reachable.
    bool is_locator_reachable(
            const Locator_t&) override;

protected:

    std::shared_ptr<SharedMemManager> shared_mem_manager_;

private:

    void clean_up();

    mutable std::mutex opened_ports_mutex_;

    std::map<uint32_t, std::shared_ptr<SharedMemManager::Port>> opened_ports_;

    mutable std::recursive_mutex input_channels_mutex_;

    std::vector<SharedMemChannelResource*> input_channels_;

    std::shared_ptr<SharedMemManager::Segment> shared_mem_segment_;

    std::shared_ptr<PacketsLog<SHMPacketFileConsumer>> packet_logger_;

    friend class SharedMemChannelResource;

protected:

    /**
     * Creates an input channel
     * @param locator Listening locator
     * @param max_msg_size Maximum message size supported by the channel
     * @throw std::exception& If the channel cannot be created
     */
    virtual SharedMemChannelResource* CreateInputChannelResource(
            const Locator& locator,
            uint32_t max_msg_size,
            TransportReceiverInterface* receiver);

private:

    /**
     * Copies a Vector of buffers into the shared_buffer.
     * @param buffers Vector of buffers to copy.
     * @param total_bytes Total amount of bytes of the whole list of buffers.
     * @param max_blocking_time_point Maximum time this function will block.
     */
    std::shared_ptr<SharedMemManager::Buffer> copy_to_shared_buffer(
            const std::vector<NetworkBuffer>& buffers,
            const uint32_t total_bytes,
            const std::chrono::steady_clock::time_point& max_blocking_time_point);

    bool send(
            const std::shared_ptr<SharedMemManager::Buffer>& buffer,
            const Locator& remote_locator);

    void cleanup_output_ports();

    std::shared_ptr<SharedMemManager::Port> find_port(
            uint32_t port_id);

    bool push_discard(
            const std::shared_ptr<SharedMemManager::Buffer>& buffer,
            const Locator& remote_locator);

    void delete_input_channel(
            SharedMemChannelResource* channel);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_TRANSPORT_H_
