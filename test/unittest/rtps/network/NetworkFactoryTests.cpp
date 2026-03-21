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

#include <vector>

#include <gtest/gtest.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <MockTransport.h>
#include <rtps/network/NetworkFactory.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

class NetworkTests : public ::testing::Test
{
public:

    RTPSParticipantAttributes pattr{};
    NetworkFactory networkFactoryUnderTest{pattr};
    void HELPER_RegisterTransportWithKindAndChannels(
            int kind,
            unsigned int channels);
};

// for uncrustify sake *INDENT-OFF*
TEST_F(NetworkTests, registering_transport_with_descriptor_instantiates_and_populates_a_transport_with_descriptor_options)
// *INDENT-ON*
{
    // Given
    const int ExpectedMaximumChannels = 10;
    const int ExpectedSupportedKind = 3;

    // When
    HELPER_RegisterTransportWithKindAndChannels(ExpectedSupportedKind, ExpectedMaximumChannels);

    // Then
    const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
    ASSERT_EQ(ExpectedMaximumChannels, lastRegisteredTransport->mockMaximumChannels);
    ASSERT_EQ(ExpectedSupportedKind, lastRegisteredTransport->kind());
}

TEST_F(NetworkTests, build_sender_resource_returns_send_resource_for_a_kind_compatible_transport)
{
    // Given
    int ArbitraryKind = 1;
    HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

    eprosima::fastdds::rtps::SendResourceList send_resource_list;

    Locator_t kindCompatibleLocator;
    kindCompatibleLocator.kind = ArbitraryKind;

    // When
    ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, kindCompatibleLocator));

    // Then
    ASSERT_EQ(1u, send_resource_list.size());
}

TEST_F(NetworkTests, BuildReceiverResource_returns_receive_resource_for_a_kind_compatible_transport)
{
    // Given
    int ArbitraryKind = 1;
    HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

    Locator_t kindCompatibleLocator;
    kindCompatibleLocator.kind = ArbitraryKind;

    // When
    std::vector<std::shared_ptr<ReceiverResource>> resources;
    bool ret = networkFactoryUnderTest.BuildReceiverResources(kindCompatibleLocator, resources, 0x8FFF);

    // Then
    ASSERT_TRUE(ret);
    ASSERT_EQ(1u, resources.size());
}

TEST_F(NetworkTests, BuildReceiverResource_returns_multiple_resources_if_multiple_transports_compatible)
{
    // Given
    HELPER_RegisterTransportWithKindAndChannels(3, 10);
    HELPER_RegisterTransportWithKindAndChannels(2, 10);
    HELPER_RegisterTransportWithKindAndChannels(2, 10);

    Locator_t locatorCompatibleWithTwoTransports;
    locatorCompatibleWithTwoTransports.kind = 2;

    // When
    std::vector<std::shared_ptr<ReceiverResource>> resources;
    bool ret = networkFactoryUnderTest.BuildReceiverResources(locatorCompatibleWithTwoTransports, resources, 0x8FFF);

    // Then
    ASSERT_TRUE(ret);
    ASSERT_EQ(2u, resources.size());
}

TEST_F(NetworkTests, build_sender_resource_returns_multiple_resources_if_multiple_transports_compatible)
{
    // Given
    HELPER_RegisterTransportWithKindAndChannels(3, 10);
    HELPER_RegisterTransportWithKindAndChannels(2, 10);
    HELPER_RegisterTransportWithKindAndChannels(2, 10);

    eprosima::fastdds::rtps::SendResourceList send_resource_list;

    Locator_t locatorCompatibleWithTwoTransports;
    locatorCompatibleWithTwoTransports.kind = 2;

    // When
    ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, locatorCompatibleWithTwoTransports));

    // Then
    ASSERT_EQ(1u, send_resource_list.size());
}

/*
   TEST_F(NetworkTests, creating_send_resource_from_locator_opens_channels_mapped_to_that_locator)
   {
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

   Locator_t locator;
   locator.kind = ArbitraryKind;

   // When
   auto resources = networkFactoryUnderTest.BuildSenderResources(locator);

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   ASSERT_TRUE(lastRegisteredTransport->IsOutputChannelOpen(locator));
   }
 */

TEST_F(NetworkTests, creating_receive_resource_from_locator_opens_channels_mapped_to_that_locator)
{
    // Given
    int ArbitraryKind = 1;
    HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

    Locator_t locator;
    locator.kind = ArbitraryKind;

    // When
    std::vector<std::shared_ptr<ReceiverResource>> resources;
    bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, resources, 0x8FFF);

    ASSERT_TRUE(ret);

    // Then
    const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
    ASSERT_TRUE(lastRegisteredTransport->IsInputChannelOpen(locator));
}

/*
   TEST_F(NetworkTests, destroying_a_send_resource_will_close_all_channels_mapped_to_it_on_destruction)
   {
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;

    // TODO review why clear is crashing but end of scope don't.
   { // Destroyed by end of scope but...
      auto resources = networkFactoryUnderTest.BuildSenderResources(locator);
   }
   // When (End of scope)

   //resources.clear(); // Why this was failing?

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   ASSERT_FALSE(lastRegisteredTransport->IsOutputChannelOpen(locator));
   }
 */

TEST_F(NetworkTests, destroying_a_receive_resource_will_close_all_channels_mapped_to_it_on_destruction)
{
    // Given
    int ArbitraryKind = 1;
    HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
    Locator_t locator;
    locator.kind = ArbitraryKind;

    std::vector<std::shared_ptr<ReceiverResource>> resources;
    bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, resources, 0x8FFF);
    ASSERT_TRUE(ret);

    // When
    resources.clear();

    // Then
    const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
    ASSERT_FALSE(lastRegisteredTransport->IsInputChannelOpen(locator));
}

TEST_F(NetworkTests, BuildSenderResources_returns_empty_vector_if_no_registered_transport_is_kind_compatible)
{
    // Given
    MockTransportDescriptor mockTransportDescriptor;
    mockTransportDescriptor.supportedKind = 1;
    mockTransportDescriptor.maximumChannels = 10;
    networkFactoryUnderTest.RegisterTransport<MockTransport, MockTransportDescriptor>(mockTransportDescriptor);

    eprosima::fastdds::rtps::SendResourceList send_resource_list;

    Locator_t locatorOfDifferentKind;
    locatorOfDifferentKind.kind = 2;

    // When
    ASSERT_FALSE(networkFactoryUnderTest.build_send_resources(send_resource_list, locatorOfDifferentKind));

    // Then
    ASSERT_TRUE(send_resource_list.empty());
}

// for uncrustify sake *INDENT-OFF*
TEST_F(NetworkTests, BuildSenderResources_returns_empty_vector_if_all_compatible_transports_have_that_channel_open_already)
// *INDENT-ON*
{
    // Given
    int ArbitraryKind = 1;
    HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
    eprosima::fastdds::rtps::SendResourceList send_resource_list;
    Locator_t locator;
    locator.kind = ArbitraryKind;

    ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, locator));

    ASSERT_EQ(1u, send_resource_list.size());

    // When
    // We do it again for a locator that maps to the same channel
    locator.address[0]++; // Address can differ, since they map to the same port

    ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, locator));

    // Then
    ASSERT_EQ(1u, send_resource_list.size());
}

TEST_F(NetworkTests, A_receiver_resource_accurately_reports_whether_it_supports_a_locator)
{
    // Given
    int ArbitraryKind = 1;
    HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
    Locator_t locator;
    locator.kind = ArbitraryKind;
    std::vector<std::shared_ptr<ReceiverResource>> resources;
    bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, resources, 0x8FFF);
    ASSERT_TRUE(ret);
    auto& resource = resources.back();

    // Then
    ASSERT_TRUE(resource->SupportsLocator(locator));

    // When
    locator.port++;

    // Then
    ASSERT_FALSE(resource->SupportsLocator(locator));
}

/*
   TEST_F(NetworkTests, A_sender_resource_accurately_reports_whether_it_supports_a_locator)
   {
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   eprosima::fastdds::rtps::SendResourceList send_resource_list;
   Locator_t locator;
   locator.kind = ArbitraryKind;
   ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, locator));
   auto& resource = resources.back();

   // Then
   ASSERT_TRUE(resource.SupportsLocator(locator));

   // When
   locator.port++;

   // Then
   ASSERT_TRUE(resource.SupportsLocator(locator));
   }
 */

/*
   TEST_F(NetworkTests, A_Sender_Resource_will_always_send_through_its_original_outbound_locator)
   {
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildSenderResources(locator);
   auto& senderResource = resources.back();

   // When
   const uint32_t testDataLength = 3;
   const char testData[testDataLength] { 'a', 'b', 'c' };
   Locator_t destinationLocator;
   destinationLocator.kind = 1;
   destinationLocator.address[0] = 5;
   senderResource.Send((octet*)testData, testDataLength, destinationLocator);

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   const auto& messageSent = lastRegisteredTransport->mockMessagesSent.back();

   ASSERT_TRUE(0 == memcmp(messageSent.data.data(), testData, testDataLength));
   ASSERT_EQ(messageSent.destination, destinationLocator);
   }
 */

/*
   TEST_F(NetworkTests, A_Receiver_Resource_will_always_receive_through_its_original_inbound_locator_and_from_the_specified_remote_locator)
   {
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   std::vector<std::shared_ptr<ReceiverResource>> resources;
   bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, resources);
   ASSERT_TRUE(ret);
   auto& receiverResource = resources.back();

   vector<octet> testData { 'a', 'b', 'c' };
   Locator_t originLocator;
   originLocator.kind = 1;
   originLocator.get_Address()[0] = 5;
   MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   MockTransport::MockMessage message {locator, originLocator, testData};
   lastRegisteredTransport->mockMessagesToReceive.push_back(message);

   // When
   octet receivedData[65536];
   uint32_t receivedDataSize;

   Locator_t receivedLocator;
   receiverResource->Receive(receivedData, 65536, receivedDataSize, receivedLocator);

   // Then
   ASSERT_EQ(memcmp(receivedData, testData.data(), 3), 0);
   ASSERT_EQ(receivedLocator, originLocator);
   }
 */

void NetworkTests::HELPER_RegisterTransportWithKindAndChannels(
        int kind,
        unsigned int channels)
{
    MockTransportDescriptor mockTransportDescriptor;
    mockTransportDescriptor.supportedKind = kind;
    mockTransportDescriptor.maximumChannels = channels;
    networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);
}

#define SHRINK_TEST_MAX_ENTRIES 4u
#define SHRINK_TEST_MAX_UNICAST_LOCATORS 4u
#define SHRINK_TEST_MAX_MULTICAST_LOCATORS 1u

struct ShrinkLocatorCase_t
{
    ShrinkLocatorCase_t()
    {
    }

    std::string name;
    std::vector<LocatorList_t> input;
    LocatorList_t output;

    std::vector<LocatorSelectorEntry> temp_entries;

    void clear(
            const char* case_name)
    {
        name = case_name;
        input.clear();
        output.clear();
    }

    void prepare_selector(
            LocatorSelector& selector)
    {
        uint32_t id = 1;
        selector.clear();
        temp_entries.clear();
        temp_entries.reserve(input.size());
        for (const LocatorList_t& loc_list : input)
        {
            temp_entries.emplace_back(SHRINK_TEST_MAX_UNICAST_LOCATORS, SHRINK_TEST_MAX_MULTICAST_LOCATORS);
            LocatorSelectorEntry& entry = temp_entries.back();
            entry.remote_guid.entityId = id++;
            for (const Locator_t& loc : loc_list)
            {
                if (IPLocator::isMulticast(loc))
                {
                    entry.multicast.push_back(loc);
                }
                else
                {
                    entry.unicast.push_back(loc);
                }
            }

            selector.add_entry(&entry);
        }
    }

    void perform_selector_test(
            const NetworkFactory& network)
    {
        LocatorSelector selector(ResourceLimitedContainerConfig::fixed_size_configuration(SHRINK_TEST_MAX_ENTRIES));
        prepare_selector(selector);
        selector.reset(true);
        network.select_locators(selector);
        ASSERT_EQ(selector.selected_size(), output.size()) << "on test " << name;
        selector.for_each(
            [this](const Locator_t& locator)
            {
                bool contains = false;
                for (auto locator_in_list : output)
                {
                    if (IsAddressDefined(locator_in_list))
                    {
                        if (locator == locator_in_list)
                        {
                            contains = true;
                            break;
                        }
                    }
                    else
                    {
                        if (locator.kind == locator_in_list.kind && locator.port == locator_in_list.port)
                        {
                            contains = true;
                            break;
                        }
                    }
                }
                ASSERT_TRUE(contains);
            });
    }

};

void add_test_three_lists(
        const char* case_name,
        const LocatorList_t& list1,
        const LocatorList_t& list2,
        const LocatorList_t& list3,
        LocatorList_t result,
        std::vector<ShrinkLocatorCase_t>& cases)
{
    ShrinkLocatorCase_t test;

    test.clear(case_name);
    test.output = result;

    test.input = { list1, list2, list3 };
    cases.push_back(test);

    test.input = { list1, list3, list2 };
    cases.push_back(test);

    test.input = { list2, list1, list3 };
    cases.push_back(test);

    test.input = { list2, list3, list1 };
    cases.push_back(test);

    test.input = { list3, list1, list2 };
    cases.push_back(test);

    test.input = { list3, list2, list1 };
    cases.push_back(test);
}

void fill_blackbox_locators_test_cases(
        std::vector<ShrinkLocatorCase_t>& cases)
{
    ShrinkLocatorCase_t test;

    Locator_t unicast1;
    Locator_t unicast1_2;
    Locator_t unicast2;
    Locator_t unicast3;

    Locator_t multicast1;
    Locator_t multicast2;
    Locator_t multicast3;

    LocatorList_t list1;
    LocatorList_t list2;
    LocatorList_t list3;
    LocatorList_t result;

    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "1.1.1.1", 7400, unicast1);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "1.1.1.2", 7400, unicast1_2);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "1.1.2.1", 7400, unicast2);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "1.1.3.1", 7400, unicast3);

    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "239.255.1.1", 7400, multicast1);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "239.255.1.2", 7400, multicast2);
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "239.255.1.3", 7400, multicast3);

    // Empty input should produce empty output
    test.clear("Empty input");
    cases.push_back(test);

    // Single locator should be output as-is.
    test.clear("Single UDP unicast");
    test.output.push_back(unicast1);
    test.input.push_back(test.output);
    cases.push_back(test);

    // Single locator should be output as-is.
    test.clear("Single UDP multicast");
    test.output.push_back(multicast1);
    test.input.push_back(test.output);
    cases.push_back(test);

    // Single pair should select unicast.
    test.clear("Single UDP list with {multicast, unicast}");
    list1.clear();
    list1.push_back(multicast1);
    list1.push_back(unicast1);
    test.input.push_back(list1);
    test.output.push_back(unicast1);
    cases.push_back(test);

    // Check shrink of only one locator list unicast.
    test.clear("Single UDP list with unicast");
    list1.clear();
    test.output.push_back(unicast1);
    test.output.push_back(unicast1_2);
    test.input.push_back(test.output);
    cases.push_back(test);

    // Check shrink of only one locator list with multicast and unicast.
    test.clear("Single UDP list with multicast and unicast");
    list1.clear();
    list1.push_back(unicast1);
    list1.push_back(multicast1);
    list1.push_back(unicast1_2);
    test.input.push_back(list1);
    test.output.push_back(unicast1);
    test.output.push_back(unicast1_2);
    cases.push_back(test);

    // Two sharing unicast.
    test.clear("Two UDP lists sharing unicast");
    list1.clear();
    list2.clear();
    list1.push_back(unicast1);
    list1.push_back(unicast1_2);
    list2.push_back(unicast1);
    list2.push_back(unicast2);
    test.input.push_back(list1);
    test.input.push_back(list2);
    test.output.push_back(unicast1);
    test.output.push_back(unicast1_2);
    test.output.push_back(unicast2);
    cases.push_back(test);

    // Three. Two use same multicast, the other unicast.
    list1.clear();
    list2.clear();
    list3.clear();
    result.clear();
    list1.push_back(unicast1);
    list1.push_back(multicast1);
    list2.push_back(multicast1);
    list2.push_back(unicast2);
    list3.push_back(unicast3);
    result.push_back(multicast1);
    result.push_back(unicast3);
    add_test_three_lists("Three UDP lists with shared multicast and unicast",
            list1, list2, list3, result, cases);

    // Three. Two use same multicast, the other another multicast.
    list1.clear();
    list2.clear();
    list3.clear();
    result.clear();
    list1.push_back(unicast1);
    list1.push_back(multicast1);
    list2.push_back(multicast1);
    list2.push_back(unicast2);
    list3.push_back(unicast3);
    list3.push_back(multicast3);
    result.push_back(multicast1);
    result.push_back(unicast3);
    add_test_three_lists("Three UDP lists with shared multicast x2",
            list1, list2, list3, result, cases);

    // Three. One uses multicast, the others unicast
    list1.clear();
    list2.clear();
    list3.clear();
    result.clear();
    list1.push_back(unicast1);
    list1.push_back(multicast1);
    list2.push_back(unicast2);
    list3.push_back(unicast3);
    result.push_back(unicast1);
    result.push_back(unicast2);
    result.push_back(unicast3);
    add_test_three_lists("Three UDP lists with only one multicast",
            list1, list2, list3, result, cases);

    // Three using same multicast
    list1.clear();
    list2.clear();
    list3.clear();
    result.clear();
    list1.push_back(multicast2);
    list2.push_back(multicast2);
    list3.push_back(multicast2);
    list3.push_back(unicast3);
    result.push_back(multicast2);
    add_test_three_lists("Three UDP lists with same shared multicast",
            list1, list2, list3, result, cases);
}

TEST_F(NetworkTests, LocatorShrink)
{
    std::vector<ShrinkLocatorCase_t> test_cases;
    fill_blackbox_locators_test_cases(test_cases);

    NetworkFactory f{pattr};
    eprosima::fastdds::rtps::UDPv4TransportDescriptor udpv4;
    f.RegisterTransport(&udpv4);
    // TODO: Register more transports

    for (ShrinkLocatorCase_t& test : test_cases)
    {
        test.perform_selector_test(f);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
