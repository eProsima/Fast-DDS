#include <fastrtps/transport/UDPv6Transport.h>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <memory>

using namespace std;
using namespace eprosima::fastrtps::rtps;
using namespace boost::interprocess;

	std::string to_IP6_string(Locator_t locator){
		std::stringstream ss;
		ss << std::hex;
      for (int i = 0; i != 14; i+= 2) 
      {
         auto field = (locator.address[i] << 8) + locator.address[i+1];
         ss << field << ":";
      }
      auto field = locator.address[14] + (locator.address[15] << 8);
      ss << field;
      return ss.str();
      }

class UDPv6Tests: public ::testing::Test 
{
   public:
   UDPv6Tests()
   {
      HELPER_SetDescriptorDefaults();
   }

   void HELPER_SetDescriptorDefaults();

   UDPv6Transport::TransportDescriptor descriptor;
   unique_ptr<boost::thread> senderThread;
   unique_ptr<boost::thread> receiverThread;
};

TEST_F(UDPv6Tests, conversion_to_ip6_string)
{
   Locator_t locator;
   locator.kind = LOCATOR_KIND_UDPv6;
   ASSERT_EQ("0:0:0:0:0:0:0:0", locator.to_IP6_string());

   locator.address[0] = 0xff;
   ASSERT_EQ("ff00:0:0:0:0:0:0:0", locator.to_IP6_string());

   locator.address[1] = 0xaa;
   ASSERT_EQ("ffaa:0:0:0:0:0:0:0", locator.to_IP6_string());

   locator.address[2] = 0x0a;
   ASSERT_EQ("ffaa:a00:0:0:0:0:0:0", locator.to_IP6_string());

   locator.address[5] = 0x0c;
   ASSERT_EQ("ffaa:a00:c:0:0:0:0:0", locator.to_IP6_string());
}

TEST_F(UDPv6Tests, setting_ip6_values_on_locators)
{
   Locator_t locator;
   locator.kind = LOCATOR_KIND_UDPv6;

   locator.set_IP6_address(0xffff,0xa, 0xaba, 0, 0, 0, 0, 0);
   ASSERT_EQ("ffff:a:aba:0:0:0:0:0", locator.to_IP6_string());
}

TEST_F(UDPv6Tests, locators_with_kind_2_supported)
{
   // Given
   UDPv6Transport transportUnderTest(descriptor);

   Locator_t supportedLocator;
   supportedLocator.kind = LOCATOR_KIND_UDPv6;
   Locator_t unsupportedLocator;
   unsupportedLocator.kind = LOCATOR_KIND_UDPv4;

   // Then
   ASSERT_TRUE(transportUnderTest.IsLocatorSupported(supportedLocator));
   ASSERT_FALSE(transportUnderTest.IsLocatorSupported(unsupportedLocator));
}

TEST_F(UDPv6Tests, opening_and_closing_output_channel)
{
   // Given
   UDPv6Transport transportUnderTest(descriptor);
   Locator_t genericOutputChannelLocator;
   genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv6;
   genericOutputChannelLocator.port = 7400; // arbitrary

   // Then
   ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.OpenOutputChannel(genericOutputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
   ASSERT_TRUE  (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
   ASSERT_FALSE (transportUnderTest.IsOutputChannelOpen(genericOutputChannelLocator));
   ASSERT_FALSE (transportUnderTest.CloseOutputChannel(genericOutputChannelLocator));
}

TEST_F(UDPv6Tests, opening_and_closing_input_channel)
{
   // Given
   UDPv6Transport transportUnderTest(descriptor);
   Locator_t multicastFilterLocator;
   multicastFilterLocator.kind = LOCATOR_KIND_UDPv6;
   multicastFilterLocator.port = 7410; // arbitrary
   multicastFilterLocator.set_IP6_address(0xff31, 0, 0, 0, 0, 0, 0x8000, 0x1234);

   // Then
   ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
   ASSERT_TRUE  (transportUnderTest.OpenInputChannel(multicastFilterLocator));
   ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
   ASSERT_TRUE  (transportUnderTest.CloseInputChannel(multicastFilterLocator));
   ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
   ASSERT_FALSE (transportUnderTest.CloseInputChannel(multicastFilterLocator));
}


void UDPv6Tests::HELPER_SetDescriptorDefaults()
{
   descriptor.sendBufferSize = 5;
   descriptor.receiveBufferSize = 5;
   descriptor.granularMode = false;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
