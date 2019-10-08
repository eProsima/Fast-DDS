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

#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <memory>
#include <vector>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/common/PortParameters.h>
#include <fastrtps/transport/TransportDescriptorInterface.h>
#include <fastrtps/transport/TransportReceiverInterface.h>
#include <fastdds/rtps/network/SenderResource.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const uint32_t s_maximumMessageSize = 65500;
static const uint32_t s_maximumInitialPeersRange = 4;
static const uint32_t s_minimumSocketBuffer = 65536;
static const std::string s_IPv4AddressAny = "0.0.0.0";
static const std::string s_IPv6AddressAny = "::";

class ChannelResource;

using SendResourceList = std::vector<std::unique_ptr<SenderResource>>;

/**
 * Interface against which to implement a transport layer, decoupled from FastRTPS internals.
 * TransportInterface expects the user to implement a logical equivalence between Locators and protocol-specific "channels".
 * This equivalence can be narrowing: For instance in UDP/IP, a port would take the role of channel, and several different
 * locators can map to the same port, and hence the same channel.
 * @ingroup TRANSPORT_MODULE
 * */
class TransportInterface
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

    /**
    * Initialize this transport. This method will prepare all the internals of the transport.
    * @return True when the transport was correctly intialized.
    */
    virtual bool init() = 0;

    /**
    * Must report whether the input channel associated to this locator is open. Channels must either be
    * fully closed or fully open, so that "open" and "close" operations are whole and definitive.
    */
    virtual bool IsInputChannelOpen(const Locator_t&) const = 0;

    //! Must report whether the given locator is supported by this transport (typically inspecting its "kind" value).
    virtual bool IsLocatorSupported(const Locator_t&) const = 0;

    //! Must report whether the given locator is allowed by this transport.
    virtual bool is_locator_allowed(const Locator_t&) const = 0;

    //! Returns the locator describing the main (most general) channel that can write to the provided remote locator.
    virtual Locator_t RemoteToMainLocal(const Locator_t& remote) const = 0;

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
            const Locator_t& remote_locator,
            Locator_t& result_locator) const
    {
        (void)remote_locator;
        (void)result_locator;
        return false;
    }

    //! Must open the channel that maps to/from the given locator. This method must allocate, reserve and mark
    //! any resources that are needed for said channel.
    virtual bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const Locator_t&) = 0;

    virtual bool OpenInputChannel(
        const Locator_t&,
        TransportReceiverInterface*, uint32_t) = 0;

    /**
    * Must close the channel that maps to/from the given locator.
    * IMPORTANT: It MUST be safe to call this method even during a Receive operation on another thread. You must implement
    * any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage.
    */
    virtual bool CloseInputChannel(const Locator_t&) = 0;

    //! Must report whether two locators map to the same internal channel.
    virtual bool DoInputLocatorsMatch(const Locator_t&, const Locator_t&) const = 0;

    virtual LocatorList_t NormalizeLocator(const Locator_t& locator) = 0;

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
    virtual void select_locators(LocatorSelector& selector) const = 0;

    virtual bool is_local_locator(const Locator_t& locator) const = 0;

    virtual TransportDescriptorInterface* get_configuration() = 0;

    virtual void AddDefaultOutputLocator(LocatorList_t &defaultList) = 0;

    virtual bool getDefaultMetatrafficMulticastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_multicast_port) const = 0;

    virtual bool getDefaultMetatrafficUnicastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_unicast_port) const = 0;

    virtual bool getDefaultUnicastLocators(
        LocatorList_t& locators,
        uint32_t unicast_port) const = 0;

    virtual bool fillMetatrafficMulticastLocator(
        Locator_t& locator,
        uint32_t metatraffic_multicast_port) const = 0;

    virtual bool fillMetatrafficUnicastLocator(
        Locator_t& locator,
        uint32_t metatraffic_unicast_port) const = 0;

    virtual bool configureInitialPeerLocator(
        Locator_t& locator,
        const PortParameters &port_params,
        uint32_t domainId,
        LocatorList_t& list) const = 0;

    virtual bool fillUnicastLocator(
        Locator_t& locator,
        uint32_t well_known_port) const = 0;

    /**
     * Shutdown method to close the connections of the transports.
    */
    virtual void shutdown() {};

    int32_t kind() const { return transport_kind_; }

protected:

    TransportInterface(int32_t transport_kind)
        : transport_kind_(transport_kind) {}

    int32_t transport_kind_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
