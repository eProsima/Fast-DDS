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

#ifndef TEST_TCPV4_TRANSPORT_H
#define TEST_TCPV4_TRANSPORT_H
#include <fastrtps/transport/TCPv4Transport.h>
#include <fastrtps/rtps/messages/RTPS_messages.h>
#include <fastrtps/rtps/common/SequenceNumber.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <vector>

#include "test_TCPv4TransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

// Handle to a persistent log of dropped packets. Defaults to length 0 (no logging) to prevent wasted resources.
static std::vector<std::vector<octet> > test_TCPv4Transport_DropLog;
static uint32_t test_TCPv4Transport_DropLogLength;
static bool test_TCPv4Transport_ShutdownAllNetwork;
static bool test_TCPv4Transport_CloseSocketConnection;
/*
 * This transport acts as a shim over TCPv4, allowing
 * packets to be dropped under certain criteria.
 */
class test_TCPv4Transport : public TCPv4Transport
{
public:
   RTPS_DllAPI test_TCPv4Transport(const test_TCPv4TransportDescriptor& descriptor);

   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator) override;

   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
                        const Locator_t& remoteLocator, ChannelResource* pChannelResource) override;

protected:
    void CalculateCRC(TCPHeader &header, const octet *data, uint32_t size);

private:
    uint8_t mInvalidCRCsPercentage;
    uint8_t mCloseSocketOnSendPercentage;
    uint8_t mDropDataMessagesPercentage;
    bool mDropParticipantBuiltinTopicData;
    bool mDropPublicationBuiltinTopicData;
    bool mDropSubscriptionBuiltinTopicData;
    uint8_t mDropDataFragMessagesPercentage;
    uint8_t mDropHeartbeatMessagesPercentage;
    uint8_t mDropAckNackMessagesPercentage;
    std::vector<SequenceNumber_t> mSequenceNumberDataMessagesToDrop;
    uint8_t mPercentageOfMessagesToDrop;

    bool LogDrop(const octet* buffer, uint32_t size);
    bool PacketShouldDrop(const octet* sendBuffer, uint32_t sendBufferSize);
    bool RandomChanceDrop();
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
