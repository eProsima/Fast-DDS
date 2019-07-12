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

#include "../../../fastrtps/qos/QosPolicies.h"

namespace eprosima {
namespace fastdds {

/**
 * Class TopicQos, containing all the possible Qos that can be set for a determined Topic.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class TopicQos
{
public:
    RTPS_DllAPI TopicQos();
    RTPS_DllAPI virtual ~TopicQos();

    bool operator==(const TopicQos& b) const
    {
        return (this->m_topicData == b.m_topicData) &&
               (this->m_durability == b.m_durability) &&
               (this->m_durabilityService == b.m_durabilityService) &&        
               (this->m_deadline == b.m_deadline) &&        
               (this->m_latencyBudget == b.m_latencyBudget) &&        
               (this->m_liveliness == b.m_liveliness) &&        
               (this->m_reliability == b.m_reliability) &&        
               (this->m_destinationOrder == b.m_destinationOrder) &&        
               (this->m_history == b.m_history) &&        
               (this->m_resourceLimits == b.m_resourceLimits) &&        
               (this->m_transportPriority == b.m_transportPriority) &&        
               (this->m_lifespan == b.m_lifespan) &&        
               (this->m_ownership == b.m_ownership);
    }

    //!Topic Data Qos, NOT implemented in the library.
    fastrtps::TopicDataQosPolicy m_topicData;

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

    //!Destination Order Qos, NOT implemented in the library.
    fastrtps::DestinationOrderQosPolicy m_destinationOrder;

    //!History Qos
    fastrtps::HistoryQosPolicy m_history;

    //!Resource Limits Qos
    fastrtps::ResourceLimitsQosPolicy m_resourceLimits;

    //!Transport Priority Qos
    fastrtps::TransportPriorityQosPolicy m_transportPriority;

    //!Lifespan Qos, NOT implemented in the library.
    fastrtps::LifespanQosPolicy m_lifespan;

    //!Ownership Qos, NOT implemented in the library.
    fastrtps::OwnershipQosPolicy m_ownership;

    /**
     * Set Qos from another class
     * @param qos Reference from a TopicQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void setQos(const TopicQos& qos, bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool checkQos() const;

    RTPS_DllAPI bool canQosBeUpdated(const TopicQos& qos) const;
};

extern TopicQos TOPIC_QOS_DEFAULT;

} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TOPICQOS_HPP
