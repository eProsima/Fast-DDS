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
#include <fastrtps/attributes/ParticipantAttributes.h>
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
    {
    }

    RTPS_DllAPI virtual ~DomainParticipantQos()
    {
    }

    bool operator ==(
            const DomainParticipantQos& b) const
    {
        return (this->user_data_ == b.user_data()) &&
               (this->entity_factory_ == b.entity_factory()) &&
               (this->allocation_ == b.allocation()) &&
               (this->properties_ == b.properties()) &&
               (this->wire_protocol_ == b.wire_protocol()) &&
               (this->transport_ == b.transport()) &&
               (this->name_ == b.name());
    }

    /**
     * Set Qos from another class
     * @param qos Reference from a DomainParticipantQos object.
     */
    RTPS_DllAPI void set_qos(
            const DomainParticipantQos& qos,
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
            const DomainParticipantQos& qos) const;


    /**
     * Getter for UserDataQosPolicy
     * @return UserDataQosPolicy reference
     */
    const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    /**
     * Getter for UserDataQosPolicy
     * @return UserDataQosPolicy reference
     */
    UserDataQosPolicy& user_data()
    {
        return user_data_;
    }

    /**
     * Setter for UserDataQosPolicy
     * @param value
     */
    void user_data(
            const UserDataQosPolicy& value)
    {
        user_data_ = value;
    }

    /**
     * Getter for EntityFactoryQosPolicy
     * @return EntityFactoryQosPolicy reference
     */
    const EntityFactoryQosPolicy& entity_factory() const
    {
        return entity_factory_;
    }

    /**
     * Getter for EntityFactoryQosPolicy
     * @return EntityFactoryQosPolicy reference
     */
    EntityFactoryQosPolicy& entity_factory()
    {
        return entity_factory_;
    }

    /**
     * Setter for EntityFactoryQosPolicy
     * @param value
     */
    void entity_factory(
            const EntityFactoryQosPolicy& value)
    {
        entity_factory_ = value;
    }

    const ParticipantResourceLimitsQos& allocation() const
    {
        return allocation_;
    }

    ParticipantResourceLimitsQos& allocation()
    {
        return allocation_;
    }

    void allocation(
            const ParticipantResourceLimitsQos& allocation)
    {
        allocation_ = allocation;
    }

    const PropertyPolicyQos& properties() const
    {
        return properties_;
    }

    PropertyPolicyQos& properties()
    {
        return properties_;
    }

    void properties(
            const PropertyPolicyQos& properties)
    {
        properties_ = properties;
    }

    const WireProtocolConfigQos& wire_protocol() const
    {
        return wire_protocol_;
    }

    WireProtocolConfigQos& wire_protocol()
    {
        return wire_protocol_;
    }

    void wire_protocol(
            const WireProtocolConfigQos& wire_protocol)
    {
        wire_protocol_ = wire_protocol;
    }

    const TransportConfigQos& transport() const
    {
        return transport_;
    }

    TransportConfigQos& transport()
    {
        return transport_;
    }

    void transport(
            const TransportConfigQos& transport)
    {
        transport_ = transport;
    }

    const fastrtps::string_255& name() const
    {
        return name_;
    }

    fastrtps::string_255& name()
    {
        return name_;
    }

    void name(
            const fastrtps::string_255& value)
    {
        name_ = value;
    }

private:

    //!UserData Qos, NOT implemented in the library.
    UserDataQosPolicy user_data_;

    //!Auto enable on creation
    EntityFactoryQosPolicy entity_factory_;

    //!Participant allocation limits
    ParticipantResourceLimitsQos allocation_;

    //!Property policies
    PropertyPolicyQos properties_;

    //!Wire Protocol options
    WireProtocolConfigQos wire_protocol_;

    //!Transport options
    TransportConfigQos transport_;

    //!Name of the participant.
    fastrtps::string_255 name_;

};

RTPS_DllAPI extern const DomainParticipantQos PARTICIPANT_QOS_DEFAULT;


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PARTICIPANTQOS_HPP_ */
