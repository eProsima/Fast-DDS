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


#ifndef _FASTDDS_PUBLISHERQOS_HPP_
#define _FASTDDS_PUBLISHERQOS_HPP_

#include <fastrtps/qos/QosPolicies.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class PublisherQos, containing all the possible Qos that can be set for a determined Publisher.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been
 * implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class PublisherQos
{
public:

    RTPS_DllAPI PublisherQos();

    RTPS_DllAPI virtual ~PublisherQos();

    bool operator==(
            const PublisherQos& b) const
    {
        return (this->durability == b.durability) &&
               (this->durability_service == b.durability_service) &&
               (this->deadline == b.deadline) &&
               (this->latency_budget == b.latency_budget) &&
               (this->liveliness == b.liveliness) &&
               (this->reliability == b.reliability) &&
               (this->lifespan == b.lifespan) &&
               (this->user_data == b.user_data) &&
               (this->time_based_filter == b.time_based_filter) &&
               (this->ownership == b.ownership) &&
               (this->ownership_strength == b.ownership_strength) &&
               (this->destination_order == b.destination_order) &&
               (this->presentation == b.presentation) &&
               (this->partition == b.partition) &&
               (this->topic_data == b.topic_data) &&
               (this->group_data == b.group_data) &&
               (this->publish_mode == b.publish_mode) &&
               (this->disable_positive_acks == b.disable_positive_acks);
    }

    //!Durability Qos, implemented in the library.
    fastrtps::DurabilityQosPolicy durability;

    //!Durability Service Qos, NOT implemented in the library.
    fastrtps::DurabilityServiceQosPolicy durability_service;

    //!Deadline Qos, implemented in the library.
    fastrtps::DeadlineQosPolicy deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    fastrtps::LatencyBudgetQosPolicy latency_budget;

    //!Liveliness Qos, implemented in the library.
    fastrtps::LivelinessQosPolicy liveliness;

    //!Reliability Qos, implemented in the library.
    fastrtps::ReliabilityQosPolicy reliability;

    //!Lifespan Qos, NOT implemented in the library.
    fastrtps::LifespanQosPolicy lifespan;

    //!UserData Qos, NOT implemented in the library.
    UserDataQosPolicy user_data;

    //!Time Based Filter Qos, NOT implemented in the library.
    fastrtps::TimeBasedFilterQosPolicy time_based_filter;

    //!Ownership Qos, NOT implemented in the library.
    fastrtps::OwnershipQosPolicy ownership;

    //!Owenership Strength Qos, NOT implemented in the library.
    fastrtps::OwnershipStrengthQosPolicy ownership_strength;

    //!Destination Order Qos, NOT implemented in the library.
    fastrtps::DestinationOrderQosPolicy destination_order;

    //!Presentation Qos, NOT implemented in the library.
    fastrtps::PresentationQosPolicy presentation;

    //!Partition Qos, implemented in the library.
    fastrtps::PartitionQosPolicy partition;

    //!Topic Data Qos, NOT implemented in the library.
    fastrtps::TopicDataQosPolicy topic_data;

    //!Group Data Qos, NOT implemented in the library.
    fastrtps::GroupDataQosPolicy group_data;

    //!Publication Mode Qos, implemented in the library.
    fastrtps::PublishModeQosPolicy publish_mode;

    //!Disable positive acks QoS, implemented in the library.
    fastrtps::DisablePositiveACKsQosPolicy disable_positive_acks;

    /**
     * Set Qos from another class
     * @param qos Reference from a PublisherQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void set_qos(
            const PublisherQos& qos,
            bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool check_qos() const;

    RTPS_DllAPI bool can_qos_be_updated(
            const PublisherQos& qos) const;
};

RTPS_DllAPI extern const PublisherQos PUBLISHER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_PUBLISHERQOS_HPP_
