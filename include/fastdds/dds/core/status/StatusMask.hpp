/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FASTDDS_STATUSMASK_HPP_
#define _FASTDDS_STATUSMASK_HPP_

#include <bitset>
#include <sstream>

//!Alias of size_t(16)
#define FASTDDS_STATUS_COUNT size_t(16)

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief
 * StatusMask is a bitmap or bitset field.
 *
 * This bitset is used to:
 * - determine which listener functions to call
 * - set conditions in dds::core::cond::StatusCondition
 * - indicate status changes when calling dds::core::Entity::status_changes
 */
class StatusMask : public std::bitset<FASTDDS_STATUS_COUNT>
{
public:

    /**
     * Convenience typedef for std::bitset<FASTDDS_STATUS_COUNT>.
     */
    typedef std::bitset<FASTDDS_STATUS_COUNT> MaskType;

    /**
     * Construct an StatusMask with no flags set.
     */
    StatusMask()
        : std::bitset<FASTDDS_STATUS_COUNT>()
    {
    }

    /**
     * Construct an StatusMask with an uint32_t bit mask.
     *
     * @param mask the bit array to initialize the bitset with
     */
    explicit StatusMask(
            uint32_t mask)
        : std::bitset<FASTDDS_STATUS_COUNT>(mask)
    {
    }

    /**
     * Add given StatusMask bits into this StatusMask bitset.
     *
     * @return StatusMask this
     */
    inline StatusMask& operator <<(
            const StatusMask& mask)
    {
        *this |= mask;
        return *this;
    }

    /**
     * Remove given StatusMask bits into this StatusMask bitset.
     *
     * @return StatusMask this
     */
    inline StatusMask& operator >>(
            const StatusMask& mask)
    {
        *this &= ~mask;
        return *this;
    }

    /**
     * Get all StatusMasks
     *
     * @return StatusMask all
     */
    inline static StatusMask all()
    {
        return StatusMask(0x80007fe7u);
    }

    /**
     * Get no StatusMasks
     *
     * @return StatusMask none
     */
    inline static StatusMask none()
    {
        return StatusMask(0u);
    }

public:

    /**
     * Get the StatusMask associated with dds::core::status::InconsistentTopicStatus
     *
     * @return StatusMask inconsistent_topic
     */
    inline static StatusMask inconsistent_topic()
    {
        return StatusMask(0x00000001 << 0u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::OfferedDeadlineMissedStatus
     *
     * @return StatusMask offered_deadline_missed
     */
    inline static StatusMask offered_deadline_missed()
    {
        return StatusMask(0x00000001 << 1u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::RequestedDeadlineMissedStatus
     *
     * @return StatusMask requested_deadline_missed
     */
    inline static StatusMask requested_deadline_missed()
    {
        return StatusMask(0x00000001 << 2u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::OfferedIncompatibleQosStatus
     *
     * @return StatusMask offered_incompatible_qos
     */
    inline static StatusMask offered_incompatible_qos()
    {
        return StatusMask(0x00000001 << 5u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::RequestedIncompatibleQosStatus
     *
     * @return StatusMask requested_incompatible_qos
     */
    inline static StatusMask requested_incompatible_qos()
    {
        return StatusMask(0x00000001 << 6u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::SampleLostStatus
     *
     * @return StatusMask sample_lost
     */
    inline static StatusMask sample_lost()
    {
        return StatusMask(0x00000001 << 7u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::SampleRejectedStatus
     *
     * @return StatusMask sample_rejected
     */
    inline static StatusMask sample_rejected()
    {
        return StatusMask(0x00000001 << 8u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::data_on_readers
     *
     * @return StatusMask data_on_readers
     */
    inline static StatusMask data_on_readers()
    {
        return StatusMask(0x00000001 << 9u);
    }

    /**
     * get the statusmask associated with dds::core::status::data_available
     *
     * @return statusmask data_available
     */
    inline static StatusMask data_available()
    {
        return StatusMask(0x00000001 << 10u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::LivelinessLostStatus
     *
     * @return StatusMask liveliness_lost
     */
    inline static StatusMask liveliness_lost()
    {
        return StatusMask(0x00000001 << 11u);
    }

    /**
     * Get the StatusMask associated with dds::core::status::LivelinessChangedStatus
     *
     * @return StatusMask liveliness_changed
     */
    inline static StatusMask liveliness_changed()
    {
        return StatusMask(0x00000001 << 12u);
    }

    /**
     * Get the statusmask associated with dds::core::status::PublicationMatchedStatus
     *
     * @return StatusMask publication_matched
     */
    inline static StatusMask publication_matched()
    {
        return StatusMask(0x00000001 << 13u);
    }

    /**
     * Get the statusmask associated with dds::core::status::SubscriptionMatchedStatus
     *
     * @return StatusMask subscription_matched
     */
    inline static StatusMask subscription_matched()
    {
        return StatusMask(0x00000001 << 14u);
    }

    /**
     * @brief Checks if the status passed as parameter is 1 in the actual StatusMask
     * @param status Status that need to be checked
     * @return true if the status is active and false if not
     */
    bool is_active(
            StatusMask status) const
    {
        MaskType r = *this & status;
        return r == status;
    }

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima


#endif //_FASTDDS_STATUSMASK_HPP_
