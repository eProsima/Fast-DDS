#ifndef MOCK_TRANSPORT_H
#define MOCK_TRANSPORT_H

#include "TransportInterface.h"
#include <pair>

class MockTransport: public TransportInterface
{
   public:

   //API implementation
   virtual bool AreLocatorChannelsOpen(Locator_t) const;
   virtual bool IsLocatorSupported(Locator_t) const;
   virtual bool OpenLocatorChannels(Locator_t);
   virtual bool CloseLocatorChannels(Locator_t);

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress);
   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t remoteAddress);

   typedef struct TransportDescriptor 
   {
      int maximumChannels;
   };

   //Helpers and message record
   typedef std::pair<Locator_t, std::vector<char> > LocatorMessagePair;
   void AppendMockMessageToReceive(LocatorMessagePair);
   std::vector<LocatorMessagePair> mockMessagesToReceive;
   std::vector<LocatorMessagePair> recordMessagesSent;

   int mockSupportedKind;

   //Channel helpers
   typedef int MockChannel;
   std::vector<MockChannel> mockOpenChannels;
};



