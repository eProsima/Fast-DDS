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

#include <rtps/network/NetworkFactory.hpp>

#include <limits>
#include <utility>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>
#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <rtps/transport/TCPTransportInterface.h>

using namespace std;

namespace eprosima {
namespace fastdds {
namespace rtps {

using SendResourceList = fastdds::rtps::SendResourceList;

NetworkFactory::NetworkFactory(
        const RTPSParticipantAttributes& PParam)
    : maxMessageSizeBetweenTransports_((std::numeric_limits<uint32_t>::max)())
    , minSendBufferSize_((std::numeric_limits<uint32_t>::max)())
    , network_configuration_(0)
{
    const std::string* enforce_metatraffic = nullptr;
    enforce_metatraffic = PropertyPolicyHelper::find_property(PParam.properties, "fastdds.shm.enforce_metatraffic");
    if (enforce_metatraffic)
    {
        if (*enforce_metatraffic == "unicast")
        {
            enforce_shm_unicast_metatraffic_ = true;
            enforce_shm_multicast_metatraffic_ = false;
        }
        else if (*enforce_metatraffic == "all")
        {
            enforce_shm_unicast_metatraffic_ = true;
            enforce_shm_multicast_metatraffic_ = true;
        }
        else if (*enforce_metatraffic == "none")
        {
            enforce_shm_unicast_metatraffic_ = false;
            enforce_shm_multicast_metatraffic_ = false;
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_NETWORK, "Unrecognized value '" << *enforce_metatraffic << "'" <<
                    " for 'fastdds.shm.enforce_metatraffic'. Using default value: 'none'");
        }
    }
}

bool NetworkFactory::build_send_resources(
        SendResourceList& sender_resource_list,
        const Locator_t& locator)
{
    bool returned_value = false;

    for (auto& transport : mRegisteredTransports)
    {
        returned_value |= transport->OpenOutputChannel(sender_resource_list, locator);
    }

    return returned_value;
}

bool NetworkFactory::build_send_resources(
        SendResourceList& sender_resource_list,
        const LocatorSelectorEntry& locator_selector_entry)
{
    bool returned_value = false;

    for (auto& transport : mRegisteredTransports)
    {
        returned_value |= transport->OpenOutputChannels(sender_resource_list, locator_selector_entry);
    }

    return returned_value;
}

bool NetworkFactory::BuildReceiverResources(
        Locator_t& local,
        std::vector<std::shared_ptr<ReceiverResource>>& returned_resources_list,
        uint32_t receiver_max_message_size)
{
    bool returnedValue = false;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(local))
        {
            if (!transport->IsInputChannelOpen(local))
            {
                uint32_t max_recv_buffer_size = (std::min)(
                    transport->max_recv_buffer_size(),
                    receiver_max_message_size);

                std::shared_ptr<ReceiverResource> newReceiverResource = std::shared_ptr<ReceiverResource>(
                    new ReceiverResource(*transport, local, max_recv_buffer_size));

                if (newReceiverResource->mValid)
                {
                    returned_resources_list.push_back(newReceiverResource);
                    returnedValue = true;
                }
            }
            else
            {
                returnedValue = true;
            }
        }
    }
    return returnedValue;
}

bool NetworkFactory::RegisterTransport(
        const TransportDescriptorInterface* descriptor,
        const fastdds::rtps::PropertyPolicy* properties,
        const uint32_t& max_msg_size_no_frag)
{
    bool wasRegistered = false;

    uint32_t minSendBufferSize = (std::numeric_limits<uint32_t>::max)();

    std::unique_ptr<TransportInterface> transport(descriptor->create_transport());

    if (transport)
    {
        int32_t kind = transport->kind();
        bool is_localhost_allowed = transport->is_localhost_allowed();

        if (transport->init(properties, max_msg_size_no_frag))
        {
            minSendBufferSize = transport->get_configuration()->min_send_buffer_size();
            mRegisteredTransports.emplace_back(std::move(transport));
            wasRegistered = true;
        }

        if (wasRegistered)
        {
            if (descriptor->max_message_size() < maxMessageSizeBetweenTransports_)
            {
                maxMessageSizeBetweenTransports_ = descriptor->max_message_size();
            }

            if (minSendBufferSize < minSendBufferSize_)
            {
                minSendBufferSize_ = minSendBufferSize;
            }

            if (is_localhost_allowed)
            {
                network_configuration_ |= kind;
            }
        }
    }

    return wasRegistered;
}

void NetworkFactory::NormalizeLocators(
        LocatorList_t& locators)
{
    LocatorList_t normalizedLocators;

    std::for_each(locators.begin(), locators.end(), [&](Locator_t& loc)
            {
                bool normalized = false;
                for (auto& transport : mRegisteredTransports)
                {
                    // Check if the locator is supported and filter unicast locators.
                    if (transport->IsLocatorSupported(loc) &&
                    (IPLocator::isMulticast(loc) ||
                    transport->is_locator_allowed(loc)))
                    {
                        // First found transport that supports it, this will normalize the locator.
                        normalizedLocators.push_back(transport->NormalizeLocator(loc));
                        normalized = true;
                    }
                }

                if (!normalized)
                {
                    normalizedLocators.push_back(loc);
                }
            });

    locators.swap(normalizedLocators);
}

bool NetworkFactory::transform_remote_locator(
        const Locator_t& remote_locator,
        Locator_t& result_locator,
        const NetworkConfigSet_t& remote_network_config) const
{
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->transform_remote_locator(remote_locator, result_locator,
                remote_network_config & remote_locator.kind, network_configuration_ & remote_locator.kind))
        {
            return true;
        }
    }

    return false;
}

bool NetworkFactory::transform_remote_locator(
        const Locator_t& remote_locator,
        Locator_t& result_locator,
        const NetworkConfigSet_t& remote_network_config,
        bool is_fastdds_local) const
{
    if (!is_locator_supported(remote_locator))
    {
        return false;
    }

    if (is_fastdds_local)
    {
        return transform_remote_locator(remote_locator, result_locator, remote_network_config);
    }
    else
    {
        result_locator = remote_locator;
        return true;
    }
}

bool NetworkFactory::is_locator_supported(
        const Locator_t& locator) const
{
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(locator))
        {
            return true;
        }
    }

    return false;
}

bool NetworkFactory::is_locator_allowed(
        const Locator_t& locator) const
{
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->is_locator_allowed(locator))
        {
            return true;
        }
    }

    return false;
}

bool NetworkFactory::is_locator_remote_or_allowed(
        const Locator_t& locator) const
{
    return !is_local_locator(locator) || is_locator_allowed(locator);
}

bool NetworkFactory::is_locator_remote_or_allowed(
        const Locator_t& locator,
        bool is_fastdds_local) const
{
    return (is_locator_supported(locator) && !is_fastdds_local) || is_locator_allowed(locator);
}

bool NetworkFactory::is_locator_reachable(
        const Locator_t& locator)
{
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->is_locator_reachable(locator))
        {
            return true;
        }
    }

    return false;
}

void NetworkFactory::select_locators(
        LocatorSelector& selector) const
{
    selector.selection_start();

    /* - for each transport:
     *   - transport_starts is called
     *   - transport handles the selection state of each entry
     *   - select may be called
     */
    for (auto& transport : mRegisteredTransports)
    {
        transport->select_locators(selector);
    }
}

bool NetworkFactory::is_local_locator(
        const Locator_t& locator) const
{
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(locator))
        {
            return transport->is_local_locator(locator);
        }
    }

    return false;
}

size_t NetworkFactory::numberOfRegisteredTransports() const
{
    return mRegisteredTransports.size();
}

bool NetworkFactory::generate_locators(
        uint16_t physical_port,
        int locator_kind,
        LocatorList_t& ret_locators)
{
    ret_locators.clear();
    if (locator_kind == LOCATOR_KIND_TCPv4 || locator_kind == LOCATOR_KIND_UDPv4)
    {
        IPFinder::getIP4Address(&ret_locators);
    }
    else if (locator_kind == LOCATOR_KIND_TCPv6 || locator_kind == LOCATOR_KIND_UDPv6)
    {
        IPFinder::getIP6Address(&ret_locators);
    }
    for (Locator_t& loc : ret_locators)
    {
        loc.kind = locator_kind;
        loc.port = physical_port;
    }
    return !ret_locators.empty();
}

void NetworkFactory::GetDefaultOutputLocators(
        LocatorList_t& defaultLocators)
{
    defaultLocators.clear();
    for (auto& transport : mRegisteredTransports)
    {
        transport->AddDefaultOutputLocator(defaultLocators);
    }
}

bool NetworkFactory::getDefaultMetatrafficMulticastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_multicast_port) const
{
    bool result = false;

    TransportInterface* shm_transport = nullptr;

    for (auto& transport : mRegisteredTransports)
    {
        // For better fault-tolerance reasons, SHM metatraffic is avoided if it is already provided
        // by another transport
        if (enforce_shm_multicast_metatraffic_ || transport->kind() != LOCATOR_KIND_SHM)
        {
            result |= transport->getDefaultMetatrafficMulticastLocators(locators, metatraffic_multicast_port);
        }
        else
        {
            shm_transport = transport.get();
        }
    }

    if (locators.size() == 0 && shm_transport)
    {
        result |= shm_transport->getDefaultMetatrafficMulticastLocators(locators, metatraffic_multicast_port);
    }

    return result;
}

bool NetworkFactory::fillMetatrafficMulticastLocator(
        Locator_t& locator,
        uint32_t metatraffic_multicast_port) const
{
    bool result = false;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(locator))
        {
            result |= transport->fillMetatrafficMulticastLocator(locator, metatraffic_multicast_port);
        }
    }
    return result;
}

bool NetworkFactory::getDefaultMetatrafficUnicastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_unicast_port) const
{
    bool result = false;

    TransportInterface* shm_transport = nullptr;

    for (auto& transport : mRegisteredTransports)
    {
        // For better fault-tolerance reasons, SHM metatraffic is avoided if it is already provided
        // by another transport
        if (enforce_shm_unicast_metatraffic_ || transport->kind() != LOCATOR_KIND_SHM)
        {
            result |= transport->getDefaultMetatrafficUnicastLocators(locators, metatraffic_unicast_port);
        }
        else
        {
            shm_transport = transport.get();
        }
    }

    if (locators.size() == 0 && shm_transport)
    {
        result |= shm_transport->getDefaultMetatrafficUnicastLocators(locators, metatraffic_unicast_port);
    }

    return result;
}

bool NetworkFactory::fillMetatrafficUnicastLocator(
        Locator_t& locator,
        uint32_t metatraffic_unicast_port) const
{
    bool result = false;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(locator))
        {
            result |= transport->fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);
        }
    }
    return result;
}

bool NetworkFactory::configureInitialPeerLocator(
        uint32_t domain_id,
        Locator_t& locator,
        RTPSParticipantAttributes& m_att) const
{
    bool result = false;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(locator))
        {
            result |= transport->configureInitialPeerLocator(locator, m_att.port, domain_id,
                            m_att.builtin.initialPeersList);
        }
    }
    return result;
}

bool NetworkFactory::getDefaultUnicastLocators(
        LocatorList_t& locators,
        uint32_t port) const
{
    bool result = false;
    for (auto& transport : mRegisteredTransports)
    {
        result |= transport->getDefaultUnicastLocators(locators, port);
    }
    return result;
}

bool NetworkFactory::fill_default_locator_port(
        Locator_t& locator,
        uint32_t port) const
{
    bool result = false;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(locator))
        {
            result |= transport->fillUnicastLocator(locator, port);
        }
    }
    return result;
}

void NetworkFactory::Shutdown()
{
    for (auto& transport : mRegisteredTransports)
    {
        transport->shutdown();
    }
}

uint16_t NetworkFactory::calculate_well_known_port(
        uint32_t domain_id,
        const RTPSParticipantAttributes& att,
        bool is_multicast) const
{

    uint32_t port = att.port.portBase +
            att.port.domainIDGain * domain_id +
            (is_multicast ?
            att.port.offsetd2 :
            att.port.offsetd3 + att.port.participantIDGain * att.participantID);

    if (port > 65535)
    {
        EPROSIMA_LOG_ERROR(RTPS, "Calculated port number is too high. Probably the domainId is over 232, there are "
                << "too much participants created or portBase is too high.");
        std::cout << "Calculated port number is too high. Probably the domainId is over 232, there are "
                  << "too much participants created or portBase is too high." << std::endl;
        std::cout.flush();
        exit(EXIT_FAILURE);
    }

    return static_cast<uint16_t>(port);
}

void NetworkFactory::update_network_interfaces()
{
    for (auto& transport : mRegisteredTransports)
    {
        transport->update_network_interfaces();
    }
}

void NetworkFactory::remove_participant_associated_send_resources(
        SendResourceList& send_resource_list,
        const LocatorList_t& remote_participant_locators,
        const LocatorList_t& participant_initial_peers_and_ds) const
{
    // TODO(eduponz): Call the overload of CloseOutputChannel that takes a LocatorSelectorEntry for
    // all transports and let them decide what to do.
    for (auto& transport : mRegisteredTransports)
    {
        TCPTransportInterface* tcp_transport = dynamic_cast<TCPTransportInterface*>(transport.get());
        if (tcp_transport)
        {
            tcp_transport->cleanup_sender_resources(
                send_resource_list,
                remote_participant_locators,
                participant_initial_peers_and_ds);
        }
    }
}

std::vector<TransportNetmaskFilterInfo> NetworkFactory::netmask_filter_info() const
{
    std::vector<TransportNetmaskFilterInfo> ret;
    for (auto& transport : mRegisteredTransports)
    {
        ret.push_back({transport->kind(), transport->netmask_filter_info()});
    }
    return ret;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
