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

#include <rtps/transport/test_UDPv4Transport.h>

#include <cstdlib>
#include <functional>

#include <asio.hpp>
#include <fastdds/utils/IPLocator.hpp>

using namespace std;

namespace eprosima {
namespace fastdds {
namespace rtps {

test_UDPv4Transport::test_UDPv4Transport(
        const test_UDPv4TransportDescriptor& descriptor)
    : test_transport_options(descriptor.test_transport_options)
    , drop_data_messages_percentage_(descriptor.dropDataMessagesPercentage)
    , drop_participant_builtin_data_messages_percentage_(descriptor.dropParticipantBuiltinDataMessagesPercentage)
    , drop_publication_builtin_data_messages_percentage_(descriptor.dropPublicationBuiltinDataMessagesPercentage)
    , drop_subscription_builtin_data_messages_percentage_(descriptor.dropSubscriptionBuiltinDataMessagesPercentage)
    , drop_data_messages_filter_(descriptor.drop_data_messages_filter_)
    , drop_builtin_data_messages_filter_(descriptor.drop_builtin_data_messages_filter_)
    , drop_participant_builtin_topic_data_(descriptor.dropParticipantBuiltinTopicData)
    , drop_publication_builtin_topic_data_(descriptor.dropPublicationBuiltinTopicData)
    , drop_subscription_builtin_topic_data_(descriptor.dropSubscriptionBuiltinTopicData)
    , drop_data_frag_messages_percentage_(descriptor.dropDataFragMessagesPercentage)
    , drop_data_frag_messages_filter_(descriptor.drop_data_frag_messages_filter_)
    , drop_heartbeat_messages_percentage_(descriptor.dropHeartbeatMessagesPercentage)
    , drop_heartbeat_messages_filter_(descriptor.drop_heartbeat_messages_filter_)
    , drop_ack_nack_messages_percentage_(descriptor.dropAckNackMessagesPercentage)
    , drop_ack_nack_messages_filter_(descriptor.drop_ack_nack_messages_filter_)
    , drop_gap_messages_percentage_(descriptor.dropGapMessagesPercentage)
    , drop_gap_messages_filter_(descriptor.drop_gap_messages_filter_)
    , sub_messages_filter_(descriptor.sub_messages_filter_)
    , percentage_of_messages_to_drop_(descriptor.percentageOfMessagesToDrop)
    , messages_filter_(descriptor.messages_filter_)
    , sequence_number_data_messages_to_drop_(descriptor.sequenceNumberDataMessagesToDrop)
    , locator_filter_(descriptor.locator_filter_)
{
    if (nullptr == test_transport_options)
    {
        throw std::runtime_error("test_UDPv4Transport: test_transport_options is nullptr");
    }

    test_transport_options->test_UDPv4Transport_DropLogLength = 0;
    test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;
    UDPv4Transport::mSendBufferSize = descriptor.sendBufferSize;
    UDPv4Transport::mReceiveBufferSize = descriptor.receiveBufferSize;
    for (auto interf : descriptor.interfaceWhiteList)
    {
        UDPv4Transport::interface_whitelist_.emplace_back(asio::ip::address_v4::from_string(interf));
    }
    test_transport_options->test_UDPv4Transport_DropLog.clear();
    test_transport_options->test_UDPv4Transport_DropLogLength = descriptor.dropLogLength;
}

test_UDPv4TransportDescriptor::test_UDPv4TransportDescriptor()
    : SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
{
}

TransportInterface* test_UDPv4TransportDescriptor::create_transport() const
{
    return new test_UDPv4Transport(*this);
}

bool test_UDPv4TransportDescriptor::operator ==(
        const test_UDPv4TransportDescriptor& t) const
{
    return (this->dropDataMessagesPercentage == t.dropDataMessagesPercentage &&
           this->dropParticipantBuiltinTopicData == t.dropParticipantBuiltinTopicData &&
           this->dropPublicationBuiltinTopicData == t.dropPublicationBuiltinTopicData &&
           this->dropSubscriptionBuiltinTopicData == t.dropSubscriptionBuiltinTopicData &&
           this->dropDataFragMessagesPercentage == t.dropDataFragMessagesPercentage &&
           this->dropHeartbeatMessagesPercentage == t.dropHeartbeatMessagesPercentage &&
           this->dropAckNackMessagesPercentage == t.dropAckNackMessagesPercentage &&
           this->dropGapMessagesPercentage == t.dropGapMessagesPercentage &&
           this->percentageOfMessagesToDrop == t.percentageOfMessagesToDrop &&
           this->sequenceNumberDataMessagesToDrop == t.sequenceNumberDataMessagesToDrop &&
           this->dropLogLength == t.dropLogLength &&
           SocketTransportDescriptor::operator ==(t));
}

bool test_UDPv4Transport::get_ips(
        std::vector<fastdds::rtps::IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup) const
{
    if (!test_transport_options->simulate_no_interfaces)
    {
        return UDPv4Transport::get_ips(locNames, return_loopback, force_lookup);
    }

    if (return_loopback)
    {
        fastdds::rtps::IPFinder::info_IP local;
        local.type = fastdds::rtps::IPFinder::IPTYPE::IP4_LOCAL;
        local.dev = "lo";
        local.name = "127.0.0.1";
        local.locator.kind = LOCATOR_KIND_UDPv4;
        fill_local_ip(local.locator);
        local.masked_locator = local.locator;
        local.masked_locator.mask(32);
        locNames.emplace_back(local);
    }
    return true;
}

LocatorList test_UDPv4Transport::NormalizeLocator(
        const Locator& locator)
{
    if (!test_transport_options->simulate_no_interfaces)
    {
        return UDPv4Transport::NormalizeLocator(locator);
    }

    LocatorList list;
    if (fastdds::rtps::IPLocator::isAny(locator))
    {
        Locator newloc(locator);
        fastdds::rtps::IPLocator::setIPv4(newloc, "127.0.0.1");
        list.push_back(newloc);
    }
    else
    {
        list.push_back(locator);
    }
    return list;
}

bool test_UDPv4Transport::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        eProsimaUDPSocket& socket,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        bool only_multicast_purpose,
        bool whitelisted,
        const std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    fastdds::rtps::LocatorsIterator& it = *destination_locators_begin;

    bool ret = true;

    while (it != *destination_locators_end)
    {
        if (!IsLocatorSupported(*it))
        {
            ++it;
            continue;
        }

        auto now = std::chrono::steady_clock::now();

        if (now < max_blocking_time_point)
        {
            ret &= send(buffers,
                            total_bytes,
                            socket,
                            *it,
                            only_multicast_purpose,
                            whitelisted,
                            std::chrono::duration_cast<std::chrono::microseconds>(now - max_blocking_time_point));

            ++it;
        }
        else // Time is out
        {
            ret = false;
            break;
        }
    }

    return ret;
}

bool test_UDPv4Transport::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        eProsimaUDPSocket& socket,
        const Locator& remote_locator,
        bool only_multicast_purpose,
        bool whitelisted,
        const std::chrono::microseconds& timeout)
{
    bool is_multicast_remote_address = fastdds::rtps::IPLocator::IPLocator::isMulticast(remote_locator);
    if (is_multicast_remote_address == only_multicast_purpose || whitelisted)
    {
        if (packet_should_drop(buffers, total_bytes) || should_drop_locator(remote_locator))
        {
            statistics_info_.set_statistics_message_data(remote_locator, buffers.back(), total_bytes);
            log_drop(buffers, total_bytes);
            return true;
        }
        else
        {
            return UDPv4Transport::send(buffers, total_bytes, socket, remote_locator, only_multicast_purpose,
                           whitelisted, timeout);
        }
    }

    return false;
}

static bool ReadSubmessageHeader(
        CDRMessage_t& msg,
        SubmessageHeader_t& smh)
{
    if (msg.length - msg.pos < 4)
    {
        return false;
    }

    smh.submessageId = msg.buffer[msg.pos]; msg.pos++;
    smh.flags = msg.buffer[msg.pos]; msg.pos++;
    msg.msg_endian = smh.flags & BIT(0) ? fastdds::rtps::LITTLEEND : fastdds::rtps::BIGEND;
    uint16_t length = 0;
    fastdds::rtps::CDRMessage::readUInt16(&msg, &length);

    if (msg.pos + length > msg.length)
    {
        return false;
    }

    if ((length == 0) && (smh.submessageId != fastdds::rtps::INFO_TS) && (smh.submessageId != fastdds::rtps::PAD))
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

bool test_UDPv4Transport::should_drop_locator(
        const Locator& remote_locator)
{
    return test_transport_options->locator_filter(remote_locator) ||
           locator_filter_(remote_locator) ||
           // If there are no interfaces (simulate_no_interfaces), only multicast and localhost traffic is sent
           (test_transport_options->simulate_no_interfaces &&
           !fastdds::rtps::IPLocator::isMulticast(remote_locator) &&
           !fastdds::rtps::IPLocator::isLocal(remote_locator));
}

bool test_UDPv4Transport::packet_should_drop(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes)
{
    if (test_transport_options->test_UDPv4Transport_ShutdownAllNetwork)
    {
        return true;
    }

    // Reconstruction of the CDRMessage_t
    CDRMessage_t cdrMessage(total_bytes);
    size_t n_bytes = 0;
    for (auto it = buffers.begin(); it != buffers.end(); ++it)
    {
        memcpy(&cdrMessage.buffer[n_bytes], it->buffer, it->size);
        n_bytes += it->size;
    }
    assert(total_bytes == n_bytes);
    cdrMessage.length = total_bytes;

    if (cdrMessage.length < RTPSMESSAGE_HEADER_SIZE)
    {
        return false;
    }

    if (cdrMessage.buffer[cdrMessage.pos++] != 'R' ||
            cdrMessage.buffer[cdrMessage.pos++] != 'T' ||
            cdrMessage.buffer[cdrMessage.pos++] != 'P' ||
            cdrMessage.buffer[cdrMessage.pos++] != 'S')
    {
        return false;
    }

    cdrMessage.pos += 4 + 12; // RTPS version + GUID

    SubmessageHeader_t cdrSubMessageHeader;
    while (cdrMessage.pos < cdrMessage.length)
    {
        if (sub_messages_filter_(cdrMessage))
        {
            return true;
        }
        ReadSubmessageHeader(cdrMessage, cdrSubMessageHeader);
        if (cdrMessage.pos + cdrSubMessageHeader.submessageLength > cdrMessage.length)
        {
            return false;
        }

        SequenceNumber_t sequence_number{SequenceNumber_t::unknown()};
        EntityId_t writer_id;
        auto old_pos = cdrMessage.pos;

        switch (cdrSubMessageHeader.submessageId)
        {
            case fastdds::rtps::DATA:
                // Get WriterID.
                cdrMessage.pos += 8;
                fastdds::rtps::CDRMessage::readEntityId(&cdrMessage, &writer_id);
                fastdds::rtps::CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                fastdds::rtps::CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;

                if (writer_id == fastdds::rtps::c_EntityId_SPDPWriter)
                {
                    if (test_transport_options->always_drop_participant_builtin_topic_data)
                    {
                        return true;
                    }
                    else if (drop_participant_builtin_topic_data_)
                    {
                        return true;
                    }
                    else if (should_be_dropped(&drop_participant_builtin_data_messages_percentage_))
                    {
                        return true;
                    }
                    else if (drop_builtin_data_messages_filter_(cdrMessage))
                    {
                        return true;
                    }
                }
                else if (writer_id == fastdds::rtps::c_EntityId_SEDPPubWriter)
                {
                    if (drop_publication_builtin_topic_data_)
                    {
                        return true;
                    }
                    else if (should_be_dropped(&drop_publication_builtin_data_messages_percentage_))
                    {
                        return true;
                    }
                    else if (drop_builtin_data_messages_filter_(cdrMessage))
                    {
                        return true;
                    }
                }
                else if (writer_id == fastdds::rtps::c_EntityId_SEDPSubWriter)
                {
                    if (drop_subscription_builtin_topic_data_)
                    {
                        return true;
                    }
                    else if (should_be_dropped(&drop_subscription_builtin_data_messages_percentage_))
                    {
                        return true;
                    }
                    else if (drop_builtin_data_messages_filter_(cdrMessage))
                    {
                        return true;
                    }
                }
                else
                {
                    if (should_be_dropped(&drop_data_messages_percentage_))
                    {
                        return true;
                    }
                    if (drop_data_messages_filter_(cdrMessage))
                    {
                        return true;
                    }
                }

                break;

            case fastdds::rtps::ACKNACK:
                if (should_be_dropped(&drop_ack_nack_messages_percentage_))
                {
                    return true;
                }
                if (drop_ack_nack_messages_filter_(cdrMessage))
                {
                    return true;
                }

                break;

            case fastdds::rtps::HEARTBEAT:
                cdrMessage.pos += 8;
                fastdds::rtps::CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                fastdds::rtps::CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;
                if (should_be_dropped(&drop_heartbeat_messages_percentage_))
                {
                    return true;
                }
                if (drop_heartbeat_messages_filter_(cdrMessage))
                {
                    return true;
                }

                break;

            case fastdds::rtps::DATA_FRAG:
                if (should_be_dropped(&drop_data_frag_messages_percentage_))
                {
                    return true;
                }
                if (drop_data_frag_messages_filter_(cdrMessage))
                {
                    return true;
                }

                break;

            case fastdds::rtps::GAP:
                cdrMessage.pos += 8;
                fastdds::rtps::CDRMessage::readInt32(&cdrMessage, &sequence_number.high);
                fastdds::rtps::CDRMessage::readUInt32(&cdrMessage, &sequence_number.low);
                cdrMessage.pos = old_pos;
                if (should_be_dropped(&drop_gap_messages_percentage_))
                {
                    return true;
                }
                if (drop_gap_messages_filter_(cdrMessage))
                {
                    return true;
                }

                break;
        }

        if (sequence_number != SequenceNumber_t::unknown() &&
                find(sequence_number_data_messages_to_drop_.begin(),
                sequence_number_data_messages_to_drop_.end(),
                sequence_number) != sequence_number_data_messages_to_drop_.end())
        {
            return true;
        }

        cdrMessage.pos += cdrSubMessageHeader.submessageLength;
    }

    if (should_be_dropped(&percentage_of_messages_to_drop_))
    {
        return true;
    }
    if (messages_filter_(cdrMessage))
    {
        return true;
    }

    return false;
}

bool test_UDPv4Transport::log_drop(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t size)
{
    static_cast<void>(size);
    if (test_transport_options->test_UDPv4Transport_DropLog.size() <
            test_transport_options->test_UDPv4Transport_DropLogLength)
    {
        vector<octet> message;
        for (const auto& buf: buffers)
        {
            auto byte_data = static_cast<const octet*>(buf.buffer);
            message.insert(message.end(), byte_data, byte_data + buf.size);
        }
        assert(message.size() == size);
        test_transport_options->test_UDPv4Transport_DropLog.push_back(message);
        return true;
    }

    return false;
}

bool test_UDPv4Transport::should_be_dropped(
        PercentageData* percent)
{
    percent->accumulator += percent->percentage.load();
    if (percent->accumulator >= 100u)
    {
        percent->accumulator -= 100u;
        return true;
    }

    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
