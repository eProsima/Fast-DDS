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
 * @file DataReaderQos.hpp
 */


#ifndef FASTDDS_DDS_SUBSCRIBER_QOS__DATAREADERQOS_HPP
#define FASTDDS_DDS_SUBSCRIBER_QOS__DATAREADERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/policy/ReaderDataLifecycleQosPolicy.hpp>
#include <fastdds/dds/core/policy/ReaderResourceLimitsQos.hpp>
#include <fastdds/dds/core/policy/RTPSReliableReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class DataReaderQos, containing all the possible Qos that can be set for a determined DataReader.
 * Although these values can be set and are transmitted
 * during the Endpoint Discovery Protocol, not all of the behaviour associated with them has been implemented in the library.
 * Please consult each of them to check for implementation details and default values.
 * @ingroup FASTDDS_QOS_MODULE
 */
class DataReaderQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DataReaderQos()
        : expects_inline_qos_(false)
    {
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const DataReaderQos& b) const
    {
        return (durability_ == b.durability()) &&
               (deadline_ == b.deadline()) &&
               (latency_budget_ == b.latency_budget()) &&
               (liveliness_ == b.liveliness()) &&
               (reliability_ == b.reliability()) &&
               (destination_order_ == b.destination_order()) &&
               (history_ == b.history()) &&
               (resource_limits_ == b.resource_limits()) &&
               (user_data_ == b.user_data()) &&
               (ownership_ == b.ownership()) &&
               (time_based_filter_ == b.time_based_filter()) &&
               (reader_data_lifecycle_ == b.reader_data_lifecycle()) &&
               (lifespan_ == b.lifespan()) &&
               (durability_service_ == b.durability_service()) &&
               (reliable_reader_qos_ == b.reliable_reader_qos()) &&
               (type_consistency_ == b.type_consistency()) &&
               (representation_ == b.representation()) &&
               (expects_inline_qos_ == b.expects_inline_qos()) &&
               (properties_ == b.properties()) &&
               (endpoint_ == b.endpoint()) &&
               (reader_resource_limits_ == b.reader_resource_limits()) &&
               (data_sharing_ == b.data_sharing());
    }

    FASTDDS_EXPORTED_API ReaderQos get_readerqos(
            const SubscriberQos& sqos) const;

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
     * @return DurabilityQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const DurabilityQosPolicy& durability() const
    {
        return durability_;
    }

    /**
     * Setter for DurabilityQosPolicy
     *
     * @param new_value new value for the DurabilityQosPolicy
     */
    FASTDDS_EXPORTED_API void durability(
            const DurabilityQosPolicy& new_value)
    {
        durability_ = new_value;
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
     * @return DeadlineQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const DeadlineQosPolicy& deadline() const
    {
        return deadline_;
    }

    /**
     * Setter for DeadlineQosPolicy
     *
     * @param new_value new value for the DeadlineQosPolicy
     */
    FASTDDS_EXPORTED_API void deadline(
            const DeadlineQosPolicy& new_value)
    {
        deadline_ = new_value;
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
     * @return LatencyBudgetQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const LatencyBudgetQosPolicy& latency_budget() const
    {
        return latency_budget_;
    }

    /**
     * Setter for LatencyBudgetQosPolicy
     *
     * @param new_value new value for the LatencyBudgetQosPolicy
     */
    FASTDDS_EXPORTED_API void latency_budget(
            const LatencyBudgetQosPolicy& new_value)
    {
        latency_budget_ = new_value;
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
     * @return LivelinessQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const LivelinessQosPolicy& liveliness() const
    {
        return liveliness_;
    }

    /**
     * Setter for LivelinessQosPolicy
     *
     * @param new_value new value for the LivelinessQosPolicy
     */
    FASTDDS_EXPORTED_API void liveliness(
            const LivelinessQosPolicy& new_value)
    {
        liveliness_ = new_value;
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
     * @return ReliabilityQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const ReliabilityQosPolicy& reliability() const
    {
        return reliability_;
    }

    /**
     * Setter for ReliabilityQosPolicy
     *
     * @param new_value new value for the ReliabilityQosPolicy
     */
    FASTDDS_EXPORTED_API void reliability(
            const ReliabilityQosPolicy& new_value)
    {
        reliability_ = new_value;
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
     * @return DestinationOrderQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const DestinationOrderQosPolicy& destination_order() const
    {
        return destination_order_;
    }

    /**
     * Setter for DestinationOrderQosPolicy
     *
     * @param new_value new value for the DestinationOrderQosPolicy
     */
    FASTDDS_EXPORTED_API void destination_order(
            const DestinationOrderQosPolicy& new_value)
    {
        destination_order_ = new_value;
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
     * @return HistoryQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const HistoryQosPolicy& history() const
    {
        return history_;
    }

    /**
     * Setter for HistoryQosPolicy
     *
     * @param new_value new value for the HistoryQosPolicy
     */
    FASTDDS_EXPORTED_API void history(
            const HistoryQosPolicy& new_value)
    {
        history_ = new_value;
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
     * @return ResourceLimitsQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const ResourceLimitsQosPolicy& resource_limits() const
    {
        return resource_limits_;
    }

    /**
     * Setter for ResourceLimitsQosPolicy
     *
     * @param new_value new value for the ResourceLimitsQosPolicy
     */
    FASTDDS_EXPORTED_API void resource_limits(
            const ResourceLimitsQosPolicy& new_value)
    {
        resource_limits_ = new_value;
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
     * @return UserDataQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    /**
     * Setter for UserDataQosPolicy
     *
     * @param new_value new value for the UserDataQosPolicy
     */
    FASTDDS_EXPORTED_API void user_data(
            const UserDataQosPolicy& new_value)
    {
        user_data_ = new_value;
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
     * @return OwnershipQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    /**
     * Setter for OwnershipQosPolicy
     *
     * @param new_value new value for the OwnershipQosPolicy
     */
    FASTDDS_EXPORTED_API void ownership(
            const OwnershipQosPolicy& new_value)
    {
        ownership_ = new_value;
    }

    /**
     * Getter for TimeBasedFilterQosPolicy
     *
     * @return TimeBasedFilterQosPolicy reference
     */
    FASTDDS_EXPORTED_API TimeBasedFilterQosPolicy& time_based_filter()
    {
        return time_based_filter_;
    }

    /**
     * Getter for TimeBasedFilterQosPolicy
     *
     * @return TimeBasedFilterQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const TimeBasedFilterQosPolicy& time_based_filter() const
    {
        return time_based_filter_;
    }

    /**
     * Setter for TimeBasedFilterQosPolicy
     *
     * @param new_value new value for the TimeBasedFilterQosPolicy
     */
    FASTDDS_EXPORTED_API void time_based_filter(
            const TimeBasedFilterQosPolicy& new_value)
    {
        time_based_filter_ = new_value;
    }

    /**
     * Getter for ReaderDataLifecycleQosPolicy
     *
     * @return ReaderDataLifecycleQosPolicy reference
     */
    FASTDDS_EXPORTED_API ReaderDataLifecycleQosPolicy& reader_data_lifecycle()
    {
        return reader_data_lifecycle_;
    }

    /**
     * Getter for ReaderDataLifecycleQosPolicy
     *
     * @return ReaderDataLifecycleQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const ReaderDataLifecycleQosPolicy& reader_data_lifecycle() const
    {
        return reader_data_lifecycle_;
    }

    /**
     * Setter for ReaderDataLifecycleQosPolicy
     *
     * @param new_value new value for the ReaderDataLifecycleQosPolicy
     */
    FASTDDS_EXPORTED_API void reader_data_lifecycle(
            const ReaderDataLifecycleQosPolicy& new_value)
    {
        reader_data_lifecycle_ = new_value;
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
     * @return LifespanQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const LifespanQosPolicy& lifespan() const
    {
        return lifespan_;
    }

    /**
     * Setter for LifespanQosPolicy
     *
     * @param new_value new value for the LifespanQosPolicy
     */
    FASTDDS_EXPORTED_API void lifespan(
            const LifespanQosPolicy& new_value)
    {
        lifespan_ = new_value;
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
     * @return DurabilityServiceQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const DurabilityServiceQosPolicy& durability_service() const
    {
        return durability_service_;
    }

    /**
     * Setter for DurabilityServiceQosPolicy
     *
     * @param new_value new value for the DurabilityServiceQosPolicy
     */
    FASTDDS_EXPORTED_API void durability_service(
            const DurabilityServiceQosPolicy& new_value)
    {
        durability_service_ = new_value;
    }

    /**
     * Getter for RTPSReliableReaderQos
     *
     * @return RTPSReliableReaderQos reference
     */
    FASTDDS_EXPORTED_API RTPSReliableReaderQos& reliable_reader_qos()
    {
        return reliable_reader_qos_;
    }

    /**
     * Getter for RTPSReliableReaderQos
     *
     * @return RTPSReliableReaderQos const reference
     */
    FASTDDS_EXPORTED_API const RTPSReliableReaderQos& reliable_reader_qos() const
    {
        return reliable_reader_qos_;
    }

    /**
     * Setter for RTPSReliableReaderQos
     *
     * @param new_value new value for the RTPSReliableReaderQos
     */
    FASTDDS_EXPORTED_API void reliable_reader_qos(
            const RTPSReliableReaderQos& new_value)
    {
        reliable_reader_qos_ = new_value;
    }

    /**
     * Getter for TypeConsistencyEnforcementQosPolicy
     *
     * @return TypeConsistencyEnforcementQosPolicy reference
     */
    FASTDDS_EXPORTED_API TypeConsistencyEnforcementQosPolicy& type_consistency()
    {
        return type_consistency_;
    }

    /**
     * Getter for TypeConsistencyEnforcementQosPolicy
     *
     * @return TypeConsistencyEnforcementQosPolicy const reference
     */
    FASTDDS_EXPORTED_API const TypeConsistencyEnforcementQosPolicy& type_consistency() const
    {
        return type_consistency_;
    }

    /**
     * Setter for TypeConsistencyEnforcementQosPolicy
     *
     * @param new_value new value for the TypeConsistencyEnforcementQosPolicy
     */
    FASTDDS_EXPORTED_API void type_consistency(
            const TypeConsistencyEnforcementQosPolicy& new_value)
    {
        type_consistency_ = new_value;
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
     * Getter for DataRepresentationQosPolicy
     *
     * @return DataRepresentationQosPolicy reference
     */
    FASTDDS_EXPORTED_API DataRepresentationQosPolicy& representation()
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
     * Getter for expects_inline_qos
     *
     * @return expects_inline_qos
     */
    FASTDDS_EXPORTED_API bool expects_inline_qos() const
    {
        return expects_inline_qos_;
    }

    /**
     * Setter for expects_inline_qos
     *
     * @param new_value new value for the expects_inline_qos
     */
    FASTDDS_EXPORTED_API void expects_inline_qos(
            bool new_value)
    {
        expects_inline_qos_ = new_value;
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
     * @return PropertyPolicyQos const reference
     */
    FASTDDS_EXPORTED_API const PropertyPolicyQos& properties() const
    {
        return properties_;
    }

    /**
     * Setter for PropertyPolicyQos
     *
     * @param new_value new value for the PropertyPolicyQos
     */
    FASTDDS_EXPORTED_API void properties(
            const PropertyPolicyQos& new_value)
    {
        properties_ = new_value;
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
     *
     * @return RTPSEndpointQos const reference
     */
    FASTDDS_EXPORTED_API const RTPSEndpointQos& endpoint() const
    {
        return endpoint_;
    }

    /**
     * Setter for RTPSEndpointQos
     *
     * @param new_value new value for the RTPSEndpointQos
     */
    FASTDDS_EXPORTED_API void endpoint(
            const RTPSEndpointQos& new_value)
    {
        endpoint_ = new_value;
    }

    /**
     * Getter for ReaderResourceLimitsQos
     *
     * @return ReaderResourceLimitsQos reference
     */
    FASTDDS_EXPORTED_API ReaderResourceLimitsQos& reader_resource_limits()
    {
        return reader_resource_limits_;
    }

    /**
     * Getter for ReaderResourceLimitsQos
     *
     * @return ReaderResourceLimitsQos const reference
     */
    FASTDDS_EXPORTED_API const ReaderResourceLimitsQos& reader_resource_limits() const
    {
        return reader_resource_limits_;
    }

    /**
     * Setter for ReaderResourceLimitsQos
     *
     * @param new_value new value for the ReaderResourceLimitsQos
     */
    FASTDDS_EXPORTED_API void reader_resource_limits(
            const ReaderResourceLimitsQos& new_value)
    {
        reader_resource_limits_ = new_value;
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

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy deadline_;

    //!Latency Budget Qos, implemented in the library.
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

    //!User Data Qos, implemented in the library.
    UserDataQosPolicy user_data_;

    //!Ownership Qos, implemented in the library.
    OwnershipQosPolicy ownership_;

    //!Time Based Filter Qos, NOT implemented in the library.
    TimeBasedFilterQosPolicy time_based_filter_;

    //!Reader Data Lifecycle Qos, NOT implemented in the library.
    ReaderDataLifecycleQosPolicy reader_data_lifecycle_;

    //!Lifespan Qos (Extension).
    LifespanQosPolicy lifespan_;

    //!Durability Service Qos (Extension).
    DurabilityServiceQosPolicy durability_service_;

    //!Reliable reader configuration (Extension)
    RTPSReliableReaderQos reliable_reader_qos_;

    //! Type consistency (Extension)
    TypeConsistencyEnforcementQosPolicy type_consistency_;

    //! Data representation (Extension)
    DataRepresentationQosPolicy representation_;

    //!Expects Inline QOS (Extension).
    bool expects_inline_qos_;

    //!Properties (Extension).
    PropertyPolicyQos properties_;

    //!Endpoint configuration (Extension)
    RTPSEndpointQos endpoint_;

    //!ReaderResourceLimitsQos
    ReaderResourceLimitsQos reader_resource_limits_;

    //!DataSharing configuration (Extension)
    DataSharingQosPolicy data_sharing_;
};

FASTDDS_EXPORTED_API extern const DataReaderQos DATAREADER_QOS_DEFAULT;
FASTDDS_EXPORTED_API extern const DataReaderQos DATAREADER_QOS_USE_TOPIC_QOS;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_SUBSCRIBER_QOS__DATAREADERQOS_HPP
