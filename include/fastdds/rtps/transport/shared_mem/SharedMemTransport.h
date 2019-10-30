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
#include <fastdds/rtps/transport/shared_mem/SharedMemChannelResource.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

namespace eprosima{
namespace fastdds{
namespace rtps{

/**
 * Shared memory transport implementation.
 * 
 *    - Opening an output channel by passing a locator will open a socket per interface on the given port.
 *       This collection of sockets constitute the "outbound channel". In other words, a channel corresponds
 *       to a port + a direction.
 * 
 *    - Opening an input channel by passing a locator will open a socket listening on the given port on every
 *       whitelisted interface, and join the multicast channel specified by the locator address. Hence, any locator
 *       that does not correspond to the multicast range will simply open the port without a subsequent join. Joining
 *       multicast groups late is supported by attempting to open the channel again with the same port + a
 *       multicast address (the OpenInputChannel function will fail, however, because no new channel has been
 *       opened in a strict sense).
 * @ingroup TRANSPORT_MODULE
 */
class SharedMemTransport : public TransportInterface
{
public:

    RTPS_DllAPI SharedMemTransport(const SharedMemTransportDescriptor&);
    void clean();
    const SharedMemTransportDescriptor* configuration() const;

    bool init() override;
    
    virtual ~SharedMemTransport() override;

    /**
        * Starts listening on the specified port, and if the specified address is in the
        * multicast range, it joins the specified multicast group,
        */
    virtual bool OpenInputChannel(const fastrtps::rtps::Locator_t&, TransportReceiverInterface*, uint32_t) override;

    //! Removes the listening socket for the specified port.
	virtual bool CloseInputChannel(
			const fastrtps::rtps::Locator_t&) override;

    //! Checks whether there are open and bound sockets for the given port.
	virtual bool IsInputChannelOpen(
			const fastrtps::rtps::Locator_t&) const override;

	//! Reports whether Locators correspond to the same port.
	virtual bool DoInputLocatorsMatch(
			const fastrtps::rtps::Locator_t&, const fastrtps::rtps::Locator_t&) const override;

    //! Checks for TCP kinds.
	virtual bool IsLocatorSupported(
			const fastrtps::rtps::Locator_t&) const override;

	//! Opens a socket on the given address and port (as long as they are white listed).
	virtual bool OpenOutputChannel(
			SendResourceList& sender_resource_list,
			const fastrtps::rtps::Locator_t&) override;

	/**
	* Converts a given remote locator (that is, a locator referring to a remote
	* destination) to the main local locator whose channel can write to that
	* destination. In this case it will return a 0.0.0.0 address on that port.
	*/
	virtual fastrtps::rtps::Locator_t RemoteToMainLocal(
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
	virtual bool transform_remote_locator(
			const fastrtps::rtps::Locator_t& remote_locator,
			fastrtps::rtps::Locator_t& result_locator) const override;

    virtual fastrtps::rtps::LocatorList_t NormalizeLocator(const fastrtps::rtps::Locator_t& locator) override;

    virtual bool is_local_locator(const fastrtps::rtps::Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &configuration_; }

    virtual void AddDefaultOutputLocator(fastrtps::rtps::LocatorList_t &defaultList) override;

    virtual bool getDefaultMetatrafficMulticastLocators(fastrtps::rtps::LocatorList_t &locators,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool getDefaultMetatrafficUnicastLocators(fastrtps::rtps::LocatorList_t &locators,
        uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(fastrtps::rtps::LocatorList_t &locators, uint32_t unicast_port) const override;

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
	virtual bool send(
			const fastrtps::rtps::octet* send_buffer,
			uint32_t send_buffer_size,
			std::shared_ptr<eProsimaSharedMem::Writter> writer,
			const fastrtps::rtps::Locator_t& remote_locator,
			bool only_multicast_purpose,
			const std::chrono::microseconds& timeout);

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
	virtual void select_locators(
			fastrtps::rtps::LocatorSelector& selector) const override;

	virtual bool fillMetatrafficMulticastLocator(
			fastrtps::rtps::Locator_t& locator,
			uint32_t metatraffic_multicast_port) const override;

	virtual bool fillMetatrafficUnicastLocator(
			fastrtps::rtps::Locator_t& locator, 
			uint32_t metatraffic_unicast_port) const override;

	virtual bool configureInitialPeerLocator(fastrtps::rtps::Locator_t& locator, const fastrtps::rtps::PortParameters& port_params, uint32_t domainId,
			fastrtps::rtps::LocatorList_t& list) const override;

	virtual bool fillUnicastLocator(
			fastrtps::rtps::Locator_t& locator, 
			uint32_t well_known_port) const override;

protected:

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    SharedMemTransport();
    SharedMemTransportDescriptor configuration_;

    //! Checks for whether locator is allowed.
    virtual bool is_locator_allowed(const fastrtps::rtps::Locator_t&) const override;

private:

    mutable std::recursive_mutex input_channels_mutex_;
	std::vector<SharedMemChannelResource*> input_channels_;

    friend class SharedMemChannelResource;

	std::shared_ptr<eProsimaSharedMem> shared_mem_;

    SharedMemChannelResource* CreateInputChannelResource(const fastrtps::rtps::Locator_t& locator,
		bool is_multicast, uint32_t maxMsgSize, TransportReceiverInterface* receiver);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_TRANSPORT_H_
