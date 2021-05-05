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
 * @file RTPSStatisticsMessages.hpp
 */

#ifndef _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_
#define _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_

#include <cstdint>

#include <fastdds/rtps/common/CDRMessage_t.h>

#define FASTDDS_STATISTICS_NETWORK_SUBMESSAGE 0x80

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

constexpr uint16_t statistics_submessage_data_length = 8 /* timestamp */ + 8 /* sequence number */;
constexpr uint16_t statistics_submessage_length =
        RTPSMESSAGE_SUBMESSAGEHEADER_SIZE + // submessage header
        statistics_submessage_data_length;  // submessage data

inline void add_statistics_submessage(
        eprosima::fastrtps::rtps::CDRMessage_t* msg)
{
#ifdef FASTDDS_STATISTICS
    using namespace eprosima::fastrtps::rtps;
    RTPSMessageCreator::addSubmessageHeader(
        msg, FASTDDS_STATISTICS_NETWORK_SUBMESSAGE, 0x00, statistics_submessage_data_length);
    Time_t ts;
    Time_t::now(ts);
    CDRMessage::addInt32(msg, ts.seconds());
    CDRMessage::addUInt32(msg, ts.fraction());
    CDRMessage::addUInt64(msg, 0);
#endif // FASTDDS_STATISTICS
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif  // _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_
