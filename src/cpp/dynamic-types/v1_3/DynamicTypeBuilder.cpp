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

#include <fastrtps/types/v1_3/DynamicTypeBuilder.hpp>
#include <dynamic-types/v1_3/DynamicTypeImpl.hpp>
#include <dynamic-types/v1_3/DynamicTypeBuilderImpl.hpp>

using namespace eprosima::fastrtps::types::v1_3;
using eprosima::fastrtps::types::ReturnCode_t;

bool DynamicTypeBuilder::operator ==(
        const DynamicTypeBuilder& other) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this) == DynamicTypeBuilderImpl::get_implementation(other);
}

bool DynamicTypeBuilder::operator !=(
        const DynamicTypeBuilder& other) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this) != DynamicTypeBuilderImpl::get_implementation(other);
}

ReturnCode_t DynamicTypeBuilder::get_descriptor(TypeDescriptor& td) const noexcept
{
    td = DynamicTypeBuilderImpl::get_implementation(*this).get_descriptor();
    return ReturnCode_t::RETCODE_OK;
}

const char* DynamicTypeBuilder::get_name() const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_name().c_str();
}

TypeKind DynamicTypeBuilder::get_kind() const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_kind();
}

const DynamicTypeMember* DynamicTypeBuilder::get_member_by_name(
        const char* name,
        ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_member_by_name(name, ec);
}

DynamicTypeMembersByName DynamicTypeBuilder::get_all_members_by_name(
        ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_all_members_by_name(ec);
}

const DynamicTypeMember* DynamicTypeBuilder::get_member(
        MemberId id,
        ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    try
    {
        if (ec)
        {
            *ec = ReturnCode_t{};
        }

        return &DynamicTypeBuilderImpl::get_implementation(*this).get_member(id).get_interface();
    }
    catch(std::system_error& e)
    {
        if (ec)
        {
            *ec = static_cast<uint32_t>(e.code().value());
        }

        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());

        return nullptr;
    }
}

ReturnCode_t DynamicTypeBuilder::add_member(const MemberDescriptor& md) noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).add_member(md);
}

DynamicTypeMembersById DynamicTypeBuilder::get_all_members(
        ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_all_members_by_id(ec);
}

uint32_t DynamicTypeBuilder::get_member_count() const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_member_count();
}

const DynamicTypeMember* DynamicTypeBuilder::get_member_by_index(
        uint32_t index,
        ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    try
    {
        if (ec)
        {
            *ec = ReturnCode_t{};
        }

        return &DynamicTypeBuilderImpl::get_implementation(*this).get_member_by_index(index).get_interface();
    }
    catch(std::system_error& e)
    {
        if (ec)
        {
            *ec = static_cast<uint32_t>(e.code().value());
        }

        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());

        return nullptr;
    }
}

uint32_t DynamicTypeBuilder::get_annotation_count() const noexcept
{
    return static_cast<uint32_t>(DynamicTypeBuilderImpl::get_implementation(*this).get_annotation_count());
}

ReturnCode_t DynamicTypeBuilder::get_annotation(
        AnnotationDescriptor& annotation,
        uint32_t index) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).get_annotation(annotation, index);
}

ReturnCode_t DynamicTypeBuilder::apply_annotation(const AnnotationDescriptor& annotation) noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this).apply_annotation(annotation);
}

ReturnCode_t DynamicTypeBuilder::apply_annotation_to_member(
        MemberId id,
        const AnnotationDescriptor& annotation) noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this)
               .apply_annotation_to_member(id, annotation);
}

const Annotations* DynamicTypeBuilder::get_all_annotations(
    ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    if (ec)
    {
        *ec = ReturnCode_t{};
    }

    return &DynamicTypeBuilderImpl::get_implementation(*this).get_annotations();
}

const DynamicType* DynamicTypeBuilder::build() const noexcept
{
    auto type = DynamicTypeBuilderImpl::get_implementation(*this).build();

    return type ? &type->get_interface() : nullptr;
}

bool DynamicTypeBuilder::equals(
        const DynamicType& other) const noexcept
{
    return DynamicTypeBuilderImpl::get_implementation(*this)
           == DynamicTypeImpl::get_implementation(other);
}
