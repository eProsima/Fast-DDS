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

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

bool is_default_value_consistent(
        TypeKind kind,
        const std::string& default_value)
{
    if (default_value.length() > 0)
    {
        try
        {
            switch (kind)
            {
                default:
                    return true;
                case TK_INT32:
                {
                    int32_t value(0);
                    value = stoi(default_value);
                    (void)value;
                }
                break;
                case TK_UINT32:
                {
                    uint32_t value(0);
                    value = stoul(default_value);
                    (void)value;
                }
                break;
                case TK_INT16:
                {
                    int16_t value(0);
                    value = static_cast<int16_t>(stoi(default_value));
                    (void)value;
                }
                break;
                case TK_UINT16:
                {
                    uint16_t value(0);
                    value = static_cast<uint16_t>(stoul(default_value));
                    (void)value;
                }
                break;
                case TK_INT64:
                {
                    int64_t value(0);
                    value = stoll(default_value);
                    (void)value;
                }
                break;
                case TK_UINT64:
                {
                    uint64_t value(0);
                    value = stoul(default_value);
                    (void)value;
                }
                break;
                case TK_FLOAT32:
                {
                    float value(0.0f);
                    value = stof(default_value);
                    (void)value;
                }
                break;
                case TK_FLOAT64:
                {
                    double value(0.0f);
                    value = stod(default_value);
                    (void)value;
                }
                break;
                case TK_FLOAT128:
                {
                    long double value(0.0f);
                    value = stold(default_value);
                    (void)value;
                }
                break;
                case TK_CHAR8:
                case TK_BYTE:
                    return default_value.length() >= 1;
                case TK_CHAR16:
                {
                    std::wstring temp = std::wstring(default_value.begin(), default_value.end());
                    (void)temp;
                }
                break;
                case TK_BOOLEAN:
                {
                    if (default_value == CONST_TRUE || default_value == CONST_FALSE)
                    {
                        return true;
                    }
                    int value(0);
                    value = stoi(default_value);
                    (void)value;
                }
                break;
                case TK_STRING16:
                case TK_STRING8:
                    return true;
                case TK_ENUM:
                {
                    uint32_t value(0);
                    value = stoul(default_value);
                    (void)value;
                }
                break;
                case TK_BITMASK:
                {
                    int value(0);
                    value = stoi(default_value);
                    (void)value;
                }
                break;
                case TK_ARRAY:
                case TK_SEQUENCE:
                case TK_MAP:
                    return true;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    return true;
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
           type_ == descriptor.type_ && // TODO(richiware) change when dynamictype has equals.
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
        return false;
    }

    // type_ cannot be nil.
    if (!type_)
    {
        return false;
    }

    // Only aggregated types must use the ID value.
    if ((MEMBER_ID_INVALID == id_ && (TK_UNION == parent_kind_ || TK_STRUCTURE == parent_kind_)) ||
            MEMBER_ID_INVALID != id_)
    {
        return false;
    }

    // Check default_label.
    if (is_default_label_ && TK_UNION != parent_kind_)
    {
        return false;
    }

    // Check labels
    if ((TK_UNION != parent_kind_ && 0 < label_.size()) ||
            (TK_UNION == parent_kind_ && !is_default_label_ && 0 == label_.size()))
    {
        return false;
    }

    if (!is_default_value_consistent(type_->get_kind(), default_value_))
    {
        return false;
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
