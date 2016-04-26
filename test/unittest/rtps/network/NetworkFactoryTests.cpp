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
   ASSERT_EQ(ExpectedSupportedKind, lastRegisteredTransport->mockSupportedKind);
}

TEST_F(NetworkTests, BuildSenderResource_returns_send_resource_for_a_kind_compatible_transport)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

   Locator_t kindCompatibleLocator;
   kindCompatibleLocator.kind = ArbitraryKind;

   // When
   auto resources = networkFactoryUnderTest.BuildSenderResources(kindCompatibleLocator);

   // Then
   ASSERT_EQ(1, resources.size());
}

TEST_F(NetworkTests, BuildReceiverResource_returns_receive_resource_for_a_kind_compatible_transport)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

   Locator_t kindCompatibleLocator;
   kindCompatibleLocator.kind = ArbitraryKind;

   // When
   auto resources = networkFactoryUnderTest.BuildReceiverResources(kindCompatibleLocator);

   // Then
   ASSERT_EQ(1, resources.size());
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
   auto resources = networkFactoryUnderTest.BuildReceiverResources(locatorCompatibleWithTwoTransports);

   // Then
   ASSERT_EQ(2, resources.size());
}

TEST_F(NetworkTests, BuildSenderResource_returns_multiple_resources_if_multiple_transports_compatible)
{
   // Given
   HELPER_RegisterTransportWithKindAndChannels(3, 10);
   HELPER_RegisterTransportWithKindAndChannels(2, 10);
   HELPER_RegisterTransportWithKindAndChannels(2, 10);

   Locator_t locatorCompatibleWithTwoTransports;
   locatorCompatibleWithTwoTransports.kind = 2;

   // When
   auto resources = networkFactoryUnderTest.BuildSenderResources(locatorCompatibleWithTwoTransports);

   // Then
   ASSERT_EQ(2, resources.size());
}

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

TEST_F(NetworkTests, creating_receive_resource_from_locator_opens_channels_mapped_to_that_locator)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);

   Locator_t locator;
   locator.kind = ArbitraryKind;

   // When
   auto resources = networkFactoryUnderTest.BuildReceiverResources(locator);

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   ASSERT_TRUE(lastRegisteredTransport->IsInputChannelOpen(locator));
}

TEST_F(NetworkTests, destroying_a_send_resource_will_close_all_channels_mapped_to_it_on_destruction)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildSenderResources(locator);

   // When
   resources.clear();

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   ASSERT_FALSE(lastRegisteredTransport->IsOutputChannelOpen(locator));
}

TEST_F(NetworkTests, destroying_a_receive_resource_will_close_all_channels_mapped_to_it_on_destruction)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildReceiverResources(locator);

   // When
   resources.clear();

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   ASSERT_FALSE(lastRegisteredTransport->IsInputChannelOpen(locator));
}

TEST_F(NetworkTests, BuildSenderResources_returns_empty_vector_if_no_registered_transport_is_kind_compatible)
{
   // Given
   MockTransport::TransportDescriptor mockTransportDescriptor;
   mockTransportDescriptor.supportedKind = 1;
   mockTransportDescriptor.maximumChannels = 10;
   networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);

   Locator_t locatorOfDifferentKind;
   locatorOfDifferentKind.kind = 2;

   // When
   auto resources = networkFactoryUnderTest.BuildSenderResources(locatorOfDifferentKind);

   // Then
   ASSERT_TRUE(resources.empty());
}

TEST_F(NetworkTests, BuildSenderResources_returns_empty_vector_if_all_compatible_transports_have_that_channel_open_already)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildSenderResources(locator);

   // When
   // We do it again for a locator that maps to the same channel
   locator.address[0]++; // Address can differ, since they map to the same port
   auto secondBatchResources = networkFactoryUnderTest.BuildSenderResources(locator);

   // Then
   ASSERT_TRUE(secondBatchResources.empty());
}

TEST_F(NetworkTests, A_receiver_resource_accurately_reports_whether_it_supports_a_locator)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildReceiverResources(locator);
   auto& resource = resources.back();

   // Then
   ASSERT_TRUE(resource.SupportsLocator(locator));

   // When
   locator.port++;

   // Then
   ASSERT_FALSE(resource.SupportsLocator(locator));
}

TEST_F(NetworkTests, A_sender_resource_accurately_reports_whether_it_supports_a_locator)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildSenderResources(locator);
   auto& resource = resources.back();

   // Then
   ASSERT_TRUE(resource.SupportsLocator(locator));

   // When
   locator.port++;

   // Then
   ASSERT_FALSE(resource.SupportsLocator(locator));
}

TEST_F(NetworkTests, A_Sender_Resource_will_always_send_through_its_original_outbound_locator_and_to_the_specified_remote_locator)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildSenderResources(locator);
   auto& senderResource = resources.back();

   // When
   vector<char> testData { 'a', 'b', 'c' };
   Locator_t destinationLocator;
   destinationLocator.kind = 1;
   destinationLocator.address[0] = 5;
   senderResource.Send(testData, destinationLocator);

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   const auto& messageSent = lastRegisteredTransport->mockMessagesSent.back();

   ASSERT_EQ(messageSent.data, testData);
   ASSERT_EQ(messageSent.origin, locator);
   ASSERT_EQ(messageSent.destination, destinationLocator);
}

TEST_F(NetworkTests, A_Receiver_Resource_will_always_receive_through_its_original_inbound_locator_and_from_the_specified_remote_locator)
{
   // Given
   int ArbitraryKind = 1;
   HELPER_RegisterTransportWithKindAndChannels(ArbitraryKind, 10);
   Locator_t locator;
   locator.kind = ArbitraryKind;
   auto resources = networkFactoryUnderTest.BuildReceiverResources(locator);
   auto& receiverResource = resources.back();

   // When
   Locator_t originLocator;
   originLocator.kind = 1;
   originLocator.address[0] = 5;

   // We fabricate a message to be received from the mock.
   vector<char> testData { 'a', 'b', 'c' };
   MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   MockTransport::MockMessage message {originLocator, locator, testData}; 

   lastRegisteredTransport->mockMessagesToReceive.push_back(message);
   vector<char> receivedData;
   receiverResource.Receive(receivedData, originLocator);

   // Then
   ASSERT_EQ(receivedData, testData);
}

void NetworkTests::HELPER_RegisterTransportWithKindAndChannels(int kind, unsigned int channels)
{
   MockTransport::TransportDescriptor mockTransportDescriptor;
   mockTransportDescriptor.supportedKind = kind;
   mockTransportDescriptor.maximumChannels = channels;
   networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
