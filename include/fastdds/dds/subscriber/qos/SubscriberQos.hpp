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
 * @file SubscriberQos.hpp
 *
 */

#ifndef _FASTDDS_SUBSCRIBERQOS_HPP_
#define _FASTDDS_SUBSCRIBERQOS_HPP_

#include <fastrtps/qos/QosPolicies.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class SubscriberQos, contains all the possible Qos that can be set for a determined Subscriber.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class SubscriberQos
{
public:
    RTPS_DllAPI SubscriberQos()
    {}

    RTPS_DllAPI virtual ~SubscriberQos()
    {}

    bool operator==(
            const SubscriberQos& b) const
    {
        return (this->durability == b.durability) &&
               (this->deadline == b.deadline) &&
               (this->latency_budget == b.latency_budget) &&
               (this->liveliness == b.liveliness) &&
               (this->reliability == b.reliability) &&
               (this->ownership == b.ownership) &&
               (this->destination_order == b.destination_order) &&
               (this->user_data == b.user_data) &&
               (this->time_based_filter == b.time_based_filter) &&
               (this->presentation == b.presentation) &&
               (this->partition == b.partition) &&
               (this->topic_data == b.topic_data) &&
               (this->group_data == b.group_data) &&
               (this->durability_service == b.durability_service) &&
               (this->lifespan == b.lifespan) &&
               (this->disable_positive_acks == b.disable_positive_acks);
    }

    //!Durability Qos, implemented in the library.
    fastrtps::DurabilityQosPolicy durability;

    //!Deadline Qos, implemented in the library.
    fastrtps::DeadlineQosPolicy deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    fastrtps::LatencyBudgetQosPolicy latency_budget;

    //!Liveliness Qos, implemented in the library.
    fastrtps::LivelinessQosPolicy liveliness;

    //!ReliabilityQos, implemented in the library.
    fastrtps::ReliabilityQosPolicy reliability;

    //!Ownership Qos, NOT implemented in the library.
    fastrtps::OwnershipQosPolicy ownership;

    //!Destinatio Order Qos, NOT implemented in the library.
    fastrtps::DestinationOrderQosPolicy destination_order;

    //!UserData Qos, NOT implemented in the library.
    UserDataQosPolicy user_data;

    //!Time Based Filter Qos, NOT implemented in the library.
    fastrtps::TimeBasedFilterQosPolicy time_based_filter;

    //!Presentation Qos, NOT implemented in the library.
    fastrtps::PresentationQosPolicy presentation;

    //!Partition Qos, implemented in the library.
    fastrtps::PartitionQosPolicy partition;

    //!Topic Data Qos, NOT implemented in the library.
    fastrtps::TopicDataQosPolicy topic_data;

    //!GroupData Qos, NOT implemented in the library.
    fastrtps::GroupDataQosPolicy group_data;

    //!Durability Service Qos, NOT implemented in the library.
    fastrtps::DurabilityServiceQosPolicy durability_service;

    //!Lifespan Qos, NOT implemented in the library.
    fastrtps::LifespanQosPolicy lifespan;

    //!Data Representation Qos, implemented in the library.
    fastrtps::DataRepresentationQosPolicy representation;

    //!Type consistency enforcement Qos, NOT implemented in the library.
    fastrtps::TypeConsistencyEnforcementQosPolicy type_consistency;

    //!Disable positive ACKs QoS
    fastrtps::DisablePositiveACKsQosPolicy disable_positive_acks;

    /**
     * Set Qos from another class
     * @param subscriberqos Reference from a SubscriberQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void set_qos(
            const SubscriberQos& subscriberqos,
            bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool check_qos() const;

    /**
     * Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated.
     */
    RTPS_DllAPI bool can_qos_be_updated(
            const SubscriberQos& qos) const;
};

RTPS_DllAPI extern const SubscriberQos SUBSCRIBER_QOS_DEFAULT;


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SUBSCRIBERQOS_HPP_ */
