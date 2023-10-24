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

#ifndef DYNAMIC_TYPES_ANNOTATIONPARAMETERVALUE_IPP
#define DYNAMIC_TYPES_ANNOTATIONPARAMETERVALUE_IPP

namespace eprosima {
namespace fastcdr {

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::ExtendedAnnotationParameterValue&,
        size_t&)
{
    return 0;
}

template<>
void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::ExtendedAnnotationParameterValue&)
{
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::ExtendedAnnotationParameterValue&)
{
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::AnnotationParameterValue& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_BOOLEAN:
            current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

            break;
        case eprosima::fastrtps::types::TK_BYTE:
            current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

            break;
        case eprosima::fastrtps::types::TK_INT16:
            current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

            break;
        case eprosima::fastrtps::types::TK_UINT16:
            current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

            break;
        case eprosima::fastrtps::types::TK_INT32:
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            break;
        case eprosima::fastrtps::types::TK_UINT32:
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            break;
        case eprosima::fastrtps::types::TK_INT64:
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);

            break;
        case eprosima::fastrtps::types::TK_UINT64:
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);

            break;
        case eprosima::fastrtps::types::TK_FLOAT32:
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            break;
        case eprosima::fastrtps::types::TK_FLOAT64:
            current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);

            break;
        case eprosima::fastrtps::types::TK_FLOAT128:
            current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16);

            break;
        case eprosima::fastrtps::types::TK_CHAR8:
            current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

            break;
        case eprosima::fastrtps::types::TK_CHAR16:
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            break;
        case eprosima::fastrtps::types::TK_ENUM:
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

            break;
        case eprosima::fastrtps::types::TK_STRING8:
            current_alignment += 4 +
                    eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.string8_value().size() + 1;
            break;
        case eprosima::fastrtps::types::TK_STRING16:
            current_alignment += 4 +
                    eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.string16_value().size() + 1;
            break;
        default:
            calculate_serialized_size(calculator, data.extended_value(), current_alignment);
            break;
    }

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data._d(), current_alignment);

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_BOOLEAN:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.boolean_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_BYTE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.byte_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_INT16:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                3), data.int16_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_UINT16:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                4), data.uint_16_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_INT32:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                5), data.int32_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_UINT32:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                6), data.uint32_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_INT64:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                7), data.int64_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_UINT64:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                8), data.uint64_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_FLOAT32:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                9), data.float32_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_FLOAT64:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                10), data.float64_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_FLOAT128:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                11), data.float128_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_CHAR8:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                12), data.char_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_CHAR16:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                13), data.wchar_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_ENUM:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                14), data.enumerated_value(), current_alignment);

            break;
        case eprosima::fastrtps::types::TK_STRING8:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                15), data.string8_value(), current_alignment);
            break;
        case eprosima::fastrtps::types::TK_STRING16:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                16), data.string16_value(), current_alignment);
            break;
        default:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                17), data.extended_value(), current_alignment);
            break;
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::AnnotationParameterValue& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_BOOLEAN:
            scdr << data.boolean_value();
            break;
        case eprosima::fastrtps::types::TK_BYTE:
            scdr << data.byte_value();
            break;
        case eprosima::fastrtps::types::TK_INT16:
            scdr << data.int16_value();
            break;
        case eprosima::fastrtps::types::TK_UINT16:
            scdr << data.uint_16_value();
            break;
        case eprosima::fastrtps::types::TK_INT32:
            scdr << data.int32_value();
            break;
        case eprosima::fastrtps::types::TK_UINT32:
            scdr << data.uint32_value();
            break;
        case eprosima::fastrtps::types::TK_INT64:
            scdr << data.int64_value();
            break;
        case eprosima::fastrtps::types::TK_UINT64:
            scdr << data.uint64_value();
            break;
        case eprosima::fastrtps::types::TK_FLOAT32:
            scdr << data.float32_value();
            break;
        case eprosima::fastrtps::types::TK_FLOAT64:
            scdr << data.float64_value();
            break;
        case eprosima::fastrtps::types::TK_FLOAT128:
            scdr << data.float128_value();
            break;
        case eprosima::fastrtps::types::TK_CHAR8:
            scdr << data.char_value();
            break;
        case eprosima::fastrtps::types::TK_CHAR16:
            scdr << data.wchar_value();
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            scdr << data.enumerated_value();
            break;
        case eprosima::fastrtps::types::TK_STRING8:
            scdr << data.string8_value();
            break;
        case eprosima::fastrtps::types::TK_STRING16:
            scdr << data.string16_value();
            break;
        default:
            scdr << data.extended_value();
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::AnnotationParameterValue& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_BOOLEAN:
            dcdr >> data.boolean_value();
            break;
        case eprosima::fastrtps::types::TK_BYTE:
            dcdr >> data.byte_value();
            break;
        case eprosima::fastrtps::types::TK_INT16:
            dcdr >> data.int16_value();
            break;
        case eprosima::fastrtps::types::TK_UINT16:
            dcdr >> data.uint_16_value();
            break;
        case eprosima::fastrtps::types::TK_INT32:
            dcdr >> data.int32_value();
            break;
        case eprosima::fastrtps::types::TK_UINT32:
            dcdr >> data.uint32_value();
            break;
        case eprosima::fastrtps::types::TK_INT64:
            dcdr >> data.int64_value();
            break;
        case eprosima::fastrtps::types::TK_UINT64:
            dcdr >> data.uint64_value();
            break;
        case eprosima::fastrtps::types::TK_FLOAT32:
            dcdr >> data.float32_value();
            break;
        case eprosima::fastrtps::types::TK_FLOAT64:
            dcdr >> data.float64_value();
            break;
        case eprosima::fastrtps::types::TK_FLOAT128:
            dcdr >> data.float128_value();
            break;
        case eprosima::fastrtps::types::TK_CHAR8:
            dcdr >> data.char_value();
            break;
        case eprosima::fastrtps::types::TK_CHAR16:
            dcdr >> data.wchar_value();
            break;
        case eprosima::fastrtps::types::TK_ENUM:
            dcdr >> data.enumerated_value();
            break;
        case eprosima::fastrtps::types::TK_STRING8:
            dcdr >> data.string8_value();
            break;
        case eprosima::fastrtps::types::TK_STRING16:
            dcdr >> data.string16_value();
            break;
        default:
            dcdr >> data.extended_value();
            break;
    }
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::AppliedAnnotationParameter& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    calculate_serialized_size(calculator, data.value(), current_alignment);

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.paramname_hash(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.value(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::AppliedAnnotationParameter& data)
{
    scdr << data.paramname_hash();
    scdr << data.value();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::AppliedAnnotationParameter& data)
{
    dcdr >> data.paramname_hash();
    dcdr >> data.value();
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::AppliedAnnotation& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    calculate_serialized_size(calculator, data.annotation_typeid(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for (size_t a = 0; a < data.param_seq().size(); ++a)
    {
        calculate_serialized_size(calculator, data.param_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.annotation_typeid(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.param_seq(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::AppliedAnnotation& data)
{
    scdr << data.annotation_typeid();
    scdr << data.param_seq();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::AppliedAnnotation& data)
{
    dcdr >> data.annotation_typeid();
    dcdr >> data.param_seq();
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::AppliedVerbatimAnnotation& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.placement().size() + 1;
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.language().size() + 1;
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.text().size() + 1;

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.placement(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.language(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.text(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::AppliedVerbatimAnnotation& data)
{
    scdr << data.placement();
    scdr << data.language();
    scdr << data.text();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::AppliedVerbatimAnnotation& data)
{
    dcdr >> data.placement();
    dcdr >> data.language();
    dcdr >> data.text();
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::AppliedBuiltinMemberAnnotations& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.unit().size() + 1;
    calculate_serialized_size(calculator, data.min(), current_alignment);
    calculate_serialized_size(calculator, data.max(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.hash_id().size() + 1;

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.unit(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.min(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.max(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.hash_id(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::AppliedBuiltinMemberAnnotations& data)
{
    scdr << data.unit();
    scdr << data.min();
    scdr << data.max();
    scdr << data.hash_id();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::AppliedBuiltinMemberAnnotations& data)
{
    dcdr >> data.unit();
    dcdr >> data.min();
    dcdr >> data.max();
    dcdr >> data.hash_id();
}

} // namespace fastcdr
} // namespace eprosima

#endif //  DYNAMIC_TYPES_ANNOTATIONPARAMETERVALUE_IPP
