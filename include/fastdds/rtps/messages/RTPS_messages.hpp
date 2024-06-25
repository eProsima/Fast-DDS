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

/**
 * @file RTPS_messages.hpp
 */

#ifndef FASTDDS_RTPS_MESSAGES__RTPS_MESSAGES_HPP
#define FASTDDS_RTPS_MESSAGES__RTPS_MESSAGES_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <iostream>
#include <bitset>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

// //!@brief Enumeration of the different Submessages types
enum SubmessageId : uint8_t
{
    PAD             = 0x01,
    ACKNACK         = 0x06,
    HEARTBEAT       = 0x07,
    GAP             = 0x08,
    INFO_TS         = 0x09,
    INFO_SRC        = 0x0c,
    INFO_REPLY_IP4  = 0x0d,
    INFO_DST        = 0x0e,
    INFO_REPLY      = 0x0f,
    NACK_FRAG       = 0x12,
    HEARTBEAT_FRAG  = 0x13,
    DATA            = 0x15,
    DATA_FRAG       = 0x16
};

//!@brief Structure Header_t, RTPS Message Header Structure.
//!@ingroup COMMON_MODULE
struct Header_t
{
    //!Protocol version
    ProtocolVersion_t version;
    //!Vendor ID
    fastdds::rtps::VendorId_t vendorId;
    //!GUID prefix
    GuidPrefix_t guidPrefix;
    Header_t()
        : version(c_ProtocolVersion)
        , vendorId(c_VendorId_eProsima)
    {
    }

    ~Header_t()
    {
    }

};

/**
 * @param output
 * @param h
 * @return
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const Header_t& h)
{
    output << "RTPS HEADER of Version: " << (int)h.version.m_major << "." << (int)h.version.m_minor;
    output << "  || VendorId: " << std::hex << (int)h.vendorId[0] << "." << (int)h.vendorId[1] << std::dec;
    output << "GuidPrefix: " << h.guidPrefix;
    return output;
}

//!@brief Structure SubmessageHeader_t, used to contain the header information of a submessage.
struct SubmessageHeader_t
{
    octet submessageId;
    uint32_t submessageLength;
    SubmessageFlag flags;
    bool is_last;

    SubmessageHeader_t()
        : submessageId(0)
        , submessageLength(0)
        , flags(0)
        , is_last(false)
    {
    }

};

using std::cout;
using std::endl;
using std::bitset;

/**
 * @param output
 * @param sh
 * @return
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const SubmessageHeader_t& sh)
{
    output << "Submessage Header, ID: " << std::hex << (int)sh.submessageId << std::dec;
    output << " length: " << (int)sh.submessageLength << " flags " << (bitset<8>)sh.flags;
    return output;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima


#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_MESSAGES__RTPS_MESSAGES_HPP
