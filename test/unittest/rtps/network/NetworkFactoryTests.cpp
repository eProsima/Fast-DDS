#include <fastrtps/rtps/network/NetworkFactory.h>
#include <MockTransport.h>
#include <gtest/gtest.h>

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
   ASSERT_TRUE(lastRegisteredTransport->IsLocatorChannelOpen(locator));
}

TEST_F(NetworkTests, destroying_a_resource_will_close_all_channels_mapped_to_it_on_destruction)
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
   ASSERT_FALSE(lastRegisteredTransport->IsLocatorChannelOpen(locator));
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
