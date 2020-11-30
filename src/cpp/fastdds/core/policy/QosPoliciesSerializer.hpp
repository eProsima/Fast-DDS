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

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include "ParameterSerializer.hpp"
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

template <typename QosPolicy>
class QosPoliciesSerializer
{
public:

    static bool add_to_cdr_message(
            const QosPolicy& qos_policy,
            fastrtps::rtps::CDRMessage_t* cdr_message)
    {
        bool valid = ParameterSerializer<QosPolicy>::add_common_to_cdr_message(qos_policy, cdr_message);
        valid &= add_content_to_cdr_message(qos_policy, cdr_message);
        return valid;
    }

    static bool read_from_cdr_message(
            QosPolicy& qos_policy,
            fastrtps::rtps::CDRMessage_t* cdr_message,
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

private:

    static bool add_content_to_cdr_message(
            const QosPolicy&,
            fastrtps::rtps::CDRMessage_t*)
    {
        static_assert(sizeof(QosPolicy) == 0, "Not implemented");
        return false;
    }

    static bool read_content_from_cdr_message(
            QosPolicy&,
            fastrtps::rtps::CDRMessage_t*,
            const uint16_t)
    {
        static_assert(sizeof(QosPolicy) == 0, "Not implemented");
        return false;
    }

};

template<>
inline bool QosPoliciesSerializer<DurabilityQosPolicy>::add_content_to_cdr_message(
        const DurabilityQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DurabilityQosPolicy>::read_content_from_cdr_message(
        DurabilityQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KIND_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DeadlineQosPolicy>::add_content_to_cdr_message(
        const DeadlineQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.period.seconds);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.period.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DeadlineQosPolicy>::read_content_from_cdr_message(
        DeadlineQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.period.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.period.fraction(frac);
    return valid;
}

template <>
inline bool QosPoliciesSerializer<LatencyBudgetQosPolicy>::add_content_to_cdr_message(
        const LatencyBudgetQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.duration.seconds);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message, qos_policy.duration.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LatencyBudgetQosPolicy>::read_content_from_cdr_message(
        LatencyBudgetQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.duration.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.duration.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LivelinessQosPolicy>::add_content_to_cdr_message(
        const LivelinessQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.lease_duration.seconds);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.lease_duration.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LivelinessQosPolicy>::read_content_from_cdr_message(
        LivelinessQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.lease_duration.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.lease_duration.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ReliabilityQosPolicy>::add_content_to_cdr_message(
        const ReliabilityQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.max_blocking_time.seconds);
    valid &=
            fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.max_blocking_time.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ReliabilityQosPolicy>::read_content_from_cdr_message(
        ReliabilityQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KIND_LENGTH + PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_blocking_time.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.max_blocking_time.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipQosPolicy>::add_content_to_cdr_message(
        const OwnershipQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipQosPolicy>::read_content_from_cdr_message(
        OwnershipQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KIND_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DestinationOrderQosPolicy>::add_content_to_cdr_message(
        const DestinationOrderQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DestinationOrderQosPolicy>::read_content_from_cdr_message(
        DestinationOrderQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KIND_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ResourceLimitsQosPolicy>::add_content_to_cdr_message(
        const ResourceLimitsQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.max_samples);
    valid &=
            fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_instances);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.max_samples_per_instance);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<ResourceLimitsQosPolicy>::read_content_from_cdr_message(
        ResourceLimitsQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != 12)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_samples);
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_instances);
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.max_samples_per_instance);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TimeBasedFilterQosPolicy>::add_content_to_cdr_message(
        const TimeBasedFilterQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid =
            fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.minimum_separation.seconds);
    valid &=
            fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.minimum_separation.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TimeBasedFilterQosPolicy>::read_content_from_cdr_message(
        TimeBasedFilterQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.minimum_separation.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.minimum_separation.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<PresentationQosPolicy>::add_content_to_cdr_message(
        const PresentationQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    qos_policy.access_scope);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);

    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    (fastrtps::rtps::octet)qos_policy.coherent_access);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    (fastrtps::rtps::octet)qos_policy.ordered_access);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);

    return valid;
}

template<>
inline bool QosPoliciesSerializer<PresentationQosPolicy>::read_content_from_cdr_message(
        PresentationQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_PRESENTATION_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.access_scope);
    cdr_message->pos += 3; //padding
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.coherent_access);
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&qos_policy.ordered_access);
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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);

    //Obtain qos_policy.length:
    uint16_t len = 4;
    for (PartitionQosPolicy::const_iterator it = qos_policy.begin(); it != qos_policy.end(); ++it)
    {
        len += 4;
        len += static_cast<uint16_t>(it->size()); //Already accounts for null char
        len = (len + 3) & ~3;
    }
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);

    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    static_cast<uint32_t>(qos_policy.size()));
    for (PartitionQosPolicy::const_iterator it = qos_policy.begin(); it != qos_policy.end(); ++it)
    {
        valid &= fastrtps::rtps::CDRMessage::add_string(cdr_message, it->name());
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<PartitionQosPolicy>::read_content_from_cdr_message(
        PartitionQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (qos_policy.max_size() != 0 && parameter_length > qos_policy.max_size() + 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;

    uint32_t pos_ref = cdr_message->pos;
    uint32_t num_partitions = 0;
    bool valid = fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &num_partitions);
    //partitions_.reserve(parameter_length - 4);

    for (size_t i = 0; i < num_partitions; ++i)
    {
        uint32_t partition_size, alignment;

        valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &partition_size);
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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.depth);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<HistoryQosPolicy>::read_content_from_cdr_message(
        HistoryQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KIND_LENGTH + 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message, (fastrtps::rtps::octet*)&qos_policy.kind);
    cdr_message->pos += 3; //padding
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message, &qos_policy.depth);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DurabilityServiceQosPolicy>::add_content_to_cdr_message(
        const DurabilityServiceQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addInt32(cdr_message,
                    qos_policy.service_cleanup_delay.seconds);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    qos_policy.service_cleanup_delay.fraction());
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.history_kind);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.history_depth);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_samples);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_instances);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.max_samples_per_instance);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DurabilityServiceQosPolicy>::read_content_from_cdr_message(
        DurabilityServiceQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_TIME_LENGTH + PARAMETER_KIND_LENGTH + 16)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message,
                    &qos_policy.service_cleanup_delay.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.service_cleanup_delay.fraction(frac);
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, (fastrtps::rtps::octet*)&qos_policy.history_kind);
    cdr_message->pos += 3; //padding
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message, &qos_policy.history_depth);
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message, &qos_policy.max_samples);
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message, &qos_policy.max_instances);
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message, &qos_policy.max_samples_per_instance);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LifespanQosPolicy>::add_content_to_cdr_message(
        const LifespanQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addInt32(cdr_message, qos_policy.duration.seconds);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message, qos_policy.duration.fraction());
    return valid;
}

template<>
inline bool QosPoliciesSerializer<LifespanQosPolicy>::read_content_from_cdr_message(
        LifespanQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message, &qos_policy.duration.seconds);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    qos_policy.duration.fraction(frac);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipStrengthQosPolicy>::add_content_to_cdr_message(
        const OwnershipStrengthQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt32(cdr_message, qos_policy.value);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<OwnershipStrengthQosPolicy>::read_content_from_cdr_message(
        OwnershipStrengthQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &qos_policy.value);
}

template<>
inline bool QosPoliciesSerializer<TransportPriorityQosPolicy>::add_content_to_cdr_message(
        const TransportPriorityQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt32(cdr_message, qos_policy.value);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TransportPriorityQosPolicy>::read_content_from_cdr_message(
        TransportPriorityQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != 4)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &qos_policy.value);
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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);

    uint16_t len = static_cast<uint16_t>(qos_policy.m_value.size() * sizeof(uint16_t)) + 4;
    len = (len + 3) & ~3;

    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);
    valid &=
            fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    static_cast<uint32_t>(qos_policy.m_value.size()));
    for (const DataRepresentationId_t& id : qos_policy.m_value)
    {
        valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(id));
    }
    if (qos_policy.m_value.size() % 2 == 1) // Odd, we must align
    {
        valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, uint16_t(0));
    }
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DataRepresentationQosPolicy>::read_content_from_cdr_message(
        DataRepresentationQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    qos_policy.length = parameter_length;

    int16_t temp(0);
    uint32_t datasize(0);
    bool valid = fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &datasize);
    for (uint32_t i = 0; i < datasize; ++i)
    {
        valid &= fastrtps::rtps::CDRMessage::readInt16(cdr_message, &temp);
        qos_policy.m_value.push_back(static_cast<DataRepresentationId_t>(temp));
    }
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeConsistencyEnforcementQosPolicy>::add_content_to_cdr_message(
        const TypeConsistencyEnforcementQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.m_kind);
    valid &=
            fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastrtps::rtps::octet>(qos_policy.m_ignore_sequence_bounds));
    valid &=
            fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastrtps::rtps::octet>(qos_policy.m_ignore_string_bounds));
    valid &=
            fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastrtps::rtps::octet>(qos_policy.m_ignore_member_names));
    valid &=
            fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastrtps::rtps::octet>(qos_policy.m_prevent_type_widening));
    valid &=
            fastrtps::rtps::CDRMessage::addOctet(cdr_message,
                    static_cast<fastrtps::rtps::octet>(qos_policy.m_force_type_validation));
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, fastrtps::rtps::octet(0x00)); // 8th byte
    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeConsistencyEnforcementQosPolicy>::read_content_from_cdr_message(
        TypeConsistencyEnforcementQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < 2)
    {
        return false;
    }

    uint16_t uKind(0);
    fastrtps::rtps::octet temp(0);
    qos_policy.m_ignore_sequence_bounds = false;
    qos_policy.m_ignore_string_bounds = false;
    qos_policy.m_ignore_member_names = false;
    qos_policy.m_prevent_type_widening = false;
    qos_policy.m_force_type_validation = false;

    bool valid = fastrtps::rtps::CDRMessage::readUInt16(cdr_message, &uKind);
    qos_policy.m_kind = static_cast<TypeConsistencyKind>(uKind);

    if (parameter_length >= 3)
    {
        valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_ignore_sequence_bounds = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 4)
    {
        valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_ignore_string_bounds = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 5)
    {
        valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_ignore_member_names = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 6)
    {
        valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_prevent_type_widening = temp == 0 ? false : true;
    }

    if (valid && parameter_length >= 7)
    {
        valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &temp);
        qos_policy.m_force_type_validation = temp == 0 ? false : true;
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::add_content_to_cdr_message(
        const DisablePositiveACKsQosPolicy&,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, (fastrtps::rtps::octet)0x01);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, (fastrtps::rtps::octet)0x00);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, (fastrtps::rtps::octet)0x00);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, (fastrtps::rtps::octet)0x00);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::read_content_from_cdr_message(
        DisablePositiveACKsQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_BOOL_LENGTH)
    {
        return false;
    }
    qos_policy.length = parameter_length;
    fastrtps::rtps::octet value(0);
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message, &value);
    qos_policy.enabled = (value == 0) ? false : true;
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<DataSharingQosPolicy>::cdr_serialized_size(
        const DataSharingQosPolicy& qos_policy)
{
    // Size of data (8 bytes each)
    uint32_t data_size = static_cast<uint32_t>(qos_policy.domain_ids().size()) * 8;
    // p_id + p_length + kind + str_length + str_data
    return 2 + 2 + 4 + 4 + data_size;
}

template<>
inline bool QosPoliciesSerializer<DataSharingQosPolicy>::add_to_cdr_message(
        const DataSharingQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);

    //Obtain qos_policy.length: kind + str_length + str_data

    uint16_t len = 4 + 4 + static_cast<uint16_t>(qos_policy.domain_ids().size()) * 8;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);

    //Add the values:
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, qos_policy.kind());
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);

    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message,
                    static_cast<uint32_t>(qos_policy.domain_ids().size()));
    for (uint64_t id : qos_policy.domain_ids())
    {
        valid &= fastrtps::rtps::CDRMessage::addUInt64(cdr_message, id);
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<DataSharingQosPolicy>::read_content_from_cdr_message(
        DataSharingQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    qos_policy.length = parameter_length;
    uint32_t pos_ref = cdr_message->pos;

    DataSharingKind kind = OFF;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message,
                    (fastrtps::rtps::octet*)&kind);
    cdr_message->pos += 3; //padding

    switch (kind)
    {
        case AUTO:
            qos_policy.automatic("default"); //< We don't really care about the directory here
            break;
        case ON:
            qos_policy.on("default"); //< We don't really care about the directory here
            break;
        case OFF:
            qos_policy.off(); //< Should not be sent, but anyways
            break;
        default:
            return false;
    }

    uint32_t num_domains = 0;
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &num_domains);

    if (!valid || (qos_policy.max_domains() != 0 && num_domains > qos_policy.max_domains()))
    {
        return false;
    }

    for (size_t i = 0; i < num_domains; ++i)
    {
        uint64_t domain;
        valid &= fastrtps::rtps::CDRMessage::readUInt64(cdr_message, &domain);
        qos_policy.add_domain_id(domain);
    }

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

template<>
inline uint32_t QosPoliciesSerializer<TypeIdV1>::cdr_serialized_size(
        const TypeIdV1& qos_policy)
{
    size_t size = fastrtps::types::TypeIdentifier::getCdrSerializedSize(qos_policy.m_type_identifier) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

template<>
inline bool QosPoliciesSerializer<TypeIdV1>::add_to_cdr_message(
        const TypeIdV1& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    size_t size = fastrtps::types::TypeIdentifier::getCdrSerializedSize(qos_policy.m_type_identifier) + 4;
    fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    qos_policy.m_type_identifier.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length

    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(payload.length));
    valid &= fastrtps::rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);

    uint32_t align = 4 - (payload.length % 4); //align
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    }

    return valid;
}

template<>
inline bool QosPoliciesSerializer<TypeIdV1>::read_content_from_cdr_message(
        TypeIdV1& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.

    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        qos_policy.m_type_identifier.deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

template<>
inline uint32_t QosPoliciesSerializer<TypeObjectV1>::cdr_serialized_size(
        const TypeObjectV1& qos_policy)
{
    size_t size = fastrtps::types::TypeObject::getCdrSerializedSize(qos_policy.m_type_object) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

template<>
inline bool QosPoliciesSerializer<TypeObjectV1>::add_to_cdr_message(
        const TypeObjectV1& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    size_t size = fastrtps::types::TypeObject::getCdrSerializedSize(qos_policy.m_type_object) + 4;
    fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    qos_policy.m_type_object.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length

    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(payload.length));

    return valid & fastrtps::rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);
}

template<>
inline bool QosPoliciesSerializer<TypeObjectV1>::read_content_from_cdr_message(
        TypeObjectV1& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.

    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        qos_policy.m_type_object.deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

template<>
inline uint32_t QosPoliciesSerializer<xtypes::TypeInformation>::cdr_serialized_size(
        const xtypes::TypeInformation& qos_policy)
{
    size_t size = fastrtps::types::TypeInformation::getCdrSerializedSize(qos_policy.type_information) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

template<>
inline bool QosPoliciesSerializer<xtypes::TypeInformation>::add_to_cdr_message(
        const xtypes::TypeInformation& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    size_t size = fastrtps::types::TypeInformation::getCdrSerializedSize(qos_policy.type_information) + 4;
    fastrtps::rtps::SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    qos_policy.type_information.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length

    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    uint16_t len = static_cast<uint16_t>(payload.length);
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);

    return valid & fastrtps::rtps::CDRMessage::addData(cdr_message, payload.data, payload.length);
}

template<>
inline bool QosPoliciesSerializer<xtypes::TypeInformation>::read_content_from_cdr_message(
        xtypes::TypeInformation& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    fastrtps::rtps::SerializedPayload_t payload(parameter_length);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, parameter_length);

    fastrtps::rtps::CDRMessage::readData(cdr_message, payload.data, parameter_length); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.

    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        qos_policy.type_information.deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    qos_policy.assigned(true);

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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, qos_policy.Pid);
    uint16_t siz = static_cast<uint16_t>(qos_policy.size());
    siz = (siz + 3) & ~3;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(4 + siz));
    valid &= fastrtps::rtps::CDRMessage::addOctetVector(cdr_message, &qos_policy.data_vec(), true);
    return valid;
}

template<>
inline bool QosPoliciesSerializer<GenericDataQosPolicy>::read_content_from_cdr_message(
        GenericDataQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    uint32_t pos_ref = cdr_message->pos;

    // Read size of data
    uint32_t len;
    if (!fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &len))
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
    if (!fastrtps::rtps::CDRMessage::readData(cdr_message, qos_policy.data(), len))
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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(qos_policy, cdr_message);
}

template<>
inline bool QosPoliciesSerializer<UserDataQosPolicy>::read_from_cdr_message(
        UserDataQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(qos_policy, cdr_message);
}

template<>
inline bool QosPoliciesSerializer<GroupDataQosPolicy>::read_from_cdr_message(
        GroupDataQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
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
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::add_to_cdr_message(qos_policy, cdr_message);
}

template<>
inline bool QosPoliciesSerializer<TopicDataQosPolicy>::read_from_cdr_message(
        TopicDataQosPolicy& qos_policy,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    return QosPoliciesSerializer<GenericDataQosPolicy>::read_from_cdr_message(qos_policy, cdr_message,
                   parameter_length);
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_CORE_PLICY__QOSPOLICIESSERIALIZER_HPP_
