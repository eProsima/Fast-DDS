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
#include <MockTransport.h>
#include <gtest/gtest.h>
#include <vector>

using namespace std;
using namespace eprosima::fastrtps::rtps;

class NetworkTests: public ::testing::Test
{
   public:
   NetworkFactory networkFactoryUnderTest;
   void HELPER_RegisterTransportWithKindAndChannels(int kind, unsigned int channels);
};

TEST_F(NetworkTests, registering_transport_with_descriptor_instantiates_and_populates_a_transport_with_descriptor_options)
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

   SendResourceList send_resource_list;

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
   bool ret = networkFactoryUnderTest.BuildReceiverResources(kindCompatibleLocator, 0x8FFF, resources);

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
   bool ret = networkFactoryUnderTest.BuildReceiverResources(locatorCompatibleWithTwoTransports, 0x8FFF, resources);

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

   SendResourceList send_resource_list;

   Locator_t locatorCompatibleWithTwoTransports;
   locatorCompatibleWithTwoTransports.kind = 2;

   // When
   ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, locatorCompatibleWithTwoTransports));

   // Then
   ASSERT_EQ(2u, send_resource_list.size());
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
   bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, 0x8FFF, resources);

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
   bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, 0x8FFF, resources);
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

   SendResourceList send_resource_list;

   Locator_t locatorOfDifferentKind;
   locatorOfDifferentKind.kind = 2;

   // When
   ASSERT_TRUE(networkFactoryUnderTest.build_send_resources(send_resource_list, locatorOfDifferentKind));

   // Then
   ASSERT_TRUE(send_resource_list.empty());
}

TEST_F(NetworkTests, BuildSenderResources_returns_empty_vector_if_all_compatible_transports_have_that_channel_open_already)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   SendResourceList send_resource_list;
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
   bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, 0x8FFF, resources);
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
   SendResourceList send_resource_list;
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
   bool ret = networkFactoryUnderTest.BuildReceiverResources(locator, 0x8FFF, resources);
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

void NetworkTests::HELPER_RegisterTransportWithKindAndChannels(int kind, unsigned int channels)
{
   MockTransportDescriptor mockTransportDescriptor;
   mockTransportDescriptor.supportedKind = kind;
   mockTransportDescriptor.maximumChannels = channels;
   networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
