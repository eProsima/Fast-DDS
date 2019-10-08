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
 * @file RTPSParticipantAllocationAttributes.hpp
 */

#ifndef _FASTDDS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_
#define _FASTDDS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_

#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @brief Holds limits for collections of remote locators.
 */
struct RemoteLocatorsAllocationAttributes
{
    /** Maximum number of unicast locators per remote entity.
     *
     * This attribute controls the maximum number of unicast locators to keep for
     * each discovered remote entity (be it a participant, reader of writer). It is
     * recommended to use the highest number of local addresses found on all the systems
     * belonging to the same domain as this participant.
     */
    size_t max_unicast_locators = 4u;

    /** Maximum number of multicast locators per remote entity.
     *
     * This attribute controls the maximum number of multicast locators to keep for
     * each discovered remote entity (be it a participant, reader of writer). The
     * default value of 1 is usually enough, as it doesn't make sense to add more
     * than one multicast locator per entity.
     */
    size_t max_multicast_locators = 1u;
};

/**
 * @brief Holds allocation limits affecting collections managed by a participant.
 */
struct RTPSParticipantAllocationAttributes
{
    //! Holds limits for collections of remote locators.
    RemoteLocatorsAllocationAttributes locators;
    //! Defines the allocation behaviour for collections dependent on the total number of participants.
    ResourceLimitedContainerConfig participants;
    //! Defines the allocation behaviour for collections dependent on the total number of readers per participant.
    ResourceLimitedContainerConfig readers;
    //! Defines the allocation behaviour for collections dependent on the total number of writers per participant.
    ResourceLimitedContainerConfig writers;

    //! @return the allocation config for the total of readers in the system (participants * readers)
    ResourceLimitedContainerConfig total_readers() const
    {
        return total_endpoints(readers);
    }

    //! @return the allocation config for the total of writers in the system (participants * writers)
    ResourceLimitedContainerConfig total_writers() const
    {
        return total_endpoints(writers);
    }

private:
    ResourceLimitedContainerConfig total_endpoints(const ResourceLimitedContainerConfig& endpoints) const
    {
        constexpr size_t max = std::numeric_limits<size_t>::max();
        size_t initial;
        size_t maximum;
        size_t increment;

        initial = participants.initial * endpoints.initial;
        maximum = (participants.maximum == max || endpoints.maximum == max)
            ? max : participants.maximum * endpoints.maximum;
        increment = std::max(participants.increment, endpoints.increment);

        return { initial, maximum, increment };
    }
};

const RTPSParticipantAllocationAttributes c_default_RTPSParticipantAllocationAttributes
        = RTPSParticipantAllocationAttributes();

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_ */
