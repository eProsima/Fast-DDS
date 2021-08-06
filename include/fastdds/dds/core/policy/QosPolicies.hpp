// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file QosPolicies.hpp
 *
 */

#ifndef _FASTDDS_DDS_QOS_QOSPOLICIES_HPP_
#define _FASTDDS_DDS_QOS_QOSPOLICIES_HPP_

#include <vector>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastrtps/types/TypeObject.h>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>


namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * The identifier for each QosPolicy.
 *
 * Each QosPolicy class has a different ID that is then used to refer
 * to the incompatible policies on OfferedIncompatibleQosStatus
 * and RequestedIncompatibleQosStatus.
 */
enum QosPolicyId_t : uint32_t
{
    INVALID_QOS_POLICY_ID                   = 0,    //< Does not refer to any valid QosPolicy

    // Standard QosPolicies
    USERDATA_QOS_POLICY_ID                  = 1,    //< UserDataQosPolicy
    DURABILITY_QOS_POLICY_ID                = 2,    //< DurabilityQosPolicy
    PRESENTATION_QOS_POLICY_ID              = 3,    //< PresentationQosPolicy
    DEADLINE_QOS_POLICY_ID                  = 4,    //< DeadlineQosPolicy
    LATENCYBUDGET_QOS_POLICY_ID             = 5,    //< LatencyBudgetQosPolicy
    OWNERSHIP_QOS_POLICY_ID                 = 6,    //< OwnershipQosPolicy
    OWNERSHIPSTRENGTH_QOS_POLICY_ID         = 7,    //< OwnershipStrengthQosPolicy
    LIVELINESS_QOS_POLICY_ID                = 8,    //< LivelinessQosPolicy
    TIMEBASEDFILTER_QOS_POLICY_ID           = 9,    //< TimeBasedFilterQosPolicy
    PARTITION_QOS_POLICY_ID                 = 10,   //< PartitionQosPolicy
    RELIABILITY_QOS_POLICY_ID               = 11,   //< ReliabilityQosPolicy
    DESTINATIONORDER_QOS_POLICY_ID          = 12,   //< DestinationOrderQosPolicy
    HISTORY_QOS_POLICY_ID                   = 13,   //< HistoryQosPolicy
    RESOURCELIMITS_QOS_POLICY_ID            = 14,   //< ResourceLimitsQosPolicy
    ENTITYFACTORY_QOS_POLICY_ID             = 15,   //< EntityFactoryQosPolicy
    WRITERDATALIFECYCLE_QOS_POLICY_ID       = 16,   //< WriterDataLifecycleQosPolicy
    READERDATALIFECYCLE_QOS_POLICY_ID       = 17,   //< ReaderDataLifecycleQosPolicy
    TOPICDATA_QOS_POLICY_ID                 = 18,   //< TopicDataQosPolicy
    GROUPDATA_QOS_POLICY_ID                 = 19,   //< GroupDataQosPolicy
    TRANSPORTPRIORITY_QOS_POLICY_ID         = 20,   //< TransportPriorityQosPolicy
    LIFESPAN_QOS_POLICY_ID                  = 21,   //< LifespanQosPolicy
    DURABILITYSERVICE_QOS_POLICY_ID         = 22,   //< DurabilityServiceQosPolicy

    //XTypes extensions
    DATAREPRESENTATION_QOS_POLICY_ID            = 23,   //< DataRepresentationQosPolicy
    TYPECONSISTENCYENFORCEMENT_QOS_POLICY_ID    = 24,   //< TypeConsistencyEnforcementQosPolicy

    //eProsima Extensions
    DISABLEPOSITIVEACKS_QOS_POLICY_ID       = 25,   //< DisablePositiveACKsQosPolicy
    PARTICIPANTRESOURCELIMITS_QOS_POLICY_ID = 26,   //< ParticipantResourceLimitsQos
    PROPERTYPOLICY_QOS_POLICY_ID            = 27,   //< PropertyPolicyQos
    PUBLISHMODE_QOS_POLICY_ID               = 28,   //< PublishModeQosPolicy
    READERRESOURCELIMITS_QOS_POLICY_ID      = 29,   //< Reader ResourceLimitsQos
    RTPSENDPOINT_QOS_POLICY_ID              = 30,   //< RTPSEndpointQos
    RTPSRELIABLEREADER_QOS_POLICY_ID        = 31,   //< RTPSReliableReaderQos
    RTPSRELIABLEWRITER_QOS_POLICY_ID        = 32,   //< RTPSReliableWriterQos
    TRANSPORTCONFIG_QOS_POLICY_ID           = 33,   //< TransportConfigQos
    TYPECONSISTENCY_QOS_POLICY_ID           = 34,   //< TipeConsistencyQos
    WIREPROTOCOLCONFIG_QOS_POLICY_ID        = 35,   //< WireProtocolConfigQos
    WRITERRESOURCELIMITS_QOS_POLICY_ID      = 36,   //< WriterResourceLimitsQos

    NEXT_QOS_POLICY_ID                              //< Keep always the last element. For internal use only
};

using PolicyMask = std::bitset<NEXT_QOS_POLICY_ID>;

/**
 * Class QosPolicy, base for all QoS policies defined for Writers and Readers.
 */
class QosPolicy
{
public:

    //! Boolean that indicates if the Qos has been changed
    bool hasChanged;

    /**
     * @brief Constructor without parameters
     */
    QosPolicy()
        : hasChanged(false)
        , send_always_(false)
    {
    }

    /**
     * @brief Constructor
     * @param send_always Boolean that set if the Qos need to be sent even if it is not changed
     */
    explicit QosPolicy(
            bool send_always)
        : hasChanged(false)
        , send_always_(send_always)
    {
    }

    /**
     * @brief Copy Constructor
     * @param b Another instance of QosPolicy
     */
    QosPolicy(
            const QosPolicy& b) = default;

    /**
     * @brief Destructor
     */
    virtual ~QosPolicy() = default;

    bool operator ==(
            const QosPolicy& b) const
    {
        // hasChanged field isn't needed to be compared to being equal two QosPolicy objects.
        return (this->send_always_ == b.send_always_);
    }

    QosPolicy& operator =(
            const QosPolicy& b) = default;

    /**
     * Whether it should always be sent.
     * @return True if it should always be sent.
     */
    virtual bool send_always() const
    {
        return send_always_;
    }

    /**
     * @brief Clears the QosPolicy object
     */
    virtual inline void clear() = 0;

protected:

    //! Boolean that indicates if the Qos has to be sent even if it is not changed
    bool send_always_;
};

/**
 * @brief Controls the behavior of the entity when acting as a factory for other entities. In other words,
 * configures the side-effects of the create_* and delete_* operations.
 * @note Mutable Qos Policy
 */
class EntityFactoryQosPolicy
{
public:

    /**
     * Specifies whether the entity acting as a factory automatically enables the instances it creates.
     * If True the factory will automatically enable each created Entity otherwise it will not. <br>
     * By default, True.
     */
    bool autoenable_created_entities;

    /**
     * @brief Constructor without parameters
     */
    RTPS_DllAPI EntityFactoryQosPolicy()
        : autoenable_created_entities(true)
    {
    }

    /**
     * @brief Constructor
     * @param autoenable Value for the autoenable_created_entities boolean
     */
    RTPS_DllAPI EntityFactoryQosPolicy(
            bool autoenable)
        : autoenable_created_entities(autoenable)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~EntityFactoryQosPolicy()
    {
    }

    bool operator ==(
            const EntityFactoryQosPolicy& b) const
    {
        return
            (this->autoenable_created_entities == b.autoenable_created_entities);
    }

    inline void clear()
    {
        EntityFactoryQosPolicy reset = EntityFactoryQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Enum DurabilityQosPolicyKind_t, different kinds of durability for DurabilityQosPolicy.
 */
typedef enum DurabilityQosPolicyKind : fastrtps::rtps::octet
{
    /**
     * The Service does not need to keep any samples of data-instances on behalf of any DataReader that is not
     * known by the DataWriter at the time the instance is written. In other words the Service will only attempt
     * to provide the data to existing subscribers
     */
    VOLATILE_DURABILITY_QOS,
    /**
     * For TRANSIENT_LOCAL, the service is only required to keep the data in the memory of the DataWriter that
     * wrote the data and the data is not required to survive the DataWriter.
     */
    TRANSIENT_LOCAL_DURABILITY_QOS,
    /**
     * For TRANSIENT, the service is only required to keep the data in memory and not in permanent storage; but
     * the data is not tied to the lifecycle of the DataWriter and will, in general, survive it.
     */
    TRANSIENT_DURABILITY_QOS,
    /**
     * Data is kept on permanent storage, so that they can outlive a system session.
     * @warning Not Supported
     */
    PERSISTENT_DURABILITY_QOS
} DurabilityQosPolicyKind_t;

#define PARAMETER_KIND_LENGTH 4
#define PARAMETER_BOOL_LENGTH 4

/**
 * This policy expresses if the data should ‘outlive’ their writing time.
 * @note Immutable Qos Policy
 */
class DurabilityQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DurabilityQosPolicy()
        : Parameter_t(PID_DURABILITY, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(VOLATILE_DURABILITY_QOS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~DurabilityQosPolicy() = default;

    /**
     * Translates kind to rtps layer equivalent
     * @return fastrtps::rtps::DurabilityKind_t
     */
    inline fastrtps::rtps::DurabilityKind_t durabilityKind() const
    {
        switch (kind)
        {
            default:
            case VOLATILE_DURABILITY_QOS: return fastrtps::rtps::VOLATILE;
            case TRANSIENT_LOCAL_DURABILITY_QOS: return fastrtps::rtps::TRANSIENT_LOCAL;
            case TRANSIENT_DURABILITY_QOS: return fastrtps::rtps::TRANSIENT;
            case PERSISTENT_DURABILITY_QOS: return fastrtps::rtps::PERSISTENT;
        }
    }

    bool operator ==(
            const DurabilityQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Set kind passing the rtps layer equivalent kind
     * @param new_kind fastrtps::rtps::DurabilityKind_t
     */
    inline void durabilityKind(
            const fastrtps::rtps::DurabilityKind_t new_kind)
    {
        switch (new_kind)
        {
            default:
            case fastrtps::rtps::VOLATILE: kind = VOLATILE_DURABILITY_QOS; break;
            case fastrtps::rtps::TRANSIENT_LOCAL: kind = TRANSIENT_LOCAL_DURABILITY_QOS; break;
            case fastrtps::rtps::TRANSIENT: kind = TRANSIENT_DURABILITY_QOS; break;
            case fastrtps::rtps::PERSISTENT: kind = PERSISTENT_DURABILITY_QOS; break;
        }

    }

    inline void clear() override
    {
        DurabilityQosPolicy reset = DurabilityQosPolicy();
        std::swap(*this, reset);
    }

public:

    /**
     * @brief DurabilityQosPolicyKind. <br>
     * By default the value for DataReaders: VOLATILE_DURABILITY_QOS, for DataWriters TRANSIENT_LOCAL_DURABILITY_QOS
     */
    DurabilityQosPolicyKind_t kind;
};

/**
 * @brief DataReader expects a new sample updating the value of each instance at least once every deadline period.
 * DataWriter indicates that the application commits to write a new value (using the DataWriter) for each instance managed
 * by the DataWriter at least once every deadline period.
 * @note Mutable Qos Policy
 */
class DeadlineQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DeadlineQosPolicy()
        : Parameter_t(PID_DEADLINE, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , period(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~DeadlineQosPolicy() = default;

    bool operator ==(
            const DeadlineQosPolicy& b) const
    {
        return (this->period == b.period) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        DeadlineQosPolicy reset = DeadlineQosPolicy();
        std::swap(*this, reset);
    }

public:

    /**
     * @brief Maximum time expected between samples.
     * It is inconsistent for a DataReader to have a DEADLINE period less than its TimeBasedFilterQosPolicy
     * minimum_separation. <br>
     * By default, c_TimeInifinite.
     */
    fastrtps::Duration_t period;
};

/**
 * Specifies the maximum acceptable delay from the time the data is written until the data is inserted in the receiver's
 * application-cache and the receiving application is notified of the fact.This policy is a hint to the Service, not something
 * that must be monitored or enforced. The Service is not required to track or alert the user of any violation.
 * @warning This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * @note Mutable Qos Policy
 */
class LatencyBudgetQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI LatencyBudgetQosPolicy()
        : Parameter_t(PID_LATENCY_BUDGET, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , duration(0, 0)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~LatencyBudgetQosPolicy() = default;

    bool operator ==(
            const LatencyBudgetQosPolicy& b) const
    {
        return (this->duration == b.duration) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        LatencyBudgetQosPolicy reset = LatencyBudgetQosPolicy();
        std::swap(*this, reset);
    }

public:

    //!Maximum acceptable delay from the time data is written until it is received. <br> By default, c_TimeZero.
    fastrtps::Duration_t duration;
};

/**
 * Enum LivelinessQosPolicyKind, different kinds of liveliness for LivelinessQosPolicy
 */
typedef enum LivelinessQosPolicyKind : fastrtps::rtps::octet
{
    /**
     * The infrastructure will automatically signal liveliness for the DataWriters at least as often as required by the lease_duration.
     */
    AUTOMATIC_LIVELINESS_QOS,
    /**
     * The Service will assume that as long as at least one Entity within the DomainParticipant has asserted its liveliness the other
     * Entities in that same DomainParticipant are also alive.
     */
    MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
    /**
     * The Service will only assume liveliness of the DataWriter if the application has asserted liveliness of that DataWriter itself.
     */
    MANUAL_BY_TOPIC_LIVELINESS_QOS

} LivelinessQosPolicyKind;

/**
 * Determines the mechanism and parameters used by the application to determine whether an Entity is “active” (alive).
 * The “liveliness” status of an Entity is used to maintain instance ownership in combination with the setting of the
 * OwnershipQosPolicy.
 * The application is also informed via listener when an Entity is no longer alive.
 *
 * The DataReader requests that liveliness of the writers is maintained by the requested means and loss of liveliness is
 * detected with delay not to exceed the lease_duration.
 *
 * The DataWriter commits to signaling its liveliness using the stated means at intervals not to exceed the lease_duration.
 * Listeners are used to notify the DataReaderof loss of liveliness and DataWriter of violations to the liveliness contract.
 */
class LivelinessQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI LivelinessQosPolicy()
        : Parameter_t(PID_LIVELINESS, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , kind(AUTOMATIC_LIVELINESS_QOS)
        , lease_duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
        , announcement_period(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~LivelinessQosPolicy() = default;

    bool operator ==(
            const LivelinessQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               (this->lease_duration == b.lease_duration) &&
               (this->announcement_period == b.announcement_period) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        LivelinessQosPolicy reset = LivelinessQosPolicy();
        std::swap(*this, reset);
    }

public:

    //! Liveliness kind <br> By default, AUTOMATIC_LIVELINESS.
    LivelinessQosPolicyKind kind;
    //! Period within which liveliness should be asserted.
    //! On a DataWriter it represents the period it commits to signal its liveliness.
    //! On a DataReader it represents the period without assertion after which a DataWriter is considered
    //! inactive.
    //! By default, c_TimeInfinite.
    fastrtps::Duration_t lease_duration;
    //! The period for automatic assertion of liveliness.
    //! Only used for DataWriters with AUTOMATIC liveliness.
    //! By default, c_TimeInfinite.
    //! @warning When not infinite, must be < lease_duration, and it is advisable to be less than 0.7*lease_duration.
    fastrtps::Duration_t announcement_period;
};

/**
 * Enum ReliabilityQosPolicyKind, different kinds of reliability for ReliabilityQosPolicy.
 */
typedef enum ReliabilityQosPolicyKind : fastrtps::rtps::octet
{
    /**
     * Indicates that it is acceptable to not retry propagation of any samples. Presumably new values for the samples
     * are generated often enough that it is not necessary to re-send or acknowledge any samples
     */
    BEST_EFFORT_RELIABILITY_QOS = 0x01,
    /**
     * Specifies the Service will attempt to deliver all samples in its history. Missed samples may be retried.
     * In steady-state (no modifications communicated via the DataWriter) the middleware guarantees that all samples
     * in the DataWriter history will eventually be delivered to all the DataReader objects. Outside steady state the
     * HistoryQosPolicy and ResourceLimitsQosPolicy will determine how samples become part of the history and whether
     * samples can be discarded from it.
     */
    RELIABLE_RELIABILITY_QOS = 0x02
} ReliabilityQosPolicyKind;

/**
 * Indicates the reliability of the endpoint.
 * @note Immutable Qos Policy
 */
class ReliabilityQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI ReliabilityQosPolicy()
        : Parameter_t(PID_RELIABILITY, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true) //indicate send always
        , kind(BEST_EFFORT_RELIABILITY_QOS)
        , max_blocking_time{0, 100000000} // max_blocking_time = 100ms
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~ReliabilityQosPolicy() = default;

    bool operator ==(
            const ReliabilityQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               (this->max_blocking_time == b.max_blocking_time) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        ReliabilityQosPolicy reset = ReliabilityQosPolicy();
        std::swap(*this, reset);
    }

public:

    /*!
     * @brief Defines the reliability kind of the endpoint. <br>
     * By default, BEST_EFFORT_RELIABILITY_QOS for DataReaders and RELIABLE_RELIABILITY_QOS for DataWriters.
     */
    ReliabilityQosPolicyKind kind;

    /*!
     * @brief Defines the maximum period of time certain methods will be blocked.
     *
     * Methods affected by this property are:
     * - DataWriter::write
     * - DataReader::takeNextData
     * - DataReader::readNextData
     * <br>
     * By default, 100 ms.
     */
    fastrtps::Duration_t max_blocking_time;
};



/**
 * Enum OwnershipQosPolicyKind, different kinds of ownership for OwnershipQosPolicy.
 */
enum OwnershipQosPolicyKind : fastrtps::rtps::octet
{
    /**
     * Indicates shared ownership for each instance. Multiple writers are allowed to update the same instance and all the
     * updates are made available to the readers. In other words there is no concept of an “owner” for the instances.
     */
    SHARED_OWNERSHIP_QOS,
    /**
     * Indicates each instance can only be owned by one DataWriter, but the owner of an instance can change dynamically.
     * The selection of the owner is controlled by the setting of the OwnershipStrengthQosPolicy. The owner is always set
     * to be the highest-strength DataWriter object among the ones currently “active” (as determined by the LivelinessQosPolicy).
     */
    EXCLUSIVE_OWNERSHIP_QOS
};

/**
 * Specifies whether it is allowed for multiple DataWriters to write the same instance of the data and if so, how these
 * modifications should be arbitrated
 * @note Immutable Qos Policy
 */
class OwnershipQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI OwnershipQosPolicy()
        : Parameter_t(PID_OWNERSHIP, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(SHARED_OWNERSHIP_QOS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~OwnershipQosPolicy() = default;

    bool operator ==(
            const OwnershipQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        OwnershipQosPolicy reset = OwnershipQosPolicy();
        std::swap(*this, reset);
    }

public:

    //!OwnershipQosPolicyKind
    OwnershipQosPolicyKind kind;
};

/**
 * Enum DestinationOrderQosPolicyKind, different kinds of destination order for DestinationOrderQosPolicy.
 */
enum DestinationOrderQosPolicyKind : fastrtps::rtps::octet
{
    /**
     * Indicates that data is ordered based on the reception time at each Subscriber. Since each subscriber may receive
     * the data at different times there is no guaranteed that the changes will be seen in the same order. Consequently,
     * it is possible for each subscriber to end up with a different final value for the data.
     */
    BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
    /**
     * Indicates that data is ordered based on a timestamp placed at the source (by the Service or by the application).
     * In any case this guarantees a consistent final value for the data in all subscribers.
     */
    BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};



/**
 * Controls the criteria used to determine the logical order among changes made by Publisher entities to the same instance of
 * data (i.e., matching Topic and key).
 * @warning This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * @note Immutable Qos Policy
 */
class DestinationOrderQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DestinationOrderQosPolicy()
        : Parameter_t(PID_DESTINATION_ORDER, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~DestinationOrderQosPolicy() = default;

    bool operator ==(
            const DestinationOrderQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        DestinationOrderQosPolicy reset = DestinationOrderQosPolicy();
        std::swap(*this, reset);
    }

public:

    //!DestinationOrderQosPolicyKind. <br> By default, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS.
    DestinationOrderQosPolicyKind kind;
};


/**
 * Class GenericDataQosPolicy, base class to transmit user data during the discovery phase.
 */
class GenericDataQosPolicy : public Parameter_t, public QosPolicy,
    public fastrtps::ResourceLimitedVector<fastrtps::rtps::octet>
{
    using ResourceLimitedOctetVector = fastrtps::ResourceLimitedVector<fastrtps::rtps::octet>;

public:

    RTPS_DllAPI GenericDataQosPolicy(
            ParameterId_t pid)
        : Parameter_t(pid, 0)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
    }

    RTPS_DllAPI GenericDataQosPolicy(
            ParameterId_t pid,
            uint16_t in_length)
        : Parameter_t(pid, in_length)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
    }

    /**
     * Construct from another GenericDataQosPolicy.
     *
     * The resulting GenericDataQosPolicy will have the same size limits
     * as the input attribute
     *
     * @param data data to copy in the newly created object
     */
    RTPS_DllAPI GenericDataQosPolicy(
            const GenericDataQosPolicy& data)
        : Parameter_t(data.Pid, data.length)
        , QosPolicy(false)
        , ResourceLimitedOctetVector(data)
    {
    }

    /**
     * Construct from underlying collection type.
     *
     * Useful to easy integration on old APIs where a traditional container was used.
     * The resulting GenericDataQosPolicy will always be unlimited in size
     *
     * @param pid Id of the parameter
     * @param data data to copy in the newly created object
     */
    RTPS_DllAPI GenericDataQosPolicy(
            ParameterId_t pid,
            const collection_type& data)
        : Parameter_t(pid, 0)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
        assign(data.begin(), data.end());
        length = static_cast<uint16_t>((size() + 7u) & ~3u);
    }

    virtual RTPS_DllAPI ~GenericDataQosPolicy() = default;

    /**
     * Copies data from underlying collection type.
     *
     * Useful to easy integration on old APIs where a traditional container was used.
     * The resulting GenericDataQosPolicy will keep the current size limit.
     * If the input data is larger than the current limit size, the elements exceeding
     * that maximum will be silently discarded.
     *
     * @param b object to be copied
     * @return reference to the current object.
     */
    GenericDataQosPolicy& operator =(
            const collection_type& b)
    {
        if (collection_ != b)
        {
            //If the object is size limited, already has max_size() allocated
            //assign() will always stop copying when reaching max_size()
            assign(b.begin(), b.end());
            length = static_cast<uint16_t>((size() + 7u) & ~3u);
            hasChanged = true;
        }
        return *this;
    }

    /**
     * Copies another GenericDataQosPolicy.
     *
     * The resulting GenericDataQosPolicy will have the same size limit
     * as the input parameter, so all data in the input will be copied.
     *
     * @param b object to be copied
     * @return reference to the current object.
     */
    GenericDataQosPolicy& operator =(
            const GenericDataQosPolicy& b)
    {
        QosPolicy::operator =(b);
        Parameter_t::operator =(b);
        configuration_ = b.configuration_;
        collection_.reserve(b.collection_.capacity());
        collection_.assign(b.collection_.begin(), b.collection_.end());
        return *this;
    }

    bool operator ==(
            const GenericDataQosPolicy& b) const
    {
        return collection_ == b.collection_ &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    bool operator ==(
            const collection_type& b) const
    {
        return collection_ == b;
    }

    /**
     * Set the maximum size of the user data and reserves memory for that much.
     * @param size new maximum size of the user data. Zero for unlimited size
     */
    void set_max_size (
            size_t size)
    {
        if (size > 0)
        {
            configuration_ = fastrtps::ResourceLimitedContainerConfig::fixed_size_configuration(size);
            collection_.reserve(configuration_.maximum);
        }
        else
        {
            configuration_ = fastrtps::ResourceLimitedContainerConfig::dynamic_allocation_configuration();
        }
    }

    void resize(
            size_t new_size)
    {
        collection_.resize(new_size);
    }

    /**
     * @return const reference to the internal raw data.
     */
    inline const collection_type& dataVec() const
    {
        return collection_;
    }

    inline void clear() override
    {
        ResourceLimitedOctetVector::clear();
        hasChanged = false;
    }

    /**
     * Returns raw data vector.
     * @return raw data as vector of octets.
     * */
    RTPS_DllAPI inline const collection_type& data_vec() const
    {
        return collection_;
    }

    /**
     * Returns raw data vector.
     * @return raw data as vector of octets.
     * */
    RTPS_DllAPI inline collection_type& data_vec()
    {
        return collection_;
    }

    /**
     * Sets raw data vector.
     * @param vec raw data to set.
     * */
    RTPS_DllAPI inline void data_vec(
            const collection_type& vec)
    {
        if (collection_ != vec)
        {
            assign(vec.begin(), vec.end());
            length = static_cast<uint16_t>((size() + 7u) & ~3u);
            hasChanged = true;
        }
    }

    /**
     * Returns raw data vector.
     * @return raw data as vector of octets.
     * */
    RTPS_DllAPI inline const collection_type& getValue() const
    {
        return collection_;
    }

    /**
     * Sets raw data vector.
     * @param vec raw data to set.
     * */
    RTPS_DllAPI inline void setValue(
            const collection_type& vec)
    {
        data_vec(vec);
    }

};

/**
 * Class TClassName, base template for data qos policies.
 * Data not known by the middleware, but distributed by means of built-in topics.
 * By default, zero-sized sequence.
 * @note Mutable Qos Policy
 */
// *INDENT-OFF*  (uncrustify seems to have problems with this macro)
#define TEMPLATE_DATA_QOS_POLICY(TClassName, TPid)                                         \
    class TClassName : public GenericDataQosPolicy                                         \
    {                                                                                      \
    public:                                                                                \
                                                                                           \
        RTPS_DllAPI TClassName()                                                           \
            : GenericDataQosPolicy(TPid)                                                   \
        {                                                                                  \
        }                                                                                  \
                                                                                           \
        RTPS_DllAPI TClassName(                                                            \
                uint16_t in_length)                                                        \
            : GenericDataQosPolicy(TPid, in_length)                                        \
        {                                                                                  \
        }                                                                                  \
                                                                                           \
        /**                                                                                \
         * Construct from another TClassName.                                              \
         *                                                                                 \
         * The resulting TClassName will have the same size limits                         \
         * as the input attribute                                                          \
         *                                                                                 \
         * @param data data to copy in the newly created object                            \
         */                                                                                \
        RTPS_DllAPI TClassName(                                                            \
                const TClassName &data) = default;                                         \
                                                                                           \
        /**                                                                                \
         * Construct from underlying collection type.                                      \
         *                                                                                 \
         * Useful to easy integration on old APIs where a traditional container was used.  \
         * The resulting TClassName will always be unlimited in size                       \
         *                                                                                 \
         * @param data data to copy in the newly created object                            \
         */                                                                                \
        RTPS_DllAPI TClassName(                                                            \
                const collection_type &data)                                               \
            : GenericDataQosPolicy(TPid, data)                                             \
        {                                                                                  \
        }                                                                                  \
                                                                                           \
        virtual RTPS_DllAPI ~TClassName() = default;                                       \
                                                                                           \
        /**                                                                                \
         * Copies another TClassName.                                                      \
         *                                                                                 \
         * The resulting TClassName will have the same size limit                          \
         * as the input parameter, so all data in the input will be copied.                \
         *                                                                                 \
         * @param b object to be copied                                                    \
         * @return reference to the current object.                                        \
         */                                                                                \
        TClassName& operator =(                                                            \
                const TClassName& b) = default;                                            \
    };
// *INDENT-ON*

//Variable used to generate the doxygen documentation for this QoS Policies
#ifdef DOXYGEN_DOCUMENTATION
/**
 * @brief Class derived from GenericDataQosPolicy
 *
 * The purpose of this QoS is to allow the application to attach additional information to the created
 * Entity objects such that when a remote application discovers their existence it can access that information and
 * use it for its own purposes.
 *
 * One possible use of this QoS is to attach security credentials or some other information that can be used by the
 * remote application to authenticate the source.
 */
class UserDataQosPolicy : public GenericDataQosPolicy
{
};
/**
 * @brief Class derived from GenericDataQosPolicy
 *
 * The purpose of this QoS is to allow the application to attach additional information to the created Topic
 * such that when a remote application discovers their existence it can examine the information and use it in an
 * application-defined way.
 *
 * In combination with the listeners on the DataReader and DataWriter as well as by means of operations such as
 * ignore_topic,these QoS can assist an application to extend the provided QoS.
 */
class TopicDataQosPolicy : public GenericDataQosPolicy
{
};
/**
 * @brief Class derived from GenericDataQosPolicy
 *
 * The purpose of this QoS is to allow the application to attach additional information to the created
 * Publisher or Subscriber. The value of the GROUP_DATA is available to the application on the DataReader and
 * DataWriter entities and is propagated by means of the built-in topics.
 *
 * This QoS can be used by an application combination with the DataReaderListener and DataWriterListener to
 * implement matching policies similar to those of the PARTITION QoS except the decision can be made based on an
 * application-defined policy.
 */
class GroupDataQosPolicy : public GenericDataQosPolicy
{
};
#endif  // DOXYGEN_DOCUMENTATION

TEMPLATE_DATA_QOS_POLICY(UserDataQosPolicy, PID_USER_DATA)
TEMPLATE_DATA_QOS_POLICY(TopicDataQosPolicy, PID_TOPIC_DATA)
TEMPLATE_DATA_QOS_POLICY(GroupDataQosPolicy, PID_GROUP_DATA)

/**
 * Filter that allows a DataReader to specify that it is interested only in (potentially) a subset of the values of the data.
 * The filter states that the DataReader does not want to receive more than one value each minimum_separation, regardless
 * of how fast the changes occur. It is inconsistent for a DataReader to have a minimum_separation longer than its
 * Deadline period.
 * @warning This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * @note Mutable Qos Policy
 */
class TimeBasedFilterQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TimeBasedFilterQosPolicy()
        : Parameter_t(PID_TIME_BASED_FILTER, PARAMETER_TIME_LENGTH)
        , QosPolicy(false)
        , minimum_separation(0, 0)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TimeBasedFilterQosPolicy() = default;

    bool operator ==(
            const TimeBasedFilterQosPolicy& b) const
    {
        return (this->minimum_separation == b.minimum_separation) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        TimeBasedFilterQosPolicy reset = TimeBasedFilterQosPolicy();
        std::swap(*this, reset);
    }

public:

    //! Minimum interval between samples. By default, c_TimeZero (the DataReader is interested in all values)
    fastrtps::Duration_t minimum_separation;
};

/**
 * Enum PresentationQosPolicyAccessScopeKind, different kinds of Presentation Policy order for PresentationQosPolicy.
 */
enum PresentationQosPolicyAccessScopeKind : fastrtps::rtps::octet
{
    /**
     * Scope spans only a single instance. Indicates that changes to one instance need not be coherent nor ordered with
     * respect to changes to any other instance. In other words, order and coherent changes apply to each instance
     * separately.
     */
    INSTANCE_PRESENTATION_QOS,
    /**
     * Scope spans to all instances within the same DataWriter (or DataReader), but not across instances in different
     * DataWriter (or DataReader).
     */
    TOPIC_PRESENTATION_QOS,
    /**
     * Scope spans to all instances belonging to DataWriter (or DataReader) entities within the same Publisher (or Subscriber).
     */
    GROUP_PRESENTATION_QOS
};

#define PARAMETER_PRESENTATION_LENGTH 8

/**
 * Specifies how the samples representing changes to data instances are presented to the subscribing application.
 * This policy affects the application’s ability to specify and receive coherent changes and to see the relative
 * order of changes.access_scope determines the largest scope spanning the entities for which the order and coherency
 * of changes can be preserved. The two booleans control whether coherent access and ordered access are supported within
 * the scope access_scope.
 * @warning This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * @note Immutable Qos Policy
 */
class PresentationQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor without parameters
     */
    RTPS_DllAPI PresentationQosPolicy()
        : Parameter_t(PID_PRESENTATION, PARAMETER_PRESENTATION_LENGTH)
        , QosPolicy(false)
        , access_scope(INSTANCE_PRESENTATION_QOS)
        , coherent_access(false)
        , ordered_access(false)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~PresentationQosPolicy() = default;

    bool operator ==(
            const PresentationQosPolicy& b) const
    {
        return (this->access_scope == b.access_scope) &&
               (this->coherent_access == b.coherent_access) &&
               (this->ordered_access == b.ordered_access) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        PresentationQosPolicy reset = PresentationQosPolicy();
        std::swap(*this, reset);
    }

public:

    //!Access Scope Kind <br> By default, INSTANCE_PRESENTATION_QOS.
    PresentationQosPolicyAccessScopeKind access_scope;
    /**
     * @brief Specifies support coherent access. That is, the ability to group a set of changes as a unit
     * on the publishing end such that they are received as a unit at the subscribing end.
     * by default, false.
     */
    bool coherent_access;
    /**
     * @brief Specifies support for ordered access to the samples received at the subscription end. That is,
     * the ability of the subscriber to see changes in the same order as they occurred on the publishing end.
     * By default, false.
     */
    bool ordered_access;
};


class Partition_t
{

    friend class PartitionQosPolicy;

private:

    const char* partition_;

private:

    Partition_t()
    {
        partition_ = nullptr;
    }

public:

    /**
     * @brief Constructor using a pointer
     * @param ptr Pointer to be set
     */
    explicit Partition_t(
            const void* ptr)
    {
        partition_ = (char*)ptr;
    }

    bool operator ==(
            const Partition_t& rhs) const
    {
        return (size() == rhs.size() &&
               (size() == 0 || strcmp(partition_ + 4, rhs.partition_ + 4)));
    }

    bool operator !=(
            const Partition_t& rhs) const
    {
        return !(*this == rhs);
    }

    /**
     * @brief Getter for the size
     * @return uint32_t with the size
     */
    uint32_t size() const
    {
        return *(uint32_t*)partition_;
    }

    /**
     * @brief Getter for the partition name
     * @return name
     */
    const char* name() const
    {
        return partition_ + 4;
    }

};

/**
 * Set of strings that introduces a logical partition among the topics visible by the Publisher and Subscriber.
 * A DataWriter within a Publisher only communicates with a DataReader in a Subscriber if (in addition to matching the
 * Topic and having compatible QoS) the Publisher and Subscriber have a common partition name string.
 *
 * The empty string ("") is considered a valid partition that is matched with other partition names using the same rules of
 * string matching and regular-expression matching used for any other partition name.
 * @note Mutable Qos Policy
 */
class PartitionQosPolicy : public Parameter_t, public QosPolicy
{
public:

    class const_iterator
    {
    public:

        typedef const_iterator self_type;
        typedef const Partition_t value_type;
        typedef const Partition_t reference;
        typedef const Partition_t* pointer;
        typedef size_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        /**
         * @brief Constructor using a pointer
         * @param ptr Pointer to be set
         */
        const_iterator(
                const fastrtps::rtps::octet* ptr)
            : ptr_(ptr)
            , value_ (ptr_)
        {
        }

        self_type operator ++()
        {
            self_type tmp = *this;
            advance();
            return tmp;
        }

        self_type operator ++(
                int)
        {
            advance();
            return *this;
        }

        reference operator *()
        {
            return value_;
        }

        pointer operator ->()
        {
            return &value_;
        }

        bool operator ==(
                const self_type& rhs)
        {
            return ptr_ == rhs.ptr_;
        }

        bool operator !=(
                const self_type& rhs)
        {
            return ptr_ != rhs.ptr_;
        }

    protected:

        /**
         * @brief Shift the pointer to the next element
         */
        void advance()
        {
            //Size of the element (with alignment)
            uint32_t size = *(uint32_t*)ptr_;
            ptr_ += (4u + ((size + 3u) & ~3u));
            value_ = Partition_t(ptr_);
        }

    private:

        //!Pointer
        const fastrtps::rtps::octet* ptr_;
        //!Partition
        Partition_t value_;

    };

public:

    /**
     * @brief Constructor without parameters
     */
    RTPS_DllAPI PartitionQosPolicy()
        : Parameter_t(PID_PARTITION, 0)
        , QosPolicy(false)
        , max_size_ (0)
        , Npartitions_ (0)
    {
    }

    /**
     * @brief Constructor using Parameter length
     * @param in_length Length of the parameter
     */
    RTPS_DllAPI PartitionQosPolicy(
            uint16_t in_length)
        : Parameter_t(PID_PARTITION, in_length)
        , QosPolicy(false)
        , max_size_ (in_length)
        , partitions_(in_length)
        , Npartitions_ (0)
    {
    }

    /**
     * @brief Copy constructor
     * @param b Another PartitionQosPolicy instance
     */
    RTPS_DllAPI PartitionQosPolicy(
            const PartitionQosPolicy& b)
        : Parameter_t(b)
        , QosPolicy(b)
        , max_size_ (b.max_size_)
        , partitions_(b.max_size_ != 0 ?
                b.partitions_.max_size :
                b.partitions_.length)
        , Npartitions_ (b.Npartitions_)
    {
        partitions_.copy(&b.partitions_, b.max_size_ != 0);
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~PartitionQosPolicy() = default;

    bool operator ==(
            const PartitionQosPolicy& b) const
    {
        return (this->max_size_ == b.max_size_) &&
               (this->Npartitions_ == b.Npartitions_) &&
               (this->partitions_ == b.partitions_) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    PartitionQosPolicy& operator =(
            const PartitionQosPolicy& b)
    {
        QosPolicy::operator =(b);
        Parameter_t::operator =(b);
        max_size_ = b.max_size_;
        partitions_.reserve(max_size_ != 0 ?
                b.partitions_.max_size :
                b.partitions_.length);
        partitions_.copy(&b.partitions_, b.max_size_ != 0);
        Npartitions_ = b.Npartitions_;

        return *this;
    }

    /**
     * @brief Getter for the first position of the partition list
     * @return const_iterator
     */
    const_iterator begin() const
    {
        return const_iterator(partitions_.data);
    }

    /**
     * @brief Getter for the end of the partition list
     * @return const_iterator
     */
    const_iterator end() const
    {
        return const_iterator(partitions_.data + partitions_.length);
    }

    /**
     * @brief Getter for the number of partitions
     * @return uint32_t with the size
     */
    uint32_t size() const
    {
        return Npartitions_;
    }

    /**
     * @brief Check if the set is empty
     * @return true if it is empty, false otherwise
     */
    uint32_t empty() const
    {
        return Npartitions_ == 0;
    }

    /**
     * @brief Setter for the maximum size reserved for partitions (in bytes)
     * @param size Size to be set
     */
    void set_max_size (
            uint32_t size)
    {
        partitions_.reserve(size);
        max_size_ = size;
    }

    /**
     * @brief Getter for the maximum size (in bytes)
     * @return uint32_t with the maximum size
     */
    uint32_t max_size () const
    {
        return max_size_;
    }

    /**
     * Appends a name to the list of partition names.
     * @param name Name to append.
     */
    RTPS_DllAPI inline void push_back(
            const char* name)
    {
        //Realloc if needed;
        uint32_t size = (uint32_t)strlen(name) + 1;
        uint32_t alignment = ((size + 3u) & ~3u) - size;

        if (max_size_ != 0 && (partitions_.max_size < partitions_.length +
                size + alignment + 4))
        {
            return;
        }

        partitions_.reserve(partitions_.length + size + alignment + 4);

        fastrtps::rtps::octet* o = (fastrtps::rtps::octet*)&size;
        memcpy(partitions_.data + partitions_.length, o, 4);
        partitions_.length += 4;

        memcpy(partitions_.data + partitions_.length, name, size);
        partitions_.length += size;

        memset(partitions_.data + partitions_.length, 0, alignment);
        partitions_.length += alignment;

        ++Npartitions_;
        hasChanged = true;
    }

    /**
     * Clears list of partition names
     */
    RTPS_DllAPI inline void clear() override
    {
        partitions_.length = 0;
        Npartitions_ = 0;
        hasChanged = false;
    }

    /**
     * Returns partition names.
     * @return Vector of partition name strings.
     */
    RTPS_DllAPI inline const std::vector<std::string> getNames() const
    {
        return names();
    }

    /**
     * Overrides partition names
     * @param nam Vector of partition name strings.
     */
    RTPS_DllAPI inline void setNames(
            std::vector<std::string>& nam)
    {
        names(nam);
    }

    /**
     * Returns partition names.
     * @return Vector of partition name strings.
     */
    RTPS_DllAPI inline const std::vector<std::string> names() const
    {
        std::vector<std::string> names;
        if (Npartitions_ > 0)
        {
            for (auto it = begin(); it != end(); ++it)
            {
                names.push_back(it->name());
            }
        }
        return names;
    }

    /**
     * Overrides partition names
     * @param nam Vector of partition name strings.
     */
    RTPS_DllAPI inline void names(
            std::vector<std::string>& nam)
    {
        clear();
        for (auto it = nam.begin(); it != nam.end(); ++it)
        {
            push_back(it->c_str());
        }
        hasChanged = true;
    }

private:

    //! Maximum size <br> By default, 0.
    uint32_t max_size_;
    //! Partitions
    fastrtps::rtps::SerializedPayload_t partitions_;
    //! Number of partitions. <br> By default, 0.
    uint32_t Npartitions_;
};

/**
 * Enum HistoryQosPolicyKind, different kinds of History Qos for HistoryQosPolicy.
 */
enum HistoryQosPolicyKind : fastrtps::rtps::octet
{
    /**
     * On the publishing side, the Service will only attempt to keep the most recent “depth” samples of each instance
     * of data (identified by its key) managed by the DataWriter. On the subscribing side, the DataReader will only attempt
     * to keep the most recent “depth” samples received for each instance (identified by its key) until the application
     * “takes” them via the DataReader’s take operation.
     */
    KEEP_LAST_HISTORY_QOS,
    /**
     * On the publishing side, the Service will attempt to keep all samples (representing each value written) of each
     * instance of data (identified by its key) managed by the DataWriter until they can be delivered to all subscribers.
     * On the subscribing side, the Service will attempt to keep all samples of each instance of data (identified by its
     * key) managed by the DataReader. These samples are kept until the application “takes” them from the Service via the
     * take operation.
     */
    KEEP_ALL_HISTORY_QOS
};

/**
 * Specifies the behavior of the Service in the case where the value of a sample changes (one or more times) before it
 * can be successfully communicated to one or more existing subscribers. This QoS policy controls whether the Service
 * should deliver only the most recent value, attempt to deliver all intermediate values, or do something in between.
 * On the publishing side this policy controls the samples that should be maintained by the DataWriter on behalf of
 * existing DataReader entities. The behavior with regards to a DataReaderentities discovered after a sample is written
 * is controlled by the DURABILITY QoS policy. On the subscribing side it controls the samples that should be maintained
 * until the application “takes” them from the Service.
 * @note Immutable Qos Policy
 */
class HistoryQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI HistoryQosPolicy()
        : Parameter_t(PID_HISTORY, PARAMETER_KIND_LENGTH + 4)
        , QosPolicy(true)
        , kind(KEEP_LAST_HISTORY_QOS)
        , depth(1)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~HistoryQosPolicy() = default;

    bool operator ==(
            const HistoryQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               (this->depth == b.depth) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        HistoryQosPolicy reset = HistoryQosPolicy();
        std::swap(*this, reset);
    }

public:

    //!HistoryQosPolicyKind. <br> By default, KEEP_LAST_HISTORY_QOS.
    HistoryQosPolicyKind kind;
    //!History depth. <br> By default, 1. If a value other than 1 is specified, it should
    //! be consistent with the settings of the ResourceLimitsQosPolicy.
    //! @warning Only takes effect if the kind is KEEP_LAST_HISTORY_QOS.
    int32_t depth;
};

/**
 * Specifies the resources that the Service can consume in order to meet the requested QoS
 * @note Immutable Qos Policy
 */
class ResourceLimitsQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Specifies the maximum number of data-samples the DataWriter (or DataReader) can manage across all the
     * instances associated with it. Represents the maximum samples the middleware can store for any one DataWriter
     * (or DataReader). <br>
     * By default, 5000.
     * @warning It is inconsistent for this value to be less than max_samples_per_instance.
     */
    int32_t max_samples;
    /**
     * @brief Represents the maximum number of instances DataWriter (or DataReader) can manage. <br>
     * By default, 10.
     */
    int32_t max_instances;
    /**
     * @brief Represents the maximum number of samples of any one instance a DataWriter(or DataReader) can manage. <br>
     * By default, 400.
     * @warning It is inconsistent for this value to be greater than max_samples.
     */
    int32_t max_samples_per_instance;
    /**
     * @brief Number of samples currently allocated. <br>
     * By default, 100.
     */
    int32_t allocated_samples;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI ResourceLimitsQosPolicy()
        : Parameter_t(PID_RESOURCE_LIMITS, 4 + 4 + 4)
        , QosPolicy(false)
        , max_samples(5000)
        , max_instances(10)
        , max_samples_per_instance(400)
        , allocated_samples(100)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~ResourceLimitsQosPolicy() = default;

    bool operator ==(
            const ResourceLimitsQosPolicy& b) const
    {
        return (this->max_samples == b.max_samples) &&
               (this->max_instances == b.max_instances) &&
               (this->max_samples_per_instance == b.max_samples_per_instance) &&
               (this->allocated_samples == b.allocated_samples) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        ResourceLimitsQosPolicy reset = ResourceLimitsQosPolicy();
        std::swap(*this, reset);
    }

};



/**
 * Specifies the configuration of the durability service. That is, the service that implements the DurabilityQosPolicy kind
 * of TRANSIENT and PERSISTENT.
 * @warning This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * @note Immutable Qos Policy
 */
class DurabilityServiceQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DurabilityServiceQosPolicy()
        : Parameter_t(PID_DURABILITY_SERVICE, PARAMETER_TIME_LENGTH + PARAMETER_KIND_LENGTH + 4 + 4 + 4 + 4)
        , QosPolicy(false)
        , history_kind(KEEP_LAST_HISTORY_QOS)
        , history_depth(1)
        , max_samples(-1)
        , max_instances(-1)
        , max_samples_per_instance(-1)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~DurabilityServiceQosPolicy() = default;

    bool operator ==(
            const DurabilityServiceQosPolicy& b) const
    {
        return (this->history_kind == b.history_kind) &&
               (this->history_depth == b.history_depth) &&
               (this->max_samples == b.max_samples) &&
               (this->max_instances == b.max_instances) &&
               (this->max_samples_per_instance == b.max_samples_per_instance) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        DurabilityServiceQosPolicy reset = DurabilityServiceQosPolicy();
        std::swap(*this, reset);
    }

public:

    /**
     * @brief Control when the service is able to remove all information regarding a data-instance. <br>
     * By default, c_TimeZero.
     */
    fastrtps::Duration_t service_cleanup_delay;
    /**
     * @brief Controls the HistoryQosPolicy of the fictitious DataReader that stores the data within the durability service.
     * <br>
     * By default, KEEP_LAST_HISTORY_QOS.
     */
    HistoryQosPolicyKind history_kind;
    /**
     * @brief Number of most recent values that should be maintained on the History. It only have effect if the history_kind
     * is KEEP_LAST_HISTORY_QOS. <br>
     * By default, 1.
     */
    int32_t history_depth;
    /**
     * @brief Control the ResourceLimitsQos of the implied DataReader that stores the data within the durability service.
     * Specifies the maximum number of data-samples the DataWriter (or DataReader) can manage across all the instances
     * associated with it. Represents the maximum samples the middleware can store for any one DataWriter (or DataReader).
     * It is inconsistent for this value to be less than max_samples_per_instance. <br>
     * By default, -1 (Length Unlimited).
     */
    int32_t max_samples;
    /**
     * @brief Control the ResourceLimitsQos of the implied DataReader that stores the data within the durability service.
     * Represents the maximum number of instances DataWriter (or DataReader) can manage. <br>
     * By default, -1 (Length Unlimited).
     */
    int32_t max_instances;
    /**
     * @brief Control the ResourceLimitsQos of the implied DataReader that stores the data within the durability service.
     * Represents the maximum number of samples of any one instance a DataWriter(or DataReader) can manage.
     * It is inconsistent for this value to be greater than max_samples. <br>
     * By default, -1 (Length Unlimited).
     */
    int32_t max_samples_per_instance;
};

/**
 * Specifies the maximum duration of validity of the data written by the DataWriter.
 * @note Mutable Qos Policy
 */
class LifespanQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI LifespanQosPolicy()
        : Parameter_t(PID_LIFESPAN, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~LifespanQosPolicy() = default;

    bool operator ==(
            const LifespanQosPolicy& b) const
    {
        return (this->duration == b.duration) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        LifespanQosPolicy reset = LifespanQosPolicy();
        std::swap(*this, reset);
    }

public:

    //! Period of validity. <br> By default, c_TimeInfinite.
    fastrtps::Duration_t duration;
};

/**
 * Specifies the value of the “strength” used to arbitrate among multiple DataWriter objects that attempt to modify the same
 * instance of a data-object (identified by Topic + key).This policy only applies if the OWNERSHIP QoS policy is of kind
 * EXCLUSIVE.
 * @note Mutable Qos Policy
 */
class OwnershipStrengthQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI OwnershipStrengthQosPolicy()
        : Parameter_t(PID_OWNERSHIP_STRENGTH, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~OwnershipStrengthQosPolicy() = default;

    bool operator ==(
            const OwnershipStrengthQosPolicy& b) const
    {
        return (this->value == b.value) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        OwnershipStrengthQosPolicy reset = OwnershipStrengthQosPolicy();
        std::swap(*this, reset);
    }

public:

    //! Strength <br> By default, 0.
    uint32_t value;
};


/**
 * This policy is a hint to the infrastructure as to how to set the priority of the underlying transport used to send the data.
 * @warning This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * @note Mutable Qos Policy
 */
class TransportPriorityQosPolicy : public Parameter_t, public QosPolicy
{
public:

    //!Priority <br> By default, 0.
    uint32_t value;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TransportPriorityQosPolicy()
        : Parameter_t(PID_TRANSPORT_PRIORITY, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TransportPriorityQosPolicy() = default;

    bool operator ==(
            const TransportPriorityQosPolicy& b) const
    {
        return (this->value == b.value) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        TransportPriorityQosPolicy reset = TransportPriorityQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Enum PublishModeQosPolicyKind, different kinds of publication synchronism
 */
typedef enum PublishModeQosPolicyKind : fastrtps::rtps::octet
{
    SYNCHRONOUS_PUBLISH_MODE,    //!< Synchronous publication mode (default for writers).
    ASYNCHRONOUS_PUBLISH_MODE    //!< Asynchronous publication mode.
} PublishModeQosPolicyKind_t;

/**
 * Class PublishModeQosPolicy, defines the publication mode for a specific writer.
 */
class PublishModeQosPolicy : public QosPolicy
{
public:

    //!PublishModeQosPolicyKind <br> By default, SYNCHRONOUS_PUBLISH_MODE.
    PublishModeQosPolicyKind kind;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI PublishModeQosPolicy()
        : kind(SYNCHRONOUS_PUBLISH_MODE)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~PublishModeQosPolicy() = default;

    inline void clear() override
    {
        PublishModeQosPolicy reset = PublishModeQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Enum DataRepresentationId, different kinds of topic data representation
 */
typedef enum DataRepresentationId : int16_t
{
    XCDR_DATA_REPRESENTATION = 0,   //!< Extended CDR Encoding version 1
    XML_DATA_REPRESENTATION = 1,    //!< XML Data Representation (Unsupported)
    XCDR2_DATA_REPRESENTATION = 2    //!< Extended CDR Encoding version 2
} DataRepresentationId_t;

/**
 * With multiple standard data Representations available, and vendor-specific extensions possible, DataWriters and
 * DataReaders must be able to negotiate which data representation(s) to use. This negotiation shall occur based on
 * DataRepresentationQosPolicy.
 * @warning If a writer’s offered representation is contained within a reader’s sequence, the offer satisfies the
 * request and the policies are compatible. Otherwise, they are incompatible.
 * @note Immutable Qos Policy
 */
class DataRepresentationQosPolicy : public Parameter_t, public QosPolicy
{
public:

    //!List of DataRepresentationId. <br> By default, empty list.
    std::vector<DataRepresentationId_t> m_value;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DataRepresentationQosPolicy()
        : Parameter_t(PID_DATA_REPRESENTATION, 0)
        , QosPolicy(true)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~DataRepresentationQosPolicy() override = default;

    /**
     * Compares the given policy to check if it's equal.
     * @param b QoS Policy.
     * @return True if the policy is equal.
     */
    bool operator ==(
            const DataRepresentationQosPolicy& b) const
    {
        return (this->m_value == b.m_value) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        DataRepresentationQosPolicy reset = DataRepresentationQosPolicy();
        std::swap(*this, reset);
    }

};

enum TypeConsistencyKind : uint16_t
{
    /**
     * The DataWriter and the DataReader must support the same data type in order for them to communicate.
     */
    DISALLOW_TYPE_COERCION,
    /**
     * The DataWriter and the DataReader need not support the same data type in order for them to communicate as long as
     * the reader’s type is assignable from the writer’s type.
     */
    ALLOW_TYPE_COERCION
};

/**
 * The TypeConsistencyEnforcementQosPolicy defines the rules for determining whether the type used to publish a given data
 * stream is consistent with that used to subscribe to it. It applies to DataReaders.
 * @note Immutable Qos Policy
 */
class TypeConsistencyEnforcementQosPolicy : public Parameter_t, public QosPolicy
{
public:

    //!TypeConsistencyKind. <br> By default, ALLOW_TYPE_COERCION.
    TypeConsistencyKind m_kind;
    /**
     * @brief This option controls whether sequence bounds are taken into consideration for type assignability. If the
     * option is set to TRUE, sequence bounds (maximum lengths) are not considered as part of the type assignability.
     * This means that a T2 sequence type with maximum length L2 would be assignable to a T1 sequence type with maximum
     * length L1, even if L2 is greater than L1. If the option is set to false, then sequence bounds are taken into
     * consideration for type assignability and in order for T1 to be assignable from T2 it is required that L1>= L2. <br>
     * By default, true.
     */
    bool m_ignore_sequence_bounds;
    /**
     * @brief This option controls whether string bounds are taken into consideration for type assignability. If the option
     *  is set to TRUE, string bounds (maximum lengths) are not considered as part of the type assignability. This means
     * that a T2 string type with maximum length L2 would be assignable to a T1 string type with maximum length L1, even
     * if L2 is greater than L1. If the option is set to false, then string bounds are taken into consideration for type
     * assignability and in order for T1 to be assignable from T2 it is required that L1>= L2. <br>
     * By default, true.
     */
    bool m_ignore_string_bounds;
    /**
     * @brief This option controls whether member names are taken into consideration for type assignability. If the option
     * is set to TRUE, member names are considered as part of assignability in addition to member IDs (so that members with
     * the same ID also have the same name). If the option is set to FALSE, then member names are not ignored. <br>
     * By default, false.
     */
    bool m_ignore_member_names;
    /**
     * @brief This option controls whether type widening is allowed. If the option is set to FALSE, type widening is
     * permitted. If the option is set to TRUE,it shall cause a wider type to not be assignable to a narrower type. <br>
     * By default, false.
     */
    bool m_prevent_type_widening;
    /**
     * @brief This option requires type information to be available in order to complete matching between a DataWriter and
     * DataReader when set to TRUE, otherwise matching can occur without complete type information when set to FALSE. <br>
     * By default, false.
     */
    bool m_force_type_validation;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TypeConsistencyEnforcementQosPolicy()
        : Parameter_t(PID_TYPE_CONSISTENCY_ENFORCEMENT, 8) // 2 + 5 + 1 alignment byte
        , QosPolicy(true)
    {
        m_kind = ALLOW_TYPE_COERCION;
        m_ignore_sequence_bounds = true;
        m_ignore_string_bounds = true;
        m_ignore_member_names = false;
        m_prevent_type_widening = false;
        m_force_type_validation = false;
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TypeConsistencyEnforcementQosPolicy() override = default;

    bool operator ==(
            const TypeConsistencyEnforcementQosPolicy& b) const
    {
        return m_kind == b.m_kind &&
               m_ignore_sequence_bounds == b.m_ignore_sequence_bounds &&
               m_ignore_string_bounds == b.m_ignore_string_bounds &&
               m_ignore_member_names == b.m_ignore_member_names &&
               m_prevent_type_widening == b.m_prevent_type_widening &&
               m_force_type_validation == b.m_force_type_validation &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        TypeConsistencyEnforcementQosPolicy reset = TypeConsistencyEnforcementQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Class DisablePositiveACKsQosPolicy to disable sending of positive ACKs
 * @note Immutable Qos Policy
 */
class DisablePositiveACKsQosPolicy : public Parameter_t, public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DisablePositiveACKsQosPolicy()
        : Parameter_t(PID_DISABLE_POSITIVE_ACKS, PARAMETER_BOOL_LENGTH)
        , QosPolicy(true)
        , enabled(false)
        , duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~DisablePositiveACKsQosPolicy() = default;

    bool operator ==(
            const DisablePositiveACKsQosPolicy& b) const
    {
        return enabled == b.enabled &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        DisablePositiveACKsQosPolicy reset = DisablePositiveACKsQosPolicy();
        std::swap(*this, reset);
    }

public:

    //! True if this QoS is enabled. <br> By default, false
    bool enabled;
    //! The duration to keep samples for (not serialized as not needed by reader). <br> By default, c_TimeInfinite
    fastrtps::Duration_t duration;
};

/**
 * Class TypeIdV1
 */
class TypeIdV1 : public Parameter_t, public QosPolicy
{
public:

    //!Type Identifier
    fastrtps::types::TypeIdentifier m_type_identifier;


    /**
     * @brief Constructor without parameters
     */
    RTPS_DllAPI TypeIdV1()
        : Parameter_t(PID_TYPE_IDV1, 0)
        , QosPolicy(false)
        , m_type_identifier()
    {
    }

    /**
     * @brief Copy constructor
     * @param type Another instance of TypeIdV1
     */
    RTPS_DllAPI TypeIdV1(
            const TypeIdV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
        , m_type_identifier(type.m_type_identifier)
    {
    }

    /**
     * @brief Constructor using a TypeIndentifier
     * @param identifier TypeIdentifier to be set
     */
    RTPS_DllAPI TypeIdV1(
            const fastrtps::types::TypeIdentifier& identifier)
        : Parameter_t(PID_TYPE_IDV1, 0)
        , QosPolicy(false)
        , m_type_identifier(identifier)
    {
    }

    /**
     * @brief Move constructor
     * @param type Another instance of TypeIdV1
     */
    RTPS_DllAPI TypeIdV1(
            TypeIdV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
        , m_type_identifier(std::move(type.m_type_identifier))
    {
    }

    RTPS_DllAPI TypeIdV1& operator =(
            const TypeIdV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        m_type_identifier = type.m_type_identifier;

        return *this;
    }

    RTPS_DllAPI TypeIdV1& operator =(
            TypeIdV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        m_type_identifier = std::move(type.m_type_identifier);

        return *this;
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TypeIdV1() override = default;

    inline void clear() override
    {
        *this = TypeIdV1();
    }

    RTPS_DllAPI TypeIdV1& operator =(
            const fastrtps::types::TypeIdentifier& type_id)
    {
        m_type_identifier = type_id;
        return *this;
    }

    /**
     * @brief Getter for the TypeIndentifier
     * @return TypeIdentifier reference
     */
    RTPS_DllAPI const fastrtps::types::TypeIdentifier& get() const
    {
        return m_type_identifier;
    }

};

/**
 * Class TypeObjectV1
 */
class TypeObjectV1 : public Parameter_t, public QosPolicy
{
public:

    //!Type Object
    fastrtps::types::TypeObject m_type_object;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TypeObjectV1()
        : Parameter_t(PID_TYPE_OBJECTV1, 0)
        , QosPolicy(false)
        , m_type_object()
    {
    }

    /**
     * @brief Copy constructor
     * @param type Another instance of TypeObjectV1
     */
    RTPS_DllAPI TypeObjectV1(
            const TypeObjectV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
        , m_type_object(type.m_type_object)
    {
    }

    /**
     * @brief Constructor using a TypeObject
     * @param type TypeObject to be set
     */
    RTPS_DllAPI TypeObjectV1(
            const fastrtps::types::TypeObject& type)
        : Parameter_t(PID_TYPE_OBJECTV1, 0)
        , QosPolicy(false)
        , m_type_object(type)
    {
    }

    /**
     * @brief Move constructor
     * @param type Another instance of TypeObjectV1
     */
    RTPS_DllAPI TypeObjectV1(
            TypeObjectV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
        , m_type_object(std::move(type.m_type_object))
    {
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            const TypeObjectV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        m_type_object = type.m_type_object;

        return *this;
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            TypeObjectV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        m_type_object = std::move(type.m_type_object);

        return *this;
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TypeObjectV1() override = default;

    inline void clear() override
    {
        *this = TypeObjectV1();
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            const fastrtps::types::TypeObject& type_object)
    {
        m_type_object = type_object;
        return *this;
    }

    /**
     * @brief Getter for the TypeObject
     * @return TypeObject reference
     */
    RTPS_DllAPI const fastrtps::types::TypeObject& get() const
    {
        return m_type_object;
    }

};

namespace xtypes {

/**
 * Class xtypes::TypeInformation
 */
class TypeInformation : public Parameter_t, public QosPolicy
{
public:

    //!Type Information
    fastrtps::types::TypeInformation type_information;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TypeInformation()
        : Parameter_t(PID_TYPE_INFORMATION, 0)
        , QosPolicy(false)
        , type_information()
        , assigned_(false)
    {
    }

    /**
     * @brief Copy constructor
     * @param type Another instance of TypeInformation
     */
    RTPS_DllAPI TypeInformation(
            const TypeInformation& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
        , type_information(type.type_information)
        , assigned_(type.assigned_)
    {
    }

    /**
     * @brief Constructor using a fastrtps::types::TypeInformation
     * @param info fastrtps::types::TypeInformation to be set
     */
    RTPS_DllAPI TypeInformation(
            const fastrtps::types::TypeInformation& info)
        : Parameter_t(PID_TYPE_INFORMATION, 0)
        , QosPolicy(false)
        , type_information(info)
        , assigned_(true)
    {
    }

    /**
     * @brief Move Constructor
     * @param type Another instance of TypeInformation
     */
    RTPS_DllAPI TypeInformation(
            TypeInformation&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
        , type_information(std::move(type.type_information))
        , assigned_(type.assigned_)
    {
    }

    RTPS_DllAPI TypeInformation& operator =(
            const TypeInformation& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        type_information = type.type_information;
        assigned_ = type.assigned_;

        return *this;
    }

    RTPS_DllAPI TypeInformation& operator =(
            TypeInformation&& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        type_information = std::move(type.type_information);
        assigned_ = type.assigned_;

        return *this;
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TypeInformation() override = default;

    inline void clear() override
    {
        *this = TypeInformation();
    }

    /**
     * @brief Check if it is assigned
     * @return true if assigned, false if not
     */
    RTPS_DllAPI bool assigned() const
    {
        return assigned_;
    }

    /**
     * @brief Setter for assigned boolean
     * @param value Boolean to be set
     */
    RTPS_DllAPI void assigned(
            bool value)
    {
        assigned_ = value;
    }

    RTPS_DllAPI TypeInformation& operator =(
            const fastrtps::types::TypeInformation& type_info)
    {
        type_information = type_info;
        assigned_ = true;
        return *this;
    }

private:

    //!Boolean that states if the TypeInformation has been asigned manually or not.
    bool assigned_;
};

} // namespace xtypes

//!Holds allocation limits affecting collections managed by a participant.
using ParticipantResourceLimitsQos = fastrtps::rtps::RTPSParticipantAllocationAttributes;

//! Property policies
using PropertyPolicyQos = fastrtps::rtps::PropertyPolicy;

//! Qos Policy that configures the wire protocol
class WireProtocolConfigQos : public QosPolicy
{

public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI WireProtocolConfigQos()
        : QosPolicy(false)
        , participant_id(-1)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~WireProtocolConfigQos() = default;

    bool operator ==(
            const WireProtocolConfigQos& b) const
    {
        return (this->prefix == b.prefix) &&
               (this->participant_id == b.participant_id) &&
               (this->builtin == b.builtin) &&
               (this->port == b.port) &&
               (this->throughput_controller == b.throughput_controller) &&
               (this->default_unicast_locator_list == b.default_unicast_locator_list) &&
               (this->default_multicast_locator_list == b.default_multicast_locator_list) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        WireProtocolConfigQos reset = WireProtocolConfigQos();
        std::swap(*this, reset);
    }

    //! Optionally allows user to define the GuidPrefix_t
    fastrtps::rtps::GuidPrefix_t prefix;

    //!Participant ID <br> By default, -1.
    int32_t participant_id;

    //! Builtin parameters.
    fastrtps::rtps::BuiltinAttributes builtin;

    //!Port Parameters
    fastrtps::rtps::PortParameters port;

    //!Throughput controller parameters. Leave default for uncontrolled flow.
    fastrtps::rtps::ThroughputControllerDescriptor throughput_controller;

    /**
     * Default list of Unicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the case
     * that it was defined with NO UnicastLocators. At least ONE locator should be included in this list.
     */
    fastrtps::rtps::LocatorList_t default_unicast_locator_list;

    /**
     * Default list of Multicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the
     * case that it was defined with NO UnicastLocators. This is usually left empty.
     */
    fastrtps::rtps::LocatorList_t default_multicast_locator_list;
};

//! Qos Policy to configure the transport layer
class TransportConfigQos : public QosPolicy
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TransportConfigQos()
        : QosPolicy(false)
        , use_builtin_transports(true)
        , send_socket_buffer_size(0)
        , listen_socket_buffer_size(0)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~TransportConfigQos() = default;

    bool operator ==(
            const TransportConfigQos& b) const
    {
        return (this->user_transports == b.user_transports) &&
               (this->use_builtin_transports == b.use_builtin_transports) &&
               (this->send_socket_buffer_size == b.send_socket_buffer_size) &&
               (this->listen_socket_buffer_size == b.listen_socket_buffer_size) &&
               QosPolicy::operator ==(b);
    }

    inline void clear() override
    {
        TransportConfigQos reset = TransportConfigQos();
        std::swap(*this, reset);
    }

    //!User defined transports to use alongside or in place of builtins.
    std::vector<std::shared_ptr<fastdds::rtps::TransportDescriptorInterface>> user_transports;

    //!Set as false to disable the default UDPv4 implementation. <br> By default, true.
    bool use_builtin_transports;

    /*!
     * @brief Send socket buffer size for the send resource. Zero value indicates to use default system buffer size. <br>
     * By default, 0.
     */
    uint32_t send_socket_buffer_size;

    /*! Listen socket buffer for all listen resources. Zero value indicates to use default system buffer size. <br>
     * By default, 0.
     */
    uint32_t listen_socket_buffer_size;
};

//!Qos Policy to configure the endpoint
class RTPSEndpointQos
{
public:

    RTPS_DllAPI RTPSEndpointQos()
        : user_defined_id(-1)
        , entity_id(-1)
        , history_memory_policy(fastrtps::rtps::PREALLOCATED_MEMORY_MODE)
    {
    }

    virtual RTPS_DllAPI ~RTPSEndpointQos() = default;

    bool operator ==(
            const RTPSEndpointQos& b) const
    {
        return (this->unicast_locator_list == b.unicast_locator_list) &&
               (this->multicast_locator_list == b.multicast_locator_list) &&
               (this->remote_locator_list == b.remote_locator_list) &&
               (this->user_defined_id == b.user_defined_id) &&
               (this->entity_id == b.entity_id) &&
               (this->history_memory_policy == b.history_memory_policy);
    }

    //!Unicast locator list
    fastrtps::rtps::LocatorList_t unicast_locator_list;

    //!Multicast locator list
    fastrtps::rtps::LocatorList_t multicast_locator_list;

    //!Remote locator list
    fastrtps::rtps::LocatorList_t remote_locator_list;

    //!User Defined ID, used for StaticEndpointDiscovery. <br> By default, -1.
    int16_t user_defined_id;

    //!Entity ID, if the user wants to specify the EntityID of the endpoint. <br> By default, -1.
    int16_t entity_id;

    //!Underlying History memory policy. <br> By default, PREALLOCATED_MEMORY_MODE.
    fastrtps::rtps::MemoryManagementPolicy_t history_memory_policy;
};

//!Qos Policy to configure the limit of the writer resources
class WriterResourceLimitsQos
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI WriterResourceLimitsQos()
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~WriterResourceLimitsQos() = default;

    bool operator ==(
            const WriterResourceLimitsQos& b) const
    {
        return (this->matched_subscriber_allocation == b.matched_subscriber_allocation);
    }

    //!Matched subscribers allocation limits.
    fastrtps::ResourceLimitedContainerConfig matched_subscriber_allocation;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_QOS_QOSPOLICIES_HPP_
