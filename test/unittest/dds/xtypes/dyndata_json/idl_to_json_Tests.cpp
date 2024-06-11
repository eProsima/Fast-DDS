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

#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

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

#include "types/comprehensive_type/gen/ComprehensiveTypePubSubTypes.h"
#include "types/comprehensive_type/gen/ComprehensiveTypeTypeObjectSupport.hpp"
#include "types/comprehensive_type/ComprehensiveType_OMG.hpp"
#include "types/comprehensive_type/ComprehensiveTypeFilled_OMG.hpp"
#include "types/comprehensive_type/ComprehensiveType_EPROSIMA.hpp"
#include "types/comprehensive_type/ComprehensiveTypeFilled_EPROSIMA.hpp"

#include <gtest/gtest.h>

/**
 * These tests are designed to validate the serialization process from DynamicData to JSON format using
 * the eProsima Fast DDS library.
 *
 * CASES:
 *  1. Test for serializing raw DynamicData to JSON using the EPROSIMA format.
 *  2. Test for serializing filled DynamicData to JSON using the EPROSIMA format.
 *  3. Test for serializing raw DynamicData to JSON using the OMG format.
 *  4. Test for serializing filled DynamicData to JSON using the OMG format.
 *
 *  Each test verifies that the generated JSON matches the expected JSON string.
 */

using namespace eprosima::fastdds::dds;

struct MyTemplateStruct
{
    bool my_bool;
    char my_char;
    double my_double;
    float my_float;
    int8_t my_int8;
    long my_long;
    long double my_longdouble;
    long long my_longlong;
    uint8_t my_octet;
    short my_short;
    uint8_t my_uint8;
    unsigned long my_ulong;
    unsigned long long my_ulonglong;
    unsigned short my_ushort;
    wchar_t my_wchar;
};

traits<DynamicData>::ref_type create_dyn_data(
        const std::string& type_name)
{
    xtypes::TypeObjectPair type_object_pair;
    if (RETCODE_OK !=
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                type_name, type_object_pair))

    {
        std::cout << "ERROR" << std::endl;
        return nullptr;
    }

    // Create DynamicType
    auto type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            type_object_pair.complete_type_object);
    const auto dynamic_type = type_builder->build();

    // Create DynamicData
    const auto dynamic_data = DynamicDataFactory::get_instance()->create_data(dynamic_type);

    return dynamic_data;
}


TEST(idl_to_json_Tests, ComprehensiveType_EPROSIMA)
{
    const auto TYPE_NAME = "ComprehensiveType";
    register_ComprehensiveType_type_objects();

    // Create Dynamic Data
    const auto dyn_data = create_dyn_data(TYPE_NAME);

    std::stringstream generated_json_comprehensive;
    generated_json_comprehensive << std::setw(4);
    const auto ret = json_serialize(
                dyn_data,
                generated_json_comprehensive,
                DynamicDataJsonFormat::EPROSIMA);

    ASSERT_EQ(ret, RETCODE_OK);
    ASSERT_EQ(generated_json_comprehensive.str(), expected_json_comprehensive_eprosima);
}

TEST(idl_to_json_Tests, ComprehensiveTypeFilled_EPROSIMA)
{
    const auto TYPE_NAME = "ComprehensiveType";
    register_ComprehensiveType_type_objects();

    // Create Dynamic Data
    const auto dyn_data = create_dyn_data(TYPE_NAME);

    MyTemplateStruct my_struct = {
        true,       // my_bool
        'A',        // my_char
        2.0,        // my_double
        3.0f,       // my_float
        7,          // my_int8
        10,         // my_long
        5.5,        // my_longdouble
        20,         // my_longlong
        100,        // my_octet
        15,         // my_short
        255,        // my_uint8
        1000,       // my_ulong
        5000,       // my_ulonglong
        30,         // my_ushort
        'B'        // my_wchar
    };

    // Fill Dynamic Data
    // Set "index" value
    dyn_data->set_uint32_value(dyn_data->get_member_id_by_name("index"), 3);

    // Set values in "inner_struct"-> ("my_bool" and "my_bounded_string")
    traits<DynamicData>::ref_type dyn_data_inner_struct = dyn_data->loan_value(dyn_data->get_member_id_by_name("inner_struct"));
    dyn_data_inner_struct->set_boolean_value(dyn_data_inner_struct->get_member_id_by_name("my_bool"), true);
    dyn_data_inner_struct->set_string_value(dyn_data_inner_struct->get_member_id_by_name("my_bounded_string"), "fastdds");

    // Set values in "inner_struct"-> "my_aliased_struct"-> ("my_bool", "my_char", "my_double", "my_float", "my_int8", "my_longdouble" and "my_wchar")
    traits<DynamicData>::ref_type dyn_data_my_aliased_struct = dyn_data_inner_struct->loan_value(dyn_data_inner_struct->get_member_id_by_name("my_aliased_struct"));
    dyn_data_my_aliased_struct->set_boolean_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_bool"), my_struct.my_bool);
    dyn_data_my_aliased_struct->set_char8_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_char"), my_struct.my_char);
    dyn_data_my_aliased_struct->set_float64_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_double"), my_struct.my_double);
    dyn_data_my_aliased_struct->set_float32_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_float"), my_struct.my_float);
    dyn_data_my_aliased_struct->set_int8_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_int8"), my_struct.my_int8);
    dyn_data_my_aliased_struct->set_float128_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_longdouble"), my_struct.my_longdouble);
    dyn_data_my_aliased_struct->set_char16_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_wchar"), my_struct.my_wchar);

    // Return loaned values
    dyn_data_inner_struct->return_loaned_value(dyn_data_my_aliased_struct);
    dyn_data->return_loaned_value(dyn_data_inner_struct);

    std::stringstream generated_json_comprehensive;
    generated_json_comprehensive << std::setw(4);
    const auto ret = json_serialize(
                dyn_data,
                generated_json_comprehensive,
                DynamicDataJsonFormat::EPROSIMA);

    ASSERT_EQ(ret, RETCODE_OK);
    ASSERT_EQ(generated_json_comprehensive.str(), expected_json_comprehensive_filled_eprosima);
}

TEST(idl_to_json_Tests, ComprehensiveType_OMG)
{
    const auto TYPE_NAME = "ComprehensiveType";
    register_ComprehensiveType_type_objects();

    // Create Dynamic Data
    const auto dyn_data = create_dyn_data(TYPE_NAME);

    std::stringstream generated_json_comprehensive;
    generated_json_comprehensive << std::setw(4);
    const auto ret = json_serialize(
                dyn_data,
                generated_json_comprehensive,
                DynamicDataJsonFormat::OMG);

    ASSERT_EQ(ret, RETCODE_OK);
    ASSERT_EQ(generated_json_comprehensive.str(), expected_json_comprehensive_omg);
}

TEST(idl_to_json_Tests, ComprehensiveTypeFilled_OMG)
{
    const auto TYPE_NAME = "ComprehensiveType";
    register_ComprehensiveType_type_objects();

    // Create Dynamic Data
    const auto dyn_data = create_dyn_data(TYPE_NAME);

    MyTemplateStruct my_struct = {
        true,       // my_bool
        'A',        // my_char
        2.0,        // my_double
        3.0f,       // my_float
        7,          // my_int8
        10,         // my_long
        5.5,        // my_longdouble
        20,         // my_longlong
        100,        // my_octet
        15,         // my_short
        255,        // my_uint8
        1000,       // my_ulong
        5000,       // my_ulonglong
        30,         // my_ushort
        'B'        // my_wchar
    };

    // Fill Dynamic Data
    // Set "index" value
    dyn_data->set_uint32_value(dyn_data->get_member_id_by_name("index"), 3);

    // Set values in "inner_struct"-> ("my_bool" and "my_bounded_string")
    traits<DynamicData>::ref_type dyn_data_inner_struct = dyn_data->loan_value(dyn_data->get_member_id_by_name("inner_struct"));
    dyn_data_inner_struct->set_boolean_value(dyn_data_inner_struct->get_member_id_by_name("my_bool"), true);
    dyn_data_inner_struct->set_string_value(dyn_data_inner_struct->get_member_id_by_name("my_bounded_string"), "fastdds");

    // Set values in "inner_struct"-> "my_aliased_struct"-> ("my_bool", "my_char", "my_double", "my_float", "my_int8", "my_longdouble" and "my_wchar")
    traits<DynamicData>::ref_type dyn_data_my_aliased_struct = dyn_data_inner_struct->loan_value(dyn_data_inner_struct->get_member_id_by_name("my_aliased_struct"));
    dyn_data_my_aliased_struct->set_boolean_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_bool"), my_struct.my_bool);
    dyn_data_my_aliased_struct->set_char8_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_char"), my_struct.my_char);
    dyn_data_my_aliased_struct->set_float64_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_double"), my_struct.my_double);
    dyn_data_my_aliased_struct->set_float32_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_float"), my_struct.my_float);
    dyn_data_my_aliased_struct->set_int8_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_int8"), my_struct.my_int8);
    dyn_data_my_aliased_struct->set_float128_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_longdouble"), my_struct.my_longdouble);
    dyn_data_my_aliased_struct->set_char16_value(dyn_data_my_aliased_struct->get_member_id_by_name("my_wchar"), my_struct.my_wchar);

    // Return loaned values
    dyn_data_inner_struct->return_loaned_value(dyn_data_my_aliased_struct);
    dyn_data->return_loaned_value(dyn_data_inner_struct);

    std::stringstream generated_json_comprehensive;
    generated_json_comprehensive << std::setw(4);
    const auto ret = json_serialize(
                dyn_data,
                generated_json_comprehensive,
                DynamicDataJsonFormat::OMG);

    ASSERT_EQ(ret, RETCODE_OK);
    ASSERT_EQ(generated_json_comprehensive.str(), expected_json_comprehensive_filled_omg);
}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
