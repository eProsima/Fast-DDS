// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReaderQos.h
 *
 */

#ifndef READERQOS_H_
#define READERQOS_H_

#include "QosPolicies.h"

namespace eprosima {
namespace fastrtps {

/**
 * Class ReaderQos, contains all the possible Qos that can be set for a determined Subscriber.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class  ReaderQos{
public:
    RTPS_DllAPI ReaderQos() {}
    RTPS_DllAPI virtual ~ReaderQos() {}

    bool operator==(const ReaderQos& b) const
    {
        return (this->m_durability == b.m_durability) &&
               (this->m_deadline == b.m_deadline) &&
               (this->m_latencyBudget == b.m_latencyBudget) &&
               (this->m_liveliness == b.m_liveliness) &&
               (this->m_reliability == b.m_reliability) &&
               (this->m_ownership == b.m_ownership) &&
               (this->m_destinationOrder == b.m_destinationOrder) &&
               (this->m_userData == b.m_userData) &&
               (this->m_timeBasedFilter == b.m_timeBasedFilter) &&
               (this->m_presentation == b.m_presentation) &&
               (this->m_partition == b.m_partition) &&
               (this->m_topicData == b.m_topicData) &&
               (this->m_groupData == b.m_groupData) &&
               (this->m_durabilityService == b.m_durabilityService) &&
               (this->m_lifespan == b.m_lifespan) &&
               (this->m_disablePositiveACKs == b.m_disablePositiveACKs);
    }

    //!Durability Qos, implemented in the library.
    DurabilityQosPolicy m_durability;
    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy m_deadline;
    //!Latency Budget Qos, NOT implemented in the library.
    LatencyBudgetQosPolicy m_latencyBudget;
    //!Liveliness Qos, implemented in the library.
    LivelinessQosPolicy m_liveliness;
    //!ReliabilityQos, implemented in the library.
    ReliabilityQosPolicy m_reliability;
    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy m_ownership;
    //!Destinatio Order Qos, NOT implemented in the library.
    DestinationOrderQosPolicy m_destinationOrder;
    //!UserData Qos, NOT implemented in the library.
    UserDataQosPolicy m_userData;
    //!Time Based Filter Qos, NOT implemented in the library.
    TimeBasedFilterQosPolicy m_timeBasedFilter;
    //!Presentation Qos, NOT implemented in the library.
    PresentationQosPolicy m_presentation;
    //!Partition Qos, implemented in the library.
    PartitionQosPolicy m_partition;
    //!Topic Data Qos, NOT implemented in the library.
    TopicDataQosPolicy m_topicData;
    //!GroupData Qos, NOT implemented in the library.
    GroupDataQosPolicy m_groupData;
    //!Durability Service Qos, NOT implemented in the library.
    DurabilityServiceQosPolicy m_durabilityService;
    //!Lifespan Qos, NOT implemented in the library.
    LifespanQosPolicy m_lifespan;
    //!Data Representation Qos, NOT implemented in the library.
    DataRepresentationQosPolicy m_dataRepresentation;
    //!Type consistency enforcement Qos, NOT implemented in the library.
    TypeConsistencyEnforcementQosPolicy m_typeConsistency;
    //!Disable positive ACKs QoS
    DisablePositiveACKsQosPolicy m_disablePositiveACKs;
    /**
     * Set Qos from another class
     * @param readerqos Reference from a ReaderQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void setQos(const ReaderQos& readerqos, bool first_time);
    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool checkQos() const;
    /**
     * Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated.
     */
    RTPS_DllAPI bool canQosBeUpdated(const ReaderQos& qos) const;
};



} /* namespace  */
} /* namespace eprosima */

#endif /* READERQOS_H_ */
