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

#include <fastdds/dds/xtypes/type_representation/TypeObjectRegistry.hpp>

#include <string>

#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

bool TypeRegistryEntry::operator !=(
        const TypeRegistryEntry& entry)
{
    return this->type_object_ != entry.type_object_ ||
        this->type_object_serialized_size_ != entry.type_object_serialized_size_;
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const std::string& type_name,
        const CompleteTypeObject& complete_type_object)
{
    if (type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#if !defined(NDEBUG)
    try
    {
        TypeObjectUtils::complete_type_object_consistency(complete_type_object);
    }
    catch (eprosima::fastdds::dds::xtypes::InvalidArgumentError& exception)
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Inconsistent CompleteTypeObject: " << exception.what());
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#endif // !defined(NDEBUG)
    TypeRegistryEntry complete_entry;
    TypeRegistryEntry minimal_entry;
    complete_entry.type_object_.complete(complete_type_object);
    minimal_entry.type_object_ = build_minimal_from_complete_type_object(complete_type_object);
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1(get_type_identifier(minimal_entry.type_object_,
        minimal_entry.type_object_serialized_size_));
    type_ids.type_identifier2(get_type_identifier(complete_entry.type_object_,
        complete_entry.type_object_serialized_size_));
    auto type_ids_result = local_type_identifiers_.insert({type_name, type_ids});
    auto min_entry_result = type_registry_entries_.insert({type_ids.type_identifier1(), minimal_entry});
    auto max_entry_result = type_registry_entries_.insert({type_ids.type_identifier2(), complete_entry});
    if (!type_ids_result.second || !min_entry_result.second || !max_entry_result.second)
    {
        if (local_type_identifiers_[type_name] != type_ids ||
            type_registry_entries_[type_ids.type_identifier1()] != minimal_entry ||
            type_registry_entries_[type_ids.type_identifier2()] != complete_entry)
        {
            if (type_ids_result.second)
            {
                local_type_identifiers_.erase(type_name);
            }
            if (min_entry_result.second)
            {
                type_registry_entries_.erase(type_ids.type_identifier1());
            }
            if (max_entry_result.second)
            {
                type_registry_entries_.erase(type_ids.type_identifier2());
            }
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::register_type_identifier(
        const std::string& type_name,
        const TypeIdentifier& type_identifier)
{
    // Preconditions
    if (TypeObjectUtils::is_direct_hash_type_identifier(type_identifier) || type_name.empty())
    {
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#if !defined(NDEBUG)
    try
    {
        TypeObjectUtils::type_identifier_consistency(type_identifier);
    }
    catch (eprosima::fastdds::dds::xtypes::InvalidArgumentError& exception)
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Inconsistent TypeIdentifier: " << exception.what());
        return eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET;
    }
#endif // !defined(NDEBUG)
    TypeIdentifierPair type_identifiers;
    type_identifiers.type_identifier1(type_identifier);
    auto result = local_type_identifiers_.insert({type_name, type_identifiers});
    if (!result.second)
    {
        if (local_type_identifiers_[type_name] != type_identifiers)
        {
            return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER;
        }
    }
    return eprosima::fastdds::dds::RETCODE_OK;
}

ReturnCode_t TypeObjectRegistry::get_type_objects(
        const std::string& type_name,
        const TypeObjectPair& type_objects)
{
    static_cast<void>(type_name);
    static_cast<void>(type_objects);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_identifiers(
        const std::string& type_name,
        const TypeIdentifierPair& type_identifiers)
{
    static_cast<void>(type_name);
    static_cast<void>(type_identifiers);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::register_type_object(
        const TypeIdentifier& type_identifier,
        const TypeObject& type_object)
{
    static_cast<void>(type_identifier);
    static_cast<void>(type_object);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_object(
        const TypeIdentifier& type_identifier,
        TypeObjectPair& type_objects)
{
    static_cast<void>(type_identifier);
    static_cast<void>(type_objects);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_information(
        const std::string& type_name,
        TypeInformation& type_information)
{
    static_cast<void>(type_name);
    static_cast<void>(type_information);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::are_types_compatible(
        const TypeIdentifierPair& type_identifiers,
        const TypeConsistencyEnforcementQosPolicy& type_consistency_qos)
{
    static_cast<void>(type_identifiers);
    static_cast<void>(type_consistency_qos);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::get_type_dependencies(
        const TypeIdentifierSeq& type_identifiers,
        std::unordered_set<TypeIdentfierWithSize> type_dependencies)
{
    static_cast<void>(type_identifiers);
    static_cast<void>(type_dependencies);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeObjectRegistry::is_type_identifier_known(
        const TypeIdentifier& type_identifier)
{
    static_cast<void>(type_identifier);
    return eprosima::fastdds::dds::RETCODE_UNSUPPORTED;
}

bool TypeObjectRegistry::is_builtin_annotation(
        const TypeIdentifier& type_identifier)
{
    static_cast<void>(type_identifier);
    return false;
}

const TypeIdentifier TypeObjectRegistry::get_type_identifier(
        const TypeObject& type_object,
        uint32_t& type_object_serialized_size)
{
    static_cast<void>(type_object);
    static_cast<void>(type_object_serialized_size);
    return TypeIdentifier();
}

const TypeObject TypeObjectRegistry::build_minimal_from_complete_type_object(
        const CompleteTypeObject& complete_type_object)
{
    MinimalTypeObject minimal_type_object;
    switch (complete_type_object._d())
    {
        case TK_ALIAS:
        {
            MinimalAliasType minimal_alias_type = build_minimal_from_complete_alias_type(
                complete_type_object.alias_type());
            minimal_type_object.alias_type(minimal_alias_type);
            break;
        }
        case TK_ANNOTATION:
        {
            MinimalAnnotationType minimal_annotation_type = build_minimal_from_complete_annotation_type(
                complete_type_object.annotation_type());
            minimal_type_object.annotation_type(minimal_annotation_type);
            break;
        }
        case TK_STRUCTURE:
        {
            MinimalStructType minimal_struct_type = build_minimal_from_complete_struct_type(
                complete_type_object.struct_type());
            minimal_type_object.struct_type(minimal_struct_type);
            break;
        }
        case TK_UNION:
        {
            MinimalUnionType minimal_union_type = build_minimal_from_complete_union_type(
                complete_type_object.union_type());
            minimal_type_object.union_type(minimal_union_type);
            break;
        }
        case TK_BITSET:
        {
            MinimalBitsetType minimal_bitset_type = build_minimal_from_complete_bitset_type(
                complete_type_object.bitset_type());
            minimal_type_object.bitset_type(minimal_bitset_type);
            break;
        }
        case TK_SEQUENCE:
        {
            MinimalSequenceType minimal_sequence_type = build_minimal_from_complete_sequence_type(
                complete_type_object.sequence_type());
            minimal_type_object.sequence_type(minimal_sequence_type);
            break;
        }
        case TK_ARRAY:
        {
            MinimalArrayType minimal_array_type = build_minimal_from_complete_array_type(
                complete_type_object.array_type());
            minimal_type_object.array_type(minimal_array_type);
            break;
        }
        case TK_MAP:
        {
            MinimalMapType minimal_map_type = build_minimal_from_complete_map_type(
                complete_type_object.map_type());
            minimal_type_object.map_type(minimal_map_type);
            break;
        }
        case TK_ENUM:
        {
            MinimalEnumeratedType minimal_enumerated_type = build_minimal_from_complete_enumerated_type(
                complete_type_object.enumerated_type());
            minimal_type_object.enumerated_type(minimal_enumerated_type);
            break;
        }
        case TK_BITMASK:
        {
            MinimalBitmaskType minimal_bitmask_type = build_minimal_from_complete_bitmask_type(
                complete_type_object.bitmask_type());
            minimal_type_object.bitmask_type(minimal_bitmask_type);
            break;
        }
    }
    TypeObject type_object;
    type_object.minimal(minimal_type_object);
    return type_object;
}

const MinimalAliasType TypeObjectRegistry::build_minimal_from_complete_alias_type(
        const CompleteAliasType& complete_alias_type)
{
    MinimalAliasType minimal_alias_type;
    // alias_flags: unused. No flags apply.
    // header: empty. Available for future extension.
    minimal_alias_type.body().common(complete_alias_type.body().common());
    return minimal_alias_type;
}

const MinimalAnnotationType TypeObjectRegistry::build_minimal_from_complete_annotation_type(
        const CompleteAnnotationType& complete_annotation_type)
{
    MinimalAnnotationType minimal_annotation_type;
    // annotation_flag: unused. No flags apply.
    // header: empty. Available for future extension.
    MinimalAnnotationParameterSeq minimal_annotation_parameter_sequence;
    for (const CompleteAnnotationParameter& complete_annotation_parameter : complete_annotation_type.member_seq())
    {
        MinimalAnnotationParameter minimal_annotation_parameter;
        minimal_annotation_parameter.common(complete_annotation_parameter.common());
        minimal_annotation_parameter.name_hash(TypeObjectUtils::name_hash(
            complete_annotation_parameter.name().c_str()));
        minimal_annotation_parameter.default_value(complete_annotation_parameter.default_value());
        minimal_annotation_parameter_sequence.push_back(minimal_annotation_parameter);
    }
    minimal_annotation_type.member_seq(minimal_annotation_parameter_sequence);
    return minimal_annotation_type;
}

const MinimalStructType TypeObjectRegistry::build_minimal_from_complete_struct_type(
        const CompleteStructType& complete_struct_type)
{
    MinimalStructType minimal_struct_type;
    minimal_struct_type.struct_flags(complete_struct_type.struct_flags());
    minimal_struct_type.header().base_type(complete_struct_type.header().base_type());
    // header().detail: empty. Available for future extension.
    MinimalStructMemberSeq minimal_struct_member_sequence;
    for (const CompleteStructMember& complete_struct_member : complete_struct_type.member_seq())
    {
        MinimalStructMember minimal_struct_member;
        minimal_struct_member.common(complete_struct_member.common());
        minimal_struct_member.detail().name_hash(TypeObjectUtils::name_hash(
            complete_struct_member.detail().name().c_str()));
        minimal_struct_member_sequence.push_back(minimal_struct_member);
    }
    minimal_struct_type.member_seq(minimal_struct_member_sequence);
    return minimal_struct_type;
}

const MinimalUnionType TypeObjectRegistry::build_minimal_from_complete_union_type(
        const CompleteUnionType& complete_union_type)
{
    MinimalUnionType minimal_union_type;
    minimal_union_type.union_flags(complete_union_type.union_flags());
    // header: empty. Available for future extension.
    minimal_union_type.discriminator().common(complete_union_type.discriminator().common());
    MinimalUnionMemberSeq minimal_union_member_sequence;
    for (const CompleteUnionMember& complete_union_member : complete_union_type.member_seq())
    {
        MinimalUnionMember minimal_union_member;
        minimal_union_member.common(complete_union_member.common());
        minimal_union_member.detail().name_hash(TypeObjectUtils::name_hash(
            complete_union_member.detail().name().c_str()));
        minimal_union_member_sequence.push_back(minimal_union_member);
    }
    minimal_union_type.member_seq(minimal_union_member_sequence);
    return minimal_union_type;
}

const MinimalBitsetType TypeObjectRegistry::build_minimal_from_complete_bitset_type(
        const CompleteBitsetType& complete_bitset_type)
{
    MinimalBitsetType minimal_bitset_type;
    // bitset_flags: unused. No flags apply.
    // header: empty. Available for future extension.
    MinimalBitfieldSeq minimal_bitfield_sequence;
    for (const CompleteBitfield& complete_bitfield : complete_bitset_type.field_seq())
    {
        MinimalBitfield minimal_bitfield;
        minimal_bitfield.common(complete_bitfield.common());
        minimal_bitfield.name_hash(TypeObjectUtils::name_hash(
            complete_bitfield.detail().name().c_str()));
        minimal_bitfield_sequence.push_back(minimal_bitfield);
    }
    minimal_bitset_type.field_seq(minimal_bitfield_sequence);
    return minimal_bitset_type;
}

const MinimalSequenceType TypeObjectRegistry::build_minimal_from_complete_sequence_type(
        const CompleteSequenceType& complete_sequence_type)
{
    MinimalSequenceType minimal_sequence_type;
    // collection_flag: unused. No flags apply.
    minimal_sequence_type.header().common(complete_sequence_type.header().common());
    minimal_sequence_type.element().common(complete_sequence_type.element().common());
    return minimal_sequence_type;
}

const MinimalArrayType TypeObjectRegistry::build_minimal_from_complete_array_type(
        const CompleteArrayType& complete_array_type)
{
    MinimalArrayType minimal_array_type;
    // collection_flag: unused. No flags apply.
    minimal_array_type.header().common(complete_array_type.header().common());
    minimal_array_type.element().common(complete_array_type.element().common());
    return minimal_array_type;
}

const MinimalMapType TypeObjectRegistry::build_minimal_from_complete_map_type(
        const CompleteMapType& complete_map_type)
{
    MinimalMapType minimal_map_type;
    // collection_flag: unused. No flags apply.
    minimal_map_type.header().common(complete_map_type.header().common());
    minimal_map_type.key().common(complete_map_type.key().common());
    minimal_map_type.element().common(complete_map_type.element().common());
    return minimal_map_type;
}

const MinimalEnumeratedType TypeObjectRegistry::build_minimal_from_complete_enumerated_type(
        const CompleteEnumeratedType& complete_enumerated_type)
{
    MinimalEnumeratedType minimal_enumerated_type;
    // enum_flags: unused. No flags apply.
    minimal_enumerated_type.header().common(complete_enumerated_type.header().common());
    MinimalEnumeratedLiteralSeq minimal_enumerated_literal_sequence;
    for (const CompleteEnumeratedLiteral& complete_enumerated_literal : complete_enumerated_type.literal_seq())
    {
        MinimalEnumeratedLiteral minimal_enumerated_literal;
        minimal_enumerated_literal.common(complete_enumerated_literal.common());
        minimal_enumerated_literal.detail().name_hash(TypeObjectUtils::name_hash(
            complete_enumerated_literal.detail().name().c_str()));
        minimal_enumerated_literal_sequence.push_back(minimal_enumerated_literal);
    }
    minimal_enumerated_type.literal_seq(minimal_enumerated_literal_sequence);
    return minimal_enumerated_type;
}

const MinimalBitmaskType TypeObjectRegistry::build_minimal_from_complete_bitmask_type(
        const CompleteBitmaskType& complete_bitmask_type)
{
    MinimalBitmaskType minimal_bitmask_type;
    // bitmask_flags: unused. No flags apply.
    minimal_bitmask_type.header().common(complete_bitmask_type.header().common());
    MinimalBitflagSeq minimal_bitflag_sequence;
    for (const CompleteBitflag& complete_bitflag : complete_bitmask_type.flag_seq())
    {
        MinimalBitflag minimal_bitflag;
        minimal_bitflag.common(complete_bitflag.common());
        minimal_bitflag.detail().name_hash(TypeObjectUtils::name_hash(
            complete_bitflag.detail().name().c_str()));
        minimal_bitflag_sequence.push_back(minimal_bitflag);
    }
    minimal_bitmask_type.flag_seq(minimal_bitflag_sequence);
    return minimal_bitmask_type;
}

} // xtypes
} // dds
} // fastdds
} // eprosima
