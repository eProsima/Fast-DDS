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
 * @file ComplexArrayCommon.cpp
 *
 */

#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

#include "../../types.hpp"

using namespace eprosima::fastrtps;

template <>
eprosima::fastrtps::types::DynamicData_ptr get_data_by_type<DataTypeKind::COMPLEX_ARRAY>(
        const unsigned int& index,
        eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    // Create and initialize new data
    eprosima::fastrtps::types::DynamicData_ptr new_data;
    new_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    // Set index
    new_data->set_uint32_value(index, 0);

    ///////////////////////////////////////////

    // Get points array
    eprosima::fastrtps::types::DynamicData* array = new_data->loan_value(1);

    // Set array element 1
    eprosima::fastrtps::types::DynamicData* elem = array->loan_value(0);
    elem->set_int32_value(0, 0);
    elem->set_int32_value(1, 1);
    elem->set_int32_value(2, 2);
    array->return_loaned_value(elem);

    // Set array element 2
    elem = array->loan_value(1);
    elem->set_int32_value(3, 0);
    elem->set_int32_value(4, 1);
    elem->set_int32_value(5, 2);
    array->return_loaned_value(elem);

    // Set array element 3
    elem = array->loan_value(2);
    elem->set_int32_value(6, 0);
    elem->set_int32_value(7, 1);
    elem->set_int32_value(8, 2);
    array->return_loaned_value(elem);

    new_data->return_loaned_value(array);

    ///////////////////////////////////////////

    // Set messages
    eprosima::fastrtps::types::DynamicData* messages_substructure = new_data->loan_value(2);

    for (int i = 0; i < 2; i++)
    {
        // Message
        elem = messages_substructure->loan_value(i);
        elem->set_string_value("message #" + std::to_string(i), 0);
        messages_substructure->return_loaned_value(elem);
    }

    new_data->return_loaned_value(messages_substructure);

    return new_data;
}
