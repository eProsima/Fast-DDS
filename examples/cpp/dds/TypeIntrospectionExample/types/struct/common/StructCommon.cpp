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
 * @file StructCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/Struct.hpp"
#include "../gen/StructTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::STRUCT>(
        const unsigned int& index,
        TypeSupport type_support)
{
    Struct_TypeIntrospectionExample* new_data = (Struct_TypeIntrospectionExample*)type_support.create_data();

    // Set index
    new_data->index(index);

    // Set internal data
    InternalStruct_TypeIntrospectionExample internal_data;
    internal_data.x_member(index + 1);
    internal_data.y_member(index * 0.5);
    internal_data.z_member(index * -1);
    new_data->internal_data(internal_data);

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::STRUCT>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    // Set index
    new_data->set_uint32_value(0, index);

    // Set internal data (it requires to loan the struct)
    traits<DynamicData>::ref_type internal_struct = new_data->loan_value(1);

    internal_struct->set_int32_value(0, index + 1);
    internal_struct->set_int32_value(1, index * 0.5);
    internal_struct->set_int32_value(2, index * -1);

    new_data->return_loaned_value(internal_struct);

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::STRUCT>()
{
    register_Struct_type_objects();
}
