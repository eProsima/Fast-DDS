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


#ifndef _FASTDDS_DATAREADERQOS_HPP
#define _FASTDDS_DATAREADERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/core/policy/ReaderDataLifecycleQosPolicy.hpp>

#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>


namespace eprosima {
namespace fastdds {
namespace dds {

using TopicAttributesQos = fastrtps::TopicAttributes;

//! Qos Policy to configure the DisablePositiveACKsQos and the reader attributes
class RTPSReliableReaderQos
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI RTPSReliableReaderQos()
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~RTPSReliableReaderQos() = default;

    bool operator ==(
            const RTPSReliableReaderQos& b) const
    {
        return (this->times == b.times) &&
               (this->disable_positive_ACKs == b.disable_positive_ACKs);
    }

    inline void clear()
    {
        RTPSReliableReaderQos reset = RTPSReliableReaderQos();
        std::swap(*this, reset);
    }

    /*!
     * @brief Times associated with the Reliable Readers events.
     */
    fastrtps::rtps::ReaderTimes times;

    /*!
     * @brief Control the sending of positive ACKs
     */
    DisablePositiveACKsQosPolicy disable_positive_ACKs;
};

//! Qos Policy to configure the limit of the reader resources
class ReaderResourceLimitsQos
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI ReaderResourceLimitsQos()
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~ReaderResourceLimitsQos() = default;

    bool operator ==(
            const ReaderResourceLimitsQos& b) const
    {
        return (this->matched_publisher_allocation == b.matched_publisher_allocation);
    }

    inline void clear()
    {
        ReaderResourceLimitsQos reset = ReaderResourceLimitsQos();
        std::swap(*this, reset);
    }

    //!Matched publishers allocation limits.
    fastrtps::ResourceLimitedContainerConfig matched_publisher_allocation;
};

//! Qos Policy to configure the XTypes Qos associated to the DataReader
class TypeConsistencyQos : public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TypeConsistencyQos()
        : QosPolicy(false)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TypeConsistencyQos() = default;

    bool operator ==(
            const TypeConsistencyQos& b) const
    {
        return (this->type_consistency == b.type_consistency) &&
               (this->representation == b.representation) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        TypeConsistencyQos reset = TypeConsistencyQos();
        std::swap(*this, reset);
    }

    //!Type consistency enforcement Qos.
    TypeConsistencyEnforcementQosPolicy type_consistency;

    //!Data Representation Qos.
    DataRepresentationQosPolicy representation;
};

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
    RTPS_DllAPI DataReaderQos()
        : expects_inline_qos_(false)
    {
    }

    RTPS_DllAPI bool operator ==(
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
               (expects_inline_qos_ == b.expects_inline_qos()) &&
               (properties_ == b.properties()) &&
               (endpoint_ == b.endpoint()) &&
               (reader_resource_limits_ == b.reader_resource_limits());
    }

    RTPS_DllAPI ReaderQos get_readerqos(
            const SubscriberQos& sqos) const;

    /**
     * Getter for DurabilityQosPolicy
     * @return DurabilityQosPolicy reference
     */
    RTPS_DllAPI DurabilityQosPolicy& durability()
    {
        return durability_;
    }

    /**
     * Getter for DurabilityQosPolicy
     * @return DurabilityQosPolicy const reference
     */
    RTPS_DllAPI const DurabilityQosPolicy& durability() const
    {
        return durability_;
    }

    /**
     * Setter for DurabilityQosPolicy
     * @param new_value new value for the DurabilityQosPolicy
     */
    RTPS_DllAPI void durability(
            const DurabilityQosPolicy& new_value)
    {
        durability_ = new_value;
    }

    /**
     * Getter for DeadlineQosPolicy
     * @return DeadlineQosPolicy reference
     */
    RTPS_DllAPI DeadlineQosPolicy& deadline()
    {
        return deadline_;
    }

    /**
     * Getter for DeadlineQosPolicy
     * @return DeadlineQosPolicy const reference
     */
    RTPS_DllAPI const DeadlineQosPolicy& deadline() const
    {
        return deadline_;
    }

    /**
     * Setter for DeadlineQosPolicy
     * @param new_value new value for the DeadlineQosPolicy
     */
    RTPS_DllAPI void deadline(
            const DeadlineQosPolicy& new_value)
    {
        deadline_ = new_value;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     * @return LatencyBudgetQosPolicy reference
     */
    RTPS_DllAPI LatencyBudgetQosPolicy& latency_budget()
    {
        return latency_budget_;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     * @return LatencyBudgetQosPolicy const reference
     */
    RTPS_DllAPI const LatencyBudgetQosPolicy& latency_budget() const
    {
        return latency_budget_;
    }

    /**
     * Setter for LatencyBudgetQosPolicy
     * @param new_value new value for the LatencyBudgetQosPolicy
     */
    RTPS_DllAPI void latency_budget(
            const LatencyBudgetQosPolicy& new_value)
    {
        latency_budget_ = new_value;
    }

    /**
     * Getter for LivelinessQosPolicy
     * @return LivelinessQosPolicy reference
     */
    RTPS_DllAPI LivelinessQosPolicy& liveliness()
    {
        return liveliness_;
    }

    /**
     * Getter for LivelinessQosPolicy
     * @return LivelinessQosPolicy const reference
     */
    RTPS_DllAPI const LivelinessQosPolicy& liveliness() const
    {
        return liveliness_;
    }

    /**
     * Setter for LivelinessQosPolicy
     * @param new_value new value for the LivelinessQosPolicy
     */
    RTPS_DllAPI void liveliness(
            const LivelinessQosPolicy& new_value)
    {
        liveliness_ = new_value;
    }

    /**
     * Getter for ReliabilityQosPolicy
     * @return ReliabilityQosPolicy reference
     */
    RTPS_DllAPI ReliabilityQosPolicy& reliability()
    {
        return reliability_;
    }

    /**
     * Getter for ReliabilityQosPolicy
     * @return ReliabilityQosPolicy const reference
     */
    RTPS_DllAPI const ReliabilityQosPolicy& reliability() const
    {
        return reliability_;
    }

    /**
     * Setter for ReliabilityQosPolicy
     * @param new_value new value for the ReliabilityQosPolicy
     */
    RTPS_DllAPI void reliability(
            const ReliabilityQosPolicy& new_value)
    {
        reliability_ = new_value;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     * @return DestinationOrderQosPolicy reference
     */
    RTPS_DllAPI DestinationOrderQosPolicy& destination_order()
    {
        return destination_order_;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     * @return DestinationOrderQosPolicy const reference
     */
    RTPS_DllAPI const DestinationOrderQosPolicy& destination_order() const
    {
        return destination_order_;
    }

    /**
     * Setter for DestinationOrderQosPolicy
     * @param new_value new value for the DestinationOrderQosPolicy
     */
    RTPS_DllAPI void destination_order(
            const DestinationOrderQosPolicy& new_value)
    {
        destination_order_ = new_value;
    }

    /**
     * Getter for HistoryQosPolicy
     * @return HistoryQosPolicy reference
     */
    RTPS_DllAPI HistoryQosPolicy& history()
    {
        return history_;
    }

    /**
     * Getter for HistoryQosPolicy
     * @return HistoryQosPolicy const reference
     */
    RTPS_DllAPI const HistoryQosPolicy& history() const
    {
        return history_;
    }

    /**
     * Setter for HistoryQosPolicy
     * @param new_value new value for the HistoryQosPolicy
     */
    RTPS_DllAPI void history(
            const HistoryQosPolicy& new_value)
    {
        history_ = new_value;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     * @return ResourceLimitsQosPolicy reference
     */
    RTPS_DllAPI ResourceLimitsQosPolicy& resource_limits()
    {
        return resource_limits_;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     * @return ResourceLimitsQosPolicy const reference
     */
    RTPS_DllAPI const ResourceLimitsQosPolicy& resource_limits() const
    {
        return resource_limits_;
    }

    /**
     * Setter for ResourceLimitsQosPolicy
     * @param new_value new value for the ResourceLimitsQosPolicy
     */
    RTPS_DllAPI void resource_limits(
            const ResourceLimitsQosPolicy& new_value)
    {
        resource_limits_ = new_value;
    }

    /**
     * Getter for UserDataQosPolicy
     * @return UserDataQosPolicy reference
     */
    RTPS_DllAPI UserDataQosPolicy& user_data()
    {
        return user_data_;
    }

    /**
     * Getter for UserDataQosPolicy
     * @return UserDataQosPolicy const reference
     */
    RTPS_DllAPI const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    /**
     * Setter for UserDataQosPolicy
     * @param new_value new value for the UserDataQosPolicy
     */
    RTPS_DllAPI void user_data(
            const UserDataQosPolicy& new_value)
    {
        user_data_ = new_value;
    }

    /**
     * Getter for OwnershipQosPolicy
     * @return OwnershipQosPolicy reference
     */
    RTPS_DllAPI OwnershipQosPolicy& ownership()
    {
        return ownership_;
    }

    /**
     * Getter for OwnershipQosPolicy
     * @return OwnershipQosPolicy const reference
     */
    RTPS_DllAPI const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    /**
     * Setter for OwnershipQosPolicy
     * @param new_value new value for the OwnershipQosPolicy
     */
    RTPS_DllAPI void ownership(
            const OwnershipQosPolicy& new_value)
    {
        ownership_ = new_value;
    }

    /**
     * Getter for TimeBasedFilterQosPolicy
     * @return TimeBasedFilterQosPolicy reference
     */
    RTPS_DllAPI TimeBasedFilterQosPolicy& time_based_filter()
    {
        return time_based_filter_;
    }

    /**
     * Getter for TimeBasedFilterQosPolicy
     * @return TimeBasedFilterQosPolicy const reference
     */
    RTPS_DllAPI const TimeBasedFilterQosPolicy& time_based_filter() const
    {
        return time_based_filter_;
    }

    /**
     * Setter for TimeBasedFilterQosPolicy
     * @param new_value new value for the TimeBasedFilterQosPolicy
     */
    RTPS_DllAPI void time_based_filter(
            const TimeBasedFilterQosPolicy& new_value)
    {
        time_based_filter_ = new_value;
    }

    /**
     * Getter for ReaderDataLifecycleQosPolicy
     * @return ReaderDataLifecycleQosPolicy reference
     */
    RTPS_DllAPI ReaderDataLifecycleQosPolicy& reader_data_lifecycle()
    {
        return reader_data_lifecycle_;
    }

    /**
     * Getter for ReaderDataLifecycleQosPolicy
     * @return ReaderDataLifecycleQosPolicy const reference
     */
    RTPS_DllAPI const ReaderDataLifecycleQosPolicy& reader_data_lifecycle() const
    {
        return reader_data_lifecycle_;
    }

    /**
     * Setter for ReaderDataLifecycleQosPolicy
     * @param new_value new value for the ReaderDataLifecycleQosPolicy
     */
    RTPS_DllAPI void reader_data_lifecycle(
            const ReaderDataLifecycleQosPolicy& new_value)
    {
        reader_data_lifecycle_ = new_value;
    }

    /**
     * Getter for LifespanQosPolicy
     * @return LifespanQosPolicy reference
     */
    RTPS_DllAPI LifespanQosPolicy& lifespan()
    {
        return lifespan_;
    }

    /**
     * Getter for LifespanQosPolicy
     * @return LifespanQosPolicy const reference
     */
    RTPS_DllAPI const LifespanQosPolicy& lifespan() const
    {
        return lifespan_;
    }

    /**
     * Setter for LifespanQosPolicy
     * @param new_value new value for the LifespanQosPolicy
     */
    RTPS_DllAPI void lifespan(
            const LifespanQosPolicy& new_value)
    {
        lifespan_ = new_value;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     * @return DurabilityServiceQosPolicy reference
     */
    RTPS_DllAPI DurabilityServiceQosPolicy& durability_service()
    {
        return durability_service_;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     * @return DurabilityServiceQosPolicy const reference
     */
    RTPS_DllAPI const DurabilityServiceQosPolicy& durability_service() const
    {
        return durability_service_;
    }

    /**
     * Setter for DurabilityServiceQosPolicy
     * @param new_value new value for the DurabilityServiceQosPolicy
     */
    RTPS_DllAPI void durability_service(
            const DurabilityServiceQosPolicy& new_value)
    {
        durability_service_ = new_value;
    }

    /**
     * Getter for RTPSReliableReaderQos
     * @return RTPSReliableReaderQos reference
     */
    RTPS_DllAPI RTPSReliableReaderQos& reliable_reader_qos()
    {
        return reliable_reader_qos_;
    }

    /**
     * Getter for RTPSReliableReaderQos
     * @return RTPSReliableReaderQos const reference
     */
    RTPS_DllAPI const RTPSReliableReaderQos& reliable_reader_qos() const
    {
        return reliable_reader_qos_;
    }

    /**
     * Setter for RTPSReliableReaderQos
     * @param new_value new value for the RTPSReliableReaderQos
     */
    RTPS_DllAPI void reliable_reader_qos(
            const RTPSReliableReaderQos& new_value)
    {
        reliable_reader_qos_ = new_value;
    }

    /**
     * Getter for TypeConsistencyQos
     * @return TypeConsistencyQos reference
     */
    RTPS_DllAPI TypeConsistencyQos& type_consistency()
    {
        return type_consistency_;
    }

    /**
     * Getter for TypeConsistencyQos
     * @return TypeConsistencyQos const reference
     */
    RTPS_DllAPI const TypeConsistencyQos& type_consistency() const
    {
        return type_consistency_;
    }

    /**
     * Setter for TypeConsistencyQos
     * @param new_value new value for the TypeConsistencyQos
     */
    RTPS_DllAPI void type_consistency(
            const TypeConsistencyQos& new_value)
    {
        type_consistency_ = new_value;
    }

    /**
     * Getter for expectsInlineQos_
     * @return expectsInlineQos_
     */
    RTPS_DllAPI bool expects_inline_qos() const
    {
        return expects_inline_qos_;
    }

    /**
     * Setter for expectsInlineQos_
     * @param new_value new value for the expectsInlineQos_
     */
    RTPS_DllAPI void expects_inline_qos(
            bool new_value)
    {
        expects_inline_qos_ = new_value;
    }

    /**
     * Getter for PropertyPolicyQos
     * @return PropertyPolicyQos reference
     */
    RTPS_DllAPI PropertyPolicyQos& properties()
    {
        return properties_;
    }

    /**
     * Getter for PropertyPolicyQos
     * @return PropertyPolicyQos const reference
     */
    RTPS_DllAPI const PropertyPolicyQos& properties() const
    {
        return properties_;
    }

    /**
     * Setter for PropertyPolicyQos
     * @param new_value new value for the PropertyPolicyQos
     */
    RTPS_DllAPI void properties(
            const PropertyPolicyQos& new_value)
    {
        properties_ = new_value;
    }

    /**
     * Getter for RTPSEndpointQos
     * @return RTPSEndpointQos reference
     */
    RTPS_DllAPI RTPSEndpointQos& endpoint()
    {
        return endpoint_;
    }

    /**
     * Getter for RTPSEndpointQos
     * @return RTPSEndpointQos const reference
     */
    RTPS_DllAPI const RTPSEndpointQos& endpoint() const
    {
        return endpoint_;
    }

    /**
     * Setter for RTPSEndpointQos
     * @param new_value new value for the RTPSEndpointQos
     */
    RTPS_DllAPI void endpoint(
            const RTPSEndpointQos& new_value)
    {
        endpoint_ = new_value;
    }

    /**
     * Getter for ReaderResourceLimitsQos
     * @return ReaderResourceLimitsQos reference
     */
    RTPS_DllAPI ReaderResourceLimitsQos& reader_resource_limits()
    {
        return reader_resource_limits_;
    }

    /**
     * Getter for ReaderResourceLimitsQos
     * @return ReaderResourceLimitsQos const reference
     */
    RTPS_DllAPI const ReaderResourceLimitsQos& reader_resource_limits() const
    {
        return reader_resource_limits_;
    }

    /**
     * Setter for ReaderResourceLimitsQos
     * @param new_value new value for the ReaderResourceLimitsQos
     */
    RTPS_DllAPI void reader_resource_limits(
            const ReaderResourceLimitsQos& new_value)
    {
        reader_resource_limits_ = new_value;
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

    //! Tipe consistency (Extension)
    TypeConsistencyQos type_consistency_;

    //!Expects Inline QOS (Extension).
    bool expects_inline_qos_;

    //!Properties (Extension).
    PropertyPolicyQos properties_;

    //!Endpoint configuration (Extension)
    RTPSEndpointQos endpoint_;

    //!ReaderResourceLimitsQos
    ReaderResourceLimitsQos reader_resource_limits_;
};

RTPS_DllAPI extern const DataReaderQos DATAREADER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DATAREADERQOS_HPP
