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
#include <string>
#include <utility>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "AnnotationDescriptorImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include "DynamicTypeMemberImpl.hpp"
#include "MemberDescriptorImpl.hpp"
#include "TypeDescriptorImpl.hpp"
#include "TypeValueConverter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds  {

DynamicTypeBuilderImpl::DynamicTypeBuilderImpl(
        const TypeDescriptorImpl& type_descriptor) noexcept
{
    type_descriptor_.copy_from(type_descriptor);

    if ((TK_STRUCTURE == type_descriptor_.kind() ||
            TK_BITSET == type_descriptor_.kind()) &&
            type_descriptor_.base_type())
    {
        // Get the members of the base type.
        auto base_type =
                traits<DynamicType>::narrow<DynamicTypeImpl>(type_descriptor_.base_type())->resolve_alias_enclosed_type();
        member_ = base_type->member_;
        member_by_name_ = base_type->member_by_name_;
        members_ = base_type->members_;

        // In case TK_BITSET, get the base type bounds.
        type_descriptor_.bound().insert(type_descriptor_.bound().begin(),
                base_type->get_descriptor().bound().begin(),
                base_type->get_descriptor().bound().end());

        // Get last member_id from the base type.
        if (0 < members_.size())
        {
            traits<DynamicTypeMemberImpl>::ref_type member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(
                                                                     members_.back())};
            assert(MEMBER_ID_INVALID != member_impl->get_descriptor().id());
            next_id_ = member_impl->get_descriptor().id() + 1;
        }

        next_index_ = static_cast<uint32_t>(members_.size());
        index_own_members_ = static_cast<uint32_t>(members_.size());
    }
    else if (TK_UNION == type_descriptor_.kind())
    {
        MemberDescriptorImpl discriminator_descriptor;
        discriminator_descriptor.id(0);
        discriminator_descriptor.index(0);
        discriminator_descriptor.name("discriminator");
        discriminator_descriptor.type(type_descriptor.discriminator_type());

        traits<DynamicTypeMemberImpl>::ref_type dyn_member = std::make_shared<DynamicTypeMemberImpl>(
            discriminator_descriptor);
        members_.push_back(dyn_member);
        member_by_name_.emplace(std::make_pair(discriminator_descriptor.name(), dyn_member));
        member_.emplace(std::make_pair(discriminator_descriptor.id(), dyn_member));
        next_index_ = 1;
        next_id_ = 1;
    }
}

ReturnCode_t DynamicTypeBuilderImpl::get_descriptor(
        traits<TypeDescriptor>::ref_type& descriptor) noexcept
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
        traits<DynamicTypeMember>::ref_type& member,
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
        traits<DynamicTypeMember>::ref_type& member,
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
    return static_cast<uint32_t>(members_.size());
}

ReturnCode_t DynamicTypeBuilderImpl::get_member_by_index(
        traits<DynamicTypeMember>::ref_type& member,
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
        traits<AnnotationDescriptor>::ref_type& descriptor,
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

    ret_value &= type_descriptor_.equals(impl->type_descriptor_);
    if (ret_value)
    {
        ret_value &= annotation_.size() == impl->annotation_.size();
        if (ret_value)
        {
            for (size_t count {0}; ret_value && count < annotation_.size(); ++count)
            {
                ret_value &= annotation_.at(count).equals(impl->annotation_.at(count));
            }
        }

        ret_value &= verbatim_.size() == impl->verbatim_.size();
        if (ret_value)
        {
            for (size_t count {0}; ret_value && count < verbatim_.size(); ++count)
            {
                ret_value &= verbatim_.at(count).equals(impl->verbatim_.at(count));
            }
        }

        ret_value &= member_.size() == impl->member_.size();
        assert(TK_ANNOTATION == type_descriptor_.kind() ||
                TK_BITMASK == type_descriptor_.kind() ||
                TK_BITSET == type_descriptor_.kind() ||
                TK_STRUCTURE == type_descriptor_.kind() ||
                TK_UNION ==  type_descriptor_.kind() ||
                0 == member_.size());
        assert(TK_ANNOTATION == impl->type_descriptor_.kind() ||
                TK_BITMASK == impl->type_descriptor_.kind() ||
                TK_BITSET == impl->type_descriptor_.kind() ||
                TK_STRUCTURE == impl->type_descriptor_.kind() ||
                TK_UNION ==  impl->type_descriptor_.kind() ||
                0 == impl->member_.size());

        assert(member_by_name_.size() == members_.size());
        assert(impl->member_by_name_.size() == impl->members_.size());
        ret_value &= member_by_name_.size() == impl->member_by_name_.size();
        if (ret_value)
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

//{{{ Auxiliary structure to revert default_value setting.
template<typename T>
struct RollbackSetting
{
    RollbackSetting(
            T& value_reference)
        : value_reference_{value_reference}
        , previous_value_{value_reference}
    {
    }

    ~RollbackSetting()
    {
        if (activate)
        {
            value_reference_ = previous_value_;
        }
    }

    bool activate {false};
    T& value_reference_;
    T previous_value_;
};
//}}}

ReturnCode_t DynamicTypeBuilderImpl::add_member(
        traits<MemberDescriptor>::ref_type descriptor) noexcept
{
    auto type_descriptor_kind = type_descriptor_.kind();

    RollbackSetting<uint32_t> id_reverter{next_id_}, index_reverter{next_index_};
    RollbackSetting<int32_t> default_value_reverter{default_value_};

    if (TK_ANNOTATION != type_descriptor_kind &&
            TK_BITMASK != type_descriptor_kind &&
            TK_BITSET != type_descriptor_kind &&
            TK_ENUM != type_descriptor_kind &&
            TK_STRUCTURE != type_descriptor_kind &&
            TK_UNION != type_descriptor_kind)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type of kind " << type_descriptor_kind << " does not support adding members");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    if (!descriptor)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor reference is nil");
        return RETCODE_BAD_PARAMETER;
    }

    auto descriptor_impl = traits<MemberDescriptor>::narrow<MemberDescriptorImpl>(descriptor);


    //{{{ Check on BITMASK doesn't exceed bound.
    assert(TK_BITMASK != type_descriptor_kind || 1 == type_descriptor_.bound().size());
    if (TK_BITMASK == type_descriptor_kind && members_.size() >= type_descriptor_.bound().at(0))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Adding new member in this BITMASK exceeds the bound.");
        return RETCODE_BAD_PARAMETER;
    }
    //}}}

    //{{{ Check on BITSET new member doesn't exceed the bound vector's size.
    if (TK_BITSET == type_descriptor_kind && members_.size() == type_descriptor_.bound().size())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Adding new member in this BITSET exceeds the size of bounds.");
        return RETCODE_BAD_PARAMETER;
    }
    //}}}

    const auto& member_name = descriptor_impl->name();

    //{{{ Check there is already a member with same name.
    auto it_by_name = member_by_name_.find(member_name);
    if (member_by_name_.end() != it_by_name)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "There is already a member with name " << member_name);
        return RETCODE_BAD_PARAMETER;
    }
    //}}}

    auto member_id = descriptor_impl->id();

    //{{{ If member_id is MEMBER_ID_INVALID and type is aggregated, find a new one.
    if (TK_ANNOTATION == type_descriptor_kind ||
            TK_BITMASK == type_descriptor_kind ||
            TK_STRUCTURE == type_descriptor_kind ||
            TK_UNION == type_descriptor_kind)
    {
        if (MEMBER_ID_INVALID == member_id)
        {
            member_id = next_id_++;
            id_reverter.activate = true;

        }
        else if (member_id >= next_id_)
        {
            next_id_ = member_id + 1;
            id_reverter.activate = true;
        }

        // Check there is already a member with same id.
        auto it_by_id = member_.find(member_id);
        if (member_.end() != it_by_id)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "There is already a member with MemberId " << member_id);
            return RETCODE_BAD_PARAMETER;
        }
    }
    //}}}
    //{{{ Else if it is mandatory to a BITSET's member to have MemberId.
    else if (TK_BITSET == type_descriptor_kind)
    {
        if (MEMBER_ID_INVALID == member_id)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "MemberId for BITSET must be different than MEMBER_ID_INVALID");
            return RETCODE_BAD_PARAMETER;
        }
    }
    //}}}
    //{{{ Else, TK_ENUM has to come with MEMBER_ID_INVALID.
    else if (MEMBER_ID_INVALID != member_id)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "MemberId must be MEMBER_ID_INVALID");
        return RETCODE_BAD_PARAMETER;
    }
    //}}}

    traits<DynamicTypeMemberImpl>::ref_type dyn_member = std::make_shared<DynamicTypeMemberImpl>(*descriptor_impl);
    dyn_member->get_descriptor().id(member_id);
    //{{{ Set index
    dyn_member->get_descriptor().index(next_index_++);
    index_reverter.activate = true;
    //}}}

    //{{{ Check bound in case of BITMASK
    if (TK_BITMASK == type_descriptor_kind && member_id >= type_descriptor_.bound().at(0))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The BITMASK member id exceeds the bound.");
        return RETCODE_BAD_PARAMETER;
    }
    //}}}

    //{{{ Specific checks for STRUCTURE
    if (TK_STRUCTURE == type_descriptor_kind)
    {
        if (descriptor_impl->is_key() && type_descriptor_.base_type())
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "A derived structure cannot contains keyed members");
            return RETCODE_BAD_PARAMETER;
        }
    }
    //}}}
    //{{{ Specific checks for UNION
    else if (TK_UNION == type_descriptor_kind)
    {
        for (auto member : members_)
        {
            const auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member)};

            // Check default label and label cases uniqueness.
            if (descriptor_impl->is_default_label() && member_impl->member_descriptor_.is_default_label())
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES,
                        "Member " << member_impl->member_descriptor_.name().c_str() <<
                        " already defined as default_label");
                return RETCODE_BAD_PARAMETER;
            }
            for (const int32_t new_label : descriptor_impl->label())
            {
                for (const int32_t label : member_impl->member_descriptor_.label())
                {
                    if (new_label == label)
                    {
                        EPROSIMA_LOG_ERROR(DYN_TYPES,
                                "Member " << member_impl->member_descriptor_.name().c_str() << " already contains the label " <<
                                label);
                        return RETCODE_BAD_PARAMETER;
                    }
                }

                // Recalculate the default discriminator value (for default case or default implicit member).
                if (new_label >= default_value_)
                {
                    default_value_ = new_label + 1;
                }
            }
        }

        // In case of default case, store the related MemberId.
        if (descriptor_impl->is_default_label())
        {
            default_union_member_ = member_id;
        }
    }
    //}}}
    //{{{ Specific checks for BITSET
    else if (TK_BITSET == type_descriptor_kind)
    {
        const MemberId new_member_id {dyn_member->get_descriptor().id()};
        const auto new_member_bound {type_descriptor_.bound().at(dyn_member->get_descriptor().index())};

        for (auto member : member_)
        {
            const MemberId mid {member.first};
            const auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member.second)};
            const auto member_index {member_impl->get_descriptor().index()};

            if (mid == new_member_id)
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistency in the new MemberId because it is equal to MemberId(" <<
                        mid << ")");
                return RETCODE_BAD_PARAMETER;
            }
            else if (mid < new_member_id)
            {
                const auto bound {type_descriptor_.bound().at(member_index)};

                if (new_member_id < mid + bound)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistency in the new MemberId because is less than MemberId(" <<
                            mid << ") + Bound(" << bound << ")");
                    return RETCODE_BAD_PARAMETER;
                }
            }
            else
            {
                if (mid < new_member_id + new_member_bound)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistency in the new MemberId because there is a member with id "
                            << "less than MemberId(" << new_member_id << ") + Bound(" << new_member_bound << ")");
                    return RETCODE_BAD_PARAMETER;
                }
            }
        }

        //TODO(richiware) Not valid when bitset refactored to support more than 64bits.
        if (64 < new_member_id + new_member_bound)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Inconsistency in the new MemberId because exceeds the maximum "
                    << "64 bits length is exceeded");
            return RETCODE_BAD_PARAMETER;
        }
    }
    //}}}
    //{{{ Specific checks for ENUM
    if (TK_ENUM == type_descriptor_kind)
    {
        if (0 < members_.size())
        {
            const auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(members_.at(0));

            if (member_impl->get_descriptor().type()->get_kind() != descriptor->type()->get_kind())
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor type kind differs from the current member types.");
                return RETCODE_BAD_PARAMETER;
            }
        }

        if (!descriptor->default_value().empty())
        {
            for (auto member : members_)
            {
                const auto member_impl {traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member)};

                // Check that there isn't already any member with the same default value.
                if (0 == descriptor->default_value().compare(member_impl->get_descriptor().default_value()))
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES,
                            "Member " << member_impl->member_descriptor_.name().c_str() <<
                            " has already the value");
                    return RETCODE_BAD_PARAMETER;
                }
            }
            TypeForKind<TK_INT32> value = TypeValueConverter::sto(descriptor->default_value());

            if (value >= default_value_)
            {
                default_value_ = value + 1;
                default_value_reverter.activate = true;
            }
        }
        else
        {
            dyn_member->get_descriptor().default_value(std::to_string(default_value_++));
            default_value_reverter.activate = true;
        }
    }
    //}}}

    // Set before calling is_consistent().
    dyn_member->get_descriptor().parent_kind(type_descriptor_kind);

    if (!dyn_member->get_descriptor().is_consistent())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor is not consistent");
        return RETCODE_BAD_PARAMETER;
    }

    assert(dyn_member->get_descriptor().index() <= members_.size());
    if (dyn_member->get_descriptor().index() < members_.size())
    {
        auto it = members_.begin() + dyn_member->get_descriptor().index();
        it = members_.insert(it, dyn_member);
        for (++it; it != members_.end(); ++it)
        {
            auto next_member = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(*it);
            next_member->get_descriptor().index(next_member->get_descriptor().index() + 1);
        }
        ++next_index_;
    }
    else
    {
        members_.push_back(dyn_member);
    }
    assert(next_index_ == traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(
                *members_.rbegin())->get_descriptor().index() + 1);

    member_by_name_.emplace(std::make_pair(member_name, dyn_member));
    if (TK_ANNOTATION == type_descriptor_kind ||
            TK_BITMASK == type_descriptor_kind ||
            TK_BITSET == type_descriptor_kind ||
            TK_STRUCTURE == type_descriptor_kind ||
            TK_UNION == type_descriptor_kind)
    {
        member_.emplace(std::make_pair(member_id, dyn_member));
    }

    id_reverter.activate = false;
    index_reverter.activate = false;
    default_value_reverter.activate = false;
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

    if (annotation_.end() != std::find_if(annotation_.begin(), annotation_.end(),
            [&descriptor_impl](AnnotationDescriptorImpl& x)
            {
                return x.equals(descriptor_impl);
            }))
    {
        return RETCODE_BAD_PARAMETER;
    }

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

    if (member_impl->annotation_.end() != std::find_if(member_impl->annotation_.begin(), member_impl->annotation_.end(),
            [&descriptor_impl](AnnotationDescriptorImpl& x)
            {
                return x.equals(descriptor_impl);
            }))
    {
        return RETCODE_BAD_PARAMETER;
    }

    member_impl->annotation_.emplace_back();
    member_impl->annotation_.back().copy_from(descriptor_impl);

    return RETCODE_OK;
}

traits<DynamicType>::ref_type DynamicTypeBuilderImpl::build() noexcept
{
    traits<DynamicTypeImpl>::ref_type ret_val;

    if (type_descriptor_.is_consistent())
    {
        bool preconditions {true};

        // In case of BITSET, verify the TypeDescriptor's bounds size is same as number of members.
        preconditions &= TK_BITSET != type_descriptor_.kind() || type_descriptor_.bound().size() == members_.size();
        if (!preconditions)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Expected more members in BITSET according to the size of bounds.");
        }

        // In case of ENUM and BITSET, it must have at least one member
        preconditions &= (TK_ENUM != type_descriptor_.kind() && TK_BITSET != type_descriptor_.kind())
                || 0 < members_.size();
        if (!preconditions)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Expected at least one member.");
        }

        if (preconditions)
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
            ret_val->default_value_ = default_value_;
            ret_val->default_union_member_ = default_union_member_;
            ret_val->index_own_members_ = index_own_members_;
        }
    }

    return ret_val;
}

ReturnCode_t DynamicTypeBuilderImpl::copy_from(
        traits<DynamicTypeImpl>::ref_type type)
{
    type_descriptor_.copy_from(type->type_descriptor_);
    for (auto& annotation : type->annotation_)
    {
        annotation_.emplace_back();
        annotation_.back().copy_from(annotation);
    }
    member_ = type->member_;
    member_by_name_ = type->member_by_name_;
    members_ = type->members_;
    for (auto& verbatim : type->verbatim_)
    {
        verbatim_.emplace_back();
        verbatim_.back().copy_from(verbatim);
    }
    return RETCODE_OK;
}

traits<DynamicTypeBuilder>::ref_type DynamicTypeBuilderImpl::_this()
{
    return shared_from_this();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
