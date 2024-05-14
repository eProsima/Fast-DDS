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
 * @file ArrayCode.cpp
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
DataType<DataTypeKind::ARRAY, GeneratorKind::CODE>::generate_type_support_()
{
    // Tmp variable to avoid calling get_instance many times
    auto builder_factory = DynamicTypeBuilderFactory::get_instance();

    /////
    // Array internal data structure
    auto array_builder =
        builder_factory->create_array_type(
            builder_factory->get_primitive_type(TK_INT32),
            {3});

    /////
    // Base structure
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ARRAY_DATA_TYPE_NAME);
    auto builder = builder_factory->create_type(type_descriptor);

    // Index
    MemberDescriptor::_ref_type index_descriptor {traits<MemberDescriptor>::make_shared()};
    index_descriptor->id(0);
    index_descriptor->name("index");
    index_descriptor->type(builder_factory->get_primitive_type(TK_UINT32));
    builder->add_member(index_descriptor);

    // Points
    MemberDescriptor::_ref_type points_descriptor {traits<MemberDescriptor>::make_shared()};
    points_descriptor->id(1);
    points_descriptor->name("points");
    points_descriptor->type(array_builder->build());
    builder->add_member(points_descriptor);

    // Create Dynamic type
    auto dynamic_type = builder->build();

    type_support_.reset(new DynamicPubSubType(dynamic_type));
}
