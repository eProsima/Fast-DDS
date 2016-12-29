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
using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

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
    if (auto concrete = dynamic_cast<const UDPv4TransportDescriptor*> (descriptor))
    {
        std::unique_ptr<UDPv4Transport> transport(new UDPv4Transport(*concrete));
        if(transport->init())
            mRegisteredTransports.emplace_back(std::move(transport));
    }
    if (auto concrete = dynamic_cast<const UDPv6TransportDescriptor*> (descriptor))
    {
        std::unique_ptr<UDPv6Transport> transport(new UDPv6Transport(*concrete));
        if(transport->init())
            mRegisteredTransports.emplace_back(std::move(transport));
    }
    if (auto concrete = dynamic_cast<const test_UDPv4TransportDescriptor*> (descriptor))
    {
        std::unique_ptr<test_UDPv4Transport> transport(new test_UDPv4Transport(*concrete));
        if(transport->init())
            mRegisteredTransports.emplace_back(std::move(transport));
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

size_t NetworkFactory::numberOfRegisteredTransports() const
{
    return mRegisteredTransports.size();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
