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

#ifndef QOS_POLICIES_H_
#define QOS_POLICIES_H_

#include <vector>
#include <fastrtps/rtps/common/Types.h>
#include <fastrtps/rtps/common/Time_t.h>
#include "ParameterTypes.h"
#include <fastrtps/types/TypeObject.h>

namespace eprosima {
namespace fastrtps {

namespace rtps {
class EDP;
}

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

    bool operator ==(
            const QosPolicy& b) const
    {
        return (this->hasChanged == b.hasChanged) &&
               (this->m_sendAlways == b.m_sendAlways);
    }

    /**
     * Whether it should always be sent.
     * @return True if it should always be sent.
     */
    virtual bool sendAlways() const
    {
        return m_sendAlways;
    }

public:

    bool hasChanged;

protected:

    bool m_sendAlways;
};

/**
 * Enum DurabilityQosPolicyKind_t, different kinds of durability for DurabilityQosPolicy.
 */
typedef enum DurabilityQosPolicyKind : rtps::octet
{
    VOLATILE_DURABILITY_QOS,        //!< Volatile Durability (default for Subscribers).
    TRANSIENT_LOCAL_DURABILITY_QOS, //!< Transient Local Durability (default for Publishers).
    TRANSIENT_DURABILITY_QOS,       //!< Transient Durability.
    PERSISTENT_DURABILITY_QOS       //!< NOT IMPLEMENTED.
} DurabilityQosPolicyKind_t;

#define PARAMETER_KIND_LENGTH 4
#define PARAMETER_BOOL_LENGTH 4

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

    /**
     * Translates kind to rtps layer equivalent
     */
    inline rtps::DurabilityKind_t durabilityKind() const
    {
        switch (kind)
        {
            default:
            case VOLATILE_DURABILITY_QOS: return rtps::VOLATILE;
            case TRANSIENT_LOCAL_DURABILITY_QOS: return rtps::TRANSIENT_LOCAL;
            case TRANSIENT_DURABILITY_QOS: return rtps::TRANSIENT;
            case PERSISTENT_DURABILITY_QOS: return rtps::PERSISTENT;
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
     * Set kind from rtps layer equivalent
     */
    inline void durabilityKind(
            const rtps::DurabilityKind_t new_kind)
    {
        switch (new_kind)
        {
            default:
            case rtps::VOLATILE: kind = VOLATILE_DURABILITY_QOS; break;
            case rtps::TRANSIENT_LOCAL: kind = TRANSIENT_LOCAL_DURABILITY_QOS; break;
            case rtps::TRANSIENT: kind = TRANSIENT_DURABILITY_QOS; break;
            case rtps::PERSISTENT: kind = PERSISTENT_DURABILITY_QOS; break;
        }

    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    DurabilityQosPolicyKind_t kind;
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

    bool operator ==(
            const DeadlineQosPolicy& b) const
    {
        return (this->period == b.period) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    Duration_t period;
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

    bool operator ==(
            const LatencyBudgetQosPolicy& b) const
    {
        return (this->duration == b.duration) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    Duration_t duration;
};

/**
 * Enum LivelinessQosPolicyKind, different kinds of liveliness for LivelinessQosPolicy
 */
typedef enum LivelinessQosPolicyKind : rtps::octet
{
    AUTOMATIC_LIVELINESS_QOS,              //!< Automatic Liveliness, default value.
    MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,//!< MANUAL_BY_PARTICIPANT_LIVELINESS_QOS
    MANUAL_BY_TOPIC_LIVELINESS_QOS       //!< MANUAL_BY_TOPIC_LIVELINESS_QOS
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
        , lease_duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
        , announcement_period(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    virtual RTPS_DllAPI ~LivelinessQosPolicy()
    {
    }

    bool operator ==(
            const LivelinessQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               (this->lease_duration == b.lease_duration) &&
               (this->announcement_period == b.announcement_period) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    LivelinessQosPolicyKind kind;
    Duration_t lease_duration;
    Duration_t announcement_period;
};

/**
 * Enum ReliabilityQosPolicyKind, different kinds of reliability for ReliabilityQosPolicy.
 */
typedef enum ReliabilityQosPolicyKind : rtps::octet
{
    BEST_EFFORT_RELIABILITY_QOS = 0x01, //!< Best Effort reliability (default for Subscribers).
    RELIABLE_RELIABILITY_QOS = 0x02 //!< Reliable reliability (default for Publishers).
} ReliabilityQosPolicyKind;

/**
 * Indicates the reliability of the endpoint.
 */
class ReliabilityQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI ReliabilityQosPolicy()
        : Parameter_t(PID_RELIABILITY, PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
        , QosPolicy(true)
        ,                  //indicate send always
        kind(BEST_EFFORT_RELIABILITY_QOS)
        , // max_blocking_time = 100ms
        max_blocking_time{0, 100000000}
    {
    }

    virtual RTPS_DllAPI ~ReliabilityQosPolicy()
    {
    }

    bool operator ==(
            const ReliabilityQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               (this->max_blocking_time == b.max_blocking_time) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    /*!
     * @brief Defined the reliability kind of the endpoint.
     *
     * Default value BEST_EFFORT_RELIABILITY_QOS for ReaderQos and RELIABLE_RELIABILITY_QOS for WriterQos.
     */
    ReliabilityQosPolicyKind kind;

    /*!
     * @brief Defines the maximum period of time certain methods will be blocked.
     *
     * Methods affected by this property are:
     * - Publisher::write
     * - Subscriber::takeNextData
     * - Subscriber::readNextData
     */
    Duration_t max_blocking_time;
};



/**
 * Enum OwnershipQosPolicyKind, different kinds of ownership for OwnershipQosPolicy.
 */
enum OwnershipQosPolicyKind : rtps::octet
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

    bool operator ==(
            const OwnershipQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    OwnershipQosPolicyKind kind;
};

/**
 * Enum OwnershipQosPolicyKind, different kinds of destination order for DestinationOrderQosPolicy.
 */
enum DestinationOrderQosPolicyKind : rtps::octet
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

    RTPS_DllAPI DestinationOrderQosPolicy()
        : Parameter_t(PID_DESTINATION_ORDER, PARAMETER_KIND_LENGTH)
        , QosPolicy(true)
        , kind(BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS)
    {
    }

    virtual RTPS_DllAPI ~DestinationOrderQosPolicy()
    {
    }

    bool operator ==(
            const DestinationOrderQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    DestinationOrderQosPolicyKind kind;
};


/**
 * Class UserDataQosPolicy, to transmit user data during the discovery phase.
 */
class UserDataQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI UserDataQosPolicy()
        : Parameter_t(PID_USER_DATA, 0)
        , QosPolicy(false)
        , dataVec_{}
    {
    }

    RTPS_DllAPI UserDataQosPolicy(uint16_t in_length)
        : Parameter_t(PID_USER_DATA, in_length)
        , QosPolicy(false)
        , dataVec_{}
    {
    }


    RTPS_DllAPI UserDataQosPolicy(const UserDataQosPolicy& data)
        : Parameter_t(PID_USER_DATA, data.length)
        , QosPolicy(false)
        , max_size_(data.max_size_)
    {
            dataVec_.reserve(max_size_);
            dataVec_.assign(data.dataVec().begin(), data.dataVec().end());
    }

    virtual RTPS_DllAPI ~UserDataQosPolicy()
    {
    }

    bool operator ==(
            const UserDataQosPolicy& b) const
    {
        return (dataVec_ == b.dataVec()) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    UserDataQosPolicy& operator =(
            const UserDataQosPolicy& b)
    {
        max_size_ = b.max_size_;
        dataVec_.reserve(max_size_);
        dataVec_.assign(b.dataVec().begin(), b.dataVec().end());
        return *this;
    }

    /**
     * @return the maximum size of the user data
     */
    size_t max_size () const
    {
        return max_size_;
    }

    /**
     * Set the maximum size of the user data and reserves memory for that much.
     * @param size new maximum size of the user data
     */
    void max_size (size_t size)
    {
        max_size_ = size;
        dataVec_.reserve(max_size_);
    }

    /**
     * @return const reference to the internal raw data.
     * */
    inline const std::vector<rtps::octet>& dataVec() const
    {
        return dataVec_;
    }

    /**
     * clears the data.
     * */
    inline void clear()
    {
        dataVec_.clear();
    }


    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

    /**
     * Returns raw data vector.
     * @return raw data as vector of octets.
     * */
    RTPS_DllAPI inline std::vector<rtps::octet> getDataVec() const
    {
        return dataVec_;
    }

    /**
     * Sets raw data vector.
     * @param vec raw data to set.
     * */
    RTPS_DllAPI inline void setDataVec(
            const std::vector<rtps::octet>& vec)
    {
        if (max_size_ == 0 || vec.size() < max_size_)
        {
            dataVec_.assign(vec.begin(), vec.end());
        }
    }

private:
    std::vector<rtps::octet> dataVec_;
    size_t max_size_ = 0;
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

    bool operator ==(
            const TimeBasedFilterQosPolicy& b) const
    {
        return (this->minimum_separation == b.minimum_separation) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    Duration_t minimum_separation;
};

/**
 * Enum PresentationQosPolicyAccessScopeKind, different kinds of Presentation Policy order for PresentationQosPolicy.
 */
enum PresentationQosPolicyAccessScopeKind : rtps::octet
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

    bool operator ==(
            const PresentationQosPolicy& b) const
    {
        return (this->access_scope == b.access_scope) &&
               (this->coherent_access == b.coherent_access) &&
               (this->ordered_access == b.ordered_access) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    PresentationQosPolicyAccessScopeKind access_scope;
    bool coherent_access;
    bool ordered_access;
};



class Partition_t {

    friend class PartitionQosPolicy;


private:

    const char* partition_;

private:
    Partition_t()
    {
        partition_ = nullptr;
    }

public:
    explicit Partition_t(const void* ptr)
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

    uint32_t size() const
    {
        return *(uint32_t*)partition_;
    }

    const char* name() const
    {
        return partition_ + 4;
    }
};

/**
 * Class PartitionQosPolicy, to indicate the Partition Qos.
 */
class PartitionQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;
    friend class rtps::EDP;

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

            const_iterator(const rtps::octet* ptr)
            : ptr_(ptr)
            , value_ (ptr_)
            { }

            self_type operator++()
            {
                self_type tmp = *this;
                advance();
                return tmp;
            }

            self_type operator++(int)
            {
                advance();
                return *this;
            }

            reference operator*()
            {
                return value_;
            }

            pointer operator->()
            {
                return &value_;
            }

            bool operator==(const self_type& rhs)
            {
                return ptr_ == rhs.ptr_;
            }

            bool operator!=(const self_type& rhs)
            {
                return ptr_ != rhs.ptr_;
            }

        protected:

            void advance()
            {
                //Size of the element (with alignment)
                uint32_t size = *(uint32_t*)ptr_;
                ptr_ += (4 + ((size + 3) & ~3));
                value_ = Partition_t(ptr_);
            }

        private:
            const rtps::octet* ptr_;
            Partition_t value_;

    };

public:

    RTPS_DllAPI PartitionQosPolicy()
        : Parameter_t(PID_PARTITION, 0)
        , QosPolicy(false)
        , max_size_ (0)
        , Npartitions_ (0)
    {
    }

    RTPS_DllAPI PartitionQosPolicy(uint16_t in_length)
        : Parameter_t(PID_PARTITION, in_length)
        , QosPolicy(false)
        , max_size_ (in_length)
        , partitions_(in_length)
        , Npartitions_ (0)
    {
    }

    RTPS_DllAPI PartitionQosPolicy(const PartitionQosPolicy& b)
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


    virtual RTPS_DllAPI ~PartitionQosPolicy()
    {
    }

    bool operator ==(
            const PartitionQosPolicy& b) const
    {
        return (this->max_size_ == b.max_size_) &&
               (this->Npartitions_ == b.Npartitions_) &&
               (this->partitions_ == b.partitions_) &&
               Parameter_t::operator==(b) &&
               QosPolicy::operator==(b);
    }

    PartitionQosPolicy& operator =(
            const PartitionQosPolicy& b)
    {
        length = b.length;
        max_size_ = b.max_size_;
        partitions_.reserve(max_size_ != 0 ?
                            b.partitions_.max_size :
                            b.partitions_.length);
        partitions_.copy(&b.partitions_, b.max_size_ != 0);
        Npartitions_ = b.Npartitions_;
        hasChanged = true;

        return *this;
    }

    const_iterator begin() const
    {
        return const_iterator(partitions_.data);
    }

    const_iterator end() const
    {
        return const_iterator(partitions_.data + partitions_.length);
    }

    uint32_t size() const
    {
        return Npartitions_;
    }

    uint32_t empty() const
    {
        return Npartitions_ == 0;
    }

    void max_size (uint32_t size)
    {
        partitions_.reserve(size);
        max_size_ = size;
    }

    uint32_t max_size ()
    {
        return max_size_;
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

    /**
     * Appends a name to the list of partition names.
     * @param name Name to append.
     */
    RTPS_DllAPI inline void push_back(
            const char* name)
    {
        //Realloc if needed;
        uint32_t size = (uint32_t)strlen(name)+1;
        uint32_t alignment = ((size + 3) & ~3) - size;

        if (max_size_ != 0 && (partitions_.max_size < partitions_.length +
                size + alignment + 4))
        {
            return;
        }

        partitions_.reserve(partitions_.length +
                size + alignment + 4);

        rtps::octet* o = (rtps::octet*)&size;
        memcpy(partitions_.data + partitions_.length,
                o, 4);
        partitions_.length += 4;

        memcpy(partitions_.data + partitions_.length,
                name, size);
        partitions_.length += size;

        memset(partitions_.data + partitions_.length, 0, alignment);
        partitions_.length += alignment;

        ++Npartitions_;
        hasChanged = true;
    }

    /**
     * Clears list of partition names
     */
    RTPS_DllAPI inline void clear()
    {
        partitions_.length = 0;
        Npartitions_ = 0;
        hasChanged = true;
    }

    /**
     * Returns partition names.
     * @return Vector of partition name strings.
     */
    RTPS_DllAPI inline const std::vector<std::string> getNames() const
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
    RTPS_DllAPI inline void setNames(
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
    uint32_t max_size_;
    rtps::SerializedPayload_t partitions_;
    uint32_t Npartitions_;
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
        , value{}
    {
    }

    RTPS_DllAPI TopicDataQosPolicy(uint16_t in_length)
        : Parameter_t(PID_TOPIC_DATA, in_length)
        , QosPolicy(false)
        , value{}
    {
    }

    virtual RTPS_DllAPI ~TopicDataQosPolicy()
    {
    }

    bool operator ==(
            const TopicDataQosPolicy& b) const
    {
        return (this->value == b.value) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

    /**
     * Appends topic data.
     * @param oc Data octet.
     */
    RTPS_DllAPI inline void push_back(
            rtps::octet oc)
    {
        value.push_back(oc);
    }

    /**
     * Clears all topic data.
     */
    RTPS_DllAPI inline void clear()
    {
        value.clear();
    }

    /**
     * Overrides topic data vector.
     * @param ocv Topic data octet vector.
     */
    RTPS_DllAPI inline void setValue(
            std::vector<rtps::octet> ocv)
    {
        value = ocv;
    }

    /**
     * Returns topic data
     * @return Vector of data octets.
     */
    RTPS_DllAPI inline std::vector<rtps::octet> getValue() const
    {
        return value;
    }

private:

    std::vector<rtps::octet> value;
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
        , value{}
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

    bool operator ==(
            const GroupDataQosPolicy& b) const
    {
        return (this->value == b.value) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

    /**
     * Appends group data.
     * @param oc Data octet.
     */
    RTPS_DllAPI inline void push_back(
            rtps::octet oc)
    {
        value.push_back(oc);
    }

    /**
     * Clears all group data.
     */
    RTPS_DllAPI inline void clear()
    {
        value.clear();
    }

    /**
     * Overrides group data vector.
     * @param ocv Group data octet vector.
     */
    RTPS_DllAPI inline void setValue(
            std::vector<rtps::octet> ocv)
    {
        value = ocv;
    }

    /**
     * Returns group data
     * @return Vector of data octets.
     */
    RTPS_DllAPI inline std::vector<rtps::octet> getValue() const
    {
        return value;
    }

private:

    std::vector<rtps::octet> value;
};

/**
 * Enum HistoryQosPolicyKind, different kinds of History Qos for HistoryQosPolicy.
 */
enum HistoryQosPolicyKind : rtps::octet
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

    bool operator ==(
            const HistoryQosPolicy& b) const
    {
        return (this->kind == b.kind) &&
               (this->depth == b.depth) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    HistoryQosPolicyKind kind;
    int32_t depth;
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    Duration_t service_cleanup_delay;
    HistoryQosPolicyKind history_kind;
    int32_t history_depth;
    int32_t max_samples;
    int32_t max_instances;
    int32_t max_samples_per_instance;
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
        , duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    virtual RTPS_DllAPI ~LifespanQosPolicy()
    {
    }

    bool operator ==(
            const LifespanQosPolicy& b) const
    {
        return (this->duration == b.duration) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    Duration_t duration;
};

/**
 * Class OwnershipStrengthQosPolicy, to indicate the strength of the ownership.
 * value: Default value 0.
 */
class OwnershipStrengthQosPolicy : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    RTPS_DllAPI OwnershipStrengthQosPolicy()
        : Parameter_t(PID_OWNERSHIP_STRENGTH, 4)
        , QosPolicy(false)
        , value(0)
    {
    }

    virtual RTPS_DllAPI ~OwnershipStrengthQosPolicy()
    {
    }

    bool operator ==(
            const OwnershipStrengthQosPolicy& b) const
    {
        return (this->value == b.value) &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    uint32_t value;
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;
};

/**
 * Enum PublishModeQosPolicyKind, different kinds of publication synchronism
 */
typedef enum PublishModeQosPolicyKind : rtps::octet
{
    SYNCHRONOUS_PUBLISH_MODE,    //!< Synchronous publication mode (default for writers).
    ASYNCHRONOUS_PUBLISH_MODE    //!< Asynchronous publication mode.
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

};

/**
 * Enum DataRepresentationId, different kinds of topic data representation
 */
typedef enum DataRepresentationId : int16_t
{
    XCDR_DATA_REPRESENTATION,    //!<
    XML_DATA_REPRESENTATION,    //!<
    XCDR2_DATA_REPRESENTATION    //!<
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;
};

enum TypeConsistencyKind : uint16_t
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
        : Parameter_t(PID_TYPE_CONSISTENCY_ENFORCEMENT, 8)
        , QosPolicy(true)
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

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;
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
        : Parameter_t(PID_DISABLE_POSITIVE_ACKS, PARAMETER_BOOL_LENGTH)
        , QosPolicy(true)
        , enabled(false)
        , duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
    }

    virtual RTPS_DllAPI ~DisablePositiveACKsQosPolicy()
    {
    }

    bool operator ==(
            const DisablePositiveACKsQosPolicy& b) const
    {
        return enabled == b.enabled &&
               Parameter_t::operator ==(b) &&
               QosPolicy::operator ==(b);
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(rtps::CDRMessage_t* msg, uint32_t size) override;

public:

    //! True if this QoS is enabled
    bool enabled;
    //! The duration to keep samples for (not serialised as not needed by reader)
    Duration_t duration;
};

/**
 * Class TypeIdV1,
 */
class TypeIdV1 : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    types::TypeIdentifier m_type_identifier;

    RTPS_DllAPI TypeIdV1()
        : Parameter_t(PID_TYPE_IDV1, 0)
        , QosPolicy(false)
        , m_type_identifier()
    {
    }

    RTPS_DllAPI TypeIdV1(
            const TypeIdV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
        , m_type_identifier(type.m_type_identifier)
    {
    }

    RTPS_DllAPI TypeIdV1(
            TypeIdV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
        , m_type_identifier(std::move(type.m_type_identifier))
    {
    }

    RTPS_DllAPI TypeIdV1& operator =(
            const TypeIdV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        m_type_identifier = type.m_type_identifier;

        return *this;
    }

    RTPS_DllAPI TypeIdV1& operator =(
            TypeIdV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        m_type_identifier = std::move(type.m_type_identifier);

        return *this;
    }

    virtual RTPS_DllAPI ~TypeIdV1()
    {
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            rtps::CDRMessage_t* msg,
            uint32_t size) override;
};

/**
 * Class TypeObjectV1,
 */
class TypeObjectV1 : public Parameter_t, public QosPolicy
{
    friend class ParameterList;

public:

    types::TypeObject m_type_object;

    RTPS_DllAPI TypeObjectV1()
        : Parameter_t(PID_TYPE_OBJECTV1, 0)
        , QosPolicy(false)
        , m_type_object()
    {
    }

    RTPS_DllAPI TypeObjectV1(
            const TypeObjectV1& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
        , m_type_object(type.m_type_object)
    {
    }

    RTPS_DllAPI TypeObjectV1(
            TypeObjectV1&& type)
        : Parameter_t(type.Pid, type.length)
        , QosPolicy(type.m_sendAlways)
        , m_type_object(std::move(type.m_type_object))
    {
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            const TypeObjectV1& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        m_type_object = type.m_type_object;

        return *this;
    }

    RTPS_DllAPI TypeObjectV1& operator =(
            TypeObjectV1&& type)
    {
        Pid = type.Pid;
        length = type.length;
        m_sendAlways = type.m_sendAlways;

        m_type_object = std::move(type.m_type_object);

        return *this;
    }

    virtual RTPS_DllAPI ~TypeObjectV1()
    {
    }

    /**
     * Appends QoS to the specified CDR message.
     * @param msg Message to append the QoS Policy to.
     * @return True if the modified CDRMessage is valid.
     */
    bool addToCDRMessage(
            rtps::CDRMessage_t* msg) override;

    /**
     * Reads QoS from the specified CDR message
     * @param msg Message from where the QoS Policy has to be taken.
     * @param size Size of the QoS Policy field to read
     * @return True if the parameter was correctly taken.
     */
    bool readFromCDRMessage(
            rtps::CDRMessage_t* msg,
            uint32_t size) override;
};

}
}





#endif /* QOS_POLICIES_H_ */
