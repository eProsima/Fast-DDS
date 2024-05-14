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
 * @file DataTestCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/DataTest.hpp"
#include "../gen/DataTestTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

void fill_primitives_struct(
        PrimitivesStruct_TypeIntrospectionExample* data,
        const unsigned int& index)
{
    index % 2 ? data->my_bool(true) : data->my_bool(false);
    data->my_octet(static_cast<uint8_t>(index));
    index % 2 ? data->my_char('e') : data->my_char('o');
    index % 2 ? data->my_wchar(L'e') : data->my_wchar(L'o');
    data->my_long(static_cast<int32_t>(index));
    data->my_ulong(static_cast<uint32_t>(index));
    data->my_int8(static_cast<int8_t>(index));
    data->my_uint8(static_cast<uint8_t>(index));
    data->my_short(static_cast<int16_t>(index));
    data->my_ushort(static_cast<uint16_t>(index));
    data->my_longlong(static_cast<int64_t>(index));
    data->my_ulonglong(static_cast<uint64_t>(index));
    data->my_float(static_cast<float>(0.5*index));
    data->my_double(static_cast<double>(0.5*index));
    data->my_longdouble(static_cast<long double>(0.5*index));
}

void fill_all_struct(
        AllStruct_TypeIntrospectionExample* data,
        const unsigned int& index)
{
    // primitives
    fill_primitives_struct(data, index);

    // strings
    data->my_string("my_string");
    data->my_wstring(L"my_wstring");
    data->my_bounded_string("my_bounded_string");
    data->my_bounded_wstring(L"my_bounded_wstring"); // are bounded wstring actually supported?

    // enum
    switch (index % 3)
    {
        case 0:
            data->my_enum(MyEnum::A);
            break;
        case 1:
            data->my_enum(MyEnum::B);
            break;
        case 2:
            data->my_enum(MyEnum::C);
            break;
    }

    // bitmask
    data->my_bitmask(index % 255);

    // alias
    data->my_aliased_struct(static_cast<PrimitivesStruct_TypeIntrospectionExample>(*data));
    data->my_aliased_enum(data->my_enum());
    data->my_aliased_bounded_string("my_aliased_bounded_string");
    data->my_recursive_alias(data->my_enum());

    // sequence
    for (unsigned int i = index; i < (index + 5); i++)
    {
        data->short_sequence().push_back(static_cast<int16_t>(i));
    }

    // array
    for (unsigned int i = 0; i < 2; i++)
    {
        for (unsigned int j = 0; j < 3; j++)
        {
            for (unsigned int k = 0; k < 4; k++)
            {
                data->long_array()[i][j][k] = static_cast<int32_t>(index + i + j + k);
            }
        }
    }

    // map
    for (unsigned int i = 0; i < 5; i++)
    {
        data->string_unbounded_map()[std::to_string(i+2)] = std::to_string(i + index);
    }
    for (unsigned int i = 0; i < 5; i++)
    {
        data->string_alias_unbounded_map()[std::to_string(i+2)] = std::to_string(i + index);
    }
    for (unsigned int i = 0; i < 2; i++)
    {
        data->short_long_map()[static_cast<int16_t>(i+2)] = static_cast<int32_t>(i + index);
    }

    // inner union
    InnerUnion inner_union;
    if (index % 2)
    {
        inner_union.first(static_cast<PrimitivesStruct_TypeIntrospectionExample>(*data));
    }
    else
    {
        inner_union.second(static_cast<int64_t>(index));
    }
    data->inner_union(inner_union);

    // complex union
    ComplexUnion complex_union;
    if (index % 2)
    {
        complex_union.fourth(inner_union);
    }
    else
    {
        complex_union.third(static_cast<int32_t>(index));
    }
    data->complex_union(complex_union);
}

void fill_data(
        DataTest_TypeIntrospectionExample* data,
        const unsigned int& index)
{
    AllStruct_TypeIntrospectionExample all_struct;
    fill_all_struct(&all_struct, index);

    // index
    data->index(index);

    // inner struct
    data->inner_struct(all_struct);

    // complex sequence
    for (unsigned int i = 0; i < 3; i++)
    {
        data->complex_sequence().push_back(all_struct);
    }

    // complex array
    for (unsigned int i = 0; i < 2; i++)
    {
        data->complex_array()[i] = all_struct;
    }

    // complex map
    for (unsigned int i = 0; i < 2; i++)
    {
        data->complex_map()[static_cast<int16_t>(i)] = all_struct;
    }
}

template <>
void* get_data_by_type_support<DataTypeKind::DATA_TEST>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DataTest_TypeIntrospectionExample* new_data = (DataTest_TypeIntrospectionExample*)type_support.create_data();
    // AllStruct_TypeIntrospectionExample* new_data = (AllStruct_TypeIntrospectionExample*)type_support.create_data();

    fill_data(new_data, index);
    // fill_all_struct(new_data, index);
    // TODO: also test empty

    return new_data;
}

template <>
void* get_dynamic_data_by_type_support<DataTypeKind::DATA_TEST>(
        const unsigned int& index,
        TypeSupport type_support)
{
    DynamicData::_ref_type* new_data_ptr = reinterpret_cast<DynamicData::_ref_type*>(type_support.create_data());

    DynamicData::_ref_type new_data = *new_data_ptr;

    ///////////////////////////////////////////
    // INDEX
    ///////////////////////////////////////////
    new_data->set_uint32_value(0, index);

    // TODO

    return new_data_ptr;
}

template <>
void register_type_object_representation_gen<DataTypeKind::DATA_TEST>()
{
    register_DataTest_type_objects();
}
