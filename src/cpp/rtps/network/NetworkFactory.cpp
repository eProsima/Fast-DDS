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

#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/transport/TransportDescriptorInterface.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPTransportDescriptor.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>
#include <utility>
#include <limits>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

NetworkFactory::NetworkFactory() : maxMessageSizeBetweenTransports_(0),
    minSendBufferSize_(std::numeric_limits<uint32_t>::max())
{
}

vector<SenderResource> NetworkFactory::BuildSenderResources(Locator_t& local, uint32_t size)
{
    vector<SenderResource> newSenderResources;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(local) && !transport->IsOutputChannelOpen(local))
        {
            SenderResource newSenderResource(*transport, local, size);
            if (newSenderResource.mValid)
                newSenderResources.push_back(move(newSenderResource));
        }
    }
    return newSenderResources;
}

bool NetworkFactory::BuildReceiverResources(Locator_t& local, RTPSParticipantImpl* participant,
    uint32_t maxMsgSize, std::vector<std::shared_ptr<ReceiverResource>>& returned_resources_list)
{
    bool returnedValue = false;
    for (auto& transport : mRegisteredTransports)
    {
        if (transport->IsLocatorSupported(local))
        {
            if (!transport->IsInputChannelOpen(local))
            {
                std::shared_ptr<ReceiverResource> newReceiverResource = std::shared_ptr<ReceiverResource>(
                    new ReceiverResource(participant, *transport, local, maxMsgSize));

                if (newReceiverResource->mValid)
                {
                    returned_resources_list.push_back(newReceiverResource);
                    returnedValue = true;
                }
            }
            else
                returnedValue = true;
        }
    }
    return returnedValue;
}

bool NetworkFactory::RegisterTransport(const TransportDescriptorInterface* descriptor)
{
    bool wasRegistered = false;
    uint32_t minSendBufferSize = std::numeric_limits<uint32_t>::max();

    std::unique_ptr<TransportInterface> transport(descriptor->create_transport());
    if(transport->init())
    {
        minSendBufferSize = transport->get_configuration()->min_send_buffer_size();
        mRegisteredTransports.emplace_back(std::move(transport));
        wasRegistered = true;
    }

    if(wasRegistered)
    {
        if(descriptor->max_message_size() > maxMessageSizeBetweenTransports_)
            maxMessageSizeBetweenTransports_ = descriptor->max_message_size();

        if(minSendBufferSize < minSendBufferSize_)
            minSendBufferSize_ = minSendBufferSize;
    }
    return wasRegistered;
}

void NetworkFactory::RegisterTransport(TransportDescriptorInterface* descriptor,
    const GuidPrefix_t& participantGuidPrefix, const RTPSParticipantAttributes* m_att)
{
    if (dynamic_cast<TCPTransportDescriptor*>(descriptor) != nullptr)
    {
        TCPTransportDescriptor* tcpDesc = dynamic_cast<TCPTransportDescriptor*>(descriptor);
        if  (tcpDesc->metadata_logical_port == 0 && m_att != nullptr)
        {
            tcpDesc->metadata_logical_port =
                static_cast<uint16_t>(m_att->port.getUnicastPort(m_att->builtin.domainId, 0));
            //tcpDesc->metadata_logical_port += 4; // Test as 2nd participant
        }
    }

    bool bWasRegistered = RegisterTransport(descriptor);
    if (bWasRegistered)
    {
        mRegisteredTransports.back()->SetParticipantGUIDPrefix(participantGuidPrefix);
    }
}

void NetworkFactory::RegisterDefaultTransport(const RTPSParticipantAttributes& PParam,
        const GuidPrefix_t& participantGuidPrefix)
{
    // The well known default transport is UDPv4
    UDPv4TransportDescriptor descriptor;
    descriptor.sendBufferSize = PParam.sendSocketBufferSize;
    descriptor.receiveBufferSize = PParam.listenSocketBufferSize;

    bool bWasRegistered = RegisterTransport(&descriptor);
    if (bWasRegistered)
    {
        mRegisteredTransports.back()->SetParticipantGUIDPrefix(participantGuidPrefix);
    }
}

void NetworkFactory::NormalizeLocators(LocatorList_t& locators)
{
    LocatorList_t normalizedLocators;

    std::for_each(locators.begin(), locators.end(), [&](Locator_t& loc) {
        bool normalized = false;
        for (auto& transport : mRegisteredTransports)
        {
            if (transport->IsLocatorSupported(loc))
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

LocatorList_t NetworkFactory::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t returnedList;

    for(auto& transport : mRegisteredTransports)
    {
        std::vector<LocatorList_t> transportLocatorLists;

        for(auto& locatorList : locatorLists)
        {
            LocatorList_t resultList;

            for(auto it = locatorList.begin(); it != locatorList.end(); ++it)
                if(transport->IsLocatorSupported(*it))
                    resultList.push_back(*it);

            transportLocatorLists.push_back(resultList);
        }

        returnedList.push_back(transport->ShrinkLocatorLists(transportLocatorLists));
    }

    return returnedList;
}

bool NetworkFactory::is_local_locator(const Locator_t& locator) const
{
    for(auto& transport : mRegisteredTransports)
    {
        if(transport->IsLocatorSupported(locator))
            return transport->is_local_locator(locator);
    }

    return false;
}

size_t NetworkFactory::numberOfRegisteredTransports() const
{
    return mRegisteredTransports.size();
}

bool NetworkFactory::generate_locators(uint16_t physical_port, int locator_kind,
        LocatorList_t &ret_locators)
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
    for (Locator_t &loc : ret_locators)
    {
        loc.kind = locator_kind;
        loc.port = physical_port;
    }
    return !ret_locators.empty();
}

void NetworkFactory::GetDefaultOutputLocators(LocatorList_t &defaultLocators)
{
    defaultLocators.clear();
    for(auto& transport : mRegisteredTransports)
    {
        transport->AddDefaultOutputLocator(defaultLocators);
    }
}

bool NetworkFactory::fillDefaultMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const
{
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    return true;
}

bool NetworkFactory::fillMetatrafficMulticastLocator(Locator_t &locator, uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    }

    if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
    {
        if (IPLocator::getLogicalPort(locator) == 0)
        {
            IPLocator::setLogicalPort(locator, static_cast<uint16_t>(metatraffic_multicast_port));
        }
    }
    return true;
}

bool NetworkFactory::fillDefaultMetatrafficUnicastLocator(Locator_t &locator, uint32_t metatraffic_unicast_port) const
{
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    locator.set_Invalid_Address();
    return true;
}

bool NetworkFactory::fillMetatrafficUnicastLocator(Locator_t &locator, uint32_t metatraffic_unicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    }

    if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
    {
        if (IPLocator::getLogicalPort(locator) == 0)
        {
            IPLocator::setLogicalPort(locator, static_cast<uint16_t>(metatraffic_unicast_port));
        }
    }
    return true;
}

bool NetworkFactory::configureInitialPeerLocator(Locator_t &locator, RTPSParticipantAttributes& m_att) const
{
    if (locator.kind == LOCATOR_KIND_UDPv4 || locator.kind == LOCATOR_KIND_UDPv6)
    {
        if(locator.port == 0)
        {
            // TODO(Ricardo) Make configurable.
            for(int32_t i = 0; i < 4; ++i)
            {
                Locator_t auxloc(locator);
                auxloc.port = static_cast<uint16_t>(m_att.port.getUnicastPort(m_att.builtin.domainId, i));

                m_att.builtin.initialPeersList.push_back(auxloc);
            }
        }
        else
            m_att.builtin.initialPeersList.push_back(locator);
    }
    else if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
    {
        if(IPLocator::getPhysicalPort(locator) == 0)
        {
            // TODO(Ricardo) Make configurable.
            for(int32_t i = 0; i < 4; ++i)
            {
                Locator_t auxloc(locator);
                auxloc.port = static_cast<uint16_t>(m_att.port.getUnicastPort(m_att.builtin.domainId, i));

                if (IPLocator::getLogicalPort(locator) == 0)
                {
                    IPLocator::setLogicalPort(auxloc, static_cast<uint16_t>(m_att.port.getUnicastPort(m_att.builtin.domainId, i)));
                }

                m_att.builtin.initialPeersList.push_back(auxloc);
            }
        }
        else
        {
            if (IPLocator::getLogicalPort(locator) == 0)
            {
                // TODO(Ricardo) Make configurable.
                for(int32_t i = 0; i < 4; ++i)
                {
                    Locator_t auxloc(locator);
                    IPLocator::setLogicalPort(auxloc, static_cast<uint16_t>(m_att.port.getUnicastPort(m_att.builtin.domainId, i)));
                    m_att.builtin.initialPeersList.push_back(auxloc);
                }
            }
            else
            {
                m_att.builtin.initialPeersList.push_back(locator);
            }
        }


    }
    return true;
}

bool NetworkFactory::fillDefaultUnicastLocator(Locator_t &locator, const RTPSParticipantAttributes& m_att) const
{
    //locator.port = calculateWellKnownPort(m_att);
    //locator.kind = LOCATOR_KIND_UDPv4;

    if (IPLocator::getPortRTPS(locator) == 0)
    {
        IPLocator::setPortRTPS(locator, calculateWellKnownPort(m_att));
    }
    if (IPLocator::getPhysicalPort(locator) == 0) // TCP Case, set physical too
    {
        locator.port = calculateWellKnownPort(m_att);
    }

    // In TCP case, calculate logical port too
    /*
    if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
    {
        if (IPLocator::getLogicalPort(locator) == 0)
        {
            IPLocator::setLogicalPort(locator, calculateWellKnownPort(m_att));
        }
    }
    */
    return true;
}

uint16_t NetworkFactory::calculateWellKnownPort(const RTPSParticipantAttributes& att) const
{
    return static_cast<uint16_t>(att.port.portBase +
            att.port.domainIDGain*att.builtin.domainId +
            att.port.offsetd3 +
            att.port.participantIDGain*att.participantID);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
