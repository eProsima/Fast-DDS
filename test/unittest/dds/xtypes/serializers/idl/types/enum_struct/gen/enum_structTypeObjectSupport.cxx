// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file enum_structTypeObjectSupport.cxx
 * Source file containing the implementation to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen (version: 4.1.0).
 */

#include "enum_structTypeObjectSupport.hpp"

#include <mutex>
#include <string>

#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include "enum_struct.hpp"


using namespace eprosima::fastdds::dds::xtypes;

void register_ColorEnum_type_identifier(
        TypeIdentifierPair& type_ids_ColorEnum)
{
    ReturnCode_t return_code_ColorEnum {eprosima::fastdds::dds::RETCODE_OK};
    return_code_ColorEnum =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "ColorEnum", type_ids_ColorEnum);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_ColorEnum)
    {
        EnumTypeFlag enum_flags_ColorEnum = 0;
        BitBound bit_bound_ColorEnum = 32;
        CommonEnumeratedHeader common_ColorEnum = TypeObjectUtils::build_common_enumerated_header(bit_bound_ColorEnum);
        QualifiedTypeName type_name_ColorEnum = "ColorEnum";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_ColorEnum;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_ColorEnum;
        CompleteTypeDetail detail_ColorEnum = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_ColorEnum, ann_custom_ColorEnum, type_name_ColorEnum.to_string());
        CompleteEnumeratedHeader header_ColorEnum = TypeObjectUtils::build_complete_enumerated_header(common_ColorEnum, detail_ColorEnum);
        CompleteEnumeratedLiteralSeq literal_seq_ColorEnum;
        {
            EnumeratedLiteralFlag flags_RED = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_RED = TypeObjectUtils::build_common_enumerated_literal(0, flags_RED);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_RED;
            ann_custom_ColorEnum.reset();
            MemberName name_RED = "RED";
            CompleteMemberDetail detail_RED = TypeObjectUtils::build_complete_member_detail(name_RED, member_ann_builtin_RED, ann_custom_ColorEnum);
            CompleteEnumeratedLiteral literal_RED = TypeObjectUtils::build_complete_enumerated_literal(common_RED, detail_RED);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_ColorEnum, literal_RED);
        }
        {
            EnumeratedLiteralFlag flags_GREEN = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_GREEN = TypeObjectUtils::build_common_enumerated_literal(1, flags_GREEN);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_GREEN;
            ann_custom_ColorEnum.reset();
            MemberName name_GREEN = "GREEN";
            CompleteMemberDetail detail_GREEN = TypeObjectUtils::build_complete_member_detail(name_GREEN, member_ann_builtin_GREEN, ann_custom_ColorEnum);
            CompleteEnumeratedLiteral literal_GREEN = TypeObjectUtils::build_complete_enumerated_literal(common_GREEN, detail_GREEN);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_ColorEnum, literal_GREEN);
        }
        {
            EnumeratedLiteralFlag flags_BLUE = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_BLUE = TypeObjectUtils::build_common_enumerated_literal(2, flags_BLUE);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_BLUE;
            ann_custom_ColorEnum.reset();
            MemberName name_BLUE = "BLUE";
            CompleteMemberDetail detail_BLUE = TypeObjectUtils::build_complete_member_detail(name_BLUE, member_ann_builtin_BLUE, ann_custom_ColorEnum);
            CompleteEnumeratedLiteral literal_BLUE = TypeObjectUtils::build_complete_enumerated_literal(common_BLUE, detail_BLUE);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_ColorEnum, literal_BLUE);
        }
        CompleteEnumeratedType enumerated_type_ColorEnum = TypeObjectUtils::build_complete_enumerated_type(enum_flags_ColorEnum, header_ColorEnum,
                literal_seq_ColorEnum);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_enumerated_type_object(enumerated_type_ColorEnum, type_name_ColorEnum.to_string(), type_ids_ColorEnum))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                "ColorEnum already registered in TypeObjectRegistry for a different type.");
        }
    }
}// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_EnumStruct_type_identifier(
        TypeIdentifierPair& type_ids_EnumStruct)
{

    ReturnCode_t return_code_EnumStruct {eprosima::fastdds::dds::RETCODE_OK};
    return_code_EnumStruct =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "EnumStruct", type_ids_EnumStruct);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_EnumStruct)
    {
        StructTypeFlag struct_flags_EnumStruct = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_EnumStruct = "EnumStruct";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_EnumStruct;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_EnumStruct;
        AppliedAnnotationSeq tmp_ann_custom_EnumStruct;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_EnumStruct;
        if (!tmp_ann_custom_EnumStruct.empty())
        {
            ann_custom_EnumStruct = tmp_ann_custom_EnumStruct;
        }

        CompleteTypeDetail detail_EnumStruct = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_EnumStruct, ann_custom_EnumStruct, type_name_EnumStruct.to_string());
        CompleteStructHeader header_EnumStruct;
        header_EnumStruct = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_EnumStruct);
        CompleteStructMemberSeq member_seq_EnumStruct;
        {
            TypeIdentifierPair type_ids_enum_value;
            ReturnCode_t return_code_enum_value {eprosima::fastdds::dds::RETCODE_OK};
            return_code_enum_value =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "ColorEnum", type_ids_enum_value);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_enum_value)
            {
            ::register_ColorEnum_type_identifier(type_ids_enum_value);
            }
            StructMemberFlag member_flags_enum_value = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_enum_value = 0x00000000;
            bool common_enum_value_ec {false};
            CommonStructMember common_enum_value {TypeObjectUtils::build_common_struct_member(member_id_enum_value, member_flags_enum_value, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_enum_value, common_enum_value_ec))};
            if (!common_enum_value_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure enum_value member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_enum_value = "enum_value";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_enum_value;
            ann_custom_EnumStruct.reset();
            CompleteMemberDetail detail_enum_value = TypeObjectUtils::build_complete_member_detail(name_enum_value, member_ann_builtin_enum_value, ann_custom_EnumStruct);
            CompleteStructMember member_enum_value = TypeObjectUtils::build_complete_struct_member(common_enum_value, detail_enum_value);
            TypeObjectUtils::add_complete_struct_member(member_seq_EnumStruct, member_enum_value);
        }
        CompleteStructType struct_type_EnumStruct = TypeObjectUtils::build_complete_struct_type(struct_flags_EnumStruct, header_EnumStruct, member_seq_EnumStruct);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_EnumStruct, type_name_EnumStruct.to_string(), type_ids_EnumStruct))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "EnumStruct already registered in TypeObjectRegistry for a different type.");
        }
    }
}
