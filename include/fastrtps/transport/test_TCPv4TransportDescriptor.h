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

#ifndef TEST_TCPV4_TRANSPORT_DESCRIPTOR
#define TEST_TCPV4_TRANSPORT_DESCRIPTOR

#include "TCPv4TransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

typedef struct test_TCPv4TransportDescriptor : public TCPv4TransportDescriptor
{
   bool granularMode;

   std::vector<uint16_t> logicalPortsBlocked;
   uint8_t invalidCRCsPercentage;
   uint8_t mCloseSocketOnSendPercentage;
   uint8_t invalidTransactionPercentage;

   // Test shim parameters
   uint8_t dropDataMessagesPercentage;
   bool dropParticipantBuiltinTopicData;
   bool dropPublicationBuiltinTopicData;
   bool dropSubscriptionBuiltinTopicData;
   uint8_t dropDataFragMessagesPercentage;
   uint8_t dropHeartbeatMessagesPercentage;
   uint8_t dropAckNackMessagesPercentage;

   // General drop percentage (indescriminate)
   uint8_t percentageOfMessagesToDrop;
   std::vector<SequenceNumber_t> sequenceNumberDataMessagesToDrop;

   uint32_t dropLogLength; // logs dropped packets.

   RTPS_DllAPI test_TCPv4TransportDescriptor();
   virtual ~test_TCPv4TransportDescriptor(){}

   virtual TransportInterface* create_transport() const override;
} test_TCPv4TransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
