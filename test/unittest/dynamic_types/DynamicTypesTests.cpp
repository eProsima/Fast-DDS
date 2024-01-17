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
#include <tinyxml2.h>

#include <algorithm>
#include <array>
#include <set>
#include <sstream>
#include <thread>
#include <tuple>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "idl/BasicPubSubTypes.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::dds;

using eprosima::fastrtps::types::TypeKind;

// Ancillary gtest formatters

/*TODO(richiware)
   void PrintTo(
        const MemberDescriptor& md,
        std::ostream* os)
   {
    if (os)
    {
 * os << md;
    }
   }

   void PrintTo(
        const TypeDescriptor& md,
        std::ostream* os)
   {
    if (os)
    {
 * os << md;
    }
   }
 */

using primitive_builder_api = const DynamicTypeBuilder * (DynamicTypeBuilderFactory::* )();
using primitive_type_api = const DynamicType * (DynamicTypeBuilderFactory::* )();

// Testing the primitive creation APIS
// and get_primitive_type() and create_primitive_type()
class DynamicTypesPrimitiveTestsAPIs
    : public testing::TestWithParam <TypeKind>
{
};

TEST_P(DynamicTypesPrimitiveTestsAPIs, primitives_apis_unit_tests)
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


TEST_F(DynamicTypesTests, TypeDescriptors_unit_tests)
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
    ASSERT_EQ(primitive->get_descriptor(state), eprosima::fastdds::dds::RETCODE_OK);
    DynamicTypeBuilder::_ref_type builder2 {factory->create_type(state)};
    ASSERT_TRUE(builder2);
    EXPECT_TRUE(builder2->equals(primitive));

    // Copy state
    TypeDescriptor::_ref_type state2 {traits<TypeDescriptor>::make_shared()};
    EXPECT_EQ(state2->copy_from(state), eprosima::fastdds::dds::RETCODE_OK);
    EXPECT_TRUE(state2->equals(state));

    TypeDescriptor::_ref_type state3 {traits<TypeDescriptor>::make_shared()};
    ASSERT_EQ(builder->get_descriptor(state3), eprosima::fastdds::dds::RETCODE_OK);
    EXPECT_EQ(state2->copy_from(state3), eprosima::fastdds::dds::RETCODE_OK);
    EXPECT_TRUE(state2->equals(state3));

    // Check state doesn't match the default descriptor
    TypeDescriptor::_ref_type default_descriptor {traits<TypeDescriptor>::make_shared()};
    EXPECT_FALSE(state->equals(default_descriptor));
}

TEST_F(DynamicTypesTests, DynamicType_basic_unit_tests)
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
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->add_member(member_descriptor));
    EXPECT_EQ(struct_type_builder->get_member_count(), 1u);

    DynamicType::_ref_type struct_type {struct_type_builder->build()};
    ASSERT_TRUE(struct_type);
    EXPECT_TRUE(struct_type_builder->equals(struct_type));

    member_descriptor->id(1);
    member_descriptor->name("int64");
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->add_member(member_descriptor));
    EXPECT_EQ(struct_type_builder->get_member_count(), 2u);

    DynamicType::_ref_type struct_type2 {struct_type_builder->build()};
    ASSERT_TRUE(struct_type2);
    EXPECT_TRUE(struct_type_builder->equals(struct_type2));
    EXPECT_FALSE(struct_type->equals(struct_type2));

    DynamicTypeMember::_ref_type member;
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        // Check members are properly added
        // • checking invalid id
        EXPECT_NE(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member(member, 0));
        EXPECT_FALSE(member);
    }

    // • checking MemberDescriptor getters
    MemberDescriptor::_ref_type md = traits<MemberDescriptor>::make_shared();
    MemberDescriptor::_ref_type md1 = traits<MemberDescriptor>::make_shared();
    md1->id(3);
    md1->index(0);
    md1->name("int32");
    md1->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));

    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member(member, 3));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, member->get_descriptor(md));

    EXPECT_TRUE(md->is_consistent());
    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());
    EXPECT_TRUE(md->equals(md1));

    // • checking MemberDescriptor comparison and construction
    MemberDescriptor::_ref_type md2 = traits<MemberDescriptor>::make_shared();
    md2->id(1);
    md2->index(1);
    md2->name("int64");
    md2->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member(member, 1));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, member->get_descriptor(md));
    EXPECT_TRUE(md->is_consistent());
    EXPECT_EQ(md->index(), 1u);
    EXPECT_TRUE(md->equals(md2));

    EXPECT_FALSE(md1->equals(md2));

    //    + checking copy_from
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, md->copy_from(md1));
    EXPECT_TRUE(md->equals(md1));

    // • checking by index retrieval
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member_by_index(member, 0));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, member->get_descriptor(md));
    EXPECT_TRUE(md->equals(md1));

    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member_by_index(member, 1));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, member->get_descriptor(md));
    EXPECT_TRUE(md->equals(md2));

    // • checking by name retrieval
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member_by_name(member, "int32"));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, member->get_descriptor(md));
    EXPECT_TRUE(md->equals(md1));

    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->get_member_by_name(member, "int64"));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, member->get_descriptor(md));
    EXPECT_TRUE(md->equals(md2));

    // • checking map indexes retrieval
    //    + indexing by id
    DynamicTypeMembersById members_by_id;
    struct_type_builder->get_all_members(members_by_id);
    EXPECT_EQ(2, members_by_id.size());

    auto dm3 = members_by_id[3];
    ASSERT_TRUE(dm3);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm3->get_descriptor(md));
    EXPECT_TRUE(md->equals(md1));

    auto dm1 = members_by_id[1];
    ASSERT_TRUE(dm1);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm1->get_descriptor(md));
    EXPECT_TRUE(md->equals(md2));

    //    + indexing by name
    DynamicTypeMembersByName members_by_name;
    struct_type_builder->get_all_members_by_name(members_by_name);
    EXPECT_EQ(2, members_by_name.size());

    dm3 = members_by_name["int32"];
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm3->get_descriptor(md));
    EXPECT_TRUE(md->equals(md1));

    dm1 = members_by_name["int64"];
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm1->get_descriptor(md));
    EXPECT_TRUE(md->equals(md2));

    // • checking indexes work according with OMG standard 1.3 section 7.5.2.7.6
    md = traits<MemberDescriptor>::make_shared();
    md->id(7);
    md->index(1);
    md->name("bool");
    md->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->add_member(md));

    struct_type_builder->get_all_members(members_by_id);
    ASSERT_EQ(3, members_by_id.size());
    md2->index(2);    // new expected position of the last element

    MemberDescriptor::_ref_type tmp = traits<MemberDescriptor>::make_shared();
    auto dm = members_by_id[3];
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_TRUE(tmp->equals(md1));

    dm = members_by_id[7];
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_TRUE(tmp->equals(md));

    dm = members_by_id[1];
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_TRUE(tmp->equals(md2));


    // • checking adding duplicates
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        //    + duplicate name
        md = traits<MemberDescriptor>::make_shared();
        md->id(1);
        md->name("int32");
        md->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
        EXPECT_NE(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->add_member(md));

        //    + duplicate id
        md = traits<MemberDescriptor>::make_shared();
        md->id(7);
        md->name("dup_bool");
        md->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN));
        EXPECT_NE(eprosima::fastdds::dds::RETCODE_OK, struct_type_builder->add_member(md));
    }
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Try to create with invalid values
    // • strings
    DynamicTypeBuilder::_ref_type created_builder {factory->create_string_type(LENGTH_UNLIMITED)};
    ASSERT_TRUE(created_builder);

    DynamicType::_ref_type type {created_builder->build()};
    ASSERT_TRUE(type);
    DynamicType::_ref_type type2 {created_builder->build()};
    ASSERT_TRUE(type2);

    ASSERT_TRUE(type->equals(type2));

    // • wstrings
    created_builder = factory->create_wstring_type(LENGTH_UNLIMITED);
    ASSERT_TRUE(created_builder);

    type = created_builder->build();
    ASSERT_TRUE(type);
    type2 = created_builder->build();
    ASSERT_TRUE(type2);

    ASSERT_TRUE(type->equals(type2));
}

TEST_F(DynamicTypesTests, DynamicType_int32_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test setters and getters.
    const int32_t test1 = 123;
    int32_t test2 = 0;
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_int32_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);

    uint32_t uTest32;
    ASSERT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    int16_t iTest16;
    ASSERT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    uint16_t uTest16;
    ASSERT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    int64_t iTest64;
    ASSERT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    uint64_t uTest64;
    ASSERT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    float fTest32;
    ASSERT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    double fTest64;
    ASSERT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    long double fTest128;
    ASSERT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    char cTest8;
    ASSERT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    wchar_t cTest16;
    ASSERT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    octet oTest;
    ASSERT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    bool bTest;
    ASSERT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    std::string sTest;
    ASSERT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
    std::wstring wsTest;
    ASSERT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_uint32_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const uint32_t test1 = 123;
    uint32_t test2 = 0;
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_uint32_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_int16_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const int16_t test1 = 123;
    int16_t test2 = 0;
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_int16_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int16_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int16_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int16_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_uint16_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const uint16_t test1 = 123;
    uint16_t test2 = 0;
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_uint16_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint16_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint16_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint16_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_int64_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_INT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const int64_t test1 = 123;
    int64_t test2 = 0;
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_int64_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        int64_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int64_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int64_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_uint64_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const uint64_t test1 = 123;
    uint64_t test2 = 0;
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_uint64_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint64_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint64_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint64_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_float32_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const float test1 = 123.0f;
    float test2 = 0.0f;
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_float32_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        float test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_float64_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const double test1 = 123.0;
    double test2 = 0.0;
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_float64_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        double test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float64_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float64_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_float128_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_FLOAT128)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const long double test1 = 123.0;
    long double test2 = 0.0;
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_float128_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        long double test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float128_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_float128_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_char8_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_CHAR8)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const char test1 = 'a';
    char test2 = 'b';
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_char8_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        char test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char8_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char8_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_char16_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_CHAR16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const wchar_t test1 = L'a';
    wchar_t test2 = L'b';
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_char16_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        wchar_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char16_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_char16_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_byte_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_BYTE)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const octet test1 = 255;
    octet test2 = 0;
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_byte_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        octet test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_byte_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_byte_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_bool_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(eprosima::fastdds::dds::TK_BOOLEAN)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const bool test1 = true;
    bool test2 = false;
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->get_boolean_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        bool test3 {false};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_boolean_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_boolean_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_enum_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(eprosima::fastdds::dds::TK_ENUM);
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    // Add three members to the enum.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("DEFAULT");
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("FIRST");
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("SECOND");
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_OK);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        // Try to add a descriptor with the same name.
        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
        member_descriptor->name("THIRD");
        EXPECT_NE(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_OK);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    ASSERT_EQ(3, created_type->get_member_count());
    DynamicTypeMember::_ref_type member;
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, created_type->get_member_by_index(member, 0));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("DEFAULT", member->get_name());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, created_type->get_member_by_index(member, 1));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("FIRST", member->get_name());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, created_type->get_member_by_index(member, 2));
    ASSERT_EQ(MEMBER_ID_INVALID, member->get_id());
    ASSERT_STREQ("SECOND", member->get_name());

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Try to set an invalid value.
    //TODO(richiware) EXPECT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 4), eprosima::fastdds::dds::RETCODE_OK);

    const uint32_t test1 {2};
    uint32_t test2 {0};
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(2, test2);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32 = {};
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16 = {};
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16 = {};
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64 = {};
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64 = {};
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32 = {};
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64 = {};
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128 = {};
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8 = {};
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16 = {};
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest = {};
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest = {};
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_string_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type builder {factory->create_string_type(0)};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    const uint32_t length = 15;
    builder = factory->create_string_type(length);
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    created_type = builder->build();
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);


    ASSERT_NE(data->set_string_value(1, ""), eprosima::fastdds::dds::RETCODE_OK);
    const std::string test1 {"STRING_TEST"};
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);

    std::string test2;
    ASSERT_NE(data->get_string_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID,
                "TEST_OVER_LENGTH_LIMITS"), eprosima::fastdds::dds::RETCODE_OK);
    }

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID, L""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(test1.length(), data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        std::string test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ("", test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_wstring_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type builder {factory->create_wstring_type(0)};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    const uint32_t length = 15;
    builder = factory->create_wstring_type(length);
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    created_type = builder->build();
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    ASSERT_NE(data->set_wstring_value(1, L""), eprosima::fastdds::dds::RETCODE_OK);
    const std::wstring test1 = L"STRING_TEST";
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);

    std::wstring test2;
    ASSERT_NE(data->get_wstring_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        ASSERT_NE(data->set_wstring_value(MEMBER_ID_INVALID,
                L"TEST_OVER_LENGTH_LIMITS"), eprosima::fastdds::dds::RETCODE_OK);
    }

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint16_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_uint64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float32_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float64_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_float128_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char8_value(MEMBER_ID_INVALID, 'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_char16_value(MEMBER_ID_INVALID, L'a'), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_byte_value(MEMBER_ID_INVALID, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_boolean_value(MEMBER_ID_INVALID, false), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);

    int32_t iTest32;
    ASSERT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int16_t iTest16;
    ASSERT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    int64_t iTest64;
    ASSERT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    float fTest32;
    ASSERT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    double fTest64;
    ASSERT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    long double fTest128;
    ASSERT_NE(data->get_float128_value(fTest128, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    char cTest8;
    ASSERT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    octet oTest;
    ASSERT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    bool bTest;
    ASSERT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string sTest;
    ASSERT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

    // Test get_item_count().
    ASSERT_EQ(test1.length(), data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        std::wstring test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_wstring_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_wstring_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(L"", test2);
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(L"", test2);
    ASSERT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(L"", test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_alias_unit_tests)
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
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    EXPECT_EQ(created_type->get_name(), name);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(1, ""), eprosima::fastdds::dds::RETCODE_OK);

    const uint32_t test1 = 2;
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);

    uint32_t test2 = 0;
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    // Test get_item_count().
    ASSERT_EQ(1, data->get_item_count());

    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        uint32_t test3 {0};
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);
    ASSERT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

/*
   TEST_F(DynamicTypesTests, DynamicType_nested_alias_unit_tests)
   {
    // Check alias comparison in dependent types
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // • Simple struct with nested aliases
    std::unique_ptr<DynamicTypeBuilder> plain_struct {factory.create_struct_type()};
    std::unique_ptr<DynamicTypeBuilder> alias_struct {factory.create_struct_type()};
    EXPECT_TRUE(plain_struct && alias_struct);

    for (auto& build : { plain_struct.get(), alias_struct.get() })
    {
        build->set_name("base_struct");
    }

    //   Add members to the plain struct
    EXPECT_EQ(plain_struct->add_member({0, "int32", factory.get_int32_type()}), eprosima::fastdds::dds::RETCODE_OK);
    EXPECT_EQ(plain_struct->add_member({1, "int64", factory.get_int64_type()}), eprosima::fastdds::dds::RETCODE_OK);

    //   Add members to the alias struct
    std::unique_ptr<const DynamicType> int32_type {factory.get_int32_type()};
    std::unique_ptr<DynamicTypeBuilder> alias_type32 {factory.create_alias_type(*int32_type, "int32_alias")};
    EXPECT_EQ(alias_struct->add_member({0, "int32", alias_type32->build()}), eprosima::fastdds::dds::RETCODE_OK);

    std::unique_ptr<const DynamicType> int64_type {factory.get_int64_type()};
    std::unique_ptr<DynamicTypeBuilder> alias_type64 {factory.create_alias_type(*int64_type, "int64_alias")};
    EXPECT_EQ(alias_struct->add_member({1, "int64", alias_type64->build()}), eprosima::fastdds::dds::RETCODE_OK);

    //   Compare
    EXPECT_EQ(*plain_struct, *alias_struct);

    // • Inheritance from an alias
    std::unique_ptr<DynamicTypeBuilder> child_struct { factory.create_child_struct_type(*plain_struct->build())};
    std::unique_ptr<DynamicTypeBuilder> child_alias_struct { factory.create_child_struct_type(*alias_struct->build())};

    for (auto& build : { child_struct.get(), child_alias_struct.get() })
    {
        build->set_name("child_struct");
    }

    //   Compare
    EXPECT_EQ(*child_struct, *child_alias_struct);

    // • Checking nesting at various levels
    unsigned int levels = 10;

    do
    {
        MemberId id{levels + 1u};

        std::string member_name{"member"};
        member_name += std::to_string(*id);

        std::string alias_name{"alias"};
        alias_name += std::to_string(*id);

        std::string struct_name{"nested"};
        struct_name += std::to_string(*id);

        auto aux = child_struct->build();
        std::unique_ptr<DynamicTypeBuilder> nested_struct {factory.create_child_struct_type(*aux)};
        ASSERT_TRUE(nested_struct);
        EXPECT_EQ(nested_struct->add_member({id, member_name.c_str(), aux}), eprosima::fastdds::dds::RETCODE_OK);

        aux = std::unique_ptr<const DynamicTypeBuilder> {factory.create_alias_type(*child_alias_struct->build(),
                                                                 alias_name.c_str())}->build();
        std::unique_ptr<DynamicTypeBuilder>nested_alias_struct {factory.create_child_struct_type(*aux)};
        ASSERT_TRUE(nested_alias_struct);
        EXPECT_EQ(nested_alias_struct->add_member({id, member_name.c_str(), aux}), eprosima::fastdds::dds::RETCODE_OK);

        for (auto& build : { nested_struct.get(), nested_alias_struct.get() })
        {
            build->set_name(struct_name.c_str());
        }
    }
    while (--levels);

    EXPECT_EQ(*nested_struct.get(), *nested_alias_struct.get());

    // • Checking serialization of aliases
    auto nested_type = nested_struct.build();
    std::unique_ptr<DynamicData> data {DynamicDataFactory::get_instance().create_data(*nested_type)};
    std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*nested_type)};
    ASSERT_TRUE(data);
    ASSERT_TRUE(data2);

    DynamicPubSubType pubsubType(*nested_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data.get())());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data.get(), &payload));
    ASSERT_EQ(payload.length, payloadSize);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
    ASSERT_TRUE(data2->equals(*data));

    // • Checking serialization of nested aliases
    auto alias_type = alias_struct.build();
    data.reset(DynamicDataFactory::get_instance().create_data(*alias_type)),
    data2.reset(DynamicDataFactory::get_instance().create_data(*alias_type));
    ASSERT_TRUE(data);
    ASSERT_TRUE(data2);

    DynamicPubSubType pubsubAliasType(*alias_type);
    payloadSize = static_cast<uint32_t>(pubsubAliasType.getSerializedSizeProvider(data.get())());
    SerializedPayload_t alias_payload(payloadSize);
    ASSERT_TRUE(pubsubAliasType.serialize(data.get(), &alias_payload));
    ASSERT_EQ(alias_payload.length, payloadSize);
    ASSERT_TRUE(pubsubAliasType.deserialize(&alias_payload, data2.get()));
    ASSERT_TRUE(data2->equals(*data));
   }
 */

TEST_F(DynamicTypesTests, DynamicType_nested_alias_unit_tests)
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
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);
    DynamicType::_ref_type nested_alias_type {builder->build()};
    ASSERT_TRUE(nested_alias_type);
    EXPECT_EQ(nested_alias_type->get_name(), nested_name);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
    ASSERT_TRUE(data);

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(1, ""), eprosima::fastdds::dds::RETCODE_OK);
    const std::string test1 {"STRING_TEST"};
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);

    int test = 0;
    ASSERT_NE(data->get_int32_value(test, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    std::string test2;
    ASSERT_NE(data->get_string_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(test1, test2);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID,
                "TEST_OVER_LENGTH_LIMITS"), eprosima::fastdds::dds::RETCODE_OK);
    }

    ASSERT_EQ(test1.length(), data->get_item_count());

    // XCDRv1
    {
        // Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(nested_alias_type);
        std::string test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(pubsubType.serialize(&data, &payload));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test1, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Test clear functions.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ("", test2);
    ASSERT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    ASSERT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ("", test2);

    DynamicDataFactory::get_instance()->delete_data(data);
}

/*
   TEST_F(DynamicTypesTests, DynamicType_bitset_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder { factory.create_byte_type()};
   ASSERT_TRUE(base_type_builder);
   auto base_type = base_type_builder->build();

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder2 { factory.create_uint32_type()};
   ASSERT_TRUE(base_type_builder2);
   auto base_type2 = base_type_builder2->build();

   std::unique_ptr<DynamicTypeBuilder> bitset_type_builder { factory.create_bitset_type()};
   ASSERT_TRUE(bitset_type_builder);

   // Add members to the struct.
   ASSERT_EQ(bitset_type_builder->add_member({0, "int2", base_type}), eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_EQ(bitset_type_builder->add_member({1, "int20", base_type2}), eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
     bitset_type_builder->apply_annotation_to_member(0, {ANNOTATION_BIT_BOUND, "value", "2"}));
   ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
     bitset_type_builder->apply_annotation_to_member(0, ANNOTATION_POSITION, "value", "0"));
   ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
     bitset_type_builder->apply_annotation_to_member(1, ANNOTATION_BIT_BOUND, "value", "20"));
   ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
     bitset_type_builder->apply_annotation_to_member(1, ANNOTATION_POSITION, "value", "10")); // 8 bits empty

   auto bitset_type = bitset_type_builder->build();
   ASSERT_TRUE(bitset_type);
   auto bitset_data = DynamicDataFactory::get_instance().create_data(*bitset_type);
   ASSERT_TRUE(bitset_data);

   ASSERT_FALSE(bitset_data->set_int32_value(10, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(bitset_data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // Set and get the child values.
   octet test1(234);
   ASSERT_TRUE(bitset_data->set_byte_value(test1, 0) == eprosima::fastdds::dds::RETCODE_OK);
   octet test2(0);
   ASSERT_TRUE(bitset_data->get_byte_value(test2, 0) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(test1 == test2);
   // 11101010
   // 00000010 (two bits)
   ASSERT_TRUE(test2 == 2);
   uint32_t test3(289582314);
   ASSERT_TRUE(bitset_data->set_uint32_value(test3, 1) == eprosima::fastdds::dds::RETCODE_OK);
   uint32_t test4(0);
   ASSERT_TRUE(bitset_data->get_uint32_value(test4, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(test3 == test4);
   // 00000001010000101010110011101010
   // 00000000000000101010110011101010 (20 bits)
   ASSERT_TRUE(test4 == 175338);

   // Bitset serialization
   // Tested in DynamicTypes_4_2_Tests
   }

   TEST_F(DynamicTypesTests, DynamicType_bitmask_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   uint32_t limit = 5;
   std::unique_ptr<DynamicTypeBuilder> created_builder { factory.create_bitmask_type(limit)};
   ASSERT_TRUE(created_builder);

   // Add two members to the bitmask
   ASSERT_EQ(created_builder->add_member({0, "TEST"}), eprosima::fastdds::dds::RETCODE_OK);

   // Try to add a descriptor with the same name
   {
   eprosima::fastdds::dds::Log::ScopeLogs _("disable");

   EXPECT_NE(created_builder->add_member({1, "TEST"}), eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_EQ(created_builder->add_member({1, "TEST2"}), eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_EQ(created_builder->add_member({4, "TEST4"}), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_NE(created_builder->add_member({5, "TEST5"}), eprosima::fastdds::dds::RETCODE_OK); // Out of bounds
   }

   std::unique_ptr<const DynamicType> created_type {created_builder->build()};
   ASSERT_TRUE(created_type);
   std::unique_ptr<DynamicData> data {DynamicDataFactory::get_instance().create_data(*created_type)};
   ASSERT_TRUE(data != nullptr);

   MemberId testId = data->get_member_by_name("TEST");
   EXPECT_NE(testId, MEMBER_ID_INVALID);
   EXPECT_EQ(testId, 0);
   MemberId test2Id = data->get_member_by_name("TEST2");
   EXPECT_NE(test2Id, MEMBER_ID_INVALID);
   EXPECT_EQ(test2Id, 1);
   MemberId test4Id = data->get_member_by_name("TEST4");
   EXPECT_NE(test4Id, MEMBER_ID_INVALID);
   EXPECT_EQ(test4Id, 4);
   MemberId test5Id = data->get_member_by_name("TEST5");
   EXPECT_EQ(test5Id, MEMBER_ID_INVALID);

   bool test1 = true;
   ASSERT_FALSE(data->set_int32_value(1, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(data->set_bool_value(test1, testId) == eprosima::fastdds::dds::RETCODE_OK);

   // Over the limit
   ASSERT_FALSE(data->set_bool_value(test1, MemberId{limit} + 1) == eprosima::fastdds::dds::RETCODE_OK);

   bool test2 = false;
   ASSERT_TRUE(data->get_bool_value(test2, 2) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test2 == false);
   ASSERT_TRUE(data->get_bool_value(test2, testId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);
   ASSERT_TRUE(data->get_bool_value(test2, testId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);
   bool test3 = data->get_bool_value("TEST");
   ASSERT_TRUE(test1 == test3);
   ASSERT_TRUE(data->set_bool_value(true, "TEST4") == eprosima::fastdds::dds::RETCODE_OK);
   bool test4 = data->get_bool_value("TEST4");
   ASSERT_TRUE(test4 == true);

   test1 = false;
   ASSERT_TRUE(data->set_bool_value(test1, testId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(data->get_bool_value(test2, test2Id) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(data->get_bool_value(test2, testId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);
   data->set_bitmask_value(55); // 00110111
   uint64_t value = data->get_bitmask_value();
   ASSERT_TRUE(value == 55);
   ASSERT_TRUE(data->get_bool_value("TEST"));
   ASSERT_TRUE(data->get_bool_value("TEST2"));
   ASSERT_TRUE(data->get_bool_value("TEST4"));
   data->set_bitmask_value(37); // 00100101
   ASSERT_TRUE(data->get_bool_value("TEST"));
   ASSERT_FALSE(data->get_bool_value("TEST2"));
   ASSERT_FALSE(data->get_bool_value("TEST4"));

   ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   //ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   //ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   int32_t iTest32;
   ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint32_t uTest32;
   ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   int16_t iTest16;
   ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint16_t uTest16;
   ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   int64_t iTest64;
   ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   //uint64_t uTest64;
   //ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   float fTest32;
   ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   double fTest64;
   ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   long double fTest128;
   ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   char cTest8;
   ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   wchar_t cTest16;
   ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   octet oTest;
   ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   bool bTest;
   ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const char* sTest;
   ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const wchar_t* wsTest;
   ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const char* sEnumTest;
   ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // Serialize <-> Deserialize Test
   //ASSERT_TRUE(data->set_bool_value(true, 0) == eprosima::fastdds::dds::RETCODE_OK);
   DynamicPubSubType pubsubType(*created_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*created_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*data));
   }
 */

TEST_F(DynamicTypesTests, DynamicType_sequence_unit_tests)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(
                                               factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32), length)};
    ASSERT_TRUE(builder);
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_INT32));
    member_descriptor->name("Wrong");
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    // Try to write on an empty position
    ASSERT_NE(data->set_int32_value(234, 1), eprosima::fastdds::dds::RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2, 3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->set_int32_values(0, {1, 2, 3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(4, 5), eprosima::fastdds::dds::RETCODE_OK);

    // Try to insert more than the limit.
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");

        ASSERT_NE(data->set_int32_values(0, {0, 1, 2, 3, 4, 5, 6}), eprosima::fastdds::dds::RETCODE_OK);
    }

    int32_t test1 {0};
    ASSERT_EQ(data->get_int32_value(test1, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(1, test1);
    ASSERT_EQ(data->get_int32_value(test1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(2, test1);
    ASSERT_EQ(data->get_int32_value(test1, 2), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(3, test1);
    ASSERT_EQ(data->get_int32_value(test1, 3), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(4, test1);
    ASSERT_EQ(data->get_int32_value(test1, 4), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(5, test1);

    Int32Seq test2;
    ASSERT_EQ(data->get_int32_values(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    Int32Seq test_all {{1, 2, 3, 4, 5}};
    ASSERT_EQ(test2, test_all);

    ASSERT_EQ(data->get_int32_values(test2, 2), eprosima::fastdds::dds::RETCODE_OK);
    Int32Seq test_less {{3, 4, 5}};
    ASSERT_EQ(test2, test_less);


    // Test get_item_count().
    ASSERT_EQ(length, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test_all, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(0, data->get_item_count());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->set_int32_values(0, {1, 2, 3}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(0, data->get_item_count());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->set_int32_values(0, {1, 2, 3}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(1));
    ASSERT_EQ(2, data->get_item_count());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({1, 3}));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_of_sequences_unit_tests)
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
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);

    auto seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2, 3}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    // Try to insert more than the limit.
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        seq_data = data->loan_value(2);
        ASSERT_FALSE(seq_data);
    }

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    UInt32Seq wrong_seq;
    ASSERT_NE(seq_data->get_uint32_values(wrong_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    Int32Seq good_seq;
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2, 3}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    // Test get_item_count().
    ASSERT_EQ(2, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 2, 3}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 2, 3}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(0, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(0, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(0));
    ASSERT_EQ(1, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_array_unit_tests)
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
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);
    // Try to write on an empty position
    ASSERT_NE(data->set_int32_value(234, 1), eprosima::fastdds::dds::RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2, 3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->set_int32_values(0, {1, 2, 3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(4, 5), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(5, 6), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(data->set_int32_value(6, 7), eprosima::fastdds::dds::RETCODE_OK);

    // Try to insert more than the limit.
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");

        ASSERT_NE(data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8, 9}), eprosima::fastdds::dds::RETCODE_OK);
    }

    int32_t test1 {0};
    ASSERT_EQ(data->get_int32_value(test1, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(1, test1);
    ASSERT_EQ(data->get_int32_value(test1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(2, test1);
    ASSERT_EQ(data->get_int32_value(test1, 2), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(3, test1);
    ASSERT_EQ(data->get_int32_value(test1, 3), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(4, test1);
    ASSERT_EQ(data->get_int32_value(test1, 4), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(5, test1);
    ASSERT_EQ(data->get_int32_value(test1, 5), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(6, test1);
    ASSERT_EQ(data->get_int32_value(test1, 6), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(7, test1);
    ASSERT_EQ(data->get_int32_value(test1, 7), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(0, test1);

    Int32Seq test2;
    ASSERT_EQ(data->get_int32_values(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
    Int32Seq test_all {{1, 2, 3, 4, 5, 6, 7, 0}};
    ASSERT_EQ(test2, test_all);

    ASSERT_EQ(data->get_int32_values(test2, 2), eprosima::fastdds::dds::RETCODE_OK);
    Int32Seq test_less {{3, 4, 5, 6, 7, 0}};
    ASSERT_EQ(test2, test_less);

    // Test get_item_count().
    ASSERT_EQ(8, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), eprosima::fastdds::dds::RETCODE_OK);
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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));
        ASSERT_EQ(data2->get_int32_values(test3, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(test_all, test3);
        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(8, data->get_item_count());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({0, 0, 0, 0, 0, 0, 0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(8, data->get_item_count());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({0, 0, 0, 0, 0, 0, 0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(1));
    ASSERT_EQ(8, data->get_item_count());
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->get_int32_values(test2, 0));
    ASSERT_EQ(test2, Int32Seq({1, 0, 3, 4, 5, 6, 7, 8}));

    DynamicDataFactory::get_instance()->delete_data(data);
}

TEST_F(DynamicTypesTests, DynamicType_array_of_arrays_unit_tests)
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
    ASSERT_EQ(builder->add_member(member_descriptor), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    ASSERT_NE(data->set_int32_value(MEMBER_ID_INVALID, 10), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_string_value(MEMBER_ID_INVALID, ""), eprosima::fastdds::dds::RETCODE_OK);

    ASSERT_NE(data->set_uint32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_NE(data->set_int32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);

    auto seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    // Try to insert more than the limit.
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        seq_data = data->loan_value(4);
        ASSERT_FALSE(seq_data);
    }

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    UInt32Seq wrong_seq;
    ASSERT_NE(seq_data->get_uint32_values(wrong_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    Int32Seq good_seq;
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({3, 4}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    // Test get_item_count().
    ASSERT_EQ(4, data->get_item_count());

    // XCDRv1
    {
        /// Serialize <-> Deserialize Test
        DynamicPubSubType pubsubType(created_type);
        Int32Seq test3;
        uint32_t payloadSize =
                static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(&data, XCDR_DATA_REPRESENTATION)());
        SerializedPayload_t payload(payloadSize);
        ASSERT_TRUE(pubsubType.serialize(&data, &payload, XCDR_DATA_REPRESENTATION));
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 2}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(2);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({3, 4}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(3);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 0}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

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
        ASSERT_TRUE(payload.length == payloadSize);
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        ASSERT_TRUE(pubsubType.deserialize(&payload, &data2));
        ASSERT_TRUE(data2->equals(data));

        seq_data = data->loan_value(0);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 2}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(1);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({0, 1}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(2);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({3, 4}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
        seq_data = data->loan_value(3);
        ASSERT_TRUE(seq_data);
        ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
        ASSERT_EQ(good_seq, Int32Seq({1, 0}));
        ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

        DynamicDataFactory::get_instance()->delete_data(data2);
    }

    // Remove the elements.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_all_values());
    ASSERT_EQ(4, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_nonkey_values());
    ASSERT_EQ(4, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(2));
    ASSERT_EQ(4, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {1, 2}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(1, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_values(0, {3, 4}), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_NE(seq_data->set_uint32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(seq_data->set_int32_value(0, 1), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->clear_value(3));
    ASSERT_EQ(4, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({1, 2}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 1}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({3, 4}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    ASSERT_EQ(seq_data->get_int32_values(good_seq, 0), eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(good_seq, Int32Seq({0, 0}));
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, data->return_loaned_value(seq_data));

    DynamicDataFactory::get_instance()->delete_data(data);
}
/*
   TEST_F(DynamicTypesTests, DynamicType_map_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   uint32_t map_length = 2;

   // Then
   std::unique_ptr<const DynamicTypeBuilder> base_type_builder { factory.create_int32_type()};
   ASSERT_TRUE(base_type_builder);
   std::unique_ptr<const DynamicType> base_type {base_type_builder->build()};

   std::unique_ptr<DynamicTypeBuilder> map_type_builder {factory.create_map_type(*base_type, *base_type, map_length)};
   ASSERT_TRUE(map_type_builder);
   auto map_type = map_type_builder->build();
   ASSERT_TRUE(map_type);

   eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

   std::unique_ptr<DynamicData> data {DynamicDataFactory::get_instance().create_data(*map_type)};

   ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to write on an empty position
   ASSERT_FALSE(data->set_int32_value(234, 0) == eprosima::fastdds::dds::RETCODE_OK);

   MemberId keyId;
   MemberId valueId;
   auto key_data = DynamicDataFactory::get_instance().create_data(*base_type);
   //TODO(richiware) ASSERT_TRUE(data->insert_map_data(key_data, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to Add the same key twice.
   //TODO(richiware) ASSERT_FALSE(data->insert_map_data(key_data, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   MemberId keyId2;
   MemberId valueId2;
   key_data = DynamicDataFactory::get_instance().create_data(*base_type);
   key_data->set_int32_value(2, MEMBER_ID_INVALID);
   //TODO(richiware) ASSERT_TRUE(data->insert_map_data(key_data, keyId2, valueId2) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to Add one more than the limit
   auto key_data2 = DynamicDataFactory::get_instance().create_data(*base_type);
   key_data2->set_int32_value(3, MEMBER_ID_INVALID);
   //TODO(richiware) ASSERT_FALSE(data->insert_map_data(key_data2, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   // Set and get a value.
   int32_t test1(234);
   ASSERT_TRUE(data->set_int32_value(test1, valueId) == eprosima::fastdds::dds::RETCODE_OK);
   int32_t test2(0);
   ASSERT_TRUE(data->get_int32_value(test2, valueId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(map_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*map_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*data));

   // Check items count with removes
   ASSERT_TRUE(data->get_item_count() == 2);
   ASSERT_FALSE(data->remove_map_data(valueId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(data->get_item_count() == 2);
   ASSERT_TRUE(data->remove_map_data(keyId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(data->get_item_count() == 1);
   ASSERT_TRUE(data->clear_all_values() == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(data->get_item_count() == 0);

   ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   int32_t iTest32;
   ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint32_t uTest32;
   ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   int16_t iTest16;
   ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint16_t uTest16;
   ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   int64_t iTest64;
   ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint64_t uTest64;
   ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   float fTest32;
   ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   double fTest64;
   ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   long double fTest128;
   ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   char cTest8;
   ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   wchar_t cTest16;
   ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   octet oTest;
   ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   bool bTest;
   ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const char* sTest;
   ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const wchar_t* wsTest;
   ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const char* sEnumTest;
   ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // SERIALIZATION TEST
   MapStruct seq;
   MapStructPubSubType seqpb;

   uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data.get())());
   SerializedPayload_t dynamic_payload(payloadSize3);
   ASSERT_TRUE(pubsubType.serialize(data.get(), &dynamic_payload));
   ASSERT_TRUE(dynamic_payload.length == payloadSize3);
   ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

   uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
   SerializedPayload_t static_payload(static_payloadSize);
   ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
   ASSERT_TRUE(static_payload.length == static_payloadSize);
   std::unique_ptr<DynamicData> data3 {DynamicDataFactory::get_instance().create_data(*map_type)};
   ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3.get()));
   ASSERT_TRUE(data3->equals(*data));
   }

   TEST_F(DynamicTypesTests, DynamicType_map_of_maps_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   uint32_t map_length = 2;

   // Then
   std::unique_ptr<const DynamicTypeBuilder> base_type_builder {factory.create_int32_type()};
   ASSERT_TRUE(base_type_builder);
   std::unique_ptr<const DynamicType> base_type {base_type_builder->build()};

   std::unique_ptr<DynamicTypeBuilder> map_type_builder {
   factory.create_map_type(*base_type, *base_type, map_length)};
   ASSERT_TRUE(map_type_builder);
   std::unique_ptr<const DynamicType> map_type {map_type_builder->build()};
   ASSERT_TRUE(map_type);

   std::unique_ptr<DynamicTypeBuilder> map_map_type_builder {
   factory.create_map_type(*base_type, *map_type, map_length)};
   ASSERT_TRUE(map_map_type_builder);
   std::unique_ptr<const DynamicType> map_map_type {map_map_type_builder->build()};
   ASSERT_TRUE(map_map_type);

   eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

   std::unique_ptr<DynamicData> data {DynamicDataFactory::get_instance().create_data(*map_map_type)};

   ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   MemberId keyId;
   MemberId valueId;
   auto key_data = DynamicDataFactory::get_instance().create_data(*base_type);
   //TODO(richiware) ASSERT_TRUE(data->insert_map_data(key_data, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to Add the same key twice.
   //TODO(richiware) ASSERT_FALSE(data->insert_map_data(key_data, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   MemberId keyId2;
   MemberId valueId2;
   key_data = DynamicDataFactory::get_instance().create_data(*base_type);
   key_data->set_int32_value(2);
   //TODO(richiware) ASSERT_TRUE(data->insert_map_data(key_data, keyId2, valueId2) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to Add one more than the limit
   std::unique_ptr<DynamicData> key_data2 {DynamicDataFactory::get_instance().create_data(*base_type)};
   key_data2->set_int32_value(3, MEMBER_ID_INVALID);
   //TODO(richiware) ASSERT_FALSE(data->insert_map_data(key_data2, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   auto seq_data = data->loan_value(valueId);
   ASSERT_TRUE(seq_data != nullptr);

   std::unique_ptr<DynamicData> key_data3 {DynamicDataFactory::get_instance().create_data(*base_type)};
   //TODO(richiware) ASSERT_TRUE(seq_data->insert_map_data(key_data3, keyId, valueId) == eprosima::fastdds::dds::RETCODE_OK);

   // Set and get a value.
   int32_t test1(234);
   ASSERT_TRUE(seq_data->set_int32_value(test1, valueId) == eprosima::fastdds::dds::RETCODE_OK);
   int32_t test2(0);
   ASSERT_TRUE(seq_data->get_int32_value(test2, valueId) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);

   ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   int32_t iTest32;
   ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint32_t uTest32;
   ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   int16_t iTest16;
   ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint16_t uTest16;
   ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   int64_t iTest64;
   ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   uint64_t uTest64;
   ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   float fTest32;
   ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   double fTest64;
   ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   long double fTest128;
   ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   char cTest8;
   ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   wchar_t cTest16;
   ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   octet oTest;
   ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   bool bTest;
   ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const char* sTest;
   ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const wchar_t* wsTest;
   ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);
   const char* sEnumTest;
   ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(map_map_type.get());
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*map_map_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*data));

   // SERIALIZATION TEST
   MapMapStruct seq;
   MapMapStructPubSubType seqpb;

   uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data.get())());
   SerializedPayload_t dynamic_payload(payloadSize3);
   ASSERT_TRUE(pubsubType.serialize(data.get(), &dynamic_payload));
   ASSERT_TRUE(dynamic_payload.length == payloadSize3);
   ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

   uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
   SerializedPayload_t static_payload(static_payloadSize);
   ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
   ASSERT_TRUE(static_payload.length == static_payloadSize);
   std::unique_ptr<DynamicData> data3 {DynamicDataFactory::get_instance().create_data(*map_map_type)};
   ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3.get()));
   ASSERT_TRUE(data3->equals(*data));
   }

   TEST_F(DynamicTypesTests, DynamicType_structure_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder { factory.create_int32_type()};
   ASSERT_TRUE(base_type_builder);
   std::unique_ptr<const DynamicType> base_type {base_type_builder->build()};

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder2 { factory.create_int64_type()};
   ASSERT_TRUE(base_type_builder2);
   std::unique_ptr<const DynamicType> base_type2 {base_type_builder2->build()};

   std::unique_ptr<DynamicTypeBuilder> struct_type_builder { factory.create_struct_type()};
   ASSERT_TRUE(struct_type_builder);

   // Add members to the struct.
   ASSERT_TRUE(struct_type_builder->add_member({0, "int32", base_type.get()}) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(struct_type_builder->add_member({1, "int64", base_type2.get()}) == eprosima::fastdds::dds::RETCODE_OK);

   std::unique_ptr<const DynamicType> struct_type {struct_type_builder->build()};
   ASSERT_TRUE(struct_type);
   std::unique_ptr<DynamicData> struct_data {DynamicDataFactory::get_instance().create_data(*struct_type)};
   ASSERT_TRUE(struct_data != nullptr);

   ASSERT_FALSE(struct_data->set_int32_value(10, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(struct_data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // Set and get the child values.
   int32_t test1(234);
   ASSERT_TRUE(struct_data->set_int32_value(test1, 0) == eprosima::fastdds::dds::RETCODE_OK);
   int32_t test2(0);
   ASSERT_TRUE(struct_data->get_int32_value(test2, 0) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);
   int64_t test3(234);
   ASSERT_TRUE(struct_data->set_int64_value(test3, 1) == eprosima::fastdds::dds::RETCODE_OK);
   int64_t test4(0);
   ASSERT_TRUE(struct_data->get_int64_value(test4, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test3 == test4);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(*struct_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(struct_data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*struct_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*struct_data));

   // SERIALIZATION TEST
   StructStruct seq;
   StructStructPubSubType seqpb;

   uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data.get())());
   SerializedPayload_t dynamic_payload(payloadSize3);
   ASSERT_TRUE(pubsubType.serialize(struct_data.get(), &dynamic_payload));
   ASSERT_TRUE(dynamic_payload.length == payloadSize3);
   ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

   uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
   SerializedPayload_t static_payload(static_payloadSize);
   ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
   ASSERT_TRUE(static_payload.length == static_payloadSize);
   std::unique_ptr<DynamicData> data3 {DynamicDataFactory::get_instance().create_data(*struct_type)};
   ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3.get()));
   ASSERT_TRUE(data3->equals(*struct_data));
   }

   TEST_F(DynamicTypesTests, DynamicType_structure_inheritance_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder { factory.create_int32_type()};
   ASSERT_TRUE(base_type_builder);
   std::unique_ptr<const DynamicType> base_type {base_type_builder->build()};

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder2 { factory.create_int64_type()};
   ASSERT_TRUE(base_type_builder2);
   std::unique_ptr<const DynamicType> base_type2 {base_type_builder2->build()};

   std::unique_ptr<DynamicTypeBuilder> struct_type_builder { factory.create_struct_type()};
   ASSERT_TRUE(struct_type_builder);

   // Add members to the struct.
   EXPECT_EQ(struct_type_builder->add_member({0, "int32", base_type.get()}), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(struct_type_builder->add_member({1, "int64", base_type2.get()}), eprosima::fastdds::dds::RETCODE_OK);

   std::unique_ptr<const DynamicType> struct_type {struct_type_builder->build()};
   ASSERT_TRUE(struct_type);

   // Create the child struct.
   std::unique_ptr<DynamicTypeBuilder> child_struct_type_builder { factory.create_child_struct_type(*struct_type)};
   ASSERT_TRUE(child_struct_type_builder);

   // Add a new member to the child struct.
   EXPECT_EQ(child_struct_type_builder->add_member({2, "child_int32",
                                              base_type.get()}), eprosima::fastdds::dds::RETCODE_OK);

   {
   eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging
   // try to add a member to override one of the parent struct.
   EXPECT_NE(child_struct_type_builder->add_member({3, "int32",
                                                  base_type.get()}), eprosima::fastdds::dds::RETCODE_OK);
   }

   // Add a new member at front
   EXPECT_EQ(child_struct_type_builder->add_member({0, "first_child",
                                              base_type2.get()}), eprosima::fastdds::dds::RETCODE_OK);

   // Add a new member at end
   EXPECT_EQ(child_struct_type_builder->add_member({INDEX_INVALID, "last_child",
                                              base_type2.get()}), eprosima::fastdds::dds::RETCODE_OK);

   std::unique_ptr<const DynamicType> child_struct_type {child_struct_type_builder->build()};
   ASSERT_TRUE(child_struct_type);

   // Validate the member related APIs

   EXPECT_EQ(child_struct_type->get_member_count(), 5u);

   MemberDescriptor members[5];

   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member(members[0], 0), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(members[0].get_name(), "int32");
   EXPECT_EQ(members[0].get_index(), 0u);
   EXPECT_EQ(members[0].get(), 0);
   EXPECT_EQ(*members[0].get_type(), *base_type);

   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member(members[1], 1), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(members[1].get_name(), "int64");
   EXPECT_EQ(members[1].get_index(), 1u);
   EXPECT_EQ(members[1].get(), 1);
   EXPECT_EQ(*members[1].get_type(), *base_type2);

   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member(members[2], 3), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(members[2].get_name(), "first_child");
   EXPECT_EQ(members[2].get_index(), 2u);
   EXPECT_EQ(members[2].get(), 3);
   EXPECT_EQ(*members[2].get_type(), *base_type2);

   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member(members[3], 2), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(members[3].get_name(), "child_int32");
   EXPECT_EQ(members[3].get_index(), 3u);
   EXPECT_EQ(members[3].get(), 2);
   EXPECT_EQ(*members[3].get_type(), *base_type);

   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member(members[4], 4), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(members[4].get_name(), "last_child");
   EXPECT_EQ(members[4].get_index(), 4u);
   EXPECT_EQ(members[4].get(), 4);
   EXPECT_EQ(*members[4].get_type(), *base_type2);

   for (auto& m : members)
   {
   //TODO(richiware) EXPECT_TRUE(child_struct_type->exists_member_by_name(m.get_name()));
   //TODO(richiware) EXPECT_TRUE(child_struct_type->exists_member_by(m.get()));
   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member_by_name(m.get_name()), m.get());
   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member_at_index(m.get_index()), m.get());

   MemberDescriptor aux;
   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member_by_index(aux, m.get_index()), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(aux, m);

   //TODO(richiware) EXPECT_EQ(child_struct_type->get_member_by_name(aux, m.get_name()), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(aux, m);
   }

   auto member_seq = child_struct_type->get_all_members();
   EXPECT_EQ(member_seq.size(), 5u);

   EXPECT_TRUE(std::equal(member_seq.begin(), member_seq.end(), members,
     [](const MemberDescriptor* a, const MemberDescriptor& b) -> bool
     {
         return *a == b;
     }));

   // ancillary collection
   std::set<MemberDescriptor, DynamicTypesTests::order_member_desc> aux;

   auto name_map = child_struct_type->get_all_members_by_name();
   EXPECT_EQ(name_map.size(), 5u);
   std::transform(name_map.begin(), name_map.end(), std::inserter(aux, aux.end()),
     [](std::pair<const std::string, const DynamicTypeMember*>& p)
     {
         return *p.second;
     });
   EXPECT_TRUE(std::equal(aux.begin(), aux.end(), members));

   aux.clear();
   auto id_map = child_struct_type->get_all_members_by();
   EXPECT_EQ(id_map.size(), 5u);
   std::transform(id_map.begin(), id_map.end(), std::inserter(aux, aux.end()),
     [](std::pair<const MemberId, const DynamicTypeMember*>& p)
     {
         return *p.second;
     });
   EXPECT_TRUE(std::equal(aux.begin(), aux.end(), members));

   // Validating data management

   std::unique_ptr<DynamicData> struct_data {DynamicDataFactory::get_instance().create_data(*child_struct_type)};
   ASSERT_TRUE(struct_data != nullptr);

   // Setting invalid types should fail
   EXPECT_NE(struct_data->set_int32_value(10, 1), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_NE(struct_data->set_string_value("", MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

   // Set and get the parent values.
   int32_t test1(234);
   EXPECT_EQ(struct_data->set_int32_value(test1, 0), eprosima::fastdds::dds::RETCODE_OK);
   int32_t test2(0);
   EXPECT_EQ(struct_data->get_int32_value(test2, 0), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(test1, test2);

   int64_t test3(234);
   EXPECT_EQ(struct_data->set_int64_value(test3, 1), eprosima::fastdds::dds::RETCODE_OK);
   int64_t test4(0);
   EXPECT_EQ(struct_data->get_int64_value(test4, 1), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(test3, test4);

   // Set and get the child value.
   EXPECT_EQ(struct_data->set_int32_value(test1, 2), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(struct_data->get_int32_value(test2, 2), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(test1, test2);

   EXPECT_EQ(struct_data->set_int64_value(test3, 3), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(struct_data->get_int64_value(test4, 3), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(test3, test4);

   EXPECT_EQ(struct_data->set_int64_value(test3, 4), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(struct_data->get_int64_value(test4, 4), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(test3, test4);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(*child_struct_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(struct_data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*child_struct_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*struct_data));
   }

   TEST_F(DynamicTypesTests, DynamicType_multi_structure_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder {factory.create_int32_type()};
   ASSERT_TRUE(base_type_builder);
   std::unique_ptr<const DynamicType> base_type {base_type_builder->build()};

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder2 {factory.create_int64_type()};
   ASSERT_TRUE(base_type_builder2);
   std::unique_ptr<const DynamicType> base_type2 {base_type_builder2->build()};

   std::unique_ptr<DynamicTypeBuilder> struct_type_builder { factory.create_struct_type()};
   ASSERT_TRUE(struct_type_builder);

   // Add members to the struct.
   ASSERT_TRUE(struct_type_builder->add_member({0, "int32", base_type.get()}) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(struct_type_builder->add_member({1, "int64", base_type2.get()}) == eprosima::fastdds::dds::RETCODE_OK);

   std::unique_ptr<const DynamicType> struct_type {struct_type_builder->build()};
   ASSERT_TRUE(struct_type);

   // Create the parent struct.
   std::unique_ptr<DynamicTypeBuilder> parent_struct_type_builder { factory.create_struct_type()};
   ASSERT_TRUE(parent_struct_type_builder);

   // Add members to the parent struct.
   ASSERT_TRUE(parent_struct_type_builder->add_member({0, "child_struct",
                                                 struct_type.get()}) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(parent_struct_type_builder->add_member({1, "child_int64",
                                                 base_type2.get()}) == eprosima::fastdds::dds::RETCODE_OK);

   auto parent_struct_type = parent_struct_type_builder->build();
   ASSERT_TRUE(parent_struct_type);

   std::unique_ptr<DynamicData> struct_data {DynamicDataFactory::get_instance().create_data(*parent_struct_type)};
   ASSERT_TRUE(struct_data != nullptr);

   ASSERT_FALSE(struct_data->set_int32_value(10, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(struct_data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   // Set and get the child values.
   int64_t test1(234);
   ASSERT_TRUE(struct_data->set_int64_value(test1, 1) == eprosima::fastdds::dds::RETCODE_OK);
   int64_t test2(0);
   ASSERT_TRUE(struct_data->get_int64_value(test2, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);

   auto child_struct_data = struct_data->loan_value(0);
   ASSERT_TRUE(child_struct_data != nullptr);

   // Set and get the child values.
   int32_t test3(234);
   ASSERT_TRUE(child_struct_data->set_int32_value(test3, 0) == eprosima::fastdds::dds::RETCODE_OK);
   int32_t test4(0);
   ASSERT_TRUE(child_struct_data->get_int32_value(test4, 0) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test3 == test4);
   int64_t test5(234);
   ASSERT_TRUE(child_struct_data->set_int64_value(test5, 1) == eprosima::fastdds::dds::RETCODE_OK);
   int64_t test6(0);
   ASSERT_TRUE(child_struct_data->get_int64_value(test6, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test5 == test6);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(parent_struct_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(struct_data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*parent_struct_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*struct_data));

   // SERIALIZATION TEST
   StructStructStruct seq;
   StructStructStructPubSubType seqpb;

   uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data.get())());
   SerializedPayload_t dynamic_payload(payloadSize3);
   ASSERT_TRUE(pubsubType.serialize(struct_data.get(), &dynamic_payload));
   ASSERT_TRUE(dynamic_payload.length == payloadSize3);
   ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

   uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
   SerializedPayload_t static_payload(static_payloadSize);
   ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
   ASSERT_TRUE(static_payload.length == static_payloadSize);
   std::unique_ptr<DynamicData> data3 {DynamicDataFactory::get_instance().create_data(*parent_struct_type)};
   ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3.get()));
   ASSERT_TRUE(data3->equals(*struct_data));
   }

   TEST_F(DynamicTypesTests, DynamicType_union_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicTypeBuilder> discriminant_builder { factory.create_int32_type()};
   ASSERT_TRUE(discriminant_builder);
   std::unique_ptr<const DynamicType> discriminant_type {discriminant_builder->build()};
   ASSERT_TRUE(discriminant_type);


   std::unique_ptr<const DynamicTypeBuilder> another_member_builder { factory.create_int64_type()};
   std::unique_ptr<const DynamicType> another_member_type {another_member_builder->build()};
   ASSERT_TRUE(another_member_type);

   std::unique_ptr<DynamicTypeBuilder> union_type_builder { factory.create_union_type(*discriminant_type)};
   ASSERT_TRUE(union_type_builder);

   // Add members to the union.
   // A plain braced-init-list cannot be used for the labels because that would inhibit
   // template argument deduction, see § 14.8.2.5/5 of the C++11 standard
   ASSERT_EQ(union_type_builder.add_member(0, "first",
     discriminant_type.get(), "", std::vector<uint64_t>{ 0 }, true),
     eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_EQ(union_type_builder.add_member(1, "second", another_member_type, "", std::vector<uint64_t>{ 1 },
     false),
     eprosima::fastdds::dds::RETCODE_OK);

   {
   eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

   // Try to add a second "DEFAULT" value to the union
   ASSERT_FALSE(union_type_builder.add_member(3, "third", discrimitor_type.get(), "",
         std::vector<uint64_t>{ 0 },
         true) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to add a second value to the same case label
   ASSERT_FALSE(union_type_builder.add_member(4, "third", discrimitor_type.get(), "",
         std::vector<uint64_t>{ 1 },
         false) == eprosima::fastdds::dds::RETCODE_OK);
   }

   // Create a data of this union
   std::unique_ptr<const DynamicType> union_type {union_type_builder->build()};
   ASSERT_TRUE(union_type);
   std::unique_ptr<DynamicData> union_data {DynamicDataFactory::get_instance().create_data(*union_type)};
   ASSERT_TRUE(union_data != nullptr);

   // Set and get the child values.
   ASSERT_FALSE(union_data->set_int32_value(10, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(union_data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   uint64_t label;
   ASSERT_TRUE(union_data->get_union_label(label) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(label == 0);

   int32_t test1(234);
   ASSERT_TRUE(union_data->set_int32_value(test1, 0) == eprosima::fastdds::dds::RETCODE_OK);
   int32_t test2(0);
   ASSERT_TRUE(union_data->get_int32_value(test2, 0) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);
   ASSERT_TRUE(union_data->get_union_label(label) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(label == 0);

   int64_t test3(234);
   int64_t test4(0);

   // Try to get values from invalid indexes and from an invalid element ( not the current one )
   ASSERT_FALSE(union_data->get_int32_value(test2, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(union_data->get_int64_value(test4, 1) == eprosima::fastdds::dds::RETCODE_OK);

   ASSERT_TRUE(union_data->set_int64_value(test3, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(union_data->get_int64_value(test4, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test3 == test4);
   ASSERT_TRUE(union_data->get_union_label(label) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(label == 1);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(*union_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(union_data.get(), &payload));
   ASSERT_TRUE(payload.length == payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*union_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*union_data));

   // SERIALIZATION TEST
   SimpleUnionStruct seq;
   SimpleUnionStructPubSubType seqpb;

   SerializedPayload_t dynamic_payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(union_data.get(), &dynamic_payload));
   ASSERT_TRUE(dynamic_payload.length == payloadSize);
   ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

   uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
   SerializedPayload_t static_payload(static_payloadSize);
   ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
   ASSERT_TRUE(static_payload.length == static_payloadSize);
   std::unique_ptr<DynamicData> data3 {DynamicDataFactory::get_instance().create_data(*union_type)};
   ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3.get()));
   ASSERT_TRUE(data3->equals(*union_data));
   }

   TEST_F(DynamicTypesTests, DynamicType_union_with_unions_unit_tests)
   {
   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder { factory.create_int32_type()};
   ASSERT_TRUE(base_type_builder);
   std::unique_ptr<const DynamicType> base_type {base_type_builder->build()};

   std::unique_ptr<const DynamicTypeBuilder> base_type_builder2 { factory.create_int64_type()};
   ASSERT_TRUE(base_type_builder2);
   std::unique_ptr<const DynamicType> base_type2 {base_type_builder2->build()};

   std::unique_ptr<DynamicTypeBuilder> union_type_builder { factory.create_union_type(*base_type)};
   ASSERT_TRUE(union_type_builder);

   // Add members to the union.
   ASSERT_TRUE(union_type_builder->add_member(0, "first", base_type, "", std::vector<uint64_t>{ 0 },
     true) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(union_type_builder->add_member(1, "second", base_type2, "", std::vector<uint64_t>{ 1 },
     false) == eprosima::fastdds::dds::RETCODE_OK);

   {
   eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

   // Try to add a second "DEFAULT" value to the union
   ASSERT_FALSE(union_type_builder->add_member(3, "third", base_type, "", std::vector<uint64_t>{ 0 },
         true) == eprosima::fastdds::dds::RETCODE_OK);

   // Try to add a second value to the same case label
   ASSERT_FALSE(union_type_builder->add_member(4, "third", base_type, "", std::vector<uint64_t>{ 1 },
         false) == eprosima::fastdds::dds::RETCODE_OK);
   }

   // Create a data of this union
   std::unique_ptr<const DynamicType> union_type {union_type_builder->build()};
   ASSERT_TRUE(union_type != nullptr);

   std::unique_ptr<DynamicTypeBuilder> parent_union_type_builder { factory.create_union_type(*base_type)};
   ASSERT_TRUE(parent_union_type_builder);

   // Add Members to the parent union
   ASSERT_TRUE(parent_union_type_builder->add_member(0, "first", base_type, "", std::vector<uint64_t>{ 0 },
     true) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(parent_union_type_builder->add_member(1, "second", union_type, "", std::vector<uint64_t>{ 1 },
     false) == eprosima::fastdds::dds::RETCODE_OK);

   std::unique_ptr<const DynamicType> created_type {parent_union_type_builder->build()};
   ASSERT_TRUE(created_type);
   std::unique_ptr<DynamicData> union_data {DynamicDataFactory::get_instance().create_data(
 * parent_union_type_builder.get()->build())};
   ASSERT_TRUE(union_data != nullptr);

   // Set and get the child values.
   ASSERT_FALSE(union_data->set_int32_value(10, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(union_data->set_string_value("", MEMBER_ID_INVALID) == eprosima::fastdds::dds::RETCODE_OK);

   uint64_t label;
   ASSERT_TRUE(union_data->get_union_label(label) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(label == 0);

   int32_t test1(234);
   ASSERT_TRUE(union_data->set_int32_value(test1, 0) == eprosima::fastdds::dds::RETCODE_OK);
   int32_t test2(0);
   ASSERT_TRUE(union_data->get_int32_value(test2, 0) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test1 == test2);
   ASSERT_TRUE(union_data->get_union_label(label) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(label == 0);

   // Loan Value ( Activates this union id )
   std::unique_ptr<DynamicData> child_data {union_data->loan_value(1)};
   ASSERT_TRUE(child_data != 0);

   int64_t test3(234);
   int64_t test4(0);

   // Try to get values from invalid indexes and from an invalid element ( not the current one )
   ASSERT_FALSE(child_data->get_int32_value(test2, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_FALSE(child_data->get_int64_value(test4, 1) == eprosima::fastdds::dds::RETCODE_OK);

   ASSERT_TRUE(child_data->set_int64_value(test3, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(child_data->get_int64_value(test4, 1) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(test3 == test4);

   ASSERT_TRUE(union_data->get_union_label(label) == eprosima::fastdds::dds::RETCODE_OK);
   ASSERT_TRUE(label == 1);

   // Serialize <-> Deserialize Test
   DynamicPubSubType pubsubType(*created_type);
   uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data.get())());
   SerializedPayload_t payload(payloadSize);
   ASSERT_TRUE(pubsubType.serialize(union_data.get(), &payload));
   EXPECT_EQ(payload.length, payloadSize);

   std::unique_ptr<DynamicData> data2 {DynamicDataFactory::get_instance().create_data(*created_type)};
   ASSERT_TRUE(pubsubType.deserialize(&payload, data2.get()));
   ASSERT_TRUE(data2->equals(*union_data));
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_EnumStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);
   {
   auto pbType = XMLProfileManager::CreateDynamicPubSubType("EnumStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Enum
   std::unique_ptr<DynamicTypeBuilder> enum_builder { factory.create_enum_type()};
   enum_builder->add_member({0, "A"});
   enum_builder->add_member({1, "B"});
   enum_builder->add_member({2, "C"});
   enum_builder->set_name("MyEnum");

   // Struct EnumStruct
   std::unique_ptr<DynamicTypeBuilder> es_builder {factory.create_struct_type()};
   es_builder->add_member({0, "my_enum", enum_builder->build()});
   es_builder->set_name("EnumStruct");

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   //TODO(richiware) ASSERT_EQ(*type, *es_builder);

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_AliasStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("AliasStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Enum
   std::unique_ptr<DynamicTypeBuilder> enum_builder { factory.create_enum_type()};
   enum_builder->add_member({0, "A"});
   enum_builder->add_member({1, "B"});
   enum_builder->add_member({2, "C"});
   enum_builder->set_name("MyEnum");
   std::unique_ptr<const DynamicType> enum_type {enum_builder->build()};

   // Alias
   std::unique_ptr<DynamicTypeBuilder> alias_builder { factory.create_alias_type(*enum_type, "MyAliasEnum")};
   std::unique_ptr<const DynamicType> alias_type {alias_builder->build()};

   // Struct AliasStruct
   std::unique_ptr<DynamicTypeBuilder> struct_alias_builder {factory.create_struct_type()};
   struct_alias_builder->add_member({0, "my_alias", alias_type.get()});
   struct_alias_builder->set_name("AliasStruct");
   std::unique_ptr<const DynamicType> struct_alias_type {struct_alias_builder->build()};

   std::unique_ptr<const DynamicType> type;
   // TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *struct_alias_type);
   EXPECT_TRUE(type->equals(*struct_alias_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_AliasAliasStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("AliasAliasStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Enum
   std::unique_ptr<DynamicTypeBuilder> enum_builder {factory.create_enum_type()};
   enum_builder->add_member({0, "A"});
   enum_builder->add_member({1, "B"});
   enum_builder->add_member({2, "C"});
   enum_builder->set_name("MyEnum");
   std::unique_ptr<const DynamicType> enum_type {enum_builder->build()};

   // Alias and aliasalias
   std::unique_ptr<DynamicTypeBuilder> alias_builder { factory.create_alias_type(*enum_type, "MyAliasEnum")};
   std::unique_ptr<const DynamicType> alias_type {alias_builder->build()};
   std::unique_ptr<DynamicTypeBuilder> alias_alias_builder { factory.create_alias_type(*alias_type,
                                                               "MyAliasAliasEnum")};
   std::unique_ptr<const DynamicType> alias_alias_type {alias_alias_builder->build()};

   // Struct AliasAliasStruct
   std::unique_ptr<DynamicTypeBuilder> aliasAliasS_builder { factory.create_struct_type()};
   aliasAliasS_builder->add_member({0, "my_alias_alias", alias_alias_type.get()});
   aliasAliasS_builder->set_name("AliasAliasStruct");
   std::unique_ptr<const DynamicType> aliasAliasS_type {aliasAliasS_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *aliasAliasS_type);
   EXPECT_TRUE(type->equals(*aliasAliasS_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_BoolStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("BoolStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Boolean
   std::unique_ptr<const DynamicType> boolean_type {factory.get_bool_type()};

   // Struct BoolStruct
   std::unique_ptr<DynamicTypeBuilder> bool_builder {factory.create_struct_type()};
   bool_builder->add_member({0, "my_bool", boolean_type.get()});
   bool_builder->set_name("BoolStruct");
   std::unique_ptr<const DynamicType> bool_type {bool_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *bool_type);
   EXPECT_TRUE(type->equals(*bool_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_OctetStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("OctetStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Byte
   std::unique_ptr<const DynamicType> byte_type { factory.get_byte_type()};

   // Struct OctetStruct
   std::unique_ptr<DynamicTypeBuilder> octet_builder { factory.create_struct_type()};
   octet_builder->add_member({0, "my_octet", byte_type.get()});
   octet_builder->set_name("OctetStruct");
   std::unique_ptr<const DynamicType> octet_type {octet_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *octet_type);
   EXPECT_TRUE(type->equals(*octet_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ShortStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Int16
   std::unique_ptr<const DynamicType> byte_type { factory.get_int16_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> int16_builder { factory.create_struct_type()};
   int16_builder->add_member({0, "my_int16", byte_type.get()});
   int16_builder->set_name("ShortStruct");
   std::unique_ptr<const DynamicType> int16_type {int16_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware)ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *int16_type);
   EXPECT_TRUE(type->equals(*int16_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_LongStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("LongStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Int32
   std::unique_ptr<const DynamicType> byte_type { factory.get_int32_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> int32_builder {factory.create_struct_type()};
   int32_builder->add_member({0, "my_int32", byte_type.get()});
   int32_builder->set_name("LongStruct");
   std::unique_ptr<const DynamicType> int32_type {int32_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *int32_type);
   EXPECT_TRUE(type->equals(*int32_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_LongLongStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("LongLongStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Int32
   std::unique_ptr<const DynamicType> byte_type { factory.get_int64_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> int64_builder { factory.create_struct_type()};
   int64_builder->add_member({0, "my_int64", byte_type.get()});
   int64_builder->set_name("LongLongStruct");
   std::unique_ptr<const DynamicType> int64_type {int64_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *int64_type);
   EXPECT_TRUE(type->equals(*int64_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_UShortStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("UShortStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // uint16
   std::unique_ptr<const DynamicType> byte_type {factory.get_uint16_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> uint16_builder {factory.create_struct_type()};
   uint16_builder->add_member({0, "my_uint16", byte_type.get()});
   uint16_builder->set_name("UShortStruct");
   std::unique_ptr<const DynamicType> uint16_type {uint16_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *uint16_type);
   EXPECT_TRUE(type->equals(*uint16_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ULongStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ULongStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // uint32
   std::unique_ptr<const DynamicType> byte_type { factory.get_uint32_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> uint32_builder {factory.create_struct_type()};
   uint32_builder->add_member({0, "my_uint32", byte_type.get()});
   uint32_builder->set_name("ULongStruct");
   std::unique_ptr<const DynamicType> uint32_type {uint32_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *uint32_type);
   EXPECT_TRUE(type->equals(*uint32_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ULongLongStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ULongLongStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // uint64
   std::unique_ptr<const DynamicType> byte_type {factory.get_uint64_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> uint64_builder {factory.create_struct_type()};
   uint64_builder->add_member({0, "my_uint64", byte_type.get()});
   uint64_builder->set_name("ULongLongStruct");
   std::unique_ptr<const DynamicType> uint64_type {uint64_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *uint64_type);
   EXPECT_TRUE(type->equals(*uint64_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_FloatStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("FloatStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // float32
   std::unique_ptr<const DynamicType> byte_type {factory.get_float32_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> float32_builder {factory.create_struct_type()};
   float32_builder->add_member({0, "my_float32", byte_type.get()});
   float32_builder->set_name("FloatStruct");
   std::unique_ptr<const DynamicType> float32_type {float32_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *float32_type);
   EXPECT_TRUE(type->equals(*float32_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_DoubleStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("DoubleStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // float64
   std::unique_ptr<const DynamicType> byte_type {factory.get_float64_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> float64_builder {factory.create_struct_type()};
   float64_builder->add_member({0, "my_float64", byte_type.get()});
   float64_builder->set_name("DoubleStruct");
   std::unique_ptr<const DynamicType> float64_type {float64_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *float64_type);
   EXPECT_TRUE(type->equals(*float64_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_LongDoubleStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("LongDoubleStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // float128
   std::unique_ptr<const DynamicType> byte_type {factory.get_float128_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> float128_builder {factory.create_struct_type()};
   float128_builder->add_member({0, "my_float128", byte_type.get()});
   float128_builder->set_name("LongDoubleStruct");
   std::unique_ptr<const DynamicType> float128_type {float128_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *float128_type);
   EXPECT_TRUE(type->equals(*float128_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_CharStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("CharStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // char8
   std::unique_ptr<const DynamicType> byte_type {factory.get_char8_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> char8_builder {factory.create_struct_type()};
   char8_builder->add_member({0, "my_char", byte_type.get()});
   char8_builder->set_name("CharStruct");
   std::unique_ptr<const DynamicType> char8_type {char8_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *char8_type);
   EXPECT_TRUE(type->equals(*char8_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_WCharStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("WCharStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // wchar
   std::unique_ptr<const DynamicType> byte_type { factory.get_char16_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> char16_builder { factory.create_struct_type()};
   char16_builder->add_member({0, "my_wchar", byte_type.get()});
   char16_builder->set_name("WCharStruct");
   std::unique_ptr<const DynamicType> char16_type {char16_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *char16_type);
   EXPECT_TRUE(type->equals(*char16_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_StringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("StringStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // string
   std::unique_ptr<const DynamicType> byte_type {factory.get_string_type()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> string_builder {factory.create_struct_type()};
   string_builder->add_member({0, "my_string", byte_type.get()});
   string_builder->set_name("StringStruct");
   std::unique_ptr<const DynamicType> string_type {string_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *string_type);
   EXPECT_TRUE(type->equals(*string_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_WStringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("WStringStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // wstring
   std::unique_ptr<DynamicTypeBuilder> string_builder {factory.create_type(
 * factory.create_wstring_type()->get_descriptor())};
   std::unique_ptr<const DynamicType> string_type {string_builder->build()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> wstring_builder { factory.create_struct_type()};
   wstring_builder->add_member({0, "my_wstring", string_type.get()});
   wstring_builder->set_name("WStringStruct");
   std::unique_ptr<const DynamicType> wstring_type {wstring_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *wstring_type);
   EXPECT_TRUE(type->equals(*wstring_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_LargeStringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("LargeStringStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // string
   std::unique_ptr<const DynamicType> byte_type {factory.get_string_type(41925)};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> string_builder {factory.create_struct_type()};
   string_builder->add_member({0, "my_large_string", byte_type.get()});
   string_builder->set_name("LargeStringStruct");
   std::unique_ptr<const DynamicType> string_type {string_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *string_type);
   EXPECT_TRUE(type->equals(*string_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_LargeWStringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("LargeWStringStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // wstring
   std::unique_ptr<DynamicTypeBuilder> string_builder {factory.create_type(*factory.create_wstring_type(41925))};
   string_builder->set_name("wstringl_41925");
   std::unique_ptr<const DynamicType> string_type {string_builder->build()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> wstring_builder {factory.create_struct_type()};
   wstring_builder->add_member({0, "my_large_wstring", string_type.get()});
   wstring_builder->set_name("LargeWStringStruct");
   std::unique_ptr<const DynamicType> wstring_type {wstring_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *wstring_type);
   EXPECT_TRUE(type->equals(*wstring_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ShortStringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStringStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // string
   std::unique_ptr<const DynamicType> byte_type {factory.get_string_type(15)};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> string_builder {factory.create_struct_type()};
   string_builder->add_member({0, "my_short_string", byte_type.get()});
   string_builder->set_name("ShortStringStruct");
   std::unique_ptr<const DynamicType> string_type {string_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *string_type);
   EXPECT_TRUE(type->equals(*string_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ShortWStringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortWStringStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // wstring
   std::unique_ptr<DynamicTypeBuilder> string_builder {factory.create_type(*factory.create_wstring_type(15))};
   string_builder->set_name("wstrings_15");
   std::unique_ptr<const DynamicType> string_type {string_builder->build()};

   // Struct ShortStruct
   std::unique_ptr<DynamicTypeBuilder> wstring_builder {factory.create_struct_type()};
   wstring_builder->add_member({0, "my_short_wstring", string_type.get()});
   wstring_builder->set_name("ShortWStringStruct");
   std::unique_ptr<const DynamicType> wstring_type {wstring_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *wstring_type);
   EXPECT_TRUE(type->equals(*wstring_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_AliasStringStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructAliasString");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // String
   std::unique_ptr<const DynamicType> string_type {factory.get_string_type()};

   // Alias
   std::unique_ptr<const DynamicType> myAlias_type {factory.get_alias_type(*string_type, "MyAliasString")};

   // Struct StructAliasString
   std::unique_ptr<DynamicTypeBuilder> alias_string_builder { factory.create_struct_type()};
   alias_string_builder->add_member({0, "my_alias_string", myAlias_type.get()});
   alias_string_builder->set_name("StructAliasString");
   std::unique_ptr<const DynamicType> alias_string_type {alias_string_builder->build()};

   std::unique_ptr<const DynamicType> type;
   //TODO(richiware) ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *alias_string_type);
   EXPECT_TRUE(type->equals(*alias_string_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_StructAliasWString_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);
   {
   auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructAliasWString");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // wstring
   std::unique_ptr<DynamicTypeBuilder> wstring_builder { factory.create_type(
 * factory.get_wstring_type())};
   std::unique_ptr<const DynamicType> wstring_type = wstring_builder.build();

   // Alias
   std::unique_ptr<const DynamicType> myAlias_type =
         factory.get_alias_type(*wstring_type, "MyAliasWString");

   // Struct StructAliasWString
   std::unique_ptr<DynamicTypeBuilder> alias_wstring_builder { factory.create_struct_type()};
   alias_wstring_builder.add_member(0, "my_alias_wstring", myAlias_type);
   alias_wstring_builder->set_name("StructAliasWString");
   std::unique_ptr<const DynamicType> alias_wstring_type = alias_wstring_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *alias_wstring_type);
   EXPECT_TRUE(type->equals(*alias_wstring_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ArraytStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ArraytStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Int32
   std::unique_ptr<const DynamicType> int32_type = factory.get_int32_type();

   // Array
   std::unique_ptr<DynamicTypeBuilder> array_builder { factory.create_array_type(*int32_type, { 2, 2, 2 })};

   // Struct ShortWStringStruct
   std::unique_ptr<DynamicTypeBuilder> array_int32_builder { factory.create_struct_type()};
   array_int32_builder.add_member(0, "my_array", array_builder.build());
   array_int32_builder->set_name("ArraytStruct");
   std::unique_ptr<const DynamicType> array_int32_type = array_int32_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *array_int32_type);
   EXPECT_TRUE(type->equals(*array_int32_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ArrayArrayStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Typedef aka Alias
   std::unique_ptr<DynamicTypeBuilder> array_builder { factory.create_array_type(
 * factory.get_int32_type(),
                                                     { 2, 2 })};
   std::unique_ptr<DynamicTypeBuilder> myArray_builder { factory.create_alias_type(*array_builder.build(), "MyArray")};

   // Struct ArrayArrayStruct
   std::unique_ptr<DynamicTypeBuilder> aas_builder { factory.create_struct_type()};
   std::unique_ptr<DynamicTypeBuilder> aMyArray_builder { factory.create_array_type(*myArray_builder.build(),
                                                            { 2, 2 })};
   aas_builder.add_member(0, "my_array_array", aMyArray_builder.build());
   aas_builder->set_name("ArrayArrayStruct");
   std::unique_ptr<const DynamicType> aas_type = aas_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *aas_type);
   EXPECT_TRUE(type->equals(*aas_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_ArrayArrayArrayStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayArrayStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   //      Manual comparision test

   //           typedef long MyArray[2][2];
   //
   //           struct ArrayArrayStruct
   //           {
   //            MyArray my_array_array[2][2];
   //           };
   //
   //           struct ArrayArrayArrayStruct
   //           {
   //            ArrayArrayStruct my_array_array_array[2][2];
   //           };
   //
   //           ======
   //
   //           <type>
   //            <typedef name="MyArray" type="int32" arrayDimensions="2,2"/>
   //           </type>
   //           <type>
   //            <struct name="ArrayArrayStruct">
   //                <member name="my_array_array" type="nonBasic" nonBasicTypeName="MyArray" arrayDimensions="2,2"/>
   //            </struct>
   //           </type>
   //           <type>
   //            <struct name="ArrayArrayArrayStruct">
   //                <member name="my_array_array_array" type="nonBasic" nonBasicTypeName="ArrayArrayStruct" arrayDimensions="2,2"/>
   //            </struct>
   //           </type>

   // Typedef aka Alias
   std::unique_ptr<DynamicTypeBuilder> array_builder { factory.create_array_type(
 * factory.get_int32_type(),
                                                     { 2, 2 })};
   std::unique_ptr<DynamicTypeBuilder> myArray_builder { factory.create_alias_type(
 * array_builder.build(),
                                                       "MyArray")};

   // Struct ArrayArrayStruct
   std::unique_ptr<DynamicTypeBuilder> aas_builder { factory.create_struct_type()};
   std::unique_ptr<DynamicTypeBuilder> aMyArray_builder { factory.create_array_type(
 * myArray_builder.build(),
                                                        { 2, 2 })};
   aas_builder.add_member(0, "my_array_array", aMyArray_builder.build());
   aas_builder->set_name("ArrayArrayStruct");

   // Struct ArrayArrayArrayStruct
   std::unique_ptr<DynamicTypeBuilder> aaas_builder { factory.create_struct_type()};
   std::unique_ptr<DynamicTypeBuilder> aas_array_builder { factory.create_array_type(
 * aas_builder.build(),
                                                         { 2, 2 })};
   aaas_builder.add_member(0, "my_array_array_array", aas_array_builder.build());
   aaas_builder->set_name("ArrayArrayArrayStruct");
   std::unique_ptr<const DynamicType> aaas_type = aaas_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *aaas_type);
   EXPECT_TRUE(type->equals(*aaas_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_SequenceStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<DynamicTypeBuilder> seq_builder { factory.create_sequence_type(
 * factory.get_int32_type(),
                                                   2)};

   std::unique_ptr<DynamicTypeBuilder> seqs_builder { factory.create_struct_type()};
   seqs_builder.add_member(0, "my_sequence", seq_builder.build());
   seqs_builder->set_name("SequenceStruct");
   std::unique_ptr<const DynamicType> seqs_type = seqs_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *seqs_type);
   EXPECT_TRUE(type->equals(*seqs_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_SequenceSequenceStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceSequenceStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<DynamicTypeBuilder> seq_builder { factory.create_sequence_type(
 * factory.get_int32_type(),
                                                   2)};
   std::unique_ptr<DynamicTypeBuilder> alias_builder { factory.create_alias_type(
 * seq_builder.build(),
                                                     "my_sequence_sequence_inner")};

   std::unique_ptr<DynamicTypeBuilder> sss_builder { factory.create_struct_type()};
   std::unique_ptr<DynamicTypeBuilder> seq_seq_builder { factory.create_sequence_type(*alias_builder.build(), 2)};
   sss_builder.add_member(0, "my_sequence_sequence", seq_seq_builder.build());
   sss_builder->set_name("SequenceSequenceStruct");
   std::unique_ptr<const DynamicType> sss_type = sss_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *sss_type);
   EXPECT_TRUE(type->equals(*sss_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_MapStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("MapStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<DynamicTypeBuilder> map_builder { factory.create_map_type(
 * factory.get_int32_type(),
 * factory.get_int32_type(),
                                                   7)};

   std::unique_ptr<DynamicTypeBuilder> maps_builder { factory.create_struct_type()};
   maps_builder.add_member(0, "my_map", map_builder.build());
   maps_builder->set_name("MapStruct");
   std::unique_ptr<const DynamicType> maps_type = maps_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *maps_type);
   EXPECT_TRUE(type->equals(*maps_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_MapMapStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("MapMapStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicType> int32_type = factory.get_int32_type();
   std::unique_ptr<DynamicTypeBuilder> map_builder { factory.create_map_type(
 * int32_type,
 * int32_type,
                                                   2)};
   std::unique_ptr<DynamicTypeBuilder> alias_builder { factory.create_alias_type(
 * map_builder.build(),
                                                     "my_map_map_inner")};
   std::unique_ptr<DynamicTypeBuilder> map_map_builder { factory.create_map_type(
 * int32_type,
 * alias_builder.build(),
                                                       2)};

   std::unique_ptr<DynamicTypeBuilder> maps_builder { factory.create_struct_type()};
   maps_builder.add_member(0, "my_map_map", map_map_builder.build());
   maps_builder->set_name("MapMapStruct");
   std::unique_ptr<const DynamicType> maps_type = maps_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *maps_type);
   EXPECT_TRUE(type->equals(*maps_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_StructStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<DynamicTypeBuilder> structs_builder { factory.create_struct_type()};
   structs_builder.add_member(0, "a", factory.get_int32_type());
   structs_builder.add_member(1, "b", factory.get_int64_type());
   structs_builder->set_name("StructStruct");
   std::unique_ptr<const DynamicType> structs_type = structs_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *structs_type);
   EXPECT_TRUE(type->equals(*structs_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_StructStructStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("StructStructStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<DynamicTypeBuilder> structs_builder { factory.create_struct_type()};
   structs_builder.add_member(0, "a", factory.get_int32_type());
   structs_builder.add_member(1, "b", factory.get_int64_type());
   structs_builder->set_name("StructStruct");

   std::unique_ptr<DynamicTypeBuilder> sss_builder { factory.create_struct_type()};
   sss_builder.add_member(0, "child_struct", structs_builder.build());
   sss_builder.add_member(1, "child_int64", factory.get_int64_type());
   sss_builder->set_name("StructStructStruct");
   std::unique_ptr<const DynamicType> sss_type = sss_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *sss_type);
   EXPECT_TRUE(type->equals(*sss_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_SimpleUnionStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("SimpleUnionStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicType> int32_type = factory.get_int32_type();

   std::unique_ptr<DynamicTypeBuilder> union_builder { factory.create_union_type(*int32_type)};
   union_builder.add_member(0, "first", int32_type, "", std::vector<uint64_t>{ 0 }, true);
   union_builder.add_member(1, "second", factory.get_int64_type(), "", std::vector<uint64_t>{ 1 }, false);
   union_builder->set_name("SimpleUnion");

   std::unique_ptr<DynamicTypeBuilder> us_builder { factory.create_struct_type()};
   us_builder.add_member(0, "my_union", union_builder.build());
   us_builder->set_name("SimpleUnionStruct");
   std::unique_ptr<const DynamicType> us_type = us_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *us_type);
   EXPECT_TRUE(type->equals(*us_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_UnionUnionStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("UnionUnionStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<const DynamicType> int32_type = factory.get_int32_type();

   std::unique_ptr<DynamicTypeBuilder> union_builder { factory.create_union_type(*int32_type)};
   union_builder.add_member(0, "first", int32_type, "", std::vector<uint64_t>{ 0 }, true);
   union_builder.add_member(1, "second", factory.get_int64_type(), "", std::vector<uint64_t>{ 1 }, false);
   union_builder->set_name("SimpleUnion");

   std::unique_ptr<DynamicTypeBuilder> union_union_builder { factory.create_union_type(*int32_type)};
   union_union_builder.add_member(0, "first", int32_type, "", std::vector<uint64_t>{ 0 }, true);
   union_union_builder.add_member(1, "second", union_builder.build(), "", std::vector<uint64_t>{ 1 }, false);
   union_union_builder->set_name("UnionUnion");

   std::unique_ptr<DynamicTypeBuilder> uus_builder { factory.create_struct_type()};
   uus_builder.add_member(0, "my_union", union_union_builder.build());
   uus_builder->set_name("UnionUnionStruct");
   std::unique_ptr<const DynamicType> uus_type = uus_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *uus_type);
   EXPECT_TRUE(type->equals(*uus_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_WCharUnionStruct_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("WCharUnionStruct");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   std::unique_ptr<DynamicTypeBuilder> union_builder { factory.create_union_type(*factory.get_char16_type())};
   union_builder.add_member(0, "first", factory.get_int32_type(), "", std::vector<uint64_t>{ 0 }, true);
   union_builder.add_member(1, "second", factory.get_int64_type(), "", std::vector<uint64_t>{ 1 }, false);
   union_builder->set_name("WCharUnion");

   std::unique_ptr<DynamicTypeBuilder> us_builder { factory.create_struct_type()};
   us_builder.add_member(0, "my_union", union_builder.build());
   us_builder->set_name("WCharUnionStruct");
   std::unique_ptr<const DynamicType> us_type = us_builder.build();

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *us_type);
   EXPECT_TRUE(type->equals(*us_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_bounded_string_unit_tests)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStringStruct");
   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   std::unique_ptr<DynamicData> data {DynamicDataFactory::get_instance().create_data(*type)};

   // SERIALIZATION TEST
   StringStruct refData;
   StringStructPubSubType refDatapb;

   uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data.get())());
   SerializedPayload_t payload(payloadSize);
   SerializedPayload_t dynamic_payload(payloadSize);
   EXPECT_TRUE(pbType->serialize(data, &dynamic_payload));
   EXPECT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

   uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
   SerializedPayload_t static_payload(static_payloadSize);
   EXPECT_TRUE(refDatapb.serialize(&refData, &static_payload));
   EXPECT_EQ(static_payload.length, static_payloadSize);
   EXPECT_NE(data->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_bounded_wstring_unit_tests)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("ShortWStringStruct");
   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   std::unique_ptr<DynamicData> data {DynamicDataFactory::get_instance().create_data(*type)};

   // SERIALIZATION TEST
   StringStruct refData;
   StringStructPubSubType refDatapb;

   uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data.get())());
   SerializedPayload_t payload(payloadSize);
   SerializedPayload_t dynamic_payload(payloadSize);
   EXPECT_TRUE(pbType->serialize(data, &dynamic_payload));
   EXPECT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

   uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
   SerializedPayload_t static_payload(static_payloadSize);
   EXPECT_TRUE(refDatapb.serialize(&refData, &static_payload));
   EXPECT_EQ(static_payload.length, static_payloadSize);
   EXPECT_NE(data->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), eprosima::fastdds::dds::RETCODE_OK);

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_Bitset_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);
   auto pbType = XMLProfileManager::CreateDynamicPubSubType("MyBitSet");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   auto a_type = factory.get_char8_type();
   auto b_type = factory.get_bool_type();
   auto c_type = factory.get_uint16_type();
   auto d_type = factory.get_int16_type();

   //
   XML:
   < bitset name = "MyBitSet" >
     < bitfield name = "a" bit_bound = "3" / >
     < bitfield name = "b" bit_bound = "1" / >
     < bitfield bit_bound = "4" / >
     < bitfield name = "c" bit_bound = "10" / >
     < bitfield name = "d" bit_bound = "12" type = "int16" / >
     < / bitset >

   IDL:
     bitset MyBitset
   {
   bitfield<3> a; // @bit_bound=3 @position=0
   bitfield<1> b; // @bit_bound=1 @position=3
   bitfield<4>;
   bitfield<10> c; // @bit_bound=10 @position=8
   bitfield<12, short> d; // @bit_bound=12 @position=18
   };
   //

   // Bitset
   std::unique_ptr<DynamicTypeBuilder> bitset_builder { factory.create_bitset_type()};

   bitset_builder.add_member(0, "a", a_type);
   bitset_builder.apply_annotation_to_member(0, ANNOTATION_BIT_BOUND, "value", "3");
   bitset_builder.apply_annotation_to_member(0, ANNOTATION_POSITION, "value", "0");

   bitset_builder.add_member(1, "b", b_type);
   bitset_builder.apply_annotation_to_member(1, ANNOTATION_BIT_BOUND, "value", "1");
   bitset_builder.apply_annotation_to_member(1, ANNOTATION_POSITION, "value", "3");

   bitset_builder.add_member(2, "", a_type);
   // The member doesn't exist so the annotation application will fail, and isn't needed.
   //bitset_builder.apply_annotation_to_member(2, ANNOTATION_BIT_BOUND, "value", "4");
   //bitset_builder.apply_annotation_to_member(2, ANNOTATION_POSITION, "value", "4");

   bitset_builder.add_member(3, "c", c_type);
   bitset_builder.apply_annotation_to_member(3, ANNOTATION_BIT_BOUND, "value", "10");
   bitset_builder.apply_annotation_to_member(3, ANNOTATION_POSITION, "value", "8"); // 4 empty

   bitset_builder.add_member(4, "d", d_type);
   bitset_builder.apply_annotation_to_member(4, ANNOTATION_BIT_BOUND, "value", "12");
   bitset_builder.apply_annotation_to_member(4, ANNOTATION_POSITION, "value", "18");

   bitset_builder->set_name("MyBitSet");
   std::unique_ptr<const DynamicType> bitset_type = bitset_builder.build();
   ASSERT_TRUE(bitset_type);

   std::unique_ptr<const DynamicType> type;
   ASSERT_EQ(pbType->GetDynamicType(type), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type, *bitset_type);
   EXPECT_TRUE(type->equals(*bitset_type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST_F(DynamicTypesTests, DynamicType_ostream_test)
   {
   using namespace xmlparser;

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   auto a_type = factory.get_char8_type();
   auto b_type = factory.get_bool_type();
   auto c_type = factory.get_uint16_type();
   auto d_type = factory.get_int16_type();

   // Bitset
   std::unique_ptr<DynamicTypeBuilder> bitset_builder { factory.create_bitset_type()};

   bitset_builder.add_member(0, "a", a_type);
   bitset_builder.apply_annotation_to_member(0, ANNOTATION_BIT_BOUND, "value", "3");
   bitset_builder.apply_annotation_to_member(0, ANNOTATION_POSITION, "value", "0");

   bitset_builder.add_member(1, "b", b_type);
   bitset_builder.apply_annotation_to_member(1, ANNOTATION_BIT_BOUND, "value", "1");
   bitset_builder.apply_annotation_to_member(1, ANNOTATION_POSITION, "value", "3");

   bitset_builder.add_member(2, "", a_type);

   bitset_builder.add_member(3, "c", c_type);
   bitset_builder.apply_annotation_to_member(3, ANNOTATION_BIT_BOUND, "value", "10");
   bitset_builder.apply_annotation_to_member(3, ANNOTATION_POSITION, "value", "8"); // 4 empty

   bitset_builder.add_member(4, "d", d_type);
   bitset_builder.apply_annotation_to_member(4, ANNOTATION_BIT_BOUND, "value", "12");
   bitset_builder.apply_annotation_to_member(4, ANNOTATION_POSITION, "value", "18");

   bitset_builder->set_name("MyBitSet");
   bitset_builder->annotation_set_final();
   std::unique_ptr<const DynamicType> bitset_type = bitset_builder.build();
   ASSERT_TRUE(bitset_type);

   std::ostringstream os;
   os << *bitset_type;

   const std::string reference =
     "\n\tname:     MyBitSet\n\tkind:     TK_BITSET\n\tbounds:   1\n\tmembers:\
   \n\t\tindex:    0\n\t\tname:     a\n\t\tid:       0\n\t\ttype:     \n\t\t\tname:     char\n\t\t\tkind:\
   TK_CHAR8\n\t\t\tbounds:   0\n\t\tmember annotations:\n\t\t\tannotation:bit_bound\n\t\t\t\tkey:\
   'value'\n\t\t\t\tvalue:   3\n\t\t\tannotation:position\n\t\t\t\tkey:     'value'\n\t\t\t\tvalue:\
   0\n\n\t\tindex:    1\n\t\tname:     b\n\t\tid:       1\n\t\ttype:     \n\t\t\tname:     bool\n\t\t\tkind:\
   TK_BOOLEAN\n\t\t\tbounds:   0\n\t\tmember annotations:\n\t\t\tannotation:bit_bound\n\t\t\t\tkey:\
   'value'\n\t\t\t\tvalue:   1\n\t\t\tannotation:position\n\t\t\t\tkey:     'value'\n\t\t\t\tvalue:\
   3\n\n\t\tindex:    2\n\t\tname:     \n\t\tid:       2\n\t\ttype:     \n\t\t\tname:     char\n\t\t\tkind:\
   TK_CHAR8\n\t\t\tbounds:   0\n\n\t\tindex:    3\n\t\tname:     c\n\t\tid:       3\n\t\ttype:     \n\t\t\tname:\
   uint16_t\n\t\t\tkind:     TK_UINT16\n\t\t\tbounds:   0\n\t\tmember annotations:\n\t\t\tannotation:\
   bit_bound\n\t\t\t\tkey:     'value'\n\t\t\t\tvalue:   10\n\t\t\tannotation:position\n\t\t\t\tkey:\
   'value'\n\t\t\t\tvalue:   8\n\n\t\tindex:    4\n\t\tname:     d\n\t\tid:       4\n\t\ttype:\
   \n\t\t\tname:     int16_t\n\t\t\tkind:     TK_INT16\n\t\t\tbounds:   0\n\t\tmember annotations:\
   \n\t\t\tannotation:bit_bound\n\t\t\t\tkey:     'value'\n\t\t\t\tvalue:   12\n\t\t\tannotation:position\n\t\t\t\tkey:\
   'value'\n\t\t\t\tvalue:   18\n";

   ASSERT_EQ(reference, os.str());
   }

   TEST_F(DynamicTypesTests, DynamicType_XML_Bitmask_test)
   {
   using namespace xmlparser;

   XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
   ASSERT_EQ(ret, XMLP_ret::XML_OK);

   auto pbType = XMLProfileManager::CreateDynamicPubSubType("MyBitMask");

   DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

   // Bitset
   std::unique_ptr<DynamicTypeBuilder> builder { factory.create_bitmask_type(8)};
   builder->add_member(0, "flag0");
   builder->add_member(1, "flag1");
   builder->add_member(2, "flag2");
   builder->add_member(5, "flag5");
   builder->set_name("MyBitMask");
   std::unique_ptr<const DynamicType> type = builder.build();
   ASSERT_TRUE(type);

   std::unique_ptr<const DynamicType> type2;
   ASSERT_EQ(pbType->GetDynamicType(type2), eprosima::fastdds::dds::RETCODE_OK);
   EXPECT_EQ(*type2, *type);
   EXPECT_TRUE(type2->equals(*type));

   delete(pbType);
   XMLProfileManager::DeleteInstance();
   }

   TEST(TypeIdentifierTests, MinimalTypeIdentifierComparision)
   {
   TypeIdentifier enum1 = *GetMyEnumIdentifier(false);
   TypeIdentifier aliasEnum1 = *GetMyAliasEnumIdentifier(false);
   TypeIdentifier aliasAliasEnum1 = *GetMyAliasAliasEnumIdentifier(false);
   TypeIdentifier boolStruct1 = *GetBoolStructIdentifier(false);
   TypeIdentifier octetStruct1 = *GetOctetStructIdentifier(false);
   TypeIdentifier shortStruct1 = *GetShortStructIdentifier(false);
   TypeIdentifier longStruct1 = *GetLongStructIdentifier(false);
   TypeIdentifier longLongStruct1 = *GetLongLongStructIdentifier(false);
   TypeIdentifier uShortStruct1 = *GetShortStructIdentifier(false);
   TypeIdentifier uLongStruct1 = *GetULongStructIdentifier(false);
   TypeIdentifier uLongLongStruct1 = *GetULongLongStructIdentifier(false);
   TypeIdentifier floatStruct1 = *GetFloatStructIdentifier(false);
   TypeIdentifier doubleStruct1 = *GetDoubleStructIdentifier(false);
   TypeIdentifier longDoubleStruct1 = *GetLongDoubleStructIdentifier(false);
   TypeIdentifier charStruct1 = *GetCharStructIdentifier(false);
   TypeIdentifier wcharStruct1 = *GetWCharStructIdentifier(false);
   TypeIdentifier stringStruct1 = *GetStringStructIdentifier(false);
   TypeIdentifier wstringStruct1 = *GetWStringStructIdentifier(false);
   TypeIdentifier largeStringStruct1 = *GetLargeStringStructIdentifier(false);
   TypeIdentifier largeWStringStruct1 = *GetLargeWStringStructIdentifier(false);
   TypeIdentifier arrayStruct1 = *GetArraytStructIdentifier(false);
   GetMyArrayIdentifier(false); // We need to generate it before arrayArrayStruct
   TypeIdentifier arrayArrayStruct1 = *GetArrayArrayStructIdentifier(false);
   TypeIdentifier sequenceStruct1 = *GetSequenceStructIdentifier(false);
   TypeIdentifier sequenceSequenceStruct1 = *GetSequenceSequenceStructIdentifier(false);
   TypeIdentifier mapStruct1 = *GetMapStructIdentifier(false);
   TypeIdentifier mapMapStruct1 = *GetMapMapStructIdentifier(false);
   TypeIdentifier structStruct1 = *GetStructStructIdentifier(false);
   TypeIdentifier structStructStruct1 = *GetStructStructStructIdentifier(false);
   TypeIdentifier simpleUnion1 = *GetSimpleUnionIdentifier(false);
   TypeIdentifier unionUnion1 = *GetUnionUnionIdentifier(false);
   TypeIdentifier wCharUnion1 = *GetWCharUnionIdentifier(false);
   TypeIdentifier unionUnionStruct1 = *GetUnionUnionUnionStructIdentifier(false);
   TypeObjectFactory::get_instance()->delete_instance(); // Force new instances instead reusing them
   registerBasicTypes(); // Register them again
   TypeIdentifier enum2 = *GetMyEnumIdentifier(false);
   TypeIdentifier aliasEnum2 = *GetMyAliasEnumIdentifier(false);
   TypeIdentifier aliasAliasEnum2 = *GetMyAliasAliasEnumIdentifier(false);
   TypeIdentifier boolStruct2 = *GetBoolStructIdentifier(false);
   TypeIdentifier octetStruct2 = *GetOctetStructIdentifier(false);
   TypeIdentifier shortStruct2 = *GetShortStructIdentifier(false);
   TypeIdentifier longStruct2 = *GetLongStructIdentifier(false);
   TypeIdentifier longLongStruct2 = *GetLongLongStructIdentifier(false);
   TypeIdentifier uShortStruct2 = *GetShortStructIdentifier(false);
   TypeIdentifier uLongStruct2 = *GetULongStructIdentifier(false);
   TypeIdentifier uLongLongStruct2 = *GetULongLongStructIdentifier(false);
   TypeIdentifier floatStruct2 = *GetFloatStructIdentifier(false);
   TypeIdentifier doubleStruct2 = *GetDoubleStructIdentifier(false);
   TypeIdentifier longDoubleStruct2 = *GetLongDoubleStructIdentifier(false);
   TypeIdentifier charStruct2 = *GetCharStructIdentifier(false);
   TypeIdentifier wcharStruct2 = *GetWCharStructIdentifier(false);
   TypeIdentifier stringStruct2 = *GetStringStructIdentifier(false);
   TypeIdentifier wstringStruct2 = *GetWStringStructIdentifier(false);
   TypeIdentifier largeStringStruct2 = *GetLargeStringStructIdentifier(false);
   TypeIdentifier largeWStringStruct2 = *GetLargeWStringStructIdentifier(false);
   TypeIdentifier arrayStruct2 = *GetArraytStructIdentifier(false);
   TypeIdentifier arrayArrayStruct2 = *GetArrayArrayStructIdentifier(false);
   TypeIdentifier sequenceStruct2 = *GetSequenceStructIdentifier(false);
   TypeIdentifier sequenceSequenceStruct2 = *GetSequenceSequenceStructIdentifier(false);
   TypeIdentifier mapStruct2 = *GetMapStructIdentifier(false);
   TypeIdentifier mapMapStruct2 = *GetMapMapStructIdentifier(false);
   TypeIdentifier structStruct2 = *GetStructStructIdentifier(false);
   TypeIdentifier structStructStruct2 = *GetStructStructStructIdentifier(false);
   TypeIdentifier simpleUnion2 = *GetSimpleUnionIdentifier(false);
   TypeIdentifier unionUnion2 = *GetUnionUnionIdentifier(false);
   TypeIdentifier wCharUnion2 = *GetWCharUnionIdentifier(false);
   TypeIdentifier unionUnionStruct2 = *GetUnionUnionUnionStructIdentifier(false);

   // Compare equals
   ASSERT_TRUE(enum1 == enum2);
   ASSERT_TRUE(aliasEnum1 == aliasEnum2 || aliasEnum2 == enum1);
   ASSERT_TRUE(aliasAliasEnum1 == aliasAliasEnum2 || aliasAliasEnum2 == enum1);
   ASSERT_TRUE(boolStruct1 == boolStruct2);
   ASSERT_TRUE(octetStruct1 == octetStruct2);
   ASSERT_TRUE(shortStruct1 == shortStruct2);
   ASSERT_TRUE(longStruct1 == longStruct2);
   ASSERT_TRUE(longLongStruct1 == longLongStruct2);
   ASSERT_TRUE(uShortStruct1 == uShortStruct2);
   ASSERT_TRUE(uLongStruct1 == uLongStruct2);
   ASSERT_TRUE(uLongLongStruct1 == uLongLongStruct2);
   ASSERT_TRUE(floatStruct1 == floatStruct2);
   ASSERT_TRUE(doubleStruct1 == doubleStruct2);
   ASSERT_TRUE(longDoubleStruct1 == longDoubleStruct2);
   ASSERT_TRUE(charStruct1 == charStruct2);
   ASSERT_TRUE(wcharStruct1 == wcharStruct2);
   ASSERT_TRUE(stringStruct1 == stringStruct2);
   ASSERT_TRUE(wstringStruct1 == wstringStruct2);
   ASSERT_TRUE(largeStringStruct1 == largeStringStruct2);
   ASSERT_TRUE(largeWStringStruct1 == largeWStringStruct2);
   ASSERT_TRUE(arrayStruct1 == arrayStruct2);
   ASSERT_TRUE(arrayArrayStruct1 == arrayArrayStruct2);
   ASSERT_TRUE(sequenceStruct1 == sequenceStruct2);
   ASSERT_TRUE(sequenceSequenceStruct1 == sequenceSequenceStruct2);
   ASSERT_TRUE(mapStruct1 == mapStruct2);
   ASSERT_TRUE(mapMapStruct1 == mapMapStruct2);
   ASSERT_TRUE(structStruct1 == structStruct2);
   ASSERT_TRUE(structStructStruct1 == structStructStruct2);
   ASSERT_TRUE(simpleUnion1 == simpleUnion2);
   ASSERT_TRUE(unionUnion1 == unionUnion2);
   ASSERT_TRUE(wCharUnion1 == wCharUnion2);
   ASSERT_TRUE(unionUnionStruct1 == unionUnionStruct2);
   ASSERT_TRUE(enum2 == aliasEnum2);

   // Compare some not equals
   ASSERT_FALSE(aliasAliasEnum1 == boolStruct1);
   ASSERT_FALSE(octetStruct1 == shortStruct1);
   ASSERT_FALSE(longStruct1 == longLongStruct1);
   ASSERT_FALSE(uShortStruct1 == uLongStruct1);
   ASSERT_FALSE(uLongStruct1 == uLongLongStruct2);
   ASSERT_FALSE(floatStruct1 == doubleStruct1);
   ASSERT_FALSE(doubleStruct1 == longDoubleStruct2);
   ASSERT_FALSE(charStruct1 == wcharStruct1);
   ASSERT_FALSE(stringStruct1 == wstringStruct1);
   ASSERT_FALSE(stringStruct1 == largeStringStruct2);
   ASSERT_FALSE(wstringStruct1 == largeWStringStruct2);
   ASSERT_FALSE(arrayStruct1 == arrayArrayStruct1);
   ASSERT_FALSE(sequenceStruct1 == sequenceSequenceStruct1);
   ASSERT_FALSE(mapStruct1 == mapMapStruct1);
   ASSERT_FALSE(structStruct1 == structStructStruct1);
   ASSERT_FALSE(simpleUnion1 == unionUnion1);
   ASSERT_FALSE(unionUnion1 == wCharUnion2);
   ASSERT_FALSE(unionUnionStruct1 == unionUnion1);
   }
 */

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
