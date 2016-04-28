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
