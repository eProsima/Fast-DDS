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
#include "../../../dds-types-test/inheritancePubSubTypes.hpp"
#include "../../../dds-types-test/inheritanceTypeObjectSupport.hpp"
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

const char* const inner_structure_helper_child_struct_name = "InnerStructureHelperChild";
const char* const inner_structure_helper_child_child_struct_name = "InnerStructureHelperChildChild";
const char* const inner_structure_helper_empty_child_struct_name = "InnerStructureHelperEmptyChild";
const char* const inner_structure_helper_empty_child_child_struct_name = "InnerStructureHelperEmptyChildChild";
const char* const inner_empty_structure_helper_child_struct_name = "InnerEmptyStructureHelperChild";
const char* const struct_alias_inheritance_struct_struct_name = "StructAliasInheritanceStruct";
const char* const structures_inheritance_struct_struct_name = "StructuresInheritanceStruct";
const char* const inner_bitset_helper_child_bitset_name = "InnerBitsetHelperChild";
const char* const inner_bitset_helper_child_child_bitset_name = "InnerBitsetHelperChildChild";
const char* const bitset_alias_inheritance_bitset_bitset_name = "BitsetAliasInheritanceBitset";
const char* const bitsets_child_inheritance_struct_struct_name = "BitsetsChildInheritanceStruct";

const char* const var_child_longlong = "var_child_longlong";
const char* const var_child_ulonglong = "var_child_ulonglong";
const char* const var_child_childlonglong2 = "var_child_childlonglong2";
const char* const var_childchild_ulonglong2 = "var_childchild_ulonglong2";
const char* const var_char = "var_char";
const char* const new_member = "new_member";
const char* const var_InnerStructureHelperChild = "var_InnerStructureHelperChild";
const char* const var_InnerStructureHelperChildChild = "var_InnerStructureHelperChildChild";
const char* const var_InnerStructureHelperEmptyChild = "var_InnerStructureHelperEmptyChild";
const char* const var_InnerStructureHelperEmptyChildChild = "var_InnerStructureHelperEmptyChildChild";
const char* const var_InnerEmptyStructureHelperChild = "var_InnerEmptyStructureHelperChild";
const char* const var_StructAliasInheritanceStruct = "var_StructAliasInheritanceStruct";
const char* const child_w = "child_w";
const char* const childchild_z = "childchild_z";
const char* const new_bitfield = "new_bitfield";
const char* const var_InnerBitsetHelperChild = "var_InnerBitsetHelperChild";
const char* const var_InnerBitsetHelperChildChild = "var_InnerBitsetHelperChildChild";
const char* const var_BitsetAliasInheritanceBitset = "var_BitsetAliasInheritanceBitset";

namespace eprosima {
namespace fastdds {
namespace dds {

DynamicType::_ref_type create_inner_struct_helper_child()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inner_structure_helper_child_struct_name);
    type_descriptor->base_type(DynamicTypesDDSTypesTest::create_inner_struct_helper());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_child_longlong);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_child_ulonglong);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);


    return type_builder->build();
}

DynamicType::_ref_type create_inner_struct_helper_child_child()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inner_structure_helper_child_child_struct_name);
    type_descriptor->base_type(create_inner_struct_helper_child());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_child_childlonglong2);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_childchild_ulonglong2);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);


    return type_builder->build();
}

DynamicType::_ref_type create_inner_struct_helper_empty_child()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inner_structure_helper_empty_child_struct_name);
    type_descriptor->base_type(DynamicTypesDDSTypesTest::create_inner_struct_helper());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    return type_builder->build();
}

DynamicType::_ref_type create_inner_struct_helper_empty_child_child()
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inner_structure_helper_empty_child_child_struct_name);
    type_descriptor->base_type(create_inner_struct_helper_empty_child());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_char);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_inner_empty_struct_helper_child()
{

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(inner_empty_structure_helper_child_struct_name);
    type_descriptor->base_type(DynamicTypesDDSTypesTest::create_inner_empty_struct_helper());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_child_longlong);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT64));
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_child_ulonglong);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_struct_alias_inheritance_struct()
{

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_alias_inheritance_struct_struct_name);
    type_descriptor->base_type(DynamicTypesDDSTypesTest::create_inner_struct_helper_alias());
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(new_member);
    member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    type_builder->add_member(member_descriptor);

    return type_builder->build();
}

DynamicType::_ref_type create_inner_bitset_helper_child()
{

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITSET);
    type_descriptor->name(inner_bitset_helper_child_bitset_name);
    type_descriptor->base_type(DynamicTypesDDSTypesTest::create_inner_bitset_helper());
    type_descriptor->bound({17});
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(child_w);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
    bitset_member->id(33);
    type_builder->add_member(bitset_member);

    return type_builder->build();
}

DynamicType::_ref_type create_inner_bitset_helper_child_child()
{

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITSET);
    type_descriptor->name(inner_bitset_helper_child_child_bitset_name);
    type_descriptor->base_type(create_inner_bitset_helper_child());
    type_descriptor->bound({14});
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(childchild_z);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    bitset_member->id(50);
    type_builder->add_member(bitset_member);

    return type_builder->build();
}

DynamicType::_ref_type create_bitset_alias_inheritance_bitset()
{

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITSET);
    type_descriptor->name(bitset_alias_inheritance_bitset_bitset_name);
    type_descriptor->base_type(DynamicTypesDDSTypesTest::create_inner_bitset_helper_alias());
    type_descriptor->bound({10});
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(new_bitfield);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    bitset_member->id(33);
    type_builder->add_member(bitset_member);

    return type_builder->build();
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_InnerStructureHelperChild)
{
    DynamicType::_ref_type struct_type {create_inner_struct_helper_child()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);


    InnerStructureHelperChild value;
    value.field1(-1000);
    value.field2(3.34f);
    value.var_child_longlong(-3000003);
    value.var_child_ulonglong(3000003);
    InnerStructureHelperChild test_value;

    // Set values.
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name(struct_long_member_name), value.field1()));
    EXPECT_EQ(RETCODE_OK,
            data->set_float32_value(data->get_member_id_by_name(struct_float_member_name), value.field2()));
    EXPECT_EQ(RETCODE_OK,
            data->set_int64_value(data->get_member_id_by_name(var_child_longlong),
            value.var_child_longlong()));
    EXPECT_EQ(RETCODE_OK,
            data->set_uint64_value(data->get_member_id_by_name(var_child_ulonglong),
            value.var_child_ulonglong()));

    // Check values.
    EXPECT_EQ(RETCODE_OK,
            data->get_int32_value(test_value.field1(),
            data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.field1(), test_value.field1());
    EXPECT_EQ(RETCODE_OK,
            data->get_float32_value(test_value.field2(),
            data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.field2(), test_value.field2());
    EXPECT_EQ(RETCODE_OK,
            data->get_int64_value(test_value.var_child_longlong(),
            data->get_member_id_by_name(var_child_longlong)));
    EXPECT_EQ(value.var_child_longlong(), test_value.var_child_longlong());
    EXPECT_EQ(RETCODE_OK,
            data->get_uint64_value(test_value.var_child_ulonglong(), data->get_member_id_by_name(var_child_ulonglong)));
    EXPECT_EQ(value.var_child_ulonglong(), test_value.var_child_ulonglong());

    for (auto encoding : encodings)
    {
        InnerStructureHelperChild struct_data;
        TypeSupport static_pubsubType {new InnerStructureHelperChildPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_InnerStructureHelperChild_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_InnerStructureHelperChildChild)
{
    DynamicType::_ref_type struct_type {create_inner_struct_helper_child_child()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);


    InnerStructureHelperChildChild value;
    value.field1(-1000);
    value.field2(3.34f);
    value.var_child_longlong(-3000003);
    value.var_child_ulonglong(3000003);
    value.var_child_childlonglong2(-4005003);
    value.var_childchild_ulonglong2(3002003);
    InnerStructureHelperChildChild test_value;

    // Set values.
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name(struct_long_member_name), value.field1()));
    EXPECT_EQ(RETCODE_OK,
            data->set_float32_value(data->get_member_id_by_name(struct_float_member_name), value.field2()));
    EXPECT_EQ(RETCODE_OK,
            data->set_int64_value(data->get_member_id_by_name(var_child_longlong),
            value.var_child_longlong()));
    EXPECT_EQ(RETCODE_OK,
            data->set_uint64_value(data->get_member_id_by_name(var_child_ulonglong),
            value.var_child_ulonglong()));
    EXPECT_EQ(RETCODE_OK,
            data->set_int64_value(data->get_member_id_by_name(var_child_childlonglong2),
            value.var_child_childlonglong2()));
    EXPECT_EQ(RETCODE_OK,
            data->set_uint64_value(data->get_member_id_by_name(
                var_childchild_ulonglong2), value.var_childchild_ulonglong2()));

    // Check values.
    EXPECT_EQ(RETCODE_OK,
            data->get_int32_value(test_value.field1(),
            data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.field1(), test_value.field1());
    EXPECT_EQ(RETCODE_OK,
            data->get_float32_value(test_value.field2(),
            data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.field2(), test_value.field2());
    EXPECT_EQ(RETCODE_OK,
            data->get_int64_value(test_value.var_child_longlong(),
            data->get_member_id_by_name(var_child_longlong)));
    EXPECT_EQ(value.var_child_longlong(), test_value.var_child_longlong());
    EXPECT_EQ(RETCODE_OK,
            data->get_uint64_value(test_value.var_child_ulonglong(), data->get_member_id_by_name(var_child_ulonglong)));
    EXPECT_EQ(value.var_child_ulonglong(), test_value.var_child_ulonglong());
    EXPECT_EQ(RETCODE_OK,
            data->get_int64_value(test_value.var_child_childlonglong2(),
            data->get_member_id_by_name(var_child_childlonglong2)));
    EXPECT_EQ(value.var_child_childlonglong2(), test_value.var_child_childlonglong2());
    EXPECT_EQ(RETCODE_OK,
            data->get_uint64_value(test_value.var_childchild_ulonglong2(),
            data->get_member_id_by_name(var_childchild_ulonglong2)));
    EXPECT_EQ(value.var_childchild_ulonglong2(), test_value.var_childchild_ulonglong2());

    for (auto encoding : encodings)
    {
        InnerStructureHelperChildChild struct_data;
        TypeSupport static_pubsubType {new InnerStructureHelperChildChildPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_InnerStructureHelperChildChild_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_InnerStructureHelperEmptyChild)
{
    DynamicType::_ref_type struct_type {create_inner_struct_helper_empty_child()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);


    InnerStructureHelperEmptyChild value;
    value.field1(-1000);
    value.field2(3.34f);
    InnerStructureHelperEmptyChild test_value;

    // Set values.
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name(struct_long_member_name), value.field1()));
    EXPECT_EQ(RETCODE_OK,
            data->set_float32_value(data->get_member_id_by_name(struct_float_member_name), value.field2()));

    // Check values.
    EXPECT_EQ(RETCODE_OK,
            data->get_int32_value(test_value.field1(),
            data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.field1(), test_value.field1());
    EXPECT_EQ(RETCODE_OK,
            data->get_float32_value(test_value.field2(),
            data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.field2(), test_value.field2());

    for (auto encoding : encodings)
    {
        InnerStructureHelperEmptyChild struct_data;
        TypeSupport static_pubsubType {new InnerStructureHelperEmptyChildPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_InnerStructureHelperEmptyChild_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_InnerStructureHelperEmptyChildChild)
{
    DynamicType::_ref_type struct_type {create_inner_struct_helper_empty_child_child()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);


    InnerStructureHelperEmptyChildChild value;
    value.field1(-300);
    value.field2(1.0);
    value.var_char('z');
    InnerStructureHelperEmptyChildChild test_value;

    // Set values.
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name(struct_long_member_name), value.field1()));
    EXPECT_EQ(RETCODE_OK,
            data->set_float32_value(data->get_member_id_by_name(struct_float_member_name), value.field2()));
    EXPECT_EQ(RETCODE_OK, data->set_char8_value(data->get_member_id_by_name(var_char), value.var_char()));

    // Check values.
    EXPECT_EQ(RETCODE_OK,
            data->get_int32_value(test_value.field1(),
            data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.field1(), test_value.field1());
    EXPECT_EQ(RETCODE_OK,
            data->get_float32_value(test_value.field2(),
            data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.field2(), test_value.field2());
    EXPECT_EQ(RETCODE_OK, data->get_char8_value(test_value.var_char(), data->get_member_id_by_name(var_char)));
    EXPECT_EQ(value.var_char(), test_value.var_char());

    for (auto encoding : encodings)
    {
        InnerStructureHelperEmptyChildChild struct_data;
        TypeSupport static_pubsubType {new InnerStructureHelperEmptyChildChildPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_InnerStructureHelperEmptyChildChild_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_InnerEmptyStructureHelperChild)
{
    DynamicType::_ref_type struct_type {create_inner_empty_struct_helper_child()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerEmptyStructureHelperChild value;
    value.var_child_longlong(-1000);
    value.var_child_ulonglong(334);
    InnerEmptyStructureHelperChild test_value;

    // Set values.
    EXPECT_EQ(RETCODE_OK,
            data->set_int64_value(data->get_member_id_by_name(var_child_longlong),
            value.var_child_longlong()));
    EXPECT_EQ(RETCODE_OK,
            data->set_uint64_value(data->get_member_id_by_name(var_child_ulonglong),
            value.var_child_ulonglong()));

    // Check values.
    EXPECT_EQ(RETCODE_OK,
            data->get_int64_value(test_value.var_child_longlong(),
            data->get_member_id_by_name(var_child_longlong)));
    EXPECT_EQ(value.var_child_longlong(), test_value.var_child_longlong());
    EXPECT_EQ(RETCODE_OK,
            data->get_uint64_value(test_value.var_child_ulonglong(), data->get_member_id_by_name(var_child_ulonglong)));
    EXPECT_EQ(value.var_child_ulonglong(), test_value.var_child_ulonglong());

    for (auto encoding : encodings)
    {
        InnerEmptyStructureHelperChild struct_data;
        TypeSupport static_pubsubType {new InnerEmptyStructureHelperChildPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_InnerEmptyStructureHelperChild_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);

}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructAliasInheritanceStruct)
{
    DynamicType::_ref_type struct_type {create_struct_alias_inheritance_struct()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);


    StructAliasInheritanceStruct value;
    value.field1(-1000);
    value.field2(3.34f);
    value.new_member(334);
    StructAliasInheritanceStruct test_value;

    // Set values.
    EXPECT_EQ(RETCODE_OK,
            data->set_int32_value(data->get_member_id_by_name(struct_long_member_name),
            value.field1()));
    EXPECT_EQ(RETCODE_OK,
            data->set_float32_value(data->get_member_id_by_name(struct_float_member_name),
            value.field2()));
    EXPECT_EQ(RETCODE_OK, data->set_int16_value(data->get_member_id_by_name(new_member), value.new_member()));

    // Check values.
    EXPECT_EQ(RETCODE_OK,
            data->get_int32_value(test_value.field1(),
            data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.field1(), test_value.field1());
    EXPECT_EQ(RETCODE_OK,
            data->get_float32_value(test_value.field2(), data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.field2(), test_value.field2());
    EXPECT_EQ(RETCODE_OK,
            data->get_int16_value(test_value.new_member(), data->get_member_id_by_name(new_member)));
    EXPECT_EQ(value.new_member(), test_value.new_member());

    for (auto encoding : encodings)
    {
        StructAliasInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new StructAliasInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructAliasInheritanceStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_StructuresInheritanceStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(structures_inheritance_struct_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_InnerStructureHelperChild);
    member_descriptor->type(create_inner_struct_helper_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_InnerStructureHelperChildChild);
    member_descriptor->type(create_inner_struct_helper_child_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_InnerStructureHelperEmptyChild);
    member_descriptor->type(create_inner_struct_helper_empty_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_InnerStructureHelperEmptyChildChild);
    member_descriptor->type(create_inner_struct_helper_empty_child_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_InnerEmptyStructureHelperChild);
    member_descriptor->type(create_inner_empty_struct_helper_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_StructAliasInheritanceStruct);
    member_descriptor->type(create_struct_alias_inheritance_struct());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    StructuresInheritanceStruct value;
    value.var_InnerStructureHelperChild().field1(-1000);
    value.var_InnerStructureHelperChild().field2(3.34f);
    value.var_InnerStructureHelperChild().var_child_longlong(-3000003);
    value.var_InnerStructureHelperChild().var_child_ulonglong(3000003);
    value.var_InnerStructureHelperChildChild().field1(-1000);
    value.var_InnerStructureHelperChildChild().field2(3.34f);
    value.var_InnerStructureHelperChildChild().var_child_longlong(-3000003);
    value.var_InnerStructureHelperChildChild().var_child_ulonglong(3000003);
    value.var_InnerStructureHelperChildChild().var_child_childlonglong2(-4005003);
    value.var_InnerStructureHelperChildChild().var_childchild_ulonglong2(3002003);
    value.var_InnerStructureHelperEmptyChild().field1(-1000);
    value.var_InnerStructureHelperEmptyChild().field2(3.34f);
    value.var_InnerStructureHelperEmptyChildChild().field1(-1000);
    value.var_InnerStructureHelperEmptyChildChild().field2(3.34f);
    value.var_InnerStructureHelperEmptyChildChild().var_char('X');
    value.var_InnerEmptyStructureHelperChild().var_child_longlong(-1000);
    value.var_InnerEmptyStructureHelperChild().var_child_ulonglong(334);
    value.var_StructAliasInheritanceStruct().field1(-1000);
    value.var_StructAliasInheritanceStruct().field2(3.34f);
    value.var_StructAliasInheritanceStruct().new_member(243);

    StructuresInheritanceStruct test_value;


    // Set values.
    auto inner_data {data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperChild))};
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int32_value(inner_data->get_member_id_by_name(struct_long_member_name),
            value.var_InnerStructureHelperChild().field1()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_float32_value(inner_data->get_member_id_by_name(struct_float_member_name),
            value.var_InnerStructureHelperChild().field2()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int64_value(inner_data->get_member_id_by_name(var_child_longlong),
            value.var_InnerStructureHelperChild().var_child_longlong()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_uint64_value(inner_data->get_member_id_by_name(var_child_ulonglong),
            value.var_InnerStructureHelperChild().var_child_ulonglong()));
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperChildChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int32_value(inner_data->get_member_id_by_name(struct_long_member_name),
            value.var_InnerStructureHelperChildChild().field1()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_float32_value(inner_data->get_member_id_by_name(struct_float_member_name),
            value.var_InnerStructureHelperChildChild().field2()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int64_value(inner_data->get_member_id_by_name(var_child_longlong),
            value.var_InnerStructureHelperChildChild().var_child_longlong()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_uint64_value(inner_data->get_member_id_by_name(var_child_ulonglong),
            value.var_InnerStructureHelperChildChild().var_child_ulonglong()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int64_value(inner_data->get_member_id_by_name(var_child_childlonglong2),
            value.var_InnerStructureHelperChildChild().var_child_childlonglong2()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_uint64_value(inner_data->get_member_id_by_name(var_childchild_ulonglong2),
            value.var_InnerStructureHelperChildChild().var_childchild_ulonglong2()));
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperEmptyChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int32_value(inner_data->get_member_id_by_name(struct_long_member_name),
            value.var_InnerStructureHelperEmptyChild().field1()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_float32_value(inner_data->get_member_id_by_name(struct_float_member_name),
            value.var_InnerStructureHelperEmptyChild().field2()));
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperEmptyChildChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int32_value(inner_data->get_member_id_by_name(struct_long_member_name),
            value.var_InnerStructureHelperEmptyChildChild().field1()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_float32_value(inner_data->get_member_id_by_name(struct_float_member_name),
            value.var_InnerStructureHelperEmptyChildChild().field2()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_char8_value(inner_data->get_member_id_by_name(var_char),
            value.var_InnerStructureHelperEmptyChildChild().var_char()));
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerEmptyStructureHelperChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int64_value(inner_data->get_member_id_by_name(var_child_longlong),
            value.var_InnerEmptyStructureHelperChild().var_child_longlong()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_uint64_value(inner_data->get_member_id_by_name(var_child_ulonglong),
            value.var_InnerEmptyStructureHelperChild().var_child_ulonglong()));
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_StructAliasInheritanceStruct));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int32_value(inner_data->get_member_id_by_name(struct_long_member_name),
            value.var_StructAliasInheritanceStruct().field1()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_float32_value(inner_data->get_member_id_by_name(struct_float_member_name),
            value.var_StructAliasInheritanceStruct().field2()));
    EXPECT_EQ(RETCODE_OK,
            inner_data->set_int16_value(inner_data->get_member_id_by_name(new_member),
            value.var_StructAliasInheritanceStruct().new_member()));
    data->return_loaned_value(inner_data);

    // Check values.
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int32_value(test_value.var_InnerStructureHelperChild().field1(),
            inner_data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperChild().field1(), test_value.var_InnerStructureHelperChild().field1());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_float32_value(test_value.var_InnerStructureHelperChild().field2(),
            inner_data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperChild().field2(), test_value.var_InnerStructureHelperChild().field2());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int64_value(test_value.var_InnerStructureHelperChild().var_child_longlong(),
            inner_data->get_member_id_by_name(var_child_longlong)));
    EXPECT_EQ(value.var_InnerStructureHelperChild().var_child_longlong(),
            test_value.var_InnerStructureHelperChild().var_child_longlong());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_uint64_value(test_value.var_InnerStructureHelperChild().var_child_ulonglong(),
            inner_data->get_member_id_by_name(var_child_ulonglong)));
    EXPECT_EQ(value.var_InnerStructureHelperChild().var_child_ulonglong(),
            test_value.var_InnerStructureHelperChild().var_child_ulonglong());
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperChildChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int32_value(test_value.var_InnerStructureHelperChildChild().field1(),
            inner_data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperChildChild().field1(),
            test_value.var_InnerStructureHelperChildChild().field1());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_float32_value(test_value.var_InnerStructureHelperChildChild().field2(),
            inner_data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperChildChild().field2(),
            test_value.var_InnerStructureHelperChildChild().field2());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int64_value(test_value.var_InnerStructureHelperChildChild().var_child_longlong(),
            inner_data->get_member_id_by_name(var_child_longlong)));
    EXPECT_EQ(value.var_InnerStructureHelperChildChild().var_child_longlong(),
            test_value.var_InnerStructureHelperChildChild().var_child_longlong());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_uint64_value(test_value.var_InnerStructureHelperChildChild().var_child_ulonglong(),
            inner_data->get_member_id_by_name(var_child_ulonglong)));
    EXPECT_EQ(value.var_InnerStructureHelperChildChild().var_child_ulonglong(),
            test_value.var_InnerStructureHelperChildChild().var_child_ulonglong());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int64_value(test_value.var_InnerStructureHelperChildChild().var_child_childlonglong2(),
            inner_data->get_member_id_by_name(var_child_childlonglong2)));
    EXPECT_EQ(value.var_InnerStructureHelperChildChild().var_child_childlonglong2(),
            test_value.var_InnerStructureHelperChildChild().var_child_childlonglong2());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_uint64_value(test_value.var_InnerStructureHelperChildChild().var_childchild_ulonglong2(),
            inner_data->get_member_id_by_name(var_childchild_ulonglong2)));
    EXPECT_EQ(value.var_InnerStructureHelperChildChild().var_childchild_ulonglong2(),
            test_value.var_InnerStructureHelperChildChild().var_childchild_ulonglong2());
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperEmptyChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int32_value(test_value.var_InnerStructureHelperEmptyChild().field1(),
            inner_data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperEmptyChild().field1(),
            test_value.var_InnerStructureHelperEmptyChild().field1());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_float32_value(test_value.var_InnerStructureHelperEmptyChild().field2(),
            inner_data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperEmptyChild().field2(),
            test_value.var_InnerStructureHelperEmptyChild().field2());
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerStructureHelperEmptyChildChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int32_value(test_value.var_InnerStructureHelperEmptyChildChild().field1(),
            inner_data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperEmptyChildChild().field1(),
            test_value.var_InnerStructureHelperEmptyChildChild().field1());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_float32_value(test_value.var_InnerStructureHelperEmptyChildChild().field2(),
            inner_data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.var_InnerStructureHelperEmptyChildChild().field2(),
            test_value.var_InnerStructureHelperEmptyChildChild().field2());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_char8_value(test_value.var_InnerStructureHelperEmptyChildChild().var_char(),
            inner_data->get_member_id_by_name(var_char)));
    EXPECT_EQ(value.var_InnerStructureHelperEmptyChildChild().var_char(),
            test_value.var_InnerStructureHelperEmptyChildChild().var_char());
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_InnerEmptyStructureHelperChild));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int64_value(test_value.var_InnerEmptyStructureHelperChild().var_child_longlong(),
            inner_data->get_member_id_by_name(var_child_longlong)));
    EXPECT_EQ(value.var_InnerEmptyStructureHelperChild().var_child_longlong(),
            test_value.var_InnerEmptyStructureHelperChild().var_child_longlong());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_uint64_value(test_value.var_InnerEmptyStructureHelperChild().var_child_ulonglong(),
            inner_data->get_member_id_by_name(var_child_ulonglong)));
    EXPECT_EQ(value.var_InnerEmptyStructureHelperChild().var_child_ulonglong(),
            test_value.var_InnerEmptyStructureHelperChild().var_child_ulonglong());
    data->return_loaned_value(inner_data);
    inner_data = data->loan_value(data->get_member_id_by_name(var_StructAliasInheritanceStruct));
    ASSERT_TRUE(inner_data);
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int32_value(test_value.var_StructAliasInheritanceStruct().field1(),
            inner_data->get_member_id_by_name(struct_long_member_name)));
    EXPECT_EQ(value.var_StructAliasInheritanceStruct().field1(),
            test_value.var_StructAliasInheritanceStruct().field1());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_float32_value(test_value.var_StructAliasInheritanceStruct().field2(),
            inner_data->get_member_id_by_name(struct_float_member_name)));
    EXPECT_EQ(value.var_StructAliasInheritanceStruct().field2(),
            test_value.var_StructAliasInheritanceStruct().field2());
    EXPECT_EQ(RETCODE_OK,
            inner_data->get_int16_value(test_value.var_StructAliasInheritanceStruct().new_member(),
            inner_data->get_member_id_by_name(new_member)));
    EXPECT_EQ(value.var_StructAliasInheritanceStruct().new_member(),
            test_value.var_StructAliasInheritanceStruct().new_member());
    data->return_loaned_value(inner_data);

    for (auto encoding : encodings)
    {
        StructuresInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new StructuresInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(value, struct_data);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_StructuresInheritanceStruct_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BitsetsChildInheritanceStruct)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(bitsets_child_inheritance_struct_struct_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_InnerBitsetHelperChild);
    member_descriptor->type(create_inner_bitset_helper_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_InnerBitsetHelperChildChild);
    member_descriptor->type(create_inner_bitset_helper_child_child());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_BitsetAliasInheritanceBitset);
    member_descriptor->type(create_bitset_alias_inheritance_bitset());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type {type_builder->build()};
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    uint8_t uint8_value {5};
    bool bool_value {true};
    uint16_t ushort_value {1000};
    int16_t short_value {2000};
    uint32_t long_value {111};
    uint8_t test_uint8_value;
    bool test_bool_value;
    uint16_t test_ushort_value;
    int16_t test_short_value;
    uint32_t test_long_value;

    // Set values.
    DynamicData::_ref_type bitset_data {data->loan_value(data->get_member_id_by_name(var_InnerBitsetHelperChild))};
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), uint8_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(bitfield_b), bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(bitfield_c), ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(bitfield_d), short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint32_value(bitset_data->get_member_id_by_name(child_w), long_value), RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = data->loan_value(data->get_member_id_by_name(var_InnerBitsetHelperChildChild));
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), uint8_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(bitfield_b), bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(bitfield_c), ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(bitfield_d), short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint32_value(bitset_data->get_member_id_by_name(child_w), long_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(childchild_z), ushort_value),
            RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = data->loan_value(data->get_member_id_by_name(var_BitsetAliasInheritanceBitset));
    EXPECT_EQ(bitset_data->set_uint8_value(bitset_data->get_member_id_by_name(bitfield_a), uint8_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_boolean_value(bitset_data->get_member_id_by_name(bitfield_b), bool_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(bitfield_c), ushort_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_int16_value(bitset_data->get_member_id_by_name(bitfield_d), short_value), RETCODE_OK);
    EXPECT_EQ(bitset_data->set_uint16_value(bitset_data->get_member_id_by_name(new_bitfield), ushort_value),
            RETCODE_OK);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);

    // Check values
    bitset_data = data->loan_value(data->get_member_id_by_name(var_InnerBitsetHelperChild));
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(bitset_data->get_uint32_value(test_long_value, bitset_data->get_member_id_by_name(child_w)),
            RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = data->loan_value(data->get_member_id_by_name(var_InnerBitsetHelperChildChild));
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(bitset_data->get_uint32_value(test_long_value, bitset_data->get_member_id_by_name(child_w)),
            RETCODE_OK);
    EXPECT_EQ(long_value, test_long_value);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                childchild_z)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);
    bitset_data = data->loan_value(data->get_member_id_by_name(var_BitsetAliasInheritanceBitset));
    ASSERT_TRUE(bitset_data);
    EXPECT_EQ(bitset_data->get_uint8_value(test_uint8_value, bitset_data->get_member_id_by_name(bitfield_a)),
            RETCODE_OK);
    EXPECT_EQ(uint8_value, test_uint8_value);
    EXPECT_EQ(bitset_data->get_boolean_value(test_bool_value, bitset_data->get_member_id_by_name(
                bitfield_b)), RETCODE_OK);
    EXPECT_EQ(bool_value, test_bool_value);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(
                bitfield_c)), RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(bitset_data->get_int16_value(test_short_value, bitset_data->get_member_id_by_name(bitfield_d)),
            RETCODE_OK);
    EXPECT_EQ(short_value, test_short_value);
    EXPECT_EQ(bitset_data->get_uint16_value(test_ushort_value, bitset_data->get_member_id_by_name(new_bitfield)),
            RETCODE_OK);
    EXPECT_EQ(ushort_value, test_ushort_value);
    EXPECT_EQ(data->return_loaned_value(bitset_data), RETCODE_OK);

    for (auto encoding : encodings)
    {
        BitsetsChildInheritanceStruct struct_data;
        TypeSupport static_pubsubType {new BitsetsChildInheritanceStructPubSubType()};
        check_serialization_deserialization(struct_type, data, encoding, struct_data,
                static_pubsubType);
        EXPECT_EQ(uint8_value, struct_data.var_InnerBitsetHelperChild().a);
        EXPECT_EQ(bool_value, struct_data.var_InnerBitsetHelperChild().b);
        EXPECT_EQ(ushort_value, struct_data.var_InnerBitsetHelperChild().c);
        EXPECT_EQ(short_value, struct_data.var_InnerBitsetHelperChild().d);
        EXPECT_EQ(long_value, struct_data.var_InnerBitsetHelperChild().child_w);
        EXPECT_EQ(uint8_value, struct_data.var_InnerBitsetHelperChildChild().a);
        EXPECT_EQ(bool_value, struct_data.var_InnerBitsetHelperChildChild().b);
        EXPECT_EQ(ushort_value, struct_data.var_InnerBitsetHelperChildChild().c);
        EXPECT_EQ(short_value, struct_data.var_InnerBitsetHelperChildChild().d);
        EXPECT_EQ(long_value, struct_data.var_InnerBitsetHelperChildChild().child_w);
        EXPECT_EQ(ushort_value, struct_data.var_InnerBitsetHelperChildChild().childchild_z);
        EXPECT_EQ(uint8_value, struct_data.var_BitsetAliasInheritanceBitset().a);
        EXPECT_EQ(bool_value, struct_data.var_BitsetAliasInheritanceBitset().b);
        EXPECT_EQ(ushort_value, struct_data.var_BitsetAliasInheritanceBitset().c);
        EXPECT_EQ(short_value, struct_data.var_BitsetAliasInheritanceBitset().d);
        EXPECT_EQ(ushort_value, struct_data.var_BitsetAliasInheritanceBitset().new_bitfield);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BitsetsChildInheritanceStruct_type_identifier(static_type_ids);
    EXPECT_NE(static_type_ids, xtypes::TypeIdentifierPair());
    xtypes::TypeIdentifierPair dynamic_type_ids;
    EXPECT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
                    register_typeobject_w_dynamic_type(struct_type, dynamic_type_ids));
    EXPECT_EQ(static_type_ids.type_identifier1(), dynamic_type_ids.type_identifier1());
    EXPECT_EQ(static_type_ids.type_identifier2(), dynamic_type_ids.type_identifier2());
    xtypes::TypeObject type_object;
    bool ec {false};
    EXPECT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                xtypes::TypeObjectUtils::retrieve_complete_type_identifier(dynamic_type_ids, ec), type_object));
    DynamicTypeBuilder::_ref_type builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        type_object);
    ASSERT_NE(builder, nullptr);
    // This `builder` will never be equal than origin `struct_type` because translation to TypeObject removes inheritance
    // and makes the bitset plain.

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

