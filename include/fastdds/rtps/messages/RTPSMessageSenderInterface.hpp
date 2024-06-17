// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSMessageSenderInterface.hpp
 *
 */

#ifndef FASTDDS_RTPS_MESSAGES__RTPSMESSAGESENDERINTERFACE_HPP
#define FASTDDS_RTPS_MESSAGES__RTPSMESSAGESENDERINTERFACE_HPP

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <chrono>
#include <vector>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CDRMessage_t;

/**
 * Interface to handle destinations management and message sending.
 */
class RTPSMessageSenderInterface
{
public:

    virtual ~RTPSMessageSenderInterface() = default;

    /**
     * Check if the destinations managed by this sender interface have changed.
     *
     * @return true if destinations have changed, false otherwise.
     */
    virtual bool destinations_have_changed() const = 0;

    /**
     * Get a GUID prefix representing all destinations.
     *
     * @return When all the destinations share the same prefix (i.e. belong to the same participant)
     * that prefix is returned. When there are no destinations, or they belong to different
     * participants, c_GuidPrefix_Unknown is returned.
     */
    virtual GuidPrefix_t destination_guid_prefix() const = 0;

    /**
     * Get the GUID prefix of all the destination participants.
     *
     * @return a const reference to a vector with the GUID prefix of all destination participants.
     */
    virtual const std::vector<GuidPrefix_t>& remote_participants() const = 0;

    /**
     * Get the GUID of all destinations.
     *
     * @return a const reference to a vector with the GUID of all destinations.
     */
    virtual const std::vector<GUID_t>& remote_guids() const = 0;

    /**
     * Send a message through this interface.
     *
     * @param buffers Vector of NetworkBuffers to send with data already serialized.
     * @param total_bytes Total number of bytes to send. Should be equal to the sum of the @c size field of all buffers.
     * @param max_blocking_time_point Future timepoint where blocking send should end.
     */
    virtual bool send(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            std::chrono::steady_clock::time_point max_blocking_time_point) const = 0;

    /*!
     * Lock the object.
     */
    virtual void lock() = 0;

    /*!
     * Lock the object.
     */
    virtual void unlock() = 0;


};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // FASTDDS_RTPS_MESSAGES__RTPSMESSAGESENDERINTERFACE_HPP
