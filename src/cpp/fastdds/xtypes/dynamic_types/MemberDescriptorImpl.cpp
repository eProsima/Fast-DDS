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

#include <algorithm>
#include <iomanip>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include "DynamicTypeBuilderFactoryImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include "MemberDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

MemberDescriptorImpl::MemberDescriptorImpl(
        const MemberDescriptor& descriptor)
    : id_{descriptor.get_id()}
    , index_{descriptor.get_index()}
    , default_label_{descriptor.get_default_label()}
{
    // copy name
    const char* name = descriptor.get_name();
    if (name != nullptr)
    {
        name_ = name;
    }

    // copy type
    const DynamicType* type = descriptor.get_type();
    if (type != nullptr)
    {
        type_ = DynamicTypeImpl::get_implementation(*type).shared_from_this();
        DynamicTypeBuilderFactory::get_instance().delete_type(type);
    }

    // copy flag
    const char* default_value = descriptor.get_default_value();
    if (default_value != nullptr)
    {
        default_value_ = default_value;
    }

    // copy labels
    uint32_t count;
    const uint32_t* labels = descriptor.get_labels(count);
    labels_ = std::set<uint64_t>{labels, labels + count};
}

void MemberDescriptorImpl::add_union_case_index(
        uint64_t value)
{
    labels_.insert(value);
}

bool MemberDescriptorImpl::operator ==(
        const MemberDescriptorImpl& other) const
{
    return name_ == other.name_ &&
           id_ == other.id_ &&
           default_value_ == other.default_value_ &&
           index_ == other.index_ &&
           labels_ == other.labels_ &&
           default_label_ == other.default_label_ &&
           (type_ == other.type_ || (type_ && other.type_ && *type_ == *other.type_ ));
}

bool MemberDescriptorImpl::operator !=(
        const MemberDescriptorImpl& other) const
{
    return !operator ==(other);
}

MemberId MemberDescriptorImpl::get_id() const
{
    return id_;
}

uint32_t MemberDescriptorImpl::get_index() const
{
    return index_;
}

TypeKind MemberDescriptorImpl::get_kind() const
{
    return type_ ? type_->get_kind() : TK_NONE;
}

const std::string& MemberDescriptorImpl::get_name() const
{
    return name_;
}

const std::set<uint64_t>& MemberDescriptorImpl::get_union_labels() const
{
    return labels_;
}

bool MemberDescriptorImpl::is_consistent(
        TypeKind parentKind) const
{
    // The type field is mandatory in every type except bitmasks and enums.
    // Structures and unions allow it for @external. This condition can only
    // be check in the DynamicTypeMember override
    if ((parentKind != TK_BITMASK && parentKind != TK_ENUM &&
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

    if (!is_default_value_consistent(default_value_))
    {
        return false;
    }

    if (type_ && !is_type_name_consistent(type_->get_name())) // Enums and bitmask don't have type
    {
        return false;
    }

    // Only Unions need the field "label"
    if (!labels_.empty() && parentKind != TK_UNION)
    {
        return false;
    }
    // If the field isn't the default value for the union, it must have a label value.
    else if (parentKind == TK_UNION && default_label_ == false && labels_.empty())
    {
        return false;
    }

    return true;
}

std::string MemberDescriptorImpl::get_default_value() const
{
    return default_value_;
}

bool MemberDescriptorImpl::is_default_union_value() const
{
    return default_label_;
}

bool MemberDescriptorImpl::is_default_value_consistent(
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
                case TK_CHAR8:
                case TK_BYTE:
                    return sDefaultValue.length() >= 1;
                case TK_CHAR16:
                {
                    std::wstring temp = std::wstring(sDefaultValue.begin(), sDefaultValue.end());
                    (void)temp;
                }
                break;
                case TK_BOOLEAN:
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
                case TK_STRING16:
                case TK_STRING8:
                    return true;
                case TK_ENUM:
                {
                    uint32_t value(0);
                    value = stoul(sDefaultValue);
                    (void)value;
                }
                break;
                case TK_BITMASK:
                {
                    int value(0);
                    value = stoi(sDefaultValue);
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

bool MemberDescriptorImpl::is_type_name_consistent(
        const std::string& sName) const
{
    return TypeState::is_type_name_consistent(sName);
}

void MemberDescriptorImpl::set_id(
        MemberId id)
{
    id_ = id;
}

void MemberDescriptorImpl::set_index(
        uint32_t index)
{
    index_ = index;
}

void MemberDescriptorImpl::set_type(
        std::shared_ptr<const DynamicTypeImpl>&& type)
{
    type_ = std::move(type);
}

std::shared_ptr<const DynamicTypeImpl> MemberDescriptorImpl::get_type() const
{
    return type_;
}

void MemberDescriptorImpl::set_default_union_value(
        bool bDefault)
{
    default_label_ = bDefault;
}

MemberDescriptor MemberDescriptorImpl::get_descriptor() const
{
    MemberDescriptor md;

    if (!name_.empty())
    {
        md.set_name(name_.c_str());
    }

    md.set_id(id_);

    if (type_)
    {
        md.set_type(&type_->get_interface());
    }

    if (!default_value_.empty())
    {
        md.set_default_value(default_value_.c_str());
    }

    md.set_index(index_);
    md.set_default_label(default_label_);

    if (!labels_.empty())
    {
        uint32_t size = static_cast<uint32_t>(labels_.size());
        std::vector<uint32_t> tmp(size);

        std::transform(
            labels_.begin(),
            labels_.end(),
            tmp.begin(),
            [](uint64_t val)
            {
                return static_cast<uint32_t>(val);
            });

        md.set_labels(tmp.data(), size);
    }

    return md;
}

std::ostream& operator <<(
        std::ostream& os,
        const MemberDescriptorImpl& md)
{
    using namespace std;

    // indentation increment
    ++os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);
    // parent object
    auto desc = static_cast<const TypeState*>(os.pword(DynamicTypeBuilderFactoryImpl::object_index));

    auto manips = [](ostream& os) -> ostream&
            {
                long indent = os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);
                return os << string(indent, '\t') << setw(10) << left;
            };

    // TODO: Barro, add support Type details and labels

    os << endl
       << manips << "index:" << md.get_index() << endl
       << manips << "name:" << md.get_name() << endl
       << manips << "id:" << md.get_id() << endl;

    if ( nullptr != desc && desc->get_kind() == TK_UNION)
    {
        os << manips << "default value:" << md.get_default_value() << endl
           << manips << "is default:" << std::boolalpha << md.is_default_union_value() << endl;
    }

    // Show type
    auto bt = md.get_type();
    if (bt)
    {
        os << manips << "type: ";
        os << *bt;
    }

    // indentation decrement
    --os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);

    return os;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
