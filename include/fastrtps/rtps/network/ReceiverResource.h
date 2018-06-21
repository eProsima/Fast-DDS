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

#ifndef RECEIVER_RESOURCE_H
#define RECEIVER_RESOURCE_H

#include <functional>
#include <vector>
#include <memory>
#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/rtps/Endpoint.h>
#include "../common/all_common.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSWriter;
class RTPSReader;
class MessageReceiver;
class RTPSParticipantImpl;

/**
 * RAII object that encapsulates the Receive operation over one chanel in an unknown transport.
 * A Receiver resource is always univocally associated to a transport channel; the
 * act of constructing a Receiver Resource opens the channel and its destruction
 * closes it.
 * @ingroup NETWORK_MODULE
 */
class ReceiverResource
{
   //! Only NetworkFactory is ever allowed to construct a ReceiverResource from scratch.
   //! In doing so, it guarantees the transport and channel are in a valid state for
   //! this resource to exist.
friend class NetworkFactory;

public:
  /**
   * Performs a blocking receive through the channel managed by this resource,
   * notifying about the origin locator.
   * @param receiveBuffer Pointer to a buffer where to store the received message.
   * @param receiveBufferCapacity Capacity of the reception buffer.
   * Will be used as a boundary check for the previous parameter.
   * @param[out] receiveBufferSize Final size of the received message.
   * @param[out] originLocator Address of the remote sender.
   * @return Success of the managed Receive operation.
   */
   //bool Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
   //             Locator_t& originLocator);

  /**
   * Reports whether this resource supports the given local locator (i.e., said locator
   * maps to the transport channel managed by this resource).
   */
   bool SupportsLocator(const Locator_t& localLocator);

   /**
    * Aborts a blocking receive (thread safe).
    */
   void Abort();

   /**
    * Resources can only be transfered through move semantics. Copy, assignment, and
    * construction outside of the factory are forbidden.
    */
   ReceiverResource(ReceiverResource&&);
   ~ReceiverResource();

   virtual MessageReceiver* CreateMessageReceiver();

   // Functions to associate/remove associatedendpoints
   void associateEndpoint(Endpoint *to_add);
   void removeEndpoint(Endpoint *to_remove);
   bool checkReaders(EntityId_t readerID);
   void processDataMsg(EntityId_t readerID, CacheChange_t* ch);

   void processDataFragMsg(EntityId_t readerID, CacheChange_t *incomingChange, uint32_t sampleSize,
       uint32_t fragmentStartingNum);

   void processHeartbeatMsg(EntityId_t readerID, GUID_t &writerGUID, uint32_t hbCount, SequenceNumber_t &firstSN,
       SequenceNumber_t &lastSN, bool finalFlag, bool livelinessFlag);

   void processGapMsg(EntityId_t readerID, GUID_t &writerGUID, SequenceNumber_t &gapStart,
       SequenceNumberSet_t &gapList);

   bool processSubMsgNackFrag(const GUID_t& readerGUID, const GUID_t& writerGUID, SequenceNumber_t& writerSN,
       FragmentNumberSet_t& fnState, uint32_t Ackcount);

   bool processAckNack(const GUID_t& readerGUID, const GUID_t& writerGUID, uint32_t ackCount,
       const SequenceNumberSet_t& snSet, bool finalFlag);

protected:
   ReceiverResource(RTPSParticipantImpl*, TransportInterface&, const Locator_t&, uint32_t maxMsgSize);
   ReceiverResource() {};
   std::function<void()> Cleanup;
   std::function<bool(const Locator_t&)> LocatorMapsToManagedChannel;
   bool mValid; // Post-construction validity check for the NetworkFactory
   RTPSParticipantImpl* m_participant;
   uint32_t m_maxMsgSize;

private:

   ReceiverResource(const ReceiverResource&)            = delete;
   ReceiverResource& operator=(const ReceiverResource&) = delete;

   std::mutex mtx;
   std::vector<RTPSWriter*> AssociatedWriters;
   std::vector<RTPSReader*> AssociatedReaders;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
