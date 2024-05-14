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
 * @file PlainCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/Plain.hpp"
#include "../gen/PlainTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::PLAIN>(
        const unsigned int& index,
        TypeSupport type_support)
{
    Plain_TypeIntrospectionExample* new_data = (Plain_TypeIntrospectionExample*)type_support.create_data();

    // Set index
    new_data->index(index);

    // Set message
    std::array<char, 20> message;
    std::string msg_string = "Hello World " + std::to_string(index % 100000);
    unsigned int i = 0;
    for (const char& c : msg_string)
    {
        message[i++] = c;
    }
    for (;i < 20; i++)
    {
        message[i] = '_';
    }
    new_data->message(message);

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::PLAIN>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    // Set index
    new_data->set_uint32_value(0, index);

    // Set points (it requires to loan the array)
    traits<DynamicData>::ref_type char_array = new_data->loan_value(1);

    std::string msg_string = "Hello World " + std::to_string(index % 100000);

    unsigned int i = 0;
    for (const char& c : msg_string)
    {
        char_array->set_char8_value(i++, c);
    }

    for (;i < 20; i++)
    {
        char_array->set_char8_value(i, '_');
    }

    new_data->return_loaned_value(char_array);

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::PLAIN>()
{
    register_Plain_type_objects();
}
