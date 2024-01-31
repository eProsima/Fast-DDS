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

#include "MemberDescriptorImpl.hpp"

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "TypeValueConverter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

template<>
traits<MemberDescriptor>::ref_type traits<MemberDescriptor>::make_shared()
{
    return std::make_shared<MemberDescriptorImpl>();
}

ReturnCode_t MemberDescriptorImpl::copy_from(
        traits<MemberDescriptor>::ref_type descriptor) noexcept
{
    if (!descriptor)
    {
        return RETCODE_BAD_PARAMETER;
    }

    return copy_from(*traits<MemberDescriptor>::narrow<MemberDescriptorImpl>(descriptor));
}

ReturnCode_t MemberDescriptorImpl::copy_from(
        const MemberDescriptorImpl& descriptor) noexcept
{
    name_ = descriptor.name_;
    id_ = descriptor.id_;
    type_ = descriptor.type_;
    default_value_ = descriptor.default_value_;
    index_ = descriptor.index_;
    label_ = descriptor.label_;
    try_construct_kind_ = descriptor.try_construct_kind_;
    is_key_ = descriptor.is_key_;
    is_optional_ = descriptor.is_optional_;
    is_must_understand_ = descriptor.is_must_understand_;
    is_shared_ = descriptor.is_shared_;
    is_default_label_ = descriptor.is_default_label_;
    parent_kind_ = descriptor.parent_kind_;

    return RETCODE_OK;
}

bool MemberDescriptorImpl::equals(
        traits<MemberDescriptor>::ref_type descriptor) noexcept
{
    return equals(*traits<MemberDescriptor>::narrow<MemberDescriptorImpl>(descriptor));
}

bool MemberDescriptorImpl::equals(
        MemberDescriptorImpl& descriptor) noexcept
{
    return name_ == descriptor.name_ &&
           id_ == descriptor.id_ &&
           ((!type_ && !descriptor.type_) || (type_ && type_->equals(descriptor.type_))) &&
           default_value_ == descriptor.default_value_ &&
           index_ == descriptor.index_ &&
           label_ == descriptor.label_ &&
           try_construct_kind_ == descriptor.try_construct_kind_ &&
           is_key_ == descriptor.is_key_ &&
           is_optional_ == descriptor.is_optional_ &&
           is_must_understand_ == descriptor.is_must_understand_ &&
           is_shared_ == descriptor.is_shared_ &&
           is_default_label_ == descriptor.is_default_label_;
}

bool MemberDescriptorImpl::is_consistent() noexcept
{
    if (TK_NONE == parent_kind_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "To check consistency the descriptor should be added as member");
        return false;
    }

    // type_ cannot be nil.
    if (!type_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor has no type and it is mandatory");
        return false;
    }

    // Only aggregated types must use the ID value.
    if ((MEMBER_ID_INVALID == id_ && (TK_UNION == parent_kind_ || TK_STRUCTURE == parent_kind_)) ||
            (MEMBER_ID_INVALID != id_ && TK_UNION != parent_kind_ && TK_STRUCTURE != parent_kind_))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor has a wrong MemberId " << id_);
        return false;
    }

    // A union member cannot have the MemberId 0 because this value is for the discriminator
    if (TK_UNION == parent_kind_ && 0 == id_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Parent type is an UNION and the MemberId 0 is reserved to the discriminator");
        return false;
    }

    // Check default_label.
    if (is_default_label_ && TK_UNION != parent_kind_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Parent type is not a UNION and it has set is_default_label");
        return false;
    }

    // Check labels
    if ((TK_UNION != parent_kind_ && 0 < label_.size()) ||
            (TK_UNION == parent_kind_ && !is_default_label_ && 0 == label_.size()))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor is not consistent with its labels");
        return false;
    }

    if (!default_value_.empty() && !TypeValueConverter::is_string_consistent(type_->get_kind(), default_value_))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Default value is not consistent");
        return false;
    }

    // Check enum enclosing type.
    if (TK_ENUM == parent_kind_ &&
            TK_INT8 != type_->get_kind() &&
            TK_UINT8 != type_->get_kind() &&
            TK_INT16 != type_->get_kind() &&
            TK_UINT16 != type_->get_kind() &&
            TK_INT32 != type_->get_kind() &&
            TK_UINT32 != type_->get_kind())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Parent type is an ENUM and the enclosing type is not valid");
        return false;
    }

    // Check bitmask enclosing type.
    if (TK_BITMASK ==  parent_kind_ && TK_BOOLEAN != type_->get_kind())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Parent type is an BITMASK and the enclosing type is not BOOLEAN");
        return false;
    }

    // Check name consistency
    if (0 == name_.size() && (TK_ANNOTATION == parent_kind_ ||
            TK_BITMASK == parent_kind_ ||
            TK_ENUM == parent_kind_ ||
            TK_STRUCTURE == parent_kind_ ||
            TK_UNION == parent_kind_))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Parent type is an ANNOTATION|BITMASK|ENUM|STRUCTURE|UNION and the member has no name.");
        return false;
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
