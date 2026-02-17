// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file QosPoliciesSerializer.hpp
 *
 */

#ifndef FASTDDS_CORE_PLICY__QOSPOLICIESSERIALIZER_HPP_
#define FASTDDS_CORE_PLICY__QOSPOLICIESSERIALIZER_HPP_

#include "ParameterSerializer.hpp"
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/policy/ReaderDataLifecycleQosPolicy.hpp>
#include <fastdds/dds/core/policy/ReaderResourceLimitsQos.hpp>
#include <fastdds/dds/core/policy/RTPSReliableReaderQos.hpp>
#include <fastdds/dds/core/policy/RTPSReliableWriterQos.hpp>
#include <fastdds/dds/core/policy/WriterDataLifecycleQosPolicy.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

template <typename QosPolicy>
class QosPoliciesSerializer
{
public:

    static bool add_to_cdr_message(
            const QosPolicy& qos_policy,
            rtps::CDRMessage_t* cdr_message)
    {
        bool valid = ParameterSerializer<QosPolicy>::add_common_to_cdr_message(qos_policy, cdr_message);
        valid &= add_content_to_cdr_message(qos_policy, cdr_message);
        return valid;
    }

    /**
     * @brief Fill a QosPolicy with the data found in a CDR message.
     * @param vendor_id VendorId to check if specific Fast DDS fields need to be read.
     * @param qos_policy QosPolicy to be filled.
     * @param cdr_message CDRMessage to read from.
     * @param parameter_length Length of the QosPolicy in the CDRMessage.
     * @return true if the CDR message reading was successful.
     */
    static bool read_from_cdr_message(
            const fastdds::rtps::VendorId_t&,
            QosPolicy& qos_policy,
            rtps::CDRMessage_t* cdr_message,
            const uint16_t parameter_length)
    {
        return read_from_cdr_message(qos_policy, cdr_message, parameter_length);
    }

    /**
     * @brief Fill a QosPolicy with the data found in a CDR message.
     * @param qos_policy QosPolicy to be filled.
     * @param cdr_message CDRMessage to read from.
     * @param parameter_length Length of the QosPolicy in the CDRMessage.
     * @return true if the CDR message reading was successful.
     */
    static bool read_from_cdr_message(
            QosPolicy& qos_policy,
            rtps::CDRMessage_t* cdr_message,
            const uint16_t parameter_length)
    {
        bool valid = true;
        valid &= read_content_from_cdr_message(qos_policy, cdr_message, parameter_length);
        return valid;
    }

    static uint32_t cdr_serialized_size(
            const QosPolicy& qos_policy)
    {
        return ParameterSerializer<QosPolicy>::cdr_serialized_size(qos_policy);
    }

    /**
     * @brief Check if the QosPolicy should be sent by checking if it is default.
     * @param qos_policy QosPolicy to check
     * @return true if the QosPolicy should be sent, false otherwise.
     */
    static bool should_be_sent(
            const QosPolicy& qos_policy)
    {
        return !(qos_policy == QosPolicy());
    }

    /**
     * @brief Check if the QosPolicy should be sent. Default implementation checks if the QosPolicy is not default
     *          by calling the should_be_sent method.
     * @param qos_policy QosPolicy to check
     * @param is_writer  Flag to indicate if the QosPolicy is for a writer. This flag is only used in overwritten methods
     *                   of QosPolicies that have different default values for readers and writers.
     * @return true if the QosPolicy should be sent, false otherwise.
     */
    static bool should_be_sent(
            const QosPolicy& qos_policy,
            bool is_writer)
    {
        (void)is_writer;
        return should_be_sent(qos_policy);
    }

    /**
     * @brief Check if the QosPolicy should be sent. Default implementation checks if the QosPolicy is not default.
     * @param optional_qos_policy Optional QosPolicy to check.
     * @return true if the QosPolicy should be sent, false otherwise.
     */
    static bool should_be_sent(
            const fastcdr::optional<QosPolicy>& optional_qos_policy)
    {
        if (optional_qos_policy.has_value())
        {
            return should_be_sent(optional_qos_policy.value());
        }
        return false;
    }

private:

    static bool add_content_to_cdr_message(
            const QosPolicy&,
            rtps::CDRMessage_t*)
    {
        static_assert(sizeof(QosPolicy) == 0, "Not implemented");
        return false;
    }

    /**
     * @brief Read the content of a CDR message and write it to a QosPolicy.
     * @param qos_policy QosPolicy to be filled.
     * @param cdr_message CDRMessage to read from.
     * @param parameter_length Length of the QosPolicy in the CDRMessage.
     * @return true if the CDR message reading was successful.
     */
    static bool read_content_from_cdr_message(
            QosPolicy&,
            rtps::CDRMessage_t*,
            const uint16_t)
    {
        static_assert(sizeof(QosPolicy) == 0, "Not implemented");
        return false;
    }

    /**
     * @brief Read the content of a CDR message and write it to a QosPolicy.
     * @param vendor_id VendorId to check if specific Fast DDS fields need to be read.
     * @param qos_policy QosPolicy to be filled.
     * @param cdr_message CDRMessage to read from.
     * @param parameter_length Length of the QosPolicy in the CDRMessage.
     * @return true if the CDR message reading was successful.
     */
    static bool read_content_from_cdr_message(
            const fastdds::rtps::VendorId_t&,
            QosPolicy& qos_policy,
            rtps::CDRMessage_t* cdr_message,
            const uint16_t parameter_length)
    {
        return read_content_from_cdr_message(qos_policy, cdr_message, parameter_length);
    }

};

template<>
inline bool QosPoliciesSerializer<DurabilityQosPolicy>::should_be_sent(
        const DurabilityQosPolicy& /*qos_policy*/,
        bool /*is_writer*/)
{
    // DurabilityQosPolicy is always sent to ensure matching
    return true;
}

template<>
inline bool QosPoliciesSerializer<DurabilityQosPolicy>::add_content_to_cdr_message(
        const DurabilityQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DurabilityQosPolicy>::read_content_from_cdr_message(
        DurabilityQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KIND_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DeadlineQosPolicy>::add_content_to_cdr_message(
        const DeadlineQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.period.seconds);
    valid &= rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.period.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DeadlineQosPolicy>::read_content_from_cdr_message(
        DeadlineQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.period.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.period.fraction(frac);
    return valid;
}

template <>
inline bool QosPoliciesSerializer<LatencyBudgetQosPolicy>::add_content_to_cdr_message(
        const LatencyBudgetQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message, qos_policy.duration.seconds);
    valid &= rtps::CDRMessage::addUInt32(cdr_message, qos_policy.duration.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LatencyBudgetQosPolicy>::read_content_from_cdr_message(
        LatencyBudgetQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.duration.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.duration.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LivelinessQosPolicy>::add_content_to_cdr_message(
        const LivelinessQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.lease_duration.seconds);
    valid &= rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.lease_duration.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LivelinessQosPolicy>::read_content_from_cdr_message(
        LivelinessQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    valid &= rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.lease_duration.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.lease_duration.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ReliabilityQosPolicy>::should_be_sent(
        const ReliabilityQosPolicy& qos_policy,
        bool is_writer)
{
    if (is_writer)
    {
        // Writer is Reliable by default
        return (qos_policy.kind != RELIABLE_RELIABILITY_QOS) ||
               (qos_policy.max_blocking_time != Duration_t(0, 100000000));
    }
    // Reader is BestEffort by default
    return (qos_policy.kind != BEST_EFFORT_RELIABILITY_QOS);
}

template<>
inline bool QosPoliciesSerializer<ReliabilityQosPolicy>::add_content_to_cdr_message(
        const ReliabilityQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.max_blocking_time.seconds);
    valid &=
            rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.max_blocking_time.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ReliabilityQosPolicy>::read_content_from_cdr_message(
        ReliabilityQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    valid &= rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_blocking_time.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.max_blocking_time.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipQosPolicy>::add_content_to_cdr_message(
        const OwnershipQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipQosPolicy>::read_content_from_cdr_message(
        OwnershipQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KIND_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DestinationOrderQosPolicy>::add_content_to_cdr_message(
        const DestinationOrderQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DestinationOrderQosPolicy>::read_content_from_cdr_message(
        DestinationOrderQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KIND_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ResourceLimitsQosPolicy>::add_content_to_cdr_message(
        const ResourceLimitsQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.max_samples);
    valid &=
            rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_instances);
    valid &= rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.max_samples_per_instance);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.allocated_samples);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.extra_samples);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ResourceLimitsQosPolicy>::read_content_from_cdr_message(
        const fastdds::rtps::VendorId_t& vendor_id,
        ResourceLimitsQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 12)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_samples);
    valid &= rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_instances);
    valid &= rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_samples_per_instance);

    if (vendor_id == rtps::c_VendorId_eProsima)
    {
        // The following two fields are not mandatory according the DDS specification
        if (parameter_length >= 20)
        {
            valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.allocated_samples);
            valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.extra_samples);
        }
    }
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ResourceLimitsQosPolicy>::read_from_cdr_message(
        const fastdds::rtps::VendorId_t& vendor_id,
        ResourceLimitsQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    return read_content_from_cdr_message(vendor_id, qos_policy, cdr_message, parameter_length);
}

template<>
inline bool QosPoliciesSerializer<ResourceLimitsQosPolicy>::read_from_cdr_message(
        ResourceLimitsQosPolicy&,
        rtps::CDRMessage_t*,
        const uint16_t)
{
    EPROSIMA_LOG_ERROR(QOS_POLICIES_SERIALIZER,
            "ResourceLimitsQosPolicy requires 'vendor_id' to be read from cdr message");
    assert(false);
    return false;
}

template<>
inline bool QosPoliciesSerializer<TimeBasedFilterQosPolicy>::add_content_to_cdr_message(
        const TimeBasedFilterQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid =
            rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.minimum_separation.seconds);
    valid &=
            rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.minimum_separation.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TimeBasedFilterQosPolicy>::read_content_from_cdr_message(
        TimeBasedFilterQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.minimum_separation.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.minimum_separation.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<PresentationQosPolicy>::add_content_to_cdr_message(
        const PresentationQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message,
                    qos_policy.access_scope);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);

    valid &= rtps::CDRMessage::addOctet(cdr_message,
                    (fastdds::rtps::octet)qos_policy.coherent_access);
    valid &= rtps::CDRMessage::addOctet(cdr_message,
                    (fastdds::rtps::octet)qos_policy.ordered_access);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<PresentationQosPolicy>::read_content_from_cdr_message(
        PresentationQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_PRESENTATION_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.access_scope);
    cdr_message->pos += 3; //padding
    valid &= rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.coherent_access);
    valid &= rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.ordered_access);
    cdr_message->pos += 2; //padding
    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<PartitionQosPolicy>::cdr_serialized_size(
        const PartitionQosPolicy& qos_policy)
{
    // p_id + p_length + partition_number
    uint32_t ret_val = 2 + 2 + 4;
    for (PartitionQosPolicy::const_iterator it = qos_policy.begin(); it != qos_policy.end(); ++it)
    {
        // str_size
        ret_val += 4;
        // str_data (including null char)
        ret_val += static_cast<uint32_t>(it->size());
        // align
        ret_val = (ret_val + 3) & ~3;
    }

    return ret_val;
}

template<>
inline bool QosPoliciesSerializer<PartitionQosPolicy>::add_to_cdr_message(
        const PartitionQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);

    //Obtain qos_policy.length:
    uint16_t len = 4;
    for (PartitionQosPolicy::const_iterator it = qos_policy.begin(); it != qos_policy.end(); ++it)
    {
        len += 4;
        len += static_cast<uint16_t>(it->size()); //Already accounts for null char
        len = (len + 3) & ~3;
    }
    valid &= rtps::CDRMessage::addUInt16(cdr_message, len);

    valid &= rtps::CDRMessage::addUInt32(cdr_message,
                    static_cast<uint32_t>(qos_policy.size()));
    for (PartitionQosPolicy::const_iterator it = qos_policy.begin(); it != qos_policy.end(); ++it)
    {
        valid &= rtps::CDRMessage::add_string(cdr_message, it->name());
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<PartitionQosPolicy>::read_content_from_cdr_message(
        PartitionQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (qos_policy.max_size() != 0 && parameter_length > qos_policy.max_size() + 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;

    uint32_t pos_ref = cdr_message->pos;
    uint32_t num_partitions = 0;
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &num_partitions);
    //partitions_.reserve(parameter_length - 4);

    for (size_t i = 0; i < num_partitions; ++i)
    {
        uint32_t partition_size, alignment;

        valid &= rtps::CDRMessage::readUInt32(cdr_message, &partition_size);
        if (!valid)
        {
            return false;
        }

        qos_policy.push_back ((const char*)&cdr_message->buffer[cdr_message->pos]);
        alignment = ((partition_size + 3u) & ~3u) - partition_size;
        cdr_message->pos += (partition_size + alignment);
    }
    //qos_policy.Npartitions_ = num_partitions;

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<HistoryQosPolicy>::add_content_to_cdr_message(
        const HistoryQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.depth);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<HistoryQosPolicy>::read_content_from_cdr_message(
        HistoryQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KIND_LENGTH + 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message, (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.depth);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DurabilityServiceQosPolicy>::add_content_to_cdr_message(
        const DurabilityServiceQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.service_cleanup_delay.seconds);
    valid &= rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.service_cleanup_delay.fraction());
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.history_kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.history_depth);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_samples);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_instances);
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_samples_per_instance);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DurabilityServiceQosPolicy>::read_content_from_cdr_message(
        DurabilityServiceQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_TIME_LENGTH + PARAMETER_KIND_LENGTH + 16)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.service_cleanup_delay.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.service_cleanup_delay.fraction(frac);
    valid &= rtps::CDRMessage::readOctet(cdr_message, (fastdds::rtps::octet*)&qos_policy.history_kind);
    cdr_message->pos += 3; //padding
    valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.history_depth);
    valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.max_samples);
    valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.max_instances);
    valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.max_samples_per_instance);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LifespanQosPolicy>::add_content_to_cdr_message(
        const LifespanQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message, qos_policy.duration.seconds);
    valid &= rtps::CDRMessage::addUInt32(cdr_message, qos_policy.duration.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LifespanQosPolicy>::read_content_from_cdr_message(
        LifespanQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = rtps::CDRMessage::readInt32(cdr_message, &qos_policy.duration.seconds);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.duration.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipStrengthQosPolicy>::add_content_to_cdr_message(
        const OwnershipStrengthQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt32(cdr_message, qos_policy.value);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipStrengthQosPolicy>::read_content_from_cdr_message(
        OwnershipStrengthQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    return rtps::CDRMessage::readUInt32(cdr_message, &qos_policy.value);
}

template<>
inline bool QosPoliciesSerializer<TransportPriorityQosPolicy>::add_content_to_cdr_message(
        const TransportPriorityQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message, qos_policy.value);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TransportPriorityQosPolicy>::read_content_from_cdr_message(
        TransportPriorityQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    return rtps::CDRMessage::readInt32(cdr_message, &qos_policy.value);
}

template<>
inline uint32_t QosPoliciesSerializer<DataRepresentationQosPolicy>::cdr_serialized_size(
        const DataRepresentationQosPolicy& qos_policy)
{
    // Size of data
    uint32_t data_size = static_cast<uint32_t>(qos_policy.m_value.size() * sizeof(uint16_t));
    // Align to next 4 byte
    data_size = (data_size + 3) & ~3;
    // p_id + p_length + data_size + data
    return 2 + 2 + 4 + data_size;
}

template<>
inline bool QosPoliciesSerializer<DataRepresentationQosPolicy>::add_to_cdr_message(
        const DataRepresentationQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);

    uint16_t len = static_cast<uint16_t>(qos_policy.m_value.size() * sizeof(uint16_t)) + 4;
    len = (len + 3) & ~3;

    valid &= rtps::CDRMessage::addUInt16(cdr_message, len);
    valid &=
            rtps::CDRMessage::addUInt32(cdr_message,
                    static_cast<uint32_t>(qos_policy.m_value.size()));
    for (const DataRepresentationId_t& id : qos_policy.m_value)
    {
        valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(id));
    }
    if (qos_policy.m_value.size() % 2 == 1) // Odd, we must align
    {
        valid &= rtps::CDRMessage::addUInt16(cdr_message, uint16_t(0));
    }
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DataRepresentationQosPolicy>::read_content_from_cdr_message(
        DataRepresentationQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    qos_policy.length = parameter_length;

    int16_t temp(0);
    uint32_t datasize(0);
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &datasize);
    for (uint32_t i = 0; i < datasize; ++i)
    {
        valid &= rtps::CDRMessage::readInt16(cdr_message, &temp);
        qos_policy.m_value.push_back(static_cast<DataRepresentationId_t>(temp));
    }
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeConsistencyEnforcementQosPolicy>::add_content_to_cdr_message(
        const TypeConsistencyEnforcementQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.m_kind);
    valid &=
            rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastdds::rtps::octet>(qos_policy.m_ignore_sequence_bounds));
    valid &=
            rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastdds::rtps::octet>(qos_policy.m_ignore_string_bounds));
    valid &=
            rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastdds::rtps::octet>(qos_policy.m_ignore_member_names));
    valid &=
            rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastdds::rtps::octet>(qos_policy.m_prevent_type_widening));
    valid &=
            rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastdds::rtps::octet>(qos_policy.m_force_type_validation));
    valid &= rtps::CDRMessage::addOctet(cdr_message, rtps::octet(0x00)); // 8th byte
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeConsistencyEnforcementQosPolicy>::read_content_from_cdr_message(
        TypeConsistencyEnforcementQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 2)
    {
        return false;
    }

    uint16_t uKind(0);
    rtps::octet temp(0);
    qos_policy.m_ignore_sequence_bounds = false;
    qos_policy.m_ignore_string_bounds = false;
    qos_policy.m_ignore_member_names = false;
    qos_policy.m_prevent_type_widening = false;
    qos_policy.m_force_type_validation = false;

    bool valid = rtps::CDRMessage::readUInt16(cdr_message, &uKind);
    qos_policy.m_kind = static_cast<TypeConsistencyKind>(uKind);

    if (parameter_length >= 3)
    {
        valid &= rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_ignore_sequence_bounds = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 4)
    {
        valid &= rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_ignore_string_bounds = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 5)
    {
        valid &= rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_ignore_member_names = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 6)
    {
        valid &= rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_prevent_type_widening = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 7)
    {
        valid &= rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_force_type_validation = temp == 0 ? false : true;
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::add_content_to_cdr_message(
        const DisablePositiveACKsQosPolicy&,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x01);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::read_content_from_cdr_message(
        DisablePositiveACKsQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_BOOL_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    rtps::octet value(0);
    bool valid = rtps::CDRMessage::readOctet(cdr_message, &value);
    qos_policy.enabled = (value == 0) ? false : true;
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::should_be_sent(
        const DisablePositiveACKsQosPolicy& qos_policy)
{
    return !(qos_policy == DisablePositiveACKsQosPolicy()) && qos_policy.enabled;
}

template<>
inline uint32_t QosPoliciesSerializer<DataSharingQosPolicy>::cdr_serialized_size(
        const DataSharingQosPolicy& qos_policy)
{
    // Size of data (8 bytes each)
    uint32_t data_size = static_cast<uint32_t>(qos_policy.domain_ids().size()) * 8;
    // p_id + p_length + str_length + str_data
    return 2 + 2 + 4 + data_size;
}

template<>
inline bool QosPoliciesSerializer<DataSharingQosPolicy>::add_to_cdr_message(
        const DataSharingQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);

    //Obtain qos_policy.length: str_length + str_data

    uint16_t len = 4 + static_cast<uint16_t>(qos_policy.domain_ids().size()) * 8;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, len);

    //Add the values:
    valid &= rtps::CDRMessage::addUInt32(cdr_message,
                    static_cast<uint32_t>(qos_policy.domain_ids().size()));
    for (uint64_t id : qos_policy.domain_ids())
    {
        valid &= rtps::CDRMessage::addUInt64(cdr_message, id);
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<DataSharingQosPolicy>::read_content_from_cdr_message(
        DataSharingQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    qos_policy.length = parameter_length;
    uint32_t pos_ref = cdr_message->pos;

    // If the parameter is sent, the remote endpoint is datasharing compatible
    qos_policy.on(".");

    uint32_t num_domains = 0;
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &num_domains);

    if (!valid || (qos_policy.max_domains() != 0 && num_domains > qos_policy.max_domains()))
    {
        return false;
    }

    for (size_t i = 0; i < num_domains; ++i)
    {
        uint64_t domain {0};
        valid &= rtps::CDRMessage::readUInt64(cdr_message, &domain);
        qos_policy.add_domain_id(domain);
    }

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DataSharingQosPolicy>::should_be_sent(
        const DataSharingQosPolicy& qos_policy)
{
    return !(qos_policy == DataSharingQosPolicy()) && qos_policy.kind() != fastdds::dds::OFF;
}

template<>
inline uint32_t QosPoliciesSerializer<TypeIdV1>::cdr_serialized_size(
        const TypeIdV1& qos_policy)
{
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(qos_policy.m_type_identifier, current_alignment) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

template<>
inline bool QosPoliciesSerializer<TypeIdV1>::add_to_cdr_message(
        const TypeIdV1& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(qos_policy.m_type_identifier, current_alignment)
            + eprosima::fastdds::rtps::SerializedPayload_t::representation_header_size;
    rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    ser << qos_policy.m_type_identifier;
    payload.length = (uint32_t)ser.get_serialized_data_length(); //Get the serialized length
    size = (ser.get_serialized_data_length() + 3) & ~3;

    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(size));
    valid &= rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);

    for (uint32_t count = payload.length; count < size; ++count)
    {
        valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeIdV1>::read_content_from_cdr_message(
        TypeIdV1& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    rtps::SerializedPayload_t payload(parameter_length);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
            );

    try
    {
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        deser >> qos_policy.m_type_identifier;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        qos_policy.clear();
    }

    return true;
}

template<>
inline uint32_t QosPoliciesSerializer<TypeObjectV1>::cdr_serialized_size(
        const TypeObjectV1& qos_policy)
{
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(qos_policy.m_type_object, current_alignment) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

template<>
inline bool QosPoliciesSerializer<TypeObjectV1>::add_to_cdr_message(
        const TypeObjectV1& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(qos_policy.m_type_object, current_alignment)
            + eprosima::fastdds::rtps::SerializedPayload_t::representation_header_size;
    rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    ser << qos_policy.m_type_object;
    payload.length = (uint32_t)ser.get_serialized_data_length(); //Get the serialized length
    size = (ser.get_serialized_data_length() + 3) & ~3;

    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(size));
    valid &= rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);

    for (uint32_t count = payload.length; count < size; ++count)
    {
        valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeObjectV1>::read_content_from_cdr_message(
        TypeObjectV1& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    rtps::SerializedPayload_t payload(parameter_length);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
            );

    try
    {
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        deser >> qos_policy.m_type_object;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        qos_policy.clear();
    }

    return true;
}

template<>
inline uint32_t QosPoliciesSerializer<xtypes::TypeInformationParameter>::cdr_serialized_size(
        const xtypes::TypeInformationParameter& qos_policy)
{
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    size_t size = calculator.calculate_serialized_size(qos_policy.type_information, current_alignment) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

template<>
inline bool QosPoliciesSerializer<xtypes::TypeInformationParameter>::add_to_cdr_message(
        const xtypes::TypeInformationParameter& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    size_t size =
            calculator.calculate_serialized_size(qos_policy.type_information,
                    current_alignment) + eprosima::fastdds::rtps::SerializedPayload_t::representation_header_size;
    rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2);

    ser << qos_policy.type_information;
    payload.length = (uint32_t)ser.get_serialized_data_length(); //Get the serialized length
    size = (ser.get_serialized_data_length() + 3) & ~3;

    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(size));
    valid &= rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);

    for (uint32_t count = payload.length; count < size; ++count)
    {
        valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<xtypes::TypeInformationParameter>::read_content_from_cdr_message(
        xtypes::TypeInformationParameter& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    rtps::SerializedPayload_t payload(parameter_length);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);
    try
    {
        deser >> qos_policy.type_information;
        qos_policy.assigned(true);
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        qos_policy.assigned(false);
    }

    return true;
}

template<>
inline uint32_t QosPoliciesSerializer<GenericDataQosPolicy>::cdr_serialized_size(
        const GenericDataQosPolicy& qos_policy)
{
    // Size of data
    uint32_t data_size = static_cast<uint32_t>(qos_policy.size());
    // Align to next 4 byte
    data_size = (data_size + 3) & ~3;
    // p_id + p_length + str_length + str_data
    return 2 + 2 + 4 + data_size;
}

template<>
inline bool QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(
        const GenericDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    uint16_t siz = static_cast<uint16_t>(qos_policy.size());
    siz = (siz + 3) & ~3;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(4 + siz));
    valid &= rtps::CDRMessage::addOctetVector(cdr_message, &qos_policy.data_vec(), true);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<GenericDataQosPolicy>::read_content_from_cdr_message(
        GenericDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    uint32_t pos_ref = cdr_message->pos;

    // Read size of data
    uint32_t len;
    if (!fastdds::rtps::CDRMessage::readUInt32(cdr_message, &len))
    {
        return false;
    }

    if ((len + sizeof(uint32_t) > parameter_length)   // Exceeds parameter length
            || (len > qos_policy.max_size()))         // Exceeds size limit
    {
        return false;
    }

    // Either the data is size limited and already has max_size() allocated
    // or it is not limited and we resize if needed
    qos_policy.resize(len);
    if (!fastdds::rtps::CDRMessage::readData(cdr_message, qos_policy.data(), len))
    {
        return false;
    }

    // Skip padding
    cdr_message->pos += ((len + 3u) & ~3u) - len;

    // Should have consumed whole size

    bool ret_value =  (pos_ref + parameter_length == cdr_message->pos);

    if (ret_value)
    {
        qos_policy.length = parameter_length;
    }

    return ret_value;
}

template<>
inline uint32_t QosPoliciesSerializer<UserDataQosPolicy>::cdr_serialized_size(
        const UserDataQosPolicy& qos_policy)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::cdr_serialized_size(qos_policy);
}

template<>
inline bool QosPoliciesSerializer<UserDataQosPolicy>::add_to_cdr_message(
        const UserDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(qos_policy, cdr_message);
}

template<>
inline bool QosPoliciesSerializer<UserDataQosPolicy>::read_from_cdr_message(
        UserDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::read_from_cdr_message(qos_policy, cdr_message,
                   parameter_length);
}

template<>
inline uint32_t QosPoliciesSerializer<GroupDataQosPolicy>::cdr_serialized_size(
        const GroupDataQosPolicy& qos_policy)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::cdr_serialized_size(qos_policy);
}

template<>
inline bool QosPoliciesSerializer<GroupDataQosPolicy>::add_to_cdr_message(
        const GroupDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(qos_policy, cdr_message);
}

template<>
inline bool QosPoliciesSerializer<GroupDataQosPolicy>::read_from_cdr_message(
        GroupDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::read_from_cdr_message(qos_policy, cdr_message,
                   parameter_length);
}

template<>
inline uint32_t QosPoliciesSerializer<TopicDataQosPolicy>::cdr_serialized_size(
        const TopicDataQosPolicy& qos_policy)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::cdr_serialized_size(qos_policy);
}

template<>
inline bool QosPoliciesSerializer<TopicDataQosPolicy>::add_to_cdr_message(
        const TopicDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(qos_policy, cdr_message);
}

template<>
inline bool QosPoliciesSerializer<TopicDataQosPolicy>::read_from_cdr_message(
        TopicDataQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::read_from_cdr_message(qos_policy, cdr_message,
                   parameter_length);
}

template<>
inline uint32_t QosPoliciesSerializer<RTPSEndpointQos>::cdr_serialized_size(
        const RTPSEndpointQos& qos_policy)
{
    // p_id + p_length
    uint32_t ret_val = 2 + 2;

    // + unicast locator list size
    ret_val += 4;
    for (rtps::LocatorListConstIterator it = qos_policy.unicast_locator_list.begin();
            it != qos_policy.unicast_locator_list.end();
            ++it)
    {
        // kind + port + address
        ret_val += 4 + 4 + 16;
    }

    // + multicast locator list size
    ret_val += 4;
    for (rtps::LocatorListConstIterator it = qos_policy.multicast_locator_list.begin();
            it != qos_policy.multicast_locator_list.end();
            ++it)
    {
        // kind + port + address
        ret_val += 4 + 4 + 16;
    }

    // + remote locator list
    ret_val += 4;
    for (rtps::LocatorListConstIterator it = qos_policy.remote_locator_list.begin();
            it != qos_policy.remote_locator_list.end();
            ++it)
    {
        // kind + port + address
        ret_val += 4 + 4 + 16;
    }

    // Do not serialize external_locators yet, but leave a length = 0 field here
    ret_val += 4;

    // + ignore_non_matching_locators(4) + user_defined_id(4) + entity_id(4) + history_management(4)
    ret_val += 16;

    return ret_val;
}

template<>
inline bool QosPoliciesSerializer<RTPSEndpointQos>::add_to_cdr_message(
        const RTPSEndpointQos& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_RTPS_ENDPOINT);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(cdr_serialized_size(qos_policy) - 4));

    // Add content to cdr_message
    // Unicast locator list
    valid &= rtps::CDRMessage::addUInt32(cdr_message, (uint32_t)qos_policy.unicast_locator_list.size());
    for (rtps::LocatorListConstIterator it = qos_policy.unicast_locator_list.begin();
            it != qos_policy.unicast_locator_list.end();
            ++it)
    {
        valid &= rtps::CDRMessage::add_locator(cdr_message, *it);
    }

    // Multicast locator list
    valid &= rtps::CDRMessage::addUInt32(cdr_message, (uint32_t)qos_policy.multicast_locator_list.size());
    for (rtps::LocatorListConstIterator it = qos_policy.multicast_locator_list.begin();
            it != qos_policy.multicast_locator_list.end();
            ++it)
    {
        valid &= rtps::CDRMessage::add_locator(cdr_message, *it);
    }

    // Remote locator list
    valid &= rtps::CDRMessage::addUInt32(cdr_message, (uint32_t)qos_policy.remote_locator_list.size());
    for (rtps::LocatorListConstIterator it = qos_policy.remote_locator_list.begin();
            it != qos_policy.remote_locator_list.end();
            ++it)
    {
        valid &= rtps::CDRMessage::add_locator(cdr_message, *it);
    }

    // Do not serialize external_locators yet.
    valid &= rtps::CDRMessage::addUInt32(cdr_message, 0); // 0 length

    // ignore_non_matching_locators
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.ignore_non_matching_locators);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);

    // user_defined_id
    valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.user_defined_id);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 0); // padding

    // entity_id
    valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.entity_id);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 0); // padding

    // history_management
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.history_memory_policy);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<RTPSEndpointQos>::read_content_from_cdr_message(
        RTPSEndpointQos& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // empty locators lists(12) + 4 + 16
    if (parameter_length < 32)
    {
        return false;
    }

    uint32_t pos_ref = cdr_message->pos;

    // Unicast locator list
    uint32_t locators_size;
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &locators_size);
    qos_policy.unicast_locator_list.reserve(locators_size);
    for (uint32_t i = 0; i < locators_size; ++i)
    {
        rtps::Locator_t loc;
        valid &= rtps::CDRMessage::read_locator(cdr_message, &loc);
        qos_policy.unicast_locator_list.push_back(loc);
    }

    // Multicast locator list
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &locators_size);
    qos_policy.multicast_locator_list.reserve(locators_size);
    for (uint32_t i = 0; i < locators_size; ++i)
    {
        rtps::Locator_t loc;
        valid &= rtps::CDRMessage::read_locator(cdr_message, &loc);
        qos_policy.multicast_locator_list.push_back(loc);
    }

    // Remote locator list
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &locators_size);
    qos_policy.remote_locator_list.reserve(locators_size);
    for (uint32_t i = 0; i < locators_size; ++i)
    {
        rtps::Locator_t loc;
        valid &= rtps::CDRMessage::read_locator(cdr_message, &loc);
        qos_policy.remote_locator_list.push_back(loc);
    }

    // Do not deserialize external_locators yet.
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &locators_size); // 0 length

    // ignore_non_matching_locators
    valid &= rtps::CDRMessage::readOctet(cdr_message, (fastdds::rtps::octet*)&qos_policy.ignore_non_matching_locators);
    cdr_message->pos += 3; // padding

    // user_defined_id
    valid &= rtps::CDRMessage::readInt16(cdr_message, &qos_policy.user_defined_id);
    cdr_message->pos += 2; // padding

    // entity_id
    valid &= rtps::CDRMessage::readInt16(cdr_message, &qos_policy.entity_id);
    cdr_message->pos += 2; // padding

    // history_management
    valid &= rtps::CDRMessage::readInt32(cdr_message, (int*)&qos_policy.history_memory_policy);

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<WriterDataLifecycleQosPolicy>::cdr_serialized_size(
        const WriterDataLifecycleQosPolicy&)
{
    // p_id + p_length + (bool + padding)(4)
    return 2 + 2 + PARAMETER_BOOL_LENGTH;
}

template<>
inline bool QosPoliciesSerializer<WriterDataLifecycleQosPolicy>::add_to_cdr_message(
        const WriterDataLifecycleQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_WRITER_DATA_LIFECYCLE);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, PARAMETER_BOOL_LENGTH);

    // Add content to cdr_message
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.autodispose_unregistered_instances);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<WriterDataLifecycleQosPolicy>::read_content_from_cdr_message(
        WriterDataLifecycleQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // Fail if length is lower than required
    if (parameter_length < PARAMETER_BOOL_LENGTH)
    {
        return false;
    }

    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.autodispose_unregistered_instances);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<PublishModeQosPolicy>::cdr_serialized_size(
        const PublishModeQosPolicy& qos_policy)
{
    // p_id + p_length + kind(1) + padding(3)
    uint32_t ret_val = 2 + 2 + 1 + 3;
    // + str_size + str_data (including null char)
    ret_val += 4 + static_cast<uint32_t>(qos_policy.flow_controller_name.size() + 1);
    // align
    ret_val = (ret_val + 3) & ~3;

    return ret_val;
}

template<>
inline bool QosPoliciesSerializer<PublishModeQosPolicy>::add_to_cdr_message(
        const PublishModeQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_PUBLISH_MODE);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(cdr_serialized_size(qos_policy) - 4));

    // Add content to cdr_message
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0); // padding

    valid &= rtps::CDRMessage::add_string(cdr_message, qos_policy.flow_controller_name);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<PublishModeQosPolicy>::read_content_from_cdr_message(
        PublishModeQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // Fail if length is lower than kind + str_size + (null str + padding)
    if (parameter_length < (PARAMETER_KIND_LENGTH + 4 + 4))
    {
        return false;
    }

    uint32_t pos_ref = cdr_message->pos;

    bool valid = rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding

    rtps::CDRMessage::read_string(cdr_message, &qos_policy.flow_controller_name);

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<RTPSReliableWriterQos>::cdr_serialized_size(
        const RTPSReliableWriterQos&)
{
    // p_id + p_length + times(32)
    uint32_t ret_val = 2 + 2 + 32;

    // + disable_positive_acks(12)
    ret_val += 12;

    // + disable_heatbeat_piggyback(4)
    ret_val += 4;

    return ret_val;
}

template<>
inline bool QosPoliciesSerializer<RTPSReliableWriterQos>::add_to_cdr_message(
        const RTPSReliableWriterQos& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_RTPS_RELIABLE_WRITER);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 48); // 32 + 12 + 4

    // Add content to cdr_message
    // times
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.times.initial_heartbeat_delay);
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.times.heartbeat_period);
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.times.nack_response_delay);
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.times.nack_supression_duration);

    // disable_positive_acks
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.disable_positive_acks.enabled);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::add_duration_t(cdr_message, qos_policy.disable_positive_acks.duration);

    // disable_heatbeat_piggyback
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.disable_heartbeat_piggyback);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<RTPSReliableWriterQos>::read_content_from_cdr_message(
        RTPSReliableWriterQos& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // times(32) + disable_positive_acks(12) + disable_heatbeat_piggyback(4)
    if (parameter_length < 48)
    {
        return false;
    }

    bool valid = rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.times.initial_heartbeat_delay);
    valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.times.heartbeat_period);
    valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.times.nack_response_delay);
    valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.times.nack_supression_duration);

    valid &= rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*) &qos_policy.disable_positive_acks.enabled);
    cdr_message->pos += 3; //padding

    valid &= rtps::CDRMessage::read_duration_t(cdr_message, qos_policy.disable_positive_acks.duration);

    valid &= rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.disable_heartbeat_piggyback);
    cdr_message->pos += 3; //padding

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<WriterResourceLimitsQos>::cdr_serialized_size(
        const WriterResourceLimitsQos&)
{
    // p_id + p_length +  2*(uint64_t(8) + uint64_t(8) + uint64_t(8))
    return 2 + 2 + 48;
}

template<>
inline bool QosPoliciesSerializer<WriterResourceLimitsQos>::add_to_cdr_message(
        const WriterResourceLimitsQos& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_WRITER_RESOURCE_LIMITS);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 48);

    // Add content to cdr_message
    // matched_subscriber_allocation
    valid &= rtps::CDRMessage::add_resource_limited_container_config(cdr_message,
                    qos_policy.matched_subscriber_allocation);
    // reader_filters_allocation
    valid &= rtps::CDRMessage::add_resource_limited_container_config(cdr_message, qos_policy.reader_filters_allocation);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<WriterResourceLimitsQos>::read_content_from_cdr_message(
        WriterResourceLimitsQos& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 48)
    {
        return false;
    }

    // Initialize to 0 in case of size_t to be more than 32 bits
    qos_policy.matched_subscriber_allocation.maximum = 0;
    qos_policy.reader_filters_allocation.maximum = 0;

    bool valid = rtps::CDRMessage::read_resource_limited_container_config(cdr_message,
                    qos_policy.matched_subscriber_allocation);
    valid &= rtps::CDRMessage::read_resource_limited_container_config(cdr_message,
                    qos_policy.reader_filters_allocation);

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<ReaderDataLifecycleQosPolicy>::cdr_serialized_size(
        const ReaderDataLifecycleQosPolicy&)
{
    // p_id + p_length + 2*(uint32_t(4) + uint32_t(4))
    return 2 + 2 + 16;
}

template<>
inline bool QosPoliciesSerializer<ReaderDataLifecycleQosPolicy>::add_to_cdr_message(
        const ReaderDataLifecycleQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_READER_DATA_LIFECYCLE);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 16);

    // Add content to cdr_message
    // autopurge_no_writer_samples_delay
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.autopurge_no_writer_samples_delay);

    // autopurge_disposed_samples_delay
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.autopurge_disposed_samples_delay);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<ReaderDataLifecycleQosPolicy>::read_content_from_cdr_message(
        ReaderDataLifecycleQosPolicy& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // autopurge_no_writer_samples_delay + autopurge_disposed_samples_delay
    if (parameter_length < 8 + 8)
    {
        return false;
    }

    bool valid = rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.autopurge_no_writer_samples_delay);
    valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.autopurge_disposed_samples_delay);

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<RTPSReliableReaderQos>::cdr_serialized_size(
        const RTPSReliableReaderQos&)
{
    // p_id + p_length + reader_times(16)
    uint32_t ret_val = 2 + 2 + 16;

    // + disable_positive_acks(12)
    ret_val += 12;

    return ret_val;
}

template<>
inline bool QosPoliciesSerializer<RTPSReliableReaderQos>::add_to_cdr_message(
        const RTPSReliableReaderQos& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_RTPS_RELIABLE_READER);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 28); // 16 + 12

    // Add content to cdr_message
    // reader times
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.times.initial_acknack_delay);
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.times.heartbeat_response_delay);

    // disable_positive_acks
    valid &= rtps::CDRMessage::addOctet(cdr_message, qos_policy.disable_positive_acks.enabled);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00);
    valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                    qos_policy.disable_positive_acks.duration);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<RTPSReliableReaderQos>::read_content_from_cdr_message(
        RTPSReliableReaderQos& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // times(16) + disable_positive_acks(12)
    if (parameter_length < 28)
    {
        return false;
    }

    bool valid = rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.times.initial_acknack_delay);

    valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.times.heartbeat_response_delay);

    valid &= rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*) &qos_policy.disable_positive_acks.enabled);
    cdr_message->pos += 3; //padding

    valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                    qos_policy.disable_positive_acks.duration);

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<ReaderResourceLimitsQos>::cdr_serialized_size(
        const ReaderResourceLimitsQos&)
{
    // p_id + p_length +  3*(uint64_t(8) + uint64_t(8) + uint64_t(8)) + max_samples_per_read(4)
    return 2 + 2 + 72 + 4;
}

template<>
inline bool QosPoliciesSerializer<ReaderResourceLimitsQos>::add_to_cdr_message(
        const ReaderResourceLimitsQos& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_READER_RESOURCE_LIMITS);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 76); // 72 + 4

    // Add content to cdr_message
    // matched_publisher_allocation
    valid &= rtps::CDRMessage::add_resource_limited_container_config(cdr_message,
                    qos_policy.matched_publisher_allocation);

    // sample_infos_allocation
    valid &= rtps::CDRMessage::add_resource_limited_container_config(cdr_message, qos_policy.sample_infos_allocation);

    // outstanding_reads_allocation
    valid &= rtps::CDRMessage::add_resource_limited_container_config(cdr_message,
                    qos_policy.outstanding_reads_allocation);

    // max_samples_per_read
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_samples_per_read);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<ReaderResourceLimitsQos>::read_content_from_cdr_message(
        ReaderResourceLimitsQos& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 76)
    {
        return false;
    }

    // Initialize to 0 in case of size_t to be more than 32 bits
    qos_policy.matched_publisher_allocation.maximum = 0;
    qos_policy.sample_infos_allocation.maximum = 0;
    qos_policy.outstanding_reads_allocation.maximum = 0;

    bool valid = rtps::CDRMessage::read_resource_limited_container_config(cdr_message,
                    qos_policy.matched_publisher_allocation);

    valid &= rtps::CDRMessage::read_resource_limited_container_config(cdr_message,
                    qos_policy.sample_infos_allocation);

    valid &= rtps::CDRMessage::read_resource_limited_container_config(cdr_message,
                    qos_policy.outstanding_reads_allocation);

    valid &= rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_samples_per_read);

    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<WireProtocolConfigQos>::cdr_serialized_size(
        const WireProtocolConfigQos& qos_policy)
{
    // p_id + p_length
    uint32_t ret_val = 2 + 2;
    // + prefix(12) + participant_id(4)
    ret_val += 12 + 4;
    // + builtin
    {
        // discovery config
        {
            // up to m_discovery_servers
            ret_val += 44;
            // m_discovery_servers
            ret_val += 4;
            // size * (kind + port + address)
            ret_val +=  static_cast<uint32_t>(qos_policy.builtin.discovery_config.m_DiscoveryServers.size()) *
                    (4 + 4 + 16);
            // ignore_participant_flags
            ret_val += 4;
            // static_edp_xml_config
            ret_val += 4 +
                    static_cast<uint32_t>(strlen(qos_policy.builtin.discovery_config.static_edp_xml_config()) +
                    1);
            // align
            ret_val = (ret_val + 3) & ~3;
        }

        // up to metatraffic_unicast_locator_list
        ret_val += 8;
        // metatraffic_unicast_locator_list
        ret_val += 4;
        ret_val += static_cast<uint32_t>(qos_policy.builtin.metatrafficUnicastLocatorList.size()) * (4 + 4 + 16); // kind + port + address
        // metatraffic_multicast_locator_list
        ret_val += 4;
        ret_val += static_cast<uint32_t>(qos_policy.builtin.metatrafficMulticastLocatorList.size()) * (4 + 4 + 16); // kind + port + address
        // metatraffic_external_unicast_locators
        ret_val += 4;
        for (const auto& externality__cost_locator_list : qos_policy.builtin.metatraffic_external_unicast_locators)
        {
            for (const auto& cost__locator_list : externality__cost_locator_list.second)
            {
                // size * (kind + port + address + (externality + cost + mask + padding))
                ret_val += static_cast<uint32_t>(cost__locator_list.second.size()) * (4 + 4 + 16 + 4);
            }
        }
        // initial_peers_list
        ret_val += 4;
        ret_val += static_cast<uint32_t>(qos_policy.builtin.initialPeersList.size()) * (4 + 4 + 16); // kind + port + address
        // up to flow_controller_name
        ret_val += 16;
        // flow_controller_name (str_size + str_data (including null char))
        ret_val += 4 + static_cast<uint32_t>(qos_policy.builtin.flow_controller_name.size() + 1);
        // align
        ret_val = (ret_val + 3) & ~3;
    }
    // port
    ret_val += 16;
    // default_unicast_locator_list
    ret_val += 4;
    ret_val += static_cast<uint32_t>(qos_policy.default_unicast_locator_list.size()) * (4 + 4 + 16); // kind + port + address
    // default_multicast_locator_list
    ret_val += 4;
    ret_val += static_cast<uint32_t>(qos_policy.default_multicast_locator_list.size()) * (4 + 4 + 16); // kind + port + address
    // default_external_unicast_locators
    ret_val += 4;
    for (const auto& externality__cost_locator_list : qos_policy.default_external_unicast_locators)
    {
        for (const auto& cost__locator_list : externality__cost_locator_list.second)
        {
            // size * (kind + port + address + (externality + cost + mask + padding))
            ret_val += static_cast<uint32_t>(cost__locator_list.second.size()) * (4 + 4 + 16 + 4);
        }
    }
    // + ignore_non_matching_locators
    ret_val += 4;
    // easy_mode (str_size + str_data (including null char))
    ret_val += 4 + static_cast<uint32_t>(qos_policy.easy_mode().size() + 1);
    // align
    ret_val = (ret_val + 3) & ~3;

    return ret_val;
}

template<>
inline bool QosPoliciesSerializer<WireProtocolConfigQos>::add_to_cdr_message(
        const WireProtocolConfigQos& qos_policy,
        rtps::CDRMessage_t* cdr_message)
{
    // Add common to cdr_message
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, PID_WIREPROTOCOL_CONFIG);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(cdr_serialized_size(qos_policy) - 4));

    // Add content to cdr_message
    // prefix
    valid &= rtps::CDRMessage::addData(cdr_message, qos_policy.prefix.value, 12);
    // participant_id
    valid &= rtps::CDRMessage::addInt32(cdr_message, qos_policy.participant_id);
    //builtin
    {
        // discovery_config
        {
            // discovery_protocol
            valid &=
                    rtps::CDRMessage::addOctet(cdr_message,
                            static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.discoveryProtocol));
            // use_simple_edp
            valid &=
                    rtps::CDRMessage::addOctet(cdr_message,
                            static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.
                                    use_SIMPLE_EndpointDiscoveryProtocol));
            // use_static_edp
            valid &=
                    rtps::CDRMessage::addOctet(cdr_message,
                            static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.
                                    use_STATIC_EndpointDiscoveryProtocol));
            valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); //padding
            // lease_duration
            valid &= rtps::CDRMessage::add_duration_t(cdr_message, qos_policy.builtin.discovery_config.leaseDuration);
            // lease_duration_announcement_period
            valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                            qos_policy.builtin.discovery_config.leaseDuration_announcementperiod);
            //initial_announcements_config
            {
                // count
                valid &= rtps::CDRMessage::addUInt32(cdr_message,
                                qos_policy.builtin.discovery_config.initial_announcements.count);
                // period
                valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                                qos_policy.builtin.discovery_config.initial_announcements.period);
            }
            // m_simple_edp
            {
                // use_PublicationWriterANDSubscriptionReader
                valid &=
                        rtps::CDRMessage::addOctet(cdr_message,
                                static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.m_simpleEDP.
                                        use_PublicationWriterANDSubscriptionReader));
                // use_PublicationReaderANDSubscriptionWriter
                valid &=
                        rtps::CDRMessage::addOctet(cdr_message,
                                static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.m_simpleEDP.
                                        use_PublicationReaderANDSubscriptionWriter));
#if HAVE_SECURITY
                // enable_builtin_secure_publications_writer_and_subscriptions_reader
                valid &=
                        rtps::CDRMessage::addOctet(cdr_message,
                                static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.m_simpleEDP.
                                        enable_builtin_secure_publications_writer_and_subscriptions_reader));
                // enable_builtin_secure_subscriptions_writer_and_publications_reader
                valid &=
                        rtps::CDRMessage::addOctet(cdr_message,
                                static_cast<fastdds::rtps::octet>(qos_policy.builtin.discovery_config.m_simpleEDP.
                                        enable_builtin_secure_subscriptions_writer_and_publications_reader));
#else
                valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
                valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
#endif // if HAVE_SECURITY
            }
            // discoveryServer_client_syncperiod
            valid &= rtps::CDRMessage::add_duration_t(cdr_message,
                            qos_policy.builtin.discovery_config.discoveryServer_client_syncperiod);
            // m_discovery_servers
            valid &= rtps::CDRMessage::add_locator_list(cdr_message,
                            qos_policy.builtin.discovery_config.m_DiscoveryServers);
            // ignore_participant_flags
            valid &=
                    rtps::CDRMessage::addUInt32(cdr_message,
                            qos_policy.builtin.discovery_config.
                                    ignoreParticipantFlags);

            // static_edp_xml_config
            valid &= rtps::CDRMessage::add_string(cdr_message,
                            qos_policy.builtin.discovery_config.static_edp_xml_config());
        }
        // use_WLP
        valid &=
                rtps::CDRMessage::addOctet(cdr_message,
                        static_cast<fastdds::rtps::octet>(qos_policy.builtin.use_WriterLivelinessProtocol));
        valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
        valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
        valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
        // network_configuration
        valid &=
                rtps::CDRMessage::addUInt32(cdr_message,
                        qos_policy.builtin.network_configuration);
        // metatraffic_unicast_locator_list
        valid &= rtps::CDRMessage::add_locator_list(cdr_message, qos_policy.builtin.metatrafficUnicastLocatorList);
        // metatraffic_multicast_locator_list
        valid &= rtps::CDRMessage::add_locator_list(cdr_message, qos_policy.builtin.metatrafficMulticastLocatorList);
        // metatraffic_external_unicast_locators
        valid &= rtps::CDRMessage::add_external_locator_list(cdr_message,
                        qos_policy.builtin.metatraffic_external_unicast_locators);
        // initial_peers_list
        valid &= rtps::CDRMessage::add_locator_list(cdr_message, qos_policy.builtin.initialPeersList);
        // reader_history_memory_policy
        valid &=
                rtps::CDRMessage::addOctet(cdr_message,
                        static_cast<fastdds::rtps::octet>(qos_policy.builtin.readerHistoryMemoryPolicy));
        // writer_history_memory_policy
        valid &=
                rtps::CDRMessage::addOctet(cdr_message,
                        static_cast<fastdds::rtps::octet>(qos_policy.builtin.writerHistoryMemoryPolicy));
        // avoid_builtin_multicast
        valid &=
                rtps::CDRMessage::addOctet(cdr_message,
                        static_cast<fastdds::rtps::octet>(qos_policy.builtin.avoid_builtin_multicast));
        valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
        // reader_payload_size
        valid &=
                rtps::CDRMessage::addUInt32(cdr_message,
                        qos_policy.builtin.readerPayloadSize);
        // writer_payload_size
        valid &=
                rtps::CDRMessage::addUInt32(cdr_message,
                        qos_policy.builtin.writerPayloadSize);
        // mutation_tries
        valid &=
                rtps::CDRMessage::addUInt32(cdr_message,
                        qos_policy.builtin.mutation_tries);
        // flow_controller_name
        valid &= rtps::CDRMessage::add_string(cdr_message, qos_policy.builtin.flow_controller_name);
    }
    // port
    {
        // port base
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.portBase);
        // domain_id_gain
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.domainIDGain);
        // participant_id_gain
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.participantIDGain);
        // offset_d0
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.offsetd0);
        // offset_d1
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.offsetd1);
        // offset_d2
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.offsetd2);
        // offset_d3
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.offsetd3);
        // offset_d4
        valid &= rtps::CDRMessage::addUInt16(cdr_message, qos_policy.port.offsetd4);
    }
    // default_unicast_locator_list
    valid &= rtps::CDRMessage::add_locator_list(cdr_message, qos_policy.default_unicast_locator_list);
    // default_multicast_locator_list
    valid &= rtps::CDRMessage::add_locator_list(cdr_message, qos_policy.default_multicast_locator_list);
    // default_external_unicast_locators
    valid &= rtps::CDRMessage::add_external_locator_list(cdr_message,
                    qos_policy.default_external_unicast_locators);
    // ignore_non_matching_locators
    valid &=
            rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastdds::rtps::octet>(qos_policy.ignore_non_matching_locators));
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding
    valid &= rtps::CDRMessage::addOctet(cdr_message, (fastdds::rtps::octet)0x00); // padding

    // easy_mode
    valid &= rtps::CDRMessage::add_string(cdr_message, qos_policy.easy_mode());

    return valid;
}

template<>
inline bool QosPoliciesSerializer<WireProtocolConfigQos>::read_content_from_cdr_message(
        WireProtocolConfigQos& qos_policy,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    // Account for the mandatory fields
    // (0 sized collections and empty strings)
    if (parameter_length < 152)
    {
        return false;
    }

    uint32_t pos_ref = cdr_message->pos;

    // prefix
    bool valid =  rtps::CDRMessage::readData(cdr_message, qos_policy.prefix.value, 12);
    // participant_id
    valid &= rtps::CDRMessage::readInt32(cdr_message, &qos_policy.participant_id);
    // builtin
    {
        // discovery_config
        {
            // discovery_protocol
            valid &= rtps::CDRMessage::readOctet(cdr_message,
                            (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.discoveryProtocol);
            // use_simple_edp
            valid &= rtps::CDRMessage::readOctet(cdr_message,
                            (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol);
            // use_static_edp
            valid &= rtps::CDRMessage::readOctet(cdr_message,
                            (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol);
            cdr_message->pos += 1; //padding
            // lease_duration
            valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                            qos_policy.builtin.discovery_config.leaseDuration);
            // lease_duration_announcement_period
            valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                            qos_policy.builtin.discovery_config.leaseDuration_announcementperiod);
            //initial_announcements_config
            {
                // count
                valid &= rtps::CDRMessage::readUInt32(cdr_message,
                                &qos_policy.builtin.discovery_config.initial_announcements.count);
                // period
                valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                                qos_policy.builtin.discovery_config.initial_announcements.period);
            }
            // m_simple_edp
            {
                // use_PublicationWriterANDSubscriptionReader
                valid &= rtps::CDRMessage::readOctet(cdr_message,
                                (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader);
                // use_PublicationReaderANDSubscriptionWriter
                valid &= rtps::CDRMessage::readOctet(cdr_message,
                                (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter);
#if HAVE_SECURITY
                // enable_builtin_secure_publications_writer_and_subscriptions_reader
                valid &= rtps::CDRMessage::readOctet(cdr_message,
                                (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader);
                // enable_builtin_secure_subscriptions_writer_and_publications_reader
                valid &= rtps::CDRMessage::readOctet(cdr_message,
                                (fastdds::rtps::octet*)&qos_policy.builtin.discovery_config.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader);
#else
                cdr_message->pos += 2; // padding
#endif // if HAVE_SECURITY
            }
            // discoveryServer_client_syncperiod
            valid &= rtps::CDRMessage::read_duration_t(cdr_message,
                            qos_policy.builtin.discovery_config.discoveryServer_client_syncperiod);
            // m_discovery_servers
            valid &= rtps::CDRMessage::read_locator_list(cdr_message,
                            &qos_policy.builtin.discovery_config.m_DiscoveryServers);
            // ignore_participant_flags
            valid &= rtps::CDRMessage::readUInt32(cdr_message,
                            (uint32_t*)&qos_policy.builtin.discovery_config.ignoreParticipantFlags);
            // static_edp_xml_config
            std::string static_edp_xml_config;
            valid &= rtps::CDRMessage::read_string(cdr_message,
                            &static_edp_xml_config);
            qos_policy.builtin.discovery_config.static_edp_xml_config(static_edp_xml_config.c_str());
        }
        // use_WLP
        valid &= rtps::CDRMessage::readOctet(cdr_message,
                        (fastdds::rtps::octet*)&qos_policy.builtin.use_WriterLivelinessProtocol);
        cdr_message->pos += 3; // padding
        // network_configuration
        valid &= rtps::CDRMessage::readUInt32(cdr_message,
                        (uint32_t*)&qos_policy.builtin.network_configuration);
        // metatraffic_unicast_locator_list
        valid &= rtps::CDRMessage::read_locator_list(cdr_message, &qos_policy.builtin.metatrafficUnicastLocatorList);
        // metatraffic_multicast_locator_list
        valid &= rtps::CDRMessage::read_locator_list(cdr_message, &qos_policy.builtin.metatrafficMulticastLocatorList);
        // metatraffic_external_unicast_locators
        valid &= rtps::CDRMessage::read_external_locator_list(cdr_message,
                        &qos_policy.builtin.metatraffic_external_unicast_locators);
        // initial_peers_list
        valid &= rtps::CDRMessage::read_locator_list(cdr_message, &qos_policy.builtin.initialPeersList);
        // reader_history_memory_policy
        valid &= rtps::CDRMessage::readOctet(cdr_message,
                        (fastdds::rtps::octet*)&qos_policy.builtin.readerHistoryMemoryPolicy);
        // writer_history_memory_policy
        valid &= rtps::CDRMessage::readOctet(cdr_message,
                        (fastdds::rtps::octet*)&qos_policy.builtin.writerHistoryMemoryPolicy);
        // avoid_builtin_multicast
        valid &= rtps::CDRMessage::readOctet(cdr_message,
                        (fastdds::rtps::octet*)&qos_policy.builtin.avoid_builtin_multicast);
        cdr_message->pos += 1; // padding
        // reader_payload_size
        valid &= rtps::CDRMessage::readUInt32(cdr_message,
                        (uint32_t*)&qos_policy.builtin.readerPayloadSize);
        // writer_payload_size
        valid &= rtps::CDRMessage::readUInt32(cdr_message,
                        (uint32_t*)&qos_policy.builtin.writerPayloadSize);
        // mutation_tries
        valid &= rtps::CDRMessage::readUInt32(cdr_message,
                        (uint32_t*)&qos_policy.builtin.mutation_tries);
        // flow_controller_name
        valid &= rtps::CDRMessage::read_string(cdr_message,
                        &qos_policy.builtin.flow_controller_name);
    }
    // port
    {
        // port base
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.portBase);
        // domain_id_gain
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.domainIDGain);
        // participant_id_gain
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.participantIDGain);
        // offset_d0
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.offsetd0);
        // offset_d1
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.offsetd1);
        // offset_d2
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.offsetd2);
        // offset_d3
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.offsetd3);
        // offset_d4
        valid &= rtps::CDRMessage::readUInt16(cdr_message,
                        (uint16_t*)&qos_policy.port.offsetd4);
    }
    // default_unicast_locator_list
    valid &= rtps::CDRMessage::read_locator_list(cdr_message, &qos_policy.default_unicast_locator_list);
    // default_multicast_locator_list
    valid &= rtps::CDRMessage::read_locator_list(cdr_message, &qos_policy.default_multicast_locator_list);
    // default_external_unicast_locators
    valid &= rtps::CDRMessage::read_external_locator_list(cdr_message, &qos_policy.default_external_unicast_locators);
    // ignore_non_matching_locators
    valid &= rtps::CDRMessage::readOctet(cdr_message,
                    (fastdds::rtps::octet*)&qos_policy.ignore_non_matching_locators);
    cdr_message->pos += 3; // padding
    // easy_mode
    std::string easy_mode;
    valid &= rtps::CDRMessage::read_string(cdr_message,
                    &easy_mode);
    qos_policy.easy_mode(easy_mode);

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_CORE_PLICY__QOSPOLICIESSERIALIZER_HPP_
