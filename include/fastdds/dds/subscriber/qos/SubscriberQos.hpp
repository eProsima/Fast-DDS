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

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastrtps/attributes/SubscriberAttributes.h>

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
    RTPS_DllAPI SubscriberQos() = default;

    RTPS_DllAPI virtual ~SubscriberQos() = default;

    bool operator==(
            const SubscriberQos& b) const
    {
        return (this->presentation == b.presentation) &&
               (this->partition == b.partition) &&
               (this->group_data == b.group_data) &&
               (this->entity_factory == b.entity_factory) &&
               (this->subscriber_attr == b.subscriber_attr);
    }

    //!Participant Attributes
    fastrtps::SubscriberAttributes subscriber_attr;

    //!Presentation Qos, NOT implemented in the library.
    PresentationQosPolicy presentation;

    //!Partition Qos, implemented in the library.
    PartitionQosPolicy partition;

    //!GroupData Qos, NOT implemented in the library.
    GroupDataQosPolicy group_data;

    //!Entity Factory Qos, implemented in the library
    EntityFactoryQosPolicy entity_factory;

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
