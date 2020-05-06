// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_TYPE_OBJECT_H
#define TYPES_TYPE_OBJECT_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/TypeObjectHashId.h>
#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/AnnotationParameterValue.h>
#include <cstdint>
#include <array>

namespace eprosima {
namespace fastcdr {
class Cdr;
}
}

// The types in this file shall be serialized with XCDR encoding version 2
namespace eprosima {
namespace fastrtps {

namespace types {

/*struct CommonStructMember final {
    MemberId                                   member_id;
    StructMemberFlag                           member_flags;
    TypeIdentifier                             member_type_id;
   };*/
class CommonStructMember
{
public:

    RTPS_DllAPI CommonStructMember();
    RTPS_DllAPI ~CommonStructMember();
    RTPS_DllAPI CommonStructMember(
            const CommonStructMember& x);
    RTPS_DllAPI CommonStructMember(
            CommonStructMember&& x);
    RTPS_DllAPI CommonStructMember& operator=(
            const CommonStructMember& x);
    RTPS_DllAPI CommonStructMember& operator=(
            CommonStructMember&& x);

    RTPS_DllAPI inline void member_id(
            const MemberId& _member_id) { m_member_id = _member_id; }
    RTPS_DllAPI inline void member_id(
            MemberId&& _member_id) { m_member_id = std::move(_member_id); }
    RTPS_DllAPI inline const MemberId& member_id() const { return m_member_id; }
    RTPS_DllAPI inline MemberId& member_id() { return m_member_id; }

    RTPS_DllAPI inline void member_flags(
            const StructMemberFlag& _member_flags) { m_member_flags = _member_flags; }
    RTPS_DllAPI inline void member_flags(
            StructMemberFlag&& _member_flags) { m_member_flags = std::move(_member_flags); }
    RTPS_DllAPI inline const StructMemberFlag& member_flags() const { return m_member_flags; }
    RTPS_DllAPI inline StructMemberFlag& member_flags() { return m_member_flags; }

    RTPS_DllAPI inline void member_type_id(
            const TypeIdentifier& _member_type_id) { m_member_type_id = _member_type_id; }
    RTPS_DllAPI inline void member_type_id(
            TypeIdentifier&& _member_type_id) { m_member_type_id = std::move(_member_type_id); }
    RTPS_DllAPI inline const TypeIdentifier& member_type_id() const { return m_member_type_id; }
    RTPS_DllAPI inline TypeIdentifier& member_type_id() { return m_member_type_id; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonStructMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonStructMember& other) const;

    RTPS_DllAPI bool consistent(
            const CommonStructMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    MemberId m_member_id;
    StructMemberFlag m_member_flags;
    TypeIdentifier m_member_type_id;
};

// COMPLETE Details for a member of an aggregate type
/*struct CompleteMemberDetail final{
    MemberName                                 name;
    AppliedBuiltinMemberAnnotations  ann_builtin; // Optional
    AppliedAnnotationSeq             ann_custom; // Optional
   };*/
class CompleteMemberDetail
{
public:

    RTPS_DllAPI CompleteMemberDetail();
    RTPS_DllAPI ~CompleteMemberDetail();
    RTPS_DllAPI CompleteMemberDetail(
            const CompleteMemberDetail& x);
    RTPS_DllAPI CompleteMemberDetail(
            CompleteMemberDetail&& x);
    RTPS_DllAPI CompleteMemberDetail& operator=(
            const CompleteMemberDetail& x);
    RTPS_DllAPI CompleteMemberDetail& operator=(
            CompleteMemberDetail&& x);

    RTPS_DllAPI inline void name(
            const MemberName& _name) { m_name = _name; }
    RTPS_DllAPI inline void name(
            MemberName&& _name) { m_name = std::move(_name); }
    RTPS_DllAPI inline const MemberName& name() const { return m_name; }
    RTPS_DllAPI inline MemberName& name() { return m_name; }

    RTPS_DllAPI inline void ann_builtin(
            const AppliedBuiltinMemberAnnotations& _ann_builtin) { m_ann_builtin = _ann_builtin; }
    RTPS_DllAPI inline void ann_builtin(
            AppliedBuiltinMemberAnnotations&& _ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    RTPS_DllAPI inline const AppliedBuiltinMemberAnnotations& ann_builtin() const { return m_ann_builtin; }
    RTPS_DllAPI inline AppliedBuiltinMemberAnnotations& ann_builtin() { return m_ann_builtin; }

    RTPS_DllAPI inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom) { m_ann_custom = _ann_custom; }
    RTPS_DllAPI inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom) { m_ann_custom = std::move(_ann_custom); }
    RTPS_DllAPI inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    RTPS_DllAPI inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteMemberDetail& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteMemberDetail& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteMemberDetail& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    MemberName m_name;
    AppliedBuiltinMemberAnnotations m_ann_builtin;
    AppliedAnnotationSeq m_ann_custom;
};

// MINIMAL Details for a member of an aggregate type
/*struct MinimalMemberDetail final{
    NameHash                                  name_hash;
   };*/

class MinimalMemberDetail final
{
public:

    RTPS_DllAPI MinimalMemberDetail();
    RTPS_DllAPI ~MinimalMemberDetail();
    RTPS_DllAPI MinimalMemberDetail(
            const MinimalMemberDetail& x);
    RTPS_DllAPI MinimalMemberDetail(
            MinimalMemberDetail&& x);
    RTPS_DllAPI MinimalMemberDetail& operator=(
            const MinimalMemberDetail& x);
    RTPS_DllAPI MinimalMemberDetail& operator=(
            MinimalMemberDetail&& x);

    RTPS_DllAPI inline void name_hash(
            const NameHash& _name_hash) { m_name_hash = _name_hash; }
    RTPS_DllAPI inline void name_hash(
            NameHash&& _name_hash) { m_name_hash = std::move(_name_hash); }
    RTPS_DllAPI inline const NameHash& name_hash() const { return m_name_hash; }
    RTPS_DllAPI inline NameHash& name_hash() { return m_name_hash; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalMemberDetail& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalMemberDetail& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalMemberDetail& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    NameHash m_name_hash;
};

// Member of an aggregate type
/*struct CompleteStructMember {
    CommonStructMember                         common;
    CompleteMemberDetail                       detail;
   };*/
class CompleteStructMember
{
public:

    RTPS_DllAPI CompleteStructMember();
    RTPS_DllAPI ~CompleteStructMember();
    RTPS_DllAPI CompleteStructMember(
            const CompleteStructMember& x);
    RTPS_DllAPI CompleteStructMember(
            CompleteStructMember&& x);
    RTPS_DllAPI CompleteStructMember& operator=(
            const CompleteStructMember& x);
    RTPS_DllAPI CompleteStructMember& operator=(
            CompleteStructMember&& x);

    RTPS_DllAPI inline void common(
            const CommonStructMember& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonStructMember&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonStructMember& common() const { return m_common; }
    RTPS_DllAPI inline CommonStructMember& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteStructMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteStructMember& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteStructMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonStructMember m_common;
    CompleteMemberDetail m_detail;
};

// Ordered by the member_index
typedef std::vector<CompleteStructMember> CompleteStructMemberSeq;

// Member of an aggregate type
/*struct MinimalStructMember {
    CommonStructMember                         common;
    MinimalMemberDetail                        detail;
   };*/
class MinimalStructMember
{
public:

    RTPS_DllAPI MinimalStructMember();
    RTPS_DllAPI ~MinimalStructMember();
    RTPS_DllAPI MinimalStructMember(
            const MinimalStructMember& x);
    RTPS_DllAPI MinimalStructMember(
            MinimalStructMember&& x);
    RTPS_DllAPI MinimalStructMember& operator=(
            const MinimalStructMember& x);
    RTPS_DllAPI MinimalStructMember& operator=(
            MinimalStructMember&& x);

    RTPS_DllAPI inline void common(
            const CommonStructMember& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonStructMember&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonStructMember& common() const { return m_common; }
    RTPS_DllAPI inline CommonStructMember& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const MinimalMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            MinimalMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const MinimalMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline MinimalMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalStructMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalStructMember& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalStructMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonStructMember m_common;
    MinimalMemberDetail m_detail;
};

// Ordered by common.member_id
typedef std::vector<MinimalStructMember> MinimalStructMemberSeq;

/*struct AppliedBuiltinTypeAnnotations {
    AppliedVerbatimAnnotation verbatim;  // verbatim(...) // optional
   };*/
class AppliedBuiltinTypeAnnotations
{
public:

    RTPS_DllAPI AppliedBuiltinTypeAnnotations();
    RTPS_DllAPI ~AppliedBuiltinTypeAnnotations();
    RTPS_DllAPI AppliedBuiltinTypeAnnotations(
            const AppliedBuiltinTypeAnnotations& x);
    RTPS_DllAPI AppliedBuiltinTypeAnnotations(
            AppliedBuiltinTypeAnnotations&& x);
    RTPS_DllAPI AppliedBuiltinTypeAnnotations& operator=(
            const AppliedBuiltinTypeAnnotations& x);
    RTPS_DllAPI AppliedBuiltinTypeAnnotations& operator=(
            AppliedBuiltinTypeAnnotations&& x);

    RTPS_DllAPI inline void verbatim(
            const AppliedVerbatimAnnotation& _verbatim) { m_verbatim = _verbatim; }
    RTPS_DllAPI inline void verbatim(
            AppliedVerbatimAnnotation&& _verbatim) { m_verbatim = std::move(_verbatim); }
    RTPS_DllAPI inline const AppliedVerbatimAnnotation& verbatim() const { return m_verbatim; }
    RTPS_DllAPI inline AppliedVerbatimAnnotation& verbatim() { return m_verbatim; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const AppliedBuiltinTypeAnnotations& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const AppliedBuiltinTypeAnnotations& other) const;

    RTPS_DllAPI bool consistent(
            const AppliedBuiltinTypeAnnotations& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AppliedVerbatimAnnotation m_verbatim;
};

/*struct MinimalTypeDetail final{
    // Empty. Available for future extension
   };*/
class MinimalTypeDetail
{
public:

    RTPS_DllAPI MinimalTypeDetail();
    RTPS_DllAPI ~MinimalTypeDetail();
    RTPS_DllAPI MinimalTypeDetail(
            const MinimalTypeDetail& x);
    RTPS_DllAPI MinimalTypeDetail(
            MinimalTypeDetail&& x);
    RTPS_DllAPI MinimalTypeDetail& operator=(
            const MinimalTypeDetail& x);
    RTPS_DllAPI MinimalTypeDetail& operator=(
            MinimalTypeDetail&& x);

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalTypeDetail& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalTypeDetail&) const { return true; }

    RTPS_DllAPI bool consistent(
            const MinimalTypeDetail& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

};

/*struct CompleteTypeDetail final{
    AppliedBuiltinTypeAnnotations  ann_builtin; // optional
    AppliedAnnotationSeq           ann_custom; // optional
    QualifiedTypeName                        type_name;
   };*/
class CompleteTypeDetail
{
public:

    RTPS_DllAPI CompleteTypeDetail();
    RTPS_DllAPI ~CompleteTypeDetail();
    RTPS_DllAPI CompleteTypeDetail(
            const CompleteTypeDetail& x);
    RTPS_DllAPI CompleteTypeDetail(
            CompleteTypeDetail&& x);
    RTPS_DllAPI CompleteTypeDetail& operator=(
            const CompleteTypeDetail& x);
    RTPS_DllAPI CompleteTypeDetail& operator=(
            CompleteTypeDetail&& x);

    RTPS_DllAPI inline void ann_builtin(
            const AppliedBuiltinTypeAnnotations& _ann_builtin) { m_ann_builtin = _ann_builtin; }
    RTPS_DllAPI inline void ann_builtin(
            AppliedBuiltinTypeAnnotations&& _ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    RTPS_DllAPI inline const AppliedBuiltinTypeAnnotations& ann_builtin() const { return m_ann_builtin; }
    RTPS_DllAPI inline AppliedBuiltinTypeAnnotations& ann_builtin() { return m_ann_builtin; }

    RTPS_DllAPI inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom) { m_ann_custom = _ann_custom; }
    RTPS_DllAPI inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom) { m_ann_custom = std::move(_ann_custom); }
    RTPS_DllAPI inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    RTPS_DllAPI inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    RTPS_DllAPI inline void type_name(
            const QualifiedTypeName& _type_name) { m_type_name = _type_name; }
    RTPS_DllAPI inline void type_name(
            QualifiedTypeName&& _type_name) { m_type_name = std::move(_type_name); }
    RTPS_DllAPI inline const QualifiedTypeName& type_name() const { return m_type_name; }
    RTPS_DllAPI inline QualifiedTypeName& type_name() { return m_type_name; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteTypeDetail& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteTypeDetail& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteTypeDetail& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AppliedBuiltinTypeAnnotations m_ann_builtin;
    AppliedAnnotationSeq m_ann_custom;
    QualifiedTypeName m_type_name;
};
/*struct CompleteStructHeader {
    TypeIdentifier                           base_type;
    CompleteTypeDetail                       detail;
   };*/
class CompleteStructHeader
{
public:

    RTPS_DllAPI CompleteStructHeader();
    RTPS_DllAPI ~CompleteStructHeader();
    RTPS_DllAPI CompleteStructHeader(
            const CompleteStructHeader& x);
    RTPS_DllAPI CompleteStructHeader(
            CompleteStructHeader&& x);
    RTPS_DllAPI CompleteStructHeader& operator=(
            const CompleteStructHeader& x);
    RTPS_DllAPI CompleteStructHeader& operator=(
            CompleteStructHeader&& x);

    RTPS_DllAPI inline void base_type(
            const TypeIdentifier& _base_type) { m_base_type = _base_type; }
    RTPS_DllAPI inline void base_type(
            TypeIdentifier&& _base_type) { m_base_type = std::move(_base_type); }
    RTPS_DllAPI inline const TypeIdentifier& base_type() const { return m_base_type; }
    RTPS_DllAPI inline TypeIdentifier& base_type() { return m_base_type; }

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteStructHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteStructHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteStructHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    TypeIdentifier m_base_type;
    CompleteTypeDetail m_detail;
};
/*struct MinimalStructHeader {
    TypeIdentifier                           base_type;
    MinimalTypeDetail                        detail;
   };*/
class MinimalStructHeader
{
public:

    RTPS_DllAPI MinimalStructHeader();
    RTPS_DllAPI ~MinimalStructHeader();
    RTPS_DllAPI MinimalStructHeader(
            const MinimalStructHeader& x);
    RTPS_DllAPI MinimalStructHeader(
            MinimalStructHeader&& x);
    RTPS_DllAPI MinimalStructHeader& operator=(
            const MinimalStructHeader& x);
    RTPS_DllAPI MinimalStructHeader& operator=(
            MinimalStructHeader&& x);

    RTPS_DllAPI inline void base_type(
            const TypeIdentifier& _base_type) { m_base_type = _base_type; }
    RTPS_DllAPI inline void base_type(
            TypeIdentifier&& _base_type) { m_base_type = std::move(_base_type); }
    RTPS_DllAPI inline const TypeIdentifier& base_type() const { return m_base_type; }
    RTPS_DllAPI inline TypeIdentifier& base_type() { return m_base_type; }

    RTPS_DllAPI inline void detail(
            const MinimalTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            MinimalTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const MinimalTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline MinimalTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalStructHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalStructHeader& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalStructHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    TypeIdentifier m_base_type;
    MinimalTypeDetail m_detail;
};

/*struct CompleteStructType final{
    StructTypeFlag             struct_flags;
    CompleteStructHeader       header;
    CompleteStructMemberSeq    member_seq;
   };*/
class CompleteStructType
{
public:

    RTPS_DllAPI CompleteStructType();
    RTPS_DllAPI ~CompleteStructType();
    RTPS_DllAPI CompleteStructType(
            const CompleteStructType& x);
    RTPS_DllAPI CompleteStructType(
            CompleteStructType&& x);
    RTPS_DllAPI CompleteStructType& operator=(
            const CompleteStructType& x);
    RTPS_DllAPI CompleteStructType& operator=(
            CompleteStructType&& x);

    RTPS_DllAPI inline void struct_flags(
            const StructTypeFlag& _struct_flags) { m_struct_flags = _struct_flags; }
    RTPS_DllAPI inline void struct_flags(
            StructTypeFlag&& _struct_flags) { m_struct_flags = std::move(_struct_flags); }
    RTPS_DllAPI inline const StructTypeFlag& struct_flags() const { return m_struct_flags; }
    RTPS_DllAPI inline StructTypeFlag& struct_flags() { return m_struct_flags; }

    RTPS_DllAPI inline void header(
            const CompleteStructHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteStructHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteStructHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteStructHeader& header() { return m_header; }

    RTPS_DllAPI inline void member_seq(
            const CompleteStructMemberSeq& _member_seq) { m_member_seq = _member_seq; }
    RTPS_DllAPI inline void member_seq(
            CompleteStructMemberSeq&& _member_seq) { m_member_seq = std::move(_member_seq); }
    RTPS_DllAPI inline const CompleteStructMemberSeq& member_seq() const { return m_member_seq; }
    RTPS_DllAPI inline CompleteStructMemberSeq& member_seq() { return m_member_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteStructType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteStructType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteStructType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    StructTypeFlag m_struct_flags;
    CompleteStructHeader m_header;
    CompleteStructMemberSeq m_member_seq;
};

/*struct MinimalStructType final{
    StructTypeFlag             struct_flags;
    MinimalStructHeader        header;
    MinimalStructMemberSeq     member_seq;
   };*/
class MinimalStructType
{
public:

    RTPS_DllAPI MinimalStructType();
    RTPS_DllAPI ~MinimalStructType();
    RTPS_DllAPI MinimalStructType(
            const MinimalStructType& x);
    RTPS_DllAPI MinimalStructType(
            MinimalStructType&& x);
    RTPS_DllAPI MinimalStructType& operator=(
            const MinimalStructType& x);
    RTPS_DllAPI MinimalStructType& operator=(
            MinimalStructType&& x);

    RTPS_DllAPI inline void struct_flags(
            const StructTypeFlag& _struct_flags) { m_struct_flags = _struct_flags; }
    RTPS_DllAPI inline void struct_flags(
            StructTypeFlag&& _struct_flags) { m_struct_flags = std::move(_struct_flags); }
    RTPS_DllAPI inline const StructTypeFlag& struct_flags() const { return m_struct_flags; }
    RTPS_DllAPI inline StructTypeFlag& struct_flags() { return m_struct_flags; }

    RTPS_DllAPI inline void header(
            const MinimalStructHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalStructHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalStructHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalStructHeader& header() { return m_header; }

    RTPS_DllAPI inline void member_seq(
            const MinimalStructMemberSeq& _member_seq) { m_member_seq = _member_seq; }
    RTPS_DllAPI inline void member_seq(
            MinimalStructMemberSeq&& _member_seq) { m_member_seq = std::move(_member_seq); }
    RTPS_DllAPI inline const MinimalStructMemberSeq& member_seq() const { return m_member_seq; }
    RTPS_DllAPI inline MinimalStructMemberSeq& member_seq() { return m_member_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalStructType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalStructType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalStructType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    StructTypeFlag m_struct_flags;
    MinimalStructHeader m_header;
    MinimalStructMemberSeq m_member_seq;
};

// --- Union: ---------------------------------------------------------

// Case labels that apply to a member of a union type
// Ordered by their values
typedef std::vector<int32_t> UnionCaseLabelSeq;

/*struct CommonUnionMember final{
    MemberId                    member_id;
    UnionMemberFlag             member_flags;
    TypeIdentifier              type_id;
    UnionCaseLabelSeq           label_seq;
   };*/
class CommonUnionMember
{
public:

    RTPS_DllAPI CommonUnionMember();
    RTPS_DllAPI ~CommonUnionMember();
    RTPS_DllAPI CommonUnionMember(
            const CommonUnionMember& x);
    RTPS_DllAPI CommonUnionMember(
            CommonUnionMember&& x);
    RTPS_DllAPI CommonUnionMember& operator=(
            const CommonUnionMember& x);
    RTPS_DllAPI CommonUnionMember& operator=(
            CommonUnionMember&& x);

    RTPS_DllAPI inline void member_id(
            const MemberId& _member_id) { m_member_id = _member_id; }
    RTPS_DllAPI inline void member_id(
            MemberId&& _member_id) { m_member_id = std::move(_member_id); }
    RTPS_DllAPI inline const MemberId& member_id() const { return m_member_id; }
    RTPS_DllAPI inline MemberId& member_id() { return m_member_id; }

    RTPS_DllAPI inline void member_flags(
            const UnionMemberFlag& _member_flags) { m_member_flags = _member_flags; }
    RTPS_DllAPI inline void member_flags(
            UnionMemberFlag&& _member_flags) { m_member_flags = std::move(_member_flags); }
    RTPS_DllAPI inline const UnionMemberFlag& member_flags() const { return m_member_flags; }
    RTPS_DllAPI inline UnionMemberFlag& member_flags() { return m_member_flags; }

    RTPS_DllAPI inline void type_id(
            const TypeIdentifier& _type_id) { m_type_id = _type_id; }
    RTPS_DllAPI inline void type_id(
            TypeIdentifier&& _type_id) { m_type_id = std::move(_type_id); }
    RTPS_DllAPI inline const TypeIdentifier& type_id() const { return m_type_id; }
    RTPS_DllAPI inline TypeIdentifier& type_id() { return m_type_id; }

    RTPS_DllAPI inline void label_seq(
            const UnionCaseLabelSeq& _label_seq) { m_label_seq = _label_seq; }
    RTPS_DllAPI inline void label_seq(
            UnionCaseLabelSeq&& _label_seq) { m_label_seq = std::move(_label_seq); }
    RTPS_DllAPI inline const UnionCaseLabelSeq& label_seq() const { return m_label_seq; }
    RTPS_DllAPI inline UnionCaseLabelSeq& label_seq() { return m_label_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonUnionMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonUnionMember& other) const;

    RTPS_DllAPI bool consistent(
            const CommonUnionMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    MemberId m_member_id;
    UnionMemberFlag m_member_flags;
    TypeIdentifier m_type_id;
    UnionCaseLabelSeq m_label_seq;
};

// Member of a union type
/*struct CompleteUnionMember {
    CommonUnionMember      common;
    CompleteMemberDetail   detail;
   };*/
class CompleteUnionMember
{
public:

    RTPS_DllAPI CompleteUnionMember();
    RTPS_DllAPI ~CompleteUnionMember();
    RTPS_DllAPI CompleteUnionMember(
            const CompleteUnionMember& x);
    RTPS_DllAPI CompleteUnionMember(
            CompleteUnionMember&& x);
    RTPS_DllAPI CompleteUnionMember& operator=(
            const CompleteUnionMember& x);
    RTPS_DllAPI CompleteUnionMember& operator=(
            CompleteUnionMember&& x);

    RTPS_DllAPI inline void common(
            const CommonUnionMember& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonUnionMember&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonUnionMember& common() const { return m_common; }
    RTPS_DllAPI inline CommonUnionMember& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteUnionMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteUnionMember& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteUnionMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonUnionMember m_common;
    CompleteMemberDetail m_detail;
};

// Ordered by member_index
typedef std::vector<CompleteUnionMember> CompleteUnionMemberSeq;

// Member of a union type
/*struct MinimalUnionMember {
    CommonUnionMember   common;
    MinimalMemberDetail detail;
   };*/
class MinimalUnionMember
{
public:

    RTPS_DllAPI MinimalUnionMember();
    RTPS_DllAPI ~MinimalUnionMember();
    RTPS_DllAPI MinimalUnionMember(
            const MinimalUnionMember& x);
    RTPS_DllAPI MinimalUnionMember(
            MinimalUnionMember&& x);
    RTPS_DllAPI MinimalUnionMember& operator=(
            const MinimalUnionMember& x);
    RTPS_DllAPI MinimalUnionMember& operator=(
            MinimalUnionMember&& x);

    RTPS_DllAPI inline void common(
            const CommonUnionMember& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonUnionMember&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonUnionMember& common() const { return m_common; }
    RTPS_DllAPI inline CommonUnionMember& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const MinimalMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            MinimalMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const MinimalMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline MinimalMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalUnionMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalUnionMember& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalUnionMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonUnionMember m_common;
    MinimalMemberDetail m_detail;
};

// Ordered by MinimalUnionMember.common.member_id
typedef std::vector<MinimalUnionMember> MinimalUnionMemberSeq;

/*struct CommonDiscriminatorMember final{
    UnionDiscriminatorFlag       member_flags;
    TypeIdentifier               type_id;
   };*/
class CommonDiscriminatorMember
{
public:

    RTPS_DllAPI CommonDiscriminatorMember();
    RTPS_DllAPI ~CommonDiscriminatorMember();
    RTPS_DllAPI CommonDiscriminatorMember(
            const CommonDiscriminatorMember& x);
    RTPS_DllAPI CommonDiscriminatorMember(
            CommonDiscriminatorMember&& x);
    RTPS_DllAPI CommonDiscriminatorMember& operator=(
            const CommonDiscriminatorMember& x);
    RTPS_DllAPI CommonDiscriminatorMember& operator=(
            CommonDiscriminatorMember&& x);

    RTPS_DllAPI inline void member_flags(
            const UnionDiscriminatorFlag& _member_flags) { m_member_flags = _member_flags; }
    RTPS_DllAPI inline void member_flags(
            UnionDiscriminatorFlag&& _member_flags) { m_member_flags = std::move(_member_flags); }
    RTPS_DllAPI inline const UnionDiscriminatorFlag& member_flags() const { return m_member_flags; }
    RTPS_DllAPI inline UnionDiscriminatorFlag& member_flags() { return m_member_flags; }

    RTPS_DllAPI inline void type_id(
            const TypeIdentifier& _type_id) { m_type_id = _type_id; }
    RTPS_DllAPI inline void type_id(
            TypeIdentifier&& _type_id) { m_type_id = std::move(_type_id); }
    RTPS_DllAPI inline const TypeIdentifier& type_id() const { return m_type_id; }
    RTPS_DllAPI inline TypeIdentifier& type_id() { return m_type_id; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonDiscriminatorMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonDiscriminatorMember& other) const;

    RTPS_DllAPI bool consistent(
            const CommonDiscriminatorMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    UnionDiscriminatorFlag m_member_flags;
    TypeIdentifier m_type_id;
};

// Member of a union type
/*struct CompleteDiscriminatorMember {
    CommonDiscriminatorMember                common;
    AppliedBuiltinTypeAnnotations  ann_builtin; // Optional
    AppliedAnnotationSeq           ann_custom; // Optional
   };*/
class CompleteDiscriminatorMember
{
public:

    RTPS_DllAPI CompleteDiscriminatorMember();
    RTPS_DllAPI ~CompleteDiscriminatorMember();
    RTPS_DllAPI CompleteDiscriminatorMember(
            const CompleteDiscriminatorMember& x);
    RTPS_DllAPI CompleteDiscriminatorMember(
            CompleteDiscriminatorMember&& x);
    RTPS_DllAPI CompleteDiscriminatorMember& operator=(
            const CompleteDiscriminatorMember& x);
    RTPS_DllAPI CompleteDiscriminatorMember& operator=(
            CompleteDiscriminatorMember&& x);

    RTPS_DllAPI inline void common(
            const CommonDiscriminatorMember& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonDiscriminatorMember&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonDiscriminatorMember& common() const { return m_common; }
    RTPS_DllAPI inline CommonDiscriminatorMember& common() { return m_common; }

    RTPS_DllAPI inline void ann_builtin(
            const AppliedBuiltinTypeAnnotations& _ann_builtin) { m_ann_builtin = _ann_builtin; }
    RTPS_DllAPI inline void ann_builtin(
            AppliedBuiltinTypeAnnotations&& _ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    RTPS_DllAPI inline const AppliedBuiltinTypeAnnotations& ann_builtin() const { return m_ann_builtin; }
    RTPS_DllAPI inline AppliedBuiltinTypeAnnotations& ann_builtin() { return m_ann_builtin; }

    RTPS_DllAPI inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom) { m_ann_custom = _ann_custom; }
    RTPS_DllAPI inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom) { m_ann_custom = std::move(_ann_custom); }
    RTPS_DllAPI inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    RTPS_DllAPI inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteDiscriminatorMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteDiscriminatorMember& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteDiscriminatorMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonDiscriminatorMember m_common;
    AppliedBuiltinTypeAnnotations m_ann_builtin;
    AppliedAnnotationSeq m_ann_custom;
};

// Member of a union type
/*struct MinimalDiscriminatorMember {
    CommonDiscriminatorMember   common;
   };*/
class MinimalDiscriminatorMember
{
public:

    RTPS_DllAPI MinimalDiscriminatorMember();
    RTPS_DllAPI ~MinimalDiscriminatorMember();
    RTPS_DllAPI MinimalDiscriminatorMember(
            const MinimalDiscriminatorMember& x);
    RTPS_DllAPI MinimalDiscriminatorMember(
            MinimalDiscriminatorMember&& x);
    RTPS_DllAPI MinimalDiscriminatorMember& operator=(
            const MinimalDiscriminatorMember& x);
    RTPS_DllAPI MinimalDiscriminatorMember& operator=(
            MinimalDiscriminatorMember&& x);

    RTPS_DllAPI inline void common(
            const CommonDiscriminatorMember& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonDiscriminatorMember&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonDiscriminatorMember& common() const { return m_common; }
    RTPS_DllAPI inline CommonDiscriminatorMember& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalDiscriminatorMember& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalDiscriminatorMember& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalDiscriminatorMember& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonDiscriminatorMember m_common;
};

/*struct CompleteUnionHeader {
    CompleteTypeDetail          detail;
   };*/
class CompleteUnionHeader
{
public:

    RTPS_DllAPI CompleteUnionHeader();
    RTPS_DllAPI ~CompleteUnionHeader();
    RTPS_DllAPI CompleteUnionHeader(
            const CompleteUnionHeader& x);
    RTPS_DllAPI CompleteUnionHeader(
            CompleteUnionHeader&& x);
    RTPS_DllAPI CompleteUnionHeader& operator=(
            const CompleteUnionHeader& x);
    RTPS_DllAPI CompleteUnionHeader& operator=(
            CompleteUnionHeader&& x);

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteUnionHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteUnionHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteUnionHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CompleteTypeDetail m_detail;
};

/*struct MinimalUnionHeader {
    MinimalTypeDetail           detail;
   };*/
class MinimalUnionHeader
{
public:

    RTPS_DllAPI MinimalUnionHeader();
    RTPS_DllAPI ~MinimalUnionHeader();
    RTPS_DllAPI MinimalUnionHeader(
            const MinimalUnionHeader& x);
    RTPS_DllAPI MinimalUnionHeader(
            MinimalUnionHeader&& x);
    RTPS_DllAPI MinimalUnionHeader& operator=(
            const MinimalUnionHeader& x);
    RTPS_DllAPI MinimalUnionHeader& operator=(
            MinimalUnionHeader&& x);

    RTPS_DllAPI inline void detail(
            const MinimalTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            MinimalTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const MinimalTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline MinimalTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalUnionHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalUnionHeader& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalUnionHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    MinimalTypeDetail m_detail;
};

/*struct CompleteUnionType final{
    UnionTypeFlag                union_flags;
    CompleteUnionHeader          header;
    CompleteDiscriminatorMember  discriminator;
    CompleteUnionMemberSeq       member_seq;
   };*/
class CompleteUnionType
{
public:

    RTPS_DllAPI CompleteUnionType();
    RTPS_DllAPI ~CompleteUnionType();
    RTPS_DllAPI CompleteUnionType(
            const CompleteUnionType& x);
    RTPS_DllAPI CompleteUnionType(
            CompleteUnionType&& x);
    RTPS_DllAPI CompleteUnionType& operator=(
            const CompleteUnionType& x);
    RTPS_DllAPI CompleteUnionType& operator=(
            CompleteUnionType&& x);

    RTPS_DllAPI inline void union_flags(
            const UnionTypeFlag& _union_flags) { m_union_flags = _union_flags; }
    RTPS_DllAPI inline void union_flags(
            UnionTypeFlag&& _union_flags) { m_union_flags = std::move(_union_flags); }
    RTPS_DllAPI inline const UnionTypeFlag& union_flags() const { return m_union_flags; }
    RTPS_DllAPI inline UnionTypeFlag& union_flags() { return m_union_flags; }

    RTPS_DllAPI inline void header(
            const CompleteUnionHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteUnionHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteUnionHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteUnionHeader& header() { return m_header; }

    RTPS_DllAPI inline void discriminator(
            const CompleteDiscriminatorMember& _discriminator) { m_discriminator = _discriminator; }
    RTPS_DllAPI inline void discriminator(
            CompleteDiscriminatorMember&& _discriminator) { m_discriminator = std::move(_discriminator); }
    RTPS_DllAPI inline const CompleteDiscriminatorMember& discriminator() const { return m_discriminator; }
    RTPS_DllAPI inline CompleteDiscriminatorMember& discriminator() { return m_discriminator; }

    RTPS_DllAPI inline void member_seq(
            const CompleteUnionMemberSeq& _member_seq) { m_member_seq = _member_seq; }
    RTPS_DllAPI inline void member_seq(
            CompleteUnionMemberSeq&& _member_seq) { m_member_seq = std::move(_member_seq); }
    RTPS_DllAPI inline const CompleteUnionMemberSeq& member_seq() const { return m_member_seq; }
    RTPS_DllAPI inline CompleteUnionMemberSeq& member_seq() { return m_member_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteUnionType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteUnionType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteUnionType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    UnionTypeFlag m_union_flags;
    CompleteUnionHeader m_header;
    CompleteDiscriminatorMember m_discriminator;
    CompleteUnionMemberSeq m_member_seq;
};

/*struct MinimalUnionType final{
    UnionTypeFlag                union_flags;
    MinimalUnionHeader           header;
    MinimalDiscriminatorMember   discriminator;
    MinimalUnionMemberSeq        member_seq;
   };*/
class MinimalUnionType
{
public:

    RTPS_DllAPI MinimalUnionType();
    RTPS_DllAPI ~MinimalUnionType();
    RTPS_DllAPI MinimalUnionType(
            const MinimalUnionType& x);
    RTPS_DllAPI MinimalUnionType(
            MinimalUnionType&& x);
    RTPS_DllAPI MinimalUnionType& operator=(
            const MinimalUnionType& x);
    RTPS_DllAPI MinimalUnionType& operator=(
            MinimalUnionType&& x);

    RTPS_DllAPI inline void union_flags(
            const UnionTypeFlag& _union_flags) { m_union_flags = _union_flags; }
    RTPS_DllAPI inline void union_flags(
            UnionTypeFlag&& _union_flags) { m_union_flags = std::move(_union_flags); }
    RTPS_DllAPI inline const UnionTypeFlag& union_flags() const { return m_union_flags; }
    RTPS_DllAPI inline UnionTypeFlag& union_flags() { return m_union_flags; }

    RTPS_DllAPI inline void header(
            const MinimalUnionHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalUnionHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalUnionHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalUnionHeader& header() { return m_header; }

    RTPS_DllAPI inline void discriminator(
            const MinimalDiscriminatorMember& _discriminator) { m_discriminator = _discriminator; }
    RTPS_DllAPI inline void discriminator(
            MinimalDiscriminatorMember&& _discriminator) { m_discriminator = std::move(_discriminator); }
    RTPS_DllAPI inline const MinimalDiscriminatorMember& discriminator() const { return m_discriminator; }
    RTPS_DllAPI inline MinimalDiscriminatorMember& discriminator() { return m_discriminator; }

    RTPS_DllAPI inline void member_seq(
            const MinimalUnionMemberSeq& _member_seq) { m_member_seq = _member_seq; }
    RTPS_DllAPI inline void member_seq(
            MinimalUnionMemberSeq&& _member_seq) { m_member_seq = std::move(_member_seq); }
    RTPS_DllAPI inline const MinimalUnionMemberSeq& member_seq() const { return m_member_seq; }
    RTPS_DllAPI inline MinimalUnionMemberSeq& member_seq() { return m_member_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalUnionType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalUnionType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalUnionType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    UnionTypeFlag m_union_flags;
    MinimalUnionHeader m_header;
    MinimalDiscriminatorMember m_discriminator;
    MinimalUnionMemberSeq m_member_seq;
};

// --- Annotation: ---------------------------------------------------
/*struct CommonAnnotationParameter final{
    AnnotationParameterFlag      member_flags;
    TypeIdentifier               member_type_id;
   };*/
class CommonAnnotationParameter
{
public:

    RTPS_DllAPI CommonAnnotationParameter();
    RTPS_DllAPI ~CommonAnnotationParameter();
    RTPS_DllAPI CommonAnnotationParameter(
            const CommonAnnotationParameter& x);
    RTPS_DllAPI CommonAnnotationParameter(
            CommonAnnotationParameter&& x);
    RTPS_DllAPI CommonAnnotationParameter& operator=(
            const CommonAnnotationParameter& x);
    RTPS_DllAPI CommonAnnotationParameter& operator=(
            CommonAnnotationParameter&& x);

    RTPS_DllAPI inline void member_flags(
            const AnnotationParameterFlag& _member_flags) { m_member_flags = _member_flags; }
    RTPS_DllAPI inline void member_flags(
            AnnotationParameterFlag&& _member_flags) { m_member_flags = std::move(_member_flags); }
    RTPS_DllAPI inline const AnnotationParameterFlag& member_flags() const { return m_member_flags; }
    RTPS_DllAPI inline AnnotationParameterFlag& member_flags() { return m_member_flags; }

    RTPS_DllAPI inline void member_type_id(
            const TypeIdentifier& _member_type_id) { m_member_type_id = _member_type_id; }
    RTPS_DllAPI inline void member_type_id(
            TypeIdentifier&& _member_type_id) { m_member_type_id = std::move(_member_type_id); }
    RTPS_DllAPI inline const TypeIdentifier& member_type_id() const { return m_member_type_id; }
    RTPS_DllAPI inline TypeIdentifier& member_type_id() { return m_member_type_id; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonAnnotationParameter& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonAnnotationParameter& other) const;

    RTPS_DllAPI bool consistent(
            const CommonAnnotationParameter& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AnnotationParameterFlag m_member_flags;
    TypeIdentifier m_member_type_id;
};

// Member of an annotation type

/*struct CompleteAnnotationParameter {
    CommonAnnotationParameter  common;
    MemberName                 name;
    AnnotationParameterValue   default_value;
   };*/
class CompleteAnnotationParameter
{
public:

    RTPS_DllAPI CompleteAnnotationParameter();
    RTPS_DllAPI ~CompleteAnnotationParameter();
    RTPS_DllAPI CompleteAnnotationParameter(
            const CompleteAnnotationParameter& x);
    RTPS_DllAPI CompleteAnnotationParameter(
            CompleteAnnotationParameter&& x);
    RTPS_DllAPI CompleteAnnotationParameter& operator=(
            const CompleteAnnotationParameter& x);
    RTPS_DllAPI CompleteAnnotationParameter& operator=(
            CompleteAnnotationParameter&& x);

    RTPS_DllAPI inline void common(
            const CommonAnnotationParameter& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonAnnotationParameter&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonAnnotationParameter& common() const { return m_common; }
    RTPS_DllAPI inline CommonAnnotationParameter& common() { return m_common; }

    RTPS_DllAPI inline void name(
            const MemberName& _name) { m_name = _name; }
    RTPS_DllAPI inline void name(
            MemberName&& _name) { m_name = std::move(_name); }
    RTPS_DllAPI inline const MemberName& name() const { return m_name; }
    RTPS_DllAPI inline MemberName& name() { return m_name; }

    RTPS_DllAPI inline void default_value(
            const AnnotationParameterValue& _default_value) { m_default_value = _default_value; }
    RTPS_DllAPI inline void default_value(
            AnnotationParameterValue&& _default_value) { m_default_value = std::move(_default_value); }
    RTPS_DllAPI inline const AnnotationParameterValue& default_value() const { return m_default_value; }
    RTPS_DllAPI inline AnnotationParameterValue& default_value() { return m_default_value; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteAnnotationParameter& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteAnnotationParameter& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteAnnotationParameter& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonAnnotationParameter m_common;
    MemberName m_name;
    AnnotationParameterValue m_default_value;
};
// Ordered by CompleteAnnotationParameter.name
typedef    std::vector<CompleteAnnotationParameter> CompleteAnnotationParameterSeq;
/*struct MinimalAnnotationParameter {
    CommonAnnotationParameter  common;
    NameHash                   name_hash;
    AnnotationParameterValue   default_value;
   };*/
class MinimalAnnotationParameter
{
public:

    RTPS_DllAPI MinimalAnnotationParameter();
    RTPS_DllAPI ~MinimalAnnotationParameter();
    RTPS_DllAPI MinimalAnnotationParameter(
            const MinimalAnnotationParameter& x);
    RTPS_DllAPI MinimalAnnotationParameter(
            MinimalAnnotationParameter&& x);
    RTPS_DllAPI MinimalAnnotationParameter& operator=(
            const MinimalAnnotationParameter& x);
    RTPS_DllAPI MinimalAnnotationParameter& operator=(
            MinimalAnnotationParameter&& x);

    RTPS_DllAPI inline void common(
            const CommonAnnotationParameter& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonAnnotationParameter&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonAnnotationParameter& common() const { return m_common; }
    RTPS_DllAPI inline CommonAnnotationParameter& common() { return m_common; }

    RTPS_DllAPI inline void name(
            const MemberName& _name) { m_name = _name; }
    RTPS_DllAPI inline void name(
            MemberName&& _name) { m_name = std::move(_name); }
    RTPS_DllAPI inline const MemberName& name() const { return m_name; }
    RTPS_DllAPI inline MemberName& name() { return m_name; }

    RTPS_DllAPI inline void default_value(
            const AnnotationParameterValue& _default_value) { m_default_value = _default_value; }
    RTPS_DllAPI inline void default_value(
            AnnotationParameterValue&& _default_value) { m_default_value = std::move(_default_value); }
    RTPS_DllAPI inline const AnnotationParameterValue& default_value() const { return m_default_value; }
    RTPS_DllAPI inline AnnotationParameterValue& default_value() { return m_default_value; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalAnnotationParameter& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalAnnotationParameter& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalAnnotationParameter& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonAnnotationParameter m_common;
    MemberName m_name;
    AnnotationParameterValue m_default_value;
};

// Ordered by MinimalAnnotationParameter.name_hash
typedef    std::vector<MinimalAnnotationParameter> MinimalAnnotationParameterSeq;
/*struct CompleteAnnotationHeader {
    QualifiedTypeName         annotation_name;
   };*/
class CompleteAnnotationHeader
{
public:

    RTPS_DllAPI CompleteAnnotationHeader();
    RTPS_DllAPI ~CompleteAnnotationHeader();
    RTPS_DllAPI CompleteAnnotationHeader(
            const CompleteAnnotationHeader& x);
    RTPS_DllAPI CompleteAnnotationHeader(
            CompleteAnnotationHeader&& x);
    RTPS_DllAPI CompleteAnnotationHeader& operator=(
            const CompleteAnnotationHeader& x);
    RTPS_DllAPI CompleteAnnotationHeader& operator=(
            CompleteAnnotationHeader&& x);

    RTPS_DllAPI inline void annotation_name(
            const QualifiedTypeName& _annotation_name) { m_annotation_name = _annotation_name; }
    RTPS_DllAPI inline void annotation_name(
            QualifiedTypeName&& _annotation_name) { m_annotation_name = std::move(_annotation_name); }
    RTPS_DllAPI inline const QualifiedTypeName& annotation_name() const { return m_annotation_name; }
    RTPS_DllAPI inline QualifiedTypeName& annotation_name() { return m_annotation_name; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteAnnotationHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteAnnotationHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteAnnotationHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    QualifiedTypeName m_annotation_name;
};

/*struct MinimalAnnotationHeader {
    // Empty. Available for future extension
   };*/
class MinimalAnnotationHeader
{
public:

    RTPS_DllAPI MinimalAnnotationHeader();
    RTPS_DllAPI ~MinimalAnnotationHeader();
    RTPS_DllAPI MinimalAnnotationHeader(
            const MinimalAnnotationHeader& x);
    RTPS_DllAPI MinimalAnnotationHeader(
            MinimalAnnotationHeader&& x);
    RTPS_DllAPI MinimalAnnotationHeader& operator=(
            const MinimalAnnotationHeader& x);
    RTPS_DllAPI MinimalAnnotationHeader& operator=(
            MinimalAnnotationHeader&& x);

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalAnnotationHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalAnnotationHeader&) const { return true; }

    RTPS_DllAPI bool consistent(
            const MinimalAnnotationHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

};

/*struct CompleteAnnotationType final{
    AnnotationTypeFlag             annotation_flag;
    CompleteAnnotationHeader       header;
    CompleteAnnotationParameterSeq member_seq;
   };*/
class CompleteAnnotationType final
{
public:

    RTPS_DllAPI CompleteAnnotationType();
    RTPS_DllAPI ~CompleteAnnotationType();
    RTPS_DllAPI CompleteAnnotationType(
            const CompleteAnnotationType& x);
    RTPS_DllAPI CompleteAnnotationType(
            CompleteAnnotationType&& x);
    RTPS_DllAPI CompleteAnnotationType& operator=(
            const CompleteAnnotationType& x);
    RTPS_DllAPI CompleteAnnotationType& operator=(
            CompleteAnnotationType&& x);

    RTPS_DllAPI inline void annotation_flag(
            const AnnotationTypeFlag& _annotation_flag) { m_annotation_flag = _annotation_flag; }
    RTPS_DllAPI inline void annotation_flag(
            AnnotationTypeFlag&& _annotation_flag) { m_annotation_flag = std::move(_annotation_flag); }
    RTPS_DllAPI inline const AnnotationTypeFlag& annotation_flag() const { return m_annotation_flag; }
    RTPS_DllAPI inline AnnotationTypeFlag& annotation_flag() { return m_annotation_flag; }

    RTPS_DllAPI inline void header(
            const CompleteAnnotationHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteAnnotationHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteAnnotationHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteAnnotationHeader& header() { return m_header; }

    RTPS_DllAPI inline void member_seq(
            const CompleteAnnotationParameterSeq& _member_seq) { m_member_seq = _member_seq; }
    RTPS_DllAPI inline void member_seq(
            CompleteAnnotationParameterSeq&& _member_seq) { m_member_seq = std::move(_member_seq); }
    RTPS_DllAPI inline const CompleteAnnotationParameterSeq& member_seq() const { return m_member_seq; }
    RTPS_DllAPI inline CompleteAnnotationParameterSeq& member_seq() { return m_member_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteAnnotationType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteAnnotationType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteAnnotationType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AnnotationTypeFlag m_annotation_flag;
    CompleteAnnotationHeader m_header;
    CompleteAnnotationParameterSeq m_member_seq;
};
/*struct MinimalAnnotationType final{
    AnnotationTypeFlag             annotation_flag;
    MinimalAnnotationHeader        header;
    MinimalAnnotationParameterSeq  member_seq;
   };*/
class MinimalAnnotationType final
{
public:

    RTPS_DllAPI MinimalAnnotationType();
    RTPS_DllAPI ~MinimalAnnotationType();
    RTPS_DllAPI MinimalAnnotationType(
            const MinimalAnnotationType& x);
    RTPS_DllAPI MinimalAnnotationType(
            MinimalAnnotationType&& x);
    RTPS_DllAPI MinimalAnnotationType& operator=(
            const MinimalAnnotationType& x);
    RTPS_DllAPI MinimalAnnotationType& operator=(
            MinimalAnnotationType&& x);

    RTPS_DllAPI inline void annotation_flag(
            const AnnotationTypeFlag& _annotation_flag) { m_annotation_flag = _annotation_flag; }
    RTPS_DllAPI inline void annotation_flag(
            AnnotationTypeFlag&& _annotation_flag) { m_annotation_flag = std::move(_annotation_flag); }
    RTPS_DllAPI inline const AnnotationTypeFlag& annotation_flag() const { return m_annotation_flag; }
    RTPS_DllAPI inline AnnotationTypeFlag& annotation_flag() { return m_annotation_flag; }

    RTPS_DllAPI inline void header(
            const MinimalAnnotationHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalAnnotationHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalAnnotationHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalAnnotationHeader& header() { return m_header; }

    RTPS_DllAPI inline void member_seq(
            const MinimalAnnotationParameterSeq& _member_seq) { m_member_seq = _member_seq; }
    RTPS_DllAPI inline void member_seq(
            MinimalAnnotationParameterSeq&& _member_seq) { m_member_seq = std::move(_member_seq); }
    RTPS_DllAPI inline const MinimalAnnotationParameterSeq& member_seq() const { return m_member_seq; }
    RTPS_DllAPI inline MinimalAnnotationParameterSeq& member_seq() { return m_member_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalAnnotationType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalAnnotationType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalAnnotationType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AnnotationTypeFlag m_annotation_flag;
    MinimalAnnotationHeader m_header;
    MinimalAnnotationParameterSeq m_member_seq;
};

// --- Alias: ---------------------------------------------------------
/*struct CommonAliasBody final{
    AliasMemberFlag       related_flags;
    TypeIdentifier        related_type;
   };*/
class CommonAliasBody
{
public:

    RTPS_DllAPI CommonAliasBody();
    RTPS_DllAPI ~CommonAliasBody();
    RTPS_DllAPI CommonAliasBody(
            const CommonAliasBody& x);
    RTPS_DllAPI CommonAliasBody(
            CommonAliasBody&& x);
    RTPS_DllAPI CommonAliasBody& operator=(
            const CommonAliasBody& x);
    RTPS_DllAPI CommonAliasBody& operator=(
            CommonAliasBody&& x);

    RTPS_DllAPI inline void related_flags(
            const AliasMemberFlag& _related_flags) { m_related_flags = _related_flags; }
    RTPS_DllAPI inline void related_flags(
            AliasMemberFlag&& _related_flags) { m_related_flags = std::move(_related_flags); }
    RTPS_DllAPI inline const AliasMemberFlag& related_flags() const { return m_related_flags; }
    RTPS_DllAPI inline AliasMemberFlag& related_flags() { return m_related_flags; }

    RTPS_DllAPI inline void related_type(
            const TypeIdentifier& _related_type) { m_related_type = _related_type; }
    RTPS_DllAPI inline void related_type(
            TypeIdentifier&& _related_type) { m_related_type = std::move(_related_type); }
    RTPS_DllAPI inline const TypeIdentifier& related_type() const { return m_related_type; }
    RTPS_DllAPI inline TypeIdentifier& related_type() { return m_related_type; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonAliasBody& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonAliasBody& other) const;

    //    RTPS_DllAPI bool consistent(const CommonAliasBody &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AliasMemberFlag m_related_flags;
    TypeIdentifier m_related_type;
};

/*struct CompleteAliasBody {
    CommonAliasBody       common;
    AppliedBuiltinMemberAnnotations  ann_builtin; // Optional
    AppliedAnnotationSeq             ann_custom; // Optional
   };*/
class CompleteAliasBody
{
public:

    RTPS_DllAPI CompleteAliasBody();
    RTPS_DllAPI ~CompleteAliasBody();
    RTPS_DllAPI CompleteAliasBody(
            const CompleteAliasBody& x);
    RTPS_DllAPI CompleteAliasBody(
            CompleteAliasBody&& x);
    RTPS_DllAPI CompleteAliasBody& operator=(
            const CompleteAliasBody& x);
    RTPS_DllAPI CompleteAliasBody& operator=(
            CompleteAliasBody&& x);

    RTPS_DllAPI inline void common(
            const CommonAliasBody& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonAliasBody&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonAliasBody& common() const { return m_common; }
    RTPS_DllAPI inline CommonAliasBody& common() { return m_common; }

    RTPS_DllAPI inline void ann_builtin(
            const AppliedBuiltinMemberAnnotations& _ann_builtin) { m_ann_builtin = _ann_builtin; }
    RTPS_DllAPI inline void ann_builtin(
            AppliedBuiltinMemberAnnotations&& _ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    RTPS_DllAPI inline const AppliedBuiltinMemberAnnotations& ann_builtin() const { return m_ann_builtin; }
    RTPS_DllAPI inline AppliedBuiltinMemberAnnotations& ann_builtin() { return m_ann_builtin; }

    RTPS_DllAPI inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom) { m_ann_custom = _ann_custom; }
    RTPS_DllAPI inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom) { m_ann_custom = std::move(_ann_custom); }
    RTPS_DllAPI inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    RTPS_DllAPI inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteAliasBody& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteAliasBody& other) const;

    //    RTPS_DllAPI bool consistent(const CompleteAliasBody &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonAliasBody m_common;
    AppliedBuiltinMemberAnnotations m_ann_builtin;
    AppliedAnnotationSeq m_ann_custom;
};

/*struct MinimalAliasBody {
    CommonAliasBody       common;
   };*/
class MinimalAliasBody
{
public:

    RTPS_DllAPI MinimalAliasBody();
    RTPS_DllAPI ~MinimalAliasBody();
    RTPS_DllAPI MinimalAliasBody(
            const MinimalAliasBody& x);
    RTPS_DllAPI MinimalAliasBody(
            MinimalAliasBody&& x);
    RTPS_DllAPI MinimalAliasBody& operator=(
            const MinimalAliasBody& x);
    RTPS_DllAPI MinimalAliasBody& operator=(
            MinimalAliasBody&& x);

    RTPS_DllAPI inline void common(
            const CommonAliasBody& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonAliasBody&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonAliasBody& common() const { return m_common; }
    RTPS_DllAPI inline CommonAliasBody& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalAliasBody& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalAliasBody& other) const;

    //    RTPS_DllAPI bool consistent(const MinimalAliasBody &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonAliasBody m_common;
};

/*struct CompleteAliasHeader {
    CompleteTypeDetail    detail;
   };*/
class CompleteAliasHeader
{
public:

    RTPS_DllAPI CompleteAliasHeader();
    RTPS_DllAPI ~CompleteAliasHeader();
    RTPS_DllAPI CompleteAliasHeader(
            const CompleteAliasHeader& x);
    RTPS_DllAPI CompleteAliasHeader(
            CompleteAliasHeader&& x);
    RTPS_DllAPI CompleteAliasHeader& operator=(
            const CompleteAliasHeader& x);
    RTPS_DllAPI CompleteAliasHeader& operator=(
            CompleteAliasHeader&& x);

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteAliasHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteAliasHeader& other) const;

    //    RTPS_DllAPI bool consistent(const CompleteAliasHeader &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CompleteTypeDetail m_detail;
};

/*struct MinimalAliasHeader {
    // Empty. Available for future extension
   };*/
class MinimalAliasHeader
{
public:

    RTPS_DllAPI MinimalAliasHeader();
    RTPS_DllAPI ~MinimalAliasHeader();
    RTPS_DllAPI MinimalAliasHeader(
            const MinimalAliasHeader& x);
    RTPS_DllAPI MinimalAliasHeader(
            MinimalAliasHeader&& x);
    RTPS_DllAPI MinimalAliasHeader& operator=(
            const MinimalAliasHeader& x);
    RTPS_DllAPI MinimalAliasHeader& operator=(
            MinimalAliasHeader&& x);

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalAliasHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalAliasHeader&) const { return true; }

    //    RTPS_DllAPI bool consistent(const MinimalAliasHeader &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

};

/*struct CompleteAliasType final{
    AliasTypeFlag         alias_flags;
    CompleteAliasHeader   header;
    CompleteAliasBody     body;
   };*/
class CompleteAliasType
{
public:

    RTPS_DllAPI CompleteAliasType();
    RTPS_DllAPI ~CompleteAliasType();
    RTPS_DllAPI CompleteAliasType(
            const CompleteAliasType& x);
    RTPS_DllAPI CompleteAliasType(
            CompleteAliasType&& x);
    RTPS_DllAPI CompleteAliasType& operator=(
            const CompleteAliasType& x);
    RTPS_DllAPI CompleteAliasType& operator=(
            CompleteAliasType&& x);

    RTPS_DllAPI inline void alias_flags(
            const AliasTypeFlag& _alias_flags) { m_alias_flags = _alias_flags; }
    RTPS_DllAPI inline void alias_flags(
            AliasTypeFlag&& _alias_flags) { m_alias_flags = std::move(_alias_flags); }
    RTPS_DllAPI inline const AliasTypeFlag& alias_flags() const { return m_alias_flags; }
    RTPS_DllAPI inline AliasTypeFlag& alias_flags() { return m_alias_flags; }

    RTPS_DllAPI inline void header(
            const CompleteAliasHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteAliasHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteAliasHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteAliasHeader& header() { return m_header; }

    RTPS_DllAPI inline void body(
            const CompleteAliasBody& _body) { m_body = _body; }
    RTPS_DllAPI inline void body(
            CompleteAliasBody&& _body) { m_body = std::move(_body); }
    RTPS_DllAPI inline const CompleteAliasBody& body() const { return m_body; }
    RTPS_DllAPI inline CompleteAliasBody& body() { return m_body; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteAliasType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteAliasType& other) const;

    //    RTPS_DllAPI bool consistent(const CompleteAliasType &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AliasTypeFlag m_alias_flags;
    CompleteAliasHeader m_header;
    CompleteAliasBody m_body;
};
/*struct MinimalAliasType final{
    AliasTypeFlag         alias_flags;
    MinimalAliasHeader    header;
    MinimalAliasBody      body;
   };*/
class MinimalAliasType
{
public:

    RTPS_DllAPI MinimalAliasType();
    RTPS_DllAPI ~MinimalAliasType();
    RTPS_DllAPI MinimalAliasType(
            const MinimalAliasType& x);
    RTPS_DllAPI MinimalAliasType(
            MinimalAliasType&& x);
    RTPS_DllAPI MinimalAliasType& operator=(
            const MinimalAliasType& x);
    RTPS_DllAPI MinimalAliasType& operator=(
            MinimalAliasType&& x);

    RTPS_DllAPI inline void alias_flags(
            const AliasTypeFlag& _alias_flags) { m_alias_flags = _alias_flags; }
    RTPS_DllAPI inline void alias_flags(
            AliasTypeFlag&& _alias_flags) { m_alias_flags = std::move(_alias_flags); }
    RTPS_DllAPI inline const AliasTypeFlag& alias_flags() const { return m_alias_flags; }
    RTPS_DllAPI inline AliasTypeFlag& alias_flags() { return m_alias_flags; }

    RTPS_DllAPI inline void header(
            const MinimalAliasHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalAliasHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalAliasHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalAliasHeader& header() { return m_header; }

    RTPS_DllAPI inline void body(
            const MinimalAliasBody& _body) { m_body = _body; }
    RTPS_DllAPI inline void body(
            MinimalAliasBody&& _body) { m_body = std::move(_body); }
    RTPS_DllAPI inline const MinimalAliasBody& body() const { return m_body; }
    RTPS_DllAPI inline MinimalAliasBody& body() { return m_body; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalAliasType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalAliasType& other) const;

    //    RTPS_DllAPI bool consistent(const MinimalAliasType &x,
    //        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AliasTypeFlag m_alias_flags;
    MinimalAliasHeader m_header;
    MinimalAliasBody m_body;
};

// --- Collections: ---------------------------------------------------
/*struct CompleteElementDetail final{
    AppliedBuiltinMemberAnnotations  ann_builtin; // Optional
    AppliedAnnotationSeq             ann_custom; // Optional
   };*/
class CompleteElementDetail
{
public:

    RTPS_DllAPI CompleteElementDetail();
    RTPS_DllAPI ~CompleteElementDetail();
    RTPS_DllAPI CompleteElementDetail(
            const CompleteElementDetail& x);
    RTPS_DllAPI CompleteElementDetail(
            CompleteElementDetail&& x);
    RTPS_DllAPI CompleteElementDetail& operator=(
            const CompleteElementDetail& x);
    RTPS_DllAPI CompleteElementDetail& operator=(
            CompleteElementDetail&& x);

    RTPS_DllAPI inline void ann_builtin(
            const AppliedBuiltinMemberAnnotations& _ann_builtin) { m_ann_builtin = _ann_builtin; }
    RTPS_DllAPI inline void ann_builtin(
            AppliedBuiltinMemberAnnotations&& _ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    RTPS_DllAPI inline const AppliedBuiltinMemberAnnotations& ann_builtin() const { return m_ann_builtin; }
    RTPS_DllAPI inline AppliedBuiltinMemberAnnotations& ann_builtin() { return m_ann_builtin; }

    RTPS_DllAPI inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom) { m_ann_custom = _ann_custom; }
    RTPS_DllAPI inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom) { m_ann_custom = std::move(_ann_custom); }
    RTPS_DllAPI inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    RTPS_DllAPI inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteElementDetail& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteElementDetail& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteElementDetail& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    AppliedBuiltinMemberAnnotations m_ann_builtin;
    AppliedAnnotationSeq m_ann_custom;
};
/*struct CommonCollectionElement final{
    CollectionElementFlag     element_flags;
    TypeIdentifier            type;
   };*/
class CommonCollectionElement final
{
public:

    RTPS_DllAPI CommonCollectionElement();
    RTPS_DllAPI ~CommonCollectionElement();
    RTPS_DllAPI CommonCollectionElement(
            const CommonCollectionElement& x);
    RTPS_DllAPI CommonCollectionElement(
            CommonCollectionElement&& x);
    RTPS_DllAPI CommonCollectionElement& operator=(
            const CommonCollectionElement& x);
    RTPS_DllAPI CommonCollectionElement& operator=(
            CommonCollectionElement&& x);

    RTPS_DllAPI inline void element_flags(
            const CollectionElementFlag& _element_flags) { m_element_flags = _element_flags; }
    RTPS_DllAPI inline void element_flags(
            CollectionElementFlag&& _element_flags) { m_element_flags = std::move(_element_flags); }
    RTPS_DllAPI inline const CollectionElementFlag& element_flags() const { return m_element_flags; }
    RTPS_DllAPI inline CollectionElementFlag& element_flags() { return m_element_flags; }

    RTPS_DllAPI inline void type(
            const TypeIdentifier& _type) { m_type = _type; }
    RTPS_DllAPI inline void type(
            TypeIdentifier&& _type) { m_type = std::move(_type); }
    RTPS_DllAPI inline const TypeIdentifier& type() const { return m_type; }
    RTPS_DllAPI inline TypeIdentifier& type() { return m_type; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonCollectionElement& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonCollectionElement& other) const;

    RTPS_DllAPI bool consistent(
            const CommonCollectionElement& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionElementFlag m_element_flags;
    TypeIdentifier m_type;
};

/*struct CompleteCollectionElement {
    CommonCollectionElement   common;
    CompleteElementDetail     detail;
   };*/
class CompleteCollectionElement
{
public:

    RTPS_DllAPI CompleteCollectionElement();
    RTPS_DllAPI ~CompleteCollectionElement();
    RTPS_DllAPI CompleteCollectionElement(
            const CompleteCollectionElement& x);
    RTPS_DllAPI CompleteCollectionElement(
            CompleteCollectionElement&& x);
    RTPS_DllAPI CompleteCollectionElement& operator=(
            const CompleteCollectionElement& x);
    RTPS_DllAPI CompleteCollectionElement& operator=(
            CompleteCollectionElement&& x);

    RTPS_DllAPI inline void common(
            const CommonCollectionElement& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonCollectionElement&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonCollectionElement& common() const { return m_common; }
    RTPS_DllAPI inline CommonCollectionElement& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteElementDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteElementDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteElementDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteElementDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteCollectionElement& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteCollectionElement& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteCollectionElement& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonCollectionElement m_common;
    CompleteElementDetail m_detail;
};

/*struct MinimalCollectionElement {
    CommonCollectionElement   common;
   };*/
class MinimalCollectionElement
{
public:

    RTPS_DllAPI MinimalCollectionElement();
    RTPS_DllAPI ~MinimalCollectionElement();
    RTPS_DllAPI MinimalCollectionElement(
            const MinimalCollectionElement& x);
    RTPS_DllAPI MinimalCollectionElement(
            MinimalCollectionElement&& x);
    RTPS_DllAPI MinimalCollectionElement& operator=(
            const MinimalCollectionElement& x);
    RTPS_DllAPI MinimalCollectionElement& operator=(
            MinimalCollectionElement&& x);

    RTPS_DllAPI inline void common(
            const CommonCollectionElement& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonCollectionElement&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonCollectionElement& common() const { return m_common; }
    RTPS_DllAPI inline CommonCollectionElement& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalCollectionElement& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalCollectionElement& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalCollectionElement& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonCollectionElement m_common;
};

/*struct CommonCollectionHeader final{
    LBound                    bound;
   };*/
class CommonCollectionHeader
{
public:

    RTPS_DllAPI CommonCollectionHeader();
    RTPS_DllAPI ~CommonCollectionHeader();
    RTPS_DllAPI CommonCollectionHeader(
            const CommonCollectionHeader& x);
    RTPS_DllAPI CommonCollectionHeader(
            CommonCollectionHeader&& x);
    RTPS_DllAPI CommonCollectionHeader& operator=(
            const CommonCollectionHeader& x);
    RTPS_DllAPI CommonCollectionHeader& operator=(
            CommonCollectionHeader&& x);

    RTPS_DllAPI inline void bound(
            const LBound& _bound) { m_bound = _bound; }
    RTPS_DllAPI inline void bound(
            LBound&& _bound) { m_bound = std::move(_bound); }
    RTPS_DllAPI inline const LBound& bound() const { return m_bound; }
    RTPS_DllAPI inline LBound& bound() { return m_bound; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonCollectionHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonCollectionHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CommonCollectionHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    LBound m_bound;
};

/*struct CompleteCollectionHeader {
    CommonCollectionHeader        common;
    CompleteTypeDetail  detail; // Optional // not present for anonymous
   };*/
class CompleteCollectionHeader
{
public:

    RTPS_DllAPI CompleteCollectionHeader();
    RTPS_DllAPI ~CompleteCollectionHeader();
    RTPS_DllAPI CompleteCollectionHeader(
            const CompleteCollectionHeader& x);
    RTPS_DllAPI CompleteCollectionHeader(
            CompleteCollectionHeader&& x);
    RTPS_DllAPI CompleteCollectionHeader& operator=(
            const CompleteCollectionHeader& x);
    RTPS_DllAPI CompleteCollectionHeader& operator=(
            CompleteCollectionHeader&& x);

    RTPS_DllAPI inline void common(
            const CommonCollectionHeader& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonCollectionHeader&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonCollectionHeader& common() const { return m_common; }
    RTPS_DllAPI inline CommonCollectionHeader& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteCollectionHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteCollectionHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteCollectionHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonCollectionHeader m_common;
    CompleteTypeDetail m_detail;
};

/*struct MinimalCollectionHeader {
    CommonCollectionHeader        common;
   };*/
class MinimalCollectionHeader
{
public:

    RTPS_DllAPI MinimalCollectionHeader();
    RTPS_DllAPI ~MinimalCollectionHeader();
    RTPS_DllAPI MinimalCollectionHeader(
            const MinimalCollectionHeader& x);
    RTPS_DllAPI MinimalCollectionHeader(
            MinimalCollectionHeader&& x);
    RTPS_DllAPI MinimalCollectionHeader& operator=(
            const MinimalCollectionHeader& x);
    RTPS_DllAPI MinimalCollectionHeader& operator=(
            MinimalCollectionHeader&& x);

    RTPS_DllAPI inline void common(
            const CommonCollectionHeader& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonCollectionHeader&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonCollectionHeader& common() const { return m_common; }
    RTPS_DllAPI inline CommonCollectionHeader& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalCollectionHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalCollectionHeader& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalCollectionHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonCollectionHeader m_common;
};

// --- Sequence: -----------------------------------------------------
/*struct CompleteSequenceType final{
    CollectionTypeFlag         collection_flag;
    CompleteCollectionHeader   header;
    CompleteCollectionElement  element;
   };*/
class CompleteSequenceType
{
public:

    RTPS_DllAPI CompleteSequenceType();
    RTPS_DllAPI ~CompleteSequenceType();
    RTPS_DllAPI CompleteSequenceType(
            const CompleteSequenceType& x);
    RTPS_DllAPI CompleteSequenceType(
            CompleteSequenceType&& x);
    RTPS_DllAPI CompleteSequenceType& operator=(
            const CompleteSequenceType& x);
    RTPS_DllAPI CompleteSequenceType& operator=(
            CompleteSequenceType&& x);

    RTPS_DllAPI inline void collection_flag(
            const CollectionTypeFlag& _collection_flag) { m_collection_flag = _collection_flag; }
    RTPS_DllAPI inline void collection_flag(
            CollectionTypeFlag&& _collection_flag) { m_collection_flag = std::move(_collection_flag); }
    RTPS_DllAPI inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    RTPS_DllAPI inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    RTPS_DllAPI inline void header(
            const CompleteCollectionHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteCollectionHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteCollectionHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteCollectionHeader& header() { return m_header; }

    RTPS_DllAPI inline void element(
            const CompleteCollectionElement& _element) { m_element = _element; }
    RTPS_DllAPI inline void element(
            CompleteCollectionElement&& _element) { m_element = std::move(_element); }
    RTPS_DllAPI inline const CompleteCollectionElement& element() const { return m_element; }
    RTPS_DllAPI inline CompleteCollectionElement& element() { return m_element; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteSequenceType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteSequenceType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteSequenceType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionTypeFlag m_collection_flag;
    CompleteCollectionHeader m_header;
    CompleteCollectionElement m_element;
};

/*struct MinimalSequenceType final{
    CollectionTypeFlag         collection_flag;
    MinimalCollectionHeader    header;
    MinimalCollectionElement   element;
   };*/
class MinimalSequenceType
{
public:

    RTPS_DllAPI MinimalSequenceType();
    RTPS_DllAPI ~MinimalSequenceType();
    RTPS_DllAPI MinimalSequenceType(
            const MinimalSequenceType& x);
    RTPS_DllAPI MinimalSequenceType(
            MinimalSequenceType&& x);
    RTPS_DllAPI MinimalSequenceType& operator=(
            const MinimalSequenceType& x);
    RTPS_DllAPI MinimalSequenceType& operator=(
            MinimalSequenceType&& x);

    RTPS_DllAPI inline void collection_flag(
            const CollectionTypeFlag& _collection_flag) { m_collection_flag = _collection_flag; }
    RTPS_DllAPI inline void collection_flag(
            CollectionTypeFlag&& _collection_flag) { m_collection_flag = std::move(_collection_flag); }
    RTPS_DllAPI inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    RTPS_DllAPI inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    RTPS_DllAPI inline void header(
            const MinimalCollectionHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalCollectionHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalCollectionHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalCollectionHeader& header() { return m_header; }

    RTPS_DllAPI inline void element(
            const MinimalCollectionElement& _element) { m_element = _element; }
    RTPS_DllAPI inline void element(
            MinimalCollectionElement&& _element) { m_element = std::move(_element); }
    RTPS_DllAPI inline const MinimalCollectionElement& element() const { return m_element; }
    RTPS_DllAPI inline MinimalCollectionElement& element() { return m_element; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalSequenceType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalSequenceType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalSequenceType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionTypeFlag m_collection_flag;
    MinimalCollectionHeader m_header;
    MinimalCollectionElement m_element;
};

// --- Array: -----------------------------------------------------
/*struct CommonArrayHeader final{
    LBoundSeq           bound_seq;
   };*/
class CommonArrayHeader
{
public:

    RTPS_DllAPI CommonArrayHeader();
    RTPS_DllAPI ~CommonArrayHeader();
    RTPS_DllAPI CommonArrayHeader(
            const CommonArrayHeader& x);
    RTPS_DllAPI CommonArrayHeader(
            CommonArrayHeader&& x);
    RTPS_DllAPI CommonArrayHeader& operator=(
            const CommonArrayHeader& x);
    RTPS_DllAPI CommonArrayHeader& operator=(
            CommonArrayHeader&& x);

    RTPS_DllAPI inline void bound_seq(
            const LBoundSeq& _bound_seq) { m_bound_seq = _bound_seq; }
    RTPS_DllAPI inline void bound_seq(
            LBoundSeq&& _bound_seq) { m_bound_seq = std::move(_bound_seq); }
    RTPS_DllAPI inline const LBoundSeq& bound_seq() const { return m_bound_seq; }
    RTPS_DllAPI inline LBoundSeq& bound_seq() { return m_bound_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonArrayHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonArrayHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CommonArrayHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    LBoundSeq m_bound_seq;
};

/*struct CompleteArrayHeader {
    CommonArrayHeader   common;
    CompleteTypeDetail  detail;
   };*/
class CompleteArrayHeader
{
public:

    RTPS_DllAPI CompleteArrayHeader();
    RTPS_DllAPI ~CompleteArrayHeader();
    RTPS_DllAPI CompleteArrayHeader(
            const CompleteArrayHeader& x);
    RTPS_DllAPI CompleteArrayHeader(
            CompleteArrayHeader&& x);
    RTPS_DllAPI CompleteArrayHeader& operator=(
            const CompleteArrayHeader& x);
    RTPS_DllAPI CompleteArrayHeader& operator=(
            CompleteArrayHeader&& x);

    RTPS_DllAPI inline void common(
            const CommonArrayHeader& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonArrayHeader&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonArrayHeader& common() const { return m_common; }
    RTPS_DllAPI inline CommonArrayHeader& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteArrayHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteArrayHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteArrayHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonArrayHeader m_common;
    CompleteTypeDetail m_detail;
};

/*struct MinimalArrayHeader {
    CommonArrayHeader   common;
   };*/
class MinimalArrayHeader
{
public:

    RTPS_DllAPI MinimalArrayHeader();
    RTPS_DllAPI ~MinimalArrayHeader();
    RTPS_DllAPI MinimalArrayHeader(
            const MinimalArrayHeader& x);
    RTPS_DllAPI MinimalArrayHeader(
            MinimalArrayHeader&& x);
    RTPS_DllAPI MinimalArrayHeader& operator=(
            const MinimalArrayHeader& x);
    RTPS_DllAPI MinimalArrayHeader& operator=(
            MinimalArrayHeader&& x);

    RTPS_DllAPI inline void common(
            const CommonArrayHeader& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonArrayHeader&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonArrayHeader& common() const { return m_common; }
    RTPS_DllAPI inline CommonArrayHeader& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalArrayHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalArrayHeader& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalArrayHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonArrayHeader m_common;
};

/*struct CompleteArrayType  {
    CollectionTypeFlag          collection_flag;
    CompleteArrayHeader         header;
    CompleteCollectionElement   element;
   };*/
class CompleteArrayType
{
public:

    RTPS_DllAPI CompleteArrayType();
    RTPS_DllAPI ~CompleteArrayType();
    RTPS_DllAPI CompleteArrayType(
            const CompleteArrayType& x);
    RTPS_DllAPI CompleteArrayType(
            CompleteArrayType&& x);
    RTPS_DllAPI CompleteArrayType& operator=(
            const CompleteArrayType& x);
    RTPS_DllAPI CompleteArrayType& operator=(
            CompleteArrayType&& x);

    RTPS_DllAPI inline void collection_flag(
            const CollectionTypeFlag& _collection_flag) { m_collection_flag = _collection_flag; }
    RTPS_DllAPI inline void collection_flag(
            CollectionTypeFlag&& _collection_flag) { m_collection_flag = std::move(_collection_flag); }
    RTPS_DllAPI inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    RTPS_DllAPI inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    RTPS_DllAPI inline void header(
            const CompleteArrayHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteArrayHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteArrayHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteArrayHeader& header() { return m_header; }

    RTPS_DllAPI inline void element(
            const CompleteCollectionElement& _element) { m_element = _element; }
    RTPS_DllAPI inline void element(
            CompleteCollectionElement&& _element) { m_element = std::move(_element); }
    RTPS_DllAPI inline const CompleteCollectionElement& element() const { return m_element; }
    RTPS_DllAPI inline CompleteCollectionElement& element() { return m_element; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteArrayType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteArrayType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteArrayType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionTypeFlag m_collection_flag;
    CompleteArrayHeader m_header;
    CompleteCollectionElement m_element;
};

/*struct MinimalArrayType final{
    CollectionTypeFlag         collection_flag;
    MinimalArrayHeader         header;
    MinimalCollectionElement   element;
   };*/
class MinimalArrayType
{
public:

    RTPS_DllAPI MinimalArrayType();
    RTPS_DllAPI ~MinimalArrayType();
    RTPS_DllAPI MinimalArrayType(
            const MinimalArrayType& x);
    RTPS_DllAPI MinimalArrayType(
            MinimalArrayType&& x);
    RTPS_DllAPI MinimalArrayType& operator=(
            const MinimalArrayType& x);
    RTPS_DllAPI MinimalArrayType& operator=(
            MinimalArrayType&& x);

    RTPS_DllAPI inline void collection_flag(
            const CollectionTypeFlag& _collection_flag) { m_collection_flag = _collection_flag; }
    RTPS_DllAPI inline void collection_flag(
            CollectionTypeFlag&& _collection_flag) { m_collection_flag = std::move(_collection_flag); }
    RTPS_DllAPI inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    RTPS_DllAPI inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    RTPS_DllAPI inline void header(
            const MinimalArrayHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalArrayHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalArrayHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalArrayHeader& header() { return m_header; }

    RTPS_DllAPI inline void element(
            const MinimalCollectionElement& _element) { m_element = _element; }
    RTPS_DllAPI inline void element(
            MinimalCollectionElement&& _element) { m_element = std::move(_element); }
    RTPS_DllAPI inline const MinimalCollectionElement& element() const { return m_element; }
    RTPS_DllAPI inline MinimalCollectionElement& element() { return m_element; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalArrayType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalArrayType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalArrayType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionTypeFlag m_collection_flag;
    MinimalArrayHeader m_header;
    MinimalCollectionElement m_element;
};

// --- Map: -----------------------------------------------------
/*struct CompleteMapType final{
    CollectionTypeFlag            collection_flag;
    CompleteCollectionHeader      header;
    CompleteCollectionElement     key;
    CompleteCollectionElement     element;
   };*/
class CompleteMapType final
{
public:

    RTPS_DllAPI CompleteMapType();
    RTPS_DllAPI ~CompleteMapType();
    RTPS_DllAPI CompleteMapType(
            const CompleteMapType& x);
    RTPS_DllAPI CompleteMapType(
            CompleteMapType&& x);
    RTPS_DllAPI CompleteMapType& operator=(
            const CompleteMapType& x);
    RTPS_DllAPI CompleteMapType& operator=(
            CompleteMapType&& x);

    RTPS_DllAPI inline void collection_flag(
            const CollectionTypeFlag& _collection_flag) { m_collection_flag = _collection_flag; }
    RTPS_DllAPI inline void collection_flag(
            CollectionTypeFlag&& _collection_flag) { m_collection_flag = std::move(_collection_flag); }
    RTPS_DllAPI inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    RTPS_DllAPI inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    RTPS_DllAPI inline void header(
            const CompleteCollectionHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteCollectionHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteCollectionHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteCollectionHeader& header() { return m_header; }

    RTPS_DllAPI inline void key(
            const CompleteCollectionElement& _key) { m_key = _key; }
    RTPS_DllAPI inline void key(
            CompleteCollectionElement&& _key) { m_key = std::move(_key); }
    RTPS_DllAPI inline const CompleteCollectionElement& key() const { return m_key; }
    RTPS_DllAPI inline CompleteCollectionElement& key() { return m_key; }

    RTPS_DllAPI inline void element(
            const CompleteCollectionElement& _element) { m_element = _element; }
    RTPS_DllAPI inline void element(
            CompleteCollectionElement&& _element) { m_element = std::move(_element); }
    RTPS_DllAPI inline const CompleteCollectionElement& element() const { return m_element; }
    RTPS_DllAPI inline CompleteCollectionElement& element() { return m_element; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteMapType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteMapType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteMapType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionTypeFlag m_collection_flag;
    CompleteCollectionHeader m_header;
    CompleteCollectionElement m_key;
    CompleteCollectionElement m_element;
};
/*struct MinimalMapType final{
    CollectionTypeFlag          collection_flag;
    MinimalCollectionHeader     header;
    MinimalCollectionElement    key;
    MinimalCollectionElement    element;
   };*/
class MinimalMapType final
{
public:

    RTPS_DllAPI MinimalMapType();
    RTPS_DllAPI ~MinimalMapType();
    RTPS_DllAPI MinimalMapType(
            const MinimalMapType& x);
    RTPS_DllAPI MinimalMapType(
            MinimalMapType&& x);
    RTPS_DllAPI MinimalMapType& operator=(
            const MinimalMapType& x);
    RTPS_DllAPI MinimalMapType& operator=(
            MinimalMapType&& x);

    RTPS_DllAPI inline void collection_flag(
            const CollectionTypeFlag& _collection_flag) { m_collection_flag = _collection_flag; }
    RTPS_DllAPI inline void collection_flag(
            CollectionTypeFlag&& _collection_flag) { m_collection_flag = std::move(_collection_flag); }
    RTPS_DllAPI inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    RTPS_DllAPI inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    RTPS_DllAPI inline void header(
            const MinimalCollectionHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalCollectionHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalCollectionHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalCollectionHeader& header() { return m_header; }

    RTPS_DllAPI inline void key(
            const MinimalCollectionElement& _key) { m_key = _key; }
    RTPS_DllAPI inline void key(
            MinimalCollectionElement&& _key) { m_key = std::move(_key); }
    RTPS_DllAPI inline const MinimalCollectionElement& key() const { return m_key; }
    RTPS_DllAPI inline MinimalCollectionElement& key() { return m_key; }

    RTPS_DllAPI inline void element(
            const MinimalCollectionElement& _element) { m_element = _element; }
    RTPS_DllAPI inline void element(
            MinimalCollectionElement&& _element) { m_element = std::move(_element); }
    RTPS_DllAPI inline const MinimalCollectionElement& element() const { return m_element; }
    RTPS_DllAPI inline MinimalCollectionElement& element() { return m_element; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalMapType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalMapType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalMapType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CollectionTypeFlag m_collection_flag;
    MinimalCollectionHeader m_header;
    MinimalCollectionElement m_key;
    MinimalCollectionElement m_element;
};

// --- Enumeration: ---------------------------------------------------
typedef uint16_t BitBound;

// Constant in an enumerated type

/*struct CommonEnumeratedLiteral {
    int32_t                     value;
    EnumeratedLiteralFlag    flags;
   };*/
class CommonEnumeratedLiteral
{
public:

    RTPS_DllAPI CommonEnumeratedLiteral();
    RTPS_DllAPI ~CommonEnumeratedLiteral();
    RTPS_DllAPI CommonEnumeratedLiteral(
            const CommonEnumeratedLiteral& x);
    RTPS_DllAPI CommonEnumeratedLiteral(
            CommonEnumeratedLiteral&& x);
    RTPS_DllAPI CommonEnumeratedLiteral& operator=(
            const CommonEnumeratedLiteral& x);
    RTPS_DllAPI CommonEnumeratedLiteral& operator=(
            CommonEnumeratedLiteral&& x);

    RTPS_DllAPI inline void value(
            const int32_t& _value) { m_value = _value; }
    RTPS_DllAPI inline void value(
            int32_t&& _value) { m_value = std::move(_value); }
    RTPS_DllAPI inline const int32_t& value() const { return m_value; }
    RTPS_DllAPI inline int32_t& value() { return m_value; }

    RTPS_DllAPI inline void flags(
            const EnumeratedLiteralFlag& _flags) { m_flags = _flags; }
    RTPS_DllAPI inline void flags(
            EnumeratedLiteralFlag&& _flags) { m_flags = std::move(_flags); }
    RTPS_DllAPI inline const EnumeratedLiteralFlag& flags() const { return m_flags; }
    RTPS_DllAPI inline EnumeratedLiteralFlag& flags() { return m_flags; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonEnumeratedLiteral& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonEnumeratedLiteral& other) const;

    RTPS_DllAPI bool consistent(
            const CommonEnumeratedLiteral& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    int32_t m_value;
    EnumeratedLiteralFlag m_flags;
};

// Constant in an enumerated type

/*struct CompleteEnumeratedLiteral {
    CommonEnumeratedLiteral  common;
    CompleteMemberDetail     detail;
   };*/
class CompleteEnumeratedLiteral
{
public:

    RTPS_DllAPI CompleteEnumeratedLiteral();
    RTPS_DllAPI ~CompleteEnumeratedLiteral();
    RTPS_DllAPI CompleteEnumeratedLiteral(
            const CompleteEnumeratedLiteral& x);
    RTPS_DllAPI CompleteEnumeratedLiteral(
            CompleteEnumeratedLiteral&& x);
    RTPS_DllAPI CompleteEnumeratedLiteral& operator=(
            const CompleteEnumeratedLiteral& x);
    RTPS_DllAPI CompleteEnumeratedLiteral& operator=(
            CompleteEnumeratedLiteral&& x);

    RTPS_DllAPI inline void common(
            const CommonEnumeratedLiteral& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonEnumeratedLiteral&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonEnumeratedLiteral& common() const { return m_common; }
    RTPS_DllAPI inline CommonEnumeratedLiteral& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteEnumeratedLiteral& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteEnumeratedLiteral& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteEnumeratedLiteral& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonEnumeratedLiteral m_common;
    CompleteMemberDetail m_detail;
};
// Ordered by EnumeratedLiteral.common.value
typedef std::vector<CompleteEnumeratedLiteral> CompleteEnumeratedLiteralSeq;

// Constant in an enumerated type

/*struct MinimalEnumeratedLiteral {
    CommonEnumeratedLiteral  common;
    MinimalMemberDetail      detail;
   };*/
class MinimalEnumeratedLiteral
{
public:

    RTPS_DllAPI MinimalEnumeratedLiteral();
    RTPS_DllAPI ~MinimalEnumeratedLiteral();
    RTPS_DllAPI MinimalEnumeratedLiteral(
            const MinimalEnumeratedLiteral& x);
    RTPS_DllAPI MinimalEnumeratedLiteral(
            MinimalEnumeratedLiteral&& x);
    RTPS_DllAPI MinimalEnumeratedLiteral& operator=(
            const MinimalEnumeratedLiteral& x);
    RTPS_DllAPI MinimalEnumeratedLiteral& operator=(
            MinimalEnumeratedLiteral&& x);

    RTPS_DllAPI inline void common(
            const CommonEnumeratedLiteral& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonEnumeratedLiteral&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonEnumeratedLiteral& common() const { return m_common; }
    RTPS_DllAPI inline CommonEnumeratedLiteral& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const MinimalMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            MinimalMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const MinimalMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline MinimalMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalEnumeratedLiteral& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalEnumeratedLiteral& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalEnumeratedLiteral& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonEnumeratedLiteral m_common;
    MinimalMemberDetail m_detail;
};

// Ordered by EnumeratedLiteral.common.value
typedef std::vector<MinimalEnumeratedLiteral> MinimalEnumeratedLiteralSeq;

/*struct CommonEnumeratedHeader final{
    BitBound                bit_bound;
   };*/
class CommonEnumeratedHeader final
{
public:

    RTPS_DllAPI CommonEnumeratedHeader();
    RTPS_DllAPI ~CommonEnumeratedHeader();
    RTPS_DllAPI CommonEnumeratedHeader(
            const CommonEnumeratedHeader& x);
    RTPS_DllAPI CommonEnumeratedHeader(
            CommonEnumeratedHeader&& x);
    RTPS_DllAPI CommonEnumeratedHeader& operator=(
            const CommonEnumeratedHeader& x);
    RTPS_DllAPI CommonEnumeratedHeader& operator=(
            CommonEnumeratedHeader&& x);

    RTPS_DllAPI inline void bit_bound(
            const BitBound& _bit_bound) { m_bit_bound = _bit_bound; }
    RTPS_DllAPI inline void bit_bound(
            BitBound&& _bit_bound) { m_bit_bound = std::move(_bit_bound); }
    RTPS_DllAPI inline const BitBound& bit_bound() const { return m_bit_bound; }
    RTPS_DllAPI inline BitBound& bit_bound() { return m_bit_bound; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonEnumeratedHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonEnumeratedHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CommonEnumeratedHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    BitBound m_bit_bound;
};

/*struct CompleteEnumeratedHeader {
    CommonEnumeratedHeader  common;
    CompleteTypeDetail      detail;
   };*/
class CompleteEnumeratedHeader
{
public:

    RTPS_DllAPI CompleteEnumeratedHeader();
    RTPS_DllAPI ~CompleteEnumeratedHeader();
    RTPS_DllAPI CompleteEnumeratedHeader(
            const CompleteEnumeratedHeader& x);
    RTPS_DllAPI CompleteEnumeratedHeader(
            CompleteEnumeratedHeader&& x);
    RTPS_DllAPI CompleteEnumeratedHeader& operator=(
            const CompleteEnumeratedHeader& x);
    RTPS_DllAPI CompleteEnumeratedHeader& operator=(
            CompleteEnumeratedHeader&& x);

    RTPS_DllAPI inline void common(
            const CommonEnumeratedHeader& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonEnumeratedHeader&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonEnumeratedHeader& common() const { return m_common; }
    RTPS_DllAPI inline CommonEnumeratedHeader& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteEnumeratedHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteEnumeratedHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteEnumeratedHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonEnumeratedHeader m_common;
    CompleteTypeDetail m_detail;
};

/*struct MinimalEnumeratedHeader {
    CommonEnumeratedHeader  common;
   };*/
class MinimalEnumeratedHeader
{
public:

    RTPS_DllAPI MinimalEnumeratedHeader();
    RTPS_DllAPI ~MinimalEnumeratedHeader();
    RTPS_DllAPI MinimalEnumeratedHeader(
            const MinimalEnumeratedHeader& x);
    RTPS_DllAPI MinimalEnumeratedHeader(
            MinimalEnumeratedHeader&& x);
    RTPS_DllAPI MinimalEnumeratedHeader& operator=(
            const MinimalEnumeratedHeader& x);
    RTPS_DllAPI MinimalEnumeratedHeader& operator=(
            MinimalEnumeratedHeader&& x);

    RTPS_DllAPI inline void common(
            const CommonEnumeratedHeader& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonEnumeratedHeader&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonEnumeratedHeader& common() const { return m_common; }
    RTPS_DllAPI inline CommonEnumeratedHeader& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalEnumeratedHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalEnumeratedHeader& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalEnumeratedHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonEnumeratedHeader m_common;
};

// Enumerated type
/*struct CompleteEnumeratedType final{
    EnumTypeFlag                    enum_flags; // unused
    CompleteEnumeratedHeader        header;
    CompleteEnumeratedLiteralSeq    literal_seq;
   };*/
class CompleteEnumeratedType
{
public:

    RTPS_DllAPI CompleteEnumeratedType();
    RTPS_DllAPI ~CompleteEnumeratedType();
    RTPS_DllAPI CompleteEnumeratedType(
            const CompleteEnumeratedType& x);
    RTPS_DllAPI CompleteEnumeratedType(
            CompleteEnumeratedType&& x);
    RTPS_DllAPI CompleteEnumeratedType& operator=(
            const CompleteEnumeratedType& x);
    RTPS_DllAPI CompleteEnumeratedType& operator=(
            CompleteEnumeratedType&& x);

    RTPS_DllAPI inline void enum_flags(
            const EnumTypeFlag& _enum_flags) { m_enum_flags = _enum_flags; }
    RTPS_DllAPI inline void enum_flags(
            EnumTypeFlag&& _enum_flags) { m_enum_flags = std::move(_enum_flags); }
    RTPS_DllAPI inline const EnumTypeFlag& enum_flags() const { return m_enum_flags; }
    RTPS_DllAPI inline EnumTypeFlag& enum_flags() { return m_enum_flags; }

    RTPS_DllAPI inline void header(
            const CompleteEnumeratedHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteEnumeratedHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteEnumeratedHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteEnumeratedHeader& header() { return m_header; }

    RTPS_DllAPI inline void literal_seq(
            const CompleteEnumeratedLiteralSeq& _literal_seq) { m_literal_seq = _literal_seq; }
    RTPS_DllAPI inline void literal_seq(
            CompleteEnumeratedLiteralSeq&& _literal_seq) { m_literal_seq = std::move(_literal_seq); }
    RTPS_DllAPI inline const CompleteEnumeratedLiteralSeq& literal_seq() const { return m_literal_seq; }
    RTPS_DllAPI inline CompleteEnumeratedLiteralSeq& literal_seq() { return m_literal_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteEnumeratedType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteEnumeratedType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteEnumeratedType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    EnumTypeFlag m_enum_flags;
    CompleteEnumeratedHeader m_header;
    CompleteEnumeratedLiteralSeq m_literal_seq;
};
// Enumerated type
/*struct MinimalEnumeratedType final{
    EnumTypeFlag                  enum_flags; // unused
    MinimalEnumeratedHeader       header;
    MinimalEnumeratedLiteralSeq   literal_seq;
   };*/
class MinimalEnumeratedType
{
public:

    RTPS_DllAPI MinimalEnumeratedType();
    RTPS_DllAPI ~MinimalEnumeratedType();
    RTPS_DllAPI MinimalEnumeratedType(
            const MinimalEnumeratedType& x);
    RTPS_DllAPI MinimalEnumeratedType(
            MinimalEnumeratedType&& x);
    RTPS_DllAPI MinimalEnumeratedType& operator=(
            const MinimalEnumeratedType& x);
    RTPS_DllAPI MinimalEnumeratedType& operator=(
            MinimalEnumeratedType&& x);

    RTPS_DllAPI inline void enum_flags(
            const EnumTypeFlag& _enum_flags) { m_enum_flags = _enum_flags; }
    RTPS_DllAPI inline void enum_flags(
            EnumTypeFlag&& _enum_flags) { m_enum_flags = std::move(_enum_flags); }
    RTPS_DllAPI inline const EnumTypeFlag& enum_flags() const { return m_enum_flags; }
    RTPS_DllAPI inline EnumTypeFlag& enum_flags() { return m_enum_flags; }

    RTPS_DllAPI inline void header(
            const MinimalEnumeratedHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalEnumeratedHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalEnumeratedHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalEnumeratedHeader& header() { return m_header; }

    RTPS_DllAPI inline void literal_seq(
            const MinimalEnumeratedLiteralSeq& _literal_seq) { m_literal_seq = _literal_seq; }
    RTPS_DllAPI inline void literal_seq(
            MinimalEnumeratedLiteralSeq&& _literal_seq) { m_literal_seq = std::move(_literal_seq); }
    RTPS_DllAPI inline const MinimalEnumeratedLiteralSeq& literal_seq() const { return m_literal_seq; }
    RTPS_DllAPI inline MinimalEnumeratedLiteralSeq& literal_seq() { return m_literal_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalEnumeratedType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalEnumeratedType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalEnumeratedType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    EnumTypeFlag m_enum_flags;
    MinimalEnumeratedHeader m_header;
    MinimalEnumeratedLiteralSeq m_literal_seq;
};

// --- Bitmask: -------------------------------------------------------
// Bit in a bit mask
/*struct CommonBitflag final{
    uint16_t         position;
    BitflagFlag            flags;
   };*/
class CommonBitflag final
{
public:

    RTPS_DllAPI CommonBitflag();
    RTPS_DllAPI ~CommonBitflag();
    RTPS_DllAPI CommonBitflag(
            const CommonBitflag& x);
    RTPS_DllAPI CommonBitflag(
            CommonBitflag&& x);
    RTPS_DllAPI CommonBitflag& operator=(
            const CommonBitflag& x);
    RTPS_DllAPI CommonBitflag& operator=(
            CommonBitflag&& x);

    RTPS_DllAPI inline void position(
            const uint16_t& _position) { m_position = _position; }
    RTPS_DllAPI inline void position(
            uint16_t&& _position) { m_position = std::move(_position); }
    RTPS_DllAPI inline const uint16_t& position() const { return m_position; }
    RTPS_DllAPI inline uint16_t& position() { return m_position; }

    RTPS_DllAPI inline void flags(
            const BitflagFlag& _flags) { m_flags = _flags; }
    RTPS_DllAPI inline void flags(
            BitflagFlag&& _flags) { m_flags = std::move(_flags); }
    RTPS_DllAPI inline const BitflagFlag& flags() const { return m_flags; }
    RTPS_DllAPI inline BitflagFlag& flags() { return m_flags; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonBitflag& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonBitflag& other) const;

    RTPS_DllAPI bool consistent(
            const CommonBitflag& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    uint16_t m_position;
    BitflagFlag m_flags;
};

/*struct CompleteBitflag {
    CommonBitflag          common;
    CompleteMemberDetail   detail;
   };*/
class CompleteBitflag
{
public:

    RTPS_DllAPI CompleteBitflag();
    RTPS_DllAPI ~CompleteBitflag();
    RTPS_DllAPI CompleteBitflag(
            const CompleteBitflag& x);
    RTPS_DllAPI CompleteBitflag(
            CompleteBitflag&& x);
    RTPS_DllAPI CompleteBitflag& operator=(
            const CompleteBitflag& x);
    RTPS_DllAPI CompleteBitflag& operator=(
            CompleteBitflag&& x);

    RTPS_DllAPI inline void common(
            const CommonBitflag& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonBitflag&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonBitflag& common() const { return m_common; }
    RTPS_DllAPI inline CommonBitflag& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteBitflag& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteBitflag& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteBitflag& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonBitflag m_common;
    CompleteMemberDetail m_detail;
};
// Ordered by Bitflag.position
typedef std::vector<CompleteBitflag> CompleteBitflagSeq;
/*struct MinimalBitflag {
    CommonBitflag        common;
    MinimalMemberDetail  detail;
   };*/
class MinimalBitflag
{
public:

    RTPS_DllAPI MinimalBitflag();
    RTPS_DllAPI ~MinimalBitflag();
    RTPS_DllAPI MinimalBitflag(
            const MinimalBitflag& x);
    RTPS_DllAPI MinimalBitflag(
            MinimalBitflag&& x);
    RTPS_DllAPI MinimalBitflag& operator=(
            const MinimalBitflag& x);
    RTPS_DllAPI MinimalBitflag& operator=(
            MinimalBitflag&& x);

    RTPS_DllAPI inline void common(
            const CommonBitflag& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonBitflag&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonBitflag& common() const { return m_common; }
    RTPS_DllAPI inline CommonBitflag& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const MinimalMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            MinimalMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const MinimalMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline MinimalMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalBitflag& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalBitflag& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalBitflag& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonBitflag m_common;
    MinimalMemberDetail m_detail;
};

// Ordered by Bitflag.position
typedef std::vector<MinimalBitflag> MinimalBitflagSeq;

/*struct CommonBitmaskHeader final{
    BitBound             bit_bound;
   };*/
class CommonBitmaskHeader final
{
public:

    RTPS_DllAPI CommonBitmaskHeader();
    RTPS_DllAPI ~CommonBitmaskHeader();
    RTPS_DllAPI CommonBitmaskHeader(
            const CommonBitmaskHeader& x);
    RTPS_DllAPI CommonBitmaskHeader(
            CommonBitmaskHeader&& x);
    RTPS_DllAPI CommonBitmaskHeader& operator=(
            const CommonBitmaskHeader& x);
    RTPS_DllAPI CommonBitmaskHeader& operator=(
            CommonBitmaskHeader&& x);

    RTPS_DllAPI inline void bit_bound(
            const BitBound& _bit_bound) { m_bit_bound = _bit_bound; }
    RTPS_DllAPI inline void bit_bound(
            BitBound&& _bit_bound) { m_bit_bound = std::move(_bit_bound); }
    RTPS_DllAPI inline const BitBound& bit_bound() const { return m_bit_bound; }
    RTPS_DllAPI inline BitBound& bit_bound() { return m_bit_bound; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonBitmaskHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonBitmaskHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CommonBitmaskHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    BitBound m_bit_bound;
};
typedef CompleteEnumeratedHeader CompleteBitmaskHeader;

typedef MinimalEnumeratedHeader MinimalBitmaskHeader;
/*struct CompleteBitmaskType {
    BitmaskTypeFlag          bitmask_flags; // unused
    CompleteBitmaskHeader    header;
    CompleteBitflagSeq       flag_seq;
   };*/
class CompleteBitmaskType
{
public:

    RTPS_DllAPI CompleteBitmaskType();
    RTPS_DllAPI ~CompleteBitmaskType();
    RTPS_DllAPI CompleteBitmaskType(
            const CompleteBitmaskType& x);
    RTPS_DllAPI CompleteBitmaskType(
            CompleteBitmaskType&& x);
    RTPS_DllAPI CompleteBitmaskType& operator=(
            const CompleteBitmaskType& x);
    RTPS_DllAPI CompleteBitmaskType& operator=(
            CompleteBitmaskType&& x);

    RTPS_DllAPI inline void bitmask_flags(
            const BitmaskTypeFlag& _bitmask_flags) { m_bitmask_flags = _bitmask_flags; }
    RTPS_DllAPI inline void bitmask_flags(
            BitmaskTypeFlag&& _bitmask_flags) { m_bitmask_flags = std::move(_bitmask_flags); }
    RTPS_DllAPI inline const BitmaskTypeFlag& bitmask_flags() const { return m_bitmask_flags; }
    RTPS_DllAPI inline BitmaskTypeFlag& bitmask_flags() { return m_bitmask_flags; }

    RTPS_DllAPI inline void header(
            const CompleteBitmaskHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteBitmaskHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteBitmaskHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteBitmaskHeader& header() { return m_header; }

    RTPS_DllAPI inline void flag_seq(
            const CompleteBitflagSeq& _flag_seq) { m_flag_seq = _flag_seq; }
    RTPS_DllAPI inline void flag_seq(
            CompleteBitflagSeq&& _flag_seq) { m_flag_seq = std::move(_flag_seq); }
    RTPS_DllAPI inline const CompleteBitflagSeq& flag_seq() const { return m_flag_seq; }
    RTPS_DllAPI inline CompleteBitflagSeq& flag_seq() { return m_flag_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteBitmaskType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteBitmaskType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteBitmaskType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    BitmaskTypeFlag m_bitmask_flags;
    CompleteBitmaskHeader m_header;
    CompleteBitflagSeq m_flag_seq;
};

/*struct MinimalBitmaskType {
    BitmaskTypeFlag          bitmask_flags; // unused
    MinimalBitmaskHeader     header;
    MinimalBitflagSeq        flag_seq;
   };*/
class MinimalBitmaskType
{
public:

    RTPS_DllAPI MinimalBitmaskType();
    RTPS_DllAPI ~MinimalBitmaskType();
    RTPS_DllAPI MinimalBitmaskType(
            const MinimalBitmaskType& x);
    RTPS_DllAPI MinimalBitmaskType(
            MinimalBitmaskType&& x);
    RTPS_DllAPI MinimalBitmaskType& operator=(
            const MinimalBitmaskType& x);
    RTPS_DllAPI MinimalBitmaskType& operator=(
            MinimalBitmaskType&& x);

    RTPS_DllAPI inline void bitmask_flags(
            const BitmaskTypeFlag& _bitmask_flags) { m_bitmask_flags = _bitmask_flags; }
    RTPS_DllAPI inline void bitmask_flags(
            BitmaskTypeFlag&& _bitmask_flags) { m_bitmask_flags = std::move(_bitmask_flags); }
    RTPS_DllAPI inline const BitmaskTypeFlag& bitmask_flags() const { return m_bitmask_flags; }
    RTPS_DllAPI inline BitmaskTypeFlag& bitmask_flags() { return m_bitmask_flags; }

    RTPS_DllAPI inline void header(
            const MinimalBitmaskHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalBitmaskHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalBitmaskHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalBitmaskHeader& header() { return m_header; }

    RTPS_DllAPI inline void flag_seq(
            const MinimalBitflagSeq& _flag_seq) { m_flag_seq = _flag_seq; }
    RTPS_DllAPI inline void flag_seq(
            MinimalBitflagSeq&& _flag_seq) { m_flag_seq = std::move(_flag_seq); }
    RTPS_DllAPI inline const MinimalBitflagSeq& flag_seq() const { return m_flag_seq; }
    RTPS_DllAPI inline MinimalBitflagSeq& flag_seq() { return m_flag_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalBitmaskType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalBitmaskType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalBitmaskType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    BitmaskTypeFlag m_bitmask_flags;
    MinimalBitmaskHeader m_header;
    MinimalBitflagSeq m_flag_seq;
};

// --- Bitset: ---------------------------------------------------------
/*struct CommonBitfield final{
    uint16_t        position;
    BitsetMemberFlag      flags;
    octet                 bitcount;
    TypeKind              holder_type; // Must be primitive integer type
   };*/
class CommonBitfield final
{
public:

    RTPS_DllAPI CommonBitfield();
    RTPS_DllAPI ~CommonBitfield();
    RTPS_DllAPI CommonBitfield(
            const CommonBitfield& x);
    RTPS_DllAPI CommonBitfield(
            CommonBitfield&& x);
    RTPS_DllAPI CommonBitfield& operator=(
            const CommonBitfield& x);
    RTPS_DllAPI CommonBitfield& operator=(
            CommonBitfield&& x);

    RTPS_DllAPI inline void position(
            const uint16_t& _position) { m_position = _position; }
    RTPS_DllAPI inline void position(
            uint16_t&& _position) { m_position = std::move(_position); }
    RTPS_DllAPI inline const uint16_t& position() const { return m_position; }
    RTPS_DllAPI inline uint16_t& position() { return m_position; }

    RTPS_DllAPI inline void flags(
            const BitsetMemberFlag& _flags) { m_flags = _flags; }
    RTPS_DllAPI inline void flags(
            BitsetMemberFlag&& _flags) { m_flags = std::move(_flags); }
    RTPS_DllAPI inline const BitsetMemberFlag& flags() const { return m_flags; }
    RTPS_DllAPI inline BitsetMemberFlag& flags() { return m_flags; }

    RTPS_DllAPI inline void bitcount(
            const octet& _bitcount) { m_bitcount = _bitcount; }
    RTPS_DllAPI inline void bitcount(
            octet&& _bitcount) { m_bitcount = std::move(_bitcount); }
    RTPS_DllAPI inline const octet& bitcount() const { return m_bitcount; }
    RTPS_DllAPI inline octet& bitcount() { return m_bitcount; }

    RTPS_DllAPI inline void holder_type(
            const TypeKind& _holder_type) { m_holder_type = _holder_type; }
    RTPS_DllAPI inline void holder_type(
            TypeKind&& _holder_type) { m_holder_type = std::move(_holder_type); }
    RTPS_DllAPI inline const TypeKind& holder_type() const { return m_holder_type; }
    RTPS_DllAPI inline TypeKind& holder_type() { return m_holder_type; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CommonBitfield& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CommonBitfield& other) const;

    RTPS_DllAPI bool consistent(
            const CommonBitfield& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    uint16_t m_position;
    BitsetMemberFlag m_flags;
    octet m_bitcount;
    TypeKind m_holder_type;
};

/*struct CompleteBitfield {
    CommonBitfield           common;
    CompleteMemberDetail     detail;
   };*/
class CompleteBitfield
{
public:

    RTPS_DllAPI CompleteBitfield();
    RTPS_DllAPI ~CompleteBitfield();
    RTPS_DllAPI CompleteBitfield(
            const CompleteBitfield& x);
    RTPS_DllAPI CompleteBitfield(
            CompleteBitfield&& x);
    RTPS_DllAPI CompleteBitfield& operator=(
            const CompleteBitfield& x);
    RTPS_DllAPI CompleteBitfield& operator=(
            CompleteBitfield&& x);

    RTPS_DllAPI inline void common(
            const CommonBitfield& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonBitfield&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonBitfield& common() const { return m_common; }
    RTPS_DllAPI inline CommonBitfield& common() { return m_common; }

    RTPS_DllAPI inline void detail(
            const CompleteMemberDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteMemberDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteMemberDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteMemberDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteBitfield& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteBitfield& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteBitfield& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonBitfield m_common;
    CompleteMemberDetail m_detail;
};
// Ordered by Bitfield.position
typedef std::vector<CompleteBitfield> CompleteBitfieldSeq;
/*struct MinimalBitfield {
    CommonBitfield       common;
    NameHash             name_hash;
   };*/
class MinimalBitfield
{
public:

    RTPS_DllAPI MinimalBitfield();
    RTPS_DllAPI ~MinimalBitfield();
    RTPS_DllAPI MinimalBitfield(
            const MinimalBitfield& x);
    RTPS_DllAPI MinimalBitfield(
            MinimalBitfield&& x);
    RTPS_DllAPI MinimalBitfield& operator=(
            const MinimalBitfield& x);
    RTPS_DllAPI MinimalBitfield& operator=(
            MinimalBitfield&& x);

    RTPS_DllAPI inline void name_hash(
            const NameHash& _name_hash) { m_name_hash = _name_hash; }
    RTPS_DllAPI inline void name_hash(
            NameHash&& _name_hash) { m_name_hash = std::move(_name_hash); }
    RTPS_DllAPI inline const NameHash& name_hash() const { return m_name_hash; }
    RTPS_DllAPI inline NameHash& name_hash() { return m_name_hash; }

    RTPS_DllAPI inline void common(
            const CommonBitfield& _common) { m_common = _common; }
    RTPS_DllAPI inline void common(
            CommonBitfield&& _common) { m_common = std::move(_common); }
    RTPS_DllAPI inline const CommonBitfield& common() const { return m_common; }
    RTPS_DllAPI inline CommonBitfield& common() { return m_common; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalBitfield& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalBitfield& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalBitfield& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    CommonBitfield m_common;
    NameHash m_name_hash;
};
// Ordered by Bitfield.position
typedef std::vector<MinimalBitfield> MinimalBitfieldSeq;
/*struct CompleteBitsetHeader {
    CompleteTypeDetail   detail;
   };*/
class CompleteBitsetHeader
{
public:

    RTPS_DllAPI CompleteBitsetHeader();
    RTPS_DllAPI ~CompleteBitsetHeader();
    RTPS_DllAPI CompleteBitsetHeader(
            const CompleteBitsetHeader& x);
    RTPS_DllAPI CompleteBitsetHeader(
            CompleteBitsetHeader&& x);
    RTPS_DllAPI CompleteBitsetHeader& operator=(
            const CompleteBitsetHeader& x);
    RTPS_DllAPI CompleteBitsetHeader& operator=(
            CompleteBitsetHeader&& x);

    RTPS_DllAPI inline void base_type(
            const TypeIdentifier& _base_type) { m_base_type = _base_type; }
    RTPS_DllAPI inline void base_type(
            TypeIdentifier&& _base_type) { m_base_type = std::move(_base_type); }
    RTPS_DllAPI inline const TypeIdentifier& base_type() const { return m_base_type; }
    RTPS_DllAPI inline TypeIdentifier& base_type() { return m_base_type; }

    RTPS_DllAPI inline void detail(
            const CompleteTypeDetail& _detail) { m_detail = _detail; }
    RTPS_DllAPI inline void detail(
            CompleteTypeDetail&& _detail) { m_detail = std::move(_detail); }
    RTPS_DllAPI inline const CompleteTypeDetail& detail() const { return m_detail; }
    RTPS_DllAPI inline CompleteTypeDetail& detail() { return m_detail; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteBitsetHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteBitsetHeader& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteBitsetHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    TypeIdentifier m_base_type;
    CompleteTypeDetail m_detail;
};

/*struct MinimalBitsetHeader {
    // Empty. Available for future extension
   };*/
class MinimalBitsetHeader
{
public:

    RTPS_DllAPI MinimalBitsetHeader();
    RTPS_DllAPI ~MinimalBitsetHeader();
    RTPS_DllAPI MinimalBitsetHeader(
            const MinimalBitsetHeader& x);
    RTPS_DllAPI MinimalBitsetHeader(
            MinimalBitsetHeader&& x);
    RTPS_DllAPI MinimalBitsetHeader& operator=(
            const MinimalBitsetHeader& x);
    RTPS_DllAPI MinimalBitsetHeader& operator=(
            MinimalBitsetHeader&& x);

    RTPS_DllAPI inline void base_type(
            const TypeIdentifier& _base_type) { m_base_type = _base_type; }
    RTPS_DllAPI inline void base_type(
            TypeIdentifier&& _base_type) { m_base_type = std::move(_base_type); }
    RTPS_DllAPI inline const TypeIdentifier& base_type() const { return m_base_type; }
    RTPS_DllAPI inline TypeIdentifier& base_type() { return m_base_type; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalBitsetHeader& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalBitsetHeader& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalBitsetHeader& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    TypeIdentifier m_base_type;
};
/*struct CompleteBitsetType  {
    BitsetTypeFlag         bitset_flags; // unused
    CompleteBitsetHeader   header;
    CompleteBitfieldSeq    field_seq;
   };*/
class CompleteBitsetType
{
public:

    RTPS_DllAPI CompleteBitsetType();
    RTPS_DllAPI ~CompleteBitsetType();
    RTPS_DllAPI CompleteBitsetType(
            const CompleteBitsetType& x);
    RTPS_DllAPI CompleteBitsetType(
            CompleteBitsetType&& x);
    RTPS_DllAPI CompleteBitsetType& operator=(
            const CompleteBitsetType& x);
    RTPS_DllAPI CompleteBitsetType& operator=(
            CompleteBitsetType&& x);

    RTPS_DllAPI inline void bitset_flags(
            const BitsetTypeFlag& _bitset_flags) { m_bitset_flags = _bitset_flags; }
    RTPS_DllAPI inline void bitset_flags(
            BitsetTypeFlag&& _bitset_flags) { m_bitset_flags = std::move(_bitset_flags); }
    RTPS_DllAPI inline const BitsetTypeFlag& bitset_flags() const { return m_bitset_flags; }
    RTPS_DllAPI inline BitsetTypeFlag& bitset_flags() { return m_bitset_flags; }

    RTPS_DllAPI inline void header(
            const CompleteBitsetHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            CompleteBitsetHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const CompleteBitsetHeader& header() const { return m_header; }
    RTPS_DllAPI inline CompleteBitsetHeader& header() { return m_header; }

    RTPS_DllAPI inline void field_seq(
            const CompleteBitfieldSeq& _field_seq) { m_field_seq = _field_seq; }
    RTPS_DllAPI inline void field_seq(
            CompleteBitfieldSeq&& _field_seq) { m_field_seq = std::move(_field_seq); }
    RTPS_DllAPI inline const CompleteBitfieldSeq& field_seq() const { return m_field_seq; }
    RTPS_DllAPI inline CompleteBitfieldSeq& field_seq() { return m_field_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteBitsetType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteBitsetType& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteBitsetType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    BitsetTypeFlag m_bitset_flags;
    CompleteBitsetHeader m_header;
    CompleteBitfieldSeq m_field_seq;
};

/*struct MinimalBitsetType  {
    BitsetTypeFlag       bitset_flags; // unused
    MinimalBitsetHeader  header;
    MinimalBitfieldSeq   field_seq;
   };*/
class MinimalBitsetType
{
public:

    RTPS_DllAPI MinimalBitsetType();
    RTPS_DllAPI ~MinimalBitsetType();
    RTPS_DllAPI MinimalBitsetType(
            const MinimalBitsetType& x);
    RTPS_DllAPI MinimalBitsetType(
            MinimalBitsetType&& x);
    RTPS_DllAPI MinimalBitsetType& operator=(
            const MinimalBitsetType& x);
    RTPS_DllAPI MinimalBitsetType& operator=(
            MinimalBitsetType&& x);

    RTPS_DllAPI inline void bitset_flags(
            const BitsetTypeFlag& _bitset_flags) { m_bitset_flags = _bitset_flags; }
    RTPS_DllAPI inline void bitset_flags(
            BitsetTypeFlag&& _bitset_flags) { m_bitset_flags = std::move(_bitset_flags); }
    RTPS_DllAPI inline const BitsetTypeFlag& bitset_flags() const { return m_bitset_flags; }
    RTPS_DllAPI inline BitsetTypeFlag& bitset_flags() { return m_bitset_flags; }

    RTPS_DllAPI inline void header(
            const MinimalBitsetHeader& _header) { m_header = _header; }
    RTPS_DllAPI inline void header(
            MinimalBitsetHeader&& _header) { m_header = std::move(_header); }
    RTPS_DllAPI inline const MinimalBitsetHeader& header() const { return m_header; }
    RTPS_DllAPI inline MinimalBitsetHeader& header() { return m_header; }

    RTPS_DllAPI inline void field_seq(
            const MinimalBitfieldSeq& _field_seq) { m_field_seq = _field_seq; }
    RTPS_DllAPI inline void field_seq(
            MinimalBitfieldSeq&& _field_seq) { m_field_seq = std::move(_field_seq); }
    RTPS_DllAPI inline const MinimalBitfieldSeq& field_seq() const { return m_field_seq; }
    RTPS_DllAPI inline MinimalBitfieldSeq& field_seq() { return m_field_seq; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalBitsetType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalBitsetType& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalBitsetType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    BitsetTypeFlag m_bitset_flags;
    MinimalBitsetHeader m_header;
    MinimalBitfieldSeq m_field_seq;
};

// --- Type Object: --------------------------------------------------
// The types associated with each case selection must have extensibility
// kind APPENDABLE or MUTABLE so that they can be extended in the future

/*struct CompleteExtendedType {
    // Empty. Available for future extension
   };*/
class CompleteExtendedType
{
public:

    RTPS_DllAPI CompleteExtendedType();
    RTPS_DllAPI ~CompleteExtendedType();
    RTPS_DllAPI CompleteExtendedType(
            const CompleteExtendedType& x);
    RTPS_DllAPI CompleteExtendedType(
            CompleteExtendedType&& x);
    RTPS_DllAPI CompleteExtendedType& operator=(
            const CompleteExtendedType& x);
    RTPS_DllAPI CompleteExtendedType& operator=(
            CompleteExtendedType&& x);

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteExtendedType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteExtendedType&) const { return true; }

    RTPS_DllAPI bool consistent(
            const CompleteExtendedType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

};

/*struct MinimalExtendedType  {
    // Empty. Available for future extension
   };*/
class MinimalExtendedType
{
public:

    RTPS_DllAPI MinimalExtendedType();
    RTPS_DllAPI ~MinimalExtendedType();
    RTPS_DllAPI MinimalExtendedType(
            const MinimalExtendedType& x);
    RTPS_DllAPI MinimalExtendedType(
            MinimalExtendedType&& x);
    RTPS_DllAPI MinimalExtendedType& operator=(
            const MinimalExtendedType& x);
    RTPS_DllAPI MinimalExtendedType& operator=(
            MinimalExtendedType&& x);

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalExtendedType& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalExtendedType&) const { return true; }

    RTPS_DllAPI bool consistent(
            const MinimalExtendedType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

};

class CompleteTypeObject final
{
public:

    RTPS_DllAPI CompleteTypeObject();
    RTPS_DllAPI ~CompleteTypeObject();
    RTPS_DllAPI CompleteTypeObject(
            const CompleteTypeObject& x);
    RTPS_DllAPI CompleteTypeObject(
            CompleteTypeObject&& x);
    RTPS_DllAPI CompleteTypeObject& operator=(
            const CompleteTypeObject& x);
    RTPS_DllAPI CompleteTypeObject& operator=(
            CompleteTypeObject&& x);
    RTPS_DllAPI void _d(
            octet __d);
    RTPS_DllAPI octet _d() const;
    RTPS_DllAPI octet& _d();

    RTPS_DllAPI void alias_type(
            CompleteAliasType _alias_type);
    RTPS_DllAPI const CompleteAliasType& alias_type() const;
    RTPS_DllAPI CompleteAliasType& alias_type();

    RTPS_DllAPI void annotation_type(
            CompleteAnnotationType _annotation_type);
    RTPS_DllAPI const CompleteAnnotationType& annotation_type() const;
    RTPS_DllAPI CompleteAnnotationType& annotation_type();

    RTPS_DllAPI void struct_type(
            CompleteStructType _struct_type);
    RTPS_DllAPI const CompleteStructType& struct_type() const;
    RTPS_DllAPI CompleteStructType& struct_type();

    RTPS_DllAPI void union_type(
            CompleteUnionType _union_type);
    RTPS_DllAPI const CompleteUnionType& union_type() const;
    RTPS_DllAPI CompleteUnionType& union_type();

    RTPS_DllAPI void bitset_type(
            CompleteBitsetType _bitset_type);
    RTPS_DllAPI const CompleteBitsetType& bitset_type() const;
    RTPS_DllAPI CompleteBitsetType& bitset_type();

    RTPS_DllAPI void sequence_type(
            CompleteSequenceType _sequence_type);
    RTPS_DllAPI const CompleteSequenceType& sequence_type() const;
    RTPS_DllAPI CompleteSequenceType& sequence_type();

    RTPS_DllAPI void array_type(
            CompleteArrayType _array_type);
    RTPS_DllAPI const CompleteArrayType& array_type() const;
    RTPS_DllAPI CompleteArrayType& array_type();

    RTPS_DllAPI void map_type(
            CompleteMapType _map_type);
    RTPS_DllAPI const CompleteMapType& map_type() const;
    RTPS_DllAPI CompleteMapType& map_type();

    RTPS_DllAPI void enumerated_type(
            CompleteEnumeratedType _enumerated_type);
    RTPS_DllAPI const CompleteEnumeratedType& enumerated_type() const;
    RTPS_DllAPI CompleteEnumeratedType& enumerated_type();

    RTPS_DllAPI void bitmask_type(
            CompleteBitmaskType _bitmask_type);
    RTPS_DllAPI const CompleteBitmaskType& bitmask_type() const;
    RTPS_DllAPI CompleteBitmaskType& bitmask_type();

    RTPS_DllAPI void extended_type(
            CompleteExtendedType _extended_type);
    RTPS_DllAPI const CompleteExtendedType& extended_type() const;
    RTPS_DllAPI CompleteExtendedType& extended_type();

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const CompleteTypeObject& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const CompleteTypeObject& other) const;

    RTPS_DllAPI bool consistent(
            const CompleteTypeObject& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    octet m__d;

    CompleteAliasType m_alias_type;
    CompleteAnnotationType m_annotation_type;
    CompleteStructType m_struct_type;
    CompleteUnionType m_union_type;
    CompleteBitsetType m_bitset_type;
    CompleteSequenceType m_sequence_type;
    CompleteArrayType m_array_type;
    CompleteMapType m_map_type;
    CompleteEnumeratedType m_enumerated_type;
    CompleteBitmaskType m_bitmask_type;
    CompleteExtendedType m_extended_type;
};

class MinimalTypeObject final
{
public:

    RTPS_DllAPI MinimalTypeObject();
    RTPS_DllAPI ~MinimalTypeObject();
    RTPS_DllAPI MinimalTypeObject(
            const MinimalTypeObject& x);
    RTPS_DllAPI MinimalTypeObject(
            MinimalTypeObject&& x);
    RTPS_DllAPI MinimalTypeObject& operator=(
            const MinimalTypeObject& x);
    RTPS_DllAPI MinimalTypeObject& operator=(
            MinimalTypeObject&& x);
    RTPS_DllAPI void _d(
            octet __d);
    RTPS_DllAPI octet _d() const;
    RTPS_DllAPI octet& _d();

    RTPS_DllAPI void alias_type(
            MinimalAliasType _alias_type);
    RTPS_DllAPI const MinimalAliasType& alias_type() const;
    RTPS_DllAPI MinimalAliasType& alias_type();

    RTPS_DllAPI void annotation_type(
            MinimalAnnotationType _annotation_type);
    RTPS_DllAPI const MinimalAnnotationType& annotation_type() const;
    RTPS_DllAPI MinimalAnnotationType& annotation_type();

    RTPS_DllAPI void struct_type(
            MinimalStructType _struct_type);
    RTPS_DllAPI const MinimalStructType& struct_type() const;
    RTPS_DllAPI MinimalStructType& struct_type();

    RTPS_DllAPI void union_type(
            MinimalUnionType _union_type);
    RTPS_DllAPI const MinimalUnionType& union_type() const;
    RTPS_DllAPI MinimalUnionType& union_type();

    RTPS_DllAPI void bitset_type(
            MinimalBitsetType _bitset_type);
    RTPS_DllAPI const MinimalBitsetType& bitset_type() const;
    RTPS_DllAPI MinimalBitsetType& bitset_type();

    RTPS_DllAPI void sequence_type(
            MinimalSequenceType _sequence_type);
    RTPS_DllAPI const MinimalSequenceType& sequence_type() const;
    RTPS_DllAPI MinimalSequenceType& sequence_type();

    RTPS_DllAPI void array_type(
            MinimalArrayType _array_type);
    RTPS_DllAPI const MinimalArrayType& array_type() const;
    RTPS_DllAPI MinimalArrayType& array_type();

    RTPS_DllAPI void map_type(
            MinimalMapType _map_type);
    RTPS_DllAPI const MinimalMapType& map_type() const;
    RTPS_DllAPI MinimalMapType& map_type();

    RTPS_DllAPI void enumerated_type(
            MinimalEnumeratedType _enumerated_type);
    RTPS_DllAPI const MinimalEnumeratedType& enumerated_type() const;
    RTPS_DllAPI MinimalEnumeratedType& enumerated_type();

    RTPS_DllAPI void bitmask_type(
            MinimalBitmaskType _bitmask_type);
    RTPS_DllAPI const MinimalBitmaskType& bitmask_type() const;
    RTPS_DllAPI MinimalBitmaskType& bitmask_type();

    RTPS_DllAPI void extended_type(
            MinimalExtendedType _extended_type);
    RTPS_DllAPI const MinimalExtendedType& extended_type() const;
    RTPS_DllAPI MinimalExtendedType& extended_type();

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const MinimalTypeObject& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const MinimalTypeObject& other) const;

    RTPS_DllAPI bool consistent(
            const MinimalTypeObject& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    octet m__d;

    MinimalAliasType m_alias_type;
    MinimalAnnotationType m_annotation_type;
    MinimalStructType m_struct_type;
    MinimalUnionType m_union_type;
    MinimalBitsetType m_bitset_type;
    MinimalSequenceType m_sequence_type;
    MinimalArrayType m_array_type;
    MinimalMapType m_map_type;
    MinimalEnumeratedType m_enumerated_type;
    MinimalBitmaskType m_bitmask_type;
    MinimalExtendedType m_extended_type;
};
class TypeObject
{
public:

    /*!
     * @brief Default constructor.
     */
    RTPS_DllAPI TypeObject();

    /*!
     * @brief Default destructor.
     */
    RTPS_DllAPI ~TypeObject();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object TypeObject that will be copied.
     */
    RTPS_DllAPI TypeObject(
            const TypeObject& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object TypeObject that will be copied.
     */
    RTPS_DllAPI TypeObject(
            TypeObject&& x);

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object TypeObject that will be copied.
     */
    RTPS_DllAPI TypeObject& operator=(
            const TypeObject& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object TypeObject that will be copied.
     */
    RTPS_DllAPI TypeObject& operator=(
            TypeObject&& x);

    /*!
     * @brief This function sets the discriminator value.
     * @param __d New value for the discriminator.
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
     */
    RTPS_DllAPI void _d(
            uint8_t __d);

    /*!
     * @brief This function returns the value of the discriminator.
     * @return Value of the discriminator
     */
    RTPS_DllAPI uint8_t _d() const;

    /*!
     * @brief This function returns a reference to the discriminator.
     * @return Reference to the discriminator.
     */
    RTPS_DllAPI uint8_t& _d();

    /*!
     * @brief This function copies the value in member complete
     * @param _complete New value to be copied in member complete
     */
    RTPS_DllAPI void complete(
            const CompleteTypeObject& _complete);

    /*!
     * @brief This function moves the value in member complete
     * @param _complete New value to be moved in member complete
     */
    RTPS_DllAPI void complete(
            CompleteTypeObject&& _complete);

    /*!
     * @brief This function returns a constant reference to member complete
     * @return Constant reference to member complete
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    RTPS_DllAPI const CompleteTypeObject& complete() const;

    /*!
     * @brief This function returns a reference to member complete
     * @return Reference to member complete
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    RTPS_DllAPI CompleteTypeObject& complete();
    /*!
     * @brief This function copies the value in member minimal
     * @param _minimal New value to be copied in member minimal
     */
    RTPS_DllAPI void minimal(
            const MinimalTypeObject& _minimal);

    /*!
     * @brief This function moves the value in member minimal
     * @param _minimal New value to be moved in member minimal
     */
    RTPS_DllAPI void minimal(
            MinimalTypeObject&& _minimal);

    /*!
     * @brief This function returns a constant reference to member minimal
     * @return Constant reference to member minimal
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    RTPS_DllAPI const MinimalTypeObject& minimal() const;

    /*!
     * @brief This function returns a reference to member minimal
     * @return Reference to member minimal
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    RTPS_DllAPI MinimalTypeObject& minimal();

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    RTPS_DllAPI static size_t getCdrSerializedSize(
            const TypeObject& data,
            size_t current_alignment = 0);
    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

    RTPS_DllAPI bool operator==(
            const TypeObject& other) const;

    /*!
     * @brief This function check type consistency enforcement with the given TypeObject x.
     * @param x TypeObject to check if can be assigned to the current instance.
     * @param consistency fastdds::dds::TypeConsistencyEnforcementQoSPolicy to apply.
     */
    RTPS_DllAPI bool consistent(
            const TypeObject& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

    uint8_t m__d;

    CompleteTypeObject m_complete;
    MinimalTypeObject m_minimal;
};

typedef std::vector<TypeObject> TypeObjectSeq;

// Set of TypeObjects representing a strong component: Equivalence class
// for the Strong Connectivity relationship (mutual reachability between
// types).
// Ordered by fully qualified typename lexicographic order
typedef TypeObjectSeq StronglyConnectedComponent;

/*struct TypeIdentifierTypeObjectPair final {
    TypeIdentifier  type_identifier;
    TypeObject      type_object;
   };*/
class TypeIdentifierTypeObjectPair final
{
public:

    RTPS_DllAPI TypeIdentifierTypeObjectPair();
    RTPS_DllAPI ~TypeIdentifierTypeObjectPair();
    RTPS_DllAPI TypeIdentifierTypeObjectPair(
            const TypeIdentifierTypeObjectPair& x);
    RTPS_DllAPI TypeIdentifierTypeObjectPair(
            TypeIdentifierTypeObjectPair&& x);
    RTPS_DllAPI TypeIdentifierTypeObjectPair& operator=(
            const TypeIdentifierTypeObjectPair& x);
    RTPS_DllAPI TypeIdentifierTypeObjectPair& operator=(
            TypeIdentifierTypeObjectPair&& x);

    RTPS_DllAPI inline void type_identifier(
            const TypeIdentifier& _type_identifier) { m_type_identifier = _type_identifier; }
    RTPS_DllAPI inline void type_identifier(
            TypeIdentifier&& _type_identifier) { m_type_identifier = std::move(_type_identifier); }
    RTPS_DllAPI inline const TypeIdentifier& type_identifier() const { return m_type_identifier; }
    RTPS_DllAPI inline TypeIdentifier& type_identifier() { return m_type_identifier; }

    RTPS_DllAPI inline void type_object(
            const TypeObject& _type_object) { m_type_object = _type_object; }
    RTPS_DllAPI inline void type_object(
            TypeObject&& _type_object) { m_type_object = std::move(_type_object); }
    RTPS_DllAPI inline const TypeObject& type_object() const { return m_type_object; }
    RTPS_DllAPI inline TypeObject& type_object() { return m_type_object; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const TypeIdentifierTypeObjectPair& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

private:

    TypeIdentifier m_type_identifier;
    TypeObject m_type_object;
};
typedef std::vector<TypeIdentifierTypeObjectPair> TypeIdentifierTypeObjectPairSeq;

/*struct TypeIdentifierPair final {
    TypeIdentifier  type_identifier1;
    TypeIdentifier  type_identifier2;
   };*/
class TypeIdentifierPair final
{
public:

    RTPS_DllAPI TypeIdentifierPair();
    RTPS_DllAPI ~TypeIdentifierPair();
    RTPS_DllAPI TypeIdentifierPair(
            const TypeIdentifierPair& x);
    RTPS_DllAPI TypeIdentifierPair(
            TypeIdentifierPair&& x);
    RTPS_DllAPI TypeIdentifierPair& operator=(
            const TypeIdentifierPair& x);
    RTPS_DllAPI TypeIdentifierPair& operator=(
            TypeIdentifierPair&& x);

    RTPS_DllAPI inline void type_identifier1(
            const TypeIdentifier& _type_identifier1) { m_type_identifier1 = _type_identifier1; }
    RTPS_DllAPI inline void type_identifier1(
            TypeIdentifier&& _type_identifier1) { m_type_identifier1 = std::move(_type_identifier1); }
    RTPS_DllAPI inline const TypeIdentifier& type_identifier1() const { return m_type_identifier1; }
    RTPS_DllAPI inline TypeIdentifier& type_identifier1() { return m_type_identifier1; }

    RTPS_DllAPI inline void type_identifier2(
            const TypeIdentifier& _type_identifier2) { m_type_identifier2 = _type_identifier2; }
    RTPS_DllAPI inline void type_identifier2(
            TypeIdentifier&& _type_identifier2) { m_type_identifier2 = std::move(_type_identifier2); }
    RTPS_DllAPI inline const TypeIdentifier& type_identifier2() const { return m_type_identifier2; }
    RTPS_DllAPI inline TypeIdentifier& type_identifier2() { return m_type_identifier2; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const TypeIdentifierPair& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

private:

    TypeIdentifier m_type_identifier1;
    TypeIdentifier m_type_identifier2;
};

typedef std::vector<TypeIdentifierPair> TypeIdentifierPairSeq;
/*struct TypeIdentifierWithSize {
    TypeIdentifier  type_id;
    uint32_t                typeobject_serialized_size;
   };*/
class TypeIdentifierWithSize
{
public:

    RTPS_DllAPI TypeIdentifierWithSize();
    RTPS_DllAPI ~TypeIdentifierWithSize();
    RTPS_DllAPI TypeIdentifierWithSize(
            const TypeIdentifierWithSize& x);
    RTPS_DllAPI TypeIdentifierWithSize(
            TypeIdentifierWithSize&& x);
    RTPS_DllAPI TypeIdentifierWithSize& operator=(
            const TypeIdentifierWithSize& x);
    RTPS_DllAPI TypeIdentifierWithSize& operator=(
            TypeIdentifierWithSize&& x);

    RTPS_DllAPI inline void type_id(
            const TypeIdentifier& _type_id) { m_type_id = _type_id; }
    RTPS_DllAPI inline void type_id(
            TypeIdentifier&& _type_id) { m_type_id = std::move(_type_id); }
    RTPS_DllAPI inline const TypeIdentifier& type_id() const { return m_type_id; }
    RTPS_DllAPI inline TypeIdentifier& type_id() { return m_type_id; }

    RTPS_DllAPI inline void typeobject_serialized_size(
            const uint32_t& _typeobject_serialized_size) { m_typeobject_serialized_size = _typeobject_serialized_size; }
    RTPS_DllAPI inline void typeobject_serialized_size(
            uint32_t&& _typeobject_serialized_size) {
        m_typeobject_serialized_size = std::move(_typeobject_serialized_size);
    }
    RTPS_DllAPI inline const uint32_t& typeobject_serialized_size() const { return m_typeobject_serialized_size; }
    RTPS_DllAPI inline uint32_t& typeobject_serialized_size() { return m_typeobject_serialized_size; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const TypeIdentifierWithSize& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

private:

    TypeIdentifier m_type_id;
    uint32_t m_typeobject_serialized_size;
};

typedef std::vector<TypeIdentifierWithSize> TypeIdentifierWithSizeSeq;

/*struct TypeIdentifierWithDependencies {
    TypeIdentifierWithSize            typeid_with_size;
    // The total additional types related to minimal_type
    int32_t                             dependent_typeid_count;
    TypeIdentifierWithSizeSeq  dependent_typeids;
   };*/
class TypeIdentifierWithDependencies
{
public:

    RTPS_DllAPI TypeIdentifierWithDependencies();
    RTPS_DllAPI ~TypeIdentifierWithDependencies();
    RTPS_DllAPI TypeIdentifierWithDependencies(
            const TypeIdentifierWithDependencies& x);
    RTPS_DllAPI TypeIdentifierWithDependencies(
            TypeIdentifierWithDependencies&& x);
    RTPS_DllAPI TypeIdentifierWithDependencies& operator=(
            const TypeIdentifierWithDependencies& x);
    RTPS_DllAPI TypeIdentifierWithDependencies& operator=(
            TypeIdentifierWithDependencies&& x);

    RTPS_DllAPI inline void typeid_with_size(
            const TypeIdentifierWithSize& _typeid_with_size) { m_typeid_with_size = _typeid_with_size; }
    RTPS_DllAPI inline void typeid_with_size(
            TypeIdentifierWithSize&& _typeid_with_size) { m_typeid_with_size = std::move(_typeid_with_size); }
    RTPS_DllAPI inline const TypeIdentifierWithSize& typeid_with_size() const { return m_typeid_with_size; }
    RTPS_DllAPI inline TypeIdentifierWithSize& typeid_with_size() { return m_typeid_with_size; }

    RTPS_DllAPI inline void dependent_typeid_count(
            const int32_t& _dependent_typeid_count) { m_dependent_typeid_count = _dependent_typeid_count; }
    RTPS_DllAPI inline void dependent_typeid_count(
            int32_t&& _dependent_typeid_count) { m_dependent_typeid_count = std::move(_dependent_typeid_count); }
    RTPS_DllAPI inline const int32_t& dependent_typeid_count() const { return m_dependent_typeid_count; }
    RTPS_DllAPI inline int32_t& dependent_typeid_count() { return m_dependent_typeid_count; }

    RTPS_DllAPI inline void dependent_typeids(
            const TypeIdentifierWithSizeSeq& _dependent_typeids) { m_dependent_typeids = _dependent_typeids; }
    RTPS_DllAPI inline void dependent_typeids(
            TypeIdentifierWithSizeSeq&& _dependent_typeids) { m_dependent_typeids = std::move(_dependent_typeids); }
    RTPS_DllAPI inline const TypeIdentifierWithSizeSeq& dependent_typeids() const { return m_dependent_typeids; }
    RTPS_DllAPI inline TypeIdentifierWithSizeSeq& dependent_typeids() { return m_dependent_typeids; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const TypeIdentifierWithDependencies& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

private:

    TypeIdentifierWithSize m_typeid_with_size;
    int32_t m_dependent_typeid_count;
    TypeIdentifierWithSizeSeq m_dependent_typeids;
};

typedef    std::vector<TypeIdentifierWithDependencies> TypeIdentifierWithDependenciesSeq;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This appears in the builtin DDS topics PublicationBuiltinTopicData
// and SubscriptionBuiltinTopicData

/*struct TypeInformation  {
    TypeIdentifierWithDependencies minimal; // @id{0x1001}
    TypeIdentifierWithDependencies complete; // @id{0x1002}
   };*/
class TypeInformation
{
public:

    RTPS_DllAPI TypeInformation();
    RTPS_DllAPI ~TypeInformation();
    RTPS_DllAPI TypeInformation(
            const TypeInformation& x);
    RTPS_DllAPI TypeInformation(
            TypeInformation&& x);
    RTPS_DllAPI TypeInformation& operator=(
            const TypeInformation& x);
    RTPS_DllAPI TypeInformation& operator=(
            TypeInformation&& x);

    RTPS_DllAPI inline void minimal(
            const TypeIdentifierWithDependencies& _minimal) { m_minimal = _minimal; }
    RTPS_DllAPI inline void minimal(
            TypeIdentifierWithDependencies&& _minimal) { m_minimal = std::move(_minimal); }
    RTPS_DllAPI inline const TypeIdentifierWithDependencies& minimal() const { return m_minimal; }
    RTPS_DllAPI inline TypeIdentifierWithDependencies& minimal() { return m_minimal; }

    RTPS_DllAPI inline void complete(
            const TypeIdentifierWithDependencies& _complete) { m_complete = _complete; }
    RTPS_DllAPI inline void complete(
            TypeIdentifierWithDependencies&& _complete) { m_complete = std::move(_complete); }
    RTPS_DllAPI inline const TypeIdentifierWithDependencies& complete() const { return m_complete; }
    RTPS_DllAPI inline TypeIdentifierWithDependencies& complete() { return m_complete; }

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const TypeInformation& data,
            size_t current_alignment = 0);
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;
    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr& cdr);

private:

    TypeIdentifierWithDependencies m_minimal;
    TypeIdentifierWithDependencies m_complete;
};

typedef std::vector<TypeInformation> TypeInformationSeq;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_OBJECT_H
