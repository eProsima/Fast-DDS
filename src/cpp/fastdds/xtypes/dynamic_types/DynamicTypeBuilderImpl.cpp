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

#include "DynamicTypeBuilderImpl.hpp"

#include <cassert>

#include "DynamicTypeImpl.hpp"
#include "DynamicTypeMemberImpl.hpp"
#include "MemberDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds  {

DynamicTypeBuilderImpl::DynamicTypeBuilderImpl(
        const TypeDescriptorImpl& type_descriptor) noexcept
{
    type_descriptor_.copy_from(type_descriptor);
}

ReturnCode_t DynamicTypeBuilderImpl::get_descriptor(
        traits<TypeDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    traits<TypeDescriptor>::narrow<TypeDescriptorImpl>(descriptor)->copy_from(type_descriptor_);
    return RETCODE_OK;
}

ObjectName DynamicTypeBuilderImpl::get_name() noexcept
{
    return type_descriptor_.name();
}

TypeKind DynamicTypeBuilderImpl::get_kind() noexcept
{
    return type_descriptor_.kind();
}

ReturnCode_t DynamicTypeBuilderImpl::get_member_by_name(
        traits<DynamicTypeMember>::ref_type member,
        const ObjectName& name) noexcept
{
    auto it = member_by_name_.find(name);

    if (member_by_name_.end() == it)
    {
        return RETCODE_BAD_PARAMETER;
    }

    member = it->second;
    return RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderImpl::get_all_members_by_name(
        DynamicTypeMembersByName& member) noexcept
{
    member = member_by_name_;
    return RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderImpl::get_member(
        traits<DynamicTypeMember>::ref_type member,
        MemberId id) noexcept
{
    auto it = member_.find(id);

    if (member_.end() == it)
    {
        return RETCODE_BAD_PARAMETER;
    }

    member = it->second;
    return RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderImpl::get_all_members(
        DynamicTypeMembersById& member) noexcept
{
    member = member_;
    return RETCODE_OK;
}

uint32_t DynamicTypeBuilderImpl::get_member_count() noexcept
{
    return members_.size();
}

ReturnCode_t DynamicTypeBuilderImpl::get_member_by_index(
        traits<DynamicTypeMember>::ref_type member,
        uint32_t index) noexcept
{
    if (index >= members_.size())
    {
        return RETCODE_BAD_PARAMETER;
    }

    member = members_.at(index);
    return RETCODE_OK;
}

uint32_t DynamicTypeBuilderImpl::get_annotation_count() noexcept
{
    return static_cast<uint32_t>(annotation_.size());
}

ReturnCode_t DynamicTypeBuilderImpl::get_annotation(
        traits<AnnotationDescriptor>::ref_type descriptor,
        uint32_t idx) noexcept
{
    if (!descriptor || idx >= annotation_.size())
    {
        return RETCODE_BAD_PARAMETER;
    }

    traits<AnnotationDescriptor>::narrow<AnnotationDescriptorImpl>(descriptor)->copy_from(annotation_.at(idx));
    return RETCODE_OK;
}

bool DynamicTypeBuilderImpl::equals(
        traits<DynamicType>::ref_type other) noexcept
{
    bool ret_value = true;
    auto impl = traits<DynamicType>::narrow<DynamicTypeImpl>(other);

    if (ret_value &= type_descriptor_.equals(impl->type_descriptor_))
    {
        if (annotation_.size() == impl->annotation_.size())
        {
            for (size_t count {0}; ret_value && count < annotation_.size(); ++count)
            {
                ret_value &= annotation_.at(count).equals(impl->annotation_.at(count));
            }
        }

        ret_value &= member_.size() == impl->member_.size();
        assert(TK_STRUCTURE == type_descriptor_.kind() ||
                TK_UNION ==  type_descriptor_.kind() ||
                0 == member_.size());
        assert(TK_STRUCTURE == impl->type_descriptor_.kind() ||
                TK_UNION ==  impl->type_descriptor_.kind() ||
                0 == impl->member_.size());
        assert((TK_STRUCTURE != type_descriptor_.kind() &&
                TK_UNION &&  type_descriptor_.kind()) ||
                0 < member_.size());
        assert((TK_STRUCTURE != impl->type_descriptor_.kind() &&
                TK_UNION &&  impl->type_descriptor_.kind()) ||
                0 < member_.size());

        assert(member_by_name_.size() == members_.size());
        assert(impl->member_by_name_.size() == impl->members_.size());
        if (member_by_name_.size() == impl->member_by_name_.size())
        {
            auto it = member_by_name_.begin();
            auto impl_it = impl->member_by_name_.begin();

            while (ret_value && member_by_name_.end() != it)
            {
                ret_value &= it->second->equals(impl_it->second);
                ++it;
                ++impl_it;
            }
        }
    }

    return ret_value;
}

ReturnCode_t DynamicTypeBuilderImpl::add_member(
        traits<MemberDescriptor>::ref_type descriptor) noexcept
{
    auto type_descriptor_kind = type_descriptor_.kind();

    if (TK_ANNOTATION != type_descriptor_kind &&
            TK_BITMASK != type_descriptor_kind &&
            TK_BITSET != type_descriptor_kind &&
            TK_ENUM != type_descriptor_kind &&
            TK_STRUCTURE != type_descriptor_kind &&
            TK_UNION != type_descriptor_kind)
    {
        return RETCODE_BAD_PARAMETER;
    }

    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    auto descriptor_impl = traits<MemberDescriptor>::narrow<MemberDescriptorImpl>(descriptor);

    // Check that there isn't any member as default label and that there isn't any member with the same case.
    if (TK_UNION == type_descriptor_kind && descriptor_impl->is_default_label())
    {
        for (auto member : members_)
        {
            auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);
            if (member_impl->member_descriptor_.is_default_label())
            {
                return RETCODE_BAD_PARAMETER;
            }
        }
    }

    const auto& member_name = descriptor_impl->name();

    // Bitsets allow multiple empty members.
    /* TODO(richiware) have we support empty bitfield to increase the offset?
       if (TK_BITSET != type_descriptor_kind && 0 == descriptor_impl->name().size())
       {
        return RETCODE_BAD_PARAMETER;
       }
     */

    // Check there is already a member with same name.
    auto it_by_name = member_by_name_.find(member_name);
    if (member_by_name_.end() != it_by_name)
    {
        return RETCODE_BAD_PARAMETER;
    }

    descriptor_impl->parent_kind(type_descriptor_kind); // Set before calling is_consistent().

    descriptor_impl->index(next_index_++);
    //TODO(richiware) if memberid is invalid in aggregated types, find a new one.
    //TODO(richiware) set index

    if (!descriptor_impl->is_consistent())
    {
        return RETCODE_BAD_PARAMETER;
    }

    // Check there is already a member with same id.
    auto member_id = descriptor_impl->id();
    if (TK_STRUCTURE == type_descriptor_kind || TK_UNION == type_descriptor_kind)
    {
        auto it_by_id = member_.find(member_id);
        if (member_.end() != it_by_id)
        {
            return RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        assert(MEMBER_ID_INVALID == member_id);
    }

    /*TODO(richiware) think
       if (get_kind() == TK_BITMASK &&
            descriptor.id() >= get_bounds(0))
       {
        // TODO(richiware) throw std::system_error(
        // TODO(richiware)           RETCODE_BAD_PARAMETER,
        // TODO(richiware)           "Error adding member, out of bounds.");
       }
     */

    traits<DynamicTypeMember>::ref_type dyn_member = std::make_shared<DynamicTypeMemberImpl>(*descriptor_impl);

    members_.push_back(dyn_member); //TODO(richiware) review if index set by user.
    member_by_name_.emplace(std::make_pair(member_name, dyn_member));
    if (TK_STRUCTURE == type_descriptor_kind || TK_UNION == type_descriptor_kind)
    {
        member_.emplace(std::make_pair(member_id, dyn_member));
    }

    return RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderImpl::apply_annotation(
        traits<AnnotationDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor || !descriptor->is_consistent())
    {
        return RETCODE_BAD_PARAMETER;
    }
    auto descriptor_impl = traits<AnnotationDescriptor>::narrow<AnnotationDescriptorImpl>(descriptor);

    annotation_.emplace_back();
    annotation_.back().copy_from(*descriptor_impl);

    return RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilderImpl::apply_annotation_to_member(
        MemberId member_id,
        traits<AnnotationDescriptor>::ref_type descriptor) noexcept
{
    auto type_descriptor_kind = type_descriptor_.kind();

    if (!descriptor || !descriptor->is_consistent() ||
            (TK_STRUCTURE != type_descriptor_kind && TK_UNION != type_descriptor_kind))
    {
        return RETCODE_BAD_PARAMETER;
    }

    auto it = member_.find(member_id);

    if (member_.end() == it)
    {
        return RETCODE_BAD_PARAMETER;
    }

    auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(it->second);
    auto descriptor_impl = traits<AnnotationDescriptor>::narrow<AnnotationDescriptorImpl>(descriptor);

    member_impl->annotation_.emplace_back();
    member_impl->annotation_.back().copy_from(descriptor_impl);

    return RETCODE_OK;
}

traits<DynamicType>::ref_type DynamicTypeBuilderImpl::build() noexcept
{
    traits<DynamicTypeImpl>::ref_type ret_val;

    if (type_descriptor_.is_consistent())
    {
        ret_val = std::make_shared<DynamicTypeImpl>(type_descriptor_);
        for (auto& annotation : annotation_)
        {
            ret_val->annotation_.emplace_back();
            ret_val->annotation_.back().copy_from(annotation);
        }
        ret_val->member_ = member_;
        ret_val->member_by_name_ = member_by_name_;
        ret_val->members_ = members_;
    }

    return ret_val;
}

} // namespace dds

} // namespace fastdds
} // namespace eprosima
