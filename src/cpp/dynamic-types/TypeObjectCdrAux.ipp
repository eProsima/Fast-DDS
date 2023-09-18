// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef DYNAMIC_TYPES_TYPEOBJECTCDRAUX_IPP
#define DYNAMIC_TYPES_TYPEOBJECTCDRAUX_IPP

namespace eprosima {
namespace fastcdr {
template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonStructMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.member_id(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.member_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.member_type_id(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonStructMember& data)
{
    scdr << data.member_id();
    scdr << data.member_flags();
    scdr << data.member_type_id();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonStructMember& data)
{
    dcdr >> data.member_id();
    dcdr >> data.member_flags();
    dcdr >> data.member_type_id();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteMemberDetail& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.name(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.ann_builtin(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.ann_custom(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteMemberDetail& data)
{
    scdr << data.name();
    scdr << data.ann_builtin();
    scdr << data.ann_custom();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteMemberDetail& data)
{
    dcdr >> data.name();
    dcdr >> data.ann_builtin();
    dcdr >> data.ann_custom();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalMemberDetail& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.name_hash(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalMemberDetail& data)
{
    scdr << data.name_hash();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalMemberDetail& data)
{
    dcdr >> data.name_hash();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteStructMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteStructMember& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteStructMember& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalStructMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalStructMember& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalStructMember& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.verbatim(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations& data)
{
    scdr << data.verbatim();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations& data)
{
    dcdr >> data.verbatim();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::MinimalTypeDetail&,
        size_t&)
{
    return 0;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::MinimalTypeDetail&)
{
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::MinimalTypeDetail&)
{
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteTypeDetail& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.ann_builtin(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.ann_custom(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.type_name(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteTypeDetail& data)
{
    scdr << data.ann_builtin();
    scdr << data.ann_custom();
    scdr << data.type_name();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteTypeDetail& data)
{
    dcdr >> data.ann_builtin();
    dcdr >> data.ann_custom();
    dcdr >> data.type_name();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteStructHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.base_type(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteStructHeader& data)
{
    scdr << data.base_type();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteStructHeader& data)
{
    dcdr >> data.base_type();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalStructHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.base_type(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalStructHeader& data)
{
    scdr << data.base_type();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalStructHeader& data)
{
    dcdr >> data.base_type();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteStructType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.struct_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.member_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteStructType& data)
{
    scdr << data.struct_flags();
    scdr << data.header();
    scdr << data.member_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteStructType& data)
{
    dcdr >> data.struct_flags();
    dcdr >> data.header();
    dcdr >> data.member_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalStructType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.struct_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.member_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalStructType& data)
{
    scdr << data.struct_flags();
    scdr << data.header();
    scdr << data.member_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalStructType& data)
{
    dcdr >> data.struct_flags();
    dcdr >> data.header();
    dcdr >> data.member_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonUnionMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.member_id(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.member_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.type_id(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.label_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonUnionMember& data)
{
    scdr << data.member_id();
    scdr << data.member_flags();
    scdr << data.type_id();
    scdr << data.label_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonUnionMember& data)
{
    dcdr >> data.member_id();
    dcdr >> data.member_flags();
    dcdr >> data.type_id();
    dcdr >> data.label_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteUnionMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteUnionMember& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteUnionMember& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalUnionMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalUnionMember& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalUnionMember& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonDiscriminatorMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.member_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.type_id(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonDiscriminatorMember& data)
{
    scdr << data.member_flags();
    scdr << data.type_id();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonDiscriminatorMember& data)
{
    dcdr >> data.member_flags();
    dcdr >> data.type_id();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteDiscriminatorMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.ann_builtin(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.ann_custom(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteDiscriminatorMember& data)
{
    scdr << data.common();
    scdr << data.ann_builtin();
    scdr << data.ann_custom();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteDiscriminatorMember& data)
{
    dcdr >> data.common();
    dcdr >> data.ann_builtin();
    dcdr >> data.ann_custom();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalDiscriminatorMember& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalDiscriminatorMember& data)
{
    scdr << data.common();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalDiscriminatorMember& data)
{
    dcdr >> data.common();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteUnionHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteUnionHeader& data)
{
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteUnionHeader& data)
{
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalUnionHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalUnionHeader& data)
{
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalUnionHeader& data)
{
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteUnionType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.union_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.discriminator(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.member_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteUnionType& data)
{
    scdr << data.union_flags();
    scdr << data.header();
    scdr << data.discriminator();
    scdr << data.member_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteUnionType& data)
{
    dcdr >> data.union_flags();
    dcdr >> data.header();
    dcdr >> data.discriminator();
    dcdr >> data.member_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalUnionType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.union_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.discriminator(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.member_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalUnionType& data)
{
    scdr << data.union_flags();
    scdr << data.header();
    scdr << data.discriminator();
    scdr << data.member_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalUnionType& data)
{
    dcdr >> data.union_flags();
    dcdr >> data.header();
    dcdr >> data.discriminator();
    dcdr >> data.member_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonAnnotationParameter& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.member_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.member_type_id(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonAnnotationParameter& data)
{
    scdr << data.member_flags();
    scdr << data.member_type_id();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonAnnotationParameter& data)
{
    dcdr >> data.member_flags();
    dcdr >> data.member_type_id();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteAnnotationParameter& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.name(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.default_value(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteAnnotationParameter& data)
{
    scdr << data.common();
    scdr << data.name();
    scdr << data.default_value();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteAnnotationParameter& data)
{
    dcdr >> data.common();
    dcdr >> data.name();
    dcdr >> data.default_value();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalAnnotationParameter& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.name(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.default_value(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalAnnotationParameter& data)
{
    scdr << data.common();
    scdr << data.name();
    scdr << data.default_value();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalAnnotationParameter& data)
{
    dcdr >> data.common();
    dcdr >> data.name();
    dcdr >> data.default_value();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteAnnotationHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.annotation_name(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteAnnotationHeader& data)
{
    scdr << data.annotation_name();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteAnnotationHeader& data)
{
    dcdr >> data.annotation_name();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::MinimalAnnotationHeader&,
        size_t&)
{
    return 0;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::MinimalAnnotationHeader&)
{
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::MinimalAnnotationHeader&)
{
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteAnnotationType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.annotation_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.member_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteAnnotationType& data)
{
    scdr << data.annotation_flag();
    scdr << data.header();
    scdr << data.member_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteAnnotationType& data)
{
    dcdr >> data.annotation_flag();
    dcdr >> data.header();
    dcdr >> data.member_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalAnnotationType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.annotation_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.member_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalAnnotationType& data)
{
    scdr << data.annotation_flag();
    scdr << data.header();
    scdr << data.member_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalAnnotationType& data)
{
    dcdr >> data.annotation_flag();
    dcdr >> data.header();
    dcdr >> data.member_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonAliasBody& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.related_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.related_type(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonAliasBody& data)
{
    scdr << data.related_flags();
    scdr << data.related_type();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonAliasBody& data)
{
    dcdr >> data.related_flags();
    dcdr >> data.related_type();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteAliasBody& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.ann_builtin(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.ann_custom(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteAliasBody& data)
{
    scdr << data.common();
    scdr << data.ann_builtin();
    scdr << data.ann_custom();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteAliasBody& data)
{
    dcdr >> data.common();
    dcdr >> data.ann_builtin();
    dcdr >> data.ann_custom();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalAliasBody& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalAliasBody& data)
{
    scdr << data.common();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalAliasBody& data)
{
    dcdr >> data.common();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteAliasHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteAliasHeader& data)
{
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteAliasHeader& data)
{
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::MinimalAliasHeader&,
        size_t&)
{
    return 0;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::MinimalAliasHeader&)
{
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::MinimalAliasHeader&)
{
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteAliasType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.alias_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.body(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteAliasType& data)
{
    scdr << data.alias_flags();
    scdr << data.header();
    scdr << data.body();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteAliasType& data)
{
    dcdr >> data.alias_flags();
    dcdr >> data.header();
    dcdr >> data.body();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalAliasType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.alias_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.body(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalAliasType& data)
{
    scdr << data.alias_flags();
    scdr << data.header();
    scdr << data.body();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalAliasType& data)
{
    dcdr >> data.alias_flags();
    dcdr >> data.header();
    dcdr >> data.body();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteElementDetail& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.ann_builtin(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.ann_custom(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteElementDetail& data)
{
    scdr << data.ann_builtin();
    scdr << data.ann_custom();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteElementDetail& data)
{
    dcdr >> data.ann_builtin();
    dcdr >> data.ann_custom();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonCollectionElement& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.element_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.type(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonCollectionElement& data)
{
    scdr << data.element_flags();
    scdr << data.type();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonCollectionElement& data)
{
    dcdr >> data.element_flags();
    dcdr >> data.type();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteCollectionElement& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteCollectionElement& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteCollectionElement& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalCollectionElement& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalCollectionElement& data)
{
    scdr << data.common();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalCollectionElement& data)
{
    dcdr >> data.common();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonCollectionHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bound(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonCollectionHeader& data)
{
    scdr << data.bound();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonCollectionHeader& data)
{
    dcdr >> data.bound();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteCollectionHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteCollectionHeader& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteCollectionHeader& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalCollectionHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalCollectionHeader& data)
{
    scdr << data.common();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalCollectionHeader& data)
{
    dcdr >> data.common();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalSequenceType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.collection_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.element(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteSequenceType& data)
{
    scdr << data.collection_flag();
    scdr << data.header();
    scdr << data.element();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteSequenceType& data)
{
    dcdr >> data.collection_flag();
    dcdr >> data.header();
    dcdr >> data.element();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteSequenceType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.collection_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.element(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalSequenceType& data)
{
    scdr << data.collection_flag();
    scdr << data.header();
    scdr << data.element();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalSequenceType& data)
{
    dcdr >> data.collection_flag();
    dcdr >> data.header();
    dcdr >> data.element();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonArrayHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bound_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonArrayHeader& data)
{
    scdr << data.bound_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonArrayHeader& data)
{
    dcdr >> data.bound_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteArrayHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteArrayHeader& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteArrayHeader& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalArrayHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalArrayHeader& data)
{
    scdr << data.common();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalArrayHeader& data)
{
    dcdr >> data.common();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteArrayType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.collection_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.element(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteArrayType& data)
{
    scdr << data.collection_flag();
    scdr << data.header();
    scdr << data.element();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteArrayType& data)
{
    dcdr >> data.collection_flag();
    dcdr >> data.header();
    dcdr >> data.element();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalArrayType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.collection_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.element(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalArrayType& data)
{
    scdr << data.collection_flag();
    scdr << data.header();
    scdr << data.element();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalArrayType& data)
{
    dcdr >> data.collection_flag();
    dcdr >> data.header();
    dcdr >> data.element();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteMapType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.collection_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.key(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.element(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteMapType& data)
{
    scdr << data.collection_flag();
    scdr << data.header();
    scdr << data.key();
    scdr << data.element();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteMapType& data)
{
    dcdr >> data.collection_flag();
    dcdr >> data.header();
    dcdr >> data.key();
    dcdr >> data.element();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalMapType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.collection_flag(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.key(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.element(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalMapType& data)
{
    scdr << data.collection_flag();
    scdr << data.header();
    scdr << data.key();
    scdr << data.element();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalMapType& data)
{
    dcdr >> data.collection_flag();
    dcdr >> data.header();
    dcdr >> data.key();
    dcdr >> data.element();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonEnumeratedLiteral& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.value(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.flags(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonEnumeratedLiteral& data)
{
    scdr << data.value();
    scdr << data.flags();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonEnumeratedLiteral& data)
{
    dcdr >> data.value();
    dcdr >> data.flags();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteEnumeratedLiteral& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteEnumeratedLiteral& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteEnumeratedLiteral& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalEnumeratedLiteral& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalEnumeratedLiteral& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalEnumeratedLiteral& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonEnumeratedHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bit_bound(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonEnumeratedHeader& data)
{
    scdr << data.bit_bound();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonEnumeratedHeader& data)
{
    dcdr >> data.bit_bound();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteEnumeratedHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteEnumeratedHeader& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteEnumeratedHeader& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalEnumeratedHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalEnumeratedHeader& data)
{
    scdr << data.common();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalEnumeratedHeader& data)
{
    dcdr >> data.common();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteEnumeratedType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.enum_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.literal_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteEnumeratedType& data)
{
    scdr << data.enum_flags();
    scdr << data.header();
    scdr << data.literal_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteEnumeratedType& data)
{
    dcdr >> data.enum_flags();
    dcdr >> data.header();
    dcdr >> data.literal_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalEnumeratedType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.enum_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.literal_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalEnumeratedType& data)
{
    scdr << data.enum_flags();
    scdr << data.header();
    scdr << data.literal_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalEnumeratedType& data)
{
    dcdr >> data.enum_flags();
    dcdr >> data.header();
    dcdr >> data.literal_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonBitflag& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.position(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.flags(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonBitflag& data)
{
    scdr << data.position();
    scdr << data.flags();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonBitflag& data)
{
    dcdr >> data.position();
    dcdr >> data.flags();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteBitflag& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteBitflag& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteBitflag& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalBitflag& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalBitflag& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalBitflag& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonBitmaskHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bit_bound(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonBitmaskHeader& data)
{
    scdr << data.bit_bound();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonBitmaskHeader& data)
{
    dcdr >> data.bit_bound();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteBitmaskType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bitmask_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.flag_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteBitmaskType& data)
{
    scdr << data.bitmask_flags();
    scdr << data.header();
    scdr << data.flag_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteBitmaskType& data)
{
    dcdr >> data.bitmask_flags();
    dcdr >> data.header();
    dcdr >> data.flag_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalBitmaskType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bitmask_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.flag_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalBitmaskType& data)
{
    scdr << data.bitmask_flags();
    scdr << data.header();
    scdr << data.flag_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalBitmaskType& data)
{
    dcdr >> data.bitmask_flags();
    dcdr >> data.header();
    dcdr >> data.flag_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CommonBitfield& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.position(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.bitcount(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.holder_type(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CommonBitfield& data)
{
    scdr << data.position();
    scdr << data.flags();
    scdr << data.bitcount();
    scdr << data.holder_type();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CommonBitfield& data)
{
    dcdr >> data.position();
    dcdr >> data.flags();
    dcdr >> data.bitcount();
    dcdr >> data.holder_type();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteBitfield& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.common(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.detail(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteBitfield& data)
{
    scdr << data.common();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteBitfield& data)
{
    dcdr >> data.common();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalBitfield& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.name_hash(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.common(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalBitfield& data)
{
    scdr << data.common();
    scdr << data.name_hash();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalBitfield& data)
{
    dcdr >> data.common();
    dcdr >> data.name_hash();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteBitsetHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.detail(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.base_type(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteBitsetHeader& data)
{
    scdr << data.base_type();
    scdr << data.detail();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteBitsetHeader& data)
{
    dcdr >> data.base_type();
    dcdr >> data.detail();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalBitsetHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.base_type(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalBitsetHeader& data)
{
    scdr << data.base_type();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalBitsetHeader& data)
{
    dcdr >> data.base_type();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteBitsetType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bitset_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.field_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteBitsetType& data)
{
    scdr << data.bitset_flags();
    scdr << data.header();
    scdr << data.field_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteBitsetType& data)
{
    dcdr >> data.bitset_flags();
    dcdr >> data.header();
    dcdr >> data.field_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalBitsetType& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.bitset_flags(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.field_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalBitsetType& data)
{
    scdr << data.bitset_flags();
    scdr << data.header();
    scdr << data.field_seq();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalBitsetType& data)
{
    dcdr >> data.bitset_flags();
    dcdr >> data.header();
    dcdr >> data.field_seq();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::CompleteExtendedType&,
        size_t&)
{
    return 0;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::CompleteExtendedType&)
{
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::CompleteExtendedType&)
{
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::MinimalExtendedType&,
        size_t&)
{
    return 0;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::MinimalExtendedType&)
{
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::MinimalExtendedType&)
{
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeIdentifierTypeObjectPair& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.type_identifier(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.type_object(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeIdentifierTypeObjectPair& data)
{
    scdr << data.type_identifier();
    scdr << data.type_object();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeIdentifierTypeObjectPair& data)
{
    dcdr >> data.type_identifier();
    dcdr >> data.type_object();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeIdentifierPair& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.type_identifier1(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.type_identifier2(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeIdentifierPair& data)
{
    scdr << data.type_identifier1();
    scdr << data.type_identifier2();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeIdentifierPair& data)
{
    dcdr >> data.type_identifier1();
    dcdr >> data.type_identifier2();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeIdentifierWithSize& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.type_id(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.typeobject_serialized_size(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeIdentifierWithSize& data)
{
    scdr << data.type_id();
    scdr << data.typeobject_serialized_size();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeIdentifierWithSize& data)
{
    dcdr >> data.type_id();
    dcdr >> data.typeobject_serialized_size();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeIdentifierWithDependencies& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.typeid_with_size(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.dependent_typeid_count(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.dependent_typeids(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeIdentifierWithDependencies& data)
{
    scdr << data.typeid_with_size();
    scdr << data.dependent_typeid_count();
    scdr << data.dependent_typeids();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeIdentifierWithDependencies& data)
{
    dcdr >> data.typeid_with_size();
    dcdr >> data.dependent_typeid_count();
    dcdr >> data.dependent_typeids();
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::CompleteTypeObject& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data._d(), current_alignment);

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_ALIAS:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.alias_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_ANNOTATION:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.annotation_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_STRUCTURE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                3), data.struct_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_UNION:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                4), data.union_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_BITSET:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                5), data.bitset_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_SEQUENCE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                6), data.sequence_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_ARRAY:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                7), data.array_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_MAP:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                8), data.map_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                9), data.enumerated_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_BITMASK:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                10), data.bitmask_type(), current_alignment);
            break;
        default:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                11), data.extended_type(), current_alignment);
            break;
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::CompleteTypeObject& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_ALIAS:
            scdr << data.alias_type();
            break;
        case eprosima::fastrtps::types::TK_ANNOTATION:
            scdr << data.annotation_type();
            break;
        case eprosima::fastrtps::types::TK_STRUCTURE:
            scdr << data.struct_type();
            break;
        case eprosima::fastrtps::types::TK_UNION:
            scdr << data.union_type();
            break;
        case eprosima::fastrtps::types::TK_BITSET:
            scdr << data.bitset_type();
            break;
        case eprosima::fastrtps::types::TK_SEQUENCE:
            scdr << data.sequence_type();
            break;
        case eprosima::fastrtps::types::TK_ARRAY:
            scdr << data.array_type();
            break;
        case eprosima::fastrtps::types::TK_MAP:
            scdr << data.map_type();
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            scdr << data.enumerated_type();
            break;
        case eprosima::fastrtps::types::TK_BITMASK:
            scdr << data.bitmask_type();
            break;
        default:
            scdr << data.extended_type();
            break;
    }
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::CompleteTypeObject& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_ALIAS:
            dcdr >> data.alias_type();
            break;
        case eprosima::fastrtps::types::TK_ANNOTATION:
            dcdr >> data.annotation_type();
            break;
        case eprosima::fastrtps::types::TK_STRUCTURE:
            dcdr >> data.struct_type();
            break;
        case eprosima::fastrtps::types::TK_UNION:
            dcdr >> data.union_type();
            break;
        case eprosima::fastrtps::types::TK_BITSET:
            dcdr >> data.bitset_type();
            break;
        case eprosima::fastrtps::types::TK_SEQUENCE:
            dcdr >> data.sequence_type();
            break;
        case eprosima::fastrtps::types::TK_ARRAY:
            dcdr >> data.array_type();
            break;
        case eprosima::fastrtps::types::TK_MAP:
            dcdr >> data.map_type();
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            dcdr >> data.enumerated_type();
            break;
        case eprosima::fastrtps::types::TK_BITMASK:
            dcdr >> data.bitmask_type();
            break;
        default:
            dcdr >> data.extended_type();
            break;
    }
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::MinimalTypeObject& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data._d(), current_alignment);

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_ALIAS:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.alias_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_ANNOTATION:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.annotation_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_STRUCTURE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                3), data.struct_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_UNION:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                4), data.union_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_BITSET:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                5), data.bitset_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_SEQUENCE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                6), data.sequence_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_ARRAY:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                7), data.array_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_MAP:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                8), data.map_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                9), data.enumerated_type(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_BITMASK:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                10), data.bitmask_type(), current_alignment);
            break;
        default:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                11), data.extended_type(), current_alignment);
            break;
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::MinimalTypeObject& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_ALIAS:
            scdr << data.alias_type();
            break;
        case eprosima::fastrtps::types::TK_ANNOTATION:
            scdr << data.annotation_type();
            break;
        case eprosima::fastrtps::types::TK_STRUCTURE:
            scdr << data.struct_type();
            break;
        case eprosima::fastrtps::types::TK_UNION:
            scdr << data.union_type();
            break;
        case eprosima::fastrtps::types::TK_BITSET:
            scdr << data.bitset_type();
            break;
        case eprosima::fastrtps::types::TK_SEQUENCE:
            scdr << data.sequence_type();
            break;
        case eprosima::fastrtps::types::TK_ARRAY:
            scdr << data.array_type();
            break;
        case eprosima::fastrtps::types::TK_MAP:
            scdr << data.map_type();
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            scdr << data.enumerated_type();
            break;
        case eprosima::fastrtps::types::TK_BITMASK:
            scdr << data.bitmask_type();
            break;
        default:
            scdr << data.extended_type();
            break;
    }
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::MinimalTypeObject& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_ALIAS:
            dcdr >> data.alias_type();
            break;
        case eprosima::fastrtps::types::TK_ANNOTATION:
            dcdr >> data.annotation_type();
            break;
        case eprosima::fastrtps::types::TK_STRUCTURE:
            dcdr >> data.struct_type();
            break;
        case eprosima::fastrtps::types::TK_UNION:
            dcdr >> data.union_type();
            break;
        case eprosima::fastrtps::types::TK_BITSET:
            dcdr >> data.bitset_type();
            break;
        case eprosima::fastrtps::types::TK_SEQUENCE:
            dcdr >> data.sequence_type();
            break;
        case eprosima::fastrtps::types::TK_ARRAY:
            dcdr >> data.array_type();
            break;
        case eprosima::fastrtps::types::TK_MAP:
            dcdr >> data.map_type();
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            dcdr >> data.enumerated_type();
            break;
        case eprosima::fastrtps::types::TK_BITMASK:
            dcdr >> data.bitmask_type();
            break;
        default:
            dcdr >> data.extended_type();
            break;
    }
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeObject& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data._d(), current_alignment);

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.complete(), current_alignment);
            break;
        case eprosima::fastrtps::types::EK_MINIMAL:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.minimal(), current_alignment);
            break;
        default:
            break;
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeObject& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
            scdr << data.complete();
            break;
        case eprosima::fastrtps::types::EK_MINIMAL:
            scdr << data.minimal();
            break;
        default:
            break;
    }
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeObject& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
            dcdr >> data.complete();
            break;
        case eprosima::fastrtps::types::EK_MINIMAL:
            dcdr >> data.minimal();
            break;
        default:
            break;
    }
}

template<>
RTPS_DllAPI size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeInformation& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.minimal(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.complete(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
RTPS_DllAPI void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeInformation& data)
{
    scdr << data.minimal();
    scdr << data.complete();
}

template<>
RTPS_DllAPI void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeInformation& data)
{
    dcdr >> data.minimal();
    dcdr >> data.complete();
}

} // namespace fastcdr
} // namespace eprosima

#endif // DYNAMIC_TYPES_TYPEOBJECTCDRAUX_IPP
