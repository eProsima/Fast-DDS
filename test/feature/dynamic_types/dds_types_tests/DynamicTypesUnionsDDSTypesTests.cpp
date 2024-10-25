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
#include "../../../dds-types-test/unionsPubSubTypes.hpp"
#include "../../../dds-types-test/unionsTypeObjectSupport.hpp"
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* struct_union_short_name {"UnionShort"};
constexpr const char* struct_union_ushort_name {"UnionUShort"};
constexpr const char* struct_union_long_name {"UnionLong"};
constexpr const char* struct_union_ulong_name {"UnionULong"};
constexpr const char* struct_union_long_long_name {"UnionLongLong"};
constexpr const char* struct_union_ulong_long_name {"UnionULongLong"};
constexpr const char* struct_union_float_name {"UnionFloat"};
constexpr const char* struct_union_double_name {"UnionDouble"};
constexpr const char* struct_union_long_double_name {"UnionLongDouble"};
constexpr const char* struct_union_bool_name {"UnionBoolean"};
constexpr const char* struct_union_byte_name {"UnionOctet"};
constexpr const char* struct_union_char_name {"UnionChar"};
constexpr const char* struct_union_wchar_name {"UnionWChar"};
constexpr const char* struct_union_string_name {"UnionString"};
constexpr const char* struct_union_wstring_name {"UnionWString"};
constexpr const char* struct_union_bounded_string_name {"UnionBoundedString"};
constexpr const char* struct_union_bounded_wstring_name {"UnionBoundedWString"};
constexpr const char* struct_union_enum_name {"UnionInnerEnumHelper"};
constexpr const char* struct_union_bitmask_name {"UnionInnerBitMaskHelper"};
constexpr const char* struct_union_alias_name {"UnionInnerAliasHelper"};
constexpr const char* struct_union_array_name {"UnionArray"};
constexpr const char* struct_union_seq_name {"UnionSequence"};
constexpr const char* struct_union_map_name {"UnionMap"};
constexpr const char* struct_union_union_name {"UnionInnerUnionHelper"};
constexpr const char* struct_union_struct_name {"UnionInnerStructureHelper"};
constexpr const char* struct_union_bitset_name {"UnionInnerBitsetHelper"};
constexpr const char* struct_union_short_discriminator_name {"UnionDiscriminatorShort"};
constexpr const char* struct_union_ushort_discriminator_name {"UnionDiscriminatorUShort"};
constexpr const char* struct_union_long_discriminator_name {"UnionDiscriminatorLong"};
constexpr const char* struct_union_ulong_discriminator_name {"UnionDiscriminatorULong"};
constexpr const char* struct_union_long_long_discriminator_name {"UnionDiscriminatorLongLong"};
constexpr const char* struct_union_ulong_long_discriminator_name {"UnionDiscriminatorULongLong"};
constexpr const char* struct_union_bool_discriminator_name {"UnionDiscriminatorBoolean"};
constexpr const char* struct_union_byte_discriminator_name {"UnionDiscriminatorOctet"};
constexpr const char* struct_union_char_discriminator_name {"UnionDiscriminatorChar"};
constexpr const char* struct_union_wchar_discriminator_name {"UnionDiscriminatorWChar"};
constexpr const char* struct_union_enum_discriminator_name {"UnionDiscriminatorEnum"};
constexpr const char* struct_union_enum_label_discriminator_name {"UnionDiscriminatorEnumLabel"};
constexpr const char* struct_union_alias_discriminator_name {"UnionDiscriminatorAlias"};
constexpr const char* struct_union_several_fields_name {"UnionSeveralFields"};
constexpr const char* struct_union_several_fields_with_default_name {"UnionSeveralFieldsWithDefault"};
constexpr const char* struct_union_short_extra_member_name {"UnionShortExtraMember"};
constexpr const char* struct_union_fixed_string_alias_name {"UnionFixedStringAlias"};

constexpr const char* union_short_name {"Union_Short"};
constexpr const char* union_ushort_name {"Union_UShort"};
constexpr const char* union_long_name {"Union_Long"};
constexpr const char* union_ulong_name {"Union_ULong"};
constexpr const char* union_long_long_name {"Union_LongLong"};
constexpr const char* union_ulong_long_name {"Union_ULongLOng"};
constexpr const char* union_float_name {"Union_Float"};
constexpr const char* union_double_name {"Union_Double"};
constexpr const char* union_long_double_name {"Union_LongDouble"};
constexpr const char* union_bool_name {"Union_Boolean"};
constexpr const char* union_byte_name {"Union_Octet"};
constexpr const char* union_char_name {"Union_Char"};
constexpr const char* union_wchar_name {"Union_WChar"};
constexpr const char* union_string_name {"Union_String"};
constexpr const char* union_wstring_name {"Union_WString"};
constexpr const char* union_bounded_string_name {"Union_BoundedString"};
constexpr const char* union_bounded_wstring_name {"Union_BoundedWString"};
constexpr const char* union_enum_name {"Union_InnerEnumHelper"};
constexpr const char* union_bitmask_name {"Union_InnerBitMaskHelper"};
constexpr const char* union_alias_name {"Union_InnerAliasHelper"};
constexpr const char* union_array_name {"Union_Array"};
constexpr const char* union_seq_name {"Union_Sequence"};
constexpr const char* union_map_name {"Union_Map"};
constexpr const char* union_union_name {"Union_InnerUnionHelper"};
constexpr const char* union_struct_name {"Union_InnerStructureHelper"};
constexpr const char* union_bitset_name {"Union_InnerBitsetHelper"};
constexpr const char* union_short_discriminator_name {"Union_Discriminator_short"};
constexpr const char* union_ushort_discriminator_name {"Union_Discriminator_unsigned_short"};
constexpr const char* union_long_discriminator_name {"Union_Discriminator_long"};
constexpr const char* union_ulong_discriminator_name {"Union_Discriminator_unsigned_long"};
constexpr const char* union_long_long_discriminator_name {"Union_Discriminator_long_long"};
constexpr const char* union_ulong_long_discriminator_name {"Union_Discriminator_unsigned_long_long"};
constexpr const char* union_bool_discriminator_name {"Union_Discriminator_boolean"};
constexpr const char* union_byte_discriminator_name {"Union_Discriminator_octet"};
constexpr const char* union_char_discriminator_name {"Union_Discriminator_char"};
constexpr const char* union_wchar_discriminator_name {"Union_Discriminator_wchar"};
constexpr const char* union_enum_discriminator_name {"Union_Discriminator_enum"};
constexpr const char* union_enum_label_discriminator_name {"Union_Discriminator_enum_labels"};
constexpr const char* union_alias_discriminator_name {"Union_Discriminator_alias"};
constexpr const char* union_several_fields_name {"Union_Several_Fields"};
constexpr const char* union_several_fields_with_default_name {"Union_Several_Fields_With_Default"};
constexpr const char* union_fixed_string_in_module_alias_name {"Union_Fixed_String_In_Module_Alias"};

constexpr const char* alias_fixed_string_module_name {"Fixed_String_Module::fixed_string_in_module"};

constexpr const char* var_union_short_name {"var_union_short"};
constexpr const char* var_union_ushort_name {"var_union_ushort"};
constexpr const char* var_union_long_name {"var_union_long"};
constexpr const char* var_union_ulong_name {"var_union_ulong"};
constexpr const char* var_union_long_long_name {"var_union_long_long"};
constexpr const char* var_union_ulong_long_name {"var_union_ulong_long"};
constexpr const char* var_union_float_name {"var_union_float"};
constexpr const char* var_union_double_name {"var_union_double"};
constexpr const char* var_union_long_double_name {"var_union_long_double"};
constexpr const char* var_union_bool_name {"var_union_boolean"};
constexpr const char* var_union_byte_name {"var_union_octet"};
constexpr const char* var_union_char_name {"var_union_char"};
constexpr const char* var_union_wchar_name {"var_union_wchar"};
constexpr const char* var_union_string_name {"var_union_string"};
constexpr const char* var_union_wstring_name {"var_union_wstring"};
constexpr const char* var_union_bounded_string_name {"var_union_bounded_string"};
constexpr const char* var_union_bounded_wstring_name {"var_union_bounded_wstring"};
constexpr const char* var_union_enum_name {"var_union_my_enum"};
constexpr const char* var_union_bitmask_name {"var_union_my_bit_mask"};
constexpr const char* var_union_alias_name {"var_union_my_alias"};
constexpr const char* var_union_array_name {"var_union_array"};
constexpr const char* var_union_seq_name {"var_union_sequence"};
constexpr const char* var_union_map_name {"var_union_map"};
constexpr const char* var_union_union_name {"var_union_my_union"};
constexpr const char* var_union_struct_name {"var_union_my_structure"};
constexpr const char* var_union_bitset_name {"var_union_my_bitset"};
constexpr const char* var_union_short_discriminator_name {"var_union_discriminator_short"};
constexpr const char* var_union_ushort_discriminator_name {"var_union_discriminator_ushort"};
constexpr const char* var_union_long_discriminator_name {"var_union_discriminator_long"};
constexpr const char* var_union_ulong_discriminator_name {"var_union_discriminator_ulong"};
constexpr const char* var_union_long_long_discriminator_name {"var_union_discriminator_long_long"};
constexpr const char* var_union_ulong_long_discriminator_name {"var_union_discriminator_ulong_long"};
constexpr const char* var_union_bool_discriminator_name {"var_union_discriminator_boolean"};
constexpr const char* var_union_byte_discriminator_name {"var_union_discriminator_octet"};
constexpr const char* var_union_char_discriminator_name {"var_union_discriminator_char"};
constexpr const char* var_union_wchar_discriminator_name {"var_union_discriminator_wchar"};
constexpr const char* var_union_enum_discriminator_name {"var_union_discriminator_enum"};
constexpr const char* var_union_alias_discriminator_name {"var_union_discriminator_alias"};
constexpr const char* var_union_several_fields_name {"var_union_several_fields"};
constexpr const char* var_union_several_fields_with_default_name {"var_union_several_fields_with_default"};
constexpr const char* var_union_fixed_string_in_module_alias_name {"var_union_fixed_string_in_module_alias"};

constexpr const char* var_union_member_a {"a"};
constexpr const char* var_union_member_b {"b"};
constexpr const char* var_union_member_c {"c"};
constexpr const char* var_union_member_d {"d"};
constexpr const char* var_union_member_e {"e"};
constexpr const char* var_union_member_f {"f"};
constexpr const char* var_union_member_g {"g"};
constexpr const char* var_union_member_h {"h"};
constexpr const char* var_union_member_i {"i"};
constexpr const char* var_union_member_j {"j"};
constexpr const char* var_union_member_k {"k"};
constexpr const char* var_union_member_l {"l"};
constexpr const char* var_union_member_m {"m"};
constexpr const char* var_union_member_n {"n"};
constexpr const char* var_union_member_o {"o"};
constexpr const char* var_union_member_bn {"bn"};
constexpr const char* var_union_member_bo {"bo"};
constexpr const char* var_union_member_p {"p"};
constexpr const char* var_union_member_q {"q"};
constexpr const char* var_union_member_r {"r"};
constexpr const char* var_union_member_s {"s"};
constexpr const char* var_union_member_t {"t"};
constexpr const char* var_union_member_u {"u"};
constexpr const char* var_union_member_v {"v"};
constexpr const char* var_union_member_w {"w"};
constexpr const char* var_union_member_x {"x"};
constexpr const char* var_union_first_member {"first"};
constexpr const char* var_union_second_member {"second"};
constexpr const char* var_union_third_member {"third"};

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_short_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_short_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_a);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    member_descriptor->label({0});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_short_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_short_name));
    ASSERT_TRUE(union_data);

    int16_t value = 16;
    int16_t test_value = 23;
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(var_union_member_a), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_value, union_data->get_member_id_by_name(var_union_member_a)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionShort struct_data;
        TypeSupport static_pubsubType {new UnionShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_short().a(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_ushort_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_ushort_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_b);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_ushort_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_ushort_name));
    ASSERT_TRUE(union_data);

    uint16_t value = 16;
    uint16_t test_value = 23;
    EXPECT_EQ(union_data->set_uint16_value(union_data->get_member_id_by_name(var_union_member_b), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_uint16_value(test_value, union_data->get_member_id_by_name(
                var_union_member_b)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionUShort struct_data;
        TypeSupport static_pubsubType {new UnionUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_ushort().b(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_long_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_long_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_c);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_long_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_long_name));
    ASSERT_TRUE(union_data);

    int32_t value = 16;
    int32_t test_value = 23;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(var_union_member_c), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_value, union_data->get_member_id_by_name(var_union_member_c)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionLong struct_data;
        TypeSupport static_pubsubType {new UnionLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_long().c(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_ulong_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_ulong_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_d);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    member_descriptor->label({3});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_ulong_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_ulong_name));
    ASSERT_TRUE(union_data);

    uint32_t value = 16;
    uint32_t test_value = 23;
    EXPECT_EQ(union_data->set_uint32_value(union_data->get_member_id_by_name(var_union_member_d), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_uint32_value(test_value, union_data->get_member_id_by_name(
                var_union_member_d)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionULong struct_data;
        TypeSupport static_pubsubType {new UnionULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_ulong().d(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_long_long_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_long_long_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_e);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({4});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_long_long_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_long_long_name));
    ASSERT_TRUE(union_data);

    int64_t value = 16;
    int64_t test_value = 23;
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(var_union_member_e), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_value, union_data->get_member_id_by_name(var_union_member_e)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionLongLong struct_data;
        TypeSupport static_pubsubType {new UnionLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_long_long().e(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_ulong_long_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_ulong_long_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_f);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    member_descriptor->label({5});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_ulong_long_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_ulong_long_name));
    ASSERT_TRUE(union_data);

    uint64_t value = 16;
    uint64_t test_value = 23;
    EXPECT_EQ(union_data->set_uint64_value(union_data->get_member_id_by_name(var_union_member_f), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_uint64_value(test_value, union_data->get_member_id_by_name(
                var_union_member_f)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionULongLong struct_data;
        TypeSupport static_pubsubType {new UnionULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_ulong_long().f(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionFloat)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_float_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_float_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_g);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    member_descriptor->label({6});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_float_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_float_name));
    ASSERT_TRUE(union_data);

    float value = 16.235f;
    float test_value = 23.0f;
    EXPECT_EQ(union_data->set_float32_value(union_data->get_member_id_by_name(var_union_member_g), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float32_value(test_value, union_data->get_member_id_by_name(
                var_union_member_g)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionFloat struct_data;
        TypeSupport static_pubsubType {new UnionFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_float().g(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_double_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_double_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_h);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT64));
    member_descriptor->label({7});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_double_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_double_name));
    ASSERT_TRUE(union_data);

    double value = 16.235;
    double test_value = 23.0;
    EXPECT_EQ(union_data->set_float64_value(union_data->get_member_id_by_name(var_union_member_h), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float64_value(test_value, union_data->get_member_id_by_name(
                var_union_member_h)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDouble struct_data;
        TypeSupport static_pubsubType {new UnionDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_double().h(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionLongDouble)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_long_double_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_long_double_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_i);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT128));
    member_descriptor->label({8});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_long_double_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_long_double_name));
    ASSERT_TRUE(union_data);

    long double value = 16.235;
    long double test_value = 23.0;
    EXPECT_EQ(union_data->set_float128_value(union_data->get_member_id_by_name(var_union_member_i), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float128_value(test_value, union_data->get_member_id_by_name(
                var_union_member_i)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionLongDouble struct_data;
        TypeSupport static_pubsubType {new UnionLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_long_double().i(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_bool_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_bool_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_j);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    member_descriptor->label({9});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_bool_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_bool_name));
    ASSERT_TRUE(union_data);

    bool value = true;
    bool test_value = false;
    EXPECT_EQ(union_data->set_boolean_value(union_data->get_member_id_by_name(var_union_member_j), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_boolean_value(test_value, union_data->get_member_id_by_name(
                var_union_member_j)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionBoolean struct_data;
        TypeSupport static_pubsubType {new UnionBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_boolean().j(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_byte_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_byte_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_k);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    member_descriptor->label({10});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_byte_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_byte_name));
    ASSERT_TRUE(union_data);

    eprosima::fastdds::rtps::octet value = 15;
    eprosima::fastdds::rtps::octet test_value = 134;
    EXPECT_EQ(union_data->set_byte_value(union_data->get_member_id_by_name(var_union_member_k), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_byte_value(test_value, union_data->get_member_id_by_name(var_union_member_k)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionOctet struct_data;
        TypeSupport static_pubsubType {new UnionOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_octet().k(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_char_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_char_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_l);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    member_descriptor->label({11});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_char_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_char_name));
    ASSERT_TRUE(union_data);

    char value = 'd';
    char test_value = '0';
    EXPECT_EQ(union_data->set_char8_value(union_data->get_member_id_by_name(var_union_member_l), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_char8_value(test_value, union_data->get_member_id_by_name(var_union_member_l)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionChar struct_data;
        TypeSupport static_pubsubType {new UnionCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_char().l(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_wchar_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_wchar_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_m);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR16));
    member_descriptor->label({12});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_wchar_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_wchar_name));
    ASSERT_TRUE(union_data);

    wchar_t value = L'd';
    wchar_t test_value = L'0';
    EXPECT_EQ(union_data->set_char16_value(union_data->get_member_id_by_name(var_union_member_m), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_char16_value(test_value, union_data->get_member_id_by_name(
                var_union_member_m)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionWChar struct_data;
        TypeSupport static_pubsubType {new UnionWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_wchar().m(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_string_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_string_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_n);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    member_descriptor->label({13});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_string_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_string_name));
    ASSERT_TRUE(union_data);

    std::string value = "HELLO WORLD";
    std::string test_value;
    EXPECT_EQ(union_data->set_string_value(union_data->get_member_id_by_name(var_union_member_n), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_string_value(test_value, union_data->get_member_id_by_name(
                var_union_member_n)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionString struct_data;
        TypeSupport static_pubsubType {new UnionStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_string().n(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_wstring_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_wstring_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_o);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_wstring_type(static_cast<uint32_t>(
                LENGTH_UNLIMITED))->build());
    member_descriptor->label({14});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_wstring_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_wstring_name));
    ASSERT_TRUE(union_data);

    std::wstring value = L"HELLO WORLD";
    std::wstring test_value;
    EXPECT_EQ(union_data->set_wstring_value(union_data->get_member_id_by_name(var_union_member_o), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_wstring_value(test_value, union_data->get_member_id_by_name(
                var_union_member_o)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionWString struct_data;
        TypeSupport static_pubsubType {new UnionWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_wstring().o(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionBoundedString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_bounded_string_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_bounded_string_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_bn);
    member_descriptor->type(create_inner_alias_bounded_string_helper());
    member_descriptor->label({13});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_bounded_string_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_bounded_string_name));
    ASSERT_TRUE(union_data);

    std::string value = "HELLO";
    std::string test_value;
    EXPECT_EQ(union_data->set_string_value(union_data->get_member_id_by_name(var_union_member_bn), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_string_value(test_value, union_data->get_member_id_by_name(
                var_union_member_bn)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionBoundedString struct_data;
        TypeSupport static_pubsubType {new UnionBoundedStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_bounded_string().bn(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionBoundedString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionBoundedWString)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_bounded_wstring_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_bounded_wstring_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_bo);
    member_descriptor->type(create_inner_alias_bounded_wstring_helper());
    member_descriptor->label({14});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_bounded_wstring_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_bounded_wstring_name));
    ASSERT_TRUE(union_data);

    std::wstring value = L"HELLO";
    std::wstring test_value;
    EXPECT_EQ(union_data->set_wstring_value(union_data->get_member_id_by_name(var_union_member_bo), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_wstring_value(test_value, union_data->get_member_id_by_name(
                var_union_member_bo)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionBoundedWString struct_data;
        TypeSupport static_pubsubType {new UnionBoundedWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_bounded_wstring().bo(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionBoundedWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionInnerEnumHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_enum_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_enum_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_p);
    member_descriptor->type(create_inner_enum_helper());
    member_descriptor->label({15});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_enum_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_enum_name));
    ASSERT_TRUE(union_data);

    InnerEnumHelper value = InnerEnumHelper::ENUM_VALUE_3;
    int32_t test_value = 1243;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(var_union_member_p),
            static_cast<int32_t>(value)), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_value, union_data->get_member_id_by_name(
                var_union_member_p)), RETCODE_OK);
    EXPECT_EQ(static_cast<int32_t>(value), test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionInnerEnumHelper struct_data;
        TypeSupport static_pubsubType {new UnionInnerEnumHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(static_cast<int32_t>(struct_data.var_union_my_enum().p()), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionInnerEnumHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionInnerBitMaskHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_bitmask_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_bitmask_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_q);
    member_descriptor->type(create_inner_bitmask_helper());
    member_descriptor->label({16});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_bitmask_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_bitmask_name));
    ASSERT_TRUE(union_data);

    InnerBitMaskHelper value = InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1 |
            InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6;
    InnerBitMaskHelper test_value = 1243;
    EXPECT_EQ(union_data->set_uint32_value(union_data->get_member_id_by_name(var_union_member_q), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_uint32_value(test_value, union_data->get_member_id_by_name(
                var_union_member_q)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionInnerBitMaskHelper struct_data;
        TypeSupport static_pubsubType {new UnionInnerBitMaskHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_my_bit_mask().q(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionInnerBitMaskHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionInnerAliasHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_alias_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_alias_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_r);
    member_descriptor->type(create_inner_alias_helper());
    member_descriptor->label({17});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_alias_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_alias_name));
    ASSERT_TRUE(union_data);

    int32_t value = 321;
    int32_t test_value = 1243;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(var_union_member_r), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_value, union_data->get_member_id_by_name(var_union_member_r)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionInnerAliasHelper struct_data;
        TypeSupport static_pubsubType {new UnionInnerAliasHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_my_alias().r(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionInnerAliasHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionArray)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_array_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_array_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_s);
    member_descriptor->type(create_inner_alias_array_helper());
    member_descriptor->label({18});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_array_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_array_name));
    ASSERT_TRUE(union_data);

    Int16Seq value = {321, 123};
    Int16Seq test_value;
    EXPECT_EQ(union_data->set_int16_values(union_data->get_member_id_by_name(var_union_member_s), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_values(test_value, union_data->get_member_id_by_name(
                var_union_member_s)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionArray struct_data;
        TypeSupport static_pubsubType {new UnionArrayPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_array().s().size(), test_value.size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_union_array().s()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionArray_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionSequence)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_seq_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_seq_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_t);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->label({19});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_seq_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_seq_name));
    ASSERT_TRUE(union_data);

    Int16Seq value = {321, 123, 5345};
    Int16Seq test_value;
    EXPECT_EQ(union_data->set_int16_values(union_data->get_member_id_by_name(var_union_member_t), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_values(test_value, union_data->get_member_id_by_name(
                var_union_member_t)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionSequence struct_data;
        TypeSupport static_pubsubType {new UnionSequencePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_sequence().t(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionSequence_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionMap)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_map_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_map_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_u);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32),
            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                TK_INT32), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->label({20});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_map_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_map_name));
    ASSERT_TRUE(union_data);

    int32_t first_key = 12214;
    int32_t second_key = 1243;
    int32_t first_value = 13;
    int32_t second_value = 352;
    int32_t test_value = 0;
    DynamicData::_ref_type map_data = union_data->loan_value(union_data->get_member_id_by_name(var_union_member_u));
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
    EXPECT_EQ(union_data->return_loaned_value(map_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionMap struct_data;
        TypeSupport static_pubsubType {new UnionMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        union_data = data->loan_value(data->get_member_id_by_name(var_union_map_name));
        ASSERT_TRUE(union_data);
        map_data = union_data->loan_value(union_data->get_member_id_by_name(var_union_member_u));
        ASSERT_TRUE(map_data);
        EXPECT_EQ(struct_data.var_union_map().u().size(), map_data->get_item_count());
        EXPECT_EQ(struct_data.var_union_map().u()[first_key], first_value);
        EXPECT_EQ(struct_data.var_union_map().u()[second_key], second_value);
        EXPECT_EQ(union_data->return_loaned_value(map_data), RETCODE_OK);
        EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionInnerUnionHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_union_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_union_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_v);
    member_descriptor->type(create_inner_union_helper());
    member_descriptor->label({21});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_union_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_union_name));
    ASSERT_TRUE(union_data);

    float float_value = 23.5;
    float test_float_value = 0;
    int16_t short_value = 55;
    int16_t test_short_value = 0;
    int32_t long_value = 31425;
    int32_t test_long_value = 0;
    DynamicData::_ref_type union_union_data =
            union_data->loan_value(union_data->get_member_id_by_name(var_union_member_v));
    ASSERT_TRUE(union_union_data);
    EXPECT_EQ(union_union_data->set_float32_value(union_union_data->get_member_id_by_name(union_float_member_name),
            float_value), RETCODE_OK);
    EXPECT_EQ(union_union_data->get_float32_value(test_float_value,
            union_union_data->get_member_id_by_name(union_float_member_name)), RETCODE_OK);
    EXPECT_EQ(float_value, test_float_value);
    EXPECT_EQ(union_union_data->set_int16_value(union_union_data->get_member_id_by_name(union_short_member_name),
            short_value), RETCODE_OK);
    EXPECT_EQ(union_union_data->get_int16_value(test_short_value,
            union_union_data->get_member_id_by_name(union_short_member_name)), RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(union_union_data->set_int32_value(union_union_data->get_member_id_by_name(union_long_member_name),
            long_value), RETCODE_OK);
    EXPECT_EQ(union_union_data->get_int32_value(test_long_value,
            union_union_data->get_member_id_by_name(union_long_member_name)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->return_loaned_value(union_union_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionInnerUnionHelper struct_data;
        TypeSupport static_pubsubType {new UnionInnerUnionHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_my_union().v().longValue(), test_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionInnerUnionHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionInnerStructureHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_struct_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_w);
    member_descriptor->type(create_inner_struct_helper());
    member_descriptor->label({22});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_struct_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_struct_name));
    ASSERT_TRUE(union_data);

    float float_value = 23.5;
    float test_float_value = 0;
    int32_t long_value = 31425;
    int32_t test_long_value = 0;
    DynamicData::_ref_type union_struct_data =
            union_data->loan_value(union_data->get_member_id_by_name(var_union_member_w));
    ASSERT_TRUE(union_struct_data);
    EXPECT_EQ(union_struct_data->set_float32_value(union_struct_data->get_member_id_by_name(struct_float_member_name),
            float_value), RETCODE_OK);
    EXPECT_EQ(union_struct_data->get_float32_value(test_float_value,
            union_struct_data->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
    EXPECT_EQ(float_value, test_float_value);
    EXPECT_EQ(union_struct_data->set_int32_value(union_struct_data->get_member_id_by_name(struct_long_member_name),
            long_value), RETCODE_OK);
    EXPECT_EQ(union_struct_data->get_int32_value(test_long_value,
            union_struct_data->get_member_id_by_name(struct_long_member_name)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->return_loaned_value(union_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionInnerStructureHelper struct_data;
        TypeSupport static_pubsubType {new UnionInnerStructureHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_my_structure().w().field1(), test_long_value);
        EXPECT_EQ(struct_data.var_union_my_structure().w().field2(), test_float_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionInnerStructureHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionInnerBitsetHelper)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_bitset_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_bitset_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_x);
    member_descriptor->type(create_inner_bitset_helper());
    member_descriptor->label({23});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_bitset_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_bitset_name));
    ASSERT_TRUE(union_data);

    uint8_t uint8_value = 5;
    uint8_t test_uint8_value = 0;
    bool bool_value = true;
    bool test_bool_value = false;
    uint16_t ushort_value = 1000;
    uint16_t test_ushort_value = 0;
    int16_t short_value = 2000;
    int16_t test_short_value = 0;
    DynamicData::_ref_type bitset_data = union_data->loan_value(union_data->get_member_id_by_name(var_union_member_x));
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
    EXPECT_EQ(union_data->return_loaned_value(bitset_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionInnerBitsetHelper struct_data;
        TypeSupport static_pubsubType {new UnionInnerBitsetHelperPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_my_bitset().x().a, test_uint8_value);
        EXPECT_EQ(struct_data.var_union_my_bitset().x().b, test_bool_value);
        EXPECT_EQ(struct_data.var_union_my_bitset().x().c, test_ushort_value);
        EXPECT_EQ(struct_data.var_union_my_bitset().x().d, test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionInnerBitsetHelper_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_short_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_short_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({-2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_short_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_short_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorShort struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_short().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorUShort)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_ushort_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_ushort_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_ushort_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(
                        var_union_ushort_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorUShort struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorUShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_ushort().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorUShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_long_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_long_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({-2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_long_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_long_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorLong struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_long().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorULong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_ulong_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_ulong_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_ulong_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_ulong_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorULong struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorULongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_ulong().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorULong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorLongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_long_long_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_long_long_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({-2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_long_long_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_long_long_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorLongLong struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_long_long().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorULongLong)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_ulong_long_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_ulong_long_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_ulong_long_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_ulong_long_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorULongLong struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorULongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_ulong_long().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorULongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorBoolean)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_bool_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_bool_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({true});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({false});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_bool_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_bool_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorBoolean struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_boolean().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorOctet)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_byte_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_byte_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({0});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_byte_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_byte_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorOctet struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_octet().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_char_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_char_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({'a'});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({'b'});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_char_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_char_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorChar struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_char().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorWChar)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_wchar_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_wchar_discriminator_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR16));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({L'a'});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({L'b'});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_wchar_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_wchar_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorWChar struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorWCharPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_wchar().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorWChar_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorEnum)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_enum_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_enum_discriminator_name);
    type_descriptor->discriminator_type(create_inner_enum_helper());
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1)});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2)});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_third_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    member_descriptor->is_default_label(true);
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_enum_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_enum_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    eprosima::fastdds::rtps::octet byte_value = 234;
    eprosima::fastdds::rtps::octet test_byte_value = 12;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_byte_value(union_data->get_member_id_by_name(
                var_union_third_member), byte_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_byte_value(test_byte_value, union_data->get_member_id_by_name(
                var_union_third_member)), RETCODE_OK);
    EXPECT_EQ(byte_value, test_byte_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorEnum struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorEnumPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_enum().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorEnum_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorEnumLabel)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_enum_label_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_enum_label_discriminator_name);
    type_descriptor->discriminator_type(create_inner_enum_helper());
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    member_descriptor->label({static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_3),
                              static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_1)});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    member_descriptor->label({static_cast<int32_t>(InnerEnumHelper::ENUM_VALUE_2)});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_enum_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_enum_discriminator_name));
    ASSERT_TRUE(union_data);

    int16_t first_short_value = 14241;
    int16_t second_short_value = 463;
    int16_t test_short_value = 34;
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                var_union_first_member), first_short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_short_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(first_short_value, test_short_value);
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                var_union_second_member), second_short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(test_short_value, union_data->get_member_id_by_name(
                var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(second_short_value, test_short_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorEnumLabel struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorEnumLabelPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_enum().second(), test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorEnumLabel_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionDiscriminatorAlias)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_alias_discriminator_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(struct_builder);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_alias_discriminator_name);
    type_descriptor->discriminator_type(create_inner_alias_helper());
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};
    ASSERT_TRUE(union_builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_first_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_second_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_alias_discriminator_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_alias_discriminator_name));
    ASSERT_TRUE(union_data);

    int32_t long_value = 14241;
    int32_t test_long_value = 0;
    int64_t long_long_value = 3221624;
    int64_t test_long_long_value = 0;
    EXPECT_EQ(union_data->set_int32_value(union_data->get_member_id_by_name(
                var_union_first_member), long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(test_long_value, union_data->get_member_id_by_name(
                var_union_first_member)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(union_data->set_int64_value(union_data->get_member_id_by_name(
                var_union_second_member), long_long_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int64_value(test_long_long_value,
            union_data->get_member_id_by_name(var_union_second_member)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionDiscriminatorAlias struct_data;
        TypeSupport static_pubsubType {new UnionDiscriminatorAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_discriminator_alias().second(), test_long_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionDiscriminatorAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionSeveralFields)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_several_fields_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_several_fields_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_a);
    member_descriptor->type(create_inner_struct_helper());
    member_descriptor->label({0});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_b);
    member_descriptor->type(create_inner_empty_struct_helper());
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_c);
    member_descriptor->type(create_inner_alias_bounded_string_helper());
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_d);
    member_descriptor->type(create_inner_alias_array_helper());
    member_descriptor->label({3});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_e);
    member_descriptor->type(create_inner_alias_sequence_helper());
    member_descriptor->label({4});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_several_fields_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data = data->loan_value(data->get_member_id_by_name(var_union_several_fields_name));
    ASSERT_TRUE(union_data);

    Int16Seq value {16, 32, -13};
    Int16Seq test_value;
    EXPECT_EQ(union_data->set_int16_values(union_data->get_member_id_by_name(var_union_member_e), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_values(test_value, union_data->get_member_id_by_name(var_union_member_e)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionSeveralFields struct_data;
        TypeSupport static_pubsubType {new UnionSeveralFieldsPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_several_fields().e(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionSeveralFields_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionSeveralFieldsWithDefault)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_several_fields_with_default_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_several_fields_with_default_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_a);
    member_descriptor->type(create_inner_struct_helper());
    member_descriptor->label({0});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_b);
    member_descriptor->type(create_inner_empty_struct_helper());
    member_descriptor->label({1});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_c);
    member_descriptor->type(create_inner_alias_bounded_string_helper());
    member_descriptor->label({2});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_d);
    member_descriptor->type(create_inner_alias_array_helper());
    member_descriptor->label({3});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_e);
    member_descriptor->type(create_inner_alias_sequence_helper());
    member_descriptor->label({4});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_member_f);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16), 30)->build());
    member_descriptor->is_default_label(true);
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_several_fields_with_default_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);
    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);
    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_several_fields_with_default_name));
    ASSERT_TRUE(union_data);

    Int16Seq value {16, 32, -13};
    Int16Seq test_value;
    EXPECT_EQ(union_data->set_int16_values(union_data->get_member_id_by_name(var_union_member_f), value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_values(test_value, union_data->get_member_id_by_name(var_union_member_f)),
            RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionSeveralFieldsWithDefault struct_data;
        TypeSupport static_pubsubType {new UnionSeveralFieldsWithDefaultPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_several_fields_with_default().f(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionSeveralFieldsWithDefault_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

// Regression test for issue #21773
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionShortExtraMember)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_short_extra_member_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_short_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_a);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    member_descriptor->label({0});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_short_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    // Set no value for union (will be empty as no default is set)

    // Set value for extra member
    int32_t test_value = 23;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_long_name), test_value), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionShortExtraMember struct_data;
        TypeSupport static_pubsubType {new UnionShortExtraMemberPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_THROW(struct_data.var_union_short().a(), eprosima::fastcdr::exception::BadParamException);
        EXPECT_EQ(struct_data.var_long(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionShortExtraMember_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_UnionFixedStringAlias)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_fixed_string_alias_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name(union_fixed_string_in_module_alias_name);
    type_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name(alias_fixed_string_module_name);
    type_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_string_type(20)->build());

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_member_a);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)->build());
    member_descriptor->label({0});
    union_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_fixed_string_in_module_alias_name);
    member_descriptor->type(union_builder->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    DynamicData::_ref_type union_data =
            data->loan_value(data->get_member_id_by_name(var_union_fixed_string_in_module_alias_name));
    ASSERT_TRUE(union_data);

    // Set value for fixed string
    const std::string value {"Test"};
    std::string test_value;
    EXPECT_EQ(union_data->set_string_value(union_data->get_member_id_by_name(var_union_member_a), value), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    union_data =
            data->loan_value(data->get_member_id_by_name(var_union_fixed_string_in_module_alias_name));
    ASSERT_TRUE(union_data);
    EXPECT_EQ(union_data->get_string_value(test_value, union_data->get_member_id_by_name(
                var_union_member_a)), RETCODE_OK);
    EXPECT_EQ(value, test_value);
    EXPECT_EQ(data->return_loaned_value(union_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        UnionFixedStringAlias struct_data;
        TypeSupport static_pubsubType {new UnionFixedStringAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(struct_data.var_union_fixed_string_in_module_alias().a(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_UnionFixedStringAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

// This union is custom and serves to test a specific case used in internal type of DDS X-Types (TypeIdentifier).
// No supported by Dynamic Language Binding.
// TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_DefaultAnnotation)

} // dds
} // fastdds
} // eprosima
