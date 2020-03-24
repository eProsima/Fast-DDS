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

#include <fastrtps/qos/QosPolicies.h>

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
    fastrtps::DurabilityQosPolicy durability;

    //!Deadline Qos, implemented in the library.
    fastrtps::DeadlineQosPolicy deadline;

    //!Latency Budget Qos, implemented in the library.
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

    //!User Data Qos, NOT implemented in the library.
    fastrtps::UserDataQosPolicy user_data;

    //!Ownership Qos, NOT implemented in the library.
    fastrtps::OwnershipQosPolicy ownership;

    //!Time Based Filter Qos, NOT implemented in the library.
    fastrtps::TimeBasedFilterQosPolicy time_based_filter;

    //!Reader Data Lifecycle Qos, NOT implemented in the library.
    fastdds::dds::ReaderDataLifecycleQosPolicy reader_data_lifecycle;

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
               (this->reader_data_lifecycle == b.reader_data_lifecycle);
    }

    /* TODO: Implement this method
     * Set Qos from another class
     * @param qos Reference from a DataReaderQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
    RTPS_DllAPI void setQos(
            const DataReaderQos& qos,
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
            const DataReaderQos& qos) const;
    */
};

//extern const DataReaderQos DATAREADER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DATAREADERQOS_HPP
