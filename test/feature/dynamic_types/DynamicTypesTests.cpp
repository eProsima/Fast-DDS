// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <set>
#include <sstream>
#include <thread>
#include <tuple>

#include <ScopedLogs.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::dds;

// Ancillary gtest formatters

using primitive_builder_api = const DynamicTypeBuilder * (DynamicTypeBuilderFactory::* )();
using primitive_type_api = const DynamicType * (DynamicTypeBuilderFactory::* )();

// Testing the primitive creation APIS
// and get_primitive_type() and create_primitive_type()
class DynamicTypesPrimitiveTestsAPIs
    : public testing::TestWithParam <TypeKind>
{
};

TEST_P(DynamicTypesPrimitiveTestsAPIs, primitives_apis)
{
    // Get the factory singleton
    traits<DynamicTypeBuilderFactory>::ref_type factory = DynamicTypeBuilderFactory::get_instance();

    // Retrieve parameters
    TypeKind kind {GetParam()};

    // Create the primitive builder,
    // note that create_xxx_type rely on create_primitive_type<TK_xxxx>()
    traits<DynamicType>::ref_type type1 {factory->get_primitive_type(kind)};
    ASSERT_TRUE(type1);

    // It must be the right builder
    ASSERT_EQ(type1->get_kind(), kind);

    // The primitive builder is statically allocated and must always be the same instance
    traits<DynamicType>::ref_type type2 {factory->get_primitive_type(kind)};
    ASSERT_TRUE(type2);
    ASSERT_EQ(type1, type2);
    ASSERT_TRUE(type1->equals(type2));

    // It must be possible to create a custom builder from a primitive one
    traits<DynamicTypeBuilder>::ref_type custom_builder {factory->create_type_copy(type1)};
    ASSERT_TRUE(custom_builder);

    // but must share its state
    EXPECT_TRUE(custom_builder->equals(type1));

    traits<DynamicType>::ref_type custom_type1 {custom_builder->build()};
    ASSERT_TRUE(custom_type1);

    // It must share the state with the builder
    ASSERT_TRUE(custom_builder->equals(custom_type1));

    // It must return a cached instances if there are not changes
    traits<DynamicType>::ref_type custom_type2 {custom_builder->build()};
    ASSERT_TRUE(custom_type2);
    ASSERT_TRUE(custom_type1->equals(custom_type2));
}

INSTANTIATE_TEST_SUITE_P(CheckingGetPrimitiveType,
        DynamicTypesPrimitiveTestsAPIs,
        testing::Values(
            eprosima::fastdds::dds::TK_INT8,
            eprosima::fastdds::dds::TK_UINT8,
            eprosima::fastdds::dds::TK_INT16,
            eprosima::fastdds::dds::TK_UINT16,
            eprosima::fastdds::dds::TK_INT32,
            eprosima::fastdds::dds::TK_UINT32,
            eprosima::fastdds::dds::TK_INT64,
            eprosima::fastdds::dds::TK_UINT64,
            eprosima::fastdds::dds::TK_FLOAT32,
            eprosima::fastdds::dds::TK_FLOAT64,
            eprosima::fastdds::dds::TK_FLOAT128,
            eprosima::fastdds::dds::TK_CHAR8,
            eprosima::fastdds::dds::TK_CHAR16,
            eprosima::fastdds::dds::TK_BOOLEAN,
            eprosima::fastdds::dds::TK_BYTE));

class DynamicTypesTests : public ::testing::Test
{
    const std::string config_file_ = "types_profile.xml";

public:

    DynamicTypesTests()
    {
    }

    ~DynamicTypesTests()
    {
        eprosima::fastdds::dds::Log::Flush();
    }

    virtual void TearDown()
    {
        DynamicDataFactory::delete_instance();
        DynamicTypeBuilderFactory::delete_instance();

    }

    const std::string& config_file()
    {
        return config_file_;
    }

    struct order_member_desc
        : public std::binary_function<const MemberDescriptor&, const MemberDescriptor&, bool>
    {
        result_type operator ()(
                first_argument_type lhs,
                second_argument_type rhs ) const
        {
            return lhs.index() < rhs.index();
        }

    };
};


TEST_F(DynamicTypesTests, TypeDescriptors)
{
    // Do not use the TypeDescriptor to:
    // + Get primitive types. Use the DynamicTypeBuilderFactory instead.
    // + Create new types. Use a Builder instead.

    // We want to create a new type based on int32_t
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    // get static builder
    DynamicType::_ref_type primitive {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32)};
    ASSERT_TRUE(primitive);
    // Create a modifiable builder copy
    DynamicTypeBuilder::_ref_type builder {factory->create_type_copy(primitive)};
    ASSERT_TRUE(builder);
    EXPECT_EQ(builder->get_kind(), eprosima::fastdds::dds::TK_INT32);

    // Use TypeDescriptor to capture the state
    TypeDescriptor::_ref_type state {traits<TypeDescriptor>::make_shared()};
    ASSERT_EQ(primitive->get_descriptor(state), RETCODE_OK);
    DynamicTypeBuilder::_ref_type builder2 {factory->create_type(state)};
    ASSERT_TRUE(builder2);
    EXPECT_TRUE(builder2->equals(primitive));

    // Copy state
    TypeDescriptor::_ref_type state2 {traits<TypeDescriptor>::make_shared()};
    EXPECT_EQ(state2->copy_from(state), RETCODE_OK);
    EXPECT_TRUE(state2->equals(state));

    TypeDescriptor::_ref_type state3 {traits<TypeDescriptor>::make_shared()};
    ASSERT_EQ(builder->get_descriptor(state3), RETCODE_OK);
    EXPECT_EQ(state2->copy_from(state3), RETCODE_OK);
    EXPECT_TRUE(state2->equals(state3));

    // Check state doesn't match the default descriptor
    TypeDescriptor::_ref_type default_descriptor {traits<TypeDescriptor>::make_shared()};
    EXPECT_FALSE(state->equals(default_descriptor));
}

TEST_F(DynamicTypesTests, DynamicType_basic)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create basic types
    TypeDescriptor::_ref_type struct_descriptor {traits<TypeDescriptor>::make_shared()};
    struct_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    struct_descriptor->name("mystructure");
    EXPECT_TRUE(struct_descriptor->is_consistent());
    DynamicTypeBuilder::_ref_type struct_type_builder {factory->create_type(struct_descriptor)};
    ASSERT_TRUE(struct_type_builder);
    EXPECT_EQ(struct_type_builder->get_kind(), eprosima::fastdds::dds::TK_STRUCTURE);
    EXPECT_EQ(struct_type_builder->get_name(), "mystructure");
    EXPECT_EQ(struct_type_builder->get_member_count(), 0u);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->id(3);
    member_descriptor->name("int32");
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->add_member(member_descriptor));
    EXPECT_EQ(struct_type_builder->get_member_count(), 1u);

    DynamicType::_ref_type struct_type {struct_type_builder->build()};
    ASSERT_TRUE(struct_type);
    EXPECT_TRUE(struct_type_builder->equals(struct_type));

    member_descriptor->id(1);
    member_descriptor->name("int64");
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->add_member(member_descriptor));
    EXPECT_EQ(struct_type_builder->get_member_count(), 2u);

    DynamicType::_ref_type struct_type2 {struct_type_builder->build()};
    ASSERT_TRUE(struct_type2);
    EXPECT_TRUE(struct_type_builder->equals(struct_type2));
    EXPECT_FALSE(struct_type->equals(struct_type2));

    DynamicTypeMember::_ref_type member;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Check members are properly added
        // • checking invalid id
        EXPECT_NE(RETCODE_OK, struct_type_builder->get_member(member, 0));
        EXPECT_FALSE(member);
    }

    // • checking MemberDescriptor getters
    MemberDescriptor::_ref_type md = traits<MemberDescriptor>::make_shared();
    MemberDescriptor::_ref_type md1 = traits<MemberDescriptor>::make_shared();
    md1->id(3);
    md1->name("int32");
    md1->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));

    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member(member, 3));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));

    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    // • checking MemberDescriptor comparison and construction
    MemberDescriptor::_ref_type md2 = traits<MemberDescriptor>::make_shared();
    md2->id(1);
    md2->name("int64");
    md2->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member(member, 1));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 1u);
    EXPECT_EQ(md->name(), md2->name());
    EXPECT_EQ(md->type(), md2->type());

    EXPECT_FALSE(md1->equals(md2));

    //    + checking copy_from
    EXPECT_EQ(RETCODE_OK, md->copy_from(md1));
    EXPECT_EQ(md->index(), md1->index());
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    // • checking by index retrieval
    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member_by_index(member, 0));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member_by_index(member, 1));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 1u);
    EXPECT_EQ(md->name(), md2->name());
    EXPECT_EQ(md->type(), md2->type());

    // • checking by name retrieval
    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member_by_name(member, "int32"));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member_by_name(member, "int64"));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 1u);
    EXPECT_EQ(md->name(), md2->name());
    EXPECT_EQ(md->type(), md2->type());

    // • checking map indexes retrieval
    //    + indexing by id
    DynamicTypeMembersById members_by_id;
    struct_type_builder->get_all_members(members_by_id);
    EXPECT_EQ(2, members_by_id.size());

    auto dm3 = members_by_id[3];
    ASSERT_TRUE(dm3);
    ASSERT_EQ(RETCODE_OK, dm3->get_descriptor(md));
    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    auto dm1 = members_by_id[1];
    ASSERT_TRUE(dm1);
    ASSERT_EQ(RETCODE_OK, dm1->get_descriptor(md));
    EXPECT_EQ(md->index(), 1u);
    EXPECT_EQ(md->name(), md2->name());
    EXPECT_EQ(md->type(), md2->type());

    //    + indexing by name
    DynamicTypeMembersByName members_by_name;
    struct_type_builder->get_all_members_by_name(members_by_name);
    EXPECT_EQ(2, members_by_name.size());

    dm3 = members_by_name["int32"];
    ASSERT_EQ(RETCODE_OK, dm3->get_descriptor(md));
    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    dm1 = members_by_name["int64"];
    ASSERT_EQ(RETCODE_OK, dm1->get_descriptor(md));
    EXPECT_EQ(md->index(), 1u);
    EXPECT_EQ(md->name(), md2->name());
    EXPECT_EQ(md->type(), md2->type());

    // • checking indexes work according with OMG standard 1.3 section 7.5.2.7.6
    md = traits<MemberDescriptor>::make_shared();
    md->id(7);
    md->name("bool");
    md->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->add_member(md));

    struct_type_builder->get_all_members(members_by_id);
    ASSERT_EQ(3, members_by_id.size());

    MemberDescriptor::_ref_type tmp = traits<MemberDescriptor>::make_shared();
    auto dm = members_by_id[3];
    ASSERT_EQ(RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_EQ(tmp->index(), 0u);
    EXPECT_EQ(tmp->name(), md1->name());
    EXPECT_EQ(tmp->type(), md1->type());

    dm = members_by_id[7];
    ASSERT_EQ(RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_EQ(tmp->name(), md->name());
    EXPECT_EQ(tmp->type(), md->type());

    dm = members_by_id[1];
    ASSERT_EQ(RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_EQ(tmp->index(), 1u);
    EXPECT_EQ(tmp->name(), md2->name());
    EXPECT_EQ(tmp->type(), md2->type());


    // • checking adding duplicates
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        //    + duplicate name
        md = traits<MemberDescriptor>::make_shared();
        md->id(1);
        md->name("int32");
        md->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
        EXPECT_NE(RETCODE_OK, struct_type_builder->add_member(md));

        //    + duplicate id
        md = traits<MemberDescriptor>::make_shared();
        md->id(7);
        md->name("dup_bool");
        md->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
        EXPECT_NE(RETCODE_OK, struct_type_builder->add_member(md));
    }
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Try to create with invalid values
    // • strings
    DynamicTypeBuilder::_ref_type created_builder {factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))};
    ASSERT_TRUE(created_builder);

    DynamicType::_ref_type type {created_builder->build()};
    ASSERT_TRUE(type);
    DynamicType::_ref_type type2 {created_builder->build()};
    ASSERT_TRUE(type2);

    ASSERT_TRUE(type->equals(type2));

    // • wstrings
    created_builder = factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED));
    ASSERT_TRUE(created_builder);

    type = created_builder->build();
    ASSERT_TRUE(type);
    type2 = created_builder->build();
    ASSERT_TRUE(type2);

    ASSERT_TRUE(type->equals(type2));
}

TEST_F(DynamicTypesTests, DynamicType_int32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test setters and getters.
    const int32_t test1 {123};
    int32_t test2 {0};
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_int32_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(100, test2);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 101), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(101, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 303), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(303, test2);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);

    uint32_t uTest32;
    ASSERT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    int16_t iTest16;
    ASSERT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    uint16_t uTest16;
    ASSERT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(iTest64, 1);
    uint64_t uTest64;
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest64, 1);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest128, 1);
    char cTest8;
    ASSERT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    wchar_t cTest16;
    ASSERT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    octet oTest;
    ASSERT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    bool bTest;
    ASSERT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    std::string sTest;
    ASSERT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    std::wstring wsTest;
    ASSERT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_uint32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint32_t test1 = 123;
    uint32_t test2 = 0;
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_uint32_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232u, test2);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 303), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(303u, test2);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1u, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(iTest64, 1);
    uint64_t uTest64;
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(uTest64, 1);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest64, 1);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest128, 1);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_int16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const int16_t test1 = 123;
    int16_t test2 = 0;
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_int16_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(100, test2);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232, test2);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest32);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest64);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest32);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest64);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int16_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int16_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_uint16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint16_t test1 = 123;
    uint16_t test2 = 0;
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_uint16_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232u, test2);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1u, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest32);
    uint32_t uTest32;
    ASSERT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, uTest32);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest64);
    uint64_t uTest64;
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, uTest64);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest32);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest64);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint16_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint16_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_int64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const int64_t test1 = 123;
    int64_t test2 = 0;
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_int64_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 3003), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(3003, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 2003), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2003, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 300), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(300, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(100, test2);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int64_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int64_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_uint64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint64_t test1 = 123;
    uint64_t test2 = 0;
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_uint64_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 3004), RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(3004u, test2);
    ASSERT_NE(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232u, test2);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(100u, test2);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1u, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint64_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint64_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_float32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const float test1 = 123.0f;
    float test2 = 0.0f;
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_float32_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(100, test2);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 200), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(200, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(10, test2);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest64);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        float test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        float test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_float64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const double test1 = 123.0;
    double test2 = 0.0;
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_float64_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, -100), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(-100, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 1000), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1000, test2);
    ASSERT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(100, test2);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(232, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, -10), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(-10, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(10, test2);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 11), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(11, test2);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        double test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        double test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_float128)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT128)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const long double test1 = 123.0;
    long double test2 = 0.0;
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_float128_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, -3000), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(-3000, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 3000), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(3000, test2);
    ASSERT_EQ(data->set_int8_value(MEMBER_ID_INVALID, -20), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(-20, test2);
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 200), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(200, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, -200), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(-200, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 200), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(200, test2);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, -2000), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(-2000, test2);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 2000), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2000, test2);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 20), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(20, test2);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 30), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(30, test2);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        long double test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float128_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        long double test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float128_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_char8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_CHAR8)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const char test1 = 'a';
    char test2 = 'b';
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_char8_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest32);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest16);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest64);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest32);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest64);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    wchar_t cTest16;
    ASSERT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, cTest16);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        char test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char8_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        char test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char8_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_char16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_CHAR16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const wchar_t test1 = L'a';
    wchar_t test2 = L'b';
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_char16_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(97, test2);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest32);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, iTest64);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest32);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest64);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        wchar_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        wchar_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_byte)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_BYTE)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const octet test1 {255};
    octet test2 {0};
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_byte_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(iTest32, test1);
    uint32_t uTest32;
    ASSERT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(uTest32, test1);
    int16_t iTest16;
    ASSERT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(iTest16, test1);
    uint16_t uTest16;
    ASSERT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(uTest16, test1);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(iTest64, test1);
    uint64_t uTest64;
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(uTest64, test1);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest32, test1);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest64, test1);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(fTest128, test1);
    char cTest8;
    ASSERT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(cTest8, static_cast<char>(test1));
    wchar_t cTest16;
    ASSERT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(cTest16, test1);
    bool bTest;
    ASSERT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(bTest, true);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        octet test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_byte_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        octet test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_byte_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_boolean)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const bool test1 = true;
    bool test2 = false;
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_NE(data->get_boolean_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(true, test2);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(false, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32 {1};
    ASSERT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, iTest32);
    uint32_t uTest32 {1};
    ASSERT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, uTest32);
    int16_t iTest16 {1};
    ASSERT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, iTest16);
    uint16_t uTest16 {1};
    ASSERT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, uTest16);
    int64_t iTest64 {1};
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, iTest64);
    uint64_t uTest64 {1};
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, uTest64);
    float fTest32 {1};
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, fTest32);
    double fTest64 {1};
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, fTest64);
    long double fTest128 {1};
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        bool test3 {false};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_boolean_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        bool test3 {false};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_boolean_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_enum)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ENUM);
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    DynamicType::_ref_type created_type {builder->build()};
    // Enumerator without literals are invalid.
    ASSERT_FALSE(created_type);

    // Add three members to the enum.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("DEFAULT");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("FIRST");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("SECOND");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Try to add a descriptor with the same name.
        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
        member_descriptor->name("THIRD");
        EXPECT_NE(builder->add_member(member_descriptor), RETCODE_OK);
    }

    created_type = builder->build();
    ASSERT_TRUE(created_type);

    ASSERT_EQ(3, created_type->get_member_count());
    DynamicTypeMember::_ref_type member;
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_index(member, 0));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("DEFAULT", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_index(member, 1));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("FIRST", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_index(member, 2));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("SECOND", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_name(member, "DEFAULT"));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("DEFAULT", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_name(member, "FIRST"));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("FIRST", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_name(member, "SECOND"));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("SECOND", member->get_name());

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("DEFAULT"));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test getters and setters.
    const uint32_t test1 {2};
    uint32_t test2 {0};
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2u, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(1u, test2);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 2), RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2u, test2);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32 {0};
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    int16_t iTest16 {0};
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    uint16_t uTest16 {0};
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    int64_t iTest64 {0};
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2, iTest64);
    uint64_t uTest64 {0};
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2, uTest64);
    float fTest32 {0};
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    double fTest64 {0};
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2, fTest64);
    long double fTest128 {0};
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(2, fTest128);
    char cTest8 {0};
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    wchar_t cTest16 {0};
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    octet oTest {0};
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    bool bTest {false};
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0u, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_string)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type builder {factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    const uint32_t length = 15;
    builder = factory->create_string_type(length);
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    created_type = builder->build();
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test getters and setters.
    ASSERT_NE(data->set_string_value(1, ""), RETCODE_OK);
    const std::string test1 {"STRING_TEST"};
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);

    std::string test2;
    ASSERT_EQ(data->get_string_value(test2, 0), RETCODE_OK);
    ASSERT_EQ("S", test2);
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID,
                "TEST_OVER_LENGTH_LIMITS"), RETCODE_OK);
    }

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(iTest32, 1), RETCODE_OK);
    ASSERT_EQ(84, iTest32);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint32_value(uTest32, 1), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(iTest16, 1), RETCODE_OK);
    ASSERT_EQ(84, iTest16);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint16_value(uTest16, 1), RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(iTest64, 1), RETCODE_OK);
    ASSERT_EQ(84, iTest64);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint64_value(uTest64, 1), RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(fTest32, 1), RETCODE_OK);
    ASSERT_EQ(84, fTest32);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(fTest64, 1), RETCODE_OK);
    ASSERT_EQ(84, fTest64);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(fTest128, 1), RETCODE_OK);
    ASSERT_EQ(84, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_char8_value(cTest8, 1), RETCODE_OK);
    ASSERT_EQ('T', cTest8);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_char16_value(cTest16, 1), RETCODE_OK);
    ASSERT_EQ(L'T', cTest16);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_byte_value(oTest, 1), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_boolean_value(bTest, 1), RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_wstring_value(wsTest, 1), RETCODE_OK);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(10, data->get_member_id_at_index(10));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(11));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(test1.length(), data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        std::string test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        std::string test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ("", test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_wstring)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type builder {factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    const uint32_t length = 15;
    builder = factory->create_wstring_type(length);
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    created_type = builder->build();
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    ASSERT_NE(data->set_wstring_value(1, L""), RETCODE_OK);
    const std::wstring test1 = L"STRING_TEST";
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), RETCODE_OK);

    std::wstring test2;
    ASSERT_EQ(data->get_wstring_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(L"S", test2);
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID,
                L"TEST_OVER_LENGTH_LIMITS"), RETCODE_OK);
    }

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(iTest32, 1), RETCODE_OK);
    ASSERT_EQ(84, iTest32);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint32_value(uTest32, 1), RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_int16_value(iTest16, 1), RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint16_value(uTest16, 1), RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(iTest64, 1), RETCODE_OK);
    ASSERT_EQ(84, iTest64);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint64_value(uTest64, 1), RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(fTest32, 1), RETCODE_OK);
    ASSERT_EQ(84, fTest32);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(fTest64, 1), RETCODE_OK);
    ASSERT_EQ(84, fTest64);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(fTest128, 1), RETCODE_OK);
    ASSERT_EQ(84, fTest128);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_char8_value(cTest8, 1), RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(data->get_char16_value(cTest16, 1), RETCODE_OK);
    ASSERT_EQ(L'T', cTest16);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_byte_value(oTest, 1), RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_boolean_value(bTest, 1), RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_string_value(sTest, 1), RETCODE_OK);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(10, data->get_member_id_at_index(10));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(11));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(test1.length(), data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        std::wstring test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_wstring_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        std::wstring test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_wstring_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(L"", test2);
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(L"", test2);
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(L"", test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_alias)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const ObjectName name = "ALIAS";

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name(name);
    type_descriptor->base_type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    EXPECT_EQ(created_type->get_name(), name);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test getters and setters.
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_NE(data->set_string_value(1, ""), RETCODE_OK);

    const uint32_t test1 = 2;
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);

    uint32_t test2 = 0;
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_nested_alias)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t length = 15;
    const ObjectName name = "ALIAS";
    const ObjectName nested_name = "NESTED_ALIAS";

    DynamicTypeBuilder::_ref_type builder {factory->create_string_type(length)};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type string_type {builder->build()};
    ASSERT_TRUE(string_type);

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name(name);
    type_descriptor->base_type(string_type);
    builder = factory->create_type(type_descriptor);
    DynamicType::_ref_type alias_type {builder->build()};
    ASSERT_TRUE(alias_type);
    EXPECT_EQ(alias_type->get_name(), name);
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name(nested_name);
    type_descriptor->base_type(alias_type);
    builder = factory->create_type(type_descriptor);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    DynamicType::_ref_type nested_alias_type {builder->build()};
    ASSERT_TRUE(nested_alias_type);
    EXPECT_EQ(nested_alias_type->get_name(), nested_name);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test getters and setters.
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_NE(data->set_string_value(1, ""), RETCODE_OK);
    const std::string test1 {"STRING_TEST"};
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);

    int test = 0;
    ASSERT_NE(data->get_int32_value(test, MEMBER_ID_INVALID), RETCODE_OK);
    std::string test2;
    ASSERT_EQ(data->get_string_value(test2, 0), RETCODE_OK);
    ASSERT_EQ("S", test2);
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID,
                "TEST_OVER_LENGTH_LIMITS"), RETCODE_OK);
    }

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(test1.length(), data->get_item_count());

    // XCDRv1
    // Encoding/decoding
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(nested_alias_type);
        std::string test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(nested_alias_type);
        std::string test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ("", test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_bitset)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITSET);
    type_descriptor->name("MyBitset");
    type_descriptor->bound({2, 20});
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_FALSE(created_type);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->id(0);
    member_descriptor->name("int2");
    member_descriptor->type(factory->get_primitive_type(TK_UINT8));
    ASSERT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    ASSERT_FALSE(builder->build());

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->id(1);
    member_descriptor->name("int20");
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    ASSERT_NE(RETCODE_OK, builder->add_member(member_descriptor));

    member_descriptor->id(60);
    ASSERT_NE(RETCODE_OK, builder->add_member(member_descriptor));

    member_descriptor->id(3);
    ASSERT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    created_type = builder->build();
    ASSERT_TRUE(created_type);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->id(25);
    member_descriptor->name("int3");
    member_descriptor->type(factory->get_primitive_type(TK_UINT8));
    ASSERT_NE(RETCODE_OK, builder->add_member(member_descriptor));

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Testing get_member_by_name and get_member_id_at_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("int3"));
    ASSERT_EQ(0, data->get_member_id_by_name("int2"));
    ASSERT_EQ(3, data->get_member_id_by_name("int20"));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(3, data->get_member_id_at_index(1));

    const uint8_t set_test_field_1 {6};
    ASSERT_NE(data->set_uint8_value(MEMBER_ID_INVALID, set_test_field_1), RETCODE_OK);
    ASSERT_NE(data->set_uint8_value(1, set_test_field_1), RETCODE_OK);
    ASSERT_EQ(data->set_uint8_value(0, set_test_field_1), RETCODE_OK);

    const int32_t set_test_field_2 {6};
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, set_test_field_2), RETCODE_OK);
    ASSERT_NE(data->set_int32_value(5, set_test_field_2), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(3, set_test_field_2), RETCODE_OK);

    uint8_t get_test_field_1 {0};
    ASSERT_NE(data->get_uint8_value(get_test_field_1, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_uint8_value(get_test_field_1, 1), RETCODE_OK);
    ASSERT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    ASSERT_EQ(set_test_field_1 & 0x3, get_test_field_1);

    int32_t get_test_field_2 {0};
    ASSERT_NE(data->get_int32_value(get_test_field_2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_NE(data->get_int32_value(get_test_field_2, 5), RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    ASSERT_EQ(set_test_field_2, get_test_field_2);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
    ASSERT_EQ(complex_data->get_uint8_value(get_test_field_1, MEMBER_ID_INVALID), RETCODE_OK);

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

    // Testing loan_value.
    ASSERT_FALSE(data->loan_value(0));

    // Testing get_item_count.
    ASSERT_EQ(2u, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
        ASSERT_EQ(set_test_field_1 & 0x3, get_test_field_1);
        ASSERT_EQ(data2->get_int32_value(get_test_field_2, 3), RETCODE_OK);
        ASSERT_EQ(set_test_field_2, get_test_field_2);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
        ASSERT_EQ(set_test_field_1 & 0x3, get_test_field_1);
        ASSERT_EQ(data2->get_int32_value(get_test_field_2, 3), RETCODE_OK);
        ASSERT_EQ(set_test_field_2, get_test_field_2);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    ASSERT_EQ(0u, get_test_field_1);
    ASSERT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    ASSERT_EQ(0, get_test_field_2);

    ASSERT_EQ(data->set_uint8_value(0, set_test_field_1), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(3, set_test_field_2), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    ASSERT_EQ(0u, get_test_field_1);
    ASSERT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    ASSERT_EQ(0, get_test_field_2);

    ASSERT_EQ(data->set_uint8_value(0, set_test_field_1), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(3, set_test_field_2), RETCODE_OK);
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(2));
    ASSERT_EQ(RETCODE_OK, data->clear_value(3));
    ASSERT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    ASSERT_EQ(set_test_field_1 & 0x3, get_test_field_1);
    ASSERT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    ASSERT_EQ(0, get_test_field_2);
}

TEST_F(DynamicTypesTests, DynamicType_bitmask)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t limit = 6;

    DynamicTypeBuilder::_ref_type builder {factory->create_bitmask_type(0)};
    ASSERT_FALSE(builder);
    builder = factory->create_bitmask_type(65);
    ASSERT_FALSE(builder);
    builder = factory->create_bitmask_type(limit);
    ASSERT_TRUE(builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("BIT0");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    member_descriptor->name("BIT1");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    member_descriptor->name("BIT2");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    member_descriptor->name("BIT3");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    member_descriptor->name("BIT5");
    member_descriptor->id(5);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    member_descriptor->name("BIT6");
    member_descriptor->id(6);
    ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    member_descriptor->name("BIT0");
    ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Testing get_member_by_name and get_member_id_at_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("BIT4"));
    ASSERT_EQ(0, data->get_member_id_by_name("BIT0"));
    ASSERT_EQ(1, data->get_member_id_by_name("BIT1"));
    ASSERT_EQ(2, data->get_member_id_by_name("BIT2"));
    ASSERT_EQ(3, data->get_member_id_by_name("BIT3"));
    ASSERT_EQ(5, data->get_member_id_by_name("BIT5"));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(2, data->get_member_id_at_index(2));
    ASSERT_EQ(3, data->get_member_id_at_index(3));
    ASSERT_EQ(5, data->get_member_id_at_index(4));

    // Testing get_item_count.
    ASSERT_EQ(0, data->get_item_count());

    // Testing getters and setters.
    const uint8_t bitmask_value_set = 0x1;
    ASSERT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, bitmask_value_set), RETCODE_OK);

    ASSERT_NE(data->set_boolean_value(4, true), RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(6, true), RETCODE_OK);

    ASSERT_EQ(data->set_boolean_value(2, true), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(5, true), RETCODE_OK);

    bool bit_get {false};
    ASSERT_EQ(data->get_boolean_value(bit_get, 0), RETCODE_OK);
    ASSERT_EQ(true, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 1), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 2), RETCODE_OK);
    ASSERT_EQ(true, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 3), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_NE(data->get_boolean_value(bit_get, 4), RETCODE_OK);
    ASSERT_EQ(data->get_boolean_value(bit_get, 5), RETCODE_OK);
    ASSERT_EQ(true, bit_get);

    uint8_t bitmask_value_get {0};
    ASSERT_EQ(data->get_uint8_value(bitmask_value_get, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(bitmask_value_get, 0x25);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Testing loan_value.
    ASSERT_FALSE(data->loan_value(0));

    // Testing get_item_count.
    ASSERT_EQ(3, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_boolean_value(bit_get, 0), RETCODE_OK);
        ASSERT_EQ(true, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 1), RETCODE_OK);
        ASSERT_EQ(false, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 2), RETCODE_OK);
        ASSERT_EQ(true, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 3), RETCODE_OK);
        ASSERT_EQ(false, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 5), RETCODE_OK);
        ASSERT_EQ(true, bit_get);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_boolean_value(bit_get, 0), RETCODE_OK);
        ASSERT_EQ(true, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 1), RETCODE_OK);
        ASSERT_EQ(false, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 2), RETCODE_OK);
        ASSERT_EQ(true, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 3), RETCODE_OK);
        ASSERT_EQ(false, bit_get);
        ASSERT_EQ(data2->get_boolean_value(bit_get, 5), RETCODE_OK);
        ASSERT_EQ(true, bit_get);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_boolean_value(bit_get, 0), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 1), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 2), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 3), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 5), RETCODE_OK);
    ASSERT_EQ(false, bit_get);

    ASSERT_EQ(data->set_boolean_value(0, true), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(2, true), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(5, true), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_boolean_value(bit_get, 0), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 1), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 2), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 3), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 5), RETCODE_OK);
    ASSERT_EQ(false, bit_get);

    ASSERT_EQ(data->set_boolean_value(0, true), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(2, true), RETCODE_OK);
    ASSERT_EQ(data->set_boolean_value(5, true), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->clear_value(1));
    ASSERT_EQ(RETCODE_OK, data->clear_value(2));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(data->get_boolean_value(bit_get, 0), RETCODE_OK);
    ASSERT_EQ(true, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 1), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 2), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 3), RETCODE_OK);
    ASSERT_EQ(false, bit_get);
    ASSERT_EQ(data->get_boolean_value(bit_get, 5), RETCODE_OK);
    ASSERT_EQ(true, bit_get);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_sequence)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(
                                               factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32),
                                               length)};
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    // Try to write on an empty position
    ASSERT_NE(data->set_int32_value(234, 1), RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2, 3, 4}), RETCODE_OK);
    ASSERT_EQ(data->set_int32_values(0, {1, 2, 3, 4}), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(4, 5), RETCODE_OK);

    // Try to insert more than the limit.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        ASSERT_NE(data->set_int32_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_OK);
    }

    int32_t test1 {0};
    ASSERT_EQ(data->get_int32_value(test1, 0), RETCODE_OK);
    ASSERT_EQ(1, test1);
    ASSERT_EQ(data->get_int32_value(test1, 1), RETCODE_OK);
    ASSERT_EQ(2, test1);
    ASSERT_EQ(data->get_int32_value(test1, 2), RETCODE_OK);
    ASSERT_EQ(3, test1);
    ASSERT_EQ(data->get_int32_value(test1, 3), RETCODE_OK);
    ASSERT_EQ(4, test1);
    ASSERT_EQ(data->get_int32_value(test1, 4), RETCODE_OK);
    ASSERT_EQ(5, test1);

    Int32Seq test2;
    ASSERT_EQ(data->get_int32_values(test2, 0), RETCODE_OK);
    Int32Seq test_all {{1, 2, 3, 4, 5}};
    ASSERT_EQ(test2, test_all);

    ASSERT_EQ(data->get_int32_values(test2, 2), RETCODE_OK);
    Int32Seq test_less {{3, 4, 5}};
    ASSERT_EQ(test2, test_less);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(4, data->get_member_id_at_index(4));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(length, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), RETCODE_OK);
        ASSERT_EQ(test_all, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), RETCODE_OK);
        ASSERT_EQ(test_all, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(0, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3}));
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(0, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3}));
    ASSERT_EQ(RETCODE_OK, data->clear_value(1));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(2, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({1, 3}));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_of_sequences)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t sequence_length = 2;
    const uint32_t inner_sequence_length = 3;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(
                                               factory->get_primitive_type(
                                                   eprosima::fastdds::dds::TK_INT32), inner_sequence_length)};
    ASSERT_TRUE(builder);
    builder = factory->create_sequence_type(builder->build(), sequence_length);
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters, setters and loan_value.
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(data->set_int32_values(0, {1, 2}), RETCODE_OK);
    Int32Seq good_seq;
    ASSERT_EQ(data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));

    auto seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2, 3}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Try to insert more than the limit.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        seq_data = data->loan_value(2);
        ASSERT_FALSE(seq_data);
    }

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    UInt32Seq wrong_seq;
    ASSERT_NE(seq_data->get_uint32_values(wrong_seq, 0), RETCODE_OK);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2, 3}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(2));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2, 3}));

    // Test set_complex_value
    good_seq = {2, 3, 4};
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(complex_data->set_int32_values(0, good_seq), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

    // Test get_item_count().
    ASSERT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data2->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({2, 3, 4}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data2->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({2, 3, 4}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(0u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(0u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    ASSERT_EQ(RETCODE_OK, data->clear_value(0));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(1u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_array)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    BoundSeq array_dimensions {{ 2, 2, 2 }};

    DynamicTypeBuilder::_ref_type builder {factory->create_array_type(
                                               factory->get_primitive_type(
                                                   eprosima::fastdds::dds::TK_INT32), array_dimensions)};
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(7, data->get_member_id_at_index(7));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(8));

    // Test getters and setters.
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    // Try to write on an empty position
    ASSERT_NE(data->set_int32_value(234, 1), RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2, 3, 4}), RETCODE_OK);
    ASSERT_EQ(data->set_int32_values(0, {1, 2, 3, 4}), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(4, 5), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(5, 6), RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(6, 7), RETCODE_OK);

    // Try to insert more than the limit.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        ASSERT_NE(data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8, 9}), RETCODE_OK);
    }

    int32_t test1 {0};
    ASSERT_EQ(data->get_int32_value(test1, 0), RETCODE_OK);
    ASSERT_EQ(1, test1);
    ASSERT_EQ(data->get_int32_value(test1, 1), RETCODE_OK);
    ASSERT_EQ(2, test1);
    ASSERT_EQ(data->get_int32_value(test1, 2), RETCODE_OK);
    ASSERT_EQ(3, test1);
    ASSERT_EQ(data->get_int32_value(test1, 3), RETCODE_OK);
    ASSERT_EQ(4, test1);
    ASSERT_EQ(data->get_int32_value(test1, 4), RETCODE_OK);
    ASSERT_EQ(5, test1);
    ASSERT_EQ(data->get_int32_value(test1, 5), RETCODE_OK);
    ASSERT_EQ(6, test1);
    ASSERT_EQ(data->get_int32_value(test1, 6), RETCODE_OK);
    ASSERT_EQ(7, test1);
    ASSERT_EQ(data->get_int32_value(test1, 7), RETCODE_OK);
    ASSERT_EQ(0, test1);

    Int32Seq test2;
    ASSERT_EQ(data->get_int32_values(test2, 0), RETCODE_OK);
    Int32Seq test_all {{1, 2, 3, 4, 5, 6, 7, 0}};
    ASSERT_EQ(test2, test_all);

    ASSERT_EQ(data->get_int32_values(test2, 2), RETCODE_OK);
    Int32Seq test_less {{3, 4, 5, 6, 7, 0}};
    ASSERT_EQ(test2, test_less);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Test loan_value
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(8u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), RETCODE_OK);
        ASSERT_EQ(test_all, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), RETCODE_OK);
        ASSERT_EQ(test_all, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(8u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({0, 0, 0, 0, 0, 0, 0, 0}));
    ASSERT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8}));
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(8u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({0, 0, 0, 0, 0, 0, 0, 0}));
    ASSERT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8}));
    ASSERT_EQ(RETCODE_OK, data->clear_value(1));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(8u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({1, 0, 3, 4, 5, 6, 7, 8}));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_array_of_arrays)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    BoundSeq array_dimensions {{ 2, 2 }};
    BoundSeq inner_array_dimensions {{ 2 }};

    DynamicTypeBuilder::_ref_type builder {factory->create_array_type(
                                               factory->get_primitive_type(
                                                   eprosima::fastdds::dds::TK_INT32), inner_array_dimensions)};

    ASSERT_TRUE(builder);
    builder = factory->create_array_type(builder->build(), array_dimensions);
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(0, data->get_member_id_at_index(0));
    ASSERT_EQ(1, data->get_member_id_at_index(1));
    ASSERT_EQ(2, data->get_member_id_at_index(2));
    ASSERT_EQ(3, data->get_member_id_at_index(3));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(4));

    // Test getters, setters and loan_value.
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(data->set_int32_values(0, {1, 2}), RETCODE_OK);
    Int32Seq good_seq;
    ASSERT_EQ(data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));

    auto seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Try to insert more than the limit.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        seq_data = data->loan_value(4);
        ASSERT_FALSE(seq_data);
    }

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    UInt32Seq wrong_seq;
    ASSERT_NE(seq_data->get_uint32_values(wrong_seq, 0), RETCODE_OK);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({3, 4}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));

    // Test set_complex_value
    good_seq = {2, 3};
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(complex_data->set_int32_values(0, good_seq), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

    // Test get_item_count().
    ASSERT_EQ(4u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data2->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({2, 3}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(2);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({3, 4}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(3);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 0}));
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({2, 3}));
        ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(2);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({3, 4}));
        ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(3);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 0}));
        ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    ASSERT_EQ(RETCODE_OK, data->clear_value(2));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    ASSERT_EQ(RETCODE_OK, data->clear_value(3));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({3, 4}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_map)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t map_length {2};

    DynamicTypeBuilder::_ref_type builder {factory->create_map_type(
                                               factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32),
                                               factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32),
                                               map_length)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);

    // Then
    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(1));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(2));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("string"));
    ASSERT_EQ(0, data->get_member_id_by_name("10"));
    ASSERT_EQ(1, data->get_member_id_by_name("20"));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("30"));

    // Testing getters and setters.
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);

    // Try to write on an empty position
    ASSERT_NE(data->set_int32_value(2, 10), RETCODE_OK);

    // Set and get a value.
    int32_t test1 {234};
    ASSERT_EQ(data->set_int32_value(0, test1), RETCODE_OK);
    int32_t test2 {0};
    ASSERT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(test1, test2);
    int32_t test3 {132};
    ASSERT_EQ(data->set_int32_value(1, test3), RETCODE_OK);
    int32_t test4 {0};
    ASSERT_EQ(data->get_int32_value(test4, 1), RETCODE_OK);
    ASSERT_EQ(test3, test4);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

    // Testing loan_value.
    ASSERT_FALSE(data->loan_value(1));
    ASSERT_FALSE(data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count().
    ASSERT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_value(test2, data->get_member_id_by_name(
                    "10")), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        ASSERT_EQ(data2->get_int32_value(test4, data->get_member_id_by_name(
                    "20")), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_value(test2, data->get_member_id_by_name(
                    "10")), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        ASSERT_EQ(data2->get_int32_value(test4, data->get_member_id_by_name(
                    "20")), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(0u, data->get_item_count());
    ASSERT_NE(RETCODE_OK, data->get_int32_value(test2, 0));
    ASSERT_NE(RETCODE_OK, data->get_int32_value(test4, 1));

    ASSERT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("10"), test1));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("20"), test3));
    ASSERT_EQ(2u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(0u, data->get_item_count());
    ASSERT_NE(RETCODE_OK, data->get_int32_value(test2, 0));
    ASSERT_NE(RETCODE_OK, data->get_int32_value(test4, 1));

    ASSERT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("10"), test1));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("20"), test3));
    ASSERT_EQ(2u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->clear_value(0));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(1u, data->get_item_count());
    ASSERT_NE(RETCODE_OK, data->get_int32_value(test2, 0));
    ASSERT_EQ(RETCODE_OK, data->get_int32_value(test4, 1));
    ASSERT_EQ(test3, test4);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_map_of_maps)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t map_length {2};
    const uint32_t inner_map_length {3};

    DynamicTypeBuilder::_ref_type builder {factory->create_map_type(
                                               factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32),
                                               factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32),
                                               inner_map_length)};

    DynamicType::_ref_type inner_type = builder->build();
    ASSERT_TRUE(inner_type);

    builder = factory->create_map_type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32),
                    inner_type, map_length);

    DynamicType::_ref_type created_type = builder->build();
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(1));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(2));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("string"));
    ASSERT_EQ(0, data->get_member_id_by_name("10"));
    ASSERT_EQ(1, data->get_member_id_by_name("20"));
    ASSERT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("30"));

    // Testing getters and setters.
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);
    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_OK);

    ASSERT_NE(data->set_int32_value(0, 10), RETCODE_OK);
    ASSERT_NE(data->set_int32_value(1, 10), RETCODE_OK);
    ASSERT_NE(data->set_int32_value(2, 10), RETCODE_OK);

    // Testing getters, setters, loan_value.
    DynamicData::_ref_type loan_data {data->loan_value(0)};
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(0u, loan_data->get_item_count());
    int32_t test1 {234};
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("3"), test1));
    int32_t test2 {123};
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("4"), test2));
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(1);
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(0u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("1"), test2));
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("2"), test1));
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));

    int32_t test_get {0};
    loan_data = data->loan_value(0);
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("3")));
    ASSERT_EQ(test1, test_get);
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("4")));
    ASSERT_EQ(test2, test_get);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(1);
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("1")));
    ASSERT_EQ(test2, test_get);
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("2")));
    ASSERT_EQ(test1, test_get);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(2u, complex_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            complex_data->get_int32_value(test_get, complex_data->get_member_id_by_name("3")));
    ASSERT_EQ(test1, test_get);
    ASSERT_EQ(RETCODE_OK,
            complex_data->get_int32_value(test_get, complex_data->get_member_id_by_name("4")));
    ASSERT_EQ(test2, test_get);


    // Test set_complex_value
    int32_t test3 {456};
    int32_t test4 {345};
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_OK,
            complex_data->set_int32_value(complex_data->get_member_id_by_name("3"), test3));
    ASSERT_EQ(RETCODE_OK,
            complex_data->set_int32_value(complex_data->get_member_id_by_name("4"), test4));
    ASSERT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

    // Test get_item_count().
    ASSERT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(2u, data2->get_item_count());
        loan_data = data2->loan_value(0);
        ASSERT_TRUE(loan_data);
        ASSERT_EQ(2u, loan_data->get_item_count());
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("3")));
        ASSERT_EQ(test3, test_get);
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("4")));
        ASSERT_EQ(test4, test_get);
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(loan_data));
        loan_data = data2->loan_value(1);
        ASSERT_TRUE(loan_data);
        ASSERT_EQ(2u, loan_data->get_item_count());
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("1")));
        ASSERT_EQ(test2, test_get);
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("2")));
        ASSERT_EQ(test1, test_get);
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(loan_data));
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(2u, data2->get_item_count());
        loan_data = data2->loan_value(0);
        ASSERT_TRUE(loan_data);
        ASSERT_EQ(2u, loan_data->get_item_count());
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("3")));
        ASSERT_EQ(test3, test_get);
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("4")));
        ASSERT_EQ(test4, test_get);
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(loan_data));
        loan_data = data2->loan_value(1);
        ASSERT_TRUE(loan_data);
        ASSERT_EQ(2u, loan_data->get_item_count());
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("1")));
        ASSERT_EQ(test2, test_get);
        ASSERT_EQ(RETCODE_OK,
                loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("2")));
        ASSERT_EQ(test1, test_get);
        ASSERT_EQ(RETCODE_OK, data2->return_loaned_value(loan_data));
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(0u, data->get_item_count());
    ASSERT_FALSE(data->loan_value(0));
    ASSERT_FALSE(data->loan_value(1));

    loan_data = data->loan_value(data->get_member_id_by_name("10"));
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(0u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("3"), test1));
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("4"), test2));
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(data->get_member_id_by_name("20"));
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(0u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("1"), test2));
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("2"), test1));
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    ASSERT_EQ(2u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(0u, data->get_item_count());
    ASSERT_FALSE(data->loan_value(0));
    ASSERT_FALSE(data->loan_value(1));

    loan_data = data->loan_value(data->get_member_id_by_name("10"));
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(0u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("3"), test1));
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("4"), test2));
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(data->get_member_id_by_name("20"));
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(0u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("1"), test2));
    ASSERT_EQ(RETCODE_OK,
            loan_data->set_int32_value(loan_data->get_member_id_by_name("2"), test1));
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    ASSERT_EQ(2u, data->get_item_count());
    ASSERT_EQ(RETCODE_OK, data->clear_value(0));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    ASSERT_EQ(1u, data->get_item_count());
    ASSERT_FALSE(data->loan_value(0));
    loan_data = data->loan_value(1);
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(2u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("1")));
    ASSERT_EQ(test2, test_get);
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_int32_value(test_get, loan_data->get_member_id_by_name("2")));
    ASSERT_EQ(test1, test_get);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_structure)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructTest");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("int32");
    member_descriptor->id(0);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->id(1);
    member_descriptor->default_value("3");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type struct_data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(struct_data);

    ASSERT_NE(struct_data->set_int64_value(0, 10), RETCODE_OK);
    ASSERT_NE(struct_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);


    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_by_name(""));
    ASSERT_EQ(0, struct_data->get_member_id_by_name("int32"));
    ASSERT_EQ(1, struct_data->get_member_id_by_name("int64"));
    ASSERT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_at_index(2));
    ASSERT_EQ(0, struct_data->get_member_id_at_index(0));
    ASSERT_EQ(1, struct_data->get_member_id_at_index(1));

    // Test getters and setters.
    int32_t test1 {234};
    ASSERT_EQ(struct_data->set_int32_value(0, test1), RETCODE_OK);
    uint32_t wrong {0};
    ASSERT_NE(struct_data->get_uint32_value(wrong, 0), RETCODE_OK);
    int32_t test2 {0};
    ASSERT_EQ(struct_data->get_int32_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(test1, test2);
    int64_t test3 {234};
    ASSERT_EQ(struct_data->set_int64_value(1, test3), RETCODE_OK);
    int64_t test4 {0};
    ASSERT_EQ(struct_data->get_int64_value(test4, 1), RETCODE_OK);
    ASSERT_EQ(test3, test4);

    // Test clone.
    auto clone = struct_data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(struct_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, struct_data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    // Test set_complex_value
    test1 = 456;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_OK,
            complex_data->set_int32_value(MEMBER_ID_INVALID, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_complex_value(0, complex_data));

    // Test loan_value
    DynamicData::_ref_type loan_data = struct_data->loan_value(1);
    ASSERT_TRUE(loan_data);
    ASSERT_FALSE(struct_data->loan_value(1));
    ASSERT_EQ(loan_data->get_int64_value(test4, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test3, test4);
    ASSERT_EQ(struct_data->return_loaned_value(loan_data), RETCODE_OK);
    ASSERT_FALSE(struct_data->loan_value(MEMBER_ID_INVALID));

    // Test get_item_count.
    ASSERT_EQ(2u, struct_data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(struct_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&struct_data,
                XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&struct_data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(struct_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(struct_data));
        ASSERT_EQ(data2->get_int32_value(test2, 0), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        ASSERT_EQ(data2->get_int64_value(test4, 1), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(struct_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&struct_data,
                XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&struct_data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(struct_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(struct_data));
        ASSERT_EQ(data2->get_int32_value(test2, 0), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        ASSERT_EQ(data2->get_int64_value(test4, 1), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, struct_data->clear_all_values());
    ASSERT_EQ(2u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    ASSERT_EQ(3, test4);

    ASSERT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_nonkey_values());
    ASSERT_EQ(2u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    ASSERT_EQ(3, test4);

    ASSERT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_value(1));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(100));
    ASSERT_EQ(2u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    ASSERT_EQ(test1, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    ASSERT_EQ(3, test4);

    DynamicDataFactory::get_instance()->delete_data(struct_data);
}

TEST_F(DynamicTypesTests, DynamicType_structure_inheritance)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create the base struct.
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("BaseStructTest");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("int32");
    member_descriptor->id(0);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->id(1);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type base_struct_type {builder->build()};
    ASSERT_TRUE(base_struct_type);

    // Create the derived struct.
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("DerivedStructTest");
    type_descriptor->base_type(base_struct_type);
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");     // avoid expected errors logging
        member_descriptor->name("child_int32");
        member_descriptor->id(1);
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);
        member_descriptor->name("int32");
        member_descriptor->id(2);
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);
    }

    member_descriptor->name("child_int32");
    member_descriptor->id(2);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("child_string");
    member_descriptor->id(4);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type derived_struct_type {builder->build()};
    ASSERT_TRUE(derived_struct_type);

    ASSERT_EQ(4u, derived_struct_type->get_member_count());

    DynamicTypeMember::_ref_type member;
    DynamicTypeMember::_ref_type member_aux;
    MemberDescriptor::_ref_type descriptor = traits<MemberDescriptor>::make_shared();

    ASSERT_EQ(derived_struct_type->get_member(member, 0), RETCODE_OK);
    ASSERT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    ASSERT_EQ(descriptor->name(), "int32");
    ASSERT_EQ(descriptor->index(), 0u);
    ASSERT_EQ(descriptor->id(), 0);
    ASSERT_TRUE(descriptor->type()->equals(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32)));
    ASSERT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));
    ASSERT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));

    ASSERT_EQ(derived_struct_type->get_member(member, 1), RETCODE_OK);
    ASSERT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    ASSERT_EQ(descriptor->name(), "int64");
    ASSERT_EQ(descriptor->index(), 1u);
    ASSERT_EQ(descriptor->id(), 1);
    ASSERT_TRUE(descriptor->type()->equals(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64)));
    ASSERT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));
    ASSERT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));

    ASSERT_EQ(derived_struct_type->get_member(member, 2), RETCODE_OK);
    ASSERT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    ASSERT_EQ(descriptor->name(), "child_int32");
    ASSERT_EQ(descriptor->index(), 2u);
    ASSERT_EQ(descriptor->id(), 2);
    ASSERT_TRUE(descriptor->type()->equals(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32)));
    ASSERT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));
    ASSERT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));

    ASSERT_EQ(derived_struct_type->get_member(member, 4), RETCODE_OK);
    ASSERT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    ASSERT_EQ(descriptor->name(), "child_string");
    ASSERT_EQ(descriptor->index(), 3u);
    ASSERT_EQ(descriptor->id(), 4);
    ASSERT_TRUE(descriptor->type()->equals(factory->create_string_type(
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build()));
    ASSERT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));
    ASSERT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    ASSERT_TRUE(member->equals(member_aux));


    DynamicTypeMembersById members_by_id;
    ASSERT_EQ(derived_struct_type->get_all_members(members_by_id), RETCODE_OK);
    ASSERT_EQ(members_by_id.size(), 4u);

    DynamicTypeMembersByName members_by_name;
    ASSERT_EQ(derived_struct_type->get_all_members_by_name(members_by_name), RETCODE_OK);
    ASSERT_EQ(members_by_name.size(), 4u);


    // Validating data management
    DynamicData::_ref_type struct_data {DynamicDataFactory::get_instance()->create_data(derived_struct_type)};
    ASSERT_TRUE(derived_struct_type);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_by_name(""));
    ASSERT_EQ(0, struct_data->get_member_id_by_name("int32"));
    ASSERT_EQ(1, struct_data->get_member_id_by_name("int64"));
    ASSERT_EQ(2, struct_data->get_member_id_by_name("child_int32"));
    ASSERT_EQ(4, struct_data->get_member_id_by_name("child_string"));
    ASSERT_EQ(0, struct_data->get_member_id_at_index(0));
    ASSERT_EQ(1, struct_data->get_member_id_at_index(1));
    ASSERT_EQ(2, struct_data->get_member_id_at_index(2));
    ASSERT_EQ(4, struct_data->get_member_id_at_index(3));
    ASSERT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_at_index(4));

    // Testing getters and setters.
    // Setting invalid types should fail
    ASSERT_NE(struct_data->set_int64_value(0, 10), RETCODE_OK);
    ASSERT_NE(struct_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);

    // Set and get the parent values.
    int32_t test1 {234};
    ASSERT_EQ(struct_data->set_int32_value(0, test1), RETCODE_OK);
    int32_t test2 {0};
    ASSERT_EQ(struct_data->get_int32_value(test2, 0), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    int64_t test3 {234};
    ASSERT_EQ(struct_data->set_int64_value(1, test3), RETCODE_OK);
    int64_t test4 {0};
    ASSERT_EQ(struct_data->get_int64_value(test4, 1), RETCODE_OK);
    ASSERT_EQ(test3, test4);

    // Set and get the child value.
    ASSERT_EQ(struct_data->set_int32_value(2, test1), RETCODE_OK);
    ASSERT_EQ(struct_data->get_int32_value(test2, 2), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    std::string test5 {"Testing"};
    ASSERT_EQ(struct_data->set_string_value(4, test5), RETCODE_OK);
    std::string test6;
    ASSERT_EQ(struct_data->get_string_value(test6, 4), RETCODE_OK);

    // Test clone.
    auto clone = struct_data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(struct_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, struct_data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    // Test set_complex_value
    uint32_t test11 {456};
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_OK,
            complex_data->set_int32_value(MEMBER_ID_INVALID, test11));
    ASSERT_EQ(RETCODE_OK, struct_data->set_complex_value(0, complex_data));

    // Test loan_value
    DynamicData::_ref_type loan_data = struct_data->loan_value(1);
    ASSERT_TRUE(loan_data);
    ASSERT_FALSE(struct_data->loan_value(1));
    ASSERT_EQ(loan_data->get_int64_value(test4, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(test3, test4);
    ASSERT_EQ(struct_data->return_loaned_value(loan_data), RETCODE_OK);
    ASSERT_FALSE(struct_data->loan_value(MEMBER_ID_INVALID));

    // Testing get_item_count
    ASSERT_EQ(4u, struct_data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(derived_struct_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&struct_data,
                XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&struct_data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(derived_struct_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(struct_data));
        ASSERT_EQ(data2->get_int32_value(test2, 0), RETCODE_OK);
        ASSERT_EQ(test11, test2);
        ASSERT_EQ(data2->get_int64_value(test4, 1), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        ASSERT_EQ(data2->get_int32_value(test2, 2), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        ASSERT_EQ(data2->get_string_value(test6, 4), RETCODE_OK);
        ASSERT_EQ(test5, test6);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(derived_struct_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&struct_data,
                XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&struct_data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(derived_struct_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(struct_data));
        ASSERT_EQ(data2->get_int32_value(test2, 0), RETCODE_OK);
        ASSERT_EQ(test11, test2);
        ASSERT_EQ(data2->get_int64_value(test4, 1), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        ASSERT_EQ(data2->get_int32_value(test2, 2), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        ASSERT_EQ(data2->get_string_value(test6, 4), RETCODE_OK);
        ASSERT_EQ(test5, test6);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, struct_data->clear_all_values());
    ASSERT_EQ(4u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    ASSERT_EQ(0, test4);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 2));
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_string_value(test6, 4));
    ASSERT_EQ("", test6);

    ASSERT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    ASSERT_EQ(RETCODE_OK, struct_data->set_int32_value(2, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_string_value(4, test5));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_nonkey_values());
    ASSERT_EQ(4u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    ASSERT_EQ(0, test4);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 2));
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_string_value(test6, 4));
    ASSERT_EQ("", test6);

    ASSERT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    ASSERT_EQ(RETCODE_OK, struct_data->set_int32_value(2, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_string_value(4, test5));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_value(1));
    ASSERT_NE(RETCODE_OK, struct_data->clear_value(3));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_value(4));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(100));
    ASSERT_EQ(4u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    ASSERT_EQ(test1, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    ASSERT_EQ(0, test4);
    ASSERT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 2));
    ASSERT_EQ(test1, test2);
    ASSERT_EQ(RETCODE_OK, struct_data->get_string_value(test6, 4));
    ASSERT_EQ("", test6);

    DynamicDataFactory::get_instance()->delete_data(struct_data);
}

TEST_F(DynamicTypesTests, DynamicType_multi_structure)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("InnerStructTest");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("int32");
    member_descriptor->id(0);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->id(1);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type inner_struct_type {builder->build()};
    ASSERT_TRUE(inner_struct_type);

    // Create the parent struct.
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructTest");
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    // Add members to the struct.
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(inner_struct_type);
    member_descriptor->name("Structure");
    member_descriptor->id(0);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->id(10);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type struct_data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(struct_data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_by_name(""));
    ASSERT_EQ(0, struct_data->get_member_id_by_name("Structure"));
    ASSERT_EQ(10, struct_data->get_member_id_by_name("int64"));
    ASSERT_EQ(0, struct_data->get_member_id_at_index(0));
    ASSERT_EQ(10, struct_data->get_member_id_at_index(1));
    ASSERT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_at_index(2));

    // Testing getter, setters and loan_value.
    ASSERT_NE(struct_data->set_int32_value(1, 10), RETCODE_OK);
    ASSERT_NE(struct_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);

    // Set and get the child values.
    const int64_t test1 {234};
    ASSERT_EQ(struct_data->set_int64_value(10, test1), RETCODE_OK);
    int64_t test2 {0};
    ASSERT_EQ(struct_data->get_int64_value(test2, 10), RETCODE_OK);
    ASSERT_EQ(test1, test2);

    auto inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);

    // Set and get the child values.
    int32_t test3 {234};
    ASSERT_EQ(inner_struct_data->set_int32_value(0, test3), RETCODE_OK);
    int32_t test4 {0};
    ASSERT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    ASSERT_EQ(test3, test4);
    int64_t test5 {234};
    ASSERT_EQ(inner_struct_data->set_int64_value(1, test5), RETCODE_OK);
    int64_t test6 {0};
    ASSERT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    ASSERT_EQ(test5, test6);

    ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    // Test clone.
    auto clone = struct_data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(struct_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_OK, struct_data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_int32_value(test4, 0), RETCODE_OK);
    ASSERT_EQ(test3, test4);
    ASSERT_EQ(complex_data->get_int64_value(test6, 1), RETCODE_OK);
    ASSERT_EQ(test5, test6);

    // Test set_complex_value
    test3 = 456;
    test5 = 567;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(complex_data->set_int32_value(0, test3), RETCODE_OK);
    ASSERT_EQ(complex_data->set_int64_value(1, test5), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, struct_data->set_complex_value(0, complex_data));

    // Testing get_item_count.
    ASSERT_EQ(2u, struct_data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(struct_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&struct_data,
                XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&struct_data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(struct_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(struct_data));
        ASSERT_EQ(data2->get_int64_value(test2, 10), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        inner_struct_data = struct_data->loan_value(0);
        ASSERT_TRUE(inner_struct_data);
        ASSERT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        ASSERT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
        ASSERT_EQ(test5, test6);
        ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(struct_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&struct_data,
                XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&struct_data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(struct_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(struct_data));
        ASSERT_EQ(data2->get_int64_value(test2, 10), RETCODE_OK);
        ASSERT_EQ(test1, test2);
        inner_struct_data = struct_data->loan_value(0);
        ASSERT_TRUE(inner_struct_data);
        ASSERT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
        ASSERT_EQ(test3, test4);
        ASSERT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
        ASSERT_EQ(test5, test6);
        ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, struct_data->clear_all_values());
    ASSERT_EQ(2u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test2, 10));
    ASSERT_EQ(0, test2);
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    ASSERT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    ASSERT_EQ(0, test4);
    ASSERT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    ASSERT_EQ(0, test6);
    ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    ASSERT_EQ(RETCODE_OK, struct_data->set_int64_value(10, test1));
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    ASSERT_EQ(inner_struct_data->set_int32_value(0, test3), RETCODE_OK);
    ASSERT_EQ(inner_struct_data->set_int64_value(1, test5), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_nonkey_values());
    ASSERT_EQ(2u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test2, 10));
    ASSERT_EQ(0, test2);
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    ASSERT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    ASSERT_EQ(0, test4);
    ASSERT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    ASSERT_EQ(0, test6);
    ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    ASSERT_EQ(RETCODE_OK, struct_data->set_int64_value(10, test1));
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    ASSERT_EQ(inner_struct_data->set_int32_value(0, test3), RETCODE_OK);
    ASSERT_EQ(inner_struct_data->set_int64_value(1, test5), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));
    ASSERT_EQ(RETCODE_OK, struct_data->clear_value(0));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(100));
    ASSERT_EQ(2u, struct_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, struct_data->get_int64_value(test2, 10));
    ASSERT_EQ(test1, test2);
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    ASSERT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    ASSERT_EQ(0, test4);
    ASSERT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    ASSERT_EQ(0, test6);
    ASSERT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    DynamicDataFactory::get_instance()->delete_data(struct_data);
}

TEST_F(DynamicTypesTests, DynamicType_union)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create the base struct.
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("UnionTest");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_FALSE(builder);
    type_descriptor->discriminator_type(factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT32));
    builder = factory->create_type(type_descriptor);
    ASSERT_FALSE(builder);
    type_descriptor->discriminator_type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);


    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("first");
    member_descriptor->id(1);
    member_descriptor->label({0, 1});
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("second");
    member_descriptor->id(3);
    member_descriptor->label({4});
    member_descriptor->default_value("default");
    member_descriptor->is_default_label(true);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");     // avoid expected errors logging

        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
        member_descriptor->name("second");
        member_descriptor->id(4);
        member_descriptor->label({5});
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

        member_descriptor->name("third");
        member_descriptor->id(0);
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

        member_descriptor->id(3);
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

        member_descriptor->id(5);
        member_descriptor->label({});
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

        member_descriptor->label({4, 5});
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

        member_descriptor->label({5, 6});
        member_descriptor->is_default_label(true);
        ASSERT_NE(builder->add_member(member_descriptor), RETCODE_OK);

    }

    // Create a data of this union
    DynamicType::_ref_type union_type {builder->build()};
    ASSERT_TRUE(union_type);

    DynamicData::_ref_type union_data {DynamicDataFactory::get_instance()->create_data(union_type)};
    ASSERT_TRUE(union_data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_by_name(""));
    ASSERT_EQ(0, union_data->get_member_id_by_name("discriminator"));
    ASSERT_EQ(1, union_data->get_member_id_by_name("first"));
    ASSERT_EQ(3, union_data->get_member_id_by_name("second"));
    ASSERT_EQ(0, union_data->get_member_id_at_index(0));
    ASSERT_EQ(1, union_data->get_member_id_at_index(1));
    ASSERT_EQ(3, union_data->get_member_id_at_index(2));
    ASSERT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_at_index(3));

    // Testing getters and setters.
    int32_t discriminator_value {0};
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);

    int64_t int64_get {0};
    ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    std::string string_get;
    ASSERT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    ASSERT_EQ("default", string_get);

    ASSERT_NE(union_data->set_int32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(union_data->set_int32_value(0, 4), RETCODE_OK);

    const int64_t int64_set {234};

    ASSERT_EQ(union_data->set_int64_value(1, int64_set), RETCODE_OK);
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 == discriminator_value);
    ASSERT_NE(union_data->get_string_value(string_get, 3), RETCODE_OK);
    ASSERT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    ASSERT_EQ(int64_set, int64_get);
    ASSERT_NE(union_data->set_int32_value(0, 4), RETCODE_OK);
    ASSERT_EQ(union_data->set_int32_value(0, 1), RETCODE_OK);

    std::string string_set {"testing_value"};
    ASSERT_EQ(union_data->set_string_value(3, string_set), RETCODE_OK);
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    ASSERT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    ASSERT_EQ(string_set, string_get);
    ASSERT_NE(union_data->set_int32_value(0, 1), RETCODE_OK);
    ASSERT_EQ(union_data->set_int32_value(0, 4), RETCODE_OK);

    // Test clone.
    auto clone = union_data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(union_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, 1));
    ASSERT_EQ(RETCODE_OK, union_data->get_complex_value(complex_data, 3));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_string_value(string_get, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(string_set, string_get);

    // Test set_complex_value
    string_set = "retesting";
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(0, complex_data));
    ASSERT_EQ(complex_data->set_string_value(MEMBER_ID_INVALID, string_set), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, union_data->set_complex_value(3, complex_data));

    // Testing loan_value.
    DynamicData::_ref_type loan_data = union_data->loan_value(1);
    ASSERT_TRUE(loan_data);
    ASSERT_FALSE(union_data->loan_value(1));
    ASSERT_EQ(loan_data->get_int64_value(int64_get, MEMBER_ID_INVALID), RETCODE_OK);
    ASSERT_EQ(int64_set, int64_get);
    ASSERT_EQ(union_data->return_loaned_value(loan_data), RETCODE_OK);
    ASSERT_FALSE(union_data->loan_value(MEMBER_ID_INVALID));
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_EQ(0, discriminator_value);

    // Testing loan_value.

    // Testing get_item_count.
    ASSERT_EQ(2u, union_data->get_item_count());

    // Encoding/decoding
    ASSERT_EQ(union_data->set_string_value(3, string_set), RETCODE_OK);
    ASSERT_EQ(union_data->set_int32_value(0, 4), RETCODE_OK);
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(union_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&union_data,
                XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&union_data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(union_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(union_data));
        ASSERT_EQ(data2->get_int32_value(discriminator_value, 0), RETCODE_OK);
        ASSERT_EQ(4, discriminator_value);
        ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
        ASSERT_EQ(data2->get_string_value(string_get, 3), RETCODE_OK);
        ASSERT_EQ(string_set, string_get);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(union_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&union_data,
                XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&union_data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(union_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(union_data));
        ASSERT_EQ(data2->get_int32_value(discriminator_value, 0), RETCODE_OK);
        ASSERT_EQ(4, discriminator_value);
        ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
        ASSERT_EQ(data2->get_string_value(string_get, 3), RETCODE_OK);
        ASSERT_EQ(string_set, string_get);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, union_data->clear_all_values());
    ASSERT_EQ(2u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    ASSERT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    ASSERT_EQ("default", string_get);

    ASSERT_EQ(union_data->set_int64_value(1, int64_set), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, union_data->clear_nonkey_values());
    ASSERT_EQ(2u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    ASSERT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    ASSERT_EQ("default", string_get);

    ASSERT_EQ(union_data->set_int64_value(1, int64_set), RETCODE_OK);
    ASSERT_NE(RETCODE_OK, union_data->clear_value(3));
    ASSERT_EQ(RETCODE_OK, union_data->clear_value(1));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->clear_value(100));
    ASSERT_EQ(2u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    ASSERT_EQ(0, int64_get);
    ASSERT_EQ(RETCODE_OK, union_data->clear_value(0));
    ASSERT_NE(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    ASSERT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    ASSERT_EQ("default", string_get);

    DynamicDataFactory::get_instance()->delete_data(union_data);
}

TEST_F(DynamicTypesTests, DynamicType_union_with_unions)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create the base struct.
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("InnerUnionTest");
    type_descriptor->discriminator_type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);


    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("first");
    member_descriptor->id(1);
    member_descriptor->label({0, 1});
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("second");
    member_descriptor->id(2);
    member_descriptor->label({4});
    member_descriptor->default_value("default");
    member_descriptor->is_default_label(true);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type inner_union_type {builder->build()};
    ASSERT_TRUE(inner_union_type);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("UnionTest");
    type_descriptor->discriminator_type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("first");
    member_descriptor->id(1);
    member_descriptor->label({1});
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(inner_union_type);
    member_descriptor->name("second");
    member_descriptor->id(2);
    member_descriptor->label({4});
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type union_type {builder->build()};
    ASSERT_TRUE(union_type);

    DynamicData::_ref_type union_data {DynamicDataFactory::get_instance()->create_data(union_type)};
    ASSERT_TRUE(union_data);

    // Test get_member_by_name and get_member_by_index.
    ASSERT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_by_name(""));
    ASSERT_EQ(0, union_data->get_member_id_by_name("discriminator"));
    ASSERT_EQ(1, union_data->get_member_id_by_name("first"));
    ASSERT_EQ(2, union_data->get_member_id_by_name("second"));
    ASSERT_EQ(0, union_data->get_member_id_at_index(0));
    ASSERT_EQ(1, union_data->get_member_id_at_index(1));
    ASSERT_EQ(2, union_data->get_member_id_at_index(2));
    ASSERT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_at_index(3));

    // Testing getters, setters and loan_value.
    // Set and get the child values.
    ASSERT_NE(union_data->set_int32_value(2, 10), RETCODE_OK);
    ASSERT_NE(union_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_OK);

    int32_t discriminator_value {0};
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(1 != discriminator_value && 4 != discriminator_value);

    const int64_t test1 {234};
    ASSERT_EQ(union_data->set_int64_value(1, test1), RETCODE_OK);
    int64_t test2 {0};
    ASSERT_EQ(union_data->get_int64_value(test2, 1), RETCODE_OK);
    ASSERT_EQ(test1, test2);
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_EQ(1, discriminator_value);

    DynamicData::_ref_type child_data {union_data->loan_value(2)};
    ASSERT_TRUE(child_data);

    std::string test3;

    ASSERT_EQ(child_data->get_string_value(test3, 2), RETCODE_OK);
    ASSERT_EQ("default", test3);

    ASSERT_EQ(union_data->return_loaned_value(child_data), RETCODE_OK);

    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_EQ(4, discriminator_value);

    // Test clone.
    auto clone = union_data->clone();
    ASSERT_TRUE(clone);
    ASSERT_TRUE(union_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, 1));
    ASSERT_EQ(RETCODE_OK, union_data->get_complex_value(complex_data, 2));
    ASSERT_TRUE(complex_data);
    ASSERT_EQ(complex_data->get_string_value(test3, 2), RETCODE_OK);
    ASSERT_EQ("default", test3);

    // Test set_complex_value
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(0, complex_data));
    ASSERT_EQ(complex_data->set_string_value(2, "another"), RETCODE_OK);
    ASSERT_EQ(RETCODE_OK, union_data->set_complex_value(2, complex_data));

    // Testing get_item_count.
    ASSERT_EQ(2u, union_data->get_item_count());

    // Encoding/decoding
    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(union_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&union_data,
                XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&union_data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(union_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(union_data));
        ASSERT_EQ(data2->get_int32_value(discriminator_value, 0), RETCODE_OK);
        ASSERT_EQ(4, discriminator_value);
        child_data = union_data->loan_value(2);
        ASSERT_TRUE(child_data);
        ASSERT_EQ(child_data->get_string_value(test3, 2), RETCODE_OK);
        ASSERT_EQ("another", test3);
        ASSERT_EQ(union_data->return_loaned_value(child_data), RETCODE_OK);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // XCDRv2
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(union_type);
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&union_data,
                XCDR2_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&union_data, &payload, XCDR2_DATA_REPRESENTATION));
        ASSERT_EQ(payload.length, payloadSize);
        ASSERT_LE(payload.length, pubsubType.m_typeSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(union_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(union_data));
        ASSERT_EQ(data2->get_int32_value(discriminator_value, 0), RETCODE_OK);
        ASSERT_EQ(4, discriminator_value);
        child_data = union_data->loan_value(2);
        ASSERT_TRUE(child_data);
        ASSERT_EQ(child_data->get_string_value(test3, 2), RETCODE_OK);
        ASSERT_EQ("another", test3);
        ASSERT_EQ(union_data->return_loaned_value(child_data), RETCODE_OK);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(RETCODE_OK, union_data->clear_all_values());
    ASSERT_EQ(1u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    ASSERT_NE(union_data->get_int64_value(test2, 1), RETCODE_OK);

    ASSERT_EQ(union_data->set_int64_value(1, test1), RETCODE_OK);
    ASSERT_EQ(2u, union_data->get_item_count());
    ASSERT_EQ(RETCODE_OK, union_data->clear_nonkey_values());
    ASSERT_EQ(1u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    ASSERT_NE(union_data->get_int64_value(test2, 1), RETCODE_OK);

    ASSERT_EQ(union_data->set_int64_value(1, test1), RETCODE_OK);
    ASSERT_EQ(2u, union_data->get_item_count());
    ASSERT_NE(RETCODE_OK, union_data->clear_value(2));
    ASSERT_EQ(RETCODE_OK, union_data->clear_value(1));
    ASSERT_EQ(RETCODE_BAD_PARAMETER, union_data->clear_value(100));
    ASSERT_EQ(2u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int64_value(test2, 1), RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(RETCODE_OK, union_data->clear_value(0));
    ASSERT_EQ(1u, union_data->get_item_count());
    ASSERT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    ASSERT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    ASSERT_NE(union_data->get_int64_value(test2, 1), RETCODE_OK);

    DynamicDataFactory::get_instance()->delete_data(union_data);
}

TEST_F(DynamicTypesTests, DynamicType_KeyHash_standard_example_1)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("Foo");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("id");
    member_descriptor->is_key(true);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("x");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("y");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    ASSERT_EQ(RETCODE_OK, data->set_int32_value(0, 0x12345678));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(1, 10));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(1, 20));

    DynamicPubSubType pubsubType(struct_type);
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    ASSERT_TRUE(pubsubType.getKey(&data, &instance_handle));

    const uint8_t expected_key_hash[] {
        0x12, 0x34, 0x56, 0x78,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    ASSERT_EQ(0, memcmp(expected_key_hash, &instance_handle, 16));
}

TEST_F(DynamicTypesTests, DynamicType_KeyHash_standard_example_2)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("Foo");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->create_string_type(12)->build());
    member_descriptor->name("label");
    member_descriptor->is_key(true);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    member_descriptor->name("id");
    member_descriptor->is_key(true);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("x");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("y");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    ASSERT_EQ(RETCODE_OK, data->set_string_value(0, "BLUE"));
    ASSERT_EQ(RETCODE_OK, data->set_int64_value(1, 0x123456789abcdef0ll));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(2, 10));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(3, 20));

    DynamicPubSubType pubsubType(struct_type);
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    ASSERT_TRUE(pubsubType.getKey(&data, &instance_handle));

    const uint8_t expected_key_hash[] {
        0xf9, 0x1a, 0x59, 0xe3,
        0x2e, 0x45, 0x35, 0xd9,
        0xa6, 0x9c, 0xd5, 0xd9,
        0xf5, 0xb6, 0xe3, 0x6e
    };

    ASSERT_EQ(0, memcmp(expected_key_hash, &instance_handle, 16));
}

TEST_F(DynamicTypesTests, DynamicType_KeyHash_standard_example_3)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("Nested");
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("m_long");
    member_descriptor->is_key(true);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("u");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("w");
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type nested_type {builder->build()};
    ASSERT_TRUE(nested_type);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("Foo");
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    // Add members to the struct.
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(16)->build());
    member_descriptor->name("label");
    member_descriptor->is_key(true);
    member_descriptor->id(40);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(nested_type);
    member_descriptor->name("m_nested");
    member_descriptor->is_key(true);
    member_descriptor->id(30);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("x");
    member_descriptor->id(20);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("y");
    member_descriptor->id(10);
    ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    ASSERT_EQ(RETCODE_OK, data->set_string_value(40, "BLUE"));
    auto nested_data = data->loan_value(30);
    ASSERT_TRUE(nested_data);
    ASSERT_EQ(RETCODE_OK, nested_data->set_int32_value(0, 0x12345678l));
    ASSERT_EQ(RETCODE_OK, nested_data->set_int32_value(1, 10));
    ASSERT_EQ(RETCODE_OK, nested_data->set_int32_value(2, 20));
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(nested_data));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(20, 100));
    ASSERT_EQ(RETCODE_OK, data->set_int32_value(10, 200));

    DynamicPubSubType pubsubType(struct_type);
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    ASSERT_TRUE(pubsubType.getKey(&data, &instance_handle));

    const uint8_t expected_key_hash[] {
        0x37, 0x4b, 0x96, 0xe2,
        0xe7, 0x27, 0x23, 0x7f,
        0x01, 0x6c, 0xc4, 0xce,
        0xbb, 0x6e, 0xb7, 0x1e
    };

    ASSERT_EQ(0, memcmp(expected_key_hash, &instance_handle, 16));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_enum)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("EnumStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Enum
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ENUM);
    type_descriptor->name("MyEnum");
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("A");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("B");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("C");
    enum_builder->add_member(member_descriptor);

    // Struct EnumStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("EnumStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(enum_builder->build());
    member_descriptor->name("my_enum");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("AliasStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Enum
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ENUM);
    type_descriptor->name("MyEnum");
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("A");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("B");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("C");
    enum_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyAliasEnum");
    type_descriptor->base_type(enum_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    // Struct AliasStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("AliasStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(alias_builder->build());
    member_descriptor->name("my_alias");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias_with_alias)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("AliasAliasStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Enum
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ENUM);
    type_descriptor->name("MyEnum");
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("A");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("B");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("C");
    enum_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyAliasEnum");
    type_descriptor->base_type(enum_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyAliasAliasEnum");
    type_descriptor->base_type(alias_builder->build());
    DynamicTypeBuilder::_ref_type alias_alias_builder {factory->create_type(type_descriptor)};

    // Struct AliasStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("AliasAliasStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(alias_alias_builder->build());
    member_descriptor->name("my_alias_alias");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_boolean)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("BoolStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct BoolStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("BoolStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->name("my_bool");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_octet)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("OctetStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct OctetStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("OctetStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BYTE));
    member_descriptor->name("my_octet");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_short)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ShortStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ShortStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->name("my_int16");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_long)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("LongStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("LongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("my_int32");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_longlong)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("LongLongStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LongLongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("LongLongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("my_int64");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_ushort)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("UShortStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct UShortStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("UShortStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    member_descriptor->name("my_uint16");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_ulong)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ULongStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ULongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ULongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    member_descriptor->name("my_uint32");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_ulonglong)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ULongLongStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ULongLongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ULongLongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_UINT64));
    member_descriptor->name("my_uint64");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_float)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("FloatStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct FloatStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("FloatStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT32));
    member_descriptor->name("my_float32");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_double)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("DoubleStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct DoubleStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("DoubleStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT64));
    member_descriptor->name("my_float64");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_longdouble)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("LongDoubleStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LongDoubleStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("LongDoubleStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT128));
    member_descriptor->name("my_float128");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_char)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("CharStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct CharStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("CharStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_CHAR8));
    member_descriptor->name("my_char");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_wchar)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("WCharStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct WCharStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("WCharStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_CHAR16));
    member_descriptor->name("my_wchar");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_string)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("StringStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct StringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("my_string");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_wstring)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("WStringStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct WStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("WStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("my_wstring");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}


TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_large_string)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("LargeStringStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LargeStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("LargeStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(41925)->build());
    member_descriptor->name("my_large_string");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_large_wstring)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("LargeWStringStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LargeWStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("LargeWStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_wstring_type(41925)->build());
    member_descriptor->name("my_large_wstring");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_short_string)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStringStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ShortStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ShortStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(15)->build());
    member_descriptor->name("my_short_string");
    builder->add_member(member_descriptor);

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_short_wstring)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortWStringStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ShortWStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ShortWStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_wstring_type(15)->build());
    member_descriptor->name("my_short_wstring");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias_of_string)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructAliasString");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Alias
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyAliasString");
    type_descriptor->base_type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    // Struct StructAliasString
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructAliasString");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(alias_builder->build());
    member_descriptor->name("my_alias_string");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias_of_wstring)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructAliasWString");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Alias
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyAliasWString");
    type_descriptor->base_type(factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    // Struct StructAliasWString
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructAliasWString");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(alias_builder->build());
    member_descriptor->name("my_alias_wstring");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_array)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Array
    DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(factory->get_primitive_type(
                                                             TK_INT32), {2, 2, 2})};

    // Struct ArrayStruct
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ArrayStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(array_builder->build());
    member_descriptor->name("my_array");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_array_of_arrays)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Typedef aka Alias
    DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(factory->get_primitive_type(
                                                             TK_INT32), {2, 2})};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyArray");
    type_descriptor->base_type(array_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type array_array_builder {factory->create_array_type(alias_builder->build(), {2, 2})};

    // Struct ArrayArrayStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ArrayArrayStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(array_array_builder->build());
    member_descriptor->name("my_array_array");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_array_struct_with_array_of_arrays)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayArrayStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Typedef aka Alias
    DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(factory->get_primitive_type(
                                                             TK_INT32), {2, 2})};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("MyArray");
    type_descriptor->base_type(array_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type array_array_builder {factory->create_array_type(alias_builder->build(), {2, 2})};

    // Struct ArrayArrayStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ArrayArrayStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(array_array_builder->build());
    member_descriptor->name("my_array_array");
    builder->add_member(member_descriptor);

    DynamicTypeBuilder::_ref_type array_struct_builder {factory->create_array_type(builder->build(), {2, 2})};


    // Struct ArrayArrayArrayStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("ArrayArrayArrayStruct");
    builder = factory->create_type(type_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(array_struct_builder->build());
    member_descriptor->name("my_array_array_array");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_sequence)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type sequence_builder {factory->create_sequence_type(factory->get_primitive_type(
                                                                TK_INT32), 2)};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("SequenceStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(sequence_builder->build());
    member_descriptor->name("my_sequence");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}


TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_sequence_of_sequences)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceSequenceStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type sequence_builder {factory->create_sequence_type(factory->get_primitive_type(
                                                                TK_INT32), 2)};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("my_sequence_sequence_inner");
    type_descriptor->base_type(sequence_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type sequence_sequence_builder {factory->create_sequence_type(alias_builder->build(), 2)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("SequenceSequenceStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(sequence_sequence_builder->build());
    member_descriptor->name("my_sequence_sequence");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_map)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("MapStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type map_builder {factory->create_map_type(factory->get_primitive_type(TK_INT32),
                                                       factory->get_primitive_type(TK_INT32), 7)};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("MapStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(map_builder->build());
    member_descriptor->name("my_map");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_map_of_maps)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("MapMapStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type map_builder {factory->create_map_type(factory->get_primitive_type(TK_INT32),
                                                       factory->get_primitive_type(TK_INT32), 2)};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ALIAS);
    type_descriptor->name("my_map_map_inner");
    type_descriptor->base_type(map_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type map_map_builder {factory->create_map_type(factory->get_primitive_type(TK_INT32),
                                                           alias_builder->build(), 2)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("MapMapStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(map_map_builder->build());
    member_descriptor->name("my_map_map");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_two_members)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type((TK_INT32)));
    member_descriptor->name("a");
    builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT64)));
    member_descriptor->name("b");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_struct)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructStructStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructStruct");
    DynamicTypeBuilder::_ref_type child_builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type((TK_INT32)));
    member_descriptor->name("a");
    child_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT64)));
    member_descriptor->name("b");
    child_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("StructStructStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(child_builder->build());
    member_descriptor->name("child_struct");
    builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT64)));
    member_descriptor->name("child_int64");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_union)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("SimpleUnionStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("SimpleUnion");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type((TK_INT32)));
    member_descriptor->name("first");
    member_descriptor->is_default_label(true);
    member_descriptor->label().push_back(0);
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT64)));
    member_descriptor->name("second");
    member_descriptor->label().push_back(1);
    union_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("SimpleUnionStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_builder->build());
    member_descriptor->name("my_union");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_union_with_union)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("UnionUnionStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("SimpleUnion");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type((TK_INT32)));
    member_descriptor->name("first");
    member_descriptor->is_default_label(true);
    member_descriptor->label().push_back(0);
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT64)));
    member_descriptor->name("second");
    member_descriptor->label().push_back(1);
    union_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("UnionUnion");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type union_union_builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT32)));
    member_descriptor->name("first");
    member_descriptor->is_default_label(true);
    member_descriptor->label().push_back(0);
    union_union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_builder->build());
    member_descriptor->name("second");
    member_descriptor->label().push_back(1);
    union_union_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("UnionUnionStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_union_builder->build());
    member_descriptor->name("my_union");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_WCharUnionStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("WCharUnionStruct");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_UNION);
    type_descriptor->name("WCharUnion");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_CHAR16));
    DynamicTypeBuilder::_ref_type union_builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type((TK_INT32)));
    member_descriptor->name("first");
    member_descriptor->is_default_label(true);
    member_descriptor->label().push_back(0);
    union_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_INT64)));
    member_descriptor->name("second");
    member_descriptor->label().push_back(1);
    union_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(eprosima::fastdds::dds::TK_STRUCTURE);
    type_descriptor->name("WCharUnionStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_builder->build());
    member_descriptor->name("my_union");
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_Bitset_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("MyBitSet");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    /*
       XML:
       <bitset name="MyBitSet">
            <bitfield name="a" bit_bound = "3"/ >
            <bitfield name="b" bit_bound="1"/ >
            <bitfield bit_bound="4"/>
            <bitfield name="c" bit_bound="10"/>
            <bitfield name="d" bit_bound="12" type="int16"/>
        </bitset>

       IDL:
       bitset MyBitset
       {
           bitfield<3> a; // @bit_bound=3 @position=0
           bitfield<1> b; // @bit_bound=1 @position=3
           bitfield<4>;
           bitfield<10> c; // @bit_bound=10 @position=8
           bitfield<12, short> d; // @bit_bound=12 @position=18
       };
     */

    // Bitset
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITSET);
    type_descriptor->name("MyBitSet");
    type_descriptor->bound({3, 1, 10, 12});
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->id(0);
    member_descriptor->name("a");
    member_descriptor->type(factory->get_primitive_type(TK_UINT8));
    ASSERT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->id(3);
    member_descriptor->name("b");
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    ASSERT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->id(8);
    member_descriptor->name("c");
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    ASSERT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->id(18);
    member_descriptor->name("d");
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    ASSERT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_Bitmask_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    auto pbType = XMLProfileManager::CreateDynamicPubSubType("MyBitMask");

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Bitmask
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_BITMASK);
    type_descriptor->element_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    type_descriptor->name("MyBitMask");
    type_descriptor->bound().push_back(8);
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type((TK_BOOLEAN)));
    member_descriptor->name("flag0");
    builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_BOOLEAN)));
    member_descriptor->name("flag1");
    builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_BOOLEAN)));
    member_descriptor->name("flag2");
    builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type((TK_BOOLEAN)));
    member_descriptor->name("flag5");
    member_descriptor->id(5);
    builder->add_member(member_descriptor);

    DynamicType::_ref_type type {pbType->get_dynamic_type()};
    ASSERT_TRUE(type->equals(builder->build()));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, TypeDescriptorFullyQualifiedName)
{
    TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};

    descriptor->name("Position");
    ASSERT_TRUE(descriptor->is_consistent());
    descriptor->name("Position_");
    ASSERT_TRUE(descriptor->is_consistent());
    descriptor->name("Position123");
    ASSERT_TRUE(descriptor->is_consistent());
    descriptor->name("position_123");
    ASSERT_TRUE(descriptor->is_consistent());
    descriptor->name("_Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("123Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("Position&");
    ASSERT_FALSE(descriptor->is_consistent());

    descriptor->name("my_interface::action::dds_::Position");
    ASSERT_TRUE(descriptor->is_consistent());
    descriptor->name("my_interface:action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("my_interface:::action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("_my_interface::action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("1my_interface::action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name(":my_interface::action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("::my_interface::action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("$my_interface::action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("my_interface::2action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("my_interface::_action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("my_interface::*action::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
    descriptor->name("my_interface::action*::dds_::Position");
    ASSERT_FALSE(descriptor->is_consistent());
}

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
