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
 * @file DataReaderQos.hpp
 */


#ifndef _FASTDDS_DATAREADERQOS_HPP
#define _FASTDDS_DATAREADERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/policy/ReaderDataLifecycleQosPolicy.hpp>
#include <fastdds/dds/topic/qos/ReaderQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/TopicAttributes.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DataReaderQos, containing all the possible Qos that can be set for a determined DataReader.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class DataReaderQos
{
public:

    //!Durability Qos, implemented in the library.
    DurabilityQosPolicy durability;

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy deadline;

    //!Latency Budget Qos, implemented in the library.
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

    //!User Data Qos, NOT implemented in the library.
    UserDataQosPolicy user_data;

    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy ownership;

    //!Time Based Filter Qos, NOT implemented in the library.
    TimeBasedFilterQosPolicy time_based_filter;

    //!Reader Data Lifecycle Qos, NOT implemented in the library.
    ReaderDataLifecycleQosPolicy reader_data_lifecycle;

    //!Data Representation Qos, implemented in the library.
    DataRepresentationQosPolicy representation;

    //!Type consistency enforcement Qos, NOT implemented in the library.
    TypeConsistencyEnforcementQosPolicy type_consistency;

    //!Disable positive ACKs QoS
    DisablePositiveACKsQosPolicy disable_positive_ACKs;

    bool operator ==(
            const DataReaderQos& b) const
    {
        return (this->durability == b.durability) &&
               (this->deadline == b.deadline) &&
               (this->latency_budget == b.latency_budget) &&
               (this->liveliness == b.liveliness) &&
               (this->reliability == b.reliability) &&
               (this->destination_order == b.destination_order) &&
               (this->history == b.history) &&
               (this->resource_limits == b.resource_limits) &&
               (this->user_data == b.user_data) &&
               (this->ownership == b.ownership) &&
               (this->time_based_filter == b.time_based_filter) &&
               (this->reader_data_lifecycle == b.reader_data_lifecycle) &&
               (this->representation == b.representation) &&
               (this->type_consistency == b.type_consistency) &&
               (this->disable_positive_ACKs == b.disable_positive_ACKs);
    }

    /* Set Qos from another class
     * @param qos Reference from a DataReaderQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set). */
    RTPS_DllAPI void setQos(
            const DataReaderQos& qos,
            bool first_time);

    /* Check if the Qos values are compatible between each other.
     * @return True if correct.    */
    RTPS_DllAPI bool checkQos() const;


    /* Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated.     */
    RTPS_DllAPI bool canQosBeUpdated(
            const DataReaderQos& qos) const;

    RTPS_DllAPI ReaderQos changeToReaderQos() const;

    RTPS_DllAPI void changeToDataReaderQos(
            const ReaderQos& qos);

    RTPS_DllAPI void copyFromTopicQos(
            const TopicQos& topic_qos);

    RTPS_DllAPI void copy_to_topic_attributes(
            eprosima::fastrtps::TopicAttributes* topic_att) const;

    RTPS_DllAPI std::string search_qos_by_id(
            eprosima::fastdds::dds::QosPolicyId_t id);

};

extern const DataReaderQos DDS_DATAREADER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DATAREADERQOS_HPP
