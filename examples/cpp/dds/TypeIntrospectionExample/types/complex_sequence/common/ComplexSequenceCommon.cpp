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
 * @file ComplexSequenceCommon.cpp
 *
 */

#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

#include "../../types.hpp"

using namespace eprosima::fastrtps;

template <>
eprosima::fastrtps::types::DynamicData_ptr get_data_by_type<DataTypeKind::COMPLEX_SEQUENCE>(
        const unsigned int& index,
        eprosima::fastrtps::types::DynamicType_ptr dyn_type)
{
    // Create and initialize new data
    eprosima::fastrtps::types::DynamicData_ptr new_data;
    new_data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    // Set index
    new_data->set_uint32_value(index, 0);

    ///////////////////////////////////////////

    // Get points sequence
    eprosima::fastrtps::types::DynamicData* sequence = new_data->loan_value(1);
    eprosima::fastrtps::types::DynamicType_ptr seq_elem_type =
            sequence->get_type()->get_descriptor()->get_element_type();

    // Set sequence element 1
    eprosima::fastrtps::types::DynamicData_ptr seq_elem;
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 0);
    seq_elem->set_int32_value(1, 1);
    seq_elem->set_int32_value(2, 2);
    eprosima::fastrtps::types::MemberId id0;
    sequence->insert_complex_value(seq_elem, id0);

    // Set sequence element 2
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(3, 0);
    seq_elem->set_int32_value(4, 1);
    seq_elem->set_int32_value(5, 2);
    eprosima::fastrtps::types::MemberId id1;
    sequence->insert_complex_value(seq_elem, id1);

    // Set sequence element 3
    seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(6, 0);
    seq_elem->set_int32_value(7, 1);
    seq_elem->set_int32_value(8, 2);
    eprosima::fastrtps::types::MemberId id2;
    sequence->insert_complex_value(seq_elem, id2);

    new_data->return_loaned_value(sequence);

    ///////////////////////////////////////////

    // Set messages
    sequence = new_data->loan_value(2);
    seq_elem_type = sequence->get_type()->get_descriptor()->get_element_type();
    eprosima::fastrtps::types::MemberId id;
    for (int i = 0; i < 2; i++)
    {
        // Message
        seq_elem = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(seq_elem_type);
        seq_elem->set_string_value("message #" + std::to_string(i), 0);
        sequence->insert_complex_value(seq_elem, id);
    }

    new_data->return_loaned_value(sequence);

    return new_data;
}
