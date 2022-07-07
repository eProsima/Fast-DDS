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
 * @file ComplexCode.h
 *
 */

#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

#include "../../types.hpp"

using namespace eprosima::fastrtps;

template <>
eprosima::fastrtps::types::DynamicData_ptr get_data_by_type<DataTypeKind::COMPLEX>(
        const unsigned int& index,
        eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    // Create and initialize new data
    eprosima::fastrtps::types::DynamicData_ptr new_data;
    new_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    // Set index
    new_data->set_uint32_value(index, 0);

    // Set points (it requires to loan the array)
    eprosima::fastrtps::types::DynamicData* points_substructure = new_data->loan_value(1);

    points_substructure->set_int32_value(index + 1, 0);
    points_substructure->set_int32_value(index * 0.5, 1);
    points_substructure->set_int32_value(index * -1, 2);

    new_data->return_loaned_value(points_substructure);

    // Set messages
    eprosima::fastrtps::types::DynamicData* messages_substructure = new_data->loan_value(2);

    for (int i=0; i<3; i++)
    {
        // Message
        eprosima::fastrtps::types::DynamicData* message;
        messages_substructure->get_complex_value(&message, i);
        message->set_string_value("message #" + std::to_string(i), 0);

        // Set complex type
        messages_substructure->set_complex_value(message, i);
    }

    new_data->return_loaned_value(messages_substructure);

    return new_data;
}
