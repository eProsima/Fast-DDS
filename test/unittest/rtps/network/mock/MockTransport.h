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

#ifndef MOCK_TRANSPORT_H
#define MOCK_TRANSPORT_H

#include <utility>
#include <vector>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/transport/SocketTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class MockTransportDescriptor;

class MockSenderResource : public SenderResource
{
public:

    MockSenderResource(
            Locator_t locator)
        : SenderResource(locator.kind)
        , locator_(locator)
    {
    }

    const Locator_t& locator() const
    {
        return locator_;
    }

private:

    Locator_t locator_;
};

class MockTransport : public fastdds::rtps::TransportInterface
{
public:

    MockTransport(
            const MockTransportDescriptor& descriptor);

    MockTransport();

    ~MockTransport();

    bool init(
            const fastdds::rtps::PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) override;

    //API implementation
    bool IsInputChannelOpen(
            const Locator_t&)  const override;

    bool OpenOutputChannel(
            fastdds::rtps::SendResourceList& sender_resource_list,
            const Locator_t&) override;

    bool OpenInputChannel(
            const Locator_t&,
            fastdds::rtps::TransportReceiverInterface*,
            uint32_t) override;

    bool CloseInputChannel(
            const Locator_t&) override;

    Locator_t RemoteToMainLocal(
            const Locator_t&) const override;

    bool IsLocatorSupported(
            const Locator_t&)  const override;

    bool is_locator_allowed(
            const Locator_t& locator) const override;

    bool is_locator_reachable(
            const Locator_t&) override;

    bool DoInputLocatorsMatch(
            const Locator_t&,
            const Locator_t&) const override;

    LocatorList_t NormalizeLocator(
            const Locator_t& locator) override;

    /**
     * Performs the locator selection algorithm for this transport.
     *
     * It basically consists of the following steps
     *   - selector.transport_starts is called
     *   - transport handles the selection state of each locator
     *   - if a locator from an entry is selected, selector.select is called for that entry
     *
     * In the case of the mock transport all unicast locators are selected.
     *
     * @param [in, out] selector Locator selector.
     */
    void select_locators(
            fastdds::rtps::LocatorSelector& selector) const override;

    bool is_local_locator(
            const Locator_t&) const override
    {
        return false;
    }

    bool is_localhost_allowed() const override
    {
        return false;
    }

    fastdds::rtps::TransportDescriptorInterface* get_configuration() override
    {
        return nullptr;
    }

    void AddDefaultOutputLocator(
            LocatorList_t&) override
    {
    }

    bool getDefaultMetatrafficMulticastLocators(
            LocatorList_t&,
            uint32_t ) const override
    {
        return true;
    }

    bool getDefaultMetatrafficUnicastLocators(
            LocatorList_t&,
            uint32_t ) const override
    {
        return true;
    }

    bool getDefaultUnicastLocators(
            LocatorList_t&,
            uint32_t ) const override
    {
        return true;
    }

    bool fillMetatrafficUnicastLocator(
            Locator_t&,
            uint32_t ) const override
    {
        return true;
    }

    bool fillMetatrafficMulticastLocator(
            Locator_t&,
            uint32_t ) const override
    {
        return true;
    }

    bool configureInitialPeerLocator(
            Locator_t&,
            const fastdds::rtps::PortParameters&,
            uint32_t,
            LocatorList_t& ) const override
    {
        return true;
    }

    bool fillUnicastLocator(
            Locator_t&,
            uint32_t) const override
    {
        return true;
    }

    uint32_t max_recv_buffer_size() const override
    {
        return 0x8FFF;
    }

    bool transform_remote_locator(
            const fastdds::rtps::Locator_t&,
            fastdds::rtps::Locator_t&) const override
    {
        return true;
    }

    bool transform_remote_locator(
            const fastdds::rtps::Locator_t&,
            fastdds::rtps::Locator_t&,
            bool,
            bool) const override
    {
        return true;
    }

    //Helpers and message record
    typedef struct
    {
        Locator_t destination;
        Locator_t origin;
        std::vector<fastdds::rtps::octet> data;
    } MockMessage;

    std::vector<MockMessage> mockMessagesToReceive;
    std::vector<MockMessage> mockMessagesSent;

    // For the mock, port + direction tuples will have a 1:1 relatonship with channels
    typedef uint32_t Port;
    std::vector<Port> mockOpenInputChannels;

    const static int DefaultKind = 1;

    const static int DefaultMaxChannels = 10;
    int mockMaximumChannels;

    //Helper persistent handles
    static std::vector<MockTransport*> mockTransportInstances;
};

class MockTransportDescriptor : public fastdds::rtps::SocketTransportDescriptor
{
public:

    MockTransportDescriptor()
        : SocketTransportDescriptor(0x8FFF, 4)
    {
    }

    int maximumChannels;
    int supportedKind;
    fastdds::rtps::TransportInterface* create_transport() const override
    {
        return new MockTransport(*this);
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef MOCK_TRANSPORT_H
