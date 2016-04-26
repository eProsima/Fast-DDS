#include <fastrtps/transport/UDPv4Transport.h>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <memory>

using namespace std;
using namespace eprosima::fastrtps::rtps;

class UDPv4Tests: public ::testing::Test 
{
   public:
   UDPv4Tests()
   {
   }

   void HELPER_SetDescriptorDefaults();
   bool HELPER_OpenGenericOutputChannel(UDPv4Transport& transport);
   void HELPER_StartSenderMulticastOutput();

   UDPv4Transport::TransportDescriptor descriptor;
   unique_ptr<boost::thread> senderThread;
   unique_ptr<boost::thread> receiverThread;
};

TEST_F(UDPv4Tests, locators_with_kind_1_supported)
{
   // Given
   HELPER_SetDescriptorDefaults(); 
   UDPv4Transport transportUnderTest(descriptor);

   Locator_t supportedLocator;
   supportedLocator.kind = LOCATOR_KIND_UDPv4;
   Locator_t unsupportedLocator;
   unsupportedLocator.kind = LOCATOR_KIND_UDPv6;

   // Then
   ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
   ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocator));
}

TEST_F(UDPv4Tests, opening_and_closing_output_channel)
{
   // Given
   HELPER_SetDescriptorDefaults(); 
   UDPv4Transport transportUnderTest(descriptor);
   Locator_t genericOutputChannelLocator;
   genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   genericOutputChannelLocator.port = 7400; // arbitrary

   // Then
   ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
   ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
   ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
}

void UDPv4Tests::HELPER_SetDescriptorDefaults()
{
   descriptor.sendBufferSize = 1024;
   descriptor.receiveBufferSize = 1024;
   descriptor.granularMode = false;
}

bool UDPv4Tests::HELPER_OpenGenericOutputChannel(UDPv4Transport& transport)
{
   Locator_t genericOutputChannelLocator;
   genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   genericOutputChannelLocator.port = 7400; // arbitrary
   return transport.OpenOutputChannel(genericOutputChannelLocator);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
