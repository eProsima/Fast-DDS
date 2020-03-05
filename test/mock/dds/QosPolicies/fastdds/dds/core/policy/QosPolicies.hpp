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
 * @file QosPolicies.h
 *
 */

#ifndef _FASTDDS_DDS_QOS_QOSPOLICIES_HPP_
#define _FASTDDS_DDS_QOS_QOSPOLICIES_HPP_

#include <vector>
#include <fastrtps/rtps/common/Types.h>
#include <fastrtps/rtps/common/Time_t.h>
#include <fastrtps/qos/ParameterTypes.h>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {

namespace fastrtps {
namespace rtps {
class EDP;
}
}

namespace fastdds {
namespace dds {

/**
 * Class QosPolicy, base for all QoS policies defined for Writers and Readers.
 */
class QosPolicy
{
public:

    QosPolicy()
        : hasChanged(false)
        , m_sendAlways(false)
    {
    }

    QosPolicy(
            bool b_sendAlways)
        : hasChanged(false)
        , m_sendAlways(b_sendAlways)
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
        return m_sendAlways;
    }

    virtual inline void clear() = 0;

    static uint32_t get_cdr_serialized_size(
            const std::vector<fastrtps::rtps::octet>& data)
    {
        // Size of data
        uint32_t data_size = static_cast<uint32_t>(data.size());
        // Align to next 4 byte
        data_size = (data_size + 3) & ~3;
        // p_id + p_length + str_length + str_data
        return 2 + 2 + 4 + data_size;
    }

protected:

    bool m_sendAlways;

};
/**
 * Enum DurabilityQosPolicyKind_t, different kinds of durability for DurabilityQosPolicy.
 */
typedef enum DurabilityQosPolicyKind : fastrtps::rtps::octet
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

    RTPS_DllAPI DurabilityQosPolicy()
        : Parameter_t(PID_DURABILITY, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(VOLATILE_DURABILITY_QOS)
    {
    }

    virtual RTPS_DllAPI ~DurabilityQosPolicy()
    {
    }

    DurabilityQosPolicyKind_t kind;

    /**
     * Translates kind to rtps layer equivalent
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

    /**
     * Set kind from rtps layer equivalent
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
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

    RTPS_DllAPI DeadlineQosPolicy()
        : Parameter_t(PID_DEADLINE, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , period(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    virtual RTPS_DllAPI ~DeadlineQosPolicy()
    {
    }

    inline void clear() override
    {
        DeadlineQosPolicy reset = DeadlineQosPolicy();
        std::swap(*this, reset);
    }

    fastrtps::Duration_t period;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Class LatencyBudgetQosPolicy, to indicate the LatencyBudget of the samples.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * period: Default value c_TimeZero.
 */
class LatencyBudgetQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI LatencyBudgetQosPolicy()
        : Parameter_t(PID_LATENCY_BUDGET, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , duration(0, 0)
    {
    }

    virtual RTPS_DllAPI ~LatencyBudgetQosPolicy()
    {
    }

    inline void clear() override
    {
        LatencyBudgetQosPolicy reset = LatencyBudgetQosPolicy();
        std::swap(*this, reset);
    }

    fastrtps::Duration_t duration;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Enum LivelinessQosPolicyKind, different kinds of liveliness for LivelinessQosPolicy
 */
typedef enum LivelinessQosPolicyKind : fastrtps::rtps::octet
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
 * lease_duration: Default value c_TimeInfinite.
 * announcement_period: Default value c_TimeInfinite (must be < lease_duration).
 */
class LivelinessQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI LivelinessQosPolicy()
        : Parameter_t(PID_LIVELINESS, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , kind(AUTOMATIC_LIVELINESS_QOS)
    {
        lease_duration = fastrtps::c_TimeInfinite;
        announcement_period = fastrtps::c_TimeInfinite;
    }

    virtual RTPS_DllAPI ~LivelinessQosPolicy()
    {
    }

    inline void clear() override
    {
        LivelinessQosPolicy reset = LivelinessQosPolicy();
        std::swap(*this, reset);
    }

    LivelinessQosPolicyKind kind;
    fastrtps::Duration_t lease_duration;
    fastrtps::Duration_t announcement_period;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Enum ReliabilityQosPolicyKind, different kinds of reliability for ReliabilityQosPolicy.
 */
typedef enum ReliabilityQosPolicyKind : fastrtps::rtps::octet
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

    RTPS_DllAPI ReliabilityQosPolicy()
        : Parameter_t(PID_RELIABILITY, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        ,                //indicate send always
        kind(BEST_EFFORT_RELIABILITY_QOS)
        ,// max_blocking_time = 100ms
        max_blocking_time{0, 100000000}
    {
    }

    virtual RTPS_DllAPI ~ReliabilityQosPolicy()
    {
    }

    inline void clear() override
    {
        ReliabilityQosPolicy reset = ReliabilityQosPolicy();
        std::swap(*this, reset);
    }

    ReliabilityQosPolicyKind kind;
    fastrtps::Duration_t max_blocking_time;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};



/**
 * Enum OwnershipQosPolicyKind, different kinds of ownership for OwnershipQosPolicy.
 */
enum OwnershipQosPolicyKind : fastrtps::rtps::octet
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

    RTPS_DllAPI OwnershipQosPolicy()
        : Parameter_t(PID_OWNERSHIP, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(SHARED_OWNERSHIP_QOS)
    {
    }

    virtual RTPS_DllAPI ~OwnershipQosPolicy()
    {
    }

    inline void clear() override
    {
        OwnershipQosPolicy reset = OwnershipQosPolicy();
        std::swap(*this, reset);
    }

    OwnershipQosPolicyKind kind;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Enum OwnershipQosPolicyKind, different kinds of destination order for DestinationOrderQosPolicy.
 */
enum DestinationOrderQosPolicyKind : fastrtps::rtps::octet
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
    RTPS_DllAPI DestinationOrderQosPolicy()
        : Parameter_t(PID_DESTINATION_ORDER, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS)
    {
    }

    virtual RTPS_DllAPI ~DestinationOrderQosPolicy()
    {
    }

    inline void clear() override
    {
        DestinationOrderQosPolicy reset = DestinationOrderQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};


/**
 * Class UserDataQosPolicy, to transmit user data during the discovery phase.
 */
class UserDataQosPolicy : public Parameter_t, public QosPolicy, public fastrtps::ResourceLimitedVector<fastrtps::rtps::octet>
{
    friend class ParameterList;
    using ResourceLimitedOctetVector = ResourceLimitedVector<fastrtps::rtps::octet>;
    using collection_type = std::vector<fastrtps::rtps::octet>;

public:

    RTPS_DllAPI UserDataQosPolicy()
        : Parameter_t(PID_USER_DATA, 0)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
    }

    RTPS_DllAPI UserDataQosPolicy(uint16_t in_length)
        : Parameter_t(PID_USER_DATA, in_length)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
    }

    RTPS_DllAPI UserDataQosPolicy(const UserDataQosPolicy& data)
        : Parameter_t(PID_USER_DATA, data.length)
        , QosPolicy(false)
        , ResourceLimitedOctetVector(data)
    {
    }

    RTPS_DllAPI UserDataQosPolicy(
            const collection_type& data)
        : Parameter_t(PID_USER_DATA, 0)
        , QosPolicy(false)
        , ResourceLimitedOctetVector()
    {
        assign(data.begin(), data.end());
    }

    virtual RTPS_DllAPI ~UserDataQosPolicy()
    {
    }

    UserDataQosPolicy& operator =(
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
      * Const cast to underlying collection.
      *
      * Useful to easy integration on old APIs where a traditional container was used.
      *
      * @return const reference to the underlying collection.
      */
     operator const collection_type& () const noexcept
     {
         return collection_;
     }

     inline void clear() override
     {
         ResourceLimitedOctetVector::clear();
     }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

    /**
     * Returns raw data vector.
     * @return raw data as vector of octets.
     * */
    RTPS_DllAPI inline std::vector<fastrtps::rtps::octet> data_vec() const
    {
        return collection_;
    }

    /**
     * Sets raw data vector.
     * @param vec raw data to set.
     * */
    RTPS_DllAPI inline void data_vec(
            const std::vector<fastrtps::rtps::octet>& vec)
    {
        collection_ = vec;
    }

    /**
     * Set the maximum size of the user data and reserves memory for that much.
     * @param size new maximum size of the user data
     */
    void set_max_size (size_t size)
    {
        configuration_.maximum = size;
    }

    /**
     * @return const reference to the internal raw data.
     * */
    inline const std::vector<fastrtps::rtps::octet>& dataVec() const
    {
        return collection_;
    }

};

/**
 * Class TimeBasedFilterQosPolicy, to indicate the Time Based Filter Qos.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * minimum_separation: Default value c_TimeZero
 */
class TimeBasedFilterQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI TimeBasedFilterQosPolicy()
        : Parameter_t(PID_TIME_BASED_FILTER, PARAMETER_TIME_LENGTH)
        , QosPolicy(false)
        , minimum_separation(0, 0)
    {
    }

    virtual RTPS_DllAPI ~TimeBasedFilterQosPolicy()
    {
    }

    inline void clear() override
    {
        TimeBasedFilterQosPolicy reset = TimeBasedFilterQosPolicy();
        std::swap(*this, reset);
    }

    fastrtps::Duration_t minimum_separation;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Enum PresentationQosPolicyAccessScopeKind, different kinds of Presentation Policy order for PresentationQosPolicy.
 */
enum PresentationQosPolicyAccessScopeKind : fastrtps::rtps::octet
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
    RTPS_DllAPI PresentationQosPolicy()
        : Parameter_t(PID_PRESENTATION, PARAMETER_PRESENTATION_LENGTH)
        , QosPolicy(false)
        , access_scope(INSTANCE_PRESENTATION_QOS)
        , coherent_access(false)
        , ordered_access(false)
    {
    }

    virtual RTPS_DllAPI ~PresentationQosPolicy()
    {
    }

    inline void clear() override
    {
        PresentationQosPolicy reset = PresentationQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};


/**
 * Class PartitionQosPolicy, to indicate the Partition Qos.
 */
class PartitionQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;
    friend class fastrtps::rtps::EDP;

public:

    RTPS_DllAPI PartitionQosPolicy()
        : Parameter_t(PID_PARTITION, 0)
        , QosPolicy(false)
    {
    }

    RTPS_DllAPI PartitionQosPolicy(uint16_t in_length)
        : Parameter_t(PID_PARTITION, in_length)
        , QosPolicy(false)
        , names_{}
    {
    }

    virtual RTPS_DllAPI ~PartitionQosPolicy()
    {
    }

    void set_max_size (uint32_t size)
    {
        max_size_ = size;
    }

    uint32_t max_size () const
    {
        return max_size_;
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

    /**
     * Appends a name to the list of partition names.
     * @param name Name to append.
     */
    RTPS_DllAPI inline void push_back(
            const char* name)
    {
        names_.push_back(std::string(name)); hasChanged = true;
    }

    /**
     * Clears list of partition names
     */
    RTPS_DllAPI inline void clear() override
    {
        names_.clear();
    }

    /**
     * Returns partition names.
     * @return Vector of partition name strings.
     */
    RTPS_DllAPI inline std::vector<std::string> names() const
    {
        return names_;
    }

    /**
     * Overrides partition names
     * @param nam Vector of partition name strings.
     */
    RTPS_DllAPI inline void names(
            std::vector<std::string>& nam)
    {
        names_ = nam; hasChanged = true;
    }

private:

    std::vector<std::string> names_;
    uint32_t max_size_ = 0;
};


/**
 * Class TopicDataQosPolicy, to indicate the Topic Data.
 */
class TopicDataQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI TopicDataQosPolicy()
        : Parameter_t(PID_TOPIC_DATA, 0)
        , QosPolicy(false)
    {
    }

    virtual RTPS_DllAPI ~TopicDataQosPolicy()
    {
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

    /**
     * Appends topic data.
     * @param oc Data octet.
     */
    RTPS_DllAPI inline void push_back(
            fastrtps::rtps::octet oc)
    {
        value.push_back(oc);
    }

    /**
     * Clears all topic data.
     */
    RTPS_DllAPI inline void clear() override
    {
        value.clear();
    }

    /**
     * Overrides topic data vector.
     * @param ocv Topic data octet vector.
     */
    RTPS_DllAPI inline void setValue(
            std::vector<fastrtps::rtps::octet> ocv)
    {
        value = ocv;
    }

    /**
     * Returns topic data
     * @return Vector of data octets.
     */
    RTPS_DllAPI inline std::vector<fastrtps::rtps::octet> getValue() const
    {
        return value;
    }

private:

    std::vector<fastrtps::rtps::octet> value;
};

/**
 * Class GroupDataQosPolicy, to indicate the Group Data.
 */
class GroupDataQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI GroupDataQosPolicy()
        : Parameter_t(PID_GROUP_DATA, 0)
        , QosPolicy(false)
    {
    }

    RTPS_DllAPI GroupDataQosPolicy(uint16_t in_length)
        : Parameter_t(PID_GROUP_DATA, in_length)
        , QosPolicy(false)
        , value{}
    {
    }

    virtual RTPS_DllAPI ~GroupDataQosPolicy()
    {
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

    /**
     * Appends group data.
     * @param oc Data octet.
     */
    RTPS_DllAPI inline void push_back(
            fastrtps::rtps::octet oc)
    {
        value.push_back(oc);
    }

    /**
     * Clears all group data.
     */
    RTPS_DllAPI inline void clear() override
    {
        value.clear();
    }

    /**
     * Overrides group data vector.
     * @param ocv Group data octet vector.
     */
    RTPS_DllAPI inline void setValue(
            std::vector<fastrtps::rtps::octet> ocv)
    {
        value = ocv;
    }

    /**
     * Returns group data
     * @return Vector of data octets.
     */
    RTPS_DllAPI inline std::vector<fastrtps::rtps::octet> getValue() const
    {
        return value;
    }

private:

    std::vector<fastrtps::rtps::octet> value;
};

/**
 * Enum HistoryQosPolicyKind, different kinds of History Qos for HistoryQosPolicy.
 */
enum HistoryQosPolicyKind : fastrtps::rtps::octet
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
    RTPS_DllAPI HistoryQosPolicy()
        : Parameter_t(PID_HISTORY, PARAMETER_KIND_LENGTH + 4)
        , QosPolicy(true)
        , kind(KEEP_LAST_HISTORY_QOS)
        , depth(1)
    {
    }

    virtual RTPS_DllAPI ~HistoryQosPolicy()
    {
    }

    inline void clear() override
    {
        HistoryQosPolicy reset = HistoryQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
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
    RTPS_DllAPI ResourceLimitsQosPolicy()
        : Parameter_t(PID_RESOURCE_LIMITS, 4 + 4 + 4)
        , QosPolicy(false)
        , max_samples(5000)
        , max_instances(10)
        , max_samples_per_instance(400)
        , allocated_samples(100)
    {
    }

    virtual RTPS_DllAPI ~ResourceLimitsQosPolicy()
    {
    }

    inline void clear() override
    {
        ResourceLimitsQosPolicy reset = ResourceLimitsQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};



/**
 * Class DurabilityServiceQosPolicy, to indicate the Durability Service.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * service_cleanup_delay: Default value c_TimeZero.
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

    fastrtps::Duration_t service_cleanup_delay;
    HistoryQosPolicyKind history_kind;
    int32_t history_depth;
    int32_t max_samples;
    int32_t max_instances;
    int32_t max_samples_per_instance;
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

    virtual RTPS_DllAPI ~DurabilityServiceQosPolicy()
    {
    }

    inline void clear() override
    {
        DurabilityServiceQosPolicy reset = DurabilityServiceQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Class LifespanQosPolicy, currently unimplemented.
 * This QosPolicy can be defined and is transmitted to the rest of the network but is not implemented in this version.
 * duration: Default value c_TimeInfinite.
 */
class LifespanQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI LifespanQosPolicy()
        : Parameter_t(PID_LIFESPAN, PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        , duration(fastrtps::c_TimeInfinite)
    {
    }

    virtual RTPS_DllAPI ~LifespanQosPolicy()
    {
    }

    inline void clear() override
    {
        LifespanQosPolicy reset = LifespanQosPolicy();
        std::swap(*this, reset);
    }

    fastrtps::Duration_t duration;
    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

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
    RTPS_DllAPI OwnershipStrengthQosPolicy()
        : Parameter_t(PID_OWNERSHIP_STRENGTH, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    virtual RTPS_DllAPI ~OwnershipStrengthQosPolicy()
    {
    }

    inline void clear() override
    {
        OwnershipStrengthQosPolicy reset = OwnershipStrengthQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
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
    RTPS_DllAPI TransportPriorityQosPolicy()
        : Parameter_t(PID_TRANSPORT_PRIORITY, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    virtual RTPS_DllAPI ~TransportPriorityQosPolicy()
    {
    }

    inline void clear() override
    {
        TransportPriorityQosPolicy reset = TransportPriorityQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Enum PublishModeQosPolicyKind, different kinds of publication synchronism
 */
typedef enum PublishModeQosPolicyKind : fastrtps::rtps::octet
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
    RTPS_DllAPI PublishModeQosPolicy()
        : kind(SYNCHRONOUS_PUBLISH_MODE)
    {
    }

    virtual RTPS_DllAPI ~PublishModeQosPolicy()
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

/**
 * Class DataRepresentationQosPolicy,
 */
class DataRepresentationQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    std::vector<DataRepresentationId_t> m_value;
    RTPS_DllAPI DataRepresentationQosPolicy()
    {
    }

    virtual RTPS_DllAPI ~DataRepresentationQosPolicy()
    {
    }

    inline void clear() override
    {
        DataRepresentationQosPolicy reset = DataRepresentationQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
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

    RTPS_DllAPI TypeConsistencyEnforcementQosPolicy()
    {
        m_kind = ALLOW_TYPE_COERCION;
        m_ignore_sequence_bounds = true;
        m_ignore_string_bounds = true;
        m_ignore_member_names = false;
        m_prevent_type_widening = false;
        m_force_type_validation = false;
    }

    virtual RTPS_DllAPI ~TypeConsistencyEnforcementQosPolicy()
    {
    }

    inline void clear() override
    {
        TypeConsistencyEnforcementQosPolicy reset = TypeConsistencyEnforcementQosPolicy();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Class DisablePositiveACKsQosPolicy to disable sending of positive ACKs
 * period: Default value c_TimeInfinite.
 */
class DisablePositiveACKsQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI DisablePositiveACKsQosPolicy()
    {
    }

    virtual RTPS_DllAPI ~DisablePositiveACKsQosPolicy()
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* msg) const override
    {
        (void)msg;
        return true;
    }

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/, uint16_t /*size*/) override
    {
        return true;
    }

public:

    //! True if this QoS is enabled
    bool enabled = false;
};

/**
 * Class TypeIdV1,
 */
class TypeIdV1 : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI TypeIdV1()
        : Parameter_t(PID_TYPE_IDV1, 0)
        , QosPolicy(false)
    {
        //m_type_identifier->_d(EK_MINIMAL);
    }

    RTPS_DllAPI TypeIdV1(
            const TypeIdV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
    {
    }

    RTPS_DllAPI TypeIdV1(
            TypeIdV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
    {
    }

    RTPS_DllAPI TypeIdV1& operator =(
            const TypeIdV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        return *this;
    }

    RTPS_DllAPI TypeIdV1& operator =(
            TypeIdV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        return *this;
    }

    virtual RTPS_DllAPI ~TypeIdV1()
    {
    }

    inline void clear() override
    {
        TypeIdV1 reset = TypeIdV1();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/,
            uint16_t /*size*/) override
    {
        return true;
    }

};

/**
 * Class TypeObjectV1,
 */
class TypeObjectV1 : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI TypeObjectV1()
        : Parameter_t(PID_TYPE_OBJECTV1, 0)
        , QosPolicy(false)
    {
    }

    RTPS_DllAPI TypeObjectV1(
            const TypeObjectV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
    {
    }

    RTPS_DllAPI TypeObjectV1(
            TypeObjectV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
    {
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            const TypeObjectV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        return *this;
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            TypeObjectV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        return *this;
    }

    virtual RTPS_DllAPI ~TypeObjectV1()
    {
    }

    inline void clear() override
    {
        TypeObjectV1 reset = TypeObjectV1();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/) const override
    {
        return true;
    }

    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t* /*msg*/,
            uint16_t /*size*/) override
    {
        return true;
    }

};

namespace types {
class TypeInformation;
}

namespace xtypes {

class TypeInformation : public Parameter_t, public QosPolicy
{
public:

    RTPS_DllAPI TypeInformation()
        : Parameter_t(PID_TYPE_INFORMATION, 0)
        , QosPolicy(false)
    {
    }

    RTPS_DllAPI TypeInformation(
            const TypeInformation& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
    {
    }

    RTPS_DllAPI TypeInformation(
            TypeInformation&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
    {
    }

    RTPS_DllAPI TypeInformation& operator =(
            const TypeInformation&)
    {
        return *this;
    }

    RTPS_DllAPI TypeInformation& operator =(
            TypeInformation&&)
    {
        return *this;
    }

    virtual RTPS_DllAPI ~TypeInformation() override
    {
    }

    inline void clear() override
    {
        TypeInformation reset = TypeInformation();
        std::swap(*this, reset);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            fastrtps::rtps::CDRMessage_t*) const override
    {
        return true;
    }

    bool readFromCDRMessage(
            fastrtps::rtps::CDRMessage_t*,
            uint16_t /*size*/) override
    {
        return true;
    }

    RTPS_DllAPI bool isAssigned() const
    {
        return true;
    }

    RTPS_DllAPI TypeInformation& operator =(
            const types::TypeInformation&)
    {
        return *this;
    }

    RTPS_DllAPI const types::TypeInformation* get() const
    {
        return nullptr;
    }

};

}

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_DDS_QOS_QOSPOLICIES_HPP_
