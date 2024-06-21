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
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "DynamicTypeImpl.hpp"
#include "TypeValueConverter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

bool default_value_compare(
        const traits<DynamicType>::ref_type type,
        const std::string& d1,
        const std::string& d2)
{
    bool ret_value {false};
    if (d1 == d2)
    {
        ret_value = true;
    }
    else
    {
        switch (type->get_kind())
        {
            case TK_BOOLEAN:
                ret_value = (d1.empty() && 0 == d2.compare(CONST_FALSE)) ||
                        (0 == d1.compare(CONST_FALSE) && d2.empty());
                break;
            case TK_BYTE:
            case TK_INT8:
            case TK_UINT8:
            case TK_INT16:
            case TK_UINT16:
            case TK_INT32:
            case TK_UINT32:
            case TK_INT64:
            case TK_UINT64:
            case TK_FLOAT32:
            case TK_FLOAT64:
            case TK_FLOAT128:
            case TK_CHAR8:
            case TK_CHAR16:
            case TK_ENUM:
                ret_value = (d1.empty() && 0 == d2.compare("0")) ||
                        (0 == d1.compare("0") && d2.empty());
                break;
            default:
                break;
        }
    }
    return ret_value;
}

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
    is_try_construct_kind_set_ = descriptor.is_try_construct_kind_set_;

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
           (type_ && type_->equals(descriptor.type_)) &&
           default_value_compare(type_, default_value_, descriptor.default_value_) &&
           index_ == descriptor.index_ &&
           equal_labels(descriptor.label_) &&
           try_construct_kind_ == descriptor.try_construct_kind_ &&
           is_key_ == descriptor.is_key_ &&
           is_optional_ == descriptor.is_optional_ &&
           is_must_understand_ == descriptor.is_must_understand_ &&
           is_shared_ == descriptor.is_shared_ &&
           is_default_label_ == descriptor.is_default_label_;
}

bool MemberDescriptorImpl::equal_labels(
        UnionCaseLabelSeq& labels) noexcept
{
    bool ret_code = true;
    ret_code &= (labels.size() == label_.size());
    if (ret_code)
    {
        for (size_t count {0}; ret_code && count < label_.size(); ++count)
        {
            ret_code &= label_.end() != std::find(label_.begin(), label_.end(), labels[count]);
        }
    }
    return ret_code;
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

    auto type = traits<DynamicType>::narrow<DynamicTypeImpl>(type_);

    // Only aggregated types must use the ID value.
    if ((MEMBER_ID_INVALID == id_ && (TK_ANNOTATION == parent_kind_ ||
            TK_BITMASK == parent_kind_ ||
            TK_BITSET == parent_kind_ ||
            TK_UNION == parent_kind_ ||
            TK_STRUCTURE == parent_kind_)) ||
            (MEMBER_ID_INVALID != id_ && TK_ANNOTATION != parent_kind_ &&
            TK_BITMASK != parent_kind_ &&
            TK_BITSET != parent_kind_ &&
            TK_UNION != parent_kind_ &&
            TK_STRUCTURE != parent_kind_))
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

    // Check uniqueness of labels.
    if (TK_UNION == parent_kind_)
    {
        UnionCaseLabelSeq label_copy {label_};
        std::sort(label_copy.begin(), label_copy.end());
        auto last = std::unique(label_copy.begin(), label_copy.end());
        if (label_copy.end() != last)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Descriptor contains duplicated labels");
            return false;
        }
    }

    if (!default_value_.empty() &&
            !TypeValueConverter::is_string_consistent(type->get_kind(), type->get_all_members_by_index(),
            default_value_))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Default value is not consistent");
        return false;
    }

    // Check bitfield|enum enclosing type.
    if (TK_BITSET == parent_kind_ ||
            TK_ENUM == parent_kind_)
    {

        TypeKind kind = type->resolve_alias_enclosed_type()->get_kind();

        switch (kind)
        {
            case TK_INT8:
            case TK_INT16:
            case TK_INT32:
                break;
            case TK_UINT8:
            case TK_UINT16:
            case TK_UINT32:
            case TK_INT64:
            case TK_UINT64:
            case TK_BOOLEAN:
            case TK_BYTE:
                if (TK_ENUM == parent_kind_)
                {
                    EPROSIMA_LOG_ERROR(DYN_TYPES, "Parent type is an ENUM and the enclosing type is not valid");
                    return false;
                }
                break;
            default:
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Parent type is an BITSET|ENUM and the enclosing type is not valid");
                return false;
                break;
        }
    }

    // Check bitmask enclosing type.
    if (TK_BITMASK ==  parent_kind_ && TK_BOOLEAN != type->get_kind())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Parent type is a BITMASK and the enclosing type is not BOOLEAN");
        return false;
    }

    // Check is_key built-in annotation.
    if (is_key_ && TK_STRUCTURE != parent_kind_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "is_key is set but parent type of member is not TK_STRUCTURE.");
        return false;
    }

    // Check is_must_understand built-in annotation.
    if (is_must_understand_)
    {
        if (TK_STRUCTURE != parent_kind_)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "is_must_understand is set but parent type of member is not TK_STRUCTURE.");
            return false;
        }
        else
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "is_must_understand is not implemented yet.");
        }
    }

    // Check is_optional built-in annotation.
    if (is_optional_)
    {
        if (TK_STRUCTURE != parent_kind_)
        {
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "is_optional is set but parent type of member is not TK_STRUCTURE.");
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "is_optional is not implemented yet.");
        }
    }

    // Check is_shared built-in annotation.
    if (is_shared_)
    {
        if (TK_STRUCTURE != parent_kind_ && TK_UNION != parent_kind_)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "is_shared is set but parent type of member is not TK_STRUCTURE or TK_UNION.");
            return false;
        }
        else
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "is_shared is not implemented yet.");
        }
    }

    // Check try_construct built-in annotation.
    if (is_try_construct_kind_set_)
    {
        if (TK_STRUCTURE != parent_kind_ && TK_UNION != parent_kind_)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "try_construct_kind is set but parent type of member is not TK_STRUCTURE or TK_UNION.");
            return false;
        }
        else if (TryConstructKind::DISCARD != try_construct_kind_)
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "try_construct_kind is not implemented yet.");
        }
    }

    // TK_MAP member cannot be key.
    if (TK_MAP == type->get_kind() && is_key_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "TK_MAP member cannot be part of the key.");
        return false;
    }

    // Check name consistency
    if (0 == name_.size() && (TK_ANNOTATION == parent_kind_ ||
            TK_BITMASK == parent_kind_ ||
            TK_BITSET == parent_kind_ ||
            TK_ENUM == parent_kind_ ||
            TK_STRUCTURE == parent_kind_ ||
            TK_UNION == parent_kind_))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Parent type is an ANNOTATION|BITMASK|BITSET|ENUM|STRUCTURE|UNION and the member has no name.");
        return false;
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
