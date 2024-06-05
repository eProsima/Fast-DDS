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

/*
 * @file CDRMessageCreator.cpp
 *
 */

#include <rtps/messages/RTPSMessageCreator.hpp>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

bool RTPSMessageCreator::addHeader(
        CDRMessage_t* msg,
        const GuidPrefix_t& guidPrefix,
        const ProtocolVersion_t& version,
        const fastdds::rtps::VendorId_t& vendorId)
{
    CDRMessage::addOctet(msg, 'R');
    CDRMessage::addOctet(msg, 'T');
    CDRMessage::addOctet(msg, 'P');
    CDRMessage::addOctet(msg, 'S');

    CDRMessage::addOctet(msg, version.m_major);
    CDRMessage::addOctet(msg, version.m_minor);

    CDRMessage::addOctet(msg, vendorId[0]);
    CDRMessage::addOctet(msg, vendorId[1]);

    CDRMessage::addData(msg, guidPrefix.value, 12);
    msg->length = msg->pos;

    return true;
}

bool RTPSMessageCreator::addHeader(
        CDRMessage_t* msg,
        const GuidPrefix_t& guidPrefix)
{
    return RTPSMessageCreator::addHeader(msg, guidPrefix, c_ProtocolVersion, c_VendorId_eProsima);
}

bool RTPSMessageCreator::addCustomContent(
        CDRMessage_t* msg,
        const octet* content,
        const size_t contentSize)
{
    CDRMessage::addData(msg, content, static_cast<uint32_t>(contentSize));
    msg->length = msg->pos;

    return true;
}

bool RTPSMessageCreator::addSubmessageHeader(
        CDRMessage_t* msg,
        octet id,
        octet flags,
        uint16_t size)
{
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    CDRMessage::addOctet(msg, id);
    CDRMessage::addOctet(msg, flags);
    CDRMessage::addUInt16(msg, size);
    msg->length = msg->pos;

    return true;
}

bool RTPSMessageCreator::addSubmessageInfoTS(
        CDRMessage_t* msg,
        const Time_t& time,
        bool invalidateFlag)
{
    octet flags = 0x0;
    uint16_t size = 8;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian  = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    if (invalidateFlag)
    {
        flags = flags | BIT(1);
        size = 0;
    }

    CDRMessage::addOctet(msg, INFO_TS);
    CDRMessage::addOctet(msg, flags);
    CDRMessage::addUInt16(msg, size);
    if (!invalidateFlag)
    {
        CDRMessage::addInt32(msg, time.seconds());
        CDRMessage::addUInt32(msg, time.fraction());
    }

    return true;
}

bool RTPSMessageCreator::addSubmessageInfoSRC(
        CDRMessage_t* msg,
        const ProtocolVersion_t& version,
        const fastdds::rtps::VendorId_t& vendorId,
        const GuidPrefix_t& guidPrefix)
{
    octet flags = 0x0;
    uint16_t size = 20;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    CDRMessage::addOctet(msg, INFO_SRC);
    CDRMessage::addOctet(msg, flags);
    CDRMessage::addUInt16(msg, size);
    CDRMessage::addUInt32(msg, 0);
    CDRMessage::addOctet(msg, version.m_major);
    CDRMessage::addOctet(msg, version.m_minor);

    CDRMessage::addOctet(msg, vendorId[0]);
    CDRMessage::addOctet(msg, vendorId[1]);

    CDRMessage::addData(msg, guidPrefix.value, 12);

    return true;
}

bool RTPSMessageCreator::addSubmessageInfoDST(
        CDRMessage_t* msg,
        const GuidPrefix_t& guidP)
{
    octet flags = 0x0;
    uint16_t size = 12;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian  = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    CDRMessage::addOctet(msg, INFO_DST);
    CDRMessage::addOctet(msg, flags);
    CDRMessage::addUInt16(msg, size);
    CDRMessage::addData(msg, guidP.value, 12);

    return true;
}

bool RTPSMessageCreator::addSubmessageInfoTS_Now(
        CDRMessage_t* msg,
        bool invalidateFlag)
{
    Time_t time_now;
    Time_t::now(time_now);
    return RTPSMessageCreator::addSubmessageInfoTS(msg, time_now, invalidateFlag);
}

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */


#include <rtps/messages/submessages/DataMsg.hpp>
#include <rtps/messages/submessages/HeartbeatMsg.hpp>
#include <rtps/messages/submessages/AckNackMsg.hpp>
#include <rtps/messages/submessages/GapMsg.hpp>
