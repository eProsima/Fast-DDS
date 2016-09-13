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

#include <fastrtps/transport/test_UDPv4Transport.h>
#include <cstdlib>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static const uint32_t maximumUDPSocketSize = 65536;
static const uint32_t maximumMessageSize = 65000;
vector<vector<octet> > test_UDPv4Transport::DropLog;
uint32_t test_UDPv4Transport::DropLogLength = 0;

test_UDPv4Transport::test_UDPv4Transport(const test_UDPv4TransportDescriptor& descriptor):
    mDropDataMessagesPercentage(descriptor.dropDataMessagesPercentage),
    mDropDataFragMessagesPercentage(descriptor.dropDataFragMessagesPercentage),
    mDropHeartbeatMessagesPercentage(descriptor.dropHeartbeatMessagesPercentage),
    mDropAckNackMessagesPercentage(descriptor.dropAckNackMessagesPercentage),
    mSequenceNumberDataMessagesToDrop(descriptor.sequenceNumberDataMessagesToDrop),
    mPercentageOfMessagesToDrop(descriptor.percentageOfMessagesToDrop)
    {
        UDPv4Transport::mSendBufferSize = descriptor.sendBufferSize;
        UDPv4Transport::mReceiveBufferSize = descriptor.receiveBufferSize;
        DropLog.clear();
        DropLogLength = descriptor.dropLogLength;
        srand(static_cast<unsigned>(time(NULL)));
    }

RTPS_DllAPI test_UDPv4TransportDescriptor::test_UDPv4TransportDescriptor():
    TransportDescriptorInterface(maximumMessageSize),
    sendBufferSize(maximumUDPSocketSize),
    receiveBufferSize(maximumUDPSocketSize),
    dropDataMessagesPercentage(0),
    dropDataFragMessagesPercentage(0),
    dropHeartbeatMessagesPercentage(0),
    dropAckNackMessagesPercentage(0),
    percentageOfMessagesToDrop(0),
    sequenceNumberDataMessagesToDrop(),
    dropLogLength(0)
    {
    }

bool test_UDPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    if (PacketShouldDrop(sendBuffer, sendBufferSize))
    {
        LogDrop(sendBuffer, sendBufferSize);
        return true;
    }
    else   
        return UDPv4Transport::Send(sendBuffer, sendBufferSize, localLocator, remoteLocator);
}

static bool ReadSubmessageHeader(CDRMessage_t& msg, SubmessageHeader_t& smh)
{
    if(msg.length - msg.pos < 4)
        return false;

    smh.submessageId = msg.buffer[msg.pos];msg.pos++;
    smh.flags = msg.buffer[msg.pos];msg.pos++;
    msg.msg_endian = smh.flags & BIT(0) ? LITTLEEND : BIGEND;
    CDRMessage::readUInt16(&msg, &smh.submessageLength);
    return true;
}

bool test_UDPv4Transport::PacketShouldDrop(const octet* sendBuffer, uint32_t sendBufferSize)
{
    CDRMessage_t cdrMessage(sendBufferSize);;
    memcpy(cdrMessage.buffer, sendBuffer, sendBufferSize);
    cdrMessage.length = sendBufferSize;

    return ( (mDropDataMessagesPercentage      > (rand()%100) && ContainsSubmessageOfID(cdrMessage, DATA))       ||
            (mDropAckNackMessagesPercentage   > (rand()%100) && ContainsSubmessageOfID(cdrMessage, ACKNACK))    ||
            (mDropHeartbeatMessagesPercentage > (rand()%100) && ContainsSubmessageOfID(cdrMessage, HEARTBEAT))  || 
            (mDropDataFragMessagesPercentage  > (rand()%100) && ContainsSubmessageOfID(cdrMessage, DATA_FRAG))  || 
            ContainsSequenceNumberToDrop(cdrMessage)                                                           ||
            RandomChanceDrop());
}

bool test_UDPv4Transport::ContainsSubmessageOfID(CDRMessage_t& cdrMessage, octet ID)
{
    if(cdrMessage.length < RTPSMESSAGE_HEADER_SIZE)
        return false;

    cdrMessage.pos = 4 + 12; // RTPS header letters + RTPS version

    SubmessageHeader_t cdrSubMessageHeader;
    while (cdrMessage.pos < cdrMessage.length)
    {  
        ReadSubmessageHeader(cdrMessage, cdrSubMessageHeader);
        if (cdrMessage.pos + cdrSubMessageHeader.submessageLength > cdrMessage.length)
            return false;
        if (cdrSubMessageHeader.submessageId == ID)
            return true;

        cdrMessage.pos += cdrSubMessageHeader.submessageLength;
    }

    return false;
}


bool test_UDPv4Transport::LogDrop(const octet* buffer, uint32_t size)
{
    if (DropLog.size() < DropLogLength)
    {
        vector<octet> message;
        message.assign(buffer, buffer + size);
        DropLog.push_back(message);
        return true;
    }

    return false;
}

bool test_UDPv4Transport::ContainsSequenceNumberToDrop(CDRMessage_t& cdrMessage)
{
    if(cdrMessage.length < RTPSMESSAGE_HEADER_SIZE)
        return false;

    cdrMessage.pos = 4 + 12; // RTPS header letters + RTPS version

    SubmessageHeader_t cdrSubMessageHeader;
    while (cdrMessage.pos < cdrMessage.length)
    {  
        ReadSubmessageHeader(cdrMessage, cdrSubMessageHeader);
        if (cdrMessage.pos + cdrSubMessageHeader.submessageLength > cdrMessage.length)
            return false;

        auto positionBeforeSubmessageData = cdrMessage.pos;

        // Skip ahead based on message fields
        switch (cdrSubMessageHeader.submessageId)
        {
            case DATA:
                cdrMessage.pos += (2+2+4+4); // Flagskip + Octets to QoS + entityID + entityID
                break;
            case GAP:
                cdrMessage.pos += (4+4); // entityID + entityID
                break;
            case HEARTBEAT:
                cdrMessage.pos += (4+4); // entityID + entityID. We read FirstSN
            default:
                cdrMessage.pos += cdrSubMessageHeader.submessageLength;
                continue; // Messages without sequence number
        }

        SequenceNumber_t sequenceNumber;
        bool valid = CDRMessage::readInt32(&cdrMessage, &sequenceNumber.high);
        valid &= CDRMessage::readUInt32(&cdrMessage, &sequenceNumber.low);
        bool contained = find(mSequenceNumberDataMessagesToDrop.begin(),
                mSequenceNumberDataMessagesToDrop.end(),
                sequenceNumber) != mSequenceNumberDataMessagesToDrop.end();

        if (valid && contained)
            return true;

        // Jump to the next submessage
        cdrMessage.pos = positionBeforeSubmessageData;
        cdrMessage.pos += cdrSubMessageHeader.submessageLength;
    }

    return false;
}

bool test_UDPv4Transport::RandomChanceDrop()
{
    return mPercentageOfMessagesToDrop > (rand()%100);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
