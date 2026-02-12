/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>

#include <rtps/transport/ethernet/EthernetPacket.hpp>
#include <rtps/transport/ethernet/EthernetSenderResource.hpp>
#include <rtps/transport/ethernet/EthernetTransport.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Invalidate all selector entries containing certain multicast locator.
 *
 * This function will process all entries from 'index' onwards and, if any
 * of them has 'locator' on its multicast list, will invalidate them
 * (i.e. their 'transport_should_process' flag will be changed to false).
 *
 * If this function returns true, the locator received should be selected.
 *
 * @param entries   Selector entries collection to process
 * @param index     Starting index to process
 * @param locator   Locator to be searched
 *
 * @return true when at least one entry was invalidated, false otherwise
 */
static bool check_and_invalidate(
        fastdds::ResourceLimitedVector<LocatorSelectorEntry*>& entries,
        size_t index,
        const Locator& locator)
{
    bool ret_val = false;
    for (; index < entries.size(); ++index)
    {
        LocatorSelectorEntry* entry = entries[index];
        if (entry->transport_should_process)
        {
            for (const Locator& loc : entry->multicast)
            {
                if (loc == locator)
                {
                    entry->transport_should_process = false;
                    ret_val = true;
                    break;
                }
            }
        }
    }

    return ret_val;
}

static constexpr EthernetAddress DEFAULT_METATRAFFIC_MULTICAST_ADDRESS{ 0x01, 0x00, 0x5E, 0x7F, 0x00, 0x01 };

EthernetTransportDescriptor::EthernetTransportDescriptor()
    : PortBasedTransportDescriptor(EthernetPacket::MAX_RTPS_PAYLOAD_SIZE, 1)
{
}

TransportInterface* EthernetTransportDescriptor::create_transport() const
{
    return new EthernetTransport(*this);
}

EthernetTransport::EthernetTransport(
        const EthernetTransportDescriptor& descriptor)
    : TransportInterface(LOCATOR_KIND_ETHERNET)
    , configuration_(descriptor)
{
}

bool EthernetTransport::init(
        const PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    static_cast<void>(properties);

    if (configuration_.interface_name.empty())
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT, "No interface name specified");
        return false;
    }

    uint32_t max_msg_size = max_msg_size_no_frag == 0 ? EthernetPacket::MAX_RTPS_PAYLOAD_SIZE : max_msg_size_no_frag;
    uint32_t cfg_max_msg_size = configuration_.maxMessageSize;
    uint32_t cfg_send_size = configuration_.send_buffer_size;
    uint32_t cfg_recv_size = configuration_.receive_buffer_size;
    constexpr uint32_t max_int_value = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());

    if (cfg_max_msg_size > EthernetPacket::MAX_RTPS_PAYLOAD_SIZE)
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Maximum message size exceeds EthernetPacket limit: "
                << cfg_max_msg_size << " > " << EthernetPacket::MAX_RTPS_PAYLOAD_SIZE);
        return false;
    }

    if (cfg_max_msg_size > max_msg_size)
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT, "maxMessageSize cannot be greater than " << max_msg_size);
        return false;
    }

    if (cfg_send_size > max_int_value)
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Send buffer size exceeds maximum integer value: " << cfg_send_size << " > " << max_int_value);
        return false;
    }

    if (cfg_recv_size > max_int_value)
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Receive buffer size exceeds maximum integer value: " << cfg_recv_size << " > " << max_int_value);
        return false;
    }

    if ((cfg_send_size > 0) && (cfg_max_msg_size > cfg_send_size))
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Send buffer size cannot be less than maximum message size: "
                << cfg_send_size << " < " << cfg_max_msg_size);
        return false;
    }

    if ((cfg_recv_size > 0) && (cfg_max_msg_size > cfg_recv_size))
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Receive buffer size cannot be less than maximum message size: "
                << cfg_recv_size << " < " << cfg_max_msg_size);
        return false;
    }

    return input_channels_.init(configuration_);
}

void EthernetTransport::shutdown()
{
    input_channels_.stop();
}

bool EthernetTransport::IsLocatorSupported(
        const Locator& locator) const
{
    return locator.kind == transport_kind_;
}

bool EthernetTransport::is_locator_allowed(
        const Locator& locator) const
{
    return IsLocatorSupported(locator);
}

bool EthernetTransport::is_locator_reachable(
        const Locator& locator)
{
    return IsLocatorSupported(locator);
}

Locator EthernetTransport::RemoteToMainLocal(
        const Locator& remote) const
{
    return remote;
}

bool EthernetTransport::transform_remote_locator(
        const Locator& remote_locator,
        Locator& result_locator) const
{
    if (IsLocatorSupported(remote_locator))
    {
        result_locator = remote_locator;
        return true;
    }

    return false;
}

bool EthernetTransport::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const Locator& locator)
{
    // Check if the locator is supported by this transport
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // Find or add the sender resource for our transport
    EthernetSenderResource* eth_sender = nullptr;
    for (auto& sender_resource : sender_resource_list)
    {
        if (sender_resource->kind() == LOCATOR_KIND_ETHERNET)
        {
            eth_sender = static_cast<EthernetSenderResource*>(sender_resource.get());
            break;
        }
    }
    if (eth_sender == nullptr)
    {
        eth_sender = new EthernetSenderResource(configuration_);
        sender_resource_list.push_back(std::unique_ptr<SenderResource>(eth_sender));
    }

    return true;
}

bool EthernetTransport::OpenOutputChannels(
        SendResourceList& sender_resource_list,
        const LocatorSelectorEntry& locator_selector_entry)
{
    bool success = false;
    for (size_t i = 0; i < locator_selector_entry.state.unicast.size(); ++i)
    {
        size_t index = locator_selector_entry.state.unicast[i];
        success = success || OpenOutputChannel(sender_resource_list, locator_selector_entry.unicast[index]);
    }
    for (size_t i = 0; i < locator_selector_entry.state.multicast.size(); ++i)
    {
        size_t index = locator_selector_entry.state.multicast[i];
        success = success || OpenOutputChannel(sender_resource_list, locator_selector_entry.multicast[index]);
    }
    return success;
}

bool EthernetTransport::IsInputChannelOpen(
        const Locator& locator) const
{
    // Check if the locator is supported by this transport
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // Extract interface address from the locator and check if the input channel is open
    EthernetAddress address = reinterpret_cast<const EthernetLocator&>(locator).address;
    uint16_t port = EthernetLocator::get_logical_port(locator);
    return input_channels_.is_open(address, port);
}

bool EthernetTransport::OpenInputChannel(
        const Locator& locator,
        TransportReceiverInterface* receiver_interface,
        uint32_t max_message_size)
{
    // Check if the locator is supported by this transport
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // Extract interface address from the locator and open the input channel
    EthernetAddress address = reinterpret_cast<const EthernetLocator&>(locator).address;
    uint16_t port = EthernetLocator::get_logical_port(locator);
    return input_channels_.open(address, port, receiver_interface, max_message_size);
}

bool EthernetTransport::CloseInputChannel(
        const Locator& locator)
{
    // Check if the locator is supported by this transport
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // Extract interface address from the locator and close the input channel
    EthernetAddress address = reinterpret_cast<const EthernetLocator&>(locator).address;
    uint16_t port = EthernetLocator::get_logical_port(locator);
    return input_channels_.close(address, port);
}

bool EthernetTransport::DoInputLocatorsMatch(
        const Locator& locator1,
        const Locator& locator2) const
{
    assert(locator1.kind == transport_kind_);
    assert(locator2.kind == transport_kind_);
    return input_channels_.locators_match(locator1, locator2);
}

LocatorList EthernetTransport::NormalizeLocator(
        const Locator& locator)
{
    assert(locator.kind == transport_kind_);

    LocatorList normalized_locators;

    if (EthernetLocator::is_ethernet_any(locator))
    {
        EthernetAddress addr = input_channels_.get_interface_address();
        Locator new_locator = EthernetLocator::create_locator(addr, 0, 0, 0);
        new_locator.port = locator.port;
        normalized_locators.push_back(new_locator);
    }
    else
    {
        normalized_locators.push_back(locator);
    }

    return normalized_locators;
}

void EthernetTransport::select_locators(
        LocatorSelector& selector) const
{
    fastdds::ResourceLimitedVector<LocatorSelectorEntry*>& entries = selector.transport_starts();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        LocatorSelectorEntry* entry = entries[i];
        if (entry->transport_should_process)
        {
            bool selected = false;

            // First try to find a multicast locator which is at least on another list.
            for (size_t j = 0; j < entry->multicast.size() && !selected; ++j)
            {
                if (IsLocatorSupported(entry->multicast[j]))
                {
                    if (check_and_invalidate(entries, i + 1, entry->multicast[j]))
                    {
                        entry->state.multicast.push_back(j);
                        selected = true;
                    }
                    else if (entry->unicast.size() == 0)
                    {
                        entry->state.multicast.push_back(j);
                        selected = true;
                    }
                }
            }

            // If we couldn't find a multicast locator, select all unicast locators
            if (!selected)
            {
                for (size_t j = 0; j < entry->unicast.size(); ++j)
                {
                    if (IsLocatorSupported(entry->unicast[j]) && !selector.is_selected(entry->unicast[j]))
                    {
                        entry->state.unicast.push_back(j);
                        selected = true;
                    }
                }
            }

            // Select this entry if necessary
            if (selected)
            {
                selector.select(i);
            }
        }
    }
}

bool EthernetTransport::is_local_locator(
        const Locator& locator) const
{
    assert(locator.kind == transport_kind_);

    EthernetAddress locator_address = reinterpret_cast<const EthernetLocator&>(locator).address;
    return input_channels_.get_interface_address() == locator_address;
}

TransportDescriptorInterface* EthernetTransport::get_configuration()
{
    return &configuration_;
}

void EthernetTransport::AddDefaultOutputLocator(
        LocatorList& defaultList)
{
    static_cast<void>(defaultList);
}

bool EthernetTransport::getDefaultMetatrafficMulticastLocators(
        LocatorList& locators,
        uint32_t metatraffic_multicast_port) const
{
    if (metatraffic_multicast_port > std::numeric_limits<uint16_t>::max())
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Metatraffic multicast port exceeds maximum uint16_t value: "
                << metatraffic_multicast_port << " > " << std::numeric_limits<uint16_t>::max());
        return false;
    }

    Locator locator = EthernetLocator::create_locator(
        DEFAULT_METATRAFFIC_MULTICAST_ADDRESS, 0, 0, 0);
    locator.port = metatraffic_multicast_port;
    locators.push_back(locator);
    return true;
}

bool EthernetTransport::getDefaultMetatrafficUnicastLocators(
        LocatorList& locators,
        uint32_t metatraffic_unicast_port) const
{
    if (metatraffic_unicast_port > std::numeric_limits<uint16_t>::max())
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Metatraffic multicast port exceeds maximum uint16_t value: "
                << metatraffic_unicast_port << " > " << std::numeric_limits<uint16_t>::max());
        return false;
    }

    Locator locator(LOCATOR_KIND_ETHERNET, metatraffic_unicast_port);
    locators.push_back(locator);
    return true;
}

bool EthernetTransport::getDefaultUnicastLocators(
        LocatorList& locators,
        uint32_t unicast_port) const
{
    if (unicast_port > std::numeric_limits<uint16_t>::max())
    {
        EPROSIMA_LOG_ERROR(RTPS_ETHERNET_TRANSPORT,
                "Unicast port exceeds maximum uint16_t value: "
                << unicast_port << " > " << std::numeric_limits<uint16_t>::max());
        return false;
    }

    Locator locator(LOCATOR_KIND_ETHERNET, unicast_port);
    locators.push_back(locator);
    return true;
}

bool EthernetTransport::fillMetatrafficMulticastLocator(
        Locator& locator,
        uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_multicast_port;
    }
    return true;
}

bool EthernetTransport::fillMetatrafficUnicastLocator(
        Locator& locator,
        uint32_t metatraffic_unicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_unicast_port;
    }
    return true;
}

bool EthernetTransport::configureInitialPeerLocator(
        Locator& locator,
        const fastdds::rtps::PortParameters& port_params,
        uint32_t domainId,
        LocatorList& list) const
{
    assert(locator.kind == transport_kind_);

    if (locator.port == 0)
    {
        if (EthernetLocator::is_ethernet_multicast(locator))
        {
            Locator auxloc(locator);
            auxloc.port = port_params.getMulticastPort(domainId);
            list.push_back(auxloc);
        }
        else
        {
            for (uint32_t i = 0; i < configuration_.maxInitialPeersRange; ++i)
            {
                Locator auxloc(locator);
                auxloc.port = port_params.getUnicastPort(domainId, i);

                list.push_back(auxloc);
            }
        }
    }
    else
    {
        list.push_back(locator);
    }

    return true;
}

bool EthernetTransport::fillUnicastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    if (locator.port == 0)
    {
        locator.port = well_known_port;
    }
    return true;
}

uint32_t EthernetTransport::max_recv_buffer_size() const
{
    return configuration_.maxMessageSize;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
