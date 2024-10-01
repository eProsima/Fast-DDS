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
#include "../../../dds-types-test/enumerationsPubSubTypes.hpp"
#include "../../../dds-types-test/enumerationsTypeObjectSupport.hpp"
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr const char* enum_enumwithvalues_name = "EnumWithValues";
constexpr const char* struct_enumstructure_name = "EnumStructure";
constexpr const char* struct_bitmaskstructure_name = "BitMaskStructure";
constexpr const char* struct_boundedbitmaskstructure_name = "BoundedBitMaskStructure";
constexpr const char* struct_enumwithvaluesstructure_name = "EnumWithValuesStructure";

constexpr const char* var_innerenumhelper_name = "var_InnerEnumHelper";
constexpr const char* var_scoped_innerenumhelper = "var_scoped_InnerEnumHelper";
constexpr const char* var_innerbitmaskhelper_name = "var_InnerBitMaskHelper";
constexpr const char* var_innerboundedbitmaskhelper_name = "var_InnerBoundedBitMaskHelper";
constexpr const char* var_enumwithvalues_name = "var_enumwithvalues";

DynamicType::_ref_type create_scoped_inner_enum_helper()
{
    TypeDescriptor::_ref_type enum_descriptor {traits<TypeDescriptor>::make_shared()};
    enum_descriptor->kind(TK_ENUM);
    enum_descriptor->name(std::string("Test::") + std::string(enum_name));
    DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                    enum_descriptor)};

    MemberDescriptor::_ref_type enum_literal_descriptor {traits<MemberDescriptor>::make_shared()};
    enum_literal_descriptor->type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
            TK_INT32));
    enum_literal_descriptor->name(enum_value_1_name);
    enum_builder->add_member(enum_literal_descriptor);
    enum_literal_descriptor = traits<MemberDescriptor>::make_shared();
    enum_literal_descriptor->type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
            TK_INT32));
    enum_literal_descriptor->name(enum_value_2_name);
    enum_builder->add_member(enum_literal_descriptor);
    enum_literal_descriptor = traits<MemberDescriptor>::make_shared();
    enum_literal_descriptor->type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
            TK_INT32));
    enum_literal_descriptor->name(enum_value_3_name);
    enum_builder->add_member(enum_literal_descriptor);

    return enum_builder->build();
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_EnumStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_enumstructure_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                    type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_innerenumhelper_name);
    member_descriptor->type(create_inner_enum_helper());
    type_builder->add_member(member_descriptor);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name(var_scoped_innerenumhelper);
    member_descriptor->type(create_scoped_inner_enum_helper());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerEnumHelper value = InnerEnumHelper::ENUM_VALUE_2;
    ::Test::InnerEnumHelper scoped_value = ::Test::InnerEnumHelper::ENUM_VALUE_3;
    int32_t test_value = 0;
    int32_t scoped_test_value = 0;
    EXPECT_EQ(
        data->set_int32_value(
            data->get_member_id_by_name(
                var_innerenumhelper_name), static_cast<int32_t>(value)), RETCODE_OK);
    EXPECT_EQ(
        data->get_int32_value(
            test_value, data->get_member_id_by_name(
                var_innerenumhelper_name)), RETCODE_OK);
    EXPECT_EQ(static_cast<int32_t>(value), test_value);

    EXPECT_EQ(
        data->set_int32_value(
            data->get_member_id_by_name(
                var_scoped_innerenumhelper), static_cast<int32_t>(scoped_value)), RETCODE_OK);
    EXPECT_EQ(
        data->get_int32_value(
            scoped_test_value, data->get_member_id_by_name(
                var_scoped_innerenumhelper)), RETCODE_OK);
    EXPECT_EQ(static_cast<int32_t>(scoped_value), scoped_test_value);

    for (auto encoding : encodings)
    {
        EnumStructure struct_data;
        TypeSupport static_pubsubType {new EnumStructurePubSubType()};
        check_serialization_deserialization(
            struct_type, data, encoding, struct_data,
            static_pubsubType);
        EXPECT_EQ(static_cast<int32_t>(struct_data.var_InnerEnumHelper()), test_value);
        EXPECT_EQ(static_cast<int32_t>(struct_data.var_scoped_InnerEnumHelper()), scoped_test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_EnumStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BitMaskStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_bitmaskstructure_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                    type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_innerbitmaskhelper_name);
    member_descriptor->type(create_inner_bitmask_helper());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerBitMaskHelper value = InnerBitMaskHelperBits::flag0 | InnerBitMaskHelperBits::flag1 |
            InnerBitMaskHelperBits::flag4 | InnerBitMaskHelperBits::flag6;
    InnerBitMaskHelper test_value = 0;
    EXPECT_EQ(
        data->set_uint32_value(
            data->get_member_id_by_name(
                var_innerbitmaskhelper_name), value), RETCODE_OK);
    EXPECT_EQ(
        data->get_uint32_value(
            test_value,
            data->get_member_id_by_name(var_innerbitmaskhelper_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        BitMaskStructure struct_data;
        TypeSupport static_pubsubType {new BitMaskStructurePubSubType()};
        check_serialization_deserialization(
            struct_type, data, encoding, struct_data,
            static_pubsubType);
        EXPECT_EQ(struct_data.var_InnerBitMaskHelper(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BitMaskStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_BoundedBitMaskStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_boundedbitmaskstructure_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                    type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_innerboundedbitmaskhelper_name);
    member_descriptor->type(create_inner_bounded_bitmask_helper());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    InnerBoundedBitMaskHelper value = InnerBoundedBitMaskHelperBits::bflag0 |
            InnerBoundedBitMaskHelperBits::bflag1 |
            InnerBoundedBitMaskHelperBits::bflag4 | InnerBoundedBitMaskHelperBits::bflag6;
    InnerBoundedBitMaskHelper test_value = 0;
    EXPECT_EQ(
        data->set_uint8_value(data->get_member_id_by_name(var_innerboundedbitmaskhelper_name), value),
        RETCODE_OK);
    EXPECT_EQ(
        data->get_uint8_value(
            test_value, data->get_member_id_by_name(
                var_innerboundedbitmaskhelper_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        BoundedBitMaskStructure struct_data;
        TypeSupport static_pubsubType {new BoundedBitMaskStructurePubSubType()};
        check_serialization_deserialization(
            struct_type, data, encoding, struct_data,
            static_pubsubType);
        EXPECT_EQ(struct_data.var_InnerBoundedBitMaskHelper(), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_BoundedBitMaskStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesDDSTypesTest, DDSTypesTest_EnumWithValuesStructure)
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(struct_enumwithvaluesstructure_name);
    DynamicTypeBuilder::_ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                    type_descriptor)};

    TypeDescriptor::_ref_type enum_descriptor {traits<TypeDescriptor>::make_shared()};
    enum_descriptor->kind(TK_ENUM);
    enum_descriptor->name(enum_enumwithvalues_name);
    DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                    enum_descriptor)};

    MemberDescriptor::_ref_type enum_literal_descriptor {traits<MemberDescriptor>::make_shared()};
    enum_literal_descriptor->type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
            TK_INT32));
    enum_literal_descriptor->name("ENUM_VALUE1");
    enum_literal_descriptor->default_value("-3");
    enum_builder->add_member(enum_literal_descriptor);
    enum_literal_descriptor = traits<MemberDescriptor>::make_shared();
    enum_literal_descriptor->type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
            TK_INT32));
    enum_literal_descriptor->name("ENUM_VALUE2");
    enum_literal_descriptor->default_value("0");
    enum_builder->add_member(enum_literal_descriptor);
    enum_literal_descriptor = traits<MemberDescriptor>::make_shared();
    enum_literal_descriptor->type(
        DynamicTypeBuilderFactory::get_instance()->get_primitive_type(
            TK_INT32));
    enum_literal_descriptor->name("ENUM_VALUE3");
    enum_literal_descriptor->default_value("3");
    enum_builder->add_member(enum_literal_descriptor);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name(var_enumwithvalues_name);
    member_descriptor->type(enum_builder->build());
    type_builder->add_member(member_descriptor);

    DynamicType::_ref_type struct_type = type_builder->build();

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    int32_t value = 3;
    int32_t test_value = 0;
    EXPECT_EQ(
        data->set_int32_value(
            data->get_member_id_by_name(
                var_enumwithvalues_name), value), RETCODE_OK);
    EXPECT_EQ(
        data->get_int32_value(test_value,
        data->get_member_id_by_name(var_enumwithvalues_name)), RETCODE_OK);
    EXPECT_EQ(value, test_value);

    for (auto encoding : encodings)
    {
        EnumWithValuesStructure struct_data;
        TypeSupport static_pubsubType {new EnumWithValuesStructurePubSubType()};
        check_serialization_deserialization(
            struct_type, data, encoding, struct_data,
            static_pubsubType);
        EXPECT_EQ(static_cast<int32_t>(struct_data.var_enumwithvalues()), test_value);
    }

    xtypes::TypeIdentifierPair static_type_ids;
    register_EnumWithValuesStructure_type_identifier(static_type_ids);
    check_typeobject_registry(struct_type, static_type_ids);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}


} // dds
} // fastdds
} // eprosima
