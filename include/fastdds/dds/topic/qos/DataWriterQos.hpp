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

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/policy/WriterDataLifecycleQosPolicy.hpp>
#include <fastdds/dds/topic/qos/WriterQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

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

    RTPS_DllAPI DataWriterQos();

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

    //!User Data Qos, implemented in the library.
    UserDataQosPolicy user_data;

    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy ownership;

    //!Ownership Strength Qos, NOT implemented in the library.
    OwnershipStrengthQosPolicy ownership_strength;

    //!Writer Data Lifecycle Qos, NOT implemented in the library.
    WriterDataLifecycleQosPolicy writer_data_lifecycle;

    //!Publication Mode Qos, implemented in the library.
    PublishModeQosPolicy publish_mode;

    //!Disable positive acks QoS, implemented in the library.
    DisablePositiveACKsQosPolicy disable_positive_ACKs;

    //!Data Representation Qos, implemented in the library.
    DataRepresentationQosPolicy representation;

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
               (this->writer_data_lifecycle == b.writer_data_lifecycle) &&
               (this->publish_mode == b.publish_mode) &&
               (this->disable_positive_ACKs == b.disable_positive_ACKs) &&
               (this->representation == b.representation);
    }

    /* Set Qos from another class
     * @param qos Reference from a TopicQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).*/
    RTPS_DllAPI void set_qos(
            const DataWriterQos& qos,
            bool first_time);


    /* Check if the Qos values are compatible between each other.
     * @return True if correct. */
    RTPS_DllAPI bool check_qos() const;


    /* Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated. */
    RTPS_DllAPI bool can_qos_be_updated(
            const DataWriterQos& qos) const;

    RTPS_DllAPI WriterQos change_to_writerqos() const;

    RTPS_DllAPI void change_to_datawriterqos(
            const WriterQos& qos);

    RTPS_DllAPI void copy_from_topicqos(
            const TopicQos& topic_qos);

    RTPS_DllAPI std::string search_qos_by_id(
            eprosima::fastdds::dds::QosPolicyId_t id);

};

extern const DataWriterQos DDS_DATAWRITER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DATAWRITERQOS_HPP
