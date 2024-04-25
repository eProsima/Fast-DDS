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

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/ComplexSequence.hpp"
#include "../gen/ComplexSequenceTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::COMPLEX_SEQUENCE>(
        const unsigned int& index,
        TypeSupport type_support)
{
    ComplexSequence_TypeIntrospectionExample* new_data = (ComplexSequence_TypeIntrospectionExample*)type_support.create_data();

    /////
    // Set index
    new_data->index(index);

    ///////////////////////////////////////////

    /////
    // Set internal data

    // Set sequence element 0
    InternalSequencePoints_TypeIntrospectionExample internal_data_0;
    internal_data_0.x_member(0 + index);
    internal_data_0.y_member(1 - index);
    internal_data_0.z_member(2 + index * 0.5);

    // Set sequence element 1
    InternalSequencePoints_TypeIntrospectionExample internal_data_1;
    internal_data_1.x_member(3 + index);
    internal_data_1.y_member(4 - index);
    internal_data_1.z_member(5 + index * 0.5);

    // Set sequence element 2
    InternalSequencePoints_TypeIntrospectionExample internal_data_2;
    internal_data_2.x_member(6 + index);
    internal_data_2.y_member(7 - index);
    internal_data_2.z_member(8 + index * 0.5);

    std::vector<InternalSequencePoints_TypeIntrospectionExample> internal_data = {internal_data_0, internal_data_1, internal_data_2};
    new_data->internal_data(internal_data);

    ///////////////////////////////////////////

    /////
    // Set messages

    // Set sequence element 0
    InternalSequenceMessage_TypeIntrospectionExample messages_0;
    messages_0.message("message #" + std::to_string(0 + index));

    // Set sequence element 1
    InternalSequenceMessage_TypeIntrospectionExample messages_1;
    messages_1.message("message #" + std::to_string(1 + index));

    std::vector<InternalSequenceMessage_TypeIntrospectionExample> messages = {messages_0, messages_1};
    new_data->messages(messages);

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::COMPLEX_SEQUENCE>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    // Set index
    new_data->set_uint32_value(0, index);

    ///////////////////////////////////////////

    // Get points sequence
    traits<DynamicData>::ref_type points_substructure = new_data->loan_value(1);

    traits<DynamicData>::ref_type elem;
    for (int i=0; i<3; i++)
    {
        elem = points_substructure->loan_value(i);
        elem->set_int32_value(0, (0 + 3*i) + index);
        elem->set_int32_value(1, (1 + 3*i) - index);
        elem->set_int32_value(2, (2 + 3*i) + index * 0.5);
        points_substructure->return_loaned_value(elem);
    }

    new_data->return_loaned_value(points_substructure);

    ///////////////////////////////////////////

    // Set messages sequence
    traits<DynamicData>::ref_type messages_substructure = new_data->loan_value(2);

    for (int i=0; i<2; i++)
    {
        // Message
        elem = messages_substructure->loan_value(i);
        elem->set_string_value(0, "message #" + std::to_string(i + index));
        messages_substructure->return_loaned_value(elem);
    }

    new_data->return_loaned_value(messages_substructure);

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::COMPLEX_SEQUENCE>()
{
    register_ComplexSequence_type_objects();
}
