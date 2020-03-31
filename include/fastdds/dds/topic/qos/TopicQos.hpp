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

#include <fastdds/dds/log/Log.hpp>

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

    bool operator ==(
            const TopicQos& b) const
    {
        //        if (!(this->topic_data == b.topic_data))
        //        {
        //            std::cout << "Topic data false" << std::endl;
        //        }
        //        if (!(this->durability == b.durability))
        //        {
        //            std::cout << "Durability false" << std::endl;
        //        }
        //        if (!(this->durability_service == b.durability_service))
        //        {
        //            std::cout << "Durability Service false" << std::endl;
        //        }
        //        if (!(this->deadline == b.deadline))
        //        {
        //            std::cout << "Deadline false" << std::endl;
        //        }
        //        if (!(this->latency_budget == b.latency_budget))
        //        {
        //            std::cout << "Latency Budget false" << std::endl;
        //        }
        //        if (!(this->liveliness == b.liveliness))
        //        {
        //            std::cout << "Liveliness false" << std::endl;
        //        }
        //        if (!(this->reliability == b.reliability))
        //        {
        //            std::cout << "Reliability false" << std::endl;
        //        }
        //        if (!(this->destination_order == b.destination_order))
        //        {
        //            std::cout << "Destination order false" << std::endl;
        //        }
        //        if (!(this->history == b.history))
        //        {
        //            std::cout << "History false" << std::endl;
        //        }
        //        if (!(this->resource_limits == b.resource_limits))
        //        {
        //            std::cout << "Resource Limits false" << std::endl;
        //        }
        //        if (!(this->transport_priority == b.transport_priority))
        //        {
        //            std::cout << "Transport Priority false" << std::endl;
        //        }
        //        if (!(this->lifespan == b.lifespan))
        //        {
        //            std::cout << "Lifespan false" << std::endl;
        //        }
        //        if (!(this->ownership == b.ownership))
        //        {
        //            std::cout << "Ownership false" << std::endl;
        //        }
        //        if (!(this->topic_attr == b.topic_attr))
        //        {
        //            std::cout << "Topic Attributes false" << std::endl;
        //        }
        return (this->topic_data_ == b.topic_data()) &&
               (this->durability_ == b.durability()) &&
               (this->durability_service_ == b.durability_service()) &&
               (this->deadline_ == b.deadline()) &&
               (this->latency_budget_ == b.latency_budget()) &&
               (this->liveliness_ == b.liveliness()) &&
               (this->reliability_ == b.reliability()) &&
               (this->destination_order_ == b.destination_order()) &&
               (this->history_ == b.history()) &&
               (this->resource_limits_ == b.resource_limits()) &&
               (this->transport_priority_ == b.transport_priority()) &&
               (this->lifespan_ == b.lifespan()) &&
               (this->ownership_ == b.ownership()) &&
               (this->topic_attr == b.topic_attr);
    }

    /* TODO: Implement this method
     * Set Qos from another class
     * @param qos Reference from a TopicQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void set_qos(
            const TopicQos& qos,
            bool first_time);

    /* TODO: Implement this method
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool check_qos() const;

    /* TODO: Implement this method
     * Check if the Qos can be update with the values provided. This method DOES NOT update anything.
     * @param qos Reference to the new qos.
     * @return True if they can be updated.
     */
    RTPS_DllAPI bool can_qos_be_updated(
            const TopicQos& qos) const;

    /**
     * Getter for TopicDataQosPolicy
     * @return TopicDataQos reference
     */
    const TopicDataQosPolicy& topic_data() const
    {
        return topic_data_;
    }

    /**
     * Setter for TopicDataQosPolicy
     * @param value
     */
    void topic_data(
            const TopicDataQosPolicy& value)
    {
        topic_data_ = value;
        topic_data_.hasChanged = true;
    }

    /**
     * Getter for DurabilityQosPolicy
     * @return DurabilityQos reference
     */
    const DurabilityQosPolicy& durability() const
    {
        return durability_;
    }

    /**
     * Setter for DurabilityQosPolicy
     * @param durability
     */
    void durability(
            const DurabilityQosPolicy& durability)
    {
        durability_ = durability;
        durability_.hasChanged = true;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     * @return DurabilityServiceQos reference
     */
    const DurabilityServiceQosPolicy& durability_service() const
    {
        return durability_service_;
    }

    /**
     * Setter for DurabilityServiceQosPolicy
     * @param durability_service
     */
    void durability_service(
            const DurabilityServiceQosPolicy& durability_service)
    {
        durability_service_ = durability_service;
        durability_service_.hasChanged = true;
    }

    /**
     * Getter for DeadlineQosPolicy
     * @return DeadlineQos reference
     */
    const DeadlineQosPolicy& deadline() const
    {
        return deadline_;
    }

    /**
     * Setter for DeadlineQosPolicy
     * @param deadline
     */
    void deadline(
            const DeadlineQosPolicy& deadline)
    {
        deadline_ = deadline;
        deadline_.hasChanged = true;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     * @return LatencyBudgetQos reference
     */
    const LatencyBudgetQosPolicy& latency_budget() const
    {
        return latency_budget_;
    }

    /**
     * Setter for LatencyBudgetQosPolicy
     * @param latency_budget
     */
    void latency_budget(
            const LatencyBudgetQosPolicy& latency_budget)
    {
        latency_budget_ = latency_budget;
        latency_budget_.hasChanged = true;
    }

    /**
     * Getter for LivelinessQosPolicy
     * @return LivelinessQos reference
     */
    const LivelinessQosPolicy& liveliness() const
    {
        return liveliness_;
    }

    /**
     * Setter for LivelinessQosPolicy
     * @param liveliness
     */
    void liveliness(
            const LivelinessQosPolicy& liveliness)
    {
        liveliness_ = liveliness;
        liveliness_.hasChanged = true;
    }

    /**
     * Getter for ReliabilityQosPolicy
     * @return ReliabilityQos reference
     */
    const ReliabilityQosPolicy& reliability() const
    {
        return reliability_;
    }

    /**
     * Setter for ReliabilityQosPolicy
     * @param reliability
     */
    void reliability(
            const ReliabilityQosPolicy& reliability)
    {
        reliability_ = reliability;
        reliability_.hasChanged = true;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     * @return DestinationOrderQos reference
     */
    const DestinationOrderQosPolicy& destination_order() const
    {
        return destination_order_;
    }

    /**
     * Setter for DestinationOrderQosPolicy
     * @param destination_order
     */
    void destination_order(
            const DestinationOrderQosPolicy& destination_order)
    {
        destination_order_ = destination_order;
        destination_order_.hasChanged = true;
    }

    /**
     * Getter for HistoryQosPolicy
     * @return HistoryQos reference
     */
    const HistoryQosPolicy& history() const
    {
        return history_;
    }

    /**
     * Setter for HistoryQosPolicy
     * @param history
     */
    void history(
            const HistoryQosPolicy& history)
    {
        history_ = history;
        history_.hasChanged = true;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     * @return ResourceLimitsQos reference
     */
    const ResourceLimitsQosPolicy& resource_limits() const
    {
        return resource_limits_;
    }

    /**
     * Setter for ResourceLimitsQosPolicy
     * @param resource_limits
     */
    void resource_limits(
            const ResourceLimitsQosPolicy& resource_limits)
    {
        resource_limits_ = resource_limits;
        resource_limits_.hasChanged = true;
    }

    /**
     * Getter for TransportPriorityQosPolicy
     * @return TransportPriorityQos reference
     */
    const TransportPriorityQosPolicy& transport_priority() const
    {
        return transport_priority_;
    }

    /**
     * Setter for TransportPriorityQosPolicy
     * @param transport_priority
     */
    void transport_priority(
            const TransportPriorityQosPolicy& transport_priority)
    {
        transport_priority_ = transport_priority;
        transport_priority_.hasChanged = true;
    }

    /**
     * Getter for LifespanQosPolicy
     * @return LifespanQos reference
     */
    const LifespanQosPolicy& lifespan() const
    {
        return lifespan_;
    }

    /**
     * Setter for LifespanQosPolicy
     * @param lifespan
     */
    void lifespan(
            const LifespanQosPolicy& lifespan)
    {
        lifespan_ = lifespan;
        lifespan_.hasChanged = true;
    }

    /**
     * Getter for OwnershipQosPolicy
     * @return OwnershipQos reference
     */
    const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    /**
     * Setter for OwnershipQosPolicy
     * @param ownership
     */
    void ownership(
            const OwnershipQosPolicy& ownership)
    {
        ownership_ = ownership;
        ownership_.hasChanged = true;
    }

    //!Topic Attributes
    fastrtps::TopicAttributes topic_attr;

private:

    //!Topic Data Qos, NOT implemented in the library.
    TopicDataQosPolicy topic_data_;

    //!Durability Qos, implemented in the library.
    DurabilityQosPolicy durability_;

    //!Durability Service Qos, NOT implemented in the library.
    DurabilityServiceQosPolicy durability_service_;

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy deadline_;

    //!Latency Budget Qos, NOT implemented in the library.
    LatencyBudgetQosPolicy latency_budget_;

    //!Liveliness Qos, implemented in the library.
    LivelinessQosPolicy liveliness_;

    //!Reliability Qos, implemented in the library.
    ReliabilityQosPolicy reliability_;

    //!Destination Order Qos, NOT implemented in the library.
    DestinationOrderQosPolicy destination_order_;

    //!History Qos, implemented in the library.
    HistoryQosPolicy history_;

    //!Resource Limits Qos, implemented in the library.
    ResourceLimitsQosPolicy resource_limits_;

    //!Transport Priority Qos, NOT implemented in the library.
    TransportPriorityQosPolicy transport_priority_;

    //!Lifespan Qos, implemented in the library.
    LifespanQosPolicy lifespan_;

    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy ownership_;
};

extern const TopicQos TOPIC_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TOPICQOS_HPP
