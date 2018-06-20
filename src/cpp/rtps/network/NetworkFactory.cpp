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
#include <fastrtps/transport/TransportDescriptor.h>
#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/utils/IPFinder.h>
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
        minSendBufferSize = transport->get_configuration()->sendBufferSize;
        mRegisteredTransports.emplace_back(std::move(transport));
        wasRegistered = true;
    }

    if(wasRegistered)
    {
        if(descriptor->maxMessageSize > maxMessageSizeBetweenTransports_)
            maxMessageSizeBetweenTransports_ = descriptor->maxMessageSize;

        if(minSendBufferSize < minSendBufferSize_)
            minSendBufferSize_ = minSendBufferSize;
    }
    return wasRegistered;
}

void NetworkFactory::RegisterTransport(const TransportDescriptorInterface* descriptor,
    const GuidPrefix_t& participantGuidPrefix)
{
    bool bWasRegistered = RegisterTransport(descriptor);
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
    for (Locator_t loc : ret_locators)
    {
        loc.kind = locator_kind;
        loc.set_port(physical_port);
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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
