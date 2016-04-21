#include <fastrtps/rtps/network/NetworkFactory.h>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;

class NetworkTests: public ::testing::Test 
{
   public:
   NetworkFactory networkFactoryUnderTest;
   MockTransport mockTransport;
   MockTransport::TransportDescriptor mockTransportDescriptor;

   NetworkTests()
   {
      mockTransportDescriptor.maximumChannels = 10;
      networkFactoryUnderTest.RegisterTransport<MockTransport>(mockTransportDescriptor);
   }
}

TEST_F(NetworkTests, trivial_test)
{
   ASSERT_TRUE(false);
}
