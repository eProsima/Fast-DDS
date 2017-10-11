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

static const uint32_t maximumMessageSize = 65500;
vector<vector<octet> > test_UDPv4Transport::DropLog;
uint32_t test_UDPv4Transport::DropLogLength = 0;
bool test_UDPv4Transport::ShutdownAllNetwork = false;

test_UDPv4Transport::test_UDPv4Transport(const test_UDPv4TransportDescriptor& descriptor):
    mDropDataMessagesPercentage(descriptor.dropDataMessagesPercentage),
    mDropParticipantBuiltinTopicData(descriptor.dropParticipantBuiltinTopicData),
    mDropPublicationBuiltinTopicData(descriptor.dropPublicationBuiltinTopicData),
    mDropSubscriptionBuiltinTopicData(descriptor.dropSubscriptionBuiltinTopicData),
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
    sendBufferSize(0),
    receiveBufferSize(0),
    dropDataMessagesPercentage(0),
    dropParticipantBuiltinTopicData(false),
    dropPublicationBuiltinTopicData(false),
    dropSubscriptionBuiltinTopicData(false),
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
    {
        return UDPv4Transport::Send(sendBuffer, sendBufferSize, localLocator, remoteLocator);
    }
}

static bool ReadSubmessageHeader(CDRMessage_t& msg, SubmessageHeader_t& smh)
{
    if(msg.length - msg.pos < 4)
        return false;

    smh.submessageId = msg.buffer[msg.pos]; msg.pos++;
    smh.flags = msg.buffer[msg.pos]; msg.pos++;
    msg.msg_endian = smh.flags & BIT(0) ? LITTLEEND : BIGEND;
    CDRMessage::readUInt16(&msg, &smh.submessageLength);
    return true;
}

bool test_UDPv4Transport::PacketShouldDrop(const octet* sendBuffer, uint32_t sendBufferSize)
{
    if(test_UDPv4Transport::ShutdownAllNetwork)
    {
        return true;
    }

    CDRMessage_t cdrMessage(sendBufferSize);;
    memcpy(cdrMessage.buffer, sendBuffer, sendBufferSize);
    cdrMessage.length = sendBufferSize;

    if(cdrMessage.length < RTPSMESSAGE_HEADER_SIZE)
        return false;

    if(cdrMessage.buffer[cdrMessage.pos++] != 'R' ||
            cdrMessage.buffer[cdrMessage.pos++] != 'T' ||
            cdrMessage.buffer[cdrMessage.pos++] != 'P' ||
            cdrMessage.buffer[cdrMessage.pos++] != 'S')
        return false;

    cdrMessage.pos += 4 + 12; // RTPS version + GUID

    SubmessageHeader_t cdrSubMessageHeader;
    while (cdrMessage.pos < cdrMessage.length)
    {  
        ReadSubmessageHeader(cdrMessage, cdrSubMessageHeader);
        if (cdrMessage.pos + cdrSubMessageHeader.submessageLength > cdrMessage.length)
            return false;

        SequenceNumber_t sequence_number{SequenceNumber_t::unknown()};
        EntityId_t writer_id;
        auto old_pos = cdrMessage.pos;

        switch(cdrSubMessageHeader.submessageId)
        {
            case DATA:
                // Get WriterID.
                cdrMessage.pos += 8;
                CDRMessage::readEntityId(&cdrMessage, &writer_id);
                CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;

                if((!mDropParticipantBuiltinTopicData && writer_id == c_EntityId_SPDPWriter) ||
                        (!mDropPublicationBuiltinTopicData && writer_id == c_EntityId_SEDPPubWriter) ||
                        (!mDropSubscriptionBuiltinTopicData && writer_id == c_EntityId_SEDPSubWriter))
                    return false;

                if(mDropDataMessagesPercentage > (rand()%100))
                    return true;

                break;

            case ACKNACK:
                if(mDropAckNackMessagesPercentage > (rand()%100))
                    return true;

                break;

            case HEARTBEAT:
                cdrMessage.pos += 8;
                CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;
                if(mDropHeartbeatMessagesPercentage > (rand()%100))
                    return true;

                break;

            case DATA_FRAG:
                if(mDropDataFragMessagesPercentage  > (rand()%100))
                    return true;

                break;

            case GAP:
                cdrMessage.pos += 8;
                CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;

                break;
        }

        if(sequence_number != SequenceNumber_t::unknown() &&
                find(mSequenceNumberDataMessagesToDrop.begin(),
                    mSequenceNumberDataMessagesToDrop.end(),
                    sequence_number) != mSequenceNumberDataMessagesToDrop.end())
            return true;

        cdrMessage.pos += cdrSubMessageHeader.submessageLength;
    }

    if(RandomChanceDrop())
        return true;

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

bool test_UDPv4Transport::RandomChanceDrop()
{
    return mPercentageOfMessagesToDrop > (rand()%100);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
