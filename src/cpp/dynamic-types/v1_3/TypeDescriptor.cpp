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

#include <fastrtps/types/v1_3/TypeBuilderFactory.hpp>
#include <fastrtps/types/v1_3/TypeDescriptor.hpp>
#include <dynamic-types/v1_3/TypeState.hpp>

#include <algorithm>

using namespace eprosima::fastrtps::types::v1_3;

TypeDescriptor::TypeDescriptor(const TypeDescriptor& type) noexcept
    : name_(type.get_name())
    , kind_(type.get_type())
    , base_type_(type.get_base_type())
    , discriminator_type_(type.get_discriminator_type())
    , element_type_(type.get_element_type())
    , key_element_type_(type.get_key_element_type())
{
    uint32_t dims;
    const uint32_t* lenghts = type.get_bounds(dims);
    set_bounds(lenghts, dims);
}

TypeDescriptor::TypeDescriptor(TypeDescriptor&& type) noexcept
    : name_(type.name_)
    , kind_(type.kind_)
    , base_type_(type.base_type_)
    , discriminator_type_(type.discriminator_type_)
    , bounds_(type.bounds_)
    , bounds_dims_(type.bounds_dims_)
    , element_type_(type.element_type_)
    , key_element_type_(type.key_element_type_)
{
    type.base_type_ = nullptr;
    type.discriminator_ = nullptr;
    type.element_type_ = nullptr;
    type.key_element_type_ = nullptr;
    type.bounds_ = nullptr;
    type.bounds_dims_ = 0;
}

TypeDescriptor::~TypeDescriptor() noexcept
{
    reset_base_type();
    reset_discriminator_type();
    reset_element_type();
    reset_key_element_type();
    set_bounds(nullptr, 0u);
}

TypeDescriptor& TypeDescriptor::operator=(const TypeDescriptor& type) noexcept
{
    name_ = type.get_name();
    kind_ = type.get_type();
    base_type_ = type.get_base_type();
    discriminator_type_ = type.get_discriminator_type();
    element_type_ = type.get_element_type();
    key_element_type_ = type.get_key_element_type();

    uint32_t dims;
    const uint32_t* lenghts = type.get_bounds(dims);
    set_bounds(lenghts, dims);
}

TypeDescriptor& TypeDescriptor::operator=(TypeDescriptor&& type) noexcept
{
    name = type.name_;
    kind_ = type.kind_;
    base_type_ = type.base_type_;
    discriminator_type_ = type.discriminator_type_;
    bounds_ = type.bounds_;
    bounds_dims_ = type.bounds_dims_;
    element_type_ = type.element_type_;
    key_element_type_ = type.key_element_type_;

    type.base_type_ = nullptr;
    type.discriminator_ = nullptr;
    type.element_type_ = nullptr;
    type.key_element_type_ = nullptr;
    type.bounds_ = nullptr;
    type.bounds_dims_ = 0;
}


bool TypeDescriptor::operator==(
        const TypeDescriptor& descriptor) const noexcept
{
    return TypeState(*this) == TypeState(descriptor);
}

bool TypeDescriptor::operator!=(
        const TypeDescriptor& descriptor) const noexcept
{
    return !this->operator==(descriptor);
}

const char* TypeDescriptor::get_name() const noexcept
{
    return name_.c_str();
}

void TypeDescriptor::set_name(
        const char* name) noexcept
{
    name_ = name;
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
        TypeBuilderFactory::get_instance().create_copy(*base_type_);
}

void TypeDescriptor::set_base_type(
        const DynamicType& type) noexcept
{
    reset_base_type();
    base_type_ = TypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::reset_base_type() noexcept
{
    if (base_type_ != nullptr)
    {
        TypeBuilderFactory::get_instance().delete_type(base_type_);
    }

    base_type_ = nullptr;
}

const DynamicType* TypeDescriptor::get_discriminator_type() const noexcept
{
    return nullptr == discriminator_type_ ?
        nullptr :
        TypeBuilderFactory::get_instance().create_copy(*discriminator_type_);
}

void TypeDescriptor::set_discriminator_type(
        const DynamicType& type) noexcept
{
    reset_discriminator_type();
    discriminator_type_ = TypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::reset_discriminator_type() noexcept
{
    if (discriminator_type_ != nullptr)
    {
        TypeBuilderFactory::get_instance().delete_type(discriminator_type_);
    }

    discriminator_type_ = nullptr;
}

const DynamicType* TypeDescriptor::get_element_type() const noexcept
{
    return nullptr == element_type_ ?
        nullptr :
        TypeBuilderFactory::get_instance().create_copy(*element_type_);
}

void TypeDescriptor::set_element_type(
        const DynamicType& type) noexcept
{
    reset_element_type();
    element_type_ = TypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::reset_element_type() noexcept
{
    if (element_type_ != nullptr)
    {
        TypeBuilderFactory::get_instance().delete_type(element_type_);
    }

    element_type_ = nullptr;
}

const DynamicType* TypeDescriptor::get_key_element_type() const noexcept
{
    return nullptr == key_element_type_ ?
        nullptr :
        TypeBuilderFactory::get_instance().create_copy(*key_element_type_);
}

void TypeDescriptor::set_key_element_type(
        const DynamicType& type) noexcept
{
    reset_key_element_type();
    key_element_type_ = TypeBuilderFactory::get_instance().create_copy(type);
}

void TypeDescriptor::reset_key_element_type() noexcept
{
    if (key_element_type_ != nullptr)
    {
        TypeBuilderFactory::get_instance().delete_type(key_element_type_);
    }

    key_element_type_ = nullptr;
}

const uint32_t* TypeDescriptor::get_bounds(
        uint32_t& dims) const noexcept
{
    dims = bounds_dims_;
    return bounds_;
}

void TypeDescriptor::set_bounds(
        const uint32_t* lengths,
        uint32_t dims) noexcept
{
   if (bounds_ != nullptr)
   {
        delete [] bounds_;
        bounds_ = nullptr;
        bounds_dims_ = 0u;
   }

    if (dims)
    {
        if (bounds_ = new (std::nothrow) uint32_t[](dims))
        {
            std::copy(lengths, lengths + dims, bounds_);
            bounds_dims_ = dims;
        }
    }
}

ReturnCode_t TypeDescriptor::copy_from(
        const TypeDescriptor& descriptor) noexcept
{
    *this = descriptor;
    return ReturnCode_t::RETCODE_OK;
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
