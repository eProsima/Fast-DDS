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
 * @file SuperComplexCommon.cpp
 *
 */

#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

#include "../../types.hpp"

using namespace eprosima::fastrtps;

template <>
eprosima::fastrtps::types::DynamicData_ptr get_data_by_type<DataTypeKind::SUPER_COMPLEX>(
        const unsigned int& index,
        eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    // Create and initialize new data
    eprosima::fastrtps::types::DynamicData_ptr new_data;
    new_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    ///////////////////////////////////////////
    // INDEX
    ///////////////////////////////////////////
    new_data->set_uint32_value(index, 0);


    ///////////////////////////////////////////
    // MAIN POINT
    ///////////////////////////////////////////
    eprosima::fastrtps::types::DynamicData* main_point = new_data->loan_value(1);
    main_point->set_int32_value(50, 0);
    main_point->set_int32_value(100, 1);
    main_point->set_int32_value(200, 2);

    // timestamp
    eprosima::fastrtps::types::DynamicData* timestamp = main_point->loan_value(3);
    timestamp->set_int32_value(1 + index, 0);
    timestamp->set_int32_value(1000 * (1 + index), 1);
    main_point->return_loaned_value(timestamp);

    new_data->return_loaned_value(main_point);


    ///////////////////////////////////////////
    // INTERNAL DATA
    ///////////////////////////////////////////
    eprosima::fastrtps::types::DynamicData* points_sequence = new_data->loan_value(2);
    eprosima::fastrtps::types::DynamicType_ptr seq_elem_type =
            points_sequence->get_type()->get_descriptor()->get_element_type();
    eprosima::fastrtps::types::DynamicData_ptr seq_elem;
    eprosima::fastrtps::types::MemberId id;

    // Set sequence element 1
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 0);
    seq_elem->set_int32_value(1, 1);
    seq_elem->set_int32_value(2, 2);
    // timestamp
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(1 + index, 0);
    timestamp->set_int32_value(1000 * (1 + index), 1);
    seq_elem->return_loaned_value(timestamp);
    points_sequence->insert_complex_value(seq_elem, id);

    // Set sequence element 2
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(3, 0);
    seq_elem->set_int32_value(4, 1);
    seq_elem->set_int32_value(5, 2);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(2 + index, 0);
    timestamp->set_int32_value(1000 * (2 + index), 1);
    seq_elem->return_loaned_value(timestamp);
    points_sequence->insert_complex_value(seq_elem, id);

    // Set sequence element 3
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(6, 0);
    seq_elem->set_int32_value(7, 1);
    seq_elem->set_int32_value(8, 2);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(3 + index, 0);
    timestamp->set_int32_value(1000 * (3 + index), 1);
    seq_elem->return_loaned_value(timestamp);
    points_sequence->insert_complex_value(seq_elem, id);

    new_data->return_loaned_value(points_sequence);


    ///////////////////////////////////////////
    // INTERNAL DATA BOUNDED
    ///////////////////////////////////////////
    points_sequence = new_data->loan_value(3);
    seq_elem_type = points_sequence->get_type()->get_descriptor()->get_element_type();

    // Set sequence element 1
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 0);
    seq_elem->set_int32_value(1, 1);
    seq_elem->set_int32_value(2, 2);
    // timestamp
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(1 + index, 0);
    timestamp->set_int32_value(1000 * (1 + index), 1);
    seq_elem->return_loaned_value(timestamp);
    points_sequence->insert_complex_value(seq_elem, id);

    // Set sequence element 2
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(3, 0);
    seq_elem->set_int32_value(4, 1);
    seq_elem->set_int32_value(5, 2);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(2 + index, 0);
    timestamp->set_int32_value(1000 * (2 + index), 1);
    seq_elem->return_loaned_value(timestamp);
    points_sequence->insert_complex_value(seq_elem, id);

    // Set sequence element 3
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(6, 0);
    seq_elem->set_int32_value(7, 1);
    seq_elem->set_int32_value(8, 2);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(3 + index, 0);
    timestamp->set_int32_value(1000 * (3 + index), 1);
    seq_elem->return_loaned_value(timestamp);
    points_sequence->insert_complex_value(seq_elem, id);

    new_data->return_loaned_value(points_sequence);


    ///////////////////////////////////////////
    // MESSAGES
    ///////////////////////////////////////////
    eprosima::fastrtps::types::DynamicData* messages_array = new_data->loan_value(4);
    eprosima::fastrtps::types::DynamicData* array_elem;
    eprosima::fastrtps::types::DynamicData* sub_elem;
    for (int i = 0; i < 2; i++)
    {
        array_elem = messages_array->loan_value(i);

        // descriptor
        sub_elem = array_elem->loan_value(0);
        // id
        sub_elem->set_uint32_value(i, 0);
        // topic
        sub_elem->set_string_value("Valuable information", 1);
        // timestamp
        timestamp = sub_elem->loan_value(2);
        timestamp->set_int32_value(index, 0);
        timestamp->set_int32_value(1000 * index, 1);
        sub_elem->return_loaned_value(timestamp);
        array_elem->return_loaned_value(sub_elem);

        // message
        array_elem->set_string_value("message #" + std::to_string(i), 1);
        messages_array->return_loaned_value(array_elem);

        // timestamps array
        sub_elem = array_elem->loan_value(2);
        // timestamps[0]
        timestamp = sub_elem->loan_value(0);
        timestamp->set_int32_value(index, 0);
        timestamp->set_int32_value(1000 * index, 1);
        sub_elem->return_loaned_value(timestamp);
        // timestamps[1]
        timestamp = sub_elem->loan_value(1);
        timestamp->set_int32_value(1 + index, 0);
        timestamp->set_int32_value(1000 * (1 + index), 1);
        sub_elem->return_loaned_value(timestamp);

        array_elem->return_loaned_value(sub_elem);
    }

    new_data->return_loaned_value(messages_array);

    return new_data;
}
