// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RPCTypeObjectSupport.cpp
 */

#include <fastdds/dds/rpc/RPCTypeObjectSupport.hpp>

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

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

using namespace eprosima::fastdds::dds::xtypes;

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_RpcException_type_identifier(
        TypeIdentifierPair& type_ids_RpcException)
{
    ReturnCode_t return_code_RpcException {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcException =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcException", type_ids_RpcException);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcException)
    {
        StructTypeFlag struct_flags_RpcException = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL,
            true, false);
        QualifiedTypeName type_name_RpcException = "eprosima::fastdds::dds::rpc::RpcException";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcException;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcException;
        AppliedAnnotationSeq tmp_ann_custom_RpcException;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RpcException;
        if (!tmp_ann_custom_RpcException.empty())
        {
            ann_custom_RpcException = tmp_ann_custom_RpcException;
        }

        CompleteTypeDetail detail_RpcException = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcException, ann_custom_RpcException,
            type_name_RpcException.to_string());
        CompleteStructHeader header_RpcException;
        header_RpcException = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_RpcException);
        CompleteStructMemberSeq member_seq_RpcException;
        {
            TypeIdentifierPair type_ids_message;
            ReturnCode_t return_code_message {eprosima::fastdds::dds::RETCODE_OK};
            return_code_message =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                            get_type_identifiers(
                "anonymous_string_unbounded", type_ids_message);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_message)
            {
                {
                    SBound bound = 0;
                    StringSTypeDefn string_sdefn = TypeObjectUtils::build_string_s_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_string_type_identifier(string_sdefn,
                            "anonymous_string_unbounded", type_ids_message))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                                "anonymous_string_unbounded already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_message = TypeObjectUtils::build_struct_member_flag(
                eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                false, false, false, false);
            MemberId member_id_message = 0x00000000;
            bool common_message_ec {false};
            CommonStructMember common_message {TypeObjectUtils::build_common_struct_member(member_id_message,
                                                       member_flags_message, TypeObjectUtils::retrieve_complete_type_identifier(
                                                           type_ids_message,
                                                           common_message_ec))};
            if (!common_message_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure message member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_message = "message";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_message;
            ann_custom_RpcException.reset();
            CompleteMemberDetail detail_message = TypeObjectUtils::build_complete_member_detail(name_message,
                            member_ann_builtin_message,
                            ann_custom_RpcException);
            CompleteStructMember member_message = TypeObjectUtils::build_complete_struct_member(common_message,
                            detail_message);
            TypeObjectUtils::add_complete_struct_member(member_seq_RpcException, member_message);
        }
        CompleteStructType struct_type_RpcException = TypeObjectUtils::build_complete_struct_type(
            struct_flags_RpcException, header_RpcException, member_seq_RpcException);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_RpcException,
                type_name_RpcException.to_string(), type_ids_RpcException))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcException already registered in TypeObjectRegistry for a different type.");
        }
    }
}

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_RpcBrokenPipeException_type_identifier(
        TypeIdentifierPair& type_ids_RpcBrokenPipeException)
{
    ReturnCode_t return_code_RpcBrokenPipeException {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcBrokenPipeException =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcBrokenPipeException", type_ids_RpcBrokenPipeException);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcBrokenPipeException)
    {
        StructTypeFlag struct_flags_RpcBrokenPipeException = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL,
            true, false);
        return_code_RpcBrokenPipeException =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                        get_type_identifiers(
            "eprosima::fastdds::dds::rpc::RpcException", type_ids_RpcBrokenPipeException);

        if (return_code_RpcBrokenPipeException != eprosima::fastdds::dds::RETCODE_OK)
        {
            eprosima::fastdds::dds::rpc::register_RpcException_type_identifier(type_ids_RpcBrokenPipeException);
        }
        QualifiedTypeName type_name_RpcBrokenPipeException = "eprosima::fastdds::dds::rpc::RpcBrokenPipeException";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcBrokenPipeException;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcBrokenPipeException;
        AppliedAnnotationSeq tmp_ann_custom_RpcBrokenPipeException;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RpcBrokenPipeException;
        if (!tmp_ann_custom_RpcBrokenPipeException.empty())
        {
            ann_custom_RpcBrokenPipeException = tmp_ann_custom_RpcBrokenPipeException;
        }

        CompleteTypeDetail detail_RpcBrokenPipeException = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcBrokenPipeException, ann_custom_RpcBrokenPipeException,
            type_name_RpcBrokenPipeException.to_string());
        CompleteStructHeader header_RpcBrokenPipeException;
        if (EK_COMPLETE == type_ids_RpcBrokenPipeException.type_identifier1()._d())
        {
            header_RpcBrokenPipeException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcBrokenPipeException.type_identifier1(), detail_RpcBrokenPipeException);
        }
        else if (EK_COMPLETE == type_ids_RpcBrokenPipeException.type_identifier2()._d())
        {
            header_RpcBrokenPipeException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcBrokenPipeException.type_identifier2(), detail_RpcBrokenPipeException);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcBrokenPipeException Structure: base_type TypeIdentifier registered in TypeObjectRegistry is inconsistent.");
            return;
        }
        CompleteStructMemberSeq member_seq_RpcBrokenPipeException;
        CompleteStructType struct_type_RpcBrokenPipeException = TypeObjectUtils::build_complete_struct_type(
            struct_flags_RpcBrokenPipeException, header_RpcBrokenPipeException,
            member_seq_RpcBrokenPipeException);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_RpcBrokenPipeException,
                type_name_RpcBrokenPipeException.to_string(), type_ids_RpcBrokenPipeException))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcBrokenPipeException already registered in TypeObjectRegistry for a different type.");
        }
    }
}

void register_RpcStatusCode_type_identifier(
        TypeIdentifierPair& type_ids_RpcStatusCode)
{
    ReturnCode_t return_code_RpcStatusCode {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcStatusCode =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcStatusCode", type_ids_RpcStatusCode);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcStatusCode)
    {
        AliasTypeFlag alias_flags_RpcStatusCode = 0;
        QualifiedTypeName type_name_RpcStatusCode = "eprosima::fastdds::dds::rpc::RpcStatusCode";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcStatusCode;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcStatusCode;
        CompleteTypeDetail detail_RpcStatusCode = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcStatusCode, ann_custom_RpcStatusCode,
            type_name_RpcStatusCode.to_string());
        CompleteAliasHeader header_RpcStatusCode = TypeObjectUtils::build_complete_alias_header(detail_RpcStatusCode);
        AliasMemberFlag related_flags_RpcStatusCode = 0;
        return_code_RpcStatusCode =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                        get_type_identifiers(
            "_uint32_t", type_ids_RpcStatusCode);

        if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcStatusCode)
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcStatusCode related TypeIdentifier unknown to TypeObjectRegistry.");
            return;
        }
        bool common_RpcStatusCode_ec {false};
        CommonAliasBody common_RpcStatusCode {TypeObjectUtils::build_common_alias_body(related_flags_RpcStatusCode,
                                                      TypeObjectUtils::retrieve_complete_type_identifier(
                                                          type_ids_RpcStatusCode, common_RpcStatusCode_ec))};
        if (!common_RpcStatusCode_ec)
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcStatusCode related TypeIdentifier inconsistent.");
            return;
        }
        eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_RpcStatusCode;
        ann_custom_RpcStatusCode.reset();
        CompleteAliasBody body_RpcStatusCode = TypeObjectUtils::build_complete_alias_body(common_RpcStatusCode,
                        member_ann_builtin_RpcStatusCode, ann_custom_RpcStatusCode);
        CompleteAliasType alias_type_RpcStatusCode = TypeObjectUtils::build_complete_alias_type(
            alias_flags_RpcStatusCode,
            header_RpcStatusCode, body_RpcStatusCode);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_alias_type_object(alias_type_RpcStatusCode,
                type_name_RpcStatusCode.to_string(), type_ids_RpcStatusCode))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcStatusCode already registered in TypeObjectRegistry for a different type.");
        }
    }
}

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_RpcFeedCancelledException_type_identifier(
        TypeIdentifierPair& type_ids_RpcFeedCancelledException)
{
    ReturnCode_t return_code_RpcFeedCancelledException {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcFeedCancelledException =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcFeedCancelledException", type_ids_RpcFeedCancelledException);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcFeedCancelledException)
    {
        StructTypeFlag struct_flags_RpcFeedCancelledException = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL,
            true, false);
        return_code_RpcFeedCancelledException =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                        get_type_identifiers(
            "eprosima::fastdds::dds::rpc::RpcException", type_ids_RpcFeedCancelledException);

        if (return_code_RpcFeedCancelledException != eprosima::fastdds::dds::RETCODE_OK)
        {
            eprosima::fastdds::dds::rpc::register_RpcException_type_identifier(type_ids_RpcFeedCancelledException);
        }
        QualifiedTypeName type_name_RpcFeedCancelledException =
                "eprosima::fastdds::dds::rpc::RpcFeedCancelledException";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcFeedCancelledException;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcFeedCancelledException;
        AppliedAnnotationSeq tmp_ann_custom_RpcFeedCancelledException;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RpcFeedCancelledException;
        if (!tmp_ann_custom_RpcFeedCancelledException.empty())
        {
            ann_custom_RpcFeedCancelledException = tmp_ann_custom_RpcFeedCancelledException;
        }

        CompleteTypeDetail detail_RpcFeedCancelledException = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcFeedCancelledException, ann_custom_RpcFeedCancelledException,
            type_name_RpcFeedCancelledException.to_string());
        CompleteStructHeader header_RpcFeedCancelledException;
        if (EK_COMPLETE == type_ids_RpcFeedCancelledException.type_identifier1()._d())
        {
            header_RpcFeedCancelledException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcFeedCancelledException.type_identifier1(),
                detail_RpcFeedCancelledException);
        }
        else if (EK_COMPLETE == type_ids_RpcFeedCancelledException.type_identifier2()._d())
        {
            header_RpcFeedCancelledException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcFeedCancelledException.type_identifier2(),
                detail_RpcFeedCancelledException);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcFeedCancelledException Structure: base_type TypeIdentifier registered in TypeObjectRegistry is inconsistent.");
            return;
        }
        CompleteStructMemberSeq member_seq_RpcFeedCancelledException;
        {
            TypeIdentifierPair type_ids_reason;
            ReturnCode_t return_code_reason {eprosima::fastdds::dds::RETCODE_OK};
            return_code_reason =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                            get_type_identifiers(
                "eprosima::fastdds::dds::rpc::RpcStatusCode", type_ids_reason);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_reason)
            {
                eprosima::fastdds::dds::rpc::register_RpcStatusCode_type_identifier(type_ids_reason);
            }
            StructMemberFlag member_flags_reason = TypeObjectUtils::build_struct_member_flag(
                eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                false, false, false, false);
            MemberId member_id_reason = 0x00000001;
            bool common_reason_ec {false};
            CommonStructMember common_reason {TypeObjectUtils::build_common_struct_member(member_id_reason,
                                                      member_flags_reason, TypeObjectUtils::retrieve_complete_type_identifier(
                                                          type_ids_reason,
                                                          common_reason_ec))};
            if (!common_reason_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure reason member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_reason = "reason";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_reason;
            ann_custom_RpcFeedCancelledException.reset();
            CompleteMemberDetail detail_reason = TypeObjectUtils::build_complete_member_detail(name_reason,
                            member_ann_builtin_reason,
                            ann_custom_RpcFeedCancelledException);
            CompleteStructMember member_reason = TypeObjectUtils::build_complete_struct_member(common_reason,
                            detail_reason);
            TypeObjectUtils::add_complete_struct_member(member_seq_RpcFeedCancelledException, member_reason);
        }
        CompleteStructType struct_type_RpcFeedCancelledException = TypeObjectUtils::build_complete_struct_type(
            struct_flags_RpcFeedCancelledException, header_RpcFeedCancelledException,
            member_seq_RpcFeedCancelledException);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_RpcFeedCancelledException,
                type_name_RpcFeedCancelledException.to_string(), type_ids_RpcFeedCancelledException))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcFeedCancelledException already registered in TypeObjectRegistry for a different type.");
        }
    }
}

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_RpcOperationError_type_identifier(
        TypeIdentifierPair& type_ids_RpcOperationError)
{
    ReturnCode_t return_code_RpcOperationError {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcOperationError =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcOperationError", type_ids_RpcOperationError);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcOperationError)
    {
        StructTypeFlag struct_flags_RpcOperationError = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL,
            true, false);
        return_code_RpcOperationError =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                        get_type_identifiers(
            "eprosima::fastdds::dds::rpc::RpcException", type_ids_RpcOperationError);

        if (return_code_RpcOperationError != eprosima::fastdds::dds::RETCODE_OK)
        {
            eprosima::fastdds::dds::rpc::register_RpcException_type_identifier(type_ids_RpcOperationError);
        }
        QualifiedTypeName type_name_RpcOperationError = "eprosima::fastdds::dds::rpc::RpcOperationError";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcOperationError;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcOperationError;
        AppliedAnnotationSeq tmp_ann_custom_RpcOperationError;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RpcOperationError;
        if (!tmp_ann_custom_RpcOperationError.empty())
        {
            ann_custom_RpcOperationError = tmp_ann_custom_RpcOperationError;
        }

        CompleteTypeDetail detail_RpcOperationError = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcOperationError, ann_custom_RpcOperationError,
            type_name_RpcOperationError.to_string());
        CompleteStructHeader header_RpcOperationError;
        if (EK_COMPLETE == type_ids_RpcOperationError.type_identifier1()._d())
        {
            header_RpcOperationError = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcOperationError.type_identifier1(), detail_RpcOperationError);
        }
        else if (EK_COMPLETE == type_ids_RpcOperationError.type_identifier2()._d())
        {
            header_RpcOperationError = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcOperationError.type_identifier2(), detail_RpcOperationError);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcOperationError Structure: base_type TypeIdentifier registered in TypeObjectRegistry is inconsistent.");
            return;
        }
        CompleteStructMemberSeq member_seq_RpcOperationError;
        CompleteStructType struct_type_RpcOperationError = TypeObjectUtils::build_complete_struct_type(
            struct_flags_RpcOperationError, header_RpcOperationError, member_seq_RpcOperationError);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_RpcOperationError,
                type_name_RpcOperationError.to_string(), type_ids_RpcOperationError))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcOperationError already registered in TypeObjectRegistry for a different type.");
        }
    }
}

void register_RemoteExceptionCode_t_type_identifier(
        TypeIdentifierPair& type_ids_RemoteExceptionCode_t)
{
    ReturnCode_t return_code_RemoteExceptionCode_t {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RemoteExceptionCode_t =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RemoteExceptionCode_t", type_ids_RemoteExceptionCode_t);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RemoteExceptionCode_t)
    {
        EnumTypeFlag enum_flags_RemoteExceptionCode_t = 0;
        BitBound bit_bound_RemoteExceptionCode_t = 32;
        CommonEnumeratedHeader common_RemoteExceptionCode_t = TypeObjectUtils::build_common_enumerated_header(
            bit_bound_RemoteExceptionCode_t);
        QualifiedTypeName type_name_RemoteExceptionCode_t = "eprosima::fastdds::dds::rpc::RemoteExceptionCode_t";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RemoteExceptionCode_t;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RemoteExceptionCode_t;
        AppliedAnnotationSeq tmp_ann_custom_RemoteExceptionCode_t;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RemoteExceptionCode_t;
        if (!tmp_ann_custom_RemoteExceptionCode_t.empty())
        {
            ann_custom_RemoteExceptionCode_t = tmp_ann_custom_RemoteExceptionCode_t;
        }

        CompleteTypeDetail detail_RemoteExceptionCode_t = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RemoteExceptionCode_t, ann_custom_RemoteExceptionCode_t,
            type_name_RemoteExceptionCode_t.to_string());
        CompleteEnumeratedHeader header_RemoteExceptionCode_t = TypeObjectUtils::build_complete_enumerated_header(
            common_RemoteExceptionCode_t, detail_RemoteExceptionCode_t);
        CompleteEnumeratedLiteralSeq literal_seq_RemoteExceptionCode_t;
        {
            EnumeratedLiteralFlag flags_REMOTE_EX_OK = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_REMOTE_EX_OK = TypeObjectUtils::build_common_enumerated_literal(0,
                            flags_REMOTE_EX_OK);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_REMOTE_EX_OK;
            ann_custom_RemoteExceptionCode_t.reset();
            MemberName name_REMOTE_EX_OK = "REMOTE_EX_OK";
            CompleteMemberDetail detail_REMOTE_EX_OK = TypeObjectUtils::build_complete_member_detail(name_REMOTE_EX_OK,
                            member_ann_builtin_REMOTE_EX_OK,
                            ann_custom_RemoteExceptionCode_t);
            CompleteEnumeratedLiteral literal_REMOTE_EX_OK = TypeObjectUtils::build_complete_enumerated_literal(
                common_REMOTE_EX_OK, detail_REMOTE_EX_OK);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_RemoteExceptionCode_t, literal_REMOTE_EX_OK);
        }
        {
            EnumeratedLiteralFlag flags_REMOTE_EX_UNSUPPORTED = TypeObjectUtils::build_enumerated_literal_flag(false);
            CommonEnumeratedLiteral common_REMOTE_EX_UNSUPPORTED = TypeObjectUtils::build_common_enumerated_literal(1,
                            flags_REMOTE_EX_UNSUPPORTED);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_REMOTE_EX_UNSUPPORTED;
            ann_custom_RemoteExceptionCode_t.reset();
            MemberName name_REMOTE_EX_UNSUPPORTED = "REMOTE_EX_UNSUPPORTED";
            CompleteMemberDetail detail_REMOTE_EX_UNSUPPORTED = TypeObjectUtils::build_complete_member_detail(
                name_REMOTE_EX_UNSUPPORTED, member_ann_builtin_REMOTE_EX_UNSUPPORTED,
                ann_custom_RemoteExceptionCode_t);
            CompleteEnumeratedLiteral literal_REMOTE_EX_UNSUPPORTED =
                    TypeObjectUtils::build_complete_enumerated_literal(common_REMOTE_EX_UNSUPPORTED,
                            detail_REMOTE_EX_UNSUPPORTED);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_RemoteExceptionCode_t,
                    literal_REMOTE_EX_UNSUPPORTED);
        }
        {
            EnumeratedLiteralFlag flags_REMOTE_EX_INVALID_ARGUMENT = TypeObjectUtils::build_enumerated_literal_flag(
                false);
            CommonEnumeratedLiteral common_REMOTE_EX_INVALID_ARGUMENT =
                    TypeObjectUtils::build_common_enumerated_literal(2,
                            flags_REMOTE_EX_INVALID_ARGUMENT);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_REMOTE_EX_INVALID_ARGUMENT;
            ann_custom_RemoteExceptionCode_t.reset();
            MemberName name_REMOTE_EX_INVALID_ARGUMENT = "REMOTE_EX_INVALID_ARGUMENT";
            CompleteMemberDetail detail_REMOTE_EX_INVALID_ARGUMENT = TypeObjectUtils::build_complete_member_detail(
                name_REMOTE_EX_INVALID_ARGUMENT, member_ann_builtin_REMOTE_EX_INVALID_ARGUMENT,
                ann_custom_RemoteExceptionCode_t);
            CompleteEnumeratedLiteral literal_REMOTE_EX_INVALID_ARGUMENT =
                    TypeObjectUtils::build_complete_enumerated_literal(
                common_REMOTE_EX_INVALID_ARGUMENT, detail_REMOTE_EX_INVALID_ARGUMENT);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_RemoteExceptionCode_t,
                    literal_REMOTE_EX_INVALID_ARGUMENT);
        }
        {
            EnumeratedLiteralFlag flags_REMOTE_EX_OUT_OF_RESOURCES = TypeObjectUtils::build_enumerated_literal_flag(
                false);
            CommonEnumeratedLiteral common_REMOTE_EX_OUT_OF_RESOURCES =
                    TypeObjectUtils::build_common_enumerated_literal(3,
                            flags_REMOTE_EX_OUT_OF_RESOURCES);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_REMOTE_EX_OUT_OF_RESOURCES;
            ann_custom_RemoteExceptionCode_t.reset();
            MemberName name_REMOTE_EX_OUT_OF_RESOURCES = "REMOTE_EX_OUT_OF_RESOURCES";
            CompleteMemberDetail detail_REMOTE_EX_OUT_OF_RESOURCES = TypeObjectUtils::build_complete_member_detail(
                name_REMOTE_EX_OUT_OF_RESOURCES, member_ann_builtin_REMOTE_EX_OUT_OF_RESOURCES,
                ann_custom_RemoteExceptionCode_t);
            CompleteEnumeratedLiteral literal_REMOTE_EX_OUT_OF_RESOURCES =
                    TypeObjectUtils::build_complete_enumerated_literal(
                common_REMOTE_EX_OUT_OF_RESOURCES, detail_REMOTE_EX_OUT_OF_RESOURCES);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_RemoteExceptionCode_t,
                    literal_REMOTE_EX_OUT_OF_RESOURCES);
        }
        {
            EnumeratedLiteralFlag flags_REMOTE_EX_UNKNOWN_OPERATION = TypeObjectUtils::build_enumerated_literal_flag(
                false);
            CommonEnumeratedLiteral common_REMOTE_EX_UNKNOWN_OPERATION =
                    TypeObjectUtils::build_common_enumerated_literal(4,
                            flags_REMOTE_EX_UNKNOWN_OPERATION);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_REMOTE_EX_UNKNOWN_OPERATION;
            ann_custom_RemoteExceptionCode_t.reset();
            MemberName name_REMOTE_EX_UNKNOWN_OPERATION = "REMOTE_EX_UNKNOWN_OPERATION";
            CompleteMemberDetail detail_REMOTE_EX_UNKNOWN_OPERATION = TypeObjectUtils::build_complete_member_detail(
                name_REMOTE_EX_UNKNOWN_OPERATION, member_ann_builtin_REMOTE_EX_UNKNOWN_OPERATION,
                ann_custom_RemoteExceptionCode_t);
            CompleteEnumeratedLiteral literal_REMOTE_EX_UNKNOWN_OPERATION =
                    TypeObjectUtils::build_complete_enumerated_literal(
                common_REMOTE_EX_UNKNOWN_OPERATION, detail_REMOTE_EX_UNKNOWN_OPERATION);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_RemoteExceptionCode_t,
                    literal_REMOTE_EX_UNKNOWN_OPERATION);
        }
        {
            EnumeratedLiteralFlag flags_REMOTE_EX_UNKNOWN_EXCEPTION = TypeObjectUtils::build_enumerated_literal_flag(
                false);
            CommonEnumeratedLiteral common_REMOTE_EX_UNKNOWN_EXCEPTION =
                    TypeObjectUtils::build_common_enumerated_literal(5,
                            flags_REMOTE_EX_UNKNOWN_EXCEPTION);
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_REMOTE_EX_UNKNOWN_EXCEPTION;
            ann_custom_RemoteExceptionCode_t.reset();
            MemberName name_REMOTE_EX_UNKNOWN_EXCEPTION = "REMOTE_EX_UNKNOWN_EXCEPTION";
            CompleteMemberDetail detail_REMOTE_EX_UNKNOWN_EXCEPTION = TypeObjectUtils::build_complete_member_detail(
                name_REMOTE_EX_UNKNOWN_EXCEPTION, member_ann_builtin_REMOTE_EX_UNKNOWN_EXCEPTION,
                ann_custom_RemoteExceptionCode_t);
            CompleteEnumeratedLiteral literal_REMOTE_EX_UNKNOWN_EXCEPTION =
                    TypeObjectUtils::build_complete_enumerated_literal(
                common_REMOTE_EX_UNKNOWN_EXCEPTION, detail_REMOTE_EX_UNKNOWN_EXCEPTION);
            TypeObjectUtils::add_complete_enumerated_literal(literal_seq_RemoteExceptionCode_t,
                    literal_REMOTE_EX_UNKNOWN_EXCEPTION);
        }
        CompleteEnumeratedType enumerated_type_RemoteExceptionCode_t = TypeObjectUtils::build_complete_enumerated_type(
            enum_flags_RemoteExceptionCode_t, header_RemoteExceptionCode_t,
            literal_seq_RemoteExceptionCode_t);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_enumerated_type_object(enumerated_type_RemoteExceptionCode_t,
                type_name_RemoteExceptionCode_t.to_string(), type_ids_RemoteExceptionCode_t))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RemoteExceptionCode_t already registered in TypeObjectRegistry for a different type.");
        }
    }
}

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_RpcRemoteException_type_identifier(
        TypeIdentifierPair& type_ids_RpcRemoteException)
{
    ReturnCode_t return_code_RpcRemoteException {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcRemoteException =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcRemoteException", type_ids_RpcRemoteException);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcRemoteException)
    {
        StructTypeFlag struct_flags_RpcRemoteException = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL,
            true, false);
        return_code_RpcRemoteException =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                        get_type_identifiers(
            "eprosima::fastdds::dds::rpc::RpcException", type_ids_RpcRemoteException);

        if (return_code_RpcRemoteException != eprosima::fastdds::dds::RETCODE_OK)
        {
            eprosima::fastdds::dds::rpc::register_RpcException_type_identifier(type_ids_RpcRemoteException);
        }
        QualifiedTypeName type_name_RpcRemoteException = "eprosima::fastdds::dds::rpc::RpcRemoteException";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcRemoteException;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcRemoteException;
        AppliedAnnotationSeq tmp_ann_custom_RpcRemoteException;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RpcRemoteException;
        if (!tmp_ann_custom_RpcRemoteException.empty())
        {
            ann_custom_RpcRemoteException = tmp_ann_custom_RpcRemoteException;
        }

        CompleteTypeDetail detail_RpcRemoteException = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcRemoteException, ann_custom_RpcRemoteException,
            type_name_RpcRemoteException.to_string());
        CompleteStructHeader header_RpcRemoteException;
        if (EK_COMPLETE == type_ids_RpcRemoteException.type_identifier1()._d())
        {
            header_RpcRemoteException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcRemoteException.type_identifier1(), detail_RpcRemoteException);
        }
        else if (EK_COMPLETE == type_ids_RpcRemoteException.type_identifier2()._d())
        {
            header_RpcRemoteException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcRemoteException.type_identifier2(), detail_RpcRemoteException);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcRemoteException Structure: base_type TypeIdentifier registered in TypeObjectRegistry is inconsistent.");
            return;
        }
        CompleteStructMemberSeq member_seq_RpcRemoteException;
        {
            TypeIdentifierPair type_ids_code;
            ReturnCode_t return_code_code {eprosima::fastdds::dds::RETCODE_OK};
            return_code_code =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                            get_type_identifiers(
                "eprosima::fastdds::dds::rpc::RemoteExceptionCode_t", type_ids_code);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_code)
            {
                eprosima::fastdds::dds::rpc::register_RemoteExceptionCode_t_type_identifier(type_ids_code);
            }
            StructMemberFlag member_flags_code = TypeObjectUtils::build_struct_member_flag(
                eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                false, false, false, false);
            MemberId member_id_code = 0x00000001;
            bool common_code_ec {false};
            CommonStructMember common_code {TypeObjectUtils::build_common_struct_member(member_id_code,
                                                    member_flags_code, TypeObjectUtils::retrieve_complete_type_identifier(
                                                        type_ids_code,
                                                        common_code_ec))};
            if (!common_code_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure code member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_code = "code";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_code;
            ann_custom_RpcRemoteException.reset();
            CompleteMemberDetail detail_code = TypeObjectUtils::build_complete_member_detail(name_code,
                            member_ann_builtin_code,
                            ann_custom_RpcRemoteException);
            CompleteStructMember member_code = TypeObjectUtils::build_complete_struct_member(common_code, detail_code);
            TypeObjectUtils::add_complete_struct_member(member_seq_RpcRemoteException, member_code);
        }
        CompleteStructType struct_type_RpcRemoteException = TypeObjectUtils::build_complete_struct_type(
            struct_flags_RpcRemoteException, header_RpcRemoteException,
            member_seq_RpcRemoteException);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_RpcRemoteException,
                type_name_RpcRemoteException.to_string(), type_ids_RpcRemoteException))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcRemoteException already registered in TypeObjectRegistry for a different type.");
        }
    }
}

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_RpcTimeoutException_type_identifier(
        TypeIdentifierPair& type_ids_RpcTimeoutException)
{

    ReturnCode_t return_code_RpcTimeoutException {eprosima::fastdds::dds::RETCODE_OK};
    return_code_RpcTimeoutException =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "eprosima::fastdds::dds::rpc::RpcTimeoutException", type_ids_RpcTimeoutException);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_RpcTimeoutException)
    {
        StructTypeFlag struct_flags_RpcTimeoutException = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::FINAL,
            true, false);
        return_code_RpcTimeoutException =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                        get_type_identifiers(
            "eprosima::fastdds::dds::rpc::RpcException", type_ids_RpcTimeoutException);

        if (return_code_RpcTimeoutException != eprosima::fastdds::dds::RETCODE_OK)
        {
            eprosima::fastdds::dds::rpc::register_RpcException_type_identifier(type_ids_RpcTimeoutException);
        }
        QualifiedTypeName type_name_RpcTimeoutException = "eprosima::fastdds::dds::rpc::RpcTimeoutException";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_RpcTimeoutException;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_RpcTimeoutException;
        AppliedAnnotationSeq tmp_ann_custom_RpcTimeoutException;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_RpcTimeoutException;
        if (!tmp_ann_custom_RpcTimeoutException.empty())
        {
            ann_custom_RpcTimeoutException = tmp_ann_custom_RpcTimeoutException;
        }

        CompleteTypeDetail detail_RpcTimeoutException = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_RpcTimeoutException, ann_custom_RpcTimeoutException,
            type_name_RpcTimeoutException.to_string());
        CompleteStructHeader header_RpcTimeoutException;
        if (EK_COMPLETE == type_ids_RpcTimeoutException.type_identifier1()._d())
        {
            header_RpcTimeoutException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcTimeoutException.type_identifier1(), detail_RpcTimeoutException);
        }
        else if (EK_COMPLETE == type_ids_RpcTimeoutException.type_identifier2()._d())
        {
            header_RpcTimeoutException = TypeObjectUtils::build_complete_struct_header(
                type_ids_RpcTimeoutException.type_identifier2(), detail_RpcTimeoutException);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcTimeoutException Structure: base_type TypeIdentifier registered in TypeObjectRegistry is inconsistent.");
            return;
        }
        CompleteStructMemberSeq member_seq_RpcTimeoutException;
        CompleteStructType struct_type_RpcTimeoutException = TypeObjectUtils::build_complete_struct_type(
            struct_flags_RpcTimeoutException, header_RpcTimeoutException,
            member_seq_RpcTimeoutException);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_RpcTimeoutException,
                type_name_RpcTimeoutException.to_string(), type_ids_RpcTimeoutException))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "eprosima::fastdds::dds::rpc::RpcTimeoutException already registered in TypeObjectRegistry for a different type.");
        }
    }
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
