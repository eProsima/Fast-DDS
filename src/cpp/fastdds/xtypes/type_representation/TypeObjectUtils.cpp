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

#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include <string>

#include <fastcdr/exceptions/BadParamException.h>

#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

const TypeObjectHashId TypeObjectUtils::build_type_object_hash_id(
        uint8_t discriminator,
        const EquivalenceHash& hash)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "StronglyConnectedComponents not yet supported.");
    TypeObjectHashId type_object_hash_id;
    type_object_hash_id.hash(hash);
    try
    {
        type_object_hash_id._d(discriminator);
    }
    catch(const eprosima::fastcdr::exception::BadParamException& e)
    {
        throw InvalidArgumentError(e.what());
    }
    return type_object_hash_id;
}

CollectionElementFlag TypeObjectUtils::build_collection_element_flag(
        TryConstructKind try_construct_kind,
        bool external)
{
    CollectionElementFlag collection_element_flag = 0;
    set_try_construct_behavior(collection_element_flag, try_construct_kind);
    if (external)
    {
        collection_element_flag |= MemberFlagBits::IS_EXTERNAL;
    }
    return collection_element_flag;
}

StructMemberFlag TypeObjectUtils::build_struct_member_flag(
        TryConstructKind try_construct_kind,
        bool optional,
        bool must_understand,
        bool key,
        bool external)
{
    StructMemberFlag struct_member_flag = 0;
    set_try_construct_behavior(struct_member_flag, try_construct_kind);
    if (optional)
    {
        struct_member_flag |= MemberFlagBits::IS_OPTIONAL;
    }
    if (must_understand)
    {
        struct_member_flag |= MemberFlagBits::IS_MUST_UNDERSTAND;
    }
    if (key)
    {
        struct_member_flag |= MemberFlagBits::IS_KEY;
    }
    if (external)
    {
        struct_member_flag |= MemberFlagBits::IS_EXTERNAL;
    }
    return struct_member_flag;
}

UnionMemberFlag TypeObjectUtils::build_union_member_flag(
        TryConstructKind try_construct_kind,
        bool default_member,
        bool external)
{
    UnionMemberFlag union_member_flag = 0;
    set_try_construct_behavior(union_member_flag, try_construct_kind);
    if (default_member)
    {
        union_member_flag |= MemberFlagBits::IS_DEFAULT;
    }
    if (external)
    {
        union_member_flag |= MemberFlagBits::IS_EXTERNAL;
    }
    return union_member_flag;
}

UnionDiscriminatorFlag TypeObjectUtils::build_union_discriminator_flag(
        TryConstructKind try_construct_kind,
        bool key)
{
    UnionDiscriminatorFlag union_discriminator_flag = 0;
    set_try_construct_behavior(union_discriminator_flag, try_construct_kind);
    if (key)
    {
        union_discriminator_flag |= MemberFlagBits::IS_KEY;
    }
    return union_discriminator_flag;
}

EnumeratedLiteralFlag TypeObjectUtils::build_enumerated_literal_flag(
        bool default_literal)
{
    EnumeratedLiteralFlag enumerated_literal_flag = 0;
    if (default_literal)
    {
        enumerated_literal_flag |= MemberFlagBits::IS_DEFAULT;
    }
    return enumerated_literal_flag;
}

StructTypeFlag TypeObjectUtils::build_struct_type_flag(
        ExtensibilityKind extensibility_kind,
        bool nested,
        bool autoid_hash)
{
    StructTypeFlag struct_type_flag = 0;
    set_type_flag(struct_type_flag, extensibility_kind, nested, autoid_hash);
    return struct_type_flag;
}

UnionTypeFlag TypeObjectUtils::build_union_type_flag(
        ExtensibilityKind extensibility_kind,
        bool nested,
        bool autoid_hash)
{
    UnionTypeFlag union_type_flag = 0;
    set_type_flag(union_type_flag, extensibility_kind, nested, autoid_hash);
    return union_type_flag;
}

const StringSTypeDefn TypeObjectUtils::build_string_s_type_defn(
        SBound bound)
{
    s_bound_consistency(bound);
    StringSTypeDefn string_s_type_defn;
    string_s_type_defn.bound(bound);
    return string_s_type_defn;
}

const StringLTypeDefn TypeObjectUtils::build_string_l_type_defn(
        LBound bound)
{
    l_bound_consistency(bound);
    StringLTypeDefn string_l_type_defn;
    string_l_type_defn.bound(bound);
    return string_l_type_defn;
}

const PlainCollectionHeader TypeObjectUtils::build_plain_collection_header(
        EquivalenceKindValue equiv_kind,
        CollectionElementFlag element_flags)
{
#if !defined(NDEBUG)
    member_flag_consistency(element_flags);
#endif // !defined(NDEBUG)
    PlainCollectionHeader plain_collection_header;
    switch (equiv_kind)
    {
        case EquivalenceKindValue::EK_MINIMAL:
            plain_collection_header.equiv_kind(EK_MINIMAL);
            break;

        case EquivalenceKindValue::EK_COMPLETE:
            plain_collection_header.equiv_kind(EK_COMPLETE);
            break;

        case EquivalenceKindValue::EK_BOTH:
            plain_collection_header.equiv_kind(EK_BOTH);
            break;

        default:
            break;
    }
    plain_collection_header.element_flags(element_flags);
    return plain_collection_header;
}

const PlainSequenceSElemDefn TypeObjectUtils::build_plain_sequence_s_elem_defn(
        const PlainCollectionHeader& header,
        SBound bound,
        const TypeIdentifier& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(element_identifier);
#endif // !defined(NDEBUG)
    s_bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, element_identifier);
    PlainSequenceSElemDefn plain_sequence_s_elem_defn;
    plain_sequence_s_elem_defn.header(header);
    plain_sequence_s_elem_defn.bound(bound);
    plain_sequence_s_elem_defn.element_identifier(element_identifier);
    return plain_sequence_s_elem_defn;
}

const PlainSequenceLElemDefn TypeObjectUtils::build_plain_sequence_l_elem_defn(
        const PlainCollectionHeader& header,
        LBound bound,
        const TypeIdentifier& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(element_identifier);
#endif // !defined(NDEBUG)
    l_bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, element_identifier);
    PlainSequenceLElemDefn plain_sequence_l_elem_defn;
    plain_sequence_l_elem_defn.header(header);
    plain_sequence_l_elem_defn.bound(bound);
    plain_sequence_l_elem_defn.element_identifier(element_identifier);
    return plain_sequence_l_elem_defn;
}

const PlainArraySElemDefn TypeObjectUtils::build_plain_array_s_elem_defn(
        const PlainCollectionHeader& header,
        const SBoundSeq& array_bound_seq,
        const TypeIdentifier& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(element_identifier);
#endif // !defined(NDEBUG)
    s_bound_seq_consistency(array_bound_seq);
    plain_collection_type_identifier_header_consistency(header, element_identifier);
    PlainArraySElemDefn plain_array_s_elem_defn;
    plain_array_s_elem_defn.header(header);
    plain_array_s_elem_defn.array_bound_seq(array_bound_seq);
    plain_array_s_elem_defn.element_identifier(element_identifier);
    return plain_array_s_elem_defn;
}

const PlainArrayLElemDefn TypeObjectUtils::build_plain_array_l_elem_defn(
        const PlainCollectionHeader& header,
        const LBoundSeq& array_bound_seq,
        const TypeIdentifier& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(element_identifier);
#endif // !defined(NDEBUG)
    l_bound_seq_consistency(array_bound_seq);
    plain_collection_type_identifier_header_consistency(header, element_identifier);
    PlainArrayLElemDefn plain_array_l_elem_defn;
    plain_array_l_elem_defn.header(header);
    plain_array_l_elem_defn.array_bound_seq(array_bound_seq);
    plain_array_l_elem_defn.element_identifier(element_identifier);
    return plain_array_l_elem_defn;
}

const PlainMapSTypeDefn TypeObjectUtils::build_plain_map_s_type_defn(
        const PlainCollectionHeader& header,
        const SBound bound,
        const TypeIdentifier& element_identifier,
        const CollectionElementFlag key_flags,
        const TypeIdentifier& key_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(element_identifier);
#endif // !defined(NDEBUG)
    s_bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, element_identifier);
    member_flag_consistency(key_flags);
    map_key_type_identifier_consistency(key_identifier);
    PlainMapSTypeDefn plain_map_s_type_defn;
    plain_map_s_type_defn.header(header);
    plain_map_s_type_defn.bound(bound);
    plain_map_s_type_defn.element_identifier(element_identifier);
    plain_map_s_type_defn.key_flags(key_flags);
    plain_map_s_type_defn.key_identifier(key_identifier);
    return plain_map_s_type_defn;
}

const PlainMapLTypeDefn TypeObjectUtils::build_plain_map_l_type_defn(
        const PlainCollectionHeader& header,
        const LBound bound,
        const TypeIdentifier& element_identifier,
        const CollectionElementFlag key_flags,
        const TypeIdentifier& key_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(element_identifier);
#endif // !defined(NDEBUG)
    l_bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, element_identifier);
    member_flag_consistency(key_flags);
    map_key_type_identifier_consistency(key_identifier);
    PlainMapLTypeDefn plain_map_l_type_defn;
    plain_map_l_type_defn.header(header);
    plain_map_l_type_defn.bound(bound);
    plain_map_l_type_defn.element_identifier(element_identifier);
    plain_map_l_type_defn.key_flags(key_flags);
    plain_map_l_type_defn.key_identifier(key_identifier);
    return plain_map_l_type_defn;
}

void TypeObjectUtils::set_try_construct_behavior(
        MemberFlag& member_flag,
        TryConstructKind try_construct_kind)
{
    switch (try_construct_kind)
    {
        case TryConstructKind::USE_DEFAULT:
            member_flag |= MemberFlagBits::TRY_CONSTRUCT2;
            break;

        case TryConstructKind::TRIM:
            member_flag |= MemberFlagBits::TRY_CONSTRUCT1 | MemberFlagBits::TRY_CONSTRUCT2;
            break;
        
        case TryConstructKind::DISCARD:
        default:
            member_flag |= MemberFlagBits::TRY_CONSTRUCT1;
            break;
    }
}

void TypeObjectUtils::set_type_flag(
        TypeFlag& type_flag,
        ExtensibilityKind extensibility_kind,
        bool nested,
        bool autoid_hash)
{
    set_extensibility_kind(type_flag, extensibility_kind);
    if (nested)
    {
        type_flag |= TypeFlagBits::IS_NESTED;
    }
    if (autoid_hash)
    {
        type_flag |= TypeFlagBits::IS_AUTOID_HASH;
    }
}

void TypeObjectUtils::set_extensibility_kind(
        TypeFlag& type_flag,
        ExtensibilityKind extensibility_kind)
{
    switch (extensibility_kind)
    {
        case ExtensibilityKind::FINAL:
            type_flag |= TypeFlagBits::IS_FINAL;
            break;

        case ExtensibilityKind::MUTABLE:
            type_flag |= TypeFlagBits::IS_MUTABLE;
            break;

        case ExtensibilityKind::APPENDABLE:
        default:
            type_flag |= TypeFlagBits::IS_APPENDABLE;
            break;
    }
}

bool TypeObjectUtils::is_fully_descriptive_type_identifier(
        const TypeIdentifier& type_identifier)
{
    return !is_direct_hash_type_identifier(type_identifier) && !is_indirect_hash_type_identifier(type_identifier);
}

bool TypeObjectUtils::is_direct_hash_type_identifier(
        const TypeIdentifier& type_identifier)
{
    bool direct_hash = false;
    if (type_identifier._d() == EK_MINIMAL ||
        type_identifier._d() == EK_COMPLETE ||
        type_identifier._d() == TI_STRONGLY_CONNECTED_COMPONENT)
    {
        direct_hash = true;
    }
    return direct_hash;
}

bool TypeObjectUtils::is_indirect_hash_type_identifier(
        const TypeIdentifier& type_identifier)
{
    bool indirect_hash = false;
    if ((type_identifier._d() == TI_PLAIN_SEQUENCE_SMALL &&
            type_identifier.seq_sdefn().header().equiv_kind() != EK_BOTH) ||
            (type_identifier._d() == TI_PLAIN_SEQUENCE_LARGE &&
            type_identifier.seq_ldefn().header().equiv_kind() != EK_BOTH) ||
            (type_identifier._d() == TI_PLAIN_ARRAY_SMALL &&
            type_identifier.array_sdefn().header().equiv_kind() != EK_BOTH) ||
            (type_identifier._d() == TI_PLAIN_ARRAY_LARGE &&
            type_identifier.array_ldefn().header().equiv_kind() != EK_BOTH) ||
            (type_identifier._d() == TI_PLAIN_MAP_SMALL &&
            type_identifier.map_sdefn().header().equiv_kind() != EK_BOTH) ||
            (type_identifier._d() == TI_PLAIN_MAP_LARGE &&
            type_identifier.map_ldefn().header().equiv_kind() != EK_BOTH))
    {
        indirect_hash = true;
    }
    return indirect_hash;
}

void TypeObjectUtils::s_bound_consistency(
        SBound bound)
{
    if (INVALID_SBOUND == bound)
    {
        throw InvalidArgumentError("bound parameter must be greater than 0");
    }
}

void TypeObjectUtils::l_bound_consistency(
        LBound bound)
{
    if (bound < 256)
    {
        throw InvalidArgumentError("bound parameter must be greater than 255");
    }
}

void TypeObjectUtils::s_bound_seq_consistency(
        const SBoundSeq& bound_seq)
{
    array_bound_seq_consistency(bound_seq);
    for (SBound bound : bound_seq)
    {
        s_bound_consistency(bound);
    }
}

void TypeObjectUtils::l_bound_seq_consistency(
        const LBoundSeq& bound_seq)
{
    array_bound_seq_consistency(bound_seq);
    bool large_dimension = false;
    for (LBound lbound : bound_seq)
    {
        if (INVALID_LBOUND == lbound)
        {
            throw InvalidArgumentError("bound must always be greater than 0");
        }
        if (lbound > 255)
        {
            large_dimension = true;
        }
    }
    if (!large_dimension)
    {
        throw InvalidArgumentError("no large bound in array_bound_seq");
    }
}

void TypeObjectUtils::member_flag_consistency(
        MemberFlag member_flags)
{
    if (!(member_flags & MemberFlagBits::TRY_CONSTRUCT1 || member_flags & MemberFlagBits::TRY_CONSTRUCT2))
    {
        throw InvalidArgumentError("Inconsistent MemberFlag: INVALID TryConstructKind");
    }
}

void TypeObjectUtils::plain_collection_header_consistency(
        const PlainCollectionHeader& header)
{
    member_flag_consistency(header.element_flags());
    if (header.equiv_kind() != EK_COMPLETE || header.equiv_kind() != EK_MINIMAL || header.equiv_kind() != EK_BOTH)
    {
        throw InvalidArgumentError("Inconsistent PlainCollectionHeader, invalid EquivalenceKind");
    }
}

void TypeObjectUtils::plain_collection_type_identifier_header_consistency(
        const PlainCollectionHeader& header,
        const TypeIdentifier& element_identifier)
{
    if ((header.equiv_kind() != EK_BOTH && header.equiv_kind() != element_identifier._d()) ||
        (header.equiv_kind() == EK_BOTH && !is_fully_descriptive_type_identifier(element_identifier)))
    {
        throw InvalidArgumentError("Inconsistency between given header and element_identifier parameters");
    }
}

void TypeObjectUtils::map_key_type_identifier_consistency(
        const TypeIdentifier& key_identifier)
{
    if (key_identifier._d() != TK_INT8 || key_identifier._d() != TK_UINT8 || key_identifier._d() != TK_INT16 ||
        key_identifier._d() != TK_UINT16 || key_identifier._d() != TK_INT32 || key_identifier._d() != TK_UINT32 ||
        key_identifier._d() != TK_INT64 || key_identifier._d() != TK_UINT64 ||
        key_identifier._d() != TI_STRING8_SMALL || key_identifier._d() != TI_STRING8_LARGE ||
        key_identifier._d() != TI_STRING16_SMALL || key_identifier._d() != TI_STRING16_LARGE)
    {
        throw InvalidArgumentError(
            "Inconsistent key identifier: only signed/unsigned integer types and w/string keys are supported");
    }
#if !defined(NDEBUG)
    type_identifier_consistency(key_identifier);
#endif // !defined(NDEBUG)
}

void TypeObjectUtils::string_sdefn_consistency(
        const StringSTypeDefn& string)
{
    s_bound_consistency(string.bound());
}

void TypeObjectUtils::string_ldefn_consistency(
        const StringLTypeDefn& string)
{
    l_bound_consistency(string.bound());
}

void TypeObjectUtils::seq_sdefn_consistency(
        const PlainSequenceSElemDefn& plain_seq)
{
    plain_collection_header_consistency(plain_seq.header());
    s_bound_consistency(plain_seq.bound());
    type_identifier_consistency(plain_seq.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_seq.header(), plain_seq.element_identifier());
}

void TypeObjectUtils::seq_ldefn_consistency(
        const PlainSequenceLElemDefn& plain_seq)
{
    plain_collection_header_consistency(plain_seq.header());
    l_bound_consistency(plain_seq.bound());
    type_identifier_consistency(plain_seq.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_seq.header(), plain_seq.element_identifier());
}

void TypeObjectUtils::array_sdefn_consistency(
        const PlainArraySElemDefn& plain_array)
{
    plain_collection_header_consistency(plain_array.header());
    s_bound_seq_consistency(plain_array.array_bound_seq());
    type_identifier_consistency(plain_array.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_array.header(), plain_array.element_identifier());
}

void TypeObjectUtils::array_ldefn_consistency(
        const PlainArrayLElemDefn& plain_array)
{
    plain_collection_header_consistency(plain_array.header());
    l_bound_seq_consistency(plain_array.array_bound_seq());
    type_identifier_consistency(plain_array.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_array.header(), plain_array.element_identifier());
}

void TypeObjectUtils::map_sdefn_consistency(
        const PlainMapSTypeDefn& plain_map)
{
    plain_collection_header_consistency(plain_map.header());
    s_bound_consistency(plain_map.bound());
    type_identifier_consistency(plain_map.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_map.header(), plain_map.element_identifier());
    member_flag_consistency(plain_map.key_flags());
    map_key_type_identifier_consistency(plain_map.key_identifier());
}

void TypeObjectUtils::map_ldefn_consistency(
        const PlainMapLTypeDefn& plain_map)
{
    plain_collection_header_consistency(plain_map.header());
    l_bound_consistency(plain_map.bound());
    type_identifier_consistency(plain_map.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_map.header(), plain_map.element_identifier());
    member_flag_consistency(plain_map.key_flags());
    map_key_type_identifier_consistency(plain_map.key_identifier());
}

void TypeObjectUtils::type_identifier_consistency(
        const TypeIdentifier& type_identifier)
{
    switch (type_identifier._d())
    {
        case TK_NONE:
            throw InvalidArgumentError("Inconsistent TypeIdentifier: non-initialized");

        case TI_STRING8_SMALL:
        case TI_STRING16_SMALL:
            string_sdefn_consistency(type_identifier.string_sdefn());
            break;

        case TI_STRING8_LARGE:
        case TI_STRING16_LARGE:
            string_ldefn_consistency(type_identifier.string_ldefn());
            break;

        case TI_PLAIN_SEQUENCE_SMALL:
            seq_sdefn_consistency(type_identifier.seq_sdefn());
            break;

        case TI_PLAIN_SEQUENCE_LARGE:
            seq_ldefn_consistency(type_identifier.seq_ldefn());
            break;

        case TI_PLAIN_ARRAY_SMALL:
            array_sdefn_consistency(type_identifier.array_sdefn());
            break;

        case TI_PLAIN_ARRAY_LARGE:
            array_ldefn_consistency(type_identifier.array_ldefn());
            break;

        case TI_PLAIN_MAP_SMALL:
            map_sdefn_consistency(type_identifier.map_sdefn());
            break;

        case TI_PLAIN_MAP_LARGE:
            map_ldefn_consistency(type_identifier.map_ldefn());

        case TI_STRONGLY_CONNECTED_COMPONENT:
            // TODO(jlbueno)

        case EK_COMPLETE:
        case EK_MINIMAL:
            // TODO(jlbueno)

        // Primitive TypeIdentifiers
        default:
            break;
    }
}

} // xtypes1_3
} // dds
} // fastdds
} // eprosima
