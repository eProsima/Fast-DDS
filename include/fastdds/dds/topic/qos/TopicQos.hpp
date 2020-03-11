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
 * @file TopicQos.hpp
 */


#ifndef _FASTDDS_TOPICQOS_HPP
#define _FASTDDS_TOPICQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastrtps/attributes/TopicAttributes.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class TopicQos, containing all the possible Qos that can be set for a determined Topic.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class TopicQos
{
public:

    TopicQos();

    //!Topic Attributes
    fastrtps::TopicAttributes topic_attr;

    //!Topic Data Qos, NOT implemented in the library.
    TopicDataQosPolicy topic_data;

    //!Durability Qos, implemented in the library.
    DurabilityQosPolicy durability;

    //!Durability Service Qos, NOT implemented in the library.
    DurabilityServiceQosPolicy durability_service;

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    LatencyBudgetQosPolicy latency_budget;

    //!Liveliness Qos, implemented in the library.
    LivelinessQosPolicy liveliness;

    //!Reliability Qos, implemented in the library.
    ReliabilityQosPolicy reliability;

    //!Destination Order Qos, NOT implemented in the library.
    DestinationOrderQosPolicy destination_order;

    //!History Qos, implemented in the library.
    HistoryQosPolicy history;

    //!Resource Limits Qos, implemented in the library.
    ResourceLimitsQosPolicy resource_limits;

    //!Transport Priority Qos, NOT implemented in the library.
    TransportPriorityQosPolicy transport_priority;

    //!Lifespan Qos, implemented in the library.
    LifespanQosPolicy lifespan;

    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy ownership;

    bool operator ==(
            const TopicQos& b) const
    {
        return (this->topic_data == b.topic_data) &&
               (this->durability == b.durability) &&
               (this->durability_service == b.durability_service) &&
               (this->deadline == b.deadline) &&
               (this->latency_budget == b.latency_budget) &&
               (this->liveliness == b.liveliness) &&
               (this->reliability == b.reliability) &&
               (this->destination_order == b.destination_order) &&
               (this->history == b.history) &&
               (this->resource_limits == b.resource_limits) &&
               (this->transport_priority == b.transport_priority) &&
               (this->lifespan == b.lifespan) &&
               (this->ownership == b.ownership);
    }

    TopicQos& operator <<(
            const fastrtps::ReliabilityQosPolicy& qos)
    {
        reliability = qos;
        return *this;
    }

    /* TODO: Implement this method
     * Set Qos from another class
     * @param qos Reference from a TopicQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void setQos(
            const TopicQos& qos,
            bool first_time);

    /* TODO: Implement this method
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool checkQos() const;

    /* TODO: Implement this method
     * Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated.
     */
    RTPS_DllAPI bool canQosBeUpdated(
            const TopicQos& qos) const;
};

extern const TopicQos TOPIC_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TOPICQOS_HPP
