// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ComprehensiveTypeImpl.cpp
 *
 */

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "../../types.hpp"
#include "../gen/ComprehensiveType.hpp"
#include "../gen/ComprehensiveTypePubSubTypes.hpp"
#include "../gen/ComprehensiveTypeTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

void fill_primitives_struct(
        traits<DynamicData>::ref_type dyn_data,
        const unsigned int& index)
{
    index % 2 ? dyn_data->set_boolean_value(dyn_data->get_member_id_by_name("my_bool"), true) :
    dyn_data->set_boolean_value(dyn_data->get_member_id_by_name("my_bool"), false);
    dyn_data->set_uint8_value(dyn_data->get_member_id_by_name("my_octet"), static_cast<uint8_t>(index));
    index % 2 ? dyn_data->set_char8_value(dyn_data->get_member_id_by_name("my_char"), static_cast<char>('e')):
    dyn_data->set_char8_value(dyn_data->get_member_id_by_name("my_char"), static_cast<char>('o'));
    index % 2 ? dyn_data->set_char16_value(dyn_data->get_member_id_by_name("my_wchar"), static_cast<wchar_t>(L'e')):
    dyn_data->set_char16_value(dyn_data->get_member_id_by_name("my_wchar"), static_cast<wchar_t>(L'o'));
    dyn_data->set_int32_value(dyn_data->get_member_id_by_name("my_long"), static_cast<int32_t>(index));
    dyn_data->set_uint32_value(dyn_data->get_member_id_by_name("my_ulong"), static_cast<uint32_t>(index));
    dyn_data->set_int8_value(dyn_data->get_member_id_by_name("my_int8"), static_cast<int8_t>(index));
    dyn_data->set_uint8_value(dyn_data->get_member_id_by_name("my_uint8"), static_cast<uint8_t>(index));
    dyn_data->set_int16_value(dyn_data->get_member_id_by_name("my_short"), static_cast<int16_t>(index));
    dyn_data->set_uint16_value(dyn_data->get_member_id_by_name("my_ushort"), static_cast<uint16_t>(index));
    dyn_data->set_int64_value(dyn_data->get_member_id_by_name("my_longlong"), static_cast<int64_t>(index));
    dyn_data->set_uint64_value(dyn_data->get_member_id_by_name("my_ulonglong"), static_cast<uint64_t>(index));
    dyn_data->set_float32_value(dyn_data->get_member_id_by_name("my_float"), static_cast<float>(0.5 * index));
    dyn_data->set_float64_value(dyn_data->get_member_id_by_name("my_double"), static_cast<double>(0.5 * index));
    dyn_data->set_float128_value(dyn_data->get_member_id_by_name("my_longdouble"),
            static_cast<long double>(0.5 * index));
}

void fill_my_enum(
        traits<DynamicData>::ref_type dyn_data,
        const std::string& member_name,
        const unsigned int& index)
{
    switch (index % 3)
    {
        case 0:
            dyn_data->set_int32_value(dyn_data->get_member_id_by_name(member_name), 0);
            break;
        case 1:
            dyn_data->set_int32_value(dyn_data->get_member_id_by_name(member_name), 1);
            break;
        case 2:
            dyn_data->set_int32_value(dyn_data->get_member_id_by_name(member_name), 2);
            break;
    }
}

void fill_all_struct(
        traits<DynamicData>::ref_type dyn_data,
        const unsigned int& index)
{
    // primitives
    fill_primitives_struct(dyn_data, index);

    // strings
    dyn_data->set_string_value(dyn_data->get_member_id_by_name("my_string"), "my_string");
    dyn_data->set_wstring_value(dyn_data->get_member_id_by_name("my_wstring"), L"my_string");
    dyn_data->set_string_value(dyn_data->get_member_id_by_name("my_bounded_string"), "my_bounded_string");
    dyn_data->set_wstring_value(dyn_data->get_member_id_by_name("my_bounded_wstring"), L"my_bounded_wstring");

    // enum
    fill_my_enum(dyn_data, "my_enum", index);

    // bitmask
    traits<DynamicData>::ref_type dyn_data_my_bitmask =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("my_bitmask"));
    index % 2 ? dyn_data_my_bitmask->set_boolean_value(dyn_data_my_bitmask->get_member_id_by_name("flag0"), true) :
    dyn_data_my_bitmask->set_boolean_value(dyn_data_my_bitmask->get_member_id_by_name("flag0"), false);
    traits<DynamicData>::ref_type dyn_data_bitmask_sequence =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("bitmask_sequence"));
    dyn_data_bitmask_sequence->set_complex_value(0, dyn_data_my_bitmask);
    dyn_data->return_loaned_value(dyn_data_my_bitmask);
    dyn_data->return_loaned_value(dyn_data_bitmask_sequence);

    // alias
    traits<DynamicData>::ref_type dyn_data_my_aliased_struct =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("my_aliased_struct"));
    fill_primitives_struct(dyn_data_my_aliased_struct, index);
    dyn_data->return_loaned_value(dyn_data_my_aliased_struct);

    fill_my_enum(dyn_data, "my_aliased_enum", index);

    dyn_data->set_string_value(dyn_data->get_member_id_by_name("my_aliased_bounded_string"),
            "my_aliased_bounded_string");

    fill_my_enum(dyn_data, "my_recursive_alias", index);

    // sequences
    Int16Seq short_seq = {0, static_cast<int16_t>(index)};
    dyn_data->set_int16_values(dyn_data->get_member_id_by_name("short_sequence"), short_seq);

    Int32Seq enum_seq;
    switch (index % 3)
    {
        case 0:
            enum_seq = {static_cast<int32_t>(0)};
            break;
        case 1:
            enum_seq = {static_cast<int32_t>(1)};
            break;
        case 2:
            enum_seq = {static_cast<int32_t>(2)};
            break;
    }
    dyn_data->set_int32_values(dyn_data->get_member_id_by_name("enum_sequence"), enum_seq);

    // array
    Int32Seq long_array_seq = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    dyn_data->set_int32_values(dyn_data->get_member_id_by_name("long_array"), long_array_seq);

    // maps
    traits<DynamicData>::ref_type dyn_data_string_unbounded_map =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("string_unbounded_map"));
    dyn_data_string_unbounded_map->set_string_value(dyn_data_string_unbounded_map->get_member_id_by_name(
                "0"), "string_unbounded_map");
    dyn_data->return_loaned_value(dyn_data_string_unbounded_map);

    traits<DynamicData>::ref_type dyn_data_string_alias_unbounded_map = dyn_data->loan_value(dyn_data->get_member_id_by_name(
                        "string_alias_unbounded_map"));
    dyn_data_string_alias_unbounded_map->set_string_value(dyn_data_string_alias_unbounded_map->get_member_id_by_name(
                "0"), "string_alias_unbounded_map");
    dyn_data->return_loaned_value(dyn_data_string_alias_unbounded_map);

    traits<DynamicData>::ref_type dyn_data_short_long_map =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("short_long_map"));
    int32_t key {1};
    dyn_data_short_long_map->set_int32_value(dyn_data_short_long_map->get_member_id_by_name(std::to_string(
                key)), static_cast<int32_t>(index));
    dyn_data->return_loaned_value(dyn_data_short_long_map);

    // union
    traits<DynamicData>::ref_type dyn_data_inner_union =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("inner_union"));
    dyn_data_inner_union->set_int64_value(dyn_data_inner_union->get_member_id_by_name("second"),
            static_cast<int64_t>(index));
    traits<DynamicData>::ref_type dyn_data_complex_union =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_union"));
    dyn_data_complex_union->set_complex_value(dyn_data_complex_union->get_member_id_by_name(
                "fourth"), dyn_data_inner_union);
    dyn_data->return_loaned_value(dyn_data_complex_union);
    dyn_data->return_loaned_value(dyn_data_inner_union);

    // bitset
    traits<DynamicData>::ref_type dyn_data_my_bitset =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("my_bitset"));
    dyn_data_my_bitset->set_int16_value(dyn_data_my_bitset->get_member_id_by_name("d"), static_cast<int16_t>(index));
    dyn_data->return_loaned_value(dyn_data_my_bitset);
}

traits<DynamicData>::ref_type fill_dyn_data(
        traits<DynamicData>::ref_type dyn_data,
        const unsigned int& index)
{
    // index
    dyn_data->set_uint32_value(dyn_data->get_member_id_by_name("index"), index);

    // inner struct
    traits<DynamicData>::ref_type dyn_data_all_struct =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("inner_struct"));
    fill_all_struct(dyn_data_all_struct, index);

    // complex sequence
    traits<DynamicData>::ref_type dyn_data_complex_sequence =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_sequence"));
    dyn_data_complex_sequence->set_complex_value(0, dyn_data_all_struct);
    dyn_data->return_loaned_value(dyn_data_complex_sequence);

    // complex array
    traits<DynamicData>::ref_type dyn_data_complex_array =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_array"));
    for (unsigned int i = 0; i < 2; i++)
    {
        dyn_data_complex_array->set_complex_value(i, dyn_data_all_struct);
    }
    dyn_data->return_loaned_value(dyn_data_complex_array);

    // complex map
    traits<DynamicData>::ref_type dyn_data_complex_map =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_map"));
    dyn_data_complex_map->set_complex_value(dyn_data_complex_map->get_member_id_by_name(std::to_string(
                0)), dyn_data_all_struct);
    dyn_data->return_loaned_value(dyn_data_complex_map);

    // Return AllStruct loaned DynamicData
    dyn_data->return_loaned_value(dyn_data_all_struct);

    return dyn_data;

}

template <>
traits<DynamicType>::ref_type create_dynamic_type<DataTypeKind::COMPREHENSIVE_TYPE>()
{
    eprosima::fastdds::dds::TypeSupport type(new ComprehensiveTypePubSubType());
    const auto type_name = type->get_name();

    type->register_type_object_representation();

    xtypes::TypeObjectPair type_object_pair;
    if (RETCODE_OK !=
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name, type_object_pair))

    {
        std::cout << "Failed to get type objects for " << type_name << " type." << std::endl;
        return nullptr;
    }

    // Create DynamicType
    auto type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        type_object_pair.complete_type_object);
    const auto dynamic_type = type_builder->build();

    return dynamic_type;
}

template <>
traits<DynamicData>::ref_type create_dynamic_data<DataTypeKind::COMPREHENSIVE_TYPE>(
        const traits<DynamicType>::ref_type& dynamic_type,
        bool filled,
        const unsigned int& index)
{
    if (dynamic_type == nullptr)
    {
        std::cout << "Error creating dynamic data because the provided dynamic type is nullptr." << std::endl;
        return nullptr;
    }

    // Create DynamicData
    auto dynamic_data = DynamicDataFactory::get_instance()->create_data(dynamic_type);

    if (!filled)
    {
        return dynamic_data;
    }

    return fill_dyn_data(dynamic_data, index);
}

template <>
std::string get_expected_json<DataTypeKind::COMPREHENSIVE_TYPE>(
        const DynamicDataJsonFormat& format,
        bool filled,
        const unsigned int& index)
{
    std::string json;

    switch (format)
    {
        case DynamicDataJsonFormat::EPROSIMA:
            if (filled)
            {
                switch (index)
                {
                    case 1:
                        json = "ComprehensiveType_Filled_EPROSIMA_1";
                        break;
                    case 2:
                        json = "ComprehensiveType_Filled_EPROSIMA_2";
                        break;
                    case 3:
                        json = "ComprehensiveType_Filled_EPROSIMA_3";
                        break;
                    default:
                        throw std::invalid_argument("Invalid index for filled EPROSIMA format");
                }
            }
            else
            {
                json = "ComprehensiveType_Unfilled_EPROSIMA";
            }
            break;

        case DynamicDataJsonFormat::OMG:
            if (filled)
            {
                switch (index)
                {
                    case 1:
                        json = "ComprehensiveType_Filled_OMG_1";
                        break;
                    case 2:
                        json = "ComprehensiveType_Filled_OMG_2";
                        break;
                    case 3:
                        json = "ComprehensiveType_Filled_OMG_3";
                        break;
                    default:
                        throw std::invalid_argument("Invalid index for filled OMG format");
                }
            }
            else
            {
                json = "ComprehensiveType_Unfilled_OMG";
            }
            break;

        default:
            throw std::invalid_argument("Unsupported format");
    }

    // JSON file to read
    const auto file_name = std::string("types/comprehensive_type/json/") + json + ".json";
    std::ifstream file(file_name);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + file_name);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
