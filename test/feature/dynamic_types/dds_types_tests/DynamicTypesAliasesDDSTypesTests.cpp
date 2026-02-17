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
#include "../../../dds-types-test/helpers/basic_inner_types.hpp"
#include "../../../dds-types-test/aliasesPubSubTypes.hpp"
#include "../../../dds-types-test/aliasesTypeObjectSupport.hpp"
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* struct_alias_short_name = "AliasInt16";
constexpr const char* struct_alias_ushort_name = "AliasUint16";
constexpr const char* struct_alias_long_name = "AliasInt32";
constexpr const char* struct_alias_ulong_name = "AliasUInt32";
constexpr const char* struct_alias_long_long_name = "AliasInt64";
constexpr const char* struct_alias_ulong_long_name = "AliasUInt64";
constexpr const char* struct_alias_float_name = "AliasFloat32";
constexpr const char* struct_alias_double_name = "AliasFloat64";
constexpr const char* struct_alias_long_double_name = "AliasFloat128";
constexpr const char* struct_alias_bool_name = "AliasBool";
constexpr const char* struct_alias_octet_name = "AliasOctet";
constexpr const char* struct_alias_char_name = "AliasChar8";
constexpr const char* struct_alias_wchar_name = "AliasChar16";
constexpr const char* struct_alias_string_name = "AliasString8";
constexpr const char* struct_alias_wstring_name = "AliasString16";
constexpr const char* struct_alias_enum_name = "AliasEnum";
constexpr const char* struct_alias_bitmask_name = "AliasBitmask";
constexpr const char* struct_alias_alias_name = "AliasAlias";
constexpr const char* struct_alias_array_name = "AliasArray";
constexpr const char* struct_alias_multiarray_name = "AliasMultiArray";
constexpr const char* struct_alias_sequence_name = "AliasSequence";
constexpr const char* struct_alias_map_name = "AliasMap";
constexpr const char* struct_alias_union_name = "AliasUnion";
constexpr const char* struct_alias_struct_name = "AliasStruct";
constexpr const char* struct_alias_bitset_name = "AliasBitset";

constexpr const char* alias_short_name = "alias_int16";
constexpr const char* alias_ushort_name = "alias_uint16";
constexpr const char* alias_long_name = "alias_int32";
constexpr const char* alias_ulong_name = "alias_uint32";
constexpr const char* alias_long_long_name = "alias_int64";
constexpr const char* alias_ulong_long_name = "alias_uint64";
constexpr const char* alias_float_name = "alias_float32";
constexpr const char* alias_double_name = "alias_float64";
constexpr const char* alias_long_double_name = "alias_float128";
constexpr const char* alias_bool_name = "alias_bool";
constexpr const char* alias_byte_name = "alias_octet";
constexpr const char* alias_char_name = "alias_char8";
constexpr const char* alias_wchar_name = "alias_char16";
constexpr const char* alias_string_name = "alias_string8";
constexpr const char* alias_wstring_name = "alias_string16";
constexpr const char* alias_enum_name = "alias_enum";
constexpr const char* alias_bitmask_name = "alias_bitmask";
constexpr const char* alias_alias_name = "alias_alias";
constexpr const char* alias_array_name = "alias_array";
constexpr const char* alias_multiarray_name = "alias_multiarray";
constexpr const char* alias_sequence_name = "alias_sequence";
constexpr const char* alias_map_name = "alias_map";
constexpr const char* alias_union_name = "alias_union";
constexpr const char* alias_struct_name = "alias_structure";
constexpr const char* alias_bitset_name = "alias_bitset";

constexpr const char* struct_member_name = "value";

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasInt16)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_short_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_short_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value = 2;
    int16_t test_value = 0;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasInt16 alias_data;
        TypeSupport static_pubsubType {new AliasInt16PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasInt16_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasUint16)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_ushort_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_ushort_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint16_t value = 2;
    uint16_t test_value = 0;
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasUint16 alias_data;
        TypeSupport static_pubsubType {new AliasUint16PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasUint16_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasInt32)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_long_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_long_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasInt32 alias_data;
        TypeSupport static_pubsubType {new AliasInt32PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasInt32_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasUInt32)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_ulong_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_ulong_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint32_t value = 2;
    uint32_t test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasUInt32 alias_data;
        TypeSupport static_pubsubType {new AliasUInt32PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasUInt32_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasInt64)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_long_long_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_long_long_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int64_t value = 2;
    int64_t test_value = 0;
    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasInt64 alias_data;
        TypeSupport static_pubsubType {new AliasInt64PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasInt64_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasUInt64)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_ulong_long_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_ulong_long_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint64_t value = 2;
    uint64_t test_value = 0;
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasUInt64 alias_data;
        TypeSupport static_pubsubType {new AliasUInt64PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasUInt64_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasFloat32)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_float_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_float_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float value = 123.0f;
    float test_value = 0.0f;
    EXPECT_EQ(data->set_float32_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasFloat32 alias_data;
        TypeSupport static_pubsubType {new AliasFloat32PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasFloat32_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasFloat64)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_double_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_double_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT64));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    double value = 123.0f;
    double test_value = 0.0f;
    EXPECT_EQ(data->set_float64_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasFloat64 alias_data;
        TypeSupport static_pubsubType {new AliasFloat64PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasFloat64_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasFloat128)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_long_double_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_long_double_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT128));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    long double value = 123.0f;
    long double test_value = 0.0f;
    EXPECT_EQ(data->set_float128_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasFloat128 alias_data;
        TypeSupport static_pubsubType {new AliasFloat128PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasFloat128_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasBool)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_bool_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_bool_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    bool value = true;
    bool test_value = false;
    EXPECT_EQ(data->set_boolean_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasBool alias_data;
        TypeSupport static_pubsubType {new AliasBoolPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasBool_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_octet_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_byte_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    eprosima::fastdds::rtps::octet value = 255;
    eprosima::fastdds::rtps::octet test_value = 0;
    EXPECT_EQ(data->set_byte_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasOctet alias_data;
        TypeSupport static_pubsubType {new AliasOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasChar8)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_char_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_char_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    char value = 'a';
    char test_value = 'b';
    EXPECT_EQ(data->set_char8_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasChar8 alias_data;
        TypeSupport static_pubsubType {new AliasChar8PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasChar8_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasChar16)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_wchar_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_wchar_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR16));
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    wchar_t value = L'a';
    wchar_t test_value = L'b';
    EXPECT_EQ(data->set_char16_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasChar16 alias_data;
        TypeSupport static_pubsubType {new AliasChar16PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasChar16_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasString8)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_string_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_string_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_string_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string value = "STRING_TEST";
    std::string test_value = "";
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasString8 alias_data;
        TypeSupport static_pubsubType {new AliasString8PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasString8_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasString16)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_wstring_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_wstring_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_wstring_type(
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::wstring value = L"STRING_TEST";
    std::wstring test_value = L"";
    EXPECT_EQ(data->set_wstring_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasString16 alias_data;
        TypeSupport static_pubsubType {new AliasString16PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasString16_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasEnum)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_enum_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_enum_name);
    alias_descriptor->base_type(create_inner_enum_helper());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerEnumHelper value = InnerEnumHelper::ENUM_VALUE_2;
    int32_t test_value {0};
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(
                struct_member_name), static_cast<int32_t>(value)), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(static_cast<int32_t>(value), test_value);

    for (auto encoding : encodings)
    {
        AliasEnum alias_data;
        TypeSupport static_pubsubType {new AliasEnumPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(static_cast<int32_t>(alias_data.value()), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasEnum_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasBitmask)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_bitmask_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_bitmask_name);
    alias_descriptor->base_type(create_inner_bitmask_helper());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerBitMaskHelper value = InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1 |
            InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6;
    InnerBitMaskHelper test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasBitmask alias_data;
        TypeSupport static_pubsubType {new AliasBitmaskPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasBitmask_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasAlias)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_alias_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_alias_name);
    alias_descriptor->base_type(create_inner_alias_helper());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasAlias alias_data;
        TypeSupport static_pubsubType {new AliasAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasArray)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_array_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_array_name);
    BoundSeq bound {2};
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), bound)->build());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {2, 5};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasArray alias_data;
        TypeSupport static_pubsubType {new AliasArrayPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(test_value.size(), alias_data.value().size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(alias_data.value()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasArray_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasMultiArray)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_multiarray_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_multiarray_name);
    BoundSeq bound {2, 2};
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), bound)->build());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {2, 5, 3, 4};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasMultiArray alias_data;
        TypeSupport static_pubsubType {new AliasMultiArrayPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(test_value.size(), alias_data.value().size() * alias_data.value()[0].size());
        for (size_t i = 0; i < alias_data.value().size(); ++i)
        {
            for (size_t j = 0; j < alias_data.value()[i].size(); ++j)
            {
                EXPECT_EQ(alias_data.value()[i][j], test_value[i * alias_data.value().size() + j]);
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasMultiArray_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasSequence)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_sequence_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_sequence_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16),
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {2, 5, 3, 4};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(struct_member_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(struct_member_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        AliasSequence alias_data;
        TypeSupport static_pubsubType {new AliasSequencePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasSequence_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_map_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_map_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_map_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16),
            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t first_key = 1;
    int16_t second_key = 5;
    int16_t first_value = 142;
    int16_t second_value = 421;
    int16_t test_value = 10;
    DynamicData::_ref_type map_data = data->loan_value(data->get_member_id_by_name(struct_member_name));
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(
                first_key)), first_value), RETCODE_OK);
    EXPECT_EQ(map_data->set_int16_value(map_data->get_member_id_by_name(std::to_string(
                second_key)), second_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                first_key))), RETCODE_OK);
    EXPECT_EQ(test_value, first_value);
    EXPECT_EQ(map_data->get_int16_value(test_value, map_data->get_member_id_by_name(std::to_string(
                second_key))), RETCODE_OK);
    EXPECT_EQ(test_value, second_value);
    EXPECT_EQ(data->return_loaned_value(map_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        AliasMap alias_data;
        TypeSupport static_pubsubType {new AliasMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        map_data = data->loan_value(data->get_member_id_by_name(struct_member_name));
        ASSERT_TRUE(map_data);
        EXPECT_EQ(alias_data.value().size(), map_data->get_item_count());
        EXPECT_EQ(alias_data.value()[first_key], first_value);
        EXPECT_EQ(alias_data.value()[second_key], second_value);
        EXPECT_EQ(data->return_loaned_value(map_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasUnion)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_union_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_union_name);
    alias_descriptor->base_type(create_inner_union_helper());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float float_value = 23.5;
    float test_float_value = 0;
    int16_t short_value = 55;
    int16_t test_short_value = 0;
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(struct_member_name));
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_float32_value(union_data->get_member_id_by_name(
                union_float_member_name), float_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float32_value(test_float_value, union_data->get_member_id_by_name(
                union_float_member_name)), RETCODE_OK);
    EXPECT_EQ(float_value, test_float_value);
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                union_short_member_name), short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_short_value, union_data->get_member_id_by_name(
                union_short_member_name)), RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        AliasUnion alias_data;
        TypeSupport static_pubsubType {new AliasUnionPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value().shortValue(), test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasUnion_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_struct_name);
    alias_descriptor->base_type(create_inner_struct_helper());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float float_value = 23.5;
    float test_float_value = 0;
    int32_t long_value = 55;
    int32_t test_long_value = 0;
    DynamicData::_ref_type struct_data = data->loan_value(data->get_member_id_by_name(struct_member_name));
    ASSERT_TRUE(struct_data);
    EXPECT_EQ(struct_data->set_float32_value(struct_data->get_member_id_by_name(
                struct_float_member_name), float_value), RETCODE_OK);
    EXPECT_EQ(struct_data->get_float32_value(test_float_value,
            struct_data->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
    EXPECT_EQ(float_value, test_float_value);
    EXPECT_EQ(struct_data->set_int32_value(struct_data->get_member_id_by_name(
                struct_long_member_name), long_value), RETCODE_OK);
    EXPECT_EQ(struct_data->get_int32_value(test_long_value, struct_data->get_member_id_by_name(
                struct_long_member_name)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(data->return_loaned_value(struct_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        AliasStruct alias_data;
        TypeSupport static_pubsubType {new AliasStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value().field1(), test_long_value);
        EXPECT_EQ(alias_data.value().field2(), test_float_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_AliasBitset)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_bitset_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(struct_member_name);

    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_bitset_name);
    alias_descriptor->base_type(create_inner_bitset_helper());
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build());
    ASSERT_EQ(RETCODE_OK, type_builder->add_member(member_descriptor));

    DynamicType::_ref_type struct_type {type_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint8_t uint8_value = 5;
    uint8_t test_uint8_value = 0;
    bool bool_value = true;
    bool test_bool_value = false;
    uint16_t ushort_value = 1000;
    uint16_t test_ushort_value = 0;
    int16_t short_value = 2000;
    int16_t test_short_value = 0;
    DynamicData::_ref_type bitset_data = data->loan_value(data->get_member_id_by_name(struct_member_name));
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), uint8_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(bitfield_b), bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(bitfield_c), ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(bitfield_d), short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        AliasBitset alias_data;
        TypeSupport static_pubsubType {new AliasBitsetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, alias_data, static_pubsubType);
        EXPECT_EQ(alias_data.value().a, test_uint8_value);
        EXPECT_EQ(alias_data.value().b, test_bool_value);
        EXPECT_EQ(alias_data.value().c, test_ushort_value);
        EXPECT_EQ(alias_data.value().d, test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_AliasBitset_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
