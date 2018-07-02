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
#include <fastrtps/types/MemberId.h>
#include <fastrtps/types/TypeObjectHashId.h>
#include <fastrtps/types/TypeIdentifier.h>
#include <fastrtps/types/AnnotationParameterValue.h>
#include <stdint.h>
#include <array>

namespace eprosima
{
    namespace fastcdr
    {
        class Cdr;
    }
}

// The types in this file shall be serialized with XCDR encoding version 2
namespace eprosima{
namespace fastrtps{

using namespace rtps;

namespace types{

/*struct CommonStructMember final {
	MemberId                                   member_id;
	StructMemberFlag                           member_flags;
	TypeIdentifier                             member_type_id;
};*/
class CommonStructMember
{
public:
    CommonStructMember();
    ~CommonStructMember();
    CommonStructMember(const CommonStructMember &x);
    CommonStructMember(CommonStructMember &&x);
    CommonStructMember& operator=(const CommonStructMember &x);
    CommonStructMember& operator=(CommonStructMember &&x);

    inline void member_id(const MemberId &_member_id) { m_member_id = _member_id; }
    inline void member_id(MemberId &&_member_id) { m_member_id = std::move(_member_id); }
    inline const MemberId& member_id() const { return m_member_id; }
    inline MemberId& member_id() { return m_member_id; }

    inline void member_flags(const StructMemberFlag &_member_flags) { m_member_flags = _member_flags; }
    inline void member_flags(StructMemberFlag &&_member_flags) { m_member_flags = std::move(_member_flags); }
    inline const StructMemberFlag& member_flags() const { return m_member_flags; }
    inline StructMemberFlag& member_flags() { return m_member_flags; }

    inline void member_type_id(const TypeIdentifier &_member_type_id) { m_member_type_id = _member_type_id; }
    inline void member_type_id(TypeIdentifier &&_member_type_id) { m_member_type_id = std::move(_member_type_id); }
    inline const TypeIdentifier& member_type_id() const { return m_member_type_id; }
    inline TypeIdentifier& member_type_id() { return m_member_type_id; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonStructMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteMemberDetail();
    ~CompleteMemberDetail();
    CompleteMemberDetail(const CompleteMemberDetail &x);
    CompleteMemberDetail(CompleteMemberDetail &&x);
    CompleteMemberDetail& operator=(const CompleteMemberDetail &x);
    CompleteMemberDetail& operator=(CompleteMemberDetail &&x);

    inline void name(const MemberName &_name) { m_name = _name; }
    inline void name(MemberName &&_name) { m_name = std::move(_name); }
    inline const MemberName& name() const { return m_name; }
    inline MemberName& name() { return m_name; }

    inline void ann_builtin(const AppliedBuiltinMemberAnnotations &_ann_builtin) { m_ann_builtin = _ann_builtin; }
    inline void ann_builtin(AppliedBuiltinMemberAnnotations &&_ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    inline const AppliedBuiltinMemberAnnotations& ann_builtin() const { return m_ann_builtin; }
    inline AppliedBuiltinMemberAnnotations& ann_builtin() { return m_ann_builtin; }

    inline void ann_custom(const AppliedAnnotationSeq &_ann_custom) { m_ann_custom = _ann_custom; }
    inline void ann_custom(AppliedAnnotationSeq &&_ann_custom) { m_ann_custom = std::move(_ann_custom); }
    inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteMemberDetail& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalMemberDetail();
    ~MinimalMemberDetail();
    MinimalMemberDetail(const MinimalMemberDetail &x);
    MinimalMemberDetail(MinimalMemberDetail &&x);
    MinimalMemberDetail& operator=(const MinimalMemberDetail &x);
    MinimalMemberDetail& operator=(MinimalMemberDetail &&x);

    inline void name_hash(const NameHash &_name_hash) { m_name_hash = _name_hash; }
    inline void name_hash(NameHash &&_name_hash) { m_name_hash = std::move(_name_hash); }
    inline const NameHash& name_hash() const { return m_name_hash; }
    inline NameHash& name_hash() { return m_name_hash; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalMemberDetail& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteStructMember();
    ~CompleteStructMember();
    CompleteStructMember(const CompleteStructMember &x);
    CompleteStructMember(CompleteStructMember &&x);
    CompleteStructMember& operator=(const CompleteStructMember &x);
    CompleteStructMember& operator=(CompleteStructMember &&x);

    inline void common(const CommonStructMember &_common) { m_common = _common; }
    inline void common(CommonStructMember &&_common) { m_common = std::move(_common); }
    inline const CommonStructMember& common() const { return m_common; }
    inline CommonStructMember& common() { return m_common; }

    inline void detail(const CompleteMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteMemberDetail& detail() const { return m_detail; }
    inline CompleteMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteStructMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalStructMember();
    ~MinimalStructMember();
    MinimalStructMember(const MinimalStructMember &x);
    MinimalStructMember(MinimalStructMember &&x);
    MinimalStructMember& operator=(const MinimalStructMember &x);
    MinimalStructMember& operator=(MinimalStructMember &&x);

    inline void common(const CommonStructMember &_common) { m_common = _common; }
    inline void common(CommonStructMember &&_common) { m_common = std::move(_common); }
    inline const CommonStructMember& common() const { return m_common; }
    inline CommonStructMember& common() { return m_common; }

    inline void detail(const MinimalMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(MinimalMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const MinimalMemberDetail& detail() const { return m_detail; }
    inline MinimalMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalStructMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CommonStructMember m_common;
    MinimalMemberDetail m_detail;
};

// Ordered by common.member_id
typedef std::vector<MinimalStructMember> MinimalStructMemberSeq;

/*struct AppliedBuiltinTypeAnnotations {
	AppliedVerbatimAnnotation verbatim;  // @verbatim(...) // @optional
};*/
class AppliedBuiltinTypeAnnotations
{
public:
    AppliedBuiltinTypeAnnotations();
    ~AppliedBuiltinTypeAnnotations();
    AppliedBuiltinTypeAnnotations(const AppliedBuiltinTypeAnnotations &x);
    AppliedBuiltinTypeAnnotations(AppliedBuiltinTypeAnnotations &&x);
    AppliedBuiltinTypeAnnotations& operator=(const AppliedBuiltinTypeAnnotations &x);
    AppliedBuiltinTypeAnnotations& operator=(AppliedBuiltinTypeAnnotations &&x);

    inline void verbatim(const AppliedVerbatimAnnotation &_verbatim) { m_verbatim = _verbatim; }
    inline void verbatim(AppliedVerbatimAnnotation &&_verbatim) { m_verbatim = std::move(_verbatim); }
    inline const AppliedVerbatimAnnotation& verbatim() const { return m_verbatim; }
    inline AppliedVerbatimAnnotation& verbatim() { return m_verbatim; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const AppliedBuiltinTypeAnnotations& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    AppliedVerbatimAnnotation m_verbatim;
};

/*struct MinimalTypeDetail final{
	// Empty. Available for future extension
};*/
class MinimalTypeDetail
{
public:
    MinimalTypeDetail();
    ~MinimalTypeDetail();
    MinimalTypeDetail(const MinimalTypeDetail &x);
    MinimalTypeDetail(MinimalTypeDetail &&x);
    MinimalTypeDetail& operator=(const MinimalTypeDetail &x);
    MinimalTypeDetail& operator=(MinimalTypeDetail &&x);

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalTypeDetail& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
};

/*struct CompleteTypeDetail final{
	AppliedBuiltinTypeAnnotations  ann_builtin; // @optional
	AppliedAnnotationSeq           ann_custom; // @optional
	QualifiedTypeName                        type_name;
};*/
class CompleteTypeDetail
{
public:
    CompleteTypeDetail();
    ~CompleteTypeDetail();
    CompleteTypeDetail(const CompleteTypeDetail &x);
    CompleteTypeDetail(CompleteTypeDetail &&x);
    CompleteTypeDetail& operator=(const CompleteTypeDetail &x);
    CompleteTypeDetail& operator=(CompleteTypeDetail &&x);

    inline void ann_builtin(const AppliedBuiltinTypeAnnotations &_ann_builtin) { m_ann_builtin = _ann_builtin; }
    inline void ann_builtin(AppliedBuiltinTypeAnnotations &&_ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    inline const AppliedBuiltinTypeAnnotations& ann_builtin() const { return m_ann_builtin; }
    inline AppliedBuiltinTypeAnnotations& ann_builtin() { return m_ann_builtin; }

    inline void ann_custom(const AppliedAnnotationSeq &_ann_custom) { m_ann_custom = _ann_custom; }
    inline void ann_custom(AppliedAnnotationSeq &&_ann_custom) { m_ann_custom = std::move(_ann_custom); }
    inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    inline void type_name(const QualifiedTypeName &_type_name) { m_type_name = _type_name; }
    inline void type_name(QualifiedTypeName &&_type_name) { m_type_name = std::move(_type_name); }
    inline const QualifiedTypeName& type_name() const { return m_type_name; }
    inline QualifiedTypeName& type_name() { return m_type_name; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteTypeDetail& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteStructHeader();
    ~CompleteStructHeader();
    CompleteStructHeader(const CompleteStructHeader &x);
    CompleteStructHeader(CompleteStructHeader &&x);
    CompleteStructHeader& operator=(const CompleteStructHeader &x);
    CompleteStructHeader& operator=(CompleteStructHeader &&x);

    inline void base_type(const TypeIdentifier &_base_type) { m_base_type = _base_type; }
    inline void base_type(TypeIdentifier &&_base_type) { m_base_type = std::move(_base_type); }
    inline const TypeIdentifier& base_type() const { return m_base_type; }
    inline TypeIdentifier& base_type() { return m_base_type; }

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteStructHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalStructHeader();
    ~MinimalStructHeader();
    MinimalStructHeader(const MinimalStructHeader &x);
    MinimalStructHeader(MinimalStructHeader &&x);
    MinimalStructHeader& operator=(const MinimalStructHeader &x);
    MinimalStructHeader& operator=(MinimalStructHeader &&x);

    inline void base_type(const TypeIdentifier &_base_type) { m_base_type = _base_type; }
    inline void base_type(TypeIdentifier &&_base_type) { m_base_type = std::move(_base_type); }
    inline const TypeIdentifier& base_type() const { return m_base_type; }
    inline TypeIdentifier& base_type() { return m_base_type; }

    inline void detail(const MinimalTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(MinimalTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const MinimalTypeDetail& detail() const { return m_detail; }
    inline MinimalTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalStructHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteStructType();
    ~CompleteStructType();
    CompleteStructType(const CompleteStructType &x);
    CompleteStructType(CompleteStructType &&x);
    CompleteStructType& operator=(const CompleteStructType &x);
    CompleteStructType& operator=(CompleteStructType &&x);

    inline void struct_flags(const StructTypeFlag &_struct_flags) { m_struct_flags = _struct_flags; }
    inline void struct_flags(StructTypeFlag &&_struct_flags) { m_struct_flags = std::move(_struct_flags); }
    inline const StructTypeFlag& struct_flags() const { return m_struct_flags; }
    inline StructTypeFlag& struct_flags() { return m_struct_flags; }

    inline void header(const CompleteStructHeader &_header) { m_header = _header; }
    inline void header(CompleteStructHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteStructHeader& header() const { return m_header; }
    inline CompleteStructHeader& header() { return m_header; }

    inline void member_seq(const CompleteStructMemberSeq &_member_seq) { m_member_seq = _member_seq; }
    inline void member_seq(CompleteStructMemberSeq &&_member_seq) { m_member_seq = std::move(_member_seq); }
    inline const CompleteStructMemberSeq& member_seq() const { return m_member_seq; }
    inline CompleteStructMemberSeq& member_seq() { return m_member_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteStructType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalStructType();
    ~MinimalStructType();
    MinimalStructType(const MinimalStructType &x);
    MinimalStructType(MinimalStructType &&x);
    MinimalStructType& operator=(const MinimalStructType &x);
    MinimalStructType& operator=(MinimalStructType &&x);

    inline void struct_flags(const StructTypeFlag &_struct_flags) { m_struct_flags = _struct_flags; }
    inline void struct_flags(StructTypeFlag &&_struct_flags) { m_struct_flags = std::move(_struct_flags); }
    inline const StructTypeFlag& struct_flags() const { return m_struct_flags; }
    inline StructTypeFlag& struct_flags() { return m_struct_flags; }

    inline void header(const MinimalStructHeader &_header) { m_header = _header; }
    inline void header(MinimalStructHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalStructHeader& header() const { return m_header; }
    inline MinimalStructHeader& header() { return m_header; }

    inline void member_seq(const MinimalStructMemberSeq &_member_seq) { m_member_seq = _member_seq; }
    inline void member_seq(MinimalStructMemberSeq &&_member_seq) { m_member_seq = std::move(_member_seq); }
    inline const MinimalStructMemberSeq& member_seq() const { return m_member_seq; }
    inline MinimalStructMemberSeq& member_seq() { return m_member_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalStructType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonUnionMember();
    ~CommonUnionMember();
    CommonUnionMember(const CommonUnionMember &x);
    CommonUnionMember(CommonUnionMember &&x);
    CommonUnionMember& operator=(const CommonUnionMember &x);
    CommonUnionMember& operator=(CommonUnionMember &&x);

    inline void member_id(const MemberId &_member_id) { m_member_id = _member_id; }
    inline void member_id(MemberId &&_member_id) { m_member_id = std::move(_member_id); }
    inline const MemberId& member_id() const { return m_member_id; }
    inline MemberId& member_id() { return m_member_id; }

    inline void member_flags(const UnionMemberFlag &_member_flags) { m_member_flags = _member_flags; }
    inline void member_flags(UnionMemberFlag &&_member_flags) { m_member_flags = std::move(_member_flags); }
    inline const UnionMemberFlag& member_flags() const { return m_member_flags; }
    inline UnionMemberFlag& member_flags() { return m_member_flags; }

    inline void type_id(const TypeIdentifier &_type_id) { m_type_id = _type_id; }
    inline void type_id(TypeIdentifier &&_type_id) { m_type_id = std::move(_type_id); }
    inline const TypeIdentifier& type_id() const { return m_type_id; }
    inline TypeIdentifier& type_id() { return m_type_id; }

    inline void label_seq(const UnionCaseLabelSeq &_label_seq) { m_label_seq = _label_seq; }
    inline void label_seq(UnionCaseLabelSeq &&_label_seq) { m_label_seq = std::move(_label_seq); }
    inline const UnionCaseLabelSeq& label_seq() const { return m_label_seq; }
    inline UnionCaseLabelSeq& label_seq() { return m_label_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonUnionMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteUnionMember();
    ~CompleteUnionMember();
    CompleteUnionMember(const CompleteUnionMember &x);
    CompleteUnionMember(CompleteUnionMember &&x);
    CompleteUnionMember& operator=(const CompleteUnionMember &x);
    CompleteUnionMember& operator=(CompleteUnionMember &&x);

    inline void common(const CommonUnionMember &_common) { m_common = _common; }
    inline void common(CommonUnionMember &&_common) { m_common = std::move(_common); }
    inline const CommonUnionMember& common() const { return m_common; }
    inline CommonUnionMember& common() { return m_common; }

    inline void detail(const CompleteMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteMemberDetail& detail() const { return m_detail; }
    inline CompleteMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteUnionMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalUnionMember();
    ~MinimalUnionMember();
    MinimalUnionMember(const MinimalUnionMember &x);
    MinimalUnionMember(MinimalUnionMember &&x);
    MinimalUnionMember& operator=(const MinimalUnionMember &x);
    MinimalUnionMember& operator=(MinimalUnionMember &&x);

    inline void common(const CommonUnionMember &_common) { m_common = _common; }
    inline void common(CommonUnionMember &&_common) { m_common = std::move(_common); }
    inline const CommonUnionMember& common() const { return m_common; }
    inline CommonUnionMember& common() { return m_common; }

    inline void detail(const MinimalMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(MinimalMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const MinimalMemberDetail& detail() const { return m_detail; }
    inline MinimalMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalUnionMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonDiscriminatorMember();
    ~CommonDiscriminatorMember();
    CommonDiscriminatorMember(const CommonDiscriminatorMember &x);
    CommonDiscriminatorMember(CommonDiscriminatorMember &&x);
    CommonDiscriminatorMember& operator=(const CommonDiscriminatorMember &x);
    CommonDiscriminatorMember& operator=(CommonDiscriminatorMember &&x);

    inline void member_flags(const UnionDiscriminatorFlag &_member_flags) { m_member_flags = _member_flags; }
    inline void member_flags(UnionDiscriminatorFlag &&_member_flags) { m_member_flags = std::move(_member_flags); }
    inline const UnionDiscriminatorFlag& member_flags() const { return m_member_flags; }
    inline UnionDiscriminatorFlag& member_flags() { return m_member_flags; }

    inline void type_id(const TypeIdentifier &_type_id) { m_type_id = _type_id; }
    inline void type_id(TypeIdentifier &&_type_id) { m_type_id = std::move(_type_id); }
    inline const TypeIdentifier& type_id() const { return m_type_id; }
    inline TypeIdentifier& type_id() { return m_type_id; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonDiscriminatorMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteDiscriminatorMember();
    ~CompleteDiscriminatorMember();
    CompleteDiscriminatorMember(const CompleteDiscriminatorMember &x);
    CompleteDiscriminatorMember(CompleteDiscriminatorMember &&x);
    CompleteDiscriminatorMember& operator=(const CompleteDiscriminatorMember &x);
    CompleteDiscriminatorMember& operator=(CompleteDiscriminatorMember &&x);

    inline void common(const CommonDiscriminatorMember &_common) { m_common = _common; }
    inline void common(CommonDiscriminatorMember &&_common) { m_common = std::move(_common); }
    inline const CommonDiscriminatorMember& common() const { return m_common; }
    inline CommonDiscriminatorMember& common() { return m_common; }

    inline void ann_builtin(const AppliedBuiltinTypeAnnotations &_ann_builtin) { m_ann_builtin = _ann_builtin; }
    inline void ann_builtin(AppliedBuiltinTypeAnnotations &&_ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    inline const AppliedBuiltinTypeAnnotations& ann_builtin() const { return m_ann_builtin; }
    inline AppliedBuiltinTypeAnnotations& ann_builtin() { return m_ann_builtin; }

    inline void ann_custom(const AppliedAnnotationSeq &_ann_custom) { m_ann_custom = _ann_custom; }
    inline void ann_custom(AppliedAnnotationSeq &&_ann_custom) { m_ann_custom = std::move(_ann_custom); }
    inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteDiscriminatorMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalDiscriminatorMember();
    ~MinimalDiscriminatorMember();
    MinimalDiscriminatorMember(const MinimalDiscriminatorMember &x);
    MinimalDiscriminatorMember(MinimalDiscriminatorMember &&x);
    MinimalDiscriminatorMember& operator=(const MinimalDiscriminatorMember &x);
    MinimalDiscriminatorMember& operator=(MinimalDiscriminatorMember &&x);

    inline void common(const CommonDiscriminatorMember &_common) { m_common = _common; }
    inline void common(CommonDiscriminatorMember &&_common) { m_common = std::move(_common); }
    inline const CommonDiscriminatorMember& common() const { return m_common; }
    inline CommonDiscriminatorMember& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalDiscriminatorMember& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CommonDiscriminatorMember m_common;
};

/*struct CompleteUnionHeader {
	CompleteTypeDetail          detail;
};*/
class CompleteUnionHeader
{
public:
    CompleteUnionHeader();
    ~CompleteUnionHeader();
    CompleteUnionHeader(const CompleteUnionHeader &x);
    CompleteUnionHeader(CompleteUnionHeader &&x);
    CompleteUnionHeader& operator=(const CompleteUnionHeader &x);
    CompleteUnionHeader& operator=(CompleteUnionHeader &&x);

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteUnionHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CompleteTypeDetail m_detail;
};

/*struct MinimalUnionHeader {
	MinimalTypeDetail           detail;
};*/
class MinimalUnionHeader
{
public:
    MinimalUnionHeader();
    ~MinimalUnionHeader();
    MinimalUnionHeader(const MinimalUnionHeader &x);
    MinimalUnionHeader(MinimalUnionHeader &&x);
    MinimalUnionHeader& operator=(const MinimalUnionHeader &x);
    MinimalUnionHeader& operator=(MinimalUnionHeader &&x);

    inline void detail(const MinimalTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(MinimalTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const MinimalTypeDetail& detail() const { return m_detail; }
    inline MinimalTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalUnionHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteUnionType();
    ~CompleteUnionType();
    CompleteUnionType(const CompleteUnionType &x);
    CompleteUnionType(CompleteUnionType &&x);
    CompleteUnionType& operator=(const CompleteUnionType &x);
    CompleteUnionType& operator=(CompleteUnionType &&x);

    inline void union_flags(const UnionTypeFlag &_union_flags) { m_union_flags = _union_flags; }
    inline void union_flags(UnionTypeFlag &&_union_flags) { m_union_flags = std::move(_union_flags); }
    inline const UnionTypeFlag& union_flags() const { return m_union_flags; }
    inline UnionTypeFlag& union_flags() { return m_union_flags; }

    inline void header(const CompleteUnionHeader &_header) { m_header = _header; }
    inline void header(CompleteUnionHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteUnionHeader& header() const { return m_header; }
    inline CompleteUnionHeader& header() { return m_header; }

    inline void discriminator(const CompleteDiscriminatorMember &_discriminator) { m_discriminator = _discriminator; }
    inline void discriminator(CompleteDiscriminatorMember &&_discriminator) { m_discriminator = std::move(_discriminator); }
    inline const CompleteDiscriminatorMember& discriminator() const { return m_discriminator; }
    inline CompleteDiscriminatorMember& discriminator() { return m_discriminator; }

    inline void member_seq(const CompleteUnionMemberSeq &_member_seq) { m_member_seq = _member_seq; }
    inline void member_seq(CompleteUnionMemberSeq &&_member_seq) { m_member_seq = std::move(_member_seq); }
    inline const CompleteUnionMemberSeq& member_seq() const { return m_member_seq; }
    inline CompleteUnionMemberSeq& member_seq() { return m_member_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteUnionType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalUnionType();
    ~MinimalUnionType();
    MinimalUnionType(const MinimalUnionType &x);
    MinimalUnionType(MinimalUnionType &&x);
    MinimalUnionType& operator=(const MinimalUnionType &x);
    MinimalUnionType& operator=(MinimalUnionType &&x);

    inline void union_flags(const UnionTypeFlag &_union_flags) { m_union_flags = _union_flags; }
    inline void union_flags(UnionTypeFlag &&_union_flags) { m_union_flags = std::move(_union_flags); }
    inline const UnionTypeFlag& union_flags() const { return m_union_flags; }
    inline UnionTypeFlag& union_flags() { return m_union_flags; }

    inline void header(const MinimalUnionHeader &_header) { m_header = _header; }
    inline void header(MinimalUnionHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalUnionHeader& header() const { return m_header; }
    inline MinimalUnionHeader& header() { return m_header; }

    inline void discriminator(const MinimalDiscriminatorMember &_discriminator) { m_discriminator = _discriminator; }
    inline void discriminator(MinimalDiscriminatorMember &&_discriminator) { m_discriminator = std::move(_discriminator); }
    inline const MinimalDiscriminatorMember& discriminator() const { return m_discriminator; }
    inline MinimalDiscriminatorMember& discriminator() { return m_discriminator; }

    inline void member_seq(const MinimalUnionMemberSeq &_member_seq) { m_member_seq = _member_seq; }
    inline void member_seq(MinimalUnionMemberSeq &&_member_seq) { m_member_seq = std::move(_member_seq); }
    inline const MinimalUnionMemberSeq& member_seq() const { return m_member_seq; }
    inline MinimalUnionMemberSeq& member_seq() { return m_member_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalUnionType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonAnnotationParameter();
    ~CommonAnnotationParameter();
    CommonAnnotationParameter(const CommonAnnotationParameter &x);
    CommonAnnotationParameter(CommonAnnotationParameter &&x);
    CommonAnnotationParameter& operator=(const CommonAnnotationParameter &x);
    CommonAnnotationParameter& operator=(CommonAnnotationParameter &&x);

    inline void member_flags(const AnnotationParameterFlag &_member_flags) { m_member_flags = _member_flags; }
    inline void member_flags(AnnotationParameterFlag &&_member_flags) { m_member_flags = std::move(_member_flags); }
    inline const AnnotationParameterFlag& member_flags() const { return m_member_flags; }
    inline AnnotationParameterFlag& member_flags() { return m_member_flags; }

    inline void member_type_id(const TypeIdentifier &_member_type_id) { m_member_type_id = _member_type_id; }
    inline void member_type_id(TypeIdentifier &&_member_type_id) { m_member_type_id = std::move(_member_type_id); }
    inline const TypeIdentifier& member_type_id() const { return m_member_type_id; }
    inline TypeIdentifier& member_type_id() { return m_member_type_id; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonAnnotationParameter& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteAnnotationParameter();
    ~CompleteAnnotationParameter();
    CompleteAnnotationParameter(const CompleteAnnotationParameter &x);
    CompleteAnnotationParameter(CompleteAnnotationParameter &&x);
    CompleteAnnotationParameter& operator=(const CompleteAnnotationParameter &x);
    CompleteAnnotationParameter& operator=(CompleteAnnotationParameter &&x);

    inline void common(const CommonAnnotationParameter &_common) { m_common = _common; }
    inline void common(CommonAnnotationParameter &&_common) { m_common = std::move(_common); }
    inline const CommonAnnotationParameter& common() const { return m_common; }
    inline CommonAnnotationParameter& common() { return m_common; }

    inline void name(const MemberName &_name) { m_name = _name; }
    inline void name(MemberName &&_name) { m_name = std::move(_name); }
    inline const MemberName& name() const { return m_name; }
    inline MemberName& name() { return m_name; }

    inline void default_value(const AnnotationParameterValue &_default_value) { m_default_value = _default_value; }
    inline void default_value(AnnotationParameterValue &&_default_value) { m_default_value = std::move(_default_value); }
    inline const AnnotationParameterValue& default_value() const { return m_default_value; }
    inline AnnotationParameterValue& default_value() { return m_default_value; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteAnnotationParameter& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalAnnotationParameter();
    ~MinimalAnnotationParameter();
    MinimalAnnotationParameter(const MinimalAnnotationParameter &x);
    MinimalAnnotationParameter(MinimalAnnotationParameter &&x);
    MinimalAnnotationParameter& operator=(const MinimalAnnotationParameter &x);
    MinimalAnnotationParameter& operator=(MinimalAnnotationParameter &&x);

    inline void common(const CommonAnnotationParameter &_common) { m_common = _common; }
    inline void common(CommonAnnotationParameter &&_common) { m_common = std::move(_common); }
    inline const CommonAnnotationParameter& common() const { return m_common; }
    inline CommonAnnotationParameter& common() { return m_common; }

    inline void name(const MemberName &_name) { m_name = _name; }
    inline void name(MemberName &&_name) { m_name = std::move(_name); }
    inline const MemberName& name() const { return m_name; }
    inline MemberName& name() { return m_name; }

    inline void default_value(const AnnotationParameterValue &_default_value) { m_default_value = _default_value; }
    inline void default_value(AnnotationParameterValue &&_default_value) { m_default_value = std::move(_default_value); }
    inline const AnnotationParameterValue& default_value() const { return m_default_value; }
    inline AnnotationParameterValue& default_value() { return m_default_value; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalAnnotationParameter& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteAnnotationHeader();
    ~CompleteAnnotationHeader();
    CompleteAnnotationHeader(const CompleteAnnotationHeader &x);
    CompleteAnnotationHeader(CompleteAnnotationHeader &&x);
    CompleteAnnotationHeader& operator=(const CompleteAnnotationHeader &x);
    CompleteAnnotationHeader& operator=(CompleteAnnotationHeader &&x);

    inline void annotation_name(const QualifiedTypeName &_annotation_name) { m_annotation_name = _annotation_name; }
    inline void annotation_name(QualifiedTypeName &&_annotation_name) { m_annotation_name = std::move(_annotation_name); }
    inline const QualifiedTypeName& annotation_name() const { return m_annotation_name; }
    inline QualifiedTypeName& annotation_name() { return m_annotation_name; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteAnnotationHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    QualifiedTypeName m_annotation_name;
};

/*struct MinimalAnnotationHeader {
	// Empty. Available for future extension
};*/
class MinimalAnnotationHeader
{
public:
    MinimalAnnotationHeader();
    ~MinimalAnnotationHeader();
    MinimalAnnotationHeader(const MinimalAnnotationHeader &x);
    MinimalAnnotationHeader(MinimalAnnotationHeader &&x);
    MinimalAnnotationHeader& operator=(const MinimalAnnotationHeader &x);
    MinimalAnnotationHeader& operator=(MinimalAnnotationHeader &&x);

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalAnnotationHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteAnnotationType();
    ~CompleteAnnotationType();
    CompleteAnnotationType(const CompleteAnnotationType &x);
    CompleteAnnotationType(CompleteAnnotationType &&x);
    CompleteAnnotationType& operator=(const CompleteAnnotationType &x);
    CompleteAnnotationType& operator=(CompleteAnnotationType &&x);

    inline void annotation_flag(const AnnotationTypeFlag &_annotation_flag) { m_annotation_flag = _annotation_flag; }
    inline void annotation_flag(AnnotationTypeFlag &&_annotation_flag) { m_annotation_flag = std::move(_annotation_flag); }
    inline const AnnotationTypeFlag& annotation_flag() const { return m_annotation_flag; }
    inline AnnotationTypeFlag& annotation_flag() { return m_annotation_flag; }

    inline void header(const CompleteAnnotationHeader &_header) { m_header = _header; }
    inline void header(CompleteAnnotationHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteAnnotationHeader& header() const { return m_header; }
    inline CompleteAnnotationHeader& header() { return m_header; }

    inline void member_seq(const CompleteAnnotationParameterSeq &_member_seq) { m_member_seq = _member_seq; }
    inline void member_seq(CompleteAnnotationParameterSeq &&_member_seq) { m_member_seq = std::move(_member_seq); }
    inline const CompleteAnnotationParameterSeq& member_seq() const { return m_member_seq; }
    inline CompleteAnnotationParameterSeq& member_seq() { return m_member_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteAnnotationType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalAnnotationType();
    ~MinimalAnnotationType();
    MinimalAnnotationType(const MinimalAnnotationType &x);
    MinimalAnnotationType(MinimalAnnotationType &&x);
    MinimalAnnotationType& operator=(const MinimalAnnotationType &x);
    MinimalAnnotationType& operator=(MinimalAnnotationType &&x);

    inline void annotation_flag(const AnnotationTypeFlag &_annotation_flag) { m_annotation_flag = _annotation_flag; }
    inline void annotation_flag(AnnotationTypeFlag &&_annotation_flag) { m_annotation_flag = std::move(_annotation_flag); }
    inline const AnnotationTypeFlag& annotation_flag() const { return m_annotation_flag; }
    inline AnnotationTypeFlag& annotation_flag() { return m_annotation_flag; }

    inline void header(const MinimalAnnotationHeader &_header) { m_header = _header; }
    inline void header(MinimalAnnotationHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalAnnotationHeader& header() const { return m_header; }
    inline MinimalAnnotationHeader& header() { return m_header; }

    inline void member_seq(const MinimalAnnotationParameterSeq &_member_seq) { m_member_seq = _member_seq; }
    inline void member_seq(MinimalAnnotationParameterSeq &&_member_seq) { m_member_seq = std::move(_member_seq); }
    inline const MinimalAnnotationParameterSeq& member_seq() const { return m_member_seq; }
    inline MinimalAnnotationParameterSeq& member_seq() { return m_member_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalAnnotationType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonAliasBody();
    ~CommonAliasBody();
    CommonAliasBody(const CommonAliasBody &x);
    CommonAliasBody(CommonAliasBody &&x);
    CommonAliasBody& operator=(const CommonAliasBody &x);
    CommonAliasBody& operator=(CommonAliasBody &&x);

    inline void related_flags(const AliasMemberFlag &_related_flags) { m_related_flags = _related_flags; }
    inline void related_flags(AliasMemberFlag &&_related_flags) { m_related_flags = std::move(_related_flags); }
    inline const AliasMemberFlag& related_flags() const { return m_related_flags; }
    inline AliasMemberFlag& related_flags() { return m_related_flags; }

    inline void related_type(const TypeIdentifier &_related_type) { m_related_type = _related_type; }
    inline void related_type(TypeIdentifier &&_related_type) { m_related_type = std::move(_related_type); }
    inline const TypeIdentifier& related_type() const { return m_related_type; }
    inline TypeIdentifier& related_type() { return m_related_type; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonAliasBody& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteAliasBody();
    ~CompleteAliasBody();
    CompleteAliasBody(const CompleteAliasBody &x);
    CompleteAliasBody(CompleteAliasBody &&x);
    CompleteAliasBody& operator=(const CompleteAliasBody &x);
    CompleteAliasBody& operator=(CompleteAliasBody &&x);

    inline void common(const CommonAliasBody &_common) { m_common = _common; }
    inline void common(CommonAliasBody &&_common) { m_common = std::move(_common); }
    inline const CommonAliasBody& common() const { return m_common; }
    inline CommonAliasBody& common() { return m_common; }

    inline void ann_builtin(const AppliedBuiltinMemberAnnotations &_ann_builtin) { m_ann_builtin = _ann_builtin; }
    inline void ann_builtin(AppliedBuiltinMemberAnnotations &&_ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    inline const AppliedBuiltinMemberAnnotations& ann_builtin() const { return m_ann_builtin; }
    inline AppliedBuiltinMemberAnnotations& ann_builtin() { return m_ann_builtin; }

    inline void ann_custom(const AppliedAnnotationSeq &_ann_custom) { m_ann_custom = _ann_custom; }
    inline void ann_custom(AppliedAnnotationSeq &&_ann_custom) { m_ann_custom = std::move(_ann_custom); }
    inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteAliasBody& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalAliasBody();
    ~MinimalAliasBody();
    MinimalAliasBody(const MinimalAliasBody &x);
    MinimalAliasBody(MinimalAliasBody &&x);
    MinimalAliasBody& operator=(const MinimalAliasBody &x);
    MinimalAliasBody& operator=(MinimalAliasBody &&x);

    inline void common(const CommonAliasBody &_common) { m_common = _common; }
    inline void common(CommonAliasBody &&_common) { m_common = std::move(_common); }
    inline const CommonAliasBody& common() const { return m_common; }
    inline CommonAliasBody& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalAliasBody& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CommonAliasBody m_common;
};

/*struct CompleteAliasHeader {
	CompleteTypeDetail    detail;
};*/
class CompleteAliasHeader
{
public:
    CompleteAliasHeader();
    ~CompleteAliasHeader();
    CompleteAliasHeader(const CompleteAliasHeader &x);
    CompleteAliasHeader(CompleteAliasHeader &&x);
    CompleteAliasHeader& operator=(const CompleteAliasHeader &x);
    CompleteAliasHeader& operator=(CompleteAliasHeader &&x);

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteAliasHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CompleteTypeDetail m_detail;
};

/*struct MinimalAliasHeader {
	// Empty. Available for future extension
};*/
class MinimalAliasHeader
{
public:
    MinimalAliasHeader();
    ~MinimalAliasHeader();
    MinimalAliasHeader(const MinimalAliasHeader &x);
    MinimalAliasHeader(MinimalAliasHeader &&x);
    MinimalAliasHeader& operator=(const MinimalAliasHeader &x);
    MinimalAliasHeader& operator=(MinimalAliasHeader &&x);

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalAliasHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteAliasType();
    ~CompleteAliasType();
    CompleteAliasType(const CompleteAliasType &x);
    CompleteAliasType(CompleteAliasType &&x);
    CompleteAliasType& operator=(const CompleteAliasType &x);
    CompleteAliasType& operator=(CompleteAliasType &&x);

    inline void alias_flags(const AliasTypeFlag &_alias_flags) { m_alias_flags = _alias_flags; }
    inline void alias_flags(AliasTypeFlag &&_alias_flags) { m_alias_flags = std::move(_alias_flags); }
    inline const AliasTypeFlag& alias_flags() const { return m_alias_flags; }
    inline AliasTypeFlag& alias_flags() { return m_alias_flags; }

    inline void header(const CompleteAliasHeader &_header) { m_header = _header; }
    inline void header(CompleteAliasHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteAliasHeader& header() const { return m_header; }
    inline CompleteAliasHeader& header() { return m_header; }

    inline void body(const CompleteAliasBody &_body) { m_body = _body; }
    inline void body(CompleteAliasBody &&_body) { m_body = std::move(_body); }
    inline const CompleteAliasBody& body() const { return m_body; }
    inline CompleteAliasBody& body() { return m_body; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteAliasType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalAliasType();
    ~MinimalAliasType();
    MinimalAliasType(const MinimalAliasType &x);
    MinimalAliasType(MinimalAliasType &&x);
    MinimalAliasType& operator=(const MinimalAliasType &x);
    MinimalAliasType& operator=(MinimalAliasType &&x);

    inline void alias_flags(const AliasTypeFlag &_alias_flags) { m_alias_flags = _alias_flags; }
    inline void alias_flags(AliasTypeFlag &&_alias_flags) { m_alias_flags = std::move(_alias_flags); }
    inline const AliasTypeFlag& alias_flags() const { return m_alias_flags; }
    inline AliasTypeFlag& alias_flags() { return m_alias_flags; }

    inline void header(const MinimalAliasHeader &_header) { m_header = _header; }
    inline void header(MinimalAliasHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalAliasHeader& header() const { return m_header; }
    inline MinimalAliasHeader& header() { return m_header; }

    inline void body(const MinimalAliasBody &_body) { m_body = _body; }
    inline void body(MinimalAliasBody &&_body) { m_body = std::move(_body); }
    inline const MinimalAliasBody& body() const { return m_body; }
    inline MinimalAliasBody& body() { return m_body; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalAliasType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteElementDetail();
    ~CompleteElementDetail();
    CompleteElementDetail(const CompleteElementDetail &x);
    CompleteElementDetail(CompleteElementDetail &&x);
    CompleteElementDetail& operator=(const CompleteElementDetail &x);
    CompleteElementDetail& operator=(CompleteElementDetail &&x);

    inline void ann_builtin(const AppliedBuiltinMemberAnnotations &_ann_builtin) { m_ann_builtin = _ann_builtin; }
    inline void ann_builtin(AppliedBuiltinMemberAnnotations &&_ann_builtin) { m_ann_builtin = std::move(_ann_builtin); }
    inline const AppliedBuiltinMemberAnnotations& ann_builtin() const { return m_ann_builtin; }
    inline AppliedBuiltinMemberAnnotations& ann_builtin() { return m_ann_builtin; }

    inline void ann_custom(const AppliedAnnotationSeq &_ann_custom) { m_ann_custom = _ann_custom; }
    inline void ann_custom(AppliedAnnotationSeq &&_ann_custom) { m_ann_custom = std::move(_ann_custom); }
    inline const AppliedAnnotationSeq& ann_custom() const { return m_ann_custom; }
    inline AppliedAnnotationSeq& ann_custom() { return m_ann_custom; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteElementDetail& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonCollectionElement();
    ~CommonCollectionElement();
    CommonCollectionElement(const CommonCollectionElement &x);
    CommonCollectionElement(CommonCollectionElement &&x);
    CommonCollectionElement& operator=(const CommonCollectionElement &x);
    CommonCollectionElement& operator=(CommonCollectionElement &&x);

    inline void element_flags(const CollectionElementFlag &_element_flags) { m_element_flags = _element_flags; }
    inline void element_flags(CollectionElementFlag &&_element_flags) { m_element_flags = std::move(_element_flags); }
    inline const CollectionElementFlag& element_flags() const { return m_element_flags; }
    inline CollectionElementFlag& element_flags() { return m_element_flags; }

    inline void type(const TypeIdentifier &_type) { m_type = _type; }
    inline void type(TypeIdentifier &&_type) { m_type = std::move(_type); }
    inline const TypeIdentifier& type() const { return m_type; }
    inline TypeIdentifier& type() { return m_type; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonCollectionElement& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteCollectionElement();
    ~CompleteCollectionElement();
    CompleteCollectionElement(const CompleteCollectionElement &x);
    CompleteCollectionElement(CompleteCollectionElement &&x);
    CompleteCollectionElement& operator=(const CompleteCollectionElement &x);
    CompleteCollectionElement& operator=(CompleteCollectionElement &&x);

    inline void common(const CommonCollectionElement &_common) { m_common = _common; }
    inline void common(CommonCollectionElement &&_common) { m_common = std::move(_common); }
    inline const CommonCollectionElement& common() const { return m_common; }
    inline CommonCollectionElement& common() { return m_common; }

    inline void detail(const CompleteElementDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteElementDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteElementDetail& detail() const { return m_detail; }
    inline CompleteElementDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteCollectionElement& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalCollectionElement();
    ~MinimalCollectionElement();
    MinimalCollectionElement(const MinimalCollectionElement &x);
    MinimalCollectionElement(MinimalCollectionElement &&x);
    MinimalCollectionElement& operator=(const MinimalCollectionElement &x);
    MinimalCollectionElement& operator=(MinimalCollectionElement &&x);

    inline void common(const CommonCollectionElement &_common) { m_common = _common; }
    inline void common(CommonCollectionElement &&_common) { m_common = std::move(_common); }
    inline const CommonCollectionElement& common() const { return m_common; }
    inline CommonCollectionElement& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalCollectionElement& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CommonCollectionElement m_common;
};

/*struct CommonCollectionHeader final{
	LBound                    bound;
};*/
class CommonCollectionHeader
{
public:
    CommonCollectionHeader();
    ~CommonCollectionHeader();
    CommonCollectionHeader(const CommonCollectionHeader &x);
    CommonCollectionHeader(CommonCollectionHeader &&x);
    CommonCollectionHeader& operator=(const CommonCollectionHeader &x);
    CommonCollectionHeader& operator=(CommonCollectionHeader &&x);

    inline void bound(const LBound &_bound) { m_bound = _bound; }
    inline void bound(LBound &&_bound) { m_bound = std::move(_bound); }
    inline const LBound& bound() const { return m_bound; }
    inline LBound& bound() { return m_bound; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonCollectionHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteCollectionHeader();
    ~CompleteCollectionHeader();
    CompleteCollectionHeader(const CompleteCollectionHeader &x);
    CompleteCollectionHeader(CompleteCollectionHeader &&x);
    CompleteCollectionHeader& operator=(const CompleteCollectionHeader &x);
    CompleteCollectionHeader& operator=(CompleteCollectionHeader &&x);

    inline void common(const CommonCollectionHeader &_common) { m_common = _common; }
    inline void common(CommonCollectionHeader &&_common) { m_common = std::move(_common); }
    inline const CommonCollectionHeader& common() const { return m_common; }
    inline CommonCollectionHeader& common() { return m_common; }

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteCollectionHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalCollectionHeader();
    ~MinimalCollectionHeader();
    MinimalCollectionHeader(const MinimalCollectionHeader &x);
    MinimalCollectionHeader(MinimalCollectionHeader &&x);
    MinimalCollectionHeader& operator=(const MinimalCollectionHeader &x);
    MinimalCollectionHeader& operator=(MinimalCollectionHeader &&x);

    inline void common(const CommonCollectionHeader &_common) { m_common = _common; }
    inline void common(CommonCollectionHeader &&_common) { m_common = std::move(_common); }
    inline const CommonCollectionHeader& common() const { return m_common; }
    inline CommonCollectionHeader& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalCollectionHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteSequenceType();
    ~CompleteSequenceType();
    CompleteSequenceType(const CompleteSequenceType &x);
    CompleteSequenceType(CompleteSequenceType &&x);
    CompleteSequenceType& operator=(const CompleteSequenceType &x);
    CompleteSequenceType& operator=(CompleteSequenceType &&x);

    inline void collection_flag(const CollectionTypeFlag &_collection_flag) { m_collection_flag = _collection_flag; }
    inline void collection_flag(CollectionTypeFlag &&_collection_flag) { m_collection_flag = std::move(_collection_flag); }
    inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    inline void header(const CompleteCollectionHeader &_header) { m_header = _header; }
    inline void header(CompleteCollectionHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteCollectionHeader& header() const { return m_header; }
    inline CompleteCollectionHeader& header() { return m_header; }

    inline void element(const CompleteCollectionElement &_element) { m_element = _element; }
    inline void element(CompleteCollectionElement &&_element) { m_element = std::move(_element); }
    inline const CompleteCollectionElement& element() const { return m_element; }
    inline CompleteCollectionElement& element() { return m_element; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteSequenceType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalSequenceType();
    ~MinimalSequenceType();
    MinimalSequenceType(const MinimalSequenceType &x);
    MinimalSequenceType(MinimalSequenceType &&x);
    MinimalSequenceType& operator=(const MinimalSequenceType &x);
    MinimalSequenceType& operator=(MinimalSequenceType &&x);

    inline void collection_flag(const CollectionTypeFlag &_collection_flag) { m_collection_flag = _collection_flag; }
    inline void collection_flag(CollectionTypeFlag &&_collection_flag) { m_collection_flag = std::move(_collection_flag); }
    inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    inline void header(const MinimalCollectionHeader &_header) { m_header = _header; }
    inline void header(MinimalCollectionHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalCollectionHeader& header() const { return m_header; }
    inline MinimalCollectionHeader& header() { return m_header; }

    inline void element(const MinimalCollectionElement &_element) { m_element = _element; }
    inline void element(MinimalCollectionElement &&_element) { m_element = std::move(_element); }
    inline const MinimalCollectionElement& element() const { return m_element; }
    inline MinimalCollectionElement& element() { return m_element; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalSequenceType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonArrayHeader();
    ~CommonArrayHeader();
    CommonArrayHeader(const CommonArrayHeader &x);
    CommonArrayHeader(CommonArrayHeader &&x);
    CommonArrayHeader& operator=(const CommonArrayHeader &x);
    CommonArrayHeader& operator=(CommonArrayHeader &&x);

    inline void bound_seq(const LBoundSeq &_bound_seq) { m_bound_seq = _bound_seq; }
    inline void bound_seq(LBoundSeq &&_bound_seq) { m_bound_seq = std::move(_bound_seq); }
    inline const LBoundSeq& bound_seq() const { return m_bound_seq; }
    inline LBoundSeq& bound_seq() { return m_bound_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonArrayHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteArrayHeader();
    ~CompleteArrayHeader();
    CompleteArrayHeader(const CompleteArrayHeader &x);
    CompleteArrayHeader(CompleteArrayHeader &&x);
    CompleteArrayHeader& operator=(const CompleteArrayHeader &x);
    CompleteArrayHeader& operator=(CompleteArrayHeader &&x);

    inline void common(const CommonArrayHeader &_common) { m_common = _common; }
    inline void common(CommonArrayHeader &&_common) { m_common = std::move(_common); }
    inline const CommonArrayHeader& common() const { return m_common; }
    inline CommonArrayHeader& common() { return m_common; }

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteArrayHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalArrayHeader();
    ~MinimalArrayHeader();
    MinimalArrayHeader(const MinimalArrayHeader &x);
    MinimalArrayHeader(MinimalArrayHeader &&x);
    MinimalArrayHeader& operator=(const MinimalArrayHeader &x);
    MinimalArrayHeader& operator=(MinimalArrayHeader &&x);

    inline void common(const CommonArrayHeader &_common) { m_common = _common; }
    inline void common(CommonArrayHeader &&_common) { m_common = std::move(_common); }
    inline const CommonArrayHeader& common() const { return m_common; }
    inline CommonArrayHeader& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalArrayHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteArrayType();
    ~CompleteArrayType();
    CompleteArrayType(const CompleteArrayType &x);
    CompleteArrayType(CompleteArrayType &&x);
    CompleteArrayType& operator=(const CompleteArrayType &x);
    CompleteArrayType& operator=(CompleteArrayType &&x);

    inline void collection_flag(const CollectionTypeFlag &_collection_flag) { m_collection_flag = _collection_flag; }
    inline void collection_flag(CollectionTypeFlag &&_collection_flag) { m_collection_flag = std::move(_collection_flag); }
    inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    inline void header(const CompleteArrayHeader &_header) { m_header = _header; }
    inline void header(CompleteArrayHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteArrayHeader& header() const { return m_header; }
    inline CompleteArrayHeader& header() { return m_header; }

    inline void element(const CompleteCollectionElement &_element) { m_element = _element; }
    inline void element(CompleteCollectionElement &&_element) { m_element = std::move(_element); }
    inline const CompleteCollectionElement& element() const { return m_element; }
    inline CompleteCollectionElement& element() { return m_element; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteArrayType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalArrayType();
    ~MinimalArrayType();
    MinimalArrayType(const MinimalArrayType &x);
    MinimalArrayType(MinimalArrayType &&x);
    MinimalArrayType& operator=(const MinimalArrayType &x);
    MinimalArrayType& operator=(MinimalArrayType &&x);

    inline void collection_flag(const CollectionTypeFlag &_collection_flag) { m_collection_flag = _collection_flag; }
    inline void collection_flag(CollectionTypeFlag &&_collection_flag) { m_collection_flag = std::move(_collection_flag); }
    inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    inline void header(const MinimalArrayHeader &_header) { m_header = _header; }
    inline void header(MinimalArrayHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalArrayHeader& header() const { return m_header; }
    inline MinimalArrayHeader& header() { return m_header; }

    inline void element(const MinimalCollectionElement &_element) { m_element = _element; }
    inline void element(MinimalCollectionElement &&_element) { m_element = std::move(_element); }
    inline const MinimalCollectionElement& element() const { return m_element; }
    inline MinimalCollectionElement& element() { return m_element; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalArrayType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteMapType();
    ~CompleteMapType();
    CompleteMapType(const CompleteMapType &x);
    CompleteMapType(CompleteMapType &&x);
    CompleteMapType& operator=(const CompleteMapType &x);
    CompleteMapType& operator=(CompleteMapType &&x);

    inline void collection_flag(const CollectionTypeFlag &_collection_flag) { m_collection_flag = _collection_flag; }
    inline void collection_flag(CollectionTypeFlag &&_collection_flag) { m_collection_flag = std::move(_collection_flag); }
    inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    inline void header(const CompleteCollectionHeader &_header) { m_header = _header; }
    inline void header(CompleteCollectionHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteCollectionHeader& header() const { return m_header; }
    inline CompleteCollectionHeader& header() { return m_header; }

    inline void key(const CompleteCollectionElement &_key) { m_key = _key; }
    inline void key(CompleteCollectionElement &&_key) { m_key = std::move(_key); }
    inline const CompleteCollectionElement& key() const { return m_key; }
    inline CompleteCollectionElement& key() { return m_key; }

    inline void element(const CompleteCollectionElement &_element) { m_element = _element; }
    inline void element(CompleteCollectionElement &&_element) { m_element = std::move(_element); }
    inline const CompleteCollectionElement& element() const { return m_element; }
    inline CompleteCollectionElement& element() { return m_element; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteMapType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalMapType();
    ~MinimalMapType();
    MinimalMapType(const MinimalMapType &x);
    MinimalMapType(MinimalMapType &&x);
    MinimalMapType& operator=(const MinimalMapType &x);
    MinimalMapType& operator=(MinimalMapType &&x);

    inline void collection_flag(const CollectionTypeFlag &_collection_flag) { m_collection_flag = _collection_flag; }
    inline void collection_flag(CollectionTypeFlag &&_collection_flag) { m_collection_flag = std::move(_collection_flag); }
    inline const CollectionTypeFlag& collection_flag() const { return m_collection_flag; }
    inline CollectionTypeFlag& collection_flag() { return m_collection_flag; }

    inline void header(const MinimalCollectionHeader &_header) { m_header = _header; }
    inline void header(MinimalCollectionHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalCollectionHeader& header() const { return m_header; }
    inline MinimalCollectionHeader& header() { return m_header; }

    inline void key(const MinimalCollectionElement &_key) { m_key = _key; }
    inline void key(MinimalCollectionElement &&_key) { m_key = std::move(_key); }
    inline const MinimalCollectionElement& key() const { return m_key; }
    inline MinimalCollectionElement& key() { return m_key; }

    inline void element(const MinimalCollectionElement &_element) { m_element = _element; }
    inline void element(MinimalCollectionElement &&_element) { m_element = std::move(_element); }
    inline const MinimalCollectionElement& element() const { return m_element; }
    inline MinimalCollectionElement& element() { return m_element; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalMapType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonEnumeratedLiteral();
    ~CommonEnumeratedLiteral();
    CommonEnumeratedLiteral(const CommonEnumeratedLiteral &x);
    CommonEnumeratedLiteral(CommonEnumeratedLiteral &&x);
    CommonEnumeratedLiteral& operator=(const CommonEnumeratedLiteral &x);
    CommonEnumeratedLiteral& operator=(CommonEnumeratedLiteral &&x);

    inline void value(const int32_t &_value) { m_value = _value; }
    inline void value(int32_t &&_value) { m_value = std::move(_value); }
    inline const int32_t& value() const { return m_value; }
    inline int32_t& value() { return m_value; }

    inline void flags(const EnumeratedLiteralFlag &_flags) { m_flags = _flags; }
    inline void flags(EnumeratedLiteralFlag &&_flags) { m_flags = std::move(_flags); }
    inline const EnumeratedLiteralFlag& flags() const { return m_flags; }
    inline EnumeratedLiteralFlag& flags() { return m_flags; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonEnumeratedLiteral& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteEnumeratedLiteral();
    ~CompleteEnumeratedLiteral();
    CompleteEnumeratedLiteral(const CompleteEnumeratedLiteral &x);
    CompleteEnumeratedLiteral(CompleteEnumeratedLiteral &&x);
    CompleteEnumeratedLiteral& operator=(const CompleteEnumeratedLiteral &x);
    CompleteEnumeratedLiteral& operator=(CompleteEnumeratedLiteral &&x);

    inline void common(const CommonEnumeratedLiteral &_common) { m_common = _common; }
    inline void common(CommonEnumeratedLiteral &&_common) { m_common = std::move(_common); }
    inline const CommonEnumeratedLiteral& common() const { return m_common; }
    inline CommonEnumeratedLiteral& common() { return m_common; }

    inline void detail(const CompleteMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteMemberDetail& detail() const { return m_detail; }
    inline CompleteMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteEnumeratedLiteral& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalEnumeratedLiteral();
    ~MinimalEnumeratedLiteral();
    MinimalEnumeratedLiteral(const MinimalEnumeratedLiteral &x);
    MinimalEnumeratedLiteral(MinimalEnumeratedLiteral &&x);
    MinimalEnumeratedLiteral& operator=(const MinimalEnumeratedLiteral &x);
    MinimalEnumeratedLiteral& operator=(MinimalEnumeratedLiteral &&x);

    inline void common(const CommonEnumeratedLiteral &_common) { m_common = _common; }
    inline void common(CommonEnumeratedLiteral &&_common) { m_common = std::move(_common); }
    inline const CommonEnumeratedLiteral& common() const { return m_common; }
    inline CommonEnumeratedLiteral& common() { return m_common; }

    inline void detail(const MinimalMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(MinimalMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const MinimalMemberDetail& detail() const { return m_detail; }
    inline MinimalMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalEnumeratedLiteral& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonEnumeratedHeader();
    ~CommonEnumeratedHeader();
    CommonEnumeratedHeader(const CommonEnumeratedHeader &x);
    CommonEnumeratedHeader(CommonEnumeratedHeader &&x);
    CommonEnumeratedHeader& operator=(const CommonEnumeratedHeader &x);
    CommonEnumeratedHeader& operator=(CommonEnumeratedHeader &&x);

    inline void bit_bound(const BitBound &_bit_bound) { m_bit_bound = _bit_bound; }
    inline void bit_bound(BitBound &&_bit_bound) { m_bit_bound = std::move(_bit_bound); }
    inline const BitBound& bit_bound() const { return m_bit_bound; }
    inline BitBound& bit_bound() { return m_bit_bound; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonEnumeratedHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteEnumeratedHeader();
    ~CompleteEnumeratedHeader();
    CompleteEnumeratedHeader(const CompleteEnumeratedHeader &x);
    CompleteEnumeratedHeader(CompleteEnumeratedHeader &&x);
    CompleteEnumeratedHeader& operator=(const CompleteEnumeratedHeader &x);
    CompleteEnumeratedHeader& operator=(CompleteEnumeratedHeader &&x);

    inline void common(const CommonEnumeratedHeader &_common) { m_common = _common; }
    inline void common(CommonEnumeratedHeader &&_common) { m_common = std::move(_common); }
    inline const CommonEnumeratedHeader& common() const { return m_common; }
    inline CommonEnumeratedHeader& common() { return m_common; }

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteEnumeratedHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalEnumeratedHeader();
    ~MinimalEnumeratedHeader();
    MinimalEnumeratedHeader(const MinimalEnumeratedHeader &x);
    MinimalEnumeratedHeader(MinimalEnumeratedHeader &&x);
    MinimalEnumeratedHeader& operator=(const MinimalEnumeratedHeader &x);
    MinimalEnumeratedHeader& operator=(MinimalEnumeratedHeader &&x);

    inline void common(const CommonEnumeratedHeader &_common) { m_common = _common; }
    inline void common(CommonEnumeratedHeader &&_common) { m_common = std::move(_common); }
    inline const CommonEnumeratedHeader& common() const { return m_common; }
    inline CommonEnumeratedHeader& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalEnumeratedHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteEnumeratedType();
    ~CompleteEnumeratedType();
    CompleteEnumeratedType(const CompleteEnumeratedType &x);
    CompleteEnumeratedType(CompleteEnumeratedType &&x);
    CompleteEnumeratedType& operator=(const CompleteEnumeratedType &x);
    CompleteEnumeratedType& operator=(CompleteEnumeratedType &&x);

    inline void enum_flags(const EnumTypeFlag &_enum_flags) { m_enum_flags = _enum_flags; }
    inline void enum_flags(EnumTypeFlag &&_enum_flags) { m_enum_flags = std::move(_enum_flags); }
    inline const EnumTypeFlag& enum_flags() const { return m_enum_flags; }
    inline EnumTypeFlag& enum_flags() { return m_enum_flags; }

    inline void header(const CompleteEnumeratedHeader &_header) { m_header = _header; }
    inline void header(CompleteEnumeratedHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteEnumeratedHeader& header() const { return m_header; }
    inline CompleteEnumeratedHeader& header() { return m_header; }

    inline void literal_seq(const CompleteEnumeratedLiteralSeq &_literal_seq) { m_literal_seq = _literal_seq; }
    inline void literal_seq(CompleteEnumeratedLiteralSeq &&_literal_seq) { m_literal_seq = std::move(_literal_seq); }
    inline const CompleteEnumeratedLiteralSeq& literal_seq() const { return m_literal_seq; }
    inline CompleteEnumeratedLiteralSeq& literal_seq() { return m_literal_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteEnumeratedType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalEnumeratedType();
    ~MinimalEnumeratedType();
    MinimalEnumeratedType(const MinimalEnumeratedType &x);
    MinimalEnumeratedType(MinimalEnumeratedType &&x);
    MinimalEnumeratedType& operator=(const MinimalEnumeratedType &x);
    MinimalEnumeratedType& operator=(MinimalEnumeratedType &&x);

    inline void enum_flags(const EnumTypeFlag &_enum_flags) { m_enum_flags = _enum_flags; }
    inline void enum_flags(EnumTypeFlag &&_enum_flags) { m_enum_flags = std::move(_enum_flags); }
    inline const EnumTypeFlag& enum_flags() const { return m_enum_flags; }
    inline EnumTypeFlag& enum_flags() { return m_enum_flags; }

    inline void header(const MinimalEnumeratedHeader &_header) { m_header = _header; }
    inline void header(MinimalEnumeratedHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalEnumeratedHeader& header() const { return m_header; }
    inline MinimalEnumeratedHeader& header() { return m_header; }

    inline void literal_seq(const MinimalEnumeratedLiteralSeq &_literal_seq) { m_literal_seq = _literal_seq; }
    inline void literal_seq(MinimalEnumeratedLiteralSeq &&_literal_seq) { m_literal_seq = std::move(_literal_seq); }
    inline const MinimalEnumeratedLiteralSeq& literal_seq() const { return m_literal_seq; }
    inline MinimalEnumeratedLiteralSeq& literal_seq() { return m_literal_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalEnumeratedType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonBitflag();
    ~CommonBitflag();
    CommonBitflag(const CommonBitflag &x);
    CommonBitflag(CommonBitflag &&x);
    CommonBitflag& operator=(const CommonBitflag &x);
    CommonBitflag& operator=(CommonBitflag &&x);

    inline void position(const uint16_t &_position) { m_position = _position; }
    inline void position(uint16_t &&_position) { m_position = std::move(_position); }
    inline const uint16_t& position() const { return m_position; }
    inline uint16_t& position() { return m_position; }

    inline void flags(const BitflagFlag &_flags) { m_flags = _flags; }
    inline void flags(BitflagFlag &&_flags) { m_flags = std::move(_flags); }
    inline const BitflagFlag& flags() const { return m_flags; }
    inline BitflagFlag& flags() { return m_flags; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonBitflag& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteBitflag();
    ~CompleteBitflag();
    CompleteBitflag(const CompleteBitflag &x);
    CompleteBitflag(CompleteBitflag &&x);
    CompleteBitflag& operator=(const CompleteBitflag &x);
    CompleteBitflag& operator=(CompleteBitflag &&x);

    inline void common(const CommonBitflag &_common) { m_common = _common; }
    inline void common(CommonBitflag &&_common) { m_common = std::move(_common); }
    inline const CommonBitflag& common() const { return m_common; }
    inline CommonBitflag& common() { return m_common; }

    inline void detail(const CompleteMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteMemberDetail& detail() const { return m_detail; }
    inline CompleteMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteBitflag& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalBitflag();
    ~MinimalBitflag();
    MinimalBitflag(const MinimalBitflag &x);
    MinimalBitflag(MinimalBitflag &&x);
    MinimalBitflag& operator=(const MinimalBitflag &x);
    MinimalBitflag& operator=(MinimalBitflag &&x);

    inline void common(const CommonBitflag &_common) { m_common = _common; }
    inline void common(CommonBitflag &&_common) { m_common = std::move(_common); }
    inline const CommonBitflag& common() const { return m_common; }
    inline CommonBitflag& common() { return m_common; }

    inline void detail(const MinimalMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(MinimalMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const MinimalMemberDetail& detail() const { return m_detail; }
    inline MinimalMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalBitflag& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonBitmaskHeader();
    ~CommonBitmaskHeader();
    CommonBitmaskHeader(const CommonBitmaskHeader &x);
    CommonBitmaskHeader(CommonBitmaskHeader &&x);
    CommonBitmaskHeader& operator=(const CommonBitmaskHeader &x);
    CommonBitmaskHeader& operator=(CommonBitmaskHeader &&x);

    inline void bit_bound(const BitBound &_bit_bound) { m_bit_bound = _bit_bound; }
    inline void bit_bound(BitBound &&_bit_bound) { m_bit_bound = std::move(_bit_bound); }
    inline const BitBound& bit_bound() const { return m_bit_bound; }
    inline BitBound& bit_bound() { return m_bit_bound; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonBitmaskHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    BitBound m_bit_bound;
};


typedef CompleteEnumeratedHeader CompleteBitmaskHeader;

typedef MinimalEnumeratedHeader  MinimalBitmaskHeader;


/*struct CompleteBitmaskType {
	BitmaskTypeFlag          bitmask_flags; // unused
	CompleteBitmaskHeader    header;
	CompleteBitflagSeq       flag_seq;
};*/
class CompleteBitmaskType
{
public:
    CompleteBitmaskType();
    ~CompleteBitmaskType();
    CompleteBitmaskType(const CompleteBitmaskType &x);
    CompleteBitmaskType(CompleteBitmaskType &&x);
    CompleteBitmaskType& operator=(const CompleteBitmaskType &x);
    CompleteBitmaskType& operator=(CompleteBitmaskType &&x);

    inline void bitmask_flags(const BitmaskTypeFlag &_bitmask_flags) { m_bitmask_flags = _bitmask_flags; }
    inline void bitmask_flags(BitmaskTypeFlag &&_bitmask_flags) { m_bitmask_flags = std::move(_bitmask_flags); }
    inline const BitmaskTypeFlag& bitmask_flags() const { return m_bitmask_flags; }
    inline BitmaskTypeFlag& bitmask_flags() { return m_bitmask_flags; }

    inline void header(const CompleteBitmaskHeader &_header) { m_header = _header; }
    inline void header(CompleteBitmaskHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteBitmaskHeader& header() const { return m_header; }
    inline CompleteBitmaskHeader& header() { return m_header; }

    inline void flag_seq(const CompleteBitflagSeq &_flag_seq) { m_flag_seq = _flag_seq; }
    inline void flag_seq(CompleteBitflagSeq &&_flag_seq) { m_flag_seq = std::move(_flag_seq); }
    inline const CompleteBitflagSeq& flag_seq() const { return m_flag_seq; }
    inline CompleteBitflagSeq& flag_seq() { return m_flag_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteBitmaskType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalBitmaskType();
    ~MinimalBitmaskType();
    MinimalBitmaskType(const MinimalBitmaskType &x);
    MinimalBitmaskType(MinimalBitmaskType &&x);
    MinimalBitmaskType& operator=(const MinimalBitmaskType &x);
    MinimalBitmaskType& operator=(MinimalBitmaskType &&x);

    inline void bitmask_flags(const BitmaskTypeFlag &_bitmask_flags) { m_bitmask_flags = _bitmask_flags; }
    inline void bitmask_flags(BitmaskTypeFlag &&_bitmask_flags) { m_bitmask_flags = std::move(_bitmask_flags); }
    inline const BitmaskTypeFlag& bitmask_flags() const { return m_bitmask_flags; }
    inline BitmaskTypeFlag& bitmask_flags() { return m_bitmask_flags; }

    inline void header(const MinimalBitmaskHeader &_header) { m_header = _header; }
    inline void header(MinimalBitmaskHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalBitmaskHeader& header() const { return m_header; }
    inline MinimalBitmaskHeader& header() { return m_header; }

    inline void flag_seq(const MinimalBitflagSeq &_flag_seq) { m_flag_seq = _flag_seq; }
    inline void flag_seq(MinimalBitflagSeq &&_flag_seq) { m_flag_seq = std::move(_flag_seq); }
    inline const MinimalBitflagSeq& flag_seq() const { return m_flag_seq; }
    inline MinimalBitflagSeq& flag_seq() { return m_flag_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalBitmaskType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CommonBitfield();
    ~CommonBitfield();
    CommonBitfield(const CommonBitfield &x);
    CommonBitfield(CommonBitfield &&x);
    CommonBitfield& operator=(const CommonBitfield &x);
    CommonBitfield& operator=(CommonBitfield &&x);

    inline void position(const uint16_t &_position) { m_position = _position; }
    inline void position(uint16_t &&_position) { m_position = std::move(_position); }
    inline const uint16_t& position() const { return m_position; }
    inline uint16_t& position() { return m_position; }

    inline void flags(const BitsetMemberFlag &_flags) { m_flags = _flags; }
    inline void flags(BitsetMemberFlag &&_flags) { m_flags = std::move(_flags); }
    inline const BitsetMemberFlag& flags() const { return m_flags; }
    inline BitsetMemberFlag& flags() { return m_flags; }

    inline void bitcount(const octet &_bitcount) { m_bitcount = _bitcount; }
    inline void bitcount(octet &&_bitcount) { m_bitcount = std::move(_bitcount); }
    inline const octet& bitcount() const { return m_bitcount; }
    inline octet& bitcount() { return m_bitcount; }

    inline void holder_type(const TypeKind &_holder_type) { m_holder_type = _holder_type; }
    inline void holder_type(TypeKind &&_holder_type) { m_holder_type = std::move(_holder_type); }
    inline const TypeKind& holder_type() const { return m_holder_type; }
    inline TypeKind& holder_type() { return m_holder_type; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CommonBitfield& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteBitfield();
    ~CompleteBitfield();
    CompleteBitfield(const CompleteBitfield &x);
    CompleteBitfield(CompleteBitfield &&x);
    CompleteBitfield& operator=(const CompleteBitfield &x);
    CompleteBitfield& operator=(CompleteBitfield &&x);

    inline void common(const CommonBitfield &_common) { m_common = _common; }
    inline void common(CommonBitfield &&_common) { m_common = std::move(_common); }
    inline const CommonBitfield& common() const { return m_common; }
    inline CommonBitfield& common() { return m_common; }

    inline void detail(const CompleteMemberDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteMemberDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteMemberDetail& detail() const { return m_detail; }
    inline CompleteMemberDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteBitfield& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalBitfield();
    ~MinimalBitfield();
    MinimalBitfield(const MinimalBitfield &x);
    MinimalBitfield(MinimalBitfield &&x);
    MinimalBitfield& operator=(const MinimalBitfield &x);
    MinimalBitfield& operator=(MinimalBitfield &&x);

    inline void name_hash(const NameHash &_name_hash) { m_name_hash = _name_hash; }
    inline void name_hash(NameHash &&_name_hash) { m_name_hash = std::move(_name_hash); }
    inline const NameHash& name_hash() const { return m_name_hash; }
    inline NameHash& name_hash() { return m_name_hash; }

    inline void common(const CommonBitfield &_common) { m_common = _common; }
    inline void common(CommonBitfield &&_common) { m_common = std::move(_common); }
    inline const CommonBitfield& common() const { return m_common; }
    inline CommonBitfield& common() { return m_common; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalBitfield& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteBitsetHeader();
    ~CompleteBitsetHeader();
    CompleteBitsetHeader(const CompleteBitsetHeader &x);
    CompleteBitsetHeader(CompleteBitsetHeader &&x);
    CompleteBitsetHeader& operator=(const CompleteBitsetHeader &x);
    CompleteBitsetHeader& operator=(CompleteBitsetHeader &&x);

    inline void detail(const CompleteTypeDetail &_detail) { m_detail = _detail; }
    inline void detail(CompleteTypeDetail &&_detail) { m_detail = std::move(_detail); }
    inline const CompleteTypeDetail& detail() const { return m_detail; }
    inline CompleteTypeDetail& detail() { return m_detail; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteBitsetHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    CompleteTypeDetail m_detail;
};

/*struct MinimalBitsetHeader {
	// Empty. Available for future extension
};*/
class MinimalBitsetHeader
{
public:
    MinimalBitsetHeader();
    ~MinimalBitsetHeader();
    MinimalBitsetHeader(const MinimalBitsetHeader &x);
    MinimalBitsetHeader(MinimalBitsetHeader &&x);
    MinimalBitsetHeader& operator=(const MinimalBitsetHeader &x);
    MinimalBitsetHeader& operator=(MinimalBitsetHeader &&x);

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalBitsetHeader& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
};


/*struct CompleteBitsetType  {
	BitsetTypeFlag         bitset_flags; // unused
	CompleteBitsetHeader   header;
	CompleteBitfieldSeq    field_seq;
};*/
class CompleteBitsetType
{
public:
    CompleteBitsetType();
    ~CompleteBitsetType();
    CompleteBitsetType(const CompleteBitsetType &x);
    CompleteBitsetType(CompleteBitsetType &&x);
    CompleteBitsetType& operator=(const CompleteBitsetType &x);
    CompleteBitsetType& operator=(CompleteBitsetType &&x);

    inline void bitset_flags(const BitsetTypeFlag &_bitset_flags) { m_bitset_flags = _bitset_flags; }
    inline void bitset_flags(BitsetTypeFlag &&_bitset_flags) { m_bitset_flags = std::move(_bitset_flags); }
    inline const BitsetTypeFlag& bitset_flags() const { return m_bitset_flags; }
    inline BitsetTypeFlag& bitset_flags() { return m_bitset_flags; }

    inline void header(const CompleteBitsetHeader &_header) { m_header = _header; }
    inline void header(CompleteBitsetHeader &&_header) { m_header = std::move(_header); }
    inline const CompleteBitsetHeader& header() const { return m_header; }
    inline CompleteBitsetHeader& header() { return m_header; }

    inline void field_seq(const CompleteBitfieldSeq &_field_seq) { m_field_seq = _field_seq; }
    inline void field_seq(CompleteBitfieldSeq &&_field_seq) { m_field_seq = std::move(_field_seq); }
    inline const CompleteBitfieldSeq& field_seq() const { return m_field_seq; }
    inline CompleteBitfieldSeq& field_seq() { return m_field_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteBitsetType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    MinimalBitsetType();
    ~MinimalBitsetType();
    MinimalBitsetType(const MinimalBitsetType &x);
    MinimalBitsetType(MinimalBitsetType &&x);
    MinimalBitsetType& operator=(const MinimalBitsetType &x);
    MinimalBitsetType& operator=(MinimalBitsetType &&x);

    inline void bitset_flags(const BitsetTypeFlag &_bitset_flags) { m_bitset_flags = _bitset_flags; }
    inline void bitset_flags(BitsetTypeFlag &&_bitset_flags) { m_bitset_flags = std::move(_bitset_flags); }
    inline const BitsetTypeFlag& bitset_flags() const { return m_bitset_flags; }
    inline BitsetTypeFlag& bitset_flags() { return m_bitset_flags; }

    inline void header(const MinimalBitsetHeader &_header) { m_header = _header; }
    inline void header(MinimalBitsetHeader &&_header) { m_header = std::move(_header); }
    inline const MinimalBitsetHeader& header() const { return m_header; }
    inline MinimalBitsetHeader& header() { return m_header; }

    inline void field_seq(const MinimalBitfieldSeq &_field_seq) { m_field_seq = _field_seq; }
    inline void field_seq(MinimalBitfieldSeq &&_field_seq) { m_field_seq = std::move(_field_seq); }
    inline const MinimalBitfieldSeq& field_seq() const { return m_field_seq; }
    inline MinimalBitfieldSeq& field_seq() { return m_field_seq; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalBitsetType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    CompleteExtendedType();
    ~CompleteExtendedType();
    CompleteExtendedType(const CompleteExtendedType &x);
    CompleteExtendedType(CompleteExtendedType &&x);
    CompleteExtendedType& operator=(const CompleteExtendedType &x);
    CompleteExtendedType& operator=(CompleteExtendedType &&x);

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteExtendedType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
};

/*struct MinimalExtendedType  {
	// Empty. Available for future extension
};*/
class MinimalExtendedType
{
public:
    MinimalExtendedType();
    ~MinimalExtendedType();
    MinimalExtendedType(const MinimalExtendedType &x);
    MinimalExtendedType(MinimalExtendedType &&x);
    MinimalExtendedType& operator=(const MinimalExtendedType &x);
    MinimalExtendedType& operator=(MinimalExtendedType &&x);

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalExtendedType& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
};

class CompleteTypeObject final
{
public:
    CompleteTypeObject();
    ~CompleteTypeObject();
    CompleteTypeObject(const CompleteTypeObject &x);
    CompleteTypeObject(CompleteTypeObject &&x);
    CompleteTypeObject& operator=(const CompleteTypeObject &x);
    CompleteTypeObject& operator=(CompleteTypeObject &&x);
    void _d(octet __d);
    octet _d() const;
    octet& _d();

    void alias_type(CompleteAliasType _alias_type);
    CompleteAliasType alias_type() const;
    CompleteAliasType& alias_type();

    void annotation_type(CompleteAnnotationType _annotation_type);
    CompleteAnnotationType annotation_type() const;
    CompleteAnnotationType& annotation_type();

    void struct_type(CompleteStructType _struct_type);
    CompleteStructType struct_type() const;
    CompleteStructType& struct_type();

    void union_type(CompleteUnionType _union_type);
    CompleteUnionType union_type() const;
    CompleteUnionType& union_type();

    void bitset_type(CompleteBitsetType _bitset_type);
    CompleteBitsetType bitset_type() const;
    CompleteBitsetType& bitset_type();

    void sequence_type(CompleteSequenceType _sequence_type);
    CompleteSequenceType sequence_type() const;
    CompleteSequenceType& sequence_type();

    void array_type(CompleteArrayType _array_type);
    CompleteArrayType array_type() const;
    CompleteArrayType& array_type();

    void map_type(CompleteMapType _map_type);
    CompleteMapType map_type() const;
    CompleteMapType& map_type();

    void enumerated_type(CompleteEnumeratedType _enumerated_type);
    CompleteEnumeratedType enumerated_type() const;
    CompleteEnumeratedType& enumerated_type();

    void bitmask_type(CompleteBitmaskType _bitmask_type);
    CompleteBitmaskType bitmask_type() const;
    CompleteBitmaskType& bitmask_type();

    void extended_type(CompleteExtendedType _extended_type);
    CompleteExtendedType extended_type() const;
    CompleteExtendedType& extended_type();

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const CompleteTypeObject& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    octet m__d;

    CompleteAliasType      m_alias_type;
    CompleteAnnotationType m_annotation_type;
    CompleteStructType     m_struct_type;
    CompleteUnionType      m_union_type;
    CompleteBitsetType     m_bitset_type;
    CompleteSequenceType   m_sequence_type;
    CompleteArrayType      m_array_type;
    CompleteMapType        m_map_type;
    CompleteEnumeratedType m_enumerated_type;
    CompleteBitmaskType    m_bitmask_type;
    CompleteExtendedType   m_extended_type;
};

class MinimalTypeObject final
{
public:
    MinimalTypeObject();
    ~MinimalTypeObject();
    MinimalTypeObject(const MinimalTypeObject &x);
    MinimalTypeObject(MinimalTypeObject &&x);
    MinimalTypeObject& operator=(const MinimalTypeObject &x);
    MinimalTypeObject& operator=(MinimalTypeObject &&x);
    void _d(octet __d);
    octet _d() const;
    octet& _d();

    void alias_type(MinimalAliasType _alias_type);
    MinimalAliasType alias_type() const;
    MinimalAliasType& alias_type();

    void annotation_type(MinimalAnnotationType _annotation_type);
    MinimalAnnotationType annotation_type() const;
    MinimalAnnotationType& annotation_type();

    void struct_type(MinimalStructType _struct_type);
    MinimalStructType struct_type() const;
    MinimalStructType& struct_type();

    void union_type(MinimalUnionType _union_type);
    MinimalUnionType union_type() const;
    MinimalUnionType& union_type();

    void bitset_type(MinimalBitsetType _bitset_type);
    MinimalBitsetType bitset_type() const;
    MinimalBitsetType& bitset_type();

    void sequence_type(MinimalSequenceType _sequence_type);
    MinimalSequenceType sequence_type() const;
    MinimalSequenceType& sequence_type();

    void array_type(MinimalArrayType _array_type);
    MinimalArrayType array_type() const;
    MinimalArrayType& array_type();

    void map_type(MinimalMapType _map_type);
    MinimalMapType map_type() const;
    MinimalMapType& map_type();

    void enumerated_type(MinimalEnumeratedType _enumerated_type);
    MinimalEnumeratedType enumerated_type() const;
    MinimalEnumeratedType& enumerated_type();

    void bitmask_type(MinimalBitmaskType _bitmask_type);
    MinimalBitmaskType bitmask_type() const;
    MinimalBitmaskType& bitmask_type();

    void extended_type(MinimalExtendedType _extended_type);
    MinimalExtendedType extended_type() const;
    MinimalExtendedType& extended_type();

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const MinimalTypeObject& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    octet m__d;

    MinimalAliasType      m_alias_type;
    MinimalAnnotationType m_annotation_type;
    MinimalStructType     m_struct_type;
    MinimalUnionType      m_union_type;
    MinimalBitsetType     m_bitset_type;
    MinimalSequenceType   m_sequence_type;
    MinimalArrayType      m_array_type;
    MinimalMapType        m_map_type;
    MinimalEnumeratedType m_enumerated_type;
    MinimalBitmaskType    m_bitmask_type;
    MinimalExtendedType   m_extended_type;
};


class TypeObject
{
public:

    /*!
     * @brief Default constructor.
     */
    TypeObject();

    /*!
     * @brief Default destructor.
     */
    ~TypeObject();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object TypeObject that will be copied.
     */
    TypeObject(const TypeObject &x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object TypeObject that will be copied.
     */
    TypeObject(TypeObject &&x);

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object TypeObject that will be copied.
     */
    TypeObject& operator=(const TypeObject &x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object TypeObject that will be copied.
     */
    TypeObject& operator=(TypeObject &&x);

    /*!
     * @brief This function sets the discriminator value.
     * @param __d New value for the discriminator.
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
     */
    void _d(uint8_t __d);

    /*!
     * @brief This function returns the value of the discriminator.
     * @return Value of the discriminator
     */
    uint8_t _d() const;

    /*!
     * @brief This function returns a reference to the discriminator.
     * @return Reference to the discriminator.
     */
    uint8_t& _d();

    /*!
     * @brief This function copies the value in member complete
     * @param _complete New value to be copied in member complete
     */
    void complete(const CompleteTypeObject &_complete);

    /*!
     * @brief This function moves the value in member complete
     * @param _complete New value to be moved in member complete
     */
    void complete(CompleteTypeObject &&_complete);

    /*!
     * @brief This function returns a constant reference to member complete
     * @return Constant reference to member complete
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const CompleteTypeObject& complete() const;

    /*!
     * @brief This function returns a reference to member complete
     * @return Reference to member complete
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    CompleteTypeObject& complete();
    /*!
     * @brief This function copies the value in member minimal
     * @param _minimal New value to be copied in member minimal
     */
    void minimal(const MinimalTypeObject &_minimal);

    /*!
     * @brief This function moves the value in member minimal
     * @param _minimal New value to be moved in member minimal
     */
    void minimal(MinimalTypeObject &&_minimal);

    /*!
     * @brief This function returns a constant reference to member minimal
     * @return Constant reference to member minimal
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const MinimalTypeObject& minimal() const;

    /*!
     * @brief This function returns a reference to member minimal
     * @return Reference to member minimal
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    MinimalTypeObject& minimal();

    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    static size_t getCdrSerializedSize(const TypeObject& data, size_t current_alignment = 0);


    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void serialize(eprosima::fastcdr::Cdr &cdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void deserialize(eprosima::fastcdr::Cdr &cdr);



    /*!
     * @brief This function returns the maximum serialized size of the Key of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);

    /*!
     * @brief This function tells you if the Key has been defined for this type
     */
    static bool isKeyDefined();

    /*!
     * @brief This function serializes the key members of an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
typedef TypeObjectSeq        StronglyConnectedComponent;

/*struct TypeIdentifierTypeObjectPair final {
	TypeIdentifier  type_identifier;
	TypeObject      type_object;
};*/
class TypeIdentifierTypeObjectPair final
{
public:
    TypeIdentifierTypeObjectPair();
    ~TypeIdentifierTypeObjectPair();
    TypeIdentifierTypeObjectPair(const TypeIdentifierTypeObjectPair &x);
    TypeIdentifierTypeObjectPair(TypeIdentifierTypeObjectPair &&x);
    TypeIdentifierTypeObjectPair& operator=(const TypeIdentifierTypeObjectPair &x);
    TypeIdentifierTypeObjectPair& operator=(TypeIdentifierTypeObjectPair &&x);

    inline void type_identifier(const TypeIdentifier &_type_identifier) { m_type_identifier = _type_identifier; }
    inline void type_identifier(TypeIdentifier &&_type_identifier) { m_type_identifier = std::move(_type_identifier); }
    inline const TypeIdentifier& type_identifier() const { return m_type_identifier; }
    inline TypeIdentifier& type_identifier() { return m_type_identifier; }

    inline void type_object(const TypeObject &_type_object) { m_type_object = _type_object; }
    inline void type_object(TypeObject &&_type_object) { m_type_object = std::move(_type_object); }
    inline const TypeObject& type_object() const { return m_type_object; }
    inline TypeObject& type_object() { return m_type_object; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const TypeIdentifierTypeObjectPair& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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
    TypeIdentifierPair();
    ~TypeIdentifierPair();
    TypeIdentifierPair(const TypeIdentifierPair &x);
    TypeIdentifierPair(TypeIdentifierPair &&x);
    TypeIdentifierPair& operator=(const TypeIdentifierPair &x);
    TypeIdentifierPair& operator=(TypeIdentifierPair &&x);

    inline void type_identifier1(const TypeIdentifier &_type_identifier1) { m_type_identifier1 = _type_identifier1; }
    inline void type_identifier1(TypeIdentifier &&_type_identifier1) { m_type_identifier1 = std::move(_type_identifier1); }
    inline const TypeIdentifier& type_identifier1() const { return m_type_identifier1; }
    inline TypeIdentifier& type_identifier1() { return m_type_identifier1; }

    inline void type_identifier2(const TypeIdentifier &_type_identifier2) { m_type_identifier2 = _type_identifier2; }
    inline void type_identifier2(TypeIdentifier &&_type_identifier2) { m_type_identifier2 = std::move(_type_identifier2); }
    inline const TypeIdentifier& type_identifier2() const { return m_type_identifier2; }
    inline TypeIdentifier& type_identifier2() { return m_type_identifier2; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const TypeIdentifierPair& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    TypeIdentifier m_type_identifier1;
    TypeIdentifier m_type_identifier2;
};

typedef std::vector<TypeIdentifierPair> TypeIdentifierPairSeq;


/*struct TypeIdentfierWithSize {
	TypeIdentifier  type_id;
	uint32_t                typeobject_serialized_size;
};*/
class TypeIdentfierWithSize
{
public:
    TypeIdentfierWithSize();
    ~TypeIdentfierWithSize();
    TypeIdentfierWithSize(const TypeIdentfierWithSize &x);
    TypeIdentfierWithSize(TypeIdentfierWithSize &&x);
    TypeIdentfierWithSize& operator=(const TypeIdentfierWithSize &x);
    TypeIdentfierWithSize& operator=(TypeIdentfierWithSize &&x);

    inline void type_id(const TypeIdentifier &_type_id) { m_type_id = _type_id; }
    inline void type_id(TypeIdentifier &&_type_id) { m_type_id = std::move(_type_id); }
    inline const TypeIdentifier& type_id() const { return m_type_id; }
    inline TypeIdentifier& type_id() { return m_type_id; }

    inline void typeobject_serialized_size(const uint32_t &_typeobject_serialized_size) { m_typeobject_serialized_size = _typeobject_serialized_size; }
    inline void typeobject_serialized_size(uint32_t &&_typeobject_serialized_size) { m_typeobject_serialized_size = std::move(_typeobject_serialized_size); }
    inline const uint32_t& typeobject_serialized_size() const { return m_typeobject_serialized_size; }
    inline uint32_t& typeobject_serialized_size() { return m_typeobject_serialized_size; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const TypeIdentfierWithSize& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    TypeIdentifier m_type_id;
    uint32_t m_typeobject_serialized_size;
};

typedef std::vector<TypeIdentfierWithSize> TypeIdentfierWithSizeSeq;

/*struct TypeIdentifierWithDependencies {
	TypeIdentfierWithSize            typeid_with_size;
	// The total additional types related to minimal_type
	int32_t                             dependent_typeid_count;
	TypeIdentfierWithSizeSeq  dependent_typeids;
};*/
class TypeIdentifierWithDependencies
{
public:
    TypeIdentifierWithDependencies();
    ~TypeIdentifierWithDependencies();
    TypeIdentifierWithDependencies(const TypeIdentifierWithDependencies &x);
    TypeIdentifierWithDependencies(TypeIdentifierWithDependencies &&x);
    TypeIdentifierWithDependencies& operator=(const TypeIdentifierWithDependencies &x);
    TypeIdentifierWithDependencies& operator=(TypeIdentifierWithDependencies &&x);

    inline void typeid_with_size(const TypeIdentfierWithSize &_typeid_with_size) { m_typeid_with_size = _typeid_with_size; }
    inline void typeid_with_size(TypeIdentfierWithSize &&_typeid_with_size) { m_typeid_with_size = std::move(_typeid_with_size); }
    inline const TypeIdentfierWithSize& typeid_with_size() const { return m_typeid_with_size; }
    inline TypeIdentfierWithSize& typeid_with_size() { return m_typeid_with_size; }

    inline void dependent_typeid_count(const int32_t &_dependent_typeid_count) { m_dependent_typeid_count = _dependent_typeid_count; }
    inline void dependent_typeid_count(int32_t &&_dependent_typeid_count) { m_dependent_typeid_count = std::move(_dependent_typeid_count); }
    inline const int32_t& dependent_typeid_count() const { return m_dependent_typeid_count; }
    inline int32_t& dependent_typeid_count() { return m_dependent_typeid_count; }

    inline void dependent_typeids(const TypeIdentfierWithSizeSeq &_dependent_typeids) { m_dependent_typeids = _dependent_typeids; }
    inline void dependent_typeids(TypeIdentfierWithSizeSeq &&_dependent_typeids) { m_dependent_typeids = std::move(_dependent_typeids); }
    inline const TypeIdentfierWithSizeSeq& dependent_typeids() const { return m_dependent_typeids; }
    inline TypeIdentfierWithSizeSeq& dependent_typeids() { return m_dependent_typeids; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const TypeIdentifierWithDependencies& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

private:
    TypeIdentfierWithSize m_typeid_with_size;
    int32_t m_dependent_typeid_count;
    TypeIdentfierWithSizeSeq m_dependent_typeids;
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
    TypeInformation();
    ~TypeInformation();
    TypeInformation(const TypeInformation &x);
    TypeInformation(TypeInformation &&x);
    TypeInformation& operator=(const TypeInformation &x);
    TypeInformation& operator=(TypeInformation &&x);

    inline void minimal(const TypeIdentifierWithDependencies &_minimal) { m_minimal = _minimal; }
    inline void minimal(TypeIdentifierWithDependencies &&_minimal) { m_minimal = std::move(_minimal); }
    inline const TypeIdentifierWithDependencies& minimal() const { return m_minimal; }
    inline TypeIdentifierWithDependencies& minimal() { return m_minimal; }

    inline void complete(const TypeIdentifierWithDependencies &_complete) { m_complete = _complete; }
    inline void complete(TypeIdentifierWithDependencies &&_complete) { m_complete = std::move(_complete); }
    inline const TypeIdentifierWithDependencies& complete() const { return m_complete; }
    inline TypeIdentifierWithDependencies& complete() { return m_complete; }

    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    static size_t getCdrSerializedSize(const TypeInformation& data, size_t current_alignment = 0);
    void serialize(eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastcdr::Cdr &cdr);
    static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    static bool isKeyDefined();
    void serializeKey(eprosima::fastcdr::Cdr &cdr) const;

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