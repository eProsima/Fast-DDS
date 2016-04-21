#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include "TransportChannel.h"

class TransportInterface
{
public:
   virtual ~TransportInterface(){};

   virtual bool AreLocatorChannelsOpen(Locator_t) const;
   virtual bool IsLocatorSupported(Locator_t) const;
   virtual bool OpenLocatorChannels(Locator_t);
   virtual bool CloseLocatorChannels(Locator_t);

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress);
   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t remoteAddress);
}

#endif
