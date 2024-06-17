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

#ifndef FASTDDS_DDS_SUBSCRIBER_QOS__SUBSCRIBERQOS_HPP
#define FASTDDS_DDS_SUBSCRIBER_QOS__SUBSCRIBERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class SubscriberQos, contains all the possible Qos that can be set for a determined Subscriber.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 *
 * @ingroup FASTDDS_QOS_MODULE
 */
class SubscriberQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API SubscriberQos()
    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~SubscriberQos()
    {
    }

    bool operator ==(
            const SubscriberQos& b) const
    {
        return (presentation_ == b.presentation_) &&
               (partition_ == b.partition_) &&
               (group_data_ == b.group_data_) &&
               (entity_factory_ == b.entity_factory_);
    }

    /**
     * Getter for PresentationQosPolicy
     *
     * @return PresentationQosPolicy reference
     */
    const PresentationQosPolicy& presentation() const
    {
        return presentation_;
    }

    /**
     * Getter for PresentationQosPolicy
     *
     * @return PresentationQosPolicy reference
     */
    PresentationQosPolicy& presentation()
    {
        return presentation_;
    }

    /**
     * Setter for PresentationQosPolicy
     *
     * @param presentation new value for the PresentationQosPolicy
     */
    void presentation(
            const PresentationQosPolicy& presentation)
    {
        presentation_ = presentation;
    }

    /**
     * Getter for PartitionQosPolicy
     *
     * @return PartitionQosPolicy reference
     */
    const PartitionQosPolicy& partition() const
    {
        return partition_;
    }

    /**
     * Getter for PartitionQosPolicy
     *
     * @return PartitionQosPolicy reference
     */
    PartitionQosPolicy& partition()
    {
        return partition_;
    }

    /**
     * Setter for PartitionQosPolicy
     *
     * @param partition new value for the PartitionQosPolicy
     */
    void partition(
            const PartitionQosPolicy& partition)
    {
        partition_ = partition;
    }

    /**
     * Getter for GroupDataQosPolicy
     *
     * @return GroupDataQosPolicy reference
     */
    const GroupDataQosPolicy& group_data() const
    {
        return group_data_;
    }

    /**
     * Getter for GroupDataQosPolicy
     *
     * @return GroupDataQosPolicy reference
     */
    GroupDataQosPolicy& group_data()
    {
        return group_data_;
    }

    /**
     * Setter for GroupDataQosPolicy
     *
     * @param group_data new value for the GroupDataQosPolicy
     */
    void group_data(
            const GroupDataQosPolicy& group_data)
    {
        group_data_ = group_data;
    }

    /**
     * Getter for EntityFactoryQosPolicy
     *
     * @return EntityFactoryQosPolicy reference
     */
    const EntityFactoryQosPolicy& entity_factory() const
    {
        return entity_factory_;
    }

    /**
     * Getter for EntityFactoryQosPolicy
     *
     * @return EntityFactoryQosPolicy reference
     */
    EntityFactoryQosPolicy& entity_factory()
    {
        return entity_factory_;
    }

    /**
     * Setter for EntityFactoryQosPolicy
     *
     * @param entity_factory new value for the EntityFactoryQosPolicy
     */
    void entity_factory(
            const EntityFactoryQosPolicy& entity_factory)
    {
        entity_factory_ = entity_factory;
    }

private:

    //!Presentation Qos, NOT implemented in the library.
    PresentationQosPolicy presentation_;

    //!Partition Qos, implemented in the library.
    PartitionQosPolicy partition_;

    //!Group Data Qos, implemented in the library.
    GroupDataQosPolicy group_data_;

    //!Entity Factory Qos, implemented in the library
    EntityFactoryQosPolicy entity_factory_;
};

FASTDDS_EXPORTED_API extern const SubscriberQos SUBSCRIBER_QOS_DEFAULT;


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_SUBSCRIBER_QOS__SUBSCRIBERQOS_HPP
