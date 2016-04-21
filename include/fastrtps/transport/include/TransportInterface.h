#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include "TransportChannel.h"
#include <vector>

class TransportInterface
{
public:
   virtual ~TransportInterface(){};

   virtual bool AreLocatorChannelsOpen(Locator_t) const = 0;
   virtual bool IsLocatorSupported(Locator_t)     const = 0;
   virtual bool OpenLocatorChannels(Locator_t)          = 0;
   virtual bool CloseLocatorChannels(Locator_t)         = 0;

   virtual bool Send(const std::vector<char>& sendBuffer, Locator_t localChannel, Locator_t remoteAddress) = 0;
   virtual bool Receive(std::vector<char>& receiveBuffer, Locator_t localChannel, Locator_t remoteAddress) = 0;
};

#endif
