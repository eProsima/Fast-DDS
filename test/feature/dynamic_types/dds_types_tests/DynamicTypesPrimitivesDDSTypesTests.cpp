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

#include <gtest/gtest.h>

#include "../DynamicTypesDDSTypesTest.hpp"
#include "../../../dds-types-test/primitivesPubSubTypes.hpp"
#include "../../../dds-types-test/primitivesTypeObjectSupport.hpp"
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

constexpr const char* struct_short_name = "ShortStruct";
constexpr const char* struct_ushort_name = "UShortStruct";
constexpr const char* struct_long_name = "LongStruct";
constexpr const char* struct_ulong_name = "ULongStruct";
constexpr const char* struct_long_long_name = "LongLongStruct";
constexpr const char* struct_ulong_long_name = "ULongLongStruct";
constexpr const char* struct_float_name = "FloatStruct";
constexpr const char* struct_double_name = "DoubleStruct";
constexpr const char* struct_long_double_name = "LongDoubleStruct";
constexpr const char* struct_boolean_name = "BooleanStruct";
constexpr const char* struct_byte_name = "OctetStruct";
constexpr const char* struct_char_name = "CharStruct";
constexpr const char* struct_wchar_name = "WCharStruct";
constexpr const char* struct_int8_name = "Int8Struct";
constexpr const char* struct_uint8_name = "Uint8Struct";
constexpr const char* struct_int16_name = "Int16Struct";
constexpr const char* struct_uint16_name = "Uint16Struct";
constexpr const char* struct_int32_name = "Int32Struct";
constexpr const char* struct_uint32_name = "Uint32Struct";
constexpr const char* struct_int64_name = "Int64Struct";
constexpr const char* struct_uint64_name = "Uint64Struct";

constexpr const char* var_int8_name = "var_int8";
constexpr const char* var_uint8_name = "var_uint8";
constexpr const char* var_int16_name = "var_int16";
constexpr const char* var_uint16_name = "var_uint16";
constexpr const char* var_int32_name = "var_int32";
constexpr const char* var_uint32_name = "var_uint32";
constexpr const char* var_int64_name = "var_int64";
constexpr const char* var_uint64_name = "var_uint64";

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ShortStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_short_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value = 2;
    int16_t test_value = 0;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ShortStruct struct_data;
        TypeSupport static_pubsubType {new ShortStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ShortStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UShortStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ushort_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint16_t value = 2;
    uint16_t test_value = 0;
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(var_ushort_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(var_ushort_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        UShortStruct struct_data;
        TypeSupport static_pubsubType {new UShortStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_ushort(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UShortStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        LongStruct struct_data;
        TypeSupport static_pubsubType {new LongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_long(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_LongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ULongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ulong_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint32_t value = 2;
    uint32_t test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_ulong_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_ulong_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ULongStruct struct_data;
        TypeSupport static_pubsubType {new ULongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_ulong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ULongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LongLongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_long_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int64_t value = 2;
    int64_t test_value = 0;
    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(var_long_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(var_long_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        LongLongStruct struct_data;
        TypeSupport static_pubsubType {new LongLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_longlong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_LongLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ULongLongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ulong_long_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint64_t value = 2;
    uint64_t test_value = 0;
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(var_ulong_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(var_ulong_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ULongLongStruct struct_data;
        TypeSupport static_pubsubType {new ULongLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_ulonglong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ULongLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_FloatStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_float_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_float_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float value = 2;
    float test_value = 0;
    EXPECT_EQ(data->set_float32_value(data->get_member_id_by_name(var_float_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test_value, data->get_member_id_by_name(var_float_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        FloatStruct struct_data;
        TypeSupport static_pubsubType {new FloatStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_float(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_FloatStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_DoubleStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_double_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_double_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    double value = 2;
    double test_value = 0;
    EXPECT_EQ(data->set_float64_value(data->get_member_id_by_name(var_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test_value, data->get_member_id_by_name(var_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        DoubleStruct struct_data;
        TypeSupport static_pubsubType {new DoubleStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_double(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_DoubleStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_LongDoubleStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_double_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_double_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT128));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    long double value = 2;
    long double test_value = 0;
    EXPECT_EQ(data->set_float128_value(data->get_member_id_by_name(var_long_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test_value, data->get_member_id_by_name(var_long_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        LongDoubleStruct struct_data;
        TypeSupport static_pubsubType {new LongDoubleStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_longdouble(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_LongDoubleStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BooleanStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_boolean_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bool_name);
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    bool value = true;
    bool test_value = false;
    EXPECT_EQ(data->set_boolean_value(data->get_member_id_by_name(var_bool_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test_value, data->get_member_id_by_name(var_bool_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        BooleanStruct alias_data;
        TypeSupport static_pubsubType {new BooleanStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.var_boolean(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BooleanStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_OctetStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_byte_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_byte_name);
    member_descriptor->type(factory->get_primitive_type(TK_BYTE));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    eprosima::fastdds::rtps::octet value = 255;
    eprosima::fastdds::rtps::octet test_value = 0;
    EXPECT_EQ(data->set_byte_value(data->get_member_id_by_name(var_byte_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test_value, data->get_member_id_by_name(var_byte_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        OctetStruct alias_data;
        TypeSupport static_pubsubType {new OctetStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.var_octet(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_OctetStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_CharStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_char_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char_name);
    member_descriptor->type(factory->get_primitive_type(TK_CHAR8));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    char value = 'a';
    char test_value = 'b';
    EXPECT_EQ(data->set_char8_value(data->get_member_id_by_name(var_char_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test_value, data->get_member_id_by_name(var_char_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        CharStruct alias_data;
        TypeSupport static_pubsubType {new CharStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.var_char8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_CharStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_WCharStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_wchar_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wchar_name);
    member_descriptor->type(factory->get_primitive_type(TK_CHAR16));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    wchar_t value = L'a';
    wchar_t test_value = L'b';
    EXPECT_EQ(data->set_char16_value(data->get_member_id_by_name(var_wchar_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test_value, data->get_member_id_by_name(var_wchar_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        WCharStruct alias_data;
        TypeSupport static_pubsubType {new WCharStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.var_char16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_WCharStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Int8Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_int8_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_int8_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT8));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int8_t value = 2;
    int8_t test_value = 0;
    EXPECT_EQ(data->set_int8_value(data->get_member_id_by_name(var_int8_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int8_value(test_value, data->get_member_id_by_name(var_int8_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Int8Struct struct_data;
        TypeSupport static_pubsubType {new Int8StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_int8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Int8Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Uint8Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_uint8_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_uint8_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT8));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint8_t value = 2;
    uint8_t test_value = 0;
    EXPECT_EQ(data->set_uint8_value(data->get_member_id_by_name(var_uint8_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_value(test_value, data->get_member_id_by_name(var_uint8_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Uint8Struct struct_data;
        TypeSupport static_pubsubType {new Uint8StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_uint8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Uint8Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Int16Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_int16_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_int16_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value = 2;
    int16_t test_value = 0;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_int16_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_int16_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Int16Struct struct_data;
        TypeSupport static_pubsubType {new Int16StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_int16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Int16Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Uint16Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_uint16_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_uint16_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint16_t value = 2;
    uint16_t test_value = 0;
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(var_uint16_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(var_uint16_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Uint16Struct struct_data;
        TypeSupport static_pubsubType {new Uint16StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_uint16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Uint16Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Int32Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_int32_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_int32_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_int32_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_int32_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Int32Struct struct_data;
        TypeSupport static_pubsubType {new Int32StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_int32(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Int32Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Uint32Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_uint32_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_uint32_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint32_t value = 2;
    uint32_t test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_uint32_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_uint32_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Uint32Struct struct_data;
        TypeSupport static_pubsubType {new Uint32StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_uint32(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Uint32Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Int64Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_int64_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_int64_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int64_t value = 2;
    int64_t test_value = 0;
    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(var_int64_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(var_int64_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Int64Struct struct_data;
        TypeSupport static_pubsubType {new Int64StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_int64(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Int64Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_Uint64Struct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_uint64_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_uint64_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint64_t value = 2;
    uint64_t test_value = 0;
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(var_uint64_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(var_uint64_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        Uint64Struct struct_data;
        TypeSupport static_pubsubType {new Uint64StructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_uint64(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Uint64Struct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
