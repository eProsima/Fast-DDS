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
#include "../../../dds-types-test/helpers/basic_inner_typesPubSubTypes.hpp"
#include "../../../dds-types-test/bitsetsPubSubTypes.hpp"
#include "../../../dds-types-test/bitsetsTypeObjectSupport.hpp"
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* innertypedbitsethelper_name = "InnerTypedBitsetHelper";
constexpr const char* boolean_bitfield_name = "boolean_bitfield";
constexpr const char* byte_bitfield_name = "byte_bitfield";
constexpr const char* int8_bitfield_name = "int8_bitfield";
constexpr const char* uint8_bitfield_name = "uint8_bitfield";
constexpr const char* short_bitfield_name = "short_bitfield";
constexpr const char* ushort_bitfield_name = "ushort_bitfield";

constexpr const char* innertypedbitsethelper2_name = "InnerTypedBitsetHelper2";
constexpr const char* long_bitfield_name = "long_bitfield";
constexpr const char* ulong_bitfield_name = "ulong_bitfield";

constexpr const char* innertypedbitsethelper3_name = "InnerTypedBitsetHelper3";
constexpr const char* long_long_bitfield_name = "long_long_bitfield";

constexpr const char* innertypedbitsethelper4_name = "InnerTypedBitsetHelper4";
constexpr const char* ulong_long_bitfield_name = "ulong_long_bitfield";

constexpr const char* struct_bitsetstructure_name = "BitsetStruct";

constexpr const char* var_innerbitsethelper_name = "var_InnerBitsetHelper";
constexpr const char* var_innertypedbitsethelper_name = "var_InnerTypedBitsetHelper";
constexpr const char* var_innertypedbitsethelper2_name = "var_InnerTypedBitsetHelper2";
constexpr const char* var_innertypedbitsethelper3_name = "var_InnerTypedBitsetHelper3";
constexpr const char* var_innertypedbitsethelper4_name = "var_InnerTypedBitsetHelper4";

DynamicType::_ref_type create_typed_inner_bitset_helper()
{
    TypeDescriptor::_ref_type bitset_descriptor {traits<TypeDescriptor>::make_shared()};
    bitset_descriptor->kind(TK_BITSET);
    bitset_descriptor->name(innertypedbitsethelper_name);
    bitset_descriptor->bound({1, 8, 8, 8, 16, 16});
    DynamicTypeBuilder::_ref_type bitset_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      bitset_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(boolean_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitset_member->id(0);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(byte_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BYTE));
    bitset_member->id(1);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(int8_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT8));
    bitset_member->id(9);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(uint8_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT8));
    bitset_member->id(17);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(short_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    bitset_member->id(25);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(ushort_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    bitset_member->id(41);
    bitset_builder->add_member(bitset_member);

    return bitset_builder->build();
}

DynamicType::_ref_type create_typed_inner_bitset_helper2()
{
    TypeDescriptor::_ref_type bitset_descriptor {traits<TypeDescriptor>::make_shared()};
    bitset_descriptor->kind(TK_BITSET);
    bitset_descriptor->name(innertypedbitsethelper2_name);
    bitset_descriptor->bound({32, 32});
    DynamicTypeBuilder::_ref_type bitset_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      bitset_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(long_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    bitset_member->id(0);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(ulong_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    bitset_member->id(32);
    bitset_builder->add_member(bitset_member);

    return bitset_builder->build();
}

DynamicType::_ref_type create_typed_inner_bitset_helper3()
{
    TypeDescriptor::_ref_type bitset_descriptor {traits<TypeDescriptor>::make_shared()};
    bitset_descriptor->kind(TK_BITSET);
    bitset_descriptor->name(innertypedbitsethelper3_name);
    bitset_descriptor->bound({64});
    DynamicTypeBuilder::_ref_type bitset_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      bitset_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(long_long_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    bitset_member->id(0);
    bitset_builder->add_member(bitset_member);

    return bitset_builder->build();
}

DynamicType::_ref_type create_typed_inner_bitset_helper4()
{
    TypeDescriptor::_ref_type bitset_descriptor {traits<TypeDescriptor>::make_shared()};
    bitset_descriptor->kind(TK_BITSET);
    bitset_descriptor->name(innertypedbitsethelper4_name);
    bitset_descriptor->bound({64});
    DynamicTypeBuilder::_ref_type bitset_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      bitset_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(ulong_long_bitfield_name);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    bitset_member->id(0);
    bitset_builder->add_member(bitset_member);

    return bitset_builder->build();
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BitsetStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bitsetstructure_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_innerbitsethelper_name);
    member_descriptor->type(create_inner_bitset_helper());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_innertypedbitsethelper_name);
    member_descriptor->type(create_typed_inner_bitset_helper());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_innertypedbitsethelper2_name);
    member_descriptor->type(create_typed_inner_bitset_helper2());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_innertypedbitsethelper3_name);
    member_descriptor->type(create_typed_inner_bitset_helper3());
    type_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_innertypedbitsethelper4_name);
    member_descriptor->type(create_typed_inner_bitset_helper4());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    bool bool_value = true;
    eprosima::fastdds::rtps::octet octet_value = 7;
    int8_t int8_value = 7;
    uint8_t uint8_value = 7;
    int16_t short_value = 2;
    uint16_t ushort_value = 15;
    int32_t long_value = 55;
    uint32_t ulong_value = 47;
    int64_t long_long_value = -125;
    uint64_t ulong_long_value = 1001;

    bool test_bool_value = false;
    eprosima::fastdds::rtps::octet test_octet_value = 0;
    int8_t test_int8_value = 0;
    uint8_t test_uint8_value = 0;
    int16_t test_short_value = 0;
    uint16_t test_ushort_value = 0;
    int32_t test_long_value = 0;
    uint32_t test_ulong_value = 0;
    int64_t test_long_long_value = 0;
    uint64_t test_ulong_long_value = 0;

    DynamicData::_ref_type var_innerbitsethelper_data =
            data->loan_value(data->get_member_id_by_name(var_innerbitsethelper_name));
    ASSERT_TRUE(var_innerbitsethelper_data);
    EXPECT_EQ(var_innerbitsethelper_data->set_uint8_value(
                var_innerbitsethelper_data->get_member_id_by_name(bitfield_a), uint8_value), RETCODE_OK);
    EXPECT_EQ(var_innerbitsethelper_data->get_uint8_value(
                test_uint8_value, var_innerbitsethelper_data->get_member_id_by_name(bitfield_a)), RETCODE_OK);
    EXPECT_EQ(uint8_value, uint8_value);
    EXPECT_EQ(var_innerbitsethelper_data->set_boolean_value(
                var_innerbitsethelper_data->get_member_id_by_name(bitfield_b), bool_value), RETCODE_OK);
    EXPECT_EQ(var_innerbitsethelper_data->get_boolean_value(
                test_bool_value, var_innerbitsethelper_data->get_member_id_by_name(bitfield_b)), RETCODE_OK);
    EXPECT_EQ(bool_value, test_bool_value);
    EXPECT_EQ(var_innerbitsethelper_data->set_uint16_value(
                var_innerbitsethelper_data->get_member_id_by_name(bitfield_c), ushort_value), RETCODE_OK);
    EXPECT_EQ(var_innerbitsethelper_data->get_uint16_value(
                test_ushort_value, var_innerbitsethelper_data->get_member_id_by_name(bitfield_c)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(var_innerbitsethelper_data->set_int16_value(
                var_innerbitsethelper_data->get_member_id_by_name(bitfield_d), short_value), RETCODE_OK);
    EXPECT_EQ(var_innerbitsethelper_data->get_int16_value(
                test_short_value, var_innerbitsethelper_data->get_member_id_by_name(bitfield_d)), RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(data->return_loaned_value(var_innerbitsethelper_data), RETCODE_OK);

    DynamicData::_ref_type var_innertypedbitsethelper_data =
            data->loan_value(data->get_member_id_by_name(var_innertypedbitsethelper_name));
    ASSERT_TRUE(var_innertypedbitsethelper_data);
    EXPECT_EQ(var_innertypedbitsethelper_data->set_boolean_value(var_innertypedbitsethelper_data->get_member_id_by_name(
                boolean_bitfield_name), bool_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper_data->get_boolean_value(test_bool_value,
            var_innertypedbitsethelper_data->get_member_id_by_name(boolean_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(bool_value, test_bool_value);
    EXPECT_EQ(var_innertypedbitsethelper_data->set_byte_value(var_innertypedbitsethelper_data->get_member_id_by_name(
                byte_bitfield_name), octet_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper_data->get_byte_value(test_octet_value,
            var_innertypedbitsethelper_data->get_member_id_by_name(byte_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(octet_value, test_octet_value);
    EXPECT_EQ(var_innertypedbitsethelper_data->set_int8_value(var_innertypedbitsethelper_data->get_member_id_by_name(
                int8_bitfield_name), int8_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper_data->get_int8_value(test_int8_value,
            var_innertypedbitsethelper_data->get_member_id_by_name(int8_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(int8_value, test_int8_value);
    EXPECT_EQ(var_innertypedbitsethelper_data->set_uint8_value(var_innertypedbitsethelper_data->get_member_id_by_name(
                uint8_bitfield_name), uint8_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper_data->get_uint8_value(test_uint8_value,
            var_innertypedbitsethelper_data->get_member_id_by_name(uint8_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(uint8_value, test_uint8_value);
    EXPECT_EQ(var_innertypedbitsethelper_data->set_int16_value(var_innertypedbitsethelper_data->get_member_id_by_name(
                short_bitfield_name), short_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper_data->get_int16_value(test_short_value,
            var_innertypedbitsethelper_data->get_member_id_by_name(short_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(var_innertypedbitsethelper_data->set_uint16_value(var_innertypedbitsethelper_data->get_member_id_by_name(
                ushort_bitfield_name), ushort_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper_data->get_uint16_value(test_ushort_value,
            var_innertypedbitsethelper_data->get_member_id_by_name(ushort_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(data->return_loaned_value(var_innertypedbitsethelper_data), RETCODE_OK);

    DynamicData::_ref_type var_innertypedbitsethelper2_data =
            data->loan_value(data->get_member_id_by_name(var_innertypedbitsethelper2_name));
    ASSERT_TRUE(var_innertypedbitsethelper2_data);
    EXPECT_EQ(var_innertypedbitsethelper2_data->set_int32_value(var_innertypedbitsethelper2_data->get_member_id_by_name(
                long_bitfield_name), long_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper2_data->get_int32_value(test_long_value,
            var_innertypedbitsethelper2_data->get_member_id_by_name(long_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(var_innertypedbitsethelper2_data->set_uint32_value(var_innertypedbitsethelper2_data->get_member_id_by_name(
                ulong_bitfield_name), ulong_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper2_data->get_uint32_value(test_ulong_value,
            var_innertypedbitsethelper2_data->get_member_id_by_name(ulong_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(ulong_value, test_ulong_value);
    EXPECT_EQ(data->return_loaned_value(var_innertypedbitsethelper2_data), RETCODE_OK);

    DynamicData::_ref_type var_innertypedbitsethelper3_data =
            data->loan_value(data->get_member_id_by_name(var_innertypedbitsethelper3_name));
    ASSERT_TRUE(var_innertypedbitsethelper3_data);
    EXPECT_EQ(var_innertypedbitsethelper3_data->set_int64_value(var_innertypedbitsethelper3_data->get_member_id_by_name(
                long_long_bitfield_name), long_long_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper3_data->get_int64_value(test_long_long_value,
            var_innertypedbitsethelper3_data->get_member_id_by_name(long_long_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(long_long_value, test_long_long_value);
    EXPECT_EQ(data->return_loaned_value(var_innertypedbitsethelper3_data), RETCODE_OK);

    DynamicData::_ref_type var_innertypedbitsethelper4_data =
            data->loan_value(data->get_member_id_by_name(var_innertypedbitsethelper4_name));
    ASSERT_TRUE(var_innertypedbitsethelper4_data);
    EXPECT_EQ(var_innertypedbitsethelper4_data->set_uint64_value(var_innertypedbitsethelper4_data->get_member_id_by_name(
                ulong_long_bitfield_name), ulong_long_value), RETCODE_OK);
    EXPECT_EQ(var_innertypedbitsethelper4_data->get_uint64_value(test_ulong_long_value,
            var_innertypedbitsethelper4_data->get_member_id_by_name(ulong_long_bitfield_name)), RETCODE_OK);
    EXPECT_EQ(ulong_long_value, test_ulong_long_value);
    EXPECT_EQ(data->return_loaned_value(var_innertypedbitsethelper4_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        BitsetStruct data_struct;
        TypeSupport static_pubsubType {new BitsetStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, data_struct,
                static_pubsubType);
        EXPECT_EQ(data_struct.var_InnerBitsetHelper().a, test_uint8_value);
        EXPECT_EQ(data_struct.var_InnerBitsetHelper().b, test_bool_value);
        EXPECT_EQ(data_struct.var_InnerBitsetHelper().c, test_ushort_value);
        EXPECT_EQ(data_struct.var_InnerBitsetHelper().d, test_short_value);

        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper().boolean_bitfield, test_bool_value);
        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper().byte_bitfield, test_octet_value);
        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper().int8_bitfield, test_int8_value);
        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper().uint8_bitfield, test_uint8_value);
        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper().short_bitfield, test_short_value);
        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper().ushort_bitfield, test_ushort_value);

        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper2().long_bitfield, test_long_value);
        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper2().ulong_bitfield, test_ulong_value);

        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper3().long_long_bitfield, test_long_long_value);

        EXPECT_EQ(data_struct.var_InnerTypedBitsetHelper4().ulong_long_bitfield, test_ulong_long_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BitsetStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // dds
} // fastdds
} // eprosima
