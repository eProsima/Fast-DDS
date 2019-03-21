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
 * @file RTPSParticipantAllocationAttributes.h
 */

#ifndef _FASTRTPS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_
#define _FASTRTPS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_

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
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTRTPS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_ */
