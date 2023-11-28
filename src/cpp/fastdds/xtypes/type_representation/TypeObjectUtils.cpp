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

#include <cassert>
#include <string>
#include <set>
#include <unordered_set>

#include <fastcdr/exceptions/BadParamException.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/exception/Exception.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

constexpr const char* begin_declaration_file = "begin-declaration-file";
constexpr const char* before_declaration = "before-declaration";
constexpr const char* begin_declaration = "begin-declaration";
constexpr const char* end_declaration = "end-declaration";
constexpr const char* after_declaration = "after-declaration";
constexpr const char* end_declaration_file = "end-declaration-file";
constexpr const char* value_member_name = "value";
constexpr const char* platform_member_name = "platform";
constexpr const EnumeratedLiteralFlag enum_literal_flag_mask = MemberFlagBits::TRY_CONSTRUCT1 |
    MemberFlagBits::TRY_CONSTRUCT2 | MemberFlagBits::IS_EXTERNAL | MemberFlagBits::IS_OPTIONAL |
    MemberFlagBits::IS_MUST_UNDERSTAND | MemberFlagBits::IS_KEY;

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
        // XTypes v1.3 Clause 7.2.2.4.4.4.8 Key members shall always have their "must understand" attribute set to true. 
        struct_member_flag |= MemberFlagBits::IS_KEY;
        struct_member_flag |= MemberFlagBits::IS_MUST_UNDERSTAND;
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
    bound_consistency(bound);
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
        case EquivalenceKindValue::MINIMAL:
            plain_collection_header.equiv_kind(EK_MINIMAL);
            break;

        case EquivalenceKindValue::COMPLETE:
            plain_collection_header.equiv_kind(EK_COMPLETE);
            break;

        case EquivalenceKindValue::BOTH:
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
        const eprosima::fastcdr::external<TypeIdentifier>& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(*element_identifier);
#endif // !defined(NDEBUG)
    bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, *element_identifier);
    PlainSequenceSElemDefn plain_sequence_s_elem_defn;
    plain_sequence_s_elem_defn.header(header);
    plain_sequence_s_elem_defn.bound(bound);
    plain_sequence_s_elem_defn.element_identifier(element_identifier);
    return plain_sequence_s_elem_defn;
}

const PlainSequenceLElemDefn TypeObjectUtils::build_plain_sequence_l_elem_defn(
        const PlainCollectionHeader& header,
        LBound bound,
        const eprosima::fastcdr::external<TypeIdentifier>& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(*element_identifier);
#endif // !defined(NDEBUG)
    l_bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, *element_identifier);
    PlainSequenceLElemDefn plain_sequence_l_elem_defn;
    plain_sequence_l_elem_defn.header(header);
    plain_sequence_l_elem_defn.bound(bound);
    plain_sequence_l_elem_defn.element_identifier(element_identifier);
    return plain_sequence_l_elem_defn;
}

const PlainArraySElemDefn TypeObjectUtils::build_plain_array_s_elem_defn(
        const PlainCollectionHeader& header,
        const SBoundSeq& array_bound_seq,
        const eprosima::fastcdr::external<TypeIdentifier>& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(*element_identifier);
#endif // !defined(NDEBUG)
    array_bound_seq_consistency(array_bound_seq);
    plain_collection_type_identifier_header_consistency(header, *element_identifier);
    PlainArraySElemDefn plain_array_s_elem_defn;
    plain_array_s_elem_defn.header(header);
    plain_array_s_elem_defn.array_bound_seq(array_bound_seq);
    plain_array_s_elem_defn.element_identifier(element_identifier);
    return plain_array_s_elem_defn;
}

const PlainArrayLElemDefn TypeObjectUtils::build_plain_array_l_elem_defn(
        const PlainCollectionHeader& header,
        const LBoundSeq& array_bound_seq,
        const eprosima::fastcdr::external<TypeIdentifier>& element_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(*element_identifier);
#endif // !defined(NDEBUG)
    l_bound_seq_consistency(array_bound_seq);
    plain_collection_type_identifier_header_consistency(header, *element_identifier);
    PlainArrayLElemDefn plain_array_l_elem_defn;
    plain_array_l_elem_defn.header(header);
    plain_array_l_elem_defn.array_bound_seq(array_bound_seq);
    plain_array_l_elem_defn.element_identifier(element_identifier);
    return plain_array_l_elem_defn;
}

const PlainMapSTypeDefn TypeObjectUtils::build_plain_map_s_type_defn(
        const PlainCollectionHeader& header,
        const SBound bound,
        const eprosima::fastcdr::external<TypeIdentifier>& element_identifier,
        const CollectionElementFlag key_flags,
        const eprosima::fastcdr::external<TypeIdentifier>& key_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(*element_identifier);
#endif // !defined(NDEBUG)
    bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, *element_identifier);
    member_flag_consistency(key_flags);
    map_key_type_identifier_consistency(*key_identifier);
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
        const eprosima::fastcdr::external<TypeIdentifier>& element_identifier,
        const CollectionElementFlag key_flags,
        const eprosima::fastcdr::external<TypeIdentifier>& key_identifier)
{
#if !defined(NDEBUG)
    plain_collection_header_consistency(header);
    type_identifier_consistency(*element_identifier);
#endif // !defined(NDEBUG)
    l_bound_consistency(bound);
    plain_collection_type_identifier_header_consistency(header, *element_identifier);
    member_flag_consistency(key_flags);
    map_key_type_identifier_consistency(*key_identifier);
    PlainMapLTypeDefn plain_map_l_type_defn;
    plain_map_l_type_defn.header(header);
    plain_map_l_type_defn.bound(bound);
    plain_map_l_type_defn.element_identifier(element_identifier);
    plain_map_l_type_defn.key_flags(key_flags);
    plain_map_l_type_defn.key_identifier(key_identifier);
    return plain_map_l_type_defn;
}

const StronglyConnectedComponentId TypeObjectUtils::build_strongly_connected_component_id(
        const TypeObjectHashId& sc_component_id,
        int32_t scc_length,
        int32_t scc_index)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "StronglyConnectedComponents not yet supported.");
    StronglyConnectedComponentId scc_id;
    scc_id.sc_component_id(sc_component_id);
    scc_id.scc_length(scc_length);
    scc_id.scc_index(scc_index);
    return scc_id;
}

const ExtendedTypeDefn TypeObjectUtils::build_extended_type_defn()
{
    ExtendedTypeDefn extended_type_defn;
    return extended_type_defn;
}

ReturnCode_t TypeObjectUtils::build_and_register_s_string_type_identifier(
        const StringSTypeDefn& string,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    string_sdefn_consistency(string);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.string_sdefn(string);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_l_string_type_identifier(
        const StringLTypeDefn& string,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    string_ldefn_consistency(string);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.string_ldefn(string);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_s_sequence_type_identifier(
        const PlainSequenceSElemDefn& plain_seq,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    seq_sdefn_consistency(plain_seq);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.seq_sdefn(plain_seq);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_l_sequence_type_identifier(
        const PlainSequenceLElemDefn& plain_seq,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    seq_ldefn_consistency(plain_seq);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.seq_ldefn(plain_seq);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_s_array_type_identifier(
        const PlainArraySElemDefn& plain_array,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    array_sdefn_consistency(plain_array);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.array_sdefn(plain_array);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_l_array_type_identifier(
        const PlainArrayLElemDefn& plain_array,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    array_ldefn_consistency(plain_array);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.array_ldefn(plain_array);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_s_map_type_identifier(
        const PlainMapSTypeDefn& plain_map,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    map_sdefn_consistency(plain_map);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.map_sdefn(plain_map);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_l_map_type_identifier(
        const PlainMapLTypeDefn& plain_map,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    map_ldefn_consistency(plain_map);
#endif // !defined(NDEBUG)
    TypeIdentifier type_identifier;
    type_identifier.map_ldefn(plain_map);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier(type_name,
        type_identifier);
}

ReturnCode_t TypeObjectUtils::build_and_register_scc_type_identifier(
        const StronglyConnectedComponentId& scc,
        const std::string& type_name)
{
/*
    TypeIdentifier type_identifier;
    type_identifier.sc_component_id(scc);
    return DomainParticipantFactory::get_instance()->type_object_registry()->register_type_identifier(type_name,
        type_identifier);
*/
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "StronglyConnectedComponents not yet supported.");
    static_cast<void>(scc);
    static_cast<void>(type_name);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

const ExtendedAnnotationParameterValue TypeObjectUtils::build_extended_annotation_parameter_value()
{
    ExtendedAnnotationParameterValue extended_annotation_parameter_value;
    return extended_annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        bool boolean_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.boolean_value(boolean_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value_byte(
        uint8_t byte_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.byte_value(byte_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int8_t int8_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.int8_value(int8_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint8_t uint8_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.uint8_value(uint8_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int16_t int16_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.int16_value(int16_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint16_t uint16_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.uint_16_value(uint16_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int32_t int32_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.int32_value(int32_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint32_t uint32_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.uint32_value(uint32_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int64_t int64_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.int64_value(int64_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint64_t uint64_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.uint64_value(uint64_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        float float32_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.float32_value(float32_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        double float64_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.float64_value(float64_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        long double float128_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.float128_value(float128_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        char char_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.char_value(char_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        wchar_t wchar_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.wchar_value(wchar_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value_enum(
        int32_t enumerated_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.enumerated_value(enumerated_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        const eprosima::fastcdr::fixed_string<128>& string8_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.string8_value(string8_value);
    return annotation_parameter_value;
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
       const std::wstring& string16_value)
{
    AnnotationParameterValue annotation_parameter_value;
    annotation_parameter_value.string16_value(string16_value);
    return annotation_parameter_value;
}

const AppliedAnnotationParameter TypeObjectUtils::build_applied_annotation_parameter(
        const NameHash& paramname_hash,
        const AnnotationParameterValue& value)
{
    AppliedAnnotationParameter applied_annotation_parameter;
    applied_annotation_parameter.paramname_hash(paramname_hash);
    applied_annotation_parameter.value(value);
    return applied_annotation_parameter;
}

void TypeObjectUtils::add_applied_annotation_parameter(
        AppliedAnnotationParameterSeq& param_seq,
        const AppliedAnnotationParameter& param)
{
#if !defined(NDEBUG)
    for (AppliedAnnotationParameter parameter : param_seq)
    {
        if (parameter.paramname_hash() == param.paramname_hash())
        {
            throw InvalidArgumentError("Annotation parameter already applied");
        }
    }
#endif // !defined(NDEBUG)
    param_seq.push_back(param);
}

const AppliedAnnotation TypeObjectUtils::build_applied_annotation(
        const TypeIdentifier& annotation_typeid,
        const eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>& param_seq)
{
#if !defined(NDEBUG)
    applied_annotation_type_identifier_consistency(annotation_typeid);
    if (param_seq.has_value())
    {
        applied_annotation_parameter_seq_consistency(param_seq.value());
    }
    if (DomainParticipantFactory::get_instance()->type_object_registry().is_builtin_annotation(annotation_typeid))
    {
        throw InvalidArgumentError("Found builtin annotation in custom annotation sequence");
    }
#endif // !defined(NDEBUG)
    AppliedAnnotation applied_annotation;
    applied_annotation.annotation_typeid(annotation_typeid);
    applied_annotation.param_seq(param_seq);
    return applied_annotation;
}

void TypeObjectUtils::add_applied_annotation(
        AppliedAnnotationSeq& ann_custom_seq,
        const AppliedAnnotation& ann_custom)
{
#if !defined(NDEBUG)
    applied_annotation_consistency(ann_custom);
    for (AppliedAnnotation annotation : ann_custom_seq)
    {
        if (annotation.annotation_typeid() == ann_custom.annotation_typeid())
        {
            throw InvalidArgumentError("Annotation already applied");
        }
    }
#endif // !defined(NDEBUG)
    ann_custom_seq.push_back(ann_custom);
}

const AppliedVerbatimAnnotation TypeObjectUtils::build_applied_verbatim_annotation(
        PlacementKindValue placement,
        const eprosima::fastcdr::fixed_string<32>& language,
        const std::string& text)
{
    AppliedVerbatimAnnotation applied_verbatim_annotation;
    switch (placement)
    {
        case PlacementKindValue::AFTER_DECLARATION:
            applied_verbatim_annotation.placement(after_declaration);
            break;
        case PlacementKindValue::BEFORE_DECLARATION:
            applied_verbatim_annotation.placement(before_declaration);
            break;
        case PlacementKindValue::BEGIN_DECLARATION:
            applied_verbatim_annotation.placement(begin_declaration);
            break;
        case PlacementKindValue::BEGIN_FILE:
            applied_verbatim_annotation.placement(begin_declaration_file);
            break;
        case PlacementKindValue::END_DECLARATION:
            applied_verbatim_annotation.placement(end_declaration);
            break;
        case PlacementKindValue::END_FILE:
            applied_verbatim_annotation.placement(end_declaration_file);
            break;
        default:
            break;
    }
    applied_verbatim_annotation.language(language);
    applied_verbatim_annotation.text(text);
    return applied_verbatim_annotation;
}

const AppliedBuiltinMemberAnnotations TypeObjectUtils::build_applied_builtin_member_annotations(
        const eprosima::fastcdr::optional<std::string>& unit,
        const eprosima::fastcdr::optional<AnnotationParameterValue>& min,
        const eprosima::fastcdr::optional<AnnotationParameterValue>& max,
        const eprosima::fastcdr::optional<std::string>& hash_id)
{
    AppliedBuiltinMemberAnnotations applied_builtin_member_annotations;
    applied_builtin_member_annotations.unit(unit);
    if (min.has_value())
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "@min annotation not yet supported.");
    }
    if (max.has_value())
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "@max annotation not yet supported.");
    }
    applied_builtin_member_annotations.hash_id(hash_id);
    return applied_builtin_member_annotations;
}

const CommonStructMember TypeObjectUtils::build_common_struct_member(
        MemberId member_id,
        StructMemberFlag member_flags,
        const TypeIdentifier& member_type_id)
{
#if !defined(NDEBUG)
    struct_member_flag_consistency(member_flags);
    type_identifier_consistency(member_type_id);
#endif // !defined(NDEBUG)
    CommonStructMember common_struct_member;
    common_struct_member.member_id(member_id);
    common_struct_member.member_flags(member_flags);
    common_struct_member.member_type_id(member_type_id);
    return common_struct_member;
}

const CompleteMemberDetail TypeObjectUtils::build_complete_member_detail(
        const MemberName& name,
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom)
{
#if !defined(NDEBUG)
    if (ann_custom.has_value())
    {
        applied_annotation_seq_consistency(ann_custom.value());
    }
#endif // !defined(NDEBUG)
    if (name.size() == 0)
    {
        throw InvalidArgumentError("Member name cannot be empty");
    }
    CompleteMemberDetail complete_member_detail;
    complete_member_detail.name(name);
    complete_member_detail.ann_builtin(ann_builtin);
    complete_member_detail.ann_custom(ann_custom);
    return complete_member_detail;
}

const CompleteStructMember TypeObjectUtils::build_complete_struct_member(
        const CommonStructMember& common,
        const CompleteMemberDetail& detail)
{
#if !defined(NDEBUG)
    common_struct_member_consistency(common);
    complete_member_detail_consistency(detail);
#endif
    common_struct_member_and_complete_member_detail_consistency(common, detail);
    CompleteStructMember complete_struct_member;
    complete_struct_member.common(common);
    complete_struct_member.detail(detail);
    return complete_struct_member;
}

void TypeObjectUtils::add_complete_struct_member(
        CompleteStructMemberSeq& member_seq,
        const CompleteStructMember& member)
{
#if !defined(NDEBUG)
    complete_struct_member_consistency(member);
    for (CompleteStructMember struct_member : member_seq)
    {
        if (struct_member.common().member_id() == member.common().member_id() ||
            struct_member.detail().name() == member.detail().name())
        {
            throw InvalidArgumentError("Sequence has another member with same ID/name");
        }
    }
#endif // !defined(NDEBUG)
    member_seq.push_back(member);
}

const AppliedBuiltinTypeAnnotations TypeObjectUtils::build_applied_builtin_type_annotations(
        const eprosima::fastcdr::optional<AppliedVerbatimAnnotation>& verbatim)
{
#if !defined(NDEBUG)
    if (verbatim.has_value())
    {
        applied_verbatim_annotation_consistency(verbatim.value());
    }
#endif // !defined(NDEBUG)
    AppliedBuiltinTypeAnnotations applied_builtin_type_annotations;
    applied_builtin_type_annotations.verbatim(verbatim);
    return applied_builtin_type_annotations;
}

const CompleteTypeDetail TypeObjectUtils::build_complete_type_detail(
        const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& ann_builtin,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom,
        const QualifiedTypeName& type_name)
{
    if (type_name.size() == 0)
    {
        throw InvalidArgumentError("QualifiedTypeName cannot be empty");
    }
#if !defined(NDEBUG)
    if (ann_builtin.has_value())
    {
        applied_builtin_type_annotations_consistency(ann_builtin.value());
    }
    if (ann_custom.has_value())
    {
        applied_annotation_seq_consistency(ann_custom.value());
    }
#endif // !defined(NDEBUG)
    CompleteTypeDetail complete_type_detail;
    complete_type_detail.ann_builtin(ann_builtin);
    complete_type_detail.ann_custom(ann_custom);
    complete_type_detail.type_name(type_name);
    return complete_type_detail;
}

const CompleteStructHeader TypeObjectUtils::build_complete_struct_header(
        const TypeIdentifier& base_type,
        const CompleteTypeDetail& detail)
{
    if (base_type._d() != TK_NONE && !is_direct_hash_type_identifier(base_type))
    {
        throw InvalidArgumentError("Base TypeIdentifier must be direct HASH TypeIdentifier");
    }
#if !defined(NDEBUG)
    if (base_type._d() != TK_NONE)
    {
        structure_base_type_consistency(base_type);
    }
    complete_type_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteStructHeader complete_struct_header;
    complete_struct_header.base_type(base_type);
    complete_struct_header.detail(detail);
    return complete_struct_header;
}

const CompleteStructType TypeObjectUtils::build_complete_struct_type(
        StructTypeFlag struct_flags,
        const CompleteStructHeader& header,
        const CompleteStructMemberSeq& member_seq)
{
#if !defined(NDEBUG)
    type_flag_consistency(struct_flags);
    complete_struct_header_consistency(header);
    complete_struct_member_seq_consistency(member_seq);
#endif // !defined(NDEBUG)
    CompleteStructType complete_struct_type;
    complete_struct_type.struct_flags(struct_flags);
    complete_struct_type.header(header);
    complete_struct_type.member_seq(member_seq);
    return complete_struct_type;
}

void TypeObjectUtils::add_union_case_label(
        UnionCaseLabelSeq& label_seq,
        int32_t label)
{
    for (int32_t label_ : label_seq)
    {
        if (label == label_)
        {
            // Not fail but not add repeated member.
            return;
        }
    }
    label_seq.push_back(label);
}

const CommonUnionMember TypeObjectUtils::build_common_union_member(
        MemberId member_id,
        UnionMemberFlag member_flags,
        const TypeIdentifier& type_id,
        const UnionCaseLabelSeq& label_seq)
{
#if !defined(NDEBUG)
    member_flag_consistency(member_flags);
    type_identifier_consistency(type_id);
    union_case_label_seq_consistency(label_seq);
#endif // !defined(NDEBUG)
    if (!(member_flags & MemberFlagBits::IS_DEFAULT) && label_seq.size() == 0)
    {
        throw InvalidArgumentError("Non default members require at least one associated case");
    }
    CommonUnionMember common_union_member;
    common_union_member.member_id(member_id);
    common_union_member.member_flags(member_flags);
    common_union_member.type_id(type_id);
    common_union_member.label_seq(label_seq);
    return common_union_member;
}

const CompleteUnionMember TypeObjectUtils::build_complete_union_member(
        const CommonUnionMember& common,
        const CompleteMemberDetail& detail)
{
#if !defined(NDEBUG)
    common_union_member_consistency(common);
    complete_member_detail_consistency(detail);
#endif // !defined(NDEBUG)
    common_union_member_complete_member_detail_consistency(common, detail);
    CompleteUnionMember complete_union_member;
    complete_union_member.common(common);
    complete_union_member.detail(detail);
    return complete_union_member;
}

void TypeObjectUtils::add_complete_union_member(
        CompleteUnionMemberSeq& complete_union_member_seq,
        const CompleteUnionMember& member)
{
#if !defined(NDEBUG)
    complete_union_member_consistency(member);
    std::set<int32_t> case_labels;
    // New member labels are ensured to be unique (complete_union_member_consistency)
    for (int32_t label : member.common().label_seq())
    {
        case_labels.insert(label);
    }
    size_t case_labels_length = member.common().label_seq().size();
    for (CompleteUnionMember union_member : complete_union_member_seq)
    {
        if (union_member.common().member_id() == member.common().member_id() ||
            union_member.detail().name() == member.detail().name())
        {
            throw InvalidArgumentError("Sequence has another member with same ID/name");
        }
        if (member.common().member_flags() & MemberFlagBits::IS_DEFAULT &&
            union_member.common().member_flags() & MemberFlagBits::IS_DEFAULT)
        {
            throw InvalidArgumentError("Union member sequence already has a default member");
        }
        for (size_t i = 0; i < union_member.common().label_seq().size(); i++)
        {
            case_labels.insert(union_member.common().label_seq()[i]);
            if (case_labels.size() != case_labels_length + i + 1)
            {
                throw InvalidArgumentError("Repeated union case label");
            }
        }
        case_labels_length += union_member.common().label_seq().size();
    }
#endif // !defined(NDEBUG)
    complete_union_member_seq.push_back(member);
}

const CommonDiscriminatorMember TypeObjectUtils::build_common_discriminator_member(
        UnionDiscriminatorFlag member_flags,
        const TypeIdentifier& type_id)
{
#if !defined(NDEBUG)
    member_flag_consistency(member_flags);
#endif
    common_discriminator_member_type_identifier_consistency(type_id);
    CommonDiscriminatorMember common_discriminator_member;
    common_discriminator_member.member_flags(member_flags);
    common_discriminator_member.type_id(type_id);
    return common_discriminator_member;
}

const CompleteDiscriminatorMember TypeObjectUtils::build_complete_discriminator_member(
        const CommonDiscriminatorMember& common,
        const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& ann_builtin,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom)
{
#if !defined(NDEBUG)
    common_discriminator_member_consistency(common);
    if (ann_builtin.has_value())
    {
        applied_builtin_type_annotations_consistency(ann_builtin.value());
    }
    if (ann_custom.has_value())
    {
        applied_annotation_seq_consistency(ann_custom.value());
    }
#endif // !defined(NDEBUG)
    CompleteDiscriminatorMember complete_discriminator_member;
    complete_discriminator_member.common(common);
    complete_discriminator_member.ann_builtin(ann_builtin);
    complete_discriminator_member.ann_custom(ann_custom);
    return complete_discriminator_member;
}

const CompleteUnionHeader TypeObjectUtils::build_complete_union_header(
        const CompleteTypeDetail& detail)
{
#if !defined(NDEBUG)
    complete_type_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteUnionHeader complete_union_header;
    complete_union_header.detail(detail);
    return complete_union_header;
}

const CompleteUnionType TypeObjectUtils::build_complete_union_type(
        UnionTypeFlag union_flags,
        const CompleteUnionHeader& header,
        const CompleteDiscriminatorMember& discriminator,
        const CompleteUnionMemberSeq& member_seq)
{
#if !defined(NDEBUG)
    type_flag_consistency(union_flags);
    complete_union_header_consistency(header);
    complete_discriminator_member_consistency(discriminator);
    complete_union_member_seq_consistency(member_seq);
#endif // !defined(NDEBUG)
    CompleteUnionType complete_union_type;
    complete_union_type.union_flags(union_flags);
    complete_union_type.header(header);
    complete_union_type.discriminator(discriminator);
    complete_union_type.member_seq(member_seq);
    return complete_union_type;
}

const CommonAnnotationParameter TypeObjectUtils::build_common_annotation_parameter(
        AnnotationParameterFlag member_flags,
        const TypeIdentifier& member_type_id)
{
#if !defined(NDEBUG)
    type_identifier_consistency(member_type_id);
#endif
    empty_flags_consistency(member_flags);
    CommonAnnotationParameter common_annotation_parameter;
    common_annotation_parameter.member_flags(member_flags);
    common_annotation_parameter.member_type_id(member_type_id);
    return common_annotation_parameter;
}

const CompleteAnnotationParameter TypeObjectUtils::build_complete_annotation_parameter(
        const CommonAnnotationParameter& common,
        const MemberName& name,
        const AnnotationParameterValue& default_value)
{
#if !defined(NDEBUG)
    common_annotation_parameter_consistency(common);
#endif // !defined(NDEBUG)
    common_annotation_parameter_type_identifier_default_value_consistency(common.member_type_id(), default_value);
    if (name.size() == 0)
    {
        throw InvalidArgumentError("Annotation parameter name cannot be empty");
    }
    CompleteAnnotationParameter complete_annotation_parameter;
    complete_annotation_parameter.common(common);
    complete_annotation_parameter.name(name);
    complete_annotation_parameter.default_value(default_value);
    return complete_annotation_parameter;
}

void TypeObjectUtils::add_complete_annotation_parameter(
        CompleteAnnotationParameterSeq& sequence,
        const CompleteAnnotationParameter& param)
{
#if !defined(NDEBUG)
    complete_annotation_parameter_consistency(param);
    for (CompleteAnnotationParameter ann_param : sequence)
    {
        if (ann_param.name() == param.name())
        {
            throw InvalidArgumentError("Sequence has another parameter with same name");
        }
    }
#endif
    sequence.push_back(param);
}

const CompleteAnnotationHeader TypeObjectUtils::build_complete_annotation_header(
        const QualifiedTypeName& annotation_name)
{
    if (annotation_name.size() == 0)
    {
        throw InvalidArgumentError("QualifiedTypeName cannot be empty");
    }
    CompleteAnnotationHeader complete_annotation_header;
    complete_annotation_header.annotation_name(annotation_name);
    return complete_annotation_header;
}

const CompleteAnnotationType TypeObjectUtils::build_complete_annotation_type(
        AnnotationTypeFlag annotation_flag,
        const CompleteAnnotationHeader& header,
        const CompleteAnnotationParameterSeq& member_seq)
{
    empty_flags_consistency(annotation_flag);
#if !defined(NDEBUG)
    complete_annotation_header_consistency(header);
    complete_annotation_parameter_seq_consistency(member_seq);
#endif // !defined(NDEBUG)
    CompleteAnnotationType complete_annotation_type;
    complete_annotation_type.annotation_flag(annotation_flag);
    complete_annotation_type.header(header);
    complete_annotation_type.member_seq(member_seq);
    return complete_annotation_type;
}

const CommonAliasBody TypeObjectUtils::build_common_alias_body(
        AliasMemberFlag related_flags,
        const TypeIdentifier& related_type)
{
    empty_flags_consistency(related_flags);
#if !defined(NDEBUG)
    type_identifier_consistency(related_type);
#endif // !defined(NDEBUG)
    CommonAliasBody common_alias_body;
    common_alias_body.related_flags(related_flags);
    common_alias_body.related_type(related_type);
    return common_alias_body;
}

const CompleteAliasBody TypeObjectUtils::build_complete_alias_body(
        const CommonAliasBody& common,
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom)
{
#if !defined(NDEBUG)
    common_alias_body_consistency(common);
    if (ann_custom.has_value())
    {
        applied_annotation_seq_consistency(ann_custom.value());
    }
#endif // !defined(NDEBUG)
    hashid_builtin_annotation_not_applied_consistency(ann_builtin);
    CompleteAliasBody complete_alias_body;
    complete_alias_body.common(common);
    complete_alias_body.ann_builtin(ann_builtin);
    complete_alias_body.ann_custom(ann_custom);
    return complete_alias_body;
}

const CompleteAliasHeader TypeObjectUtils::build_complete_alias_header(
        const CompleteTypeDetail& detail)
{
#if !defined(NDEBUG)
    complete_type_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteAliasHeader complete_alias_header;
    complete_alias_header.detail(detail);
    return complete_alias_header;
}

const CompleteAliasType TypeObjectUtils::build_complete_alias_type(
        AliasTypeFlag alias_flags,
        const CompleteAliasHeader& header,
        const CompleteAliasBody& body)
{
    empty_flags_consistency(alias_flags);
#if !defined(NDEBUG)
    complete_alias_header_consistency(header);
    complete_alias_body_consistency(body);
#endif // !defined(NDEBUF)
    CompleteAliasType complete_alias_type;
    complete_alias_type.alias_flags(alias_flags);
    complete_alias_type.header(header);
    complete_alias_type.body(body);
    return complete_alias_type;
}

const CompleteElementDetail TypeObjectUtils::build_complete_element_detail(
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& ann_custom)
{
#if !defined(NDEBUG)
    if (ann_custom.has_value())
    {
        applied_annotation_seq_consistency(ann_custom.value());
    }
#endif // !defined(NDEBUG)
    hashid_builtin_annotation_not_applied_consistency(ann_builtin);
    CompleteElementDetail complete_element_detail;
    complete_element_detail.ann_builtin(ann_builtin);
    complete_element_detail.ann_custom(ann_custom);
    return complete_element_detail;
}

const CommonCollectionElement TypeObjectUtils::build_common_collection_element(
        CollectionElementFlag element_flags,
        const TypeIdentifier& type)
{
#if !defined(NDEBUG)
    member_flag_consistency(element_flags);
    type_identifier_consistency(type);
#endif // !defined(NDEBUG)
    CommonCollectionElement common_collection_element;
    common_collection_element.element_flags(element_flags);
    common_collection_element.type(type);
    return common_collection_element;
}

const CompleteCollectionElement TypeObjectUtils::build_complete_collection_element(
        const CommonCollectionElement& common,
        const CompleteElementDetail& detail)
{
#if !defined(NDEBUG)
    common_collection_element_consistency(common);
    complete_element_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteCollectionElement complete_collection_element;
    complete_collection_element.common(common);
    complete_collection_element.detail(detail);
    return complete_collection_element;
}

const CommonCollectionHeader TypeObjectUtils::build_common_collection_header(
        LBound bound)
{
    bound_consistency(bound);
    CommonCollectionHeader common_collection_header;
    common_collection_header.bound(bound);
    return common_collection_header;
}

const CompleteCollectionHeader TypeObjectUtils::build_complete_collection_header(
        const CommonCollectionHeader& common,
        const eprosima::fastcdr::optional<CompleteTypeDetail>& detail)
{
#if !defined(NDEBUG)
    common_collection_header_consistency(common);
    if (detail.has_value())
    {
        complete_type_detail_consistency(detail.value());
    }
#endif // !defined(NDEBUG)
    CompleteCollectionHeader complete_collection_header;
    complete_collection_header.common(common);
    complete_collection_header.detail(detail);
    return complete_collection_header;
}

const CompleteSequenceType TypeObjectUtils::build_complete_sequence_type(
        CollectionTypeFlag collection_flag,
        const CompleteCollectionHeader& header,
        const CompleteCollectionElement& element)
{
    empty_flags_consistency(collection_flag);
#if !defined(NDEBUG)
    complete_collection_header_consistency(header);
    complete_collection_element_consistency(element);
#endif // !defined(NDEBUG)
    CompleteSequenceType complete_sequence_type;
    complete_sequence_type.collection_flag(collection_flag);
    complete_sequence_type.header(header);
    complete_sequence_type.element(element);
    return complete_sequence_type;
}

const CommonArrayHeader TypeObjectUtils::build_common_array_header(
        const LBoundSeq& bound_seq)
{
    array_bound_seq_consistency(bound_seq);
    CommonArrayHeader common_array_header;
    common_array_header.bound_seq(bound_seq);
    return common_array_header;
}

const CompleteArrayHeader TypeObjectUtils::build_complete_array_header(
        const CommonArrayHeader& common,
        const CompleteTypeDetail& detail)
{
#if !defined(NDEBUG)
    common_array_header_consistency(common);
    complete_type_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteArrayHeader complete_array_header;
    complete_array_header.common(common);
    complete_array_header.detail(detail);
    return complete_array_header;
}

const CompleteArrayType TypeObjectUtils::build_complete_array_type(
        CollectionTypeFlag collection_flag,
        const CompleteArrayHeader& header,
        const CompleteCollectionElement& element)
{
    empty_flags_consistency(collection_flag);
#if !defined(NDEBUG)
    complete_array_header_consistency(header);
    complete_collection_element_consistency(element);
#endif // !defined(NDEBUG)
    CompleteArrayType complete_array_type;
    complete_array_type.collection_flag(collection_flag);
    complete_array_type.header(header);
    complete_array_type.element(element);
    return complete_array_type;
}

const CompleteMapType TypeObjectUtils::build_complete_map_type(
        CollectionElementFlag collection_flag,
        const CompleteCollectionHeader& header,
        const CompleteCollectionElement& key,
        const CompleteCollectionElement& element)
{
    empty_flags_consistency(collection_flag);
#if !defined(NDEBUG)
    complete_collection_header_consistency(header);
    complete_collection_element_consistency(key);
    complete_collection_element_consistency(element);
#endif // !defined(NDEBUG)
    map_key_type_identifier_consistency(key.common().type());
    CompleteMapType complete_map_type;
    complete_map_type.collection_flag(collection_flag);
    complete_map_type.header(header);
    complete_map_type.key(key);
    complete_map_type.element(element);
    return complete_map_type;
}

const CommonEnumeratedLiteral TypeObjectUtils::build_common_enumerated_literal(
        int32_t value,
        EnumeratedLiteralFlag flags)
{
#if !defined(NDEBUG)
    enumerated_literal_flag_consistency(flags);
#endif // !defined(NDEBUG)
    CommonEnumeratedLiteral common_enumerated_literal;
    common_enumerated_literal.value(value);
    common_enumerated_literal.flags(flags);
    return common_enumerated_literal;
}

const CompleteEnumeratedLiteral TypeObjectUtils::build_complete_enumerated_literal(
        const CommonEnumeratedLiteral& common,
        const CompleteMemberDetail& detail)
{
#if !defined(NDEBUG)
    common_enumerated_literal_consistency(common);
    complete_member_detail_consistency(detail);
#endif // !defined(NDEBUG)
    if (detail.ann_builtin().has_value())
    {
        throw InvalidArgumentError("Only @default_literal and @value builtin annotations apply to enum literals");
    }
    CompleteEnumeratedLiteral complete_enumerated_literal;
    complete_enumerated_literal.common(common);
    complete_enumerated_literal.detail(detail);
    return complete_enumerated_literal;
}

void TypeObjectUtils::add_complete_enumerated_literal(
        CompleteEnumeratedLiteralSeq& sequence,
        const CompleteEnumeratedLiteral& enum_literal)
{
#if !defined(NDEBUG)
    complete_enumerated_literal_consistency(enum_literal);
    // TODO(jlbueno): check uniqueness
#endif // !defined(NDEBUG)
    sequence.push_back(enum_literal);
}

const CommonEnumeratedHeader TypeObjectUtils::build_common_enumerated_header(
        BitBound bit_bound,
        bool bitmask)
{
    if (bitmask)
    {
        bitmask_bit_bound_consistency(bit_bound);
    }
    else
    {
        enum_bit_bound_consistency(bit_bound);
    }
    CommonEnumeratedHeader common_enumerated_header;
    common_enumerated_header.bit_bound(bit_bound);
    return common_enumerated_header;
}

const CompleteEnumeratedHeader TypeObjectUtils::build_complete_enumerated_header(
        const CommonEnumeratedHeader& common,
        const CompleteTypeDetail& detail,
        bool bitmask)
{
#if !defined(NDEBUG)
    common_enumerated_header_consistency(common, bitmask);
    complete_type_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteEnumeratedHeader complete_enumerated_header;
    complete_enumerated_header.common(common);
    complete_enumerated_header.detail(detail);
    return complete_enumerated_header;
}

const CompleteEnumeratedType TypeObjectUtils::build_complete_enumerated_type(
        EnumTypeFlag enum_flags,
        const CompleteEnumeratedHeader& header,
        const CompleteEnumeratedLiteralSeq& literal_seq)
{
    empty_flags_consistency(enum_flags);
#if !defined(NDEBUG)
    complete_enumerated_header_consistency(header);
    complete_enumerated_literal_seq_consistency(literal_seq);
#endif // !defined(NDEBUG)
    CompleteEnumeratedType complete_enumerated_type;
    complete_enumerated_type.enum_flags(enum_flags);
    complete_enumerated_type.header(header);
    complete_enumerated_type.literal_seq(literal_seq);
    return complete_enumerated_type;
}

const CommonBitflag TypeObjectUtils::build_common_bitflag(
        uint16_t position,
        BitflagFlag flags)
{
    bit_position_consistency(position);
    empty_flags_consistency(flags);
    CommonBitflag common_bitflag;
    common_bitflag.position(position);
    common_bitflag.flags(flags);
    return common_bitflag;
}

const CompleteBitflag TypeObjectUtils::build_complete_bitflag(
        const CommonBitflag& common,
        const CompleteMemberDetail& detail)
{
#if !defined(NDEBUG)
    common_bitflag_consistency(common);
    complete_member_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteBitflag complete_bitflag;
    complete_bitflag.common(common);
    complete_bitflag.detail(detail);
    return complete_bitflag;
}

void TypeObjectUtils::add_complete_bitflag(
        CompleteBitflagSeq& sequence,
        const CompleteBitflag& bitflag)
{
#if !defined(NDEBUG)
    complete_bitflag_consistency(bitflag);
    // TODO(jlbueno): check uniqueness
#endif // !defined(NDEBUG)
    sequence.push_back(bitflag);
}

const CompleteBitmaskType TypeObjectUtils::build_complete_bitmask_type(
        BitmaskTypeFlag bitmask_flags,
        const CompleteBitmaskHeader& header,
        const CompleteBitflagSeq& flag_seq)
{
    empty_flags_consistency(bitmask_flags);
#if !defined(NDEBUG)
    complete_enumerated_header_consistency(header, true);
    complete_bitflag_seq_consistency(flag_seq);
#endif // !defined(NDEBUG)
    CompleteBitmaskType complete_bitmask_type;
    complete_bitmask_type.bitmask_flags(bitmask_flags);
    complete_bitmask_type.header(header);
    complete_bitmask_type.flag_seq(flag_seq);
    return complete_bitmask_type;
}

const CommonBitfield TypeObjectUtils::build_common_bitfield(
        uint16_t position,
        BitsetMemberFlag flags,
        uint8_t bitcount,
        TypeKind holder_type)
{
    bit_position_consistency(position);
    empty_flags_consistency(flags);
    bitmask_bit_bound_consistency(bitcount);
    bitmask_bit_bound_consistency(bitcount + position);
    bitfield_holder_type_consistency(holder_type, bitcount);
    CommonBitfield common_bitfield;
    common_bitfield.position(position);
    common_bitfield.flags(flags);
    common_bitfield.bitcount(bitcount);
    common_bitfield.holder_type(holder_type);
    return common_bitfield;
}

const CompleteBitfield TypeObjectUtils::build_complete_bitfield(
        const CommonBitfield& common,
        const CompleteMemberDetail& detail)
{
#if !defined(NDEBUG)
    common_bitfield_consistency(common);
    complete_member_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteBitfield complete_bitfield;
    complete_bitfield.common(common);
    complete_bitfield.detail(detail);
    return complete_bitfield;
}

void TypeObjectUtils::add_complete_bitfield(
        CompleteBitfieldSeq& sequence,
        const CompleteBitfield& bitfield)
{
#if !defined(NDEBUG)
    complete_bitfield_consistency(bitfield);
    // TODO(jlbueno): check uniqueness and no overlapping.
#endif // !defined(NDEBUG)
    sequence.push_back(bitfield);
}

const CompleteBitsetHeader TypeObjectUtils::build_complete_bitset_header(
        const CompleteTypeDetail& detail)
{
#if !defined(NDEBUG)
    complete_type_detail_consistency(detail);
#endif // !defined(NDEBUG)
    CompleteBitsetHeader complete_bitset_header;
    complete_bitset_header.detail(detail);
    return complete_bitset_header;
}

const CompleteBitsetType TypeObjectUtils::build_complete_bitset_type(
        BitsetTypeFlag bitset_flags,
        const CompleteBitsetHeader& header,
        const CompleteBitfieldSeq& field_seq)
{
    empty_flags_consistency(bitset_flags);
#if !defined(NDEBUG)
    complete_bitset_header_consistency(header);
    complete_bitfield_seq_consistency(field_seq);
#endif // !defined(NDEBUG)
    CompleteBitsetType complete_bitset_type;
    complete_bitset_type.bitset_flags(bitset_flags);
    complete_bitset_type.header(header);
    complete_bitset_type.field_seq(field_seq);
    return complete_bitset_type;
}

const CompleteExtendedType TypeObjectUtils::build_complete_extended_type()
{
    CompleteExtendedType complete_extended_type;
    return complete_extended_type;
}

ReturnCode_t TypeObjectUtils::build_and_register_alias_type_object(
        const CompleteAliasType& alias_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_alias_type_consistency(alias_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.alias_type(alias_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_annotation_type_object(
        const CompleteAnnotationType& annotation_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_annotation_type_consistency(annotation_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.annotation_type(annotation_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_struct_type_object(
        const CompleteStructType& struct_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_struct_type_consistency(struct_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.struct_type(struct_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_union_type_object(
        const CompleteUnionType& union_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_union_type_consistency(union_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.union_type(union_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_bitset_type_object(
        const CompleteBitsetType& bitset_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_bitset_type_consistency(bitset_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.bitset_type(bitset_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_sequence_type_object(
        const CompleteSequenceType& sequence_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_sequence_type_consistency(sequence_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.sequence_type(sequence_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_array_type_object(
        const CompleteArrayType& array_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_array_type_consistency(array_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.array_type(array_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_map_type_object(
        const CompleteMapType& map_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_map_type_consistency(map_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.map_type(map_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_enumerated_type_object(
        const CompleteEnumeratedType& enumerated_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_enumerated_type_consistency(enumerated_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.enumerated_type(enumerated_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

ReturnCode_t TypeObjectUtils::build_and_register_bitmask_type_object(
        const CompleteBitmaskType& bitmask_type,
        const std::string& type_name)
{
#if !defined(NDEBUG)
    complete_bitmask_type_consistency(bitmask_type);
#endif // !defined(NDEBUG)
    CompleteTypeObject type_object;
    type_object.bitmask_type(bitmask_type);
    return DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(type_name,
        type_object);
}

const NameHash TypeObjectUtils::name_hash(
        const std::string& name)
{
    NameHash name_hashed;
    MD5 hash(name);
    for (size_t i = 0; i < name_hashed.size(); i++)
    {
        name_hashed[i] = hash.digest[i];
    }
    return name_hashed;
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

void TypeObjectUtils::l_bound_consistency(
        LBound bound)
{
    if (bound < 256)
    {
        throw InvalidArgumentError("bound parameter must be greater than 255");
    }
}

void TypeObjectUtils::l_bound_seq_consistency(
        const LBoundSeq& bound_seq)
{
    if (bound_seq.empty())
    {
        throw InvalidArgumentError("array_bound_seq parameter must not be empty");
    }
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

void TypeObjectUtils::struct_member_flag_consistency(
        StructMemberFlag member_flags)
{
    member_flag_consistency(member_flags);
    if (member_flags & MemberFlagBits::IS_KEY && member_flags & MemberFlagBits::IS_OPTIONAL)
    {
        throw InvalidArgumentError("Keyed members cannot be optional");
    }
    if (member_flags & MemberFlagBits::IS_KEY && !(member_flags & MemberFlagBits::IS_MUST_UNDERSTAND))
    {
        throw InvalidArgumentError("Keyed members must have their \"must understand\" attribute set to true");
    }
}

void TypeObjectUtils::type_flag_consistency(
        TypeFlag type_flag)
{
    if ((type_flag & TypeFlagBits::IS_APPENDABLE && type_flag & TypeFlagBits::IS_FINAL) ||
        (type_flag & TypeFlagBits::IS_APPENDABLE && type_flag & TypeFlagBits::IS_MUTABLE) ||
        (type_flag & TypeFlagBits::IS_FINAL && type_flag & TypeFlagBits::IS_MUTABLE))
    {
        throw InvalidArgumentError("Exactly one extensibility flag must be set");
    }
}

void TypeObjectUtils::enumerated_literal_flag_consistency(
        EnumeratedLiteralFlag enumerated_literal_flag)
{
    if ((enumerated_literal_flag & enum_literal_flag_mask) != 0)
    {
        throw InvalidArgumentError("Only default flag applies to enumerated literals");
    }
}

void TypeObjectUtils::plain_collection_header_consistency(
        const PlainCollectionHeader& header)
{
    member_flag_consistency(header.element_flags());
    if (header.equiv_kind() != EK_COMPLETE && header.equiv_kind() != EK_MINIMAL && header.equiv_kind() != EK_BOTH)
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
    if (key_identifier._d() != TK_INT8 && key_identifier._d() != TK_UINT8 && key_identifier._d() != TK_INT16 &&
        key_identifier._d() != TK_UINT16 && key_identifier._d() != TK_INT32 && key_identifier._d() != TK_UINT32 &&
        key_identifier._d() != TK_INT64 && key_identifier._d() != TK_UINT64 &&
        key_identifier._d() != TI_STRING8_SMALL && key_identifier._d() != TI_STRING8_LARGE &&
        key_identifier._d() != TI_STRING16_SMALL && key_identifier._d() != TI_STRING16_LARGE)
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
    bound_consistency(string.bound());
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
    bound_consistency(plain_seq.bound());
    type_identifier_consistency(*plain_seq.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_seq.header(), *plain_seq.element_identifier());
}

void TypeObjectUtils::seq_ldefn_consistency(
        const PlainSequenceLElemDefn& plain_seq)
{
    plain_collection_header_consistency(plain_seq.header());
    l_bound_consistency(plain_seq.bound());
    type_identifier_consistency(*plain_seq.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_seq.header(), *plain_seq.element_identifier());
}

void TypeObjectUtils::array_sdefn_consistency(
        const PlainArraySElemDefn& plain_array)
{
    plain_collection_header_consistency(plain_array.header());
    array_bound_seq_consistency(plain_array.array_bound_seq());
    type_identifier_consistency(*plain_array.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_array.header(), *plain_array.element_identifier());
}

void TypeObjectUtils::array_ldefn_consistency(
        const PlainArrayLElemDefn& plain_array)
{
    plain_collection_header_consistency(plain_array.header());
    l_bound_seq_consistency(plain_array.array_bound_seq());
    type_identifier_consistency(*plain_array.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_array.header(), *plain_array.element_identifier());
}

void TypeObjectUtils::map_sdefn_consistency(
        const PlainMapSTypeDefn& plain_map)
{
    plain_collection_header_consistency(plain_map.header());
    bound_consistency(plain_map.bound());
    type_identifier_consistency(*plain_map.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_map.header(), *plain_map.element_identifier());
    member_flag_consistency(plain_map.key_flags());
    map_key_type_identifier_consistency(*plain_map.key_identifier());
}

void TypeObjectUtils::map_ldefn_consistency(
        const PlainMapLTypeDefn& plain_map)
{
    plain_collection_header_consistency(plain_map.header());
    l_bound_consistency(plain_map.bound());
    type_identifier_consistency(*plain_map.element_identifier());
    plain_collection_type_identifier_header_consistency(plain_map.header(), *plain_map.element_identifier());
    member_flag_consistency(plain_map.key_flags());
    map_key_type_identifier_consistency(*plain_map.key_identifier());
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
            break;

        case TI_STRONGLY_CONNECTED_COMPONENT:
            // No inconsistency rule apply.
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "StronglyConnectedComponents not yet supported.");
            break;

        case EK_COMPLETE:
        case EK_MINIMAL:
            // TODO(jlbueno)

        // Primitive TypeIdentifiers/ExtendedTypeDefn: no inconsistency rule apply.
        default:
            break;
    }
}

void TypeObjectUtils::applied_annotation_parameter_seq_consistency(
        const AppliedAnnotationParameterSeq& applied_annotation_parameter_seq)
{
    std::set<NameHash> param_hashes;
    for (size_t i; i < applied_annotation_parameter_seq.size(); i++)
    {
        param_hashes.insert(applied_annotation_parameter_seq[i].paramname_hash());
        if (param_hashes.size() != (i + 1))
        {
            throw InvalidArgumentError("Repeated annotation parameters in the sequence");
        }
    }
}

void TypeObjectUtils::applied_annotation_type_identifier_consistency(
        const TypeIdentifier& annotation_type_id)
{
    TypeObjectPair type_objects;
    ReturnCode_t ret_code = DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
        annotation_type_id, type_objects);
    if (ReturnCode_t::RETCODE_OK == ret_code)
    {
        if (type_objects.complete_type_object._d() != TK_ANNOTATION)
        {
            throw InvalidArgumentError("AppliedAnnotation TypeIdentifier does not correspond with an Annotation type");
        }
    }
    else if (ReturnCode_t::RETCODE_NO_DATA == ret_code)
    {
        throw InvalidArgumentError("Annotation TypeIdentifier unknown to TypeObjectRegistry");
    }
    else
    {
        throw InvalidArgumentError("Annotation TypeIdentifier is not direct HASH");
    }
}

void TypeObjectUtils::applied_annotation_consistency(
        const AppliedAnnotation& applied_annotation)
{
    applied_annotation_type_identifier_consistency(applied_annotation.annotation_typeid());
    if (applied_annotation.param_seq().has_value())
    {
        applied_annotation_parameter_seq_consistency(applied_annotation.param_seq().value());
    }
    if (DomainParticipantFactory::get_instance()->type_object_registry().is_builtin_annotation(
        applied_annotation.annotation_typeid()))
    {
        throw InvalidArgumentError("Builtin annotation cannot be defined as custom annotation");
    }
}

void TypeObjectUtils::applied_annotation_seq_consistency(
        const AppliedAnnotationSeq& applied_annotation_seq)
{
    std::unordered_set<TypeIdentifier> annotation_typeids;
    for (size_t i = 0; i < applied_annotation_seq.size(); i++)
    {
        annotation_typeids.insert(applied_annotation_seq[i].annotation_typeid());
        if (annotation_typeids.size() != (i + 1))
        {
            throw InvalidArgumentError("Repeated annotation in the sequence");
        }
        applied_annotation_consistency(applied_annotation_seq[i]);
    }
}

void TypeObjectUtils::applied_verbatim_annotation_consistency(
        const AppliedVerbatimAnnotation& applied_verbatim_annotation)
{
    // Placement
    if (applied_verbatim_annotation.placement().compare(begin_declaration_file) != 0 &&
        applied_verbatim_annotation.placement().compare(before_declaration) != 0 &&
        applied_verbatim_annotation.placement().compare(begin_declaration) != 0 &&
        applied_verbatim_annotation.placement().compare(end_declaration) != 0 &&
        applied_verbatim_annotation.placement().compare(after_declaration) != 0 &&
        applied_verbatim_annotation.placement().compare(end_declaration_file) != 0)
    {
        throw InvalidArgumentError("Verbatim annotation placement unknown");
    }
    // Language: IDL v4.2 The defined values are as follows: "c", "c++", "java", "idl", "*". Note - Other values may be
    // used when relevant. Consequently no consistency check is done with this parameter.
    // Text
    if (applied_verbatim_annotation.text().empty())
    {
        throw InvalidArgumentError("Verbatim annotation requires text parameter");
    }
}

void TypeObjectUtils::common_struct_member_consistency(
        const CommonStructMember& common_struct_member)
{
    struct_member_flag_consistency(common_struct_member.member_flags());
    type_identifier_consistency(common_struct_member.member_type_id());
}

void TypeObjectUtils::complete_member_detail_consistency(
        const CompleteMemberDetail& complete_member_detail)
{
    if (complete_member_detail.name().size() == 0)
    {
        throw InvalidArgumentError("Member name cannot be empty");
    }
    if (complete_member_detail.ann_custom().has_value())
    {
        applied_annotation_seq_consistency(complete_member_detail.ann_custom().value());
    }
}

void TypeObjectUtils::common_struct_member_and_complete_member_detail_consistency(
        const CommonStructMember& common_struct_member,
        const CompleteMemberDetail& complete_member_detail)
{
    // Check @hashid consistency with MemberId
    if (complete_member_detail.ann_builtin().has_value() &&
        complete_member_detail.ann_builtin().value().hash_id().has_value())
    {
        std::string string_value;
        // If the annotation [@hashid] is used without any parameter or with the empty string as a parameter, then the
        // Member ID shall be computed from the member name.
        if (complete_member_detail.ann_builtin().value().hash_id().value().empty())
        {
            string_value = complete_member_detail.name();
        }
        else
        {
            string_value = complete_member_detail.ann_builtin().value().hash_id().value();
        }
        string_member_id_consistency(common_struct_member.member_id(), string_value);
    }
}

void TypeObjectUtils::string_member_id_consistency(
        MemberId member_id,
        const std::string& string_value)
{
    NameHash hash = name_hash(string_value);
    if (member_id != ((hash[3] << 24 | hash[2] << 16 | hash[1] << 8 | hash[0]) & 0x0FFFFFFF))
    {
        throw InvalidArgumentError("Inconsistent member id coming from a string value");
    }
}

void TypeObjectUtils::complete_struct_member_consistency(
        const CompleteStructMember& complete_struct_member)
{
    common_struct_member_consistency(complete_struct_member.common());
    complete_member_detail_consistency(complete_struct_member.detail());
    common_struct_member_and_complete_member_detail_consistency(complete_struct_member.common(),
        complete_struct_member.detail());
}

void TypeObjectUtils::complete_struct_member_seq_consistency(
        const CompleteStructMemberSeq& complete_struct_member_seq)
{
    std::set<MemberId> member_ids;
    std::set<MemberName> member_names;
    for (size_t i = 0; i < complete_struct_member_seq.size(); i++)
    {
        member_ids.insert(complete_struct_member_seq[i].common().member_id());
        member_names.insert(complete_struct_member_seq[i].detail().name());
        if ((member_ids.size() != (i + 1)) || (member_names.size() != (i + 1)))
        {
            throw InvalidArgumentError("Repeated member id/name in the sequence");
        }
        complete_struct_member_consistency(complete_struct_member_seq[i]);
    }
}

void TypeObjectUtils::applied_builtin_type_annotations_consistency(
        const AppliedBuiltinTypeAnnotations& applied_builtin_type_annotations)
{
    if (applied_builtin_type_annotations.verbatim().has_value())
    {
        applied_verbatim_annotation_consistency(applied_builtin_type_annotations.verbatim().value());
    }
}

void TypeObjectUtils::complete_type_detail_consistency(
        const CompleteTypeDetail& complete_type_detail)
{
    if (complete_type_detail.type_name().size() == 0)
    {
        throw InvalidArgumentError("QualifiedTypeName cannot be empty");
    }
    if (complete_type_detail.ann_builtin().has_value())
    {
        applied_builtin_type_annotations_consistency(complete_type_detail.ann_builtin().value());
    }
    if (complete_type_detail.ann_custom().has_value())
    {
        applied_annotation_seq_consistency(complete_type_detail.ann_custom().value());
    }
}

void TypeObjectUtils::structure_base_type_consistency(
        const TypeIdentifier& base_type)
{
    TypeObjectPair type_objects;
    ReturnCode_t ret_code = DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(base_type,
        type_objects);
    if (ret_code != ReturnCode_t::RETCODE_OK || type_objects.complete_type_object._d() != TK_STRUCTURE)
    {
        throw InvalidArgumentError("Inconsistent base TypeIdentifier: must be related to a structure TypeObject");
    }
}

void TypeObjectUtils::complete_struct_header_consistency(
        const CompleteStructHeader& complete_struct_header)
{
    if (complete_struct_header.base_type()._d() != TK_NONE)
    {
        if (!is_direct_hash_type_identifier(complete_struct_header.base_type()))
        {
            throw InvalidArgumentError("Base TypeIdentifier must be direct HASH TypeIdentifier");
        }
        structure_base_type_consistency(complete_struct_header.base_type());
    }
    complete_type_detail_consistency(complete_struct_header.detail());
}

void TypeObjectUtils::complete_struct_type_consistency(
        const CompleteStructType& complete_struct_type)
{
    type_flag_consistency(complete_struct_type.struct_flags());
    complete_struct_header_consistency(complete_struct_type.header());
    complete_struct_member_seq_consistency(complete_struct_type.member_seq());
}

void TypeObjectUtils::union_case_label_seq_consistency(
        const UnionCaseLabelSeq& union_case_label_seq)
{
    std::set<int32_t> labels;
    for (size_t i = 0; i < union_case_label_seq.size(); i++)
    {
        labels.insert(union_case_label_seq[i]);
        if (labels.size() != (i + 1))
        {
            throw InvalidArgumentError("Repeated union case labels");
        }
    }
}

void TypeObjectUtils::common_union_member_consistency(
        const CommonUnionMember& common_union_member)
{
    member_flag_consistency(common_union_member.member_flags());
    type_identifier_consistency(common_union_member.type_id());
    union_case_label_seq_consistency(common_union_member.label_seq());
    if (!(common_union_member.member_flags() & MemberFlagBits::IS_DEFAULT) &&
        common_union_member.label_seq().size() == 0)
    {
        throw InvalidArgumentError("Non default members require at least one associated case");
    }
}

void TypeObjectUtils::common_union_member_complete_member_detail_consistency(
        const CommonUnionMember& common_union_member,
        const CompleteMemberDetail& complete_member_detail)
{
    // Check @hashid consistency with MemberId
    if (complete_member_detail.ann_builtin().has_value() &&
        complete_member_detail.ann_builtin().value().hash_id().has_value())
    {
        std::string string_value;
        // If the annotation [@hashid] is used without any parameter or with the empty string as a parameter, then the
        // Member ID shall be computed from the member name.
        if (complete_member_detail.ann_builtin().value().hash_id().value().empty())
        {
            string_value = complete_member_detail.name();
        }
        else
        {
            string_value = complete_member_detail.ann_builtin().value().hash_id().value();
        }
        string_member_id_consistency(common_union_member.member_id(), string_value);
    }
}

void TypeObjectUtils::complete_union_member_consistency(
        const CompleteUnionMember& complete_union_member)
{
    common_union_member_consistency(complete_union_member.common());
    complete_member_detail_consistency(complete_union_member.detail());
    common_union_member_complete_member_detail_consistency(complete_union_member.common(),
        complete_union_member.detail());
}

void TypeObjectUtils::complete_union_member_seq_consistency(
        const CompleteUnionMemberSeq& complete_member_union_seq)
{
    std::set<MemberId> member_ids;
    std::set<MemberName> member_names;
    std::set<int32_t> case_labels;
    size_t case_labels_length = 0;
    bool default_member = false;
    for (size_t i = 0; i < complete_member_union_seq.size(); i++)
    {
        if (complete_member_union_seq.size() == 0)
        {
            throw InvalidArgumentError("Unions require at least one union member");
        }
        member_ids.insert(complete_member_union_seq[i].common().member_id());
        member_names.insert(complete_member_union_seq[i].detail().name());
        if ((member_ids.size() != (i + 1)) || (member_names.size() != (i + 1)))
        {
            throw InvalidArgumentError("Repeated member id/name in the sequence");
        }
        if (complete_member_union_seq[i].common().member_flags() & MemberFlagBits::IS_DEFAULT)
        {
            if (default_member)
            {
                throw InvalidArgumentError("Union should have at most one default member");
            }
            default_member = true;
        }
        for (size_t j = 0; j < complete_member_union_seq[i].common().label_seq().size(); j++)
        {
            case_labels.insert(complete_member_union_seq[i].common().label_seq()[j]);
            if (case_labels.size() != (case_labels_length + j + 1))
            {
                throw InvalidArgumentError("Repeated union case label");
            }
        }
        case_labels_length += complete_member_union_seq[i].common().label_seq().size();
        complete_union_member_consistency(complete_member_union_seq[i]);
    }
}

void TypeObjectUtils::common_discriminator_member_type_identifier_consistency(
        const TypeIdentifier& type_id)
{
    if (type_id._d() != TK_BOOLEAN && type_id._d() != TK_BYTE && type_id._d() != TK_CHAR8 && type_id._d() != TK_CHAR16
        && type_id._d() != TK_INT8 && type_id._d() != TK_UINT8 && type_id._d() != TK_INT16 && type_id._d() != TK_UINT16
        && type_id._d() != TK_INT32 && type_id._d() != TK_UINT32 && type_id._d() != TK_INT64
        && type_id._d() != TK_UINT64 && type_id._d() != EK_COMPLETE && type_id._d() != EK_MINIMAL)
    {
        throw InvalidArgumentError("Inconsistent CommonDiscriminatorMember TypeIdentifier");
    }
    TypeObjectPair type_objects;
    if (is_direct_hash_type_identifier(type_id))
    {
        if (ReturnCode_t::RETCODE_OK ==
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(type_id, type_objects))
        {
            if (type_objects.complete_type_object._d() == TK_ALIAS)
            {
                common_discriminator_member_type_identifier_consistency(
                    type_objects.complete_type_object.alias_type().body().common().related_type());
            }
            else if (type_objects.complete_type_object._d() != TK_ENUM)
            {
                throw InvalidArgumentError("Inconsistent CommonDiscriminatorMember TypeIdentifier");
            }
        }
    }
}

void TypeObjectUtils::common_discriminator_member_consistency(
        const CommonDiscriminatorMember& common_discriminator_member)
{
    member_flag_consistency(common_discriminator_member.member_flags());
    common_discriminator_member_type_identifier_consistency(common_discriminator_member.type_id());
}

void TypeObjectUtils::complete_union_header_consistency(
        const CompleteUnionHeader& complete_union_header)
{
    complete_type_detail_consistency(complete_union_header.detail());
}

void TypeObjectUtils::complete_discriminator_member_consistency(
        const CompleteDiscriminatorMember& complete_discriminator_member)
{
    common_discriminator_member_consistency(complete_discriminator_member.common());
    if (complete_discriminator_member.ann_builtin().has_value())
    {
        applied_builtin_type_annotations_consistency(complete_discriminator_member.ann_builtin().value());
    }
    if (complete_discriminator_member.ann_custom().has_value())
    {
        applied_annotation_seq_consistency(complete_discriminator_member.ann_custom().value());
    }
}

void TypeObjectUtils::complete_union_type_consistency(
        const CompleteUnionType& complete_union_type)
{
    type_flag_consistency(complete_union_type.union_flags());
    complete_union_header_consistency(complete_union_type.header());
    complete_discriminator_member_consistency(complete_union_type.discriminator());
    complete_union_member_seq_consistency(complete_union_type.member_seq());
}

void TypeObjectUtils::common_annotation_parameter_type_identifier_default_value_consistency(
        const TypeIdentifier& type_id,
        const AnnotationParameterValue& value)
{
    TypeObjectPair type_objects;
    // Primitive types
    if (((type_id._d() > 0 && type_id._d() <= 0x0D) || (type_id._d() == TK_CHAR8 || type_id._d() == TK_CHAR16)) &&
        (type_id._d() != value._d()))
    {
        throw InvalidArgumentError("Given annotation parameter value is inconsistent with given TypeIdentifier");
    }
    // String
    else if ((type_id._d() == TI_STRING8_SMALL || type_id._d() == TI_STRING8_LARGE) && value._d() != TK_STRING8)
    {
        throw InvalidArgumentError("Given annotation parameter value is inconsistent with given TypeIdentifier");
    }
    // Wstring
    else if ((type_id._d() == TI_STRING16_SMALL || type_id._d() == TI_STRING16_LARGE) && value._d() != TK_STRING16)
    {
        throw InvalidArgumentError("Given annotation parameter value is inconsistent with given TypeIdentifier");
    }
    // Enum
    else if (is_direct_hash_type_identifier(type_id) && value._d() == TK_ENUM)
    {
        if (ReturnCode_t::RETCODE_OK ==
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(type_id, type_objects))
        {
            if (type_objects.complete_type_object._d() == TK_ALIAS)
            {
                common_annotation_parameter_type_identifier_default_value_consistency(
                    type_objects.complete_type_object.alias_type().body().common().related_type(), value);
            }
            else if (type_objects.complete_type_object._d() != TK_ENUM)
            {
                throw InvalidArgumentError(
                    "Given annotation parameter value is inconsistent with given TypeIdentifier");
            }
        }
    }
    else
    {
        throw InvalidArgumentError("Given annotation parameter value is inconsistent with given TypeIdentifier");
    }
}

void TypeObjectUtils::common_annotation_parameter_consistency(
        const CommonAnnotationParameter& common_annotation_parameter)
{
    empty_flags_consistency(common_annotation_parameter.member_flags());
    type_identifier_consistency(common_annotation_parameter.member_type_id());
}

void TypeObjectUtils::complete_annotation_parameter_consistency(
        const CompleteAnnotationParameter& complete_annotation_parameter)
{
    common_annotation_parameter_consistency(complete_annotation_parameter.common());
    common_annotation_parameter_type_identifier_default_value_consistency(
        complete_annotation_parameter.common().member_type_id(), complete_annotation_parameter.default_value());
    if (complete_annotation_parameter.name().size() == 0)
    {
        throw InvalidArgumentError("Annotation parameter name cannot be empty");
    }
}

void TypeObjectUtils::complete_annotation_parameter_seq_consistency(
        const CompleteAnnotationParameterSeq& complete_annotation_parameter_seq)
{
    std::set<MemberName> member_names;
    for (size_t i = 0; i < complete_annotation_parameter_seq.size(); i++)
    {
        member_names.insert(complete_annotation_parameter_seq[i].name());
        if (member_names.size() != (i + 1))
        {
            throw InvalidArgumentError("Repeated parameter name in the sequence");
        }
        complete_annotation_parameter_consistency(complete_annotation_parameter_seq[i]);
    }
}

void TypeObjectUtils::complete_annotation_header_consistency(
        const CompleteAnnotationHeader& complete_annotation_header)
{
    if (complete_annotation_header.annotation_name().size() == 0)
    {
        throw InvalidArgumentError("QualifiedTypeName cannot be empty");
    }
}

void TypeObjectUtils::complete_annotation_type_consistency(
        const CompleteAnnotationType& complete_annotation_type)
{
    empty_flags_consistency(complete_annotation_type.annotation_flag());
    complete_annotation_header_consistency(complete_annotation_type.header());
    complete_annotation_parameter_seq_consistency(complete_annotation_type.member_seq());
}

void TypeObjectUtils::common_alias_body_consistency(
        const CommonAliasBody& common_alias_body)
{
    empty_flags_consistency(common_alias_body.related_flags());
    type_identifier_consistency(common_alias_body.related_type());
}

void TypeObjectUtils::hashid_builtin_annotation_not_applied_consistency(
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& ann_builtin)
{
    if (ann_builtin.has_value() && ann_builtin.value().hash_id().has_value())
    {
        throw InvalidArgumentError("@hashid builtin annotation cannot be applied to alias declaration");
    }
}

void TypeObjectUtils::complete_alias_body_consistency(
        const CompleteAliasBody& complete_alias_body)
{
    common_alias_body_consistency(complete_alias_body.common());
    if (complete_alias_body.ann_custom().has_value())
    {
        applied_annotation_seq_consistency(complete_alias_body.ann_custom().value());
    }
    hashid_builtin_annotation_not_applied_consistency(complete_alias_body.ann_builtin());
}

void TypeObjectUtils::complete_alias_header_consistency(
        const CompleteAliasHeader& complete_alias_header)
{
    complete_type_detail_consistency(complete_alias_header.detail());
}

void TypeObjectUtils::complete_alias_type_consistency(
        const CompleteAliasType& complete_alias_type)
{
    empty_flags_consistency(complete_alias_type.alias_flags());
    complete_alias_header_consistency(complete_alias_type.header());
    complete_alias_body_consistency(complete_alias_type.body());
}

void TypeObjectUtils::complete_element_detail_consistency(
        const CompleteElementDetail& complete_element_detail)
{
    if (complete_element_detail.ann_custom().has_value())
    {
        applied_annotation_seq_consistency(complete_element_detail.ann_custom().value());
    }
    hashid_builtin_annotation_not_applied_consistency(complete_element_detail.ann_builtin());
}

void TypeObjectUtils::common_collection_element_consistency(
        const CommonCollectionElement& common_collection_element)
{
    member_flag_consistency(common_collection_element.element_flags());
    type_identifier_consistency(common_collection_element.type());
}

void TypeObjectUtils::complete_collection_element_consistency(
        const CompleteCollectionElement& complete_collection_element)
{
    common_collection_element_consistency(complete_collection_element.common());
    complete_element_detail_consistency(complete_collection_element.detail());
}

void TypeObjectUtils::common_collection_header_consistency(
        const CommonCollectionHeader& common_collection_header)
{
    bound_consistency(common_collection_header.bound());
}

void TypeObjectUtils::complete_collection_header_consistency(
        const CompleteCollectionHeader& complete_collection_header)
{
    common_collection_header_consistency(complete_collection_header.common());
    if (complete_collection_header.detail().has_value())
    {
        complete_type_detail_consistency(complete_collection_header.detail().value());
    }
}

void TypeObjectUtils::complete_sequence_type_consistency(
        const CompleteSequenceType& complete_sequence_type)
{
    empty_flags_consistency(complete_sequence_type.collection_flag());
    complete_collection_header_consistency(complete_sequence_type.header());
    complete_collection_element_consistency(complete_sequence_type.element());
}

void TypeObjectUtils::common_array_header_consistency(
        const CommonArrayHeader& common_array_header)
{
    array_bound_seq_consistency(common_array_header.bound_seq());
}

void TypeObjectUtils::complete_array_header_consistency(
        const CompleteArrayHeader& complete_array_header)
{
    common_array_header_consistency(complete_array_header.common());
    complete_type_detail_consistency(complete_array_header.detail());
}

void TypeObjectUtils::complete_array_type_consistency(
        const CompleteArrayType& complete_array_type)
{
    empty_flags_consistency(complete_array_type.collection_flag());
    complete_array_header_consistency(complete_array_type.header());
    complete_collection_element_consistency(complete_array_type.element());
}

void TypeObjectUtils::complete_map_type_consistency(
        const CompleteMapType& complete_map_type)
{
    empty_flags_consistency(complete_map_type.collection_flag());
    complete_collection_header_consistency(complete_map_type.header());
    map_key_type_identifier_consistency(complete_map_type.key().common().type());
    complete_collection_element_consistency(complete_map_type.key());
    complete_collection_element_consistency(complete_map_type.element());
}

void TypeObjectUtils::common_enumerated_literal_consistency(
        const CommonEnumeratedLiteral& common_enumerated_literal)
{
    enumerated_literal_flag_consistency(common_enumerated_literal.flags());
}

void TypeObjectUtils::complete_enumerated_literal_consistency(
        const CompleteEnumeratedLiteral& complete_enumerated_literal)
{
    common_enumerated_literal_consistency(complete_enumerated_literal.common());
    complete_member_detail_consistency(complete_enumerated_literal.detail());
    if (complete_enumerated_literal.detail().ann_builtin().has_value())
    {
        throw InvalidArgumentError("Only @default_literal and @value builtin annotations apply to enum literals");
    }
}

void TypeObjectUtils::complete_enumerated_literal_seq_consistency(
        const CompleteEnumeratedLiteralSeq& complete_enumerated_literal_seq)
{
    for (size_t i = 0; i < complete_enumerated_literal_seq.size(); i++)
    {
        complete_enumerated_literal_consistency(complete_enumerated_literal_seq[i]);
        // TODO(jlbueno): check uniqueness of default literal and literal value. At least one literal.
    }
}

void TypeObjectUtils::enum_bit_bound_consistency(
        BitBound bit_bound)
{
    if (bit_bound == 0 || bit_bound > 32)
    {
        throw InvalidArgumentError("Enumeration bit_bound must take a value between 1 and 32");
    }
}

void TypeObjectUtils::bitmask_bit_bound_consistency(
        BitBound bit_bound)
{
    if (bit_bound == 0 || bit_bound > 64)
    {
        throw InvalidArgumentError("Bitmask bit_bound must be greater than zero and no greater than 64");
    }
}

void TypeObjectUtils::common_enumerated_header_consistency(
        const CommonEnumeratedHeader& common_enumerated_header,
        bool bitmask)
{
    if (bitmask)
    {
        bitmask_bit_bound_consistency(common_enumerated_header.bit_bound());
    }
    else
    {
        enum_bit_bound_consistency(common_enumerated_header.bit_bound());
    }
}

void TypeObjectUtils::complete_enumerated_header_consistency(
        const CompleteEnumeratedHeader& complete_enumerated_header,
        bool bitmask)
{
    common_enumerated_header_consistency(complete_enumerated_header.common(), bitmask);
    complete_type_detail_consistency(complete_enumerated_header.detail());
}

void TypeObjectUtils::complete_enumerated_type_consistency(
        const CompleteEnumeratedType& complete_enumerated_type)
{
    empty_flags_consistency(complete_enumerated_type.enum_flags());
    complete_enumerated_header_consistency(complete_enumerated_type.header());
    complete_enumerated_literal_seq_consistency(complete_enumerated_type.literal_seq());
}

void TypeObjectUtils::bit_position_consistency(
        uint16_t position)
{
    if (position >= 64)
    {
        throw InvalidArgumentError("Bitflag/Bitfield position must take a value under 64");
    }
}

void TypeObjectUtils::common_bitflag_consistency(
        const CommonBitflag& common_bitflag)
{
    bit_position_consistency(common_bitflag.position());
    empty_flags_consistency(common_bitflag.flags());
}

void TypeObjectUtils::complete_bitflag_consistency(
        const CompleteBitflag& complete_bitflag)
{
    common_bitflag_consistency(complete_bitflag.common());
    complete_member_detail_consistency(complete_bitflag.detail());
    // TODO(jlbueno): check consistency between @bit_bound annotation and CommonBitflag
}

void TypeObjectUtils::complete_bitflag_seq_consistency(
        const CompleteBitflagSeq& complete_bitflag_seq)
{
    for (size_t i = 0; i < complete_bitflag_seq.size(); i++)
    {
        complete_bitflag_consistency(complete_bitflag_seq[i]);
        // TODO(jlbueno): check uniqueness
    }
}

void TypeObjectUtils::complete_bitmask_type_consistency(
        const CompleteBitmaskType& complete_bitmask_type)
{
    empty_flags_consistency(complete_bitmask_type.bitmask_flags());
    complete_enumerated_header_consistency(complete_bitmask_type.header(), true);
    complete_bitflag_seq_consistency(complete_bitmask_type.flag_seq());
}

void TypeObjectUtils::bitfield_holder_type_consistency(
        TypeKind holder_type,
        uint8_t bitcount)
{
    if (holder_type != TK_BOOLEAN && holder_type != TK_BYTE && holder_type != TK_INT16 && holder_type != TK_INT32 &&
        holder_type != TK_INT64 && holder_type != TK_UINT16 && holder_type != TK_UINT32 && holder_type != TK_UINT64 &&
        holder_type != TK_INT8 && holder_type != TK_UINT8)
    {
        throw InvalidArgumentError("Inconsistent bitfield holder type");
    }
    if (bitcount < 9 && holder_type == TK_BOOLEAN)
    {
        throw InvalidArgumentError("Inconsistent bitfield holder type");
    }
    else if ((bitcount < 17 && bitcount > 8) && (holder_type == TK_BOOLEAN || holder_type == TK_BYTE ||
        holder_type == TK_INT8 || holder_type == TK_UINT8))
    {
        throw InvalidArgumentError("Inconsistent bitfield holder type");
    }
    else if ((bitcount < 33 && bitcount > 16) && (holder_type != TK_INT64 && holder_type != TK_UINT64 &&
        holder_type != TK_INT32 && holder_type != TK_UINT32))
    {
        throw InvalidArgumentError("Inconsistent bitfield holder type");
    }
    else if (bitcount > 32 && (holder_type != TK_INT64 && holder_type != TK_UINT64))
    {
        throw InvalidArgumentError("Inconsistent bitfield holder type");
    }
}

void TypeObjectUtils::common_bitfield_consistency(
        const CommonBitfield& common_bitfield)
{
    bit_position_consistency(common_bitfield.position());
    empty_flags_consistency(common_bitfield.flags());
    bitmask_bit_bound_consistency(common_bitfield.bitcount());
    bitmask_bit_bound_consistency(common_bitfield.bitcount() + common_bitfield.position());
    bitfield_holder_type_consistency(common_bitfield.holder_type(), common_bitfield.bitcount());
}

void TypeObjectUtils::complete_bitfield_consistency(
        const CompleteBitfield& complete_bitfield)
{
    common_bitfield_consistency(complete_bitfield.common());
    complete_member_detail_consistency(complete_bitfield.detail());
}

void TypeObjectUtils::complete_bitfield_seq_consistency(
        const CompleteBitfieldSeq& complete_bitfield_seq)
{
    for (size_t i = 0; i < complete_bitfield_seq.size(); i++)
    {
        complete_bitfield_consistency(complete_bitfield_seq[i]);
        // TODO(jlbueno): check uniqueness and no overlapping.
    }
}

void TypeObjectUtils::complete_bitset_header_consistency(
        const CompleteBitsetHeader& complete_bitset_header)
{
    complete_type_detail_consistency(complete_bitset_header.detail());
}

void TypeObjectUtils::complete_bitset_type_consistency(
        const CompleteBitsetType& complete_bitset_type)
{
    empty_flags_consistency(complete_bitset_type.bitset_flags());
    complete_bitset_header_consistency(complete_bitset_type.header());
    complete_bitfield_seq_consistency(complete_bitset_type.field_seq());
}

} // xtypes1_3
} // dds
} // fastdds
} // eprosima
