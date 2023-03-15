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
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastdds/dds/log/Log.hpp>

#include <iomanip>

using namespace eprosima::fastrtps::types;

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name)
    : name_(name)
    , id_(id)
{
}

MemberDescriptor::MemberDescriptor(
        uint32_t index,
        MemberId id,
        const std::string& name)
    : name_(name)
    , id_(id)
    , index_(index)
{
}

MemberDescriptor::MemberDescriptor(
        uint32_t index,
        MemberId id,
        const std::string& name,
        DynamicType_ptr type)
    : name_(name)
    , id_(id)
    , type_(type)
    , index_(index)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type)
    : name_(name)
    , id_(id)
    , type_(type)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type,
        const std::string& defaultValue)
    : name_(name)
    , id_(id)
    , type_(type)
    , default_value_(defaultValue)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type,
        const std::string& defaultValue,
        const std::vector<uint64_t>& unionLabels,
        bool isDefaultLabel)
    : name_(name)
    , id_(id)
    , type_(type)
    , default_value_(defaultValue)
    , labels_{unionLabels.begin(), unionLabels.end()}
    , default_label_(isDefaultLabel)
{
}

void MemberDescriptor::add_union_case_index(
        uint64_t value)
{
    labels_.insert(value);
}

bool MemberDescriptor::check_union_labels(
        const std::vector<uint64_t>& labels) const
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

ReturnCode_t MemberDescriptor::copy_from(
        const MemberDescriptor& other)
{
    *this = other;
    return ReturnCode_t::RETCODE_OK;
}

bool MemberDescriptor::operator==(const MemberDescriptor& other) const
{
    return name_ == other.name_ &&
           id_ == other.id_ &&
           default_value_ == other.default_value_ &&
           index_ == other.index_ &&
           labels_ == other.labels_ &&
           default_label_ == other.default_label_ &&
           (type_ == other.type_ || (type_ && other.type_ && *type_ == *other.type_ ));
}

bool MemberDescriptor::operator!=(const MemberDescriptor& other) const
{
    return !operator==(other);
}

bool MemberDescriptor::equals(
        const MemberDescriptor& other) const
{
    return *this == other;
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
    return type_ ? type_->get_kind() : TypeKind::TK_NONE;
}

std::string MemberDescriptor::get_name() const
{
    return name_;
}

std::vector<uint64_t> MemberDescriptor::get_union_labels() const
{
    return std::vector<uint64_t>(labels_.begin(), labels_.end());
}

bool MemberDescriptor::is_consistent(
        TypeKind parentKind) const
{
    // The type field is mandatory in every type except bitmasks and enums.
    if ((parentKind != TypeKind::TK_BITMASK && parentKind != TypeKind::TK_ENUM) && !type_)
    {
        return false;
    }

    // Only enums, bitmaks and aggregated types must use the ID value.
    if (id_ != MEMBER_ID_INVALID && parentKind != TypeKind::TK_UNION &&
            parentKind != TypeKind::TK_STRUCTURE && parentKind != TypeKind::TK_BITSET &&
            parentKind != TypeKind::TK_ANNOTATION && parentKind != TypeKind::TK_ENUM &&
            parentKind != TypeKind::TK_BITMASK)
    {
        return false;
    }

    if (!is_default_value_consistent(default_value_))
    {
        return false;
    }

    if (type_ && !is_type_name_consistent(type_->get_name())) // Enums and bitmask don't have type
    {
        return false;
    }

    // Only Unions need the field "label"
    if (!labels_.empty() && parentKind != TypeKind::TK_UNION)
    {
        return false;
    }
    // If the field isn't the default value for the union, it must have a label value.
    else if (parentKind == TypeKind::TK_UNION && default_label_ == false && labels_.empty())
    {
        return false;
    }

    return true;
}

std::string MemberDescriptor::get_default_value() const
{
    return default_value_;
}

bool MemberDescriptor::is_default_union_value() const
{
    return default_label_;
}

bool MemberDescriptor::is_default_value_consistent(
        const std::string& sDefaultValue) const
{
    if (sDefaultValue.length() > 0)
    {
        try
        {
            switch (get_kind())
            {
                default:
                    return true;
                case TypeKind::TK_INT32:
                {
                    int32_t value(0);
                    value = stoi(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_UINT32:
                {
                    uint32_t value(0);
                    value = stoul(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_INT16:
                {
                    int16_t value(0);
                    value = static_cast<int16_t>(stoi(sDefaultValue));
                    (void)value;
                }
                break;
                case TypeKind::TK_UINT16:
                {
                    uint16_t value(0);
                    value = static_cast<uint16_t>(stoul(sDefaultValue));
                    (void)value;
                }
                break;
                case TypeKind::TK_INT64:
                {
                    int64_t value(0);
                    value = stoll(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_UINT64:
                {
                    uint64_t value(0);
                    value = stoul(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_FLOAT32:
                {
                    float value(0.0f);
                    value = stof(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_FLOAT64:
                {
                    double value(0.0f);
                    value = stod(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_FLOAT128:
                {
                    long double value(0.0f);
                    value = stold(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_CHAR8:
                case TypeKind::TK_BYTE:
                    return sDefaultValue.length() >= 1;
                case TypeKind::TK_CHAR16:
                {
                    std::wstring temp = std::wstring(sDefaultValue.begin(), sDefaultValue.end());
                    (void)temp;
                }
                break;
                case TypeKind::TK_BOOLEAN:
                {
                    if (sDefaultValue == CONST_TRUE || sDefaultValue == CONST_FALSE)
                    {
                        return true;
                    }
                    int value(0);
                    value = stoi(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_STRING16:
                case TypeKind::TK_STRING8:
                    return true;
                case TypeKind::TK_ENUM:
                {
                    uint32_t value(0);
                    value = stoul(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_BITMASK:
                {
                    int value(0);
                    value = stoi(sDefaultValue);
                    (void)value;
                }
                break;
                case TypeKind::TK_ARRAY:
                case TypeKind::TK_SEQUENCE:
                case TypeKind::TK_MAP:
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

bool MemberDescriptor::is_type_name_consistent(
        const std::string& sName) const
{
    return TypeDescriptor::is_type_name_consistent(sName);
}

void MemberDescriptor::set_id(
        MemberId id)
{
    id_ = id;
}

void MemberDescriptor::set_index(
        uint32_t index)
{
    index_ = index;
}

void MemberDescriptor::set_name(
        const std::string& name)
{
    name_ = name;
}

void MemberDescriptor::set_type(
        DynamicType_ptr&& type)
{
    type_ = std::move(type);
}

void MemberDescriptor::set_type(
        const DynamicType_ptr& type)
{
    type_ = type;
}

DynamicType_ptr MemberDescriptor::get_type() const
{
    return type_;
}

void MemberDescriptor::set_default_union_value(
        bool bDefault)
{
    default_label_ = bDefault;
}

std::ostream& eprosima::fastrtps::types::operator<<( std::ostream& os, const MemberDescriptor& md)
{
    using namespace std;

    // indentation increment
    ++os.iword(DynamicTypeBuilderFactory::indentation_index);

    auto manips = [](ostream& os) -> ostream&
    {
        long indent = os.iword(DynamicTypeBuilderFactory::indentation_index);
        return os << setw(10) << left << string(indent, '\t');
    };

    // TODO: Barro, add support Type details and labels

    os << endl
       << manips << "index:" << md.get_index() << endl
       << manips << "name:" << md.get_name() << endl
       << manips << "id:" << md.get_id() << endl;

    // Show type
    auto bt = md.get_type();
    if (bt)
    {
        os << manips << "type: ";
        os << *bt << endl;
    }

    // indentation decrement
    --os.iword(DynamicTypeBuilderFactory::indentation_index);

    return os;
}
