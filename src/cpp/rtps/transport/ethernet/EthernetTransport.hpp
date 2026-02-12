/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_ETH_TRANSPORT__SRC__ETHERNETTRANSPORT_HPP_
#define FASTDDS_ETH_TRANSPORT__SRC__ETHERNETTRANSPORT_HPP_

#include <cstdint>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>

#include <rtps/transport/ethernet/InputChannelManager.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct EthernetTransport : public TransportInterface
{
    using TransportInterface::transform_remote_locator;

    explicit EthernetTransport(
            const EthernetTransportDescriptor& descriptor);

    ~EthernetTransport() override = default;

    bool init(
            const PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) override;

    void shutdown() override;

    bool IsInputChannelOpen(
            const Locator& locator) const override;

    bool IsLocatorSupported(
            const Locator& locator) const override;

    bool is_locator_allowed(
            const Locator& locator) const override;

    bool is_locator_reachable(
            const Locator& locator) override;

    Locator RemoteToMainLocal(
            const Locator& remote) const override;

    bool transform_remote_locator(
            const Locator& remote_locator,
            Locator& result_locator) const override;

    bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const Locator& locator) override;

    bool OpenOutputChannels(
            SendResourceList& sender_resource_list,
            const LocatorSelectorEntry& locator_selector_entry) override;

    bool OpenInputChannel(
            const Locator& locator,
            TransportReceiverInterface* receiver_interface,
            uint32_t max_message_size) override;

    bool CloseInputChannel(
            const Locator& locator) override;

    bool DoInputLocatorsMatch(
            const Locator& locator1,
            const Locator& locator2) const override;

    LocatorList NormalizeLocator(
            const Locator& locator) override;

    void select_locators(
            LocatorSelector& selector) const override;

    bool is_local_locator(
            const Locator& locator) const override;

    TransportDescriptorInterface* get_configuration() override;

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

    bool fillMetatrafficMulticastLocator(
            Locator& locator,
            uint32_t metatraffic_multicast_port) const override;

    bool fillMetatrafficUnicastLocator(
            Locator& locator,
            uint32_t metatraffic_unicast_port) const override;

    bool configureInitialPeerLocator(
            Locator& locator,
            const fastdds::rtps::PortParameters& port_params,
            uint32_t domainId,
            LocatorList& list) const override;

    bool fillUnicastLocator(
            Locator& locator,
            uint32_t well_known_port) const override;

    uint32_t max_recv_buffer_size() const override;

private:

    EthernetTransportDescriptor configuration_;
    InputChannelManager input_channels_;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_ETH_TRANSPORT__SRC__ETHERNETTRANSPORT_HPP_
