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
 * @file SimpleLargeCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/SimpleLarge.hpp"
#include "../gen/SimpleLargeTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::SIMPLELARGE>(
        const unsigned int& index,
        TypeSupport type_support)
{
    SimpleLarge_TypeIntrospectionExample* new_data = (SimpleLarge_TypeIntrospectionExample*)type_support.create_data();

    // Set index
    new_data->index(index);

    // Set message
    new_data->message("Hello World");

    // Set points
    std::array<int32_t, 3> points = {index + 1, index * 0.5, index * -1};
    new_data->points(points);

    // Set second message
    new_data->second_message(std::to_string(index));

    // Set some values
    std::vector<int16_t> some_values = {index + 1, index * 0.5, index * -1};
    new_data->some_values(some_values);

    // Set is_it_not_true_that_true_is_not_true
    new_data->is_it_not_true_that_true_is_not_true(index % 2 == 0);

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::SIMPLELARGE>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    // Set index
    new_data->set_uint32_value(0, index);
    // Set message
    new_data->set_string_value(1, "Hello World");
    // Set second_message
    new_data->set_string_value(3, std::to_string(index));
    // Set is_it_not_true_that_true_is_not_true
    new_data->set_boolean_value(5, (index % 2 == 0));

    // Set points (it requires to loan the array)
    traits<DynamicData>::ref_type array = new_data->loan_value(2);

    array->set_int32_value(0, index + 1);
    array->set_int32_value(1, index * 0.5);
    array->set_int32_value(2, index * -1);

    new_data->return_loaned_value(array);

    // Set points (it requires to loan the sequence)
    traits<DynamicData>::ref_type sequence = new_data->loan_value(4);

    sequence->set_int16_value(0, index + 1);
    sequence->set_int16_value(1, index * 0.5);
    sequence->set_int16_value(2, index * -1);

    new_data->return_loaned_value(sequence);

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::SIMPLELARGE>()
{
    register_SimpleLarge_type_objects();
}
