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
 * @file DomainParticipantQos.hpp
 *
 */

#ifndef _FASTDDS_PARTICIPANTQOS_HPP_
#define _FASTDDS_PARTICIPANTQOS_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DomainParticipantQos, contains all the possible Qos that can be set for a determined participant.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class DomainParticipantQos
{
public:
    RTPS_DllAPI DomainParticipantQos()
    {}

    RTPS_DllAPI virtual ~DomainParticipantQos()
    {}

    bool operator ==(
        const DomainParticipantQos& b) const
    {
        return (this->user_data == b.user_data) &&
               (this->entity_factory == b.entity_factory);
    }

    //!UserData Qos, NOT implemented in the library.
    UserDataQosPolicy user_data;

    //!Auto enable on creation
    EntityFactoryQosPolicy entity_factory;

    /**
     * Set Qos from another class
     * @param qos Reference from a DomainParticipantQos object.
     */
    RTPS_DllAPI void set_qos(
            const DomainParticipantQos& qos);

};

RTPS_DllAPI extern const DomainParticipantQos PARTICIPANT_QOS_DEFAULT;


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PARTICIPANTQOS_HPP_ */
