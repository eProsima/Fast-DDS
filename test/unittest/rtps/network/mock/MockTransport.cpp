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

#include <algorithm>
#include <vector>

#include <MockTransport.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

std::vector<MockTransport*> MockTransport::mockTransportInstances;

MockTransport::MockTransport(
        const MockTransportDescriptor& descriptor)
    : TransportInterface(descriptor.supportedKind)
    , mockMaximumChannels(descriptor.maximumChannels)
{
    mockTransportInstances.push_back(this);
}

MockTransport::MockTransport()
    : TransportInterface(DefaultKind)
    , mockMaximumChannels(DefaultMaxChannels)
{
    mockTransportInstances.push_back(this);
}

MockTransport::~MockTransport()
{
    // Remove this mock from the handle vector
    mockTransportInstances.erase(std::remove(mockTransportInstances.begin(),
            mockTransportInstances.end(),
            this),
            mockTransportInstances.end());
}

bool MockTransport::init(
        const fastdds::rtps::PropertyPolicy* /*properties*/,
        const uint32_t& /*max_msg_size_no_frag*/)
{
    return true;
}

bool MockTransport::IsInputChannelOpen(
        const Locator_t& locator) const
{
    return (find(mockOpenInputChannels.begin(), mockOpenInputChannels.end(),
           locator.port) != mockOpenInputChannels.end());
}

bool MockTransport::IsLocatorSupported(
        const Locator_t& locator) const
{
    return locator.kind == transport_kind_;
}

bool MockTransport::is_locator_allowed(
        const Locator_t& /*locator*/) const
{
    return true;
}

bool MockTransport::is_locator_reachable(
        const Locator_t& /*locator*/)
{
    return true;
}

bool MockTransport::OpenOutputChannel(
        fastdds::rtps::SendResourceList& send_resource_list,
        const Locator_t& locator)
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    for (auto& send_resource : send_resource_list)
    {
        MockSenderResource* mock_send_resource = dynamic_cast<MockSenderResource*>(send_resource.get());
        if (mock_send_resource->locator().port == locator.port)
        {
            return true;
        }
    }

    send_resource_list.emplace_back(static_cast<SenderResource*>(new MockSenderResource(locator)));
    return true;
}

bool MockTransport::OpenInputChannel(
        const Locator_t& locator,
        fastdds::rtps::TransportReceiverInterface*,
        uint32_t)
{
    mockOpenInputChannels.push_back(locator.port);
    return true;
}

bool MockTransport::DoInputLocatorsMatch(
        const Locator_t& left,
        const Locator_t& right) const
{
    return left.port == right.port;
}

Locator_t MockTransport::RemoteToMainLocal(
        const Locator_t& remote) const
{
    Locator_t mainLocal(remote);
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

bool MockTransport::CloseInputChannel(
        const Locator_t& locator)
{
    mockOpenInputChannels.erase(std::remove(mockOpenInputChannels.begin(),
            mockOpenInputChannels.end(),
            locator.port),
            mockOpenInputChannels.end());
    return true;
}

LocatorList_t MockTransport::NormalizeLocator(
        const Locator_t& locator)
{
    LocatorList_t list;
    list.push_back(locator);
    return list;
}

void MockTransport::select_locators(
        fastdds::rtps::LocatorSelector& selector) const
{
    fastdds::ResourceLimitedVector<fastdds::rtps::LocatorSelectorEntry*>& entries = selector.transport_starts();
    for (size_t i = 0; i < entries.size(); ++i)
    {
        fastdds::rtps::LocatorSelectorEntry* entry = entries[i];
        if (entry->transport_should_process)
        {
            for (size_t j = 0; j < entry->unicast.size(); ++j)
            {
                entry->state.unicast.push_back(j);
            }

            selector.select(i);
        }
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
