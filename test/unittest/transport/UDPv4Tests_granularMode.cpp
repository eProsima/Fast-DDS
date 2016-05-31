
#include <fastrtps/transport/GranularUDPv4Transport.h>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <fastrtps/utils/IPFinder.h>
#include <memory>

using namespace std;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace boost::interprocess;

class UDPv4Tests_granularMode: public ::testing::Test 
{
   public:
   UDPv4Tests_granularMode()
   {
      HELPER_SetDescriptorDefaults();
   }

   void HELPER_SetDescriptorDefaults();

   GranularUDPv4Transport::TransportDescriptor descriptor;
   unique_ptr<boost::thread> senderThread;
   unique_ptr<boost::thread> receiverThread;
};

TEST_F(UDPv4Tests_granularMode, locators_match_if_port_AND_address_matches)
{
   // Given
   GranularUDPv4Transport transportUnderTest(descriptor);
   LocatorList_t ips;
   IPFinder::getIP4Address(&ips);

   // We need enough valid IPs for the test
   ASSERT_GE(ips.size(), 2);
   auto it = ips.begin();
   Locator_t locatorAlpha = *(it++);
   Locator_t locatorBeta = locatorAlpha;

   // Then
   ASSERT_TRUE(transportUnderTest.DoLocatorsMatch(locatorAlpha, locatorBeta));

   locatorBeta = *(it++);
   locatorAlpha.port = 5000;
   locatorBeta.port = 5000;

   // Then
   ASSERT_FALSE(transportUnderTest.DoLocatorsMatch(locatorAlpha, locatorBeta));
}

TEST_F(UDPv4Tests_granularMode, opening_and_closing_granular_output_channel)
{
   // Given
   GranularUDPv4Transport transportUnderTest(descriptor);
   LocatorList_t ips;
   IPFinder::getIP4Address(&ips);

   // We need enough valid IPs for the test
   ASSERT_GE(ips.size(), 2);
   auto it = ips.begin();

   Locator_t outputChannelLocator = *(it++);
   outputChannelLocator.port = 7400;
   Locator_t otherLocatorSamePort = *(it++);
   otherLocatorSamePort.port = 7400;

   // Then
   ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(outputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(outputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(outputChannelLocator));

   // Granularity allows for this distinction to be made.
   ASSERT_FALSE  (transportUnderTest.IsOutputChannelOpen(otherLocatorSamePort));

   ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(outputChannelLocator));
   ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(outputChannelLocator));
   ASSERT_FALSE (transportUnderTest.CloseOutputChannel(outputChannelLocator));
}

void UDPv4Tests_granularMode::HELPER_SetDescriptorDefaults()
{
   descriptor.sendBufferSize = 5;
   descriptor.receiveBufferSize = 5;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
