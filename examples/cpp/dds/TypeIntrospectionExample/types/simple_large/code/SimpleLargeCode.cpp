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
 * @file SimpleLargeCode.cpp
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
DataType<DataTypeKind::SIMPLELARGE, GeneratorKind::CODE>::generate_type_support_()
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
    // Sequence internal data structure
    auto sequence_builder =
        builder_factory->create_sequence_type(
            builder_factory->get_primitive_type(TK_INT16),
            static_cast<uint32_t>(LENGTH_UNLIMITED));

    /////
    // Main Data structure
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(SIMPLELARGE_DATA_TYPE_NAME);
    auto builder = builder_factory->create_type(type_descriptor);

    /////
    // Add values

    // Index
    MemberDescriptor::_ref_type index_descriptor {traits<MemberDescriptor>::make_shared()};
    index_descriptor->id(0);
    index_descriptor->name("index");
    index_descriptor->type(builder_factory->get_primitive_type(TK_UINT32));
    builder->add_member(index_descriptor);

    // Message
    MemberDescriptor::_ref_type message_descriptor {traits<MemberDescriptor>::make_shared()};
    message_descriptor->id(1);
    message_descriptor->name("message");
    message_descriptor->type(builder_factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    builder->add_member(message_descriptor);

    // Points
    MemberDescriptor::_ref_type points_descriptor {traits<MemberDescriptor>::make_shared()};
    points_descriptor->id(2);
    points_descriptor->name("points");
    points_descriptor->type(array_builder->build());
    builder->add_member(points_descriptor);

    // Second message
    MemberDescriptor::_ref_type second_message_descriptor {traits<MemberDescriptor>::make_shared()};
    second_message_descriptor->id(3);
    second_message_descriptor->name("second_message");
    second_message_descriptor->type(builder_factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    builder->add_member(second_message_descriptor);

    // Some values
    MemberDescriptor::_ref_type some_values_descriptor {traits<MemberDescriptor>::make_shared()};
    some_values_descriptor->id(4);
    some_values_descriptor->name("some_values");
    some_values_descriptor->type(sequence_builder->build());
    builder->add_member(some_values_descriptor);

    // is_it_not_true_that_true_is_not_true
    MemberDescriptor::_ref_type bool_descriptor {traits<MemberDescriptor>::make_shared()};
    bool_descriptor->id(5);
    bool_descriptor->name("is_it_not_true_that_true_is_not_true");
    bool_descriptor->type(builder_factory->get_primitive_type(TK_BOOLEAN));
    builder->add_member(bool_descriptor);

    // Create Dynamic type
    auto dynamic_type = builder->build();

    type_support_.reset(new DynamicPubSubType(dynamic_type));
}
