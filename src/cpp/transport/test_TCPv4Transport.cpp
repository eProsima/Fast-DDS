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
    , invalid_crcs_percentage_(descriptor.invalidCRCsPercentage)
    , close_socket_on_send_percentage_(descriptor.close_socket_on_send_percentage_)
    , drop_data_messages_percentage_(descriptor.dropDataMessagesPercentage)
    , drop_participant_builtin_topic_data_(descriptor.dropParticipantBuiltinTopicData)
    , drop_publication_builtin_topic_data_(descriptor.dropPublicationBuiltinTopicData)
    , drop_subscription_builtin_topic_data_(descriptor.dropSubscriptionBuiltinTopicData)
    , drop_data_frag_messages_percentage_(descriptor.dropDataFragMessagesPercentage)
    , drop_heartbeat_messages_percentage_(descriptor.dropHeartbeatMessagesPercentage)
    , drop_ack_nack_messages_percentage_(descriptor.dropAckNackMessagesPercentage)
    , sequence_number_data_messages_to_drop_(descriptor.sequenceNumberDataMessagesToDrop)
    , percentage_of_messages_to_drop_(descriptor.percentageOfMessagesToDrop)
{
    g_test_TCPv4Transport_DropLog.clear();
    g_test_TCPv4Transport_DropLogLength = descriptor.dropLogLength;
    srand(static_cast<unsigned>(time(NULL)));

    rtcp_message_manager_ = new test_RTCPMessageManager(this);
    test_RTCPMessageManager* pMgr = ((test_RTCPMessageManager*)rtcp_message_manager_);
    pMgr->SetInvalidTransactionPercentage(descriptor.invalidTransactionPercentage);
    pMgr->SetLogicalPortsBlocked(descriptor.logicalPortsBlocked);
}

test_TCPv4TransportDescriptor::test_TCPv4TransportDescriptor()
    : TCPv4TransportDescriptor()
    , invalidCRCsPercentage(0)
    , close_socket_on_send_percentage_(0)
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

bool test_TCPv4Transport::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const Locator_t& local_locator,
        const Locator_t& remote_locator)
{
    if (close_socket_on_send_percentage_ <= (rand() % 100))
    {
        if (packet_should_drop(send_buffer, send_buffer_size))
        {
            log_drop(send_buffer, send_buffer_size);
            return true;
        }
        else
        {
            if (g_test_TCPv4Transport_CloseSocketConnection)
            {
                g_test_TCPv4Transport_CloseSocketConnection = false;
                CloseOutputChannel(local_locator);
                return true;
            }
            else
            {
                return TCPv4Transport::send(send_buffer, send_buffer_size, local_locator, remote_locator);
            }
        }
    }
    else
    {
        auto it = channel_resources_.find(IPLocator::toPhysicalLocator(local_locator));
        if (it != channel_resources_.end())
        {
            try
            {
                it->second->cancel();
                it->second->shutdown(asio::ip::tcp::socket::shutdown_both);
            }
            catch (std::exception&)
            {
                // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
            }
            it->second->close();
        }

        return true;
    }
}

bool test_TCPv4Transport::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const Locator_t& local_locator,
        const Locator_t& remote_locator,
        ChannelResource *p_channel_resource)
{
    if (close_socket_on_send_percentage_ <= (rand() % 100))
    {
        if (packet_should_drop(send_buffer, send_buffer_size))
        {
            log_drop(send_buffer, send_buffer_size);
            return true;
        }
        else
        {
            if (g_test_TCPv4Transport_CloseSocketConnection)
            {
                g_test_TCPv4Transport_CloseSocketConnection = false;
                p_channel_resource->disable();
                CloseOutputChannel(local_locator);
                return true;
            }
            else
            {
                return TCPv4Transport::send(send_buffer, send_buffer_size, local_locator, remote_locator, p_channel_resource);
            }
        }
    }
    else
    {
        auto it = channel_resources_.find(IPLocator::toPhysicalLocator(local_locator));
        if (it != channel_resources_.end())
        {
            try
            {
                it->second->cancel();
                it->second->shutdown(asio::ip::tcp::socket::shutdown_both);
            }
            catch (std::exception&)
            {
                // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
            }
            it->second->close();
        }
        return true;
    }
}

void test_TCPv4Transport::calculate_crc(
        TCPHeader &header,
        const octet *data,
        uint32_t size)
{
    if (invalid_crcs_percentage_ <= (rand() % 100))
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

static bool ReadSubmessageHeader(
        CDRMessage_t& msg,
        SubmessageHeader_t& smh)
{
    if (msg.length - msg.pos < 4)
        return false;

    smh.submessageId = msg.buffer[msg.pos]; msg.pos++;
    smh.flags = msg.buffer[msg.pos]; msg.pos++;
    msg.msg_endian = smh.flags & BIT(0) ? LITTLEEND : BIGEND;
    uint16_t length = 0;
    CDRMessage::readUInt16(&msg, &length);

    if (msg.pos + length > msg.length)
    {
        return false;
    }

    if (length == 0)
    {
        // THIS IS THE LAST SUBMESSAGE
        smh.submessageLength = msg.length - msg.pos;
        smh.is_last = true;
    }
    else
    {
        smh.submessageLength = length;
        smh.is_last = false;
    }
    return true;
}

bool test_TCPv4Transport::packet_should_drop(
        const octet* send_buffer,
        uint32_t send_buffer_size)
{
    if(g_test_TCPv4Transport_ShutdownAllNetwork)
    {
        return true;
    }

    CDRMessage_t cdrMessage(send_buffer_size);;
    memcpy(cdrMessage.buffer, send_buffer, send_buffer_size);
    cdrMessage.length = send_buffer_size;

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

                if((!drop_participant_builtin_topic_data_ && writer_id == c_EntityId_SPDPWriter) ||
                        (!drop_publication_builtin_topic_data_ && writer_id == c_EntityId_SEDPPubWriter) ||
                        (!drop_subscription_builtin_topic_data_ && writer_id == c_EntityId_SEDPSubWriter))
                    return false;

                if(drop_data_messages_percentage_ > (rand()%100))
                    return true;

                break;

            case ACKNACK:
                if(drop_ack_nack_messages_percentage_ > (rand()%100))
                    return true;

                break;

            case HEARTBEAT:
                cdrMessage.pos += 8;
                CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;
                if(drop_heartbeat_messages_percentage_ > (rand()%100))
                    return true;

                break;

            case DATA_FRAG:
                if(drop_data_frag_messages_percentage_  > (rand()%100))
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
                find(sequence_number_data_messages_to_drop_.begin(),
                    sequence_number_data_messages_to_drop_.end(),
                    sequence_number) != sequence_number_data_messages_to_drop_.end())
            return true;

        cdrMessage.pos += cdrSubMessageHeader.submessageLength;
    }

    if(random_chance_drop())
        return true;

    return false;
}

bool test_TCPv4Transport::log_drop(
        const octet* buffer,
        uint32_t size)
{
    if (g_test_TCPv4Transport_DropLog.size() < g_test_TCPv4Transport_DropLogLength)
    {
        vector<octet> message;
        message.assign(buffer, buffer + size);
        g_test_TCPv4Transport_DropLog.push_back(message);
        return true;
    }

    return false;
}

bool test_TCPv4Transport::random_chance_drop()
{
    return percentage_of_messages_to_drop_ > (rand()%100);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
