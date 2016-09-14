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

#include <fastrtps/rtps/network/SenderResource.h>

using namespace std;
namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, Locator_t& locator)
{
   // Internal channel is opened and assigned to this resource.
   mValid = transport.OpenOutputChannel(locator);
   if (!mValid)
      return; // Invalid resource, will be discarded by the factory.

   // Implementation functions are bound to the right transport parameters
   Cleanup = [&transport,locator](){ transport.CloseOutputChannel(locator); };
   SendThroughAssociatedChannel = [&transport, locator](const octet* data, uint32_t dataSize, const Locator_t& destination)-> bool
                                  { return transport.Send(data,dataSize, locator, destination); };
   LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
   ManagedChannelMapsToRemote = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, transport.RemoteToMainLocal(locatorToCheck)); };
}

bool SenderResource::Send(const octet* data, uint32_t dataLength, const Locator_t& destinationLocator)
{
   if (SendThroughAssociatedChannel)
      return SendThroughAssociatedChannel(data, dataLength, destinationLocator);
   return false;
}

SenderResource::SenderResource(SenderResource&& rValueResource)
{
    mValid = rValueResource.mValid;
    Cleanup.swap(rValueResource.Cleanup); 
    SendThroughAssociatedChannel.swap(rValueResource.SendThroughAssociatedChannel);
    LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
    ManagedChannelMapsToRemote.swap(rValueResource.ManagedChannelMapsToRemote);
}

bool SenderResource::SupportsLocator(const Locator_t& local)
{
   if (LocatorMapsToManagedChannel)
      return LocatorMapsToManagedChannel(local);
   return false;
}

bool SenderResource::CanSendToRemoteLocator(const Locator_t& remote)
{
   if (ManagedChannelMapsToRemote)
      return ManagedChannelMapsToRemote(remote);
   return false;
}

SenderResource::~SenderResource()
{
   if (Cleanup)
      Cleanup();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
