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

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/log/Log.hpp>

#include <fastrtps/types/v1_3/DynamicType.hpp>
#include <dynamic-types/v1_3/DynamicTypeImpl.hpp>

using namespace eprosima::fastrtps::types::v1_3;
using eprosima::fastrtps::types::ReturnCode_t;

bool DynamicType::operator ==(
        const DynamicType& other) const noexcept
{
    return DynamicTypeImpl::get_implementation(*this)
           == DynamicTypeImpl::get_implementation(other);
}

bool DynamicType::operator !=(
        const DynamicType& other) const noexcept
{
    return DynamicTypeImpl::get_implementation(*this)
           != DynamicTypeImpl::get_implementation(other);
}

ReturnCode_t DynamicType::get_descriptor(TypeDescriptor& td) const noexcept
{
    td = DynamicTypeImpl::get_implementation(*this).get_descriptor();
    return ReturnCode_t::RETCODE_OK;
}

const char* DynamicType::get_name() const noexcept
{
    return DynamicTypeImpl::get_implementation(*this).get_name().c_str();
}

TypeKind DynamicType::get_kind() const noexcept
{
    return DynamicTypeImpl::get_implementation(*this).get_kind();
}

const DynamicTypeMember* DynamicType::get_member_by_name(
        const char* name,
        ReturnCode_t* ec /*= nullptr*/) const noexcept
{
    return DynamicTypeImpl::get_implementation(*this).get_member_by_name(name, ec);
}

//const DynamicTypeMembersByName* DynamicType::get_all_members_by_name(
//        ReturnCode_t* ec /*= nullptr*/) const noexcept
//{
//
//}
