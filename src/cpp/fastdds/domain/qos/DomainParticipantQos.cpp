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
 * @file DomainParticipantQos.cpp
 *
 */

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

const DomainParticipantQos PARTICIPANT_QOS_DEFAULT;

void DomainParticipantQos::set_qos(
        const DomainParticipantQos& qos,
        bool first_time)
{
    if (entity_factory_.autoenable_created_entities != qos.entity_factory().autoenable_created_entities)
    {
        entity_factory_ = qos.entity_factory();
        entity_factory_.hasChanged = true;
    }
    if (user_data_.data_vec() != qos.user_data().data_vec())
    {
        user_data_ = qos.user_data();
        user_data_.hasChanged = true;
    }
    if (first_time && !(allocation_ == qos.allocation()))
    {
        allocation_ = qos.allocation();
    }
    if (first_time && !(properties_ == qos.properties()))
    {
        properties_ = qos.properties();
    }
    if (first_time && !(wire_protocol_ == qos.wire_protocol()))
    {
        wire_protocol_ = qos.wire_protocol();
        wire_protocol_.hasChanged = true;
    }
    if (first_time && !(transport_ == qos.transport()))
    {
        transport_ = qos.transport();
        transport_.hasChanged = true;
    }
    if (first_time && name_ != qos.name())
    {
        name_ = qos.name();
    }
}

bool DomainParticipantQos::check_qos() const
{
    //There is no restriction by the moment with the contained Qos
    return true;
}

bool DomainParticipantQos::can_qos_be_updated(
        const DomainParticipantQos& qos) const
{
    bool updatable = true;
    if (!(allocation_ == qos.allocation()))
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "ParticipantResourceLimitsQos cannot be changed after the participant creation");
    }
    if (!(properties_ == qos.properties()))
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "PropertyPolilyQos cannot be changed after the participant creation");
    }
    if (!(wire_protocol_ == qos.wire_protocol()))
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "WireProtocolConfigQos cannot be changed after the participant creation");
    }
    if (!(transport_ == qos.transport()))
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "TransportConfigQos cannot be changed after the participant creation");
    }
    if (!(name_ == qos.name()))
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Participant name cannot be changed after the participant creation");
    }
    return updatable;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
