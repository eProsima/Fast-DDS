// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeObjectUtils.cpp
 *
 * Definitions of the TypeObjectUtils mock. Consistency checks never throw, builders return default constructed
 * values, boolean queries return false and ReturnCode_t methods return RETCODE_OK.
 */

#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

const TypeObjectHashId TypeObjectUtils::build_type_object_hash_id(
        uint8_t /*discriminator*/,
        const EquivalenceHash& /*hash*/)
{
    return {};
}

CollectionElementFlag TypeObjectUtils::build_collection_element_flag(
        TryConstructFailAction /*try_construct_kind*/,
        bool /*external*/)
{
    return {};
}

StructMemberFlag TypeObjectUtils::build_struct_member_flag(
        TryConstructFailAction /*try_construct_kind*/,
        bool /*optional*/,
        bool /*must_understand*/,
        bool /*key*/,
        bool /*external*/)
{
    return {};
}

UnionMemberFlag TypeObjectUtils::build_union_member_flag(
        TryConstructFailAction /*try_construct_kind*/,
        bool /*default_member*/,
        bool /*external*/)
{
    return {};
}

UnionDiscriminatorFlag TypeObjectUtils::build_union_discriminator_flag(
        TryConstructFailAction /*try_construct_kind*/,
        bool /*key*/)
{
    return {};
}

EnumeratedLiteralFlag TypeObjectUtils::build_enumerated_literal_flag(
        bool /*default_literal*/)
{
    return {};
}

StructTypeFlag TypeObjectUtils::build_struct_type_flag(
        ExtensibilityKind /*extensibility_kind*/,
        bool /*nested*/,
        bool /*autoid_hash*/)
{
    return {};
}

UnionTypeFlag TypeObjectUtils::build_union_type_flag(
        ExtensibilityKind /*extensibility_kind*/,
        bool /*nested*/,
        bool /*autoid_hash*/)
{
    return {};
}

const StringSTypeDefn TypeObjectUtils::build_string_s_type_defn(
        SBound /*bound*/)
{
    return {};
}

const StringLTypeDefn TypeObjectUtils::build_string_l_type_defn(
        LBound /*bound*/)
{
    return {};
}

const PlainCollectionHeader TypeObjectUtils::build_plain_collection_header(
        EquivalenceKind /*equiv_kind*/,
        CollectionElementFlag /*element_flags*/)
{
    return {};
}

const PlainSequenceSElemDefn TypeObjectUtils::build_plain_sequence_s_elem_defn(
        const PlainCollectionHeader& /*header*/,
        SBound /*s_bound*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*element_identifier*/)
{
    return {};
}

const PlainSequenceLElemDefn TypeObjectUtils::build_plain_sequence_l_elem_defn(
        const PlainCollectionHeader& /*header*/,
        LBound /*l_bound*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*element_identifier*/)
{
    return {};
}

const PlainArraySElemDefn TypeObjectUtils::build_plain_array_s_elem_defn(
        const PlainCollectionHeader& /*header*/,
        const SBoundSeq& /*array_bound_seq*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*element_identifier*/)
{
    return {};
}

const PlainArrayLElemDefn TypeObjectUtils::build_plain_array_l_elem_defn(
        const PlainCollectionHeader& /*header*/,
        const LBoundSeq& /*array_bound_seq*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*element_identifier*/)
{
    return {};
}

const PlainMapSTypeDefn TypeObjectUtils::build_plain_map_s_type_defn(
        const PlainCollectionHeader& /*header*/,
        const SBound /*bound*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*element_identifier*/,
        const CollectionElementFlag /*key_flags*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*key_identifier*/)
{
    return {};
}

const PlainMapLTypeDefn TypeObjectUtils::build_plain_map_l_type_defn(
        const PlainCollectionHeader& /*header*/,
        const LBound /*bound*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*element_identifier*/,
        const CollectionElementFlag /*key_flags*/,
        const eprosima::fastcdr::external<TypeIdentifier>& /*key_identifier*/)
{
    return {};
}

const StronglyConnectedComponentId TypeObjectUtils::build_strongly_connected_component_id(
        const TypeObjectHashId& /*sc_component_id*/,
        int32_t /*scc_length*/,
        int32_t /*scc_index*/)
{
    return {};
}

const ExtendedTypeDefn TypeObjectUtils::build_extended_type_defn()
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_s_string_type_identifier(
        const StringSTypeDefn& /*string*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/,
        bool /*wstring*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_l_string_type_identifier(
        const StringLTypeDefn& /*string*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/,
        bool /*wstring*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_s_sequence_type_identifier(
        const PlainSequenceSElemDefn& /*plain_seq*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_l_sequence_type_identifier(
        const PlainSequenceLElemDefn& /*plain_seq*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_s_array_type_identifier(
        const PlainArraySElemDefn& /*plain_array*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_l_array_type_identifier(
        const PlainArrayLElemDefn& /*plain_array*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_s_map_type_identifier(
        const PlainMapSTypeDefn& /*plain_map*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_l_map_type_identifier(
        const PlainMapLTypeDefn& /*plain_map*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_scc_type_identifier(
        const StronglyConnectedComponentId& /*scc*/,
        const std::string& /*type_name*/)
{
    return {};
}

const ExtendedAnnotationParameterValue TypeObjectUtils::build_extended_annotation_parameter_value()
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        bool /*boolean_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value_byte(
        uint8_t /*byte_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int8_t /*int8_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint8_t /*uint8_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int16_t /*int16_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint16_t /*uint16_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int32_t /*int32_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint32_t /*uint32_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        int64_t /*int64_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        uint64_t /*uint64_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        float /*float32_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        double /*float64_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        long double /*float128_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        char /*char_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        wchar_t /*wchar_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value_enum(
        int32_t /*enumerated_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        const eprosima::fastcdr::fixed_string<128>& /*string8_value*/)
{
    return {};
}

const AnnotationParameterValue TypeObjectUtils::build_annotation_parameter_value(
        const std::wstring& /*string16_value*/)
{
    return {};
}

const AppliedAnnotationParameter TypeObjectUtils::build_applied_annotation_parameter(
        const NameHash& /*paramname_hash*/,
        const AnnotationParameterValue& /*value*/)
{
    return {};
}

void TypeObjectUtils::add_applied_annotation_parameter(
        AppliedAnnotationParameterSeq& /*param_seq*/,
        const AppliedAnnotationParameter& /*param*/)
{
}

const AppliedAnnotation TypeObjectUtils::build_applied_annotation(
        const TypeIdentifier& /*annotation_typeid*/,
        const eprosima::fastcdr::optional<AppliedAnnotationParameterSeq>& /*param_seq*/)
{
    return {};
}

void TypeObjectUtils::add_applied_annotation(
        AppliedAnnotationSeq& /*ann_custom_seq*/,
        const AppliedAnnotation& /*ann_custom*/)
{
}

const AppliedVerbatimAnnotation TypeObjectUtils::build_applied_verbatim_annotation(
        PlacementKind /*placement*/,
        const eprosima::fastcdr::fixed_string<32>& /*language*/,
        const std::string& /*text*/)
{
    return {};
}

const AppliedBuiltinMemberAnnotations TypeObjectUtils::build_applied_builtin_member_annotations(
        const eprosima::fastcdr::optional<std::string>& /*unit*/,
        const eprosima::fastcdr::optional<AnnotationParameterValue>& /*min*/,
        const eprosima::fastcdr::optional<AnnotationParameterValue>& /*max*/,
        const eprosima::fastcdr::optional<std::string>& /*hash_id*/)
{
    return {};
}

const CommonStructMember TypeObjectUtils::build_common_struct_member(
        MemberId /*member_id*/,
        StructMemberFlag /*member_flags*/,
        const TypeIdentifier& /*member_type_id*/)
{
    return {};
}

const CompleteMemberDetail TypeObjectUtils::build_complete_member_detail(
        const MemberName& /*name*/,
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& /*ann_builtin*/,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& /*ann_custom*/)
{
    return {};
}

const CompleteStructMember TypeObjectUtils::build_complete_struct_member(
        const CommonStructMember& /*common*/,
        const CompleteMemberDetail& /*detail*/)
{
    return {};
}

void TypeObjectUtils::add_complete_struct_member(
        CompleteStructMemberSeq& /*member_seq*/,
        const CompleteStructMember& /*member*/)
{
}

const AppliedBuiltinTypeAnnotations TypeObjectUtils::build_applied_builtin_type_annotations(
        const eprosima::fastcdr::optional<AppliedVerbatimAnnotation>& /*verbatim*/)
{
    return {};
}

const CompleteTypeDetail TypeObjectUtils::build_complete_type_detail(
        const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& /*ann_builtin*/,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& /*ann_custom*/,
        const QualifiedTypeName& /*type_name*/)
{
    return {};
}

const CompleteStructHeader TypeObjectUtils::build_complete_struct_header(
        const TypeIdentifier& /*base_type*/,
        const CompleteTypeDetail& /*detail*/)
{
    return {};
}

const CompleteStructType TypeObjectUtils::build_complete_struct_type(
        StructTypeFlag /*struct_flags*/,
        const CompleteStructHeader& /*header*/,
        const CompleteStructMemberSeq& /*member_seq*/)
{
    return {};
}

void TypeObjectUtils::add_union_case_label(
        UnionCaseLabelSeq& /*label_seq*/,
        int32_t /*label*/)
{
}

const CommonUnionMember TypeObjectUtils::build_common_union_member(
        MemberId /*member_id*/,
        UnionMemberFlag /*member_flags*/,
        const TypeIdentifier& /*type_id*/,
        const UnionCaseLabelSeq& /*label_seq*/)
{
    return {};
}

const CompleteUnionMember TypeObjectUtils::build_complete_union_member(
        const CommonUnionMember& /*common*/,
        const CompleteMemberDetail& /*detail*/)
{
    return {};
}

void TypeObjectUtils::add_complete_union_member(
        CompleteUnionMemberSeq& /*complete_union_member_seq*/,
        const CompleteUnionMember& /*member*/)
{
}

const CommonDiscriminatorMember TypeObjectUtils::build_common_discriminator_member(
        UnionDiscriminatorFlag /*member_flags*/,
        const TypeIdentifier& /*type_id*/)
{
    return {};
}

const CompleteDiscriminatorMember TypeObjectUtils::build_complete_discriminator_member(
        const CommonDiscriminatorMember& /*common*/,
        const eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations>& /*ann_builtin*/,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& /*ann_custom*/)
{
    return {};
}

const CompleteUnionHeader TypeObjectUtils::build_complete_union_header(
        const CompleteTypeDetail& /*detail*/)
{
    return {};
}

const CompleteUnionType TypeObjectUtils::build_complete_union_type(
        UnionTypeFlag /*union_flags*/,
        const CompleteUnionHeader& /*header*/,
        const CompleteDiscriminatorMember& /*discriminator*/,
        const CompleteUnionMemberSeq& /*member_seq*/)
{
    return {};
}

const CommonAnnotationParameter TypeObjectUtils::build_common_annotation_parameter(
        AnnotationParameterFlag /*member_flags*/,
        const TypeIdentifier& /*member_type_id*/)
{
    return {};
}

const CompleteAnnotationParameter TypeObjectUtils::build_complete_annotation_parameter(
        const CommonAnnotationParameter& /*common*/,
        const MemberName& /*name*/,
        const AnnotationParameterValue& /*default_value*/)
{
    return {};
}

void TypeObjectUtils::add_complete_annotation_parameter(
        CompleteAnnotationParameterSeq& /*sequence*/,
        const CompleteAnnotationParameter& /*param*/)
{
}

const CompleteAnnotationHeader TypeObjectUtils::build_complete_annotation_header(
        const QualifiedTypeName& /*annotation_name*/)
{
    return {};
}

const CompleteAnnotationType TypeObjectUtils::build_complete_annotation_type(
        AnnotationTypeFlag /*annotation_flag*/,
        const CompleteAnnotationHeader& /*header*/,
        const CompleteAnnotationParameterSeq& /*member_seq*/)
{
    return {};
}

const CommonAliasBody TypeObjectUtils::build_common_alias_body(
        AliasMemberFlag /*related_flags*/,
        const TypeIdentifier& /*related_type*/)
{
    return {};
}

const CompleteAliasBody TypeObjectUtils::build_complete_alias_body(
        const CommonAliasBody& /*common*/,
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& /*ann_builtin*/,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& /*ann_custom*/)
{
    return {};
}

const CompleteAliasHeader TypeObjectUtils::build_complete_alias_header(
        const CompleteTypeDetail& /*detail*/)
{
    return {};
}

const CompleteAliasType TypeObjectUtils::build_complete_alias_type(
        AliasTypeFlag /*alias_flags*/,
        const CompleteAliasHeader& /*header*/,
        const CompleteAliasBody& /*body*/)
{
    return {};
}

const CompleteElementDetail TypeObjectUtils::build_complete_element_detail(
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& /*ann_builtin*/,
        const eprosima::fastcdr::optional<AppliedAnnotationSeq>& /*ann_custom*/)
{
    return {};
}

const CommonCollectionElement TypeObjectUtils::build_common_collection_element(
        CollectionElementFlag /*element_flags*/,
        const TypeIdentifier& /*type*/)
{
    return {};
}

const CompleteCollectionElement TypeObjectUtils::build_complete_collection_element(
        const CommonCollectionElement& /*common*/,
        const CompleteElementDetail& /*detail*/)
{
    return {};
}

const CommonCollectionHeader TypeObjectUtils::build_common_collection_header(
        LBound /*bound*/)
{
    return {};
}

const CompleteCollectionHeader TypeObjectUtils::build_complete_collection_header(
        const CommonCollectionHeader& /*common*/,
        const eprosima::fastcdr::optional<CompleteTypeDetail>& /*detail*/)
{
    return {};
}

const CompleteSequenceType TypeObjectUtils::build_complete_sequence_type(
        CollectionTypeFlag /*collection_flag*/,
        const CompleteCollectionHeader& /*header*/,
        const CompleteCollectionElement& /*element*/)
{
    return {};
}

const CommonArrayHeader TypeObjectUtils::build_common_array_header(
        const LBoundSeq& /*bound_seq*/)
{
    return {};
}

const CompleteArrayHeader TypeObjectUtils::build_complete_array_header(
        const CommonArrayHeader& /*common*/,
        const CompleteTypeDetail& /*detail*/)
{
    return {};
}

const CompleteArrayType TypeObjectUtils::build_complete_array_type(
        CollectionTypeFlag /*collection_flag*/,
        const CompleteArrayHeader& /*header*/,
        const CompleteCollectionElement& /*element*/)
{
    return {};
}

const CompleteMapType TypeObjectUtils::build_complete_map_type(
        CollectionTypeFlag /*collection_flag*/,
        const CompleteCollectionHeader& /*header*/,
        const CompleteCollectionElement& /*key*/,
        const CompleteCollectionElement& /*element*/)
{
    return {};
}

const CommonEnumeratedLiteral TypeObjectUtils::build_common_enumerated_literal(
        int32_t /*value*/,
        EnumeratedLiteralFlag /*flags*/)
{
    return {};
}

const CompleteEnumeratedLiteral TypeObjectUtils::build_complete_enumerated_literal(
        const CommonEnumeratedLiteral& /*common*/,
        const CompleteMemberDetail& /*detail*/)
{
    return {};
}

void TypeObjectUtils::add_complete_enumerated_literal(
        CompleteEnumeratedLiteralSeq& /*sequence*/,
        const CompleteEnumeratedLiteral& /*enum_literal*/)
{
}

const CommonEnumeratedHeader TypeObjectUtils::build_common_enumerated_header(
        BitBound /*bit_bound*/,
        bool /*bitmask*/)
{
    return {};
}

const CompleteEnumeratedHeader TypeObjectUtils::build_complete_enumerated_header(
        const CommonEnumeratedHeader& /*common*/,
        const CompleteTypeDetail& /*detail*/,
        bool /*bitmask*/)
{
    return {};
}

const CompleteEnumeratedType TypeObjectUtils::build_complete_enumerated_type(
        EnumTypeFlag /*enum_flags*/,
        const CompleteEnumeratedHeader& /*header*/,
        const CompleteEnumeratedLiteralSeq& /*literal_seq*/)
{
    return {};
}

const CommonBitflag TypeObjectUtils::build_common_bitflag(
        uint16_t /*position*/,
        BitflagFlag /*flags*/)
{
    return {};
}

const CompleteBitflag TypeObjectUtils::build_complete_bitflag(
        const CommonBitflag& /*common*/,
        const CompleteMemberDetail& /*detail*/)
{
    return {};
}

void TypeObjectUtils::add_complete_bitflag(
        CompleteBitflagSeq& /*sequence*/,
        const CompleteBitflag& /*bitflag*/)
{
}

const CompleteBitmaskType TypeObjectUtils::build_complete_bitmask_type(
        BitmaskTypeFlag /*bitmask_flags*/,
        const CompleteBitmaskHeader& /*header*/,
        const CompleteBitflagSeq& /*flag_seq*/)
{
    return {};
}

const CommonBitfield TypeObjectUtils::build_common_bitfield(
        uint16_t /*position*/,
        BitsetMemberFlag /*flags*/,
        uint8_t /*bitcount*/,
        TypeKind /*holder_type*/)
{
    return {};
}

const CompleteBitfield TypeObjectUtils::build_complete_bitfield(
        const CommonBitfield& /*common*/,
        const CompleteMemberDetail& /*detail*/)
{
    return {};
}

void TypeObjectUtils::add_complete_bitfield(
        CompleteBitfieldSeq& /*sequence*/,
        const CompleteBitfield& /*bitfield*/)
{
}

const CompleteBitsetHeader TypeObjectUtils::build_complete_bitset_header(
        const CompleteTypeDetail& /*detail*/)
{
    return {};
}

const CompleteBitsetType TypeObjectUtils::build_complete_bitset_type(
        BitsetTypeFlag /*bitset_flags*/,
        const CompleteBitsetHeader& /*header*/,
        const CompleteBitfieldSeq& /*field_seq*/)
{
    return {};
}

const CompleteExtendedType TypeObjectUtils::build_complete_extended_type()
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_alias_type_object(
        const CompleteAliasType& /*alias_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_annotation_type_object(
        const CompleteAnnotationType& /*annotation_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_struct_type_object(
        const CompleteStructType& /*struct_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_union_type_object(
        const CompleteUnionType& /*union_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_bitset_type_object(
        const CompleteBitsetType& /*bitset_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_sequence_type_object(
        const CompleteSequenceType& /*sequence_type*/,
        const std::string& /*type_name*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_array_type_object(
        const CompleteArrayType& /*array_type*/,
        const std::string& /*type_name*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_map_type_object(
        const CompleteMapType& /*map_type*/,
        const std::string& /*type_name*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_enumerated_type_object(
        const CompleteEnumeratedType& /*enumerated_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

ReturnCode_t TypeObjectUtils::build_and_register_bitmask_type_object(
        const CompleteBitmaskType& /*bitmask_type*/,
        const std::string& /*type_name*/,
        TypeIdentifierPair& /*type_ids*/)
{
    return {};
}

const NameHash TypeObjectUtils::name_hash(
        const std::string& /*name*/)
{
    return {};
}

void TypeObjectUtils::type_object_consistency(
        const TypeObject& /*type_object*/)
{
}

const TypeIdentifier& TypeObjectUtils::retrieve_minimal_type_identifier(
        const TypeIdentifierPair& /*type_ids*/,
        bool& /*ec*/)
{
    static const TypeIdentifier instance {};
    return instance;
}

const TypeIdentifier& TypeObjectUtils::retrieve_complete_type_identifier(
        const TypeIdentifierPair& /*type_ids*/,
        bool& /*ec*/)
{
    static const TypeIdentifier instance {};
    return instance;
}

void TypeObjectUtils::set_try_construct_behavior(
        MemberFlag& /*member_flag*/,
        TryConstructFailAction /*try_construct_kind*/)
{
}

void TypeObjectUtils::set_type_flag(
        TypeFlag& /*type_flag*/,
        ExtensibilityKind /*extensibility_kind*/,
        bool /*nested*/,
        bool /*autoid_hash*/)
{
}

void TypeObjectUtils::set_extensibility_kind(
        TypeFlag& /*type_flag*/,
        ExtensibilityKind /*extensibility_kind*/)
{
}

bool TypeObjectUtils::is_fully_descriptive_type_identifier(
        const TypeIdentifier& /*type_identifier*/)
{
    return false;
}

bool TypeObjectUtils::is_direct_hash_type_identifier(
        const TypeIdentifier& /*type_identifier*/)
{
    return false;
}

bool TypeObjectUtils::is_indirect_hash_type_identifier(
        const TypeIdentifier& /*type_identifier*/)
{
    return false;
}

void TypeObjectUtils::l_bound_consistency(
        LBound /*bound*/)
{
}

void TypeObjectUtils::l_bound_seq_consistency(
        const LBoundSeq& /*bound_seq*/)
{
}

void TypeObjectUtils::collection_element_flag_consistency(
        CollectionElementFlag /*collection_element_flag*/)
{
}

void TypeObjectUtils::struct_member_flag_consistency(
        StructMemberFlag /*member_flags*/)
{
}

void TypeObjectUtils::union_member_flag_consistency(
        UnionMemberFlag /*union_member_flag*/)
{
}

void TypeObjectUtils::union_discriminator_flag_consistency(
        UnionDiscriminatorFlag /*union_discriminator_flag*/)
{
}

void TypeObjectUtils::enumerated_literal_flag_consistency(
        EnumeratedLiteralFlag /*enumerated_literal_flag*/)
{
}

void TypeObjectUtils::type_flag_consistency(
        TypeFlag /*type_flag*/)
{
}

void TypeObjectUtils::equivalence_kind_consistency(
        EquivalenceKind /*equiv_kind*/)
{
}

void TypeObjectUtils::plain_collection_header_consistency(
        const PlainCollectionHeader& /*header*/)
{
}

void TypeObjectUtils::plain_collection_type_identifier_header_consistency(
        const PlainCollectionHeader& /*header*/,
        const TypeIdentifier& /*element_identifier*/)
{
}

EquivalenceKind TypeObjectUtils::get_map_component_equiv_kind_for_consistency(
        const TypeIdentifier& /*identifier*/)
{
    return {};
}

void TypeObjectUtils::plain_map_type_components_consistency(
        const PlainCollectionHeader& /*header*/,
        const TypeIdentifier& /*type_identifier*/)
{
}

void TypeObjectUtils::map_key_type_identifier_consistency(
        const TypeIdentifier& /*key_identifier*/)
{
}

void TypeObjectUtils::string_ldefn_consistency(
        const StringLTypeDefn& /*string*/)
{
}

void TypeObjectUtils::seq_sdefn_consistency(
        const PlainSequenceSElemDefn& /*plain_seq*/)
{
}

void TypeObjectUtils::seq_ldefn_consistency(
        const PlainSequenceLElemDefn& /*plain_seq*/)
{
}

void TypeObjectUtils::array_sdefn_consistency(
        const PlainArraySElemDefn& /*plain_array*/)
{
}

void TypeObjectUtils::array_ldefn_consistency(
        const PlainArrayLElemDefn& /*plain_array*/)
{
}

void TypeObjectUtils::map_sdefn_consistency(
        const PlainMapSTypeDefn& /*plain_map*/)
{
}

void TypeObjectUtils::map_ldefn_consistency(
        const PlainMapLTypeDefn& /*plain_map*/)
{
}

void TypeObjectUtils::direct_hash_type_identifier_consistency(
        const TypeIdentifier& /*type_id*/)
{
}

void TypeObjectUtils::type_identifier_consistency(
        const TypeIdentifier& /*type_identifier*/)
{
}

void TypeObjectUtils::applied_annotation_parameter_seq_consistency(
        const AppliedAnnotationParameterSeq& /*applied_annotation_parameter_seq*/)
{
}

void TypeObjectUtils::applied_annotation_type_identifier_consistency(
        const TypeIdentifier& /*annotation_type_id*/)
{
}

void TypeObjectUtils::applied_annotation_consistency(
        const AppliedAnnotation& /*applied_annotation*/)
{
}

void TypeObjectUtils::applied_annotation_seq_consistency(
        const AppliedAnnotationSeq& /*applied_annotation_seq*/)
{
}

void TypeObjectUtils::applied_verbatim_annotation_consistency(
        const AppliedVerbatimAnnotation& /*applied_verbatim_annotation*/)
{
}

void TypeObjectUtils::common_struct_member_consistency(
        const CommonStructMember& /*common_struct_member*/)
{
}

void TypeObjectUtils::complete_member_detail_consistency(
        const CompleteMemberDetail& /*complete_member_detail*/)
{
}

void TypeObjectUtils::common_struct_member_and_complete_member_detail_consistency(
        const CommonStructMember& /*common_struct_member*/,
        const CompleteMemberDetail& /*complete_member_detail*/)
{
}

void TypeObjectUtils::string_member_id_consistency(
        MemberId /*member_id*/,
        const std::string& /*string_value*/)
{
}

void TypeObjectUtils::complete_struct_member_consistency(
        const CompleteStructMember& /*complete_struct_member*/)
{
}

void TypeObjectUtils::complete_struct_member_seq_consistency(
        const CompleteStructMemberSeq& /*complete_struct_member_seq*/)
{
}

void TypeObjectUtils::applied_builtin_type_annotations_consistency(
        const AppliedBuiltinTypeAnnotations& /*applied_builtin_type_annotations*/)
{
}

void TypeObjectUtils::complete_type_detail_consistency(
        const CompleteTypeDetail& /*complete_type_detail*/)
{
}

void TypeObjectUtils::structure_base_type_consistency(
        const TypeIdentifier& /*base_type*/)
{
}

void TypeObjectUtils::complete_struct_header_consistency(
        const CompleteStructHeader& /*complete_struct_header*/)
{
}

void TypeObjectUtils::complete_struct_type_consistency(
        const CompleteStructType& /*complete_struct_type*/)
{
}

void TypeObjectUtils::minimal_struct_type_consistency(
        const MinimalStructType& /*minimal_struct_type*/)
{
}

void TypeObjectUtils::union_case_label_seq_consistency(
        const UnionCaseLabelSeq& /*union_case_label_seq*/)
{
}

void TypeObjectUtils::common_union_member_consistency(
        const CommonUnionMember& /*common_union_member*/)
{
}

void TypeObjectUtils::common_union_member_complete_member_detail_consistency(
        const CommonUnionMember& /*common_union_member*/,
        const CompleteMemberDetail& /*complete_member_detail*/)
{
}

void TypeObjectUtils::complete_union_member_consistency(
        const CompleteUnionMember& /*complete_union_member*/)
{
}

void TypeObjectUtils::complete_union_member_seq_consistency(
        const CompleteUnionMemberSeq& /*complete_union_member_seq*/)
{
}

void TypeObjectUtils::common_discriminator_member_type_identifier_consistency(
        const TypeIdentifier& /*type_id*/)
{
}

void TypeObjectUtils::common_discriminator_member_consistency(
        const CommonDiscriminatorMember& /*common_discriminator_member*/)
{
}

void TypeObjectUtils::complete_discriminator_member_consistency(
        const CompleteDiscriminatorMember& /*complete_discriminator_member*/)
{
}

void TypeObjectUtils::complete_union_header_consistency(
        const CompleteUnionHeader& /*complete_union_header*/)
{
}

void TypeObjectUtils::complete_union_type_consistency(
        const CompleteUnionType& /*complete_union_type*/)
{
}

void TypeObjectUtils::minimal_union_type_consistency(
        const MinimalUnionType& /*minimal_union_type*/)
{
}

void TypeObjectUtils::common_annotation_parameter_type_identifier_default_value_consistency(
        const TypeIdentifier& /*type_id*/,
        const AnnotationParameterValue& /*value*/)
{
}

void TypeObjectUtils::common_annotation_parameter_consistency(
        const CommonAnnotationParameter& /*common_annotation_parameter*/)
{
}

void TypeObjectUtils::complete_annotation_parameter_consistency(
        const CompleteAnnotationParameter& /*complete_annotation_parameter*/)
{
}

void TypeObjectUtils::complete_annotation_parameter_seq_consistency(
        const CompleteAnnotationParameterSeq& /*complete_annotation_parameter_seq*/)
{
}

void TypeObjectUtils::complete_annotation_header_consistency(
        const CompleteAnnotationHeader& /*complete_annotation_header*/)
{
}

void TypeObjectUtils::complete_annotation_type_consistency(
        const CompleteAnnotationType& /*complete_annotation_type*/)
{
}

void TypeObjectUtils::minimal_annotation_type_consistency(
        const MinimalAnnotationType& /*minimal_annotation_type*/)
{
}

void TypeObjectUtils::common_alias_body_consistency(
        const CommonAliasBody& /*common_alias_body*/)
{
}

void TypeObjectUtils::hashid_builtin_annotation_not_applied_consistency(
        const eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations>& /*ann_builtin*/)
{
}

void TypeObjectUtils::complete_alias_body_consistency(
        const CompleteAliasBody& /*complete_alias_body*/)
{
}

void TypeObjectUtils::complete_alias_header_consistency(
        const CompleteAliasHeader& /*complete_alias_header*/)
{
}

void TypeObjectUtils::complete_alias_type_consistency(
        const CompleteAliasType& /*complete_alias_type*/)
{
}

void TypeObjectUtils::minimal_alias_type_consistency(
        const MinimalAliasType& /*minimal_alias_type*/)
{
}

void TypeObjectUtils::complete_element_detail_consistency(
        const CompleteElementDetail& /*complete_element_detail*/)
{
}

void TypeObjectUtils::common_collection_element_consistency(
        const CommonCollectionElement& /*common_collection_element*/)
{
}

void TypeObjectUtils::complete_collection_element_consistency(
        const CompleteCollectionElement& /*complete_collection_element*/)
{
}

void TypeObjectUtils::complete_collection_header_consistency(
        const CompleteCollectionHeader& /*complete_collection_header*/)
{
}

void TypeObjectUtils::complete_sequence_type_consistency(
        const CompleteSequenceType& /*complete_sequence_type*/)
{
}

void TypeObjectUtils::minimal_sequence_type_consistency(
        const MinimalSequenceType& /*minimal_sequence_type*/)
{
}

void TypeObjectUtils::common_array_header_consistency(
        const CommonArrayHeader& /*common_array_header*/)
{
}

void TypeObjectUtils::complete_array_header_consistency(
        const CompleteArrayHeader& /*complete_array_header*/)
{
}

void TypeObjectUtils::complete_array_type_consistency(
        const CompleteArrayType& /*complete_array_type*/)
{
}

void TypeObjectUtils::minimal_array_type_consistency(
        const MinimalArrayType& /*minimal_array_type*/)
{
}

void TypeObjectUtils::complete_map_type_consistency(
        const CompleteMapType& /*complete_map_type*/)
{
}

void TypeObjectUtils::minimal_map_type_consistency(
        const MinimalMapType& /*minimal_map_type*/)
{
}

void TypeObjectUtils::common_enumerated_literal_consistency(
        const CommonEnumeratedLiteral& /*common_enumerated_literal*/)
{
}

void TypeObjectUtils::complete_enumerated_literal_consistency(
        const CompleteEnumeratedLiteral& /*complete_enumerated_literal*/)
{
}

void TypeObjectUtils::complete_enumerated_literal_seq_consistency(
        const CompleteEnumeratedLiteralSeq& /*complete_enumerated_literal_seq*/)
{
}

void TypeObjectUtils::enum_bit_bound_consistency(
        BitBound /*bit_bound*/)
{
}

void TypeObjectUtils::bitmask_bit_bound_consistency(
        BitBound /*bit_bound*/)
{
}

void TypeObjectUtils::common_enumerated_header_consistency(
        const CommonEnumeratedHeader& /*common_enumerated_header*/,
        bool /*bitmask*/)
{
}

void TypeObjectUtils::complete_enumerated_header_consistency(
        const CompleteEnumeratedHeader& /*complete_enumerated_header*/,
        bool /*bitmask*/)
{
}

void TypeObjectUtils::complete_enumerated_type_consistency(
        const CompleteEnumeratedType& /*complete_enumerated_type*/)
{
}

void TypeObjectUtils::minimal_enumerated_type_consistency(
        const MinimalEnumeratedType& /*minimal_enumerated_type*/)
{
}

void TypeObjectUtils::bit_position_consistency(
        uint16_t /*position*/)
{
}

void TypeObjectUtils::common_bitflag_consistency(
        const CommonBitflag& /*common_bitflag*/)
{
}

void TypeObjectUtils::complete_bitflag_consistency(
        const CompleteBitflag& /*complete_bitflag*/)
{
}

void TypeObjectUtils::complete_bitflag_seq_consistency(
        const CompleteBitflagSeq& /*complete_bitflag_seq*/)
{
}

void TypeObjectUtils::complete_bitmask_type_consistency(
        const CompleteBitmaskType& /*complete_bitmask_type*/)
{
}

void TypeObjectUtils::minimal_bitmask_type_consistency(
        const MinimalBitmaskType& /*minimal_bitmask_type*/)
{
}

void TypeObjectUtils::bitfield_holder_type_consistency(
        TypeKind /*holder_type*/,
        uint8_t /*bitcount*/)
{
}

void TypeObjectUtils::common_bitfield_consistency(
        const CommonBitfield& /*common_bitfield*/)
{
}

void TypeObjectUtils::complete_bitfield_consistency(
        const CompleteBitfield& /*complete_bitfield*/)
{
}

void TypeObjectUtils::complete_bitfield_seq_consistency(
        const CompleteBitfieldSeq& /*complete_bitfield_seq*/)
{
}

void TypeObjectUtils::complete_bitset_header_consistency(
        const CompleteBitsetHeader& /*complete_bitset_header*/)
{
}

void TypeObjectUtils::complete_bitset_type_consistency(
        const CompleteBitsetType& /*complete_bitset_type*/)
{
}

void TypeObjectUtils::minimal_bitset_type_consistency(
        const MinimalBitsetType& /*minimal_bitset_type*/)
{
}

void TypeObjectUtils::complete_type_object_consistency(
        const CompleteTypeObject& /*complete_type_object*/)
{
}

void TypeObjectUtils::minimal_type_object_consistency(
        const MinimalTypeObject& /*minimal_type_object*/)
{
}

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima
