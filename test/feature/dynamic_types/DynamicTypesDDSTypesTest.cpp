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

#include "DynamicTypesDDSTypesTest.hpp"

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

std::array<DataRepresentationId_t, 2> encodings {{XCDR_DATA_REPRESENTATION, XCDR2_DATA_REPRESENTATION}};

DynamicTypesDDSTypesTest::~DynamicTypesDDSTypesTest()
{
    eprosima::fastdds::dds::Log::KillThread();
}

void DynamicTypesDDSTypesTest::TearDown()
{
    DynamicDataFactory::delete_instance();
    DynamicTypeBuilderFactory::delete_instance();
}

void DynamicTypesDDSTypesTest::check_typeobject_registry(
        const DynamicType::_ref_type& dyn_type,
        const xtypes::TypeIdentifierPair& static_type_ids)
{
    EXPECT_NE(static_type_ids.type_identifier1(), xtypes::TypeIdentifier());
    EXPECT_NE(static_type_ids.type_identifier2(), xtypes::TypeIdentifier());
    xtypes::TypeIdentifierPair dynamic_type_ids;
    EXPECT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().
                    register_typeobject_w_dynamic_type(dyn_type, dynamic_type_ids));
    EXPECT_EQ(static_type_ids.type_identifier1(), dynamic_type_ids.type_identifier1());
    EXPECT_EQ(static_type_ids.type_identifier2(), dynamic_type_ids.type_identifier2());
    xtypes::TypeObject type_object;
    bool ec {false};
    EXPECT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                xtypes::TypeObjectUtils::retrieve_complete_type_identifier(dynamic_type_ids, ec), type_object));
    DynamicTypeBuilder::_ref_type builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        type_object);
    ASSERT_NE(builder, nullptr);
    EXPECT_TRUE(builder->equals(dyn_type));
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_enum_helper()
{
    TypeDescriptor::_ref_type enum_descriptor {traits<TypeDescriptor>::make_shared()};
    enum_descriptor->kind(TK_ENUM);
    enum_descriptor->name(enum_name);
    DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->create_type(enum_descriptor)};

    MemberDescriptor::_ref_type enum_literal_descriptor {traits<MemberDescriptor>::make_shared()};
    enum_literal_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    enum_literal_descriptor->name(enum_value_1_name);
    enum_builder->add_member(enum_literal_descriptor);
    enum_literal_descriptor = traits<MemberDescriptor>::make_shared();
    enum_literal_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    enum_literal_descriptor->name(enum_value_2_name);
    enum_builder->add_member(enum_literal_descriptor);
    enum_literal_descriptor = traits<MemberDescriptor>::make_shared();
    enum_literal_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    enum_literal_descriptor->name(enum_value_3_name);
    enum_builder->add_member(enum_literal_descriptor);

    return enum_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_bitmask_helper()
{
    TypeDescriptor::_ref_type bitmask_descriptor {traits<TypeDescriptor>::make_shared()};
    bitmask_descriptor->kind(TK_BITMASK);
    bitmask_descriptor->name(bitmask_name);
    bitmask_descriptor->element_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitmask_descriptor->bound().push_back(32);
    DynamicTypeBuilder::_ref_type bitmask_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                       bitmask_descriptor)};

    MemberDescriptor::_ref_type bitfield_descriptor {traits<MemberDescriptor>::make_shared()};
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bitmask_flag_0_name);
    bitfield_descriptor->id(0);
    bitmask_builder->add_member(bitfield_descriptor);
    bitfield_descriptor = traits<MemberDescriptor>::make_shared();
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bitmask_flag_1_name);
    bitfield_descriptor->id(1);
    bitmask_builder->add_member(bitfield_descriptor);
    bitfield_descriptor = traits<MemberDescriptor>::make_shared();
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bitmask_flag_4_name);
    bitfield_descriptor->id(4);
    bitmask_builder->add_member(bitfield_descriptor);
    bitfield_descriptor = traits<MemberDescriptor>::make_shared();
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bitmask_flag_6_name);
    bitfield_descriptor->id(6);
    bitmask_builder->add_member(bitfield_descriptor);

    return bitmask_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_bounded_bitmask_helper()
{
    TypeDescriptor::_ref_type bounded_bitmask_descriptor {traits<TypeDescriptor>::make_shared()};
    bounded_bitmask_descriptor->kind(TK_BITMASK);
    bounded_bitmask_descriptor->name(bounded_bitmask_name);
    bounded_bitmask_descriptor->element_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bounded_bitmask_descriptor->bound().push_back(8);
    DynamicTypeBuilder::_ref_type bounded_bitmask_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                               bounded_bitmask_descriptor)};

    MemberDescriptor::_ref_type bitfield_descriptor {traits<MemberDescriptor>::make_shared()};
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bounded_bitmask_bflag_0_name);
    bitfield_descriptor->id(0);
    bounded_bitmask_builder->add_member(bitfield_descriptor);
    bitfield_descriptor = traits<MemberDescriptor>::make_shared();
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bounded_bitmask_bflag_1_name);
    bitfield_descriptor->id(1);
    bounded_bitmask_builder->add_member(bitfield_descriptor);
    bitfield_descriptor = traits<MemberDescriptor>::make_shared();
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bounded_bitmask_bflag_4_name);
    bitfield_descriptor->id(4);
    bounded_bitmask_builder->add_member(bitfield_descriptor);
    bitfield_descriptor = traits<MemberDescriptor>::make_shared();
    bitfield_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitfield_descriptor->name(bounded_bitmask_bflag_6_name);
    bitfield_descriptor->id(6);
    bounded_bitmask_builder->add_member(bitfield_descriptor);

    return bounded_bitmask_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_alias_helper()
{
    TypeDescriptor::_ref_type inner_alias_descriptor {traits<TypeDescriptor>::make_shared()};
    inner_alias_descriptor->kind(TK_ALIAS);
    inner_alias_descriptor->name(alias_name);
    inner_alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));

    return DynamicTypeBuilderFactory::get_instance()->create_type(inner_alias_descriptor)->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_union_helper()
{
    TypeDescriptor::_ref_type union_descriptor {traits<TypeDescriptor>::make_shared()};
    union_descriptor->kind(TK_UNION);
    union_descriptor->name(union_name);
    union_descriptor->discriminator_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    union_descriptor->is_nested(true);
    DynamicTypeBuilder::_ref_type union_builder {DynamicTypeBuilderFactory::get_instance()->create_type(union_descriptor)};

    MemberDescriptor::_ref_type union_member {traits<MemberDescriptor>::make_shared()};
    union_member->name(union_long_member_name);
    union_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    union_member->label({0});
    union_builder->add_member(union_member);
    union_member = traits<MemberDescriptor>::make_shared();
    union_member->name(union_float_member_name);
    union_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    union_member->label({1});
    union_builder->add_member(union_member);
    union_member = traits<MemberDescriptor>::make_shared();
    union_member->name(union_short_member_name);
    union_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    union_member->is_default_label(true);
    union_builder->add_member(union_member);

    return union_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_empty_struct_helper()
{
    TypeDescriptor::_ref_type struct_descriptor {traits<TypeDescriptor>::make_shared()};
    struct_descriptor->kind(TK_STRUCTURE);
    struct_descriptor->name(empty_struct_name);
    struct_descriptor->is_nested(true);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      struct_descriptor)};

    return struct_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_struct_helper()
{
    TypeDescriptor::_ref_type struct_descriptor {traits<TypeDescriptor>::make_shared()};
    struct_descriptor->kind(TK_STRUCTURE);
    struct_descriptor->name(struct_name);
    struct_descriptor->is_nested(true);
    DynamicTypeBuilder::_ref_type struct_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      struct_descriptor)};

    MemberDescriptor::_ref_type struct_member {traits<MemberDescriptor>::make_shared()};
    struct_member->name(struct_long_member_name);
    struct_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
    struct_builder->add_member(struct_member);
    struct_member = traits<MemberDescriptor>::make_shared();
    struct_member->name(struct_float_member_name);
    struct_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_FLOAT32));
    struct_builder->add_member(struct_member);

    return struct_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_bitset_helper()
{
    TypeDescriptor::_ref_type bitset_descriptor {traits<TypeDescriptor>::make_shared()};
    bitset_descriptor->kind(TK_BITSET);
    bitset_descriptor->name(bitset_name);
    bitset_descriptor->bound({3, 1, 10, 12});
    DynamicTypeBuilder::_ref_type bitset_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                      bitset_descriptor)};

    MemberDescriptor::_ref_type bitset_member {traits<MemberDescriptor>::make_shared()};
    bitset_member->name(bitfield_a);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT8));
    bitset_member->id(0);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(bitfield_b);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitset_member->id(3);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(bitfield_c);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
    bitset_member->id(8);
    bitset_builder->add_member(bitset_member);
    bitset_member = traits<MemberDescriptor>::make_shared();
    bitset_member->name(bitfield_d);
    bitset_member->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16));
    bitset_member->id(21);
    bitset_builder->add_member(bitset_member);

    return bitset_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_alias_bounded_string_helper()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(bounded_string_alias);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_string_type(10)->build());

    return DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_alias_bounded_wstring_helper()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(bounded_wstring_alias);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_wstring_type(10)->build());

    return DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_alias_array_helper()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(array_alias);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_array_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT16), {2})->build());

    return DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_alias_sequence_helper()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(seq_alias);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_sequence_type(
                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                    TK_INT16), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());

    return DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_alias_map_helper()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(map_alias);
    alias_descriptor->base_type(DynamicTypeBuilderFactory::get_instance()->create_map_type(DynamicTypeBuilderFactory::
                    get_instance()->get_primitive_type(TK_INT32),
            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
                TK_INT32), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());

    return DynamicTypeBuilderFactory::get_instance()->create_type(alias_descriptor)->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_struct_helper_alias()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(inner_struct_helper_alias_struct_name);
    alias_descriptor->base_type(create_inner_struct_helper());

    DynamicTypeBuilder::_ref_type alias_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                     alias_descriptor)};

    return alias_builder->build();
}

DynamicType::_ref_type DynamicTypesDDSTypesTest::create_inner_bitset_helper_alias()
{
    TypeDescriptor::_ref_type alias_descriptor {traits<TypeDescriptor>::make_shared()};
    alias_descriptor->kind(TK_ALIAS);
    alias_descriptor->name(inner_bitset_helper_alias_struct_name);
    alias_descriptor->base_type(create_inner_bitset_helper());

    DynamicTypeBuilder::_ref_type alias_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                     alias_descriptor)};

    return alias_builder->build();
}

} // dds
} // fastdds
} // eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
