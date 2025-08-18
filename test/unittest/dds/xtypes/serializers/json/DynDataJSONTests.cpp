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

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "types/types.hpp"

#include <gtest/gtest.h>

/**
 * This test is designed to validate the serialization process from DynamicData to JSON format using
 * the eProsima Fast DDS library.
 *
 * CASES:
 *  1. Serialization of an unfilled DynamicData to JSON using the EPROSIMA format.
 *  2. Serialization of a filled DynamicData to JSON using the EPROSIMA format.
 *  3. Serialization of an unfilled DynamicData to JSON using the OMG format.
 *  4. Serialization of a filled DynamicData to JSON using the OMG format.
 *
 *  Each case verifies that the generated JSON matches the expected JSON string.
 */

using namespace eprosima::fastdds::dds;

template <DataTypeKind Data>
void test_generic()
{
    std::vector<DynamicDataJsonFormat> format_options = {DynamicDataJsonFormat::EPROSIMA, DynamicDataJsonFormat::OMG};
    auto dyn_type = create_dynamic_type<Data>();

    ASSERT_NE(dyn_type, nullptr);

    for (const auto& format_kind : format_options)
    {
        for (unsigned int fill_index = 0; fill_index <= 3; fill_index++)
        {
            bool filled = fill_index != 0;

            auto dyn_data = create_dynamic_data<Data>(dyn_type, filled, fill_index);

            ASSERT_NE(dyn_data, nullptr);

            // Test DynamicData to JSON serialization
            std::stringstream generated_json;
            generated_json << std::setw(4);
            auto ret = json_serialize(
                dyn_data,
                format_kind,
                generated_json);
            EXPECT_EQ(ret, RETCODE_OK);
            EXPECT_EQ(generated_json.str(), get_expected_json<Data>(format_kind, filled, fill_index));

            // Test JSON to DynamicData deserialization
            DynamicData::_ref_type dyn_data_from_json;
            ret = json_deserialize(
                generated_json.str(),
                dyn_type,
                format_kind,
                dyn_data_from_json);
            EXPECT_EQ(ret, RETCODE_OK);

            // Check that the deserialized DynamicData matches the original
            ASSERT_NE(dyn_data_from_json, nullptr);
            EXPECT_TRUE(dyn_data->equals(dyn_data_from_json));
        }
    }
}

TEST(DynDataJSONTests, ComprehensiveType)
{
    test_generic<DataTypeKind::COMPREHENSIVE_TYPE>();
}

template<typename AddMembersFn>
DynamicType::_ref_type create_struct_type(
        const std::string& name,
        AddMembersFn&& add_members)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(name);
    DynamicTypeBuilder::_ref_type builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    add_members(builder);

    return builder->build();
}

void test_negative_case(
        const std::string& json,
        const DynamicType::_ref_type& dyn_type)
{
    DynamicData::_ref_type dyn_data;
    ReturnCode_t ret = json_deserialize(json, dyn_type, DynamicDataJsonFormat::OMG, dyn_data);

    EXPECT_NE(ret, RETCODE_OK);
    EXPECT_EQ(dyn_data, nullptr);  // nothing was allocated
}

TEST(DynDataJSONTests, json_deserialize_negative)
{
    // Malformed JSON
    {
        std::string json = R"({"my_bool":true)";  // malformed (missing closing brace)

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("my_bool");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_BOOLEAN));
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Numeric overflow
    {
        std::string json = R"({"my_long":8589934592})";  // Overflowing int32_t

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("my_long");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_INT32));
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Array overflow
    {
        std::string json = R"({"my_array":[1,2,3,4]})";  // Overflowing array of size 3

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type array_member_descriptor {traits<MemberDescriptor>::make_shared()};
                            array_member_descriptor->name("my_array");
                            array_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32), {3})->build());
                            b->add_member(array_member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Missing struct member
    {
        std::string json = R"({"first_attr":42})";  // "second_attr" is absent

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("first_attr");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_INT32));
                            b->add_member(member_descriptor);
                            member_descriptor = traits<MemberDescriptor>::make_shared();
                            member_descriptor->name("second_attr");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_BOOLEAN));
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Unknown member
    {
        std::string json = R"({"value":10,"intruder":99})";  // "intruder" is not defined in the struct

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("value");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_INT32));
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Bounded string overflow
    {
        std::string json = R"({"my_bounded_str":")" + std::string(65, 'X') + R"("})"; // 65 > bound 64

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("my_bounded_str");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(64)->
                                    build());
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Invalid enum value
    {
        std::string json = R"({"color":"BLUE"})";  // "BLUE" is not a valid enum value

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            // Define enum type
                            TypeDescriptor::_ref_type enum_type_descriptor {traits<TypeDescriptor>::make_shared()};
                            enum_type_descriptor->kind(TK_ENUM);
                            enum_type_descriptor->name("MyEnum");
                            DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                                            enum_type_descriptor)};

                            // Add enum literals to the type
                            MemberDescriptor::_ref_type enum_member_descriptor {traits<MemberDescriptor>::make_shared()};
                            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_INT32));
                            enum_member_descriptor->name("RED");
                            enum_builder->add_member(enum_member_descriptor);
                            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
                            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_INT32));
                            enum_member_descriptor->name("GREEN");
                            enum_builder->add_member(enum_member_descriptor);

                            // Build enum type
                            DynamicType::_ref_type enum_type = enum_builder->build();

                            // Add enum member to the struct
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("color");
                            member_descriptor->type(enum_type);
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Type mismatch
    {
        std::string json = R"({"id":"not_an_int"})";  // expecting int, got string

        auto dyn_type = create_struct_type("TestStruct",
                        [](auto& b)
                        {
                            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                            member_descriptor->name("id");
                            member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                                TK_INT32));
                            b->add_member(member_descriptor);
                        });

        ASSERT_NE(dyn_type, nullptr);
        test_negative_case(json, dyn_type);
    }

    // Flush log before finishing to avoid deadlock in Windows (Redmine #23458)
    Log::Flush();
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
