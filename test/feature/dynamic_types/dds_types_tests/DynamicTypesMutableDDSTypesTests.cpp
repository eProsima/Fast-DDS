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
#include "../../../dds-types-test/helpers/basic_inner_types.hpp"
#include "../../../dds-types-test/mutablePubSubTypes.hpp"
#include "../../../dds-types-test/mutableTypeObjectSupport.hpp"
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

constexpr const char* const mutable_short_struct_name = "MutableShortStruct";
constexpr const char* const mutable_ushort_struct_name = "MutableUShortStruct";
constexpr const char* const mutable_long_struct_name = "MutableLongStruct";
constexpr const char* const mutable_ulong_struct_name = "MutableULongStruct";
constexpr const char* const mutable_longlong_struct_name = "MutableLongLongStruct";
constexpr const char* const mutable_ulonglong_struct_name = "MutableULongLongStruct";
constexpr const char* const mutable_float_struct_name = "MutableFloatStruct";
constexpr const char* const mutable_double_struct_name = "MutableDoubleStruct";
constexpr const char* const mutable_longdouble_struct_name = "MutableLongDoubleStruct";
constexpr const char* const mutable_bool_struct_name = "MutableBooleanStruct";
constexpr const char* const mutable_byte_struct_name = "MutableOctetStruct";
constexpr const char* const mutable_char_struct_name = "MutableCharStruct";
constexpr const char* const mutable_wchar_struct_name = "MutableWCharStruct";
constexpr const char* const mutable_union_struct_name = "MutableUnionStruct";
constexpr const char* const mutable_empty_struct_name = "MutableEmptyStruct";
constexpr const char* const mutable_emptyinheritance_struct_name = "MutableEmptyInheritanceStruct";
constexpr const char* const mutable_inheritance_struct_name = "MutableInheritanceStruct";
constexpr const char* const mutable_inheritanceempty_struct_name = "MutableInheritanceEmptyStruct";
constexpr const char* const mutable_extensibilityinheritance_struct_name = "MutableExtensibilityInheritance";

DynamicType::_ref_type create_mutable_short_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_short_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_mutable_empty_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_empty_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    return type_builder->build();
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableShortStruct)
{
    DynamicType::_ref_type struct_type = create_mutable_short_struct();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value = 2;
    int16_t test_value = 0;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableShortStruct struct_data;
        TypeSupport static_pubsubType {new MutableShortStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableShortStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableUShortStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_ushort_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint16_t value = 2;
    uint16_t test_value = 0;
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(var_ushort_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(var_ushort_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableUShortStruct struct_data;
        TypeSupport static_pubsubType {new MutableUShortStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_ushort(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableUShortStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableLongStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_long_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableLongStruct struct_data;
        TypeSupport static_pubsubType {new MutableLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_long(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableULongStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_ulong_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint32_t value = 2;
    uint32_t test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_ulong_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_ulong_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableULongStruct struct_data;
        TypeSupport static_pubsubType {new MutableULongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_ulong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableULongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableLongLongStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_longlong_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int64_t value = 2;
    int64_t test_value = 0;
    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(var_long_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(var_long_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableLongLongStruct struct_data;
        TypeSupport static_pubsubType {new MutableLongLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_longlong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableLongLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableULongLongStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_ulonglong_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint64_t value = 2;
    uint64_t test_value = 0;
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(var_ulong_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(var_ulong_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableULongLongStruct struct_data;
        TypeSupport static_pubsubType {new MutableULongLongStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_ulonglong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableULongLongStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableFloatStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_float_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_float_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float value = 2;
    float test_value = 0;
    EXPECT_EQ(data->set_float32_value(data->get_member_id_by_name(var_float_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test_value, data->get_member_id_by_name(var_float_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableFloatStruct struct_data;
        TypeSupport static_pubsubType {new MutableFloatStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_float(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableFloatStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableDoubleStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_double_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_double_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT64));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    double value = 2;
    double test_value = 0;
    EXPECT_EQ(data->set_float64_value(data->get_member_id_by_name(var_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test_value, data->get_member_id_by_name(var_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableDoubleStruct struct_data;
        TypeSupport static_pubsubType {new MutableDoubleStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_double(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableDoubleStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableLongDoubleStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_longdouble_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_double_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT128));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    long double value = 2;
    long double test_value = 0;
    EXPECT_EQ(data->set_float128_value(data->get_member_id_by_name(var_long_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test_value, data->get_member_id_by_name(var_long_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableLongDoubleStruct struct_data;
        TypeSupport static_pubsubType {new MutableLongDoubleStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_longdouble(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableLongDoubleStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableBooleanStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_bool_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bool_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    bool value = true;
    bool test_value = false;
    EXPECT_EQ(data->set_boolean_value(data->get_member_id_by_name(var_bool_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test_value, data->get_member_id_by_name(var_bool_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableBooleanStruct struct_data;
        TypeSupport static_pubsubType {new MutableBooleanStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_boolean(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableBooleanStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableOctetStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_byte_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_byte_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    eprosima::fastdds::rtps::octet value = 255;
    eprosima::fastdds::rtps::octet test_value = 0;
    EXPECT_EQ(data->set_byte_value(data->get_member_id_by_name(var_byte_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test_value, data->get_member_id_by_name(var_byte_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableOctetStruct struct_data;
        TypeSupport static_pubsubType {new MutableOctetStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_octet(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableOctetStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableCharStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_char_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    char value = 'a';
    char test_value = 'b';
    EXPECT_EQ(data->set_char8_value(data->get_member_id_by_name(var_char_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test_value, data->get_member_id_by_name(var_char_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableCharStruct struct_data;
        TypeSupport static_pubsubType {new MutableCharStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_char8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableCharStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableWCharStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_wchar_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wchar_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR16));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    wchar_t value = L'a';
    wchar_t test_value = L'b';
    EXPECT_EQ(data->set_char16_value(data->get_member_id_by_name(var_wchar_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test_value, data->get_member_id_by_name(var_wchar_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        MutableWCharStruct struct_data;
        TypeSupport static_pubsubType {new MutableWCharStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_char16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableWCharStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableUnionStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_union_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_union_helper());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float float_value = 23.5;
    float test_float_value = 0;
    int16_t short_value = 55;
    int16_t test_short_value = 0;
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_name));
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
        MutableUnionStruct struct_data;
        TypeSupport static_pubsubType {new MutableUnionStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union().shortValue(), test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableUnionStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableEmptyStruct)
{
    DynamicType::_ref_type struct_type = create_mutable_empty_struct();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    for (auto encoding : encodings)
    {
        MutableEmptyStruct struct_data;
        TypeSupport static_pubsubType {new MutableEmptyStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableEmptyStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableEmptyInheritanceStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_emptyinheritance_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    type_descriptor->base_type(create_mutable_empty_struct());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_str_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string str_value = "STRING_TEST";
    std::string str_test_value = "";
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_str_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(str_test_value, data->get_member_id_by_name(var_str_name)), RETCODE_OK);
    EXPECT_EQ(str_value, str_test_value);

    for (auto encoding : encodings)
    {
        MutableEmptyInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new MutableEmptyInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_str(), str_test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableEmptyInheritanceStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableInheritanceStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_inheritance_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    type_descriptor->base_type(create_mutable_short_struct());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_str_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t short_value = 2;
    int16_t short_test_value = 0;
    std::string str_value = "STRING_TEST";
    std::string str_test_value = "";
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), short_value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(short_test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(short_value, short_test_value);

    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_str_name), str_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(str_test_value, data->get_member_id_by_name(var_str_name)), RETCODE_OK);
    EXPECT_EQ(str_value, str_test_value);

    for (auto encoding : encodings)
    {
        MutableInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new MutableInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_short(), short_test_value);
        EXPECT_EQ(struct_data.var_str(), str_test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableInheritanceStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableInheritanceEmptyStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_inheritanceempty_struct_name);
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    type_descriptor->base_type(create_mutable_short_struct());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t short_value = 2;
    int16_t short_test_value = 0;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), short_value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(short_test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(short_value, short_test_value);

    for (auto encoding : encodings)
    {
        MutableInheritanceEmptyStruct struct_data;
        TypeSupport static_pubsubType {new MutableInheritanceEmptyStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_short(), short_test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableInheritanceEmptyStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_MutableExtensibilityInheritance)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(mutable_extensibilityinheritance_struct_name);
    type_descriptor->base_type(create_mutable_short_struct());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t short_value = 2;
    int16_t short_test_value = 0;
    int32_t long_value = 443;
    int32_t long_test_value = 0;

    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), short_value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(short_test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(short_value, short_test_value);

    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_long_name), long_value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(long_test_value, data->get_member_id_by_name(var_long_name)), RETCODE_OK);
    EXPECT_EQ(long_value, long_test_value);

    for (auto encoding : encodings)
    {
        MutableExtensibilityInheritance struct_data;
        TypeSupport static_pubsubType {new MutableExtensibilityInheritancePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_short(), short_test_value);
        EXPECT_EQ(struct_data.var_long(), long_test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_MutableExtensibilityInheritance_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
