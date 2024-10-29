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
#include "../../../dds-types-test/helpers/basic_inner_typesPubSubTypes.hpp"
#include "../../../dds-types-test/sequencesPubSubTypes.hpp"
#include "../../../dds-types-test/sequencesTypeObjectSupport.hpp"
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* short_seq_struct_name = "SequenceShort";
constexpr const char* ushort_seq_struct_name = "SequenceUShort";
constexpr const char* long_seq_struct_name = "SequenceLong";
constexpr const char* ulong_seq_struct_name = "SequenceULong";
constexpr const char* longlong_seq_struct_name = "SequenceLongLong";
constexpr const char* ulonglong_seq_struct_name = "SequenceULongLong";
constexpr const char* float_seq_struct_name = "SequenceFloat";
constexpr const char* double_seq_struct_name = "SequenceDouble";
constexpr const char* longdouble_seq_struct_name = "SequenceLongDouble";
constexpr const char* bool_seq_struct_name = "SequenceBoolean";
constexpr const char* byte_seq_struct_name = "SequenceOctet";
constexpr const char* char_seq_struct_name = "SequenceChar";
constexpr const char* wchar_seq_struct_name = "SequenceWChar";
constexpr const char* string_seq_struct_name = "SequenceString";
constexpr const char* wstring_seq_struct_name = "SequenceWString";
constexpr const char* bounded_string_seq_struct_name = "SequenceStringBounded";
constexpr const char* bounded_wstring_seq_struct_name = "SequenceWStringBounded";
constexpr const char* enum_seq_struct_name = "SequenceEnum";
constexpr const char* bitmask_seq_struct_name = "SequenceBitMask";
constexpr const char* alias_seq_struct_name = "SequenceAlias";
constexpr const char* array_seq_struct_name = "SequenceShortArray";
constexpr const char* seq_seq_struct_name = "SequenceSequence";
constexpr const char* map_seq_struct_name = "SequenceMap";
constexpr const char* union_seq_struct_name = "SequenceUnion";
constexpr const char* struct_seq_struct_name = "SequenceStructure";
constexpr const char* bitset_seq_struct_name = "SequenceBitset";
constexpr const char* small_bounded_seq_struct_name = "BoundedSmallSequences";
constexpr const char* large_bounded_seq_struct_name = "BoundedBigSequences";
constexpr const char* my_structure_struct_name = "NoCommon_Module::My_Structure";

constexpr const char* var_short_seq = "var_sequence_short";
constexpr const char* var_ushort_seq = "var_sequence_ushort";
constexpr const char* var_long_seq = "var_sequence_long";
constexpr const char* var_ulong_seq = "var_sequence_ulong";
constexpr const char* var_longlong_seq = "var_sequence_longlong";
constexpr const char* var_ulonglong_seq = "var_sequence_ulonglong";
constexpr const char* var_float_seq = "var_sequence_float";
constexpr const char* var_double_seq = "var_sequence_double";
constexpr const char* var_longdouble_seq = "var_sequence_longdouble";
constexpr const char* var_bool_seq = "var_sequence_boolean";
constexpr const char* var_byte_seq = "var_sequence_octet";
constexpr const char* var_char_seq = "var_sequence_char";
constexpr const char* var_wchar_seq = "var_sequence_wchar";
constexpr const char* var_string_seq = "var_sequence_string";
constexpr const char* var_wstring_seq = "var_sequence_wstring";
constexpr const char* var_bounded_string_seq = "var_sequence_bounded_string";
constexpr const char* var_bounded_wstring_seq = "var_sequence_bounded_wstring";
constexpr const char* var_enum_seq = "var_sequence_enum";
constexpr const char* var_bitmask_seq = "var_sequence_bitmask";
constexpr const char* var_alias_seq = "var_sequence_alias";
constexpr const char* var_array_seq = "var_sequence_short_array";
constexpr const char* var_seq_seq = "var_sequence_sequence";
constexpr const char* var_map_seq = "var_sequence_map";
constexpr const char* var_union_seq = "var_sequence_union";
constexpr const char* var_struct_seq = "var_sequence_structure";
constexpr const char* var_bitset_seq = "var_sequence_bitset";
constexpr const char* var_small_bounded_short_seq = "var_sequence_small";
constexpr const char* var_small_bounded_string_seq = "var_unbounded_string_small_bounded_sequence";
constexpr const char* var_large_bounded_short_seq = "var_sequence_big";
constexpr const char* var_large_bounded_string_seq = "var_unbounded_string_large_bounded_sequence";
constexpr const char* var_list = "list";

constexpr const char* alias_my_char_name = "Common_Module::My_Char";
constexpr const char* alias_my_string_name = "Common_Module::My_String";
constexpr const char* alias_my_aliasstring_name = "NoCommon_Module::My_AliasString";
constexpr const char* alias_my_sequencestring_name = "NoCommon_Module::My_SequenceString";

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {0, -100, 1000, 10000, 200, -20000};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_short_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(var_short_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceShort struct_data;
        TypeSupport static_pubsubType {new SequenceShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT16),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt16Seq value = {0, 100, 1000, 10000, 200, 20000, 555, 1234};
    UInt16Seq test_value;
    EXPECT_EQ(data->set_uint16_values(data->get_member_id_by_name(var_ushort_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(test_value, data->get_member_id_by_name(var_ushort_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceUShort struct_data;
        TypeSupport static_pubsubType {new SequenceUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_ushort(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {0, 100, 1000, 10000, 200, 20000, 555, 1234, -123456789};
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_long_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_long_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceLong struct_data;
        TypeSupport static_pubsubType {new SequenceLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_long(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT32),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt32Seq value = {0, 100, 1000, 10000, 200, 20000, 555, 1234, 123456789};
    UInt32Seq test_value;
    EXPECT_EQ(data->set_uint32_values(data->get_member_id_by_name(var_ulong_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(test_value, data->get_member_id_by_name(var_ulong_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceULong struct_data;
        TypeSupport static_pubsubType {new SequenceULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_ulong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT64),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int64Seq value = {0, 100, 1000, 10000, 200, 20000, 555, 1234, -123456789090};
    Int64Seq test_value;
    EXPECT_EQ(data->set_int64_values(data->get_member_id_by_name(var_longlong_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(test_value, data->get_member_id_by_name(var_longlong_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceLongLong struct_data;
        TypeSupport static_pubsubType {new SequenceLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_longlong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT64),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt64Seq value = {0, 100, 1000, 10000, 200, 20000, 555, 1234, 123456789090};
    UInt64Seq test_value;
    EXPECT_EQ(data->set_uint64_values(data->get_member_id_by_name(var_ulonglong_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(test_value, data->get_member_id_by_name(var_ulonglong_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceULongLong struct_data;
        TypeSupport static_pubsubType {new SequenceULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_ulonglong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(float_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_float_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT32),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float32Seq value {0.5f, 1.1f, 13.12f, 5.67884f};
    Float32Seq test_value;
    EXPECT_EQ(data->set_float32_values(data->get_member_id_by_name(var_float_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(test_value, data->get_member_id_by_name(var_float_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceFloat struct_data;
        TypeSupport static_pubsubType {new SequenceFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_float(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(double_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_double_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT64),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float64Seq value = {0.5, 1.1, 13.12, 5.67884, 0.0000001032, -1000000.0001};
    Float64Seq test_value;
    EXPECT_EQ(data->set_float64_values(data->get_member_id_by_name(var_double_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(test_value, data->get_member_id_by_name(var_double_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceDouble struct_data;
        TypeSupport static_pubsubType {new SequenceDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_double(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longdouble_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longdouble_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT128),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float128Seq value = {0.5, 1.1, 13.12, 5.67884, 0.0000001032, -1000000.0001};
    Float128Seq test_value;
    EXPECT_EQ(data->set_float128_values(data->get_member_id_by_name(var_longdouble_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(test_value, data->get_member_id_by_name(var_longdouble_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceLongDouble struct_data;
        TypeSupport static_pubsubType {new SequenceLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_longdouble(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bool_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bool_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_BOOLEAN),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    BooleanSeq value = {true, false, true, true, false, true, false, false};
    BooleanSeq test_value;
    EXPECT_EQ(data->set_boolean_values(data->get_member_id_by_name(var_bool_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_values(test_value, data->get_member_id_by_name(var_bool_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceBoolean struct_data;
        TypeSupport static_pubsubType {new SequenceBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_boolean(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(byte_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_byte_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_BYTE),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    ByteSeq value = {255, 123, 54, 12, 0, 128, 254};
    ByteSeq test_value;
    EXPECT_EQ(data->set_byte_values(data->get_member_id_by_name(var_byte_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_values(test_value, data->get_member_id_by_name(var_byte_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceOctet struct_data;
        TypeSupport static_pubsubType {new SequenceOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_octet(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(char_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_CHAR8),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    CharSeq value = {'e', 'P', 'r', 'o', 's', 'i', 'm', 'a'};
    CharSeq test_value;
    EXPECT_EQ(data->set_char8_values(data->get_member_id_by_name(var_char_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_values(test_value, data->get_member_id_by_name(var_char_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceChar struct_data;
        TypeSupport static_pubsubType {new SequenceCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_char(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(wchar_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wchar_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_CHAR16),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WcharSeq value = {L'e', L'P', L'r', L'o', L's', L'i', L'm', L'a'};
    WcharSeq test_value;
    EXPECT_EQ(data->set_char16_values(data->get_member_id_by_name(var_wchar_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(test_value, data->get_member_id_by_name(var_wchar_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceWChar struct_data;
        TypeSupport static_pubsubType {new SequenceWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_wchar(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->create_string_type(static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->
                    build(),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StringSeq value = {"Hello", ",", "how", "are", "you", "?"};
    StringSeq test_value;
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(var_string_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_value, data->get_member_id_by_name(var_string_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceString struct_data;
        TypeSupport static_pubsubType {new SequenceStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_string(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(wstring_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wstring_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->create_wstring_type(static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))
                    ->build(),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WstringSeq value = {L"Hello", L",", L"how", L"are", L"you", L"?"};
    WstringSeq test_value;
    EXPECT_EQ(data->set_wstring_values(data->get_member_id_by_name(var_wstring_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_values(test_value, data->get_member_id_by_name(var_wstring_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceWString struct_data;
        TypeSupport static_pubsubType {new SequenceWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_wstring(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceStringBounded)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_string_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_string_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                create_inner_alias_bounded_string_helper(),
                static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StringSeq value = {"Hello", ",", "how", "are", "you", "?"};
    StringSeq test_value;
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(var_bounded_string_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_value, data->get_member_id_by_name(var_bounded_string_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceStringBounded struct_data;
        TypeSupport static_pubsubType {new SequenceStringBoundedPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_bounded_string().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_sequence_bounded_string()[i], test_value[i].c_str());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceStringBounded_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceWStringBounded)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_wstring_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_wstring_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                create_inner_alias_bounded_wstring_helper(),
                static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WstringSeq value = {L"Hello", L",", L"how", L"are", L"you", L"?"};
    WstringSeq test_value;
    EXPECT_EQ(data->set_wstring_values(data->get_member_id_by_name(var_bounded_wstring_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_values(test_value, data->get_member_id_by_name(var_bounded_wstring_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceWStringBounded struct_data;
        TypeSupport static_pubsubType {new SequenceWStringBoundedPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_bounded_wstring().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_sequence_bounded_wstring()[i], test_value[i].c_str());
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceWStringBounded_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceEnum)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(enum_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_enum_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(create_inner_enum_helper(),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value =
    {static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
     static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
     static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3)};
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_enum_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_enum_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceEnum struct_data;
        TypeSupport static_pubsubType {new SequenceEnumPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_enum().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(static_cast<int32_t>(struct_data.var_sequence_enum()[i]), test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceEnum_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceBitMask)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitmask_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitmask_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(create_inner_bitmask_helper(),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt32Seq value =
    {InnerBitMaskHelperBits::flag0, InnerBitMaskHelperBits::flag1,
     InnerBitMaskHelperBits::flag6 | InnerBitMaskHelperBits::flag0,
     InnerBitMaskHelperBits::flag4};
    UInt32Seq test_value;
    EXPECT_EQ(data->set_uint32_values(data->get_member_id_by_name(var_bitmask_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(test_value, data->get_member_id_by_name(var_bitmask_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceBitMask struct_data;
        TypeSupport static_pubsubType {new SequenceBitMaskPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_bitmask(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceBitMask_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceAlias)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(alias_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_alias_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(create_inner_alias_helper(),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {10, -100, 1000, -10000, 100000, -1000000};
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_alias_seq), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_alias_seq)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        SequenceAlias struct_data;
        TypeSupport static_pubsubType {new SequenceAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_alias(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceShortArray)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(array_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_array_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                create_inner_alias_array_helper(),
                static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq first_array_value = {10, -100};
    Int16Seq second_array_value = {200, -20};
    Int16Seq test_value;
    DynamicData::_ref_type seq_data = data->loan_value(data->get_member_id_by_name(var_array_seq));
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int16_values(0, first_array_value), RETCODE_OK);
    EXPECT_EQ(seq_data->get_int16_values(test_value, 0), RETCODE_OK);
    EXPECT_EQ(first_array_value, test_value);
    EXPECT_EQ(seq_data->set_int16_values(1, second_array_value), RETCODE_OK);
    EXPECT_EQ(seq_data->get_int16_values(test_value, 1), RETCODE_OK);
    EXPECT_EQ(second_array_value, test_value);
    EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        SequenceShortArray struct_data;
        TypeSupport static_pubsubType {new SequenceShortArrayPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        seq_data = data->loan_value(data->get_member_id_by_name(var_array_seq));
        EXPECT_EQ(struct_data.var_sequence_short_array().size(), seq_data->get_item_count());
        for (size_t i = 0; i < first_array_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_sequence_short_array()[0][i], first_array_value[i]);
        }
        for (size_t i = 0; i < second_array_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_sequence_short_array()[1][i], second_array_value[i]);
        }
        EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceShortArray_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceSequence)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(seq_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_seq_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                create_inner_alias_sequence_helper(),
                static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq first_seq_value = {10, -100, 1000};
    Int16Seq second_seq_value = {200, -20};
    Int16Seq test_value;
    DynamicData::_ref_type seq_data = data->loan_value(data->get_member_id_by_name(var_seq_seq));
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int16_values(0, first_seq_value), RETCODE_OK);
    EXPECT_EQ(seq_data->get_int16_values(test_value, 0), RETCODE_OK);
    EXPECT_EQ(first_seq_value, test_value);
    EXPECT_EQ(seq_data->set_int16_values(1, second_seq_value), RETCODE_OK);
    EXPECT_EQ(seq_data->get_int16_values(test_value, 1), RETCODE_OK);
    EXPECT_EQ(second_seq_value, test_value);
    EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        SequenceSequence struct_data;
        TypeSupport static_pubsubType {new SequenceSequencePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        seq_data = data->loan_value(data->get_member_id_by_name(var_seq_seq));
        EXPECT_EQ(struct_data.var_sequence_sequence().size(), seq_data->get_item_count());
        EXPECT_EQ(struct_data.var_sequence_sequence()[0], first_seq_value);
        EXPECT_EQ(struct_data.var_sequence_sequence()[1], second_seq_value);
        EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceSequence_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(map_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_map_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                create_inner_alias_map_helper(),
                static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_key = 121;
    int32_t second_key = -11;
    int32_t third_key = 1001;
    int32_t first_value = 1;
    int32_t second_value = 2;
    int32_t third_value = 3;
    int32_t test_value = 0;
    DynamicData::_ref_type seq_data = data->loan_value(data->get_member_id_by_name(var_map_seq));
    ASSERT_TRUE(seq_data);
    DynamicData::_ref_type map_data = seq_data->loan_value(0);
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                first_key)), first_value), RETCODE_OK);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                second_key)), second_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                first_key))), RETCODE_OK);
    EXPECT_EQ(test_value, first_value);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                second_key))), RETCODE_OK);
    EXPECT_EQ(test_value, second_value);
    EXPECT_EQ(seq_data->return_loaned_value(map_data), RETCODE_OK);
    map_data = seq_data->loan_value(1);
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                third_key)), third_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                third_key))), RETCODE_OK);
    EXPECT_EQ(test_value, third_value);
    EXPECT_EQ(seq_data->return_loaned_value(map_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        SequenceMap struct_data;
        TypeSupport static_pubsubType {new SequenceMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        seq_data = data->loan_value(data->get_member_id_by_name(var_map_seq));
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(struct_data.var_sequence_map().size(), seq_data->get_item_count());
        map_data = seq_data->loan_value(0);
        ASSERT_TRUE(map_data);
        EXPECT_EQ(struct_data.var_sequence_map()[0].size(), map_data->get_item_count());
        EXPECT_EQ(struct_data.var_sequence_map()[0][first_key], first_value);
        EXPECT_EQ(struct_data.var_sequence_map()[0][second_key], second_value);
        EXPECT_EQ(seq_data->return_loaned_value(map_data), RETCODE_OK);
        map_data = seq_data->loan_value(1);
        ASSERT_TRUE(map_data);
        EXPECT_EQ(struct_data.var_sequence_map()[1].size(), map_data->get_item_count());
        EXPECT_EQ(struct_data.var_sequence_map()[1][third_key], third_value);
        EXPECT_EQ(seq_data->return_loaned_value(map_data), RETCODE_OK);
        EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceUnion)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(union_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(create_inner_union_helper(),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t long_value = 121;
    int32_t test_long_value = 0;
    float float_value = 10.01f;
    float test_float_value = 0;
    int16_t short_value = -2;
    int16_t test_short_value = 0;
    DynamicData::_ref_type seq_data = data->loan_value(data->get_member_id_by_name(var_union_seq));
    ASSERT_TRUE(seq_data);
    DynamicData::_ref_type union_data = seq_data->loan_value(0);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                union_long_member_name), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                union_long_member_name)), RETCODE_OK);
    EXPECT_EQ(test_long_value, long_value);
    EXPECT_EQ(seq_data->return_loaned_value(union_data), RETCODE_OK);
    union_data = seq_data->loan_value(1);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_float32_value(union_data->get_member_id_by_name(
                union_float_member_name), float_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float32_value(test_float_value, union_data->get_member_id_by_name(
                union_float_member_name)), RETCODE_OK);
    EXPECT_EQ(test_float_value, float_value);
    EXPECT_EQ(seq_data->return_loaned_value(union_data), RETCODE_OK);
    union_data = seq_data->loan_value(2);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                union_short_member_name), short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_short_value, union_data->get_member_id_by_name(
                union_short_member_name)), RETCODE_OK);
    EXPECT_EQ(test_short_value, short_value);
    EXPECT_EQ(seq_data->return_loaned_value(union_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        SequenceUnion struct_data;
        TypeSupport static_pubsubType {new SequenceUnionPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        seq_data = data->loan_value(data->get_member_id_by_name(var_union_seq));
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(struct_data.var_sequence_union().size(), seq_data->get_item_count());
        EXPECT_EQ(struct_data.var_sequence_union()[0].longValue(), long_value);
        EXPECT_EQ(struct_data.var_sequence_union()[1].floatValue(), float_value);
        EXPECT_EQ(struct_data.var_sequence_union()[2].shortValue(), short_value);
        EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceUnion_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_struct_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(create_inner_struct_helper(),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_long_value = 121;
    int32_t second_long_value = 10001;
    int32_t test_long_value = 10001;
    float first_float_value = 10.01f;
    float second_float_value = 3.14f;
    float test_float_value = 0;
    DynamicData::_ref_type seq_data = data->loan_value(data->get_member_id_by_name(var_struct_seq));
    ASSERT_TRUE(seq_data);
    DynamicData::_ref_type data_struct = seq_data->loan_value(0);
    ASSERT_TRUE(data_struct);
    EXPECT_EQ(data_struct->set_int32_value(data_struct->get_member_id_by_name(
                struct_long_member_name), first_long_value), RETCODE_OK);
    EXPECT_EQ(data_struct->get_int32_value(test_long_value, data_struct->get_member_id_by_name(
                struct_long_member_name)), RETCODE_OK);
    EXPECT_EQ(test_long_value, first_long_value);
    EXPECT_EQ(data_struct->set_float32_value(data_struct->get_member_id_by_name(struct_float_member_name),
            first_float_value), RETCODE_OK);
    EXPECT_EQ(data_struct->get_float32_value(test_float_value,
            data_struct->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
    EXPECT_EQ(test_float_value, first_float_value);
    EXPECT_EQ(seq_data->return_loaned_value(data_struct), RETCODE_OK);
    data_struct = seq_data->loan_value(1);
    ASSERT_TRUE(data_struct);
    EXPECT_EQ(data_struct->set_int32_value(data_struct->get_member_id_by_name(struct_long_member_name),
            second_long_value), RETCODE_OK);
    EXPECT_EQ(data_struct->get_int32_value(test_long_value, data_struct->get_member_id_by_name(
                struct_long_member_name)), RETCODE_OK);
    EXPECT_EQ(test_long_value, second_long_value);
    EXPECT_EQ(data_struct->set_float32_value(data_struct->get_member_id_by_name(struct_float_member_name),
            second_float_value), RETCODE_OK);
    EXPECT_EQ(data_struct->get_float32_value(test_float_value,
            data_struct->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
    EXPECT_EQ(test_float_value, second_float_value);
    EXPECT_EQ(seq_data->return_loaned_value(data_struct), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        SequenceStructure struct_data;
        TypeSupport static_pubsubType {new SequenceStructurePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        seq_data = data->loan_value(data->get_member_id_by_name(var_struct_seq));
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(struct_data.var_sequence_structure().size(), seq_data->get_item_count());
        EXPECT_EQ(struct_data.var_sequence_structure()[0].field1(), first_long_value);
        EXPECT_EQ(struct_data.var_sequence_structure()[0].field2(), first_float_value);
        EXPECT_EQ(struct_data.var_sequence_structure()[1].field1(), second_long_value);
        EXPECT_EQ(struct_data.var_sequence_structure()[1].field2(), second_float_value);
        EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_SequenceBitset)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitset_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitset_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(create_inner_bitset_helper(),
            static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint8_t first_uint8_value = 5;
    uint8_t second_uint8_value = 7;
    uint8_t test_uint8_value = 0;
    bool first_bool_value = true;
    bool second_bool_value = false;
    bool test_bool_value = false;
    uint16_t first_ushort_value = 1000;
    uint16_t second_ushort_value = 555;
    uint16_t test_ushort_value = 0;
    int16_t first_short_value = 2000;
    int16_t second_short_value = 20;
    int16_t test_short_value = 0;
    DynamicData::_ref_type seq_data = data->loan_value(data->get_member_id_by_name(var_bitset_seq));
    ASSERT_TRUE(seq_data);
    DynamicData::_ref_type bitset_data = seq_data->loan_value(0);
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), first_uint8_value),
            RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(first_uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(
                bitfield_b), first_bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(first_bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(
                bitfield_c), first_ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(first_ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(
                bitfield_d), first_short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(first_short_value, test_short_value);
    EXPECT_EQ(seq_data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = seq_data->loan_value(1);
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(
                bitfield_a), second_uint8_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(second_uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(
                bitfield_b), second_bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(second_bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(
                bitfield_c), second_ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(second_ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(
                bitfield_d), second_short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(second_short_value, test_short_value);
    EXPECT_EQ(seq_data->return_loaned_value(bitset_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        SequenceBitset struct_data;
        TypeSupport static_pubsubType {new SequenceBitsetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        seq_data = data->loan_value(data->get_member_id_by_name(var_bitset_seq));
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(struct_data.var_sequence_bitset().size(), seq_data->get_item_count());
        EXPECT_EQ(struct_data.var_sequence_bitset()[0].a, first_uint8_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[0].b, first_bool_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[0].c, first_ushort_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[0].d, first_short_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[1].a, second_uint8_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[1].b, second_bool_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[1].c, second_ushort_value);
        EXPECT_EQ(struct_data.var_sequence_bitset()[1].d, second_short_value);
        EXPECT_EQ(data->return_loaned_value(seq_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_SequenceBitset_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BoundedSmallSequences)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(small_bounded_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_small_bounded_short_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), 1)->build());
    struct_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_small_bounded_string_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->create_string_type(static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->
                    build(), 5)->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq short_value = {-555};
    Int16Seq test_short_value;
    StringSeq string_value = {"Hello world", ",", "how", "are", "you"};
    StringSeq test_string_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_small_bounded_short_seq), short_value),
            RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_short_value, data->get_member_id_by_name(
                var_small_bounded_short_seq)), RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(
                var_small_bounded_string_seq), string_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_string_value, data->get_member_id_by_name(
                var_small_bounded_string_seq)), RETCODE_OK);
    EXPECT_EQ(string_value, test_string_value);

    for (auto encoding : encodings)
    {
        BoundedSmallSequences struct_data;
        TypeSupport static_pubsubType {new BoundedSmallSequencesPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_small(), test_short_value);
        EXPECT_EQ(struct_data.var_unbounded_string_small_bounded_sequence(), test_string_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BoundedSmallSequences_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BoundedBigSequences)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(large_bounded_seq_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_large_bounded_short_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), 41925)->build());
    struct_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_large_bounded_string_seq);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->create_string_type(static_cast<uint32_t>(static_cast<uint32_t>(LENGTH_UNLIMITED)))->
                    build(),
            256)->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq short_value = {-555, 342, -124, 54};
    Int16Seq test_short_value;
    StringSeq string_value = {"Hello world", ",", "how", "are", "you"};
    StringSeq test_string_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_large_bounded_short_seq), short_value),
            RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_short_value, data->get_member_id_by_name(
                var_large_bounded_short_seq)), RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(
                var_large_bounded_string_seq), string_value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_string_value, data->get_member_id_by_name(
                var_large_bounded_string_seq)), RETCODE_OK);
    EXPECT_EQ(string_value, test_string_value);

    for (auto encoding : encodings)
    {
        BoundedBigSequences struct_data;
        TypeSupport static_pubsubType {new BoundedBigSequencesPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence_big(), test_short_value);
        EXPECT_EQ(struct_data.var_unbounded_string_large_bounded_sequence(), test_string_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BoundedBigSequences_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_My_Structure)
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_my_char_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    DynamicTypeBuilder::_ref_type alias_builder {DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)};
    ASSERT_TRUE(alias_builder);
    DynamicType::_ref_type alias_type {alias_builder->build()};
    ASSERT_TRUE(alias_type);

    alias_descriptor = traits<TypeDescriptor>::make_shared();
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_my_string_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(alias_type,
            20)->build());
    alias_builder = DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor);
    ASSERT_TRUE(alias_builder);
    alias_type = alias_builder->build();
    ASSERT_TRUE(alias_type);

    alias_descriptor = traits<TypeDescriptor>::make_shared();
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_my_aliasstring_name);
    alias_descriptor->base_type(alias_type);
    alias_builder = DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor);
    ASSERT_TRUE(alias_builder);
    alias_type = alias_builder->build();
    ASSERT_TRUE(alias_type);

    alias_descriptor = traits<TypeDescriptor>::make_shared();
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(alias_my_sequencestring_name);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(alias_type,
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    alias_builder = DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor);
    ASSERT_TRUE(alias_builder);
    alias_type = alias_builder->build();
    ASSERT_TRUE(alias_type);

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(my_structure_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_list);
    member_descriptor->type(alias_type);
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    DynamicData::_ref_type element_data{data->loan_value(0)};
    ASSERT_TRUE(element_data);
    CharSeq char_value = {'a', 'b', 'y', 'z'};
    EXPECT_EQ(element_data->set_char8_values(0, char_value), RETCODE_OK);
    data->return_loaned_value(element_data);

    element_data = data->loan_value(0);
    CharSeq test_char_value;
    EXPECT_EQ(element_data->get_char8_values(test_char_value, 0), RETCODE_OK);
    EXPECT_EQ(char_value, test_char_value);
    data->return_loaned_value(element_data);

    for (auto encoding : encodings)
    {
        NoCommon_Module::My_Structure struct_data;
        TypeSupport static_pubsubType {new NoCommon_Module::My_StructurePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(1, struct_data.list().size());
        EXPECT_EQ(struct_data.list().at(0), test_char_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    NoCommon_Module::register_My_Structure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
