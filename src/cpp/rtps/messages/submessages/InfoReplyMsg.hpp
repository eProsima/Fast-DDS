// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * InfoReplyMsg.hpp
 *
 */

namespace eprosima {
namespace fastrtps {
namespace rtps {

static void add_locator_udp4(
        CDRMessage_t* msg,
        const Locator_t& locator)
{
    uint32_t address =
        static_cast<uint32_t>(locator.address[3]) << 24 |
        static_cast<uint32_t>(locator.address[2]) << 16 |
        static_cast<uint32_t>(locator.address[1]) << 8 |
        static_cast<uint32_t>(locator.address[0]);

    CDRMessage::addUInt32(msg, address);
    CDRMessage::addUInt32(msg, locator.port);
}

static bool addSubmessageInfoReplyV4(
        CDRMessage_t* msg,
        const Locator_t& unicast_locator,
        const Locator_t& multicast_locator)
{
    // Calculate total submessage size
    bool has_multicast = false;
    uint16_t submessage_size = 2 * sizeof(uint32_t);
    if (LOCATOR_KIND_UDPv4 == multicast_locator.kind)
    {
        has_multicast = true;
        submessage_size *= 2;
    }

    // Check if submessage fits into message
    if (msg->pos + submessage_size + 4 > msg->max_size)
    {
        return false;
    }

    octet flags = 0x0;
    Endianness_t old_endianess = msg->msg_endian;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    if (has_multicast)
    {
        flags = flags | BIT(1);
    }

    // Submessage header.
    CDRMessage::addOctet(msg, INFO_REPLY_IP4);
    CDRMessage::addOctet(msg, flags);
    CDRMessage::addUInt16(msg, submessage_size);

    add_locator_udp4(msg, unicast_locator);
    if (has_multicast)
    {
        add_locator_udp4(msg, multicast_locator);
    }

    msg->msg_endian = old_endianess;
    return true;
}

bool RTPSMessageCreator::addSubmessageInfoReply(
        CDRMessage_t* msg,
        const LocatorList_t& unicast_locators,
        const LocatorList_t& multicast_locators)
{
    bool compact_unicast = (unicast_locators.size() == 1) && (LOCATOR_KIND_UDPv4 == unicast_locators.begin()->kind);
    bool compact_version = compact_unicast && (multicast_locators.size() <= 1);
    if ( (multicast_locators.size() == 1) && (LOCATOR_KIND_UDPv4 != multicast_locators.begin()->kind))
    {
        compact_version = false;
    }

    if (compact_version)
    {
        Locator_t multicast_locator;
        if (multicast_locators.size() == 1)
        {
            multicast_locator = *multicast_locators.begin();
        }
        return addSubmessageInfoReplyV4(msg, *unicast_locators.begin(), multicast_locator);
    }

    return false;
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
