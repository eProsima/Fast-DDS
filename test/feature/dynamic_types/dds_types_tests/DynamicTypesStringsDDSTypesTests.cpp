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

#include <gtest/gtest.h>

#include "../DynamicTypesDDSTypesTest.hpp"
#include "../../../dds-types-test/stringsPubSubTypes.hpp"
#include "../../../dds-types-test/stringsTypeObjectSupport.hpp"
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* struct_string_name = "StringStruct";
constexpr const char* struct_wstring_name = "WStringStruct";
constexpr const char* struct_small_string_name = "SmallStringStruct";
constexpr const char* struct_small_wstring_name = "SmallWStringStruct";
constexpr const char* struct_large_string_name = "LargeStringStruct";
constexpr const char* struct_large_wstring_name = "LargeWStringStruct";

constexpr const char* var_string_name = "var_string8";
constexpr const char* var_wstring_name = "var_string16";
constexpr const char* var_small_string_name = "var_small_string";
constexpr const char* var_small_wstring_name = "var_small_wstring";
constexpr const char* var_large_string_name = "var_large_string";
constexpr const char* var_large_wstring_name = "var_large_wstring";

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StringStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_string_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string value = "HELLO WORLD";
    std::string test_value;
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_string_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_value, data->get_member_id_by_name(var_string_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StringStruct struct_data;
        TypeSupport static_pubsubType {new StringStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_string8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StringStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_WStringStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_wstring_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wstring_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_wstring_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::wstring value = L"HELLO WORLD";
    std::wstring test_value;
    EXPECT_EQ(data->set_wstring_value(data->get_member_id_by_name(var_wstring_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test_value, data->get_member_id_by_name(var_wstring_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        WStringStruct struct_data;
        TypeSupport static_pubsubType {new WStringStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_string16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_WStringStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SmallStringStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_small_string_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_small_string_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(1)->build());
    type_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string value = "H";
    std::string test_value;
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_small_string_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_value, data->get_member_id_by_name(var_small_string_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SmallStringStruct struct_data;
        TypeSupport static_pubsubType {new SmallStringStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_small_string(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SmallStringStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SmallWStringStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_small_wstring_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_small_wstring_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_wstring_type(1)->build());
    type_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::wstring value = L"H";
    std::wstring test_value;
    EXPECT_EQ(data->set_wstring_value(data->get_member_id_by_name(var_small_wstring_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test_value, data->get_member_id_by_name(var_small_wstring_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SmallWStringStruct struct_data;
        TypeSupport static_pubsubType {new SmallWStringStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_small_wstring(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SmallWStringStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LargeStringStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_large_string_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_large_string_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(41925)->build());
    type_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string value = "HELLO WORLD";
    std::string test_value;
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_large_string_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_value, data->get_member_id_by_name(var_large_string_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        LargeStringStruct struct_data;
        TypeSupport static_pubsubType {new LargeStringStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_large_string(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_LargeStringStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LargeWStringStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_large_wstring_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_large_wstring_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_wstring_type(41925)->build());
    type_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::wstring value = L"HELLO WORLD";
    std::wstring test_value;
    EXPECT_EQ(data->set_wstring_value(data->get_member_id_by_name(var_large_wstring_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test_value, data->get_member_id_by_name(var_large_wstring_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        LargeWStringStruct struct_data;
        TypeSupport static_pubsubType {new LargeWStringStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_large_wstring(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_LargeWStringStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
