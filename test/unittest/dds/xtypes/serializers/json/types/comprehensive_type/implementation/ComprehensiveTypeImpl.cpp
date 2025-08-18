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

#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;

traits<DynamicData>::ref_type create_primitives_struct()
{
    eprosima::fastdds::dds::TypeSupport type(new PrimitivesStructPubSubType());
    const auto type_name = type->get_name();

    xtypes::TypeObjectPair type_object_pair;
    if (RETCODE_OK !=
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name, type_object_pair))
    {
        std::cout << "Failed to get type objects for " << type_name << " type." << std::endl;
    }

    // Create DynamicType
    auto type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        type_object_pair.complete_type_object);
    const auto dynamic_type = type_builder->build();

    return DynamicDataFactory::get_instance()->create_data(dynamic_type);
}

void fill_primitives_struct(
        traits<DynamicData>::ref_type dyn_data,
        const unsigned int& index)
{
    if (index % 2)
    {
        EXPECT_EQ(dyn_data->set_boolean_value(dyn_data->get_member_id_by_name("my_bool"), true), RETCODE_OK);
    }
    else
    {
        EXPECT_EQ(dyn_data->set_boolean_value(dyn_data->get_member_id_by_name("my_bool"), false), RETCODE_OK);
    }
    EXPECT_EQ(dyn_data->set_byte_value(dyn_data->get_member_id_by_name("my_octet"),
            static_cast<eprosima::fastdds::rtps::octet>(index)), RETCODE_OK);
    if (index % 2)
    {
        EXPECT_EQ(dyn_data->set_char8_value(dyn_data->get_member_id_by_name(
                    "my_char"), static_cast<char>('e')), RETCODE_OK);
    }
    else
    {
        EXPECT_EQ(dyn_data->set_char8_value(dyn_data->get_member_id_by_name(
                    "my_char"), static_cast<char>('o')), RETCODE_OK);
    }
    if (index % 2)
    {
        EXPECT_EQ(dyn_data->set_char16_value(dyn_data->get_member_id_by_name(
                    "my_wchar"), static_cast<wchar_t>(L'e')), RETCODE_OK);
    }
    else
    {
        EXPECT_EQ(dyn_data->set_char16_value(dyn_data->get_member_id_by_name(
                    "my_wchar"), static_cast<wchar_t>(L'o')), RETCODE_OK);
    }
    EXPECT_EQ(dyn_data->set_int32_value(dyn_data->get_member_id_by_name(
                "my_long"), static_cast<int32_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_uint32_value(dyn_data->get_member_id_by_name(
                "my_ulong"), static_cast<uint32_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_int8_value(dyn_data->get_member_id_by_name(
                "my_int8"), static_cast<int8_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_uint8_value(dyn_data->get_member_id_by_name(
                "my_uint8"), static_cast<uint8_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_int16_value(dyn_data->get_member_id_by_name(
                "my_short"), static_cast<int16_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_uint16_value(dyn_data->get_member_id_by_name(
                "my_ushort"), static_cast<uint16_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_int64_value(dyn_data->get_member_id_by_name(
                "my_longlong"), static_cast<int64_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_uint64_value(dyn_data->get_member_id_by_name(
                "my_ulonglong"), static_cast<uint64_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_float32_value(dyn_data->get_member_id_by_name(
                "my_float"), static_cast<float>(0.5 * index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_float64_value(dyn_data->get_member_id_by_name(
                "my_double"), static_cast<double>(0.5 * index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_float128_value(dyn_data->get_member_id_by_name("my_longdouble"),
            static_cast<long double>(0.5 * index)), RETCODE_OK);
}

void fill_my_enum(
        traits<DynamicData>::ref_type dyn_data,
        const std::string& member_name,
        const unsigned int& index)
{
    switch (index % 3)
    {
        case 0:
            EXPECT_EQ(dyn_data->set_int32_value(dyn_data->get_member_id_by_name(member_name), 0), RETCODE_OK);
            break;
        case 1:
            EXPECT_EQ(dyn_data->set_int32_value(dyn_data->get_member_id_by_name(member_name), 1), RETCODE_OK);
            break;
        case 2:
            EXPECT_EQ(dyn_data->set_int32_value(dyn_data->get_member_id_by_name(member_name), 2), RETCODE_OK);
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
    EXPECT_EQ(dyn_data->set_string_value(dyn_data->get_member_id_by_name("my_string"), "my_string"), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_wstring_value(dyn_data->get_member_id_by_name("my_wstring"), L"my_string"), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_string_value(dyn_data->get_member_id_by_name(
                "my_bounded_string"), "my_bounded_string"), RETCODE_OK);
    EXPECT_EQ(dyn_data->set_wstring_value(dyn_data->get_member_id_by_name(
                "my_bounded_wstring"), L"my_bounded_wstring"), RETCODE_OK);

    // enum
    fill_my_enum(dyn_data, "my_enum", index);

    // bitmask
    traits<DynamicData>::ref_type dyn_data_my_bitmask =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("my_bitmask"));
    ASSERT_NE(dyn_data_my_bitmask, nullptr);
    if (index % 2)
    {
        EXPECT_EQ(dyn_data_my_bitmask->set_boolean_value(dyn_data_my_bitmask->get_member_id_by_name(
                    "flag0"), true), RETCODE_OK);
    }
    else
    {
        EXPECT_EQ(dyn_data_my_bitmask->set_boolean_value(dyn_data_my_bitmask->get_member_id_by_name(
                    "flag0"), false), RETCODE_OK);
    }
    traits<DynamicData>::ref_type dyn_data_bitmask_sequence =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("bitmask_sequence"));
    ASSERT_NE(dyn_data_bitmask_sequence, nullptr);
    EXPECT_EQ(dyn_data_bitmask_sequence->set_complex_value(0, dyn_data_my_bitmask), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_my_bitmask), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_bitmask_sequence), RETCODE_OK);

    // alias
    traits<DynamicData>::ref_type dyn_data_my_aliased_struct =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("my_aliased_struct"));
    ASSERT_NE(dyn_data_my_aliased_struct, nullptr);
    fill_primitives_struct(dyn_data_my_aliased_struct, index);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_my_aliased_struct), RETCODE_OK);

    fill_my_enum(dyn_data, "my_aliased_enum", index);

    EXPECT_EQ(dyn_data->set_string_value(dyn_data->get_member_id_by_name("my_aliased_bounded_string"),
            "my_aliased_bounded_string"), RETCODE_OK);

    fill_my_enum(dyn_data, "my_recursive_alias", index);

    // sequences
    Int16Seq short_seq = {0, static_cast<int16_t>(index)};
    EXPECT_EQ(dyn_data->set_int16_values(dyn_data->get_member_id_by_name("short_sequence"), short_seq), RETCODE_OK);

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
    EXPECT_EQ(dyn_data->set_int32_values(dyn_data->get_member_id_by_name("enum_sequence"), enum_seq), RETCODE_OK);

    // array
    Int32Seq long_array_seq = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    EXPECT_EQ(dyn_data->set_int32_values(dyn_data->get_member_id_by_name("long_array"), long_array_seq), RETCODE_OK);

    // maps
    traits<DynamicData>::ref_type dyn_data_string_unbounded_map =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("string_unbounded_map"));
    ASSERT_NE(dyn_data_string_unbounded_map, nullptr);
    EXPECT_EQ(dyn_data_string_unbounded_map->set_string_value(
                dyn_data_string_unbounded_map->get_member_id_by_name("0"), "string_unbounded_map"), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_string_unbounded_map), RETCODE_OK);

    traits<DynamicData>::ref_type dyn_data_string_alias_unbounded_map = dyn_data->loan_value(dyn_data->get_member_id_by_name(
                        "string_alias_unbounded_map"));
    ASSERT_NE(dyn_data_string_alias_unbounded_map, nullptr);
    EXPECT_EQ(dyn_data_string_alias_unbounded_map->set_string_value(
                dyn_data_string_alias_unbounded_map->get_member_id_by_name(
                    "0"), "string_alias_unbounded_map"), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_string_alias_unbounded_map), RETCODE_OK);

    traits<DynamicData>::ref_type dyn_data_short_long_map =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("short_long_map"));
    ASSERT_NE(dyn_data_short_long_map, nullptr);
    int32_t key {1};
    EXPECT_EQ(dyn_data_short_long_map->set_int32_value(
                dyn_data_short_long_map->get_member_id_by_name(std::to_string(
                    key)), static_cast<int32_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_short_long_map), RETCODE_OK);

    // unions
    traits<DynamicData>::ref_type dyn_data_inner_union =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("inner_union"));
    ASSERT_NE(dyn_data_inner_union, nullptr);
    if (index % 2)
    {
        EXPECT_EQ(dyn_data_inner_union->set_int64_value(
                    dyn_data_inner_union->get_member_id_by_name("second"), static_cast<int64_t>(index)), RETCODE_OK);
    }
    else
    {
        auto primitive_struct = create_primitives_struct();
        ASSERT_NE(primitive_struct, nullptr);
        fill_primitives_struct(primitive_struct, index);
        EXPECT_EQ(dyn_data_inner_union->set_complex_value(
                    dyn_data_inner_union->get_member_id_by_name("first"), primitive_struct), RETCODE_OK);

        // At the time of this writing, the DynamicData API does not support setting a complex non-alias value
        // with a DynamicData whose type is an alias of the type being set.
        // Substitute the block above with the following lines if it is supported in the future.
        /* dyn_data_my_aliased_struct = dyn_data->loan_value(dyn_data->get_member_id_by_name("my_aliased_struct"));
           ASSERT_NE(dyn_data_my_aliased_struct, nullptr);
           EXPECT_EQ(dyn_data_inner_union->set_complex_value(
                    dyn_data_inner_union->get_member_id_by_name("first"), dyn_data_my_aliased_struct), RETCODE_OK);
           EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_my_aliased_struct), RETCODE_OK); */
    }

    traits<DynamicData>::ref_type dyn_data_complex_union =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_union"));
    ASSERT_NE(dyn_data_complex_union, nullptr);
    if (index % 2)
    {
        EXPECT_EQ(dyn_data_complex_union->set_complex_value(
                    dyn_data_complex_union->get_member_id_by_name("fourth"), dyn_data_inner_union), RETCODE_OK);
    }
    else
    {
        EXPECT_EQ(dyn_data_complex_union->set_int32_value(
                    dyn_data_complex_union->get_member_id_by_name("third"), static_cast<int32_t>(index)), RETCODE_OK);
    }
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_complex_union), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_inner_union), RETCODE_OK);

    // bitset
    traits<DynamicData>::ref_type dyn_data_my_bitset =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("my_bitset"));
    ASSERT_NE(dyn_data_my_bitset, nullptr);
    EXPECT_EQ(dyn_data_my_bitset->set_int16_value(
                dyn_data_my_bitset->get_member_id_by_name("d"), static_cast<int16_t>(index)), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_my_bitset), RETCODE_OK);
}

void fill_dyn_data(
        traits<DynamicData>::ref_type dyn_data,
        const unsigned int& index)
{
    // index
    EXPECT_EQ(dyn_data->set_uint32_value(dyn_data->get_member_id_by_name("index"), index), RETCODE_OK);

    // inner struct
    traits<DynamicData>::ref_type dyn_data_all_struct =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("inner_struct"));
    ASSERT_NE(dyn_data_all_struct, nullptr);
    fill_all_struct(dyn_data_all_struct, index);

    // complex sequence
    traits<DynamicData>::ref_type dyn_data_complex_sequence =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_sequence"));
    ASSERT_NE(dyn_data_complex_sequence, nullptr);
    EXPECT_EQ(dyn_data_complex_sequence->set_complex_value(0, dyn_data_all_struct), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_complex_sequence), RETCODE_OK);

    // complex array
    traits<DynamicData>::ref_type dyn_data_complex_array =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_array"));
    ASSERT_NE(dyn_data_complex_array, nullptr);
    for (unsigned int i = 0; i < 2; i++)
    {
        EXPECT_EQ(dyn_data_complex_array->set_complex_value(i, dyn_data_all_struct), RETCODE_OK);
    }
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_complex_array), RETCODE_OK);

    // complex map
    traits<DynamicData>::ref_type dyn_data_complex_map =
            dyn_data->loan_value(dyn_data->get_member_id_by_name("complex_map"));
    ASSERT_NE(dyn_data_complex_map, nullptr);
    EXPECT_EQ(dyn_data_complex_map->set_complex_value(
                dyn_data_complex_map->get_member_id_by_name(std::to_string(0)), dyn_data_all_struct), RETCODE_OK);
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_complex_map), RETCODE_OK);

    // Return AllStruct loaned DynamicData
    EXPECT_EQ(dyn_data->return_loaned_value(dyn_data_all_struct), RETCODE_OK);
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

    if (filled)
    {
        fill_dyn_data(dynamic_data, index);
    }

    return dynamic_data;
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
