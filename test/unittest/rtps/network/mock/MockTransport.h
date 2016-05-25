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
   virtual bool IsOutputChannelOpen(const Locator_t&) const;
   virtual bool IsInputChannelOpen(const Locator_t&)  const;

   virtual bool OpenOutputChannel(const Locator_t&); 
   virtual bool OpenInputChannel(const Locator_t&); 

   virtual bool CloseOutputChannel(const Locator_t&);
   virtual bool CloseInputChannel(const Locator_t&);

   virtual Locator_t RemoteToMainLocal(const Locator_t&) const;

   virtual bool IsLocatorSupported(const Locator_t&)  const;
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const;

   virtual bool Send(const std::vector<char>& sendBuffer, const Locator_t& localChannel, const Locator_t& remoteAddress);
   virtual bool Receive(std::vector<char>& receiveBuffer, const Locator_t& localChannel, Locator_t& remoteAddress);

   //Helpers and message record
   typedef struct
   {
      Locator_t destination;
      Locator_t origin;
      std::vector<char> data;
   } MockMessage;

   std::vector<MockMessage> mockMessagesToReceive;
   std::vector<MockMessage> mockMessagesSent;

   // For the mock, port + direction tuples will have a 1:1 relatonship with channels
   
   typedef uint16_t Port;
   std::vector<Port> mockOpenOutputChannels;
   std::vector<Port> mockOpenInputChannels;

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
