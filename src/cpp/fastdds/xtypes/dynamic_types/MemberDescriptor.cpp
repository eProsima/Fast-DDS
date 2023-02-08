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

#include <sstream>

#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include "MemberDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

MemberDescriptor::~MemberDescriptor() noexcept
{
    if (type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(type_);
    }
}

MemberDescriptor::MemberDescriptor(
        const MemberDescriptor& member) noexcept
    : name_(member.name_)
    , id_(member.id_)
    , default_value_(member.default_value_)
    , index_(member.index_)
    , labels_(member.labels_)
    , default_label_(member.default_label_)
{
    if (member.type_ != nullptr)
    {
        type_ = DynamicTypeBuilderFactory::get_instance().create_copy(*member.type_);
    }
}

MemberDescriptor::MemberDescriptor(
        MemberDescriptor&& member) noexcept
    : name_(std::move(member.name_))
    , id_(member.id_)
    , type_(member.type_)
    , default_value_(std::move(member.default_value_))
    , index_(member.index_)
    , labels_(std::move(member.labels_))
    , default_label_(member.default_label_)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const char* name) noexcept
    : name_(name)
    , id_(id)
{
}

MemberDescriptor::MemberDescriptor(
        MemberId id,
        const char* name,
        const DynamicType* type) noexcept
    : name_(name)
    , id_(id)
    , type_(type)
{
}

/* TODO(richiware) remove index
   MemberDescriptor::MemberDescriptor(
        uint32_t index,
        const char* name,
        const DynamicType* type) noexcept
    : name_(name)
    , type_(type)
    , index_(index)
   {
   }
 */

MemberDescriptor& MemberDescriptor::operator =(
        const MemberDescriptor& member) noexcept
{
    name_ = member.name_;
    id_ = member.id_;
    default_value_ = member.default_value_;
    index_ = member.index_;
    labels_ = member.labels_;
    default_label_ = member.default_label_;

    if (member.type_ != nullptr)
    {
        type_ = DynamicTypeBuilderFactory::get_instance().create_copy(*member.type_);
    }

    return *this;
}

MemberDescriptor& MemberDescriptor::operator =(
        MemberDescriptor&& member) noexcept
{
    name_ = std::move(member.name_);
    id_ = member.id_;
    type_ = member.type_;
    default_value_ = std::move(member.default_value_);
    index_ = member.index_;
    labels_ = std::move(member.labels_);
    default_label_ = member.default_label_;

    return *this;
}

bool MemberDescriptor::operator ==(
        const MemberDescriptor& d) const noexcept
{
    return name_ == d.name_
           && id_ == d.id_
           && (type_ == d.type_ || (type_ != nullptr && d.type_ != nullptr && *type_ == *d.type_ ))
           && default_value_ == d.default_value_
           && index_ == d.index_
           && labels_ == d.labels_
           && default_label_ == d.default_label_;
}

bool MemberDescriptor::operator !=(
        const MemberDescriptor& descriptor) const noexcept
{
    return !this->operator ==(descriptor);
}

const char* MemberDescriptor::get_name() const noexcept
{
    return name_.c_str();
}

void MemberDescriptor::set_name(
        const char* name) noexcept
{
    if (nullptr == name)
    {
        return;
    }

    name_ = name;
}

const DynamicType* MemberDescriptor::get_type() const noexcept
{
    if (type_ != nullptr)
    {
        return DynamicTypeBuilderFactory::get_instance().create_copy(*type_);
    }

    return nullptr;
}

void MemberDescriptor::set_type(
        const DynamicType& type) noexcept
{
    reset_type();
    type_ = DynamicTypeBuilderFactory::get_instance().create_copy(type);
}

void MemberDescriptor::set_type(
        const DynamicType* type) noexcept
{
    type_ = type;
}

void MemberDescriptor::reset_type() noexcept
{
    if (type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(type_);
    }

    type_ = nullptr;
}

const char* MemberDescriptor::get_default_value() const noexcept
{
    return default_value_.c_str();
}

void MemberDescriptor::set_default_value(
        const char* value) noexcept
{
    if (nullptr == value)
    {
        return;
    }

    default_value_ = value;
}

const uint32_t* MemberDescriptor::get_labels(
        uint32_t& count) const noexcept
{
    count = static_cast<uint32_t>(labels_.size());
    return labels_.data();
}

void MemberDescriptor::set_labels(
        const uint32_t* labels,
        uint32_t count) noexcept
{
    labels_.assign(labels, labels + count);
}

ReturnCode_t MemberDescriptor::copy_from(
        const MemberDescriptor& descriptor) noexcept
{
    this->operator =(descriptor);
    return RETCODE_OK;
}

bool MemberDescriptor::equals(
        const MemberDescriptor& descriptor) const noexcept
{
    return this->operator ==(descriptor);
}

bool MemberDescriptor::is_consistent(
        TypeKind parentKind /* = TypeKind::TK_STRUCTURE*/) const noexcept
{
    return MemberDescriptorImpl(*this).is_consistent(parentKind);
}

ReturnCode_t MemberDescriptor::pretty_print(
        char* buffer,
        uint32_t size,
        uint32_t& required) const noexcept
{
    std::ostringstream os;
    os << MemberDescriptorImpl(*this);
    auto str = os.str();
    required = str.size() + 1;

    if (size >= required)
    {
        str.copy(buffer, size);
        return {};
    }

    return RETCODE_OUT_OF_RESOURCES;
}

std::ostream& operator <<(
        std::ostream& os,
        const MemberDescriptor& md)
{
    uint32_t size = 256;
    uint32_t required;
    std::vector<char> buf(size);

    if (!md.pretty_print(buf.data(), size, required))
    {
        size = required;
        buf.resize(size);
        md.pretty_print(buf.data(), size, required);
    }

    return os.write(buf.data(), required);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
