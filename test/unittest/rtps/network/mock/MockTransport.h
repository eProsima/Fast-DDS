#ifndef MOCK_TRANSPORT_H
#define MOCK_TRANSPORT_H

#include <fastrtps/transport/TransportInterface.h>
#include <utility>
#include <vector>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class MockTransport: public TransportInterface
{
   public:
   typedef struct  
   {
      int maximumChannels;
      int supportedKind;
   } TransportDescriptor;

   MockTransport(const TransportDescriptor& descriptor);
   MockTransport();
  ~MockTransport();

   //API implementation
   virtual bool IsLocatorChannelOpen(Locator_t) const;
   virtual bool IsLocatorSupported(Locator_t) const;
   virtual bool OpenLocatorChannel(Locator_t); 
   virtual bool CloseLocatorChannel(Locator_t);

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress);

   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t remoteAddress) { return false; };

   //Helpers and message record
   typedef struct
   {
      Locator_t origin;
      Locator_t destination;
      std::vector<char> data;
   } MockMessage;

   void AppendMockMessageToReceive(MockMessage);
   std::vector<MockMessage> mockMessagesToReceive;
   std::vector<MockMessage> mockMessagesSent;

   // For the mock, ports will have a 1:1 relatonship with channels
   typedef uint32_t Channel;
   std::vector<Channel> mockOpenChannels;


   const static int DefaultKind = 1;
   int mockSupportedKind;

   const static int DefaultMaxChannels = 10;
   int mockMaximumChannels;

   //Helper persistent handles
   static std::vector<MockTransport*> mockTransportInstances;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
