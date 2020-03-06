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
 * @file DataWriterQos.hpp
 */


#ifndef _FASTDDS_DATAWRITERQOS_HPP
#define _FASTDDS_DATAWRITERQOS_HPP

#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/core/policy/WriterDataLifecycleQosPolicy.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DataWriterQos, containing all the possible Qos that can be set for a determined Topic.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class DataWriterQos
{
public:
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

    //!Destination Order Qos, NOT implemented in the library.
    fastrtps::DestinationOrderQosPolicy destination_order;

    //!History Qos, implemented in the library.
    fastrtps::HistoryQosPolicy history;

    //!Resource Limits Qos, implemented in the library.
    fastrtps::ResourceLimitsQosPolicy resource_limits;

    //!Transport Priority Qos, NOT implemented in the library.
    fastrtps::TransportPriorityQosPolicy transport_priority;

    //!Lifespan Qos, implemented in the library.
    fastrtps::LifespanQosPolicy lifespan;

    //!User Data Qos, implemented in the library.
    UserDataQosPolicy user_data;

    //!Ownership Qos, NOT implemented in the library.
    fastrtps::OwnershipQosPolicy ownership;

    //!Ownership Strength Qos, NOT implemented in the library.
    fastrtps::OwnershipStrengthQosPolicy ownership_strength;

    //!Writer Data Lifecycle Qos, NOT implemented in the library.
    fastdds::dds::WriterDataLifecycleQosPolicy writer_data_lifecycle;

    bool operator ==(
            const DataWriterQos& b) const
    {
        return (this->durability == b.durability) &&
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
               (this->user_data == b.user_data) &&
               (this->ownership == b.ownership) &&
               (this->ownership_strength == b.ownership_strength) &&
               (this->writer_data_lifecycle == b.writer_data_lifecycle);
    }

    /* TODO: Implement this method
     * Set Qos from another class
     * @param qos Reference from a TopicQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
    RTPS_DllAPI void setQos(
            const DataWriterQos& qos,
            bool first_time);
    */

    /* TODO: Implement this method
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
    RTPS_DllAPI bool checkQos() const;
    */

    /* TODO: Implement this method
     * Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated.
    RTPS_DllAPI bool canQosBeUpdated(
            const DataWriterQos& qos) const;
    */
};

//extern const DataWriterQos DATAWRITER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DATAWRITERQOS_HPP
