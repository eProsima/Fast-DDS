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

#ifndef FASTDDS_DDS_CORE_POLICY__QOSPOLICIES_HPP
#define FASTDDS_DDS_CORE_POLICY__QOSPOLICIES_HPP

#include <bitset>
#include <vector>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {

namespace fastdds {
namespace rtps {
class EDP;
} // namespace rtps

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
    INVALID_QOS_POLICY_ID                    = 0,    //< Does not refer to any valid QosPolicy

    // Standard QosPolicies
    USERDATA_QOS_POLICY_ID                   = 1,    //< UserDataQosPolicy
    DURABILITY_QOS_POLICY_ID                 = 2,    //< DurabilityQosPolicy
    PRESENTATION_QOS_POLICY_ID               = 3,    //< PresentationQosPolicy
    DEADLINE_QOS_POLICY_ID                   = 4,    //< DeadlineQosPolicy
    LATENCYBUDGET_QOS_POLICY_ID              = 5,    //< LatencyBudgetQosPolicy
    OWNERSHIP_QOS_POLICY_ID                  = 6,    //< OwnershipQosPolicy
    OWNERSHIPSTRENGTH_QOS_POLICY_ID          = 7,    //< OwnershipStrengthQosPolicy
    LIVELINESS_QOS_POLICY_ID                 = 8,    //< LivelinessQosPolicy
    TIMEBASEDFILTER_QOS_POLICY_ID            = 9,    //< TimeBasedFilterQosPolicy
    PARTITION_QOS_POLICY_ID                  = 10,   //< PartitionQosPolicy
    RELIABILITY_QOS_POLICY_ID                = 11,   //< ReliabilityQosPolicy
    DESTINATIONORDER_QOS_POLICY_ID           = 12,   //< DestinationOrderQosPolicy
    HISTORY_QOS_POLICY_ID                    = 13,   //< HistoryQosPolicy
    RESOURCELIMITS_QOS_POLICY_ID             = 14,   //< ResourceLimitsQosPolicy
    ENTITYFACTORY_QOS_POLICY_ID              = 15,   //< EntityFactoryQosPolicy
    WRITERDATALIFECYCLE_QOS_POLICY_ID        = 16,   //< WriterDataLifecycleQosPolicy
    READERDATALIFECYCLE_QOS_POLICY_ID        = 17,   //< ReaderDataLifecycleQosPolicy
    TOPICDATA_QOS_POLICY_ID                  = 18,   //< TopicDataQosPolicy
    GROUPDATA_QOS_POLICY_ID                  = 19,   //< GroupDataQosPolicy
    TRANSPORTPRIORITY_QOS_POLICY_ID          = 20,   //< TransportPriorityQosPolicy
    LIFESPAN_QOS_POLICY_ID                   = 21,   //< LifespanQosPolicy
    DURABILITYSERVICE_QOS_POLICY_ID          = 22,   //< DurabilityServiceQosPolicy

    //XTypes extensions
    DATAREPRESENTATION_QOS_POLICY_ID         = 23,   //< DataRepresentationQosPolicy
    TYPECONSISTENCYENFORCEMENT_QOS_POLICY_ID = 24,   //< TypeConsistencyEnforcementQosPolicy

    //eProsima Extensions
    DISABLEPOSITIVEACKS_QOS_POLICY_ID        = 25,   //< DisablePositiveACKsQosPolicy
    PARTICIPANTRESOURCELIMITS_QOS_POLICY_ID  = 26,   //< ParticipantResourceLimitsQos
    PROPERTYPOLICY_QOS_POLICY_ID             = 27,   //< PropertyPolicyQos
    PUBLISHMODE_QOS_POLICY_ID                = 28,   //< PublishModeQosPolicy
    READERRESOURCELIMITS_QOS_POLICY_ID       = 29,   //< Reader ResourceLimitsQos
    RTPSENDPOINT_QOS_POLICY_ID               = 30,   //< RTPSEndpointQos
    RTPSRELIABLEREADER_QOS_POLICY_ID         = 31,   //< RTPSReliableReaderQos
    RTPSRELIABLEWRITER_QOS_POLICY_ID         = 32,   //< RTPSReliableWriterQos
    TRANSPORTCONFIG_QOS_POLICY_ID            = 33,   //< TransportConfigQos
    TYPECONSISTENCY_QOS_POLICY_ID            = 34,   //< TipeConsistencyQos
    WIREPROTOCOLCONFIG_QOS_POLICY_ID         = 35,   //< WireProtocolConfigQos
    WRITERRESOURCELIMITS_QOS_POLICY_ID       = 36,   //< WriterResourceLimitsQos

    NEXT_QOS_POLICY_ID                              //< Keep always the last element. For internal use only
};
using PolicyMask = std::bitset<NEXT_QOS_POLICY_ID>;

/**
 * Class QosPolicy, base for all QoS policies defined for Writers and Readers.
 */
class QosPolicy
{
public:

    QosPolicy()
        : hasChanged(false)
        , send_always_(false)
    {
    }

    QosPolicy(
            bool b_sendAlways)
        : hasChanged(false)
        , send_always_(b_sendAlways)
    {
    }

    virtual ~QosPolicy()
    {
    }

    bool hasChanged;
    /**
     * Whether it should always be sent.
     * @return True if it should always be sent.
     */
    virtual bool sendAlways() const
    {
        return send_always_;
    }

    virtual inline void clear() = 0;

    static uint32_t get_cdr_serialized_size(
            const std::vector<fastdds::rtps::octet>& data)
    {
        // Size of data
        uint32_t data_size = static_cast<uint32_t>(data.size());
        // Align to next 4 byte
        data_size = (data_size + 3) & ~3;
        // p_id + p_length + str_length + str_data
        return 2 + 2 + 4 + data_size;
    }

protected:

    bool send_always_;

};
/**
 * Enum DurabilityQosPolicyKind_t, different kinds of durability for DurabilityQosPolicy.
 */
typedef enum DurabilityQosPolicyKind : fastdds::rtps::octet
{
    VOLATILE_DURABILITY_QOS,        //!< Volatile Durability (default for Subscribers).
    TRANSIENT_LOCAL_DURABILITY_QOS, //!< Transient Local Durability (default for Publishers).
    TRANSIENT_DURABILITY_QOS,       //!< Transient Durability.
    PERSISTENT_DURABILITY_QOS       //!< NOT IMPLEMENTED.
} DurabilityQosPolicyKind_t;

#define PARAMETER_KIND_LENGTH 4

/**
 * Class DurabilityQosPolicy, to indicate the durability of the samples.
 * kind: Default value for Subscribers: VOLATILE_DURABILITY_QOS, for Publishers TRANSIENT_LOCAL_DURABILITY_QOS
 */
class DurabilityQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API DurabilityQosPolicy()
        : Parameter_t(PID_DURABILITY, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(VOLATILE_DURABILITY_QOS)
    {
    }

    virtual FASTDDS_EXPORTED_API ~DurabilityQosPolicy()
    {
    }

    DurabilityQosPolicyKind_t kind;

    /**
     * Translates kind to rtps layer equivalent
     */
    inline fastdds::rtps::DurabilityKind_t durabilityKind() const
    {
        switch (kind)
        {
            default:
            case VOLATILE_DURABILITY_QOS: return fastdds::rtps::VOLATILE;
            case TRANSIENT_LOCAL_DURABILITY_QOS: return fastdds::rtps::TRANSIENT_LOCAL;
            case TRANSIENT_DURABILITY_QOS: return fastdds::rtps::TRANSIENT;
            case PERSISTENT_DURABILITY_QOS: return fastdds::rtps::PERSISTENT;
        }
    }

    /**
     * Set kind from rtps layer equivalent
     */
    inline void durabilityKind(
            const fastdds::rtps::DurabilityKind_t new_kind)
    {
        switch (new_kind)
        {
            default:
            case fastdds::rtps::VOLATILE: kind = VOLATILE_DURABILITY_QOS; break;
            case fastdds::rtps::TRANSIENT_LOCAL: kind = TRANSIENT_LOCAL_DURABILITY_QOS; break;
            case fastdds::rtps::TRANSIENT: kind = TRANSIENT_DURABILITY_QOS; break;
            case fastdds::rtps::PERSISTENT: kind = PERSISTENT_DURABILITY_QOS; break;
        }

    }

    inline void clear() override
    {
        DurabilityQosPolicy reset = DurabilityQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Class DeadlineQosPolicy, to indicate the Deadline of the samples.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * period: Default value c_TimeInifinite.
 */
class DeadlineQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API DeadlineQosPolicy()
        : Parameter_t(PID_DEADLINE, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , period(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    virtual FASTDDS_EXPORTED_API ~DeadlineQosPolicy()
    {
    }

    inline void clear() override
    {
        DeadlineQosPolicy reset = DeadlineQosPolicy();
        std::swap(*this, reset);
    }

    fastdds::dds::Duration_t period;
};

/**
 * Class LatencyBudgetQosPolicy, to indicate the LatencyBudget of the samples.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * period: Default value dds::c_TimeZero.
 */
class LatencyBudgetQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API LatencyBudgetQosPolicy()
        : Parameter_t(PID_LATENCY_BUDGET, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , duration(0, 0)
    {
    }

    virtual FASTDDS_EXPORTED_API ~LatencyBudgetQosPolicy()
    {
    }

    inline void clear() override
    {
        LatencyBudgetQosPolicy reset = LatencyBudgetQosPolicy();
        std::swap(*this, reset);
    }

    fastdds::dds::Duration_t duration;
};

/**
 * Enum LivelinessQosPolicyKind, different kinds of liveliness for LivelinessQosPolicy
 */
typedef enum LivelinessQosPolicyKind : fastdds::rtps::octet
{
    AUTOMATIC_LIVELINESS_QOS,             //!< Automatic Liveliness, default value.
    MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, //!< MANUAL_BY_PARTICIPANT_LIVELINESS_QOS
    MANUAL_BY_TOPIC_LIVELINESS_QOS        //!< MANUAL_BY_TOPIC_LIVELINESS_QOS
} LivelinessQosPolicyKind;

/**
 * Class LivelinessQosPolicy, to indicate the Liveliness of the Writers.
 * This QosPolicy can be defined for the Subscribers and is transmitted but only the Writer Liveliness protocol
 * is implemented in this version. The user should set the lease_duration and the announcement_period with values that differ
 * in at least 30%. Values too close to each other may cause the failure of the writer liveliness assertion in networks
 * with high latency or with lots of communication errors.
 * kind: Default value AUTOMATIC_LIVELINESS_QOS
 * lease_duration: Default value dds::c_TimeInfinite.
 * announcement_period: Default value dds::c_TimeInfinite (must be < lease_duration).
 */
class LivelinessQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API LivelinessQosPolicy()
        : Parameter_t(PID_LIVELINESS, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , kind(AUTOMATIC_LIVELINESS_QOS)
    {
        lease_duration = fastdds::dds::c_TimeInfinite;
        announcement_period = fastdds::dds::c_TimeInfinite;
    }

    virtual FASTDDS_EXPORTED_API ~LivelinessQosPolicy()
    {
    }

    inline void clear() override
    {
        LivelinessQosPolicy reset = LivelinessQosPolicy();
        std::swap(*this, reset);
    }

    LivelinessQosPolicyKind kind;
    fastdds::dds::Duration_t lease_duration;
    fastdds::dds::Duration_t announcement_period;
};

/**
 * Enum ReliabilityQosPolicyKind, different kinds of reliability for ReliabilityQosPolicy.
 */
typedef enum ReliabilityQosPolicyKind : fastdds::rtps::octet
{
    BEST_EFFORT_RELIABILITY_QOS = 0x01, //!< Best Effort reliability (default for Subscribers).
    RELIABLE_RELIABILITY_QOS = 0x02 //!< Reliable reliability (default for Publishers).
} ReliabilityQosPolicyKind;

/**
 * Class ReliabilityQosPolicy, to indicate the reliability of the endpoints.
 * kind: Default value BEST_EFFORT_RELIABILITY_QOS for ReaderQos and RELIABLE_RELIABILITY_QOS for WriterQos.
 * max_blocking_time: Not Used in this version.
 */
class ReliabilityQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API ReliabilityQosPolicy()
        : Parameter_t(PID_RELIABILITY, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        ,                //indicate send always
        kind(BEST_EFFORT_RELIABILITY_QOS)
        ,// max_blocking_time = 100ms
        max_blocking_time{0, 100000000}
    {
    }

    virtual FASTDDS_EXPORTED_API ~ReliabilityQosPolicy()
    {
    }

    inline void clear() override
    {
        ReliabilityQosPolicy reset = ReliabilityQosPolicy();
        std::swap(*this, reset);
    }

    ReliabilityQosPolicyKind kind;
    fastdds::dds::Duration_t max_blocking_time;
};



/**
 * Enum OwnershipQosPolicyKind, different kinds of ownership for OwnershipQosPolicy.
 */
enum OwnershipQosPolicyKind : fastdds::rtps::octet
{
    SHARED_OWNERSHIP_QOS, //!< Shared Ownership, default value.
    EXCLUSIVE_OWNERSHIP_QOS //!< Exclusive ownership
};

/**
 * Class OwnershipQosPolicy, to indicate the ownership kind of the endpoints.
 * kind: Default value SHARED_OWNERSHIP_QOS.
 */
class OwnershipQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API OwnershipQosPolicy()
        : Parameter_t(PID_OWNERSHIP, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(SHARED_OWNERSHIP_QOS)
    {
    }

    virtual FASTDDS_EXPORTED_API ~OwnershipQosPolicy()
    {
    }

    inline void clear() override
    {
        OwnershipQosPolicy reset = OwnershipQosPolicy();
        std::swap(*this, reset);
    }

    OwnershipQosPolicyKind kind;
};

/**
 * Enum OwnershipQosPolicyKind, different kinds of destination order for DestinationOrderQosPolicy.
 */
enum DestinationOrderQosPolicyKind : fastdds::rtps::octet
{
    BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, //!< By Reception Timestamp, default value.
    BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS //!< By Source Timestamp.
};



/**
 * Class DestinationOrderQosPolicy, to indicate the Destination Order Qos.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * kind: Default value BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS
 */
class DestinationOrderQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    DestinationOrderQosPolicyKind kind;
    FASTDDS_EXPORTED_API DestinationOrderQosPolicy()
        : Parameter_t(PID_DESTINATION_ORDER, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS)
    {
    }

    virtual FASTDDS_EXPORTED_API ~DestinationOrderQosPolicy()
    {
    }

    inline void clear() override
    {
        DestinationOrderQosPolicy reset = DestinationOrderQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Class GenericDataQosPolicy, base class to transmit user data during the discovery phase.
 */
class GenericDataQosPolicy : public Parameter_t, public QosPolicy,
    public fastdds::ResourceLimitedVector<fastdds::rtps::octet>
{
    using ResourceLimitedOctetVector = fastdds::ResourceLimitedVector<fastdds::rtps::octet>;

public:

    FASTDDS_EXPORTED_API GenericDataQosPolicy(
            ParameterId_t pid)
        : Parameter_t(pid, 0)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
    }

    FASTDDS_EXPORTED_API GenericDataQosPolicy(
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
    FASTDDS_EXPORTED_API GenericDataQosPolicy(
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
    FASTDDS_EXPORTED_API GenericDataQosPolicy(
            ParameterId_t pid,
            const collection_type& data)
        : Parameter_t(pid, 0)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
        assign(data.begin(), data.end());
        length = static_cast<uint16_t>((size() + 7u) & ~3u);
    }

    virtual FASTDDS_EXPORTED_API ~GenericDataQosPolicy()
    {
    }

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
               send_always_ == b.send_always_ &&
               hasChanged == b.hasChanged;
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
    void set_max_size(
            size_t size)
    {
        if (size > 0)
        {
            configuration_ = fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(size);
            collection_.reserve(configuration_.maximum);
        }
        else
        {
            configuration_ = fastdds::ResourceLimitedContainerConfig::dynamic_allocation_configuration();
        }
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
    FASTDDS_EXPORTED_API inline const collection_type& data_vec() const
    {
        return collection_;
    }

    /**
     * Sets raw data vector.
     * @param vec raw data to set.
     * */
    FASTDDS_EXPORTED_API inline void data_vec(
            const collection_type& vec)
    {
        assign(vec.begin(), vec.end());
    }

    /**
     * Returns raw data vector.
     * @return raw data as vector of octets.
     * */
    FASTDDS_EXPORTED_API inline const collection_type& getValue() const
    {
        return collection_;
    }

    /**
     * Sets raw data vector.
     * @param vec raw data to set.
     * */
    FASTDDS_EXPORTED_API inline void setValue(
            const collection_type& vec)
    {
        assign(vec.begin(), vec.end());
    }

};

/**
 * Class TClassName, base template for user data qos policies.
 */
#define TEMPLATE_DATA_QOS_POLICY(TClassName, TPid)                                     \
    class TClassName : public GenericDataQosPolicy                                         \
    {                                                                                      \
    public:                                                                                \
                                                                                       \
        FASTDDS_EXPORTED_API TClassName()                                                           \
            : GenericDataQosPolicy(TPid)                                                   \
        {                                                                                  \
        }                                                                                  \
                                                                                       \
        FASTDDS_EXPORTED_API TClassName(                                                            \
            uint16_t in_length)                                                        \
            : GenericDataQosPolicy(TPid, in_length)                                        \
        {                                                                                  \
        }                                                                                  \
                                                                                       \
        /** \
         * Construct from another TClassName. \
         * \
         * The resulting TClassName will have the same size limits \
         * as the input attribute \
         * \
         * @param data data to copy in the newly created object \
         */                                                                            \
        FASTDDS_EXPORTED_API TClassName(                                                            \
            const TClassName& data) = default;                                         \
                                                                                       \
        /** \
         * Construct from underlying collection type. \
         * \
         * Useful to easy integration on old APIs where a traditional container was used. \
         * The resulting TClassName will always be unlimited in size \
         * \
         * @param data data to copy in the newly created object \
         */                                                                            \
        FASTDDS_EXPORTED_API TClassName(                                                            \
            const collection_type& data)                                               \
            : GenericDataQosPolicy(TPid, data)                                             \
        {                                                                                  \
        }                                                                                  \
                                                                                       \
        virtual FASTDDS_EXPORTED_API ~TClassName() = default;                                       \
                                                                                       \
        /** \
         * Copies another TClassName. \
         * \
         * The resulting TClassName will have the same size limit \
         * as the input parameter, so all data in the input will be copied. \
         * \
         * @param b object to be copied \
         * @return reference to the current object. \
         */                                                                            \
        TClassName& operator =(                                                            \
            const TClassName& b) = default;                                            \
                                                                                       \
    };

TEMPLATE_DATA_QOS_POLICY(UserDataQosPolicy, PID_USER_DATA)
TEMPLATE_DATA_QOS_POLICY(TopicDataQosPolicy, PID_TOPIC_DATA)
TEMPLATE_DATA_QOS_POLICY(GroupDataQosPolicy, PID_GROUP_DATA)

/**
 * Class TimeBasedFilterQosPolicy, to indicate the Time Based Filter Qos.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * minimum_separation: Default value dds::c_TimeZero
 */
class TimeBasedFilterQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API TimeBasedFilterQosPolicy()
        : Parameter_t(PID_TIME_BASED_FILTER, PARAMETER_TIME_LENGTH)
        , QosPolicy(false)
        , minimum_separation(0, 0)
    {
    }

    virtual FASTDDS_EXPORTED_API ~TimeBasedFilterQosPolicy()
    {
    }

    inline void clear() override
    {
        TimeBasedFilterQosPolicy reset = TimeBasedFilterQosPolicy();
        std::swap(*this, reset);
    }

    fastdds::dds::Duration_t minimum_separation;
};

/**
 * Enum PresentationQosPolicyAccessScopeKind, different kinds of Presentation Policy order for PresentationQosPolicy.
 */
enum PresentationQosPolicyAccessScopeKind : fastdds::rtps::octet
{
    INSTANCE_PRESENTATION_QOS, //!< Instance Presentation, default value.
    TOPIC_PRESENTATION_QOS, //!< Topic Presentation.
    GROUP_PRESENTATION_QOS //!< Group Presentation.
};

#define PARAMETER_PRESENTATION_LENGTH 8

/**
 * Class PresentationQosPolicy, to indicate the Presentation Qos Policy.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * access_scope: Default value INSTANCE_PRESENTATION_QOS
 * coherent_access: Default value false.
 * ordered_access: Default value false.
 */
class PresentationQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    PresentationQosPolicyAccessScopeKind access_scope;
    bool coherent_access;
    bool ordered_access;
    FASTDDS_EXPORTED_API PresentationQosPolicy()
        : Parameter_t(PID_PRESENTATION, PARAMETER_PRESENTATION_LENGTH)
        , QosPolicy(false)
        , access_scope(INSTANCE_PRESENTATION_QOS)
        , coherent_access(false)
        , ordered_access(false)
    {
    }

    virtual FASTDDS_EXPORTED_API ~PresentationQosPolicy()
    {
    }

    inline void clear() override
    {
        PresentationQosPolicy reset = PresentationQosPolicy();
        std::swap(*this, reset);
    }

};


/**
 * Class PartitionQosPolicy, to indicate the Partition Qos.
 */
class PartitionQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;
    friend class fastdds::rtps::EDP;

public:

    FASTDDS_EXPORTED_API PartitionQosPolicy()
        : Parameter_t(PID_PARTITION, 0)
        , QosPolicy(false)
    {
    }

    FASTDDS_EXPORTED_API PartitionQosPolicy(
            uint16_t in_length)
        : Parameter_t(PID_PARTITION, in_length)
        , QosPolicy(false)
        , names_{}
    {
    }

    virtual FASTDDS_EXPORTED_API ~PartitionQosPolicy()
    {
    }

    void set_max_size (
            uint32_t size)
    {
        max_size_ = size;
    }

    uint32_t max_size () const
    {
        return max_size_;
    }

    /**
     * Appends a name to the list of partition names.
     * @param name Name to append.
     */
    FASTDDS_EXPORTED_API inline void push_back(
            const char* name)
    {
        names_.push_back(std::string(name)); hasChanged = true;
    }

    /**
     * Clears list of partition names
     */
    FASTDDS_EXPORTED_API inline void clear() override
    {
        names_.clear();
    }

    /**
     * Returns partition names.
     * @return Vector of partition name strings.
     */
    FASTDDS_EXPORTED_API inline std::vector<std::string> names() const
    {
        return names_;
    }

    /**
     * Overrides partition names
     * @param nam Vector of partition name strings.
     */
    FASTDDS_EXPORTED_API inline void names(
            std::vector<std::string>& nam)
    {
        names_ = nam; hasChanged = true;
    }

    /**
     * Returns partition names.
     * @return Vector of partition name strings.
     */
    FASTDDS_EXPORTED_API inline const std::vector<std::string> getNames() const
    {
        return names();
    }

    /**
     * Overrides partition names
     * @param nam Vector of partition name strings.
     */
    FASTDDS_EXPORTED_API inline void setNames(
            std::vector<std::string>& nam)
    {
        names(nam);
    }

private:

    std::vector<std::string> names_;
    uint32_t max_size_ = 0;
};

/**
 * Enum HistoryQosPolicyKind, different kinds of History Qos for HistoryQosPolicy.
 */
enum HistoryQosPolicyKind : fastdds::rtps::octet
{
    KEEP_LAST_HISTORY_QOS, //!< Keep only a number of samples, default value.
    KEEP_ALL_HISTORY_QOS //!< Keep all samples until the ResourceLimitsQosPolicy are exhausted.
};

/**
 * Class HistoryQosPolicy, defines the HistoryQos of the topic in the Writer or Reader side.
 * kind: Default value KEEP_LAST_HISTORY_QOS.
 * depth: Default value 1000.
 */
class HistoryQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    HistoryQosPolicyKind kind;
    int32_t depth;
    FASTDDS_EXPORTED_API HistoryQosPolicy()
        : Parameter_t(PID_HISTORY, PARAMETER_KIND_LENGTH + 4)
        , QosPolicy(true)
        , kind(KEEP_LAST_HISTORY_QOS)
        , depth(1)
    {
    }

    virtual FASTDDS_EXPORTED_API ~HistoryQosPolicy()
    {
    }

    inline void clear() override
    {
        HistoryQosPolicy reset = HistoryQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Class ResourceLimitsQosPolicy, defines the ResourceLimits for the Writer or the Reader.
 * max_samples: Default value 5000.
 * max_instances: Default value 10.
 * max_samples_per_instance: Default value 400.
 * allocated_samples: Default value 100.
 */
class ResourceLimitsQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    int32_t max_samples;
    int32_t max_instances;
    int32_t max_samples_per_instance;
    int32_t allocated_samples;
    FASTDDS_EXPORTED_API ResourceLimitsQosPolicy()
        : Parameter_t(PID_RESOURCE_LIMITS, 4 + 4 + 4)
        , QosPolicy(false)
        , max_samples(5000)
        , max_instances(10)
        , max_samples_per_instance(400)
        , allocated_samples(100)
    {
    }

    virtual FASTDDS_EXPORTED_API ~ResourceLimitsQosPolicy()
    {
    }

    inline void clear() override
    {
        ResourceLimitsQosPolicy reset = ResourceLimitsQosPolicy();
        std::swap(*this, reset);
    }

};



/**
 * Class DurabilityServiceQosPolicy, to indicate the Durability Service.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * service_cleanup_delay: Default value dds::c_TimeZero.
 * history_kind: Default value KEEP_LAST_HISTORY_QOS.
 * history_depth: Default value 1.
 * max_samples: Default value -1.
 * max_instances: Default value -1.
 * max_samples_per_instance: Default value -1.
 */
class DurabilityServiceQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    fastdds::dds::Duration_t service_cleanup_delay;
    HistoryQosPolicyKind history_kind;
    int32_t history_depth;
    int32_t max_samples;
    int32_t max_instances;
    int32_t max_samples_per_instance;
    FASTDDS_EXPORTED_API DurabilityServiceQosPolicy()
        : Parameter_t(PID_DURABILITY_SERVICE, PARAMETER_TIME_LENGTH + PARAMETER_KIND_LENGTH + 4 + 4 + 4 + 4)
        , QosPolicy(false)
        , history_kind(KEEP_LAST_HISTORY_QOS)
        , history_depth(1)
        , max_samples(-1)
        , max_instances(-1)
        , max_samples_per_instance(-1)
    {
    }

    virtual FASTDDS_EXPORTED_API ~DurabilityServiceQosPolicy()
    {
    }

    inline void clear() override
    {
        DurabilityServiceQosPolicy reset = DurabilityServiceQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Class LifespanQosPolicy, currently unimplemented.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * duration: Default value dds::c_TimeInfinite.
 */
class LifespanQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API LifespanQosPolicy()
        : Parameter_t(PID_LIFESPAN, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , duration(fastdds::dds::c_TimeInfinite)
    {
    }

    virtual FASTDDS_EXPORTED_API ~LifespanQosPolicy()
    {
    }

    inline void clear() override
    {
        LifespanQosPolicy reset = LifespanQosPolicy();
        std::swap(*this, reset);
    }

    fastdds::dds::Duration_t duration;
};

/**
 * Class OwnershipStrengthQosPolicy, to indicate the strength of the ownership.
 * value: Default value 0.
 */
class OwnershipStrengthQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    uint32_t value;
    FASTDDS_EXPORTED_API OwnershipStrengthQosPolicy()
        : Parameter_t(PID_OWNERSHIP_STRENGTH, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    virtual FASTDDS_EXPORTED_API ~OwnershipStrengthQosPolicy()
    {
    }

    inline void clear() override
    {
        OwnershipStrengthQosPolicy reset = OwnershipStrengthQosPolicy();
        std::swap(*this, reset);
    }

};



/**
 * Class TransportPriorityQosPolicy, currently unimplemented.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * value: Default value 0.
 */
class TransportPriorityQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    uint32_t value;
    FASTDDS_EXPORTED_API TransportPriorityQosPolicy()
        : Parameter_t(PID_TRANSPORT_PRIORITY, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    virtual FASTDDS_EXPORTED_API ~TransportPriorityQosPolicy()
    {
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
typedef enum PublishModeQosPolicyKind : fastdds::rtps::octet
{
    SYNCHRONOUS_PUBLISH_MODE,   //!< Synchronous publication mode (default for writers).
    ASYNCHRONOUS_PUBLISH_MODE   //!< Asynchronous publication mode.
} PublishModeQosPolicyKind_t;

/**
 * Class PublishModeQosPolicy, defines the publication mode for a specific writer.
 * kind: Default value SYNCHRONOUS_PUBLISH_MODE.
 */
class PublishModeQosPolicy : public QosPolicy
{
public:

    PublishModeQosPolicyKind kind;
    FASTDDS_EXPORTED_API PublishModeQosPolicy()
        : kind(SYNCHRONOUS_PUBLISH_MODE)
    {
    }

    virtual FASTDDS_EXPORTED_API ~PublishModeQosPolicy()
    {
    }

    bool operator == (
            const PublishModeQosPolicy& other) const
    {
        return other.kind == kind;
    }

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
    XCDR_DATA_REPRESENTATION,   //!<
    XML_DATA_REPRESENTATION,    //!<
    XCDR2_DATA_REPRESENTATION   //!<
} DataRepresentationId_t;

//! Default @ref DataRepresentationId used in Fast DDS.
constexpr DataRepresentationId_t DEFAULT_DATA_REPRESENTATION {DataRepresentationId_t::XCDR_DATA_REPRESENTATION};

/**
 * Class DataRepresentationQosPolicy,
 */
class DataRepresentationQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    std::vector<DataRepresentationId_t> m_value;
    FASTDDS_EXPORTED_API DataRepresentationQosPolicy()
    {
    }

    virtual FASTDDS_EXPORTED_API ~DataRepresentationQosPolicy()
    {
    }

    inline void clear() override
    {
        DataRepresentationQosPolicy reset = DataRepresentationQosPolicy();
        std::swap(*this, reset);
    }

};

enum TypeConsistencyKind : uint32_t
{
    DISALLOW_TYPE_COERCION,
    ALLOW_TYPE_COERCION
};

/**
 * Class DataRepresentationQosPolicy,
 */
class TypeConsistencyEnforcementQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    TypeConsistencyKind m_kind;
    bool m_ignore_sequence_bounds;
    bool m_ignore_string_bounds;
    bool m_ignore_member_names;
    bool m_prevent_type_widening;
    bool m_force_type_validation;

    FASTDDS_EXPORTED_API TypeConsistencyEnforcementQosPolicy()
    {
        m_kind = ALLOW_TYPE_COERCION;
        m_ignore_sequence_bounds = true;
        m_ignore_string_bounds = true;
        m_ignore_member_names = false;
        m_prevent_type_widening = false;
        m_force_type_validation = false;
    }

    virtual FASTDDS_EXPORTED_API ~TypeConsistencyEnforcementQosPolicy()
    {
    }

    inline void clear() override
    {
        TypeConsistencyEnforcementQosPolicy reset = TypeConsistencyEnforcementQosPolicy();
        std::swap(*this, reset);
    }

};

/**
 * Class DisablePositiveACKsQosPolicy to disable sending of positive ACKs
 * period: Default value dds::c_TimeInfinite.
 */
class DisablePositiveACKsQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API DisablePositiveACKsQosPolicy()
    {
    }

    virtual FASTDDS_EXPORTED_API ~DisablePositiveACKsQosPolicy()
    {
    }

    bool operator ==(
            const DisablePositiveACKsQosPolicy& b) const
    {
        return enabled == b.enabled;
    }

    inline void clear() override
    {
        DisablePositiveACKsQosPolicy reset = DisablePositiveACKsQosPolicy();
        std::swap(*this, reset);
    }

public:

    //! True if this QoS is enabled
    bool enabled = false;
};

//! Qos Policy to configure the endpoint
class RTPSEndpointQos
{
public:

    FASTDDS_EXPORTED_API RTPSEndpointQos() = default;

    virtual FASTDDS_EXPORTED_API ~RTPSEndpointQos() = default;

    bool operator ==(
            const RTPSEndpointQos& b) const
    {
        return (this->unicast_locator_list == b.unicast_locator_list) &&
               (this->multicast_locator_list == b.multicast_locator_list) &&
               (this->remote_locator_list == b.remote_locator_list) &&
               (this->external_unicast_locators == b.external_unicast_locators) &&
               (this->ignore_non_matching_locators == b.ignore_non_matching_locators) &&
               (this->user_defined_id == b.user_defined_id) &&
               (this->entity_id == b.entity_id) &&
               (this->history_memory_policy == b.history_memory_policy);
    }

    //! Unicast locator list
    rtps::LocatorList unicast_locator_list;

    //! Multicast locator list
    rtps::LocatorList multicast_locator_list;

    //! Remote locator list
    rtps::LocatorList remote_locator_list;

    //! The collection of external locators to use for communication.
    fastdds::rtps::ExternalLocators external_unicast_locators;

    //! Whether locators that don't match with the announced locators should be kept.
    bool ignore_non_matching_locators = false;

    //! User Defined ID, used for StaticEndpointDiscovery. <br> By default, -1.
    int16_t user_defined_id = -1;

    //! Entity ID, if the user wants to specify the EntityID of the endpoint. <br> By default, -1.
    int16_t entity_id = -1;

    //! Underlying History memory policy. <br> By default, PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
    fastdds::rtps::MemoryManagementPolicy_t history_memory_policy =
            fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
};

//!Qos Policy to configure the limit of the writer resources
class WriterResourceLimitsQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API WriterResourceLimitsQos()
        : matched_subscriber_allocation()
        , reader_filters_allocation(0, 32u, 1u)
    {
    }

    /**
     * @brief Destructor
     */
    virtual FASTDDS_EXPORTED_API ~WriterResourceLimitsQos() = default;

    bool operator ==(
            const WriterResourceLimitsQos& b) const
    {
        return (matched_subscriber_allocation == b.matched_subscriber_allocation) &&
               (reader_filters_allocation == b.reader_filters_allocation);
    }

    //!Matched subscribers allocation limits.
    fastdds::ResourceLimitedContainerConfig matched_subscriber_allocation;
    //!Reader filters allocation limits.
    fastdds::ResourceLimitedContainerConfig reader_filters_allocation;
};

enum DataSharingKind : uint16_t
{
    AUTO,
    ON,
    OFF
};

/**
 * Information to check data sharing compatibility.
 * Will only be sent through the wire if this endpoint is data sharing compatible.
 * @note Immutable Qos Policy
 */
class DataSharingQosPolicy : public Parameter_t, public QosPolicy
{
public:

    FASTDDS_EXPORTED_API DataSharingQosPolicy()
        : Parameter_t(PID_DATASHARING, 0)
        , QosPolicy(true)
    {
        domain_ids_.push_back(1);
    }

    virtual FASTDDS_EXPORTED_API ~DataSharingQosPolicy() = default;

    FASTDDS_EXPORTED_API DataSharingQosPolicy(
            const DataSharingQosPolicy& b) = default;

    FASTDDS_EXPORTED_API DataSharingQosPolicy& operator =(
            const DataSharingQosPolicy& b) = default;

    bool operator ==(
            const DataSharingQosPolicy& b) const
    {
        return kind_ == b.kind_ &&
               shm_directory_ == b.shm_directory_ &&
               domain_ids_ == b.domain_ids_;
    }

    inline void clear() override
    {
        DataSharingQosPolicy reset = DataSharingQosPolicy();
        std::swap(*this, reset);
    }

    const DataSharingKind& kind() const
    {
        return kind_;
    }

    const std::string& shm_directory() const
    {
        return shm_directory_;
    }

    const std::vector<uint64_t>& domain_ids() const
    {
        return domain_ids_;
    }

    void set_max_domains(
            uint32_t size)
    {
        domain_ids_.reserve(size);
        max_domains_ = size;
    }

    const uint32_t& max_domains() const
    {
        return max_domains_;
    }

    void automatic()
    {
        automatic(std::string(), std::vector<uint16_t> (1, 1));
    }

    void automatic(
            const std::vector<uint16_t>& domain_ids)
    {
        automatic(std::string(), domain_ids);
    }

    void automatic(
            const std::string& directory)
    {
        automatic(directory, std::vector<uint16_t> (1, 1));
    }

    void automatic(
            const std::string& directory,
            const std::vector<uint16_t>& domain_ids)
    {
        kind_ = AUTO;
        shm_directory_ = directory;
        domain_ids_.clear();
        for (auto id : domain_ids)
        {
            domain_ids_.push_back(id);
        }
    }

    void on(
            const std::string& directory)
    {
        on(directory, std::vector<uint16_t> (1, 1));
    }

    void on(
            const std::string& directory,
            const std::vector<uint16_t>& domain_ids)
    {
        kind_ = ON;
        shm_directory_ = directory;
        domain_ids_.clear();
        for (auto id : domain_ids)
        {
            domain_ids_.push_back(id);
        }
    }

    /**
     * @brief Configures the DataSharing in disabled mode
     */
    FASTDDS_EXPORTED_API void off()
    {
        kind_ = OFF;
        shm_directory_ = "directory";
        domain_ids_.clear();
    }

public:

    DataSharingKind kind_ = AUTO;
    std::string shm_directory_;
    std::vector<uint64_t> domain_ids_;
    uint32_t max_domains_ = 0;
};


/**
 * Class TypeIdV1,
 */
class TypeIdV1 : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API TypeIdV1()
        : Parameter_t(PID_TYPE_IDV1, 0)
        , QosPolicy(false)
    {
        //m_type_identifier->_d(EK_MINIMAL);
    }

    FASTDDS_EXPORTED_API TypeIdV1(
            const TypeIdV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
    {
    }

    FASTDDS_EXPORTED_API TypeIdV1(
            TypeIdV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
    {
    }

    FASTDDS_EXPORTED_API TypeIdV1& operator =(
            const TypeIdV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        return *this;
    }

    FASTDDS_EXPORTED_API TypeIdV1& operator =(
            TypeIdV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        return *this;
    }

    virtual FASTDDS_EXPORTED_API ~TypeIdV1()
    {
    }

    inline void clear() override
    {
        TypeIdV1 reset = TypeIdV1();
        std::swap(*this, reset);
    }

};

/**
 * Class TypeObjectV1,
 */
class TypeObjectV1 : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    FASTDDS_EXPORTED_API TypeObjectV1()
        : Parameter_t(PID_TYPE_OBJECTV1, 0)
        , QosPolicy(false)
    {
    }

    FASTDDS_EXPORTED_API TypeObjectV1(
            const TypeObjectV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
    {
    }

    FASTDDS_EXPORTED_API TypeObjectV1(
            TypeObjectV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
    {
    }

    FASTDDS_EXPORTED_API TypeObjectV1& operator =(
            const TypeObjectV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        return *this;
    }

    FASTDDS_EXPORTED_API TypeObjectV1& operator =(
            TypeObjectV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        send_always_ = type.send_always_;

        return *this;
    }

    virtual FASTDDS_EXPORTED_API ~TypeObjectV1()
    {
    }

    inline void clear() override
    {
        TypeObjectV1 reset = TypeObjectV1();
        std::swap(*this, reset);
    }

};

using PropertyPolicyQos = fastdds::rtps::PropertyPolicy;

namespace xtypes {
class TypeInformation;
} // namespace types

namespace xtypes {

class TypeInformationParameter : public Parameter_t, public QosPolicy
{
public:

    FASTDDS_EXPORTED_API TypeInformationParameter()
        : Parameter_t(PID_TYPE_INFORMATION, 0)
        , QosPolicy(false)
    {
    }

    FASTDDS_EXPORTED_API TypeInformationParameter(
            const TypeInformationParameter& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
    {
    }

    FASTDDS_EXPORTED_API TypeInformationParameter(
            TypeInformationParameter&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.send_always_)
    {
    }

    FASTDDS_EXPORTED_API TypeInformationParameter& operator =(
            const TypeInformationParameter&)
    {
        return *this;
    }

    FASTDDS_EXPORTED_API TypeInformationParameter& operator =(
            TypeInformationParameter&&)
    {
        return *this;
    }

    virtual FASTDDS_EXPORTED_API ~TypeInformationParameter() override
    {
    }

    inline void clear() override
    {
        TypeInformationParameter reset = TypeInformationParameter();
        std::swap(*this, reset);
    }

    FASTDDS_EXPORTED_API bool assigned() const
    {
        return true;
    }

    FASTDDS_EXPORTED_API TypeInformationParameter& operator =(
            const xtypes::TypeInformation&)
    {
        return *this;
    }

    FASTDDS_EXPORTED_API const xtypes::TypeInformation* get() const
    {
        return nullptr;
    }

};

} // namespace xtypes

//! Qos Policy that configures the wire protocol
class WireProtocolConfigQos : public QosPolicy
{

public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API WireProtocolConfigQos()
        : QosPolicy(false)
        , participant_id(-1)
    {
    }

    /**
     * @brief Destructor
     */
    virtual FASTDDS_EXPORTED_API ~WireProtocolConfigQos() = default;

    bool operator ==(
            const WireProtocolConfigQos& b) const
    {
        return (this->prefix == b.prefix) &&
               (this->participant_id == b.participant_id) &&
               (this->builtin == b.builtin) &&
               (this->port == b.port) &&
               (this->default_unicast_locator_list == b.default_unicast_locator_list) &&
               (this->default_multicast_locator_list == b.default_multicast_locator_list) &&
               (this->default_external_unicast_locators == b.default_external_unicast_locators) &&
               (this->ignore_non_matching_locators == b.ignore_non_matching_locators) &&
               send_always_ == b.send_always_ &&
               hasChanged == b.hasChanged;
    }

    inline void clear() override
    {
        WireProtocolConfigQos reset = WireProtocolConfigQos();
        std::swap(*this, reset);
    }

    //! Optionally allows user to define the GuidPrefix_t
    fastdds::rtps::GuidPrefix_t prefix;

    //! Participant ID <br> By default, -1.
    int32_t participant_id;

    //! Builtin parameters.
    fastdds::rtps::BuiltinAttributes builtin;

    //! Port Parameters
    fastdds::rtps::PortParameters port;

    /**
     * Default list of Unicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the case
     * that it was defined with NO UnicastLocators. At least ONE locator should be included in this list.
     */
    rtps::LocatorList default_unicast_locator_list;

    /**
     * Default list of Multicast Locators to be used for any Endpoint defined inside this RTPSParticipant in the
     * case that it was defined with NO MulticastLocators. This is usually left empty.
     */
    rtps::LocatorList default_multicast_locator_list;

    /**
     * The collection of external locators to use for communication on user created topics.
     */
    rtps::ExternalLocators default_external_unicast_locators;

    /**
     * Whether locators that don't match with the announced locators should be kept.
     */
    bool ignore_non_matching_locators = false;
};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_POLICY__QOSPOLICIES_HPP
