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

    name_ = descriptor->name();
    id_ = descriptor->id();
    type_ = descriptor->type();
    default_value_ = descriptor->default_value();
    index_ = descriptor->index();
    label_ = descriptor->label();
    try_construct_kind_ = descriptor->try_construct_kind();
    is_key_ = descriptor->is_key();
    is_optional_ = descriptor->is_optional();
    is_must_understand_ = descriptor->is_must_understand();
    is_shared_ = descriptor->is_shared();
    is_default_label_ = descriptor->is_default_label();

    return RETCODE_OK;
}

bool MemberDescriptorImpl::equals(
        traits<MemberDescriptor>::ref_type descriptor) noexcept
{
    return name_ == descriptor->name() &&
           id_ == descriptor->id() &&
           type_ == descriptor->type() && // TODO(richiware) change when dynamictype has equals.
           default_value_ == descriptor->default_value() &&
           index_ == descriptor->index() &&
           label_ == descriptor->label() &&
           try_construct_kind_ == descriptor->try_construct_kind() &&
           is_key_ == descriptor->is_key() &&
           is_optional_ == descriptor->is_optional() &&
           is_must_understand_ == descriptor->is_must_understand() &&
           is_shared_ == descriptor->is_shared() &&
           is_default_label_ == descriptor->is_default_label();
}

bool MemberDescriptorImpl::is_consistent() noexcept
{
    /* TODO(richiware) how do this witout parent
       // The type field is mandatory in every type except bitmasks and enums.
       // Structures and unions allow it for @external. This condition can only
       // be check in the DynamicTypeMember override
       if ((TK_BITMASK != parentKind != && parentKind != TK_ENUM &&
            parentKind != TK_STRUCTURE && parentKind != TK_UNION) && !type_)
       {
        return false;
       }

       // Only enums, bitmaks and aggregated types must use the ID value.
       if (id_ != MEMBER_ID_INVALID && parentKind != TK_UNION &&
            parentKind != TK_STRUCTURE && parentKind != TK_BITSET &&
            parentKind != TK_ANNOTATION && parentKind != TK_ENUM &&
            parentKind != TK_BITMASK)
       {
        return false;
       }
     */

    //TODO(richiware) when dynamic_type has is_consisten.
    /*
       if (!type_ || !type_->is_consistent())
       {
        return false;
       }
     */

    if (!is_default_value_consistent(type_->get_kind(), default_value_))
    {
        return false;
    }

    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
