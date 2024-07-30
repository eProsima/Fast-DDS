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

/**
 * @file TransportInterface.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__TRANSPORTINTERFACE_HPP
#define FASTDDS_RTPS_TRANSPORT__TRANSPORTINTERFACE_HPP

#include <memory>
#include <vector>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/LocatorWithMask.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/rtps/transport/network/AllowedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

//! Default maximum message size
constexpr uint32_t s_maximumMessageSize = 65500;
//! Default maximum initial peers range
constexpr uint32_t s_maximumInitialPeersRange = 4;
//! Default IPv4 address
static const std::string s_IPv4AddressAny = "0.0.0.0";
//! Default IPv6 address
static const std::string s_IPv6AddressAny = "::";

using SendResourceList = std::vector<std::unique_ptr<fastdds::rtps::SenderResource>>;
using NetmaskFilterInfo = std::pair<NetmaskFilterKind, std::vector<AllowedNetworkInterface>>;
using TransportNetmaskFilterInfo = std::pair<int32_t, NetmaskFilterInfo>;

/**
 * Interface against which to implement a transport layer, decoupled from Fast DDS internals.
 * TransportInterface expects the user to implement a logical equivalence between Locators and protocol-specific "channels".
 * This equivalence can be narrowing: For instance in UDP/IP, a port would take the role of channel, and several different
 * locators can map to the same port, and hence the same channel.
 * @ingroup TRANSPORT_MODULE
 * */
class FASTDDS_EXPORTED_API TransportInterface
{
public:

    /**
     * Aside from the API defined here, an user-defined Transport must define a descriptor data type and a constructor that
     * expects a constant reference to such descriptor. e.g:
     *
     * class MyTransport:
     * public:
     *    typedef struct { ... } MyTransportDescriptor;
     *    MyTransport(const MyTransportDescriptor&);
     *    ...
     */
    virtual ~TransportInterface() = default;

    //! Copy constructor
    TransportInterface(
            const TransportInterface& t) = delete;

    //! Copy assignment
    TransportInterface& operator =(
            const TransportInterface& t) = delete;

    //! Move constructor
    TransportInterface(
            TransportInterface&& t) = delete;

    //! Move assignment
    TransportInterface& operator =(
            TransportInterface&& t) = delete;

    /**
     * Initialize this transport. This method will prepare all the internals of the transport.
     * @param properties Optional policy to specify additional parameters of the created transport.
     * @param max_msg_size_no_frag Optional maximum message size to avoid 65500 KB fragmentation limit.
     * @return True when the transport was correctly initialized.
     */
    virtual bool init(
            const fastdds::rtps::PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) = 0;

    /**
     * Must report whether the input channel associated to this locator is open. Channels must either be
     * fully closed or fully open, so that "open" and "close" operations are whole and definitive.
     */
    virtual bool IsInputChannelOpen(
            const Locator&) const = 0;

    //! Must report whether the given locator is supported by this transport (typically inspecting its "kind" value).
    virtual bool IsLocatorSupported(
            const Locator&) const = 0;

    //! Must report whether the given locator is allowed by this transport.
    virtual bool is_locator_allowed(
            const Locator&) const = 0;

    /**
     * Must report whether the given locator is reachable by this transport.
     *
     * @param [in] locator @ref Locator for which the reachability is checked.
     *
     * @return true if the input locator is reachable by this transport, false otherwise.
     */
    virtual bool is_locator_reachable(
            const Locator_t& locator) = 0;


    //! Returns the locator describing the main (most general) channel that can write to the provided remote locator.
    virtual Locator RemoteToMainLocal(
            const Locator& remote) const = 0;

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
            const Locator& remote_locator,
            Locator& result_locator) const
    {
        (void)remote_locator;
        (void)result_locator;
        return false;
    }

    //! Must open the channel that maps to/from the given locator. This method must allocate, reserve and mark
    //! any resources that are needed for said channel.
    virtual bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const Locator&) = 0;

    /**
     * Must open the channel that maps to/from the given locator selector entry. This method must allocate,
     * reserve and mark any resources that are needed for said channel.
     *
     * @param sender_resource_list Participant's send resource list.
     * @param locator_selector_entry Locator selector entry with the remote entity locators.
     *
     * @return true if the channel was correctly opened or if finding an already opened one.
     */
    virtual bool OpenOutputChannels(
            SendResourceList& sender_resource_list,
            const fastdds::rtps::LocatorSelectorEntry& locator_selector_entry);

    /**
     * Close the channel that maps to/from the given locator selector entry.
     *
     * @param sender_resource_list Participant's send resource list.
     * @param locator_selector_entry Locator selector entry with the remote entity locators.
     */
    virtual void CloseOutputChannels(
            SendResourceList& sender_resource_list,
            const fastdds::rtps::LocatorSelectorEntry& locator_selector_entry);

    /** Opens an input channel to receive incoming connections.
     *   If there is an existing channel it registers the receiver interface.
     */
    virtual bool OpenInputChannel(
            const Locator&,
            TransportReceiverInterface*,
            uint32_t) = 0;

    /**
     * Must close the channel that maps to/from the given locator.
     * IMPORTANT: It MUST be safe to call this method even during a Receive operation on another thread. You must implement
     * any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage.
     */
    virtual bool CloseInputChannel(
            const Locator&) = 0;

    //! Must report whether two locators map to the same internal channel.
    virtual bool DoInputLocatorsMatch(
            const Locator&,
            const Locator&) const = 0;

    //! Performs locator normalization (assign valid IP if not defined by user)
    virtual LocatorList NormalizeLocator(
            const Locator& locator) = 0;

    /**
     * Performs the locator selection algorithm for this transport.
     *
     * It basically consists of the following steps
     *   - selector.transport_starts is called
     *   - transport handles the selection state of each locator
     *   - if a locator from an entry is selected, selector.select is called for that entry
     *
     * @param [in, out] selector Locator selector.
     */
    virtual void select_locators(
            fastdds::rtps::LocatorSelector& selector) const = 0;

    //! Must report whether the given locator is from the local host
    virtual bool is_local_locator(
            const Locator& locator) const = 0;

    //! Return the transport configuration (Transport Descriptor)
    virtual TransportDescriptorInterface* get_configuration() = 0;

    //! Add default output locator to the locator list
    virtual void AddDefaultOutputLocator(
            LocatorList& defaultList) = 0;

    //! Add metatraffic multicast locator with the given port
    virtual bool getDefaultMetatrafficMulticastLocators(
            LocatorList& locators,
            uint32_t metatraffic_multicast_port) const = 0;

    //! Add metatraffic unicast locator with the given port
    virtual bool getDefaultMetatrafficUnicastLocators(
            LocatorList& locators,
            uint32_t metatraffic_unicast_port) const = 0;

    //! Add unicast locator with the given port
    virtual bool getDefaultUnicastLocators(
            LocatorList& locators,
            uint32_t unicast_port) const = 0;

    //! Assign port to the given metatraffic multicast locator if not already defined
    virtual bool fillMetatrafficMulticastLocator(
            Locator& locator,
            uint32_t metatraffic_multicast_port) const = 0;

    //! Assign port to the given metatraffic unicast locator if not already defined
    virtual bool fillMetatrafficUnicastLocator(
            Locator& locator,
            uint32_t metatraffic_unicast_port) const = 0;

    //! Configure the initial peer locators list
    virtual bool configureInitialPeerLocator(
            Locator& locator,
            const fastdds::rtps::PortParameters& port_params,
            uint32_t domainId,
            LocatorList& list) const = 0;

    //! Assign port to the given unicast locator if not already defined
    virtual bool fillUnicastLocator(
            Locator& locator,
            uint32_t well_known_port) const = 0;

    /**
     * @return The maximum datagram size for reception supported by the transport
     */
    virtual uint32_t max_recv_buffer_size() const = 0;

    /**
     * Shutdown method to close the connections of the transports.
     */
    virtual void shutdown()
    {
    }

    /**
     * @brief Update network interfaces.
     */
    virtual void update_network_interfaces()
    {
    }

    //! Return the transport kind
    int32_t kind() const
    {
        return transport_kind_;
    }

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
    virtual bool transform_remote_locator(
            const Locator& remote_locator,
            Locator& result_locator,
            bool allowed_remote_localhost,
            bool allowed_local_localhost) const
    {
        static_cast<void>(allowed_remote_localhost);
        static_cast<void>(allowed_local_localhost);
        return transform_remote_locator(remote_locator, result_locator);
    }

    //! Must report whether localhost locator is allowed
    virtual bool is_localhost_allowed() const
    {
        return true;
    }

    //! Returns netmask filter information (transport's netmask filter kind and allowlist)
    virtual NetmaskFilterInfo netmask_filter_info() const
    {
        return {NetmaskFilterKind::AUTO, {}};
    }

protected:

    TransportInterface(
            int32_t transport_kind)
        : transport_kind_(transport_kind)
    {
    }

    int32_t transport_kind_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__TRANSPORTINTERFACE_HPP
