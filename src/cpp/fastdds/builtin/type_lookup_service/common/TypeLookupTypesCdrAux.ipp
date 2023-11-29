// Copyright 2023 y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_BUILTIN_TYPELOOKUP_COMMON_TYPELOOKUPTYPESCDRAUX_IPP
#define FASTDDS_BUILTIN_TYPELOOKUP_COMMON_TYPELOOKUPTYPESCDRAUX_IPP

namespace eprosima {
namespace fastcdr {

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypes_In& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for (size_t a = 0; a < data.type_ids.size(); ++a)
    {
        current_alignment += calculate_serialized_size(calculator, data.type_ids.at(a), current_alignment);
    }

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.type_ids, current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypes_In& data)
{

    scdr << data.type_ids;
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_getTypes_In& data)
{

    dcdr >> data.type_ids;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Out& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    for (size_t a = 0; a < data.types.size(); ++a)
    {
        current_alignment += calculate_serialized_size(calculator, data.types.at(a), current_alignment);
    }

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for (size_t a = 0; a < data.complete_to_minimal.size(); ++a)
    {
        current_alignment +=
                calculate_serialized_size(calculator, data.complete_to_minimal.at(a), current_alignment);
    }

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.types, current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.complete_to_minimal, current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);


    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Out& data)
{

    scdr << data.types;
    scdr << data.complete_to_minimal;
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Out& data)
{

    dcdr >> data.types;
    dcdr >> data.complete_to_minimal;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Result& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch (data._d())
    {
        case 0:
            current_alignment += calculate_serialized_size(calculator, data.result(), current_alignment);
            break;
        default:
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
        case 0:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.result(), current_alignment);
            break;
        default:
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
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Result& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case 0:
            scdr << data.result();
            break;
        default:
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Result& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case 0:
            dcdr >> data.result();
            break;
        default:
            break;
    }
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_In& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    for (size_t a = 0; a < data.type_ids.size(); ++a)
    {
        current_alignment += calculate_serialized_size(calculator, data.type_ids.at(a), current_alignment);
    }

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += (data.continuation_point.size() * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.type_ids, current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.continuation_point, current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_In& data)
{

    scdr << data.type_ids;
    scdr << data.continuation_point;
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_In& data)
{

    dcdr >> data.type_ids;
    dcdr >> data.continuation_point;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_Out& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    for (size_t a = 0; a < data.dependent_typeids.size(); ++a)
    {
        current_alignment += calculate_serialized_size(calculator, data.dependent_typeids.at(
                            a), current_alignment);
    }

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += (data.continuation_point.size() * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.dependent_typeids, current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.continuation_point, current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_Out& data)
{

    scdr << data.dependent_typeids;
    scdr << data.continuation_point;
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_Out& data)
{

    dcdr >> data.dependent_typeids;
    dcdr >> data.continuation_point;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_Result& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch (data._d())
    {
        case 0 /* TODO DDS_RETCODE_OK */:
            current_alignment += calculate_serialized_size(calculator, data.result(), current_alignment);
            break;
        default:
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
        case 0 /* TODO DDS_RETCODE_OK */:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.result(), current_alignment);
            break;
        default:
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
        const eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_Result& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case 0 /* TODO DDS_RETCODE_OK */:
            scdr << data.result();
            break;
        default:
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_getTypeDependencies_Result& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case 0 /* TODO DDS_RETCODE_OK */:
            dcdr >> data.result();
            break;
        default:
            break;
    }
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_Call& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch (data._d())
    {
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            current_alignment += calculate_serialized_size(calculator, data.getTypes(), current_alignment);
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            current_alignment += calculate_serialized_size(calculator, data.getTypeDependencies(), current_alignment);
            break;
        default:
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
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.getTypes(), current_alignment);
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.getTypeDependencies(), current_alignment);
            break;
        default:
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
        const eprosima::fastdds::dds::builtin::TypeLookup_Call& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            scdr << data.getTypes();
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            scdr << data.getTypeDependencies();
            break;
        default:
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_Call& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            dcdr >> data.getTypes();
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            dcdr >> data.getTypeDependencies();
            break;
        default:
            break;
    }
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_Request& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += calculate_serialized_size(calculator, data.header, current_alignment);
    current_alignment += calculate_serialized_size(calculator, data.data, current_alignment);

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header, current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.data, current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::builtin::TypeLookup_Request& data)
{

    scdr << data.header;
    scdr << data.data;
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_Request& data)
{

    dcdr >> data.header;
    dcdr >> data.data;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_Return& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch (data._d())
    {
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            current_alignment += calculate_serialized_size(calculator, data.getType(), current_alignment);
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            current_alignment += calculate_serialized_size(calculator, data.getTypeDependencies(), current_alignment);
            break;
        default:
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
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.getType(), current_alignment);
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.getTypeDependencies(), current_alignment);
            break;
        default:
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
        const eprosima::fastdds::dds::builtin::TypeLookup_Return& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            scdr << data.getType();
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            scdr << data.getTypeDependencies();
            break;
        default:
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_Return& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastdds::dds::builtin::TypeLookup_getTypes_Hash:
            dcdr >> data.getType();
            break;
        case eprosima::fastdds::dds::builtin::TypeLookup_getDependencies_Hash:
            dcdr >> data.getTypeDependencies();
            break;
        default:
            break;
    }
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastdds::dds::builtin::TypeLookup_Reply& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += calculate_serialized_size(calculator, data.header, current_alignment);
    current_alignment += calculate_serialized_size(calculator, data.return_value, current_alignment);

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header, current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.return_value, current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastdds::dds::builtin::TypeLookup_Reply& data)
{

    scdr << data.header;
    scdr << data.return_value;
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastdds::dds::builtin::TypeLookup_Reply& data)
{

    dcdr >> data.header;
    dcdr >> data.return_value;
}

} // namespace fastcdr
} // namespace eprosima

#endif // FASTDDS_BUILTIN_TYPELOOKUP_COMMON_TYPELOOKUPTYPESCDRAUX_IPP
