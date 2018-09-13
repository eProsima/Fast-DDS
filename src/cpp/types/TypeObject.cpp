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

#include <fastrtps/types/TypeObject.h>
#include <fastcdr/exceptions/BadParamException.h>

// The types in this file shall be serialized with XCDR encoding version 2
namespace eprosima{
namespace fastrtps{

using namespace rtps;
using namespace eprosima::fastcdr::exception;

namespace types{

CommonStructMember::CommonStructMember()
{
}

CommonStructMember::~CommonStructMember()
{
}

CommonStructMember::CommonStructMember(const CommonStructMember &x)
{
    m_member_id = x.m_member_id;
    m_member_flags = x.m_member_flags;
    m_member_type_id = x.m_member_type_id;
}

CommonStructMember::CommonStructMember(CommonStructMember &&x)
{
    m_member_id = std::move(x.m_member_id);
    m_member_flags = std::move(x.m_member_flags);
    m_member_type_id = std::move(x.m_member_type_id);
}

CommonStructMember& CommonStructMember::operator=(const CommonStructMember &x)
{
    m_member_id = x.m_member_id;
    m_member_flags = x.m_member_flags;
    m_member_type_id = x.m_member_type_id;

    return *this;
}

CommonStructMember& CommonStructMember::operator=(CommonStructMember &&x)
{
    m_member_id = std::move(x.m_member_id);
    m_member_flags = std::move(x.m_member_flags);
    m_member_type_id = std::move(x.m_member_type_id);

    return *this;
}

size_t CommonStructMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += StructMemberFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonStructMember::getCdrSerializedSize(const CommonStructMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += StructMemberFlag::getCdrSerializedSize(data.member_flags(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.member_type_id(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonStructMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_member_id;
    scdr << m_member_flags;
    scdr << m_member_type_id;
}

void CommonStructMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_member_id;
    dcdr >> m_member_flags;
    dcdr >> m_member_type_id;
}

size_t CommonStructMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonStructMember::isKeyDefined()
{
    return false;
}

void CommonStructMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteMemberDetail::CompleteMemberDetail()
{
}

CompleteMemberDetail::~CompleteMemberDetail()
{
}

CompleteMemberDetail::CompleteMemberDetail(const CompleteMemberDetail &x)
{
    m_name = x.m_name;
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;
}

CompleteMemberDetail::CompleteMemberDetail(CompleteMemberDetail &&x)
{
    m_name = std::move(x.m_name);
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);
}

CompleteMemberDetail& CompleteMemberDetail::operator=(const CompleteMemberDetail &x)
{
    m_name = x.m_name;
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;

    return *this;
}

CompleteMemberDetail& CompleteMemberDetail::operator=(CompleteMemberDetail &&x)
{
    m_name = std::move(x.m_name);
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);

    return *this;
}

size_t CompleteMemberDetail::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;
    current_alignment += AppliedBuiltinMemberAnnotations::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotation::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteMemberDetail::getCdrSerializedSize(const CompleteMemberDetail& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.name().size()  + 1;
    current_alignment += AppliedBuiltinMemberAnnotations::getCdrSerializedSize(data.ann_builtin(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.ann_custom().size(); ++a)
    {
        current_alignment += AppliedAnnotation::getCdrSerializedSize(data.ann_custom().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteMemberDetail::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_name;
    scdr << m_ann_builtin;
    scdr << m_ann_custom;
}

void CompleteMemberDetail::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_name;
    dcdr >> m_ann_builtin;
    dcdr >> m_ann_custom;
}

size_t CompleteMemberDetail::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteMemberDetail::isKeyDefined()
{
    return false;
}

void CompleteMemberDetail::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalMemberDetail::MinimalMemberDetail()
{
}

MinimalMemberDetail::~MinimalMemberDetail()
{
}

MinimalMemberDetail::MinimalMemberDetail(const MinimalMemberDetail &x)
{
    m_name_hash = x.m_name_hash;
}

MinimalMemberDetail::MinimalMemberDetail(MinimalMemberDetail &&x)
{
    m_name_hash = std::move(x.m_name_hash);
}

MinimalMemberDetail& MinimalMemberDetail::operator=(const MinimalMemberDetail &x)
{
    m_name_hash = x.m_name_hash;

    return *this;
}

MinimalMemberDetail& MinimalMemberDetail::operator=(MinimalMemberDetail &&x)
{
    m_name_hash = std::move(x.m_name_hash);

    return *this;
}

size_t MinimalMemberDetail::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

size_t MinimalMemberDetail::getCdrSerializedSize(const MinimalMemberDetail& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

void MinimalMemberDetail::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_name_hash;
}

void MinimalMemberDetail::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_name_hash;
}

size_t MinimalMemberDetail::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalMemberDetail::isKeyDefined()
{
    return false;
}

void MinimalMemberDetail::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteStructMember::CompleteStructMember()
{
}

CompleteStructMember::~CompleteStructMember()
{
}

CompleteStructMember::CompleteStructMember(const CompleteStructMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteStructMember::CompleteStructMember(CompleteStructMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteStructMember& CompleteStructMember::operator=(const CompleteStructMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteStructMember& CompleteStructMember::operator=(CompleteStructMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteStructMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonStructMember::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteStructMember::getCdrSerializedSize(const CompleteStructMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonStructMember::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteStructMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteStructMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteStructMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteStructMember::isKeyDefined()
{
    return false;
}

void CompleteStructMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalStructMember::MinimalStructMember()
{
}

MinimalStructMember::~MinimalStructMember()
{
}

MinimalStructMember::MinimalStructMember(const MinimalStructMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

MinimalStructMember::MinimalStructMember(MinimalStructMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

MinimalStructMember& MinimalStructMember::operator=(const MinimalStructMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

MinimalStructMember& MinimalStructMember::operator=(MinimalStructMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t MinimalStructMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonStructMember::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalStructMember::getCdrSerializedSize(const MinimalStructMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonStructMember::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += MinimalMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalStructMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void MinimalStructMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t MinimalStructMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalStructMember::isKeyDefined()
{
    return false;
}

void MinimalStructMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

AppliedBuiltinTypeAnnotations::AppliedBuiltinTypeAnnotations()
{
}

AppliedBuiltinTypeAnnotations::~AppliedBuiltinTypeAnnotations()
{
}

AppliedBuiltinTypeAnnotations::AppliedBuiltinTypeAnnotations(const AppliedBuiltinTypeAnnotations &x)
{
    m_verbatim = x.m_verbatim;
}

AppliedBuiltinTypeAnnotations::AppliedBuiltinTypeAnnotations(AppliedBuiltinTypeAnnotations &&x)
{
    m_verbatim = std::move(x.m_verbatim);
}

AppliedBuiltinTypeAnnotations& AppliedBuiltinTypeAnnotations::operator=(const AppliedBuiltinTypeAnnotations &x)
{
    m_verbatim = x.m_verbatim;

    return *this;
}

AppliedBuiltinTypeAnnotations& AppliedBuiltinTypeAnnotations::operator=(AppliedBuiltinTypeAnnotations &&x)
{
    m_verbatim = std::move(x.m_verbatim);

    return *this;
}

size_t AppliedBuiltinTypeAnnotations::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AppliedVerbatimAnnotation::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t AppliedBuiltinTypeAnnotations::getCdrSerializedSize(const AppliedBuiltinTypeAnnotations& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AppliedVerbatimAnnotation::getCdrSerializedSize(data.verbatim(), current_alignment);

    return current_alignment - initial_alignment;
}

void AppliedBuiltinTypeAnnotations::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_verbatim;
}

void AppliedBuiltinTypeAnnotations::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_verbatim;
}

size_t AppliedBuiltinTypeAnnotations::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool AppliedBuiltinTypeAnnotations::isKeyDefined()
{
    return false;
}

void AppliedBuiltinTypeAnnotations::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalTypeDetail::MinimalTypeDetail()
{
}

MinimalTypeDetail::~MinimalTypeDetail()
{
}

MinimalTypeDetail::MinimalTypeDetail(const MinimalTypeDetail &x)
{
}

MinimalTypeDetail::MinimalTypeDetail(MinimalTypeDetail &&x)
{
}

MinimalTypeDetail& MinimalTypeDetail::operator=(const MinimalTypeDetail &x)
{
    return *this;
}

MinimalTypeDetail& MinimalTypeDetail::operator=(MinimalTypeDetail &&x)
{
    return *this;
}

size_t MinimalTypeDetail::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    return current_alignment - initial_alignment;
}

size_t MinimalTypeDetail::getCdrSerializedSize(const MinimalTypeDetail& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    return current_alignment - initial_alignment;
}

void MinimalTypeDetail::serialize(eprosima::fastcdr::Cdr &scdr) const
{
}

void MinimalTypeDetail::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
}

size_t MinimalTypeDetail::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalTypeDetail::isKeyDefined()
{
    return false;
}

void MinimalTypeDetail::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteTypeDetail::CompleteTypeDetail()
{
}

CompleteTypeDetail::~CompleteTypeDetail()
{
}

CompleteTypeDetail::CompleteTypeDetail(const CompleteTypeDetail &x)
{
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;
    m_type_name = x.m_type_name;
}

CompleteTypeDetail::CompleteTypeDetail(CompleteTypeDetail &&x)
{
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);
    m_type_name = std::move(x.m_type_name);
}

CompleteTypeDetail& CompleteTypeDetail::operator=(const CompleteTypeDetail &x)
{
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;
    m_type_name = x.m_type_name;

    return *this;
}

CompleteTypeDetail& CompleteTypeDetail::operator=(CompleteTypeDetail &&x)
{
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);
    m_type_name = std::move(x.m_type_name);

    return *this;
}

size_t CompleteTypeDetail::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += AppliedBuiltinTypeAnnotations::getMaxCdrSerializedSize(current_alignment);
	current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotation::getMaxCdrSerializedSize(current_alignment);
    }
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;

    return current_alignment - initial_alignment;
}

size_t CompleteTypeDetail::getCdrSerializedSize(const CompleteTypeDetail& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AppliedBuiltinTypeAnnotations::getCdrSerializedSize(data.ann_builtin(), current_alignment);
	current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.ann_custom().size(); ++a)
    {
        current_alignment += AppliedAnnotation::getCdrSerializedSize(data.ann_custom().at(a), current_alignment);
    }
	current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.type_name().size() + 1;

    return current_alignment - initial_alignment;
}

void CompleteTypeDetail::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_ann_builtin;
    scdr << m_ann_custom;
    scdr << m_type_name;
}

void CompleteTypeDetail::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_ann_builtin;
    dcdr >> m_ann_custom;
    dcdr >> m_type_name;
}

size_t CompleteTypeDetail::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteTypeDetail::isKeyDefined()
{
    return false;
}

void CompleteTypeDetail::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteStructHeader::CompleteStructHeader()
{
}

CompleteStructHeader::~CompleteStructHeader()
{
}

CompleteStructHeader::CompleteStructHeader(const CompleteStructHeader &x)
{
    m_base_type = x.m_base_type;
    m_detail = x.m_detail;
}

CompleteStructHeader::CompleteStructHeader(CompleteStructHeader &&x)
{
    m_base_type = std::move(x.m_base_type);
    m_detail = std::move(x.m_detail);
}

CompleteStructHeader& CompleteStructHeader::operator=(const CompleteStructHeader &x)
{
    m_base_type = x.m_base_type;
    m_detail = x.m_detail;

    return *this;
}

CompleteStructHeader& CompleteStructHeader::operator=(CompleteStructHeader &&x)
{
    m_base_type = std::move(x.m_base_type);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteStructHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteStructHeader::getCdrSerializedSize(const CompleteStructHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getCdrSerializedSize(data.base_type(), current_alignment);
    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteStructHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_base_type;
    scdr << m_detail;
}

void CompleteStructHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_base_type;
    dcdr >> m_detail;
}

size_t CompleteStructHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteStructHeader::isKeyDefined()
{
    return false;
}

void CompleteStructHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalStructHeader::MinimalStructHeader()
{
}

MinimalStructHeader::~MinimalStructHeader()
{
}

MinimalStructHeader::MinimalStructHeader(const MinimalStructHeader &x)
{
    m_base_type = x.m_base_type;
    m_detail = x.m_detail;
}

MinimalStructHeader::MinimalStructHeader(MinimalStructHeader &&x)
{
    m_base_type = std::move(x.m_base_type);
    m_detail = std::move(x.m_detail);
}

MinimalStructHeader& MinimalStructHeader::operator=(const MinimalStructHeader &x)
{
    m_base_type = x.m_base_type;
    m_detail = x.m_detail;

    return *this;
}

MinimalStructHeader& MinimalStructHeader::operator=(MinimalStructHeader &&x)
{
    m_base_type = std::move(x.m_base_type);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t MinimalStructHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalStructHeader::getCdrSerializedSize(const MinimalStructHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getCdrSerializedSize(data.base_type(), current_alignment);
    current_alignment += MinimalTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalStructHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_base_type;
    scdr << m_detail;
}

void MinimalStructHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_base_type;
    dcdr >> m_detail;
}

size_t MinimalStructHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalStructHeader::isKeyDefined()
{
    return false;
}

void MinimalStructHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteStructType::CompleteStructType()
{
}

CompleteStructType::~CompleteStructType()
{
}

CompleteStructType::CompleteStructType(const CompleteStructType &x)
{
    m_struct_flags = x.m_struct_flags;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;
}

CompleteStructType::CompleteStructType(CompleteStructType &&x)
{
    m_struct_flags = std::move(x.m_struct_flags);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);
}

CompleteStructType& CompleteStructType::operator=(const CompleteStructType &x)
{
    m_struct_flags = x.m_struct_flags;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;

    return *this;
}

CompleteStructType& CompleteStructType::operator=(CompleteStructType &&x)
{
    m_struct_flags = std::move(x.m_struct_flags);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);

    return *this;
}

size_t CompleteStructType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += StructTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteStructHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += CompleteStructMember::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteStructType::getCdrSerializedSize(const CompleteStructType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += StructTypeFlag::getCdrSerializedSize(data.struct_flags(), current_alignment);
    current_alignment += CompleteStructHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.member_seq().size(); ++a)
    {
        current_alignment += CompleteStructMember::getCdrSerializedSize(data.member_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteStructType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_struct_flags;
    scdr << m_header;
    scdr << m_member_seq;
}

void CompleteStructType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_struct_flags;
    dcdr >> m_header;
    dcdr >> m_member_seq;
}

size_t CompleteStructType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteStructType::isKeyDefined()
{
    return false;
}

void CompleteStructType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalStructType::MinimalStructType()
{
}

MinimalStructType::~MinimalStructType()
{
}

MinimalStructType::MinimalStructType(const MinimalStructType &x)
{
    m_struct_flags = x.m_struct_flags;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;
}

MinimalStructType::MinimalStructType(MinimalStructType &&x)
{
    m_struct_flags = std::move(x.m_struct_flags);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);
}

MinimalStructType& MinimalStructType::operator=(const MinimalStructType &x)
{
    m_struct_flags = x.m_struct_flags;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;

    return *this;
}

MinimalStructType& MinimalStructType::operator=(MinimalStructType &&x)
{
    m_struct_flags = std::move(x.m_struct_flags);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);

    return *this;
}

size_t MinimalStructType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += StructTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalStructHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += MinimalStructMember::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t MinimalStructType::getCdrSerializedSize(const MinimalStructType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += StructTypeFlag::getCdrSerializedSize(data.struct_flags(), current_alignment);
    current_alignment += MinimalStructHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.member_seq().size(); ++a)
    {
        current_alignment += MinimalStructMember::getCdrSerializedSize(data.member_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void MinimalStructType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_struct_flags;
    scdr << m_header;
    scdr << m_member_seq;
}

void MinimalStructType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_struct_flags;
    dcdr >> m_header;
    dcdr >> m_member_seq;
}

size_t MinimalStructType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalStructType::isKeyDefined()
{
    return false;
}

void MinimalStructType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonUnionMember::CommonUnionMember()
{
}

CommonUnionMember::~CommonUnionMember()
{
}

CommonUnionMember::CommonUnionMember(const CommonUnionMember &x)
{
	m_member_id = x.m_member_id;
    m_member_flags = x.m_member_flags;
    m_type_id = x.m_type_id;
    m_label_seq = x.m_label_seq;
}

CommonUnionMember::CommonUnionMember(CommonUnionMember &&x)
{
	m_member_id = std::move(x.m_member_id);
    m_member_flags = std::move(x.m_member_flags);
    m_type_id = std::move(x.m_type_id);
    m_label_seq = std::move(x.m_label_seq);
}

CommonUnionMember& CommonUnionMember::operator=(const CommonUnionMember &x)
{
	m_member_id = x.m_member_id;
    m_member_flags = x.m_member_flags;
    m_type_id = x.m_type_id;
    m_label_seq = x.m_label_seq;

    return *this;
}

CommonUnionMember& CommonUnionMember::operator=(CommonUnionMember &&x)
{
	m_member_id = std::move(x.m_member_id);
    m_member_flags = std::move(x.m_member_flags);
    m_type_id = std::move(x.m_type_id);
    m_label_seq = std::move(x.m_label_seq);

    return *this;
}

size_t CommonUnionMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += UnionMemberFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

	current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    }

    return current_alignment - initial_alignment;
}

size_t CommonUnionMember::getCdrSerializedSize(const CommonUnionMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += UnionMemberFlag::getCdrSerializedSize(data.member_flags(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_id(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.label_seq().size(); ++a)
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    }

    return current_alignment - initial_alignment;
}

void CommonUnionMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_member_id;
    scdr << m_member_flags;
    scdr << m_type_id;
    scdr << m_label_seq;
}

void CommonUnionMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_member_id;
    dcdr >> m_member_flags;
    dcdr >> m_type_id;
    dcdr >> m_label_seq;
}

size_t CommonUnionMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonUnionMember::isKeyDefined()
{
    return false;
}

void CommonUnionMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteUnionMember::CompleteUnionMember()
{
}

CompleteUnionMember::~CompleteUnionMember()
{
}

CompleteUnionMember::CompleteUnionMember(const CompleteUnionMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteUnionMember::CompleteUnionMember(CompleteUnionMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteUnionMember& CompleteUnionMember::operator=(const CompleteUnionMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteUnionMember& CompleteUnionMember::operator=(CompleteUnionMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteUnionMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonUnionMember::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteUnionMember::getCdrSerializedSize(const CompleteUnionMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonUnionMember::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteUnionMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteUnionMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteUnionMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteUnionMember::isKeyDefined()
{
    return false;
}

void CompleteUnionMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalUnionMember::MinimalUnionMember()
{
}

MinimalUnionMember::~MinimalUnionMember()
{
}

MinimalUnionMember::MinimalUnionMember(const MinimalUnionMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

MinimalUnionMember::MinimalUnionMember(MinimalUnionMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

MinimalUnionMember& MinimalUnionMember::operator=(const MinimalUnionMember &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

MinimalUnionMember& MinimalUnionMember::operator=(MinimalUnionMember &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t MinimalUnionMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonUnionMember::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalUnionMember::getCdrSerializedSize(const MinimalUnionMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonUnionMember::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += MinimalMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalUnionMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void MinimalUnionMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t MinimalUnionMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalUnionMember::isKeyDefined()
{
    return false;
}

void MinimalUnionMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonDiscriminatorMember::CommonDiscriminatorMember()
{
}

CommonDiscriminatorMember::~CommonDiscriminatorMember()
{
}

CommonDiscriminatorMember::CommonDiscriminatorMember(const CommonDiscriminatorMember &x)
{
    m_member_flags = x.m_member_flags;
    m_type_id = x.m_type_id;
}

CommonDiscriminatorMember::CommonDiscriminatorMember(CommonDiscriminatorMember &&x)
{
    m_member_flags = std::move(x.m_member_flags);
    m_type_id = std::move(x.m_type_id);
}

CommonDiscriminatorMember& CommonDiscriminatorMember::operator=(const CommonDiscriminatorMember &x)
{
    m_member_flags = x.m_member_flags;
    m_type_id = x.m_type_id;

    return *this;
}

CommonDiscriminatorMember& CommonDiscriminatorMember::operator=(CommonDiscriminatorMember &&x)
{
    m_member_flags = std::move(x.m_member_flags);
    m_type_id = std::move(x.m_type_id);

    return *this;
}

size_t CommonDiscriminatorMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += UnionDiscriminatorFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonDiscriminatorMember::getCdrSerializedSize(const CommonDiscriminatorMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += UnionDiscriminatorFlag::getCdrSerializedSize(data.member_flags(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_id(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonDiscriminatorMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_member_flags;
    scdr << m_type_id;
}

void CommonDiscriminatorMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_member_flags;
    dcdr >> m_type_id;
}

size_t CommonDiscriminatorMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonDiscriminatorMember::isKeyDefined()
{
    return false;
}

void CommonDiscriminatorMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteDiscriminatorMember::CompleteDiscriminatorMember()
{
}

CompleteDiscriminatorMember::~CompleteDiscriminatorMember()
{
}

CompleteDiscriminatorMember::CompleteDiscriminatorMember(const CompleteDiscriminatorMember &x)
{
    m_common = x.m_common;
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;
}

CompleteDiscriminatorMember::CompleteDiscriminatorMember(CompleteDiscriminatorMember &&x)
{
    m_common = std::move(x.m_common);
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);
}

CompleteDiscriminatorMember& CompleteDiscriminatorMember::operator=(const CompleteDiscriminatorMember &x)
{
    m_common = x.m_common;
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;

    return *this;
}

CompleteDiscriminatorMember& CompleteDiscriminatorMember::operator=(CompleteDiscriminatorMember &&x)
{
    m_common = std::move(x.m_common);
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);

    return *this;
}

size_t CompleteDiscriminatorMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonDiscriminatorMember::getMaxCdrSerializedSize(current_alignment);
    current_alignment += AppliedBuiltinTypeAnnotations::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotation::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteDiscriminatorMember::getCdrSerializedSize(const CompleteDiscriminatorMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonDiscriminatorMember::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += AppliedBuiltinTypeAnnotations::getCdrSerializedSize(data.ann_builtin(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.ann_custom().size(); ++a)
    {
        current_alignment += AppliedAnnotation::getCdrSerializedSize(data.ann_custom().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteDiscriminatorMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_ann_builtin;
    scdr << m_ann_custom;
}

void CompleteDiscriminatorMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_ann_builtin;
    dcdr >> m_ann_custom;
}

size_t CompleteDiscriminatorMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteDiscriminatorMember::isKeyDefined()
{
    return false;
}

void CompleteDiscriminatorMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalDiscriminatorMember::MinimalDiscriminatorMember()
{
}

MinimalDiscriminatorMember::~MinimalDiscriminatorMember()
{
}

MinimalDiscriminatorMember::MinimalDiscriminatorMember(const MinimalDiscriminatorMember &x)
{
    m_common = x.m_common;
}

MinimalDiscriminatorMember::MinimalDiscriminatorMember(MinimalDiscriminatorMember &&x)
{
    m_common = std::move(x.m_common);
}

MinimalDiscriminatorMember& MinimalDiscriminatorMember::operator=(const MinimalDiscriminatorMember &x)
{
    m_common = x.m_common;

    return *this;
}

MinimalDiscriminatorMember& MinimalDiscriminatorMember::operator=(MinimalDiscriminatorMember &&x)
{
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalDiscriminatorMember::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonDiscriminatorMember::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalDiscriminatorMember::getCdrSerializedSize(const MinimalDiscriminatorMember& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonDiscriminatorMember::getCdrSerializedSize(data.common(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalDiscriminatorMember::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
}

void MinimalDiscriminatorMember::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
}

size_t MinimalDiscriminatorMember::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalDiscriminatorMember::isKeyDefined()
{
    return false;
}

void MinimalDiscriminatorMember::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteUnionHeader::CompleteUnionHeader()
{
}

CompleteUnionHeader::~CompleteUnionHeader()
{
}

CompleteUnionHeader::CompleteUnionHeader(const CompleteUnionHeader &x)
{
    m_detail = x.m_detail;
}

CompleteUnionHeader::CompleteUnionHeader(CompleteUnionHeader &&x)
{
    m_detail = std::move(x.m_detail);
}

CompleteUnionHeader& CompleteUnionHeader::operator=(const CompleteUnionHeader &x)
{
    m_detail = x.m_detail;

    return *this;
}

CompleteUnionHeader& CompleteUnionHeader::operator=(CompleteUnionHeader &&x)
{
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteUnionHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteUnionHeader::getCdrSerializedSize(const CompleteUnionHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteUnionHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_detail;
}

void CompleteUnionHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_detail;
}

size_t CompleteUnionHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteUnionHeader::isKeyDefined()
{
    return false;
}

void CompleteUnionHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalUnionHeader::MinimalUnionHeader()
{
}

MinimalUnionHeader::~MinimalUnionHeader()
{
}

MinimalUnionHeader::MinimalUnionHeader(const MinimalUnionHeader &x)
{
    m_detail = x.m_detail;
}

MinimalUnionHeader::MinimalUnionHeader(MinimalUnionHeader &&x)
{
    m_detail = std::move(x.m_detail);
}

MinimalUnionHeader& MinimalUnionHeader::operator=(const MinimalUnionHeader &x)
{
    m_detail = x.m_detail;

    return *this;
}

MinimalUnionHeader& MinimalUnionHeader::operator=(MinimalUnionHeader &&x)
{
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t MinimalUnionHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += MinimalTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalUnionHeader::getCdrSerializedSize(const MinimalUnionHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += MinimalTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalUnionHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_detail;
}

void MinimalUnionHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_detail;
}

size_t MinimalUnionHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalUnionHeader::isKeyDefined()
{
    return false;
}

void MinimalUnionHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteUnionType::CompleteUnionType()
{
}

CompleteUnionType::~CompleteUnionType()
{
}

CompleteUnionType::CompleteUnionType(const CompleteUnionType &x)
{
    m_union_flags = x.m_union_flags;
    m_header = x.m_header;
    m_discriminator = x.m_discriminator;
    m_member_seq = x.m_member_seq;
}

CompleteUnionType::CompleteUnionType(CompleteUnionType &&x)
{
    m_union_flags = std::move(x.m_union_flags);
    m_header = std::move(x.m_header);
    m_discriminator = std::move(x.m_discriminator);
    m_member_seq = std::move(x.m_member_seq);
}

CompleteUnionType& CompleteUnionType::operator=(const CompleteUnionType &x)
{
    m_union_flags = x.m_union_flags;
    m_header = x.m_header;
    m_discriminator = x.m_discriminator;
    m_member_seq = x.m_member_seq;

    return *this;
}

CompleteUnionType& CompleteUnionType::operator=(CompleteUnionType &&x)
{
    m_union_flags = std::move(x.m_union_flags);
    m_header = std::move(x.m_header);
    m_discriminator = std::move(x.m_discriminator);
    m_member_seq = std::move(x.m_member_seq);

    return *this;
}

size_t CompleteUnionType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

	current_alignment += UnionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteUnionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteDiscriminatorMember::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += CompleteUnionMember::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteUnionType::getCdrSerializedSize(const CompleteUnionType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += UnionTypeFlag::getCdrSerializedSize(data.union_flags(), current_alignment);
    current_alignment += CompleteUnionHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += CompleteDiscriminatorMember::getCdrSerializedSize(data.discriminator(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.member_seq().size(); ++a)
    {
        current_alignment += CompleteUnionMember::getCdrSerializedSize(data.member_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteUnionType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_union_flags;
    scdr << m_header;
    scdr << m_discriminator;
    scdr << m_member_seq;
}

void CompleteUnionType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_union_flags;
    dcdr >> m_header;
    dcdr >> m_discriminator;
    dcdr >> m_member_seq;
}

size_t CompleteUnionType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteUnionType::isKeyDefined()
{
    return false;
}

void CompleteUnionType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalUnionType::MinimalUnionType()
{
}

MinimalUnionType::~MinimalUnionType()
{
}

MinimalUnionType::MinimalUnionType(const MinimalUnionType &x)
{
    m_union_flags = x.m_union_flags;
    m_header = x.m_header;
    m_discriminator = x.m_discriminator;
    m_member_seq = x.m_member_seq;
}

MinimalUnionType::MinimalUnionType(MinimalUnionType &&x)
{
    m_union_flags = std::move(x.m_union_flags);
    m_header = std::move(x.m_header);
    m_discriminator = std::move(x.m_discriminator);
    m_member_seq = std::move(x.m_member_seq);
}

MinimalUnionType& MinimalUnionType::operator=(const MinimalUnionType &x)
{
    m_union_flags = x.m_union_flags;
    m_header = x.m_header;
    m_discriminator = x.m_discriminator;
    m_member_seq = x.m_member_seq;

    return *this;
}

MinimalUnionType& MinimalUnionType::operator=(MinimalUnionType &&x)
{
    m_union_flags = std::move(x.m_union_flags);
    m_header = std::move(x.m_header);
    m_discriminator = std::move(x.m_discriminator);
    m_member_seq = std::move(x.m_member_seq);

    return *this;
}

size_t MinimalUnionType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

	current_alignment += UnionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalUnionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalDiscriminatorMember::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += MinimalUnionMember::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t MinimalUnionType::getCdrSerializedSize(const MinimalUnionType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += UnionTypeFlag::getCdrSerializedSize(data.union_flags(), current_alignment);
    current_alignment += MinimalUnionHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += MinimalDiscriminatorMember::getCdrSerializedSize(data.discriminator(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.member_seq().size(); ++a)
    {
        current_alignment += MinimalUnionMember::getCdrSerializedSize(data.member_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void MinimalUnionType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_union_flags;
    scdr << m_header;
    scdr << m_discriminator;
    scdr << m_member_seq;
}

void MinimalUnionType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_union_flags;
    dcdr >> m_header;
    dcdr >> m_discriminator;
    dcdr >> m_member_seq;
}

size_t MinimalUnionType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalUnionType::isKeyDefined()
{
    return false;
}

void MinimalUnionType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonAnnotationParameter::CommonAnnotationParameter()
{
}

CommonAnnotationParameter::~CommonAnnotationParameter()
{
}

CommonAnnotationParameter::CommonAnnotationParameter(const CommonAnnotationParameter &x)
{
    m_member_flags = x.m_member_flags;
    m_member_type_id = x.m_member_type_id;
}

CommonAnnotationParameter::CommonAnnotationParameter(CommonAnnotationParameter &&x)
{
    m_member_flags = std::move(x.m_member_flags);
    m_member_type_id = std::move(x.m_member_type_id);
}

CommonAnnotationParameter& CommonAnnotationParameter::operator=(const CommonAnnotationParameter &x)
{
    m_member_flags = x.m_member_flags;
    m_member_type_id = x.m_member_type_id;

    return *this;
}

CommonAnnotationParameter& CommonAnnotationParameter::operator=(CommonAnnotationParameter &&x)
{
    m_member_flags = std::move(x.m_member_flags);
    m_member_type_id = std::move(x.m_member_type_id);

    return *this;
}

size_t CommonAnnotationParameter::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AnnotationParameterFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonAnnotationParameter::getCdrSerializedSize(const CommonAnnotationParameter& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AnnotationParameterFlag::getCdrSerializedSize(data.member_flags(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.member_type_id(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonAnnotationParameter::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_member_flags;
    scdr << m_member_type_id;
}

void CommonAnnotationParameter::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_member_flags;
    dcdr >> m_member_type_id;
}

size_t CommonAnnotationParameter::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonAnnotationParameter::isKeyDefined()
{
    return false;
}

void CommonAnnotationParameter::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteAnnotationParameter::CompleteAnnotationParameter()
{
}

CompleteAnnotationParameter::~CompleteAnnotationParameter()
{
}

CompleteAnnotationParameter::CompleteAnnotationParameter(const CompleteAnnotationParameter &x)
{
    m_common = x.m_common;
    m_name = x.m_name;
    m_default_value = x.m_default_value;
}

CompleteAnnotationParameter::CompleteAnnotationParameter(CompleteAnnotationParameter &&x)
{
    m_common = std::move(x.m_common);
    m_name = std::move(x.m_name);
    m_default_value = std::move(x.m_default_value);
}

CompleteAnnotationParameter& CompleteAnnotationParameter::operator=(const CompleteAnnotationParameter &x)
{
    m_common = x.m_common;
    m_name = x.m_name;
    m_default_value = x.m_default_value;

    return *this;
}

CompleteAnnotationParameter& CompleteAnnotationParameter::operator=(CompleteAnnotationParameter &&x)
{
    m_common = std::move(x.m_common);
    m_name = std::move(x.m_name);
    m_default_value = std::move(x.m_default_value);

    return *this;
}

size_t CompleteAnnotationParameter::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAnnotationParameter::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;
    current_alignment += AnnotationParameterValue::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteAnnotationParameter::getCdrSerializedSize(const CompleteAnnotationParameter& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAnnotationParameter::getCdrSerializedSize(data.common(), current_alignment);
	current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.name().size() + 1;
    current_alignment += AnnotationParameterValue::getCdrSerializedSize(data.default_value(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteAnnotationParameter::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_name;
    scdr << m_default_value;
}

void CompleteAnnotationParameter::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_name;
    dcdr >> m_default_value;
}

size_t CompleteAnnotationParameter::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteAnnotationParameter::isKeyDefined()
{
    return false;
}

void CompleteAnnotationParameter::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalAnnotationParameter::MinimalAnnotationParameter()
{
}

MinimalAnnotationParameter::~MinimalAnnotationParameter()
{
}

MinimalAnnotationParameter::MinimalAnnotationParameter(const MinimalAnnotationParameter &x)
{
    m_common = x.m_common;
    m_name = x.m_name;
    m_default_value = x.m_default_value;
}

MinimalAnnotationParameter::MinimalAnnotationParameter(MinimalAnnotationParameter &&x)
{
    m_common = std::move(x.m_common);
    m_name = std::move(x.m_name);
    m_default_value = std::move(x.m_default_value);
}

MinimalAnnotationParameter& MinimalAnnotationParameter::operator=(const MinimalAnnotationParameter &x)
{
    m_common = x.m_common;
    m_name = x.m_name;
    m_default_value = x.m_default_value;

    return *this;
}

MinimalAnnotationParameter& MinimalAnnotationParameter::operator=(MinimalAnnotationParameter &&x)
{
    m_common = std::move(x.m_common);
    m_name = std::move(x.m_name);
    m_default_value = std::move(x.m_default_value);

    return *this;
}

size_t MinimalAnnotationParameter::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAnnotationParameter::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;
    current_alignment += AnnotationParameterValue::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalAnnotationParameter::getCdrSerializedSize(const MinimalAnnotationParameter& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAnnotationParameter::getCdrSerializedSize(data.common(), current_alignment);
	current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.name().size() + 1;
    current_alignment += AnnotationParameterValue::getCdrSerializedSize(data.default_value(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalAnnotationParameter::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_name;
    scdr << m_default_value;
}

void MinimalAnnotationParameter::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_name;
    dcdr >> m_default_value;
}

size_t MinimalAnnotationParameter::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalAnnotationParameter::isKeyDefined()
{
    return false;
}

void MinimalAnnotationParameter::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteAnnotationHeader::CompleteAnnotationHeader()
{
}

CompleteAnnotationHeader::~CompleteAnnotationHeader()
{
}

CompleteAnnotationHeader::CompleteAnnotationHeader(const CompleteAnnotationHeader &x)
{
    m_annotation_name = x.m_annotation_name;
}

CompleteAnnotationHeader::CompleteAnnotationHeader(CompleteAnnotationHeader &&x)
{
    m_annotation_name = std::move(x.m_annotation_name);
}

CompleteAnnotationHeader& CompleteAnnotationHeader::operator=(const CompleteAnnotationHeader &x)
{
    m_annotation_name = x.m_annotation_name;

    return *this;
}

CompleteAnnotationHeader& CompleteAnnotationHeader::operator=(CompleteAnnotationHeader &&x)
{
    m_annotation_name = std::move(x.m_annotation_name);

    return *this;
}

size_t CompleteAnnotationHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;

    return current_alignment - initial_alignment;
}

size_t CompleteAnnotationHeader::getCdrSerializedSize(const CompleteAnnotationHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.annotation_name().size() + 1;

    return current_alignment - initial_alignment;
}

void CompleteAnnotationHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_annotation_name;
}

void CompleteAnnotationHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_annotation_name;
}

size_t CompleteAnnotationHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteAnnotationHeader::isKeyDefined()
{
    return false;
}

void CompleteAnnotationHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalAnnotationHeader::MinimalAnnotationHeader()
{
}

MinimalAnnotationHeader::~MinimalAnnotationHeader()
{
}

MinimalAnnotationHeader::MinimalAnnotationHeader(const MinimalAnnotationHeader &)
{
}

MinimalAnnotationHeader::MinimalAnnotationHeader(MinimalAnnotationHeader &&)
{
}

MinimalAnnotationHeader& MinimalAnnotationHeader::operator=(const MinimalAnnotationHeader &)
{
    return *this;
}

MinimalAnnotationHeader& MinimalAnnotationHeader::operator=(MinimalAnnotationHeader &&)
{
    return *this;
}

size_t MinimalAnnotationHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

size_t MinimalAnnotationHeader::getCdrSerializedSize(const MinimalAnnotationHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

void MinimalAnnotationHeader::serialize(eprosima::fastcdr::Cdr &) const
{
}

void MinimalAnnotationHeader::deserialize(eprosima::fastcdr::Cdr &)
{
}

size_t MinimalAnnotationHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalAnnotationHeader::isKeyDefined()
{
    return false;
}

void MinimalAnnotationHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteAnnotationType::CompleteAnnotationType()
{
}

CompleteAnnotationType::~CompleteAnnotationType()
{
}

CompleteAnnotationType::CompleteAnnotationType(const CompleteAnnotationType &x)
{
    m_annotation_flag = x.m_annotation_flag;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;
}

CompleteAnnotationType::CompleteAnnotationType(CompleteAnnotationType &&x)
{
    m_annotation_flag = std::move(x.m_annotation_flag);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);
}

CompleteAnnotationType& CompleteAnnotationType::operator=(const CompleteAnnotationType &x)
{
    m_annotation_flag = x.m_annotation_flag;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;

    return *this;
}

CompleteAnnotationType& CompleteAnnotationType::operator=(CompleteAnnotationType &&x)
{
    m_annotation_flag = std::move(x.m_annotation_flag);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);

    return *this;
}

size_t CompleteAnnotationType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AnnotationTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteAnnotationHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += CompleteAnnotationParameter::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteAnnotationType::getCdrSerializedSize(const CompleteAnnotationType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AnnotationTypeFlag::getCdrSerializedSize(data.annotation_flag(), current_alignment);
    current_alignment += CompleteAnnotationHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.member_seq().size(); ++a)
    {
        current_alignment += CompleteAnnotationParameter::getCdrSerializedSize(data.member_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteAnnotationType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_annotation_flag;
    scdr << m_header;
    scdr << m_member_seq;
}

void CompleteAnnotationType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_annotation_flag;
    dcdr >> m_header;
    dcdr >> m_member_seq;
}

size_t CompleteAnnotationType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteAnnotationType::isKeyDefined()
{
    return false;
}

void CompleteAnnotationType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalAnnotationType::MinimalAnnotationType()
{
}

MinimalAnnotationType::~MinimalAnnotationType()
{
}

MinimalAnnotationType::MinimalAnnotationType(const MinimalAnnotationType &x)
{
    m_annotation_flag = x.m_annotation_flag;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;
}

MinimalAnnotationType::MinimalAnnotationType(MinimalAnnotationType &&x)
{
    m_annotation_flag = std::move(x.m_annotation_flag);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);
}

MinimalAnnotationType& MinimalAnnotationType::operator=(const MinimalAnnotationType &x)
{
    m_annotation_flag = x.m_annotation_flag;
    m_header = x.m_header;
    m_member_seq = x.m_member_seq;

    return *this;
}

MinimalAnnotationType& MinimalAnnotationType::operator=(MinimalAnnotationType &&x)
{
    m_annotation_flag = std::move(x.m_annotation_flag);
    m_header = std::move(x.m_header);
    m_member_seq = std::move(x.m_member_seq);

    return *this;
}

size_t MinimalAnnotationType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AnnotationTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalAnnotationHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += MinimalAnnotationParameter::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t MinimalAnnotationType::getCdrSerializedSize(const MinimalAnnotationType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AnnotationTypeFlag::getCdrSerializedSize(data.annotation_flag(), current_alignment);
    current_alignment += MinimalAnnotationHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.member_seq().size(); ++a)
    {
        current_alignment += MinimalAnnotationParameter::getCdrSerializedSize(data.member_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void MinimalAnnotationType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_annotation_flag;
    scdr << m_header;
    scdr << m_member_seq;
}

void MinimalAnnotationType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_annotation_flag;
    dcdr >> m_header;
    dcdr >> m_member_seq;
}

size_t MinimalAnnotationType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalAnnotationType::isKeyDefined()
{
    return false;
}

void MinimalAnnotationType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonAliasBody::CommonAliasBody()
{
}

CommonAliasBody::~CommonAliasBody()
{
}

CommonAliasBody::CommonAliasBody(const CommonAliasBody &x)
{
    m_related_flags = x.m_related_flags;
    m_related_type = x.m_related_type;
}

CommonAliasBody::CommonAliasBody(CommonAliasBody &&x)
{
    m_related_flags = std::move(x.m_related_flags);
    m_related_type = std::move(x.m_related_type);
}

CommonAliasBody& CommonAliasBody::operator=(const CommonAliasBody &x)
{
    m_related_flags = x.m_related_flags;
    m_related_type = x.m_related_type;

    return *this;
}

CommonAliasBody& CommonAliasBody::operator=(CommonAliasBody &&x)
{
    m_related_flags = std::move(x.m_related_flags);
    m_related_type = std::move(x.m_related_type);

    return *this;
}

size_t CommonAliasBody::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AliasMemberFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonAliasBody::getCdrSerializedSize(const CommonAliasBody& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AliasMemberFlag::getCdrSerializedSize(data.related_flags(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.related_type(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonAliasBody::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_related_flags;
    scdr << m_related_type;
}

void CommonAliasBody::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_related_flags;
    dcdr >> m_related_type;
}

size_t CommonAliasBody::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonAliasBody::isKeyDefined()
{
    return false;
}

void CommonAliasBody::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteAliasBody::CompleteAliasBody()
{
}

CompleteAliasBody::~CompleteAliasBody()
{
}

CompleteAliasBody::CompleteAliasBody(const CompleteAliasBody &x)
{
    m_common = x.m_common;
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;
}

CompleteAliasBody::CompleteAliasBody(CompleteAliasBody &&x)
{
    m_common = std::move(x.m_common);
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);
}

CompleteAliasBody& CompleteAliasBody::operator=(const CompleteAliasBody &x)
{
    m_common = x.m_common;
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;

    return *this;
}

CompleteAliasBody& CompleteAliasBody::operator=(CompleteAliasBody &&x)
{
    m_common = std::move(x.m_common);
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);

    return *this;
}

size_t CompleteAliasBody::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAliasBody::getMaxCdrSerializedSize(current_alignment);
    current_alignment += AppliedBuiltinMemberAnnotations::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotation::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteAliasBody::getCdrSerializedSize(const CompleteAliasBody& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAliasBody::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += AppliedBuiltinMemberAnnotations::getCdrSerializedSize(data.ann_builtin(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.ann_custom().size(); ++a)
    {
        current_alignment += AppliedAnnotation::getCdrSerializedSize(data.ann_custom().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteAliasBody::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_ann_builtin;
    scdr << m_ann_custom;
}

void CompleteAliasBody::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_ann_builtin;
    dcdr >> m_ann_custom;
}

size_t CompleteAliasBody::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteAliasBody::isKeyDefined()
{
    return false;
}

void CompleteAliasBody::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalAliasBody::MinimalAliasBody()
{
}

MinimalAliasBody::~MinimalAliasBody()
{
}

MinimalAliasBody::MinimalAliasBody(const MinimalAliasBody &x)
{
    m_common = x.m_common;
}

MinimalAliasBody::MinimalAliasBody(MinimalAliasBody &&x)
{
    m_common = std::move(x.m_common);
}

MinimalAliasBody& MinimalAliasBody::operator=(const MinimalAliasBody &x)
{
    m_common = x.m_common;

    return *this;
}

MinimalAliasBody& MinimalAliasBody::operator=(MinimalAliasBody &&x)
{
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalAliasBody::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAliasBody::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalAliasBody::getCdrSerializedSize(const MinimalAliasBody& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonAliasBody::getCdrSerializedSize(data.common(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalAliasBody::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
}

void MinimalAliasBody::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
}

size_t MinimalAliasBody::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalAliasBody::isKeyDefined()
{
    return false;
}

void MinimalAliasBody::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteAliasHeader::CompleteAliasHeader()
{
}

CompleteAliasHeader::~CompleteAliasHeader()
{
}

CompleteAliasHeader::CompleteAliasHeader(const CompleteAliasHeader &x)
{
    m_detail = x.m_detail;
}

CompleteAliasHeader::CompleteAliasHeader(CompleteAliasHeader &&x)
{
    m_detail = std::move(x.m_detail);
}

CompleteAliasHeader& CompleteAliasHeader::operator=(const CompleteAliasHeader &x)
{
    m_detail = x.m_detail;

    return *this;
}

CompleteAliasHeader& CompleteAliasHeader::operator=(CompleteAliasHeader &&x)
{
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteAliasHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteAliasHeader::getCdrSerializedSize(const CompleteAliasHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteAliasHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_detail;
}

void CompleteAliasHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_detail;
}

size_t CompleteAliasHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteAliasHeader::isKeyDefined()
{
    return false;
}

void CompleteAliasHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalAliasHeader::MinimalAliasHeader()
{
}

MinimalAliasHeader::~MinimalAliasHeader()
{
}

MinimalAliasHeader::MinimalAliasHeader(const MinimalAliasHeader &x)
{
}

MinimalAliasHeader::MinimalAliasHeader(MinimalAliasHeader &&x)
{
}

MinimalAliasHeader& MinimalAliasHeader::operator=(const MinimalAliasHeader &x)
{
    return *this;
}

MinimalAliasHeader& MinimalAliasHeader::operator=(MinimalAliasHeader &&x)
{
    return *this;
}

size_t MinimalAliasHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

size_t MinimalAliasHeader::getCdrSerializedSize(const MinimalAliasHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

void MinimalAliasHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
}

void MinimalAliasHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
}

size_t MinimalAliasHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalAliasHeader::isKeyDefined()
{
    return false;
}

void MinimalAliasHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteAliasType::CompleteAliasType()
{
}

CompleteAliasType::~CompleteAliasType()
{
}

CompleteAliasType::CompleteAliasType(const CompleteAliasType &x)
{
    m_alias_flags = x.m_alias_flags;
    m_header = x.m_header;
    m_body = x.m_body;
}

CompleteAliasType::CompleteAliasType(CompleteAliasType &&x)
{
    m_alias_flags = std::move(x.m_alias_flags);
    m_header = std::move(x.m_header);
    m_body = std::move(x.m_body);
}

CompleteAliasType& CompleteAliasType::operator=(const CompleteAliasType &x)
{
    m_alias_flags = x.m_alias_flags;
    m_header = x.m_header;
    m_body = x.m_body;

    return *this;
}

CompleteAliasType& CompleteAliasType::operator=(CompleteAliasType &&x)
{
    m_alias_flags = std::move(x.m_alias_flags);
    m_header = std::move(x.m_header);
    m_body = std::move(x.m_body);

    return *this;
}

size_t CompleteAliasType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AliasTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteAliasHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteAliasBody::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteAliasType::getCdrSerializedSize(const CompleteAliasType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AliasTypeFlag::getCdrSerializedSize(data.alias_flags(), current_alignment);
    current_alignment += CompleteAliasHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += CompleteAliasBody::getCdrSerializedSize(data.body(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteAliasType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_alias_flags;
    scdr << m_header;
    scdr << m_body;
}

void CompleteAliasType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_alias_flags;
    dcdr >> m_header;
    dcdr >> m_body;
}

size_t CompleteAliasType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteAliasType::isKeyDefined()
{
    return false;
}

void CompleteAliasType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalAliasType::MinimalAliasType()
{
}

MinimalAliasType::~MinimalAliasType()
{
}

MinimalAliasType::MinimalAliasType(const MinimalAliasType &x)
{
    m_alias_flags = x.m_alias_flags;
    m_header = x.m_header;
    m_body = x.m_body;
}

MinimalAliasType::MinimalAliasType(MinimalAliasType &&x)
{
    m_alias_flags = std::move(x.m_alias_flags);
    m_header = std::move(x.m_header);
    m_body = std::move(x.m_body);
}

MinimalAliasType& MinimalAliasType::operator=(const MinimalAliasType &x)
{
    m_alias_flags = x.m_alias_flags;
    m_header = x.m_header;
    m_body = x.m_body;

    return *this;
}

MinimalAliasType& MinimalAliasType::operator=(MinimalAliasType &&x)
{
    m_alias_flags = std::move(x.m_alias_flags);
    m_header = std::move(x.m_header);
    m_body = std::move(x.m_body);

    return *this;
}

size_t MinimalAliasType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AliasTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalAliasHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalAliasBody::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalAliasType::getCdrSerializedSize(const MinimalAliasType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AliasTypeFlag::getCdrSerializedSize(data.alias_flags(), current_alignment);
    current_alignment += MinimalAliasHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += MinimalAliasBody::getCdrSerializedSize(data.body(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalAliasType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_alias_flags;
    scdr << m_header;
    scdr << m_body;
}

void MinimalAliasType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_alias_flags;
    dcdr >> m_header;
    dcdr >> m_body;
}

size_t MinimalAliasType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalAliasType::isKeyDefined()
{
    return false;
}

void MinimalAliasType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteElementDetail::CompleteElementDetail()
{
}

CompleteElementDetail::~CompleteElementDetail()
{
}

CompleteElementDetail::CompleteElementDetail(const CompleteElementDetail &x)
{
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;
}

CompleteElementDetail::CompleteElementDetail(CompleteElementDetail &&x)
{
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);
}

CompleteElementDetail& CompleteElementDetail::operator=(const CompleteElementDetail &x)
{
    m_ann_builtin = x.m_ann_builtin;
    m_ann_custom = x.m_ann_custom;

    return *this;
}

CompleteElementDetail& CompleteElementDetail::operator=(CompleteElementDetail &&x)
{
    m_ann_builtin = std::move(x.m_ann_builtin);
    m_ann_custom = std::move(x.m_ann_custom);

    return *this;
}

size_t CompleteElementDetail::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AppliedBuiltinMemberAnnotations::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotation::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteElementDetail::getCdrSerializedSize(const CompleteElementDetail& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += AppliedBuiltinMemberAnnotations::getCdrSerializedSize(data.ann_builtin(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.ann_custom().size(); ++a)
    {
        current_alignment += AppliedAnnotation::getCdrSerializedSize(data.ann_custom().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteElementDetail::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_ann_builtin;
    scdr << m_ann_custom;
}

void CompleteElementDetail::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_ann_builtin;
    dcdr >> m_ann_custom;
}

size_t CompleteElementDetail::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteElementDetail::isKeyDefined()
{
    return false;
}

void CompleteElementDetail::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonCollectionElement::CommonCollectionElement()
{
}

CommonCollectionElement::~CommonCollectionElement()
{
}

CommonCollectionElement::CommonCollectionElement(const CommonCollectionElement &x)
{
    m_element_flags = x.m_element_flags;
    m_type = x.m_type;
}

CommonCollectionElement::CommonCollectionElement(CommonCollectionElement &&x)
{
    m_element_flags = std::move(x.m_element_flags);
    m_type = std::move(x.m_type);
}

CommonCollectionElement& CommonCollectionElement::operator=(const CommonCollectionElement &x)
{
    m_element_flags = x.m_element_flags;
    m_type = x.m_type;

    return *this;
}

CommonCollectionElement& CommonCollectionElement::operator=(CommonCollectionElement &&x)
{
    m_element_flags = std::move(x.m_element_flags);
    m_type = std::move(x.m_type);

    return *this;
}

size_t CommonCollectionElement::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionElementFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonCollectionElement::getCdrSerializedSize(const CommonCollectionElement& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionElementFlag::getCdrSerializedSize(data.element_flags(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonCollectionElement::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_element_flags;
    scdr << m_type;
}

void CommonCollectionElement::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_element_flags;
    dcdr >> m_type;
}

size_t CommonCollectionElement::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonCollectionElement::isKeyDefined()
{
    return false;
}

void CommonCollectionElement::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteCollectionElement::CompleteCollectionElement()
{
}

CompleteCollectionElement::~CompleteCollectionElement()
{
}

CompleteCollectionElement::CompleteCollectionElement(const CompleteCollectionElement &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteCollectionElement::CompleteCollectionElement(CompleteCollectionElement &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteCollectionElement& CompleteCollectionElement::operator=(const CompleteCollectionElement &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteCollectionElement& CompleteCollectionElement::operator=(CompleteCollectionElement &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteCollectionElement::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionElement::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteElementDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteCollectionElement::getCdrSerializedSize(const CompleteCollectionElement& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionElement::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteElementDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteCollectionElement::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteCollectionElement::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteCollectionElement::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteCollectionElement::isKeyDefined()
{
    return false;
}

void CompleteCollectionElement::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalCollectionElement::MinimalCollectionElement()
{
}

MinimalCollectionElement::~MinimalCollectionElement()
{
}

MinimalCollectionElement::MinimalCollectionElement(const MinimalCollectionElement &x)
{
    m_common = x.m_common;
}

MinimalCollectionElement::MinimalCollectionElement(MinimalCollectionElement &&x)
{
    m_common = std::move(x.m_common);
}

MinimalCollectionElement& MinimalCollectionElement::operator=(const MinimalCollectionElement &x)
{
    m_common = x.m_common;

    return *this;
}

MinimalCollectionElement& MinimalCollectionElement::operator=(MinimalCollectionElement &&x)
{
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalCollectionElement::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionElement::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalCollectionElement::getCdrSerializedSize(const MinimalCollectionElement& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionElement::getCdrSerializedSize(data.common(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalCollectionElement::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
}

void MinimalCollectionElement::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
}

size_t MinimalCollectionElement::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalCollectionElement::isKeyDefined()
{
    return false;
}

void MinimalCollectionElement::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonCollectionHeader::CommonCollectionHeader()
{
}

CommonCollectionHeader::~CommonCollectionHeader()
{
}

CommonCollectionHeader::CommonCollectionHeader(const CommonCollectionHeader &x)
{
    m_bound = x.m_bound;
}

CommonCollectionHeader::CommonCollectionHeader(CommonCollectionHeader &&x)
{
    m_bound = std::move(x.m_bound);
}

CommonCollectionHeader& CommonCollectionHeader::operator=(const CommonCollectionHeader &x)
{
    m_bound = x.m_bound;

    return *this;
}

CommonCollectionHeader& CommonCollectionHeader::operator=(CommonCollectionHeader &&x)
{
    m_bound = std::move(x.m_bound);

    return *this;
}

size_t CommonCollectionHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;

    return current_alignment - initial_alignment;
}

size_t CommonCollectionHeader::getCdrSerializedSize(const CommonCollectionHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;

    return current_alignment - initial_alignment;
}

void CommonCollectionHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bound;
}

void CommonCollectionHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bound;
}

size_t CommonCollectionHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonCollectionHeader::isKeyDefined()
{
    return false;
}

void CommonCollectionHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteCollectionHeader::CompleteCollectionHeader()
{
}

CompleteCollectionHeader::~CompleteCollectionHeader()
{
}

CompleteCollectionHeader::CompleteCollectionHeader(const CompleteCollectionHeader &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteCollectionHeader::CompleteCollectionHeader(CompleteCollectionHeader &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteCollectionHeader& CompleteCollectionHeader::operator=(const CompleteCollectionHeader &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteCollectionHeader& CompleteCollectionHeader::operator=(CompleteCollectionHeader &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteCollectionHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteCollectionHeader::getCdrSerializedSize(const CompleteCollectionHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionHeader::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteCollectionHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteCollectionHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteCollectionHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteCollectionHeader::isKeyDefined()
{
    return false;
}

void CompleteCollectionHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalCollectionHeader::MinimalCollectionHeader()
{
}

MinimalCollectionHeader::~MinimalCollectionHeader()
{
}

MinimalCollectionHeader::MinimalCollectionHeader(const MinimalCollectionHeader &x)
{
    m_common = x.m_common;
}

MinimalCollectionHeader::MinimalCollectionHeader(MinimalCollectionHeader &&x)
{
    m_common = std::move(x.m_common);
}

MinimalCollectionHeader& MinimalCollectionHeader::operator=(const MinimalCollectionHeader &x)
{
    m_common = x.m_common;

    return *this;
}

MinimalCollectionHeader& MinimalCollectionHeader::operator=(MinimalCollectionHeader &&x)
{
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalCollectionHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionHeader::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalCollectionHeader::getCdrSerializedSize(const MinimalCollectionHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonCollectionHeader::getCdrSerializedSize(data.common(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalCollectionHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
}

void MinimalCollectionHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
}

size_t MinimalCollectionHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalCollectionHeader::isKeyDefined()
{
    return false;
}

void MinimalCollectionHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteSequenceType::CompleteSequenceType()
{
}

CompleteSequenceType::~CompleteSequenceType()
{
}

CompleteSequenceType::CompleteSequenceType(const CompleteSequenceType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;
}

CompleteSequenceType::CompleteSequenceType(CompleteSequenceType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);
}

CompleteSequenceType& CompleteSequenceType::operator=(const CompleteSequenceType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;

    return *this;
}

CompleteSequenceType& CompleteSequenceType::operator=(CompleteSequenceType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);

    return *this;
}

size_t CompleteSequenceType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    // FIXED SIZE current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    current_alignment += CollectionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteCollectionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteCollectionElement::getMaxCdrSerializedSize(current_alignment);

    // STRING current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;
    // STRING<X> current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + X  + 1;
    // SEQUENCE
    /*
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotationParameter::getMaxCdrSerializedSize(current_alignment);
    }
    */

    return current_alignment - initial_alignment;
}

size_t CompleteSequenceType::getCdrSerializedSize(const CompleteSequenceType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    // FIXED_SIXE current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += CollectionTypeFlag::getCdrSerializedSize(data.collection_flag(), current_alignment);
    current_alignment += CompleteCollectionHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += CompleteCollectionElement::getCdrSerializedSize(data.element(), current_alignment);

    // STRING current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.str().size() + 1;
    // SEQUENCE
    /*
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.param_seq().size(); ++a)
    {
        current_alignment += AppliedAnnotationParameter::getCdrSerializedSize(data.param_seq().at(a), current_alignment);
    }
    */

    return current_alignment - initial_alignment;
}

void CompleteSequenceType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_collection_flag;
    scdr << m_header;
    scdr << m_element;
}

void CompleteSequenceType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_collection_flag;
    dcdr >> m_header;
    dcdr >> m_element;
}

size_t CompleteSequenceType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteSequenceType::isKeyDefined()
{
    return false;
}

void CompleteSequenceType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalSequenceType::MinimalSequenceType()
{
}

MinimalSequenceType::~MinimalSequenceType()
{
}

MinimalSequenceType::MinimalSequenceType(const MinimalSequenceType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;
}

MinimalSequenceType::MinimalSequenceType(MinimalSequenceType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);
}

MinimalSequenceType& MinimalSequenceType::operator=(const MinimalSequenceType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;

    return *this;
}

MinimalSequenceType& MinimalSequenceType::operator=(MinimalSequenceType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);

    return *this;
}

size_t MinimalSequenceType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    // FIXED SIZE current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    current_alignment += CollectionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalCollectionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalCollectionElement::getMaxCdrSerializedSize(current_alignment);

    // STRING current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255  + 1;
    // STRING<X> current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + X  + 1;
    // SEQUENCE
    /*
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += AppliedAnnotationParameter::getMaxCdrSerializedSize(current_alignment);
    }
    */

    return current_alignment - initial_alignment;
}

size_t MinimalSequenceType::getCdrSerializedSize(const MinimalSequenceType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    // FIXED_SIXE current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += CollectionTypeFlag::getCdrSerializedSize(data.collection_flag(), current_alignment);
    current_alignment += MinimalCollectionHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += MinimalCollectionElement::getCdrSerializedSize(data.element(), current_alignment);

    // STRING current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.str().size() + 1;
    // SEQUENCE
    /*
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.param_seq().size(); ++a)
    {
        current_alignment += AppliedAnnotationParameter::getCdrSerializedSize(data.param_seq().at(a), current_alignment);
    }
    */

    return current_alignment - initial_alignment;
}

void MinimalSequenceType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_collection_flag;
    scdr << m_header;
    scdr << m_element;
}

void MinimalSequenceType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_collection_flag;
    dcdr >> m_header;
    dcdr >> m_element;
}

size_t MinimalSequenceType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalSequenceType::isKeyDefined()
{
    return false;
}

void MinimalSequenceType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonArrayHeader::CommonArrayHeader()
{
}

CommonArrayHeader::~CommonArrayHeader()
{
}

CommonArrayHeader::CommonArrayHeader(const CommonArrayHeader &x)
{
    m_bound_seq = x.m_bound_seq;
}

CommonArrayHeader::CommonArrayHeader(CommonArrayHeader &&x)
{
    m_bound_seq = std::move(x.m_bound_seq);
}

CommonArrayHeader& CommonArrayHeader::operator=(const CommonArrayHeader &x)
{
    m_bound_seq = x.m_bound_seq;

    return *this;
}

CommonArrayHeader& CommonArrayHeader::operator=(CommonArrayHeader &&x)
{
    m_bound_seq = std::move(x.m_bound_seq);

    return *this;
}

size_t CommonArrayHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    }

    return current_alignment - initial_alignment;
}

size_t CommonArrayHeader::getCdrSerializedSize(const CommonArrayHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.bound_seq().size(); ++a)
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    }

    return current_alignment - initial_alignment;
}

void CommonArrayHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bound_seq;
}

void CommonArrayHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bound_seq;
}

size_t CommonArrayHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonArrayHeader::isKeyDefined()
{
    return false;
}

void CommonArrayHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteArrayHeader::CompleteArrayHeader()
{
}

CompleteArrayHeader::~CompleteArrayHeader()
{
}

CompleteArrayHeader::CompleteArrayHeader(const CompleteArrayHeader &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteArrayHeader::CompleteArrayHeader(CompleteArrayHeader &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteArrayHeader& CompleteArrayHeader::operator=(const CompleteArrayHeader &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteArrayHeader& CompleteArrayHeader::operator=(CompleteArrayHeader &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteArrayHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonArrayHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteArrayHeader::getCdrSerializedSize(const CompleteArrayHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonArrayHeader::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteArrayHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteArrayHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteArrayHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteArrayHeader::isKeyDefined()
{
    return false;
}

void CompleteArrayHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalArrayHeader::MinimalArrayHeader()
{
}

MinimalArrayHeader::~MinimalArrayHeader()
{
}

MinimalArrayHeader::MinimalArrayHeader(const MinimalArrayHeader &x)
{
    m_common = x.m_common;
}

MinimalArrayHeader::MinimalArrayHeader(MinimalArrayHeader &&x)
{
    m_common = std::move(x.m_common);
}

MinimalArrayHeader& MinimalArrayHeader::operator=(const MinimalArrayHeader &x)
{
    m_common = x.m_common;

    return *this;
}

MinimalArrayHeader& MinimalArrayHeader::operator=(MinimalArrayHeader &&x)
{
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalArrayHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonArrayHeader::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalArrayHeader::getCdrSerializedSize(const MinimalArrayHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonArrayHeader::getCdrSerializedSize(data.common(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalArrayHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
}

void MinimalArrayHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
}

size_t MinimalArrayHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalArrayHeader::isKeyDefined()
{
    return false;
}

void MinimalArrayHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteArrayType::CompleteArrayType()
{
}

CompleteArrayType::~CompleteArrayType()
{
}

CompleteArrayType::CompleteArrayType(const CompleteArrayType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;
}

CompleteArrayType::CompleteArrayType(CompleteArrayType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);
}

CompleteArrayType& CompleteArrayType::operator=(const CompleteArrayType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;

    return *this;
}

CompleteArrayType& CompleteArrayType::operator=(CompleteArrayType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);

    return *this;
}

size_t CompleteArrayType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteArrayHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteCollectionElement::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteArrayType::getCdrSerializedSize(const CompleteArrayType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getCdrSerializedSize(data.collection_flag(), current_alignment);
    current_alignment += CompleteArrayHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += CompleteCollectionElement::getCdrSerializedSize(data.element(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteArrayType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_collection_flag;
    scdr << m_header;
    scdr << m_element;
}

void CompleteArrayType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_collection_flag;
    dcdr >> m_header;
    dcdr >> m_element;
}

size_t CompleteArrayType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteArrayType::isKeyDefined()
{
    return false;
}

void CompleteArrayType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalArrayType::MinimalArrayType()
{
}

MinimalArrayType::~MinimalArrayType()
{
}

MinimalArrayType::MinimalArrayType(const MinimalArrayType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;
}

MinimalArrayType::MinimalArrayType(MinimalArrayType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);
}

MinimalArrayType& MinimalArrayType::operator=(const MinimalArrayType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_element = x.m_element;

    return *this;
}

MinimalArrayType& MinimalArrayType::operator=(MinimalArrayType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_element = std::move(x.m_element);

    return *this;
}

size_t MinimalArrayType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalArrayHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalCollectionElement::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalArrayType::getCdrSerializedSize(const MinimalArrayType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getCdrSerializedSize(data.collection_flag(), current_alignment);
    current_alignment += MinimalArrayHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += MinimalCollectionElement::getCdrSerializedSize(data.element(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalArrayType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_collection_flag;
    scdr << m_header;
    scdr << m_element;
}

void MinimalArrayType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_collection_flag;
    dcdr >> m_header;
    dcdr >> m_element;
}

size_t MinimalArrayType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalArrayType::isKeyDefined()
{
    return false;
}

void MinimalArrayType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteMapType::CompleteMapType()
{
}

CompleteMapType::~CompleteMapType()
{
}

CompleteMapType::CompleteMapType(const CompleteMapType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_key = x.m_key;
    m_element = x.m_element;
}

CompleteMapType::CompleteMapType(CompleteMapType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_key = std::move(x.m_key);
    m_element = std::move(x.m_element);
}

CompleteMapType& CompleteMapType::operator=(const CompleteMapType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_key = x.m_key;
    m_element = x.m_element;

    return *this;
}

CompleteMapType& CompleteMapType::operator=(CompleteMapType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_key = std::move(x.m_key);
    m_element = std::move(x.m_element);

    return *this;
}

size_t CompleteMapType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteCollectionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteCollectionElement::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteCollectionElement::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteMapType::getCdrSerializedSize(const CompleteMapType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getCdrSerializedSize(data.collection_flag(), current_alignment);
    current_alignment += CompleteCollectionHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += CompleteCollectionElement::getCdrSerializedSize(data.key(), current_alignment);
    current_alignment += CompleteCollectionElement::getCdrSerializedSize(data.element(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteMapType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_collection_flag;
    scdr << m_header;
    scdr << m_key;
    scdr << m_element;
}

void CompleteMapType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_collection_flag;
    dcdr >> m_header;
    dcdr >> m_key;
    dcdr >> m_element;
}

size_t CompleteMapType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteMapType::isKeyDefined()
{
    return false;
}

void CompleteMapType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalMapType::MinimalMapType()
{
}

MinimalMapType::~MinimalMapType()
{
}

MinimalMapType::MinimalMapType(const MinimalMapType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_key = x.m_key;
    m_element = x.m_element;
}

MinimalMapType::MinimalMapType(MinimalMapType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_key = std::move(x.m_key);
    m_element = std::move(x.m_element);
}

MinimalMapType& MinimalMapType::operator=(const MinimalMapType &x)
{
    m_collection_flag = x.m_collection_flag;
    m_header = x.m_header;
    m_key = x.m_key;
    m_element = x.m_element;

    return *this;
}

MinimalMapType& MinimalMapType::operator=(MinimalMapType &&x)
{
    m_collection_flag = std::move(x.m_collection_flag);
    m_header = std::move(x.m_header);
    m_key = std::move(x.m_key);
    m_element = std::move(x.m_element);

    return *this;
}

size_t MinimalMapType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalCollectionHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalCollectionElement::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalCollectionElement::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalMapType::getCdrSerializedSize(const MinimalMapType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CollectionTypeFlag::getCdrSerializedSize(data.collection_flag(), current_alignment);
    current_alignment += MinimalCollectionHeader::getCdrSerializedSize(data.header(), current_alignment);
    current_alignment += MinimalCollectionElement::getCdrSerializedSize(data.key(), current_alignment);
    current_alignment += MinimalCollectionElement::getCdrSerializedSize(data.element(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalMapType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_collection_flag;
    scdr << m_header;
    scdr << m_key;
    scdr << m_element;
}

void MinimalMapType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_collection_flag;
    dcdr >> m_header;
    dcdr >> m_key;
    dcdr >> m_element;
}

size_t MinimalMapType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalMapType::isKeyDefined()
{
    return false;
}

void MinimalMapType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonEnumeratedLiteral::CommonEnumeratedLiteral()
{
}

CommonEnumeratedLiteral::~CommonEnumeratedLiteral()
{
}

CommonEnumeratedLiteral::CommonEnumeratedLiteral(const CommonEnumeratedLiteral &x)
{
    m_value = x.m_value;
    m_flags = x.m_flags;
}

CommonEnumeratedLiteral::CommonEnumeratedLiteral(CommonEnumeratedLiteral &&x)
{
    m_value = std::move(x.m_value);
    m_flags = std::move(x.m_flags);
}

CommonEnumeratedLiteral& CommonEnumeratedLiteral::operator=(const CommonEnumeratedLiteral &x)
{
    m_value = x.m_value;
    m_flags = x.m_flags;

    return *this;
}

CommonEnumeratedLiteral& CommonEnumeratedLiteral::operator=(CommonEnumeratedLiteral &&x)
{
    m_value = std::move(x.m_value);
    m_flags = std::move(x.m_flags);

    return *this;
}

size_t CommonEnumeratedLiteral::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += EnumeratedLiteralFlag::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonEnumeratedLiteral::getCdrSerializedSize(const CommonEnumeratedLiteral& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += EnumeratedLiteralFlag::getCdrSerializedSize(data.flags(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonEnumeratedLiteral::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_value;
    scdr << m_flags;
}

void CommonEnumeratedLiteral::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_value;
    dcdr >> m_flags;
}

size_t CommonEnumeratedLiteral::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonEnumeratedLiteral::isKeyDefined()
{
    return false;
}

void CommonEnumeratedLiteral::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteEnumeratedLiteral::CompleteEnumeratedLiteral()
{
}

CompleteEnumeratedLiteral::~CompleteEnumeratedLiteral()
{
}

CompleteEnumeratedLiteral::CompleteEnumeratedLiteral(const CompleteEnumeratedLiteral &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteEnumeratedLiteral::CompleteEnumeratedLiteral(CompleteEnumeratedLiteral &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteEnumeratedLiteral& CompleteEnumeratedLiteral::operator=(const CompleteEnumeratedLiteral &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteEnumeratedLiteral& CompleteEnumeratedLiteral::operator=(CompleteEnumeratedLiteral &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteEnumeratedLiteral::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedLiteral::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteEnumeratedLiteral::getCdrSerializedSize(const CompleteEnumeratedLiteral& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedLiteral::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteEnumeratedLiteral::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteEnumeratedLiteral::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteEnumeratedLiteral::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteEnumeratedLiteral::isKeyDefined()
{
    return false;
}

void CompleteEnumeratedLiteral::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalEnumeratedLiteral::MinimalEnumeratedLiteral()
{
}

MinimalEnumeratedLiteral::~MinimalEnumeratedLiteral()
{
}

MinimalEnumeratedLiteral::MinimalEnumeratedLiteral(const MinimalEnumeratedLiteral &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

MinimalEnumeratedLiteral::MinimalEnumeratedLiteral(MinimalEnumeratedLiteral &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

MinimalEnumeratedLiteral& MinimalEnumeratedLiteral::operator=(const MinimalEnumeratedLiteral &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

MinimalEnumeratedLiteral& MinimalEnumeratedLiteral::operator=(MinimalEnumeratedLiteral &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t MinimalEnumeratedLiteral::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedLiteral::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalEnumeratedLiteral::getCdrSerializedSize(const MinimalEnumeratedLiteral& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedLiteral::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += MinimalMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalEnumeratedLiteral::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void MinimalEnumeratedLiteral::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t MinimalEnumeratedLiteral::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalEnumeratedLiteral::isKeyDefined()
{
    return false;
}

void MinimalEnumeratedLiteral::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonEnumeratedHeader::CommonEnumeratedHeader()
{
}

CommonEnumeratedHeader::~CommonEnumeratedHeader()
{
}

CommonEnumeratedHeader::CommonEnumeratedHeader(const CommonEnumeratedHeader &x)
{
    m_bit_bound = x.m_bit_bound;
}

CommonEnumeratedHeader::CommonEnumeratedHeader(CommonEnumeratedHeader &&x)
{
    m_bit_bound = std::move(x.m_bit_bound);
}

CommonEnumeratedHeader& CommonEnumeratedHeader::operator=(const CommonEnumeratedHeader &x)
{
    m_bit_bound = x.m_bit_bound;

    return *this;
}

CommonEnumeratedHeader& CommonEnumeratedHeader::operator=(CommonEnumeratedHeader &&x)
{
    m_bit_bound = std::move(x.m_bit_bound);

    return *this;
}

size_t CommonEnumeratedHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    return current_alignment - initial_alignment;
}

size_t CommonEnumeratedHeader::getCdrSerializedSize(const CommonEnumeratedHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    return current_alignment - initial_alignment;
}

void CommonEnumeratedHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bit_bound;
}

void CommonEnumeratedHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bit_bound;
}

size_t CommonEnumeratedHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonEnumeratedHeader::isKeyDefined()
{
    return false;
}

void CommonEnumeratedHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteEnumeratedHeader::CompleteEnumeratedHeader()
{
}

CompleteEnumeratedHeader::~CompleteEnumeratedHeader()
{
}

CompleteEnumeratedHeader::CompleteEnumeratedHeader(const CompleteEnumeratedHeader &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteEnumeratedHeader::CompleteEnumeratedHeader(CompleteEnumeratedHeader &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteEnumeratedHeader& CompleteEnumeratedHeader::operator=(const CompleteEnumeratedHeader &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteEnumeratedHeader& CompleteEnumeratedHeader::operator=(CompleteEnumeratedHeader &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteEnumeratedHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedHeader::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteEnumeratedHeader::getCdrSerializedSize(const CompleteEnumeratedHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedHeader::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteEnumeratedHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteEnumeratedHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteEnumeratedHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteEnumeratedHeader::isKeyDefined()
{
    return false;
}

void CompleteEnumeratedHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalEnumeratedHeader::MinimalEnumeratedHeader()
{
}

MinimalEnumeratedHeader::~MinimalEnumeratedHeader()
{
}

MinimalEnumeratedHeader::MinimalEnumeratedHeader(const MinimalEnumeratedHeader &x)
{
    m_common = x.m_common;
}

MinimalEnumeratedHeader::MinimalEnumeratedHeader(MinimalEnumeratedHeader &&x)
{
    m_common = std::move(x.m_common);
}

MinimalEnumeratedHeader& MinimalEnumeratedHeader::operator=(const MinimalEnumeratedHeader &x)
{
    m_common = x.m_common;

    return *this;
}

MinimalEnumeratedHeader& MinimalEnumeratedHeader::operator=(MinimalEnumeratedHeader &&x)
{
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalEnumeratedHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedHeader::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalEnumeratedHeader::getCdrSerializedSize(const MinimalEnumeratedHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonEnumeratedHeader::getCdrSerializedSize(data.common(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalEnumeratedHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
}

void MinimalEnumeratedHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
}

size_t MinimalEnumeratedHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalEnumeratedHeader::isKeyDefined()
{
    return false;
}

void MinimalEnumeratedHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteEnumeratedType::CompleteEnumeratedType()
{
}

CompleteEnumeratedType::~CompleteEnumeratedType()
{
}

CompleteEnumeratedType::CompleteEnumeratedType(const CompleteEnumeratedType &x)
{
    m_enum_flags = x.m_enum_flags;
    m_header = x.m_header;
    m_literal_seq = x.m_literal_seq;
}

CompleteEnumeratedType::CompleteEnumeratedType(CompleteEnumeratedType &&x)
{
    m_enum_flags = std::move(x.m_enum_flags);
    m_header = std::move(x.m_header);
    m_literal_seq = std::move(x.m_literal_seq);
}

CompleteEnumeratedType& CompleteEnumeratedType::operator=(const CompleteEnumeratedType &x)
{
    m_enum_flags = x.m_enum_flags;
    m_header = x.m_header;
    m_literal_seq = x.m_literal_seq;

    return *this;
}

CompleteEnumeratedType& CompleteEnumeratedType::operator=(CompleteEnumeratedType &&x)
{
    m_enum_flags = std::move(x.m_enum_flags);
    m_header = std::move(x.m_header);
    m_literal_seq = std::move(x.m_literal_seq);

    return *this;
}

size_t CompleteEnumeratedType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += EnumTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteEnumeratedHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += CompleteEnumeratedLiteral::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteEnumeratedType::getCdrSerializedSize(const CompleteEnumeratedType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += EnumTypeFlag::getCdrSerializedSize(data.enum_flags(), current_alignment);
    current_alignment += CompleteEnumeratedHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.literal_seq().size(); ++a)
    {
        current_alignment += CompleteEnumeratedLiteral::getCdrSerializedSize(data.literal_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteEnumeratedType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_enum_flags;
    scdr << m_header;
    scdr << m_literal_seq;
}

void CompleteEnumeratedType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_enum_flags;
    dcdr >> m_header;
    dcdr >> m_literal_seq;
}

size_t CompleteEnumeratedType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteEnumeratedType::isKeyDefined()
{
    return false;
}

void CompleteEnumeratedType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalEnumeratedType::MinimalEnumeratedType()
{
}

MinimalEnumeratedType::~MinimalEnumeratedType()
{
}

MinimalEnumeratedType::MinimalEnumeratedType(const MinimalEnumeratedType &x)
{
    m_enum_flags = x.m_enum_flags;
    m_header = x.m_header;
    m_literal_seq = x.m_literal_seq;
}

MinimalEnumeratedType::MinimalEnumeratedType(MinimalEnumeratedType &&x)
{
    m_enum_flags = std::move(x.m_enum_flags);
    m_header = std::move(x.m_header);
    m_literal_seq = std::move(x.m_literal_seq);
}

MinimalEnumeratedType& MinimalEnumeratedType::operator=(const MinimalEnumeratedType &x)
{
    m_enum_flags = x.m_enum_flags;
    m_header = x.m_header;
    m_literal_seq = x.m_literal_seq;

    return *this;
}

MinimalEnumeratedType& MinimalEnumeratedType::operator=(MinimalEnumeratedType &&x)
{
    m_enum_flags = std::move(x.m_enum_flags);
    m_header = std::move(x.m_header);
    m_literal_seq = std::move(x.m_literal_seq);

    return *this;
}

size_t MinimalEnumeratedType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += EnumTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalEnumeratedHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += MinimalEnumeratedLiteral::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t MinimalEnumeratedType::getCdrSerializedSize(const MinimalEnumeratedType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += EnumTypeFlag::getCdrSerializedSize(data.enum_flags(), current_alignment);
    current_alignment += MinimalEnumeratedHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.literal_seq().size(); ++a)
    {
        current_alignment += MinimalEnumeratedLiteral::getCdrSerializedSize(data.literal_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void MinimalEnumeratedType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_enum_flags;
    scdr << m_header;
    scdr << m_literal_seq;
}

void MinimalEnumeratedType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_enum_flags;
    dcdr >> m_header;
    dcdr >> m_literal_seq;
}

size_t MinimalEnumeratedType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalEnumeratedType::isKeyDefined()
{
    return false;
}

void MinimalEnumeratedType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonBitflag::CommonBitflag()
{
}

CommonBitflag::~CommonBitflag()
{
}

CommonBitflag::CommonBitflag(const CommonBitflag &x)
{
    m_position = x.m_position;
    m_flags = x.m_flags;
}

CommonBitflag::CommonBitflag(CommonBitflag &&x)
{
    m_position = std::move(x.m_position);
    m_flags = std::move(x.m_flags);
}

CommonBitflag& CommonBitflag::operator=(const CommonBitflag &x)
{
    m_position = x.m_position;
    m_flags = x.m_flags;

    return *this;
}

CommonBitflag& CommonBitflag::operator=(CommonBitflag &&x)
{
    m_position = std::move(x.m_position);
    m_flags = std::move(x.m_flags);

    return *this;
}

size_t CommonBitflag::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
    current_alignment += BitflagFlag::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CommonBitflag::getCdrSerializedSize(const CommonBitflag& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
    current_alignment += BitflagFlag::getCdrSerializedSize(data.flags(), current_alignment);

    return current_alignment - initial_alignment;
}

void CommonBitflag::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_position;
    scdr << m_flags;
}

void CommonBitflag::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_position;
    dcdr >> m_flags;
}

size_t CommonBitflag::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonBitflag::isKeyDefined()
{
    return false;
}

void CommonBitflag::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteBitflag::CompleteBitflag()
{
}

CompleteBitflag::~CompleteBitflag()
{
}

CompleteBitflag::CompleteBitflag(const CompleteBitflag &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteBitflag::CompleteBitflag(CompleteBitflag &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteBitflag& CompleteBitflag::operator=(const CompleteBitflag &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteBitflag& CompleteBitflag::operator=(CompleteBitflag &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteBitflag::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitflag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteBitflag::getCdrSerializedSize(const CompleteBitflag& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitflag::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteBitflag::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteBitflag::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteBitflag::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteBitflag::isKeyDefined()
{
    return false;
}

void CompleteBitflag::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalBitflag::MinimalBitflag()
{
}

MinimalBitflag::~MinimalBitflag()
{
}

MinimalBitflag::MinimalBitflag(const MinimalBitflag &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

MinimalBitflag::MinimalBitflag(MinimalBitflag &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

MinimalBitflag& MinimalBitflag::operator=(const MinimalBitflag &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

MinimalBitflag& MinimalBitflag::operator=(MinimalBitflag &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t MinimalBitflag::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitflag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t MinimalBitflag::getCdrSerializedSize(const MinimalBitflag& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitflag::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += MinimalMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void MinimalBitflag::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void MinimalBitflag::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t MinimalBitflag::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalBitflag::isKeyDefined()
{
    return false;
}

void MinimalBitflag::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonBitmaskHeader::CommonBitmaskHeader()
{
}

CommonBitmaskHeader::~CommonBitmaskHeader()
{
}

CommonBitmaskHeader::CommonBitmaskHeader(const CommonBitmaskHeader &x)
{
    m_bit_bound = x.m_bit_bound;
}

CommonBitmaskHeader::CommonBitmaskHeader(CommonBitmaskHeader &&x)
{
    m_bit_bound = std::move(x.m_bit_bound);
}

CommonBitmaskHeader& CommonBitmaskHeader::operator=(const CommonBitmaskHeader &x)
{
    m_bit_bound = x.m_bit_bound;

    return *this;
}

CommonBitmaskHeader& CommonBitmaskHeader::operator=(CommonBitmaskHeader &&x)
{
    m_bit_bound = std::move(x.m_bit_bound);

    return *this;
}

size_t CommonBitmaskHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    return current_alignment - initial_alignment;
}

size_t CommonBitmaskHeader::getCdrSerializedSize(const CommonBitmaskHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    return current_alignment - initial_alignment;
}

void CommonBitmaskHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bit_bound;
}

void CommonBitmaskHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bit_bound;
}

size_t CommonBitmaskHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonBitmaskHeader::isKeyDefined()
{
    return false;
}

void CommonBitmaskHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteBitmaskType::CompleteBitmaskType()
{
}

CompleteBitmaskType::~CompleteBitmaskType()
{
}

CompleteBitmaskType::CompleteBitmaskType(const CompleteBitmaskType &x)
{
    m_bitmask_flags = x.m_bitmask_flags;
    m_header = x.m_header;
    m_flag_seq = x.m_flag_seq;
}

CompleteBitmaskType::CompleteBitmaskType(CompleteBitmaskType &&x)
{
    m_bitmask_flags = std::move(x.m_bitmask_flags);
    m_header = std::move(x.m_header);
    m_flag_seq = std::move(x.m_flag_seq);
}

CompleteBitmaskType& CompleteBitmaskType::operator=(const CompleteBitmaskType &x)
{
    m_bitmask_flags = x.m_bitmask_flags;
    m_header = x.m_header;
    m_flag_seq = x.m_flag_seq;

    return *this;
}

CompleteBitmaskType& CompleteBitmaskType::operator=(CompleteBitmaskType &&x)
{
    m_bitmask_flags = std::move(x.m_bitmask_flags);
    m_header = std::move(x.m_header);
    m_flag_seq = std::move(x.m_flag_seq);

    return *this;
}

size_t CompleteBitmaskType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitmaskTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteBitmaskHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += CompleteBitflag::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteBitmaskType::getCdrSerializedSize(const CompleteBitmaskType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitmaskTypeFlag::getCdrSerializedSize(data.bitmask_flags(), current_alignment);
    current_alignment += CompleteBitmaskHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.flag_seq().size(); ++a)
    {
        current_alignment += CompleteBitflag::getCdrSerializedSize(data.flag_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteBitmaskType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bitmask_flags;
    scdr << m_header;
    scdr << m_flag_seq;
}

void CompleteBitmaskType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bitmask_flags;
    dcdr >> m_header;
    dcdr >> m_flag_seq;
}

size_t CompleteBitmaskType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteBitmaskType::isKeyDefined()
{
    return false;
}

void CompleteBitmaskType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalBitmaskType::MinimalBitmaskType()
{
}

MinimalBitmaskType::~MinimalBitmaskType()
{
}

MinimalBitmaskType::MinimalBitmaskType(const MinimalBitmaskType &x)
{
    m_bitmask_flags = x.m_bitmask_flags;
    m_header = x.m_header;
    m_flag_seq = x.m_flag_seq;
}

MinimalBitmaskType::MinimalBitmaskType(MinimalBitmaskType &&x)
{
    m_bitmask_flags = std::move(x.m_bitmask_flags);
    m_header = std::move(x.m_header);
    m_flag_seq = std::move(x.m_flag_seq);
}

MinimalBitmaskType& MinimalBitmaskType::operator=(const MinimalBitmaskType &x)
{
    m_bitmask_flags = x.m_bitmask_flags;
    m_header = x.m_header;
    m_flag_seq = x.m_flag_seq;

    return *this;
}

MinimalBitmaskType& MinimalBitmaskType::operator=(MinimalBitmaskType &&x)
{
    m_bitmask_flags = std::move(x.m_bitmask_flags);
    m_header = std::move(x.m_header);
    m_flag_seq = std::move(x.m_flag_seq);

    return *this;
}

size_t MinimalBitmaskType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitmaskTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalBitmaskHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += MinimalBitflag::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t MinimalBitmaskType::getCdrSerializedSize(const MinimalBitmaskType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitmaskTypeFlag::getCdrSerializedSize(data.bitmask_flags(), current_alignment);
    current_alignment += MinimalBitmaskHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.flag_seq().size(); ++a)
    {
        current_alignment += MinimalBitflag::getCdrSerializedSize(data.flag_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void MinimalBitmaskType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bitmask_flags;
    scdr << m_header;
    scdr << m_flag_seq;
}

void MinimalBitmaskType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bitmask_flags;
    dcdr >> m_header;
    dcdr >> m_flag_seq;
}

size_t MinimalBitmaskType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalBitmaskType::isKeyDefined()
{
    return false;
}

void MinimalBitmaskType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CommonBitfield::CommonBitfield()
{
}

CommonBitfield::~CommonBitfield()
{
}

CommonBitfield::CommonBitfield(const CommonBitfield &x)
{
    m_position = x.m_position;
    m_flags = x.m_flags;
    m_bitcount = x.m_bitcount;
    m_holder_type = x.m_holder_type;
}

CommonBitfield::CommonBitfield(CommonBitfield &&x)
{
    m_position = std::move(x.m_position);
    m_flags = std::move(x.m_flags);
    m_bitcount = std::move(x.m_bitcount);
    m_holder_type = std::move(x.m_holder_type);
}

CommonBitfield& CommonBitfield::operator=(const CommonBitfield &x)
{
    m_position = x.m_position;
    m_flags = x.m_flags;
    m_bitcount = x.m_bitcount;
    m_holder_type = x.m_holder_type;

    return *this;
}

CommonBitfield& CommonBitfield::operator=(CommonBitfield &&x)
{
    m_position = std::move(x.m_position);
    m_flags = std::move(x.m_flags);
    m_bitcount = std::move(x.m_bitcount);
    m_holder_type = std::move(x.m_holder_type);

    return *this;
}

size_t CommonBitfield::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
    current_alignment += BitsetMemberFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

size_t CommonBitfield::getCdrSerializedSize(const CommonBitfield& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
    current_alignment += BitsetMemberFlag::getCdrSerializedSize(data.flags(), current_alignment);
    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

void CommonBitfield::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_position;
    scdr << m_flags;
    scdr << m_bitcount;
    scdr << m_holder_type;
}

void CommonBitfield::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_position;
    dcdr >> m_flags;
    dcdr >> m_bitcount;
    dcdr >> m_holder_type;
}

size_t CommonBitfield::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CommonBitfield::isKeyDefined()
{
    return false;
}

void CommonBitfield::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteBitfield::CompleteBitfield()
{
}

CompleteBitfield::~CompleteBitfield()
{
}

CompleteBitfield::CompleteBitfield(const CompleteBitfield &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;
}

CompleteBitfield::CompleteBitfield(CompleteBitfield &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);
}

CompleteBitfield& CompleteBitfield::operator=(const CompleteBitfield &x)
{
    m_common = x.m_common;
    m_detail = x.m_detail;

    return *this;
}

CompleteBitfield& CompleteBitfield::operator=(CompleteBitfield &&x)
{
    m_common = std::move(x.m_common);
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteBitfield::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitfield::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteMemberDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteBitfield::getCdrSerializedSize(const CompleteBitfield& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitfield::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += CompleteMemberDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteBitfield::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_detail;
}

void CompleteBitfield::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_detail;
}

size_t CompleteBitfield::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteBitfield::isKeyDefined()
{
    return false;
}

void CompleteBitfield::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalBitfield::MinimalBitfield()
{
}

MinimalBitfield::~MinimalBitfield()
{
}

MinimalBitfield::MinimalBitfield(const MinimalBitfield &x)
{
    m_name_hash = x.m_name_hash;
    m_common = x.m_common;
}

MinimalBitfield::MinimalBitfield(MinimalBitfield &&x)
{
    m_name_hash = std::move(x.m_name_hash);
    m_common = std::move(x.m_common);
}

MinimalBitfield& MinimalBitfield::operator=(const MinimalBitfield &x)
{
    m_name_hash = x.m_name_hash;
    m_common = x.m_common;

    return *this;
}

MinimalBitfield& MinimalBitfield::operator=(MinimalBitfield &&x)
{
    m_name_hash = std::move(x.m_name_hash);
    m_common = std::move(x.m_common);

    return *this;
}

size_t MinimalBitfield::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitfield::getMaxCdrSerializedSize(current_alignment);
    current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

size_t MinimalBitfield::getCdrSerializedSize(const MinimalBitfield& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CommonBitfield::getCdrSerializedSize(data.common(), current_alignment);
    current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

void MinimalBitfield::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_common;
    scdr << m_name_hash;
}

void MinimalBitfield::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_common;
    dcdr >> m_name_hash;
}

size_t MinimalBitfield::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalBitfield::isKeyDefined()
{
    return false;
}

void MinimalBitfield::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteBitsetHeader::CompleteBitsetHeader()
{
}

CompleteBitsetHeader::~CompleteBitsetHeader()
{
}

CompleteBitsetHeader::CompleteBitsetHeader(const CompleteBitsetHeader &x)
{
    m_detail = x.m_detail;
}

CompleteBitsetHeader::CompleteBitsetHeader(CompleteBitsetHeader &&x)
{
    m_detail = std::move(x.m_detail);
}

CompleteBitsetHeader& CompleteBitsetHeader::operator=(const CompleteBitsetHeader &x)
{
    m_detail = x.m_detail;

    return *this;
}

CompleteBitsetHeader& CompleteBitsetHeader::operator=(CompleteBitsetHeader &&x)
{
    m_detail = std::move(x.m_detail);

    return *this;
}

size_t CompleteBitsetHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += CompleteTypeDetail::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t CompleteBitsetHeader::getCdrSerializedSize(const CompleteBitsetHeader& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += CompleteTypeDetail::getCdrSerializedSize(data.detail(), current_alignment);

    return current_alignment - initial_alignment;
}

void CompleteBitsetHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_detail;
}

void CompleteBitsetHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_detail;
}

size_t CompleteBitsetHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteBitsetHeader::isKeyDefined()
{
    return false;
}

void CompleteBitsetHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalBitsetHeader::MinimalBitsetHeader()
{
}

MinimalBitsetHeader::~MinimalBitsetHeader()
{
}

MinimalBitsetHeader::MinimalBitsetHeader(const MinimalBitsetHeader &)
{
}

MinimalBitsetHeader::MinimalBitsetHeader(MinimalBitsetHeader &&)
{
}

MinimalBitsetHeader& MinimalBitsetHeader::operator=(const MinimalBitsetHeader &)
{
    return *this;
}

MinimalBitsetHeader& MinimalBitsetHeader::operator=(MinimalBitsetHeader &&)
{
    return *this;
}

size_t MinimalBitsetHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

size_t MinimalBitsetHeader::getCdrSerializedSize(const MinimalBitsetHeader& , size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

void MinimalBitsetHeader::serialize(eprosima::fastcdr::Cdr &) const
{
}

void MinimalBitsetHeader::deserialize(eprosima::fastcdr::Cdr &)
{
}

size_t MinimalBitsetHeader::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalBitsetHeader::isKeyDefined()
{
    return false;
}

void MinimalBitsetHeader::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteBitsetType::CompleteBitsetType()
{
}

CompleteBitsetType::~CompleteBitsetType()
{
}

CompleteBitsetType::CompleteBitsetType(const CompleteBitsetType &x)
{
    m_bitset_flags = x.m_bitset_flags;
    m_header = x.m_header;
    m_field_seq = x.m_field_seq;
}

CompleteBitsetType::CompleteBitsetType(CompleteBitsetType &&x)
{
    m_bitset_flags = std::move(x.m_bitset_flags);
    m_header = std::move(x.m_header);
    m_field_seq = std::move(x.m_field_seq);
}

CompleteBitsetType& CompleteBitsetType::operator=(const CompleteBitsetType &x)
{
    m_bitset_flags = x.m_bitset_flags;
    m_header = x.m_header;
    m_field_seq = x.m_field_seq;

    return *this;
}

CompleteBitsetType& CompleteBitsetType::operator=(CompleteBitsetType &&x)
{
    m_bitset_flags = std::move(x.m_bitset_flags);
    m_header = std::move(x.m_header);
    m_field_seq = std::move(x.m_field_seq);

    return *this;
}

size_t CompleteBitsetType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitsetTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += CompleteBitsetHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += CompleteBitfield::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t CompleteBitsetType::getCdrSerializedSize(const CompleteBitsetType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitsetTypeFlag::getCdrSerializedSize(data.bitset_flags(), current_alignment);
    current_alignment += CompleteBitsetHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.field_seq().size(); ++a)
    {
        current_alignment += CompleteBitfield::getCdrSerializedSize(data.field_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void CompleteBitsetType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bitset_flags;
    scdr << m_header;
    scdr << m_field_seq;
}

void CompleteBitsetType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bitset_flags;
    dcdr >> m_header;
    dcdr >> m_field_seq;
}

size_t CompleteBitsetType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteBitsetType::isKeyDefined()
{
    return false;
}

void CompleteBitsetType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalBitsetType::MinimalBitsetType()
{
}

MinimalBitsetType::~MinimalBitsetType()
{
}

MinimalBitsetType::MinimalBitsetType(const MinimalBitsetType &x)
{
    m_bitset_flags = x.m_bitset_flags;
    m_header = x.m_header;
    m_field_seq = x.m_field_seq;
}

MinimalBitsetType::MinimalBitsetType(MinimalBitsetType &&x)
{
    m_bitset_flags = std::move(x.m_bitset_flags);
    m_header = std::move(x.m_header);
    m_field_seq = std::move(x.m_field_seq);
}

MinimalBitsetType& MinimalBitsetType::operator=(const MinimalBitsetType &x)
{
    m_bitset_flags = x.m_bitset_flags;
    m_header = x.m_header;
    m_field_seq = x.m_field_seq;

    return *this;
}

MinimalBitsetType& MinimalBitsetType::operator=(MinimalBitsetType &&x)
{
    m_bitset_flags = std::move(x.m_bitset_flags);
    m_header = std::move(x.m_header);
    m_field_seq = std::move(x.m_field_seq);

    return *this;
}

size_t MinimalBitsetType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitsetTypeFlag::getMaxCdrSerializedSize(current_alignment);
    current_alignment += MinimalBitsetHeader::getMaxCdrSerializedSize(current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += MinimalBitfield::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t MinimalBitsetType::getCdrSerializedSize(const MinimalBitsetType& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += BitsetTypeFlag::getCdrSerializedSize(data.bitset_flags(), current_alignment);
    current_alignment += MinimalBitsetHeader::getCdrSerializedSize(data.header(), current_alignment);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.field_seq().size(); ++a)
    {
        current_alignment += MinimalBitfield::getCdrSerializedSize(data.field_seq().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void MinimalBitsetType::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_bitset_flags;
    scdr << m_header;
    scdr << m_field_seq;
}

void MinimalBitsetType::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_bitset_flags;
    dcdr >> m_header;
    dcdr >> m_field_seq;
}

size_t MinimalBitsetType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalBitsetType::isKeyDefined()
{
    return false;
}

void MinimalBitsetType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

CompleteExtendedType::CompleteExtendedType()
{
}

CompleteExtendedType::~CompleteExtendedType()
{
}

CompleteExtendedType::CompleteExtendedType(const CompleteExtendedType &x)
{
}

CompleteExtendedType::CompleteExtendedType(CompleteExtendedType &&x)
{
}

CompleteExtendedType& CompleteExtendedType::operator=(const CompleteExtendedType &x)
{
    return *this;
}

CompleteExtendedType& CompleteExtendedType::operator=(CompleteExtendedType &&x)
{
    return *this;
}

size_t CompleteExtendedType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

size_t CompleteExtendedType::getCdrSerializedSize(const CompleteExtendedType& , size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

void CompleteExtendedType::serialize(eprosima::fastcdr::Cdr &) const
{
}

void CompleteExtendedType::deserialize(eprosima::fastcdr::Cdr &)
{
}

size_t CompleteExtendedType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool CompleteExtendedType::isKeyDefined()
{
    return false;
}

void CompleteExtendedType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

MinimalExtendedType::MinimalExtendedType()
{
}

MinimalExtendedType::~MinimalExtendedType()
{
}

MinimalExtendedType::MinimalExtendedType(const MinimalExtendedType &x)
{
}

MinimalExtendedType::MinimalExtendedType(MinimalExtendedType &&x)
{
}

MinimalExtendedType& MinimalExtendedType::operator=(const MinimalExtendedType &x)
{
    return *this;
}

MinimalExtendedType& MinimalExtendedType::operator=(MinimalExtendedType &&x)
{
    return *this;
}

size_t MinimalExtendedType::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

size_t MinimalExtendedType::getCdrSerializedSize(const MinimalExtendedType& , size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    return current_alignment - initial_alignment;
}

void MinimalExtendedType::serialize(eprosima::fastcdr::Cdr &) const
{
}

void MinimalExtendedType::deserialize(eprosima::fastcdr::Cdr &)
{
}

size_t MinimalExtendedType::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool MinimalExtendedType::isKeyDefined()
{
    return false;
}

void MinimalExtendedType::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

TypeIdentifierTypeObjectPair::TypeIdentifierTypeObjectPair()
{
}

TypeIdentifierTypeObjectPair::~TypeIdentifierTypeObjectPair()
{
}

TypeIdentifierTypeObjectPair::TypeIdentifierTypeObjectPair(const TypeIdentifierTypeObjectPair &x)
{
    m_type_identifier = x.m_type_identifier;
    m_type_object = x.m_type_object;
}

TypeIdentifierTypeObjectPair::TypeIdentifierTypeObjectPair(TypeIdentifierTypeObjectPair &&x)
{
    m_type_identifier = std::move(x.m_type_identifier);
    m_type_object = std::move(x.m_type_object);
}

TypeIdentifierTypeObjectPair& TypeIdentifierTypeObjectPair::operator=(const TypeIdentifierTypeObjectPair &x)
{
    m_type_identifier = x.m_type_identifier;
    m_type_object = x.m_type_object;

    return *this;
}

TypeIdentifierTypeObjectPair& TypeIdentifierTypeObjectPair::operator=(TypeIdentifierTypeObjectPair &&x)
{
    m_type_identifier = std::move(x.m_type_identifier);
    m_type_object = std::move(x.m_type_object);

    return *this;
}

size_t TypeIdentifierTypeObjectPair::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeObject::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t TypeIdentifierTypeObjectPair::getCdrSerializedSize(const TypeIdentifierTypeObjectPair& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_identifier(), current_alignment);
    current_alignment += TypeObject::getCdrSerializedSize(data.type_object(), current_alignment);

    return current_alignment - initial_alignment;
}

void TypeIdentifierTypeObjectPair::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_type_identifier;
    scdr << m_type_object;
}

void TypeIdentifierTypeObjectPair::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_type_identifier;
    dcdr >> m_type_object;
}

size_t TypeIdentifierTypeObjectPair::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool TypeIdentifierTypeObjectPair::isKeyDefined()
{
    return false;
}

void TypeIdentifierTypeObjectPair::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

TypeIdentifierPair::TypeIdentifierPair()
{
}

TypeIdentifierPair::~TypeIdentifierPair()
{
}

TypeIdentifierPair::TypeIdentifierPair(const TypeIdentifierPair &x)
{
    m_type_identifier1 = x.m_type_identifier1;
    m_type_identifier2 = x.m_type_identifier2;
}

TypeIdentifierPair::TypeIdentifierPair(TypeIdentifierPair &&x)
{
    m_type_identifier1 = std::move(x.m_type_identifier1);
    m_type_identifier2 = std::move(x.m_type_identifier2);
}

TypeIdentifierPair& TypeIdentifierPair::operator=(const TypeIdentifierPair &x)
{
    m_type_identifier1 = x.m_type_identifier1;
    m_type_identifier2 = x.m_type_identifier2;

    return *this;
}

TypeIdentifierPair& TypeIdentifierPair::operator=(TypeIdentifierPair &&x)
{
    m_type_identifier1 = std::move(x.m_type_identifier1);
    m_type_identifier2 = std::move(x.m_type_identifier2);

    return *this;
}

size_t TypeIdentifierPair::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t TypeIdentifierPair::getCdrSerializedSize(const TypeIdentifierPair& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_identifier1(), current_alignment);
    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_identifier2(), current_alignment);

    return current_alignment - initial_alignment;
}

void TypeIdentifierPair::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_type_identifier1;
    scdr << m_type_identifier2;
}

void TypeIdentifierPair::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_type_identifier1;
    dcdr >> m_type_identifier2;
}

size_t TypeIdentifierPair::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool TypeIdentifierPair::isKeyDefined()
{
    return false;
}

void TypeIdentifierPair::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

TypeIdentfierWithSize::TypeIdentfierWithSize()
{
}

TypeIdentfierWithSize::~TypeIdentfierWithSize()
{
}

TypeIdentfierWithSize::TypeIdentfierWithSize(const TypeIdentfierWithSize &x)
{
    m_type_id = x.m_type_id;
    m_typeobject_serialized_size = x.m_typeobject_serialized_size;
}

TypeIdentfierWithSize::TypeIdentfierWithSize(TypeIdentfierWithSize &&x)
{
    m_type_id = std::move(x.m_type_id);
    m_typeobject_serialized_size = std::move(x.m_typeobject_serialized_size);
}

TypeIdentfierWithSize& TypeIdentfierWithSize::operator=(const TypeIdentfierWithSize &x)
{
    m_type_id = x.m_type_id;
    m_typeobject_serialized_size = x.m_typeobject_serialized_size;

    return *this;
}

TypeIdentfierWithSize& TypeIdentfierWithSize::operator=(TypeIdentfierWithSize &&x)
{
    m_type_id = std::move(x.m_type_id);
    m_typeobject_serialized_size = std::move(x.m_typeobject_serialized_size);

    return *this;
}

size_t TypeIdentfierWithSize::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    return current_alignment - initial_alignment;
}

size_t TypeIdentfierWithSize::getCdrSerializedSize(const TypeIdentfierWithSize& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_id(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    return current_alignment - initial_alignment;
}

void TypeIdentfierWithSize::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_type_id;
    scdr << m_typeobject_serialized_size;
}

void TypeIdentfierWithSize::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_type_id;
    dcdr >> m_typeobject_serialized_size;
}

size_t TypeIdentfierWithSize::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool TypeIdentfierWithSize::isKeyDefined()
{
    return false;
}

void TypeIdentfierWithSize::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}

TypeIdentifierWithDependencies::TypeIdentifierWithDependencies()
{
}

TypeIdentifierWithDependencies::~TypeIdentifierWithDependencies()
{
}

TypeIdentifierWithDependencies::TypeIdentifierWithDependencies(const TypeIdentifierWithDependencies &x)
{
    m_typeid_with_size = x.m_typeid_with_size;
    m_dependent_typeid_count = x.m_dependent_typeid_count;
    m_dependent_typeids = x.m_dependent_typeids;
}

TypeIdentifierWithDependencies::TypeIdentifierWithDependencies(TypeIdentifierWithDependencies &&x)
{
    m_typeid_with_size = std::move(x.m_typeid_with_size);
    m_dependent_typeid_count = std::move(x.m_dependent_typeid_count);
    m_dependent_typeids = std::move(x.m_dependent_typeids);
}

TypeIdentifierWithDependencies& TypeIdentifierWithDependencies::operator=(const TypeIdentifierWithDependencies &x)
{
    m_typeid_with_size = x.m_typeid_with_size;
    m_dependent_typeid_count = x.m_dependent_typeid_count;
    m_dependent_typeids = x.m_dependent_typeids;

    return *this;
}

TypeIdentifierWithDependencies& TypeIdentifierWithDependencies::operator=(TypeIdentifierWithDependencies &&x)
{
    m_typeid_with_size = std::move(x.m_typeid_with_size);
    m_dependent_typeid_count = std::move(x.m_dependent_typeid_count);
    m_dependent_typeids = std::move(x.m_dependent_typeids);

    return *this;
}

size_t TypeIdentifierWithDependencies::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentfierWithSize::getMaxCdrSerializedSize(current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < 100; ++a)
    {
        current_alignment += TypeIdentfierWithSize::getMaxCdrSerializedSize(current_alignment);
    }

    return current_alignment - initial_alignment;
}

size_t TypeIdentifierWithDependencies::getCdrSerializedSize(const TypeIdentifierWithDependencies& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentfierWithSize::getCdrSerializedSize(data.typeid_with_size(), current_alignment);
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    for(size_t a = 0; a < data.dependent_typeids().size(); ++a)
    {
        current_alignment += TypeIdentfierWithSize::getCdrSerializedSize(data.dependent_typeids().at(a), current_alignment);
    }

    return current_alignment - initial_alignment;
}

void TypeIdentifierWithDependencies::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_typeid_with_size;
    scdr << m_dependent_typeid_count;
    scdr << m_dependent_typeids;
}

void TypeIdentifierWithDependencies::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_typeid_with_size;
    dcdr >> m_dependent_typeid_count;
    dcdr >> m_dependent_typeids;
}

size_t TypeIdentifierWithDependencies::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool TypeIdentifierWithDependencies::isKeyDefined()
{
    return false;
}

void TypeIdentifierWithDependencies::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

CompleteTypeObject::CompleteTypeObject()
{
    m__d = 0x00;
}

CompleteTypeObject::~CompleteTypeObject()
{
}

CompleteTypeObject::CompleteTypeObject(const CompleteTypeObject &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
}

CompleteTypeObject::CompleteTypeObject(CompleteTypeObject &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
}

CompleteTypeObject& CompleteTypeObject::operator=(const CompleteTypeObject &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
    return *this;
}

CompleteTypeObject& CompleteTypeObject::operator=(CompleteTypeObject &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
    return *this;
}

void CompleteTypeObject::_d(octet __d)
{
    if(!m__d != __d) throw BadParamException("Discriminator doesn't correspond with the selected union member");
}

octet CompleteTypeObject::_d() const
{
    return m__d;
}

octet& CompleteTypeObject::_d()
{
    return m__d;
}

void CompleteTypeObject::alias_type(CompleteAliasType _alias_type)
{
    m_alias_type = _alias_type;
    m__d = TK_ALIAS;
}

CompleteAliasType CompleteTypeObject::alias_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_alias_type;
}

CompleteAliasType& CompleteTypeObject::alias_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_alias_type;
}

void CompleteTypeObject::annotation_type(CompleteAnnotationType _annotation_type)
{
    m_annotation_type = _annotation_type;
    m__d = TK_ANNOTATION;
}

CompleteAnnotationType CompleteTypeObject::annotation_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ANNOTATION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_annotation_type;
}

CompleteAnnotationType& CompleteTypeObject::annotation_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ANNOTATION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_annotation_type;
}

void CompleteTypeObject::struct_type(CompleteStructType _struct_type)
{
    m_struct_type = _struct_type;
    m__d = TK_STRUCTURE;
}

CompleteStructType CompleteTypeObject::struct_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_STRUCTURE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_struct_type;
}

CompleteStructType& CompleteTypeObject::struct_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_STRUCTURE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_struct_type;
}

void CompleteTypeObject::union_type(CompleteUnionType _union_type)
{
    m_union_type = _union_type;
    m__d = TK_UNION;
}

CompleteUnionType CompleteTypeObject::union_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_UNION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_union_type;
}

CompleteUnionType& CompleteTypeObject::union_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_UNION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_union_type;
}

void CompleteTypeObject::bitset_type(CompleteBitsetType _bitset_type)
{
    m_bitset_type = _bitset_type;
    m__d = TK_BITSET;
}

CompleteBitsetType CompleteTypeObject::bitset_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITSET:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitset_type;
}

CompleteBitsetType& CompleteTypeObject::bitset_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITSET:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitset_type;
}

void CompleteTypeObject::sequence_type(CompleteSequenceType _sequence_type)
{
    m_sequence_type = _sequence_type;
    m__d = TK_SEQUENCE;
}

CompleteSequenceType CompleteTypeObject::sequence_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_SEQUENCE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_sequence_type;
}

CompleteSequenceType& CompleteTypeObject::sequence_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_SEQUENCE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_sequence_type;
}

void CompleteTypeObject::array_type(CompleteArrayType _array_type)
{
    m_array_type = _array_type;
    m__d = TK_ARRAY;
}

CompleteArrayType CompleteTypeObject::array_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ARRAY:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_array_type;
}

CompleteArrayType& CompleteTypeObject::array_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ARRAY:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_array_type;
}

void CompleteTypeObject::map_type(CompleteMapType _map_type)
{
    m_map_type = _map_type;
    m__d = TK_MAP;
}

CompleteMapType CompleteTypeObject::map_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_MAP:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_map_type;
}

CompleteMapType& CompleteTypeObject::map_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_MAP:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_map_type;
}

void CompleteTypeObject::enumerated_type(CompleteEnumeratedType _enumerated_type)
{
    m_enumerated_type = _enumerated_type;
    m__d = TK_ENUM;
}

CompleteEnumeratedType CompleteTypeObject::enumerated_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ENUM:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_enumerated_type;
}

CompleteEnumeratedType& CompleteTypeObject::enumerated_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ENUM:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_enumerated_type;
}

void CompleteTypeObject::bitmask_type(CompleteBitmaskType _bitmask_type)
{
    m_bitmask_type = _bitmask_type;
    m__d = TK_BITMASK;
}

CompleteBitmaskType CompleteTypeObject::bitmask_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITMASK:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitmask_type;
}

CompleteBitmaskType& CompleteTypeObject::bitmask_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITMASK:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitmask_type;
}

void CompleteTypeObject::extended_type(CompleteExtendedType _extended_type)
{
    m_extended_type = _extended_type;
    m__d = 0x00; // Default
}

CompleteExtendedType CompleteTypeObject::extended_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        case TK_ANNOTATION:
        case TK_STRUCTURE:
        case TK_UNION:
        case TK_BITSET:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        case TK_ENUM:
        case TK_BITMASK:
        break;
        default:
        b = true;
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_extended_type;
}

CompleteExtendedType& CompleteTypeObject::extended_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        case TK_ANNOTATION:
        case TK_STRUCTURE:
        case TK_UNION:
        case TK_BITSET:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        case TK_ENUM:
        case TK_BITMASK:
        break;
        default:
        b = true;
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_extended_type;
}

size_t CompleteTypeObject::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    size_t reset_alignment = 0;
    size_t union_max_size_serialized = 0;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


        reset_alignment = current_alignment;

        reset_alignment += CompleteAliasType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;


        reset_alignment = current_alignment;

        reset_alignment += CompleteAnnotationType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteStructType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteUnionType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteBitsetType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteSequenceType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteArrayType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteMapType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteEnumeratedType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteBitmaskType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += CompleteExtendedType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;


    return union_max_size_serialized - initial_alignment;
}

size_t CompleteTypeObject::getCdrSerializedSize(const CompleteTypeObject& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    switch(data.m__d)
    {
        case TK_ALIAS:
        current_alignment += CompleteAliasType::getCdrSerializedSize(data.alias_type(), current_alignment);
        break;
        case TK_ANNOTATION:
        current_alignment += CompleteAnnotationType::getCdrSerializedSize(data.annotation_type(), current_alignment);
        break;
        case TK_STRUCTURE:
        current_alignment += CompleteStructType::getCdrSerializedSize(data.struct_type(), current_alignment);
        break;
        case TK_UNION:
        current_alignment += CompleteUnionType::getCdrSerializedSize(data.union_type(), current_alignment);
        break;
        case TK_BITSET:
        current_alignment += CompleteBitsetType::getCdrSerializedSize(data.bitset_type(), current_alignment);
        break;
        case TK_SEQUENCE:
        current_alignment += CompleteSequenceType::getCdrSerializedSize(data.sequence_type(), current_alignment);
        break;
        case TK_ARRAY:
        current_alignment += CompleteArrayType::getCdrSerializedSize(data.array_type(), current_alignment);
        break;
        case TK_MAP:
        current_alignment += CompleteMapType::getCdrSerializedSize(data.map_type(), current_alignment);
        break;
        case TK_ENUM:
        current_alignment += CompleteEnumeratedType::getCdrSerializedSize(data.enumerated_type(), current_alignment);
        break;
        case TK_BITMASK:
        current_alignment += CompleteBitmaskType::getCdrSerializedSize(data.bitmask_type(), current_alignment);
        break;
        default:
        current_alignment += CompleteExtendedType::getCdrSerializedSize(data.extended_type(), current_alignment);
        break;
    }

    return current_alignment - initial_alignment;
}

void CompleteTypeObject::serialize(eprosima::fastcdr::Cdr &cdr) const
{
    cdr << m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        cdr << m_alias_type;
        break;
        case TK_ANNOTATION:
        cdr << m_annotation_type;
        break;
        case TK_STRUCTURE:
        cdr << m_struct_type;
        break;
        case TK_UNION:
        cdr << m_union_type;
        break;
        case TK_BITSET:
        cdr << m_bitset_type;
        break;
        case TK_SEQUENCE:
        cdr << m_sequence_type;
        break;
        case TK_ARRAY:
        cdr << m_array_type;
        break;
        case TK_MAP:
        cdr << m_map_type;
        break;
        case TK_ENUM:
        cdr << m_enumerated_type;
        break;
        case TK_BITMASK:
        cdr << m_bitmask_type;
        break;
        default:
        cdr << m_extended_type;
        break;
    }
}

void CompleteTypeObject::deserialize(eprosima::fastcdr::Cdr &cdr)
{
    cdr >> m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        cdr >> m_alias_type;
        break;
        case TK_ANNOTATION:
        cdr >> m_annotation_type;
        break;
        case TK_STRUCTURE:
        cdr >> m_struct_type;
        break;
        case TK_UNION:
        cdr >> m_union_type;
        break;
        case TK_BITSET:
        cdr >> m_bitset_type;
        break;
        case TK_SEQUENCE:
        cdr >> m_sequence_type;
        break;
        case TK_ARRAY:
        cdr >> m_array_type;
        break;
        case TK_MAP:
        cdr >> m_map_type;
        break;
        case TK_ENUM:
        cdr >> m_enumerated_type;
        break;
        case TK_BITMASK:
        cdr >> m_bitmask_type;
        break;
        default:
        cdr >> m_extended_type;
        break;
    }
}

size_t CompleteTypeObject::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    return 0;
}

bool CompleteTypeObject::isKeyDefined()
{
    return false;
}

void CompleteTypeObject::serializeKey(eprosima::fastcdr::Cdr &) const
{
}

/****************************************************************************************************************/

MinimalTypeObject::MinimalTypeObject()
{
    m__d = 0x00;
}

MinimalTypeObject::~MinimalTypeObject()
{
}

MinimalTypeObject::MinimalTypeObject(const MinimalTypeObject &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
}

MinimalTypeObject::MinimalTypeObject(MinimalTypeObject &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
}

MinimalTypeObject& MinimalTypeObject::operator=(const MinimalTypeObject &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
    return *this;
}

MinimalTypeObject& MinimalTypeObject::operator=(MinimalTypeObject &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        m_alias_type = x.m_alias_type;
        break;
        case TK_ANNOTATION:
        m_annotation_type = x.m_annotation_type;
        break;
        case TK_STRUCTURE:
        m_struct_type = x.m_struct_type;
        break;
        case TK_UNION:
        m_union_type = x.m_union_type;
        break;
        case TK_BITSET:
        m_bitset_type = x.m_bitset_type;
        break;
        case TK_SEQUENCE:
        m_sequence_type = x.m_sequence_type;
        break;
        case TK_ARRAY:
        m_array_type = x.m_array_type;
        break;
        case TK_MAP:
        m_map_type = x.m_map_type;
        break;
        case TK_ENUM:
        m_enumerated_type = x.m_enumerated_type;
        break;
        case TK_BITMASK:
        m_bitmask_type = x.m_bitmask_type;
        break;
        default:
        m_extended_type = x.m_extended_type;
        break;
    }
    return *this;
}

void MinimalTypeObject::_d(octet __d)
{
    if(!m__d != __d) throw BadParamException("Discriminator doesn't correspond with the selected union member");
}

octet MinimalTypeObject::_d() const
{
    return m__d;
}

octet& MinimalTypeObject::_d()
{
    return m__d;
}

void MinimalTypeObject::alias_type(MinimalAliasType _alias_type)
{
    m_alias_type = _alias_type;
    m__d = TK_ALIAS;
}

MinimalAliasType MinimalTypeObject::alias_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_alias_type;
}

MinimalAliasType& MinimalTypeObject::alias_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_alias_type;
}

void MinimalTypeObject::annotation_type(MinimalAnnotationType _annotation_type)
{
    m_annotation_type = _annotation_type;
    m__d = TK_ANNOTATION;
}

MinimalAnnotationType MinimalTypeObject::annotation_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ANNOTATION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_annotation_type;
}

MinimalAnnotationType& MinimalTypeObject::annotation_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ANNOTATION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_annotation_type;
}

void MinimalTypeObject::struct_type(MinimalStructType _struct_type)
{
    m_struct_type = _struct_type;
    m__d = TK_STRUCTURE;
}

MinimalStructType MinimalTypeObject::struct_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_STRUCTURE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_struct_type;
}

MinimalStructType& MinimalTypeObject::struct_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_STRUCTURE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_struct_type;
}

void MinimalTypeObject::union_type(MinimalUnionType _union_type)
{
    m_union_type = _union_type;
    m__d = TK_UNION;
}

MinimalUnionType MinimalTypeObject::union_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_UNION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_union_type;
}

MinimalUnionType& MinimalTypeObject::union_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_UNION:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_union_type;
}

void MinimalTypeObject::bitset_type(MinimalBitsetType _bitset_type)
{
    m_bitset_type = _bitset_type;
    m__d = TK_BITSET;
}

MinimalBitsetType MinimalTypeObject::bitset_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITSET:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitset_type;
}

MinimalBitsetType& MinimalTypeObject::bitset_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITSET:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitset_type;
}

void MinimalTypeObject::sequence_type(MinimalSequenceType _sequence_type)
{
    m_sequence_type = _sequence_type;
    m__d = TK_SEQUENCE;
}

MinimalSequenceType MinimalTypeObject::sequence_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_SEQUENCE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_sequence_type;
}

MinimalSequenceType& MinimalTypeObject::sequence_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_SEQUENCE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_sequence_type;
}

void MinimalTypeObject::array_type(MinimalArrayType _array_type)
{
    m_array_type = _array_type;
    m__d = TK_ARRAY;
}

MinimalArrayType MinimalTypeObject::array_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ARRAY:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_array_type;
}

MinimalArrayType& MinimalTypeObject::array_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ARRAY:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_array_type;
}

void MinimalTypeObject::map_type(MinimalMapType _map_type)
{
    m_map_type = _map_type;
    m__d = TK_MAP;
}

MinimalMapType MinimalTypeObject::map_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_MAP:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_map_type;
}

MinimalMapType& MinimalTypeObject::map_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_MAP:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_map_type;
}

void MinimalTypeObject::enumerated_type(MinimalEnumeratedType _enumerated_type)
{
    m_enumerated_type = _enumerated_type;
    m__d = TK_ENUM;
}

MinimalEnumeratedType MinimalTypeObject::enumerated_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ENUM:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_enumerated_type;
}

MinimalEnumeratedType& MinimalTypeObject::enumerated_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ENUM:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_enumerated_type;
}

void MinimalTypeObject::bitmask_type(MinimalBitmaskType _bitmask_type)
{
    m_bitmask_type = _bitmask_type;
    m__d = TK_BITMASK;
}

MinimalBitmaskType MinimalTypeObject::bitmask_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITMASK:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitmask_type;
}

MinimalBitmaskType& MinimalTypeObject::bitmask_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_BITMASK:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_bitmask_type;
}

void MinimalTypeObject::extended_type(MinimalExtendedType _extended_type)
{
    m_extended_type = _extended_type;
    m__d = 0x00; // Default
}

MinimalExtendedType MinimalTypeObject::extended_type() const
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        case TK_ANNOTATION:
        case TK_STRUCTURE:
        case TK_UNION:
        case TK_BITSET:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        case TK_ENUM:
        case TK_BITMASK:
        break;
        default:
        b = true;
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_extended_type;
}

MinimalExtendedType& MinimalTypeObject::extended_type()
{
    bool b = false;

    switch(m__d)
    {
        case TK_ALIAS:
        case TK_ANNOTATION:
        case TK_STRUCTURE:
        case TK_UNION:
        case TK_BITSET:
        case TK_SEQUENCE:
        case TK_ARRAY:
        case TK_MAP:
        case TK_ENUM:
        case TK_BITMASK:
        break;
        default:
        b = true;
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_extended_type;
}

size_t MinimalTypeObject::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    size_t reset_alignment = 0;
    size_t union_max_size_serialized = 0;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

        reset_alignment = current_alignment;

        reset_alignment += MinimalAliasType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;


        reset_alignment = current_alignment;

        reset_alignment += MinimalAnnotationType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalStructType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalUnionType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalBitsetType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalSequenceType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalArrayType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalMapType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalEnumeratedType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalBitmaskType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;

        reset_alignment = current_alignment;

        reset_alignment += MinimalExtendedType::getMaxCdrSerializedSize(current_alignment);


        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;


    return union_max_size_serialized - initial_alignment;
}

size_t MinimalTypeObject::getCdrSerializedSize(const MinimalTypeObject& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    switch(data.m__d)
    {
        case TK_ALIAS:
        current_alignment += MinimalAliasType::getCdrSerializedSize(data.alias_type(), current_alignment);
        break;
        case TK_ANNOTATION:
        current_alignment += MinimalAnnotationType::getCdrSerializedSize(data.annotation_type(), current_alignment);
        break;
        case TK_STRUCTURE:
        current_alignment += MinimalStructType::getCdrSerializedSize(data.struct_type(), current_alignment);
        break;
        case TK_UNION:
        current_alignment += MinimalUnionType::getCdrSerializedSize(data.union_type(), current_alignment);
        break;
        case TK_BITSET:
        current_alignment += MinimalBitsetType::getCdrSerializedSize(data.bitset_type(), current_alignment);
        break;
        case TK_SEQUENCE:
        current_alignment += MinimalSequenceType::getCdrSerializedSize(data.sequence_type(), current_alignment);
        break;
        case TK_ARRAY:
        current_alignment += MinimalArrayType::getCdrSerializedSize(data.array_type(), current_alignment);
        break;
        case TK_MAP:
        current_alignment += MinimalMapType::getCdrSerializedSize(data.map_type(), current_alignment);
        break;
        case TK_ENUM:
        current_alignment += MinimalEnumeratedType::getCdrSerializedSize(data.enumerated_type(), current_alignment);
        break;
        case TK_BITMASK:
        current_alignment += MinimalBitmaskType::getCdrSerializedSize(data.bitmask_type(), current_alignment);
        break;
        default:
        current_alignment += MinimalExtendedType::getCdrSerializedSize(data.extended_type(), current_alignment);
        break;
    }

    return current_alignment - initial_alignment;
}

void MinimalTypeObject::serialize(eprosima::fastcdr::Cdr &cdr) const
{
    cdr << m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        cdr << m_alias_type;
        break;
        case TK_ANNOTATION:
        cdr << m_annotation_type;
        break;
        case TK_STRUCTURE:
        cdr << m_struct_type;
        break;
        case TK_UNION:
        cdr << m_union_type;
        break;
        case TK_BITSET:
        cdr << m_bitset_type;
        break;
        case TK_SEQUENCE:
        cdr << m_sequence_type;
        break;
        case TK_ARRAY:
        cdr << m_array_type;
        break;
        case TK_MAP:
        cdr << m_map_type;
        break;
        case TK_ENUM:
        cdr << m_enumerated_type;
        break;
        case TK_BITMASK:
        cdr << m_bitmask_type;
        break;
        default:
        cdr << m_extended_type;
        break;
    }
}

void MinimalTypeObject::deserialize(eprosima::fastcdr::Cdr &cdr)
{
    cdr >> m__d;

    switch(m__d)
    {
        case TK_ALIAS:
        cdr >> m_alias_type;
        break;
        case TK_ANNOTATION:
        cdr >> m_annotation_type;
        break;
        case TK_STRUCTURE:
        cdr >> m_struct_type;
        break;
        case TK_UNION:
        cdr >> m_union_type;
        break;
        case TK_BITSET:
        cdr >> m_bitset_type;
        break;
        case TK_SEQUENCE:
        cdr >> m_sequence_type;
        break;
        case TK_ARRAY:
        cdr >> m_array_type;
        break;
        case TK_MAP:
        cdr >> m_map_type;
        break;
        case TK_ENUM:
        cdr >> m_enumerated_type;
        break;
        case TK_BITMASK:
        cdr >> m_bitmask_type;
        break;
        default:
        cdr >> m_extended_type;
        break;
    }
}

size_t MinimalTypeObject::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    return 0;
}

bool MinimalTypeObject::isKeyDefined()
{
    return false;
}

void MinimalTypeObject::serializeKey(eprosima::fastcdr::Cdr &) const
{
}

TypeObject::TypeObject()
{
    m__d = EK_COMPLETE;


}

TypeObject::~TypeObject()
{
}

TypeObject::TypeObject(const TypeObject &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case EK_COMPLETE:
        m_complete = x.m_complete;
        break;
        case EK_MINIMAL:
        m_minimal = x.m_minimal;
        break;
        default:
        break;
    }
}

TypeObject::TypeObject(TypeObject &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case EK_COMPLETE:
        m_complete = std::move(x.m_complete);
        break;
        case EK_MINIMAL:
        m_minimal = std::move(x.m_minimal);
        break;
        default:
        break;
    }
}

TypeObject& TypeObject::operator=(const TypeObject &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case EK_COMPLETE:
        m_complete = x.m_complete;
        break;
        case EK_MINIMAL:
        m_minimal = x.m_minimal;
        break;
        default:
        break;
    }

    return *this;
}

TypeObject& TypeObject::operator=(TypeObject &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
        case EK_COMPLETE:
        m_complete = std::move(x.m_complete);
        break;
        case EK_MINIMAL:
        m_minimal = std::move(x.m_minimal);
        break;
        default:
        break;
    }

    return *this;
}

void TypeObject::_d(uint8_t __d)
{
    bool b = false;

    switch(m__d)
    {
        case EK_COMPLETE:
        switch(__d)
        {
            case EK_COMPLETE:
            b = true;
            break;
            default:
            break;
        }
        break;
        case EK_MINIMAL:
        switch(__d)
        {
            case EK_MINIMAL:
            b = true;
            break;
            default:
            break;
        }
        break;
    }

    if(!b) throw BadParamException("Discriminator doesn't correspond with the selected union member");

    m__d = __d;
}

uint8_t TypeObject::_d() const
{
    return m__d;
}

uint8_t& TypeObject::_d()
{
    return m__d;
}

void TypeObject::complete(const CompleteTypeObject &_complete)
{
    m_complete = _complete;
    m__d = EK_COMPLETE;
}

void TypeObject::complete(CompleteTypeObject &&_complete)
{
    m_complete = std::move(_complete);
    m__d = EK_COMPLETE;
}

const CompleteTypeObject& TypeObject::complete() const
{
    bool b = false;

    switch(m__d)
    {
        case EK_COMPLETE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_complete;
}

CompleteTypeObject& TypeObject::complete()
{
    bool b = false;

    switch(m__d)
    {
        case EK_COMPLETE:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_complete;
}
void TypeObject::minimal(const MinimalTypeObject &_minimal)
{
    m_minimal = _minimal;
    m__d = EK_MINIMAL;
}

void TypeObject::minimal(MinimalTypeObject &&_minimal)
{
    m_minimal = std::move(_minimal);
    m__d = EK_MINIMAL;
}

const MinimalTypeObject& TypeObject::minimal() const
{
    bool b = false;

    switch(m__d)
    {
        case EK_MINIMAL:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_minimal;
}

MinimalTypeObject& TypeObject::minimal()
{
    bool b = false;

    switch(m__d)
    {
        case EK_MINIMAL:
        b = true;
        break;
        default:
        break;
    }
    if(!b) throw BadParamException("This member hasn't been selected");

    return m_minimal;
}

size_t TypeObject::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
    size_t reset_alignment = 0;
    size_t union_max_size_serialized = 0;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


        reset_alignment = current_alignment;

        reset_alignment += CompleteTypeObject::getMaxCdrSerializedSize(reset_alignment);

        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;


        reset_alignment = current_alignment;

        reset_alignment += MinimalTypeObject::getMaxCdrSerializedSize(reset_alignment);

        if(union_max_size_serialized < reset_alignment)
            union_max_size_serialized = reset_alignment;



    return union_max_size_serialized - initial_alignment;
}

// TODO(Ricardo) Review
size_t TypeObject::getCdrSerializedSize(const TypeObject& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    switch(data.m__d)
    {
        case EK_COMPLETE:
        current_alignment += CompleteTypeObject::getCdrSerializedSize(data.complete(), current_alignment);
        break;
        case EK_MINIMAL:
        current_alignment += MinimalTypeObject::getCdrSerializedSize(data.minimal(), current_alignment);
        break;
        default:
        break;
    }

    return current_alignment - initial_alignment;
}

void TypeObject::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m__d;

    switch(m__d)
    {
        case EK_COMPLETE:
        scdr << m_complete;
        break;
        case EK_MINIMAL:
        scdr << m_minimal;
        break;
        default:
        break;
    }
}

void TypeObject::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m__d;

    switch(m__d)
    {
        case EK_COMPLETE:
        dcdr >> m_complete;
        break;
        case EK_MINIMAL:
        dcdr >> m_minimal;
        break;
        default:
        break;
    }
}

TypeInformation::TypeInformation()
{
}

TypeInformation::~TypeInformation()
{
}

TypeInformation::TypeInformation(const TypeInformation &x)
{
    m_minimal = x.m_minimal;
    m_complete = x.m_complete;
}

TypeInformation::TypeInformation(TypeInformation &&x)
{
    m_minimal = std::move(x.m_minimal);
    m_complete = std::move(x.m_complete);
}

TypeInformation& TypeInformation::operator=(const TypeInformation &x)
{
    m_minimal = x.m_minimal;
    m_complete = x.m_complete;

    return *this;
}

TypeInformation& TypeInformation::operator=(TypeInformation &&x)
{
    m_minimal = std::move(x.m_minimal);
    m_complete = std::move(x.m_complete);

    return *this;
}

size_t TypeInformation::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifierWithDependencies::getMaxCdrSerializedSize(current_alignment);
    current_alignment += TypeIdentifierWithDependencies::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t TypeInformation::getCdrSerializedSize(const TypeInformation& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += TypeIdentifierWithDependencies::getCdrSerializedSize(data.minimal(), current_alignment);
    current_alignment += TypeIdentifierWithDependencies::getCdrSerializedSize(data.complete(), current_alignment);

    return current_alignment - initial_alignment;
}

void TypeInformation::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_minimal;
    scdr << m_complete;
}

void TypeInformation::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_minimal;
    dcdr >> m_complete;
}

size_t TypeInformation::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;

    return current_align;
}

bool TypeInformation::isKeyDefined()
{
    return false;
}

void TypeInformation::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}




} // namespace types
} // namespace fastrtps
} // namespace eprosima
