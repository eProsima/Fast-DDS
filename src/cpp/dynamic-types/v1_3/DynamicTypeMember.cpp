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

#include <fastrtps/types/v1_3/DynamicTypeMember.hpp>
#include <dynamic-types/v1_3/DynamicTypeMemberImpl.hpp>

using namespace eprosima::fastrtps::types::v1_3;
using eprosima::fastrtps::types::ReturnCode_t;

bool DynamicTypeMember::operator ==(
        const DynamicTypeMember& descriptor) const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this)
           == DynamicTypeMemberImpl::get_implementation(descriptor);
}

bool DynamicTypeMember::operator !=(
        const DynamicTypeMember& descriptor) const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this)
           != DynamicTypeMemberImpl::get_implementation(descriptor);
}

ReturnCode_t DynamicTypeMember::get_descriptor(MemberDescriptor & md) const noexcept
{
    md = DynamicTypeMemberImpl::get_implementation(*this).get_descriptor();
    return ReturnCode_t::RETCODE_OK;
}

bool DynamicTypeMember::equals(
        const DynamicTypeMember& descriptor) const noexcept
{
    return *this == descriptor;
}

const char* DynamicTypeMember::get_name() const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this).get_name().c_str();
}

MemberId DynamicTypeMember::get_id() const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this).get_id();
}

const Annotations* DynamicTypeMember::get_annotation() const noexcept
{
    return &DynamicTypeMemberImpl::get_implementation(*this).get_annotations();
}
