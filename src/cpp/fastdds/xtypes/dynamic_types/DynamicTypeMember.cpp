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

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include "DynamicTypeMemberImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

bool DynamicTypeMember::operator ==(
        const DynamicTypeMember& descriptor) const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this)
           == DynamicTypeMemberImpl::get_implementation(descriptor);
}

bool DynamicTypeMember::operator !=(
        const DynamicTypeMember& descriptor) const noexcept
{
    /*TODO(richiware)
       return DynamicTypeMemberImpl::get_implementation(*this)
           != DynamicTypeMemberImpl::get_implementation(descriptor);
     */
    return false;
}

ReturnCode_t DynamicTypeMember::get_descriptor(
        MemberDescriptor& md) const noexcept
{
    //TODO(richiware) md = DynamicTypeMemberImpl::get_implementation(*this).get_descriptor();
    return RETCODE_OK;
}

bool DynamicTypeMember::equals(
        const DynamicTypeMember& descriptor) const noexcept
{
    return *this == descriptor;
}

const char* DynamicTypeMember::get_name() const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this).name().c_str();
}

MemberId DynamicTypeMember::get_id() const noexcept
{
    return DynamicTypeMemberImpl::get_implementation(*this).id();
}

const Annotations* DynamicTypeMember::get_annotation() const noexcept
{
    return &DynamicTypeMemberImpl::get_implementation(*this).get_annotations();
}

const DynamicTypeMember* DynamicTypeMembersByName::operator [](
        const char* key) const noexcept
{
    auto it = map_->find(key);

    if (it != map_->end())
    {
        return &it->second->get_interface();
    }

    return nullptr;
}

uint64_t DynamicTypeMembersByName::size() const noexcept
{
    return map_->size();
}

bool DynamicTypeMembersByName::empty() const noexcept
{
    return map_->empty();
}

const char* DynamicTypeMembersByName::next_key(
        const char* key)
{
    if (nullptr == key)
    {
        return map_->begin()->first.c_str();
    }
    else
    {
        auto it = map_->find(key);

        if (it++ != map_->end() && it != map_->end())
        {
            return it->first.c_str();
        }
    }

    return nullptr;
}

const DynamicTypeMember* DynamicTypeMembersById::operator [](
        MemberId key) const noexcept
{
    auto it = map_->find(key);

    if (it != map_->end())
    {
        return &it->second->get_interface();
    }

    return nullptr;
}

uint64_t DynamicTypeMembersById::size() const noexcept
{
    return map_->size();
}

bool DynamicTypeMembersById::empty() const noexcept
{
    return map_->empty();
}

MemberId DynamicTypeMembersById::next_key(
        MemberId key)
{
    if (MEMBER_ID_INVALID == key)
    {
        return map_->begin()->first;
    }
    else
    {
        auto it = map_->find(key);

        if (it++ != map_->end() && it != map_->end())
        {
            return it->first;
        }
    }

    return MEMBER_ID_INVALID;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
