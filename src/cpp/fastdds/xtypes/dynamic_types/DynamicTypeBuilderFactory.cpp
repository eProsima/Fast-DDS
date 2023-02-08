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

#include <fastdds/dds/xtypes/dynamic_types//DynamicTypeBuilderFactory.hpp>
#include "DynamicTypeImpl.hpp"
#include "DynamicTypeBuilderImpl.hpp"
#include "DynamicTypeBuilderFactoryImpl.hpp"
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

DynamicTypeBuilderFactory& DynamicTypeBuilderFactory::get_instance() noexcept
{
    // C++11 guarantees the construction to be atomic
    static DynamicTypeBuilderFactory instance;
    return instance;
}

ReturnCode_t DynamicTypeBuilderFactory::delete_instance() noexcept
{
    // Delegate into the implementation class
    return DynamicTypeBuilderFactoryImpl::delete_instance();
}

const DynamicType* DynamicTypeBuilderFactory::get_primitive_type(
        TypeKind kind) noexcept
{
    // Delegate into the implementation class
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().get_primitive_type(kind);
    return &type->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_primitive_type(
        TypeKind kind) noexcept
{
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().create_primitive_type(kind);
    return &type->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_type(
        const TypeDescriptor& td) noexcept
{
    // Delegate into the implementation class
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_type(td);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_type_copy(
        const DynamicType& type) noexcept
{
    const auto& ti = DynamicTypeImpl::get_implementation(type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_type_copy(ti);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_type_copy(
        const DynamicTypeBuilder& build) noexcept
{
    const auto& ti = DynamicTypeBuilderImpl::get_implementation(build);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_type_copy(ti);
    return &builder->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::create_copy(
        const DynamicType& type) noexcept
{
    const auto& ti = DynamicTypeImpl::get_implementation(type);
    const auto& cp = DynamicTypeBuilderFactoryImpl::get_instance().create_copy(ti);
    return &cp.get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_string_type(
        uint32_t bound /*= LENGTH_UNLIMITED*/) noexcept
{
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_string_type(bound);
    return &builder->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_wstring_type(
        uint32_t bound /*= LENGTH_UNLIMITED*/) noexcept
{
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_wstring_type(bound);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_sequence_type(
        const DynamicType& type,
        uint32_t bound /*= LENGTH_UNLIMITED*/) noexcept
{
    const auto& impl = DynamicTypeImpl::get_implementation(type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_sequence_type(impl, bound);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_array_type(
        const DynamicType& type,
        const uint32_t* bounds,
        uint32_t count) noexcept
{
    const auto& impl = DynamicTypeImpl::get_implementation(type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_array_type(impl, {bounds, bounds + count});
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_map_type(
        const DynamicType& key_type,
        const DynamicType& value_type,
        uint32_t bound /*= LENGTH_UNLIMITED*/) noexcept
{
    const auto& key_impl = DynamicTypeImpl::get_implementation(key_type);
    const auto& value_impl = DynamicTypeImpl::get_implementation(value_type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_map_type(key_impl, value_impl, bound);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bitmask_type(
        uint32_t bound /*= 32*/) noexcept
{
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_bitmask_type(bound);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bitset_type(
        uint32_t bound /*= 32*/) noexcept
{
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_bitset_type(bound);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_alias_type(
        const DynamicType& base_type,
        const char* sName) noexcept
{
    const auto& impl = DynamicTypeImpl::get_implementation(base_type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_alias_type(impl, std::string(sName));
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_enum_type() noexcept
{
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_enum_type();
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_struct_type() noexcept
{
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_struct_type();
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_child_struct_type(
        const DynamicType& parent_type) noexcept
{
    const auto& impl = DynamicTypeImpl::get_implementation(parent_type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_child_struct_type(impl);
    return &builder->get_interface();
}

DynamicTypeBuilder* DynamicTypeBuilderFactory::create_union_type(
        const DynamicType& discriminator_type) noexcept
{
    const auto& impl = DynamicTypeImpl::get_implementation(discriminator_type);
    auto builder = DynamicTypeBuilderFactoryImpl::get_instance().create_union_type(impl);
    return &builder->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::create_annotation_primitive(
        const char* name) noexcept
{
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().create_annotation_primitive(std::string(name));
    return &type->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_alias_type(
        const DynamicType& base_type,
        const char* sName) noexcept
{
    const auto& impl = DynamicTypeImpl::get_implementation(base_type);
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().get_alias_type(impl, std::string(sName));
    return &type->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_string_type(
        uint32_t bound /*= LENGTH_UNLIMITED*/) noexcept
{
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().get_string_type(bound);
    return &type->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_wstring_type(
        uint32_t bound /*= LENGTH_UNLIMITED*/) noexcept
{
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().get_wstring_type(bound);
    return &type->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_bitset_type(
        uint32_t bound) noexcept
{
    auto type = DynamicTypeBuilderFactoryImpl::get_instance().get_bitset_type(bound);
    return &type->get_interface();
}

ReturnCode_t DynamicTypeBuilderFactory::delete_type(
        const DynamicType* type) noexcept
{
    if (nullptr == type)
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    const auto& ti = DynamicTypeImpl::get_implementation(*type);
    return DynamicTypeBuilderFactoryImpl::get_instance().delete_type(ti);
}

ReturnCode_t DynamicTypeBuilderFactory::delete_type(
        const DynamicTypeBuilder* type) noexcept
{
    if (nullptr == type)
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    const auto& ti = DynamicTypeBuilderImpl::get_implementation(*type);
    return DynamicTypeBuilderFactoryImpl::get_instance().delete_type(ti);
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int32_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_int32_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint32_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_uint32_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int16_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_int16_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint16_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_uint16_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_int64_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_int64_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_uint64_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_uint64_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float32_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_float32_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float64_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_float64_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_float128_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_float128_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_char8_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_char8_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_char16_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_char16_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_bool_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_bool_type();
    return &ti->get_interface();
}

const DynamicTypeBuilder* DynamicTypeBuilderFactory::create_byte_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().create_byte_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_int16_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_int16_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_uint16_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_uint16_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_int32_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_int32_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_uint32_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_uint32_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_int64_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_int64_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_uint64_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_uint64_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_float32_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_float32_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_float64_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_float64_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_float128_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_float128_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_char8_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_char8_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_char16_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_char16_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_bool_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_bool_type();
    return &ti->get_interface();
}

const DynamicType* DynamicTypeBuilderFactory::get_byte_type() noexcept
{
    const auto ti = DynamicTypeBuilderFactoryImpl::get_instance().get_byte_type();
    return &ti->get_interface();
}

void DynamicTypeBuilderFactory::build_type_identifier(
        const DynamicTypeBuilder& bld,
        fastrtps::types::TypeIdentifier& identifier,
        bool complete /*= true*/) const noexcept
{
    const auto& builder = DynamicTypeBuilderImpl::get_implementation(bld);
    return DynamicTypeBuilderFactoryImpl::get_instance()
                   .build_type_identifier(builder, identifier, complete);
}

void DynamicTypeBuilderFactory::build_type_object(
        const DynamicTypeBuilder& bld,
        fastrtps::types::TypeObject& object,
        bool complete /*= true*/,
        bool force /*= false*/) const noexcept
{
    const auto& builder = DynamicTypeBuilderImpl::get_implementation(bld);
    return DynamicTypeBuilderFactoryImpl::get_instance()
                   .build_type_object(builder, object, complete, force);
}

void DynamicTypeBuilderFactory::build_type_identifier(
        const DynamicType& tp,
        fastrtps::types::TypeIdentifier& identifier,
        bool complete /*= true*/) const noexcept
{
    const auto& type = DynamicTypeImpl::get_implementation(tp);
    return DynamicTypeBuilderFactoryImpl::get_instance()
                   .build_type_identifier(type, identifier, complete);
}

void DynamicTypeBuilderFactory::build_type_object(
        const DynamicType& tp,
        fastrtps::types::TypeObject& object,
        bool complete /*= true*/,
        bool force /*= false*/) const noexcept
{
    const auto& type = DynamicTypeImpl::get_implementation(tp);
    return DynamicTypeBuilderFactoryImpl::get_instance()
                   .build_type_object(type, object, complete, force);
}

bool DynamicTypeBuilderFactory::is_empty() const noexcept
{
    return DynamicTypeBuilderFactoryImpl::get_instance().is_empty();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
