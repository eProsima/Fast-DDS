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

#include <array>
#include <cstdint>

#include <fastrtps/types/AnnotationParameterValue.h>
#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/TypeObjectHashId.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

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

    FASTDDS_EXPORTED_API CommonStructMember();
    FASTDDS_EXPORTED_API ~CommonStructMember();
    FASTDDS_EXPORTED_API CommonStructMember(
            const CommonStructMember& x);
    FASTDDS_EXPORTED_API CommonStructMember(
            CommonStructMember&& x);
    FASTDDS_EXPORTED_API CommonStructMember& operator =(
            const CommonStructMember& x);
    FASTDDS_EXPORTED_API CommonStructMember& operator =(
            CommonStructMember&& x);

    FASTDDS_EXPORTED_API inline void member_id(
            const MemberId& _member_id)
    {
        m_member_id = _member_id;
    }

    FASTDDS_EXPORTED_API inline void member_id(
            MemberId&& _member_id)
    {
        m_member_id = std::move(_member_id);
    }

    FASTDDS_EXPORTED_API inline const MemberId& member_id() const
    {
        return m_member_id;
    }

    FASTDDS_EXPORTED_API inline MemberId& member_id()
    {
        return m_member_id;
    }

    FASTDDS_EXPORTED_API inline void member_flags(
            const StructMemberFlag& _member_flags)
    {
        m_member_flags = _member_flags;
    }

    FASTDDS_EXPORTED_API inline void member_flags(
            StructMemberFlag&& _member_flags)
    {
        m_member_flags = std::move(_member_flags);
    }

    FASTDDS_EXPORTED_API inline const StructMemberFlag& member_flags() const
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline StructMemberFlag& member_flags()
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline void member_type_id(
            const TypeIdentifier& _member_type_id)
    {
        m_member_type_id = _member_type_id;
    }

    FASTDDS_EXPORTED_API inline void member_type_id(
            TypeIdentifier&& _member_type_id)
    {
        m_member_type_id = std::move(_member_type_id);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& member_type_id() const
    {
        return m_member_type_id;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& member_type_id()
    {
        return m_member_type_id;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonStructMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteMemberDetail();
    FASTDDS_EXPORTED_API ~CompleteMemberDetail();
    FASTDDS_EXPORTED_API CompleteMemberDetail(
            const CompleteMemberDetail& x);
    FASTDDS_EXPORTED_API CompleteMemberDetail(
            CompleteMemberDetail&& x);
    FASTDDS_EXPORTED_API CompleteMemberDetail& operator =(
            const CompleteMemberDetail& x);
    FASTDDS_EXPORTED_API CompleteMemberDetail& operator =(
            CompleteMemberDetail&& x);

    FASTDDS_EXPORTED_API inline void name(
            const MemberName& _name)
    {
        m_name = _name;
    }

    FASTDDS_EXPORTED_API inline void name(
            MemberName&& _name)
    {
        m_name = std::move(_name);
    }

    FASTDDS_EXPORTED_API inline const MemberName& name() const
    {
        return m_name;
    }

    FASTDDS_EXPORTED_API inline MemberName& name()
    {
        return m_name;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            const AppliedBuiltinMemberAnnotations& _ann_builtin)
    {
        m_ann_builtin = _ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            AppliedBuiltinMemberAnnotations&& _ann_builtin)
    {
        m_ann_builtin = std::move(_ann_builtin);
    }

    FASTDDS_EXPORTED_API inline const AppliedBuiltinMemberAnnotations& ann_builtin() const
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline AppliedBuiltinMemberAnnotations& ann_builtin()
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom)
    {
        m_ann_custom = _ann_custom;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom)
    {
        m_ann_custom = std::move(_ann_custom);
    }

    FASTDDS_EXPORTED_API inline const AppliedAnnotationSeq& ann_custom() const
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API inline AppliedAnnotationSeq& ann_custom()
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteMemberDetail& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalMemberDetail();
    FASTDDS_EXPORTED_API ~MinimalMemberDetail();
    FASTDDS_EXPORTED_API MinimalMemberDetail(
            const MinimalMemberDetail& x);
    FASTDDS_EXPORTED_API MinimalMemberDetail(
            MinimalMemberDetail&& x);
    FASTDDS_EXPORTED_API MinimalMemberDetail& operator =(
            const MinimalMemberDetail& x);
    FASTDDS_EXPORTED_API MinimalMemberDetail& operator =(
            MinimalMemberDetail&& x);

    FASTDDS_EXPORTED_API inline void name_hash(
            const NameHash& _name_hash)
    {
        m_name_hash = _name_hash;
    }

    FASTDDS_EXPORTED_API inline void name_hash(
            NameHash&& _name_hash)
    {
        m_name_hash = std::move(_name_hash);
    }

    FASTDDS_EXPORTED_API inline const NameHash& name_hash() const
    {
        return m_name_hash;
    }

    FASTDDS_EXPORTED_API inline NameHash& name_hash()
    {
        return m_name_hash;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalMemberDetail& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteStructMember();
    FASTDDS_EXPORTED_API ~CompleteStructMember();
    FASTDDS_EXPORTED_API CompleteStructMember(
            const CompleteStructMember& x);
    FASTDDS_EXPORTED_API CompleteStructMember(
            CompleteStructMember&& x);
    FASTDDS_EXPORTED_API CompleteStructMember& operator =(
            const CompleteStructMember& x);
    FASTDDS_EXPORTED_API CompleteStructMember& operator =(
            CompleteStructMember&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonStructMember& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonStructMember&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonStructMember& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonStructMember& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteStructMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalStructMember();
    FASTDDS_EXPORTED_API ~MinimalStructMember();
    FASTDDS_EXPORTED_API MinimalStructMember(
            const MinimalStructMember& x);
    FASTDDS_EXPORTED_API MinimalStructMember(
            MinimalStructMember&& x);
    FASTDDS_EXPORTED_API MinimalStructMember& operator =(
            const MinimalStructMember& x);
    FASTDDS_EXPORTED_API MinimalStructMember& operator =(
            MinimalStructMember&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonStructMember& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonStructMember&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonStructMember& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonStructMember& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const MinimalMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            MinimalMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const MinimalMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline MinimalMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalStructMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API AppliedBuiltinTypeAnnotations();
    FASTDDS_EXPORTED_API ~AppliedBuiltinTypeAnnotations();
    FASTDDS_EXPORTED_API AppliedBuiltinTypeAnnotations(
            const AppliedBuiltinTypeAnnotations& x);
    FASTDDS_EXPORTED_API AppliedBuiltinTypeAnnotations(
            AppliedBuiltinTypeAnnotations&& x);
    FASTDDS_EXPORTED_API AppliedBuiltinTypeAnnotations& operator =(
            const AppliedBuiltinTypeAnnotations& x);
    FASTDDS_EXPORTED_API AppliedBuiltinTypeAnnotations& operator =(
            AppliedBuiltinTypeAnnotations&& x);

    FASTDDS_EXPORTED_API inline void verbatim(
            const AppliedVerbatimAnnotation& _verbatim)
    {
        m_verbatim = _verbatim;
    }

    FASTDDS_EXPORTED_API inline void verbatim(
            AppliedVerbatimAnnotation&& _verbatim)
    {
        m_verbatim = std::move(_verbatim);
    }

    FASTDDS_EXPORTED_API inline const AppliedVerbatimAnnotation& verbatim() const
    {
        return m_verbatim;
    }

    FASTDDS_EXPORTED_API inline AppliedVerbatimAnnotation& verbatim()
    {
        return m_verbatim;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const AppliedBuiltinTypeAnnotations& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalTypeDetail();
    FASTDDS_EXPORTED_API ~MinimalTypeDetail();
    FASTDDS_EXPORTED_API MinimalTypeDetail(
            const MinimalTypeDetail& x);
    FASTDDS_EXPORTED_API MinimalTypeDetail(
            MinimalTypeDetail&& x);
    FASTDDS_EXPORTED_API MinimalTypeDetail& operator =(
            const MinimalTypeDetail& x);
    FASTDDS_EXPORTED_API MinimalTypeDetail& operator =(
            MinimalTypeDetail&& x);

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalTypeDetail&) const
    {
        return true;
    }

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteTypeDetail();
    FASTDDS_EXPORTED_API ~CompleteTypeDetail();
    FASTDDS_EXPORTED_API CompleteTypeDetail(
            const CompleteTypeDetail& x);
    FASTDDS_EXPORTED_API CompleteTypeDetail(
            CompleteTypeDetail&& x);
    FASTDDS_EXPORTED_API CompleteTypeDetail& operator =(
            const CompleteTypeDetail& x);
    FASTDDS_EXPORTED_API CompleteTypeDetail& operator =(
            CompleteTypeDetail&& x);

    FASTDDS_EXPORTED_API inline void ann_builtin(
            const AppliedBuiltinTypeAnnotations& _ann_builtin)
    {
        m_ann_builtin = _ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            AppliedBuiltinTypeAnnotations&& _ann_builtin)
    {
        m_ann_builtin = std::move(_ann_builtin);
    }

    FASTDDS_EXPORTED_API inline const AppliedBuiltinTypeAnnotations& ann_builtin() const
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline AppliedBuiltinTypeAnnotations& ann_builtin()
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom)
    {
        m_ann_custom = _ann_custom;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom)
    {
        m_ann_custom = std::move(_ann_custom);
    }

    FASTDDS_EXPORTED_API inline const AppliedAnnotationSeq& ann_custom() const
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API inline AppliedAnnotationSeq& ann_custom()
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API inline void type_name(
            const QualifiedTypeName& _type_name)
    {
        m_type_name = _type_name;
    }

    FASTDDS_EXPORTED_API inline void type_name(
            QualifiedTypeName&& _type_name)
    {
        m_type_name = std::move(_type_name);
    }

    FASTDDS_EXPORTED_API inline const QualifiedTypeName& type_name() const
    {
        return m_type_name;
    }

    FASTDDS_EXPORTED_API inline QualifiedTypeName& type_name()
    {
        return m_type_name;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteTypeDetail& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteStructHeader();
    FASTDDS_EXPORTED_API ~CompleteStructHeader();
    FASTDDS_EXPORTED_API CompleteStructHeader(
            const CompleteStructHeader& x);
    FASTDDS_EXPORTED_API CompleteStructHeader(
            CompleteStructHeader&& x);
    FASTDDS_EXPORTED_API CompleteStructHeader& operator =(
            const CompleteStructHeader& x);
    FASTDDS_EXPORTED_API CompleteStructHeader& operator =(
            CompleteStructHeader&& x);

    FASTDDS_EXPORTED_API inline void base_type(
            const TypeIdentifier& _base_type)
    {
        m_base_type = _base_type;
    }

    FASTDDS_EXPORTED_API inline void base_type(
            TypeIdentifier&& _base_type)
    {
        m_base_type = std::move(_base_type);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& base_type() const
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& base_type()
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteStructHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalStructHeader();
    FASTDDS_EXPORTED_API ~MinimalStructHeader();
    FASTDDS_EXPORTED_API MinimalStructHeader(
            const MinimalStructHeader& x);
    FASTDDS_EXPORTED_API MinimalStructHeader(
            MinimalStructHeader&& x);
    FASTDDS_EXPORTED_API MinimalStructHeader& operator =(
            const MinimalStructHeader& x);
    FASTDDS_EXPORTED_API MinimalStructHeader& operator =(
            MinimalStructHeader&& x);

    FASTDDS_EXPORTED_API inline void base_type(
            const TypeIdentifier& _base_type)
    {
        m_base_type = _base_type;
    }

    FASTDDS_EXPORTED_API inline void base_type(
            TypeIdentifier&& _base_type)
    {
        m_base_type = std::move(_base_type);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& base_type() const
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& base_type()
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const MinimalTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            MinimalTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const MinimalTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline MinimalTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalStructHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteStructType();
    FASTDDS_EXPORTED_API ~CompleteStructType();
    FASTDDS_EXPORTED_API CompleteStructType(
            const CompleteStructType& x);
    FASTDDS_EXPORTED_API CompleteStructType(
            CompleteStructType&& x);
    FASTDDS_EXPORTED_API CompleteStructType& operator =(
            const CompleteStructType& x);
    FASTDDS_EXPORTED_API CompleteStructType& operator =(
            CompleteStructType&& x);

    FASTDDS_EXPORTED_API inline void struct_flags(
            const StructTypeFlag& _struct_flags)
    {
        m_struct_flags = _struct_flags;
    }

    FASTDDS_EXPORTED_API inline void struct_flags(
            StructTypeFlag&& _struct_flags)
    {
        m_struct_flags = std::move(_struct_flags);
    }

    FASTDDS_EXPORTED_API inline const StructTypeFlag& struct_flags() const
    {
        return m_struct_flags;
    }

    FASTDDS_EXPORTED_API inline StructTypeFlag& struct_flags()
    {
        return m_struct_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteStructHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteStructHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteStructHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteStructHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            const CompleteStructMemberSeq& _member_seq)
    {
        m_member_seq = _member_seq;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            CompleteStructMemberSeq&& _member_seq)
    {
        m_member_seq = std::move(_member_seq);
    }

    FASTDDS_EXPORTED_API inline const CompleteStructMemberSeq& member_seq() const
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API inline CompleteStructMemberSeq& member_seq()
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteStructType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalStructType();
    FASTDDS_EXPORTED_API ~MinimalStructType();
    FASTDDS_EXPORTED_API MinimalStructType(
            const MinimalStructType& x);
    FASTDDS_EXPORTED_API MinimalStructType(
            MinimalStructType&& x);
    FASTDDS_EXPORTED_API MinimalStructType& operator =(
            const MinimalStructType& x);
    FASTDDS_EXPORTED_API MinimalStructType& operator =(
            MinimalStructType&& x);

    FASTDDS_EXPORTED_API inline void struct_flags(
            const StructTypeFlag& _struct_flags)
    {
        m_struct_flags = _struct_flags;
    }

    FASTDDS_EXPORTED_API inline void struct_flags(
            StructTypeFlag&& _struct_flags)
    {
        m_struct_flags = std::move(_struct_flags);
    }

    FASTDDS_EXPORTED_API inline const StructTypeFlag& struct_flags() const
    {
        return m_struct_flags;
    }

    FASTDDS_EXPORTED_API inline StructTypeFlag& struct_flags()
    {
        return m_struct_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalStructHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalStructHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalStructHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalStructHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            const MinimalStructMemberSeq& _member_seq)
    {
        m_member_seq = _member_seq;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            MinimalStructMemberSeq&& _member_seq)
    {
        m_member_seq = std::move(_member_seq);
    }

    FASTDDS_EXPORTED_API inline const MinimalStructMemberSeq& member_seq() const
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API inline MinimalStructMemberSeq& member_seq()
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalStructType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonUnionMember();
    FASTDDS_EXPORTED_API ~CommonUnionMember();
    FASTDDS_EXPORTED_API CommonUnionMember(
            const CommonUnionMember& x);
    FASTDDS_EXPORTED_API CommonUnionMember(
            CommonUnionMember&& x);
    FASTDDS_EXPORTED_API CommonUnionMember& operator =(
            const CommonUnionMember& x);
    FASTDDS_EXPORTED_API CommonUnionMember& operator =(
            CommonUnionMember&& x);

    FASTDDS_EXPORTED_API inline void member_id(
            const MemberId& _member_id)
    {
        m_member_id = _member_id;
    }

    FASTDDS_EXPORTED_API inline void member_id(
            MemberId&& _member_id)
    {
        m_member_id = std::move(_member_id);
    }

    FASTDDS_EXPORTED_API inline const MemberId& member_id() const
    {
        return m_member_id;
    }

    FASTDDS_EXPORTED_API inline MemberId& member_id()
    {
        return m_member_id;
    }

    FASTDDS_EXPORTED_API inline void member_flags(
            const UnionMemberFlag& _member_flags)
    {
        m_member_flags = _member_flags;
    }

    FASTDDS_EXPORTED_API inline void member_flags(
            UnionMemberFlag&& _member_flags)
    {
        m_member_flags = std::move(_member_flags);
    }

    FASTDDS_EXPORTED_API inline const UnionMemberFlag& member_flags() const
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline UnionMemberFlag& member_flags()
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline void type_id(
            const TypeIdentifier& _type_id)
    {
        m_type_id = _type_id;
    }

    FASTDDS_EXPORTED_API inline void type_id(
            TypeIdentifier&& _type_id)
    {
        m_type_id = std::move(_type_id);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type_id() const
    {
        return m_type_id;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type_id()
    {
        return m_type_id;
    }

    FASTDDS_EXPORTED_API inline void label_seq(
            const UnionCaseLabelSeq& _label_seq)
    {
        m_label_seq = _label_seq;
    }

    FASTDDS_EXPORTED_API inline void label_seq(
            UnionCaseLabelSeq&& _label_seq)
    {
        m_label_seq = std::move(_label_seq);
    }

    FASTDDS_EXPORTED_API inline const UnionCaseLabelSeq& label_seq() const
    {
        return m_label_seq;
    }

    FASTDDS_EXPORTED_API inline UnionCaseLabelSeq& label_seq()
    {
        return m_label_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonUnionMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteUnionMember();
    FASTDDS_EXPORTED_API ~CompleteUnionMember();
    FASTDDS_EXPORTED_API CompleteUnionMember(
            const CompleteUnionMember& x);
    FASTDDS_EXPORTED_API CompleteUnionMember(
            CompleteUnionMember&& x);
    FASTDDS_EXPORTED_API CompleteUnionMember& operator =(
            const CompleteUnionMember& x);
    FASTDDS_EXPORTED_API CompleteUnionMember& operator =(
            CompleteUnionMember&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonUnionMember& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonUnionMember&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonUnionMember& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonUnionMember& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteUnionMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalUnionMember();
    FASTDDS_EXPORTED_API ~MinimalUnionMember();
    FASTDDS_EXPORTED_API MinimalUnionMember(
            const MinimalUnionMember& x);
    FASTDDS_EXPORTED_API MinimalUnionMember(
            MinimalUnionMember&& x);
    FASTDDS_EXPORTED_API MinimalUnionMember& operator =(
            const MinimalUnionMember& x);
    FASTDDS_EXPORTED_API MinimalUnionMember& operator =(
            MinimalUnionMember&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonUnionMember& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonUnionMember&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonUnionMember& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonUnionMember& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const MinimalMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            MinimalMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const MinimalMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline MinimalMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalUnionMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonDiscriminatorMember();
    FASTDDS_EXPORTED_API ~CommonDiscriminatorMember();
    FASTDDS_EXPORTED_API CommonDiscriminatorMember(
            const CommonDiscriminatorMember& x);
    FASTDDS_EXPORTED_API CommonDiscriminatorMember(
            CommonDiscriminatorMember&& x);
    FASTDDS_EXPORTED_API CommonDiscriminatorMember& operator =(
            const CommonDiscriminatorMember& x);
    FASTDDS_EXPORTED_API CommonDiscriminatorMember& operator =(
            CommonDiscriminatorMember&& x);

    FASTDDS_EXPORTED_API inline void member_flags(
            const UnionDiscriminatorFlag& _member_flags)
    {
        m_member_flags = _member_flags;
    }

    FASTDDS_EXPORTED_API inline void member_flags(
            UnionDiscriminatorFlag&& _member_flags)
    {
        m_member_flags = std::move(_member_flags);
    }

    FASTDDS_EXPORTED_API inline const UnionDiscriminatorFlag& member_flags() const
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline UnionDiscriminatorFlag& member_flags()
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline void type_id(
            const TypeIdentifier& _type_id)
    {
        m_type_id = _type_id;
    }

    FASTDDS_EXPORTED_API inline void type_id(
            TypeIdentifier&& _type_id)
    {
        m_type_id = std::move(_type_id);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type_id() const
    {
        return m_type_id;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type_id()
    {
        return m_type_id;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonDiscriminatorMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteDiscriminatorMember();
    FASTDDS_EXPORTED_API ~CompleteDiscriminatorMember();
    FASTDDS_EXPORTED_API CompleteDiscriminatorMember(
            const CompleteDiscriminatorMember& x);
    FASTDDS_EXPORTED_API CompleteDiscriminatorMember(
            CompleteDiscriminatorMember&& x);
    FASTDDS_EXPORTED_API CompleteDiscriminatorMember& operator =(
            const CompleteDiscriminatorMember& x);
    FASTDDS_EXPORTED_API CompleteDiscriminatorMember& operator =(
            CompleteDiscriminatorMember&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonDiscriminatorMember& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonDiscriminatorMember&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonDiscriminatorMember& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonDiscriminatorMember& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            const AppliedBuiltinTypeAnnotations& _ann_builtin)
    {
        m_ann_builtin = _ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            AppliedBuiltinTypeAnnotations&& _ann_builtin)
    {
        m_ann_builtin = std::move(_ann_builtin);
    }

    FASTDDS_EXPORTED_API inline const AppliedBuiltinTypeAnnotations& ann_builtin() const
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline AppliedBuiltinTypeAnnotations& ann_builtin()
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom)
    {
        m_ann_custom = _ann_custom;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom)
    {
        m_ann_custom = std::move(_ann_custom);
    }

    FASTDDS_EXPORTED_API inline const AppliedAnnotationSeq& ann_custom() const
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API inline AppliedAnnotationSeq& ann_custom()
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteDiscriminatorMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalDiscriminatorMember();
    FASTDDS_EXPORTED_API ~MinimalDiscriminatorMember();
    FASTDDS_EXPORTED_API MinimalDiscriminatorMember(
            const MinimalDiscriminatorMember& x);
    FASTDDS_EXPORTED_API MinimalDiscriminatorMember(
            MinimalDiscriminatorMember&& x);
    FASTDDS_EXPORTED_API MinimalDiscriminatorMember& operator =(
            const MinimalDiscriminatorMember& x);
    FASTDDS_EXPORTED_API MinimalDiscriminatorMember& operator =(
            MinimalDiscriminatorMember&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonDiscriminatorMember& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonDiscriminatorMember&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonDiscriminatorMember& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonDiscriminatorMember& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalDiscriminatorMember& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteUnionHeader();
    FASTDDS_EXPORTED_API ~CompleteUnionHeader();
    FASTDDS_EXPORTED_API CompleteUnionHeader(
            const CompleteUnionHeader& x);
    FASTDDS_EXPORTED_API CompleteUnionHeader(
            CompleteUnionHeader&& x);
    FASTDDS_EXPORTED_API CompleteUnionHeader& operator =(
            const CompleteUnionHeader& x);
    FASTDDS_EXPORTED_API CompleteUnionHeader& operator =(
            CompleteUnionHeader&& x);

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteUnionHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalUnionHeader();
    FASTDDS_EXPORTED_API ~MinimalUnionHeader();
    FASTDDS_EXPORTED_API MinimalUnionHeader(
            const MinimalUnionHeader& x);
    FASTDDS_EXPORTED_API MinimalUnionHeader(
            MinimalUnionHeader&& x);
    FASTDDS_EXPORTED_API MinimalUnionHeader& operator =(
            const MinimalUnionHeader& x);
    FASTDDS_EXPORTED_API MinimalUnionHeader& operator =(
            MinimalUnionHeader&& x);

    FASTDDS_EXPORTED_API inline void detail(
            const MinimalTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            MinimalTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const MinimalTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline MinimalTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalUnionHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteUnionType();
    FASTDDS_EXPORTED_API ~CompleteUnionType();
    FASTDDS_EXPORTED_API CompleteUnionType(
            const CompleteUnionType& x);
    FASTDDS_EXPORTED_API CompleteUnionType(
            CompleteUnionType&& x);
    FASTDDS_EXPORTED_API CompleteUnionType& operator =(
            const CompleteUnionType& x);
    FASTDDS_EXPORTED_API CompleteUnionType& operator =(
            CompleteUnionType&& x);

    FASTDDS_EXPORTED_API inline void union_flags(
            const UnionTypeFlag& _union_flags)
    {
        m_union_flags = _union_flags;
    }

    FASTDDS_EXPORTED_API inline void union_flags(
            UnionTypeFlag&& _union_flags)
    {
        m_union_flags = std::move(_union_flags);
    }

    FASTDDS_EXPORTED_API inline const UnionTypeFlag& union_flags() const
    {
        return m_union_flags;
    }

    FASTDDS_EXPORTED_API inline UnionTypeFlag& union_flags()
    {
        return m_union_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteUnionHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteUnionHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteUnionHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteUnionHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void discriminator(
            const CompleteDiscriminatorMember& _discriminator)
    {
        m_discriminator = _discriminator;
    }

    FASTDDS_EXPORTED_API inline void discriminator(
            CompleteDiscriminatorMember&& _discriminator)
    {
        m_discriminator = std::move(_discriminator);
    }

    FASTDDS_EXPORTED_API inline const CompleteDiscriminatorMember& discriminator() const
    {
        return m_discriminator;
    }

    FASTDDS_EXPORTED_API inline CompleteDiscriminatorMember& discriminator()
    {
        return m_discriminator;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            const CompleteUnionMemberSeq& _member_seq)
    {
        m_member_seq = _member_seq;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            CompleteUnionMemberSeq&& _member_seq)
    {
        m_member_seq = std::move(_member_seq);
    }

    FASTDDS_EXPORTED_API inline const CompleteUnionMemberSeq& member_seq() const
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API inline CompleteUnionMemberSeq& member_seq()
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteUnionType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalUnionType();
    FASTDDS_EXPORTED_API ~MinimalUnionType();
    FASTDDS_EXPORTED_API MinimalUnionType(
            const MinimalUnionType& x);
    FASTDDS_EXPORTED_API MinimalUnionType(
            MinimalUnionType&& x);
    FASTDDS_EXPORTED_API MinimalUnionType& operator =(
            const MinimalUnionType& x);
    FASTDDS_EXPORTED_API MinimalUnionType& operator =(
            MinimalUnionType&& x);

    FASTDDS_EXPORTED_API inline void union_flags(
            const UnionTypeFlag& _union_flags)
    {
        m_union_flags = _union_flags;
    }

    FASTDDS_EXPORTED_API inline void union_flags(
            UnionTypeFlag&& _union_flags)
    {
        m_union_flags = std::move(_union_flags);
    }

    FASTDDS_EXPORTED_API inline const UnionTypeFlag& union_flags() const
    {
        return m_union_flags;
    }

    FASTDDS_EXPORTED_API inline UnionTypeFlag& union_flags()
    {
        return m_union_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalUnionHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalUnionHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalUnionHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalUnionHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void discriminator(
            const MinimalDiscriminatorMember& _discriminator)
    {
        m_discriminator = _discriminator;
    }

    FASTDDS_EXPORTED_API inline void discriminator(
            MinimalDiscriminatorMember&& _discriminator)
    {
        m_discriminator = std::move(_discriminator);
    }

    FASTDDS_EXPORTED_API inline const MinimalDiscriminatorMember& discriminator() const
    {
        return m_discriminator;
    }

    FASTDDS_EXPORTED_API inline MinimalDiscriminatorMember& discriminator()
    {
        return m_discriminator;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            const MinimalUnionMemberSeq& _member_seq)
    {
        m_member_seq = _member_seq;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            MinimalUnionMemberSeq&& _member_seq)
    {
        m_member_seq = std::move(_member_seq);
    }

    FASTDDS_EXPORTED_API inline const MinimalUnionMemberSeq& member_seq() const
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API inline MinimalUnionMemberSeq& member_seq()
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalUnionType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonAnnotationParameter();
    FASTDDS_EXPORTED_API ~CommonAnnotationParameter();
    FASTDDS_EXPORTED_API CommonAnnotationParameter(
            const CommonAnnotationParameter& x);
    FASTDDS_EXPORTED_API CommonAnnotationParameter(
            CommonAnnotationParameter&& x);
    FASTDDS_EXPORTED_API CommonAnnotationParameter& operator =(
            const CommonAnnotationParameter& x);
    FASTDDS_EXPORTED_API CommonAnnotationParameter& operator =(
            CommonAnnotationParameter&& x);

    FASTDDS_EXPORTED_API inline void member_flags(
            const AnnotationParameterFlag& _member_flags)
    {
        m_member_flags = _member_flags;
    }

    FASTDDS_EXPORTED_API inline void member_flags(
            AnnotationParameterFlag&& _member_flags)
    {
        m_member_flags = std::move(_member_flags);
    }

    FASTDDS_EXPORTED_API inline const AnnotationParameterFlag& member_flags() const
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline AnnotationParameterFlag& member_flags()
    {
        return m_member_flags;
    }

    FASTDDS_EXPORTED_API inline void member_type_id(
            const TypeIdentifier& _member_type_id)
    {
        m_member_type_id = _member_type_id;
    }

    FASTDDS_EXPORTED_API inline void member_type_id(
            TypeIdentifier&& _member_type_id)
    {
        m_member_type_id = std::move(_member_type_id);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& member_type_id() const
    {
        return m_member_type_id;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& member_type_id()
    {
        return m_member_type_id;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonAnnotationParameter& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteAnnotationParameter();
    FASTDDS_EXPORTED_API ~CompleteAnnotationParameter();
    FASTDDS_EXPORTED_API CompleteAnnotationParameter(
            const CompleteAnnotationParameter& x);
    FASTDDS_EXPORTED_API CompleteAnnotationParameter(
            CompleteAnnotationParameter&& x);
    FASTDDS_EXPORTED_API CompleteAnnotationParameter& operator =(
            const CompleteAnnotationParameter& x);
    FASTDDS_EXPORTED_API CompleteAnnotationParameter& operator =(
            CompleteAnnotationParameter&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonAnnotationParameter& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonAnnotationParameter&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonAnnotationParameter& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonAnnotationParameter& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void name(
            const MemberName& _name)
    {
        m_name = _name;
    }

    FASTDDS_EXPORTED_API inline void name(
            MemberName&& _name)
    {
        m_name = std::move(_name);
    }

    FASTDDS_EXPORTED_API inline const MemberName& name() const
    {
        return m_name;
    }

    FASTDDS_EXPORTED_API inline MemberName& name()
    {
        return m_name;
    }

    FASTDDS_EXPORTED_API inline void default_value(
            const AnnotationParameterValue& _default_value)
    {
        m_default_value = _default_value;
    }

    FASTDDS_EXPORTED_API inline void default_value(
            AnnotationParameterValue&& _default_value)
    {
        m_default_value = std::move(_default_value);
    }

    FASTDDS_EXPORTED_API inline const AnnotationParameterValue& default_value() const
    {
        return m_default_value;
    }

    FASTDDS_EXPORTED_API inline AnnotationParameterValue& default_value()
    {
        return m_default_value;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteAnnotationParameter& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalAnnotationParameter();
    FASTDDS_EXPORTED_API ~MinimalAnnotationParameter();
    FASTDDS_EXPORTED_API MinimalAnnotationParameter(
            const MinimalAnnotationParameter& x);
    FASTDDS_EXPORTED_API MinimalAnnotationParameter(
            MinimalAnnotationParameter&& x);
    FASTDDS_EXPORTED_API MinimalAnnotationParameter& operator =(
            const MinimalAnnotationParameter& x);
    FASTDDS_EXPORTED_API MinimalAnnotationParameter& operator =(
            MinimalAnnotationParameter&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonAnnotationParameter& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonAnnotationParameter&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonAnnotationParameter& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonAnnotationParameter& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void name(
            const MemberName& _name)
    {
        m_name = _name;
    }

    FASTDDS_EXPORTED_API inline void name(
            MemberName&& _name)
    {
        m_name = std::move(_name);
    }

    FASTDDS_EXPORTED_API inline const MemberName& name() const
    {
        return m_name;
    }

    FASTDDS_EXPORTED_API inline MemberName& name()
    {
        return m_name;
    }

    FASTDDS_EXPORTED_API inline void default_value(
            const AnnotationParameterValue& _default_value)
    {
        m_default_value = _default_value;
    }

    FASTDDS_EXPORTED_API inline void default_value(
            AnnotationParameterValue&& _default_value)
    {
        m_default_value = std::move(_default_value);
    }

    FASTDDS_EXPORTED_API inline const AnnotationParameterValue& default_value() const
    {
        return m_default_value;
    }

    FASTDDS_EXPORTED_API inline AnnotationParameterValue& default_value()
    {
        return m_default_value;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalAnnotationParameter& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteAnnotationHeader();
    FASTDDS_EXPORTED_API ~CompleteAnnotationHeader();
    FASTDDS_EXPORTED_API CompleteAnnotationHeader(
            const CompleteAnnotationHeader& x);
    FASTDDS_EXPORTED_API CompleteAnnotationHeader(
            CompleteAnnotationHeader&& x);
    FASTDDS_EXPORTED_API CompleteAnnotationHeader& operator =(
            const CompleteAnnotationHeader& x);
    FASTDDS_EXPORTED_API CompleteAnnotationHeader& operator =(
            CompleteAnnotationHeader&& x);

    FASTDDS_EXPORTED_API inline void annotation_name(
            const QualifiedTypeName& _annotation_name)
    {
        m_annotation_name = _annotation_name;
    }

    FASTDDS_EXPORTED_API inline void annotation_name(
            QualifiedTypeName&& _annotation_name)
    {
        m_annotation_name = std::move(_annotation_name);
    }

    FASTDDS_EXPORTED_API inline const QualifiedTypeName& annotation_name() const
    {
        return m_annotation_name;
    }

    FASTDDS_EXPORTED_API inline QualifiedTypeName& annotation_name()
    {
        return m_annotation_name;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteAnnotationHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalAnnotationHeader();
    FASTDDS_EXPORTED_API ~MinimalAnnotationHeader();
    FASTDDS_EXPORTED_API MinimalAnnotationHeader(
            const MinimalAnnotationHeader& x);
    FASTDDS_EXPORTED_API MinimalAnnotationHeader(
            MinimalAnnotationHeader&& x);
    FASTDDS_EXPORTED_API MinimalAnnotationHeader& operator =(
            const MinimalAnnotationHeader& x);
    FASTDDS_EXPORTED_API MinimalAnnotationHeader& operator =(
            MinimalAnnotationHeader&& x);

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalAnnotationHeader&) const
    {
        return true;
    }

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteAnnotationType();
    FASTDDS_EXPORTED_API ~CompleteAnnotationType();
    FASTDDS_EXPORTED_API CompleteAnnotationType(
            const CompleteAnnotationType& x);
    FASTDDS_EXPORTED_API CompleteAnnotationType(
            CompleteAnnotationType&& x);
    FASTDDS_EXPORTED_API CompleteAnnotationType& operator =(
            const CompleteAnnotationType& x);
    FASTDDS_EXPORTED_API CompleteAnnotationType& operator =(
            CompleteAnnotationType&& x);

    FASTDDS_EXPORTED_API inline void annotation_flag(
            const AnnotationTypeFlag& _annotation_flag)
    {
        m_annotation_flag = _annotation_flag;
    }

    FASTDDS_EXPORTED_API inline void annotation_flag(
            AnnotationTypeFlag&& _annotation_flag)
    {
        m_annotation_flag = std::move(_annotation_flag);
    }

    FASTDDS_EXPORTED_API inline const AnnotationTypeFlag& annotation_flag() const
    {
        return m_annotation_flag;
    }

    FASTDDS_EXPORTED_API inline AnnotationTypeFlag& annotation_flag()
    {
        return m_annotation_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteAnnotationHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteAnnotationHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteAnnotationHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteAnnotationHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            const CompleteAnnotationParameterSeq& _member_seq)
    {
        m_member_seq = _member_seq;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            CompleteAnnotationParameterSeq&& _member_seq)
    {
        m_member_seq = std::move(_member_seq);
    }

    FASTDDS_EXPORTED_API inline const CompleteAnnotationParameterSeq& member_seq() const
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API inline CompleteAnnotationParameterSeq& member_seq()
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteAnnotationType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalAnnotationType();
    FASTDDS_EXPORTED_API ~MinimalAnnotationType();
    FASTDDS_EXPORTED_API MinimalAnnotationType(
            const MinimalAnnotationType& x);
    FASTDDS_EXPORTED_API MinimalAnnotationType(
            MinimalAnnotationType&& x);
    FASTDDS_EXPORTED_API MinimalAnnotationType& operator =(
            const MinimalAnnotationType& x);
    FASTDDS_EXPORTED_API MinimalAnnotationType& operator =(
            MinimalAnnotationType&& x);

    FASTDDS_EXPORTED_API inline void annotation_flag(
            const AnnotationTypeFlag& _annotation_flag)
    {
        m_annotation_flag = _annotation_flag;
    }

    FASTDDS_EXPORTED_API inline void annotation_flag(
            AnnotationTypeFlag&& _annotation_flag)
    {
        m_annotation_flag = std::move(_annotation_flag);
    }

    FASTDDS_EXPORTED_API inline const AnnotationTypeFlag& annotation_flag() const
    {
        return m_annotation_flag;
    }

    FASTDDS_EXPORTED_API inline AnnotationTypeFlag& annotation_flag()
    {
        return m_annotation_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalAnnotationHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalAnnotationHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalAnnotationHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalAnnotationHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            const MinimalAnnotationParameterSeq& _member_seq)
    {
        m_member_seq = _member_seq;
    }

    FASTDDS_EXPORTED_API inline void member_seq(
            MinimalAnnotationParameterSeq&& _member_seq)
    {
        m_member_seq = std::move(_member_seq);
    }

    FASTDDS_EXPORTED_API inline const MinimalAnnotationParameterSeq& member_seq() const
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API inline MinimalAnnotationParameterSeq& member_seq()
    {
        return m_member_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalAnnotationType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonAliasBody();
    FASTDDS_EXPORTED_API ~CommonAliasBody();
    FASTDDS_EXPORTED_API CommonAliasBody(
            const CommonAliasBody& x);
    FASTDDS_EXPORTED_API CommonAliasBody(
            CommonAliasBody&& x);
    FASTDDS_EXPORTED_API CommonAliasBody& operator =(
            const CommonAliasBody& x);
    FASTDDS_EXPORTED_API CommonAliasBody& operator =(
            CommonAliasBody&& x);

    FASTDDS_EXPORTED_API inline void related_flags(
            const AliasMemberFlag& _related_flags)
    {
        m_related_flags = _related_flags;
    }

    FASTDDS_EXPORTED_API inline void related_flags(
            AliasMemberFlag&& _related_flags)
    {
        m_related_flags = std::move(_related_flags);
    }

    FASTDDS_EXPORTED_API inline const AliasMemberFlag& related_flags() const
    {
        return m_related_flags;
    }

    FASTDDS_EXPORTED_API inline AliasMemberFlag& related_flags()
    {
        return m_related_flags;
    }

    FASTDDS_EXPORTED_API inline void related_type(
            const TypeIdentifier& _related_type)
    {
        m_related_type = _related_type;
    }

    FASTDDS_EXPORTED_API inline void related_type(
            TypeIdentifier&& _related_type)
    {
        m_related_type = std::move(_related_type);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& related_type() const
    {
        return m_related_type;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& related_type()
    {
        return m_related_type;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonAliasBody& other) const;

    //    FASTDDS_EXPORTED_API bool consistent(const CommonAliasBody &x,
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

    FASTDDS_EXPORTED_API CompleteAliasBody();
    FASTDDS_EXPORTED_API ~CompleteAliasBody();
    FASTDDS_EXPORTED_API CompleteAliasBody(
            const CompleteAliasBody& x);
    FASTDDS_EXPORTED_API CompleteAliasBody(
            CompleteAliasBody&& x);
    FASTDDS_EXPORTED_API CompleteAliasBody& operator =(
            const CompleteAliasBody& x);
    FASTDDS_EXPORTED_API CompleteAliasBody& operator =(
            CompleteAliasBody&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonAliasBody& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonAliasBody&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonAliasBody& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonAliasBody& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            const AppliedBuiltinMemberAnnotations& _ann_builtin)
    {
        m_ann_builtin = _ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            AppliedBuiltinMemberAnnotations&& _ann_builtin)
    {
        m_ann_builtin = std::move(_ann_builtin);
    }

    FASTDDS_EXPORTED_API inline const AppliedBuiltinMemberAnnotations& ann_builtin() const
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline AppliedBuiltinMemberAnnotations& ann_builtin()
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom)
    {
        m_ann_custom = _ann_custom;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom)
    {
        m_ann_custom = std::move(_ann_custom);
    }

    FASTDDS_EXPORTED_API inline const AppliedAnnotationSeq& ann_custom() const
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API inline AppliedAnnotationSeq& ann_custom()
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteAliasBody& other) const;

    //    FASTDDS_EXPORTED_API bool consistent(const CompleteAliasBody &x,
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

    FASTDDS_EXPORTED_API MinimalAliasBody();
    FASTDDS_EXPORTED_API ~MinimalAliasBody();
    FASTDDS_EXPORTED_API MinimalAliasBody(
            const MinimalAliasBody& x);
    FASTDDS_EXPORTED_API MinimalAliasBody(
            MinimalAliasBody&& x);
    FASTDDS_EXPORTED_API MinimalAliasBody& operator =(
            const MinimalAliasBody& x);
    FASTDDS_EXPORTED_API MinimalAliasBody& operator =(
            MinimalAliasBody&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonAliasBody& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonAliasBody&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonAliasBody& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonAliasBody& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalAliasBody& other) const;

    //    FASTDDS_EXPORTED_API bool consistent(const MinimalAliasBody &x,
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

    FASTDDS_EXPORTED_API CompleteAliasHeader();
    FASTDDS_EXPORTED_API ~CompleteAliasHeader();
    FASTDDS_EXPORTED_API CompleteAliasHeader(
            const CompleteAliasHeader& x);
    FASTDDS_EXPORTED_API CompleteAliasHeader(
            CompleteAliasHeader&& x);
    FASTDDS_EXPORTED_API CompleteAliasHeader& operator =(
            const CompleteAliasHeader& x);
    FASTDDS_EXPORTED_API CompleteAliasHeader& operator =(
            CompleteAliasHeader&& x);

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteAliasHeader& other) const;

    //    FASTDDS_EXPORTED_API bool consistent(const CompleteAliasHeader &x,
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

    FASTDDS_EXPORTED_API MinimalAliasHeader();
    FASTDDS_EXPORTED_API ~MinimalAliasHeader();
    FASTDDS_EXPORTED_API MinimalAliasHeader(
            const MinimalAliasHeader& x);
    FASTDDS_EXPORTED_API MinimalAliasHeader(
            MinimalAliasHeader&& x);
    FASTDDS_EXPORTED_API MinimalAliasHeader& operator =(
            const MinimalAliasHeader& x);
    FASTDDS_EXPORTED_API MinimalAliasHeader& operator =(
            MinimalAliasHeader&& x);

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalAliasHeader&) const
    {
        return true;
    }

    //    FASTDDS_EXPORTED_API bool consistent(const MinimalAliasHeader &x,
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

    FASTDDS_EXPORTED_API CompleteAliasType();
    FASTDDS_EXPORTED_API ~CompleteAliasType();
    FASTDDS_EXPORTED_API CompleteAliasType(
            const CompleteAliasType& x);
    FASTDDS_EXPORTED_API CompleteAliasType(
            CompleteAliasType&& x);
    FASTDDS_EXPORTED_API CompleteAliasType& operator =(
            const CompleteAliasType& x);
    FASTDDS_EXPORTED_API CompleteAliasType& operator =(
            CompleteAliasType&& x);

    FASTDDS_EXPORTED_API inline void alias_flags(
            const AliasTypeFlag& _alias_flags)
    {
        m_alias_flags = _alias_flags;
    }

    FASTDDS_EXPORTED_API inline void alias_flags(
            AliasTypeFlag&& _alias_flags)
    {
        m_alias_flags = std::move(_alias_flags);
    }

    FASTDDS_EXPORTED_API inline const AliasTypeFlag& alias_flags() const
    {
        return m_alias_flags;
    }

    FASTDDS_EXPORTED_API inline AliasTypeFlag& alias_flags()
    {
        return m_alias_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteAliasHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteAliasHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteAliasHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteAliasHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void body(
            const CompleteAliasBody& _body)
    {
        m_body = _body;
    }

    FASTDDS_EXPORTED_API inline void body(
            CompleteAliasBody&& _body)
    {
        m_body = std::move(_body);
    }

    FASTDDS_EXPORTED_API inline const CompleteAliasBody& body() const
    {
        return m_body;
    }

    FASTDDS_EXPORTED_API inline CompleteAliasBody& body()
    {
        return m_body;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteAliasType& other) const;

    //    FASTDDS_EXPORTED_API bool consistent(const CompleteAliasType &x,
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

    FASTDDS_EXPORTED_API MinimalAliasType();
    FASTDDS_EXPORTED_API ~MinimalAliasType();
    FASTDDS_EXPORTED_API MinimalAliasType(
            const MinimalAliasType& x);
    FASTDDS_EXPORTED_API MinimalAliasType(
            MinimalAliasType&& x);
    FASTDDS_EXPORTED_API MinimalAliasType& operator =(
            const MinimalAliasType& x);
    FASTDDS_EXPORTED_API MinimalAliasType& operator =(
            MinimalAliasType&& x);

    FASTDDS_EXPORTED_API inline void alias_flags(
            const AliasTypeFlag& _alias_flags)
    {
        m_alias_flags = _alias_flags;
    }

    FASTDDS_EXPORTED_API inline void alias_flags(
            AliasTypeFlag&& _alias_flags)
    {
        m_alias_flags = std::move(_alias_flags);
    }

    FASTDDS_EXPORTED_API inline const AliasTypeFlag& alias_flags() const
    {
        return m_alias_flags;
    }

    FASTDDS_EXPORTED_API inline AliasTypeFlag& alias_flags()
    {
        return m_alias_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalAliasHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalAliasHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalAliasHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalAliasHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void body(
            const MinimalAliasBody& _body)
    {
        m_body = _body;
    }

    FASTDDS_EXPORTED_API inline void body(
            MinimalAliasBody&& _body)
    {
        m_body = std::move(_body);
    }

    FASTDDS_EXPORTED_API inline const MinimalAliasBody& body() const
    {
        return m_body;
    }

    FASTDDS_EXPORTED_API inline MinimalAliasBody& body()
    {
        return m_body;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalAliasType& other) const;

    //    FASTDDS_EXPORTED_API bool consistent(const MinimalAliasType &x,
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

    FASTDDS_EXPORTED_API CompleteElementDetail();
    FASTDDS_EXPORTED_API ~CompleteElementDetail();
    FASTDDS_EXPORTED_API CompleteElementDetail(
            const CompleteElementDetail& x);
    FASTDDS_EXPORTED_API CompleteElementDetail(
            CompleteElementDetail&& x);
    FASTDDS_EXPORTED_API CompleteElementDetail& operator =(
            const CompleteElementDetail& x);
    FASTDDS_EXPORTED_API CompleteElementDetail& operator =(
            CompleteElementDetail&& x);

    FASTDDS_EXPORTED_API inline void ann_builtin(
            const AppliedBuiltinMemberAnnotations& _ann_builtin)
    {
        m_ann_builtin = _ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_builtin(
            AppliedBuiltinMemberAnnotations&& _ann_builtin)
    {
        m_ann_builtin = std::move(_ann_builtin);
    }

    FASTDDS_EXPORTED_API inline const AppliedBuiltinMemberAnnotations& ann_builtin() const
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline AppliedBuiltinMemberAnnotations& ann_builtin()
    {
        return m_ann_builtin;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            const AppliedAnnotationSeq& _ann_custom)
    {
        m_ann_custom = _ann_custom;
    }

    FASTDDS_EXPORTED_API inline void ann_custom(
            AppliedAnnotationSeq&& _ann_custom)
    {
        m_ann_custom = std::move(_ann_custom);
    }

    FASTDDS_EXPORTED_API inline const AppliedAnnotationSeq& ann_custom() const
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API inline AppliedAnnotationSeq& ann_custom()
    {
        return m_ann_custom;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteElementDetail& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonCollectionElement();
    FASTDDS_EXPORTED_API ~CommonCollectionElement();
    FASTDDS_EXPORTED_API CommonCollectionElement(
            const CommonCollectionElement& x);
    FASTDDS_EXPORTED_API CommonCollectionElement(
            CommonCollectionElement&& x);
    FASTDDS_EXPORTED_API CommonCollectionElement& operator =(
            const CommonCollectionElement& x);
    FASTDDS_EXPORTED_API CommonCollectionElement& operator =(
            CommonCollectionElement&& x);

    FASTDDS_EXPORTED_API inline void element_flags(
            const CollectionElementFlag& _element_flags)
    {
        m_element_flags = _element_flags;
    }

    FASTDDS_EXPORTED_API inline void element_flags(
            CollectionElementFlag&& _element_flags)
    {
        m_element_flags = std::move(_element_flags);
    }

    FASTDDS_EXPORTED_API inline const CollectionElementFlag& element_flags() const
    {
        return m_element_flags;
    }

    FASTDDS_EXPORTED_API inline CollectionElementFlag& element_flags()
    {
        return m_element_flags;
    }

    FASTDDS_EXPORTED_API inline void type(
            const TypeIdentifier& _type)
    {
        m_type = _type;
    }

    FASTDDS_EXPORTED_API inline void type(
            TypeIdentifier&& _type)
    {
        m_type = std::move(_type);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type() const
    {
        return m_type;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type()
    {
        return m_type;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonCollectionElement& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteCollectionElement();
    FASTDDS_EXPORTED_API ~CompleteCollectionElement();
    FASTDDS_EXPORTED_API CompleteCollectionElement(
            const CompleteCollectionElement& x);
    FASTDDS_EXPORTED_API CompleteCollectionElement(
            CompleteCollectionElement&& x);
    FASTDDS_EXPORTED_API CompleteCollectionElement& operator =(
            const CompleteCollectionElement& x);
    FASTDDS_EXPORTED_API CompleteCollectionElement& operator =(
            CompleteCollectionElement&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonCollectionElement& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonCollectionElement&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonCollectionElement& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonCollectionElement& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteElementDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteElementDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteElementDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteElementDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteCollectionElement& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalCollectionElement();
    FASTDDS_EXPORTED_API ~MinimalCollectionElement();
    FASTDDS_EXPORTED_API MinimalCollectionElement(
            const MinimalCollectionElement& x);
    FASTDDS_EXPORTED_API MinimalCollectionElement(
            MinimalCollectionElement&& x);
    FASTDDS_EXPORTED_API MinimalCollectionElement& operator =(
            const MinimalCollectionElement& x);
    FASTDDS_EXPORTED_API MinimalCollectionElement& operator =(
            MinimalCollectionElement&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonCollectionElement& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonCollectionElement&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonCollectionElement& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonCollectionElement& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalCollectionElement& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonCollectionHeader();
    FASTDDS_EXPORTED_API ~CommonCollectionHeader();
    FASTDDS_EXPORTED_API CommonCollectionHeader(
            const CommonCollectionHeader& x);
    FASTDDS_EXPORTED_API CommonCollectionHeader(
            CommonCollectionHeader&& x);
    FASTDDS_EXPORTED_API CommonCollectionHeader& operator =(
            const CommonCollectionHeader& x);
    FASTDDS_EXPORTED_API CommonCollectionHeader& operator =(
            CommonCollectionHeader&& x);

    FASTDDS_EXPORTED_API inline void bound(
            const LBound& _bound)
    {
        m_bound = _bound;
    }

    FASTDDS_EXPORTED_API inline void bound(
            LBound&& _bound)
    {
        m_bound = std::move(_bound);
    }

    FASTDDS_EXPORTED_API inline const LBound& bound() const
    {
        return m_bound;
    }

    FASTDDS_EXPORTED_API inline LBound& bound()
    {
        return m_bound;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonCollectionHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteCollectionHeader();
    FASTDDS_EXPORTED_API ~CompleteCollectionHeader();
    FASTDDS_EXPORTED_API CompleteCollectionHeader(
            const CompleteCollectionHeader& x);
    FASTDDS_EXPORTED_API CompleteCollectionHeader(
            CompleteCollectionHeader&& x);
    FASTDDS_EXPORTED_API CompleteCollectionHeader& operator =(
            const CompleteCollectionHeader& x);
    FASTDDS_EXPORTED_API CompleteCollectionHeader& operator =(
            CompleteCollectionHeader&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonCollectionHeader& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonCollectionHeader&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonCollectionHeader& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonCollectionHeader& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteCollectionHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalCollectionHeader();
    FASTDDS_EXPORTED_API ~MinimalCollectionHeader();
    FASTDDS_EXPORTED_API MinimalCollectionHeader(
            const MinimalCollectionHeader& x);
    FASTDDS_EXPORTED_API MinimalCollectionHeader(
            MinimalCollectionHeader&& x);
    FASTDDS_EXPORTED_API MinimalCollectionHeader& operator =(
            const MinimalCollectionHeader& x);
    FASTDDS_EXPORTED_API MinimalCollectionHeader& operator =(
            MinimalCollectionHeader&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonCollectionHeader& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonCollectionHeader&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonCollectionHeader& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonCollectionHeader& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalCollectionHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteSequenceType();
    FASTDDS_EXPORTED_API ~CompleteSequenceType();
    FASTDDS_EXPORTED_API CompleteSequenceType(
            const CompleteSequenceType& x);
    FASTDDS_EXPORTED_API CompleteSequenceType(
            CompleteSequenceType&& x);
    FASTDDS_EXPORTED_API CompleteSequenceType& operator =(
            const CompleteSequenceType& x);
    FASTDDS_EXPORTED_API CompleteSequenceType& operator =(
            CompleteSequenceType&& x);

    FASTDDS_EXPORTED_API inline void collection_flag(
            const CollectionTypeFlag& _collection_flag)
    {
        m_collection_flag = _collection_flag;
    }

    FASTDDS_EXPORTED_API inline void collection_flag(
            CollectionTypeFlag&& _collection_flag)
    {
        m_collection_flag = std::move(_collection_flag);
    }

    FASTDDS_EXPORTED_API inline const CollectionTypeFlag& collection_flag() const
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline CollectionTypeFlag& collection_flag()
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteCollectionHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteCollectionHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteCollectionHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteCollectionHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void element(
            const CompleteCollectionElement& _element)
    {
        m_element = _element;
    }

    FASTDDS_EXPORTED_API inline void element(
            CompleteCollectionElement&& _element)
    {
        m_element = std::move(_element);
    }

    FASTDDS_EXPORTED_API inline const CompleteCollectionElement& element() const
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API inline CompleteCollectionElement& element()
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteSequenceType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalSequenceType();
    FASTDDS_EXPORTED_API ~MinimalSequenceType();
    FASTDDS_EXPORTED_API MinimalSequenceType(
            const MinimalSequenceType& x);
    FASTDDS_EXPORTED_API MinimalSequenceType(
            MinimalSequenceType&& x);
    FASTDDS_EXPORTED_API MinimalSequenceType& operator =(
            const MinimalSequenceType& x);
    FASTDDS_EXPORTED_API MinimalSequenceType& operator =(
            MinimalSequenceType&& x);

    FASTDDS_EXPORTED_API inline void collection_flag(
            const CollectionTypeFlag& _collection_flag)
    {
        m_collection_flag = _collection_flag;
    }

    FASTDDS_EXPORTED_API inline void collection_flag(
            CollectionTypeFlag&& _collection_flag)
    {
        m_collection_flag = std::move(_collection_flag);
    }

    FASTDDS_EXPORTED_API inline const CollectionTypeFlag& collection_flag() const
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline CollectionTypeFlag& collection_flag()
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalCollectionHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalCollectionHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalCollectionHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalCollectionHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void element(
            const MinimalCollectionElement& _element)
    {
        m_element = _element;
    }

    FASTDDS_EXPORTED_API inline void element(
            MinimalCollectionElement&& _element)
    {
        m_element = std::move(_element);
    }

    FASTDDS_EXPORTED_API inline const MinimalCollectionElement& element() const
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API inline MinimalCollectionElement& element()
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalSequenceType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonArrayHeader();
    FASTDDS_EXPORTED_API ~CommonArrayHeader();
    FASTDDS_EXPORTED_API CommonArrayHeader(
            const CommonArrayHeader& x);
    FASTDDS_EXPORTED_API CommonArrayHeader(
            CommonArrayHeader&& x);
    FASTDDS_EXPORTED_API CommonArrayHeader& operator =(
            const CommonArrayHeader& x);
    FASTDDS_EXPORTED_API CommonArrayHeader& operator =(
            CommonArrayHeader&& x);

    FASTDDS_EXPORTED_API inline void bound_seq(
            const LBoundSeq& _bound_seq)
    {
        m_bound_seq = _bound_seq;
    }

    FASTDDS_EXPORTED_API inline void bound_seq(
            LBoundSeq&& _bound_seq)
    {
        m_bound_seq = std::move(_bound_seq);
    }

    FASTDDS_EXPORTED_API inline const LBoundSeq& bound_seq() const
    {
        return m_bound_seq;
    }

    FASTDDS_EXPORTED_API inline LBoundSeq& bound_seq()
    {
        return m_bound_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonArrayHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteArrayHeader();
    FASTDDS_EXPORTED_API ~CompleteArrayHeader();
    FASTDDS_EXPORTED_API CompleteArrayHeader(
            const CompleteArrayHeader& x);
    FASTDDS_EXPORTED_API CompleteArrayHeader(
            CompleteArrayHeader&& x);
    FASTDDS_EXPORTED_API CompleteArrayHeader& operator =(
            const CompleteArrayHeader& x);
    FASTDDS_EXPORTED_API CompleteArrayHeader& operator =(
            CompleteArrayHeader&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonArrayHeader& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonArrayHeader&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonArrayHeader& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonArrayHeader& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteArrayHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalArrayHeader();
    FASTDDS_EXPORTED_API ~MinimalArrayHeader();
    FASTDDS_EXPORTED_API MinimalArrayHeader(
            const MinimalArrayHeader& x);
    FASTDDS_EXPORTED_API MinimalArrayHeader(
            MinimalArrayHeader&& x);
    FASTDDS_EXPORTED_API MinimalArrayHeader& operator =(
            const MinimalArrayHeader& x);
    FASTDDS_EXPORTED_API MinimalArrayHeader& operator =(
            MinimalArrayHeader&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonArrayHeader& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonArrayHeader&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonArrayHeader& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonArrayHeader& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalArrayHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteArrayType();
    FASTDDS_EXPORTED_API ~CompleteArrayType();
    FASTDDS_EXPORTED_API CompleteArrayType(
            const CompleteArrayType& x);
    FASTDDS_EXPORTED_API CompleteArrayType(
            CompleteArrayType&& x);
    FASTDDS_EXPORTED_API CompleteArrayType& operator =(
            const CompleteArrayType& x);
    FASTDDS_EXPORTED_API CompleteArrayType& operator =(
            CompleteArrayType&& x);

    FASTDDS_EXPORTED_API inline void collection_flag(
            const CollectionTypeFlag& _collection_flag)
    {
        m_collection_flag = _collection_flag;
    }

    FASTDDS_EXPORTED_API inline void collection_flag(
            CollectionTypeFlag&& _collection_flag)
    {
        m_collection_flag = std::move(_collection_flag);
    }

    FASTDDS_EXPORTED_API inline const CollectionTypeFlag& collection_flag() const
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline CollectionTypeFlag& collection_flag()
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteArrayHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteArrayHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteArrayHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteArrayHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void element(
            const CompleteCollectionElement& _element)
    {
        m_element = _element;
    }

    FASTDDS_EXPORTED_API inline void element(
            CompleteCollectionElement&& _element)
    {
        m_element = std::move(_element);
    }

    FASTDDS_EXPORTED_API inline const CompleteCollectionElement& element() const
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API inline CompleteCollectionElement& element()
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteArrayType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalArrayType();
    FASTDDS_EXPORTED_API ~MinimalArrayType();
    FASTDDS_EXPORTED_API MinimalArrayType(
            const MinimalArrayType& x);
    FASTDDS_EXPORTED_API MinimalArrayType(
            MinimalArrayType&& x);
    FASTDDS_EXPORTED_API MinimalArrayType& operator =(
            const MinimalArrayType& x);
    FASTDDS_EXPORTED_API MinimalArrayType& operator =(
            MinimalArrayType&& x);

    FASTDDS_EXPORTED_API inline void collection_flag(
            const CollectionTypeFlag& _collection_flag)
    {
        m_collection_flag = _collection_flag;
    }

    FASTDDS_EXPORTED_API inline void collection_flag(
            CollectionTypeFlag&& _collection_flag)
    {
        m_collection_flag = std::move(_collection_flag);
    }

    FASTDDS_EXPORTED_API inline const CollectionTypeFlag& collection_flag() const
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline CollectionTypeFlag& collection_flag()
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalArrayHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalArrayHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalArrayHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalArrayHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void element(
            const MinimalCollectionElement& _element)
    {
        m_element = _element;
    }

    FASTDDS_EXPORTED_API inline void element(
            MinimalCollectionElement&& _element)
    {
        m_element = std::move(_element);
    }

    FASTDDS_EXPORTED_API inline const MinimalCollectionElement& element() const
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API inline MinimalCollectionElement& element()
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalArrayType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteMapType();
    FASTDDS_EXPORTED_API ~CompleteMapType();
    FASTDDS_EXPORTED_API CompleteMapType(
            const CompleteMapType& x);
    FASTDDS_EXPORTED_API CompleteMapType(
            CompleteMapType&& x);
    FASTDDS_EXPORTED_API CompleteMapType& operator =(
            const CompleteMapType& x);
    FASTDDS_EXPORTED_API CompleteMapType& operator =(
            CompleteMapType&& x);

    FASTDDS_EXPORTED_API inline void collection_flag(
            const CollectionTypeFlag& _collection_flag)
    {
        m_collection_flag = _collection_flag;
    }

    FASTDDS_EXPORTED_API inline void collection_flag(
            CollectionTypeFlag&& _collection_flag)
    {
        m_collection_flag = std::move(_collection_flag);
    }

    FASTDDS_EXPORTED_API inline const CollectionTypeFlag& collection_flag() const
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline CollectionTypeFlag& collection_flag()
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteCollectionHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteCollectionHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteCollectionHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteCollectionHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void key(
            const CompleteCollectionElement& _key)
    {
        m_key = _key;
    }

    FASTDDS_EXPORTED_API inline void key(
            CompleteCollectionElement&& _key)
    {
        m_key = std::move(_key);
    }

    FASTDDS_EXPORTED_API inline const CompleteCollectionElement& key() const
    {
        return m_key;
    }

    FASTDDS_EXPORTED_API inline CompleteCollectionElement& key()
    {
        return m_key;
    }

    FASTDDS_EXPORTED_API inline void element(
            const CompleteCollectionElement& _element)
    {
        m_element = _element;
    }

    FASTDDS_EXPORTED_API inline void element(
            CompleteCollectionElement&& _element)
    {
        m_element = std::move(_element);
    }

    FASTDDS_EXPORTED_API inline const CompleteCollectionElement& element() const
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API inline CompleteCollectionElement& element()
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteMapType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalMapType();
    FASTDDS_EXPORTED_API ~MinimalMapType();
    FASTDDS_EXPORTED_API MinimalMapType(
            const MinimalMapType& x);
    FASTDDS_EXPORTED_API MinimalMapType(
            MinimalMapType&& x);
    FASTDDS_EXPORTED_API MinimalMapType& operator =(
            const MinimalMapType& x);
    FASTDDS_EXPORTED_API MinimalMapType& operator =(
            MinimalMapType&& x);

    FASTDDS_EXPORTED_API inline void collection_flag(
            const CollectionTypeFlag& _collection_flag)
    {
        m_collection_flag = _collection_flag;
    }

    FASTDDS_EXPORTED_API inline void collection_flag(
            CollectionTypeFlag&& _collection_flag)
    {
        m_collection_flag = std::move(_collection_flag);
    }

    FASTDDS_EXPORTED_API inline const CollectionTypeFlag& collection_flag() const
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline CollectionTypeFlag& collection_flag()
    {
        return m_collection_flag;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalCollectionHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalCollectionHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalCollectionHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalCollectionHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void key(
            const MinimalCollectionElement& _key)
    {
        m_key = _key;
    }

    FASTDDS_EXPORTED_API inline void key(
            MinimalCollectionElement&& _key)
    {
        m_key = std::move(_key);
    }

    FASTDDS_EXPORTED_API inline const MinimalCollectionElement& key() const
    {
        return m_key;
    }

    FASTDDS_EXPORTED_API inline MinimalCollectionElement& key()
    {
        return m_key;
    }

    FASTDDS_EXPORTED_API inline void element(
            const MinimalCollectionElement& _element)
    {
        m_element = _element;
    }

    FASTDDS_EXPORTED_API inline void element(
            MinimalCollectionElement&& _element)
    {
        m_element = std::move(_element);
    }

    FASTDDS_EXPORTED_API inline const MinimalCollectionElement& element() const
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API inline MinimalCollectionElement& element()
    {
        return m_element;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalMapType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonEnumeratedLiteral();
    FASTDDS_EXPORTED_API ~CommonEnumeratedLiteral();
    FASTDDS_EXPORTED_API CommonEnumeratedLiteral(
            const CommonEnumeratedLiteral& x);
    FASTDDS_EXPORTED_API CommonEnumeratedLiteral(
            CommonEnumeratedLiteral&& x);
    FASTDDS_EXPORTED_API CommonEnumeratedLiteral& operator =(
            const CommonEnumeratedLiteral& x);
    FASTDDS_EXPORTED_API CommonEnumeratedLiteral& operator =(
            CommonEnumeratedLiteral&& x);

    FASTDDS_EXPORTED_API inline void value(
            const int32_t& _value)
    {
        m_value = _value;
    }

    FASTDDS_EXPORTED_API inline void value(
            int32_t&& _value)
    {
        m_value = std::move(_value);
    }

    FASTDDS_EXPORTED_API inline const int32_t& value() const
    {
        return m_value;
    }

    FASTDDS_EXPORTED_API inline int32_t& value()
    {
        return m_value;
    }

    FASTDDS_EXPORTED_API inline void flags(
            const EnumeratedLiteralFlag& _flags)
    {
        m_flags = _flags;
    }

    FASTDDS_EXPORTED_API inline void flags(
            EnumeratedLiteralFlag&& _flags)
    {
        m_flags = std::move(_flags);
    }

    FASTDDS_EXPORTED_API inline const EnumeratedLiteralFlag& flags() const
    {
        return m_flags;
    }

    FASTDDS_EXPORTED_API inline EnumeratedLiteralFlag& flags()
    {
        return m_flags;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonEnumeratedLiteral& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteEnumeratedLiteral();
    FASTDDS_EXPORTED_API ~CompleteEnumeratedLiteral();
    FASTDDS_EXPORTED_API CompleteEnumeratedLiteral(
            const CompleteEnumeratedLiteral& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedLiteral(
            CompleteEnumeratedLiteral&& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedLiteral& operator =(
            const CompleteEnumeratedLiteral& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedLiteral& operator =(
            CompleteEnumeratedLiteral&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonEnumeratedLiteral& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonEnumeratedLiteral&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonEnumeratedLiteral& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonEnumeratedLiteral& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteEnumeratedLiteral& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalEnumeratedLiteral();
    FASTDDS_EXPORTED_API ~MinimalEnumeratedLiteral();
    FASTDDS_EXPORTED_API MinimalEnumeratedLiteral(
            const MinimalEnumeratedLiteral& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedLiteral(
            MinimalEnumeratedLiteral&& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedLiteral& operator =(
            const MinimalEnumeratedLiteral& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedLiteral& operator =(
            MinimalEnumeratedLiteral&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonEnumeratedLiteral& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonEnumeratedLiteral&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonEnumeratedLiteral& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonEnumeratedLiteral& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const MinimalMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            MinimalMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const MinimalMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline MinimalMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalEnumeratedLiteral& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonEnumeratedHeader();
    FASTDDS_EXPORTED_API ~CommonEnumeratedHeader();
    FASTDDS_EXPORTED_API CommonEnumeratedHeader(
            const CommonEnumeratedHeader& x);
    FASTDDS_EXPORTED_API CommonEnumeratedHeader(
            CommonEnumeratedHeader&& x);
    FASTDDS_EXPORTED_API CommonEnumeratedHeader& operator =(
            const CommonEnumeratedHeader& x);
    FASTDDS_EXPORTED_API CommonEnumeratedHeader& operator =(
            CommonEnumeratedHeader&& x);

    FASTDDS_EXPORTED_API inline void bit_bound(
            const BitBound& _bit_bound)
    {
        m_bit_bound = _bit_bound;
    }

    FASTDDS_EXPORTED_API inline void bit_bound(
            BitBound&& _bit_bound)
    {
        m_bit_bound = std::move(_bit_bound);
    }

    FASTDDS_EXPORTED_API inline const BitBound& bit_bound() const
    {
        return m_bit_bound;
    }

    FASTDDS_EXPORTED_API inline BitBound& bit_bound()
    {
        return m_bit_bound;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonEnumeratedHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteEnumeratedHeader();
    FASTDDS_EXPORTED_API ~CompleteEnumeratedHeader();
    FASTDDS_EXPORTED_API CompleteEnumeratedHeader(
            const CompleteEnumeratedHeader& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedHeader(
            CompleteEnumeratedHeader&& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedHeader& operator =(
            const CompleteEnumeratedHeader& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedHeader& operator =(
            CompleteEnumeratedHeader&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonEnumeratedHeader& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonEnumeratedHeader&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonEnumeratedHeader& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonEnumeratedHeader& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteEnumeratedHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalEnumeratedHeader();
    FASTDDS_EXPORTED_API ~MinimalEnumeratedHeader();
    FASTDDS_EXPORTED_API MinimalEnumeratedHeader(
            const MinimalEnumeratedHeader& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedHeader(
            MinimalEnumeratedHeader&& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedHeader& operator =(
            const MinimalEnumeratedHeader& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedHeader& operator =(
            MinimalEnumeratedHeader&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonEnumeratedHeader& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonEnumeratedHeader&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonEnumeratedHeader& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonEnumeratedHeader& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalEnumeratedHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteEnumeratedType();
    FASTDDS_EXPORTED_API ~CompleteEnumeratedType();
    FASTDDS_EXPORTED_API CompleteEnumeratedType(
            const CompleteEnumeratedType& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedType(
            CompleteEnumeratedType&& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedType& operator =(
            const CompleteEnumeratedType& x);
    FASTDDS_EXPORTED_API CompleteEnumeratedType& operator =(
            CompleteEnumeratedType&& x);

    FASTDDS_EXPORTED_API inline void enum_flags(
            const EnumTypeFlag& _enum_flags)
    {
        m_enum_flags = _enum_flags;
    }

    FASTDDS_EXPORTED_API inline void enum_flags(
            EnumTypeFlag&& _enum_flags)
    {
        m_enum_flags = std::move(_enum_flags);
    }

    FASTDDS_EXPORTED_API inline const EnumTypeFlag& enum_flags() const
    {
        return m_enum_flags;
    }

    FASTDDS_EXPORTED_API inline EnumTypeFlag& enum_flags()
    {
        return m_enum_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteEnumeratedHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteEnumeratedHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteEnumeratedHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteEnumeratedHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void literal_seq(
            const CompleteEnumeratedLiteralSeq& _literal_seq)
    {
        m_literal_seq = _literal_seq;
    }

    FASTDDS_EXPORTED_API inline void literal_seq(
            CompleteEnumeratedLiteralSeq&& _literal_seq)
    {
        m_literal_seq = std::move(_literal_seq);
    }

    FASTDDS_EXPORTED_API inline const CompleteEnumeratedLiteralSeq& literal_seq() const
    {
        return m_literal_seq;
    }

    FASTDDS_EXPORTED_API inline CompleteEnumeratedLiteralSeq& literal_seq()
    {
        return m_literal_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteEnumeratedType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalEnumeratedType();
    FASTDDS_EXPORTED_API ~MinimalEnumeratedType();
    FASTDDS_EXPORTED_API MinimalEnumeratedType(
            const MinimalEnumeratedType& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedType(
            MinimalEnumeratedType&& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedType& operator =(
            const MinimalEnumeratedType& x);
    FASTDDS_EXPORTED_API MinimalEnumeratedType& operator =(
            MinimalEnumeratedType&& x);

    FASTDDS_EXPORTED_API inline void enum_flags(
            const EnumTypeFlag& _enum_flags)
    {
        m_enum_flags = _enum_flags;
    }

    FASTDDS_EXPORTED_API inline void enum_flags(
            EnumTypeFlag&& _enum_flags)
    {
        m_enum_flags = std::move(_enum_flags);
    }

    FASTDDS_EXPORTED_API inline const EnumTypeFlag& enum_flags() const
    {
        return m_enum_flags;
    }

    FASTDDS_EXPORTED_API inline EnumTypeFlag& enum_flags()
    {
        return m_enum_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalEnumeratedHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalEnumeratedHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalEnumeratedHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalEnumeratedHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void literal_seq(
            const MinimalEnumeratedLiteralSeq& _literal_seq)
    {
        m_literal_seq = _literal_seq;
    }

    FASTDDS_EXPORTED_API inline void literal_seq(
            MinimalEnumeratedLiteralSeq&& _literal_seq)
    {
        m_literal_seq = std::move(_literal_seq);
    }

    FASTDDS_EXPORTED_API inline const MinimalEnumeratedLiteralSeq& literal_seq() const
    {
        return m_literal_seq;
    }

    FASTDDS_EXPORTED_API inline MinimalEnumeratedLiteralSeq& literal_seq()
    {
        return m_literal_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalEnumeratedType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonBitflag();
    FASTDDS_EXPORTED_API ~CommonBitflag();
    FASTDDS_EXPORTED_API CommonBitflag(
            const CommonBitflag& x);
    FASTDDS_EXPORTED_API CommonBitflag(
            CommonBitflag&& x);
    FASTDDS_EXPORTED_API CommonBitflag& operator =(
            const CommonBitflag& x);
    FASTDDS_EXPORTED_API CommonBitflag& operator =(
            CommonBitflag&& x);

    FASTDDS_EXPORTED_API inline void position(
            const uint16_t& _position)
    {
        m_position = _position;
    }

    FASTDDS_EXPORTED_API inline void position(
            uint16_t&& _position)
    {
        m_position = std::move(_position);
    }

    FASTDDS_EXPORTED_API inline const uint16_t& position() const
    {
        return m_position;
    }

    FASTDDS_EXPORTED_API inline uint16_t& position()
    {
        return m_position;
    }

    FASTDDS_EXPORTED_API inline void flags(
            const BitflagFlag& _flags)
    {
        m_flags = _flags;
    }

    FASTDDS_EXPORTED_API inline void flags(
            BitflagFlag&& _flags)
    {
        m_flags = std::move(_flags);
    }

    FASTDDS_EXPORTED_API inline const BitflagFlag& flags() const
    {
        return m_flags;
    }

    FASTDDS_EXPORTED_API inline BitflagFlag& flags()
    {
        return m_flags;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonBitflag& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteBitflag();
    FASTDDS_EXPORTED_API ~CompleteBitflag();
    FASTDDS_EXPORTED_API CompleteBitflag(
            const CompleteBitflag& x);
    FASTDDS_EXPORTED_API CompleteBitflag(
            CompleteBitflag&& x);
    FASTDDS_EXPORTED_API CompleteBitflag& operator =(
            const CompleteBitflag& x);
    FASTDDS_EXPORTED_API CompleteBitflag& operator =(
            CompleteBitflag&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonBitflag& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonBitflag&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonBitflag& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonBitflag& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteBitflag& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalBitflag();
    FASTDDS_EXPORTED_API ~MinimalBitflag();
    FASTDDS_EXPORTED_API MinimalBitflag(
            const MinimalBitflag& x);
    FASTDDS_EXPORTED_API MinimalBitflag(
            MinimalBitflag&& x);
    FASTDDS_EXPORTED_API MinimalBitflag& operator =(
            const MinimalBitflag& x);
    FASTDDS_EXPORTED_API MinimalBitflag& operator =(
            MinimalBitflag&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonBitflag& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonBitflag&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonBitflag& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonBitflag& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const MinimalMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            MinimalMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const MinimalMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline MinimalMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalBitflag& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonBitmaskHeader();
    FASTDDS_EXPORTED_API ~CommonBitmaskHeader();
    FASTDDS_EXPORTED_API CommonBitmaskHeader(
            const CommonBitmaskHeader& x);
    FASTDDS_EXPORTED_API CommonBitmaskHeader(
            CommonBitmaskHeader&& x);
    FASTDDS_EXPORTED_API CommonBitmaskHeader& operator =(
            const CommonBitmaskHeader& x);
    FASTDDS_EXPORTED_API CommonBitmaskHeader& operator =(
            CommonBitmaskHeader&& x);

    FASTDDS_EXPORTED_API inline void bit_bound(
            const BitBound& _bit_bound)
    {
        m_bit_bound = _bit_bound;
    }

    FASTDDS_EXPORTED_API inline void bit_bound(
            BitBound&& _bit_bound)
    {
        m_bit_bound = std::move(_bit_bound);
    }

    FASTDDS_EXPORTED_API inline const BitBound& bit_bound() const
    {
        return m_bit_bound;
    }

    FASTDDS_EXPORTED_API inline BitBound& bit_bound()
    {
        return m_bit_bound;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonBitmaskHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteBitmaskType();
    FASTDDS_EXPORTED_API ~CompleteBitmaskType();
    FASTDDS_EXPORTED_API CompleteBitmaskType(
            const CompleteBitmaskType& x);
    FASTDDS_EXPORTED_API CompleteBitmaskType(
            CompleteBitmaskType&& x);
    FASTDDS_EXPORTED_API CompleteBitmaskType& operator =(
            const CompleteBitmaskType& x);
    FASTDDS_EXPORTED_API CompleteBitmaskType& operator =(
            CompleteBitmaskType&& x);

    FASTDDS_EXPORTED_API inline void bitmask_flags(
            const BitmaskTypeFlag& _bitmask_flags)
    {
        m_bitmask_flags = _bitmask_flags;
    }

    FASTDDS_EXPORTED_API inline void bitmask_flags(
            BitmaskTypeFlag&& _bitmask_flags)
    {
        m_bitmask_flags = std::move(_bitmask_flags);
    }

    FASTDDS_EXPORTED_API inline const BitmaskTypeFlag& bitmask_flags() const
    {
        return m_bitmask_flags;
    }

    FASTDDS_EXPORTED_API inline BitmaskTypeFlag& bitmask_flags()
    {
        return m_bitmask_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteBitmaskHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteBitmaskHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteBitmaskHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteBitmaskHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void flag_seq(
            const CompleteBitflagSeq& _flag_seq)
    {
        m_flag_seq = _flag_seq;
    }

    FASTDDS_EXPORTED_API inline void flag_seq(
            CompleteBitflagSeq&& _flag_seq)
    {
        m_flag_seq = std::move(_flag_seq);
    }

    FASTDDS_EXPORTED_API inline const CompleteBitflagSeq& flag_seq() const
    {
        return m_flag_seq;
    }

    FASTDDS_EXPORTED_API inline CompleteBitflagSeq& flag_seq()
    {
        return m_flag_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteBitmaskType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalBitmaskType();
    FASTDDS_EXPORTED_API ~MinimalBitmaskType();
    FASTDDS_EXPORTED_API MinimalBitmaskType(
            const MinimalBitmaskType& x);
    FASTDDS_EXPORTED_API MinimalBitmaskType(
            MinimalBitmaskType&& x);
    FASTDDS_EXPORTED_API MinimalBitmaskType& operator =(
            const MinimalBitmaskType& x);
    FASTDDS_EXPORTED_API MinimalBitmaskType& operator =(
            MinimalBitmaskType&& x);

    FASTDDS_EXPORTED_API inline void bitmask_flags(
            const BitmaskTypeFlag& _bitmask_flags)
    {
        m_bitmask_flags = _bitmask_flags;
    }

    FASTDDS_EXPORTED_API inline void bitmask_flags(
            BitmaskTypeFlag&& _bitmask_flags)
    {
        m_bitmask_flags = std::move(_bitmask_flags);
    }

    FASTDDS_EXPORTED_API inline const BitmaskTypeFlag& bitmask_flags() const
    {
        return m_bitmask_flags;
    }

    FASTDDS_EXPORTED_API inline BitmaskTypeFlag& bitmask_flags()
    {
        return m_bitmask_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalBitmaskHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalBitmaskHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalBitmaskHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalBitmaskHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void flag_seq(
            const MinimalBitflagSeq& _flag_seq)
    {
        m_flag_seq = _flag_seq;
    }

    FASTDDS_EXPORTED_API inline void flag_seq(
            MinimalBitflagSeq&& _flag_seq)
    {
        m_flag_seq = std::move(_flag_seq);
    }

    FASTDDS_EXPORTED_API inline const MinimalBitflagSeq& flag_seq() const
    {
        return m_flag_seq;
    }

    FASTDDS_EXPORTED_API inline MinimalBitflagSeq& flag_seq()
    {
        return m_flag_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalBitmaskType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CommonBitfield();
    FASTDDS_EXPORTED_API ~CommonBitfield();
    FASTDDS_EXPORTED_API CommonBitfield(
            const CommonBitfield& x);
    FASTDDS_EXPORTED_API CommonBitfield(
            CommonBitfield&& x);
    FASTDDS_EXPORTED_API CommonBitfield& operator =(
            const CommonBitfield& x);
    FASTDDS_EXPORTED_API CommonBitfield& operator =(
            CommonBitfield&& x);

    FASTDDS_EXPORTED_API inline void position(
            const uint16_t& _position)
    {
        m_position = _position;
    }

    FASTDDS_EXPORTED_API inline void position(
            uint16_t&& _position)
    {
        m_position = std::move(_position);
    }

    FASTDDS_EXPORTED_API inline const uint16_t& position() const
    {
        return m_position;
    }

    FASTDDS_EXPORTED_API inline uint16_t& position()
    {
        return m_position;
    }

    FASTDDS_EXPORTED_API inline void flags(
            const BitsetMemberFlag& _flags)
    {
        m_flags = _flags;
    }

    FASTDDS_EXPORTED_API inline void flags(
            BitsetMemberFlag&& _flags)
    {
        m_flags = std::move(_flags);
    }

    FASTDDS_EXPORTED_API inline const BitsetMemberFlag& flags() const
    {
        return m_flags;
    }

    FASTDDS_EXPORTED_API inline BitsetMemberFlag& flags()
    {
        return m_flags;
    }

    FASTDDS_EXPORTED_API inline void bitcount(
            const octet& _bitcount)
    {
        m_bitcount = _bitcount;
    }

    FASTDDS_EXPORTED_API inline void bitcount(
            octet&& _bitcount)
    {
        m_bitcount = std::move(_bitcount);
    }

    FASTDDS_EXPORTED_API inline const octet& bitcount() const
    {
        return m_bitcount;
    }

    FASTDDS_EXPORTED_API inline octet& bitcount()
    {
        return m_bitcount;
    }

    FASTDDS_EXPORTED_API inline void holder_type(
            const TypeKind& _holder_type)
    {
        m_holder_type = _holder_type;
    }

    FASTDDS_EXPORTED_API inline void holder_type(
            TypeKind&& _holder_type)
    {
        m_holder_type = std::move(_holder_type);
    }

    FASTDDS_EXPORTED_API inline const TypeKind& holder_type() const
    {
        return m_holder_type;
    }

    FASTDDS_EXPORTED_API inline TypeKind& holder_type()
    {
        return m_holder_type;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CommonBitfield& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteBitfield();
    FASTDDS_EXPORTED_API ~CompleteBitfield();
    FASTDDS_EXPORTED_API CompleteBitfield(
            const CompleteBitfield& x);
    FASTDDS_EXPORTED_API CompleteBitfield(
            CompleteBitfield&& x);
    FASTDDS_EXPORTED_API CompleteBitfield& operator =(
            const CompleteBitfield& x);
    FASTDDS_EXPORTED_API CompleteBitfield& operator =(
            CompleteBitfield&& x);

    FASTDDS_EXPORTED_API inline void common(
            const CommonBitfield& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonBitfield&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonBitfield& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonBitfield& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteMemberDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteMemberDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteMemberDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteMemberDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteBitfield& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalBitfield();
    FASTDDS_EXPORTED_API ~MinimalBitfield();
    FASTDDS_EXPORTED_API MinimalBitfield(
            const MinimalBitfield& x);
    FASTDDS_EXPORTED_API MinimalBitfield(
            MinimalBitfield&& x);
    FASTDDS_EXPORTED_API MinimalBitfield& operator =(
            const MinimalBitfield& x);
    FASTDDS_EXPORTED_API MinimalBitfield& operator =(
            MinimalBitfield&& x);

    FASTDDS_EXPORTED_API inline void name_hash(
            const NameHash& _name_hash)
    {
        m_name_hash = _name_hash;
    }

    FASTDDS_EXPORTED_API inline void name_hash(
            NameHash&& _name_hash)
    {
        m_name_hash = std::move(_name_hash);
    }

    FASTDDS_EXPORTED_API inline const NameHash& name_hash() const
    {
        return m_name_hash;
    }

    FASTDDS_EXPORTED_API inline NameHash& name_hash()
    {
        return m_name_hash;
    }

    FASTDDS_EXPORTED_API inline void common(
            const CommonBitfield& _common)
    {
        m_common = _common;
    }

    FASTDDS_EXPORTED_API inline void common(
            CommonBitfield&& _common)
    {
        m_common = std::move(_common);
    }

    FASTDDS_EXPORTED_API inline const CommonBitfield& common() const
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API inline CommonBitfield& common()
    {
        return m_common;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalBitfield& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteBitsetHeader();
    FASTDDS_EXPORTED_API ~CompleteBitsetHeader();
    FASTDDS_EXPORTED_API CompleteBitsetHeader(
            const CompleteBitsetHeader& x);
    FASTDDS_EXPORTED_API CompleteBitsetHeader(
            CompleteBitsetHeader&& x);
    FASTDDS_EXPORTED_API CompleteBitsetHeader& operator =(
            const CompleteBitsetHeader& x);
    FASTDDS_EXPORTED_API CompleteBitsetHeader& operator =(
            CompleteBitsetHeader&& x);

    FASTDDS_EXPORTED_API inline void base_type(
            const TypeIdentifier& _base_type)
    {
        m_base_type = _base_type;
    }

    FASTDDS_EXPORTED_API inline void base_type(
            TypeIdentifier&& _base_type)
    {
        m_base_type = std::move(_base_type);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& base_type() const
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& base_type()
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline void detail(
            const CompleteTypeDetail& _detail)
    {
        m_detail = _detail;
    }

    FASTDDS_EXPORTED_API inline void detail(
            CompleteTypeDetail&& _detail)
    {
        m_detail = std::move(_detail);
    }

    FASTDDS_EXPORTED_API inline const CompleteTypeDetail& detail() const
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API inline CompleteTypeDetail& detail()
    {
        return m_detail;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteBitsetHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalBitsetHeader();
    FASTDDS_EXPORTED_API ~MinimalBitsetHeader();
    FASTDDS_EXPORTED_API MinimalBitsetHeader(
            const MinimalBitsetHeader& x);
    FASTDDS_EXPORTED_API MinimalBitsetHeader(
            MinimalBitsetHeader&& x);
    FASTDDS_EXPORTED_API MinimalBitsetHeader& operator =(
            const MinimalBitsetHeader& x);
    FASTDDS_EXPORTED_API MinimalBitsetHeader& operator =(
            MinimalBitsetHeader&& x);

    FASTDDS_EXPORTED_API inline void base_type(
            const TypeIdentifier& _base_type)
    {
        m_base_type = _base_type;
    }

    FASTDDS_EXPORTED_API inline void base_type(
            TypeIdentifier&& _base_type)
    {
        m_base_type = std::move(_base_type);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& base_type() const
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& base_type()
    {
        return m_base_type;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalBitsetHeader& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteBitsetType();
    FASTDDS_EXPORTED_API ~CompleteBitsetType();
    FASTDDS_EXPORTED_API CompleteBitsetType(
            const CompleteBitsetType& x);
    FASTDDS_EXPORTED_API CompleteBitsetType(
            CompleteBitsetType&& x);
    FASTDDS_EXPORTED_API CompleteBitsetType& operator =(
            const CompleteBitsetType& x);
    FASTDDS_EXPORTED_API CompleteBitsetType& operator =(
            CompleteBitsetType&& x);

    FASTDDS_EXPORTED_API inline void bitset_flags(
            const BitsetTypeFlag& _bitset_flags)
    {
        m_bitset_flags = _bitset_flags;
    }

    FASTDDS_EXPORTED_API inline void bitset_flags(
            BitsetTypeFlag&& _bitset_flags)
    {
        m_bitset_flags = std::move(_bitset_flags);
    }

    FASTDDS_EXPORTED_API inline const BitsetTypeFlag& bitset_flags() const
    {
        return m_bitset_flags;
    }

    FASTDDS_EXPORTED_API inline BitsetTypeFlag& bitset_flags()
    {
        return m_bitset_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const CompleteBitsetHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            CompleteBitsetHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const CompleteBitsetHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline CompleteBitsetHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void field_seq(
            const CompleteBitfieldSeq& _field_seq)
    {
        m_field_seq = _field_seq;
    }

    FASTDDS_EXPORTED_API inline void field_seq(
            CompleteBitfieldSeq&& _field_seq)
    {
        m_field_seq = std::move(_field_seq);
    }

    FASTDDS_EXPORTED_API inline const CompleteBitfieldSeq& field_seq() const
    {
        return m_field_seq;
    }

    FASTDDS_EXPORTED_API inline CompleteBitfieldSeq& field_seq()
    {
        return m_field_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteBitsetType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalBitsetType();
    FASTDDS_EXPORTED_API ~MinimalBitsetType();
    FASTDDS_EXPORTED_API MinimalBitsetType(
            const MinimalBitsetType& x);
    FASTDDS_EXPORTED_API MinimalBitsetType(
            MinimalBitsetType&& x);
    FASTDDS_EXPORTED_API MinimalBitsetType& operator =(
            const MinimalBitsetType& x);
    FASTDDS_EXPORTED_API MinimalBitsetType& operator =(
            MinimalBitsetType&& x);

    FASTDDS_EXPORTED_API inline void bitset_flags(
            const BitsetTypeFlag& _bitset_flags)
    {
        m_bitset_flags = _bitset_flags;
    }

    FASTDDS_EXPORTED_API inline void bitset_flags(
            BitsetTypeFlag&& _bitset_flags)
    {
        m_bitset_flags = std::move(_bitset_flags);
    }

    FASTDDS_EXPORTED_API inline const BitsetTypeFlag& bitset_flags() const
    {
        return m_bitset_flags;
    }

    FASTDDS_EXPORTED_API inline BitsetTypeFlag& bitset_flags()
    {
        return m_bitset_flags;
    }

    FASTDDS_EXPORTED_API inline void header(
            const MinimalBitsetHeader& _header)
    {
        m_header = _header;
    }

    FASTDDS_EXPORTED_API inline void header(
            MinimalBitsetHeader&& _header)
    {
        m_header = std::move(_header);
    }

    FASTDDS_EXPORTED_API inline const MinimalBitsetHeader& header() const
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline MinimalBitsetHeader& header()
    {
        return m_header;
    }

    FASTDDS_EXPORTED_API inline void field_seq(
            const MinimalBitfieldSeq& _field_seq)
    {
        m_field_seq = _field_seq;
    }

    FASTDDS_EXPORTED_API inline void field_seq(
            MinimalBitfieldSeq&& _field_seq)
    {
        m_field_seq = std::move(_field_seq);
    }

    FASTDDS_EXPORTED_API inline const MinimalBitfieldSeq& field_seq() const
    {
        return m_field_seq;
    }

    FASTDDS_EXPORTED_API inline MinimalBitfieldSeq& field_seq()
    {
        return m_field_seq;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalBitsetType& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API CompleteExtendedType();
    FASTDDS_EXPORTED_API ~CompleteExtendedType();
    FASTDDS_EXPORTED_API CompleteExtendedType(
            const CompleteExtendedType& x);
    FASTDDS_EXPORTED_API CompleteExtendedType(
            CompleteExtendedType&& x);
    FASTDDS_EXPORTED_API CompleteExtendedType& operator =(
            const CompleteExtendedType& x);
    FASTDDS_EXPORTED_API CompleteExtendedType& operator =(
            CompleteExtendedType&& x);

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteExtendedType&) const
    {
        return true;
    }

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalExtendedType();
    FASTDDS_EXPORTED_API ~MinimalExtendedType();
    FASTDDS_EXPORTED_API MinimalExtendedType(
            const MinimalExtendedType& x);
    FASTDDS_EXPORTED_API MinimalExtendedType(
            MinimalExtendedType&& x);
    FASTDDS_EXPORTED_API MinimalExtendedType& operator =(
            const MinimalExtendedType& x);
    FASTDDS_EXPORTED_API MinimalExtendedType& operator =(
            MinimalExtendedType&& x);

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalExtendedType&) const
    {
        return true;
    }

    FASTDDS_EXPORTED_API bool consistent(
            const MinimalExtendedType& x,
            const fastdds::dds::TypeConsistencyEnforcementQosPolicy& consistency) const;

private:

};

class CompleteTypeObject final
{
public:

    FASTDDS_EXPORTED_API CompleteTypeObject();
    FASTDDS_EXPORTED_API ~CompleteTypeObject();
    FASTDDS_EXPORTED_API CompleteTypeObject(
            const CompleteTypeObject& x);
    FASTDDS_EXPORTED_API CompleteTypeObject(
            CompleteTypeObject&& x);
    FASTDDS_EXPORTED_API CompleteTypeObject& operator =(
            const CompleteTypeObject& x);
    FASTDDS_EXPORTED_API CompleteTypeObject& operator =(
            CompleteTypeObject&& x);
    FASTDDS_EXPORTED_API void _d(
            octet __d);
    FASTDDS_EXPORTED_API octet _d() const;
    FASTDDS_EXPORTED_API octet& _d();

    FASTDDS_EXPORTED_API void alias_type(
            CompleteAliasType _alias_type);
    FASTDDS_EXPORTED_API const CompleteAliasType& alias_type() const;
    FASTDDS_EXPORTED_API CompleteAliasType& alias_type();

    FASTDDS_EXPORTED_API void annotation_type(
            CompleteAnnotationType _annotation_type);
    FASTDDS_EXPORTED_API const CompleteAnnotationType& annotation_type() const;
    FASTDDS_EXPORTED_API CompleteAnnotationType& annotation_type();

    FASTDDS_EXPORTED_API void struct_type(
            CompleteStructType _struct_type);
    FASTDDS_EXPORTED_API const CompleteStructType& struct_type() const;
    FASTDDS_EXPORTED_API CompleteStructType& struct_type();

    FASTDDS_EXPORTED_API void union_type(
            CompleteUnionType _union_type);
    FASTDDS_EXPORTED_API const CompleteUnionType& union_type() const;
    FASTDDS_EXPORTED_API CompleteUnionType& union_type();

    FASTDDS_EXPORTED_API void bitset_type(
            CompleteBitsetType _bitset_type);
    FASTDDS_EXPORTED_API const CompleteBitsetType& bitset_type() const;
    FASTDDS_EXPORTED_API CompleteBitsetType& bitset_type();

    FASTDDS_EXPORTED_API void sequence_type(
            CompleteSequenceType _sequence_type);
    FASTDDS_EXPORTED_API const CompleteSequenceType& sequence_type() const;
    FASTDDS_EXPORTED_API CompleteSequenceType& sequence_type();

    FASTDDS_EXPORTED_API void array_type(
            CompleteArrayType _array_type);
    FASTDDS_EXPORTED_API const CompleteArrayType& array_type() const;
    FASTDDS_EXPORTED_API CompleteArrayType& array_type();

    FASTDDS_EXPORTED_API void map_type(
            CompleteMapType _map_type);
    FASTDDS_EXPORTED_API const CompleteMapType& map_type() const;
    FASTDDS_EXPORTED_API CompleteMapType& map_type();

    FASTDDS_EXPORTED_API void enumerated_type(
            CompleteEnumeratedType _enumerated_type);
    FASTDDS_EXPORTED_API const CompleteEnumeratedType& enumerated_type() const;
    FASTDDS_EXPORTED_API CompleteEnumeratedType& enumerated_type();

    FASTDDS_EXPORTED_API void bitmask_type(
            CompleteBitmaskType _bitmask_type);
    FASTDDS_EXPORTED_API const CompleteBitmaskType& bitmask_type() const;
    FASTDDS_EXPORTED_API CompleteBitmaskType& bitmask_type();

    FASTDDS_EXPORTED_API void extended_type(
            CompleteExtendedType _extended_type);
    FASTDDS_EXPORTED_API const CompleteExtendedType& extended_type() const;
    FASTDDS_EXPORTED_API CompleteExtendedType& extended_type();

    FASTDDS_EXPORTED_API bool operator ==(
            const CompleteTypeObject& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API MinimalTypeObject();
    FASTDDS_EXPORTED_API ~MinimalTypeObject();
    FASTDDS_EXPORTED_API MinimalTypeObject(
            const MinimalTypeObject& x);
    FASTDDS_EXPORTED_API MinimalTypeObject(
            MinimalTypeObject&& x);
    FASTDDS_EXPORTED_API MinimalTypeObject& operator =(
            const MinimalTypeObject& x);
    FASTDDS_EXPORTED_API MinimalTypeObject& operator =(
            MinimalTypeObject&& x);
    FASTDDS_EXPORTED_API void _d(
            octet __d);
    FASTDDS_EXPORTED_API octet _d() const;
    FASTDDS_EXPORTED_API octet& _d();

    FASTDDS_EXPORTED_API void alias_type(
            MinimalAliasType _alias_type);
    FASTDDS_EXPORTED_API const MinimalAliasType& alias_type() const;
    FASTDDS_EXPORTED_API MinimalAliasType& alias_type();

    FASTDDS_EXPORTED_API void annotation_type(
            MinimalAnnotationType _annotation_type);
    FASTDDS_EXPORTED_API const MinimalAnnotationType& annotation_type() const;
    FASTDDS_EXPORTED_API MinimalAnnotationType& annotation_type();

    FASTDDS_EXPORTED_API void struct_type(
            MinimalStructType _struct_type);
    FASTDDS_EXPORTED_API const MinimalStructType& struct_type() const;
    FASTDDS_EXPORTED_API MinimalStructType& struct_type();

    FASTDDS_EXPORTED_API void union_type(
            MinimalUnionType _union_type);
    FASTDDS_EXPORTED_API const MinimalUnionType& union_type() const;
    FASTDDS_EXPORTED_API MinimalUnionType& union_type();

    FASTDDS_EXPORTED_API void bitset_type(
            MinimalBitsetType _bitset_type);
    FASTDDS_EXPORTED_API const MinimalBitsetType& bitset_type() const;
    FASTDDS_EXPORTED_API MinimalBitsetType& bitset_type();

    FASTDDS_EXPORTED_API void sequence_type(
            MinimalSequenceType _sequence_type);
    FASTDDS_EXPORTED_API const MinimalSequenceType& sequence_type() const;
    FASTDDS_EXPORTED_API MinimalSequenceType& sequence_type();

    FASTDDS_EXPORTED_API void array_type(
            MinimalArrayType _array_type);
    FASTDDS_EXPORTED_API const MinimalArrayType& array_type() const;
    FASTDDS_EXPORTED_API MinimalArrayType& array_type();

    FASTDDS_EXPORTED_API void map_type(
            MinimalMapType _map_type);
    FASTDDS_EXPORTED_API const MinimalMapType& map_type() const;
    FASTDDS_EXPORTED_API MinimalMapType& map_type();

    FASTDDS_EXPORTED_API void enumerated_type(
            MinimalEnumeratedType _enumerated_type);
    FASTDDS_EXPORTED_API const MinimalEnumeratedType& enumerated_type() const;
    FASTDDS_EXPORTED_API MinimalEnumeratedType& enumerated_type();

    FASTDDS_EXPORTED_API void bitmask_type(
            MinimalBitmaskType _bitmask_type);
    FASTDDS_EXPORTED_API const MinimalBitmaskType& bitmask_type() const;
    FASTDDS_EXPORTED_API MinimalBitmaskType& bitmask_type();

    FASTDDS_EXPORTED_API void extended_type(
            MinimalExtendedType _extended_type);
    FASTDDS_EXPORTED_API const MinimalExtendedType& extended_type() const;
    FASTDDS_EXPORTED_API MinimalExtendedType& extended_type();

    FASTDDS_EXPORTED_API bool operator ==(
            const MinimalTypeObject& other) const;

    FASTDDS_EXPORTED_API bool consistent(
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
    FASTDDS_EXPORTED_API TypeObject();

    /*!
     * @brief Default destructor.
     */
    FASTDDS_EXPORTED_API ~TypeObject();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object TypeObject that will be copied.
     */
    FASTDDS_EXPORTED_API TypeObject(
            const TypeObject& x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object TypeObject that will be copied.
     */
    FASTDDS_EXPORTED_API TypeObject(
            TypeObject&& x);

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object TypeObject that will be copied.
     */
    FASTDDS_EXPORTED_API TypeObject& operator =(
            const TypeObject& x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object TypeObject that will be copied.
     */
    FASTDDS_EXPORTED_API TypeObject& operator =(
            TypeObject&& x);

    /*!
     * @brief This function sets the discriminator value.
     * @param __d New value for the discriminator.
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
     */
    FASTDDS_EXPORTED_API void _d(
            uint8_t __d);

    /*!
     * @brief This function returns the value of the discriminator.
     * @return Value of the discriminator
     */
    FASTDDS_EXPORTED_API uint8_t _d() const;

    /*!
     * @brief This function returns a reference to the discriminator.
     * @return Reference to the discriminator.
     */
    FASTDDS_EXPORTED_API uint8_t& _d();

    /*!
     * @brief This function copies the value in member complete
     * @param _complete New value to be copied in member complete
     */
    FASTDDS_EXPORTED_API void complete(
            const CompleteTypeObject& _complete);

    /*!
     * @brief This function moves the value in member complete
     * @param _complete New value to be moved in member complete
     */
    FASTDDS_EXPORTED_API void complete(
            CompleteTypeObject&& _complete);

    /*!
     * @brief This function returns a constant reference to member complete
     * @return Constant reference to member complete
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    FASTDDS_EXPORTED_API const CompleteTypeObject& complete() const;

    /*!
     * @brief This function returns a reference to member complete
     * @return Reference to member complete
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    FASTDDS_EXPORTED_API CompleteTypeObject& complete();
    /*!
     * @brief This function copies the value in member minimal
     * @param _minimal New value to be copied in member minimal
     */
    FASTDDS_EXPORTED_API void minimal(
            const MinimalTypeObject& _minimal);

    /*!
     * @brief This function moves the value in member minimal
     * @param _minimal New value to be moved in member minimal
     */
    FASTDDS_EXPORTED_API void minimal(
            MinimalTypeObject&& _minimal);

    /*!
     * @brief This function returns a constant reference to member minimal
     * @return Constant reference to member minimal
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    FASTDDS_EXPORTED_API const MinimalTypeObject& minimal() const;

    /*!
     * @brief This function returns a reference to member minimal
     * @return Reference to member minimal
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    FASTDDS_EXPORTED_API MinimalTypeObject& minimal();

    FASTDDS_EXPORTED_API bool operator ==(
            const TypeObject& other) const;

    /*!
     * @brief This function check type consistency enforcement with the given TypeObject x.
     * @param x TypeObject to check if can be assigned to the current instance.
     * @param consistency fastdds::dds::TypeConsistencyEnforcementQoSPolicy to apply.
     */
    FASTDDS_EXPORTED_API bool consistent(
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

    FASTDDS_EXPORTED_API TypeIdentifierTypeObjectPair();
    FASTDDS_EXPORTED_API ~TypeIdentifierTypeObjectPair();
    FASTDDS_EXPORTED_API TypeIdentifierTypeObjectPair(
            const TypeIdentifierTypeObjectPair& x);
    FASTDDS_EXPORTED_API TypeIdentifierTypeObjectPair(
            TypeIdentifierTypeObjectPair&& x);
    FASTDDS_EXPORTED_API TypeIdentifierTypeObjectPair& operator =(
            const TypeIdentifierTypeObjectPair& x);
    FASTDDS_EXPORTED_API TypeIdentifierTypeObjectPair& operator =(
            TypeIdentifierTypeObjectPair&& x);

    FASTDDS_EXPORTED_API inline void type_identifier(
            const TypeIdentifier& _type_identifier)
    {
        m_type_identifier = _type_identifier;
    }

    FASTDDS_EXPORTED_API inline void type_identifier(
            TypeIdentifier&& _type_identifier)
    {
        m_type_identifier = std::move(_type_identifier);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type_identifier() const
    {
        return m_type_identifier;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type_identifier()
    {
        return m_type_identifier;
    }

    FASTDDS_EXPORTED_API inline void type_object(
            const TypeObject& _type_object)
    {
        m_type_object = _type_object;
    }

    FASTDDS_EXPORTED_API inline void type_object(
            TypeObject&& _type_object)
    {
        m_type_object = std::move(_type_object);
    }

    FASTDDS_EXPORTED_API inline const TypeObject& type_object() const
    {
        return m_type_object;
    }

    FASTDDS_EXPORTED_API inline TypeObject& type_object()
    {
        return m_type_object;
    }

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

    FASTDDS_EXPORTED_API TypeIdentifierPair();
    FASTDDS_EXPORTED_API ~TypeIdentifierPair();
    FASTDDS_EXPORTED_API TypeIdentifierPair(
            const TypeIdentifierPair& x);
    FASTDDS_EXPORTED_API TypeIdentifierPair(
            TypeIdentifierPair&& x);
    FASTDDS_EXPORTED_API TypeIdentifierPair& operator =(
            const TypeIdentifierPair& x);
    FASTDDS_EXPORTED_API TypeIdentifierPair& operator =(
            TypeIdentifierPair&& x);

    FASTDDS_EXPORTED_API inline void type_identifier1(
            const TypeIdentifier& _type_identifier1)
    {
        m_type_identifier1 = _type_identifier1;
    }

    FASTDDS_EXPORTED_API inline void type_identifier1(
            TypeIdentifier&& _type_identifier1)
    {
        m_type_identifier1 = std::move(_type_identifier1);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type_identifier1() const
    {
        return m_type_identifier1;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type_identifier1()
    {
        return m_type_identifier1;
    }

    FASTDDS_EXPORTED_API inline void type_identifier2(
            const TypeIdentifier& _type_identifier2)
    {
        m_type_identifier2 = _type_identifier2;
    }

    FASTDDS_EXPORTED_API inline void type_identifier2(
            TypeIdentifier&& _type_identifier2)
    {
        m_type_identifier2 = std::move(_type_identifier2);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type_identifier2() const
    {
        return m_type_identifier2;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type_identifier2()
    {
        return m_type_identifier2;
    }

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

    FASTDDS_EXPORTED_API TypeIdentifierWithSize();
    FASTDDS_EXPORTED_API ~TypeIdentifierWithSize();
    FASTDDS_EXPORTED_API TypeIdentifierWithSize(
            const TypeIdentifierWithSize& x);
    FASTDDS_EXPORTED_API TypeIdentifierWithSize(
            TypeIdentifierWithSize&& x);
    FASTDDS_EXPORTED_API TypeIdentifierWithSize& operator =(
            const TypeIdentifierWithSize& x);
    FASTDDS_EXPORTED_API TypeIdentifierWithSize& operator =(
            TypeIdentifierWithSize&& x);

    FASTDDS_EXPORTED_API inline void type_id(
            const TypeIdentifier& _type_id)
    {
        m_type_id = _type_id;
    }

    FASTDDS_EXPORTED_API inline void type_id(
            TypeIdentifier&& _type_id)
    {
        m_type_id = std::move(_type_id);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifier& type_id() const
    {
        return m_type_id;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifier& type_id()
    {
        return m_type_id;
    }

    FASTDDS_EXPORTED_API inline void typeobject_serialized_size(
            const uint32_t& _typeobject_serialized_size)
    {
        m_typeobject_serialized_size = _typeobject_serialized_size;
    }

    FASTDDS_EXPORTED_API inline void typeobject_serialized_size(
            uint32_t&& _typeobject_serialized_size)
    {
        m_typeobject_serialized_size = std::move(_typeobject_serialized_size);
    }

    FASTDDS_EXPORTED_API inline const uint32_t& typeobject_serialized_size() const
    {
        return m_typeobject_serialized_size;
    }

    FASTDDS_EXPORTED_API inline uint32_t& typeobject_serialized_size()
    {
        return m_typeobject_serialized_size;
    }

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

    FASTDDS_EXPORTED_API TypeIdentifierWithDependencies();
    FASTDDS_EXPORTED_API ~TypeIdentifierWithDependencies();
    FASTDDS_EXPORTED_API TypeIdentifierWithDependencies(
            const TypeIdentifierWithDependencies& x);
    FASTDDS_EXPORTED_API TypeIdentifierWithDependencies(
            TypeIdentifierWithDependencies&& x);
    FASTDDS_EXPORTED_API TypeIdentifierWithDependencies& operator =(
            const TypeIdentifierWithDependencies& x);
    FASTDDS_EXPORTED_API TypeIdentifierWithDependencies& operator =(
            TypeIdentifierWithDependencies&& x);

    FASTDDS_EXPORTED_API inline void typeid_with_size(
            const TypeIdentifierWithSize& _typeid_with_size)
    {
        m_typeid_with_size = _typeid_with_size;
    }

    FASTDDS_EXPORTED_API inline void typeid_with_size(
            TypeIdentifierWithSize&& _typeid_with_size)
    {
        m_typeid_with_size = std::move(_typeid_with_size);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifierWithSize& typeid_with_size() const
    {
        return m_typeid_with_size;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifierWithSize& typeid_with_size()
    {
        return m_typeid_with_size;
    }

    FASTDDS_EXPORTED_API inline void dependent_typeid_count(
            const int32_t& _dependent_typeid_count)
    {
        m_dependent_typeid_count = _dependent_typeid_count;
    }

    FASTDDS_EXPORTED_API inline void dependent_typeid_count(
            int32_t&& _dependent_typeid_count)
    {
        m_dependent_typeid_count = std::move(_dependent_typeid_count);
    }

    FASTDDS_EXPORTED_API inline const int32_t& dependent_typeid_count() const
    {
        return m_dependent_typeid_count;
    }

    FASTDDS_EXPORTED_API inline int32_t& dependent_typeid_count()
    {
        return m_dependent_typeid_count;
    }

    FASTDDS_EXPORTED_API inline void dependent_typeids(
            const TypeIdentifierWithSizeSeq& _dependent_typeids)
    {
        m_dependent_typeids = _dependent_typeids;
    }

    FASTDDS_EXPORTED_API inline void dependent_typeids(
            TypeIdentifierWithSizeSeq&& _dependent_typeids)
    {
        m_dependent_typeids = std::move(_dependent_typeids);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifierWithSizeSeq& dependent_typeids() const
    {
        return m_dependent_typeids;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifierWithSizeSeq& dependent_typeids()
    {
        return m_dependent_typeids;
    }

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

    FASTDDS_EXPORTED_API TypeInformation();
    FASTDDS_EXPORTED_API ~TypeInformation();
    FASTDDS_EXPORTED_API TypeInformation(
            const TypeInformation& x);
    FASTDDS_EXPORTED_API TypeInformation(
            TypeInformation&& x);
    FASTDDS_EXPORTED_API TypeInformation& operator =(
            const TypeInformation& x);
    FASTDDS_EXPORTED_API TypeInformation& operator =(
            TypeInformation&& x);

    FASTDDS_EXPORTED_API inline void minimal(
            const TypeIdentifierWithDependencies& _minimal)
    {
        m_minimal = _minimal;
    }

    FASTDDS_EXPORTED_API inline void minimal(
            TypeIdentifierWithDependencies&& _minimal)
    {
        m_minimal = std::move(_minimal);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifierWithDependencies& minimal() const
    {
        return m_minimal;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifierWithDependencies& minimal()
    {
        return m_minimal;
    }

    FASTDDS_EXPORTED_API inline void complete(
            const TypeIdentifierWithDependencies& _complete)
    {
        m_complete = _complete;
    }

    FASTDDS_EXPORTED_API inline void complete(
            TypeIdentifierWithDependencies&& _complete)
    {
        m_complete = std::move(_complete);
    }

    FASTDDS_EXPORTED_API inline const TypeIdentifierWithDependencies& complete() const
    {
        return m_complete;
    }

    FASTDDS_EXPORTED_API inline TypeIdentifierWithDependencies& complete()
    {
        return m_complete;
    }

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
