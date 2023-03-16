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

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "idl/BasicPubSubTypes.h"
#include "idl/BasicTypeObject.h"

#include <tinyxml2.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <tuple>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// Ancillary gtest formatters

void PrintTo(const MemberDescriptor& md, std::ostream* os) {
    if (os)
    {
        *os << md;
    }
}

void PrintTo(const TypeDescriptor& md, std::ostream* os) {
    if (os)
    {
        *os << md;
    }
}

using primitive_builder_api = DynamicTypeBuilder_cptr& (DynamicTypeBuilderFactory::* )();
using primitive_type_api = DynamicType_ptr (DynamicTypeBuilderFactory::* )();

// Testing the primitive creation APIS
// and get_primitive_type() and create_primitive_builder()
class DynamicTypesPrimitiveTestsAPIs
    : public testing::TestWithParam<std::tuple<TypeKind, primitive_builder_api, primitive_type_api>>
{};

TEST_P(DynamicTypesPrimitiveTestsAPIs, primitives_apis_unit_tests)
{
    // Get the factory singleton
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Retrieve parameters
    TypeKind kind;
    primitive_builder_api bapi;
    primitive_type_api tapi;
    std::tie(kind, bapi, tapi) = GetParam();

    // Create the primitive builder,
    // note that create_xxx_builder rely on create_primitive_builder<TK_xxxx>()
    DynamicTypeBuilder_cptr builder1 = (factory.*bapi)();
    ASSERT_TRUE(builder1);

    // It must be the right builder
    EXPECT_EQ(builder1->get_kind(), kind);

    // It must be consistent
    EXPECT_TRUE(builder1->is_consistent());

    // The primitive builder is statically allocated and must always be the same instance
    DynamicTypeBuilder_cptr builder2 = (factory.*bapi)();
    ASSERT_TRUE(builder2);
    EXPECT_EQ(builder1, builder2);

    // It must match the one created by the generic api
    DynamicTypeBuilder_cptr builder3 = factory.create_primitive_builder(kind);
    ASSERT_TRUE(builder3);
    EXPECT_EQ(builder1, builder3);

    // The builder must be able to generate the associated type
    DynamicType_ptr type1 = builder1->build();
    ASSERT_TRUE(type1);

    // It must be the right type
    EXPECT_EQ(type1->get_kind(), kind);

    // It must be consistent
    EXPECT_TRUE(type1->is_consistent());

    // It must share the same state with the builder
    EXPECT_TRUE(*type1 == *builder1);
    EXPECT_TRUE(builder1->equals(*type1));

    // It must return always the same type instance
    EXPECT_EQ(type1, builder1->build());
    EXPECT_EQ(type1, builder2->build());
    EXPECT_EQ(type1, builder3->build());

    // The primitives types can be retrieved directly from the factory
    DynamicType_ptr type2 = factory.get_primitive_type(kind);
    ASSERT_TRUE(type2);

    // and must be the very same instance
    EXPECT_EQ(type1, type2);

    // It must match the ones return by the factory primitive api calls
    DynamicType_ptr type3 = (factory.*tapi)();
    ASSERT_TRUE(type3);
    EXPECT_EQ(type1, type3);
    EXPECT_EQ(type2, type3);

    // All the instances are static, not dynamic ones should have been allocated
    EXPECT_TRUE(factory.is_empty());

    // It must be possible to create a custom builder from a primitive one
    DynamicTypeBuilder_ptr custom_builder = factory.create_builder_copy(*builder1);
    ASSERT_TRUE(custom_builder);

    // It must be consistent
    EXPECT_TRUE(type1->is_consistent());

    // It must not be the static instance
    EXPECT_NE(builder1, custom_builder);
    // but must share its state
    EXPECT_TRUE(*custom_builder == *builder1);

    // It must be customizable
    std::string name = "custom_type_name";
    custom_builder->set_name(name);
    EXPECT_EQ(custom_builder->get_name(), name);

    // no longer share the state
    EXPECT_FALSE(*custom_builder == *builder1);

    // the custom type must not be a static instance
    EXPECT_FALSE(factory.is_empty());

    // The custom instance must be able to create a new type
    DynamicType_ptr custom_type1 = custom_builder->build();
    ASSERT_TRUE(custom_type1);

    // It must be consistent
    EXPECT_TRUE(custom_type1->is_consistent());

    // It must share the state with the builder
    EXPECT_TRUE(custom_builder->equals(*custom_type1));

    // It must return a cached instances if there are not changes
    DynamicType_ptr custom_type2 = custom_builder->build();
    ASSERT_TRUE(custom_type2);
    EXPECT_EQ(custom_type1, custom_type2);

    // If there are state changes it must provide a new instance
    name = "another_name";
    custom_builder->set_name(name);
    EXPECT_EQ(custom_builder->get_name(), name);

    DynamicType_ptr custom_type3 = custom_builder->build();
    ASSERT_TRUE(custom_type3);
    EXPECT_NE(custom_type1, custom_type3);

    // The new types shouldn't be static
    custom_builder.reset();
    EXPECT_FALSE(factory.is_empty());

    // All resources should be freed out of scope
    custom_type1.reset();
    custom_type2.reset();
    custom_type1.reset();
}

INSTANTIATE_TEST_SUITE_P(CheckingGetPrimitiveType,
                         DynamicTypesPrimitiveTestsAPIs,
                         testing::Values(
                             std::make_tuple(TypeKind::TK_INT32,
                                    &DynamicTypeBuilderFactory::create_int32_builder,
                                    &DynamicTypeBuilderFactory::create_int32_type),
                             std::make_tuple(TypeKind::TK_UINT32,
                                    &DynamicTypeBuilderFactory::create_uint32_builder,
                                    &DynamicTypeBuilderFactory::create_uint32_type),
                             std::make_tuple(TypeKind::TK_INT16,
                                    &DynamicTypeBuilderFactory::create_int16_builder,
                                    &DynamicTypeBuilderFactory::create_int16_type),
                             std::make_tuple(TypeKind::TK_UINT16,
                                    &DynamicTypeBuilderFactory::create_uint16_builder,
                                    &DynamicTypeBuilderFactory::create_uint16_type),
                             std::make_tuple(TypeKind::TK_INT64,
                                    &DynamicTypeBuilderFactory::create_int64_builder,
                                    &DynamicTypeBuilderFactory::create_int64_type),
                             std::make_tuple(TypeKind::TK_UINT64,
                                    &DynamicTypeBuilderFactory::create_uint64_builder,
                                    &DynamicTypeBuilderFactory::create_uint64_type),
                             std::make_tuple(TypeKind::TK_FLOAT32,
                                    &DynamicTypeBuilderFactory::create_float32_builder,
                                    &DynamicTypeBuilderFactory::create_float32_type),
                             std::make_tuple(TypeKind::TK_FLOAT64,
                                    &DynamicTypeBuilderFactory::create_float64_builder,
                                    &DynamicTypeBuilderFactory::create_float64_type),
                             std::make_tuple(TypeKind::TK_FLOAT128,
                                    &DynamicTypeBuilderFactory::create_float128_builder,
                                    &DynamicTypeBuilderFactory::create_float128_type),
                             std::make_tuple(TypeKind::TK_CHAR8,
                                    &DynamicTypeBuilderFactory::create_char8_builder,
                                    &DynamicTypeBuilderFactory::create_char8_type),
                             std::make_tuple(TypeKind::TK_CHAR16,
                                    &DynamicTypeBuilderFactory::create_char16_builder,
                                    &DynamicTypeBuilderFactory::create_char16_type),
                             std::make_tuple(TypeKind::TK_BOOLEAN,
                                    &DynamicTypeBuilderFactory::create_bool_builder,
                                    &DynamicTypeBuilderFactory::create_bool_type),
                             std::make_tuple(TypeKind::TK_BYTE,
                                    &DynamicTypeBuilderFactory::create_byte_builder,
                                    &DynamicTypeBuilderFactory::create_byte_type)));

// Testing create_primitive_builder<TypeKind>

// ancillary class, gtest only allows parametrized tests on types
template<TypeKind> struct TypeKindType {};

#define GTEST_CONST2TYPE(type)                   \
template<>                                       \
struct TypeKindType<TypeKind::type>              \
{                                                \
    static const TypeKind kind = TypeKind::type; \
};

// specializations
GTEST_CONST2TYPE(TK_BOOLEAN)
GTEST_CONST2TYPE(TK_BYTE)
GTEST_CONST2TYPE(TK_INT16)
GTEST_CONST2TYPE(TK_INT32)
GTEST_CONST2TYPE(TK_INT64)
GTEST_CONST2TYPE(TK_UINT16)
GTEST_CONST2TYPE(TK_UINT32)
GTEST_CONST2TYPE(TK_UINT64)
GTEST_CONST2TYPE(TK_FLOAT32)
GTEST_CONST2TYPE(TK_FLOAT64)
GTEST_CONST2TYPE(TK_FLOAT128)
GTEST_CONST2TYPE(TK_CHAR8)
GTEST_CONST2TYPE(TK_CHAR16)

template<class T>
class StaticTypesPrimitiveTests
    : public testing::Test
{};

#undef GTEST_CONST2TYPE
#define GTEST_CONST2TYPE(type) TypeKindType<TypeKind::type>

using TypeKindTypes = ::testing::Types<
    GTEST_CONST2TYPE(TK_BOOLEAN),
    GTEST_CONST2TYPE(TK_BYTE),
    GTEST_CONST2TYPE(TK_INT16),
    GTEST_CONST2TYPE(TK_INT32),
    GTEST_CONST2TYPE(TK_INT64),
    GTEST_CONST2TYPE(TK_UINT16),
    GTEST_CONST2TYPE(TK_UINT32),
    GTEST_CONST2TYPE(TK_UINT64),
    GTEST_CONST2TYPE(TK_FLOAT32),
    GTEST_CONST2TYPE(TK_FLOAT64),
    GTEST_CONST2TYPE(TK_FLOAT128),
    GTEST_CONST2TYPE(TK_CHAR8),
    GTEST_CONST2TYPE(TK_CHAR16)>;

TYPED_TEST_SUITE(StaticTypesPrimitiveTests, TypeKindTypes, );

TYPED_TEST(StaticTypesPrimitiveTests, create_primitive_template_unit_tests)
{
    // Get the factory singleton
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Create the primitive builder,
    // note that create_xxx_builder rely on create_primitive_builder<TK_xxxx>()
    DynamicTypeBuilder_cptr builder1 = factory.create_primitive_builder<TypeParam::kind>();
    ASSERT_TRUE(builder1);

    // It must return the same builder than the runtime counterpart
    DynamicTypeBuilder_cptr builder2 = factory.create_primitive_builder(TypeParam::kind);
    ASSERT_TRUE(builder2);
    EXPECT_EQ(builder1, builder2);
}

#undef GTEST_CONST2TYPE

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
        EXPECT_TRUE(DynamicTypeBuilderFactory::get_instance().is_empty());
        EXPECT_TRUE(DynamicDataFactory::get_instance()->is_empty());

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
         result_type operator()(first_argument_type lhs, second_argument_type rhs ) const
         {
            return lhs.get_index() < rhs.get_index();
         }
    };
};

TEST_F(DynamicTypesTests, TypeDescriptors_unit_tests)
{
    // Do not use the TypeDescriptor to:
    // + Get primitive types. Use the DynamicTypeBuilderFactory instead.
    // + Create new types. Use a Builder instead.

    // We want to create a new type based on int32_t
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();
    // get static builder
    DynamicTypeBuilder_cptr primitive = factory.create_int32_builder();
    ASSERT_TRUE(primitive);
    // Create a modifiable builder copy
    DynamicTypeBuilder_ptr builder = factory.create_builder_copy(*primitive);
    ASSERT_TRUE(builder);
    EXPECT_EQ(builder->get_kind(), TypeKind::TK_INT32);

    // Use TypeDescriptor to capture the state
    TypeDescriptor state = *primitive;
    DynamicTypeBuilder_ptr builder2 = factory.create_builder(state);

    ASSERT_TRUE(builder2);
    EXPECT_TRUE(builder->equals(*builder2));
    EXPECT_EQ(*primitive, *builder2);
    EXPECT_EQ(*builder, *builder2);
    EXPECT_EQ(*builder, state);
    EXPECT_EQ(state, *builder2);

    // Copy state
    TypeDescriptor state2;
    EXPECT_EQ(state2.copy_from(state), ReturnCode_t::RETCODE_OK);
    EXPECT_TRUE(state2.equals(state));
    EXPECT_EQ(state, state2);

    state2 = state;
    EXPECT_TRUE(state2.equals(state));
    EXPECT_EQ(state, state2);

    EXPECT_EQ(state2.copy_from(*builder), ReturnCode_t::RETCODE_OK);
    EXPECT_TRUE(state2.equals(*builder));
    EXPECT_TRUE(builder->equals(state2));
    EXPECT_EQ(state, *builder);

    state2 = *builder;
    EXPECT_TRUE(state2.equals(*builder));
    EXPECT_TRUE(builder->equals(state2));
    EXPECT_EQ(state, *builder);

    // Check state doesn't match the default descriptor
    TypeDescriptor defaultDescriptor;
    EXPECT_NE(state, defaultDescriptor);
    EXPECT_FALSE(state.equals(defaultDescriptor));

    // Compare with builder
    EXPECT_FALSE(builder->equals(defaultDescriptor));
    EXPECT_NE(*builder, defaultDescriptor);
    EXPECT_NE(defaultDescriptor, *builder);

    DynamicType_ptr type = builder->build();
    ASSERT_TRUE(type);
    EXPECT_EQ(type, builder->build()); // once created a type is cached

    // Modify the builder state
    builder->set_name("TEST_INT32");
    EXPECT_EQ("TEST_INT32", builder->get_name());

    EXPECT_NE(type, builder->build()); // cache is invalidated after changes

    EXPECT_FALSE(builder->equals(state));
    EXPECT_FALSE(builder->equals(*primitive));
    EXPECT_NE(*builder, state);
    EXPECT_NE(*builder, *primitive);
}

TEST_F(DynamicTypesTests, DynamicType_basic_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Create basic types
    DynamicTypeBuilder_ptr struct_type_builder = factory.create_struct_builder();
    ASSERT_TRUE(struct_type_builder);
    EXPECT_TRUE(struct_type_builder->is_consistent());
    EXPECT_EQ(struct_type_builder->get_kind(), TypeKind::TK_STRUCTURE);
    EXPECT_EQ(struct_type_builder->get_member_count(), 0u);

    // Add members to the struct.
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->add_member(3, "int32", factory.create_int32_type()));
    EXPECT_TRUE(struct_type_builder->is_consistent());
    EXPECT_EQ(struct_type_builder->get_member_count(), 1u);

    DynamicType_ptr struct_type = struct_type_builder->build();
    ASSERT_TRUE(struct_type);
    EXPECT_EQ(struct_type, struct_type_builder->build()); // Build objects are cached

    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->add_member(1, "int64", factory.create_int64_type()));
    EXPECT_TRUE(struct_type_builder->is_consistent());
    EXPECT_EQ(struct_type_builder->get_member_count(), 2u);

    EXPECT_NE(struct_type, struct_type_builder->build()); // The cached object is invalidated

    DynamicType_ptr struct_type2 = struct_type_builder->build();
    ASSERT_TRUE(struct_type2);
    EXPECT_FALSE(struct_type->equals(*struct_type2));

    // Check members are properly added
    // • checking invalid id
    MemberDescriptor md;
    {
        eprosima::fastdds::dds::Log::ScopeLogs _;
        ASSERT_NE(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member(md, 0));
    }

    // • checking MemberDescriptor getters
    MemberDescriptor md1{3, "int32", factory.create_int32_type()};
    md1.set_index(0); // first addition
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member(md, 3));

    EXPECT_TRUE(md.is_consistent(struct_type_builder->get_kind()));
    EXPECT_EQ(md.get_index(), 0u);
    EXPECT_EQ(md.get_id(), md1.get_id());
    EXPECT_EQ(md.get_name(), md1.get_name());
    EXPECT_EQ(md.get_type(), md1.get_type());

    // • checking MemberDescriptor comparisson and construction
    MemberDescriptor md2{1, "int64", factory.create_int64_type()};
    md2.set_index(1); // second addition
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member(md, 1));
    EXPECT_TRUE(md.is_consistent(struct_type_builder->get_kind()));
    EXPECT_EQ(md.get_index(), 1u);

    //    + checking operators ==, != and method equals
    EXPECT_TRUE(md.equals(md2));
    EXPECT_EQ(md, md2);

    EXPECT_FALSE(md1.equals(md2));
    EXPECT_NE(md1, md2);

    //    + checking copy_from
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, md.copy_from(md1));
    EXPECT_TRUE(md.equals(md1));
    EXPECT_EQ(md, md1);

    // • checking by index retrieval
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member_by_index(md, 0));
    EXPECT_EQ(md, md1);

    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member_by_index(md, 1));
    EXPECT_EQ(md, md2);

    // • checking by name retrieval
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member_by_name(md, "int32"));
    EXPECT_EQ(md, md1);

    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->get_member_by_name(md, "int64"));
    EXPECT_EQ(md, md2);

    // • checking map indexes retrieval
    //    + indexing by id
    auto members_by_id = struct_type_builder->get_all_members_by_id();
    EXPECT_EQ(members_by_id.size(), 2);
    EXPECT_EQ(*members_by_id[3], md1);
    EXPECT_EQ(*members_by_id[1], md2);

    //    + indexing by name
    auto members_by_name = struct_type_builder->get_all_members_by_name();
    EXPECT_EQ(members_by_name.size(), 2);
    EXPECT_EQ(*members_by_name["int32"], md1);
    EXPECT_EQ(*members_by_name["int64"], md2);

    //    + indexing by index (actual sequence)
    auto members = struct_type_builder->get_all_members();
    ASSERT_EQ(members.size(), 2);
    auto it = members.begin();
    EXPECT_EQ(**it++, md1);
    EXPECT_EQ(**it, md2);

    // • checking indexes work according with OMG standard 1.3 section 7.5.2.7.6
    md = MemberDescriptor(7, "bool", factory.create_bool_type());
    md.set_index(1); // insert int the middle
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, struct_type_builder->add_member(md));

    members = struct_type_builder->get_all_members();
    ASSERT_EQ(members.size(), 3);
    md2.set_index(2); // new expected position of the last element

    it = members.begin();
    EXPECT_EQ(**it++, md1);
    EXPECT_EQ(**it++, md);
    EXPECT_EQ(**it, md2);

    // • checking adding duplicates
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
    //    + duplicate name
        md = MemberDescriptor(0u, "int32");
        EXPECT_NE(ReturnCode_t::RETCODE_OK, struct_type_builder->add_member(md));

    //    + duplicate id
        md = MemberDescriptor(7, "dup_bool", factory.create_bool_type(), "true");
        EXPECT_NE(ReturnCode_t::RETCODE_OK, struct_type_builder->add_member(md));
    }
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Try to create with invalid values
    // • strings
    DynamicTypeBuilder_cptr created_builder = factory.create_string_builder(BOUND_UNLIMITED);
    ASSERT_TRUE(created_builder);

    DynamicType_ptr type = created_builder->build();
    ASSERT_TRUE(type);
    DynamicType_ptr type2 = created_builder->build();
    ASSERT_TRUE(type2);

    ASSERT_TRUE(type->equals(*type2));
    EXPECT_EQ(*type, *type2);
    EXPECT_EQ(type, type2); // type objects are cached

    // • wstrings
    created_builder = factory.create_wstring_builder(BOUND_UNLIMITED);
    ASSERT_TRUE(created_builder);

    type = created_builder->build();
    ASSERT_TRUE(type);
    type2 = created_builder->build();
    ASSERT_TRUE(type2);

    ASSERT_TRUE(type->equals(*type2));
    EXPECT_EQ(*type, *type2);
    EXPECT_EQ(type, type2); // type objects are cached
}

TEST_F(DynamicTypesTests, DynamicType_int32_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_int32_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    int32_t test1 = 123;
    int32_t test2 = 0;
    ASSERT_TRUE(data->set_int32_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_int32_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    //ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    //int32_t iTest32;
    //ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    LongStruct wlong;
    LongStructPubSubType wlongpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wlongpb.deserialize(&dynamic_payload, &wlong));

    uint32_t static_payloadSize = static_cast<uint32_t>(wlongpb.getSerializedSizeProvider(&wlong)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wlongpb.serialize(&wlong, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint32_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_uint32_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    uint32_t test1 = 123;
    uint32_t test2 = 0;
    ASSERT_TRUE(data->set_uint32_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_uint32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_uint32_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //uint32_t uTest32;
    //ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    ULongStruct wlong;
    ULongStructPubSubType wlongpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wlongpb.deserialize(&dynamic_payload, &wlong));

    uint32_t static_payloadSize = static_cast<uint32_t>(wlongpb.getSerializedSizeProvider(&wlong)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wlongpb.serialize(&wlong, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_int16_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_int16_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    int16_t test1 = 123;
    int16_t test2 = 0;
    ASSERT_TRUE(data->set_int16_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_int16_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_int16_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //int16_t iTest16;
    //ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    ShortStruct wshort;
    ShortStructPubSubType wshortpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wshortpb.deserialize(&dynamic_payload, &wshort));

    uint32_t static_payloadSize = static_cast<uint32_t>(wshortpb.getSerializedSizeProvider(&wshort)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wshortpb.serialize(&wshort, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint16_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_uint16_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    uint16_t test1 = 123;
    uint16_t test2 = 0;
    ASSERT_TRUE(data->set_uint16_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_uint16_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_uint16_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //uint16_t uTest16;
    //ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    UShortStruct wshort;
    UShortStructPubSubType wshortpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wshortpb.deserialize(&dynamic_payload, &wshort));

    uint32_t static_payloadSize = static_cast<uint32_t>(wshortpb.getSerializedSizeProvider(&wshort)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wshortpb.serialize(&wshort, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_int64_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_int64_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    int64_t test1 = 123;
    int64_t test2 = 0;
    ASSERT_TRUE(data->set_int64_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_int64_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_int64_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //int64_t iTest64;
    //ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    LongLongStruct wlonglong;
    LongLongStructPubSubType wlonglongpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wlonglongpb.deserialize(&dynamic_payload, &wlonglong));

    uint32_t static_payloadSize = static_cast<uint32_t>(wlonglongpb.getSerializedSizeProvider(&wlonglong)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wlonglongpb.serialize(&wlonglong, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint64_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_uint64_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    uint64_t test1 = 123;
    uint64_t test2 = 0;
    ASSERT_TRUE(data->set_uint64_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_uint64_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_uint64_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //uint64_t uTest64;
    //ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    ULongLongStruct wlonglong;
    ULongLongStructPubSubType wlonglongpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wlonglongpb.deserialize(&dynamic_payload, &wlonglong));

    uint32_t static_payloadSize = static_cast<uint32_t>(wlonglongpb.getSerializedSizeProvider(&wlonglong)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wlonglongpb.serialize(&wlonglong, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_float32_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_float32_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    float test1 = 123.0f;
    float test2 = 0.0f;
    ASSERT_TRUE(data->set_float32_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_float32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_float32_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //float fTest32;
    //ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    FloatStruct wfloat;
    FloatStructPubSubType wfloatpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wfloatpb.deserialize(&dynamic_payload, &wfloat));

    uint32_t static_payloadSize = static_cast<uint32_t>(wfloatpb.getSerializedSizeProvider(&wfloat)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wfloatpb.serialize(&wfloat, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_float64_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_float64_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    double test1 = 123.0;
    double test2 = 0.0;
    ASSERT_TRUE(data->set_float64_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_float64_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_float64_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //double fTest64;
    //ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    DoubleStruct wdouble;
    DoubleStructPubSubType wdoublepb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wdoublepb.deserialize(&dynamic_payload, &wdouble));

    uint32_t static_payloadSize = static_cast<uint32_t>(wdoublepb.getSerializedSizeProvider(&wdouble)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wdoublepb.serialize(&wdouble, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_float128_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_float128_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    long double test1 = 123.0;
    long double test2 = 0.0;
    ASSERT_TRUE(data->set_float128_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_float128_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_float128_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //long double fTest128;
    //ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    LongDoubleStruct wldouble;
    LongDoubleStructPubSubType wldoublepb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wldoublepb.deserialize(&dynamic_payload, &wldouble));

    uint32_t static_payloadSize = static_cast<uint32_t>(wldoublepb.getSerializedSizeProvider(&wldouble)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wldoublepb.serialize(&wldouble, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_char8_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_char8_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    char test1 = 'a';
    char test2 = 'b';
    ASSERT_TRUE(data->set_char8_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_char8_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_char8_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //char cTest8;
    //ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    CharStruct wchar;
    CharStructPubSubType wcharpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wcharpb.deserialize(&dynamic_payload, &wchar));

    uint32_t static_payloadSize = static_cast<uint32_t>(wcharpb.getSerializedSizeProvider(&wchar)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wcharpb.serialize(&wchar, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_char16_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_char16_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    wchar_t test1 = L'a';
    wchar_t test2 = L'b';
    ASSERT_TRUE(data->set_char16_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_char16_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_char16_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //wchar_t cTest16;
    //ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    WCharStruct wchar;
    WCharStructPubSubType wcharpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wcharpb.deserialize(&dynamic_payload, &wchar));

    uint32_t static_payloadSize = static_cast<uint32_t>(wcharpb.getSerializedSizeProvider(&wchar)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wcharpb.serialize(&wchar, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_byte_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_byte_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    octet test1 = 255;
    octet test2 = 0;
    ASSERT_TRUE(data->set_byte_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_byte_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_byte_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //octet oTest;
    //ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    OctetStruct wchar;
    OctetStructPubSubType wcharpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wcharpb.deserialize(&dynamic_payload, &wchar));

    uint32_t static_payloadSize = static_cast<uint32_t>(wcharpb.getSerializedSizeProvider(&wchar)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wcharpb.serialize(&wchar, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_bool_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_bool_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    bool test1 = true;
    bool test2 = false;
    ASSERT_TRUE(data->set_bool_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_bool_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_bool_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //bool bTest;
    //ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    BoolStruct wbool;
    BoolStructPubSubType wboolpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wboolpb.deserialize(&dynamic_payload, &wbool));

    uint32_t static_payloadSize = static_cast<uint32_t>(wboolpb.getSerializedSizeProvider(&wbool)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wboolpb.serialize(&wbool, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_enum_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr created_builder = factory.create_enum_builder();
    ASSERT_TRUE(created_builder);
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    // Add three members to the enum.
    EXPECT_EQ(created_builder->add_member(0u, "DEFAULT"), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(created_builder->add_member(1u, "FIRST"), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(created_builder->add_member(2u, "SECOND"), ReturnCode_t::RETCODE_OK);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        // Try to add a descriptor with the same name.
        EXPECT_NE(created_builder->add_member(4u, "DEFAULT"), ReturnCode_t::RETCODE_OK);
    }

    created_type = created_builder->build();
    ASSERT_TRUE(created_type);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), ReturnCode_t::RETCODE_OK);
    data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Try to set an invalid value.
    ASSERT_FALSE(data->set_enum_value("BAD", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    std::string test1 = "SECOND";
    ASSERT_FALSE(data->set_enum_value(test1, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->set_enum_value(test1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    std::string test2;
    int iTest;
    ASSERT_FALSE(data->get_int32_value(iTest, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_enum_value(test2, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_enum_value(test2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Work as uint32_t
    uint32_t uTest1 = 2;
    ASSERT_FALSE(data->set_enum_value(uTest1, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->set_enum_value(uTest1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    uint32_t uTest2;
    ASSERT_FALSE(data->get_int32_value(iTest, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->get_enum_value(uTest2, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_enum_value(uTest2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(uTest1 == uTest2);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //std::string sEnumTest;
    //ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    EnumStruct wenum;
    EnumStructPubSubType wenumpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wenumpb.deserialize(&dynamic_payload, &wenum));

    uint32_t static_payloadSize = static_cast<uint32_t>(wenumpb.getSerializedSizeProvider(&wenum)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wenumpb.serialize(&wenum, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_string_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_string_builder();
    ASSERT_TRUE(created_builder);
    EXPECT_EQ(created_builder, factory.create_string_builder()); // unbounded builders are cached
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);

    uint32_t length = 15;
    created_builder = factory.create_string_builder(length);
    ASSERT_TRUE(created_builder);
    EXPECT_NE(created_builder, factory.create_string_builder()); // bounded builders are not cached
    created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);


    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", 1) == ReturnCode_t::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(data->set_string_value(sTest1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(data->get_int32_value(test, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(data->get_string_value(sTest2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_string_value(sTest2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        ASSERT_NE(data->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), ReturnCode_t::RETCODE_OK);
    }

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //std::string sTest;
    //ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    StringStruct wstring;
    StringStructPubSubType wstringpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wstringpb.deserialize(&dynamic_payload, &wstring));

    uint32_t static_payloadSize = static_cast<uint32_t>(wstringpb.getSerializedSizeProvider(&wstring)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wstringpb.serialize(&wstring, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_wstring_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr created_builder = factory.create_wstring_builder();
    ASSERT_TRUE(created_builder);
    EXPECT_EQ(created_builder, factory.create_wstring_builder()); // unbounded builders are cached
    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);

    uint32_t length = 15;
    created_builder = factory.create_wstring_builder(length);
    ASSERT_TRUE(created_builder);
    EXPECT_NE(created_builder, factory.create_wstring_builder()); // bounded builders are not cached
    created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", 1) == ReturnCode_t::RETCODE_OK);
    std::wstring sTest1 = L"STRING_TEST";
    ASSERT_TRUE(data->set_wstring_value(sTest1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(data->get_int32_value(test, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring sTest2 = L"";
    ASSERT_FALSE(data->get_wstring_value(sTest2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_wstring_value(sTest2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        ASSERT_NE(data->set_wstring_value(L"TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), ReturnCode_t::RETCODE_OK);
    }

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //std::wstring wsTest;
    //ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    WStringStruct wwstring;
    WStringStructPubSubType wwstringpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(wwstringpb.deserialize(&dynamic_payload, &wwstring));

    uint32_t static_payloadSize = static_cast<uint32_t>(wwstringpb.getSerializedSizeProvider(&wwstring)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(wwstringpb.serialize(&wwstring, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_alias_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    std::string name = "ALIAS";
    DynamicTypeBuilder_cptr base_builder = factory.create_uint32_builder();
    ASSERT_TRUE(base_builder);
    DynamicTypeBuilder_ptr alias_builder = factory.create_alias_builder(*base_builder->build(), name);
    ASSERT_TRUE(alias_builder);

    DynamicType_ptr created_type = alias_builder->build();
    ASSERT_TRUE(created_type);
    DynamicType_ptr base_type = base_builder->build();
    ASSERT_TRUE(base_type);
    ASSERT_EQ(*created_type, *base_type);
    ASSERT_TRUE(created_type->equals(*base_type));

    EXPECT_EQ(created_type->get_name(), "ALIAS");
    DynamicData* aliasData = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(aliasData != nullptr);

    ASSERT_FALSE(aliasData->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(aliasData->set_string_value("", 1) == ReturnCode_t::RETCODE_OK);

    uint32_t uTest1 = 2;
    ASSERT_TRUE(aliasData->set_uint32_value(uTest1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    uint32_t uTest2 = 0;
    ASSERT_TRUE(aliasData->get_uint32_value(uTest2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(uTest1 == uTest2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(aliasData)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(aliasData, &payload));
    ASSERT_TRUE(payload.length == payloadSize);
    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(aliasData));

    // SERIALIZATION TEST
    AliasStruct walias;
    AliasStructPubSubType waliaspb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(aliasData, &dynamic_payload));
    ASSERT_TRUE(waliaspb.deserialize(&dynamic_payload, &walias));

    uint32_t static_payloadSize = static_cast<uint32_t>(waliaspb.getSerializedSizeProvider(&walias)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(waliaspb.serialize(&walias, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(aliasData));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(aliasData) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_nested_alias_unit_tests)
{
    // Check alias comparisson in dependent types
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // • Simple struct with nested aliases
    DynamicTypeBuilder_ptr plain_struct = factory.create_struct_builder(),
                           alias_struct = factory.create_struct_builder();
    EXPECT_TRUE(plain_struct && alias_struct);

    for (auto& build : { plain_struct, alias_struct })
    {
        build->set_name("base_struct");
    }

    //   Add members to the plain struct
    EXPECT_EQ(plain_struct->add_member(0, "int32", factory.create_int32_type()), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(plain_struct->add_member(1, "int64", factory.create_int64_type()), ReturnCode_t::RETCODE_OK);

    //   Add members to the alias struct
    EXPECT_EQ(alias_struct->add_member(0, "int32", factory.create_alias_builder(*factory.create_int32_type(), "int32_alias")->build()), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(alias_struct->add_member(1, "int64", factory.create_alias_builder(*factory.create_int64_type(), "int64_alias")->build()), ReturnCode_t::RETCODE_OK);

    //   Compare
    EXPECT_EQ(*plain_struct, *alias_struct);
    EXPECT_TRUE(plain_struct->equals(*alias_struct));
    EXPECT_TRUE(alias_struct->equals(*plain_struct));

    // • Inheritance from an alias
    DynamicTypeBuilder_ptr child_struct = factory.create_child_struct_builder(*plain_struct->build()),
                           child_alias_struct = factory.create_child_struct_builder(*alias_struct->build());

    for (auto& build : { child_struct, child_alias_struct })
    {
        build->set_name("child_struct");
    }

    //   Compare
    EXPECT_EQ(*child_struct, *child_alias_struct);
    EXPECT_TRUE(child_struct->equals(*child_alias_struct));
    EXPECT_TRUE(child_alias_struct->equals(*child_struct));

    // • Checking nesting at various levels
    int levels = 10;
    DynamicTypeBuilder_ptr nested_struct = child_struct,
                           nested_alias_struct = child_alias_struct;

    do
    {
        MemberId id = levels + 1;

        std::string member_name{"member"};
        member_name += std::to_string(id);

        std::string alias_name{"alias"};
        alias_name += std::to_string(id);

        std::string struct_name{"nested"};
        struct_name += std::to_string(id);

        auto aux = nested_struct->build();
        nested_struct = factory.create_child_struct_builder(*aux);
        ASSERT_TRUE(nested_struct);
        EXPECT_EQ(nested_struct->add_member(id, member_name, aux), ReturnCode_t::RETCODE_OK);

        aux = factory.create_alias_builder(*nested_alias_struct->build(), alias_name)->build();
        nested_alias_struct = factory.create_child_struct_builder(*aux);
        ASSERT_TRUE(nested_alias_struct);
        EXPECT_EQ(nested_alias_struct->add_member(id, member_name, aux), ReturnCode_t::RETCODE_OK);

        for (auto& build : { nested_struct, nested_alias_struct })
        {
            build->set_name(struct_name);
        }
    }
    while(--levels);

    EXPECT_EQ(*nested_struct, *nested_alias_struct);
    EXPECT_TRUE(nested_struct->equals(*nested_alias_struct));
    EXPECT_TRUE(nested_alias_struct->equals(*nested_struct));

    // • Checking serialization of aliases
    auto nested_type = nested_struct->build();
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(nested_type),
               * data2 = DynamicDataFactory::get_instance()->create_data(nested_type);
    ASSERT_NE(data, nullptr);
    ASSERT_NE(data2, nullptr);

    DynamicPubSubType pubsubType(nested_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_EQ(payload.length, payloadSize);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    ASSERT_EQ(DynamicDataFactory::get_instance()->delete_data(data), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), ReturnCode_t::RETCODE_OK);

    // • Checking serialization of nested aliases
    auto alias_type = alias_struct->build();
    data = DynamicDataFactory::get_instance()->create_data(alias_type),
    data2 = DynamicDataFactory::get_instance()->create_data(alias_type);
    ASSERT_NE(data, nullptr);
    ASSERT_NE(data2, nullptr);

    DynamicPubSubType pubsubAliasType(alias_type);
    payloadSize = static_cast<uint32_t>(pubsubAliasType.getSerializedSizeProvider(data)());
    SerializedPayload_t alias_payload(payloadSize);
    ASSERT_TRUE(pubsubAliasType.serialize(data, &alias_payload));
    ASSERT_EQ(alias_payload.length, payloadSize);
    ASSERT_TRUE(pubsubAliasType.deserialize(&alias_payload, data2));
    ASSERT_TRUE(data2->equals(data));

    ASSERT_EQ(DynamicDataFactory::get_instance()->delete_data(data), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_multi_alias_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    uint32_t length = 15;
    std::string name = "ALIAS";
    std::string name2 = "ALIAS2";

    DynamicType_ptr base_type = factory.create_string_type(length);
    ASSERT_TRUE(base_type);

    // alias
    DynamicTypeBuilder_ptr base_alias_builder = factory.create_alias_builder(*base_type, name);
    ASSERT_TRUE(base_alias_builder);
    DynamicType_ptr alias_type = base_alias_builder->build();
    ASSERT_TRUE(alias_type);
    EXPECT_EQ(alias_type->get_name(), name);

    // alias of an alias
    DynamicTypeBuilder_ptr alias_builder = factory.create_alias_builder(*alias_type, name2);
    ASSERT_TRUE(alias_builder);
    DynamicType_ptr created_type = alias_builder->build();
    ASSERT_TRUE(created_type);
    EXPECT_EQ(created_type->get_name(), name2);

    DynamicData* aliasData = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(aliasData != nullptr);

    ASSERT_FALSE(aliasData->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(aliasData->set_string_value("", 1) == ReturnCode_t::RETCODE_OK);
    std::string sTest1 = "STRING_TEST";
    ASSERT_TRUE(aliasData->set_string_value(sTest1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int test = 0;
    ASSERT_FALSE(aliasData->get_int32_value(test, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest2 = "";
    ASSERT_FALSE(aliasData->get_string_value(sTest2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(aliasData->get_string_value(sTest2, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(sTest1 == sTest2);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");
        ASSERT_NE(aliasData->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), ReturnCode_t::RETCODE_OK);
    }

    ASSERT_EQ(DynamicDataFactory::get_instance()->delete_data(aliasData), ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_bitset_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr base_type_builder = factory.create_byte_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_cptr base_type_builder2 = factory.create_uint32_builder();
    ASSERT_TRUE(base_type_builder2);
    auto base_type2 = base_type_builder2->build();

    DynamicTypeBuilder_ptr bitset_type_builder = factory.create_bitset_builder();
    ASSERT_TRUE(bitset_type_builder);

    // Add members to the struct.
    ASSERT_EQ(bitset_type_builder->add_member(0, "int2", base_type), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(bitset_type_builder->add_member(1, "int20", base_type2), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
            bitset_type_builder->apply_annotation_to_member(0, ANNOTATION_BIT_BOUND_ID, "value", "2"));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
            bitset_type_builder->apply_annotation_to_member(0, ANNOTATION_POSITION_ID, "value", "0"));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
            bitset_type_builder->apply_annotation_to_member(1, ANNOTATION_BIT_BOUND_ID, "value", "20"));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
            bitset_type_builder->apply_annotation_to_member(1, ANNOTATION_POSITION_ID, "value", "10")); // 8 bits empty

    auto bitset_type = bitset_type_builder->build();
    ASSERT_TRUE(bitset_type);
    auto bitset_data = DynamicDataFactory::get_instance()->create_data(bitset_type);
    ASSERT_TRUE(bitset_data);

    ASSERT_FALSE(bitset_data->set_int32_value(10, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(bitset_data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Set and get the child values.
    octet test1(234);
    ASSERT_TRUE(bitset_data->set_byte_value(test1, 0) == ReturnCode_t::RETCODE_OK);
    octet test2(0);
    ASSERT_TRUE(bitset_data->get_byte_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(test1 == test2);
    // 11101010
    // 00000010 (two bits)
    ASSERT_TRUE(test2 == 2);
    uint32_t test3(289582314);
    ASSERT_TRUE(bitset_data->set_uint32_value(test3, 1) == ReturnCode_t::RETCODE_OK);
    uint32_t test4(0);
    ASSERT_TRUE(bitset_data->get_uint32_value(test4, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(test3 == test4);
    // 00000001010000101010110011101010
    // 00000000000000101010110011101010 (20 bits)
    ASSERT_TRUE(test4 == 175338);

    // Bitset serialization
    // Tested in DynamicTypes_4_2_Tests

    // Delete the structure
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(bitset_data) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_bitmask_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    uint32_t limit = 5;
    DynamicTypeBuilder_ptr created_builder = factory.create_bitmask_builder(limit);
    ASSERT_TRUE(created_builder);

    // Add two members to the bitmask
    ASSERT_EQ(created_builder->add_member(0u, "TEST"), ReturnCode_t::RETCODE_OK);

    // Try to add a descriptor with the same name
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");

        EXPECT_NE(created_builder->add_member(1u, "TEST"), ReturnCode_t::RETCODE_OK);
        ASSERT_EQ(created_builder->add_member(1u, "TEST2"), ReturnCode_t::RETCODE_OK);
        ASSERT_EQ(created_builder->add_member(4u, "TEST4"), ReturnCode_t::RETCODE_OK);
        EXPECT_NE(created_builder->add_member(5u, "TEST5"), ReturnCode_t::RETCODE_OK); // Out of bounds
    }

    DynamicType_ptr created_type = created_builder->build();
    ASSERT_TRUE(created_type);
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(data != nullptr);

    MemberId testId = data->get_member_id_by_name("TEST");
    EXPECT_NE(testId, MEMBER_ID_INVALID);
    EXPECT_EQ(testId, 0u);
    MemberId test2Id = data->get_member_id_by_name("TEST2");
    EXPECT_NE(test2Id, MEMBER_ID_INVALID);
    EXPECT_EQ(test2Id, 1u);
    MemberId test4Id = data->get_member_id_by_name("TEST4");
    EXPECT_NE(test4Id, MEMBER_ID_INVALID);
    EXPECT_EQ(test4Id, 4u);
    MemberId test5Id = data->get_member_id_by_name("TEST5");
    EXPECT_EQ(test5Id, MEMBER_ID_INVALID);

    bool test1 = true;
    ASSERT_FALSE(data->set_int32_value(1, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->set_bool_value(test1, testId) == ReturnCode_t::RETCODE_OK);

    // Over the limit
    ASSERT_FALSE(data->set_bool_value(test1, limit + 1) == ReturnCode_t::RETCODE_OK);

    bool test2 = false;
    ASSERT_TRUE(data->get_bool_value(test2, 2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test2 == false);
    ASSERT_TRUE(data->get_bool_value(test2, testId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    ASSERT_TRUE(data->get_bool_value(test2, testId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    bool test3 = data->get_bool_value("TEST");
    ASSERT_TRUE(test1 == test3);
    ASSERT_TRUE(data->set_bool_value(true, "TEST4") == ReturnCode_t::RETCODE_OK);
    bool test4 = data->get_bool_value("TEST4");
    ASSERT_TRUE(test4 == true);

    test1 = false;
    ASSERT_TRUE(data->set_bool_value(test1, testId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_bool_value(test2, test2Id) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_bool_value(test2, testId) == ReturnCode_t::RETCODE_OK);
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

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    //uint64_t uTest64;
    //ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    ASSERT_TRUE(data->set_bool_value(true, 0) == ReturnCode_t::RETCODE_OK);
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    uint32_t length = 2;

    // Then
    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    DynamicTypeBuilder_ptr seq_type_builder = factory.create_sequence_builder(*base_type_builder->build(), length);
    ASSERT_TRUE(seq_type_builder);
    auto seq_type = seq_type_builder->build();
    ASSERT_TRUE(seq_type);

    auto data = DynamicDataFactory::get_instance()->create_data(seq_type);
    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Try to write on an empty position
    ASSERT_FALSE(data->set_int32_value(234, 1) == ReturnCode_t::RETCODE_OK);

    MemberId newId;
    ASSERT_TRUE(data->insert_sequence_data(newId) == ReturnCode_t::RETCODE_OK);
    MemberId newId2;
    ASSERT_TRUE(data->insert_sequence_data(newId2) == ReturnCode_t::RETCODE_OK);

    // Try to insert more than the limit.
    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable");

        MemberId newId3;
        ASSERT_FALSE(data->insert_sequence_data(newId3) == ReturnCode_t::RETCODE_OK);
    }

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(data->set_int32_value(test1, newId2) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(data->get_int32_value(test2, newId2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(seq_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(seq_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // Remove the elements.
    ASSERT_TRUE(data->remove_sequence_data(newId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);

    // New Insert Methods
    ASSERT_TRUE(data->insert_int32_value(test1, newId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_int32_value(test2, newId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);

    // Check that the sequence is empty.
    ASSERT_FALSE(data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // SERIALIZATION TEST
    SequenceStruct seq;
    SequenceStructPubSubType seqpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(seq_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_of_sequences_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    uint32_t sequence_length = 2;
    uint32_t sup_sequence_length = 3;

    // Then
    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);

    DynamicTypeBuilder_ptr seq_type_builder = factory.create_sequence_builder(*base_type_builder->build(), sequence_length);
    ASSERT_TRUE(seq_type_builder);
    auto seq_type = seq_type_builder->build();
    ASSERT_TRUE(seq_type);

    DynamicTypeBuilder_ptr seq_seq_type_builder = factory.create_sequence_builder(*seq_type_builder->build(), sup_sequence_length);
    ASSERT_TRUE(seq_seq_type_builder);
    auto seq_seq_type = seq_seq_type_builder->build();
    ASSERT_TRUE(seq_seq_type);

    auto data = DynamicDataFactory::get_instance()->create_data(seq_seq_type);
    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    MemberId newId;
    ASSERT_TRUE(data->insert_sequence_data(newId) == ReturnCode_t::RETCODE_OK);
    MemberId newId2;
    ASSERT_TRUE(data->insert_sequence_data(newId2) == ReturnCode_t::RETCODE_OK);

    // Loan Value to modify the first sequence
    auto seq_data = data->loan_value(newId);
    ASSERT_TRUE(seq_data != nullptr);

    MemberId newSeqId;
    ASSERT_TRUE(seq_data->insert_sequence_data(newSeqId) == ReturnCode_t::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(seq_data->set_int32_value(test1, newSeqId) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(seq_data->get_int32_value(test2, newSeqId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Return the pointer of the sequence
    ASSERT_TRUE(data->return_loaned_value(seq_data) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->return_loaned_value(seq_data) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(seq_seq_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(seq_seq_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // Remove the elements.
    ASSERT_TRUE(data->remove_sequence_data(newId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);

    // Check that the sequence is empty.
    ASSERT_FALSE(data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // SERIALIZATION TEST
    SequenceSequenceStruct seq;
    SequenceSequenceStructPubSubType seqpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(seq_seq_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    // New Insert Methods
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);
    seq_data = DynamicDataFactory::get_instance()->create_data(seq_type);
    ASSERT_TRUE(seq_data->insert_int32_value(test1, newSeqId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(seq_data->get_int32_value(test2, newSeqId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    ASSERT_TRUE(data->insert_complex_value(seq_data, newId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_array_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    std::vector<uint32_t> sequence_lengths = { 2, 2, 2 };

    // Then
    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_ptr array_type_builder = factory.create_array_builder(*base_type_builder->build(), sequence_lengths);
    ASSERT_TRUE(array_type_builder);
    auto array_type = array_type_builder->build();
    ASSERT_TRUE(array_type);

    auto data = DynamicDataFactory::get_instance()->create_data(array_type);
    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    MemberId newId;
    ASSERT_FALSE(data->insert_sequence_data(newId) == ReturnCode_t::RETCODE_OK);

    // Get an index in the multidimensional array.
    std::vector<uint32_t> vPosition = { 1, 1, 1 };
    MemberId testPos(0);
    testPos = data->get_array_index(vPosition);
    ASSERT_TRUE(testPos != MEMBER_ID_INVALID);

    // Invalid input vectors.
    std::vector<uint32_t> vPosition2 = { 1, 1 };
    ASSERT_FALSE(data->get_array_index(vPosition2) != MEMBER_ID_INVALID);
    std::vector<uint32_t> vPosition3 = { 1, 1, 1, 1 };
    ASSERT_FALSE(data->get_array_index(vPosition3) != MEMBER_ID_INVALID);

    // Set and get a value.
    int32_t test1 = 156;
    ASSERT_TRUE(data->set_int32_value(test1, testPos) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(data->get_int32_value(test2, testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(array_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(array_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // Check items count before and after remove an element.
    ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());
    ASSERT_TRUE(data->clear_value(testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());
    ASSERT_TRUE(data->clear_array_data(testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());

    // Check the clear values method
    ASSERT_TRUE(data->set_int32_value(test1, testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == array_type->get_total_bounds());

    // Try to set a value out of the array.
    ASSERT_FALSE(data->set_int32_value(test1, 100) == ReturnCode_t::RETCODE_OK);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // SERIALIZATION TEST
    ArraytStruct seq;
    ArraytStructPubSubType seqpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(array_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_array_of_arrays_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    std::vector<uint32_t> sequence_lengths = { 2, 2 };

    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_ptr array_type_builder = factory.create_array_builder(*base_type_builder->build(), sequence_lengths);
    ASSERT_TRUE(array_type_builder);
    auto array_type = array_type_builder->build();
    ASSERT_TRUE(array_type);

    DynamicTypeBuilder_ptr parent_array_type_builder = factory.create_array_builder(*array_type_builder->build(), sequence_lengths);
    ASSERT_TRUE(parent_array_type_builder);
    auto parent_array_type = parent_array_type_builder->build();
    ASSERT_TRUE(parent_array_type);

    eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

    auto data = DynamicDataFactory::get_instance()->create_data(parent_array_type);

    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    MemberId newId;
    ASSERT_FALSE(data->insert_sequence_data(newId) == ReturnCode_t::RETCODE_OK);

    // Get an index in the multidimensional array.
    std::vector<uint32_t> vPosition = { 1, 1 };
    MemberId testPos(0);
    testPos = data->get_array_index(vPosition);
    ASSERT_TRUE(testPos != MEMBER_ID_INVALID);

    // Invalid input vectors.
    std::vector<uint32_t> vPosition2 = { 1, 1, 1 };
    ASSERT_FALSE(data->get_array_index(vPosition2) != MEMBER_ID_INVALID);
    std::vector<uint32_t> vPosition3 = { 1, 1, 1, 1 };
    ASSERT_FALSE(data->get_array_index(vPosition3) != MEMBER_ID_INVALID);

    // Loan Complex values.
    DynamicData* temp = data->loan_value(testPos);
    ASSERT_TRUE(temp != nullptr);
    DynamicData* temp2 = data->loan_value(testPos);
    ASSERT_FALSE(temp2 != nullptr);

    int32_t test1 = 156;
    ASSERT_TRUE(temp->set_int32_value(test1, testPos) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(temp->get_int32_value(test2, testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(data->return_loaned_value(temp) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->return_loaned_value(temp) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->return_loaned_value(temp2) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(parent_array_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(parent_array_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // Check items count before and after remove an element.
    ASSERT_TRUE(data->get_item_count() == parent_array_type->get_total_bounds());
    ASSERT_TRUE(data->clear_value(testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == parent_array_type->get_total_bounds());
    ASSERT_TRUE(data->clear_array_data(testPos) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == parent_array_type->get_total_bounds());

    // Try to set a value out of the array.
    ASSERT_FALSE(data->set_int32_value(test1, 100) == ReturnCode_t::RETCODE_OK);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // SERIALIZATION TEST
    ArrayArrayStruct seq;
    ArrayArrayStructPubSubType seqpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(parent_array_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_map_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    uint32_t map_length = 2;

    // Then
    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_ptr map_type_builder =
            factory.create_map_builder(*base_type, *base_type, map_length);
    ASSERT_TRUE(map_type_builder);
    auto map_type = map_type_builder->build();
    ASSERT_TRUE(map_type);

    eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

    DynamicData* data = DynamicDataFactory::get_instance()->create_data(map_type);

    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Try to write on an empty position
    ASSERT_FALSE(data->set_int32_value(234, 0) == ReturnCode_t::RETCODE_OK);

    MemberId keyId;
    MemberId valueId;
    auto key_data = DynamicDataFactory::get_instance()->create_data(base_type);
    ASSERT_TRUE(data->insert_map_data(key_data, keyId, valueId) == ReturnCode_t::RETCODE_OK);

    // Try to Add the same key twice.
    ASSERT_FALSE(data->insert_map_data(key_data, keyId, valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data) == ReturnCode_t::RETCODE_OK);

    MemberId keyId2;
    MemberId valueId2;
    key_data = DynamicDataFactory::get_instance()->create_data(base_type);
    key_data->set_int32_value(2, MEMBER_ID_INVALID);
    ASSERT_TRUE(data->insert_map_data(key_data, keyId2, valueId2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data) == ReturnCode_t::RETCODE_OK);

    // Try to Add one more than the limit
    auto key_data2 = DynamicDataFactory::get_instance()->create_data(base_type);
    key_data2->set_int32_value(3, MEMBER_ID_INVALID);
    ASSERT_FALSE(data->insert_map_data(key_data2, keyId, valueId) == ReturnCode_t::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(data->set_int32_value(test1, valueId) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(data->get_int32_value(test2, valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(map_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(map_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // Check items count with removes
    ASSERT_TRUE(data->get_item_count() == 2);
    ASSERT_FALSE(data->remove_map_data(valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == 2);
    ASSERT_TRUE(data->remove_map_data(keyId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == 1);
    ASSERT_TRUE(data->clear_all_values() == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(data->get_item_count() == 0);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // SERIALIZATION TEST
    MapStruct seq;
    MapStructPubSubType seqpb;

    uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t dynamic_payload(payloadSize3);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(dynamic_payload.length == payloadSize3);
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(map_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data2) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_map_of_maps_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    uint32_t map_length = 2;

    // Then
    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_ptr map_type_builder =
        factory.create_map_builder(*base_type, *base_type, map_length);
    ASSERT_TRUE(map_type_builder);
    auto map_type = map_type_builder->build();
    ASSERT_TRUE(map_type);

    DynamicTypeBuilder_ptr map_map_type_builder =
            factory.create_map_builder(*base_type,*map_type, map_length);
    ASSERT_TRUE(map_map_type_builder);
    auto map_map_type = map_map_type_builder->build();
    ASSERT_TRUE(map_map_type);

    eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

    DynamicData* data = DynamicDataFactory::get_instance()->create_data(map_map_type);

    ASSERT_FALSE(data->set_int32_value(10, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    MemberId keyId;
    MemberId valueId;
    auto key_data = DynamicDataFactory::get_instance()->create_data(base_type);
    ASSERT_TRUE(data->insert_map_data(key_data, keyId, valueId) == ReturnCode_t::RETCODE_OK);

    // Try to Add the same key twice.
    ASSERT_FALSE(data->insert_map_data(key_data, keyId, valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data) == ReturnCode_t::RETCODE_OK);

    MemberId keyId2;
    MemberId valueId2;
    key_data = DynamicDataFactory::get_instance()->create_data(base_type);
    key_data->set_int32_value(2);
    ASSERT_TRUE(data->insert_map_data(key_data, keyId2, valueId2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data) == ReturnCode_t::RETCODE_OK);

    // Try to Add one more than the limit
    auto key_data2 = DynamicDataFactory::get_instance()->create_data(base_type);
    key_data2->set_int32_value(3, MEMBER_ID_INVALID);
    ASSERT_FALSE(data->insert_map_data(key_data2, keyId, valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data2) == ReturnCode_t::RETCODE_OK);

    auto seq_data = data->loan_value(valueId);
    ASSERT_TRUE(seq_data != nullptr);

    auto key_data3 = DynamicDataFactory::get_instance()->create_data(base_type);
    ASSERT_TRUE(seq_data->insert_map_data(key_data3, keyId, valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(key_data3) == ReturnCode_t::RETCODE_OK);

    // Set and get a value.
    int32_t test1(234);
    ASSERT_TRUE(seq_data->set_int32_value(test1, valueId) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(seq_data->get_int32_value(test2, valueId) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    ASSERT_TRUE(data->return_loaned_value(seq_data) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->return_loaned_value(seq_data) == ReturnCode_t::RETCODE_OK);

    ASSERT_FALSE(data->set_int32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint16_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_int64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_uint64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float32_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float64_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_float128_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char8_value('a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_char16_value(L'a', MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_byte_value(0, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_bool_value(false, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_wstring_value(L"", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(data->set_enum_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    int32_t iTest32;
    ASSERT_FALSE(data->get_int32_value(iTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint32_t uTest32;
    ASSERT_FALSE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int16_t iTest16;
    ASSERT_FALSE(data->get_int16_value(iTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint16_t uTest16;
    ASSERT_FALSE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    int64_t iTest64;
    ASSERT_FALSE(data->get_int64_value(iTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    uint64_t uTest64;
    ASSERT_FALSE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    float fTest32;
    ASSERT_FALSE(data->get_float32_value(fTest32, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    double fTest64;
    ASSERT_FALSE(data->get_float64_value(fTest64, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    long double fTest128;
    ASSERT_FALSE(data->get_float128_value(fTest128, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    char cTest8;
    ASSERT_FALSE(data->get_char8_value(cTest8, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    wchar_t cTest16;
    ASSERT_FALSE(data->get_char16_value(cTest16, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    octet oTest;
    ASSERT_FALSE(data->get_byte_value(oTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    bool bTest;
    ASSERT_FALSE(data->get_bool_value(bTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sTest;
    ASSERT_FALSE(data->get_string_value(sTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::wstring wsTest;
    ASSERT_FALSE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);
    std::string sEnumTest;
    ASSERT_FALSE(data->get_enum_value(sEnumTest, MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(map_map_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(map_map_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(data));

    // SERIALIZATION TEST
    MapMapStruct seq;
    MapMapStructPubSubType seqpb;

    uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(data)());
    SerializedPayload_t dynamic_payload(payloadSize3);
    ASSERT_TRUE(pubsubType.serialize(data, &dynamic_payload));
    ASSERT_TRUE(dynamic_payload.length == payloadSize3);
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(map_map_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_structure_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_cptr base_type_builder2 = factory.create_int64_builder();
    ASSERT_TRUE(base_type_builder2);
    auto base_type2 = base_type_builder2->build();

    DynamicTypeBuilder_ptr struct_type_builder = factory.create_struct_builder();
    ASSERT_TRUE(struct_type_builder);

    // Add members to the struct.
    ASSERT_TRUE(struct_type_builder->add_member(0, "int32", base_type) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(struct_type_builder->add_member(1, "int64", base_type2) == ReturnCode_t::RETCODE_OK);

    auto struct_type = struct_type_builder->build();
    ASSERT_TRUE(struct_type);
    auto struct_data = DynamicDataFactory::get_instance()->create_data(struct_type);
    ASSERT_TRUE(struct_data != nullptr);

    ASSERT_FALSE(struct_data->set_int32_value(10, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(struct_data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Set and get the child values.
    int32_t test1(234);
    ASSERT_TRUE(struct_data->set_int32_value(test1, 0) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(struct_data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    int64_t test3(234);
    ASSERT_TRUE(struct_data->set_int64_value(test3, 1) == ReturnCode_t::RETCODE_OK);
    int64_t test4(0);
    ASSERT_TRUE(struct_data->get_int64_value(test4, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(struct_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(struct_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(struct_data));

    // SERIALIZATION TEST
    StructStruct seq;
    StructStructPubSubType seqpb;

    uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
    SerializedPayload_t dynamic_payload(payloadSize3);
    ASSERT_TRUE(pubsubType.serialize(struct_data, &dynamic_payload));
    ASSERT_TRUE(dynamic_payload.length == payloadSize3);
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(struct_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(struct_data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);

    // Delete the structure
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(struct_data) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_structure_inheritance_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_cptr base_type_builder2 = factory.create_int64_builder();
    ASSERT_TRUE(base_type_builder2);
    auto base_type2 = base_type_builder2->build();

    DynamicTypeBuilder_ptr struct_type_builder = factory.create_struct_builder();
    ASSERT_TRUE(struct_type_builder);

    // Add members to the struct.
    EXPECT_EQ(struct_type_builder->add_member(0, "int32", base_type), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(struct_type_builder->add_member(1, "int64", base_type2), ReturnCode_t::RETCODE_OK);

    auto struct_type = struct_type_builder->build();
    ASSERT_TRUE(struct_type);

    // Create the child struct.
    DynamicTypeBuilder_ptr child_struct_type_builder = factory.create_child_struct_builder(*struct_type);
    ASSERT_TRUE(child_struct_type_builder);

    // Add a new member to the child struct.
    EXPECT_EQ(child_struct_type_builder->add_member(2, "child_int32", base_type), ReturnCode_t::RETCODE_OK);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging
        // try to add a member to override one of the parent struct.
        EXPECT_NE(child_struct_type_builder->add_member(3, "int32", base_type), ReturnCode_t::RETCODE_OK);
    }

    // Add a new member at front
    EXPECT_EQ(child_struct_type_builder->add_member(0, 3, "first_child", base_type2), ReturnCode_t::RETCODE_OK);

    // Add a new member at end
    EXPECT_EQ(child_struct_type_builder->add_member(INDEX_INVALID, 4, "last_child", base_type2), ReturnCode_t::RETCODE_OK);

    auto child_struct_type = child_struct_type_builder->build();
    ASSERT_TRUE(child_struct_type);

    // Validate the member related APIs

    EXPECT_EQ(child_struct_type->get_member_count(), 5u);

    MemberDescriptor members[5];

    EXPECT_EQ(child_struct_type->get_member(members[0], 0), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(members[0].get_name(), "int32");
    EXPECT_EQ(members[0].get_index(), 0u);
    EXPECT_EQ(members[0].get_id(), 0u);
    EXPECT_EQ(*members[0].get_type(), *base_type);

    EXPECT_EQ(child_struct_type->get_member(members[1], 1), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(members[1].get_name(), "int64");
    EXPECT_EQ(members[1].get_index(), 1u);
    EXPECT_EQ(members[1].get_id(), 1u);
    EXPECT_EQ(*members[1].get_type(), *base_type2);

    EXPECT_EQ(child_struct_type->get_member(members[2], 3), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(members[2].get_name(), "first_child");
    EXPECT_EQ(members[2].get_index(), 2u);
    EXPECT_EQ(members[2].get_id(), 3u);
    EXPECT_EQ(*members[2].get_type(), *base_type2);

    EXPECT_EQ(child_struct_type->get_member(members[3], 2), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(members[3].get_name(), "child_int32");
    EXPECT_EQ(members[3].get_index(), 3u);
    EXPECT_EQ(members[3].get_id(), 2u);
    EXPECT_EQ(*members[3].get_type(), *base_type);

    EXPECT_EQ(child_struct_type->get_member(members[4], 4), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(members[4].get_name(), "last_child");
    EXPECT_EQ(members[4].get_index(), 4u);
    EXPECT_EQ(members[4].get_id(), 4u);
    EXPECT_EQ(*members[4].get_type(), *base_type2);

    for (auto& m : members)
    {
        EXPECT_TRUE(child_struct_type->exists_member_by_name(m.get_name()));
        EXPECT_TRUE(child_struct_type->exists_member_by_id(m.get_id()));
        EXPECT_EQ(child_struct_type->get_member_id_by_name(m.get_name()), m.get_id());
        EXPECT_EQ(child_struct_type->get_member_id_at_index(m.get_index()), m.get_id());

        MemberDescriptor aux;
        EXPECT_EQ(child_struct_type->get_member_by_index(aux, m.get_index()), ReturnCode_t::RETCODE_OK);
        EXPECT_EQ(aux, m);

        EXPECT_EQ(child_struct_type->get_member_by_name(aux, m.get_name()), ReturnCode_t::RETCODE_OK);
        EXPECT_EQ(aux, m);
    }

    auto member_seq = child_struct_type->get_all_members();
    EXPECT_EQ(member_seq.size(), 5u);
    EXPECT_TRUE(std::equal(member_seq.begin(), member_seq.end(), members,
            [](const MemberDescriptor* a, const MemberDescriptor& b) -> bool { return *a == b; }));

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
    auto id_map = child_struct_type->get_all_members_by_id();
    EXPECT_EQ(id_map.size(), 5u);
    std::transform(id_map.begin(), id_map.end(), std::inserter(aux, aux.end()),
            [](std::pair<const MemberId, const DynamicTypeMember*>& p)
            {
                return *p.second;
            });
    EXPECT_TRUE(std::equal(aux.begin(), aux.end(), members));

    // Validating data management

    auto struct_data = DynamicDataFactory::get_instance()->create_data(child_struct_type);
    ASSERT_TRUE(struct_data != nullptr);

    // Setting invalid types should fail
    EXPECT_NE(struct_data->set_int32_value(10, 1), ReturnCode_t::RETCODE_OK);
    EXPECT_NE(struct_data->set_string_value("", MEMBER_ID_INVALID), ReturnCode_t::RETCODE_OK);

    // Set and get the parent values.
    int32_t test1(234);
    EXPECT_EQ(struct_data->set_int32_value(test1, 0), ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    EXPECT_EQ(struct_data->get_int32_value(test2, 0), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(test1, test2);

    int64_t test3(234);
    EXPECT_EQ(struct_data->set_int64_value(test3, 1), ReturnCode_t::RETCODE_OK);
    int64_t test4(0);
    EXPECT_EQ(struct_data->get_int64_value(test4, 1), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(test3, test4);

    // Set and get the child value.
    EXPECT_EQ(struct_data->set_int32_value(test1, 2), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(struct_data->get_int32_value(test2, 2), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(struct_data->set_int64_value(test3, 3), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(struct_data->get_int64_value(test4, 3), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(test3, test4);

    EXPECT_EQ(struct_data->set_int64_value(test3, 4), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(struct_data->get_int64_value(test4, 4), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(test3, test4);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(child_struct_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(child_struct_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(struct_data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(struct_data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_multi_structure_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_cptr base_type_builder2 = factory.create_int64_builder();
    ASSERT_TRUE(base_type_builder2);
    auto base_type2 = base_type_builder2->build();

    DynamicTypeBuilder_ptr struct_type_builder = factory.create_struct_builder();
    ASSERT_TRUE(struct_type_builder);

    // Add members to the struct.
    ASSERT_TRUE(struct_type_builder->add_member(0, "int32", base_type) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(struct_type_builder->add_member(1, "int64", base_type2) == ReturnCode_t::RETCODE_OK);

    auto struct_type = struct_type_builder->build();
    ASSERT_TRUE(struct_type);

    // Create the parent struct.
    DynamicTypeBuilder_ptr parent_struct_type_builder = factory.create_struct_builder();
    ASSERT_TRUE(parent_struct_type_builder);

    // Add members to the parent struct.
    ASSERT_TRUE(parent_struct_type_builder->add_member(0, "child_struct", struct_type) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(parent_struct_type_builder->add_member(1, "child_int64", base_type2) == ReturnCode_t::RETCODE_OK);

    auto parent_struct_type = parent_struct_type_builder->build();
    ASSERT_TRUE(parent_struct_type);

    auto struct_data = DynamicDataFactory::get_instance()->create_data(parent_struct_type);
    ASSERT_TRUE(struct_data != nullptr);

    ASSERT_FALSE(struct_data->set_int32_value(10, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(struct_data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    // Set and get the child values.
    int64_t test1(234);
    ASSERT_TRUE(struct_data->set_int64_value(test1, 1) == ReturnCode_t::RETCODE_OK);
    int64_t test2(0);
    ASSERT_TRUE(struct_data->get_int64_value(test2, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);

    auto child_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(child_struct_data != nullptr);

    // Set and get the child values.
    int32_t test3(234);
    ASSERT_TRUE(child_struct_data->set_int32_value(test3, 0) == ReturnCode_t::RETCODE_OK);
    int32_t test4(0);
    ASSERT_TRUE(child_struct_data->get_int32_value(test4, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);
    int64_t test5(234);
    ASSERT_TRUE(child_struct_data->set_int64_value(test5, 1) == ReturnCode_t::RETCODE_OK);
    int64_t test6(0);
    ASSERT_TRUE(child_struct_data->get_int64_value(test6, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test5 == test6);

    ASSERT_TRUE(struct_data->return_loaned_value(child_struct_data) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(struct_data->return_loaned_value(child_struct_data) == ReturnCode_t::RETCODE_OK);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(parent_struct_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(struct_data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(parent_struct_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(struct_data));

    // SERIALIZATION TEST
    StructStructStruct seq;
    StructStructStructPubSubType seqpb;

    uint32_t payloadSize3 = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(struct_data)());
    SerializedPayload_t dynamic_payload(payloadSize3);
    ASSERT_TRUE(pubsubType.serialize(struct_data, &dynamic_payload));
    ASSERT_TRUE(dynamic_payload.length == payloadSize3);
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(parent_struct_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(struct_data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(struct_data) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_union_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr discriminant_builder = factory.create_int32_builder();
    ASSERT_TRUE(discriminant_builder);
    auto discriminant_type = discriminant_builder->build();
    ASSERT_TRUE(discriminant_type);

    auto member_type = discriminant_type;
    ASSERT_TRUE(member_type);

    DynamicTypeBuilder_cptr another_member_builder = factory.create_int64_builder();
    auto another_member_type = another_member_builder->build();
    ASSERT_TRUE(another_member_type);

    DynamicTypeBuilder_ptr union_type_builder = factory.create_union_builder(*discriminant_type);
    ASSERT_TRUE(union_type_builder);

    // Add members to the union.
    // A plain braced-init-list cannot be used for the labels because that would inhibit
    // template argument deduction, see § 14.8.2.5/5 of the C++11 standard
    ASSERT_EQ(union_type_builder->add_member(0, "first", member_type, "", std::vector<uint64_t>{ 0 }, true),
              ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(union_type_builder->add_member(1, "second", another_member_type, "", std::vector<uint64_t>{ 1 }, false),
              ReturnCode_t::RETCODE_OK);

    {
        eprosima::fastdds::dds::Log::ScopeLogs _("disable"); // avoid expected errors logging

        // Try to add a second "DEFAULT" value to the union
        ASSERT_FALSE(union_type_builder->add_member(0, "third", member_type, "", std::vector<uint64_t>{ 0 },
                    true) == ReturnCode_t::RETCODE_OK);

        // Try to add a second value to the same case label
        ASSERT_FALSE(union_type_builder->add_member(0, "third", member_type, "", std::vector<uint64_t>{ 1 },
                    false) == ReturnCode_t::RETCODE_OK);
    }

    // Create a data of this union
    auto union_type = union_type_builder->build();
    ASSERT_TRUE(union_type);
    auto union_data = DynamicDataFactory::get_instance()->create_data(union_type);
    ASSERT_TRUE(union_data != nullptr);

    // Set and get the child values.
    ASSERT_FALSE(union_data->set_int32_value(10, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(union_data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    uint64_t label;
    ASSERT_TRUE(union_data->get_union_label(label) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(label == 0);

    int32_t test1(234);
    ASSERT_TRUE(union_data->set_int32_value(test1, 0) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(union_data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    ASSERT_TRUE(union_data->get_union_label(label) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(label == 0);

    int64_t test3(234);
    int64_t test4(0);

    // Try to get values from invalid indexes and from an invalid element ( not the current one )
    ASSERT_FALSE(union_data->get_int32_value(test2, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(union_data->get_int64_value(test4, 1) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(union_data->set_int64_value(test3, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(union_data->get_int64_value(test4, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);
    ASSERT_TRUE(union_data->get_union_label(label) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(label == 1);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(union_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(union_data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(union_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(union_data));

    // SERIALIZATION TEST
    SimpleUnionStruct seq;
    SimpleUnionStructPubSubType seqpb;

    SerializedPayload_t dynamic_payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(union_data, &dynamic_payload));
    ASSERT_TRUE(dynamic_payload.length == payloadSize);
    ASSERT_TRUE(seqpb.deserialize(&dynamic_payload, &seq));

    uint32_t static_payloadSize = static_cast<uint32_t>(seqpb.getSerializedSizeProvider(&seq)());
    SerializedPayload_t static_payload(static_payloadSize);
    ASSERT_TRUE(seqpb.serialize(&seq, &static_payload));
    ASSERT_TRUE(static_payload.length == static_payloadSize);
    types::DynamicData* data3 = DynamicDataFactory::get_instance()->create_data(union_type);
    ASSERT_TRUE(pubsubType.deserialize(&static_payload, data3));
    ASSERT_TRUE(data3->equals(union_data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data3) == ReturnCode_t::RETCODE_OK);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(union_data) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_union_with_unions_unit_tests)
{
    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_cptr base_type_builder = factory.create_int32_builder();
    ASSERT_TRUE(base_type_builder);
    auto base_type = base_type_builder->build();

    DynamicTypeBuilder_cptr base_type_builder2 = factory.create_int64_builder();
    ASSERT_TRUE(base_type_builder2);
    auto base_type2 = base_type_builder2->build();

    DynamicTypeBuilder_ptr union_type_builder = factory.create_union_builder(*base_type);
    ASSERT_TRUE(union_type_builder);

    // Add members to the union.
    ASSERT_TRUE(union_type_builder->add_member(0, "first", base_type, "", std::vector<uint64_t>{ 0 }, true) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(union_type_builder->add_member(1, "second", base_type2, "", std::vector<uint64_t>{ 1 },
                false) == ReturnCode_t::RETCODE_OK);

    // Try to add a second "DEFAULT" value to the union
    ASSERT_FALSE(union_type_builder->add_member(0, "third", base_type, "", std::vector<uint64_t>{ 0 },
                true) == ReturnCode_t::RETCODE_OK);

    // Try to add a second value to the same case label
    ASSERT_FALSE(union_type_builder->add_member(0, "third", base_type, "", std::vector<uint64_t>{ 1 },
                false) == ReturnCode_t::RETCODE_OK);

    // Create a data of this union
    auto union_type = union_type_builder->build();
    ASSERT_TRUE(union_type != nullptr);

    DynamicTypeBuilder_ptr parent_union_type_builder = factory.create_union_builder(*base_type);
    ASSERT_TRUE(parent_union_type_builder);

    // Add Members to the parent union
    ASSERT_TRUE(parent_union_type_builder->add_member(0, "first", base_type, "", std::vector<uint64_t>{ 0 },
                true) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(parent_union_type_builder->add_member(1, "second", union_type, "", std::vector<uint64_t>{ 1 },
                false) == ReturnCode_t::RETCODE_OK);

    DynamicType_ptr created_type = parent_union_type_builder->build();
    ASSERT_TRUE(created_type);
    auto union_data = DynamicDataFactory::get_instance()->create_data(parent_union_type_builder.get());
    ASSERT_TRUE(union_data != nullptr);

    // Set and get the child values.
    ASSERT_FALSE(union_data->set_int32_value(10, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(union_data->set_string_value("", MEMBER_ID_INVALID) == ReturnCode_t::RETCODE_OK);

    uint64_t label;
    ASSERT_TRUE(union_data->get_union_label(label) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(label == 0);

    int32_t test1(234);
    ASSERT_TRUE(union_data->set_int32_value(test1, 0) == ReturnCode_t::RETCODE_OK);
    int32_t test2(0);
    ASSERT_TRUE(union_data->get_int32_value(test2, 0) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test1 == test2);
    ASSERT_TRUE(union_data->get_union_label(label) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(label == 0);

    // Loan Value ( Activates this union id )
    DynamicData* child_data = union_data->loan_value(1);
    ASSERT_TRUE(child_data != 0);

    int64_t test3(234);
    int64_t test4(0);

    // Try to get values from invalid indexes and from an invalid element ( not the current one )
    ASSERT_FALSE(child_data->get_int32_value(test2, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(child_data->get_int64_value(test4, 1) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(child_data->set_int64_value(test3, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(child_data->get_int64_value(test4, 1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(test3 == test4);

    ASSERT_TRUE(union_data->return_loaned_value(child_data) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(union_data->get_union_label(label) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(label == 1);

    // Serialize <-> Deserialize Test
    DynamicPubSubType pubsubType(created_type);
    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(union_data)());
    SerializedPayload_t payload(payloadSize);
    ASSERT_TRUE(pubsubType.serialize(union_data, &payload));
    ASSERT_TRUE(payload.length == payloadSize);

    types::DynamicData* data2 = DynamicDataFactory::get_instance()->create_data(created_type);
    ASSERT_TRUE(pubsubType.deserialize(&payload, data2));
    ASSERT_TRUE(data2->equals(union_data));

    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(data2) == ReturnCode_t::RETCODE_OK);

    // Delete the map
    ASSERT_TRUE(DynamicDataFactory::get_instance()->delete_data(union_data) == ReturnCode_t::RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_XML_EnumStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("EnumStruct");

        DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

        // Enum
        DynamicTypeBuilder_ptr enum_builder = factory.create_enum_builder();
        enum_builder->add_member(0, "A");
        enum_builder->add_member(1, "B");
        enum_builder->add_member(2, "C");
        enum_builder->set_name("MyEnum");

        // Struct EnumStruct
        DynamicTypeBuilder_ptr es_builder = factory.create_struct_builder();
        es_builder->add_member(0, "my_enum", enum_builder->build());
        es_builder->set_name("EnumStruct");

        ASSERT_EQ(*pbType->GetDynamicType(), *es_builder);

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesTests, DynamicType_XML_AliasStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("AliasStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Enum
    DynamicTypeBuilder_ptr enum_builder = factory.create_enum_builder();
    enum_builder->add_member(0, "A");
    enum_builder->add_member(1, "B");
    enum_builder->add_member(2, "C");
    enum_builder->set_name("MyEnum");
    DynamicType_ptr enum_type = enum_builder->build();

    // Alias
    DynamicTypeBuilder_ptr alias_builder = factory.create_alias_builder(*enum_type, "MyAliasEnum");
    DynamicType_ptr alias_type = alias_builder->build();

    // Struct AliasStruct
    DynamicTypeBuilder_ptr struct_alias_builder = factory.create_struct_builder();
    struct_alias_builder->add_member(0, "my_alias", alias_type);
    struct_alias_builder->set_name("AliasStruct");
    DynamicType_ptr struct_alias_type = struct_alias_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *struct_alias_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*struct_alias_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_AliasAliasStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("AliasAliasStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Enum
    DynamicTypeBuilder_ptr enum_builder = factory.create_enum_builder();
    enum_builder->add_member(0, "A");
    enum_builder->add_member(1, "B");
    enum_builder->add_member(2, "C");
    enum_builder->set_name("MyEnum");
    DynamicType_ptr enum_type = enum_builder->build();

    // Alias and aliasalias
    DynamicTypeBuilder_ptr alias_builder = factory.create_alias_builder(*enum_type, "MyAliasEnum");
    DynamicType_ptr alias_type = alias_builder->build();
    DynamicTypeBuilder_ptr alias_alias_builder = factory.create_alias_builder(*alias_type, "MyAliasAliasEnum");
    DynamicType_ptr alias_alias_type = alias_alias_builder->build();

    // Struct AliasAliasStruct
    DynamicTypeBuilder_ptr aliasAliasS_builder = factory.create_struct_builder();
    aliasAliasS_builder->add_member(0, "my_alias_alias", alias_alias_type);
    aliasAliasS_builder->set_name("AliasAliasStruct");
    DynamicType_ptr aliasAliasS_type = aliasAliasS_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*aliasAliasS_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*aliasAliasS_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_BoolStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("BoolStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Boolean
    DynamicType_ptr boolean_type = factory.create_bool_type();

    // Struct BoolStruct
    DynamicTypeBuilder_ptr bool_builder = factory.create_struct_builder();
    bool_builder->add_member(0, "my_bool", boolean_type);
    bool_builder->set_name("BoolStruct");
    DynamicType_ptr bool_type = bool_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *bool_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*bool_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_OctetStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("OctetStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Byte
    DynamicType_ptr byte_type = factory.create_byte_type();

    // Struct OctetStruct
    DynamicTypeBuilder_ptr octet_builder = factory.create_struct_builder();
    octet_builder->add_member(0, "my_octet", byte_type);
    octet_builder->set_name("OctetStruct");
    DynamicType_ptr octet_type = octet_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*octet_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*octet_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ShortStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Int16
    DynamicType_ptr byte_type = factory.create_int16_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr int16_builder = factory.create_struct_builder();
    int16_builder->add_member(0, "my_int16", byte_type);
    int16_builder->set_name("ShortStruct");
    DynamicType_ptr int16_type = int16_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*int16_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*int16_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_LongStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("LongStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Int32
    DynamicType_ptr byte_type = factory.create_int32_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr int32_builder = factory.create_struct_builder();
    int32_builder->add_member(0, "my_int32", byte_type);
    int32_builder->set_name("LongStruct");
    DynamicType_ptr int32_type = int32_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*int32_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*int32_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_LongLongStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("LongLongStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Int32
    DynamicType_ptr byte_type = factory.create_int64_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr int64_builder = factory.create_struct_builder();
    int64_builder->add_member(0, "my_int64", byte_type);
    int64_builder->set_name("LongLongStruct");
    DynamicType_ptr int64_type = int64_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*int64_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*int64_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_UShortStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("UShortStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // uint16
    DynamicType_ptr byte_type = factory.create_uint16_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr uint16_builder = factory.create_struct_builder();
    uint16_builder->add_member(0, "my_uint16", byte_type);
    uint16_builder->set_name("UShortStruct");
    DynamicType_ptr uint16_type = uint16_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*uint16_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*uint16_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ULongStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ULongStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // uint32
    DynamicType_ptr byte_type = factory.create_uint32_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr uint32_builder = factory.create_struct_builder();
    uint32_builder->add_member(0, "my_uint32", byte_type);
    uint32_builder->set_name("ULongStruct");
    DynamicType_ptr uint32_type = uint32_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*uint32_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*uint32_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ULongLongStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ULongLongStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // uint64
    DynamicType_ptr byte_type = factory.create_uint64_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr uint64_builder = factory.create_struct_builder();
    uint64_builder->add_member(0, "my_uint64", byte_type);
    uint64_builder->set_name("ULongLongStruct");
    DynamicType_ptr uint64_type = uint64_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*uint64_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*uint64_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_FloatStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("FloatStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // float32
    DynamicType_ptr byte_type = factory.create_float32_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr float32_builder = factory.create_struct_builder();
    float32_builder->add_member(0, "my_float32", byte_type);
    float32_builder->set_name("FloatStruct");
    DynamicType_ptr float32_type = float32_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*float32_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*float32_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_DoubleStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("DoubleStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // float64
    DynamicType_ptr byte_type = factory.create_float64_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr float64_builder = factory.create_struct_builder();
    float64_builder->add_member(0, "my_float64", byte_type);
    float64_builder->set_name("DoubleStruct");
    DynamicType_ptr float64_type = float64_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*float64_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*float64_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_LongDoubleStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("LongDoubleStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // float128
    DynamicType_ptr byte_type = factory.create_float128_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr float128_builder = factory.create_struct_builder();
    float128_builder->add_member(0, "my_float128", byte_type);
    float128_builder->set_name("LongDoubleStruct");
    DynamicType_ptr float128_type = float128_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*float128_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*float128_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_CharStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("CharStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // char8
    DynamicType_ptr byte_type = factory.create_char8_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr char8_builder = factory.create_struct_builder();
    char8_builder->add_member(0, "my_char", byte_type);
    char8_builder->set_name("CharStruct");
    DynamicType_ptr char8_type = char8_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*char8_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*char8_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_WCharStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("WCharStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // wchar
    DynamicType_ptr byte_type = factory.create_char16_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr char16_builder = factory.create_struct_builder();
    char16_builder->add_member(0, "my_wchar", byte_type);
    char16_builder->set_name("WCharStruct");
    DynamicType_ptr char16_type = char16_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*char16_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*char16_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_StringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("StringStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // string
    DynamicType_ptr byte_type = factory.create_string_type();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr string_builder = factory.create_struct_builder();
    string_builder->add_member(0, "my_string", byte_type);
    string_builder->set_name("StringStruct");
    DynamicType_ptr string_type = string_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*string_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*string_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_WStringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("WStringStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // wstring
    DynamicTypeBuilder_ptr string_builder = factory.create_builder_copy(*factory.create_wstring_builder());
    string_builder->set_name("strings_255");
    DynamicType_ptr string_type = string_builder->build();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr wstring_builder = factory.create_struct_builder();
    wstring_builder->add_member(0, "my_wstring", string_type);
    wstring_builder->set_name("WStringStruct");
    DynamicType_ptr wstring_type = wstring_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*wstring_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*wstring_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_LargeStringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("LargeStringStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // string
    DynamicType_ptr byte_type = factory.create_string_type(41925);

    // Struct ShortStruct
    DynamicTypeBuilder_ptr string_builder = factory.create_struct_builder();
    string_builder->add_member(0, "my_large_string", byte_type);
    string_builder->set_name("LargeStringStruct");
    DynamicType_ptr string_type = string_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*string_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*string_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_LargeWStringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("LargeWStringStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // wstring
    DynamicTypeBuilder_ptr string_builder = factory.create_builder_copy(*factory.create_wstring_builder(41925));
    string_builder->set_name("wstringl_41925");
    DynamicType_ptr string_type = string_builder->build();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr wstring_builder = factory.create_struct_builder();
    wstring_builder->add_member(0, "my_large_wstring", string_type);
    wstring_builder->set_name("LargeWStringStruct");
    DynamicType_ptr wstring_type = wstring_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*wstring_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*wstring_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ShortStringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStringStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // string
    DynamicType_ptr byte_type = factory.create_string_type(15);

    // Struct ShortStruct
    DynamicTypeBuilder_ptr string_builder = factory.create_struct_builder();
    string_builder->add_member(0, "my_short_string", byte_type);
    string_builder->set_name("ShortStringStruct");
    DynamicType_ptr string_type = string_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*string_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*string_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ShortWStringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ShortWStringStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // wstring
    DynamicTypeBuilder_ptr string_builder = factory.create_builder_copy(*factory.create_wstring_builder(15));
    string_builder->set_name("wstrings_15");
    DynamicType_ptr string_type = string_builder->build();

    // Struct ShortStruct
    DynamicTypeBuilder_ptr wstring_builder = factory.create_struct_builder();
    wstring_builder->add_member(0, "my_short_wstring", string_type);
    wstring_builder->set_name("ShortWStringStruct");
    DynamicType_ptr wstring_type = wstring_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*wstring_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*wstring_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_AliasStringStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("StructAliasString");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // String
    DynamicType_ptr string_type = factory.create_string_type();

    // Alias
    DynamicType_ptr myAlias_type = factory.create_alias_type(*string_type, "MyAliasString");

    // Struct StructAliasString
    DynamicTypeBuilder_ptr alias_string_builder = factory.create_struct_builder();
    alias_string_builder->add_member(0, "my_alias_string", myAlias_type);
    alias_string_builder->set_name("StructAliasString");
    DynamicType_ptr alias_string_type = alias_string_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *alias_string_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*alias_string_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_StructAliasWString_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    {
        DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("StructAliasWString");

        DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

        // wstring
        DynamicTypeBuilder_ptr wstring_builder = factory.create_builder_copy(
                *factory.create_wstring_type());
        wstring_builder->set_name("strings_255");
        DynamicType_ptr wstring_type = wstring_builder->build();

        // Alias
        DynamicType_ptr myAlias_type =
                factory.create_alias_type(*wstring_type, "MyAliasWString");

        // Struct StructAliasWString
        DynamicTypeBuilder_ptr alias_wstring_builder = factory.create_struct_builder();
        alias_wstring_builder->add_member(0, "my_alias_wstring", myAlias_type);
        alias_wstring_builder->set_name("StructAliasWString");
        DynamicType_ptr alias_wstring_type = alias_wstring_builder->build();

        EXPECT_EQ(*pbType->GetDynamicType(), *alias_wstring_type);
        EXPECT_TRUE(pbType->GetDynamicType()->equals(*alias_wstring_type));

        delete(pbType);
        XMLProfileManager::DeleteInstance();
    }
}

TEST_F(DynamicTypesTests, DynamicType_XML_ArraytStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ArraytStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Int32
    DynamicType_ptr int32_type = factory.create_int32_type();

    // Array
    DynamicTypeBuilder_ptr array_builder = factory.create_array_builder(*int32_type, { 2, 2, 2 });

    // Struct ShortWStringStruct
    DynamicTypeBuilder_ptr array_int32_builder = factory.create_struct_builder();
    array_int32_builder->add_member(0, "my_array", array_builder->build());
    array_int32_builder->set_name("ArraytStruct");
    DynamicType_ptr array_int32_type = array_int32_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*array_int32_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*array_int32_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ArrayArrayStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Typedef aka Alias
    DynamicTypeBuilder_ptr array_builder = factory.create_array_builder(
            *factory.create_int32_type(),
            { 2, 2 });
    DynamicTypeBuilder_ptr myArray_builder = factory.create_alias_builder(*array_builder->build(), "MyArray");

    // Struct ArrayArrayStruct
    DynamicTypeBuilder_ptr aas_builder = factory.create_struct_builder();
    DynamicTypeBuilder_ptr aMyArray_builder = factory.create_array_builder(*myArray_builder->build(), { 2, 2 });
    aas_builder->add_member(0, "my_array_array", aMyArray_builder->build());
    aas_builder->set_name("ArrayArrayStruct");
    DynamicType_ptr aas_type = aas_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *aas_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*aas_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_ArrayArrayArrayStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ArrayArrayArrayStruct");

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
    DynamicTypeBuilder_ptr array_builder = factory.create_array_builder(
            *factory.create_int32_type(),
            { 2, 2 });
    DynamicTypeBuilder_ptr myArray_builder = factory.create_alias_builder(
            *array_builder->build(),
            "MyArray");

    // Struct ArrayArrayStruct
    DynamicTypeBuilder_ptr aas_builder = factory.create_struct_builder();
    DynamicTypeBuilder_ptr aMyArray_builder = factory.create_array_builder(
            *myArray_builder->build(),
            { 2, 2 });
    aas_builder->add_member(0, "my_array_array", aMyArray_builder->build());
    aas_builder->set_name("ArrayArrayStruct");

    // Struct ArrayArrayArrayStruct
    DynamicTypeBuilder_ptr aaas_builder = factory.create_struct_builder();
    DynamicTypeBuilder_ptr aas_array_builder = factory.create_array_builder(
            *aas_builder->build(),
            { 2, 2 });
    aaas_builder->add_member(0, "my_array_array_array", aas_array_builder->build());
    aaas_builder->set_name("ArrayArrayArrayStruct");
    DynamicType_ptr aaas_type = aaas_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *aaas_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*aaas_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_SequenceStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr seq_builder = factory.create_sequence_builder(
            *factory.create_int32_type(),
            2);

    DynamicTypeBuilder_ptr seqs_builder = factory.create_struct_builder();
    seqs_builder->add_member(0, "my_sequence", seq_builder->build());
    seqs_builder->set_name("SequenceStruct");
    DynamicType_ptr seqs_type = seqs_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *seqs_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*seqs_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_SequenceSequenceStruct_test)
{
    using namespace xmlparser;
    using namespace types;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("SequenceSequenceStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr seq_builder = factory.create_sequence_builder(
            *factory.create_int32_type(),
            2);
    DynamicTypeBuilder_ptr alias_builder = factory.create_alias_builder(
            *seq_builder->build(),
            "my_sequence_sequence_inner");

    DynamicTypeBuilder_ptr sss_builder = factory.create_struct_builder();
    DynamicTypeBuilder_ptr seq_seq_builder = factory.create_sequence_builder(*alias_builder->build(), 2);
    sss_builder->add_member(0, "my_sequence_sequence", seq_seq_builder->build());
    sss_builder->set_name("SequenceSequenceStruct");
    DynamicType_ptr sss_type = sss_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *sss_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*sss_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_MapStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("MapStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr map_builder = factory.create_map_builder(
            *factory.create_int32_type(),
            *factory.create_int32_type(),
            2);

    DynamicTypeBuilder_ptr maps_builder = factory.create_struct_builder();
    maps_builder->add_member(0, "my_map", map_builder->build());
    maps_builder->set_name("MapStruct");
    DynamicType_ptr maps_type = maps_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *maps_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*maps_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_MapMapStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("MapMapStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicType_ptr int32_type = factory.create_int32_type();
    DynamicTypeBuilder_ptr map_builder = factory.create_map_builder(
            *int32_type,
            *int32_type,
            2);
    DynamicTypeBuilder_ptr alias_builder = factory.create_alias_builder(
            *map_builder->build(),
            "my_map_map_inner");
    DynamicTypeBuilder_ptr map_map_builder = factory.create_map_builder(
            *int32_type,
            *alias_builder->build(),
            2);

    DynamicTypeBuilder_ptr maps_builder = factory.create_struct_builder();
    maps_builder->add_member(0, "my_map_map", map_map_builder->build());
    maps_builder->set_name("MapMapStruct");
    DynamicType_ptr maps_type = maps_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *maps_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*maps_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_StructStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("StructStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr structs_builder = factory.create_struct_builder();
    structs_builder->add_member(0, "a", factory.create_int32_type());
    structs_builder->add_member(1, "b", factory.create_int64_type());
    structs_builder->set_name("StructStruct");
    DynamicType_ptr structs_type = structs_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *structs_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*structs_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_StructStructStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("StructStructStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr structs_builder = factory.create_struct_builder();
    structs_builder->add_member(0, "a", factory.create_int32_type());
    structs_builder->add_member(1, "b", factory.create_int64_type());
    structs_builder->set_name("StructStruct");

    DynamicTypeBuilder_ptr sss_builder = factory.create_struct_builder();
    sss_builder->add_member(0, "child_struct", structs_builder->build());
    sss_builder->add_member(1, "child_int64", factory.create_int64_type());
    sss_builder->set_name("StructStructStruct");
    DynamicType_ptr sss_type = sss_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *sss_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*sss_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_SimpleUnionStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("SimpleUnionStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicType_ptr int32_type = factory.create_int32_type();

    DynamicTypeBuilder_ptr union_builder = factory.create_union_builder(*int32_type);
    union_builder->add_member(0, "first", int32_type, "", std::vector<uint64_t>{ 0 }, true);
    union_builder->add_member(1, "second", factory.create_int64_type(), "", std::vector<uint64_t>{ 1 }, false);
    union_builder->set_name("SimpleUnion");

    DynamicTypeBuilder_ptr us_builder = factory.create_struct_builder();
    us_builder->add_member(0, "my_union", union_builder->build());
    us_builder->set_name("SimpleUnionStruct");
    DynamicType_ptr us_type = us_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *us_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*us_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_UnionUnionStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("UnionUnionStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicType_ptr int32_type = factory.create_int32_type();

    DynamicTypeBuilder_ptr union_builder = factory.create_union_builder(*int32_type);
    union_builder->add_member(0, "first", int32_type, "", std::vector<uint64_t>{ 0 }, true);
    union_builder->add_member(1, "second", factory.create_int64_type(), "", std::vector<uint64_t>{ 1 }, false);
    union_builder->set_name("SimpleUnion");

    DynamicTypeBuilder_ptr union_union_builder = factory.create_union_builder(*int32_type);
    union_union_builder->add_member(0, "first", int32_type, "", std::vector<uint64_t>{ 0 }, true);
    union_union_builder->add_member(1, "second", union_builder->build(), "", std::vector<uint64_t>{ 1 }, false);
    union_union_builder->set_name("UnionUnion");

    DynamicTypeBuilder_ptr uus_builder = factory.create_struct_builder();
    uus_builder->add_member(0, "my_union", union_union_builder->build());
    uus_builder->set_name("UnionUnionStruct");
    DynamicType_ptr uus_type = uus_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *uus_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*uus_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_WCharUnionStruct_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("WCharUnionStruct");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    DynamicTypeBuilder_ptr union_builder = factory.create_union_builder(*factory.create_char16_type());
    union_builder->add_member(0, "first", factory.create_int32_type(), "", std::vector<uint64_t>{ 0 }, true);
    union_builder->add_member(1, "second", factory.create_int64_type(), "", std::vector<uint64_t>{ 1 }, false);
    union_builder->set_name("WCharUnion");

    DynamicTypeBuilder_ptr us_builder = factory.create_struct_builder();
    us_builder->add_member(0, "my_union", union_builder->build());
    us_builder->set_name("WCharUnionStruct");
    DynamicType_ptr us_type = us_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(), *us_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*us_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_bounded_string_unit_tests)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ShortStringStruct");
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(pbType->GetDynamicType());

    // SERIALIZATION TEST
    StringStruct refData;
    StringStructPubSubType refDatapb;

    uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    SerializedPayload_t dynamic_payload(payloadSize);
    EXPECT_TRUE(pbType->serialize(data, &dynamic_payload));
    EXPECT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

    uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
    SerializedPayload_t static_payload(static_payloadSize);
    EXPECT_TRUE(refDatapb.serialize(&refData, &static_payload));
    EXPECT_EQ(static_payload.length, static_payloadSize);
    EXPECT_NE(data->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), ReturnCode_t::RETCODE_OK);

    EXPECT_TRUE(DynamicDataFactory::get_instance()->delete_data(data) == ReturnCode_t::RETCODE_OK);
    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_bounded_wstring_unit_tests)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("ShortWStringStruct");
    DynamicData* data = DynamicDataFactory::get_instance()->create_data(pbType->GetDynamicType());

    // SERIALIZATION TEST
    StringStruct refData;
    StringStructPubSubType refDatapb;

    uint32_t payloadSize = static_cast<uint32_t>(pbType->getSerializedSizeProvider(data)());
    SerializedPayload_t payload(payloadSize);
    SerializedPayload_t dynamic_payload(payloadSize);
    EXPECT_TRUE(pbType->serialize(data, &dynamic_payload));
    EXPECT_TRUE(refDatapb.deserialize(&dynamic_payload, &refData));

    uint32_t static_payloadSize = static_cast<uint32_t>(refDatapb.getSerializedSizeProvider(&refData)());
    SerializedPayload_t static_payload(static_payloadSize);
    EXPECT_TRUE(refDatapb.serialize(&refData, &static_payload));
    EXPECT_EQ(static_payload.length, static_payloadSize);
    EXPECT_NE(data->set_string_value("TEST_OVER_LENGTH_LIMITS", MEMBER_ID_INVALID), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), ReturnCode_t::RETCODE_OK);

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_Bitset_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);
    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("MyBitSet");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    auto a_type = factory.create_char8_type();
    auto b_type = factory.create_bool_type();
    auto c_type = factory.create_uint16_type();
    auto d_type = factory.create_int16_type();

/*
 XML:
    <bitset name="MyBitSet">
        <bitfield name="a" bit_bound="3"/>
        <bitfield name="b" bit_bound="1"/>
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
    DynamicTypeBuilder_ptr bitset_builder = factory.create_bitset_builder();

    bitset_builder->add_member(0, "a", a_type);
    bitset_builder->apply_annotation_to_member(0, ANNOTATION_BIT_BOUND_ID, "value", "3");
    bitset_builder->apply_annotation_to_member(0, ANNOTATION_POSITION_ID, "value", "0");

    bitset_builder->add_member(1, "b", b_type);
    bitset_builder->apply_annotation_to_member(1, ANNOTATION_BIT_BOUND_ID, "value", "1");
    bitset_builder->apply_annotation_to_member(1, ANNOTATION_POSITION_ID, "value", "3");

    bitset_builder->add_member(2, "", a_type);
    // The member doesn't exist so the annotation application will fail, and isn't needed.
    //bitset_builder->apply_annotation_to_member(2, ANNOTATION_BIT_BOUND_ID, "value", "4");
    //bitset_builder->apply_annotation_to_member(2, ANNOTATION_POSITION_ID, "value", "4");

    bitset_builder->add_member(3, "c", c_type);
    bitset_builder->apply_annotation_to_member(3, ANNOTATION_BIT_BOUND_ID, "value", "10");
    bitset_builder->apply_annotation_to_member(3, ANNOTATION_POSITION_ID, "value", "8"); // 4 empty

    bitset_builder->add_member(4, "d", d_type);
    bitset_builder->apply_annotation_to_member(4, ANNOTATION_BIT_BOUND_ID, "value", "12");
    bitset_builder->apply_annotation_to_member(4, ANNOTATION_POSITION_ID, "value", "18");

    bitset_builder->set_name("MyBitSet");
    DynamicType_ptr bitset_type = bitset_builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*bitset_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*bitset_type));

    delete(pbType);
    XMLProfileManager::DeleteInstance();
}

TEST_F(DynamicTypesTests, DynamicType_XML_Bitmask_test)
{
    using namespace xmlparser;

    XMLP_ret ret = XMLProfileManager::loadXMLFile(DynamicTypesTests::config_file());
    ASSERT_EQ(ret, XMLP_ret::XML_OK);

    DynamicPubSubType* pbType = XMLProfileManager::CreateDynamicPubSubType("MyBitMask");

    DynamicTypeBuilderFactory& factory = DynamicTypeBuilderFactory::get_instance();

    // Bitset
    DynamicTypeBuilder_ptr builder = factory.create_bitmask_builder(8);
    builder->add_member(0, "flag0");
    builder->add_member(1, "flag1");
    builder->add_member(2, "flag2");
    builder->add_member(5, "flag5");
    builder->set_name("MyBitMask");
    DynamicType_ptr builder_type = builder->build();

    EXPECT_EQ(*pbType->GetDynamicType(),*builder_type);
    EXPECT_TRUE(pbType->GetDynamicType()->equals(*builder_type));

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

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
