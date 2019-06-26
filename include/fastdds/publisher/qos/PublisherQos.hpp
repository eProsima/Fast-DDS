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
 * @file PublisherQos.hpp
 */


#ifndef _FASTDDS_PUBLISHERQOS_HPP
#define _FASTDDS_PUBLISHERQOS_HPP

#include "../../../fastrtps/qos/QosPolicies.h"

namespace eprosima {
namespace fastdds {

/**
 * Class PublisherQos, containing all the possible Qos that can be set for a determined Publisher.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class PublisherQos
{
public:
    RTPS_DllAPI PublisherQos();
    RTPS_DllAPI virtual ~PublisherQos();

    bool operator==(const PublisherQos& b) const
    {
        return (this->m_durability == b.m_durability) &&
               (this->m_durabilityService == b.m_durabilityService) &&
               (this->m_deadline == b.m_deadline) &&
               (this->m_latencyBudget == b.m_latencyBudget) &&
               (this->m_liveliness == b.m_liveliness) &&
               (this->m_reliability == b.m_reliability) &&
               (this->m_lifespan == b.m_lifespan) &&
               (this->m_userData == b.m_userData) &&
               (this->m_timeBasedFilter == b.m_timeBasedFilter) &&
               (this->m_ownership == b.m_ownership) &&
               (this->m_ownershipStrength == b.m_ownershipStrength) &&
               (this->m_destinationOrder == b.m_destinationOrder) &&
               (this->m_presentation == b.m_presentation) &&
               (this->m_partition == b.m_partition) &&
               (this->m_topicData == b.m_topicData) &&
               (this->m_groupData == b.m_groupData) &&
               (this->m_publishMode == b.m_publishMode) &&
               (this->m_disablePositiveACKs == b.m_disablePositiveACKs);
    }

    //!Durability Qos, implemented in the library.
    fastrtps::DurabilityQosPolicy m_durability;

    //!Durability Service Qos, NOT implemented in the library.
    fastrtps::DurabilityServiceQosPolicy m_durabilityService;

    //!Deadline Qos, implemented in the library.
    fastrtps::DeadlineQosPolicy m_deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    fastrtps::LatencyBudgetQosPolicy m_latencyBudget;

    //!Liveliness Qos, implemented in the library.
    fastrtps::LivelinessQosPolicy m_liveliness;

    //!Reliability Qos, implemented in the library.
    fastrtps::ReliabilityQosPolicy m_reliability;

    //!Lifespan Qos, NOT implemented in the library.
    fastrtps::LifespanQosPolicy m_lifespan;

    //!UserData Qos, NOT implemented in the library.
    fastrtps::UserDataQosPolicy m_userData;

    //!Time Based Filter Qos, NOT implemented in the library.
    fastrtps::TimeBasedFilterQosPolicy m_timeBasedFilter;

    //!Ownership Qos, NOT implemented in the library.
    fastrtps::OwnershipQosPolicy m_ownership;

    //!Owenership Strength Qos, NOT implemented in the library.
    fastrtps::OwnershipStrengthQosPolicy m_ownershipStrength;

    //!Destination Order Qos, NOT implemented in the library.
    fastrtps::DestinationOrderQosPolicy m_destinationOrder;

    //!Presentation Qos, NOT implemented in the library.
    fastrtps::PresentationQosPolicy m_presentation;

    //!Partition Qos, implemented in the library.
    fastrtps::PartitionQosPolicy m_partition;

    //!Topic Data Qos, NOT implemented in the library.
    fastrtps::TopicDataQosPolicy m_topicData;

    //!Group Data Qos, NOT implemented in the library.
    fastrtps::GroupDataQosPolicy m_groupData;

    //!Publication Mode Qos, implemented in the library.
    fastrtps::PublishModeQosPolicy m_publishMode;

    //!Disable positive acks QoS, implemented in the library.
    fastrtps::DisablePositiveACKsQosPolicy m_disablePositiveACKs;

    /**
     * Set Qos from another class
     * @param qos Reference from a PublisherQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void setQos(const PublisherQos& qos, bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool checkQos() const;

    RTPS_DllAPI bool canQosBeUpdated(const PublisherQos& qos) const;
};

} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_PUBLISHERQOS_HPP
