// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSGapBuilder.hpp
 *
 */

#ifndef FASTDDS_RTPS_MESSAGES__RTPSGAPBUILDER_HPP
#define FASTDDS_RTPS_MESSAGES__RTPSGAPBUILDER_HPP

#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <rtps/messages/RTPSMessageGroup.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * A helper class to add GAP messages to a @ref RTPSMessageGroup.
 * @ingroup WRITER_MODULE
 */
class RTPSGapBuilder
{
public:

    /**
     * RTPSGapBuilder constructor.
     *
     * @param group Reference to the @ref RTPSMessageGroup that will be used to send GAP messages.
     */
    explicit RTPSGapBuilder(
            RTPSMessageGroup& /*group*/)
    {
    }

    /**
     * RTPSGapBuilder constructor.
     *
     * @param group Reference to the @ref RTPSMessageGroup that will be used to send GAP messages.
     * @param reader_guid Specific destination reader guid
     */
    explicit RTPSGapBuilder(
            RTPSMessageGroup& /*group*/,
            const GUID_t& /*reader_guid*/)
    {
    }

    ~RTPSGapBuilder()
    {
    }

    /**
     * Adds a sequence number to the GAP list.
     *
     * @remark Sequence numbers should be added in strict increasing order.
     *
     * @param gap_sequence Sequence number to be added to the GAP list.
     * @return false if a GAP message couldn't be added to the message group,
     *         true if no GAP message was needed or it was successfully added.
     *
     * @throws RTPSMessageGroup::timeout if a network operation was necessary and
     *         it blocked for more than the maximum time allowed.
     */
    MOCK_METHOD1(add, bool(const SequenceNumber_t& gap_sequence));

    /**
     * Adds a GAP message to the message group if necessary.
     *
     * @return false if a GAP message couldn't be added to the message group,
     *         true if no GAP message was needed or it was successfully added.
     *
     * @throws RTPSMessageGroup::timeout if a network operation was necessary and
     *         it blocked for more than the maximum time allowed.
     */
    MOCK_METHOD0(flush, bool());
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_MESSAGES__RTPSGAPBUILDER_HPP
