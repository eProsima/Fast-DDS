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
 * @file ArrayCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/Array.hpp"
#include "../gen/ArrayTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::ARRAY>(
        const unsigned int& index,
        TypeSupport type_support)
{
    Array_TypeIntrospectionExample* new_data = (Array_TypeIntrospectionExample*)type_support.create_data();

    // Set index
    new_data->index(index);

    // Set points
    std::array<int32_t, 3> points = {index + 1, index * 0.5, index * -1};
    new_data->points(points);

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::ARRAY>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    // Set index
    new_data->set_uint32_value(0, index);

    // Set points (it requires to loan the array)
    traits<DynamicData>::ref_type array = new_data->loan_value(1);

    array->set_int32_value(0, index + 1);
    array->set_int32_value(1, index * 0.5);
    array->set_int32_value(2, index * -1);

    new_data->return_loaned_value(array);

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::ARRAY>()
{
    register_Array_type_objects();
}
