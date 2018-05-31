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
#include <fastrtps/rtps/messages/MessageReceiver.h>

using namespace std;
namespace eprosima{
namespace fastrtps{
namespace rtps{

SenderResource::SenderResource(TransportInterface& transport, Locator_t& locator) : m_pSocketInfo(nullptr)
{
    // Internal channel is opened and assigned to this resource.
    mValid = transport.OpenOutputChannel(locator, this);
    if (!mValid)
        return; // Invalid resource, will be discarded by the factory.

    // Implementation functions are bound to the right transport parameters
    Cleanup = [&transport, locator, this]()
        {
            transport.CloseOutputChannel(locator);
            this->m_pSocketInfo = nullptr;
        };

    AddSenderLocatorToManagedChannel = [&transport, this](Locator_t& destination)->bool
        { return transport.OpenExtraOutputChannel(destination, this); };

    SendThroughAssociatedChannel =
        [&transport, locator, this]
        (const octet* data, uint32_t dataSize, const Locator_t& destination, SocketInfo* socketInfo)-> bool
        {
            if (true || socketInfo == nullptr)
            {
                return transport.Send(data, dataSize, locator, destination);
            }
            else
            {
                return transport.Send(data, dataSize, locator, destination, socketInfo);
            }
        };
    LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, locatorToCheck); };
    ManagedChannelMapsToRemote = [&transport, locator](const Locator_t& locatorToCheck) -> bool
                                 { return transport.DoLocatorsMatch(locator, transport.RemoteToMainLocal(locatorToCheck)); };
}

bool SenderResource::AddSenderLocator(Locator_t& destination)
{
    if (AddSenderLocatorToManagedChannel)
        return AddSenderLocatorToManagedChannel(destination);
    return false;
}

bool SenderResource::Send(const octet* data, uint32_t dataLength, const Locator_t& destinationLocator)
{
   if (SendThroughAssociatedChannel)
      return SendThroughAssociatedChannel(data, dataLength, destinationLocator, this->m_pSocketInfo);
   return false;
}

SenderResource::SenderResource(SenderResource&& rValueResource)
{
    mValid = rValueResource.mValid;
    Cleanup.swap(rValueResource.Cleanup);
    AddSenderLocatorToManagedChannel.swap(rValueResource.AddSenderLocatorToManagedChannel);
    SendThroughAssociatedChannel.swap(rValueResource.SendThroughAssociatedChannel);
    LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
    ManagedChannelMapsToRemote.swap(rValueResource.ManagedChannelMapsToRemote);
    m_pSocketInfo = rValueResource.m_pSocketInfo;
    rValueResource.m_pSocketInfo = nullptr;
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
