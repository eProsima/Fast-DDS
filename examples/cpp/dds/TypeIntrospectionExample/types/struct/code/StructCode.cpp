// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file StructCode.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>

#include "../../types.hpp"

using namespace eprosima::fastdds::dds;

template <>
void
DataType<DataTypeKind::STRUCT, GeneratorKind::CODE>::generate_type_support_()
{
    // Tmp variable to avoid calling get_instance many times
    auto builder_factory = DynamicTypeBuilderFactory::get_instance();

    /////
    // Internal data structure
    TypeDescriptor::_ref_type internal_data_type_descriptor {traits<TypeDescriptor>::make_shared()};
    internal_data_type_descriptor->kind(TK_STRUCTURE);
    internal_data_type_descriptor->name("InternalData");
    auto internal_data_builder = builder_factory->create_type(internal_data_type_descriptor);

    // x member
    MemberDescriptor::_ref_type x_descriptor {traits<MemberDescriptor>::make_shared()};
    x_descriptor->id(0);
    x_descriptor->name("x_member");
    x_descriptor->type(builder_factory->get_primitive_type(TK_INT32));
    internal_data_builder->add_member(x_descriptor);

    // y member
    MemberDescriptor::_ref_type y_descriptor {traits<MemberDescriptor>::make_shared()};
    y_descriptor->id(1);
    y_descriptor->name("y_member");
    y_descriptor->type(builder_factory->get_primitive_type(TK_INT32));
    internal_data_builder->add_member(y_descriptor);

    // z member
    MemberDescriptor::_ref_type z_descriptor {traits<MemberDescriptor>::make_shared()};
    z_descriptor->id(2);
    z_descriptor->name("z_member");
    z_descriptor->type(builder_factory->get_primitive_type(TK_INT32));
    internal_data_builder->add_member(z_descriptor);

    /////
    // Main Data structure
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(STRUCT_DATA_TYPE_NAME);
    auto builder = builder_factory->create_type(type_descriptor);

    /////
    // Add values

    // Index
    MemberDescriptor::_ref_type index_descriptor {traits<MemberDescriptor>::make_shared()};
    index_descriptor->id(0);
    index_descriptor->name("index");
    index_descriptor->type(builder_factory->get_primitive_type(TK_UINT32));
    builder->add_member(index_descriptor);

    // Internal data
    MemberDescriptor::_ref_type internal_data_member_descriptor {traits<MemberDescriptor>::make_shared()};
    internal_data_member_descriptor->id(1);
    internal_data_member_descriptor->name("internal_data");
    internal_data_member_descriptor->type(internal_data_builder->build());
    builder->add_member(internal_data_member_descriptor);

    // Create Dynamic type
    auto dynamic_type = builder->build();

    type_support_.reset(new DynamicPubSubType(dynamic_type));
}
