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

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

#include <rtps/transport/shared_mem/SharedMemManager.hpp>

namespace eprosima{
namespace fastdds{
namespace rtps{

class SharedMemChannelResource;

/**
 * Shared memory transport implementation.
 * 
 *    - Opening an output channel by passing a locator will prepare the output channel.
 *     	The opening of the shared memory port will be done when a send operation is issued, based on
 * 		the port indicated in the destination locator. 
 * 
 *    - Opening an input channel by passing a locator will open a shared memory port and a listener attached 
 * 		to the port.
 * 
 * @ingroup TRANSPORT_MODULE
 */
class SharedMemTransport : public TransportInterface
{
public:

	static const uint32_t maximum_message_size = (std::numeric_limits<uint32_t>::max)();
	static const uint32_t default_segment_size = s_maximumMessageSize;
	static const uint32_t default_port_queue_capacity = 512;
	static const SharedMemTransportDescriptor::OverflowPolicy default_overflow_policy =
        SharedMemTransportDescriptor::OverflowPolicy::DISCARD;
	static const uint32_t default_healthy_check_timeout_ms = 1000;

    RTPS_DllAPI SharedMemTransport(const SharedMemTransportDescriptor&);
    void clean();
    const SharedMemTransportDescriptor* configuration() const;

    bool init() override;
    
    virtual ~SharedMemTransport() override;

    /**
	* Starts listening on the specified port, and if the specified address is in the
	* multicast range, it joins the specified multicast group,
	*/
    bool OpenInputChannel(
			const fastrtps::rtps::Locator_t&, 
			TransportReceiverInterface*, uint32_t) override;

    //! Removes the listening socket for the specified port.
	bool CloseInputChannel(
			const fastrtps::rtps::Locator_t&) override;

    //! Checks whether there are open and bound sockets for the given port.
	bool IsInputChannelOpen(
			const fastrtps::rtps::Locator_t&) const override;

	//! Reports whether Locators correspond to the same port.
	bool DoInputLocatorsMatch(
			const fastrtps::rtps::Locator_t&, const fastrtps::rtps::Locator_t&) const override;

    //! Checks for TCP kinds.
	bool IsLocatorSupported(
			const fastrtps::rtps::Locator_t&) const override;

	//! Opens a socket on the given address and port (as long as they are white listed).
	bool OpenOutputChannel(
			SendResourceList& sender_resource_list,
			const fastrtps::rtps::Locator_t&) override;

	/**
	* Converts a given remote locator (that is, a locator referring to a remote
	* destination) to the main local locator whose channel can write to that
	* destination. In this case it will return a 0.0.0.0 address on that port.
	*/
	fastrtps::rtps::Locator_t RemoteToMainLocal(
			const fastrtps::rtps::Locator_t&) const override;

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
			const fastrtps::rtps::Locator_t& remote_locator,
			fastrtps::rtps::Locator_t& result_locator) const override;

    fastrtps::rtps::LocatorList_t NormalizeLocator(
			const fastrtps::rtps::Locator_t& locator) override;

    bool is_local_locator(
			const fastrtps::rtps::Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &configuration_; }

    void AddDefaultOutputLocator(
			fastrtps::rtps::LocatorList_t &defaultList) override;

    bool getDefaultMetatrafficMulticastLocators(
			fastrtps::rtps::LocatorList_t &locators,
        	uint32_t metatraffic_multicast_port) const override;

    bool getDefaultMetatrafficUnicastLocators(
			fastrtps::rtps::LocatorList_t &locators,
        	uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
			fastrtps::rtps::LocatorList_t &locators, 
			uint32_t unicast_port) const override;

    /**
	* Blocking Send through the specified channel. In both modes, using a localLocator of 0.0.0.0 will
	* send through all whitelisted interfaces provided the channel is open.
	* @param send_buffer Slice into the raw data to send.
	* @param send_buffer_size Size of the raw data. It will be used as a bounds check for the previous argument.
	* It must not exceed the send_buffer_size fed to this class during construction.
	* @param socket channel we're sending from.
	* @param remote_locator Locator describing the remote destination we're sending to.
	* @param only_multicast_purpose
	* @param timeout Maximum time this function will block
	*/
	bool send(
			const fastrtps::rtps::octet* send_buffer,
			uint32_t send_buffer_size,
			fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            fastrtps::rtps::LocatorsIterator* destination_locators_end,
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
			fastrtps::rtps::Locator_t& locator,
			uint32_t metatraffic_multicast_port) const override;

	bool fillMetatrafficUnicastLocator(
			fastrtps::rtps::Locator_t& locator, 
			uint32_t metatraffic_unicast_port) const override;

	bool configureInitialPeerLocator(
			fastrtps::rtps::Locator_t& locator,
			const fastrtps::rtps::PortParameters& port_params,
			uint32_t domainId,
			fastrtps::rtps::LocatorList_t& list) const override;

	bool fillUnicastLocator(
			fastrtps::rtps::Locator_t& locator, 
			uint32_t well_known_port) const override;

	uint32_t max_recv_buffer_size() const override 
	{ 
		return (std::numeric_limits<uint32_t>::max)();
	}

private:

	//! Constructor with no descriptor is necessary for implementations derived from this class.
    SharedMemTransport();
	
    SharedMemTransportDescriptor configuration_;

	std::map<uint32_t, std::shared_ptr<SharedMemManager::Port>> opened_ports_;

    //! Checks for whether locator is allowed.
    bool is_locator_allowed(const fastrtps::rtps::Locator_t&) const override;

    mutable std::recursive_mutex input_channels_mutex_;
	std::vector<SharedMemChannelResource*> input_channels_;

	std::shared_ptr<SharedMemManager> shared_mem_manager_;
	std::shared_ptr<SharedMemManager::Segment> shared_mem_segment_;

	friend class SharedMemChannelResource;

	/**
	 * Creates an input channel
	 * @param locator Listening locator
	 * @param max_msg_size Maximum message size supported by the channel
	 * @throw std::exception& If the channel cannot be created
	 */
    SharedMemChannelResource* CreateInputChannelResource(
        const fastrtps::rtps::Locator_t& locator,
        uint32_t max_msg_size,
        TransportReceiverInterface* receiver);

	std::shared_ptr<SharedMemManager::Buffer> copy_to_shared_buffer(
        const fastrtps::rtps::octet* send_buffer,
        uint32_t send_buffer_size);

	bool send(
		const std::shared_ptr<SharedMemManager::Buffer>& buffer,
		const fastrtps::rtps::Locator_t& remote_locator,
		const std::chrono::microseconds& timeout);

	std::shared_ptr<SharedMemManager::Port> find_port(
        uint32_t port_id);

	bool push_fail(
        const std::shared_ptr<SharedMemManager::Buffer>& buffer,
		const fastrtps::rtps::Locator_t& remote_locator,
		const std::chrono::microseconds& timeout);
	
	bool push_discard(
        const std::shared_ptr<SharedMemManager::Buffer>& buffer,
		const fastrtps::rtps::Locator_t& remote_locator,
		const std::chrono::microseconds& timeout);

	std::function<bool(
			SharedMemTransport&,
            const std::shared_ptr<SharedMemManager::Buffer>& buffer,
			const fastrtps::rtps::Locator_t& remote_locator,
			const std::chrono::microseconds& timeout)> push_lambda_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_TRANSPORT_H_
