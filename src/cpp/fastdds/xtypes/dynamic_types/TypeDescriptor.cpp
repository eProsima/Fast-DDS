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
#include <sstream>

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include "TypeState.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

TypeDescriptor::TypeDescriptor() noexcept
    : name_(new(std::nothrow) std::string)
    , bounds_(new(std::nothrow) std::vector<uint32_t>)
{
}

TypeDescriptor::TypeDescriptor(
        const char* name,
        TypeKind kind) noexcept
    : name_(new(std::nothrow) std::string{name})
    , kind_{kind}
    , bounds_(new(std::nothrow) std::vector<uint32_t>)
{
}

TypeDescriptor::TypeDescriptor(
        const TypeDescriptor& type) noexcept
    : name_(new(std::nothrow) std::string)
    , kind_(type.get_kind())
    , base_type_(type.get_base_type())
    , discriminator_type_(type.get_discriminator_type())
    , bounds_(new(std::nothrow) std::vector<uint32_t>)
    , element_type_(type.get_element_type())
    , key_element_type_(type.get_key_element_type())
{
    if (name_ != nullptr && type.name_ != nullptr)
    {
        *name_ = *type.name_;
    }

    if (bounds_ != nullptr && type.bounds_ != nullptr)
    {
        *bounds_ = *type.bounds_;
    }
}

TypeDescriptor::TypeDescriptor(
        TypeDescriptor&& type) noexcept
    : name_(type.name_)
    , kind_(type.kind_)
    , base_type_(type.base_type_)
    , discriminator_type_(type.discriminator_type_)
    , bounds_(type.bounds_)
    , element_type_(type.element_type_)
    , key_element_type_(type.key_element_type_)
{
}

TypeDescriptor::~TypeDescriptor() noexcept
{
    reset_base_type();
    reset_discriminator_type();
    reset_element_type();
    reset_key_element_type();

    delete name_;
    delete bounds_;
}

TypeDescriptor& TypeDescriptor::operator =(
        const TypeDescriptor& type) noexcept
{
    if (name_ != nullptr && type.name_ != nullptr)
    {
        *name_ = *type.name_;
    }

    if (bounds_ != nullptr && type.bounds_ != nullptr)
    {
        *bounds_ = *type.bounds_;
    }

    kind_ = type.kind_;
    base_type_ = type.get_base_type();
    discriminator_type_ = type.get_discriminator_type();
    element_type_ = type.get_element_type();
    key_element_type_ = type.get_key_element_type();

    return *this;
}

TypeDescriptor& TypeDescriptor::operator =(
        TypeDescriptor&& type) noexcept
{
    name_ = type.name_;
    kind_ = type.kind_;
    base_type_ = type.base_type_;
    discriminator_type_ = type.discriminator_type_;
    bounds_ = type.bounds_;
    element_type_ = type.element_type_;
    key_element_type_ = type.key_element_type_;

    return *this;
}

bool TypeDescriptor::operator ==(
        const TypeDescriptor& descriptor) const noexcept
{
    return TypeState(*this) == TypeState(descriptor);
}

bool TypeDescriptor::operator !=(
        const TypeDescriptor& descriptor) const noexcept
{
    return !this->operator ==(descriptor);
}

const char* TypeDescriptor::get_name() const noexcept
{
    return name_->c_str();
}

void TypeDescriptor::set_name(
        const char* name) noexcept
{
    if (nullptr == name_)
    {
        name_ = new(std::nothrow) std::string(name);
    }
    else
    {
        *name_ = name;
    }
}

void TypeDescriptor::set_kind(
        TypeKind kind) noexcept
{
    kind_ = kind;
}

TypeKind TypeDescriptor::get_kind() const noexcept
{
    return kind_;
}

const DynamicType* TypeDescriptor::get_base_type() const noexcept
{
    return nullptr == base_type_ ?
           nullptr :
           DynamicTypeBuilderFactory::get_instance().create_copy(*base_type_);
}

void TypeDescriptor::set_base_type(
        const DynamicType& type) noexcept
{
    reset_base_type();
    base_type_ = DynamicTypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::set_base_type(
        const DynamicType* type) noexcept
{
    reset_base_type();
    base_type_ = type;
}

void TypeDescriptor::reset_base_type() noexcept
{
    if (base_type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(base_type_);
    }

    base_type_ = nullptr;
}

const DynamicType* TypeDescriptor::get_discriminator_type() const noexcept
{
    return nullptr == discriminator_type_ ?
           nullptr :
           DynamicTypeBuilderFactory::get_instance().create_copy(*discriminator_type_);
}

void TypeDescriptor::set_discriminator_type(
        const DynamicType& type) noexcept
{
    reset_discriminator_type();
    discriminator_type_ = DynamicTypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::set_discriminator_type(
        const DynamicType* type) noexcept
{
    reset_discriminator_type();
    discriminator_type_ = type;
}

void TypeDescriptor::reset_discriminator_type() noexcept
{
    if (discriminator_type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(discriminator_type_);
    }

    discriminator_type_ = nullptr;
}

const DynamicType* TypeDescriptor::get_element_type() const noexcept
{
    return nullptr == element_type_ ?
           nullptr :
           DynamicTypeBuilderFactory::get_instance().create_copy(*element_type_);
}

void TypeDescriptor::set_element_type(
        const DynamicType& type) noexcept
{
    reset_element_type();
    element_type_ = DynamicTypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::set_element_type(
        const DynamicType* type) noexcept
{
    reset_element_type();
    element_type_ = type;
}

void TypeDescriptor::reset_element_type() noexcept
{
    if (element_type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(element_type_);
    }

    element_type_ = nullptr;
}

const DynamicType* TypeDescriptor::get_key_element_type() const noexcept
{
    return nullptr == key_element_type_ ?
           nullptr :
           DynamicTypeBuilderFactory::get_instance().create_copy(*key_element_type_);
}

void TypeDescriptor::set_key_element_type(
        const DynamicType& type) noexcept
{
    reset_key_element_type();
    key_element_type_ = DynamicTypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::set_key_element_type(
        const DynamicType* type) noexcept
{
    reset_key_element_type();
    key_element_type_ = type;
}

void TypeDescriptor::reset_key_element_type() noexcept
{
    if (key_element_type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(key_element_type_);
    }

    key_element_type_ = nullptr;
}

const uint32_t* TypeDescriptor::get_bounds(
        uint32_t& dims) const noexcept
{
    if ( nullptr == bounds_)
    {
        dims = 0u;
        return nullptr;
    }

    dims = static_cast<uint32_t>(bounds_->size());
    return bounds_->data();
}

void TypeDescriptor::set_bounds(
        const uint32_t* lengths,
        uint32_t dims) noexcept
{
    if ( nullptr == bounds_)
    {
        bounds_ = new(std::nothrow) std::vector<uint32_t>(lengths, lengths + dims);
    }
    else
    {
        bounds_->assign(lengths, lengths + dims);
    }
}

ReturnCode_t TypeDescriptor::copy_from(
        const TypeDescriptor& descriptor) noexcept
{
    *this = descriptor;
    return RETCODE_OK;
}

bool TypeDescriptor::equals(
        const TypeDescriptor& descriptor) const noexcept
{
    return *this == descriptor;
}

bool TypeDescriptor::is_consistent() const noexcept
{
    return TypeState(*this).is_consistent();
}

ReturnCode_t TypeDescriptor::pretty_print(
        char* buffer,
        uint32_t size,
        uint32_t& required) const noexcept
{
    std::ostringstream os;
    os << TypeState(*this);
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
        const TypeDescriptor& md)
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
