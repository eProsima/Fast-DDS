#include <fastrtps/transport/UDPv4Transport.h>
#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <memory>

using namespace std;
using namespace eprosima::fastrtps::rtps;
using namespace boost::interprocess;

class UDPv4Tests: public ::testing::Test 
{
   public:
   UDPv4Tests()
   {
   }

   void HELPER_SetDescriptorDefaults();
   bool HELPER_OpenGenericOutputChannel(UDPv4Transport& transport, uint16_t port);
   bool HELPER_OpenGenericInputChannel(UDPv4Transport& transport, uint16_t port);
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

TEST_F(UDPv4Tests, opening_and_closing_input_channel)
{
   // Given
   HELPER_SetDescriptorDefaults(); 
   UDPv4Transport transportUnderTest(descriptor);
   Locator_t multicastFilterLocator;
   multicastFilterLocator.kind = LOCATOR_KIND_UDPv4;
   multicastFilterLocator.port = 7410; // arbitrary
   multicastFilterLocator.set_IP4_address(239, 255, 0, 1);

   // Then
   ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
   ASSERT_TRUE  (transportUnderTest.OpenInputChannel(multicastFilterLocator));
   ASSERT_TRUE  (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
   ASSERT_TRUE  (transportUnderTest.CloseInputChannel(multicastFilterLocator));
   ASSERT_FALSE (transportUnderTest.IsInputChannelOpen(multicastFilterLocator));
   ASSERT_FALSE (transportUnderTest.CloseInputChannel(multicastFilterLocator));
}

TEST_F(UDPv4Tests, send_and_receive_between_ports)
{
   HELPER_SetDescriptorDefaults(); 
   UDPv4Transport transportUnderTest(descriptor);

   Locator_t multicastLocator;
   multicastLocator.port = 7410;
   multicastLocator.kind = LOCATOR_KIND_UDPv4;
   multicastLocator.set_IP4_address(239, 255, 0, 1);

   Locator_t outputChannelLocator;
   outputChannelLocator.port = 7400;
   outputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   ASSERT_TRUE(transportUnderTest.OpenOutputChannel(outputChannelLocator)); // Includes loopback
   ASSERT_TRUE(transportUnderTest.OpenInputChannel(multicastLocator));
   vector<char> message = { 'H','e','l','l','o' };

   auto sendThreadFunction = [&]()
   {
      Locator_t destinationLocator;
      destinationLocator.port = 7410;
      destinationLocator.kind = LOCATOR_KIND_UDPv4;
      EXPECT_TRUE(transportUnderTest.Send(message, outputChannelLocator, multicastLocator));
   };

   auto receiveThreadFunction = [&]() 
   {
      vector<char> receiveBuffer(descriptor.receiveBufferSize);
      Locator_t remoteLocatorToReceive;
      EXPECT_TRUE(transportUnderTest.Receive(receiveBuffer, multicastLocator, remoteLocatorToReceive));
      EXPECT_EQ(message, receiveBuffer);
   };

   receiverThread.reset(new boost::thread(receiveThreadFunction));      
   senderThread.reset(new boost::thread(sendThreadFunction));      
   senderThread->join();
   receiverThread->join();
}

void UDPv4Tests::HELPER_SetDescriptorDefaults()
{
   descriptor.sendBufferSize = 5;
   descriptor.receiveBufferSize = 5;
   descriptor.granularMode = false;
}

bool UDPv4Tests::HELPER_OpenGenericOutputChannel(UDPv4Transport& transport, uint16_t port)
{
   Locator_t genericOutputChannelLocator;
   genericOutputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   genericOutputChannelLocator.port = port;
   return transport.OpenInputChannel(genericOutputChannelLocator);
}

bool UDPv4Tests::HELPER_OpenGenericInputChannel(UDPv4Transport& transport, uint16_t port)
{
   Locator_t genericInputChannelLocator;
   genericInputChannelLocator.kind = LOCATOR_KIND_UDPv4;
   genericInputChannelLocator.port = port;
   return transport.OpenInputChannel(genericInputChannelLocator);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
