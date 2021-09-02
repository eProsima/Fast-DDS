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

#ifndef CHAINING_TRANSPORT_H
#define CHAINING_TRANSPORT_H

#include "TransportInterface.h"
#include "ChainingTransportDescriptor.h"

#include <map>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ChainingReceiverResource;

/**
 * This is the base class for chaining adapter transports.
 *    - Directly proxies all operations except Send and Receive
 *
 *    - Has a pointer to the low level transport
 * @ingroup TRANSPORT_MODULE
 */
class ChainingTransport : public TransportInterface
{

public:

    RTPS_DllAPI
    ChainingTransport(
            const ChainingTransportDescriptor& t)
        : TransportInterface(0)
        , low_level_transport_(t.low_level_descriptor->create_transport())
    {
        transport_kind_ = low_level_transport_->kind();
    }

    RTPS_DllAPI
    virtual ~ChainingTransport()
    {
    }

    RTPS_DllAPI
    bool init(
            const fastrtps::rtps::PropertyPolicy* properties = nullptr) override
    {
        return low_level_transport_->init(properties);
    }

    //! Checks whether there are open and bound sockets for the given port.
    RTPS_DllAPI
    bool IsInputChannelOpen(
            const fastrtps::rtps::Locator_t& loc) const override
    {
        return low_level_transport_->IsInputChannelOpen(loc);
    }

    //! Checks for low level support.
    RTPS_DllAPI
    bool IsLocatorSupported(
            const fastrtps::rtps::Locator_t& loc) const override
    {
        return low_level_transport_->IsLocatorSupported(loc);
    }

    /**
     * Converts a given remote locator (that is, a locator referring to a remote
     * destination) to the main local locator whose channel can write to that
     * destination.
     */
    RTPS_DllAPI
    fastrtps::rtps::Locator_t RemoteToMainLocal(
            const fastrtps::rtps::Locator_t& loc) const override
    {
        return low_level_transport_->RemoteToMainLocal(loc);
    }

    /**
     * Starts listening on the specified locator.
     */
    RTPS_DllAPI
    bool OpenInputChannel(
            const fastrtps::rtps::Locator_t& loc,
            TransportReceiverInterface* receiver_interface,
            uint32_t max_message_size) override;

    /**
     * Opens a socket on the given address and port (as long as they are white listed).
     */
    RTPS_DllAPI
    bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const fastrtps::rtps::Locator_t& loc) override;

    //! Removes the listening socket for the specified port.
    RTPS_DllAPI
    bool CloseInputChannel(
            const fastrtps::rtps::Locator_t& loc) override
    {
        return low_level_transport_->CloseInputChannel(loc);
    }

    RTPS_DllAPI
    fastrtps::rtps::LocatorList_t NormalizeLocator(
            const fastrtps::rtps::Locator_t& locator) override
    {
        return low_level_transport_->NormalizeLocator(locator);
    }

    RTPS_DllAPI
    bool is_local_locator(
            const fastrtps::rtps::Locator_t& locator) const override
    {
        return low_level_transport_->is_local_locator(locator);
    }

    RTPS_DllAPI
    bool is_locator_allowed(
            const fastrtps::rtps::Locator_t& locator) const override
    {
        return low_level_transport_->is_locator_allowed(locator);
    }

    RTPS_DllAPI
    bool DoInputLocatorsMatch(
            const fastrtps::rtps::Locator_t& locator_1,
            const fastrtps::rtps::Locator_t& locator_2) const override
    {
        return low_level_transport_->DoInputLocatorsMatch(locator_1, locator_2);
    }

    RTPS_DllAPI
    void select_locators(
            fastrtps::rtps::LocatorSelector& selector) const override
    {
        return low_level_transport_->select_locators(selector);
    }

    RTPS_DllAPI
    void AddDefaultOutputLocator(
            fastrtps::rtps::LocatorList_t& defaultList) override
    {
        return low_level_transport_->AddDefaultOutputLocator(defaultList);
    }

    RTPS_DllAPI
    bool getDefaultMetatrafficMulticastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t metatraffic_multicast_port) const override
    {
        return low_level_transport_->getDefaultMetatrafficMulticastLocators(locators, metatraffic_multicast_port);
    }

    RTPS_DllAPI
    bool getDefaultMetatrafficUnicastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t metatraffic_unicast_port) const override
    {
        return low_level_transport_->getDefaultMetatrafficUnicastLocators(locators, metatraffic_unicast_port);
    }

    RTPS_DllAPI
    bool getDefaultUnicastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t unicast_port) const override
    {
        return low_level_transport_->getDefaultUnicastLocators(locators, unicast_port);
    }

    RTPS_DllAPI
    bool fillMetatrafficMulticastLocator(
            fastrtps::rtps::Locator_t& locator,
            uint32_t metatraffic_multicast_port) const override
    {
        return low_level_transport_->fillMetatrafficMulticastLocator(locator, metatraffic_multicast_port);
    }

    RTPS_DllAPI
    bool fillMetatrafficUnicastLocator(
            fastrtps::rtps::Locator_t& locator,
            uint32_t metatraffic_unicast_port) const override
    {
        return low_level_transport_->fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);
    }

    RTPS_DllAPI
    bool configureInitialPeerLocator(
            fastrtps::rtps::Locator_t& locator,
            const fastrtps::rtps::PortParameters& port_params,
            uint32_t domainId,
            fastrtps::rtps::LocatorList_t& list) const override
    {
        return low_level_transport_->configureInitialPeerLocator(locator, port_params, domainId, list);
    }

    RTPS_DllAPI
    bool fillUnicastLocator(
            fastrtps::rtps::Locator_t& locator,
            uint32_t well_known_port) const override
    {
        return low_level_transport_->fillUnicastLocator(locator, well_known_port);
    }

    RTPS_DllAPI
    bool transform_remote_locator(
            const fastrtps::rtps::Locator_t& remote_locator,
            fastrtps::rtps::Locator_t& result_locator) const override
    {
        return low_level_transport_->transform_remote_locator(remote_locator, result_locator);
    }

    RTPS_DllAPI
    virtual uint32_t max_recv_buffer_size() const override
    {
        return low_level_transport_->max_recv_buffer_size();
    }

    RTPS_DllAPI
    virtual bool send(
            fastrtps::rtps::SenderResource* low_sender_resource,
            const fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            fastrtps::rtps::LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& timeout) = 0;

    RTPS_DllAPI
    virtual void receive(
            TransportReceiverInterface* next_receiver,
            const fastrtps::rtps::octet* receive_buffer,
            uint32_t receive_buffer_size,
            const fastrtps::rtps::Locator_t& local_locator,
            const fastrtps::rtps::Locator_t& remote_locator) = 0;

protected:

    std::unique_ptr<TransportInterface> low_level_transport_;

private:

    std::map<fastrtps::rtps::Locator_t, ChainingReceiverResource*> receiver_resources_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // ifndef CHAINING_TRANSPORT_H
