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
   virtual bool AreLocatorChannelsOpen(Locator_t) const;
   virtual bool IsLocatorSupported(Locator_t) const;
   virtual bool OpenLocatorChannels(Locator_t) { return false; };
   virtual bool CloseLocatorChannels(Locator_t) { return false; };

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress) { return false; };

   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t remoteAddress) { return false; };

   //Helpers and message record
   typedef std::pair<Locator_t, std::vector<char> > LocatorMessagePair;
   void AppendMockMessageToReceive(LocatorMessagePair);
   std::vector<LocatorMessagePair> mockMessagesToReceive;
   std::vector<LocatorMessagePair> recordMessagesSent;

   const static int DefaultKind = 1;
   int mockSupportedKind;

   const static int DefaultMaxChannels = 10;
   int mockMaximumChannels;

   //Channel helpers
   typedef int MockChannel;
   std::vector<MockChannel> mockOpenChannels;

   //Helper persistent handles
   static std::vector<MockTransport*> mockTransportInstances;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
