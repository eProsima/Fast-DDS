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

#ifndef DYNAMIC_TYPES_TYPEIDENTIFIERTYPESCDRAUX_IPP
#define DYNAMIC_TYPES_TYPEIDENTIFIERTYPESCDRAUX_IPP

namespace eprosima {
namespace fastcdr {
template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::StringSTypeDefn& data,
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
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::StringLTypeDefn& data,
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
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainCollectionHeader& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.equiv_kind(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.element_flags(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);


    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainSequenceSElemDefn& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.bound(), current_alignment);
    if (data.element_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), *data.element_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), emptyId, current_alignment);
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainSequenceLElemDefn& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.bound(), current_alignment);
    if (data.element_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), *data.element_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), emptyId, current_alignment);
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainArraySElemDefn& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.array_bound_seq(), current_alignment);
    if (data.element_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), *data.element_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), emptyId, current_alignment);
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainArrayLElemDefn& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.array_bound_seq(), current_alignment);
    if (data.element_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), *data.element_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), emptyId, current_alignment);
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainMapSTypeDefn& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.bound(), current_alignment);
    if (data.element_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), *data.element_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), emptyId, current_alignment);
    }
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.key_flags(), current_alignment);
    if (data.key_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            4), *data.key_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            4), emptyId, current_alignment);
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::PlainMapLTypeDefn& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.header(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.bound(), current_alignment);
    if (data.element_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), *data.element_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            2), emptyId, current_alignment);
    }
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        3), data.key_flags(), current_alignment);
    if (data.key_identifier() != nullptr)
    {
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            4), *data.key_identifier(), current_alignment);
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                            4), emptyId, current_alignment);
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::StronglyConnectedComponentId& data,
        size_t& current_alignment)
{
    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data.sc_component_id(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        1), data.scc_length(), current_alignment);
    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        2), data.scc_index(), current_alignment);

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;
}

template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator&,
        const eprosima::fastrtps::types::ExtendedTypeDefn&,
        size_t&)
{
    return 0;
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::StringSTypeDefn& data)
{
    scdr << data.bound();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::StringSTypeDefn& data)
{
    dcdr >> data.bound();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::StringLTypeDefn& data)
{
    scdr << data.bound();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::StringLTypeDefn& data)
{
    dcdr >> data.bound();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainCollectionHeader& data)
{
    scdr << data.equiv_kind();
    scdr << data.element_flags();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainCollectionHeader& data)
{
    dcdr >> data.equiv_kind();
    dcdr >> data.element_flags();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainSequenceSElemDefn& data)
{
    scdr << data.header();
    scdr << data.bound();
    if (data.element_identifier() == nullptr)
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
    else
    {
        scdr << *data.element_identifier();
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainSequenceSElemDefn& data)
{
    dcdr >> data.header();
    dcdr >> data.bound();
    if (data.element_identifier() == nullptr)
    {
        data.element_identifier(new eprosima::fastrtps::types::TypeIdentifier);
    }
    dcdr >> *data.element_identifier();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainSequenceLElemDefn& data)
{
    scdr << data.header();
    scdr << data.bound();
    if (data.element_identifier() != nullptr)
    {
        scdr << *data.element_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainSequenceLElemDefn& data)
{
    dcdr >> data.header();
    dcdr >> data.bound();
    if (data.element_identifier() == nullptr)
    {
        data.element_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.element_identifier();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainArraySElemDefn& data)
{
    scdr << data.header();
    scdr << data.array_bound_seq();

    if (data.element_identifier() != nullptr)
    {
        scdr << *data.element_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainArraySElemDefn& data)
{
    dcdr >> data.header();
    dcdr >> data.array_bound_seq();
    if (data.element_identifier() == nullptr)
    {
        data.element_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.element_identifier();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainArrayLElemDefn& data)
{
    scdr << data.header();
    scdr << data.array_bound_seq();
    if (data.element_identifier() != nullptr)
    {
        scdr << *data.element_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainArrayLElemDefn& data)
{
    dcdr >> data.header();
    dcdr >> data.array_bound_seq();
    if (data.element_identifier() == nullptr)
    {
        data.element_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.element_identifier();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainMapSTypeDefn& data)
{
    scdr << data.header();
    scdr << data.bound();
    if (data.element_identifier() != nullptr)
    {
        scdr << *data.element_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
    scdr << data.key_flags();
    if (data.key_identifier() != nullptr)
    {
        scdr << *data.key_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainMapSTypeDefn& data)
{
    dcdr >> data.header();
    dcdr >> data.bound();
    if (data.element_identifier() == nullptr)
    {
        data.element_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.element_identifier();
    dcdr >> data.key_flags();
    if (data.key_identifier() == nullptr)
    {
        data.key_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.key_identifier();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::PlainMapLTypeDefn& data)
{
    scdr << data.header();
    scdr << data.bound();
    if (data.element_identifier() != nullptr)
    {
        scdr << *data.element_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
    scdr << data.key_flags();
    if (data.key_identifier() != nullptr)
    {
        scdr << *data.key_identifier();
    }
    else
    {
        eprosima::fastrtps::types::TypeIdentifier emptyId;
        scdr << emptyId;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::PlainMapLTypeDefn& data)
{
    dcdr >> data.header();
    dcdr >> data.bound();
    if (data.element_identifier() == nullptr)
    {
        data.element_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.element_identifier();
    dcdr >> data.key_flags();
    if (data.key_identifier() == nullptr)
    {
        data.key_identifier(new eprosima::fastrtps::types::TypeIdentifier());
    }
    dcdr >> *data.key_identifier();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::StronglyConnectedComponentId& data)
{
    scdr << data.sc_component_id();
    scdr << data.scc_length();
    scdr << data.scc_index();
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::StronglyConnectedComponentId& data)
{
    dcdr >> data.sc_component_id();
    dcdr >> data.scc_length();
    dcdr >> data.scc_index();
}

template<>
void serialize(
        eprosima::fastcdr::Cdr&,
        const eprosima::fastrtps::types::ExtendedTypeDefn&)
{
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr&,
        eprosima::fastrtps::types::ExtendedTypeDefn&)
{
}

} // namespace fastcdr
} // namespace eprosima

#endif // DYNAMIC_TYPES_TYPEIDENTIFIERTYPESCDRAUX_IPP
