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
#include "../../../dds-types-test/arraysPubSubTypes.hpp"
#include "../../../dds-types-test/arraysTypeObjectSupport.hpp"
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

constexpr const char* short_array_struct_name = "ArrayShort";
constexpr const char* ushort_array_struct_name = "ArrayUShort";
constexpr const char* long_array_struct_name = "ArrayLong";
constexpr const char* ulong_array_struct_name = "ArrayULong";
constexpr const char* longlong_array_struct_name = "ArrayLongLong";
constexpr const char* ulonglong_array_struct_name = "ArrayULongLong";
constexpr const char* float_array_struct_name = "ArrayFloat";
constexpr const char* double_array_struct_name = "ArrayDouble";
constexpr const char* longdouble_array_struct_name = "ArrayLongDouble";
constexpr const char* bool_array_struct_name = "ArrayBoolean";
constexpr const char* byte_array_struct_name = "ArrayOctet";
constexpr const char* uint8_array_struct_name = "ArrayUInt8";
constexpr const char* char_array_struct_name = "ArrayChar";
constexpr const char* wchar_array_struct_name = "ArrayWChar";
constexpr const char* string_array_struct_name = "ArrayString";
constexpr const char* wstring_array_struct_name = "ArrayWString";
constexpr const char* bounded_string_array_struct_name = "ArrayBoundedString";
constexpr const char* bounded_wstring_array_struct_name = "ArrayBoundedWString";
constexpr const char* enum_array_struct_name = "ArrayEnum";
constexpr const char* bitmask_array_struct_name = "ArrayBitMask";
constexpr const char* alias_array_struct_name = "ArrayAlias";
constexpr const char* array_array_struct_name = "ArrayShortArray";
constexpr const char* seq_array_struct_name = "ArraySequence";
constexpr const char* map_array_struct_name = "ArrayMap";
constexpr const char* union_array_struct_name = "ArrayUnion";
constexpr const char* struct_array_struct_name = "ArrayStructure";
constexpr const char* bitset_array_struct_name = "ArrayBitset";
constexpr const char* short_multiarray_struct_name = "ArrayMultiDimensionShort";
constexpr const char* ushort_multiarray_struct_name = "ArrayMultiDimensionUShort";
constexpr const char* long_multiarray_struct_name = "ArrayMultiDimensionLong";
constexpr const char* ulong_multiarray_struct_name = "ArrayMultiDimensionULong";
constexpr const char* longlong_multiarray_struct_name = "ArrayMultiDimensionLongLong";
constexpr const char* ulonglong_multiarray_struct_name = "ArrayMultiDimensionULongLong";
constexpr const char* float_multiarray_struct_name = "ArrayMultiDimensionFloat";
constexpr const char* double_multiarray_struct_name = "ArrayMultiDimensionDouble";
constexpr const char* longdouble_multiarray_struct_name = "ArrayMultiDimensionLongDouble";
constexpr const char* bool_multiarray_struct_name = "ArrayMultiDimensionBoolean";
constexpr const char* byte_multiarray_struct_name = "ArrayMultiDimensionOctet";
constexpr const char* char_multiarray_struct_name = "ArrayMultiDimensionChar";
constexpr const char* wchar_multiarray_struct_name = "ArrayMultiDimensionWChar";
constexpr const char* string_multiarray_struct_name = "ArrayMultiDimensionString";
constexpr const char* wstring_multiarray_struct_name = "ArrayMultiDimensionWString";
constexpr const char* bounded_string_multiarray_struct_name = "ArrayMultiDimensionBoundedString";
constexpr const char* bounded_wstring_multiarray_struct_name = "ArrayMultiDimensionBoundedWString";
constexpr const char* enum_multiarray_struct_name = "ArrayMultiDimensionEnum";
constexpr const char* bitmask_multiarray_struct_name = "ArrayMultiDimensionBitMask";
constexpr const char* alias_multiarray_struct_name = "ArrayMultiDimensionAlias";
constexpr const char* seq_multiarray_struct_name = "ArrayMultiDimensionSequence";
constexpr const char* map_multiarray_struct_name = "ArrayMultiDimensionMap";
constexpr const char* union_multiarray_struct_name = "ArrayMultiDimensionUnion";
constexpr const char* struct_multiarray_struct_name = "ArrayMultiDimensionStructure";
constexpr const char* bitset_multiarray_struct_name = "ArrayMultiDimensionBitset";
constexpr const char* large_array_struct_name = "BoundedBigArrays";

constexpr const char* var_short_array = "var_array_short";
constexpr const char* var_ushort_array = "var_array_ushort";
constexpr const char* var_long_array = "var_array_long";
constexpr const char* var_ulong_array = "var_array_ulong";
constexpr const char* var_longlong_array = "var_array_longlong";
constexpr const char* var_ulonglong_array = "var_array_ulonglong";
constexpr const char* var_float_array = "var_array_float";
constexpr const char* var_double_array = "var_array_double";
constexpr const char* var_longdouble_array = "var_array_longdouble";
constexpr const char* var_bool_array = "var_array_boolean";
constexpr const char* var_byte_array = "var_array_octet";
constexpr const char* var_uint8_array = "var_array_uint8";
constexpr const char* var_char_array = "var_array_char";
constexpr const char* var_wchar_array = "var_array_wchar";
constexpr const char* var_string_array = "var_array_string";
constexpr const char* var_wstring_array = "var_array_wstring";
constexpr const char* var_bounded_string_array = "var_array_bounded_string";
constexpr const char* var_bounded_wstring_array = "var_array_bounded_wstring";
constexpr const char* var_enum_array = "var_array_enum";
constexpr const char* var_bitmask_array = "var_array_bitmask";
constexpr const char* var_alias_array = "var_array_alias";
constexpr const char* var_array_array = "var_array_short_array";
constexpr const char* var_seq_array = "var_array_sequence";
constexpr const char* var_map_array = "var_array_map";
constexpr const char* var_union_array = "var_array_union";
constexpr const char* var_struct_array = "var_array_structure";
constexpr const char* var_bitset_array = "var_array_bitset";
constexpr const char* var_large_array = "var_array_big";

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {1, 2, 3, 4, 5, -6, -7, -8, -9, -10};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_short_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(var_short_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayShort struct_data;
        TypeSupport static_pubsubType {new ArrayShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_short().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_short()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT16), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt16Seq value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    UInt16Seq test_value;
    EXPECT_EQ(data->set_uint16_values(data->get_member_id_by_name(var_ushort_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(test_value, data->get_member_id_by_name(var_ushort_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayUShort struct_data;
        TypeSupport static_pubsubType {new ArrayUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_ushort().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_ushort()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_long_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_long_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayLong struct_data;
        TypeSupport static_pubsubType {new ArrayLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_long().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_long()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT32), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt32Seq value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    UInt32Seq test_value;
    EXPECT_EQ(data->set_uint32_values(data->get_member_id_by_name(var_ulong_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(test_value, data->get_member_id_by_name(var_ulong_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayULong struct_data;
        TypeSupport static_pubsubType {new ArrayULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_ulong().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_ulong()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT64), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int64Seq value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Int64Seq test_value;
    EXPECT_EQ(data->set_int64_values(data->get_member_id_by_name(var_longlong_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(test_value, data->get_member_id_by_name(var_longlong_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayLongLong struct_data;
        TypeSupport static_pubsubType {new ArrayLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_longlong().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_longlong()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT64), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt64Seq value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    UInt64Seq test_value;
    EXPECT_EQ(data->set_uint64_values(data->get_member_id_by_name(var_ulonglong_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(test_value, data->get_member_id_by_name(var_ulonglong_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayULongLong struct_data;
        TypeSupport static_pubsubType {new ArrayULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_ulonglong().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_ulonglong()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(float_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_float_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT32), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float32Seq value {1.01f, 2.002f, 3.0003f, 4.4f, 5.4f, 6.67f, 7.4567f, 8.122f, 9.868f, -10.52f};
    Float32Seq test_value;
    EXPECT_EQ(data->set_float32_values(data->get_member_id_by_name(var_float_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(test_value, data->get_member_id_by_name(var_float_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayFloat struct_data;
        TypeSupport static_pubsubType {new ArrayFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_float().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_float()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(double_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_double_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT64), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float64Seq value = {1.01, 2.002, 3.0003, 4.4, 5.4, 6.67, 7.4567, 8.122, 9.868, -10.52};
    Float64Seq test_value;
    EXPECT_EQ(data->set_float64_values(data->get_member_id_by_name(var_double_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(test_value, data->get_member_id_by_name(var_double_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayDouble struct_data;
        TypeSupport static_pubsubType {new ArrayDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_double().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_double()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longdouble_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longdouble_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT128), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float128Seq value = {1.01, 2.002, 3.0003, 4.4, 5.4, 6.67, 7.4567, 8.122, 9.868, -10.52};
    Float128Seq test_value;
    EXPECT_EQ(data->set_float128_values(data->get_member_id_by_name(var_longdouble_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(test_value, data->get_member_id_by_name(var_longdouble_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayLongDouble struct_data;
        TypeSupport static_pubsubType {new ArrayLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_longdouble().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_longdouble()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bool_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bool_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_BOOLEAN), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    BooleanSeq value = {true, true, false, false, false, true, false, true, true, true};
    BooleanSeq test_value;
    EXPECT_EQ(data->set_boolean_values(data->get_member_id_by_name(var_bool_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_values(test_value, data->get_member_id_by_name(var_bool_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayBoolean struct_data;
        TypeSupport static_pubsubType {new ArrayBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_boolean().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_boolean()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(byte_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_byte_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_BYTE), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    ByteSeq value = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
    ByteSeq test_value;
    EXPECT_EQ(data->set_byte_values(data->get_member_id_by_name(var_byte_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_values(test_value, data->get_member_id_by_name(var_byte_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayOctet struct_data;
        TypeSupport static_pubsubType {new ArrayOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_octet().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_octet()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

void DDSTypesTest_ArrayUInt8_common(
        DynamicTypesDDSTypesTest& support)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(uint8_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_uint8_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT8), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt8Seq value = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
    UInt8Seq test_value;
    EXPECT_EQ(data->set_uint8_values(data->get_member_id_by_name(var_uint8_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_values(test_value, data->get_member_id_by_name(var_uint8_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayUInt8 struct_data;
        TypeSupport static_pubsubType {new ArrayUInt8PubSubType()};
        support.check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_uint8().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_uint8()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayUInt8_type_identifier(static_type_ids);
    support.check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayUInt8)
{
    DDSTypesTest_ArrayUInt8_common(*this);
}

// Regression test for redmine ticket #20878.
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayUInt8_Regression20878)
{
    xtypes::TypeIdentifierPair regression_type_ids;
    register_ArrayOctet_type_identifier(regression_type_ids);

    DDSTypesTest_ArrayUInt8_common(*this);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(char_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_CHAR8), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    CharSeq value = {'e', 'P', 'r', 'o', 's', 'i', 'm', 'a', 'a', 'a'};
    CharSeq test_value;
    EXPECT_EQ(data->set_char8_values(data->get_member_id_by_name(var_char_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_values(test_value, data->get_member_id_by_name(var_char_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayChar struct_data;
        TypeSupport static_pubsubType {new ArrayCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_char().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_char()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(wchar_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wchar_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_CHAR16), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WcharSeq value = {L'e', L'P', L'r', L'o', L's', L'i', L'm', L'a', L'a', L'a'};
    WcharSeq test_value;
    EXPECT_EQ(data->set_char16_values(data->get_member_id_by_name(var_wchar_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(test_value, data->get_member_id_by_name(var_wchar_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayWChar struct_data;
        TypeSupport static_pubsubType {new ArrayWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_wchar().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_wchar()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StringSeq value = {"Lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing", "elit", "sed", "do"};
    StringSeq test_value;
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(var_string_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_value, data->get_member_id_by_name(var_string_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayString struct_data;
        TypeSupport static_pubsubType {new ArrayStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_string().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_string()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(wstring_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wstring_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WstringSeq value =
    {L"Lorem", L"ipsum", L"dolor", L"sit", L"amet", L"consectetur", L"adipiscing", L"elit", L"sed",
     L"do"};
    WstringSeq test_value;
    EXPECT_EQ(data->set_wstring_values(data->get_member_id_by_name(var_wstring_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_values(test_value, data->get_member_id_by_name(var_wstring_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayWString struct_data;
        TypeSupport static_pubsubType {new ArrayWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_wstring().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_wstring()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayBoundedString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_string_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_string_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(
                create_inner_alias_bounded_string_helper(), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StringSeq value = {"Lorem", "ipsum", "dolor", "sit", "amet", "consectetu", "adipiscing", "elit", "sed", "do"};
    StringSeq test_value;
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(var_bounded_string_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_value, data->get_member_id_by_name(var_bounded_string_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayBoundedString struct_data;
        TypeSupport static_pubsubType {new ArrayBoundedStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_bounded_string().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_bounded_string()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayBoundedString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayBoundedWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_wstring_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_wstring_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(
                create_inner_alias_bounded_wstring_helper(), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WstringSeq value =
    {L"Lorem", L"ipsum", L"dolor", L"sit", L"amet", L"consectetu", L"adipiscing", L"elit", L"sed",
     L"do"};
    WstringSeq test_value;
    EXPECT_EQ(data->set_wstring_values(data->get_member_id_by_name(var_bounded_wstring_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_values(test_value, data->get_member_id_by_name(var_bounded_wstring_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayBoundedWString struct_data;
        TypeSupport static_pubsubType {new ArrayBoundedWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_bounded_wstring().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_bounded_wstring()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayBoundedWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayEnum)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(enum_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_enum_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_enum_helper(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3)
    };
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_enum_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_enum_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayEnum struct_data;
        TypeSupport static_pubsubType {new ArrayEnumPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_enum().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(static_cast<int32_t>(struct_data.var_array_enum()[i]), test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayEnum_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayBitMask)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitmask_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitmask_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_bitmask_helper(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt32Seq value = {
        InnerBitMaskHelperBits::flag0,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag4,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag1 |
        InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6
    };
    UInt32Seq test_value;
    EXPECT_EQ(data->set_uint32_values(data->get_member_id_by_name(var_bitmask_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(test_value, data->get_member_id_by_name(var_bitmask_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayBitMask struct_data;
        TypeSupport static_pubsubType {new ArrayBitMaskPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_bitmask().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_bitmask()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayBitMask_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayAlias)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(alias_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_alias_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_alias_helper(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {10, -100, 1000, -20, 200, -20000, 30, -300, 30000, 45};
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_alias_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_alias_array)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayAlias struct_data;
        TypeSupport static_pubsubType {new ArrayAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_alias().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_alias()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayShortArray)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(array_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_array_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), {10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {10, -100, 1000, -20, 200, -20000, 30, -300, 30000, 45,
                      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                      0, -1, -2, -3, -4, -5, -6, -7, -8, -9,
                      1000};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_array_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(var_array_array)), RETCODE_OK);
    value.insert(value.end(), 69, 0);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayShortArray struct_data;
        TypeSupport static_pubsubType {new ArrayShortArrayPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_short_array().size() * struct_data.var_array_short_array()[0].size(),
            test_value.size());
        for (size_t i = 0; i < struct_data.var_array_short_array().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_short_array()[i].size(); ++j)
            {
                EXPECT_EQ(struct_data.var_array_short_array()[i][j],
                        test_value[i * struct_data.var_array_short_array().size() + j]);
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayShortArray_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArraySequence)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(seq_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_seq_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_sequence_type(DynamicTypeBuilderFactory::get_instance()->
                    get_primitive_type(TK_INT32), static_cast<uint32_t>(LENGTH_UNLIMITED))->build(), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq first_value = {10, -100, 1000, -20, 200};
    Int32Seq second_value = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Int32Seq third_value = {0, -1, -2};
    Int32Seq test_value;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_seq_array));
    ASSERT_TRUE(array_data);
    EXPECT_EQ(array_data->set_int32_values(0, first_value), RETCODE_OK);
    EXPECT_EQ(array_data->get_int32_values(test_value, 0), RETCODE_OK);
    EXPECT_EQ(first_value, test_value);
    EXPECT_EQ(array_data->set_int32_values(1, second_value), RETCODE_OK);
    EXPECT_EQ(array_data->get_int32_values(test_value, 1), RETCODE_OK);
    EXPECT_EQ(second_value, test_value);
    EXPECT_EQ(array_data->set_int32_values(2, third_value), RETCODE_OK);
    EXPECT_EQ(array_data->get_int32_values(test_value, 2), RETCODE_OK);
    EXPECT_EQ(third_value, test_value);
    for (uint32_t i = 3; i < array_data->get_item_count(); ++i)
    {
        EXPECT_EQ(array_data->get_int32_values(test_value, i), RETCODE_OK);
        EXPECT_TRUE(test_value.empty());
    }
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        ArraySequence struct_data;
        TypeSupport static_pubsubType {new ArraySequencePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        array_data = data->loan_value(data->get_member_id_by_name(var_seq_array));
        EXPECT_EQ(struct_data.var_array_sequence().size(), array_data->get_item_count());
        EXPECT_EQ(struct_data.var_array_sequence()[0], first_value);
        EXPECT_EQ(struct_data.var_array_sequence()[1], second_value);
        EXPECT_EQ(struct_data.var_array_sequence()[2], third_value);
        for (size_t i = 3; i < array_data->get_item_count(); ++i)
        {
            EXPECT_TRUE(struct_data.var_array_sequence()[i].empty());
        }
        EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArraySequence_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(map_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_map_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_map_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                TK_INT32), DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build(), {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_key = 121;
    int32_t second_key = -11;
    int32_t third_key = 1001;
    int32_t first_value = 1;
    int32_t second_value = 2;
    int32_t third_value = 3;
    int32_t test_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_map_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type map_data = array_data->loan_value(0);
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
    EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
    map_data = array_data->loan_value(1);
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                third_key)), third_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                third_key))), RETCODE_OK);
    EXPECT_EQ(test_value, third_value);
    EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);
    for (uint32_t i = 2; i < array_data->get_item_count(); ++i)
    {
        map_data = array_data->loan_value(i);
        ASSERT_TRUE(map_data);
        EXPECT_EQ(map_data->get_item_count(), 0u);
        EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
    }

    for (auto encoding : encodings)
    {
        ArrayMap struct_data;
        TypeSupport static_pubsubType {new ArrayMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        array_data = data->loan_value(data->get_member_id_by_name(var_map_array));
        ASSERT_TRUE(array_data);
        EXPECT_EQ(struct_data.var_array_map().size(), array_data->get_item_count());
        map_data = array_data->loan_value(0);
        ASSERT_TRUE(map_data);
        EXPECT_EQ(struct_data.var_array_map()[0].size(), map_data->get_item_count());
        EXPECT_EQ(struct_data.var_array_map()[0][first_key], first_value);
        EXPECT_EQ(struct_data.var_array_map()[0][second_key], second_value);
        EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
        map_data = array_data->loan_value(1);
        ASSERT_TRUE(map_data);
        EXPECT_EQ(struct_data.var_array_map()[1].size(), map_data->get_item_count());
        EXPECT_EQ(struct_data.var_array_map()[1][third_key], third_value);
        EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
        for (size_t i = 2; i < array_data->get_item_count(); ++i)
        {
            EXPECT_TRUE(struct_data.var_array_map()[i].empty());
        }
        EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayUnion)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(union_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_union_helper(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t long_value = 121;
    int32_t test_long_value = 0;
    float float_value = 10.01f;
    float test_float_value = 0;
    int16_t short_value = -2;
    int16_t test_short_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_union_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type union_data = array_data->loan_value(0);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                union_long_member_name), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                union_long_member_name)), RETCODE_OK);
    EXPECT_EQ(test_long_value, long_value);
    EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    union_data = array_data->loan_value(1);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_float32_value(union_data->get_member_id_by_name(
                union_float_member_name), float_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float32_value(test_float_value, union_data->get_member_id_by_name(
                union_float_member_name)), RETCODE_OK);
    EXPECT_EQ(test_float_value, float_value);
    EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    union_data = array_data->loan_value(2);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                union_short_member_name), short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_short_value, union_data->get_member_id_by_name(
                union_short_member_name)), RETCODE_OK);
    EXPECT_EQ(test_short_value, short_value);
    EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);
    for (uint32_t i = 3; i < array_data->get_item_count(); ++i)
    {
        union_data = array_data->loan_value(i);
        ASSERT_TRUE(union_data);
        EXPECT_EQ(union_data->get_int16_value(test_short_value,
                union_data->get_member_id_by_name(union_short_member_name)), RETCODE_OK);
        EXPECT_EQ(test_short_value, 0);
        EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    }

    for (auto encoding : encodings)
    {
        ArrayUnion struct_data;
        TypeSupport static_pubsubType {new ArrayUnionPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_union()[0].longValue(), long_value);
        EXPECT_EQ(struct_data.var_array_union()[1].floatValue(), float_value);
        EXPECT_EQ(struct_data.var_array_union()[2].shortValue(), short_value);
        for (size_t i = 3; i < struct_data.var_array_union().size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_union()[i].shortValue(), 0);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayUnion_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_struct_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_struct_helper(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_long_value = 121;
    int32_t second_long_value = 10001;
    int32_t test_long_value = 10001;
    float first_float_value = 10.01f;
    float second_float_value = 3.14f;
    float test_float_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_struct_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type data_struct = array_data->loan_value(0);
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
    EXPECT_EQ(array_data->return_loaned_value(data_struct), RETCODE_OK);
    data_struct = array_data->loan_value(1);
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
    EXPECT_EQ(array_data->return_loaned_value(data_struct), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);
    for (uint32_t i = 2; i < array_data->get_item_count(); ++i)
    {
        data_struct = array_data->loan_value(i);
        ASSERT_TRUE(data_struct);
        EXPECT_EQ(data_struct->get_int32_value(test_long_value,
                data_struct->get_member_id_by_name(struct_long_member_name)), RETCODE_OK);
        EXPECT_EQ(data_struct->get_float32_value(test_float_value,
                data_struct->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
        EXPECT_EQ(test_long_value, 0);
        EXPECT_EQ(test_float_value, 0);
        EXPECT_EQ(array_data->return_loaned_value(data_struct), RETCODE_OK);
    }

    for (auto encoding : encodings)
    {
        ArrayStructure struct_data;
        TypeSupport static_pubsubType {new ArrayStructurePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_structure()[0].field1(), first_long_value);
        EXPECT_EQ(struct_data.var_array_structure()[0].field2(), first_float_value);
        EXPECT_EQ(struct_data.var_array_structure()[1].field1(), second_long_value);
        EXPECT_EQ(struct_data.var_array_structure()[1].field2(), second_float_value);
        for (size_t i = 2; i < struct_data.var_array_structure().size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_structure()[i].field1(), 0);
            EXPECT_EQ(struct_data.var_array_structure()[i].field2(), 0);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayBitset)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitset_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitset_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_bitset_helper(),
            {10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
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
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_bitset_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type bitset_data = array_data->loan_value(0);
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
    EXPECT_EQ(array_data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = array_data->loan_value(1);
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
    EXPECT_EQ(array_data->return_loaned_value(bitset_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);
    for (uint32_t i = 2; i < array_data->get_item_count(); ++i)
    {
        bitset_data = array_data->loan_value(i);
        ASSERT_TRUE(bitset_data);
        EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(
                    bitfield_a)), RETCODE_OK);
        EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                    bitfield_b)), RETCODE_OK);
        EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                    bitfield_c)), RETCODE_OK);
        EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(
                    bitfield_d)), RETCODE_OK);
        EXPECT_EQ(test_uint8_value, 0u);
        EXPECT_EQ(test_bool_value, false);
        EXPECT_EQ(test_ushort_value, 0u);
        EXPECT_EQ(test_short_value, 0);
        EXPECT_EQ(array_data->return_loaned_value(bitset_data), RETCODE_OK);
    }

    for (auto encoding : encodings)
    {
        ArrayBitset struct_data;
        TypeSupport static_pubsubType {new ArrayBitsetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_bitset()[0].a, first_uint8_value);
        EXPECT_EQ(struct_data.var_array_bitset()[0].b, first_bool_value);
        EXPECT_EQ(struct_data.var_array_bitset()[0].c, first_ushort_value);
        EXPECT_EQ(struct_data.var_array_bitset()[0].d, first_short_value);
        EXPECT_EQ(struct_data.var_array_bitset()[1].a, second_uint8_value);
        EXPECT_EQ(struct_data.var_array_bitset()[1].b, second_bool_value);
        EXPECT_EQ(struct_data.var_array_bitset()[1].c, second_ushort_value);
        EXPECT_EQ(struct_data.var_array_bitset()[1].d, second_short_value);
        for (size_t i = 2; i < struct_data.var_array_bitset().size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_bitset()[i].a, 0u);
            EXPECT_EQ(struct_data.var_array_bitset()[i].b, false);
            EXPECT_EQ(struct_data.var_array_bitset()[i].c, 0u);
            EXPECT_EQ(struct_data.var_array_bitset()[i].d, 0);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayBitset_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(short_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {1, 2, 3, 4, 5, -6, -7, -8, -9, -10, 100, 1000, 10000, 15000, 30000, 32000};
    value.insert(value.end(), 84, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_short_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(var_short_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionShort struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_short().size() * struct_data.var_array_short()[0].size() *
            struct_data.var_array_short()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_short().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_short()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_short()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_short()[i][j][k],
                            test_value[i * struct_data.var_array_short().size() *
                            struct_data.var_array_short()[i].size() + j *
                            struct_data.var_array_short()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ushort_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT16), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt16Seq value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 1000, 10000, 15000, 30000, 32000};
    value.insert(value.end(), 84, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    UInt16Seq test_value;
    EXPECT_EQ(data->set_uint16_values(data->get_member_id_by_name(var_ushort_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(test_value, data->get_member_id_by_name(var_ushort_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionUShort struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_ushort().size() * struct_data.var_array_ushort()[0].size() *
            struct_data.var_array_ushort()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_ushort().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_ushort()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_ushort()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_ushort()[i][j][k],
                            test_value[i * struct_data.var_array_ushort().size() *
                            struct_data.var_array_ushort()[i].size() + j *
                            struct_data.var_array_ushort()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(long_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value =
    {1, 2, 3, 4, 5, -6, -7, -8, -9, -10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
     1000000000};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_long_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_long_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionLong struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_long().size() * struct_data.var_array_long()[0].size() *
            struct_data.var_array_long()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_long().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_long()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_long()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_long()[i][j][k],
                            test_value[i * struct_data.var_array_long().size() *
                            struct_data.var_array_long()[i].size() + j *
                            struct_data.var_array_long()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulong_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT32), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt32Seq value =
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
     1000000000};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    UInt32Seq test_value;
    EXPECT_EQ(data->set_uint32_values(data->get_member_id_by_name(var_ulong_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(test_value, data->get_member_id_by_name(var_ulong_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionULong struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_ulong().size() * struct_data.var_array_ulong()[0].size() *
            struct_data.var_array_ulong()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_ulong().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_ulong()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_ulong()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_ulong()[i][j][k],
                            test_value[i * struct_data.var_array_ulong().size() *
                            struct_data.var_array_ulong()[i].size() + j *
                            struct_data.var_array_ulong()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longlong_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longlong_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT64), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int64Seq value =
    {1, 2, 3, 4, 5, -6, -7, -8, -9, -10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
     1000000000, 1000000000000000000};
    value.insert(value.end(), 81, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Int64Seq test_value;
    EXPECT_EQ(data->set_int64_values(data->get_member_id_by_name(var_longlong_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(test_value, data->get_member_id_by_name(var_longlong_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionLongLong struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_longlong().size() * struct_data.var_array_longlong()[0].size() *
            struct_data.var_array_longlong()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_longlong().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_longlong()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_longlong()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_longlong()[i][j][k],
                            test_value[i * struct_data.var_array_longlong().size() *
                            struct_data.var_array_longlong()[i].size() + j *
                            struct_data.var_array_longlong()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ulonglong_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulonglong_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_UINT64), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt64Seq value =
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000,
     1000000000000000000};
    value.insert(value.end(), 81, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    UInt64Seq test_value;
    EXPECT_EQ(data->set_uint64_values(data->get_member_id_by_name(var_ulonglong_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(test_value, data->get_member_id_by_name(var_ulonglong_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionULongLong struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_ulonglong().size() * struct_data.var_array_ulonglong()[0].size() *
            struct_data.var_array_ulonglong()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_ulonglong().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_ulonglong()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_ulonglong()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_ulonglong()[i][j][k],
                            test_value[i * struct_data.var_array_ulonglong().size() *
                            struct_data.var_array_ulonglong()[i].size() + j *
                            struct_data.var_array_ulonglong()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(float_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_float_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT32), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float32Seq value =
    {-1.234f, 2.000103f, 3.14f, -4.01f, 5.00000001f, 6.0f, 7.5f, 8.21f, 9.0f, 10.0f, 100.0f, 1000.0f, 10000.0f,
     100000.0f, 1000000.0f, 10000000.0f, 100000000.0f, 1000000000.0f};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Float32Seq test_value;
    EXPECT_EQ(data->set_float32_values(data->get_member_id_by_name(var_float_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(test_value, data->get_member_id_by_name(var_float_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionFloat struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_float().size() * struct_data.var_array_float()[0].size() *
            struct_data.var_array_float()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_float().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_float()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_float()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_float()[i][j][k],
                            test_value[i * struct_data.var_array_float().size() *
                            struct_data.var_array_float()[i].size() + j *
                            struct_data.var_array_float()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(double_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_double_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT64), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float64Seq value =
    {-1.234, 2.000103, 3.14, -4.01, 5.00000001, 6, 7.5, 8.21, 9, 10, 100, 1000, 10000, 100000,
     1000000, 10000000, 100000000, 1000000000.4622537257};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Float64Seq test_value;
    EXPECT_EQ(data->set_float64_values(data->get_member_id_by_name(var_double_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(test_value, data->get_member_id_by_name(var_double_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionDouble struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_double().size() * struct_data.var_array_double()[0].size() *
            struct_data.var_array_double()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_double().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_double()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_double()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_double()[i][j][k],
                            test_value[i * struct_data.var_array_double().size() *
                            struct_data.var_array_double()[i].size() + j *
                            struct_data.var_array_double()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(longdouble_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_longdouble_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_FLOAT128), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Float128Seq value =
    {-1.234, 2.000103, 3.14, -4.01, 5.00000001, 6, 7.5, 8.21, 9, 10, 100, 1000, 10000, 100000,
     1000000, 10000000, 100000000, 1000000000.4622537257};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Float128Seq test_value;
    EXPECT_EQ(data->set_float128_values(data->get_member_id_by_name(var_longdouble_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(test_value, data->get_member_id_by_name(var_longdouble_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionLongDouble struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_longdouble().size() * struct_data.var_array_longdouble()[0].size() *
            struct_data.var_array_longdouble()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_longdouble().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_longdouble()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_longdouble()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_longdouble()[i][j][k],
                            test_value[i * struct_data.var_array_longdouble().size() *
                            struct_data.var_array_longdouble()[i].size() + j *
                            struct_data.var_array_longdouble()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bool_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bool_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_BOOLEAN), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    BooleanSeq value = {true, true, false, true, true, true, false, true, false, false, false, true};
    value.insert(value.end(), 88, false);
    value.insert(value.end(), true);
    value.insert(value.end(), 100, false);
    value.insert(value.end(), true);
    value.insert(value.end(), 100, false);
    value.insert(value.end(), true);
    BooleanSeq test_value;
    EXPECT_EQ(data->set_boolean_values(data->get_member_id_by_name(var_bool_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_values(test_value, data->get_member_id_by_name(var_bool_array)), RETCODE_OK);
    value.insert(value.end(), 697, false);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionBoolean struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_boolean().size() * struct_data.var_array_boolean()[0].size() *
            struct_data.var_array_boolean()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_boolean().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_boolean()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_boolean()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_boolean()[i][j][k],
                            test_value[i * struct_data.var_array_boolean().size() *
                            struct_data.var_array_boolean()[i].size() + j *
                            struct_data.var_array_boolean()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(byte_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_byte_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_BYTE), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    ByteSeq value = {0, 1, 5, 10, 50, 100, 150, 200, 255, 12, 124, 234};
    value.insert(value.end(), 88, 0);
    value.insert(value.end(), 1);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 143);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 12);
    ByteSeq test_value;
    EXPECT_EQ(data->set_byte_values(data->get_member_id_by_name(var_byte_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_values(test_value, data->get_member_id_by_name(var_byte_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionOctet struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_octet().size() * struct_data.var_array_octet()[0].size() *
            struct_data.var_array_octet()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_octet().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_octet()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_octet()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_octet()[i][j][k],
                            test_value[i * struct_data.var_array_octet().size() *
                            struct_data.var_array_octet()[i].size() + j *
                            struct_data.var_array_octet()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(char_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_CHAR8), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    CharSeq value = {'e', 'P', 'r', 'o', 's', 'i', 'm', 'a', '.', ' ', 'a', 'm', 'i', 's', 'o', 'r', 'P', 'e'};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 'E');
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 'p');
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 'R');
    CharSeq test_value;
    EXPECT_EQ(data->set_char8_values(data->get_member_id_by_name(var_char_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_values(test_value, data->get_member_id_by_name(var_char_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionChar struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_char().size() * struct_data.var_array_char()[0].size() *
            struct_data.var_array_char()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_char().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_char()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_char()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_char()[i][j][k],
                            test_value[i * struct_data.var_array_char().size() *
                            struct_data.var_array_char()[i].size() + j *
                            struct_data.var_array_char()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(wchar_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wchar_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_CHAR16), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WcharSeq value =
    {L'e', L'P', L'r', L'o', L's', L'i', L'm', L'a', L'.', L' ', L'a', L'm', L'i', L's', L'o', L'r',
     L'P', L'e'};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), L'E');
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), L'p');
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), L'R');
    WcharSeq test_value;
    EXPECT_EQ(data->set_char16_values(data->get_member_id_by_name(var_wchar_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(test_value, data->get_member_id_by_name(var_wchar_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionWChar struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_wchar().size() * struct_data.var_array_wchar()[0].size() *
            struct_data.var_array_wchar()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_wchar().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_wchar()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_wchar()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_wchar()[i][j][k],
                            test_value[i * struct_data.var_array_wchar().size() *
                            struct_data.var_array_wchar()[i].size() + j *
                            struct_data.var_array_wchar()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(string_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_string_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_string_type(
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build(), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StringSeq value =
    {"Lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing", "elit", "sed", "do",
     "eiusmod", "tempor", "incididunt", "ut", "labore", "et", "dolore", "magna", "aliqua"};
    value.insert(value.end(), 81, "");
    value.insert(value.end(), "Ut");
    value.insert(value.end(), 100, "");
    value.insert(value.end(), "enim");
    value.insert(value.end(), 100, "");
    value.insert(value.end(), "ad");
    StringSeq test_value;
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(var_string_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_value, data->get_member_id_by_name(var_string_array)), RETCODE_OK);
    value.insert(value.end(), 697, "");
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionString struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_string().size() * struct_data.var_array_string()[0].size() *
            struct_data.var_array_string()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_string().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_string()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_string()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_string()[i][j][k],
                            test_value[i * struct_data.var_array_string().size() *
                            struct_data.var_array_string()[i].size() + j *
                            struct_data.var_array_string()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(wstring_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wstring_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_wstring_type(
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build(), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WstringSeq value =
    {L"Lorem", L"ipsum", L"dolor", L"sit", L"amet", L"consectetur", L"adipiscing", L"elit", L"sed",
     L"do", L"eiusmod", L"tempor", L"incididunt", L"ut", L"labore", L"et", L"dolore", L"magna",
     L"aliqua"};
    value.insert(value.end(), 81, L"");
    value.insert(value.end(), L"Ut");
    value.insert(value.end(), 100, L"");
    value.insert(value.end(), L"enim");
    value.insert(value.end(), 100, L"");
    value.insert(value.end(), L"ad");
    WstringSeq test_value;
    EXPECT_EQ(data->set_wstring_values(data->get_member_id_by_name(var_wstring_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_values(test_value, data->get_member_id_by_name(var_wstring_array)), RETCODE_OK);
    value.insert(value.end(), 697, L"");
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionWString struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_wstring().size() * struct_data.var_array_wstring()[0].size() *
            struct_data.var_array_wstring()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_wstring().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_wstring()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_wstring()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_wstring()[i][j][k],
                            test_value[i * struct_data.var_array_wstring().size() *
                            struct_data.var_array_wstring()[i].size() + j *
                            struct_data.var_array_wstring()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionBoundedString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_string_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_string_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(
                create_inner_alias_bounded_string_helper(), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StringSeq value =
    {"Lorem", "ipsum", "dolor", "sit", "amet", "consectetu", "adipiscing", "elit", "sed", "do",
     "eiusmod", "tempor", "incididunt", "ut", "labore", "et", "dolore", "magna", "aliqua"};
    value.insert(value.end(), 81, "");
    value.insert(value.end(), "Ut");
    value.insert(value.end(), 100, "");
    value.insert(value.end(), "enim");
    value.insert(value.end(), 100, "");
    value.insert(value.end(), "ad");
    StringSeq test_value;
    EXPECT_EQ(data->set_string_values(data->get_member_id_by_name(var_bounded_string_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_values(test_value, data->get_member_id_by_name(var_bounded_string_array)), RETCODE_OK);
    value.insert(value.end(), 697, "");
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionBoundedString struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionBoundedStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_bounded_string().size() * struct_data.var_array_bounded_string()[0].size() *
            struct_data.var_array_bounded_string()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_bounded_string().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_bounded_string()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_bounded_string()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_bounded_string()[i][j][k],
                            test_value[i * struct_data.var_array_bounded_string().size() *
                            struct_data.var_array_bounded_string()[i].size() + j *
                            struct_data.var_array_bounded_string()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionBoundedString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionBoundedWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bounded_wstring_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_wstring_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(
                create_inner_alias_bounded_wstring_helper(), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    WstringSeq value =
    {L"Lorem", L"ipsum", L"dolor", L"sit", L"amet", L"consectetu", L"adipiscing", L"elit", L"sed",
     L"do", L"eiusmod", L"tempor", L"incididunt", L"ut", L"labore", L"et", L"dolore", L"magna",
     L"aliqua"};
    value.insert(value.end(), 81, L"");
    value.insert(value.end(), L"Ut");
    value.insert(value.end(), 100, L"");
    value.insert(value.end(), L"enim");
    value.insert(value.end(), 100, L"");
    value.insert(value.end(), L"ad");
    WstringSeq test_value;
    EXPECT_EQ(data->set_wstring_values(data->get_member_id_by_name(var_bounded_wstring_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_values(test_value, data->get_member_id_by_name(var_bounded_wstring_array)), RETCODE_OK);
    value.insert(value.end(), 697, L"");
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionBoundedWString struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionBoundedWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_bounded_wstring().size() * struct_data.var_array_bounded_wstring()[0].size() *
            struct_data.var_array_bounded_wstring()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_bounded_wstring().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_bounded_wstring()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_bounded_wstring()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_bounded_wstring()[i][j][k],
                            test_value[i * struct_data.var_array_bounded_wstring().size() *
                            struct_data.var_array_bounded_wstring()[i].size() + j *
                            struct_data.var_array_bounded_wstring()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionBoundedWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionEnum)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(enum_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_enum_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_enum_helper(),
            {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1),
        static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3)
    };
    value.insert(value.end(), 88, 0);
    value.insert(value.end(), static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3));
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1));
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2));
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_enum_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_enum_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionEnum struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionEnumPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_enum().size() * struct_data.var_array_enum()[0].size() *
            struct_data.var_array_enum()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_enum().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_enum()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_enum()[i][j].size(); ++k)
                {
                    EXPECT_EQ(static_cast<int32_t>(struct_data.var_array_enum()[i][j][k]),
                            test_value[i * struct_data.var_array_enum().size() *
                            struct_data.var_array_enum()[i].size() + j *
                            struct_data.var_array_enum()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionEnum_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionBitMask)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitmask_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitmask_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_bitmask_helper(),
            {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    UInt32Seq value = {
        InnerBitMaskHelperBits::flag0,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag4,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag1 |
        InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6,
        InnerBitMaskHelperBits::flag0,
        InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1
    };
    value.insert(value.end(), 88, 0);
    value.insert(value.end(), InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag4);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag6);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), InnerBitMaskHelperBits::flag1 | InnerBitMaskHelperBits::flag4);
    UInt32Seq test_value;
    EXPECT_EQ(data->set_uint32_values(data->get_member_id_by_name(var_bitmask_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(test_value, data->get_member_id_by_name(var_bitmask_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionBitMask struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionBitMaskPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_bitmask().size() * struct_data.var_array_bitmask()[0].size() *
            struct_data.var_array_bitmask()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_bitmask().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_bitmask()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_bitmask()[i][j].size(); ++k)
                {
                    EXPECT_EQ(static_cast<uint32_t>(struct_data.var_array_bitmask()[i][j][k]),
                            test_value[i * struct_data.var_array_bitmask().size() *
                            struct_data.var_array_bitmask()[i].size() + j *
                            struct_data.var_array_bitmask()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionBitMask_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionAlias)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(alias_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_alias_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_alias_helper(),
            {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value =
    {1, 2, 3, 4, 5, -6, -7, -8, -9, -10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
     1000000000};
    value.insert(value.end(), 82, 0);
    value.insert(value.end(), 10);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 100);
    value.insert(value.end(), 100, 0);
    value.insert(value.end(), 1000);
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_alias_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_alias_array)), RETCODE_OK);
    value.insert(value.end(), 697, 0);
    EXPECT_EQ(value.size(), test_value.size());
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionAlias struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(
            struct_data.var_array_alias().size() * struct_data.var_array_alias()[0].size() *
            struct_data.var_array_alias()[0][0].size(), test_value.size());
        for (size_t i = 0; i < struct_data.var_array_alias().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_alias()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_alias()[i][j].size(); ++k)
                {
                    EXPECT_EQ(struct_data.var_array_alias()[i][j][k],
                            test_value[i * struct_data.var_array_alias().size() *
                            struct_data.var_array_alias()[i].size() + j *
                            struct_data.var_array_alias()[i].size() + k]);
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionSequence)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(seq_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_seq_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_sequence_type(DynamicTypeBuilderFactory::get_instance()->
                    get_primitive_type(TK_INT32),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build(), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq first_value = {10, -100, 1000, -20, 200};
    Int32Seq second_value = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Int32Seq third_value = {0, -1, -2};
    Int32Seq test_value;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_seq_array));
    ASSERT_TRUE(array_data);
    EXPECT_EQ(array_data->set_int32_values(0, first_value), RETCODE_OK);
    EXPECT_EQ(array_data->get_int32_values(test_value, 0), RETCODE_OK);
    EXPECT_EQ(first_value, test_value);
    EXPECT_EQ(array_data->set_int32_values(33, second_value), RETCODE_OK);
    EXPECT_EQ(array_data->get_int32_values(test_value, 33), RETCODE_OK);
    EXPECT_EQ(second_value, test_value);
    EXPECT_EQ(array_data->set_int32_values(857, third_value), RETCODE_OK);
    EXPECT_EQ(array_data->get_int32_values(test_value, 857), RETCODE_OK);
    EXPECT_EQ(third_value, test_value);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionSequence struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionSequencePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        for (size_t i = 0; i < struct_data.var_array_sequence().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_sequence()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_sequence()[i][j].size(); ++k)
                {
                    if (i == 0 && j == 0 && k == 0)
                    {
                        EXPECT_EQ(struct_data.var_array_sequence()[i][j][k], first_value);
                    }
                    else if (i == 0 && j == 3 && k == 3)
                    {
                        EXPECT_EQ(struct_data.var_array_sequence()[i][j][k], second_value);
                    }
                    else if (i == 8 && j == 5 && k == 7)
                    {
                        EXPECT_EQ(struct_data.var_array_sequence()[i][j][k], third_value);
                    }
                    else
                    {
                        EXPECT_TRUE(struct_data.var_array_sequence()[i][j][k].empty());
                    }
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionSequence_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(map_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_map_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->create_map_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                TK_INT32), DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build(), {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_key = 121;
    int32_t second_key = -11;
    int32_t third_key = 1001;
    int32_t first_value = 1;
    int32_t second_value = 2;
    int32_t third_value = 3;
    int32_t test_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_map_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type map_data = array_data->loan_value(0);
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                first_key)), first_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                first_key))), RETCODE_OK);
    EXPECT_EQ(first_value, test_value);
    EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
    map_data = array_data->loan_value(33);
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                second_key)), second_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                second_key))), RETCODE_OK);
    EXPECT_EQ(second_value, test_value);
    EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
    map_data = array_data->loan_value(857);
    ASSERT_TRUE(map_data);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                third_key)), third_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(test_value, map_data->get_member_id_by_name(std::to_string(
                third_key))), RETCODE_OK);
    EXPECT_EQ(third_value, test_value);
    EXPECT_EQ(array_data->return_loaned_value(map_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionMap struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        for (size_t i = 0; i < struct_data.var_array_map().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_map()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_map()[i][j].size(); ++k)
                {
                    if (i == 0 && j == 0 && k == 0)
                    {
                        EXPECT_EQ(struct_data.var_array_map()[i][j][k][first_key], first_value);
                    }
                    else if (i == 0 && j == 3 && k == 3)
                    {
                        EXPECT_EQ(struct_data.var_array_map()[i][j][k][second_key], second_value);
                    }
                    else if (i == 8 && j == 5 && k == 7)
                    {
                        EXPECT_EQ(struct_data.var_array_map()[i][j][k][third_key], third_value);
                    }
                    else
                    {
                        EXPECT_TRUE(struct_data.var_array_map()[i][j][k].empty());
                    }
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionUnion)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(union_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_union_helper(),
            {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t long_value = 121;
    int32_t test_long_value = 0;
    float float_value = 10.01f;
    float test_float_value = 0;
    int16_t short_value = -2;
    int16_t test_short_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_union_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type union_data = array_data->loan_value(0);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                union_long_member_name), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                union_long_member_name)), RETCODE_OK);
    EXPECT_EQ(test_long_value, long_value);
    EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    union_data = array_data->loan_value(33);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_float32_value(union_data->get_member_id_by_name(
                union_float_member_name), float_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float32_value(test_float_value, union_data->get_member_id_by_name(
                union_float_member_name)), RETCODE_OK);
    EXPECT_EQ(test_float_value, float_value);
    EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    union_data = array_data->loan_value(857);
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                union_short_member_name), short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_short_value, union_data->get_member_id_by_name(
                union_short_member_name)), RETCODE_OK);
    EXPECT_EQ(test_short_value, short_value);
    EXPECT_EQ(array_data->return_loaned_value(union_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionUnion struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionUnionPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        for (size_t i = 0; i < struct_data.var_array_union().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_union()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_union()[i][j].size(); ++k)
                {
                    if (i == 0 && j == 0 && k == 0)
                    {
                        EXPECT_EQ(struct_data.var_array_union()[i][j][k].longValue(), long_value);
                    }
                    else if (i == 0 && j == 3 && k == 3)
                    {
                        EXPECT_EQ(struct_data.var_array_union()[i][j][k].floatValue(), float_value);
                    }
                    else if (i == 8 && j == 5 && k == 7)
                    {
                        EXPECT_EQ(struct_data.var_array_union()[i][j][k].shortValue(), short_value);
                    }
                    else
                    {
                        EXPECT_EQ(struct_data.var_array_union()[i][j][k].shortValue(), 0);
                    }
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionUnion_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_struct_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_struct_helper(),
            {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_long_value = 121;
    int32_t second_long_value = 10001;
    int32_t third_long_value = -421523;
    int32_t test_long_value = 10001;
    float first_float_value = 10.01f;
    float second_float_value = 3.14f;
    float third_float_value = 62346.757f;
    float test_float_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_struct_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type data_struct = array_data->loan_value(0);
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
    EXPECT_EQ(array_data->return_loaned_value(data_struct), RETCODE_OK);
    data_struct = array_data->loan_value(33);
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
    EXPECT_EQ(array_data->return_loaned_value(data_struct), RETCODE_OK);
    data_struct = array_data->loan_value(857);
    ASSERT_TRUE(data_struct);
    EXPECT_EQ(data_struct->set_int32_value(data_struct->get_member_id_by_name(
                struct_long_member_name), third_long_value), RETCODE_OK);
    EXPECT_EQ(data_struct->get_int32_value(test_long_value, data_struct->get_member_id_by_name(
                struct_long_member_name)), RETCODE_OK);
    EXPECT_EQ(test_long_value, third_long_value);
    EXPECT_EQ(data_struct->set_float32_value(data_struct->get_member_id_by_name(struct_float_member_name),
            third_float_value), RETCODE_OK);
    EXPECT_EQ(data_struct->get_float32_value(test_float_value,
            data_struct->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
    EXPECT_EQ(test_float_value, third_float_value);
    EXPECT_EQ(array_data->return_loaned_value(data_struct), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionStructure struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionStructurePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        for (size_t i = 0; i < struct_data.var_array_structure().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_structure()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_structure()[i][j].size(); ++k)
                {
                    if (i == 0 && j == 0 && k == 0)
                    {
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field1(), first_long_value);
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field2(), first_float_value);
                    }
                    else if (i == 0 && j == 3 && k == 3)
                    {
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field1(), second_long_value);
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field2(), second_float_value);
                    }
                    else if (i == 8 && j == 5 && k == 7)
                    {
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field1(), third_long_value);
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field2(), third_float_value);
                    }
                    else
                    {
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field1(), 0);
                        EXPECT_EQ(struct_data.var_array_structure()[i][j][k].field2(), 0);
                    }
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_ArrayMultiDimensionBitset)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitset_multiarray_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitset_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(create_inner_bitset_helper(),
            {10, 10, 10})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint8_t first_uint8_value = 5;
    uint8_t second_uint8_value = 7;
    uint8_t third_uint8_value = 1;
    uint8_t test_uint8_value = 0;
    bool first_bool_value = true;
    bool second_bool_value = false;
    bool third_bool_value = true;
    bool test_bool_value = false;
    uint16_t first_ushort_value = 1000;
    uint16_t second_ushort_value = 555;
    uint16_t third_ushort_value = 12;
    uint16_t test_ushort_value = 0;
    int16_t first_short_value = 2000;
    int16_t second_short_value = 20;
    int16_t third_short_value = 13;
    int16_t test_short_value = 0;
    DynamicData::_ref_type array_data = data->loan_value(data->get_member_id_by_name(var_bitset_array));
    ASSERT_TRUE(array_data);
    DynamicData::_ref_type bitset_data = array_data->loan_value(0);
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
    EXPECT_EQ(array_data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = array_data->loan_value(33);
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
    EXPECT_EQ(array_data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = array_data->loan_value(857);
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), third_uint8_value),
            RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(third_uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(
                bitfield_b), third_bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(third_bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(
                bitfield_c), third_ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(third_ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(
                bitfield_d), third_short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(third_short_value, test_short_value);
    EXPECT_EQ(array_data->return_loaned_value(bitset_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        ArrayMultiDimensionBitset struct_data;
        TypeSupport static_pubsubType {new ArrayMultiDimensionBitsetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        for (size_t i = 0; i < struct_data.var_array_bitset().size(); ++i)
        {
            for (size_t j = 0; j < struct_data.var_array_bitset()[i].size(); ++j)
            {
                for (size_t k = 0; k < struct_data.var_array_bitset()[i][j].size(); ++k)
                {
                    if (i == 0 && j == 0 && k == 0)
                    {
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].a, first_uint8_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].b, first_bool_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].c, first_ushort_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].d, first_short_value);
                    }
                    else if (i == 0 && j == 3 && k == 3)
                    {
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].a, second_uint8_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].b, second_bool_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].c, second_ushort_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].d, second_short_value);
                    }
                    else if (i == 8 && j == 5 && k == 7)
                    {
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].a, third_uint8_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].b, third_bool_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].c, third_ushort_value);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].d, third_short_value);
                    }
                    else
                    {
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].a, 0u);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].b, false);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].c, 0u);
                        EXPECT_EQ(struct_data.var_array_bitset()[i][j][k].d, 0);
                    }
                }
            }
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_ArrayMultiDimensionBitset_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

// IDLs checking constants and constant operations are not included as they do not add any new check in Dynamic
// language binding API.

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BoundedBigArrays)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(large_array_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(type_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_large_array);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), {41925})->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {1, 2, 3, 4, 5, -6, -7, -8, -9, -10, 11, 12, 13};
    value.insert(value.end(), 987, 0);
    value.insert(value.end(), 1001);
    value.insert(value.end(), 40000, 0);
    value.insert(value.end(), -346);
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_large_array), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(var_large_array)), RETCODE_OK);
    value.insert(value.end(), 923, 0);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        BoundedBigArrays struct_data;
        TypeSupport static_pubsubType {new BoundedBigArraysPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_array_big().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_big()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BoundedBigArrays_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
