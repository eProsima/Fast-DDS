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
#include "../../../dds-types-test/keyPubSubTypes.hpp"
#include "../../../dds-types-test/keyTypeObjectSupport.hpp"
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

constexpr const char* struct_short_name = "KeyedShortStruct";
constexpr const char* struct_ushort_name = "KeyedUShortStruct";
constexpr const char* struct_long_name = "KeyedLongStruct";
constexpr const char* struct_ulong_name = "KeyedULongStruct";
constexpr const char* struct_long_long_name = "KeyedLongLongStruct";
constexpr const char* struct_ulong_long_name = "KeyedULongLongStruct";
constexpr const char* struct_float_name = "KeyedFloatStruct";
constexpr const char* struct_double_name = "KeyedDoubleStruct";
constexpr const char* struct_long_double_name = "KeyedLongDoubleStruct";
constexpr const char* struct_boolean_name = "KeyedBooleanStruct";
constexpr const char* struct_byte_name = "KeyedOctetStruct";
constexpr const char* struct_char_name = "KeyedCharStruct";
constexpr const char* struct_wchar_name = "KeyedWCharStruct";
constexpr const char* struct_empty_name = "KeyedEmptyStruct";
constexpr const char* struct_empty_inheritance_name = "KeyedEmptyInheritanceStruct";
constexpr const char* struct_inheritance_name = "KeyedInheritanceStruct";
constexpr const char* struct_inheritance_empty_name = "InheritanceKeyedEmptyStruct";
constexpr const char* struct_final_name = "KeyedFinal";
constexpr const char* struct_appendable_name = "KeyedAppendable";
constexpr const char* struct_mutable_name = "KeyedMutable";

constexpr const char* var_key_short_name = "key_short";
constexpr const char* var_key_ushort_name = "key_ushort";
constexpr const char* var_key_long_name = "key_long";
constexpr const char* var_key_ulong_name = "key_ulong";
constexpr const char* var_key_long_long_name = "key_longlong";
constexpr const char* var_key_ulong_long_name = "key_ulonglong";
constexpr const char* var_key_float_name = "key_float";
constexpr const char* var_key_double_name = "key_double";
constexpr const char* var_key_long_double_name = "key_longdouble";
constexpr const char* var_key_boolean_name = "key_boolean";
constexpr const char* var_key_byte_name = "key_octet";
constexpr const char* var_key_char_name = "key_char8";
constexpr const char* var_key_wchar_name = "key_char16";
constexpr const char* var_key_str_name = "key_str";
constexpr const char* var_var_str_name = "var_str";
constexpr const char* var_key_string_name = "key_string";

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedShortStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_short_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value {2};
    int16_t test_value {0};
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedShortStruct struct_data;
        TypeSupport static_pubsubType {new KeyedShortStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_short(), test_value);
        EXPECT_EQ(struct_data.var_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedShortStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedUShortStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ushort_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_ushort_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_ushort_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint16_t value {2};
    uint16_t test_value {0};
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(var_key_ushort_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(var_key_ushort_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(var_ushort_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(var_ushort_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedUShortStruct struct_data;
        TypeSupport static_pubsubType {new KeyedUShortStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_ushort(), test_value);
        EXPECT_EQ(struct_data.var_ushort(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedUShortStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedLongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value {2};
    int32_t test_value {0};
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_key_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_key_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedLongStruct struct_data;
        TypeSupport static_pubsubType {new KeyedLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_long(), test_value);
        EXPECT_EQ(struct_data.var_long(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedULongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ulong_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_ulong_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_ulong_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint32_t value {2};
    uint32_t test_value {0};
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_key_ulong_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_key_ulong_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_ulong_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_ulong_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedULongStruct struct_data;
        TypeSupport static_pubsubType {new KeyedULongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_ulong(), test_value);
        EXPECT_EQ(struct_data.var_ulong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedULongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedLongLongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_long_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_long_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int64_t value {2};
    int64_t test_value {0};

    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(var_key_long_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(var_key_long_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(var_long_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(var_long_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedLongLongStruct struct_data;
        TypeSupport static_pubsubType {new KeyedLongLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_longlong(), test_value);
        EXPECT_EQ(struct_data.var_longlong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedLongLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedULongLongStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ulong_long_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_ulong_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT64));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_ulong_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint64_t value {2};
    uint64_t test_value {0};
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(var_key_ulong_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(var_key_ulong_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(var_ulong_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(var_ulong_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedULongLongStruct struct_data;
        TypeSupport static_pubsubType {new KeyedULongLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_ulonglong(), test_value);
        EXPECT_EQ(struct_data.var_ulonglong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedULongLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedFloatStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_float_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_float_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT32));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_float_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float value {2};
    float test_value {0};
    EXPECT_EQ(data->set_float32_value(data->get_member_id_by_name(var_key_float_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test_value, data->get_member_id_by_name(var_key_float_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_float32_value(data->get_member_id_by_name(var_float_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test_value, data->get_member_id_by_name(var_float_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedFloatStruct struct_data;
        TypeSupport static_pubsubType {new KeyedFloatStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_float(), test_value);
        EXPECT_EQ(struct_data.var_float(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedFloatStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedDoubleStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_double_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_double_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT64));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_double_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    double value {2};
    double test_value {0};
    EXPECT_EQ(data->set_float64_value(data->get_member_id_by_name(var_key_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test_value, data->get_member_id_by_name(var_key_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_float64_value(data->get_member_id_by_name(var_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test_value, data->get_member_id_by_name(var_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedDoubleStruct struct_data;
        TypeSupport static_pubsubType {new KeyedDoubleStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_double(), test_value);
        EXPECT_EQ(struct_data.var_double(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedDoubleStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedLongDoubleStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_double_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_long_double_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT128));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_double_name);
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT128));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    long double value {2};
    long double test_value {0};
    EXPECT_EQ(data->set_float128_value(data->get_member_id_by_name(var_key_long_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test_value, data->get_member_id_by_name(var_key_long_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_float128_value(data->get_member_id_by_name(var_long_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test_value, data->get_member_id_by_name(var_long_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedLongDoubleStruct struct_data;
        TypeSupport static_pubsubType {new KeyedLongDoubleStructPubSubType()};
        // Experimental analysis showed that long double is mostly accepted to truly
        // use/have 10 bytes across implementations.
        // Compare only those number of bytes of the instance handle.
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType, 10);
        EXPECT_EQ(struct_data.key_longdouble(), test_value);
        EXPECT_EQ(struct_data.var_longdouble(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedLongDoubleStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedBooleanStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_boolean_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_boolean_name);
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->is_key(true);
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bool_name);
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    bool value = true;
    bool test_value = false;
    EXPECT_EQ(data->set_boolean_value(data->get_member_id_by_name(var_key_boolean_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test_value, data->get_member_id_by_name(var_key_boolean_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_boolean_value(data->get_member_id_by_name(var_bool_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test_value, data->get_member_id_by_name(var_bool_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedBooleanStruct alias_data;
        TypeSupport static_pubsubType {new KeyedBooleanStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.key_boolean(), test_value);
        EXPECT_EQ(alias_data.var_boolean(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedBooleanStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedOctetStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_byte_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_byte_name);
    member_descriptor->type(factory->get_primitive_type(TK_BYTE));
    member_descriptor->is_key(true);
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_byte_name);
    member_descriptor->type(factory->get_primitive_type(TK_BYTE));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    eprosima::fastdds::rtps::octet value {2};
    eprosima::fastdds::rtps::octet test_value {0};
    EXPECT_EQ(data->set_byte_value(data->get_member_id_by_name(var_key_byte_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test_value, data->get_member_id_by_name(var_key_byte_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_byte_value(data->get_member_id_by_name(var_byte_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test_value, data->get_member_id_by_name(var_byte_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedOctetStruct alias_data;
        TypeSupport static_pubsubType {new KeyedOctetStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.key_octet(), test_value);
        EXPECT_EQ(alias_data.var_octet(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedOctetStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedCharStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_char_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_char_name);
    member_descriptor->type(factory->get_primitive_type(TK_CHAR8));
    member_descriptor->is_key(true);
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_char_name);
    member_descriptor->type(factory->get_primitive_type(TK_CHAR8));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    char value = 'a';
    char test_value = 'b';
    EXPECT_EQ(data->set_char8_value(data->get_member_id_by_name(var_key_char_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test_value, data->get_member_id_by_name(var_key_char_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_char8_value(data->get_member_id_by_name(var_char_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test_value, data->get_member_id_by_name(var_char_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedCharStruct alias_data;
        TypeSupport static_pubsubType {new KeyedCharStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.key_char8(), test_value);
        EXPECT_EQ(alias_data.var_char8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedCharStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedWCharStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_wchar_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_wchar_name);
    member_descriptor->type(factory->get_primitive_type(TK_CHAR16));
    member_descriptor->is_key(true);
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_wchar_name);
    member_descriptor->type(factory->get_primitive_type(TK_CHAR16));
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    wchar_t value = L'a';
    wchar_t test_value = L'b';
    EXPECT_EQ(data->set_char16_value(data->get_member_id_by_name(var_key_wchar_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test_value, data->get_member_id_by_name(var_key_wchar_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_char16_value(data->get_member_id_by_name(var_wchar_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test_value, data->get_member_id_by_name(var_wchar_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedWCharStruct alias_data;
        TypeSupport static_pubsubType {new KeyedWCharStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.key_char16(), test_value);
        EXPECT_EQ(alias_data.var_char16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedWCharStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedEmptyStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_empty_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value {2};
    int16_t test_value {0};
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        KeyedEmptyStruct struct_data;
        TypeSupport static_pubsubType {new KeyedEmptyStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedEmptyStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedEmptyInheritanceStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_empty_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type base_type {type_builder->build()};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_empty_inheritance_name);
    type_descriptor->base_type(base_type);
    type_builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(type_builder);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_str_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_var_str_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value {2};
    int16_t test_value {0};
    std::string str_value {"my_string"};
    std::string test_str_value;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_key_str_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_key_str_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_var_str_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_var_str_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);

    for (auto encoding : encodings)
    {
        KeyedEmptyInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new KeyedEmptyInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_short(), test_value);
        EXPECT_EQ(struct_data.key_str(), test_str_value);
        EXPECT_EQ(struct_data.var_str(), test_str_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedEmptyInheritanceStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedInheritanceStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_short_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type base_type {type_builder->build()};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_inheritance_name);
    type_descriptor->base_type(base_type);
    type_builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(type_builder);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_str_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_var_str_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value {2};
    int16_t test_value {0};
    std::string str_value {"my_string"};
    std::string test_str_value;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_key_str_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_key_str_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_var_str_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_var_str_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);

    for (auto encoding : encodings)
    {
        KeyedInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new KeyedInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_short(), test_value);
        EXPECT_EQ(struct_data.var_short(), test_value);
        EXPECT_EQ(struct_data.key_str(), test_str_value);
        EXPECT_EQ(struct_data.var_str(), test_str_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedInheritanceStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_InheritanceKeyedEmptyStruct)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_short_name);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type base_type {type_builder->build()};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_inheritance_empty_name);
    type_descriptor->base_type(base_type);
    type_builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(type_builder);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value {2};
    int16_t test_value {0};
    std::string str_value {"my_string"};
    std::string test_str_value;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        InheritanceKeyedEmptyStruct struct_data;
        TypeSupport static_pubsubType {new InheritanceKeyedEmptyStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_short(), test_value);
        EXPECT_EQ(struct_data.var_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_InheritanceKeyedEmptyStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedFinal)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_final_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::FINAL);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->is_key(true);
    member_descriptor->id(2);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    member_descriptor->id(1);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_string_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->is_key(true);
    member_descriptor->id(0);
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t l_value {3};
    int32_t test_l_value {0};
    int16_t s_value {2};
    int16_t test_s_value {0};
    std::string str_value {"my_string"};
    std::string test_str_value;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_key_long_name), l_value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_l_value, data->get_member_id_by_name(var_key_long_name)), RETCODE_OK);
    EXPECT_EQ(l_value, test_l_value);
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), s_value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_s_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(s_value, test_s_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_key_string_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_key_string_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);

    for (auto encoding : encodings)
    {
        KeyedFinal struct_data;
        TypeSupport static_pubsubType {new KeyedFinalPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_long(), test_l_value);
        EXPECT_EQ(struct_data.key_short(), test_s_value);
        EXPECT_EQ(struct_data.key_string(), test_str_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedFinal_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedAppendable)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_appendable_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::APPENDABLE);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->is_key(true);
    member_descriptor->id(2);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    member_descriptor->id(1);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_string_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->is_key(true);
    member_descriptor->id(0);
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t l_value {3};
    int32_t test_l_value {0};
    int16_t s_value {2};
    int16_t test_s_value {0};
    std::string str_value {"my_string"};
    std::string test_str_value;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_key_long_name), l_value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_l_value, data->get_member_id_by_name(var_key_long_name)), RETCODE_OK);
    EXPECT_EQ(l_value, test_l_value);
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), s_value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_s_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(s_value, test_s_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_key_string_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_key_string_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);

    for (auto encoding : encodings)
    {
        KeyedAppendable struct_data;
        TypeSupport static_pubsubType {new KeyedAppendablePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_long(), test_l_value);
        EXPECT_EQ(struct_data.key_short(), test_s_value);
        EXPECT_EQ(struct_data.key_string(), test_str_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedAppendable_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_KeyedMutable)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_mutable_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_key_long_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->is_key(true);
    member_descriptor->id(2);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_short_name);
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->is_key(true);
    member_descriptor->id(1);
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_key_string_name);
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->is_key(true);
    member_descriptor->id(0);
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t l_value {3};
    int32_t test_l_value {0};
    int16_t s_value {2};
    int16_t test_s_value {0};
    std::string str_value {"my_string"};
    std::string test_str_value;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_key_long_name), l_value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_l_value, data->get_member_id_by_name(var_key_long_name)), RETCODE_OK);
    EXPECT_EQ(l_value, test_l_value);
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_key_short_name), s_value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_s_value, data->get_member_id_by_name(var_key_short_name)), RETCODE_OK);
    EXPECT_EQ(s_value, test_s_value);
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_key_string_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_str_value, data->get_member_id_by_name(var_key_string_name)), RETCODE_OK);
    EXPECT_EQ(str_value, test_str_value);

    for (auto encoding : encodings)
    {
        KeyedMutable struct_data;
        TypeSupport static_pubsubType {new KeyedMutablePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.key_long(), test_l_value);
        EXPECT_EQ(struct_data.key_short(), test_s_value);
        EXPECT_EQ(struct_data.key_string(), test_str_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_KeyedMutable_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
