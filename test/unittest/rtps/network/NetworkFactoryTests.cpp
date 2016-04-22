#include <fastrtps/rtps/network/NetworkFactory.h>
#include <MockTransport.h>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;

class NetworkTests: public ::testing::Test 
{
   public:
   NetworkFactory networkFactoryUnderTest;
};

TEST_F(NetworkTests, registering_transport_with_descriptor_instantiates_and_populates_a_transport_with_descriptor_options)
{
   // Given
   const int ExpectedMaximumChannels = 10;
   const int ExpectedSupportedKind = 3;
   MockTransport::TransportDescriptor mockTransportDescriptor;
   mockTransportDescriptor.maximumChannels = ExpectedMaximumChannels;
   mockTransportDescriptor.supportedKind = ExpectedSupportedKind;

   // When
   networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);

   // Then
   const MockTransport* lastRegisteredTransport = MockTransport::mockTransportInstances.back();
   ASSERT_EQ(ExpectedMaximumChannels, lastRegisteredTransport->mockMaximumChannels);
   ASSERT_EQ(ExpectedSupportedKind, lastRegisteredTransport->mockSupportedKind);
}

TEST_F(NetworkTests, BuildSenderResource_returns_send_resource_for_a_kind_compatible_transport)
{
   // Given
   MockTransport::TransportDescriptor mockTransportDescriptor;
   mockTransportDescriptor.supportedKind = 1;
   mockTransportDescriptor.maximumChannels = 10;
   networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);

   Locator_t kindCompatibleLocator;
   kindCompatibleLocator.kind = 1;

   auto resources = networkFactoryUnderTest.BuildSenderResources(kindCompatibleLocator);

   // Then
   ASSERT_EQ(1, resources.size());
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


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
