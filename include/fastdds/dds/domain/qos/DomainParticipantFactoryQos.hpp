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
 * @file DomainParticipantFactoryQos.hpp
 *
 */

#ifndef _FASTDDS_PARTICIPANTFACTORYQOS_HPP_
#define _FASTDDS_PARTICIPANTFACTORYQOS_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DomainParticipantFactoryQos, contains all the possible Qos that can be set for a determined participant.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class DomainParticipantFactoryQos
{
public:

    RTPS_DllAPI DomainParticipantFactoryQos()
    {
    }

    RTPS_DllAPI virtual ~DomainParticipantFactoryQos()
    {
    }

    bool operator ==(
            const DomainParticipantFactoryQos& b) const
    {
        return (this->entity_factory == b.entity_factory);
    }

    //!Auto enable on creation
    EntityFactoryQosPolicy entity_factory;

    /**
     * Set Qos from another class
     * @param qos Reference from a DomainParticipantQos object.
     */
    RTPS_DllAPI void set_qos(
            const DomainParticipantFactoryQos& qos);

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PARTICIPANTFACTORYQOS_HPP_ */
