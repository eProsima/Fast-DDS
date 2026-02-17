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
#include "../../../dds-types-test/structuresPubSubTypes.hpp"
#include "../../../dds-types-test/structuresTypeObjectSupport.hpp"
#include <fastdds/dds/topic/TypeSupport.hpp>
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

constexpr const char* struct_short_name = "StructShort";
constexpr const char* struct_ushort_name = "StructUnsignedShort";
constexpr const char* struct_long_name = "StructLong";
constexpr const char* struct_ulong_name = "StructUnsignedLong";
constexpr const char* struct_long_long_name = "StructLongLong";
constexpr const char* struct_ulong_long_name = "StructUnsignedLongLong";
constexpr const char* struct_float_name = "StructFloat";
constexpr const char* struct_double_name = "StructDouble";
constexpr const char* struct_long_double_name = "StructLongDouble";
constexpr const char* struct_bool_name = "StructBoolean";
constexpr const char* struct_byte_name = "StructOctet";
constexpr const char* struct_char_name = "StructChar8";
constexpr const char* struct_wchar_name = "StructChar16";
constexpr const char* struct_string_name = "StructString";
constexpr const char* struct_wstring_name = "StructWString";
constexpr const char* struct_bounded_string_name = "StructBoundedString";
constexpr const char* struct_bounded_wstring_name = "StructBoundedWString";
constexpr const char* struct_enum_name = "StructEnum";
constexpr const char* struct_bitmask_name = "StructBitMask";
constexpr const char* struct_alias_name = "StructAlias";
constexpr const char* struct_array_name = "StructShortArray";
constexpr const char* struct_seq_name = "StructSequence";
constexpr const char* struct_map_name = "StructMap";
constexpr const char* struct_union_name = "StructUnion";
constexpr const char* struct_structure_name = "StructStructure";
constexpr const char* struct_bitset_name = "StructBitset";
constexpr const char* struct_empty_name = "StructEmpty";

constexpr const char* var_string_name = "var_string";
constexpr const char* var_wstring_name = "var_wstring";
constexpr const char* var_bounded_string_name = "var_bounded_string";
constexpr const char* var_bounded_wstring_name = "var_bounded_wstring";
constexpr const char* var_enum_name = "var_enum";
constexpr const char* var_bitmask_name = "var_bitmask";
constexpr const char* var_alias_name = "var_alias";
constexpr const char* var_array_name = "var_array_short";
constexpr const char* var_seq_name = "var_sequence";
constexpr const char* var_map_name = "var_map";
constexpr const char* var_struct_name = "var_structure";
constexpr const char* var_bitset_name = "var_bitset";

constexpr const char* var_short_struct_name = "var_StructShort";
constexpr const char* var_ushort_struct_name = "var_StructUnsignedShort";
constexpr const char* var_long_struct_name = "var_StructLong";
constexpr const char* var_ulong_struct_name = "var_StructUnsignedLong";
constexpr const char* var_long_long_struct_name = "var_StructLongLong";
constexpr const char* var_ulong_long_struct_name = "var_StructUnsignedLongLong";
constexpr const char* var_float_struct_name = "var_StructFloat";
constexpr const char* var_double_struct_name = "var_StructDouble";
constexpr const char* var_long_double_struct_name = "var_StructLongDouble";
constexpr const char* var_bool_struct_name = "var_StructBoolean";
constexpr const char* var_byte_struct_name = "var_StructOctet";
constexpr const char* var_char_struct_name = "var_StructChar8";
constexpr const char* var_wchar_struct_name = "var_StructChar16";
constexpr const char* var_string_struct_name = "var_StructString";
constexpr const char* var_wstring_struct_name = "var_StructWString";
constexpr const char* var_bounded_string_struct_name = "var_StructBoundedString";
constexpr const char* var_bounded_wstring_struct_name = "var_StructBoundedWString";
constexpr const char* var_enum_struct_name = "var_StructEnum";
constexpr const char* var_bitmask_struct_name = "var_StructBitMask";
constexpr const char* var_alias_struct_name = "var_StructAlias";
constexpr const char* var_array_struct_name = "var_StructShortArray";
constexpr const char* var_seq_struct_name = "var_StructSequence";
constexpr const char* var_map_struct_name = "var_StructMap";
constexpr const char* var_union_struct_name = "var_StructUnion";
constexpr const char* var_struct_struct_name = "var_StructStructure";
constexpr const char* var_bitset_struct_name = "var_StructBitset";
constexpr const char* var_empty_struct_name = "var_StructEmpty";

constexpr const int32_t first_map_key = 10;
constexpr const int32_t second_map_key = 101;

constexpr const char* root_struct_name = "root";
constexpr const char* root1_struct_name = "root1";
constexpr const char* root2_struct_name = "root2";
constexpr const char* foo_struct_name = "foo";
constexpr const char* bar_struct_name = "bar";
constexpr const char* var_foo_struct_name = "foo_struct";
constexpr const char* var_bar_struct_name = "bar_struct";
constexpr const char* var_root1_struct_name = "var_root1";
constexpr const char* var_root2_struct_name = "var_root2";
constexpr const char* var_a_name = "a";
constexpr const char* var_b_name = "b";
constexpr const char* var_c_name = "c";
constexpr const char* var_d_name = "d";
constexpr const char* var_e_name = "e";

/**
 * Auxiliary functions to create structures
 */
DynamicType::_ref_type create_short_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_short_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_unsigned_short_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ushort_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ushort_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_long_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_unsigned_long_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ulong_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_long_long_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_long_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_unsigned_long_long_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_ulong_long_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_ulong_long_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_float_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_float_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_float_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_double_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_double_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_double_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT64));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_long_double_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_long_double_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_long_double_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT128));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_boolean_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bool_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bool_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_octet_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_byte_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_byte_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_char_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_char_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_wchar_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_wchar_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_wchar_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR16));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_string_struct()
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

    return type_builder->build();
}

DynamicType::_ref_type create_wstring_struct()
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

    return type_builder->build();
}

DynamicType::_ref_type create_bounded_string_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bounded_string_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_string_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_alias_bounded_string_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_bounded_wstring_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bounded_wstring_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bounded_wstring_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_alias_bounded_wstring_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_enum_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_enum_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_enum_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_enum_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_bitmask_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bitmask_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitmask_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_bitmask_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_alias_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_alias_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_alias_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_short_array_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_array_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_array_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), {10})->build());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_sequence_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_seq_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_seq_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_map_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_map_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_map_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_map_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32),
            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32),
            static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_union_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_union_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_union_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_union_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_structure_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_structure_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_struct_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_struct_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_bitset_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bitset_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_bitset_name);
    member_descriptor->type(DynamicTypesDDSTypesTest::create_inner_bitset_helper());
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_empty_struct()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_empty_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    return type_builder->build();
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructShort)
{
    DynamicType::_ref_type struct_type = create_short_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t value = 2;
    int16_t test_value = 0;
    EXPECT_EQ(data->set_int16_value(data->get_member_id_by_name(var_short_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test_value, data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructShort struct_data;
        TypeSupport static_pubsubType {new StructShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_short(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructUnsignedShort)
{
    DynamicType::_ref_type struct_type = create_unsigned_short_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint16_t value = 2;
    uint16_t test_value = 0;
    EXPECT_EQ(data->set_uint16_value(data->get_member_id_by_name(var_ushort_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test_value, data->get_member_id_by_name(var_ushort_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructUnsignedShort struct_data;
        TypeSupport static_pubsubType {new StructUnsignedShortPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_ushort(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructUnsignedShort_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructLong)
{
    DynamicType::_ref_type struct_type = create_long_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructLong struct_data;
        TypeSupport static_pubsubType {new StructLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_long(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructUnsignedLong)
{
    DynamicType::_ref_type struct_type = create_unsigned_long_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint32_t value = 2;
    uint32_t test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_ulong_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_ulong_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructUnsignedLong struct_data;
        TypeSupport static_pubsubType {new StructUnsignedLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_ulong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructUnsignedLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructLongLong)
{
    DynamicType::_ref_type struct_type = create_long_long_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int64_t value = 2;
    int64_t test_value = 0;
    EXPECT_EQ(data->set_int64_value(data->get_member_id_by_name(var_long_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test_value, data->get_member_id_by_name(var_long_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructLongLong struct_data;
        TypeSupport static_pubsubType {new StructLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_longlong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructUnsignedLongLong)
{
    DynamicType::_ref_type struct_type = create_unsigned_long_long_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint64_t value = 2;
    uint64_t test_value = 0;
    EXPECT_EQ(data->set_uint64_value(data->get_member_id_by_name(var_ulong_long_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test_value, data->get_member_id_by_name(var_ulong_long_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructUnsignedLongLong struct_data;
        TypeSupport static_pubsubType {new StructUnsignedLongLongPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_ulonglong(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructUnsignedLongLong_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructFloat)
{
    DynamicType::_ref_type struct_type = create_float_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float value = 2;
    float test_value = 0;
    EXPECT_EQ(data->set_float32_value(data->get_member_id_by_name(var_float_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test_value, data->get_member_id_by_name(var_float_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructFloat struct_data;
        TypeSupport static_pubsubType {new StructFloatPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_float(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructFloat_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructDouble)
{
    DynamicType::_ref_type struct_type = create_double_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    double value = 2;
    double test_value = 0;
    EXPECT_EQ(data->set_float64_value(data->get_member_id_by_name(var_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test_value, data->get_member_id_by_name(var_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructDouble struct_data;
        TypeSupport static_pubsubType {new StructDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_double(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructLongDouble)
{
    DynamicType::_ref_type struct_type = create_long_double_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    long double value = 2;
    long double test_value = 0;
    EXPECT_EQ(data->set_float128_value(data->get_member_id_by_name(var_long_double_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test_value, data->get_member_id_by_name(var_long_double_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructLongDouble struct_data;
        TypeSupport static_pubsubType {new StructLongDoublePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_longdouble(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructLongDouble_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructBoolean)
{
    DynamicType::_ref_type struct_type = create_boolean_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    bool value = true;
    bool test_value = false;
    EXPECT_EQ(data->set_boolean_value(data->get_member_id_by_name(var_bool_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test_value, data->get_member_id_by_name(var_bool_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructBoolean struct_data;
        TypeSupport static_pubsubType {new StructBooleanPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_boolean(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructBoolean_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructOctet)
{
    DynamicType::_ref_type struct_type = create_octet_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    eprosima::fastdds::rtps::octet value = 255;
    eprosima::fastdds::rtps::octet test_value = 0;
    EXPECT_EQ(data->set_byte_value(data->get_member_id_by_name(var_byte_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test_value, data->get_member_id_by_name(var_byte_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructOctet struct_data;
        TypeSupport static_pubsubType {new StructOctetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_octet(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructOctet_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructChar8)
{
    DynamicType::_ref_type struct_type = create_char_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    char value = 'a';
    char test_value = 'b';
    EXPECT_EQ(data->set_char8_value(data->get_member_id_by_name(var_char_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test_value, data->get_member_id_by_name(var_char_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructChar8 struct_data;
        TypeSupport static_pubsubType {new StructChar8PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_char8(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructChar8_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructChar16)
{
    DynamicType::_ref_type struct_type = create_wchar_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    wchar_t value = L'a';
    wchar_t test_value = L'b';
    EXPECT_EQ(data->set_char16_value(data->get_member_id_by_name(var_wchar_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test_value, data->get_member_id_by_name(var_wchar_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructChar16 struct_data;
        TypeSupport static_pubsubType {new StructChar16PubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_char16(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructChar16_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructString)
{
    DynamicType::_ref_type struct_type = create_string_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string value = "STRING_TEST";
    std::string test_value = "";
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_string_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_value, data->get_member_id_by_name(var_string_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructString struct_data;
        TypeSupport static_pubsubType {new StructStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_string(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructWString)
{
    DynamicType::_ref_type struct_type = create_wstring_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::wstring value = L"STRING_TEST";
    std::wstring test_value = L"";
    EXPECT_EQ(data->set_wstring_value(data->get_member_id_by_name(var_wstring_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test_value, data->get_member_id_by_name(var_wstring_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructWString struct_data;
        TypeSupport static_pubsubType {new StructWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_wstring(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructBoundedString)
{
    DynamicType::_ref_type struct_type = create_bounded_string_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::string value = "TEST";
    std::string test_value = "";
    EXPECT_EQ(data->set_string_value(data->get_member_id_by_name(var_bounded_string_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test_value, data->get_member_id_by_name(var_bounded_string_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructBoundedString struct_data;
        TypeSupport static_pubsubType {new StructBoundedStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_bounded_string(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructBoundedString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructBoundedWString)
{
    DynamicType::_ref_type struct_type = create_bounded_wstring_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    std::wstring value = L"TEST";
    std::wstring test_value = L"";
    EXPECT_EQ(data->set_wstring_value(data->get_member_id_by_name(var_bounded_wstring_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test_value, data->get_member_id_by_name(var_bounded_wstring_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructBoundedWString struct_data;
        TypeSupport static_pubsubType {new StructBoundedWStringPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_bounded_wstring(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructBoundedWString_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructEnum)
{
    DynamicType::_ref_type struct_type = create_enum_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerEnumHelper value = InnerEnumHelper::ENUM_VALUE_2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(
                var_enum_name), static_cast<uint32_t>(value)), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_enum_name)), RETCODE_OK);
    EXPECT_EQ(static_cast<int32_t>(value), test_value);

    for (auto encoding : encodings)
    {
        StructEnum struct_data;
        TypeSupport static_pubsubType {new StructEnumPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(static_cast<int32_t>(struct_data.var_enum()), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructEnum_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructBitMask)
{
    DynamicType::_ref_type struct_type = create_bitmask_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerBitMaskHelper value = InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1 |
            InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6;
    InnerBitMaskHelper test_value = 0;
    EXPECT_EQ(data->set_uint32_value(data->get_member_id_by_name(var_bitmask_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test_value, data->get_member_id_by_name(var_bitmask_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructBitMask struct_data;
        TypeSupport static_pubsubType {new StructBitMaskPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_bitmask(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructBitMask_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructAlias)
{
    DynamicType::_ref_type struct_type = create_alias_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 2;
    int32_t test_value = 0;
    EXPECT_EQ(data->set_int32_value(data->get_member_id_by_name(var_alias_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test_value, data->get_member_id_by_name(var_alias_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructAlias struct_data;
        TypeSupport static_pubsubType {new StructAliasPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_alias(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructAlias_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructShortArray)
{
    DynamicType::_ref_type struct_type = create_short_array_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int16Seq value = {2, 5, 0, 0, 0, 0, 0, 0, 0, 0};
    Int16Seq test_value;
    EXPECT_EQ(data->set_int16_values(data->get_member_id_by_name(var_array_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(test_value, data->get_member_id_by_name(var_array_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructShortArray struct_data;
        TypeSupport static_pubsubType {new StructShortArrayPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(test_value.size(), struct_data.var_array_short().size());
        for (size_t i = 0; i < test_value.size(); ++i)
        {
            EXPECT_EQ(struct_data.var_array_short()[i], test_value[i]);
        }
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructShortArray_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructSequence)
{
    DynamicType::_ref_type struct_type = create_sequence_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    Int32Seq value = {2, 5};
    Int32Seq test_value;
    EXPECT_EQ(data->set_int32_values(data->get_member_id_by_name(var_seq_name), value), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(test_value, data->get_member_id_by_name(var_seq_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        StructSequence struct_data;
        TypeSupport static_pubsubType {new StructSequencePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_sequence(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructSequence_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructMap)
{
    DynamicType::_ref_type struct_type = create_map_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t first_key = 1;
    int32_t second_key = 5;
    int32_t first_value = 1;
    int32_t second_value = 10;
    int32_t test_value = 0;
    DynamicData::_ref_type map_data = data->loan_value(data->get_member_id_by_name(var_map_name));
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
    EXPECT_EQ(data->return_loaned_value(map_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        StructMap struct_data;
        TypeSupport static_pubsubType {new StructMapPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        map_data = data->loan_value(data->get_member_id_by_name(var_map_name));
        ASSERT_TRUE(map_data);
        EXPECT_EQ(struct_data.var_map().size(), map_data->get_item_count());
        EXPECT_EQ(struct_data.var_map()[first_key], first_value);
        EXPECT_EQ(struct_data.var_map()[second_key], second_value);
        EXPECT_EQ(data->return_loaned_value(map_data), RETCODE_OK);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructMap_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructUnion)
{
    DynamicType::_ref_type struct_type = create_union_struct();

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
        StructUnion struct_data;
        TypeSupport static_pubsubType {new StructUnionPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_union().shortValue(), test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructUnion_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructStructure)
{
    DynamicType::_ref_type struct_type = create_structure_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    float float_value = 23.5;
    float test_float_value = 0;
    int32_t long_value = 55;
    int32_t test_long_value = 0;
    DynamicData::_ref_type struct_data = data->loan_value(data->get_member_id_by_name(var_struct_name));
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
        StructStructure data_struct;
        TypeSupport static_pubsubType {new StructStructurePubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, data_struct, static_pubsubType);
        EXPECT_EQ(data_struct.var_structure().field1(), test_long_value);
        EXPECT_EQ(data_struct.var_structure().field2(), test_float_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructBitset)
{
    DynamicType::_ref_type struct_type = create_bitset_struct();

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
    DynamicData::_ref_type bitset_data = data->loan_value(data->get_member_id_by_name(var_bitset_name));
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
        StructBitset data_struct;
        TypeSupport static_pubsubType {new StructBitsetPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, data_struct, static_pubsubType);
        EXPECT_EQ(data_struct.var_bitset().a, test_uint8_value);
        EXPECT_EQ(data_struct.var_bitset().b, test_bool_value);
        EXPECT_EQ(data_struct.var_bitset().c, test_ushort_value);
        EXPECT_EQ(data_struct.var_bitset().d, test_short_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructBitset_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructEmpty)
{
    DynamicType::_ref_type struct_type = create_empty_struct();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructEmpty_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

struct testing_values_struct
{
    int16_t test_short_value;
    uint16_t test_ushort_value;
    int32_t test_long_value;
    uint32_t test_ulong_value;
    int64_t test_long_long_value;
    uint64_t test_ulong_long_value;
    float test_float_value;
    double test_double_value;
    long double test_long_double_value;
    bool test_bool_value;
    eprosima::fastdds::rtps::octet test_byte_value;
    char test_char_value;
    wchar_t test_wchar_value;
    std::string test_string_value;
    std::wstring test_wstring_value;
    std::string test_bounded_string_value;
    std::wstring test_bounded_wstring_value;
    int32_t test_enum_value;
    InnerBitMaskHelper test_bitmask_value;
    Int16Seq test_array_value;
    Int32Seq test_seq_value;
    int32_t test_map_value;
    uint8_t test_uint8_value;
};

void check_structure_static_data(
        const DynamicData::_ref_type& data,
        const Structures& struct_data,
        const testing_values_struct& testing_values)
{
    DynamicData::_ref_type map_struct_data = data->loan_value(data->get_member_id_by_name(var_map_struct_name));
    ASSERT_TRUE(map_struct_data);
    DynamicData::_ref_type map_data = map_struct_data->loan_value(map_struct_data->get_member_id_by_name(var_map_name));
    ASSERT_TRUE(map_data);

    EXPECT_EQ(struct_data.var_StructShort().var_short(), testing_values.test_short_value);
    EXPECT_EQ(struct_data.var_StructUnsignedShort().var_ushort(), testing_values.test_ushort_value);
    EXPECT_EQ(struct_data.var_StructLong().var_long(), testing_values.test_long_value);
    EXPECT_EQ(struct_data.var_StructUnsignedLong().var_ulong(), testing_values.test_ulong_value);
    EXPECT_EQ(struct_data.var_StructLongLong().var_longlong(), testing_values.test_long_long_value);
    EXPECT_EQ(struct_data.var_StructUnsignedLongLong().var_ulonglong(), testing_values.test_ulong_long_value);
    EXPECT_EQ(struct_data.var_StructFloat().var_float(), testing_values.test_float_value);
    EXPECT_EQ(struct_data.var_StructDouble().var_double(), testing_values.test_double_value);
    EXPECT_EQ(struct_data.var_StructLongDouble().var_longdouble(), testing_values.test_long_double_value);
    EXPECT_EQ(struct_data.var_StructBoolean().var_boolean(), testing_values.test_bool_value);
    EXPECT_EQ(struct_data.var_StructOctet().var_octet(), testing_values.test_byte_value);
    EXPECT_EQ(struct_data.var_StructChar8().var_char8(), testing_values.test_char_value);
    EXPECT_EQ(struct_data.var_StructChar16().var_char16(), testing_values.test_wchar_value);
    EXPECT_EQ(struct_data.var_StructString().var_string(), testing_values.test_string_value);
    EXPECT_EQ(struct_data.var_StructWString().var_wstring(), testing_values.test_wstring_value);
    EXPECT_EQ(struct_data.var_StructBoundedString().var_bounded_string(), testing_values.test_bounded_string_value);
    EXPECT_EQ(struct_data.var_StructBoundedWString().var_bounded_wstring(), testing_values.test_bounded_wstring_value);
    EXPECT_EQ(static_cast<int32_t>(struct_data.var_StructEnum().var_enum()), testing_values.test_enum_value);
    EXPECT_EQ(struct_data.var_StructBitMask().var_bitmask(), testing_values.test_bitmask_value);
    EXPECT_EQ(struct_data.var_StructAlias().var_alias(), testing_values.test_long_value);
    EXPECT_EQ(struct_data.var_StructShortArray().var_array_short().size(), testing_values.test_array_value.size());
    for (size_t i = 0; i < testing_values.test_array_value.size(); ++i)
    {
        EXPECT_EQ(struct_data.var_StructShortArray().var_array_short()[i], testing_values.test_array_value[i]);
    }
    EXPECT_EQ(struct_data.var_StructSequence().var_sequence(), testing_values.test_seq_value);
    EXPECT_EQ(struct_data.var_StructMap().var_map().size(), map_data->get_item_count());
    EXPECT_EQ(struct_data.var_StructMap().var_map().at(first_map_key), testing_values.test_long_value);
    EXPECT_EQ(struct_data.var_StructMap().var_map().at(second_map_key), testing_values.test_map_value);
    EXPECT_EQ(struct_data.var_StructUnion().var_union().shortValue(), testing_values.test_short_value);
    EXPECT_EQ(struct_data.var_StructStructure().var_structure().field1(), testing_values.test_long_value);
    EXPECT_EQ(struct_data.var_StructStructure().var_structure().field2(), testing_values.test_float_value);
    EXPECT_EQ(struct_data.var_StructBitset().var_bitset().a, testing_values.test_uint8_value);
    EXPECT_EQ(struct_data.var_StructBitset().var_bitset().b, testing_values.test_bool_value);
    EXPECT_EQ(struct_data.var_StructBitset().var_bitset().c, testing_values.test_ushort_value);
    EXPECT_EQ(struct_data.var_StructBitset().var_bitset().d, testing_values.test_short_value);

    EXPECT_EQ(map_struct_data->return_loaned_value(map_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(map_struct_data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructStructures)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("Structures");
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_short_struct_name);
    member_descriptor->type(create_short_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_ushort_struct_name);
    member_descriptor->type(create_unsigned_short_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_struct_name);
    member_descriptor->type(create_long_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_ulong_struct_name);
    member_descriptor->type(create_unsigned_long_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_long_struct_name);
    member_descriptor->type(create_long_long_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_ulong_long_struct_name);
    member_descriptor->type(create_unsigned_long_long_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_float_struct_name);
    member_descriptor->type(create_float_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_double_struct_name);
    member_descriptor->type(create_double_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_long_double_struct_name);
    member_descriptor->type(create_long_double_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bool_struct_name);
    member_descriptor->type(create_boolean_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_byte_struct_name);
    member_descriptor->type(create_octet_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_char_struct_name);
    member_descriptor->type(create_char_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_wchar_struct_name);
    member_descriptor->type(create_wchar_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_string_struct_name);
    member_descriptor->type(create_string_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_wstring_struct_name);
    member_descriptor->type(create_wstring_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bounded_string_struct_name);
    member_descriptor->type(create_bounded_string_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bounded_wstring_struct_name);
    member_descriptor->type(create_bounded_wstring_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_enum_struct_name);
    member_descriptor->type(create_enum_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bitmask_struct_name);
    member_descriptor->type(create_bitmask_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_alias_struct_name);
    member_descriptor->type(create_alias_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_array_struct_name);
    member_descriptor->type(create_short_array_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_seq_struct_name);
    member_descriptor->type(create_sequence_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_map_struct_name);
    member_descriptor->type(create_map_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_union_struct_name);
    member_descriptor->type(create_union_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_struct_struct_name);
    member_descriptor->type(create_structure_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bitset_struct_name);
    member_descriptor->type(create_bitset_struct());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_empty_struct_name);
    member_descriptor->type(create_empty_struct());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int16_t short_value = 2;
    uint16_t ushort_value = 15;
    int32_t long_value = 55;
    uint32_t ulong_value = 47;
    int64_t long_long_value = -125;
    uint64_t ulong_long_value = 1001;
    float float_value = 14.3f;
    double double_value = 502.12;
    long double long_double_value = 13.2;
    bool bool_value = true;
    eprosima::fastdds::rtps::octet byte_value = 234;
    char char_value = 'a';
    wchar_t wchar_value = L'0';
    std::string string_value = "TESTING_STRING";
    std::wstring wstring_value = L"TESTING_WSTRING";
    std::string bounded_string_value = "TESTING";
    std::wstring bounded_wstring_value = L"TESTING";
    InnerEnumHelper enum_value = InnerEnumHelper::ENUM_VALUE_3;
    InnerBitMaskHelper bitmask_value = InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1 |
            InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6;
    Int16Seq array_value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Int32Seq seq_value = {10, 20};
    int32_t map_value = 253;
    uint8_t uint8_value = 7;
    testing_values_struct testing_values;

    DynamicData::_ref_type short_struct_data = data->loan_value(data->get_member_id_by_name(var_short_struct_name));
    ASSERT_TRUE(short_struct_data);
    DynamicData::_ref_type ushort_struct_data = data->loan_value(data->get_member_id_by_name(var_ushort_struct_name));
    ASSERT_TRUE(ushort_struct_data);
    DynamicData::_ref_type long_struct_data = data->loan_value(data->get_member_id_by_name(var_long_struct_name));
    ASSERT_TRUE(long_struct_data);
    DynamicData::_ref_type ulong_struct_data = data->loan_value(data->get_member_id_by_name(var_ulong_struct_name));
    ASSERT_TRUE(ulong_struct_data);
    DynamicData::_ref_type long_long_struct_data =
            data->loan_value(data->get_member_id_by_name(var_long_long_struct_name));
    ASSERT_TRUE(long_long_struct_data);
    DynamicData::_ref_type ulong_long_struct_data =
            data->loan_value(data->get_member_id_by_name(var_ulong_long_struct_name));
    ASSERT_TRUE(ulong_long_struct_data);
    DynamicData::_ref_type float_struct_data = data->loan_value(data->get_member_id_by_name(var_float_struct_name));
    ASSERT_TRUE(float_struct_data);
    DynamicData::_ref_type double_struct_data = data->loan_value(data->get_member_id_by_name(var_double_struct_name));
    ASSERT_TRUE(double_struct_data);
    DynamicData::_ref_type long_double_struct_data =
            data->loan_value(data->get_member_id_by_name(var_long_double_struct_name));
    ASSERT_TRUE(long_double_struct_data);
    DynamicData::_ref_type bool_struct_data = data->loan_value(data->get_member_id_by_name(var_bool_struct_name));
    ASSERT_TRUE(bool_struct_data);
    DynamicData::_ref_type byte_struct_data = data->loan_value(data->get_member_id_by_name(var_byte_struct_name));
    ASSERT_TRUE(byte_struct_data);
    DynamicData::_ref_type char_struct_data = data->loan_value(data->get_member_id_by_name(var_char_struct_name));
    ASSERT_TRUE(char_struct_data);
    DynamicData::_ref_type wchar_struct_data = data->loan_value(data->get_member_id_by_name(var_wchar_struct_name));
    ASSERT_TRUE(wchar_struct_data);
    DynamicData::_ref_type string_struct_data = data->loan_value(data->get_member_id_by_name(var_string_struct_name));
    ASSERT_TRUE(string_struct_data);
    DynamicData::_ref_type wstring_struct_data = data->loan_value(data->get_member_id_by_name(var_wstring_struct_name));
    ASSERT_TRUE(wstring_struct_data);
    DynamicData::_ref_type bounded_string_struct_data =
            data->loan_value(data->get_member_id_by_name(var_bounded_string_struct_name));
    ASSERT_TRUE(bounded_string_struct_data);
    DynamicData::_ref_type bounded_wstring_struct_data =
            data->loan_value(data->get_member_id_by_name(var_bounded_wstring_struct_name));
    ASSERT_TRUE(bounded_wstring_struct_data);
    DynamicData::_ref_type enum_struct_data = data->loan_value(data->get_member_id_by_name(var_enum_struct_name));
    ASSERT_TRUE(enum_struct_data);
    DynamicData::_ref_type bitmask_struct_data = data->loan_value(data->get_member_id_by_name(var_bitmask_struct_name));
    ASSERT_TRUE(bitmask_struct_data);
    DynamicData::_ref_type alias_struct_data = data->loan_value(data->get_member_id_by_name(var_alias_struct_name));
    ASSERT_TRUE(alias_struct_data);
    DynamicData::_ref_type array_struct_data = data->loan_value(data->get_member_id_by_name(var_array_struct_name));
    ASSERT_TRUE(array_struct_data);
    DynamicData::_ref_type seq_struct_data = data->loan_value(data->get_member_id_by_name(var_seq_struct_name));
    ASSERT_TRUE(seq_struct_data);
    DynamicData::_ref_type map_struct_data = data->loan_value(data->get_member_id_by_name(var_map_struct_name));
    ASSERT_TRUE(map_struct_data);
    DynamicData::_ref_type map_data = map_struct_data->loan_value(map_struct_data->get_member_id_by_name(var_map_name));
    ASSERT_TRUE(map_data);
    DynamicData::_ref_type union_struct_data = data->loan_value(data->get_member_id_by_name(var_union_struct_name));
    ASSERT_TRUE(union_struct_data);
    DynamicData::_ref_type union_data =
            union_struct_data->loan_value(union_struct_data->get_member_id_by_name(var_union_name));
    ASSERT_TRUE(union_data);
    DynamicData::_ref_type struct_struct_data = data->loan_value(data->get_member_id_by_name(var_struct_struct_name));
    ASSERT_TRUE(struct_struct_data);
    DynamicData::_ref_type structure_data =
            struct_struct_data->loan_value(struct_struct_data->get_member_id_by_name(var_struct_name));
    ASSERT_TRUE(structure_data);
    DynamicData::_ref_type bitset_struct_data = data->loan_value(data->get_member_id_by_name(var_bitset_struct_name));
    ASSERT_TRUE(bitset_struct_data);
    DynamicData::_ref_type bitset_data =
            bitset_struct_data->loan_value(bitset_struct_data->get_member_id_by_name(var_bitset_name));
    ASSERT_TRUE(bitset_data);

    EXPECT_EQ(short_struct_data->set_int16_value(short_struct_data->get_member_id_by_name(
                var_short_name), short_value), RETCODE_OK);
    EXPECT_EQ(short_struct_data->get_int16_value(testing_values.test_short_value,
            short_struct_data->get_member_id_by_name(var_short_name)), RETCODE_OK);
    EXPECT_EQ(short_value, testing_values.test_short_value);
    EXPECT_EQ(ushort_struct_data->set_uint16_value(ushort_struct_data->get_member_id_by_name(var_ushort_name),
            ushort_value), RETCODE_OK);
    EXPECT_EQ(ushort_struct_data->get_uint16_value(testing_values.test_ushort_value,
            ushort_struct_data->get_member_id_by_name(var_ushort_name)), RETCODE_OK);
    EXPECT_EQ(ushort_value, testing_values.test_ushort_value);
    EXPECT_EQ(long_struct_data->set_int32_value(long_struct_data->get_member_id_by_name(
                var_long_name), long_value), RETCODE_OK);
    EXPECT_EQ(long_struct_data->get_int32_value(testing_values.test_long_value,
            long_struct_data->get_member_id_by_name(var_long_name)), RETCODE_OK);
    EXPECT_EQ(long_value, testing_values.test_long_value);
    EXPECT_EQ(ulong_struct_data->set_uint32_value(ulong_struct_data->get_member_id_by_name(
                var_ulong_name), ulong_value), RETCODE_OK);
    EXPECT_EQ(ulong_struct_data->get_uint32_value(testing_values.test_ulong_value,
            ulong_struct_data->get_member_id_by_name(var_ulong_name)), RETCODE_OK);
    EXPECT_EQ(ulong_value, testing_values.test_ulong_value);
    EXPECT_EQ(long_long_struct_data->set_int64_value(long_long_struct_data->get_member_id_by_name(var_long_long_name),
            long_long_value), RETCODE_OK);
    EXPECT_EQ(long_long_struct_data->get_int64_value(testing_values.test_long_long_value,
            long_long_struct_data->get_member_id_by_name(var_long_long_name)), RETCODE_OK);
    EXPECT_EQ(long_long_value, testing_values.test_long_long_value);
    EXPECT_EQ(ulong_long_struct_data->set_uint64_value(ulong_long_struct_data->get_member_id_by_name(var_ulong_long_name),
            ulong_long_value), RETCODE_OK);
    EXPECT_EQ(ulong_long_struct_data->get_uint64_value(testing_values.test_ulong_long_value,
            ulong_long_struct_data->get_member_id_by_name(var_ulong_long_name)), RETCODE_OK);
    EXPECT_EQ(ulong_long_value, testing_values.test_ulong_long_value);
    EXPECT_EQ(float_struct_data->set_float32_value(float_struct_data->get_member_id_by_name(
                var_float_name), float_value), RETCODE_OK);
    EXPECT_EQ(float_struct_data->get_float32_value(testing_values.test_float_value,
            float_struct_data->get_member_id_by_name(var_float_name)), RETCODE_OK);
    EXPECT_EQ(float_value, testing_values.test_float_value);
    EXPECT_EQ(double_struct_data->set_float64_value(double_struct_data->get_member_id_by_name(var_double_name),
            double_value), RETCODE_OK);
    EXPECT_EQ(double_struct_data->get_float64_value(testing_values.test_double_value,
            double_struct_data->get_member_id_by_name(var_double_name)), RETCODE_OK);
    EXPECT_EQ(double_value, testing_values.test_double_value);
    EXPECT_EQ(long_double_struct_data->set_float128_value(long_double_struct_data->get_member_id_by_name(
                var_long_double_name), long_double_value), RETCODE_OK);
    EXPECT_EQ(long_double_struct_data->get_float128_value(testing_values.test_long_double_value,
            long_double_struct_data->get_member_id_by_name(var_long_double_name)), RETCODE_OK);
    EXPECT_EQ(long_double_value, testing_values.test_long_double_value);
    EXPECT_EQ(bool_struct_data->set_boolean_value(bool_struct_data->get_member_id_by_name(
                var_bool_name), bool_value), RETCODE_OK);
    EXPECT_EQ(bool_struct_data->get_boolean_value(testing_values.test_bool_value,
            bool_struct_data->get_member_id_by_name(var_bool_name)), RETCODE_OK);
    EXPECT_EQ(bool_value, testing_values.test_bool_value);
    EXPECT_EQ(byte_struct_data->set_byte_value(byte_struct_data->get_member_id_by_name(
                var_byte_name), byte_value), RETCODE_OK);
    EXPECT_EQ(byte_struct_data->get_byte_value(testing_values.test_byte_value,
            byte_struct_data->get_member_id_by_name(var_byte_name)), RETCODE_OK);
    EXPECT_EQ(byte_value, testing_values.test_byte_value);
    EXPECT_EQ(char_struct_data->set_char8_value(char_struct_data->get_member_id_by_name(
                var_char_name), char_value), RETCODE_OK);
    EXPECT_EQ(char_struct_data->get_char8_value(testing_values.test_char_value,
            char_struct_data->get_member_id_by_name(var_char_name)), RETCODE_OK);
    EXPECT_EQ(char_value, testing_values.test_char_value);
    EXPECT_EQ(wchar_struct_data->set_char16_value(wchar_struct_data->get_member_id_by_name(
                var_wchar_name), wchar_value), RETCODE_OK);
    EXPECT_EQ(wchar_struct_data->get_char16_value(testing_values.test_wchar_value,
            wchar_struct_data->get_member_id_by_name(var_wchar_name)), RETCODE_OK);
    EXPECT_EQ(wchar_value, testing_values.test_wchar_value);
    EXPECT_EQ(string_struct_data->set_string_value(string_struct_data->get_member_id_by_name(var_string_name),
            string_value), RETCODE_OK);
    EXPECT_EQ(string_struct_data->get_string_value(testing_values.test_string_value,
            string_struct_data->get_member_id_by_name(var_string_name)), RETCODE_OK);
    EXPECT_EQ(string_value, testing_values.test_string_value);
    EXPECT_EQ(wstring_struct_data->set_wstring_value(wstring_struct_data->get_member_id_by_name(var_wstring_name),
            wstring_value), RETCODE_OK);
    EXPECT_EQ(wstring_struct_data->get_wstring_value(testing_values.test_wstring_value,
            wstring_struct_data->get_member_id_by_name(var_wstring_name)), RETCODE_OK);
    EXPECT_EQ(wstring_value, testing_values.test_wstring_value);
    EXPECT_EQ(bounded_string_struct_data->set_string_value(bounded_string_struct_data->get_member_id_by_name(
                var_bounded_string_name), bounded_string_value), RETCODE_OK);
    EXPECT_EQ(bounded_string_struct_data->get_string_value(testing_values.test_bounded_string_value,
            bounded_string_struct_data->get_member_id_by_name(var_bounded_string_name)), RETCODE_OK);
    EXPECT_EQ(bounded_string_value, testing_values.test_bounded_string_value);
    EXPECT_EQ(bounded_wstring_struct_data->set_wstring_value(bounded_wstring_struct_data->get_member_id_by_name(
                var_bounded_wstring_name), bounded_wstring_value), RETCODE_OK);
    EXPECT_EQ(bounded_wstring_struct_data->get_wstring_value(testing_values.test_bounded_wstring_value,
            bounded_wstring_struct_data->get_member_id_by_name(var_bounded_wstring_name)), RETCODE_OK);
    EXPECT_EQ(bounded_wstring_value, testing_values.test_bounded_wstring_value);
    EXPECT_EQ(enum_struct_data->set_int32_value(enum_struct_data->get_member_id_by_name(var_enum_name),
            static_cast<int32_t>(enum_value)), RETCODE_OK);
    EXPECT_EQ(enum_struct_data->get_int32_value(testing_values.test_enum_value,
            enum_struct_data->get_member_id_by_name(var_enum_name)), RETCODE_OK);
    EXPECT_EQ(static_cast<int32_t>(enum_value), testing_values.test_enum_value);
    EXPECT_EQ(bitmask_struct_data->set_uint32_value(bitmask_struct_data->get_member_id_by_name(var_bitmask_name),
            bitmask_value), RETCODE_OK);
    EXPECT_EQ(bitmask_struct_data->get_uint32_value(testing_values.test_bitmask_value,
            bitmask_struct_data->get_member_id_by_name(var_bitmask_name)), RETCODE_OK);
    EXPECT_EQ(bitmask_value, testing_values.test_bitmask_value);
    EXPECT_EQ(alias_struct_data->set_int32_value(alias_struct_data->get_member_id_by_name(
                var_alias_name), long_value), RETCODE_OK);
    testing_values.test_long_value = 0;
    EXPECT_EQ(alias_struct_data->get_int32_value(testing_values.test_long_value,
            alias_struct_data->get_member_id_by_name(var_alias_name)), RETCODE_OK);
    EXPECT_EQ(long_value, testing_values.test_long_value);
    EXPECT_EQ(array_struct_data->set_int16_values(array_struct_data->get_member_id_by_name(
                var_array_name), array_value), RETCODE_OK);
    EXPECT_EQ(array_struct_data->get_int16_values(testing_values.test_array_value,
            array_struct_data->get_member_id_by_name(var_array_name)), RETCODE_OK);
    EXPECT_EQ(array_value, testing_values.test_array_value);
    EXPECT_EQ(seq_struct_data->set_int32_values(seq_struct_data->get_member_id_by_name(
                var_seq_name), seq_value), RETCODE_OK);
    EXPECT_EQ(seq_struct_data->get_int32_values(testing_values.test_seq_value,
            seq_struct_data->get_member_id_by_name(var_seq_name)), RETCODE_OK);
    EXPECT_EQ(seq_value, testing_values.test_seq_value);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                first_map_key)), long_value), RETCODE_OK);
    EXPECT_EQ(map_data->set_int32_value(map_data->get_member_id_by_name(std::to_string(
                second_map_key)), map_value), RETCODE_OK);
    EXPECT_EQ(map_data->get_int32_value(testing_values.test_long_value,
            map_data->get_member_id_by_name(std::to_string(first_map_key))), RETCODE_OK);
    EXPECT_EQ(long_value, testing_values.test_long_value);
    EXPECT_EQ(map_data->get_int32_value(testing_values.test_map_value,
            map_data->get_member_id_by_name(std::to_string(second_map_key))), RETCODE_OK);
    EXPECT_EQ(map_value, testing_values.test_map_value);
    testing_values.test_float_value = 0.0;
    testing_values.test_short_value = 0;
    EXPECT_EQ(union_data->set_float32_value(union_data->get_member_id_by_name(
                union_float_member_name), float_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_float32_value(testing_values.test_float_value,
            union_data->get_member_id_by_name(union_float_member_name)), RETCODE_OK);
    EXPECT_EQ(float_value, testing_values.test_float_value);
    EXPECT_EQ(union_data->set_int16_value(union_data->get_member_id_by_name(
                union_short_member_name), short_value), RETCODE_OK);
    EXPECT_EQ(union_data->get_int16_value(testing_values.test_short_value,
            union_data->get_member_id_by_name(union_short_member_name)), RETCODE_OK);
    EXPECT_EQ(short_value, testing_values.test_short_value);
    testing_values.test_float_value = 0.0;
    testing_values.test_long_value = 0;
    EXPECT_EQ(structure_data->set_float32_value(structure_data->get_member_id_by_name(struct_float_member_name),
            float_value), RETCODE_OK);
    EXPECT_EQ(structure_data->set_int32_value(structure_data->get_member_id_by_name(
                struct_long_member_name), long_value), RETCODE_OK);
    EXPECT_EQ(structure_data->get_float32_value(testing_values.test_float_value,
            structure_data->get_member_id_by_name(struct_float_member_name)), RETCODE_OK);
    EXPECT_EQ(structure_data->get_int32_value(testing_values.test_long_value,
            structure_data->get_member_id_by_name(struct_long_member_name)), RETCODE_OK);
    EXPECT_EQ(testing_values.test_float_value, float_value);
    EXPECT_EQ(testing_values.test_long_value, long_value);
    testing_values.test_bool_value = false;
    testing_values.test_ushort_value = 0;
    testing_values.test_short_value = 0;
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), uint8_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(bitfield_b), bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(bitfield_c), ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(bitfield_d), short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint8_value(testing_values.test_uint8_value,
            bitset_data->get_member_id_by_name(bitfield_a)), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_boolean_value(testing_values.test_bool_value,
            bitset_data->get_member_id_by_name(bitfield_b)), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_uint16_value(testing_values.test_ushort_value,
            bitset_data->get_member_id_by_name(bitfield_c)), RETCODE_OK);
    EXPECT_EQ(bitset_data->get_int16_value(testing_values.test_short_value,
            bitset_data->get_member_id_by_name(bitfield_d)), RETCODE_OK);
    EXPECT_EQ(uint8_value, testing_values.test_uint8_value);
    EXPECT_EQ(bool_value, testing_values.test_bool_value);
    EXPECT_EQ(ushort_value, testing_values.test_ushort_value);
    EXPECT_EQ(short_value, testing_values.test_short_value);

    EXPECT_EQ(data->return_loaned_value(short_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(ushort_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(long_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(ulong_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(long_long_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(ulong_long_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(float_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(double_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(long_double_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bool_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(byte_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(char_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(wchar_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(string_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(wstring_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bounded_string_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bounded_wstring_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(enum_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bitmask_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(alias_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(array_struct_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(seq_struct_data), RETCODE_OK);
    EXPECT_EQ(map_struct_data->return_loaned_value(map_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(map_struct_data), RETCODE_OK);
    EXPECT_EQ(union_struct_data->return_loaned_value(union_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(union_struct_data), RETCODE_OK);
    EXPECT_EQ(struct_struct_data->return_loaned_value(structure_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(struct_struct_data), RETCODE_OK);
    EXPECT_EQ(bitset_struct_data->return_loaned_value(bitset_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bitset_struct_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        Structures struct_data;
        TypeSupport static_pubsubType {new StructuresPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        check_structure_static_data(data, struct_data, testing_values);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_Structures_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

/**
 * This tests checks the following test case which has also been translated into IDL.
 *
 * struct root1
 * {
 *     struct foo
 *     {
 *         long a;
 *         long b;
 *     } foo_struct;
 *     float c;
 * };
 *
 * struct root2
 * {
 *     struct foo
 *     {
 *         bool d;
 *     } foo_struct;
 *     struct bar
 *     {
 *         double e;
 *     } bar_struct;
 * };
 *
 * struct root
 * {
 *     root1 var_root1;
 *     root2 var_root2;
 * };
 */
TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_root)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(root_struct_name);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(root1_struct_name);
    DynamicTypeBuilder::_ref_type root1_struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                            type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(root2_struct_name);
    DynamicTypeBuilder::_ref_type root2_struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                            type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(foo_struct_name);
    DynamicTypeBuilder::_ref_type foo1_struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                           type_descriptor)};
    DynamicTypeBuilder::_ref_type foo2_struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                           type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bar_struct_name);
    DynamicTypeBuilder::_ref_type bar_struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                          type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_a_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    foo1_struct_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_b_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    foo1_struct_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_foo_struct_name);
    member_descriptor->type(foo1_struct_builder->build());
    root1_struct_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_c_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    root1_struct_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_d_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    foo2_struct_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_e_name);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT64));
    bar_struct_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_foo_struct_name);
    member_descriptor->type(foo2_struct_builder->build());
    root2_struct_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_bar_struct_name);
    member_descriptor->type(bar_struct_builder->build());
    root2_struct_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_root1_struct_name);
    member_descriptor->type(root1_struct_builder->build());
    struct_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_root2_struct_name);
    member_descriptor->type(root2_struct_builder->build());
    struct_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {struct_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t a_value = 1001;
    int32_t b_value = 10001;
    float c_value = 50.5;
    bool d_value = true;
    double e_value = 123.123;
    int32_t test_long_value = 0;
    float test_float_value = 0;
    bool test_bool_value = 0;
    double test_double_value = 0;

    DynamicData::_ref_type root1_data = data->loan_value(data->get_member_id_by_name(var_root1_struct_name));
    ASSERT_TRUE(root1_data);
    DynamicData::_ref_type root2_data = data->loan_value(data->get_member_id_by_name(var_root2_struct_name));
    ASSERT_TRUE(root2_data);
    DynamicData::_ref_type foo1_data = root1_data->loan_value(root1_data->get_member_id_by_name(var_foo_struct_name));
    ASSERT_TRUE(foo1_data);
    DynamicData::_ref_type foo2_data = root2_data->loan_value(root2_data->get_member_id_by_name(var_foo_struct_name));
    ASSERT_TRUE(foo2_data);
    DynamicData::_ref_type bar_data = root2_data->loan_value(root2_data->get_member_id_by_name(var_bar_struct_name));
    ASSERT_TRUE(bar_data);

    EXPECT_EQ(foo1_data->set_int32_value(foo1_data->get_member_id_by_name(var_a_name), a_value), RETCODE_OK);
    EXPECT_EQ(foo1_data->get_int32_value(test_long_value, foo1_data->get_member_id_by_name(var_a_name)), RETCODE_OK);
    EXPECT_EQ(a_value, test_long_value);
    EXPECT_EQ(foo1_data->set_int32_value(foo1_data->get_member_id_by_name(var_b_name), b_value), RETCODE_OK);
    EXPECT_EQ(foo1_data->get_int32_value(test_long_value, foo1_data->get_member_id_by_name(var_b_name)), RETCODE_OK);
    EXPECT_EQ(b_value, test_long_value);
    EXPECT_EQ(root1_data->set_float32_value(root1_data->get_member_id_by_name(var_c_name), c_value), RETCODE_OK);
    EXPECT_EQ(root1_data->get_float32_value(test_float_value, root1_data->get_member_id_by_name(var_c_name)),
            RETCODE_OK);
    EXPECT_EQ(c_value, test_float_value);
    EXPECT_EQ(foo2_data->set_boolean_value(foo2_data->get_member_id_by_name(var_d_name), d_value), RETCODE_OK);
    EXPECT_EQ(foo2_data->get_boolean_value(test_bool_value, foo2_data->get_member_id_by_name(var_d_name)), RETCODE_OK);
    EXPECT_EQ(d_value, test_bool_value);
    EXPECT_EQ(bar_data->set_float64_value(bar_data->get_member_id_by_name(var_e_name), e_value), RETCODE_OK);
    EXPECT_EQ(bar_data->get_float64_value(test_double_value, bar_data->get_member_id_by_name(var_e_name)), RETCODE_OK);
    EXPECT_EQ(e_value, test_double_value);

    EXPECT_EQ(root1_data->return_loaned_value(foo1_data), RETCODE_OK);
    EXPECT_EQ(root2_data->return_loaned_value(foo2_data), RETCODE_OK);
    EXPECT_EQ(root2_data->return_loaned_value(bar_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(root1_data), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(root2_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        root struct_data;
        TypeSupport static_pubsubType {new rootPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data, static_pubsubType);
        EXPECT_EQ(struct_data.var_root1().foo_struct().a(), a_value);
        EXPECT_EQ(struct_data.var_root1().foo_struct().b(), b_value);
        EXPECT_EQ(struct_data.var_root1().c(), c_value);
        EXPECT_EQ(struct_data.var_root2().foo_struct().d(), d_value);
        EXPECT_EQ(struct_data.var_root2().bar_struct().e(), e_value);
    }

    // DynamicType defined for this test is not the same as the one defined in the IDL file as the modules are not
    // defined for the DynamicType. Specifically, this test checks that using Dynamic Language Binding, the unique
    // name rule is not applied.

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
