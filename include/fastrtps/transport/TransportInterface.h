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

#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <memory>
#include <vector>
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/common/Guid.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const uint32_t s_maximumMessageSize = 65500;
static const uint32_t s_minimumSocketBuffer = 65536;
static const uint8_t s_defaultTTL = 1;

struct TransportDescriptorInterface;
class ReceiverResource;
class SenderResource;
class SocketInfo;

/**
 * Interface against which to implement a transport layer, decoupled from FastRTPS internals.
 * TransportInterface expects the user to implement a logical equivalence between Locators and protocol-specific "channels".
 * This equivalence can be narrowing: For instance in UDP/IP, a port would take the role of channel, and several different
 * locators can map to the same port, and hence the same channel.
 * @ingroup TRANSPORT_MODULE
 * */
class TransportInterface
{
public:

   /**
    * Aside from the API defined here, an user-defined Transport must define a descriptor data type and a constructor that
    * expects a constant reference to such descriptor. e.g:
    *
    * class MyTransport:
    * public:
    *    typedef struct { ... } MyTransportDescriptor;
    *    MyTransport(const MyTransportDescriptor&);
    *    ...
    */
   virtual ~TransportInterface() = default;

   virtual bool init() = 0;

   /**
    * Must report whether the output channel associated to this locator is open. Channels must either be
    * fully closed or fully open, so that "open" and "close" operations are whole and definitive.
    */
   virtual bool IsOutputChannelOpen(const Locator_t&) const = 0;

   /**
    * Must report whether the input channel associated to this locator is open. Channels must either be
    * fully closed or fully open, so that "open" and "close" operations are whole and definitive.
    */
   virtual bool IsInputChannelOpen(const Locator_t&) const = 0;

   //! Must report whether the given locator is supported by this transport (typically inspecting its "kind" value).
   virtual bool IsLocatorSupported(const Locator_t&) const = 0;

   //! Returns the locator describing the main (most general) channel that can write to the provided remote locator.
   virtual Locator_t RemoteToMainLocal(const Locator_t& remote) const = 0;

   //! Must open the channel that maps to/from the given locator. This method must allocate, reserve and mark
   //! any resources that are needed for said channel.
   virtual bool OpenOutputChannel(Locator_t&, SenderResource* senderResource) = 0;
   virtual bool OpenInputChannel(const Locator_t&, ReceiverResource*) = 0;

   /**
    * Must close the channel that maps to/from the given locator.
    * IMPORTANT: It MUST be safe to call this method even during a Send operation on another thread. You must implement
    * any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage.
    */
   virtual bool CloseOutputChannel(const Locator_t&) = 0;

   /**
    * Must close the channel that maps to/from the given locator.
    * IMPORTANT: It MUST be safe to call this method even during a Receive operation on another thread. You must implement
    * any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage.
    */
   virtual bool CloseInputChannel(const Locator_t&) = 0;

   /**
    * Must release the channel that maps to/from the given locator.
    * IMPORTANT: It MUST be safe to call this method even during a Receive operation on another thread. You must implement
    * any necessary mutual exclusion and timeout mechanisms to make sure the channel can be closed without damage.
    */
   virtual bool ReleaseInputChannel(const Locator_t&) = 0;

   //! Must report whether two locators map to the same internal channel.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const = 0;
  /**
   * Must execute a blocking send, through the outbound channel that maps to the localLocator, targeted to the
   * remote address defined by remoteLocator. Must be threadsafe between channels, but not necessarily
   * within the same channel.
   */
   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator) = 0;

   /**
    * Must execute a blocking receive, on the inbound channel that maps to the localLocator, receiving from the
    * address that gets written to remoteLocator. Must be threadsafe between channels, but not necessarily
    * within the same channel.
    */
   virtual bool Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
       SocketInfo* socketInfo, Locator_t& remoteLocator) = 0;

   virtual void SetParticipantGUIDPrefix(const GuidPrefix_t& prefix) = 0;

   virtual LocatorList_t NormalizeLocator(const Locator_t& locator) = 0;

   virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) = 0;

   virtual bool is_local_locator(const Locator_t& locator) const = 0;

   virtual TransportDescriptorInterface* get_configuration() = 0;

   virtual void AddDefaultLocator(LocatorList_t &defaultList) = 0;
};

/**
 * Virtual base class for the data type used to define transport configuration.
 * @ingroup RTPS_MODULE
 * */
struct TransportDescriptorInterface
{
    TransportDescriptorInterface(uint32_t maximumMessageSize) : maxMessageSize(maximumMessageSize)
        , sendBufferSize(0)
        , receiveBufferSize(0)
        , TTL(s_defaultTTL)
    {}

    TransportDescriptorInterface(const TransportDescriptorInterface& t) : maxMessageSize(t.maxMessageSize)
        , sendBufferSize(t.sendBufferSize)
        , receiveBufferSize(t.receiveBufferSize)
        , TTL(t.TTL)
        , rtpsParticipantGuidPrefix(t.rtpsParticipantGuidPrefix)
    {}

    virtual ~TransportDescriptorInterface(){}

    virtual TransportInterface* create_transport() const = 0;

    uint32_t maxMessageSize;

    //! Length of the send buffer.
    uint32_t sendBufferSize;
    //! Length of the receive buffer.
    uint32_t receiveBufferSize;
    //! Allowed interfaces in an IP string format.
    std::vector<std::string> interfaceWhiteList;
    //! Specified time to live (8bit - 255 max TTL)
    uint8_t TTL;
    //! Participant GUID prefix.
    GuidPrefix_t rtpsParticipantGuidPrefix;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
