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

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/SuperComplex.hpp"
#include "../gen/SuperComplexTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void* get_data_by_type_support<DataTypeKind::SUPER_COMPLEX>(
        const unsigned int& index,
        TypeSupport type_support)
{
    SuperComplex_TypeIntrospectionExample* new_data = (SuperComplex_TypeIntrospectionExample*)type_support.create_data();

    ///////////////////////////////////////////
    // INDEX
    ///////////////////////////////////////////
    new_data->index(index);


    ///////////////////////////////////////////
    // MAIN POINT
    ///////////////////////////////////////////
    InternalPoints_TypeIntrospectionExample main_point;
    main_point.x_member(50);
    main_point.y_member(100);
    main_point.z_member(200);

    // timestamp
    InternalTimestamp_TypeIntrospectionExample timestamp;
    timestamp.seconds(1 + index);
    timestamp.milliseconds(1000 * (1 + index));
    main_point.t_member(timestamp);

    new_data->main_point(main_point);


    ///////////////////////////////////////////
    // INTERNAL DATA
    ///////////////////////////////////////////
    std::vector<InternalPoints_TypeIntrospectionExample> internal_data;
    InternalPoints_TypeIntrospectionExample seq_elem;

    // Set sequence element 0
    seq_elem.x_member(0);
    seq_elem.y_member(1);
    seq_elem.z_member(2);
    // timestamp
    timestamp.seconds(1 + index);
    timestamp.milliseconds(1000 * (1 + index));
    seq_elem.t_member(timestamp);
    // Add to sequence
    internal_data.push_back(seq_elem);

    // Set sequence element 1
    seq_elem.x_member(3);
    seq_elem.y_member(4);
    seq_elem.z_member(5);
    // timestamp
    timestamp.seconds(2 + index);
    timestamp.milliseconds(1000 * (2 + index));
    seq_elem.t_member(timestamp);
    // Add to sequence
    internal_data.push_back(seq_elem);

    // Set sequence element 2
    seq_elem.x_member(6);
    seq_elem.y_member(7);
    seq_elem.z_member(8);
    // timestamp
    timestamp.seconds(3 + index);
    timestamp.milliseconds(1000 * (3 + index));
    seq_elem.t_member(timestamp);
    // Add to sequence
    internal_data.push_back(seq_elem);

    new_data->internal_data(internal_data);


    ///////////////////////////////////////////
    // INTERNAL DATA BOUNDED
    ///////////////////////////////////////////
    std::vector<InternalPoints_TypeIntrospectionExample> internal_data_bounded;

    // Set sequence element 0
    seq_elem.x_member(0);
    seq_elem.y_member(1);
    seq_elem.z_member(2);
    // timestamp
    timestamp.seconds(1 + index);
    timestamp.milliseconds(1000 * (1 + index));
    seq_elem.t_member(timestamp);
    // Add to sequence
    internal_data_bounded.push_back(seq_elem);

    // Set sequence element 1
    seq_elem.x_member(3);
    seq_elem.y_member(4);
    seq_elem.z_member(5);
    // timestamp
    timestamp.seconds(2 + index);
    timestamp.milliseconds(1000 * (2 + index));
    seq_elem.t_member(timestamp);
    // Add to sequence
    internal_data_bounded.push_back(seq_elem);

    // Set sequence element 2
    seq_elem.x_member(6);
    seq_elem.y_member(7);
    seq_elem.z_member(8);
    // timestamp
    timestamp.seconds(3 + index);
    timestamp.milliseconds(1000 * (3 + index));
    seq_elem.t_member(timestamp);
    // Add to sequence
    internal_data_bounded.push_back(seq_elem);

    new_data->internal_data_bounded(internal_data_bounded);


    ///////////////////////////////////////////
    // MESSAGES
    ///////////////////////////////////////////
    std::array<InternalMessage_TypeIntrospectionExample, 2> messages;

    for (int i=0; i<2; i++)
    {
        InternalMessage_TypeIntrospectionExample array_elem;

        ////////////////
        // descriptor //
        ////////////////
        InternalMsgDescriptor_TypeIntrospectionExample descriptor;
        // id
        descriptor.id(i);
        // topic
        descriptor.topic("Valuable information");
        // timestamp
        timestamp.seconds(index);
        timestamp.milliseconds(1000 * index);
        descriptor.timestamp(timestamp);

        array_elem.descriptor(descriptor);

        /////////////
        // message //
        /////////////
        array_elem.message("message #" + std::to_string(i));

        //////////////////////
        // timestamps array //
        //////////////////////
        // timestamps[0]
        InternalTimestamp_TypeIntrospectionExample timestamp_0;
        timestamp_0.seconds(index);
        timestamp_0.milliseconds(1000 * index);
        // timestamps[1]
        InternalTimestamp_TypeIntrospectionExample timestamp_1;
        timestamp_1.seconds(1 + index);
        timestamp_1.milliseconds(1000 * (1 + index));

        array_elem.timestamps({timestamp_0, timestamp_1});

        messages[i] = array_elem;
    }
    new_data->messages(messages);

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::SUPER_COMPLEX>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    ///////////////////////////////////////////
    // INDEX
    ///////////////////////////////////////////
    new_data->set_uint32_value(0, index);


    ///////////////////////////////////////////
    // MAIN POINT
    ///////////////////////////////////////////
    traits<DynamicData>::ref_type main_point = new_data->loan_value(1);
    main_point->set_int32_value(0, 50);
    main_point->set_int32_value(1, 100);
    main_point->set_int32_value(2, 200);

    // timestamp
    traits<DynamicData>::ref_type timestamp = main_point->loan_value(3);
    timestamp->set_int32_value(0, 1 + index);
    timestamp->set_int32_value(1, 1000 * (1 + index));
    main_point->return_loaned_value(timestamp);

    new_data->return_loaned_value(main_point);


    ///////////////////////////////////////////
    // INTERNAL DATA
    ///////////////////////////////////////////
    traits<DynamicData>::ref_type points_sequence = new_data->loan_value(2);
    TypeDescriptor::_ref_type seq_descriptor {traits<TypeDescriptor>::make_shared()};
    points_sequence->type()->get_descriptor(seq_descriptor);
    traits<DynamicType>::ref_type seq_elem_type = seq_descriptor->element_type();
    traits<DynamicData>::ref_type seq_elem;

    // Set sequence element 0
    seq_elem = DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 0);
    seq_elem->set_int32_value(1, 1);
    seq_elem->set_int32_value(2, 2);
    // timestamp
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(0, 1 + index);
    timestamp->set_int32_value(1, 1000 * (1 + index));
    seq_elem->return_loaned_value(timestamp);
    points_sequence->set_complex_value(0, seq_elem);

    // Set sequence element 1
    seq_elem = DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 3);
    seq_elem->set_int32_value(1, 4);
    seq_elem->set_int32_value(2, 5);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(0, 2 + index);
    timestamp->set_int32_value(1, 1000 * (2 + index));
    seq_elem->return_loaned_value(timestamp);
    points_sequence->set_complex_value(1, seq_elem);

    // Set sequence element 2
    seq_elem = DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 6);
    seq_elem->set_int32_value(1, 7);
    seq_elem->set_int32_value(2, 8);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(0, 3 + index);
    timestamp->set_int32_value(1, 1000 * (3 + index));
    seq_elem->return_loaned_value(timestamp);
    points_sequence->set_complex_value(2, seq_elem);

    new_data->return_loaned_value(points_sequence);


    ///////////////////////////////////////////
    // INTERNAL DATA BOUNDED
    ///////////////////////////////////////////
    points_sequence = new_data->loan_value(3);
    points_sequence->type()->get_descriptor(seq_descriptor);
    seq_elem_type = seq_descriptor->element_type();

    // Set sequence element 0
    seq_elem = DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 0);
    seq_elem->set_int32_value(1, 1);
    seq_elem->set_int32_value(2, 2);
    // timestamp
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(0, 1 + index);
    timestamp->set_int32_value(1, 1000 * (1 + index));
    seq_elem->return_loaned_value(timestamp);
    points_sequence->set_complex_value(0, seq_elem);

    // Set sequence element 1
    seq_elem = DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 3);
    seq_elem->set_int32_value(1, 4);
    seq_elem->set_int32_value(2, 5);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(0, 2 + index);
    timestamp->set_int32_value(1, 1000 * (2 + index));
    seq_elem->return_loaned_value(timestamp);
    points_sequence->set_complex_value(1, seq_elem);

    // Set sequence element 2
    seq_elem = DynamicDataFactory::get_instance()->create_data(seq_elem_type);
    seq_elem->set_int32_value(0, 6);
    seq_elem->set_int32_value(1, 7);
    seq_elem->set_int32_value(2, 8);
    timestamp = seq_elem->loan_value(3);
    timestamp->set_int32_value(0, 3 + index);
    timestamp->set_int32_value(1, 1000 * (3 + index));
    seq_elem->return_loaned_value(timestamp);
    points_sequence->set_complex_value(2, seq_elem);

    new_data->return_loaned_value(points_sequence);


    /////////////////////////////////////////
    // MESSAGES
    /////////////////////////////////////////
    traits<DynamicData>::ref_type messages_array = new_data->loan_value(4);
    traits<DynamicData>::ref_type array_elem;
    traits<DynamicData>::ref_type sub_elem;
    for (int i=0; i<2; i++)
    {
        array_elem = messages_array->loan_value(i);

        // descriptor
        sub_elem = array_elem->loan_value(0);
        // id
        sub_elem->set_uint32_value(0, i);
        // topic
        sub_elem->set_string_value(1, "Valuable information");
        // timestamp
        timestamp = sub_elem->loan_value(2);
        timestamp->set_int32_value(0, index);
        timestamp->set_int32_value(1, 1000 * index);
        sub_elem->return_loaned_value(timestamp);
        array_elem->return_loaned_value(sub_elem);

        // message
        array_elem->set_string_value(1, "message #" + std::to_string(i));
        messages_array->return_loaned_value(array_elem);

        // timestamps array
        sub_elem = array_elem->loan_value(2);
        // timestamps[0]
        timestamp = sub_elem->loan_value(0);
        timestamp->set_int32_value(0, index);
        timestamp->set_int32_value(1, 1000 * index);
        sub_elem->return_loaned_value(timestamp);
        // timestamps[1]
        timestamp = sub_elem->loan_value(1);
        timestamp->set_int32_value(0, 1 + index);
        timestamp->set_int32_value(1, 1000 * (1 + index));
        sub_elem->return_loaned_value(timestamp);

        array_elem->return_loaned_value(sub_elem);
    }

    new_data->return_loaned_value(messages_array);

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::SUPER_COMPLEX>()
{
    register_SuperComplex_type_objects();
}
