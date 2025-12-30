// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IRTPSMessageGroupLimitation.h
 *
 */

#ifndef FASTDDS_RTPS_MESSAGES__IRTPSMESSAGEGROUPLIMITATION_HPP
#define FASTDDS_RTPS_MESSAGES__IRTPSMESSAGEGROUPLIMITATION_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CacheChange_t;
class RTPSMessageSenderInterface;

/*!
 * Interface to implement mechanisms that limit the data sent by an RTPSMessageGroup under certain conditions,
 * taking into account the bytes already sent by said group.
 */
class IRTPSMessageGroupLimitation
{
public:

    /*!
     * RTPSMessageGroup uses this method to announce the number of bytes just sent by the group.
     *
     * @param bytes Number of bytes just sent by the group.
     * @param sender RTPSMessageSenderInterface used for delivering sent bytes.
     */
    virtual void add_sent_bytes_by_group(
            uint32_t bytes,
            RTPSMessageSenderInterface& sender) = 0;

    /*!
     * RTPSMessageGroup uses this method to query whether adding a new change to the group would exceed the
     * limitation or not.
     *
     * @param change Change to be added.
     * @param size_to_add Size in bytes that adding the change would imply.
     * @param pending_to_send Number of bytes pending to be sent by the group.
     * @param sender RTPSMessageSenderInterface that the group will use for data delivery.
     *
     * @return True if adding the change would exceed the limitation, false otherwise.
     */
    virtual bool data_exceeds_limitation(
            CacheChange_t& change,
            uint32_t size_to_add,
            uint32_t pending_to_send,
            RTPSMessageSenderInterface& sender) = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_MESSAGES__IRTPSMESSAGEGROUPLIMITATION_HPP

