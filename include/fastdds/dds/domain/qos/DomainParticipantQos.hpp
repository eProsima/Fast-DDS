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

#ifndef FASTDDS_DDS_DOMAIN_QOS__DOMAINPARTICIPANTQOS_HPP
#define FASTDDS_DDS_DOMAIN_QOS__DOMAINPARTICIPANTQOS_HPP

#include <string>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/BuiltinTransports.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DomainParticipantQos, contains all the possible Qos that can be set for a determined participant.
 * Please consult each of them to check for implementation details and default values.
 *
 * @ingroup FASTDDS_QOS_MODULE
 */
class DomainParticipantQos
{
public:

    friend class DomainParticipantExtendedQos;

    /*!
     * User defined flow controllers to use alongside.
     *
     * @since 2.4.0
     */
    using FlowControllerDescriptorList = std::vector<std::shared_ptr<fastdds::rtps::FlowControllerDescriptor>>;

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DomainParticipantQos()
    {
#ifdef FASTDDS_STATISTICS
        /*
         * In the case of Statistics, the following properties are set with an empty value. This is because if these
         * properties are set and empty during the enabling of the DomainParticipant, they are fill with the default
         * mechanism
         */
        properties_.properties().emplace_back(parameter_policy_physical_data_host, "");
        properties_.properties().emplace_back(parameter_policy_physical_data_user, "");
        properties_.properties().emplace_back(parameter_policy_physical_data_process, "");
#endif // ifdef FASTDDS_STATISTICS
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~DomainParticipantQos()
    {
    }

    virtual bool operator ==(
            const DomainParticipantQos& b) const
    {
        return (this->user_data_ == b.user_data()) &&
               (this->entity_factory_ == b.entity_factory()) &&
               (this->allocation_ == b.allocation()) &&
               (this->properties_ == b.properties()) &&
               (this->wire_protocol_ == b.wire_protocol()) &&
               (this->transport_ == b.transport()) &&
               (this->name_ == b.name()) &&
               (this->builtin_controllers_sender_thread_ == b.builtin_controllers_sender_thread()) &&
               (this->timed_events_thread_ == b.timed_events_thread()) &&
               (this->discovery_server_thread_ == b.discovery_server_thread()) &&
               (this->typelookup_service_thread_ == b.typelookup_service_thread()) &&
#if HAVE_SECURITY
               (this->security_log_thread_ == b.security_log_thread()) &&
#endif // if HAVE_SECURITY
               (compare_flow_controllers(b));
    }

    /**
     * Getter for UserDataQosPolicy
     *
     * @return UserDataQosPolicy reference
     */
    const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    /**
     * Getter for UserDataQosPolicy
     *
     * @return UserDataQosPolicy reference
     */
    UserDataQosPolicy& user_data()
    {
        return user_data_;
    }

    /**
     * Setter for UserDataQosPolicy
     *
     * @param value UserDataQosPolicy
     */
    void user_data(
            const UserDataQosPolicy& value)
    {
        user_data_ = value;
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
     * @param value EntityFactoryQosPolicy
     */
    void entity_factory(
            const EntityFactoryQosPolicy& value)
    {
        entity_factory_ = value;
    }

    /**
     * Getter for ParticipantResourceLimitsQos
     *
     * @return ParticipantResourceLimitsQos reference
     */
    const ParticipantResourceLimitsQos& allocation() const
    {
        return allocation_;
    }

    /**
     * Getter for ParticipantResourceLimitsQos
     *
     * @return ParticipantResourceLimitsQos reference
     */
    ParticipantResourceLimitsQos& allocation()
    {
        return allocation_;
    }

    /**
     * Setter for ParticipantResourceLimitsQos
     *
     * @param allocation ParticipantResourceLimitsQos
     */
    void allocation(
            const ParticipantResourceLimitsQos& allocation)
    {
        allocation_ = allocation;
    }

    /**
     * Getter for PropertyPolicyQos
     *
     * @return PropertyPolicyQos reference
     */
    const PropertyPolicyQos& properties() const
    {
        return properties_;
    }

    /**
     * Getter for PropertyPolicyQos
     *
     * @return PropertyPolicyQos reference
     */
    PropertyPolicyQos& properties()
    {
        return properties_;
    }

    /**
     * Setter for PropertyPolicyQos
     *
     * @param properties PropertyPolicyQos
     */
    void properties(
            const PropertyPolicyQos& properties)
    {
        properties_ = properties;
    }

    /**
     * Getter for WireProtocolConfigQos
     *
     * @return WireProtocolConfigQos reference
     */
    const WireProtocolConfigQos& wire_protocol() const
    {
        return wire_protocol_;
    }

    /**
     * Getter for WireProtocolConfigQos
     *
     * @return WireProtocolConfigQos reference
     */
    WireProtocolConfigQos& wire_protocol()
    {
        return wire_protocol_;
    }

    /**
     * Setter for WireProtocolConfigQos
     *
     * @param wire_protocol WireProtocolConfigQos
     */
    void wire_protocol(
            const WireProtocolConfigQos& wire_protocol)
    {
        wire_protocol_ = wire_protocol;
    }

    /**
     * Getter for TransportConfigQos
     *
     * @return TransportConfigQos reference
     */
    const TransportConfigQos& transport() const
    {
        return transport_;
    }

    /**
     * Getter for TransportConfigQos
     *
     * @return TransportConfigQos reference
     */
    TransportConfigQos& transport()
    {
        return transport_;
    }

    /**
     * Setter for TransportConfigQos
     *
     * @param transport TransportConfigQos
     */
    void transport(
            const TransportConfigQos& transport)
    {
        transport_ = transport;
    }

    /**
     * Getter for the Participant name
     *
     * @return name
     */
    const fastcdr::string_255& name() const
    {
        return name_;
    }

    /**
     * Getter for the Participant name
     *
     * @return name
     */
    fastcdr::string_255& name()
    {
        return name_;
    }

    /**
     * Setter for the Participant name
     *
     * @param value New name to be set
     */
    void name(
            const fastcdr::string_255& value)
    {
        name_ = value;
    }

    /**
     * Getter for FlowControllerDescriptorList
     *
     * @return FlowControllerDescriptorList reference
     */
    FlowControllerDescriptorList& flow_controllers()
    {
        return flow_controllers_;
    }

    /**
     * Compares the flow controllers of two DomainParticipantQos element-wise.
     *
     * @param qos The DomainParticipantQos to compare with.
     * @return true if the flow controllers are the same, false otherwise.
     */
    FASTDDS_EXPORTED_API bool compare_flow_controllers(
            const DomainParticipantQos& qos) const;

    /**
     * Getter for FlowControllerDescriptorList
     *
     * @return FlowControllerDescriptorList reference
     */
    const FlowControllerDescriptorList& flow_controllers() const
    {
        return flow_controllers_;
    }

    /**
     * Getter for builtin flow controllers sender threads ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& builtin_controllers_sender_thread()
    {
        return builtin_controllers_sender_thread_;
    }

    /**
     * Getter for builtin flow controllers sender threads ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& builtin_controllers_sender_thread() const
    {
        return builtin_controllers_sender_thread_;
    }

    /**
     * Provides a way of easily configuring transport related configuration on certain pre-defined scenarios with
     * certain options.
     *
     * @param transports Defines the transport configuration scenario to setup.
     * @param options Defines the options to be used in the transport configuration.
     */
    FASTDDS_EXPORTED_API void setup_transports(
            rtps::BuiltinTransports transports,
            const rtps::BuiltinTransportsOptions& options = rtps::BuiltinTransportsOptions());

    /**
     * Setter for the builtin flow controllers sender threads ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void builtin_controllers_sender_thread(
            const rtps::ThreadSettings& value)
    {
        builtin_controllers_sender_thread_ = value;
    }

    /**
     * Getter for timed event ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& timed_events_thread()
    {
        return timed_events_thread_;
    }

    /**
     * Getter for timed event ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& timed_events_thread() const
    {
        return timed_events_thread_;
    }

    /**
     * Setter for the timed event ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void timed_events_thread(
            const rtps::ThreadSettings& value)
    {
        timed_events_thread_ = value;
    }

    /**
     * Getter for discovery server ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& discovery_server_thread()
    {
        return discovery_server_thread_;
    }

    /**
     * Getter for discovery server ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& discovery_server_thread() const
    {
        return discovery_server_thread_;
    }

    /**
     * Setter for the discovery server ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void discovery_server_thread(
            const rtps::ThreadSettings& value)
    {
        discovery_server_thread_ = value;
    }

    /**
     * Getter for TypeLookup service ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& typelookup_service_thread()
    {
        return typelookup_service_thread_;
    }

    /**
     * Getter for TypeLookup service ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& typelookup_service_thread() const
    {
        return typelookup_service_thread_;
    }

    /**
     * Setter for the TypeLookup service ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void typelookup_service_thread(
            const rtps::ThreadSettings& value)
    {
        typelookup_service_thread_ = value;
    }

#if HAVE_SECURITY
    /**
     * Getter for security log ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    rtps::ThreadSettings& security_log_thread()
    {
        return security_log_thread_;
    }

    /**
     * Getter for security log ThreadSettings
     *
     * @return rtps::ThreadSettings reference
     */
    const rtps::ThreadSettings& security_log_thread() const
    {
        return security_log_thread_;
    }

    /**
     * Setter for the security log ThreadSettings
     *
     * @param value New ThreadSettings to be set
     */
    void security_log_thread(
            const rtps::ThreadSettings& value)
    {
        security_log_thread_ = value;
    }

#endif // if HAVE_SECURITY

private:

    //!UserData Qos, implemented in the library.
    UserDataQosPolicy user_data_;

    //!EntityFactory Qos, implemented in the library.
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
    fastcdr::string_255 name_ = "RTPSParticipant";

    /*! User defined flow controller to use alongside.
     *
     *  @since 2.4.0
     */
    FlowControllerDescriptorList flow_controllers_;

    //! Thread settings for the builtin flow controllers sender threads
    rtps::ThreadSettings builtin_controllers_sender_thread_;

    //! Thread settings for the timed events thread
    rtps::ThreadSettings timed_events_thread_;

    //! Thread settings for the discovery server thread
    rtps::ThreadSettings discovery_server_thread_;

    //! Thread settings for the builtin TypeLookup service requests and replies threads
    rtps::ThreadSettings typelookup_service_thread_;

#if HAVE_SECURITY
    //! Thread settings for the security log thread
    rtps::ThreadSettings security_log_thread_;
#endif // if HAVE_SECURITY

};

FASTDDS_EXPORTED_API extern const DomainParticipantQos PARTICIPANT_QOS_DEFAULT;


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_DOMAIN_QOS__DOMAINPARTICIPANTQOS_HPP
