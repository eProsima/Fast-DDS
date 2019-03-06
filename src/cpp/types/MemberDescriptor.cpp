// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

MemberDescriptor::MemberDescriptor()
: name_("")
, id_(MEMBER_ID_INVALID)
, type_(nullptr)
, default_value_("")
, index_(INDEX_INVALID)
, default_label_(false)
{
}

MemberDescriptor::MemberDescriptor(
        uint32_t index,
        const std::string& name)
    : name_(name)
    , id_(MEMBER_ID_INVALID)
    , type_(nullptr)
    , default_value_("")
    , index_(index)
    , default_label_(false)
{
}

MemberDescriptor::MemberDescriptor(const MemberDescriptor* descriptor)
: name_("")
, id_(MEMBER_ID_INVALID)
, type_(nullptr)
, default_value_("")
, index_(INDEX_INVALID)
, default_label_(false)
{
    copy_from(descriptor);
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_)
    : name_(name)
    , id_(id)
    , type_(type_)
    , default_value_("")
    , index_(INDEX_INVALID)
    , default_label_(false)
{

}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_,
        const std::string& defaultValue)
    : name_(name)
    , id_(id)
    , type_(type_)
    , default_value_(defaultValue)
    , index_(INDEX_INVALID)
    , default_label_(false)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_,
        const std::string& defaultValue,
        const std::vector<uint64_t>& unionLabels,
        bool isDefaultLabel)
    : name_(name)
    , id_(id)
    , type_(type_)
    , default_value_(defaultValue)
    , index_(INDEX_INVALID)
    , default_label_(isDefaultLabel)
{
    labels_ = unionLabels;
}

MemberDescriptor::~MemberDescriptor()
{
    type_ = nullptr;
}

void MemberDescriptor::add_union_case_index(uint64_t value)
{
    labels_.push_back(value);
}

bool MemberDescriptor::check_union_labels(const std::vector<uint64_t>& labels) const
{
    for (auto it = labels.begin(); it != labels.end(); ++it)
    {
        if (std::find(labels_.begin(), labels_.end(), *it) != labels_.end())
        {
            return false;
        }
    }
    return true;
}

ResponseCode MemberDescriptor::copy_from(const MemberDescriptor* other)
{
    if (other != nullptr)
    {
        try
        {
            type_ = other->type_;
            name_ = other->name_;
            id_ = other->id_;
            default_value_ = other->default_value_;
            index_ = other->index_;
            default_label_ = other->default_label_;
            labels_ = other->labels_;
            return ResponseCode::RETCODE_OK;
        }
        catch (std::exception& /*e*/)
        {
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error copying MemberDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool MemberDescriptor::equals(const MemberDescriptor* other) const
{
    if (other != nullptr && name_ == other->name_ && id_ == other->id_ &&
        ((type_ == nullptr && other->type_ == nullptr) || type_->equals(other->type_.get())) &&
        default_value_ == other->default_value_ && index_ == other->index_ && default_label_ == other->default_label_ &&
        labels_.size() == other->labels_.size())
    {
        for (auto it = labels_.begin(), it2 = other->labels_.begin(); it != labels_.end(); ++it, ++it2)
        {
            if (*it != *it2)
                return false;
        }
        return true;
    }
    return false;
}

MemberId MemberDescriptor::get_id() const
{
    return id_;
}

uint32_t MemberDescriptor::get_index() const
{
    return index_;
}

TypeKind MemberDescriptor::get_kind() const
{
    if (type_ != nullptr)
    {
        return type_->get_kind();
    }
    return 0;
}

std::string MemberDescriptor::get_name() const
{
    return name_;
}

std::vector<uint64_t> MemberDescriptor::get_union_labels() const
{
    return labels_;
}

bool MemberDescriptor::is_consistent(TypeKind parentKind) const
{
    // The type field is mandatory in every type except bitmasks and enums.
    if ((parentKind != TK_BITMASK && parentKind != TK_ENUM) && type_ == nullptr)
    {
        return false;
    }

    // Only aggregated types must use the ID value.
    if (id_ != MEMBER_ID_INVALID && parentKind != TK_UNION && parentKind != TK_STRUCTURE &&
        parentKind != TK_ANNOTATION)
    {
        return false;
    }

    if (!is_default_value_consistent(default_value_))
    {
        return false;
    }

    if (!is_type_name_consistent(name_))
    {
        return false;
    }

    // Only Unions need the field "label"
    if (labels_.size() != 0 && parentKind != TK_UNION)
    {
        return false;
    }
    // If the field isn't the default value for the union, it must have a label value.
    else if (parentKind == TK_UNION && default_label_ == false && labels_.size() == 0)
    {
        return false;
    }

    return true;
}

bool MemberDescriptor::is_default_union_value() const
{
    return default_label_;
}

bool MemberDescriptor::is_default_value_consistent(const std::string& sDefaultValue) const
{
    if (sDefaultValue.length() > 0)
    {
        try
        {
            switch (get_kind())
            {
            default:
                return true;
            case TK_INT32:
            {
                int32_t value(0);
                value = stoi(sDefaultValue);
                (void)value;
            }
            break;
            case TK_UINT32:
            {
                uint32_t value(0);
                value = stoul(sDefaultValue);
                (void)value;
            }
            break;
            case TK_INT16:
            {
                int16_t value(0);
                value = static_cast<int16_t>(stoi(sDefaultValue));
                (void)value;
            }
            break;
            case TK_UINT16:
            {
                uint16_t value(0);
                value = static_cast<uint16_t>(stoul(sDefaultValue));
                (void)value;
            }
            break;
            case TK_INT64:
            {
                int64_t value(0);
                value = stoll(sDefaultValue);
                (void)value;
            }
            break;
            case TK_UINT64:
            {
                uint64_t value(0);
                value = stoul(sDefaultValue);
                (void)value;
            }
            break;
            case TK_FLOAT32:
            {
                float value(0.0f);
                value = stof(sDefaultValue);
                (void)value;
            }
            break;
            case TK_FLOAT64:
            {
                double value(0.0f);
                value = stod(sDefaultValue);
                (void)value;
            }
            break;
            case TK_FLOAT128:
            {
                long double value(0.0f);
                value = stold(sDefaultValue);
                (void)value;
            }
            break;
            case TK_CHAR8: {   return sDefaultValue.length() >= 1; }
            case TK_CHAR16:
            {
                std::wstring temp = std::wstring(sDefaultValue.begin(), sDefaultValue.end());
                (void)temp;
            }
            break;
            case TK_BOOLEAN:
            {
                int value(0);
                value = stoi(sDefaultValue);
                (void)value;
            }
            break;
            case TK_BYTE: {   return sDefaultValue.length() >= 1; }   break;
            case TK_STRING16: {   return true;    }
            case TK_STRING8: {   return true;    }
            case TK_ENUM:
            {
                uint32_t value(0);
                value = stoul(sDefaultValue);
                (void)value;
            }
            break;
            case TK_BITSET:
            case TK_BITMASK:
            {
                int value(0);
                value = stoi(sDefaultValue);
                (void)value;
            }
            break;
            case TK_ARRAY: {   return true;    }
            case TK_SEQUENCE: {   return true;    }
            case TK_MAP: {   return true;    }
            }
        }
        catch (...)
        {
            return false;
        }
    }
    return true;
}

bool MemberDescriptor::is_type_name_consistent(const std::string& sName) const
{
    // The first letter must start with a letter ( uppercase or lowercase )
    if (sName.length() > 0 && std::isalpha(sName[0]))
    {
        // All characters must be letters, numbers or underscore.
        for (uint32_t i = 1; i < sName.length(); ++i)
        {
            if (!std::isalnum(sName[i]) && sName[i] != 95)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

void MemberDescriptor::set_id(MemberId id)
{
    id_ = id;
}

void MemberDescriptor::set_index(uint32_t index)
{
    index_ = index;
}

void MemberDescriptor::set_name(const std::string& name)
{
    name_ = name;
}

void MemberDescriptor::set_type(DynamicType_ptr type)
{
    type_ = type;
}

void MemberDescriptor::set_default_union_value(bool bDefault)
{
    default_label_ = bDefault;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
