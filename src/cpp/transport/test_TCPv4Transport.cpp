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

#include <fastrtps/transport/test_TCPv4Transport.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/tcp/test_RTCPMessageManager.h>
#include <fastrtps/transport/TCPChannelResource.h>
#include <fastrtps/utils/IPLocator.h>
#include <cstdlib>

using namespace std;

namespace eprosima{
namespace fastrtps{
namespace rtps{

test_TCPv4Transport::test_TCPv4Transport(const test_TCPv4TransportDescriptor& descriptor)
    : TCPv4Transport(descriptor)
    , mInvalidCRCsPercentage(descriptor.invalidCRCsPercentage)
    , mCloseSocketOnSendPercentage(descriptor.mCloseSocketOnSendPercentage)
    , mDropDataMessagesPercentage(descriptor.dropDataMessagesPercentage)
    , mDropParticipantBuiltinTopicData(descriptor.dropParticipantBuiltinTopicData)
    , mDropPublicationBuiltinTopicData(descriptor.dropPublicationBuiltinTopicData)
    , mDropSubscriptionBuiltinTopicData(descriptor.dropSubscriptionBuiltinTopicData)
    , mDropDataFragMessagesPercentage(descriptor.dropDataFragMessagesPercentage)
    , mDropHeartbeatMessagesPercentage(descriptor.dropHeartbeatMessagesPercentage)
    , mDropAckNackMessagesPercentage(descriptor.dropAckNackMessagesPercentage)
    , mSequenceNumberDataMessagesToDrop(descriptor.sequenceNumberDataMessagesToDrop)
    , mPercentageOfMessagesToDrop(descriptor.percentageOfMessagesToDrop)
    {
        test_TCPv4Transport_DropLog.clear();
        test_TCPv4Transport_DropLogLength = descriptor.dropLogLength;
        srand(static_cast<unsigned>(time(NULL)));

        mRTCPMessageManager = new test_RTCPMessageManager(this);
        test_RTCPMessageManager* pMgr = ((test_RTCPMessageManager*)mRTCPMessageManager);
        pMgr->SetInvalidTransactionPercentage(descriptor.invalidTransactionPercentage);
        pMgr->SetLogicalPortsBlocked(descriptor.logicalPortsBlocked);
    }

test_TCPv4TransportDescriptor::test_TCPv4TransportDescriptor()
    : TCPv4TransportDescriptor()
    , invalidCRCsPercentage(0)
    , mCloseSocketOnSendPercentage(0)
    , invalidTransactionPercentage(0)
    , dropDataMessagesPercentage(0)
    , dropParticipantBuiltinTopicData(false)
    , dropPublicationBuiltinTopicData(false)
    , dropSubscriptionBuiltinTopicData(false)
    , dropDataFragMessagesPercentage(0)
    , dropHeartbeatMessagesPercentage(0)
    , dropAckNackMessagesPercentage(0)
    , percentageOfMessagesToDrop(0)
    , sequenceNumberDataMessagesToDrop()
    , dropLogLength(0)
    {
    }

TransportInterface* test_TCPv4TransportDescriptor::create_transport() const
{
    return new test_TCPv4Transport(*this);
}

bool test_TCPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    if (mCloseSocketOnSendPercentage <= (rand() % 100))
    {
        if (PacketShouldDrop(sendBuffer, sendBufferSize))
        {
            LogDrop(sendBuffer, sendBufferSize);
            return true;
        }
        else
        {
            if (test_TCPv4Transport_CloseSocketConnection)
            {
                test_TCPv4Transport_CloseSocketConnection = false;
                CloseOutputChannel(localLocator);
                return true;
            }
            else
            {
                return TCPv4Transport::Send(sendBuffer, sendBufferSize, localLocator, remoteLocator);
            }
        }
    }
    else
    {
        auto it = mChannelResources.find(IPLocator::toPhysicalLocator(localLocator));
        if (it != mChannelResources.end())
        {
            try
            {
                it->second->getSocket()->cancel();
                it->second->getSocket()->shutdown(asio::ip::tcp::socket::shutdown_both);
            }
            catch (std::exception&)
            {
                // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
            }
            it->second->getSocket()->close();
        }

        return true;
    }
}

bool test_TCPv4Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator, ChannelResource *pChannelResource)
{
    if (mCloseSocketOnSendPercentage <= (rand() % 100))
    {
        if (PacketShouldDrop(sendBuffer, sendBufferSize))
        {
            LogDrop(sendBuffer, sendBufferSize);
            return true;
        }
        else
        {
            if (test_TCPv4Transport_CloseSocketConnection)
            {
                test_TCPv4Transport_CloseSocketConnection = false;
                pChannelResource->Disable();
                CloseOutputChannel(localLocator);
                return true;
            }
            else
            {
                return TCPv4Transport::Send(sendBuffer, sendBufferSize, localLocator, remoteLocator, pChannelResource);
            }
        }
    }
    else
    {
        auto it = mChannelResources.find(IPLocator::toPhysicalLocator(localLocator));
        if (it != mChannelResources.end())
        {
            try
            {
                it->second->getSocket()->cancel();
                it->second->getSocket()->shutdown(asio::ip::tcp::socket::shutdown_both);
            }
            catch (std::exception&)
            {
                // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
            }
            it->second->getSocket()->close();
        }
        return true;
    }
}

void test_TCPv4Transport::CalculateCRC(TCPHeader &header, const octet *data, uint32_t size)
{
    if (mInvalidCRCsPercentage <= (rand() % 100))
    {
        uint32_t crc(0);
        for (uint32_t i = 0; i < size; ++i)
        {
            crc = RTCPMessageManager::addToCRC(crc, data[i]);
        }
    }
    else
    {
        header.crc = 0;
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

bool test_TCPv4Transport::PacketShouldDrop(const octet* sendBuffer, uint32_t sendBufferSize)
{
    if(test_TCPv4Transport_ShutdownAllNetwork)
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

bool test_TCPv4Transport::LogDrop(const octet* buffer, uint32_t size)
{
    if (test_TCPv4Transport_DropLog.size() < test_TCPv4Transport_DropLogLength)
    {
        vector<octet> message;
        message.assign(buffer, buffer + size);
        test_TCPv4Transport_DropLog.push_back(message);
        return true;
    }

    return false;
}

bool test_TCPv4Transport::RandomChanceDrop()
{
    return mPercentageOfMessagesToDrop > (rand()%100);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
