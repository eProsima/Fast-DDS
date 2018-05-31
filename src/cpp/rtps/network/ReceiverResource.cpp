// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fastrtps/rtps/network/ReceiverResource.h>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

ReceiverResource::ReceiverResource(TransportInterface& transport, const Locator_t& locator)
{
   // Internal channel is opened and assigned to this resource.
   mValid = transport.OpenInputChannel(locator);
   if (!mValid)
      return; // Invalid resource to be discarded by the factory.

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.ReleaseInputChannel(locator); };
   Close = [&transport,locator](){ transport.CloseInputChannel(locator); };
   ReceiveFromAssociatedChannel = [&transport, locator](octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize, Locator_t& origin)-> bool
                                  { return transport.Receive(receiveBuffer, receiveBufferCapacity, receiveBufferSize, locator, origin); };
   LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
}

bool ReceiverResource::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
             Locator_t& originLocator)
{
   if (ReceiveFromAssociatedChannel)
   {
      return ReceiveFromAssociatedChannel(receiveBuffer, receiveBufferCapacity, receiveBufferSize, originLocator);
   }

   return false;
}

ReceiverResource::ReceiverResource(ReceiverResource&& rValueResource)
{
   Cleanup.swap(rValueResource.Cleanup);
   Close.swap(rValueResource.Close);
   ReceiveFromAssociatedChannel.swap(rValueResource.ReceiveFromAssociatedChannel);
   LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
}

bool ReceiverResource::SupportsLocator(const Locator_t& localLocator)
{
   if (LocatorMapsToManagedChannel)
      return LocatorMapsToManagedChannel(localLocator);
   return false;
}

void ReceiverResource::Abort()
{
   if(Cleanup)
   {
      Cleanup();
   }
}

ReceiverResource::~ReceiverResource()
{
   if(Cleanup)
   {
      Cleanup();
   }

   if(Close)
   {
       Close();
   }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
