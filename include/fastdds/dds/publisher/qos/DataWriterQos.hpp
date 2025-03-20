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
 * @file DataWriterQos.hpp
 */


#ifndef FASTDDS_DDS_PUBLISHER_QOS__DATAWRITERQOS_HPP
#define FASTDDS_DDS_PUBLISHER_QOS__DATAWRITERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/policy/RTPSReliableWriterQos.hpp>
#include <fastdds/dds/core/policy/WriterDataLifecycleQosPolicy.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DataWriterQos, containing all the possible Qos that can be set for a determined DataWriter.
 * Although these values can be  and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 *
 * @ingroup FASTDDS_QOS_MODULE
 */
class DataWriterQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DataWriterQos();

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API ~DataWriterQos() = default;

    FASTDDS_EXPORTED_API bool operator ==(
            const DataWriterQos& b) const
    {
        return (this->durability_ == b.durability()) &&
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
               (this->user_data_ == b.user_data()) &&
               (this->ownership_ == b.ownership()) &&
               (this->ownership_strength_ == b.ownership_strength()) &&
               (this->writer_data_lifecycle_ == b.writer_data_lifecycle()) &&
               (this->publish_mode_ == b.publish_mode()) &&
               (this->representation_ == b.representation()) &&
               (this->properties_ == b.properties()) &&
               (this->reliable_writer_qos_ == b.reliable_writer_qos()) &&
               (this->endpoint_ == b.endpoint()) &&
               (this->writer_resource_limits_ == b.writer_resource_limits()) &&
               (this->data_sharing_ == b.data_sharing());
    }

    FASTDDS_EXPORTED_API WriterQos get_writerqos(
            const PublisherQos& pqos,
            const TopicQos& tqos) const;

    /**
     * Getter for DurabilityQosPolicy
     *
     * @return DurabilityQosPolicy reference
     */
    FASTDDS_EXPORTED_API DurabilityQosPolicy& durability()
    {
        return durability_;
    }

    /**
     * Getter for DurabilityQosPolicy
     *
     * @return DurabilityQosPolicy reference
     */
    FASTDDS_EXPORTED_API const DurabilityQosPolicy& durability() const
    {
        return durability_;
    }

    /**
     * Setter for DurabilityQosPolicy
     *
     * @param durability new value for the DurabilityQosPolicy
     */
    FASTDDS_EXPORTED_API void durability(
            const DurabilityQosPolicy& durability)
    {
        durability_ = durability;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     *
     * @return DurabilityServiceQosPolicy reference
     */
    FASTDDS_EXPORTED_API DurabilityServiceQosPolicy& durability_service()
    {
        return durability_service_;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     *
     * @return DurabilityServiceQosPolicy reference
     */
    FASTDDS_EXPORTED_API const DurabilityServiceQosPolicy& durability_service() const
    {
        return durability_service_;
    }

    /**
     * Setter for DurabilityServiceQosPolicy
     *
     * @param durability_service new value for the DurabilityServiceQosPolicy
     */
    FASTDDS_EXPORTED_API void durability_service(
            const DurabilityServiceQosPolicy& durability_service)
    {
        durability_service_ = durability_service;
    }

    /**
     * Getter for DeadlineQosPolicy
     *
     * @return DeadlineQosPolicy reference
     */
    FASTDDS_EXPORTED_API DeadlineQosPolicy& deadline()
    {
        return deadline_;
    }

    /**
     * Getter for DeadlineQosPolicy
     *
     * @return DeadlineQosPolicy reference
     */
    FASTDDS_EXPORTED_API const DeadlineQosPolicy& deadline() const
    {
        return deadline_;
    }

    /**
     * Setter for DeadlineQosPolicy
     *
     * @param deadline new value for the DeadlineQosPolicy
     */
    FASTDDS_EXPORTED_API void deadline(
            const DeadlineQosPolicy& deadline)
    {
        deadline_ = deadline;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     *
     * @return LatencyBudgetQosPolicy reference
     */
    FASTDDS_EXPORTED_API LatencyBudgetQosPolicy& latency_budget()
    {
        return latency_budget_;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     *
     * @return LatencyBudgetQosPolicy reference
     */
    FASTDDS_EXPORTED_API const LatencyBudgetQosPolicy& latency_budget() const
    {
        return latency_budget_;
    }

    /**
     * Setter for LatencyBudgetQosPolicy
     *
     * @param latency_budget new value for the LatencyBudgetQosPolicy
     */
    FASTDDS_EXPORTED_API void latency_budget(
            const LatencyBudgetQosPolicy& latency_budget)
    {
        latency_budget_ = latency_budget;
    }

    /**
     * Getter for LivelinessQosPolicy
     *
     * @return LivelinessQosPolicy reference
     */
    FASTDDS_EXPORTED_API LivelinessQosPolicy& liveliness()
    {
        return liveliness_;
    }

    /**
     * Getter for LivelinessQosPolicy
     *
     * @return LivelinessQosPolicy reference
     */
    FASTDDS_EXPORTED_API const LivelinessQosPolicy& liveliness() const
    {
        return liveliness_;
    }

    /**
     * Setter for LivelinessQosPolicy
     *
     * @param liveliness new value for the LivelinessQosPolicy
     */
    FASTDDS_EXPORTED_API void liveliness(
            const LivelinessQosPolicy& liveliness)
    {
        liveliness_ = liveliness;
    }

    /**
     * Getter for ReliabilityQosPolicy
     *
     * @return ReliabilityQosPolicy reference
     */
    FASTDDS_EXPORTED_API ReliabilityQosPolicy& reliability()
    {
        return reliability_;
    }

    /**
     * Getter for ReliabilityQosPolicy
     *
     * @return ReliabilityQosPolicy reference
     */
    FASTDDS_EXPORTED_API const ReliabilityQosPolicy& reliability() const
    {
        return reliability_;
    }

    /**
     * Setter for ReliabilityQosPolicy
     *
     * @param reliability new value for the ReliabilityQosPolicy
     */
    FASTDDS_EXPORTED_API void reliability(
            const ReliabilityQosPolicy& reliability)
    {
        reliability_ = reliability;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     *
     * @return DestinationOrderQosPolicy reference
     */
    FASTDDS_EXPORTED_API DestinationOrderQosPolicy& destination_order()
    {
        return destination_order_;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     *
     * @return DestinationOrderQosPolicy reference
     */
    FASTDDS_EXPORTED_API const DestinationOrderQosPolicy& destination_order() const
    {
        return destination_order_;
    }

    /**
     * Setter for DestinationOrderQosPolicy
     *
     * @param destination_order new value for the DestinationOrderQosPolicy
     */
    FASTDDS_EXPORTED_API void destination_order(
            const DestinationOrderQosPolicy& destination_order)
    {
        destination_order_ = destination_order;
    }

    /**
     * Getter for HistoryQosPolicy
     *
     * @return HistoryQosPolicy reference
     */
    FASTDDS_EXPORTED_API HistoryQosPolicy& history()
    {
        return history_;
    }

    /**
     * Getter for HistoryQosPolicy
     *
     * @return HistoryQosPolicy reference
     */
    FASTDDS_EXPORTED_API const HistoryQosPolicy& history() const
    {
        return history_;
    }

    /**
     * Setter for HistoryQosPolicy
     *
     * @param history new value for the HistoryQosPolicy
     */
    FASTDDS_EXPORTED_API void history(
            const HistoryQosPolicy& history)
    {
        history_ = history;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     *
     * @return ResourceLimitsQosPolicy reference
     */
    FASTDDS_EXPORTED_API ResourceLimitsQosPolicy& resource_limits()
    {
        return resource_limits_;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     *
     * @return ResourceLimitsQosPolicy reference
     */
    FASTDDS_EXPORTED_API const ResourceLimitsQosPolicy& resource_limits() const
    {
        return resource_limits_;
    }

    /**
     * Setter for ResourceLimitsQosPolicy
     *
     * @param resource_limits new value for the ResourceLimitsQosPolicy
     */
    FASTDDS_EXPORTED_API void resource_limits(
            const ResourceLimitsQosPolicy& resource_limits)
    {
        resource_limits_ = resource_limits;
    }

    /**
     * Getter for TransportPriorityQosPolicy
     *
     * @return TransportPriorityQosPolicy reference
     */
    FASTDDS_EXPORTED_API TransportPriorityQosPolicy& transport_priority()
    {
        return transport_priority_;
    }

    /**
     * Getter for TransportPriorityQosPolicy
     *
     * @return TransportPriorityQosPolicy reference
     */
    FASTDDS_EXPORTED_API const TransportPriorityQosPolicy& transport_priority() const
    {
        return transport_priority_;
    }

    /**
     * Setter for TransportPriorityQosPolicy
     *
     * @param transport_priority new value for the TransportPriorityQosPolicy
     */
    FASTDDS_EXPORTED_API void transport_priority(
            const TransportPriorityQosPolicy& transport_priority)
    {
        transport_priority_ = transport_priority;
    }

    /**
     * Getter for LifespanQosPolicy
     *
     * @return LifespanQosPolicy reference
     */
    FASTDDS_EXPORTED_API LifespanQosPolicy& lifespan()
    {
        return lifespan_;
    }

    /**
     * Getter for LifespanQosPolicy
     *
     * @return LifespanQosPolicy reference
     */
    FASTDDS_EXPORTED_API const LifespanQosPolicy& lifespan() const
    {
        return lifespan_;
    }

    /**
     * Setter for LifespanQosPolicy
     *
     * @param lifespan new value for the LifespanQosPolicy
     */
    FASTDDS_EXPORTED_API void lifespan(
            const LifespanQosPolicy& lifespan)
    {
        lifespan_ = lifespan;
    }

    /**
     * Getter for UserDataQosPolicy
     *
     * @return UserDataQosPolicy reference
     */
    FASTDDS_EXPORTED_API UserDataQosPolicy& user_data()
    {
        return user_data_;
    }

    /**
     * Getter for UserDataQosPolicy
     *
     * @return UserDataQosPolicy reference
     */
    FASTDDS_EXPORTED_API const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    /**
     * Setter for UserDataQosPolicy
     *
     * @param user_data new value for the UserDataQosPolicy
     */
    FASTDDS_EXPORTED_API void user_data(
            const UserDataQosPolicy& user_data)
    {
        user_data_ = user_data;
    }

    /**
     * Getter for OwnershipQosPolicy
     *
     * @return OwnershipQosPolicy reference
     */
    FASTDDS_EXPORTED_API OwnershipQosPolicy& ownership()
    {
        return ownership_;
    }

    /**
     * Getter for OwnershipQosPolicy
     *
     * @return OwnershipQosPolicy reference
     */
    FASTDDS_EXPORTED_API const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    /**
     * Setter for OwnershipQosPolicy
     *
     * @param ownership new value for the OwnershipQosPolicy
     */
    FASTDDS_EXPORTED_API void ownership(
            const OwnershipQosPolicy& ownership)
    {
        ownership_ = ownership;
    }

    /**
     * Getter for OwnershipStrengthQosPolicy
     *
     * @return OwnershipStrengthQosPolicy reference
     */
    FASTDDS_EXPORTED_API OwnershipStrengthQosPolicy& ownership_strength()
    {
        return ownership_strength_;
    }

    /**
     * Getter for OwnershipStrengthQosPolicy
     *
     * @return OwnershipStrengthQosPolicy reference
     */
    FASTDDS_EXPORTED_API const OwnershipStrengthQosPolicy& ownership_strength() const
    {
        return ownership_strength_;
    }

    /**
     * Setter for OwnershipStrengthQosPolicy
     *
     * @param ownership_strength new value for the OwnershipStrengthQosPolicy
     */
    FASTDDS_EXPORTED_API void ownership_strength(
            const OwnershipStrengthQosPolicy& ownership_strength)
    {
        ownership_strength_ = ownership_strength;
    }

    /**
     * Getter for WriterDataLifecycleQosPolicy
     *
     * @return WriterDataLifecycleQosPolicy reference
     */
    FASTDDS_EXPORTED_API WriterDataLifecycleQosPolicy& writer_data_lifecycle()
    {
        return writer_data_lifecycle_;
    }

    /**
     * Getter for WriterDataLifecycleQosPolicy
     *
     * @return WriterDataLifecycleQosPolicy reference
     */
    FASTDDS_EXPORTED_API const WriterDataLifecycleQosPolicy& writer_data_lifecycle() const
    {
        return writer_data_lifecycle_;
    }

    /**
     * Setter for WriterDataLifecycleQosPolicy
     *
     * @param writer_data_lifecycle new value for the WriterDataLifecycleQosPolicy
     */
    FASTDDS_EXPORTED_API void writer_data_lifecycle(
            const WriterDataLifecycleQosPolicy& writer_data_lifecycle)
    {
        writer_data_lifecycle_ = writer_data_lifecycle;
    }

    /**
     * Getter for PublishModeQosPolicy
     *
     * @return PublishModeQosPolicy reference
     */
    FASTDDS_EXPORTED_API PublishModeQosPolicy& publish_mode()
    {
        return publish_mode_;
    }

    /**
     * Getter for PublishModeQosPolicy
     *
     * @return PublishModeQosPolicy reference
     */
    FASTDDS_EXPORTED_API const PublishModeQosPolicy& publish_mode() const
    {
        return publish_mode_;
    }

    /**
     * Setter for PublishModeQosPolicy
     *
     * @param publish_mode new value for the PublishModeQosPolicy
     */
    FASTDDS_EXPORTED_API void publish_mode(
            const PublishModeQosPolicy& publish_mode)
    {
        publish_mode_ = publish_mode;
    }

    /**
     * Getter for DataRepresentationQosPolicy
     *
     * @return DataRepresentationQosPolicy reference
     */
    FASTDDS_EXPORTED_API DataRepresentationQosPolicy& representation()
    {
        return representation_;
    }

    /**
     * Getter for DataRepresentationQosPolicy
     *
     * @return DataRepresentationQosPolicy reference
     */
    FASTDDS_EXPORTED_API const DataRepresentationQosPolicy& representation() const
    {
        return representation_;
    }

    /**
     * Setter for DataRepresentationQosPolicy
     *
     * @param representation new value for the DataRepresentationQosPolicy
     */
    FASTDDS_EXPORTED_API void representation(
            const DataRepresentationQosPolicy& representation)
    {
        representation_ = representation;
    }

    /**
     * Getter for PropertyPolicyQos
     *
     * @return PropertyPolicyQos reference
     */
    FASTDDS_EXPORTED_API PropertyPolicyQos& properties()
    {
        return properties_;
    }

    /**
     * Getter for PropertyPolicyQos
     *
     * @return PropertyPolicyQos reference
     */
    FASTDDS_EXPORTED_API const PropertyPolicyQos& properties() const
    {
        return properties_;
    }

    /**
     * Setter for PropertyPolicyQos
     *
     * @param properties new value for the PropertyPolicyQos
     */
    FASTDDS_EXPORTED_API void properties(
            const PropertyPolicyQos& properties)
    {
        properties_ = properties;
    }

    /**
     * Getter for RTPSReliableWriterQos
     *
     * @return RTPSReliableWriterQos reference
     */
    FASTDDS_EXPORTED_API RTPSReliableWriterQos& reliable_writer_qos()
    {
        return reliable_writer_qos_;
    }

    /**
     * Getter for RTPSReliableWriterQos
     *
     * @return RTPSReliableWriterQos reference
     */
    FASTDDS_EXPORTED_API const RTPSReliableWriterQos& reliable_writer_qos() const
    {
        return reliable_writer_qos_;
    }

    /**
     * Setter for RTPSReliableWriterQos
     *
     * @param reliable_writer_qos new value for the RTPSReliableWriterQos
     */
    FASTDDS_EXPORTED_API void reliable_writer_qos(
            const RTPSReliableWriterQos& reliable_writer_qos)
    {
        reliable_writer_qos_ = reliable_writer_qos;
    }

    /**
     * Getter for RTPSEndpointQos
     *
     * @return RTPSEndpointQos reference
     */
    FASTDDS_EXPORTED_API RTPSEndpointQos& endpoint()
    {
        return endpoint_;
    }

    /**
     * Getter for RTPSEndpointQos
     * @return RTPSEndpointQos reference
     */
    FASTDDS_EXPORTED_API const RTPSEndpointQos& endpoint() const
    {
        return endpoint_;
    }

    /**
     * Setter for RTPSEndpointQos
     *
     * @param endpoint new value for the RTPSEndpointQos
     */
    FASTDDS_EXPORTED_API void endpoint(
            const RTPSEndpointQos& endpoint)
    {
        endpoint_ = endpoint;
    }

    /**
     * Getter for WriterResourceLimitsQos
     *
     * @return WriterResourceLimitsQos reference
     */
    FASTDDS_EXPORTED_API WriterResourceLimitsQos& writer_resource_limits()
    {
        return writer_resource_limits_;
    }

    /**
     * Getter for WriterResourceLimitsQos
     *
     * @return WriterResourceLimitsQos reference
     */
    FASTDDS_EXPORTED_API const WriterResourceLimitsQos& writer_resource_limits() const
    {
        return writer_resource_limits_;
    }

    /**
     * Setter for WriterResourceLimitsQos
     *
     * @param writer_resource_limits new value for the WriterResourceLimitsQos
     */
    FASTDDS_EXPORTED_API void writer_resource_limits(
            const WriterResourceLimitsQos& writer_resource_limits)
    {
        writer_resource_limits_ = writer_resource_limits;
    }

    /**
     * Getter for DataSharingQosPolicy
     *
     * @return DataSharingQosPolicy reference
     */
    FASTDDS_EXPORTED_API DataSharingQosPolicy& data_sharing()
    {
        return data_sharing_;
    }

    /**
     * Getter for DataSharingQosPolicy
     *
     * @return DataSharingQosPolicy reference
     */
    FASTDDS_EXPORTED_API const DataSharingQosPolicy& data_sharing() const
    {
        return data_sharing_;
    }

    /**
     * Setter for DataSharingQosPolicy
     *
     * @param data_sharing new value for the DataSharingQosPolicy
     */
    FASTDDS_EXPORTED_API void data_sharing(
            const DataSharingQosPolicy& data_sharing)
    {
        data_sharing_ = data_sharing;
    }

private:

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

    //!User Data Qos, implemented in the library.
    UserDataQosPolicy user_data_;

    //!Ownership Qos, implemented in the library.
    OwnershipQosPolicy ownership_;

    //!Ownership Strength Qos, implemented in the library.
    OwnershipStrengthQosPolicy ownership_strength_;

    //!Writer Data Lifecycle Qos, NOT implemented in the library.
    WriterDataLifecycleQosPolicy writer_data_lifecycle_;

    //!Publication Mode Qos, implemented in the library.
    PublishModeQosPolicy publish_mode_;

    //!Data Representation Qos, implemented in the library.
    DataRepresentationQosPolicy representation_;

    //!Property policies
    PropertyPolicyQos properties_;

    //!RTPS Reliable Writer Qos
    RTPSReliableWriterQos reliable_writer_qos_;

    //!RTPS Endpoint Qos
    RTPSEndpointQos endpoint_;

    //!Writer Resource Limits Qos
    WriterResourceLimitsQos writer_resource_limits_;

    //!DataSharing configuration
    DataSharingQosPolicy data_sharing_;
};

FASTDDS_EXPORTED_API extern const DataWriterQos DATAWRITER_QOS_DEFAULT;
FASTDDS_EXPORTED_API extern const DataWriterQos DATAWRITER_QOS_USE_TOPIC_QOS;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_PUBLISHER_QOS__DATAWRITERQOS_HPP
