// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef RTPS_WRITER__LOCATORSELECTORSENDER_HPP
#define RTPS_WRITER__LOCATORSELECTORSENDER_HPP

#include <vector>

#include <fastdds/rtps/common/LocatorSelector.hpp>
#include <fastdds/rtps/messages/RTPSMessageSenderInterface.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseWriter;

/*!
 * Class used by writers to inform a RTPSMessageGroup object which remote participants will be addressees of next RTPS
 * submessages.
 */
class LocatorSelectorSender : public RTPSMessageSenderInterface
{
public:

    LocatorSelectorSender(
            BaseWriter& writer,
            ResourceLimitedContainerConfig matched_readers_allocation
            )
        : locator_selector(matched_readers_allocation)
        , all_remote_readers(matched_readers_allocation)
        , all_remote_participants(matched_readers_allocation)
        , writer_(writer)
    {
    }

    bool destinations_have_changed() const override
    {
        return false;
    }

    /*!
     * Get a GUID prefix representing all destinations.
     *
     * @return If only one remote participant is an addressee, return its GUIDPrefix_t. c_GuidPrefix_Unknown otherwise.
     */
    GuidPrefix_t destination_guid_prefix() const override
    {
        return all_remote_participants.size() == 1 ? all_remote_participants.at(0) : c_GuidPrefix_Unknown;
    }

    /*!
     * Get the GUID prefix of all the destination participants.
     *
     * @return a const reference to a vector with the GUID prefix of all destination participants.
     */
    const std::vector<GuidPrefix_t>& remote_participants() const override
    {
        return all_remote_participants;
    }

    /*!
     * Get the GUID of all destinations.
     *
     * @return a const reference to a vector with the GUID of all destinations.
     */
    const std::vector<GUID_t>& remote_guids() const override
    {
        return all_remote_readers;
    }

    /*!
     * Send a message through this interface.
     *
     * @param buffers Vector of NetworkBuffers to send with data already serialized.
     * @param total_bytes Total number of bytes to send. Should be equal to the sum of the @c size field of all buffers.
     * @param max_blocking_time_point Future timepoint where blocking send should end.
     */
    bool send(
            const std::vector<fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            std::chrono::steady_clock::time_point max_blocking_time_point) const override;

    /*!
     * Lock the object.
     *
     * This kind of object needs to be locked because could be used outside the writer's mutex.
     */
    void lock() override
    {
        mutex_.lock();
    }

    /*!
     * Unlock the object.
     */
    void unlock() override
    {
        mutex_.unlock();
    }

    /*!
     * Try to lock the object.
     *
     * This kind of object needs to be locked because could be used outside the writer's mutex.
     */
    template <class Clock, class Duration>
    bool try_lock_until(
            const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return mutex_.try_lock_until(abs_time);
    }

    fastdds::rtps::LocatorSelector locator_selector;

    ResourceLimitedVector<GUID_t> all_remote_readers;

    ResourceLimitedVector<GuidPrefix_t> all_remote_participants;

private:

    BaseWriter& writer_;

    RecursiveTimedMutex mutex_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // RTPS_WRITER__LOCATORSELECTORSENDER_HPP
