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

#include <numeric>
#include <string>
#include <iostream>
#include <fstream>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
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

template<DataTypeKind Data>
void test_generic()
{
    std::vector<DynamicDataJsonMapping> mapping_options = {DynamicDataJsonMapping::EPROSIMA,
                                                           DynamicDataJsonMapping::OMG};
    auto dyn_type = create_dynamic_type<Data>();

    ASSERT_NE(dyn_type, nullptr);

    for (const auto& mapping : mapping_options)
    {
        FormatOptions format_options{mapping, 0};
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
                format_options,
                generated_json);
            EXPECT_EQ(ret, RETCODE_OK);
            EXPECT_EQ(generated_json.str(), get_expected_json<Data>(format_options, filled, fill_index));

            // Test JSON to DynamicData deserialization
            DynamicData::_ref_type dyn_data_from_json;
            ret = json_deserialize(
                generated_json.str(),
                dyn_type,
                format_options,
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
    FormatOptions format_options{DynamicDataJsonMapping::OMG, 0};
    ReturnCode_t ret = json_deserialize(json, dyn_type, format_options, dyn_data);

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
                            DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->
                                                                                create_type(
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

TEST(DynDataJSONTests, json_serialize_max_collection_items_sequence_truncated)
{
    // A sequence with more elements than max_collection_items must emit the compact label.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_int32_values(seq_id, {0, 1, 2, 3, 4}));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<sequence: 5 long>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_sequence_not_truncated)
{
    // A sequence at or below the limit must be serialized in full.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_int32_values(seq_id, {0, 1, 2}));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3; // exactly at the limit (must not truncate)

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_EQ(output.str().find("<sequence:"), std::string::npos);
    EXPECT_EQ(output.str().find("<array:"), std::string::npos);
    EXPECT_EQ(output.str().find("<binary:"), std::string::npos);
    EXPECT_NE(output.str().find("my_seq"), std::string::npos);
    EXPECT_NE(output.str().find("0"), std::string::npos);
    EXPECT_NE(output.str().find("2"), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_byte_sequence_truncated)
{
    // A byte sequence exceeding the limit must emit the generic compact label
    // using the IDL type name for TK_BYTE ("octet").
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_bytes");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_bytes");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_byte_values(seq_id, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 5;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<sequence: 10 octet>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_zero_no_truncation)
{
    // max_collection_items == 0 means no limit; full serialization must occur.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    std::vector<int32_t> values(100);
    std::iota(values.begin(), values.end(), 0);
    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_int32_values(seq_id, values));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 0;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_EQ(output.str().find("<sequence:"), std::string::npos);
    EXPECT_EQ(output.str().find("<array:"), std::string::npos);
    EXPECT_EQ(output.str().find("<binary:"), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_multidim_array_truncated)
{
    // A multi-dimensional array reports its flat element count to get_item_count(),
    // so the whole array collapses to a single label when the flat total exceeds
    // the limit. The compact label uses the array bounds (not the flat count).
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                {3, 4})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 5;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: long[3,4]>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_nested_sequence_truncated_per_level)
{
    // Truncation applies at every nesting level: the outer sequence stays expanded
    // (2 elements, within the limit), but the inner sequence exceeding the limit
    // collapses to a compact label.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        auto inner_seq_type =
                        DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                            static_cast<uint32_t>(LENGTH_UNLIMITED))->build();

                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq_of_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                inner_seq_type,
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq_of_seq");
    DynamicData::_ref_type outer_seq = dyn_data->loan_value(seq_id);
    ASSERT_TRUE(outer_seq);
    ASSERT_EQ(RETCODE_OK, outer_seq->set_int32_values(0, {0, 1}));
    ASSERT_EQ(RETCODE_OK, outer_seq->set_int32_values(1, {10, 11, 12, 13, 14, 15, 16, 17}));
    ASSERT_EQ(RETCODE_OK, dyn_data->return_loaned_value(outer_seq));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    // Outer sequence (2 elements) is within the limit, so it must be expanded.
    EXPECT_NE(output.str().find("my_seq_of_seq"), std::string::npos);
    // Inner sequence with 8 elements must be truncated to the compact label.
    EXPECT_NE(output.str().find("\"<sequence: 8 long>\""), std::string::npos);
    // The first inner sequence has 2 elements and must NOT be truncated.
    EXPECT_EQ(output.str().find("<sequence: 2 "), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_single_dim_array_truncated)
{
    // A single-dimensional array still uses the bounds form (e.g. "<10>") for the size.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                {10})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 5;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: long[10]>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_three_dim_array_truncated)
{
    // Bounds of every dimension must appear in the label, comma-separated.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                {2, 3, 4})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 5;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: long[2,3,4]>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_long_long_sequence_truncated)
{
    // The type name in the label keeps its full form for multi-word types
    // ("long long"), without any plural "s" suffix.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_int64_values(seq_id, {0, 1, 2, 3, 4, 5, 6}));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<sequence: 7 long long>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_string_array_truncated)
{
    // The type-name slot uses the IDL keyword for string members; the label
    // must not append a plural "s" that would yield "strings".
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        auto string_type =
                        DynamicTypeBuilderFactory::get_instance()->create_string_type(
                            static_cast<uint32_t>(LENGTH_UNLIMITED))->build();

                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                string_type,
                                {6})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: string[6]>\""), std::string::npos);
    EXPECT_EQ(output.str().find("strings"), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_map_element_label)
{
    // A truncated array whose element is a map exposes the map's key/value types
    // in the element slot: "map<long, string>". A fixed-bound array exceeds the
    // limit by virtue of its bound, so no population is required to trigger it.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        auto map_type =
                        DynamicTypeBuilderFactory::get_instance()->create_map_type(
                            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                            DynamicTypeBuilderFactory::get_instance()->create_string_type(
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build(),
                            static_cast<uint32_t>(LENGTH_UNLIMITED))->build();

                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                map_type,
                                {2})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 1;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: map<long, string>[2]>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_alias_element_resolved_label)
{
    // The element slot resolves through alias chains rather than emitting the
    // alias name: an array of "alias-to-long" reports "long", not the alias.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
                        alias_descriptor->kind(TK_ALIAS);
                        alias_descriptor->name("MyInt32Alias");
                        alias_descriptor->base_type(
                            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
                        auto alias_type =
                        DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build();

                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                alias_type,
                                {2})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 1;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: long[2]>\""), std::string::npos);
    EXPECT_EQ(output.str().find("MyInt32Alias"), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_struct_element_label)
{
    // An element that is a named construct is described by its own type name:
    // an array of "ElemStruct" reports "ElemStruct" in the element slot.
    auto struct_type = create_struct_type("ElemStruct",
                    [](auto&)
                    {
                    });
    ASSERT_NE(struct_type, nullptr);

    auto dyn_type = create_struct_type("TestStruct",
                    [&struct_type](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                struct_type,
                                {2})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 1;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<array: ElemStruct[2]>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_sequence_of_sequence_label)
{
    // When the outer sequence is truncated, the element label exposes one level
    // of detail (sequence<long>) rather than the bare "sequence" word.
    auto inner_seq_type =
            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
        static_cast<uint32_t>(LENGTH_UNLIMITED))->build();

    auto dyn_type = create_struct_type("TestStruct",
                    [&inner_seq_type](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq_of_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                inner_seq_type,
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });
    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq_of_seq");
    DynamicData::_ref_type outer_seq = dyn_data->loan_value(seq_id);
    ASSERT_TRUE(outer_seq);
    // Five inner sequences so the outer collection exceeds the limit and truncates.
    for (uint32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(RETCODE_OK, outer_seq->set_int32_values(i, {0, 1}));
    }
    ASSERT_EQ(RETCODE_OK, dyn_data->return_loaned_value(outer_seq));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<sequence: 5 sequence<long>>\""), std::string::npos);
}

TEST(DynDataJSONTests, json_deserialize_truncated_sequence_fails)
{
    // A document produced with max_collection_items that triggered truncation
    // cannot be round-tripped: the compact label is a JSON string where the
    // schema expects a JSON array, so deserialization must reject it.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_int32_values(seq_id, {0, 1, 2, 3, 4}));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));
    ASSERT_NE(output.str().find("\"<sequence: 5 long>\""), std::string::npos);

    // max_collection_items is ignored on the read path, but the truncated
    // payload itself is invalid: the string-shaped label cannot match a
    // sequence member, so deserialization fails and no data is allocated.
    DynamicData::_ref_type roundtripped;
    EXPECT_EQ(RETCODE_BAD_PARAMETER, json_deserialize(output.str(), dyn_type, opts, roundtripped));
    EXPECT_EQ(roundtripped, nullptr);
}

TEST(DynDataJSONTests, json_deserialize_truncated_array_fails)
{
    // Same as the sequence case: a truncated multi-dim array emits a
    // "<array: <idl-type>[d1,d2,...]>" label, which is not a JSON array, so
    // the deserializer must reject it.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_array");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_array_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                {3, 4})->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::EPROSIMA;
    opts.max_collection_items = 5;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));
    ASSERT_NE(output.str().find("\"<array: long[3,4]>\""), std::string::npos);

    DynamicData::_ref_type roundtripped;
    EXPECT_EQ(RETCODE_BAD_PARAMETER, json_deserialize(output.str(), dyn_type, opts, roundtripped));
    EXPECT_EQ(roundtripped, nullptr);
}

TEST(DynDataJSONTests, json_serialize_max_collection_items_omg_mapping)
{
    // The compact-label truncation must work identically under the OMG mapping.
    auto dyn_type = create_struct_type("TestStruct",
                    [](auto& b)
                    {
                        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                        member_descriptor->name("my_seq");
                        member_descriptor->type(
                            DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
                                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
                        b->add_member(member_descriptor);
                    });

    ASSERT_NE(dyn_type, nullptr);

    auto dyn_data = DynamicDataFactory::get_instance()->create_data(dyn_type);
    ASSERT_NE(dyn_data, nullptr);

    MemberId seq_id = dyn_data->get_member_id_by_name("my_seq");
    ASSERT_EQ(RETCODE_OK, dyn_data->set_int32_values(seq_id, {0, 1, 2, 3, 4}));

    FormatOptions opts;
    opts.mapping = DynamicDataJsonMapping::OMG;
    opts.max_collection_items = 3;

    std::stringstream output;
    ASSERT_EQ(RETCODE_OK, json_serialize(dyn_data, opts, output));

    EXPECT_NE(output.str().find("\"<sequence: 5 long>\""), std::string::npos);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
