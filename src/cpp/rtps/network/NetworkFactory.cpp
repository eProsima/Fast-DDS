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
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/transport/UDPv6Transport.h>
#include <fastrtps/transport/test_UDPv4Transport.h>
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

vector<SenderResource> NetworkFactory::BuildSenderResources(Locator_t& local)
{
    vector<SenderResource> newSenderResources;

    for(auto& transport : mRegisteredTransports)
    {
        if ( transport->IsLocatorSupported(local) &&
                !transport->IsOutputChannelOpen(local) )
        {
            SenderResource newSenderResource(*transport, local);
            if (newSenderResource.mValid)
                newSenderResources.push_back(move(newSenderResource));
        }
    }
    return newSenderResources;
}

// TODO(Ricardo) Review if necessary
vector<SenderResource> NetworkFactory::BuildSenderResourcesForRemoteLocator(const Locator_t& remote)
{
    vector<SenderResource> newSenderResources;

    for(auto& transport : mRegisteredTransports)
    {
        Locator_t local = transport->RemoteToMainLocal(remote);
        if ( transport->IsLocatorSupported(local) &&
                !transport->IsOutputChannelOpen(local) )
        {
            SenderResource newSenderResource(*transport, local);
            if (newSenderResource.mValid)
                newSenderResources.push_back(move(newSenderResource));
        }
    }
    return newSenderResources;
}

bool NetworkFactory::BuildReceiverResources (const Locator_t& local, std::vector<ReceiverResource>& returned_resources_list)
{
    bool returnedValue = false;

    for(auto& transport : mRegisteredTransports)
    {
        if(transport->IsLocatorSupported(local))
        {
            if(!transport->IsInputChannelOpen(local))
            {
                ReceiverResource newReceiverResource(*transport, local);
                if(newReceiverResource.mValid)
                {
                    returned_resources_list.push_back(std::move(newReceiverResource));
                    returnedValue = true;
                }
            }
            else
                returnedValue = true;
        }
    }

    return returnedValue;
}

void NetworkFactory::RegisterTransport(const TransportDescriptorInterface* descriptor)
{
    bool wasRegistered = false;
    uint32_t minSendBufferSize = std::numeric_limits<uint32_t>::max();

    if (auto concrete = dynamic_cast<const UDPv4TransportDescriptor*> (descriptor))
    {
        std::unique_ptr<UDPv4Transport> transport(new UDPv4Transport(*concrete));
        if(transport->init())
        {
            minSendBufferSize = transport->get_configuration().sendBufferSize;
            mRegisteredTransports.emplace_back(std::move(transport));
            wasRegistered = true;
        }
    }
    if (auto concrete = dynamic_cast<const UDPv6TransportDescriptor*> (descriptor))
    {
        std::unique_ptr<UDPv6Transport> transport(new UDPv6Transport(*concrete));
        if(transport->init())
        {
            minSendBufferSize = transport->get_configuration().sendBufferSize;
            mRegisteredTransports.emplace_back(std::move(transport));
            wasRegistered = true;
        }
    }
    if (auto concrete = dynamic_cast<const test_UDPv4TransportDescriptor*> (descriptor))
    {
        std::unique_ptr<test_UDPv4Transport> transport(new test_UDPv4Transport(*concrete));
        if(transport->init())
        {
            minSendBufferSize = transport->get_configuration().sendBufferSize;
            mRegisteredTransports.emplace_back(std::move(transport));
            wasRegistered = true;
        }
    }

    if(wasRegistered)
    {
        if(descriptor->maxMessageSize > maxMessageSizeBetweenTransports_)
            maxMessageSizeBetweenTransports_ = descriptor->maxMessageSize;

        if(minSendBufferSize < minSendBufferSize_)
            minSendBufferSize_ = minSendBufferSize;
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
            normalizedLocators.push_back(loc);
            });

    locators.swap(normalizedLocators);
}

void NetworkFactory::FilterLocators(LocatorList_t& locators)
{
    LocatorList_t filteredLocators;

    std::for_each(locators.begin(), locators.end(), [&](Locator_t& loc) {
            bool allowed = false;
            for (auto& transport : mRegisteredTransports)
            {
            if (transport->IsLocatorAllowed(loc))
            {
            allowed = true;
            }
            }

            if (allowed)
            filteredLocators.push_back(loc);
            });

    locators.swap(filteredLocators);
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
    LocatorList_t returnedList;

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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
