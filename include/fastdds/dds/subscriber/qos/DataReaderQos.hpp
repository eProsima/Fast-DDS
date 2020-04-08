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

class RTPSReliableReaderQos : public QosPolicy
{
public:

    RTPS_DllAPI RTPSReliableReaderQos()
        : QosPolicy(false)
    {
    }

    virtual RTPS_DllAPI ~RTPSReliableReaderQos() = default;

    bool operator ==(
            const RTPSReliableReaderQos& b) const
    {
        return (this->reader_times == b.reader_times) &&
               (this->disablePositiveACKs == b.disablePositiveACKs) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        RTPSReliableReaderQos reset = RTPSReliableReaderQos();
        std::swap(*this, reset);
    }

    /*!
     * @brief Times associated with the Reliable Readers events.
     */
    fastrtps::rtps::ReaderTimes reader_times;

    /*!
     * @brief Control the sending of positive ACKs
     */
    DisablePositiveACKsQosPolicy disablePositiveACKs;
};

class RTPSEndpointQos : public QosPolicy
{
public:

    RTPS_DllAPI RTPSEndpointQos()
        : QosPolicy(false)
        , historyMemoryPolicy(fastrtps::rtps::PREALLOCATED_MEMORY_MODE)
        , m_userDefinedID(-1)
        , m_entityID(-1)
    {
    }

    virtual RTPS_DllAPI ~RTPSEndpointQos() = default;

    bool operator ==(
            const RTPSEndpointQos& b) const
    {
        return (this->unicastLocatorList == b.unicastLocatorList) &&
                (this->multicastLocatorList == b.multicastLocatorList) &&
                (this->remoteLocatorList == b.remoteLocatorList) &&
                (this->historyMemoryPolicy == b.historyMemoryPolicy) &&
                (this->m_userDefinedID == b.m_userDefinedID) &&
                (this->m_entityID == b.m_entityID) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        RTPSEndpointQos reset = RTPSEndpointQos();
        std::swap(*this, reset);
    }

    //!Unicast locator list.
    fastrtps::rtps::LocatorList_t unicastLocatorList;

    //!Multicast locator list.
    fastrtps::rtps::LocatorList_t multicastLocatorList;

    //!Remote locator list.
    fastrtps::rtps::LocatorList_t remoteLocatorList;

    //!Underlying History memory policy (Extension).
    fastrtps::rtps::MemoryManagementPolicy_t historyMemoryPolicy;

    //!User Defined ID, used for StaticEndpointDiscovery (Extension).
    int16_t m_userDefinedID;

    //!Entity ID, to specify the EntityID of the enpoint (Extension).
    int16_t m_entityID;
};

class ReaderResourceLimitsQos : public QosPolicy
{
public:

    RTPS_DllAPI ReaderResourceLimitsQos()
        : QosPolicy(false)
    {
    }

    virtual RTPS_DllAPI ~ReaderResourceLimitsQos() = default;

    bool operator ==(
            const ReaderResourceLimitsQos& b) const
    {
        return (this->matched_publisher_allocation == b.matched_publisher_allocation) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        ReaderResourceLimitsQos reset = ReaderResourceLimitsQos();
        std::swap(*this, reset);
    }

    //!Matched publishers allocation limits.
    fastrtps::ResourceLimitedContainerConfig matched_publisher_allocation;
};

class TypeConsistencyQos : public QosPolicy
{
public:

    RTPS_DllAPI TypeConsistencyQos()
        : QosPolicy(false)
    {
    }

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

    DataReaderQos()
        : expectsInlineQos_(false)
    {
    }

    bool operator ==(
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
               (topicData_ == b.topicData()) &&
               (durabilityService_ == b.durabilityService()) &&
               (reliable_reader_qos_ == b.reliable_reader_qos()) &&
               (type_consistency_ == b.type_consistency()) &&
               (expectsInlineQos_ == b.expectsInlineQos()) &&
               (properties_ == b.properties()) &&
               (enpoint_ == b.enpoint()) &&
               (reader_resource_limits_ == b.reader_resource_limits());
    }

    RTPS_DllAPI ReaderQos get_readerqos(
            const SubscriberQos& pqos) const;

    /**
     * Getter for DurabilityQosPolicy
     * @return DurabilityQosPolicy reference
     */
    DurabilityQosPolicy& durability()
    {
        return durability_;
    }

    /**
     * Getter for DurabilityQosPolicy
     * @return DurabilityQosPolicy const reference
     */
    const DurabilityQosPolicy& durability() const
    {
        return durability_;
    }

    /**
     * Setter for DurabilityQosPolicy
     * @param new_value new value for the DurabilityQosPolicy
     */
    void durability(
            const DurabilityQosPolicy& new_value)
    {
        durability_ = new_value;
    }

    /**
     * Getter for DeadlineQosPolicy
     * @return DeadlineQosPolicy reference
     */
    DeadlineQosPolicy& deadline()
    {
        return deadline_;
    }

    /**
     * Getter for DeadlineQosPolicy
     * @return DeadlineQosPolicy const reference
     */
    const DeadlineQosPolicy& deadline() const
    {
        return deadline_;
    }

    /**
     * Setter for DeadlineQosPolicy
     * @param new_value new value for the DeadlineQosPolicy
     */
    void deadline(
            const DeadlineQosPolicy& new_value)
    {
        deadline_ = new_value;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     * @return LatencyBudgetQosPolicy reference
     */
    LatencyBudgetQosPolicy& latency_budget()
    {
        return latency_budget_;
    }

    /**
     * Getter for LatencyBudgetQosPolicy
     * @return LatencyBudgetQosPolicy const reference
     */
    const LatencyBudgetQosPolicy& latency_budget() const
    {
        return latency_budget_;
    }

    /**
     * Setter for LatencyBudgetQosPolicy
     * @param new_value new value for the LatencyBudgetQosPolicy
     */
    void latency_budget(
            const LatencyBudgetQosPolicy& new_value)
    {
        latency_budget_ = new_value;
    }

    /**
     * Getter for LivelinessQosPolicy
     * @return LivelinessQosPolicy reference
     */
    LivelinessQosPolicy& liveliness()
    {
        return liveliness_;
    }

    /**
     * Getter for LivelinessQosPolicy
     * @return LivelinessQosPolicy const reference
     */
    const LivelinessQosPolicy& liveliness() const
    {
        return liveliness_;
    }

    /**
     * Setter for LivelinessQosPolicy
     * @param new_value new value for the LivelinessQosPolicy
     */
    void liveliness(
            const LivelinessQosPolicy& new_value)
    {
        liveliness_ = new_value;
    }

    /**
     * Getter for ReliabilityQosPolicy
     * @return ReliabilityQosPolicy reference
     */
    ReliabilityQosPolicy& reliability()
    {
        return reliability_;
    }

    /**
     * Getter for ReliabilityQosPolicy
     * @return ReliabilityQosPolicy const reference
     */
    const ReliabilityQosPolicy& reliability() const
    {
        return reliability_;
    }

    /**
     * Setter for ReliabilityQosPolicy
     * @param new_value new value for the ReliabilityQosPolicy
     */
    void reliability(
            const ReliabilityQosPolicy& new_value)
    {
        reliability_ = new_value;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     * @return DestinationOrderQosPolicy reference
     */
    DestinationOrderQosPolicy& destination_order()
    {
        return destination_order_;
    }

    /**
     * Getter for DestinationOrderQosPolicy
     * @return DestinationOrderQosPolicy const reference
     */
    const DestinationOrderQosPolicy& destination_order() const
    {
        return destination_order_;
    }

    /**
     * Setter for DestinationOrderQosPolicy
     * @param new_value new value for the DestinationOrderQosPolicy
     */
    void destination_order(
            const DestinationOrderQosPolicy& new_value)
    {
        destination_order_ = new_value;
    }

    /**
     * Getter for HistoryQosPolicy
     * @return HistoryQosPolicy reference
     */
    HistoryQosPolicy& history()
    {
        return history_;
    }

    /**
     * Getter for HistoryQosPolicy
     * @return HistoryQosPolicy const reference
     */
    const HistoryQosPolicy& history() const
    {
        return history_;
    }

    /**
     * Setter for HistoryQosPolicy
     * @param new_value new value for the HistoryQosPolicy
     */
    void history(
            const HistoryQosPolicy& new_value)
    {
        history_ = new_value;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     * @return ResourceLimitsQosPolicy reference
     */
    ResourceLimitsQosPolicy& resource_limits()
    {
        return resource_limits_;
    }

    /**
     * Getter for ResourceLimitsQosPolicy
     * @return ResourceLimitsQosPolicy const reference
     */
    const ResourceLimitsQosPolicy& resource_limits() const
    {
        return resource_limits_;
    }

    /**
     * Setter for ResourceLimitsQosPolicy
     * @param new_value new value for the ResourceLimitsQosPolicy
     */
    void resource_limits(
            const ResourceLimitsQosPolicy& new_value)
    {
        resource_limits_ = new_value;
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
     * Getter for UserDataQosPolicy
     * @return UserDataQosPolicy const reference
     */
    const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    /**
     * Setter for UserDataQosPolicy
     * @param new_value new value for the UserDataQosPolicy
     */
    void user_data(
            const UserDataQosPolicy& new_value)
    {
        user_data_ = new_value;
    }
    /**
     * Getter for OwnershipQosPolicy
     * @return OwnershipQosPolicy reference
     */
    OwnershipQosPolicy& ownership()
    {
        return ownership_;
    }

    /**
     * Getter for OwnershipQosPolicy
     * @return OwnershipQosPolicy const reference
     */
    const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    /**
     * Setter for OwnershipQosPolicy
     * @param new_value new value for the OwnershipQosPolicy
     */
    void ownership(
            const OwnershipQosPolicy& new_value)
    {
        ownership_ = new_value;
    }

    /**
     * Getter for TimeBasedFilterQosPolicy
     * @return TimeBasedFilterQosPolicy reference
     */
    TimeBasedFilterQosPolicy& time_based_filter()
    {
        return time_based_filter_;
    }

    /**
     * Getter for TimeBasedFilterQosPolicy
     * @return TimeBasedFilterQosPolicy const reference
     */
    const TimeBasedFilterQosPolicy& time_based_filter() const
    {
        return time_based_filter_;
    }

    /**
     * Setter for TimeBasedFilterQosPolicy
     * @param new_value new value for the TimeBasedFilterQosPolicy
     */
    void time_based_filter(
            const TimeBasedFilterQosPolicy& new_value)
    {
        time_based_filter_ = new_value;
    }

    /**
     * Getter for ReaderDataLifecycleQosPolicy
     * @return ReaderDataLifecycleQosPolicy reference
     */
    ReaderDataLifecycleQosPolicy& reader_data_lifecycle()
    {
        return reader_data_lifecycle_;
    }

    /**
     * Getter for ReaderDataLifecycleQosPolicy
     * @return ReaderDataLifecycleQosPolicy const reference
     */
    const ReaderDataLifecycleQosPolicy& reader_data_lifecycle() const
    {
        return reader_data_lifecycle_;
    }

    /**
     * Setter for ReaderDataLifecycleQosPolicy
     * @param new_value new value for the ReaderDataLifecycleQosPolicy
     */
    void reader_data_lifecycle(
            const ReaderDataLifecycleQosPolicy& new_value)
    {
        reader_data_lifecycle_ = new_value;
    }

    /**
     * Getter for LifespanQosPolicy
     * @return LifespanQosPolicy reference
     */
    LifespanQosPolicy& lifespan()
    {
        return lifespan_;
    }

    /**
     * Getter for LifespanQosPolicy
     * @return LifespanQosPolicy const reference
     */
    const LifespanQosPolicy& lifespan() const
    {
        return lifespan_;
    }

    /**
     * Setter for LifespanQosPolicy
     * @param new_value new value for the LifespanQosPolicy
     */
    void lifespan(
            const LifespanQosPolicy& new_value)
    {
        lifespan_ = new_value;
    }

    /**
     * Getter for TopicDataQosPolicy
     * @return TopicDataQosPolicy reference
     */
    TopicDataQosPolicy& topicData()
    {
        return topicData_;
    }

    /**
     * Getter for TopicDataQosPolicy
     * @return TopicDataQosPolicy const reference
     */
    const TopicDataQosPolicy& topicData() const
    {
        return topicData_;
    }

    /**
     * Setter for TopicDataQosPolicy
     * @param new_value new value for the TopicDataQosPolicy
     */
    void topicData(
            const TopicDataQosPolicy& new_value)
    {
        topicData_ = new_value;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     * @return DurabilityServiceQosPolicy reference
     */
    DurabilityServiceQosPolicy& durabilityService()
    {
        return durabilityService_;
    }

    /**
     * Getter for DurabilityServiceQosPolicy
     * @return DurabilityServiceQosPolicy const reference
     */
    const DurabilityServiceQosPolicy& durabilityService() const
    {
        return durabilityService_;
    }

    /**
     * Setter for DurabilityServiceQosPolicy
     * @param new_value new value for the DurabilityServiceQosPolicy
     */
    void durabilityService(
            const DurabilityServiceQosPolicy& new_value)
    {
        durabilityService_ = new_value;
    }

    /**
     * Getter for RTPSReliableReaderQos
     * @return RTPSReliableReaderQos reference
     */
    RTPSReliableReaderQos& reliable_reader_qos()
    {
        return reliable_reader_qos_;
    }

    /**
     * Getter for RTPSReliableReaderQos
     * @return RTPSReliableReaderQos const reference
     */
    const RTPSReliableReaderQos& reliable_reader_qos() const
    {
        return reliable_reader_qos_;
    }

    /**
     * Setter for RTPSReliableReaderQos
     * @param new_value new value for the RTPSReliableReaderQos
     */
    void reliable_reader_qos(
            const RTPSReliableReaderQos& new_value)
    {
        reliable_reader_qos_ = new_value;
    }

    /**
     * Getter for TypeConsistencyQos
     * @return TypeConsistencyQos reference
     */
    TypeConsistencyQos& type_consistency()
    {
        return type_consistency_;
    }

    /**
     * Getter for TypeConsistencyQos
     * @return TypeConsistencyQos const reference
     */
    const TypeConsistencyQos& type_consistency() const
    {
        return type_consistency_;
    }

    /**
     * Setter for TypeConsistencyQos
     * @param new_value new value for the TypeConsistencyQos
     */
    void type_consistency(
            const TypeConsistencyQos& new_value)
    {
        type_consistency_ = new_value;
    }

    /**
     * Getter for expectsInlineQos_
     * @return expectsInlineQos_
     */
    bool expectsInlineQos() const
    {
        return expectsInlineQos_;
    }

    /**
     * Setter for expectsInlineQos_
     * @param new_value new value for the expectsInlineQos_
     */
    void expectsInlineQos(
            bool new_value)
    {
        expectsInlineQos_ = new_value;
    }

    /**
     * Getter for PropertyPolicyQos
     * @return PropertyPolicyQos reference
     */
    PropertyPolicyQos& properties()
    {
        return properties_;
    }

    /**
     * Getter for PropertyPolicyQos
     * @return PropertyPolicyQos const reference
     */
    const PropertyPolicyQos& properties() const
    {
        return properties_;
    }

    /**
     * Setter for PropertyPolicyQos
     * @param new_value new value for the PropertyPolicyQos
     */
    void properties(
            const PropertyPolicyQos& new_value)
    {
        properties_ = new_value;
    }

    /**
     * Getter for RTPSEndpointQos
     * @return RTPSEndpointQos reference
     */
    RTPSEndpointQos& enpoint()
    {
        return enpoint_;
    }

    /**
     * Getter for RTPSEndpointQos
     * @return RTPSEndpointQos const reference
     */
    const RTPSEndpointQos& enpoint() const
    {
        return enpoint_;
    }

    /**
     * Setter for RTPSEndpointQos
     * @param new_value new value for the RTPSEndpointQos
     */
    void enpoint(
            const RTPSEndpointQos& new_value)
    {
        enpoint_ = new_value;
    }

    /**
     * Getter for ReaderResourceLimitsQos
     * @return ReaderResourceLimitsQos reference
     */
    ReaderResourceLimitsQos& reader_resource_limits()
    {
        return reader_resource_limits_;
    }

    /**
     * Getter for ReaderResourceLimitsQos
     * @return ReaderResourceLimitsQos const reference
     */
    const ReaderResourceLimitsQos& reader_resource_limits() const
    {
        return reader_resource_limits_;
    }

    /**
     * Setter for ReaderResourceLimitsQos
     * @param new_value new value for the ReaderResourceLimitsQos
     */
    void reader_resource_limits(
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

    //!User Data Qos, NOT implemented in the library.
    UserDataQosPolicy user_data_;

    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy ownership_;

    //!Time Based Filter Qos, NOT implemented in the library.
    TimeBasedFilterQosPolicy time_based_filter_;

    //!Reader Data Lifecycle Qos, NOT implemented in the library.
    ReaderDataLifecycleQosPolicy reader_data_lifecycle_;


    //!Lifespan Qos (Extension).
    LifespanQosPolicy lifespan_;

    //!Topic Data Qos (Extension).
    TopicDataQosPolicy topicData_;

    //!Durability Service Qos (Extension).
    DurabilityServiceQosPolicy durabilityService_;

    //!Reliable reader configuration (Extension)
    RTPSReliableReaderQos reliable_reader_qos_;

    //! Tipe consistency (Extension)
    TypeConsistencyQos type_consistency_;

    //from SubscriberAttributes

    //!Topic Attributes (Extension).
    //TopicAttributesQos topic;

    //!Expects Inline QOS (Extension).
    bool expectsInlineQos_;

    //!Properties (Extension).
    PropertyPolicyQos properties_;

    //!Endpoint configuration (Extension)
    RTPSEndpointQos enpoint_;

    //!ReaderResourceLimitsQos
    ReaderResourceLimitsQos reader_resource_limits_;
};

extern const DataReaderQos DATAREADER_QOS_DEFAULT;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DATAREADERQOS_HPP
