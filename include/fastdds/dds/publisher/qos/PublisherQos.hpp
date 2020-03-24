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
 * @file PublisherQos.hpp
 */


#ifndef _FASTDDS_PUBLISHERQOS_HPP_
#define _FASTDDS_PUBLISHERQOS_HPP_

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastrtps/attributes/PublisherAttributes.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class PublisherQos, containing all the possible Qos that can be set for a determined Publisher.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been
 * implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class PublisherQos
{
public:

    RTPS_DllAPI PublisherQos();

    RTPS_DllAPI virtual ~PublisherQos();

    bool operator ==(
            const PublisherQos& b) const
    {
        return (this->partition == b.partition) &&
               (this->presentation == b.presentation) &&
               (this->group_data == b.group_data) &&
               (this->entity_factory == b.entity_factory) &&
               (this->publish_mode == b.publish_mode) &&
               (this->disable_positive_acks == b.disable_positive_acks) &&
               (this->publisher_attr == b.publisher_attr);
    }

    //!Publisher Attributes
    fastrtps::PublisherAttributes publisher_attr;

    //!Presentation Qos, NOT implemented in the library.
    PresentationQosPolicy presentation;

    //!Partition Qos, implemented in the library.
    PartitionQosPolicy partition;

    //!Group Data Qos, NOT implemented in the library.
    GroupDataQosPolicy group_data;

    //!Entity Factory Qos, implemented in the library
    EntityFactoryQosPolicy entity_factory;

    //!Publication Mode Qos, implemented in the library.
    PublishModeQosPolicy publish_mode;

    //!Disable positive acks QoS, implemented in the library.
    DisablePositiveACKsQosPolicy disable_positive_acks;

    /**
     * Set Qos from another class
     * @param qos Reference from a PublisherQos object.
     * @param first_time Boolean indicating whether is the first time (If not some parameters cannot be set).
     */
    RTPS_DllAPI void set_qos(
            const PublisherQos& qos,
            bool first_time);

    /**
     * Check if the Qos values are compatible between each other.
     * @return True if correct.
     */
    RTPS_DllAPI bool check_qos() const;

    RTPS_DllAPI bool can_qos_be_updated(
            const PublisherQos& qos) const;
};

RTPS_DllAPI extern const PublisherQos PUBLISHER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_PUBLISHERQOS_HPP_
