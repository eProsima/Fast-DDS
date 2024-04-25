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
 * @file KeyCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/Key.hpp"
#include "../gen/KeyTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::KEY>(
        const unsigned int& index,
        TypeSupport type_support)
{
    Key_TypeIntrospectionExample* new_data = (Key_TypeIntrospectionExample*)type_support.create_data();

    // Set index
    new_data->index(index);

    // Set message and value (depending on instance)
    // There will be 2 instances of the message, one sent in odd indexes and one sent in even indexes.
    if (index % 2 == 0)
    {
        new_data->instance_key("Even_Instance");
        new_data->value(index);
    }
    else
    {
        new_data->instance_key("Odd_Instance");
        new_data->value(-index);
    }

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::KEY>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    // Set index
    new_data->set_uint32_value(1, index);

    // Set message and value (depending on instance)
    // There will be 2 instances of the message, one sent in odd indexes and one sent in even indexes.
    if (index % 2 == 0)
    {
        new_data->set_string_value(0, "Even_Instance");
        new_data->set_int32_value(2, index);
    }
    else
    {
        new_data->set_string_value(0, "Odd_Instance");
        new_data->set_int32_value(2, -index);
    }

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::KEY>()
{
    register_Key_type_objects();
}
