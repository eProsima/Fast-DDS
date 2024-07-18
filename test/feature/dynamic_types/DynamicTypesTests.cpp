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

#include <array>
#include <functional>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <ScopedLogs.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

std::array<DataRepresentationId_t, 2> encodings {{XCDR_DATA_REPRESENTATION, XCDR2_DATA_REPRESENTATION}};

void encoding_decoding_test(
        DynamicType::_ref_type created_type,
        DynamicData::_ref_type encoding_data,
        DynamicData::_ref_type decoding_data,
        DataRepresentationId_t encoding
        )
{
    TypeSupport pubsubType {new DynamicPubSubType(created_type)};
    uint32_t payloadSize =
            static_cast<uint32_t>(pubsubType.calculate_serialized_size(&encoding_data, encoding));
    SerializedPayload_t payload(payloadSize);
    EXPECT_TRUE(pubsubType.serialize(&encoding_data, payload, encoding));
    EXPECT_EQ(payload.length, payloadSize);
    EXPECT_LE(payload.length, pubsubType->max_serialized_type_size);
    EXPECT_TRUE(pubsubType.deserialize(payload, &decoding_data));
    EXPECT_TRUE(decoding_data->equals(encoding_data));

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(
        0, PARTICIPANT_QOS_DEFAULT);
    if (0 == created_type->get_name().size())
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType, "test"));
    }
    else
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType));
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

// Testing the primitive creation APIS
// and get_primitive_type().
class DynamicTypesPrimitiveTestsAPIs
    : public testing::TestWithParam <TypeKind>
{
};

TEST_P(DynamicTypesPrimitiveTestsAPIs, primitives_apis)
{
    // Get the factory singleton
    traits<DynamicTypeBuilderFactory>::ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    ASSERT_TRUE(factory);

    // Retrieve parameters
    TypeKind kind {GetParam()};

    // Get the primitive type through factory.
    traits<DynamicType>::ref_type type1 {factory->get_primitive_type(kind)};
    ASSERT_TRUE(type1);

    // It must be the right type kind.
    ASSERT_EQ(type1->get_kind(), kind);

    // The primitive type is statically allocated and must always be the same instance
    traits<DynamicType>::ref_type type2 {factory->get_primitive_type(kind)};
    ASSERT_TRUE(type2);
    EXPECT_EQ(type1, type2);
    EXPECT_TRUE(type1->equals(type2));

    // It must be possible to create a custom builder from a primitive one
    traits<DynamicTypeBuilder>::ref_type custom_builder {factory->create_type_copy(type1)};
    ASSERT_TRUE(custom_builder);

    // Builder's content must be equal than the copied type
    EXPECT_TRUE(custom_builder->equals(type1));

    traits<DynamicType>::ref_type custom_type1 {custom_builder->build()};
    ASSERT_TRUE(custom_type1);
    EXPECT_NE(type1, custom_type1);

    // Content must be equal with the builder
    EXPECT_TRUE(custom_builder->equals(custom_type1));

    // Content must be equal if there are not changes
    traits<DynamicType>::ref_type custom_type2 {custom_builder->build()};
    ASSERT_TRUE(custom_type2);
    EXPECT_NE(custom_type1, custom_type2);
    EXPECT_TRUE(custom_type1->equals(custom_type2));

    EXPECT_EQ(RETCODE_OK,
            DynamicTypeBuilderFactory::get_instance()->delete_type(custom_type1));
    EXPECT_EQ(RETCODE_OK,
            DynamicTypeBuilderFactory::get_instance()->delete_type(custom_type2));
}

INSTANTIATE_TEST_SUITE_P(CheckingGetPrimitiveType,
        DynamicTypesPrimitiveTestsAPIs,
        testing::Values(
            TK_INT8,
            TK_UINT8,
            TK_INT16,
            TK_UINT16,
            TK_INT32,
            TK_UINT32,
            TK_INT64,
            TK_UINT64,
            TK_FLOAT32,
            TK_FLOAT64,
            TK_FLOAT128,
            TK_CHAR8,
            TK_CHAR16,
            TK_BOOLEAN,
            TK_BYTE));

class DynamicTypesTests : public ::testing::Test
{
    const std::string config_file_ = "types_profile.xml";

public:

    DynamicTypesTests() = default;

    ~DynamicTypesTests()
    {
        Log::Flush();
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

};


TEST_F(DynamicTypesTests, TypeDescriptors)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    //{{{ Test descriptor of a primitive type.
    {
        DynamicType::_ref_type primitive {factory->get_primitive_type(TK_INT32)};
        ASSERT_TRUE(primitive);
        // Create a modifiable builder copy
        DynamicTypeBuilder::_ref_type builder {factory->create_type_copy(primitive)};
        ASSERT_TRUE(builder);
        EXPECT_EQ(builder->get_kind(), TK_INT32);

        // Retrieve the type's descriptor.
        TypeDescriptor::_ref_type state {traits<TypeDescriptor>::make_shared()};
        ASSERT_EQ(primitive->get_descriptor(state), RETCODE_OK);
        EXPECT_FALSE(state->base_type());
        EXPECT_TRUE(state->bound().empty());
        EXPECT_FALSE(state->discriminator_type());
        EXPECT_FALSE(state->element_type());
        EXPECT_EQ(state->extensibility_kind(), ExtensibilityKind::APPENDABLE);
        EXPECT_FALSE(state->is_nested());
        EXPECT_FALSE(state->key_element_type());
        EXPECT_EQ(TK_INT32, state->kind());
        EXPECT_EQ(state->name(), "");
        EXPECT_TRUE(state->is_consistent());
        DynamicTypeBuilder::_ref_type builder2 {factory->create_type(state)};
        ASSERT_TRUE(builder2);
        EXPECT_TRUE(builder2->equals(primitive));

        // Copy descriptor
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
    //}}}

    //{{{ Test descriptor of a union type.
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        descriptor->name("union");
        descriptor->kind(TK_UNION);
        descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        DynamicType::_ref_type type {factory->create_type(descriptor)->build()};
        ASSERT_TRUE(type);
        // Create a modifiable builder copy
        DynamicTypeBuilder::_ref_type builder {factory->create_type_copy(type)};
        ASSERT_TRUE(builder);
        EXPECT_EQ(builder->get_kind(), TK_UNION);

        // Retrieve the type's descriptor.
        TypeDescriptor::_ref_type state {traits<TypeDescriptor>::make_shared()};
        ASSERT_EQ(type->get_descriptor(state), RETCODE_OK);
        EXPECT_FALSE(state->base_type());
        EXPECT_EQ(state->bound(), BoundSeq{});
        EXPECT_TRUE(state->discriminator_type());
        EXPECT_EQ(state->discriminator_type(), factory->get_primitive_type(TK_UINT32));
        EXPECT_FALSE(state->element_type());
        EXPECT_EQ(state->extensibility_kind(), ExtensibilityKind::APPENDABLE);
        EXPECT_FALSE(state->is_nested());
        EXPECT_FALSE(state->key_element_type());
        EXPECT_EQ(TK_UNION, state->kind());
        EXPECT_EQ(state->name(), "union");
        EXPECT_TRUE(state->is_consistent());
        DynamicTypeBuilder::_ref_type builder2 {factory->create_type(state)};
        ASSERT_TRUE(builder2);
        EXPECT_TRUE(builder2->equals(type));

        // Copy descriptor
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
    //}}}

    //{{{ Test descriptor of a map type.
    {
        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        descriptor->name("map");
        descriptor->kind(TK_MAP);
        descriptor->bound({4});
        descriptor->key_element_type(factory->get_primitive_type(TK_INT8));
        descriptor->element_type(factory->get_primitive_type(TK_FLOAT32));
        DynamicType::_ref_type type {factory->create_type(descriptor)->build()};
        ASSERT_TRUE(type);
        // Create a modifiable builder copy
        DynamicTypeBuilder::_ref_type builder {factory->create_type_copy(type)};
        ASSERT_TRUE(builder);
        EXPECT_EQ(builder->get_kind(), TK_MAP);

        // Retrieve the type's descriptor.
        TypeDescriptor::_ref_type state {traits<TypeDescriptor>::make_shared()};
        ASSERT_EQ(type->get_descriptor(state), RETCODE_OK);
        EXPECT_FALSE(state->base_type());
        EXPECT_EQ(state->bound(), BoundSeq({4}));
        EXPECT_FALSE(state->discriminator_type());
        EXPECT_EQ(state->element_type(), factory->get_primitive_type(TK_FLOAT32));
        EXPECT_EQ(state->extensibility_kind(), ExtensibilityKind::APPENDABLE);
        EXPECT_FALSE(state->is_nested());
        EXPECT_TRUE(state->key_element_type());
        EXPECT_EQ(state->key_element_type(), factory->get_primitive_type(TK_INT8));
        EXPECT_EQ(TK_MAP, state->kind());
        EXPECT_EQ(state->name(), "map");
        EXPECT_TRUE(state->is_consistent());
        DynamicTypeBuilder::_ref_type builder2 {factory->create_type(state)};
        ASSERT_TRUE(builder2);
        EXPECT_TRUE(builder2->equals(type));

        // Copy descriptor
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
    //}}}

    //{{{ Test descriptor of an struct type.
    {
        // Create a base type.
        TypeDescriptor::_ref_type base_descriptor {traits<TypeDescriptor>::make_shared()};
        base_descriptor->name("base");
        base_descriptor->kind(TK_STRUCTURE);
        base_descriptor->extensibility_kind(ExtensibilityKind::FINAL);
        DynamicType::_ref_type base_type {factory->create_type(base_descriptor)->build()};

        TypeDescriptor::_ref_type descriptor {traits<TypeDescriptor>::make_shared()};
        descriptor->name("struct");
        descriptor->kind(TK_STRUCTURE);
        descriptor->base_type(base_type);
        descriptor->is_nested(true);
        descriptor->extensibility_kind(ExtensibilityKind::FINAL);
        DynamicType::_ref_type type {factory->create_type(descriptor)->build()};
        ASSERT_TRUE(type);
        // Create a modifiable builder copy
        DynamicTypeBuilder::_ref_type builder {factory->create_type_copy(type)};
        ASSERT_TRUE(builder);
        EXPECT_EQ(builder->get_kind(), TK_STRUCTURE);

        // Retrieve the type's descriptor.
        TypeDescriptor::_ref_type state {traits<TypeDescriptor>::make_shared()};
        ASSERT_EQ(type->get_descriptor(state), RETCODE_OK);
        EXPECT_TRUE(state->base_type());
        EXPECT_EQ(state->base_type(), base_type);
        EXPECT_EQ(state->bound(), BoundSeq{});
        EXPECT_FALSE(state->discriminator_type());
        EXPECT_FALSE(state->element_type());
        EXPECT_EQ(state->extensibility_kind(), ExtensibilityKind::FINAL);
        EXPECT_TRUE(state->is_nested());
        EXPECT_FALSE(state->key_element_type());
        EXPECT_EQ(TK_STRUCTURE, state->kind());
        EXPECT_EQ(state->name(), "struct");
        EXPECT_TRUE(state->is_consistent());
        DynamicTypeBuilder::_ref_type builder2 {factory->create_type(state)};
        ASSERT_TRUE(builder2);
        EXPECT_TRUE(builder2->equals(type));

        // Copy descriptor
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
    //}}}
}

TEST_F(DynamicTypesTests, DynamicType_basic)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create basic types
    TypeDescriptor::_ref_type struct_descriptor {traits<TypeDescriptor>::make_shared()};
    struct_descriptor->kind(TK_STRUCTURE);
    struct_descriptor->name("mystructure");
    EXPECT_TRUE(struct_descriptor->is_consistent());
    DynamicTypeBuilder::_ref_type struct_type_builder {factory->create_type(struct_descriptor)};
    ASSERT_TRUE(struct_type_builder);
    EXPECT_EQ(struct_type_builder->get_kind(), TK_STRUCTURE);
    EXPECT_EQ(struct_type_builder->get_name(), "mystructure");
    EXPECT_EQ(struct_type_builder->get_member_count(), 0u);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->id(3);
    member_descriptor->name("int32");
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    EXPECT_EQ(RETCODE_OK, struct_type_builder->add_member(member_descriptor));
    EXPECT_EQ(struct_type_builder->get_member_count(), 1u);

    DynamicType::_ref_type struct_type {struct_type_builder->build()};
    ASSERT_TRUE(struct_type);
    EXPECT_TRUE(struct_type_builder->equals(struct_type));

    member_descriptor->id(1);
    member_descriptor->name("int64");
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    EXPECT_EQ(RETCODE_OK, struct_type_builder->add_member(member_descriptor));
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
    md1->type(factory->get_primitive_type(TK_INT32));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member(member, 3));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 0u);
    EXPECT_EQ(md->id(), 3u);
    EXPECT_EQ(md->name(), md1->name());
    EXPECT_EQ(md->type(), md1->type());

    // • checking MemberDescriptor comparison and construction
    MemberDescriptor::_ref_type md2 = traits<MemberDescriptor>::make_shared();
    md2->id(1);
    md2->name("int64");
    md2->type(factory->get_primitive_type(TK_INT64));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->get_member(member, 1));
    ASSERT_EQ(RETCODE_OK, member->get_descriptor(md));
    EXPECT_EQ(md->index(), 1u);
    EXPECT_EQ(md->id(), 1u);
    EXPECT_EQ(md->name(), md2->name());
    EXPECT_EQ(md->type(), md2->type());

    EXPECT_FALSE(md->equals(md2));

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
    EXPECT_EQ(RETCODE_OK, struct_type_builder->get_all_members(members_by_id));
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
    EXPECT_EQ(RETCODE_OK, struct_type_builder->get_all_members_by_name(members_by_name));
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
    md->type(factory->get_primitive_type(TK_BOOLEAN));
    ASSERT_EQ(RETCODE_OK, struct_type_builder->add_member(md));

    EXPECT_EQ(RETCODE_OK, struct_type_builder->get_all_members(members_by_id));
    ASSERT_EQ(3, members_by_id.size());

    MemberDescriptor::_ref_type tmp = traits<MemberDescriptor>::make_shared();
    auto dm = members_by_id[3];
    ASSERT_EQ(RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_EQ(tmp->index(), 0u);
    EXPECT_EQ(tmp->id(), 3u);
    EXPECT_EQ(tmp->name(), md1->name());
    EXPECT_EQ(tmp->type(), md1->type());

    dm = members_by_id[7];
    ASSERT_EQ(RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_EQ(tmp->index(), 2u);
    EXPECT_EQ(tmp->id(), 7u);
    EXPECT_EQ(tmp->name(), md->name());
    EXPECT_EQ(tmp->type(), md->type());

    dm = members_by_id[1];
    ASSERT_EQ(RETCODE_OK, dm->get_descriptor(tmp));
    EXPECT_EQ(tmp->index(), 1u);
    EXPECT_EQ(tmp->id(), 1u);
    EXPECT_EQ(tmp->name(), md2->name());
    EXPECT_EQ(tmp->type(), md2->type());


    // • checking adding duplicates
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        //    + duplicate name
        md = traits<MemberDescriptor>::make_shared();
        md->id(8);
        md->name("int32");
        md->type(factory->get_primitive_type(TK_INT32));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_type_builder->add_member(md));

        //    + duplicate id
        md = traits<MemberDescriptor>::make_shared();
        md->id(7);
        md->name("dup_bool");
        md->type(factory->get_primitive_type(TK_BOOLEAN));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_type_builder->add_member(md));
    }
}

TEST_F(DynamicTypesTests, DynamicTypeBuilderFactory_create_strings)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // • strings
    DynamicTypeBuilder::_ref_type created_builder {factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))};
    ASSERT_TRUE(created_builder);

    DynamicType::_ref_type type {created_builder->build()};
    ASSERT_TRUE(type);
    DynamicType::_ref_type type2 {created_builder->build()};
    ASSERT_TRUE(type2);

    EXPECT_TRUE(type->equals(type2));

    EXPECT_EQ(RETCODE_OK, factory->delete_type(type));
    EXPECT_EQ(RETCODE_OK, factory->delete_type(type2));

    // • wstrings
    created_builder = factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED));
    ASSERT_TRUE(created_builder);

    type = created_builder->build();
    ASSERT_TRUE(type);
    type2 = created_builder->build();
    ASSERT_TRUE(type2);

    EXPECT_TRUE(type->equals(type2));

    EXPECT_EQ(RETCODE_OK, factory->delete_type(type));
    EXPECT_EQ(RETCODE_OK, factory->delete_type(type2));
    EXPECT_FALSE(type);
    EXPECT_FALSE(type2);
}

TEST_F(DynamicTypesTests, DynamicType_int32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_INT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test setters and getters.
    const int32_t test1 {123};
    int32_t test2 {0};

    //{{{ Successful setters
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(100, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232, test2);

    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 101), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(101, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 303), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(303, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(iTest64, 1);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest64, 1);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest128, 1);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        int32_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        EXPECT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        encoding_decoding_test(created_type, data, data2, encoding);
        ASSERT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_UINT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint32_t test1 = 123;
    uint32_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232u, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 303), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(303u, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_uint32_value(test2, 0), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(iTest64, 1);

    uint64_t uTest64;
    EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(uTest64, 1);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest64, 1);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest128, 1);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        uint32_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_int8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_INT8)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const int8_t test1 = 123;
    int8_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_int8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_int8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_int8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int8_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int16_t iTest16;
    EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest16);

    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest32);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        int8_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int8_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_int8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_int8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_int8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_UINT8)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint8_t test1 = 123;
    uint8_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_uint8_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int16_t iTest16;
    EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest16);

    uint16_t uTest16;
    EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, uTest16);

    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest32);

    uint32_t uTest32;
    EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, uTest32);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest64);

    uint64_t uTest64;
    EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, uTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        uint8_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint8_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_uint8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_int16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_INT16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const int16_t test1 = 123;
    int16_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(100, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int16_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest32);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        int16_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_int16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_UINT16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint16_t test1 = 123;
    uint16_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232u, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_uint16_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest32);

    uint32_t uTest32;
    EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, uTest32);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest64);

    uint64_t uTest64;
    EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, uTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        uint16_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_uint16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_int64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_INT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const int64_t test1 = 123;
    int64_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 3003), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(3003, test2);

    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 2003), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2003, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 30), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(30, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(10, test2);

    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 300), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(300, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(100, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'b'), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(98, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int64_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        int64_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_int64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_uint64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_UINT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const uint64_t test1 = 123;
    uint64_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 3004), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(3004u, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232u, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(100u, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1u, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_uint64_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_NE(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
        uint32_t uTest32;
        EXPECT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
        int16_t iTest16;
        EXPECT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
        uint16_t uTest16;
        EXPECT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
        int64_t iTest64;
        EXPECT_NE(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
        float fTest32;
        EXPECT_NE(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
        double fTest64;
        EXPECT_NE(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
        char cTest8;
        EXPECT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
        wchar_t cTest16;
        EXPECT_NE(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
        octet oTest;
        EXPECT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
        bool bTest;
        EXPECT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
        std::string sTest;
        EXPECT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
        std::wstring wsTest;
        EXPECT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        uint64_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_uint64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_float32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_FLOAT32)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const float test1 = 123.0f;
    float test2 = 0.0f;

    //{{{ Successful setters
    EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(100, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232, test2);

    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 200), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(200, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(10, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_float32_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        float test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_float32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_float32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_float64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_FLOAT64)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const double test1 = 123.0;
    double test2 = 0.0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, -100), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-100, test2);

    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 1000), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1000, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 100), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(100, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232, test2);

    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, -10), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-10, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 10), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(10, test2);

    EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 11), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(11, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'b'), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(98, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_float64_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        double test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_float64_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_float64_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_float128)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_FLOAT128)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const long double test1 = 123.0;
    long double test2 = 0.0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, -3000), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-3000, test2);

    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 3000), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(3000, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, -20), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-20, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 200), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(200, test2);

    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, -200), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-200, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 200), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(200, test2);

    EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, -2000), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-2000, test2);

    EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 2000), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2000, test2);

    EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 20), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(20, test2);

    EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 30), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(30, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'b'), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(98, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_float128_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        long double test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_float128_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_float128_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_char8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_CHAR8)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const char test1 = 'a';
    char test2 = 'b';

    //{{{ Successful setters
    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_char8_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest32);

    int16_t iTest16;
    EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest16);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);

    wchar_t cTest16;
    EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, cTest16);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        char test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_char8_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_char8_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_char16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_CHAR16)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const wchar_t test1 = L'a';
    wchar_t test2 = L'b';

    //{{{ Successful setters
    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 1), RETCODE_OK);
    EXPECT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_char16_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest32);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32;
        EXPECT_NE(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_NE(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
        uint16_t uTest16;
        EXPECT_NE(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
        uint64_t uTest64;
        EXPECT_NE(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
        char cTest8;
        EXPECT_NE(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
        octet oTest;
        EXPECT_NE(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_OK);
        bool bTest;
        EXPECT_NE(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
        std::string sTest;
        EXPECT_NE(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_OK);
        std::wstring wsTest;
        EXPECT_NE(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_OK);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        wchar_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_char16_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_char16_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_byte)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_BYTE)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const octet test1 {255};
    octet test2 {0};

    //{{{ Successful setters
    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_byte_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(iTest32, test1);

    uint32_t uTest32;
    EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(uTest32, test1);

    int8_t iTest8;
    EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(static_cast<uint8_t>(iTest8), test1);

    uint8_t uTest8;
    EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(uTest8, test1);

    int16_t iTest16;
    EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(iTest16, test1);

    uint16_t uTest16;
    EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(uTest16, test1);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(iTest64, test1);

    uint64_t uTest64;
    EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(uTest64, test1);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest32, test1);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest64, test1);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest128, test1);

    char cTest8;
    EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(cTest8, static_cast<char>(test1));

    wchar_t cTest16;
    EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(cTest16, test1);

    bool bTest;
    EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(bTest, true);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        octet test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_byte_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_byte_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_boolean)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicType::_ref_type created_type {factory->get_primitive_type(TK_BOOLEAN)};
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    const bool test1 = true;
    bool test2 = false;

    //{{{ Successful setters
    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(false, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_boolean_value(test2, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int32_t iTest32 {1};
    EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, iTest32);

    uint32_t uTest32 {1};
    EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, uTest32);

    int8_t iTest8 {1};
    EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, iTest8);

    uint8_t uTest8 {1};
    EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, uTest8);

    int16_t iTest16 {1};
    EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, iTest16);

    uint16_t uTest16 {1};
    EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, uTest16);

    int64_t iTest64 {1};
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, iTest64);

    uint64_t uTest64 {1};
    EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, uTest64);

    float fTest32 {1};
    EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, fTest32);

    double fTest64 {1};
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, fTest64);

    long double fTest128 {1};
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    for (auto encoding : encodings)
    {
        bool test3 {false};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_boolean_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_boolean_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_enum)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ENUM);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with base_type being not nil.
        type_descriptor->base_type(factory->get_primitive_type(TK_INT32));
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with bound being not nil.
        type_descriptor->bound({1});
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->bound({});
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with element_type set
        type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicType::_ref_type created_type {builder->build()};
        // Enumerator without literals are invalid.
        EXPECT_FALSE(created_type);
    }

    // Add three members to the enum.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    {
        // Negative case: descriptor without name.
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }
    member_descriptor->name("DEFAULT");
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    {
        // Negative case: descriptor type is unsigned integer
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("FIRST");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("SECOND");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        member_descriptor = traits<MemberDescriptor>::make_shared();
        // Try to add a descriptor with diferent kind.
        member_descriptor->type(factory->get_primitive_type(TK_INT16));
        member_descriptor->name("THIRD");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        // Try to add a descriptor with a MemberId.
        member_descriptor->id(100);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->id(MEMBER_ID_INVALID);
        // Try to add a descriptor with is_key set.
        member_descriptor->is_key(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_key(false);
        // Try to add a descriptor with is_must_understand set.
        member_descriptor->is_must_understand(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_must_understand(false);
        // Try to add a descriptor with is_optional set.
        member_descriptor->is_optional(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_optional(false);
        // Try to add a descriptor with is_shared set.
        member_descriptor->is_shared(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_shared(false);
        // Try to add a descriptor with is_default_label set.
        member_descriptor->is_default_label(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_default_label(false);
        // Try to add a descriptor with label set.
        member_descriptor->label({1});
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->label({});
        // Try to add a descriptor with try_construct_kind set.
        member_descriptor->try_construct_kind(TryConstructKind::DISCARD);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    EXPECT_EQ(3u, created_type->get_member_count());
    DynamicTypeMember::_ref_type member;
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_index(member, 0));
    EXPECT_EQ(MEMBER_ID_INVALID, member->get_id());
    EXPECT_STREQ("DEFAULT", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_index(member, 1));
    EXPECT_EQ(MEMBER_ID_INVALID, member->get_id());
    EXPECT_STREQ("FIRST", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_index(member, 2));
    EXPECT_EQ(MEMBER_ID_INVALID, member->get_id());
    EXPECT_STREQ("SECOND", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_name(member, "DEFAULT"));
    EXPECT_EQ(MEMBER_ID_INVALID, member->get_id());
    EXPECT_STREQ("DEFAULT", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_name(member, "FIRST"));
    EXPECT_EQ(MEMBER_ID_INVALID, member->get_id());
    EXPECT_STREQ("FIRST", member->get_name());
    ASSERT_EQ(RETCODE_OK, created_type->get_member_by_name(member, "SECOND"));
    EXPECT_EQ(MEMBER_ID_INVALID, member->get_id());
    EXPECT_STREQ("SECOND", member->get_name());

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("DEFAULT"));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test getters and setters.
    const int32_t test1 {2};
    int32_t test2 {0};

    //{{{ Successful setters
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2, test2);

    EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, -3), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-3, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 3u), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(3, test2);

    EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, -1), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(-1, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 1u), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(1, test2);

    EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 2), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(MEMBER_ID_INVALID, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(MEMBER_ID_INVALID, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(MEMBER_ID_INVALID, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(MEMBER_ID_INVALID, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(MEMBER_ID_INVALID, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int64_t iTest64 {0};
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2, iTest64);

    double fTest64 {0};
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2, fTest64);

    long double fTest128 {0};
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2, fTest128);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32 {0};
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16 {0};
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16 {0};
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64 {0};
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32 {0};
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8 {0};
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16 {0};
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest {0};
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest {false};
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        int32_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_string)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRING8);
    type_descriptor->element_type(factory->get_primitive_type(TK_CHAR8));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with element_type set
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_CHAR8));
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_string_type(0)};
        ASSERT_FALSE(builder);
    }

    DynamicTypeBuilder::_ref_type builder {factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type created_type {builder->build()};
    EXPECT_TRUE(created_type);

    const uint32_t length = 15;
    builder = factory->create_string_type(length);
    ASSERT_TRUE(builder);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    created_type = builder->build();
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test getters and setters.
    const std::string test1 {"STRING_TEST"};
    std::string test2;

    //{{{ Successful setters
    EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test2, 0), RETCODE_OK);
    EXPECT_EQ("S", test2);
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_string_value(1, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID,
                "TEST_OVER_LENGTH_LIMITS"), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(MEMBER_ID_INVALID, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(MEMBER_ID_INVALID, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(MEMBER_ID_INVALID, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(MEMBER_ID_INVALID, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(MEMBER_ID_INVALID, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters

    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest32);

    int16_t iTest16;
    EXPECT_EQ(data->get_int16_value(iTest16, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest16);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest128);

    char cTest8;
    EXPECT_EQ(data->get_char8_value(cTest8, 1), RETCODE_OK);
    EXPECT_EQ('T', cTest8);

    wchar_t cTest16;
    EXPECT_EQ(data->get_char16_value(cTest16, 1), RETCODE_OK);
    EXPECT_EQ(L'T', cTest16);

    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_value(uTest32, 1), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_value(iTest8, 1), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_value(uTest8, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_value(uTest16, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint64_value(uTest64, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_value(oTest, 1), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_value(bTest, 1), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_wstring_value(wsTest, 1), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(10u, data->get_member_id_at_index(10));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(11));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(test1.length(), data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        std::string test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ("", test2);
    EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ("", test2);
    EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ("", test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_wstring)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRING16);
    type_descriptor->element_type(factory->get_primitive_type(TK_CHAR16));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with element_type set
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_CHAR16));
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_wstring_type(0)};
        ASSERT_FALSE(builder);
    }

    DynamicTypeBuilder::_ref_type builder {factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))};
    ASSERT_TRUE(builder);
    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    const uint32_t length = 15;
    builder = factory->create_wstring_type(length);
    ASSERT_TRUE(builder);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    created_type = builder->build();
    ASSERT_TRUE(created_type);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    const std::wstring test1 = L"STRING_TEST";
    std::wstring test2;

    //{{{ Successful setters
    EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_wstring_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(L"S", test2);
    EXPECT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_wstring_value(1, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L"TEST_OVER_LENGTH_LIMITS"), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(MEMBER_ID_INVALID, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(MEMBER_ID_INVALID, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(MEMBER_ID_INVALID, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(MEMBER_ID_INVALID, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(MEMBER_ID_INVALID, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}


    //{{{ Successful getters
    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest32);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest128);

    wchar_t cTest16;
    EXPECT_EQ(data->get_char16_value(cTest16, 1), RETCODE_OK);
    EXPECT_EQ(L'T', cTest16);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_value(uTest32, 1), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_value(iTest8, 1), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_value(uTest8, 1), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_value(iTest16, 1), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_value(uTest16, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint64_value(uTest64, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_value(cTest8, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_value(oTest, 1), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_value(bTest, 1), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_string_value(sTest, 1), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(10u, data->get_member_id_at_index(10));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(11));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(test1.length(), data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        std::wstring test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_wstring_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(L"", test2);
    EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(L"", test2);
    EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_wstring_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(L"", test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_alias)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const ObjectName name = "ALIAS";

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with name being nil.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
    }
    type_descriptor->name(name);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with base_type being nil.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
    }
    type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with bound being not nil.
        type_descriptor->bound({1});
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({});
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with element_type set
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT32));
        member_descriptor->name("Wrong");
        ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    EXPECT_EQ(created_type->get_name(), name);
    EXPECT_EQ(created_type->get_kind(), TK_ALIAS);
    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test getters and setters.
    const uint32_t test1 = 2;
    uint32_t test2 = 0;

    //{{{ Successful setters
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 232), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(232u, test2);

    EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 303), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(303u, test2);

    EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 2), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(2u, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_uint32_value(test2, 0), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(MEMBER_ID_INVALID, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(MEMBER_ID_INVALID, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(MEMBER_ID_INVALID, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(MEMBER_ID_INVALID, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(MEMBER_ID_INVALID, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}


    //{{{ Successful getters
    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(iTest64, 2);

    uint64_t uTest64;
    EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(uTest64, 2);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest64, 2);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(fTest128, 2);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    }
    //}}}

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test clone.
        auto clone = data->clone();
        ASSERT_TRUE(clone);
        EXPECT_TRUE(data->equals(clone));

        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(1u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        uint32_t test3 {0};
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint32_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);
    EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_uint32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(0u, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
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
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name(name);
    type_descriptor->base_type(string_type);
    builder = factory->create_type(type_descriptor);
    DynamicType::_ref_type alias_type {builder->build()};
    ASSERT_TRUE(alias_type);
    EXPECT_EQ(alias_type->get_name(), name);
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name(nested_name);
    type_descriptor->base_type(alias_type);
    builder = factory->create_type(type_descriptor);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT32));
        member_descriptor->name("Wrong");
        ASSERT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }
    DynamicType::_ref_type nested_alias_type {builder->build()};
    ASSERT_TRUE(nested_alias_type);
    EXPECT_EQ(nested_alias_type->get_name(), nested_name);
    EXPECT_EQ(nested_alias_type->get_kind(), TK_ALIAS);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
    ASSERT_TRUE(data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));

    // Test getters and setters.
    const std::string test1 {"STRING_TEST"};
    std::string test2;

    //{{{ Successful setters
    EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(data->get_string_value(test2, 0), RETCODE_OK);
    EXPECT_EQ("S", test2);
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_string_value(1, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID,
                "TEST_OVER_LENGTH_LIMITS"), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, L'a'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, false), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(MEMBER_ID_INVALID, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(MEMBER_ID_INVALID, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(MEMBER_ID_INVALID, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(MEMBER_ID_INVALID, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(MEMBER_ID_INVALID, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(MEMBER_ID_INVALID, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(MEMBER_ID_INVALID, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters

    int32_t iTest32;
    EXPECT_EQ(data->get_int32_value(iTest32, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest32);

    int16_t iTest16;
    EXPECT_EQ(data->get_int16_value(iTest16, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest16);

    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, 1), RETCODE_OK);
    EXPECT_EQ(84, iTest64);

    float fTest32;
    EXPECT_EQ(data->get_float32_value(fTest32, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest32);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest64);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, 1), RETCODE_OK);
    EXPECT_EQ(84, fTest128);

    char cTest8;
    EXPECT_EQ(data->get_char8_value(cTest8, 1), RETCODE_OK);
    EXPECT_EQ('T', cTest8);

    wchar_t cTest16;
    EXPECT_EQ(data->get_char16_value(cTest16, 1), RETCODE_OK);
    EXPECT_EQ(L'T', cTest16);

    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_value(uTest32, 1), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_value(iTest8, 1), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_value(uTest8, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_value(uTest16, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint64_value(uTest64, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_value(oTest, 1), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_value(bTest, 1), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_wstring_value(wsTest, 1), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
    }
    //}}}

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test clone.
        auto clone = data->clone();
        ASSERT_TRUE(clone);
        EXPECT_TRUE(data->equals(clone));

        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(test1.length(), data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        std::string test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(nested_alias_type)};
        encoding_decoding_test(nested_alias_type, data, data2, encoding);
        EXPECT_EQ(data2->get_string_value(test3, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(test1, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Test clear functions.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ("", test2);
    EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ("", test2);
    EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, test1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(MEMBER_ID_INVALID));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_string_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ("", test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_bitset)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITSET);
    type_descriptor->name("MyBitset");
    type_descriptor->bound({2, 20});

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with base_type being not nil and not a bitset (or alias).
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with element_type set
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicType::_ref_type created_type {builder->build()};
        EXPECT_FALSE(created_type);
    }

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->id(0);
    member_descriptor->name("int2");
    member_descriptor->type(factory->get_primitive_type(TK_UINT8));
    EXPECT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        EXPECT_FALSE(builder->build());

        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->id(1);
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        // Try to add without name.
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->name("int20");
        // Try to add a descriptor with diferent kind.
        member_descriptor->type(factory->get_primitive_type(TK_UINT32));
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        // Try to add a descriptor with is_key set.
        member_descriptor->is_key(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_key(false);
        // Try to add a descriptor with is_must_understand set.
        member_descriptor->is_must_understand(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_must_understand(false);
        // Try to add a descriptor with is_optional set.
        member_descriptor->is_optional(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_optional(false);
        // Try to add a descriptor with is_shared set.
        member_descriptor->is_shared(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_shared(false);
        // Try to add a descriptor with is_default_label set.
        member_descriptor->is_default_label(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_default_label(false);
        // Try to add a descriptor with label set.
        member_descriptor->label({1});
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->label({});
        // Try to add a descriptor with try_construct_kind set.
        member_descriptor->try_construct_kind(TryConstructKind::DISCARD);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->name("int20");
    member_descriptor->type(factory->get_primitive_type(TK_INT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test a bitfield starting in a position occupied by previous bitfield.
        member_descriptor->id(1);
        EXPECT_EQ(RETCODE_BAD_PARAMETER, builder->add_member(member_descriptor));

        // Test a bitfield exceeding the current maximum limit of 64bits.
        member_descriptor->id(60);
        EXPECT_EQ(RETCODE_BAD_PARAMETER, builder->add_member(member_descriptor));
    }

    member_descriptor->id(3);
    EXPECT_EQ(RETCODE_OK, builder->add_member(member_descriptor));

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test adding a bitfield didn't describe in TypeDescriptor's bound.
        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->id(25);
        member_descriptor->name("int3");
        member_descriptor->type(factory->get_primitive_type(TK_UINT8));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, builder->add_member(member_descriptor));
    }

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};
    ASSERT_TRUE(data);

    // Testing get_member_by_name and get_member_id_at_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("int3"));
    EXPECT_EQ(0u, data->get_member_id_by_name("int2"));
    EXPECT_EQ(3u, data->get_member_id_by_name("int20"));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(3u, data->get_member_id_at_index(1));

    const uint8_t set_test_field_1 {6};
    const int32_t set_test_field_2 {6};

    //{{{ Successful setters
    EXPECT_EQ(data->set_uint8_value(0, set_test_field_1), RETCODE_OK);
    EXPECT_EQ(data->set_int32_value(3, set_test_field_2), RETCODE_OK);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, set_test_field_1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(1, set_test_field_1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, set_test_field_2), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_value(5, set_test_field_2), RETCODE_BAD_PARAMETER);
    }
    //}}}

    uint8_t get_test_field_1 {0};
    int32_t get_test_field_2 {0};

    //{{{ Successful getters
    EXPECT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    EXPECT_EQ(set_test_field_1 & 0x3, get_test_field_1);
    EXPECT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    EXPECT_EQ(set_test_field_2, get_test_field_2);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_uint8_value(get_test_field_1, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_value(get_test_field_1, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int32_value(get_test_field_2, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int32_value(get_test_field_2, 5), RETCODE_BAD_PARAMETER);
    }
    //}}}

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test clone.
        auto clone = data->clone();
        ASSERT_TRUE(clone);
        EXPECT_TRUE(data->equals(clone));

        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 2)); // Non-existing ID.
        EXPECT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
        EXPECT_EQ(complex_data->get_uint8_value(get_test_field_1, MEMBER_ID_INVALID), RETCODE_OK);

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(2, complex_data)); // Non-existing ID.
        EXPECT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

        // Testing loan_value.
        EXPECT_FALSE(data->loan_value(0));
    }

    // Testing get_item_count.
    EXPECT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
        EXPECT_EQ(set_test_field_1 & 0x3, get_test_field_1);
        EXPECT_EQ(data2->get_int32_value(get_test_field_2, 3), RETCODE_OK);
        EXPECT_EQ(set_test_field_2, get_test_field_2);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    EXPECT_EQ(0u, get_test_field_1);
    EXPECT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    EXPECT_EQ(0, get_test_field_2);

    EXPECT_EQ(data->set_uint8_value(0, set_test_field_1), RETCODE_OK);
    EXPECT_EQ(data->set_int32_value(3, set_test_field_2), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    EXPECT_EQ(0u, get_test_field_1);
    EXPECT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    EXPECT_EQ(0, get_test_field_2);

    EXPECT_EQ(data->set_uint8_value(0, set_test_field_1), RETCODE_OK);
    EXPECT_EQ(data->set_int32_value(3, set_test_field_2), RETCODE_OK);
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(2));
    EXPECT_EQ(RETCODE_OK, data->clear_value(3));
    EXPECT_EQ(data->get_uint8_value(get_test_field_1, 0), RETCODE_OK);
    EXPECT_EQ(set_test_field_1 & 0x3, get_test_field_1);
    EXPECT_EQ(data->get_int32_value(get_test_field_2, 3), RETCODE_OK);
    EXPECT_EQ(0, get_test_field_2);
}

TEST_F(DynamicTypesTests, DynamicType_bitmask)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITMASK);
    type_descriptor->element_type(factory->get_primitive_type(TK_BOOLEAN));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test with invalid element_type.
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_BOOLEAN));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    const uint32_t limit = 6;

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_bitmask_type(0)};
        EXPECT_FALSE(builder);
        builder = factory->create_bitmask_type(65);
        EXPECT_FALSE(builder);
    }
    DynamicTypeBuilder::_ref_type builder {factory->create_bitmask_type(limit)};
    ASSERT_TRUE(builder);

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    member_descriptor->name("BIT0");
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->name("BIT1");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->name("BIT2");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->name("BIT3");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->name("BIT5");
    member_descriptor->id(5);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
        member_descriptor->name("BIT6");
        // Test that not setting the id it will try with next id (6) which is invalid due to bound.
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        // Test setting the id 6 which is invalid due to bound.
        member_descriptor->id(6);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
        // Test setting with already existing name.
        member_descriptor->name("BIT0");
        member_descriptor->id(4);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Testing get_member_by_name and get_member_id_at_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("BIT4"));
    EXPECT_EQ(0u, data->get_member_id_by_name("BIT0"));
    EXPECT_EQ(1u, data->get_member_id_by_name("BIT1"));
    EXPECT_EQ(2u, data->get_member_id_by_name("BIT2"));
    EXPECT_EQ(3u, data->get_member_id_by_name("BIT3"));
    EXPECT_EQ(5u, data->get_member_id_by_name("BIT5"));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(2u, data->get_member_id_at_index(2));
    EXPECT_EQ(3u, data->get_member_id_at_index(3));
    EXPECT_EQ(5u, data->get_member_id_at_index(4));

    // Testing get_item_count.
    EXPECT_EQ(0u, data->get_item_count());

    // Testing getters and setters.
    const uint8_t bitmask_value_set = 0x1;
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, bitmask_value_set), RETCODE_OK);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_boolean_value(4, true), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(6, true), RETCODE_BAD_PARAMETER);
    }

    EXPECT_EQ(data->set_boolean_value(2, true), RETCODE_OK);
    EXPECT_EQ(data->set_boolean_value(5, true), RETCODE_OK);

    bool bit_get {false};
    EXPECT_EQ(data->get_boolean_value(bit_get, 0), RETCODE_OK);
    EXPECT_EQ(true, bit_get);
    EXPECT_EQ(data->get_boolean_value(bit_get, 1), RETCODE_OK);
    EXPECT_EQ(false, bit_get);
    EXPECT_EQ(data->get_boolean_value(bit_get, 2), RETCODE_OK);
    EXPECT_EQ(true, bit_get);
    EXPECT_EQ(data->get_boolean_value(bit_get, 3), RETCODE_OK);
    EXPECT_EQ(false, bit_get);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->get_boolean_value(bit_get, 4), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(data->get_boolean_value(bit_get, 5), RETCODE_OK);
    EXPECT_EQ(true, bit_get);

    uint8_t bitmask_value_get {0};
    EXPECT_EQ(data->get_uint8_value(bitmask_value_get, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(bitmask_value_get, 0x25);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Testing loan_value.
        EXPECT_FALSE(data->loan_value(0));
    }

    // Testing get_item_count.
    EXPECT_EQ(3u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data->get_uint8_value(bitmask_value_get, MEMBER_ID_INVALID), RETCODE_OK);
        EXPECT_EQ(bitmask_value_get, 0x25);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(data->get_uint8_value(bitmask_value_get, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(bitmask_value_get, 0);
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0x25), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(data->get_uint8_value(bitmask_value_get, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(bitmask_value_get, 0);
    EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0x25), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_OK, data->clear_value(2));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(data->get_uint8_value(bitmask_value_get, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(bitmask_value_get, 0x21);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_int32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_INT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_INT32), 0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_INT32),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Int32Seq get_test_value;

    const Int32Seq iTestSeq32 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_int32_values(0, iTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq32, get_test_value);

    Int8Seq iTestSeq8 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, ::testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, ::testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int16Seq iTestSeq16 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int16_values(0, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16 {{L'b', L'c', L'd', L'e'}};
    EXPECT_EQ(data->set_char16_values(0, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    // Try to writer to MEMBER_ID_INVALID
    EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, iTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq32, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_int32_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_int32_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_int32_value(4, 5), RETCODE_OK);
    int32_t test1 {0};
    EXPECT_EQ(data->get_int32_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_int32_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_int32_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_int32_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_int32_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Int32Seq test_all {{1, 2, 3, 4, 5}};
    Int32Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_int32_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int32Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int32_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int32_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int32Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_uint32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT32));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_UINT32),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_UINT32),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    UInt32Seq get_test_value;

    const UInt32Seq uTestSeq32 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_uint32_values(0, uTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(uTestSeq32, get_test_value);

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_uint32_values(MEMBER_ID_INVALID, uTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_uint32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(uTestSeq32, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_uint32_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_uint32_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    UInt64Seq uTestSeq64;
    EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq64, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_uint32_value(4, 5), RETCODE_OK);
    uint32_t test1 {0};
    EXPECT_EQ(data->get_uint32_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1u, test1);
    EXPECT_EQ(data->get_uint32_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2u, test1);
    EXPECT_EQ(data->get_uint32_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3u, test1);
    EXPECT_EQ(data->get_uint32_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4u, test1);
    EXPECT_EQ(data->get_uint32_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5u, test1);

    UInt32Seq test_all {{1, 2, 3, 4, 5}};
    UInt32Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_uint32_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        UInt32Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint32_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint32_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint32_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_uint32_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, UInt32Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_int8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_INT8));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_INT8));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_INT8), 0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_INT8),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT8));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Int8Seq get_test_value;

    const Int8Seq iTestSeq8 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq8, get_test_value);

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(bTestSeq, testing::ElementsAreArray(get_test_value.begin(), get_test_value.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(oTestSeq, testing::ElementsAreArray(get_test_value.begin(), get_test_value.end()));

    EXPECT_EQ(data->set_int8_values(MEMBER_ID_INVALID, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq8, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_int8_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_int8_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'a'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'a'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int16Seq iTestSeq16;
    EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq16, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_int8_value(4, 5), RETCODE_OK);
    int8_t test1 {0};
    EXPECT_EQ(data->get_int8_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_int8_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_int8_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_int8_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_int8_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Int8Seq test_all {{1, 2, 3, 4, 5}};
    Int8Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_int8_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int8Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int8_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int8_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int8_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int8_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int8Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_uint8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_UINT8));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT8));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_UINT8),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_UINT8),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT8));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    UInt8Seq get_test_value;

    UInt8Seq uTestSeq8 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq8, get_test_value);

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(bTestSeq, testing::ElementsAreArray(get_test_value.begin(), get_test_value.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(oTestSeq, testing::ElementsAreArray(get_test_value.begin(), get_test_value.end()));

    EXPECT_EQ(data->set_uint8_values(MEMBER_ID_INVALID, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_uint8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq8, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_uint8_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_uint8_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int16Seq iTestSeq16;
    EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq16, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    UInt16Seq uTestSeq16;
    EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq16, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    UInt32Seq uTestSeq32;
    EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq32, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    UInt64Seq uTestSeq64;
    EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq64, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_uint8_value(4, 5), RETCODE_OK);
    uint8_t test1 {0};
    EXPECT_EQ(data->get_uint8_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_uint8_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_uint8_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_uint8_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_uint8_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    UInt8Seq test_all {{1, 2, 3, 4, 5}};
    UInt8Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_uint8_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        UInt8Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint8_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint8_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint8_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_uint8_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, UInt8Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_int16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_INT16));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_INT16));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_INT16), 0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_INT16),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT16));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Int16Seq get_test_value;

    const Int16Seq iTestSeq16 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_int16_values(0, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq16, get_test_value);

    Int8Seq iTestSeq8 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_int16_values(MEMBER_ID_INVALID, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq16, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_int16_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_int16_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'a'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_int16_value(4, 5), RETCODE_OK);
    int16_t test1 {0};
    EXPECT_EQ(data->get_int16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_int16_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_int16_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_int16_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_int16_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Int16Seq test_all {{1, 2, 3, 4, 5}};
    Int16Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_int16_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int16Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int16_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int16_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int16_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int16_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int16Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_uint16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_UINT16));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT16));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_UINT16),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_UINT16),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT16));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    UInt16Seq get_test_value;

    const UInt16Seq uTestSeq16 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(uTestSeq16, get_test_value);

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_uint16_values(MEMBER_ID_INVALID, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_uint16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(uTestSeq16, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_uint16_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_uint16_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    UInt32Seq uTestSeq32;
    EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq32, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    UInt64Seq uTestSeq64;
    EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq64, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_uint16_value(4, 5), RETCODE_OK);
    uint16_t test1 {0};
    EXPECT_EQ(data->get_uint16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_uint16_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_uint16_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_uint16_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_uint16_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    UInt16Seq test_all {{1, 2, 3, 4, 5}};
    UInt16Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_uint16_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        UInt16Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint16_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint16_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint16_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_uint16_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, UInt16Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_int64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_INT64));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_INT64));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_INT64), 0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_INT64),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT64));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Int64Seq get_test_value;

    const Int64Seq iTestSeq64 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_int64_values(0, iTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq64, get_test_value);

    Int32Seq iTestSeq32 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int32_values(0, iTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    UInt32Seq uTestSeq32 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint32_values(0, uTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    Int8Seq iTestSeq8 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int16Seq iTestSeq16 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int16_values(0, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16 {{L'b', L'c', L'd', L'e'}};
    EXPECT_EQ(data->set_char16_values(0, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_int64_values(MEMBER_ID_INVALID, iTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_int64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq64, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_int64_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_int64_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Float128Seq fTestSeq128;
    Float128Seq fTestSeq128_result {{1.0, 2.0, 3.0, 4.0}};
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, fTestSeq128_result);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_int64_value(4, 5), RETCODE_OK);
    int64_t test1 {0};
    EXPECT_EQ(data->get_int64_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_int64_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_int64_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_int64_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_int64_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Int64Seq test_all {{1, 2, 3, 4, 5}};
    Int64Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_int64_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int64Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int64_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int64_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_int64_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int64_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int64Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_uint64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_UINT64));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_UINT64));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_UINT64),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_UINT64),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_UINT64));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    UInt64Seq get_test_value;

    const UInt64Seq uTestSeq64 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_uint64_values(0, uTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(uTestSeq64, get_test_value);

    UInt32Seq uTestSeq32 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_uint32_values(0, uTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_uint64_values(MEMBER_ID_INVALID, uTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_uint64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(uTestSeq64, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_uint64_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_uint64_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Float128Seq fTestSeq128;
    Float128Seq fTestSeq128_result {{1.0, 2.0, 3.0, 4.0}};
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, fTestSeq128_result);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_uint64_value(4, 5), RETCODE_OK);
    uint64_t test1 {0};
    EXPECT_EQ(data->get_uint64_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_uint64_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_uint64_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_uint64_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_uint64_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    UInt64Seq test_all {{1, 2, 3, 4, 5}};
    UInt64Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_uint64_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        UInt64Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_uint64_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint64_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_uint64_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_uint64_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, UInt64Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_float32)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_FLOAT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_FLOAT32));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_FLOAT32),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_FLOAT32),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_FLOAT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Float32Seq get_test_value;

    const Float32Seq fTestSeq32 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_float32_values(0, fTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(fTestSeq32, get_test_value);

    Int8Seq iTestSeq8 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int16Seq iTestSeq16 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int16_values(0, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16 {{L'b', L'c', L'd', L'e'}};
    EXPECT_EQ(data->set_char16_values(0, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_float32_values(MEMBER_ID_INVALID, fTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(fTestSeq32, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_float32_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_float32_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(fTestSeq32.begin(), fTestSeq32.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(fTestSeq32.begin(), fTestSeq32.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_float32_value(4, 5), RETCODE_OK);
    float test1 {0};
    EXPECT_EQ(data->get_float32_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_float32_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_float32_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_float32_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_float32_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Float32Seq test_all {{1, 2, 3, 4, 5}};
    Float32Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_float32_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Float32Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_float32_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_float32_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_float32_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_float32_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Float32Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_float64)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_FLOAT64));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_FLOAT64));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_FLOAT64),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_FLOAT64),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_FLOAT64));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Float64Seq get_test_value;

    const Float64Seq fTestSeq64 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_float64_values(0, fTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(fTestSeq64, get_test_value);

    Int32Seq iTestSeq32 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int32_values(0, iTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    UInt32Seq uTestSeq32 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint32_values(0, uTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    Int8Seq iTestSeq8 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int16Seq iTestSeq16 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int16_values(0, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    Float32Seq fTestSeq32 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_float32_values(0, fTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(fTestSeq32.begin(), fTestSeq32.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16 {{L'b', L'c', L'd', L'e'}};
    EXPECT_EQ(data->set_char16_values(0, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_float64_values(MEMBER_ID_INVALID, fTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_float64_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(fTestSeq64, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_float64_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_float64_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(fTestSeq64.begin(), fTestSeq64.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_float64_value(4, 5), RETCODE_OK);
    double test1 {0};
    EXPECT_EQ(data->get_float64_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_float64_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_float64_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_float64_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_float64_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Float64Seq test_all {{1, 2, 3, 4, 5}};
    Float64Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_float64_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Float64Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_float64_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_float64_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_float64_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_float64_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Float64Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_float128)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_FLOAT128));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_FLOAT128));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_FLOAT128),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_FLOAT128),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_FLOAT128));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    Float128Seq get_test_value;

    const Float128Seq fTestSeq128 {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_float128_values(0, fTestSeq128), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(fTestSeq128, get_test_value);

    Int32Seq iTestSeq32 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int32_values(0, iTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    UInt32Seq uTestSeq32 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint32_values(0, uTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq32.begin(), uTestSeq32.end()));

    Int8Seq iTestSeq8 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int8_values(0, iTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint8_values(0, uTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int16Seq iTestSeq16 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_int16_values(0, iTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6}};
    EXPECT_EQ(data->set_uint16_values(0, uTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    Int64Seq iTestSeq64 {{2, 3, 4, 5}};
    Float128Seq iTestSeq64_result {{2.0, 3.0, 4.0, 5.0}};
    EXPECT_EQ(data->set_int64_values(0, iTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, iTestSeq64_result);

    UInt64Seq uTestSeq64 {{3, 4, 5, 6}};
    Float128Seq uTestSeq64_result {{3.0, 4.0, 5.0, 6.0} };
    EXPECT_EQ(data->set_uint64_values(0, uTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, uTestSeq64_result);

    Float32Seq fTestSeq32 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_float32_values(0, fTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(fTestSeq32.begin(), fTestSeq32.end()));

    Float64Seq fTestSeq64 {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_float64_values(0, fTestSeq64), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(fTestSeq64.begin(), fTestSeq64.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16 {{L'b', L'c', L'd', L'e'}};
    EXPECT_EQ(data->set_char16_values(0, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_float128_values(MEMBER_ID_INVALID, fTestSeq128), RETCODE_OK);
    EXPECT_EQ(data->get_float128_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(fTestSeq128, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_float128_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_float128_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_float128_value(4, 5), RETCODE_OK);
    long double test1 {0};
    EXPECT_EQ(data->get_float128_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_float128_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_float128_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_float128_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_float128_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    Float128Seq test_all {{1, 2, 3, 4, 5}};
    Float128Seq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_float128_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Float128Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_float128_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_float128_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_float128_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_float128_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Float128Seq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_char8)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_CHAR8));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_CHAR8));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_CHAR8), 0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_CHAR8),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_CHAR8));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    CharSeq get_test_value;

    const CharSeq cTestSeq8 {{'a', 'b', 'c', 'd'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_char8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(cTestSeq8, get_test_value);

    ByteSeq oTestSeq {{98, 99, 100, 101}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_char8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(oTestSeq, testing::ElementsAreArray(get_test_value.begin(), get_test_value.end()));

    EXPECT_EQ(data->set_char8_values(MEMBER_ID_INVALID, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_char8_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(cTestSeq8, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_char8_values(0, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_char8_value(234, 'z'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(10, {'z'}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'a'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int16Seq iTestSeq16;
    EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq16, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16;
    EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(cTestSeq16, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_char8_value(4, 'e'), RETCODE_OK);
    char test1 {0};
    EXPECT_EQ(data->get_char8_value(test1, 0), RETCODE_OK);
    EXPECT_EQ('a', test1);
    EXPECT_EQ(data->get_char8_value(test1, 1), RETCODE_OK);
    EXPECT_EQ('b', test1);
    EXPECT_EQ(data->get_char8_value(test1, 2), RETCODE_OK);
    EXPECT_EQ('c', test1);
    EXPECT_EQ(data->get_char8_value(test1, 3), RETCODE_OK);
    EXPECT_EQ('d', test1);
    EXPECT_EQ(data->get_char8_value(test1, 4), RETCODE_OK);
    EXPECT_EQ('e', test1);

    CharSeq test_all {{'a', 'b', 'c', 'd', 'e'}};
    CharSeq test_less {{'c', 'd', 'e'}};
    EXPECT_EQ(data->get_char8_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        CharSeq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_char8_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_char8_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_char8_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_char8_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, CharSeq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_char16)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_CHAR16));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_CHAR16));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_CHAR16),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_CHAR16),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_CHAR16));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    WcharSeq get_test_value;

    const WcharSeq cTestSeq16 {{L'a', L'b', L'c', L'd'}};
    EXPECT_EQ(data->set_char16_values(0, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(cTestSeq16, get_test_value);

    CharSeq cTestSeq8 {{'b', 'c', 'd', 'e'}};
    EXPECT_EQ(data->set_char8_values(0, cTestSeq8), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    ByteSeq oTestSeq {{98, 99, 100, 101}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_char16_values(MEMBER_ID_INVALID, cTestSeq16), RETCODE_OK);
    EXPECT_EQ(data->get_char16_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(cTestSeq16, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_char16_values(0, {L'a', L'b', L'c', L'd', L'e', L'f', L'g'}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_char16_value(234, L'z'), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(10, {L'z'}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_char16_value(4, 'e'), RETCODE_OK);
    wchar_t test1 {0};
    EXPECT_EQ(data->get_char16_value(test1, 0), RETCODE_OK);
    EXPECT_EQ('a', test1);
    EXPECT_EQ(data->get_char16_value(test1, 1), RETCODE_OK);
    EXPECT_EQ('b', test1);
    EXPECT_EQ(data->get_char16_value(test1, 2), RETCODE_OK);
    EXPECT_EQ('c', test1);
    EXPECT_EQ(data->get_char16_value(test1, 3), RETCODE_OK);
    EXPECT_EQ('d', test1);
    EXPECT_EQ(data->get_char16_value(test1, 4), RETCODE_OK);
    EXPECT_EQ('e', test1);

    WcharSeq test_all {{L'a', L'b', L'c', L'd', L'e'}};
    WcharSeq test_less {{L'c', L'd', L'e'}};
    EXPECT_EQ(data->get_char16_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        WcharSeq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_char16_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_char16_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_char16_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_char16_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, WcharSeq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_byte)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_BYTE));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_BYTE));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_BYTE),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_BYTE),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_BYTE));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    ByteSeq get_test_value;

    ByteSeq oTestSeq {{1, 2, 3, 4}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_byte_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(oTestSeq, get_test_value);

    EXPECT_EQ(data->set_byte_values(MEMBER_ID_INVALID, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_byte_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(oTestSeq, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_byte_values(0, {0, 1, 2, 3, 4, 5, 6}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_byte_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int8Seq iTestSeq8;
    EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq8, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    UInt8Seq uTestSeq8;
    EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq8, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    Int16Seq iTestSeq16;
    EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq16, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    UInt16Seq uTestSeq16;
    EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq16, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    UInt32Seq uTestSeq32;
    EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq32, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    UInt64Seq uTestSeq64;
    EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq64, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    CharSeq cTestSeq8;
    EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_OK);
    EXPECT_THAT(cTestSeq8, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    WcharSeq cTestSeq16;
    EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(cTestSeq16, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    BooleanSeq bTestSeq;
    EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_OK);
    EXPECT_THAT(bTestSeq, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_byte_value(4, 5), RETCODE_OK);
    octet test1 {0};
    EXPECT_EQ(data->get_byte_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(1, test1);
    EXPECT_EQ(data->get_byte_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(2, test1);
    EXPECT_EQ(data->get_byte_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(3, test1);
    EXPECT_EQ(data->get_byte_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(4, test1);
    EXPECT_EQ(data->get_byte_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(5, test1);

    ByteSeq test_all {{1, 2, 3, 4, 5}};
    ByteSeq test_less {{3, 4, 5}};
    EXPECT_EQ(data->get_byte_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        ByteSeq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_byte_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_byte_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_byte_values(0, {1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_byte_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, ByteSeq({1, 3}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_boolean)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_SEQUENCE);
    type_descriptor->element_type(factory->get_primitive_type(TK_BOOLEAN));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({10});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_BOOLEAN));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(factory->get_primitive_type(TK_BOOLEAN),
                                                       0)};
        EXPECT_FALSE(builder);
    }

    const uint32_t length = 5;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type( factory->get_primitive_type(TK_BOOLEAN),
                                                   length)};
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters and setters.


    //{{{ Successful setters
    BooleanSeq get_test_value;

    BooleanSeq bTestSeq {{true, false, false, true}};
    EXPECT_EQ(data->set_boolean_values(0, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(bTestSeq, get_test_value);

    ByteSeq oTestSeq {{0, 1, 1, 0}};
    EXPECT_EQ(data->set_byte_values(0, oTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(oTestSeq, testing::ElementsAreArray(get_test_value.begin(), get_test_value.end()));

    EXPECT_EQ(data->set_boolean_values(MEMBER_ID_INVALID, bTestSeq), RETCODE_OK);
    EXPECT_EQ(data->get_boolean_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(bTestSeq, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_boolean_values(0, {true, false, false, true, false, false, true}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_boolean_value(234, true), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(10, {true}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    Int8Seq iTestSeq8;
    EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq8, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    UInt8Seq uTestSeq8;
    EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq8, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    Int16Seq iTestSeq16;
    EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq16, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    UInt16Seq uTestSeq16;
    EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq16, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    Int32Seq iTestSeq32;
    EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq32, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    UInt32Seq uTestSeq32;
    EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq32, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    UInt64Seq uTestSeq64;
    EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(uTestSeq64, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    Float32Seq fTestSeq32;
    EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq32, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    EXPECT_EQ(data->set_boolean_value(4, true), RETCODE_OK);
    bool test1 {false};
    EXPECT_EQ(data->get_boolean_value(test1, 0), RETCODE_OK);
    EXPECT_EQ(true, test1);
    EXPECT_EQ(data->get_boolean_value(test1, 1), RETCODE_OK);
    EXPECT_EQ(false, test1);
    EXPECT_EQ(data->get_boolean_value(test1, 2), RETCODE_OK);
    EXPECT_EQ(false, test1);
    EXPECT_EQ(data->get_boolean_value(test1, 3), RETCODE_OK);
    EXPECT_EQ(true, test1);
    EXPECT_EQ(data->get_boolean_value(test1, 4), RETCODE_OK);
    EXPECT_EQ(true, test1);

    BooleanSeq test_all {{true, false, false, true, true}};
    BooleanSeq test_less {{false, true, true}};
    EXPECT_EQ(data->get_boolean_values(get_test_value, 2), RETCODE_OK);
    EXPECT_EQ(get_test_value, test_less);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(4u, data->get_member_id_at_index(4));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(5));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(length, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        BooleanSeq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_boolean_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(test_all, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_boolean_values(0, {true, false, true}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->set_boolean_values(0, {true, false, true}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_boolean_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, BooleanSeq({true, true}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_sequence_of_sequences)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t sequence_length = 2;
    const uint32_t inner_sequence_length = 3;

    DynamicTypeBuilder::_ref_type builder {factory->create_sequence_type(
                                               factory->get_primitive_type(
                                                   TK_INT32), inner_sequence_length)};
    ASSERT_TRUE(builder);
    builder = factory->create_sequence_type(builder->build(), sequence_length);
    ASSERT_TRUE(builder);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test getters, setters and loan_value.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {1, 2}), RETCODE_BAD_PARAMETER);
    }

    EXPECT_EQ(data->set_int32_values(0, {1, 2}), RETCODE_OK);
    Int32Seq good_seq;
    EXPECT_EQ(data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2}));

    auto seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->set_uint32_values(0, {1, 2, 3}), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->set_uint32_value(1, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Try to loan non-existent element
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        seq_data = data->loan_value(2);
        EXPECT_FALSE(seq_data);
    }

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    UInt32Seq wrong_seq;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->get_uint32_values(wrong_seq, 0), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2, 3}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 1}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(2));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    }
    EXPECT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2, 3}));

    // Test set_complex_value
    good_seq = {2, 3, 4};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    }
    EXPECT_EQ(complex_data->set_int32_values(0, good_seq), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

    // Test get_item_count().
    EXPECT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int32Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        seq_data = data2->loan_value(0);
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        EXPECT_EQ(good_seq, Int32Seq({2, 3, 4}));
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(1);
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        EXPECT_EQ(good_seq, Int32Seq({0, 1}));
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2, 3}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    EXPECT_EQ(RETCODE_OK, data->clear_value(0));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    }
    EXPECT_EQ(1u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 1}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_array)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ARRAY);
    type_descriptor->element_type(factory->get_primitive_type(TK_INT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        // Test bounds with a zero value.
        type_descriptor->bound({2, 0, 2});
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->bound({2, 2, 2});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type set
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    BoundSeq array_dimensions {2, 2, 2};

    DynamicTypeBuilder::_ref_type builder {factory->create_array_type(
                                               factory->get_primitive_type(
                                                   TK_INT32), array_dimensions)};
    ASSERT_TRUE(builder);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(7u, data->get_member_id_at_index(7));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(8));

    //{{{ Successful setters
    Int32Seq get_test_value;

    Int32Seq iTestSeq32 {{1, 2, 3, 4, 5, 6, 7, 0}};
    EXPECT_EQ(data->set_int32_values(0, {1, 2, 3, 4}), RETCODE_OK);
    EXPECT_EQ(data->set_int32_value(4, 5), RETCODE_OK);
    EXPECT_EQ(data->set_int32_value(5, 6), RETCODE_OK);
    EXPECT_EQ(data->set_int32_value(6, 7), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq32, get_test_value);

    Int8Seq iTestSeq8 {{2, 3, 4, 5, 6, 7, 8, 0}};
    EXPECT_EQ(data->set_int8_values(0, {2, 3, 4, 5}), RETCODE_OK);
    EXPECT_EQ(data->set_int8_value(4, 6), RETCODE_OK);
    EXPECT_EQ(data->set_int8_value(5, 7), RETCODE_OK);
    EXPECT_EQ(data->set_int8_value(6, 8), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq8.begin(), iTestSeq8.end()));

    UInt8Seq uTestSeq8 {{3, 4, 5, 6, 7, 8, 9, 0}};
    EXPECT_EQ(data->set_uint8_values(0, {3, 4, 5, 6}), RETCODE_OK);
    EXPECT_EQ(data->set_uint8_value(4, 7), RETCODE_OK);
    EXPECT_EQ(data->set_uint8_value(5, 8), RETCODE_OK);
    EXPECT_EQ(data->set_uint8_value(6, 9), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq8.begin(), uTestSeq8.end()));

    Int16Seq iTestSeq16 {{2, 3, 4, 5, 6, 7, 8, 0}};
    EXPECT_EQ(data->set_int16_values(0, {2, 3, 4, 5}), RETCODE_OK);
    EXPECT_EQ(data->set_int16_value(4, 6), RETCODE_OK);
    EXPECT_EQ(data->set_int16_value(5, 7), RETCODE_OK);
    EXPECT_EQ(data->set_int16_value(6, 8), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(iTestSeq16.begin(), iTestSeq16.end()));

    UInt16Seq uTestSeq16 {{3, 4, 5, 6, 7, 8, 9, 0}};
    EXPECT_EQ(data->set_uint16_values(0, {3, 4, 5, 6}), RETCODE_OK);
    EXPECT_EQ(data->set_uint16_value(4, 7), RETCODE_OK);
    EXPECT_EQ(data->set_uint16_value(5, 8), RETCODE_OK);
    EXPECT_EQ(data->set_uint16_value(6, 9), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(uTestSeq16.begin(), uTestSeq16.end()));

    CharSeq cTestSeq8 {{'a', 'b', 'c', 'd', 'e', 'f', 'g', 0}};
    EXPECT_EQ(data->set_char8_values(0, {'a', 'b', 'c', 'd'}), RETCODE_OK);
    EXPECT_EQ(data->set_char8_value(4, 'e'), RETCODE_OK);
    EXPECT_EQ(data->set_char8_value(5, 'f'), RETCODE_OK);
    EXPECT_EQ(data->set_char8_value(6, 'g'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq8.begin(), cTestSeq8.end()));

    WcharSeq cTestSeq16 {{L'b', L'c', L'd', L'e', L'f', L'g', L'h', 0}};
    EXPECT_EQ(data->set_char16_values(0, {L'b', L'c', L'd', L'e'}), RETCODE_OK);
    EXPECT_EQ(data->set_char16_value(4, L'f'), RETCODE_OK);
    EXPECT_EQ(data->set_char16_value(5, L'g'), RETCODE_OK);
    EXPECT_EQ(data->set_char16_value(6, L'h'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(cTestSeq16.begin(), cTestSeq16.end()));

    BooleanSeq bTestSeq {{1, 0, 0, 1, 1, 0, 1, 0}};
    EXPECT_EQ(data->set_boolean_values(0, {true, false, false, true}), RETCODE_OK);
    EXPECT_EQ(data->set_boolean_value(4, true), RETCODE_OK);
    EXPECT_EQ(data->set_boolean_value(5, false), RETCODE_OK);
    EXPECT_EQ(data->set_boolean_value(6, true), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(bTestSeq.begin(), bTestSeq.end()));

    ByteSeq oTestSeq {{2, 3, 4, 5, 6, 7, 8, 0}};
    EXPECT_EQ(data->set_byte_values(0, {2, 3, 4, 5}), RETCODE_OK);
    EXPECT_EQ(data->set_byte_value(4, 6), RETCODE_OK);
    EXPECT_EQ(data->set_byte_value(5, 7), RETCODE_OK);
    EXPECT_EQ(data->set_byte_value(6, 8), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_THAT(get_test_value, testing::ElementsAreArray(oTestSeq.begin(), oTestSeq.end()));

    EXPECT_EQ(data->set_int32_values(MEMBER_ID_INVALID, iTestSeq32), RETCODE_OK);
    EXPECT_EQ(data->get_int32_values(get_test_value, 0), RETCODE_OK);
    EXPECT_EQ(iTestSeq32, get_test_value);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        // Try to insert more than the limit.
        EXPECT_EQ(data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8, 9}), RETCODE_BAD_PARAMETER);
        // Try to write on an empty position
        EXPECT_EQ(data->set_int32_value(234, 1), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(10, {1}), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_value(MEMBER_ID_INVALID, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(MEMBER_ID_INVALID, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);

    }
    //}}}

    //{{{ Successful getters
    int32_t test_value {0};
    EXPECT_EQ(data->get_int32_value(test_value, 0), RETCODE_OK);
    EXPECT_EQ(1, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 1), RETCODE_OK);
    EXPECT_EQ(2, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 2), RETCODE_OK);
    EXPECT_EQ(3, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 3), RETCODE_OK);
    EXPECT_EQ(4, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 4), RETCODE_OK);
    EXPECT_EQ(5, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 5), RETCODE_OK);
    EXPECT_EQ(6, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 6), RETCODE_OK);
    EXPECT_EQ(7, test_value);
    EXPECT_EQ(data->get_int32_value(test_value, 7), RETCODE_OK);
    EXPECT_EQ(0, test_value);

    EXPECT_EQ(data->get_int32_values(get_test_value, 2), RETCODE_OK);
    Int32Seq test_less {{3, 4, 5, 6, 7, 0}};
    EXPECT_EQ(get_test_value, test_less);

    Int64Seq iTestSeq64;
    EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(iTestSeq64, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    Float64Seq fTestSeq64;
    EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq64, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));

    Float128Seq fTestSeq128;
    EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_OK);
    EXPECT_THAT(fTestSeq128, testing::ElementsAreArray(iTestSeq32.begin(), iTestSeq32.end()));
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");

        int32_t iTest32;
        EXPECT_EQ(data->get_int32_value(iTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        int64_t iTest64;
        EXPECT_EQ(data->get_int64_value(iTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        double fTest64;
        EXPECT_EQ(data->get_float64_value(fTest64, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        long double fTest128;
        EXPECT_EQ(data->get_float128_value(fTest128, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, MEMBER_ID_INVALID), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Test loan_value
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(8u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int32Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int32_values(test3, 0), RETCODE_OK);
        EXPECT_EQ(iTestSeq32, test3);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(8u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int32_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int32Seq({0, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8}));
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(8u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int32_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int32Seq({0, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(RETCODE_OK, data->set_int32_values(0, {1, 2, 3, 4, 5, 6, 7, 8}));
    EXPECT_EQ(RETCODE_OK, data->clear_value(1));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    }
    EXPECT_EQ(8u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->get_int32_values(get_test_value, 0));
    EXPECT_EQ(get_test_value, Int32Seq({1, 0, 3, 4, 5, 6, 7, 8}));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_array_of_arrays)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    BoundSeq array_dimensions {2, 2};
    BoundSeq inner_array_dimensions {2};

    DynamicTypeBuilder::_ref_type builder {factory->create_array_type(
                                               factory->get_primitive_type(
                                                   TK_INT32), inner_array_dimensions)};

    ASSERT_TRUE(builder);
    builder = factory->create_array_type(builder->build(), array_dimensions);
    ASSERT_TRUE(builder);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));
    EXPECT_EQ(2u, data->get_member_id_at_index(2));
    EXPECT_EQ(3u, data->get_member_id_at_index(3));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(4));

    // Test getters, setters and loan_value.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {1, 2}), RETCODE_BAD_PARAMETER);
    }

    EXPECT_EQ(data->set_int32_values(0, {1, 2}), RETCODE_OK);
    Int32Seq good_seq;
    EXPECT_EQ(data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2}));

    auto seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->set_uint32_values(0, {1, 2}), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->set_uint32_value(1, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->set_uint32_values(0, {3, 4}), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(seq_data->set_uint32_value(0, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Try to insert more than the limit.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        seq_data = data->loan_value(4);
        ASSERT_FALSE(seq_data);
    }

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        UInt32Seq wrong_seq;
        EXPECT_EQ(seq_data->get_uint32_values(wrong_seq, 0), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 1}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({3, 4}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    }
    EXPECT_EQ(RETCODE_OK, data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2}));

    // Test set_complex_value
    good_seq = {2, 3};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    }
    EXPECT_EQ(complex_data->set_int32_values(0, good_seq), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->set_complex_value(0, complex_data));

    // Test get_item_count().
    EXPECT_EQ(4u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        Int32Seq test3;
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        seq_data = data2->loan_value(0);
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        EXPECT_EQ(good_seq, Int32Seq({2, 3}));
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(1);
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        EXPECT_EQ(good_seq, Int32Seq({0, 1}));
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(2);
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        EXPECT_EQ(good_seq, Int32Seq({3, 4}));
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        seq_data = data2->loan_value(3);
        ASSERT_TRUE(seq_data);
        EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
        EXPECT_EQ(good_seq, Int32Seq({1, 0}));
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(seq_data));
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    EXPECT_EQ(RETCODE_OK, data->clear_value(2));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    }
    EXPECT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 1}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {1, 2}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(1, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_values(0, {3, 4}), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->set_int32_value(0, 1), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    EXPECT_EQ(RETCODE_OK, data->clear_value(3));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    EXPECT_EQ(4u, data->get_item_count());
    seq_data = data->loan_value(0);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({1, 2}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(1);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 1}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(2);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({3, 4}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));
    seq_data = data->loan_value(3);
    ASSERT_TRUE(seq_data);
    EXPECT_EQ(seq_data->get_int32_values(good_seq, 0), RETCODE_OK);
    EXPECT_EQ(good_seq, Int32Seq({0, 0}));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(seq_data));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_map)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_MAP);
    type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
    type_descriptor->key_element_type(factory->get_primitive_type(TK_INT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test without bound.
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        // Test bounds with a zero value.
        type_descriptor->bound({0});
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->bound({2});
        // Test without element_type.
        type_descriptor->element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
        // Test with a base_type
        type_descriptor->base_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test without key_element_type
        type_descriptor->key_element_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        // Test with invalid key type.
        type_descriptor->key_element_type(factory->create_wstring_type(20)->build());
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        // Test with invalid key type.
        type_descriptor->key_element_type(factory->create_array_type(factory->get_primitive_type(TK_UINT32),
                {20})->build());
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
    }

    const uint32_t map_length {2};

    DynamicTypeBuilder::_ref_type builder {factory->create_map_type(
                                               factory->get_primitive_type(TK_INT32),
                                               factory->get_primitive_type(TK_INT32),
                                               map_length)};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        member_descriptor->name("Wrong");
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_PRECONDITION_NOT_MET);
    }

    DynamicType::_ref_type created_type {builder->build()};
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(1));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(2));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("string"));
    EXPECT_EQ(0u, data->get_member_id_by_name("10"));
    EXPECT_EQ(1u, data->get_member_id_by_name("20"));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("30"));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));

    // Testing getters and setters.
    const int32_t test1 {123};
    int32_t test2 {0};

    //{{{ Successful setters
    EXPECT_EQ(data->set_int32_value(0, test1), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    EXPECT_EQ(data->set_int8_value(0, 100), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(100, test2);

    EXPECT_EQ(data->set_uint8_value(0, 232), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(232, test2);

    EXPECT_EQ(data->set_int16_value(0, 101), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(101, test2);

    EXPECT_EQ(data->set_uint16_value(0, 303), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(303, test2);

    EXPECT_EQ(data->set_char8_value(0, 'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_char16_value(0, L'a'), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(97, test2);

    EXPECT_EQ(data->set_boolean_value(0, false), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(0, test2);

    EXPECT_EQ(data->set_byte_value(0, 1), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(1, test2);
    //}}}

    //{{{ Failing setters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_int32_value(MEMBER_ID_INVALID, 10), RETCODE_BAD_PARAMETER);
        // Try to write on an invalid position
        EXPECT_EQ(data->set_int32_value(2, 10), RETCODE_BAD_PARAMETER);

        EXPECT_EQ(data->set_uint32_value(0, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_value(0, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_value(0, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_value(0, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_value(0, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_value(0, 0), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(0, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_value(0, L""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint8_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint16_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint32_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_uint64_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float32_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float64_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_float128_values(0, {0.0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char8_values(0, {'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_char16_values(0, {L'c'}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_byte_values(0, {0}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_boolean_values(0, {true}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_values(0, {"str"}), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_wstring_values(0, {L"wstr"}), RETCODE_BAD_PARAMETER);
    }
    //}}}

    //{{{ Successful getters
    int64_t iTest64;
    EXPECT_EQ(data->get_int64_value(iTest64, 0), RETCODE_OK);
    EXPECT_EQ(iTest64, 1);

    double fTest64;
    EXPECT_EQ(data->get_float64_value(fTest64, 0), RETCODE_OK);
    EXPECT_EQ(fTest64, 1);

    long double fTest128;
    EXPECT_EQ(data->get_float128_value(fTest128, 0), RETCODE_OK);
    EXPECT_EQ(fTest128, 1);
    //}}}

    //{{{ Failing getters
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t uTest32;
        EXPECT_EQ(data->get_uint32_value(uTest32, 0), RETCODE_BAD_PARAMETER);
        int8_t iTest8;
        EXPECT_EQ(data->get_int8_value(iTest8, 0), RETCODE_BAD_PARAMETER);
        uint8_t uTest8;
        EXPECT_EQ(data->get_uint8_value(uTest8, 0), RETCODE_BAD_PARAMETER);
        int16_t iTest16;
        EXPECT_EQ(data->get_int16_value(iTest16, 0), RETCODE_BAD_PARAMETER);
        uint16_t uTest16;
        EXPECT_EQ(data->get_uint16_value(uTest16, 0), RETCODE_BAD_PARAMETER);
        uint64_t uTest64;
        EXPECT_EQ(data->get_uint64_value(uTest64, 0), RETCODE_BAD_PARAMETER);
        float fTest32;
        EXPECT_EQ(data->get_float32_value(fTest32, 0), RETCODE_BAD_PARAMETER);
        char cTest8;
        EXPECT_EQ(data->get_char8_value(cTest8, 0), RETCODE_BAD_PARAMETER);
        wchar_t cTest16;
        EXPECT_EQ(data->get_char16_value(cTest16, 0), RETCODE_BAD_PARAMETER);
        octet oTest;
        EXPECT_EQ(data->get_byte_value(oTest, 0), RETCODE_BAD_PARAMETER);
        bool bTest;
        EXPECT_EQ(data->get_boolean_value(bTest, 0), RETCODE_BAD_PARAMETER);
        std::string sTest;
        EXPECT_EQ(data->get_string_value(sTest, 0), RETCODE_BAD_PARAMETER);
        std::wstring wsTest;
        EXPECT_EQ(data->get_wstring_value(wsTest, 0), RETCODE_BAD_PARAMETER);
        Int8Seq iTestSeq8;
        EXPECT_EQ(data->get_int8_values(iTestSeq8, 0), RETCODE_BAD_PARAMETER);
        UInt8Seq uTestSeq8;
        EXPECT_EQ(data->get_uint8_values(uTestSeq8, 0), RETCODE_BAD_PARAMETER);
        Int16Seq iTestSeq16;
        EXPECT_EQ(data->get_int16_values(iTestSeq16, 0), RETCODE_BAD_PARAMETER);
        UInt16Seq uTestSeq16;
        EXPECT_EQ(data->get_uint16_values(uTestSeq16, 0), RETCODE_BAD_PARAMETER);
        Int32Seq iTestSeq32;
        EXPECT_EQ(data->get_int32_values(iTestSeq32, 0), RETCODE_BAD_PARAMETER);
        UInt32Seq uTestSeq32;
        EXPECT_EQ(data->get_uint32_values(uTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Int64Seq iTestSeq64;
        EXPECT_EQ(data->get_int64_values(iTestSeq64, 0), RETCODE_BAD_PARAMETER);
        UInt64Seq uTestSeq64;
        EXPECT_EQ(data->get_uint64_values(uTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float32Seq fTestSeq32;
        EXPECT_EQ(data->get_float32_values(fTestSeq32, 0), RETCODE_BAD_PARAMETER);
        Float64Seq fTestSeq64;
        EXPECT_EQ(data->get_float64_values(fTestSeq64, 0), RETCODE_BAD_PARAMETER);
        Float128Seq fTestSeq128;
        EXPECT_EQ(data->get_float128_values(fTestSeq128, 0), RETCODE_BAD_PARAMETER);
        CharSeq cTestSeq8;
        EXPECT_EQ(data->get_char8_values(cTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WcharSeq cTestSeq16;
        EXPECT_EQ(data->get_char16_values(cTestSeq16, 0), RETCODE_BAD_PARAMETER);
        ByteSeq oTestSeq;
        EXPECT_EQ(data->get_byte_values(oTestSeq, 0), RETCODE_BAD_PARAMETER);
        BooleanSeq bTestSeq;
        EXPECT_EQ(data->get_boolean_values(bTestSeq, 0), RETCODE_BAD_PARAMETER);
        StringSeq sTestSeq8;
        EXPECT_EQ(data->get_string_values(sTestSeq8, 0), RETCODE_BAD_PARAMETER);
        WstringSeq sTestSeq16;
        EXPECT_EQ(data->get_wstring_values(sTestSeq16, 0), RETCODE_BAD_PARAMETER);
    }
    //}}}


    // Set and get a value.
    EXPECT_EQ(data->set_int32_value(0, test1), RETCODE_OK);
    int32_t test3 {132};
    EXPECT_EQ(data->set_int32_value(1, test3), RETCODE_OK);
    EXPECT_EQ(data->get_int32_value(test2, 1), RETCODE_OK);
    EXPECT_EQ(test3, test2);

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test get_complex_value
        DynamicData::_ref_type complex_data;
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, 0));

        // Test set_complex_value
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(0, complex_data));

        // Testing loan_value.
        EXPECT_FALSE(data->loan_value(1));
        EXPECT_FALSE(data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count().
    EXPECT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(data2->get_int32_value(test2, data->get_member_id_by_name(
                    "10")), RETCODE_OK);
        EXPECT_EQ(test1, test2);
        EXPECT_EQ(data2->get_int32_value(test2, data->get_member_id_by_name(
                    "20")), RETCODE_OK);
        EXPECT_EQ(test3, test2);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_int32_value(test2, 0));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_int32_value(test2, 1));
    }

    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("10"), test1));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("20"), test3));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_int32_value(test2, 0));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_int32_value(test2, 1));
    }

    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("10"), test1));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(data->get_member_id_by_name("20"), test3));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->clear_value(0));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    }
    EXPECT_EQ(1u, data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_int32_value(test2, 0));
    }
    EXPECT_EQ(RETCODE_OK, data->get_int32_value(test2, 1));
    EXPECT_EQ(test3, test2);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_map_of_maps)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const uint32_t map_length {2};
    const uint32_t inner_map_length {3};

    DynamicTypeBuilder::_ref_type builder {factory->create_map_type(
                                               factory->create_string_type(100)->build(),
                                               factory->create_string_type(100)->build(),
                                               inner_map_length)};

    ASSERT_TRUE(builder);
    DynamicType::_ref_type inner_type = builder->build();
    ASSERT_TRUE(inner_type);

    builder = factory->create_map_type(factory->get_primitive_type(TK_INT16),
                    inner_type, map_length);

    DynamicType::_ref_type created_type = builder->build();
    ASSERT_TRUE(created_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(created_type)};

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(0));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(1));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_at_index(2));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name(""));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("string"));
    EXPECT_EQ(0u, data->get_member_id_by_name("-2"));
    EXPECT_EQ(1u, data->get_member_id_by_name("14"));
    EXPECT_EQ(MEMBER_ID_INVALID, data->get_member_id_by_name("30"));
    EXPECT_EQ(0u, data->get_member_id_at_index(0));
    EXPECT_EQ(1u, data->get_member_id_at_index(1));

    // Testing getters and setters.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_int16_value(MEMBER_ID_INVALID, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(data->set_string_value(0, ""), RETCODE_BAD_PARAMETER);
    }

    // Testing getters, setters, loan_value.
    DynamicData::_ref_type loan_data {data->loan_value(data->get_member_id_by_name("-2"))};
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(0u, loan_data->get_item_count());
    std::string test1 {"str1"};
    std::string test2 {"str2"};
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key3"), test1));
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key4"), test2));
    EXPECT_EQ(2u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(data->get_member_id_by_name("14"));
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(0u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key1"), test2));
    EXPECT_EQ(1u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));

    std::string test_get {0};
    loan_data = data->loan_value(data->get_member_id_by_name("-2"));
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(2u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key3")));
    EXPECT_EQ(test1, test_get);
    EXPECT_EQ(RETCODE_OK,
            loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key4")));
    EXPECT_EQ(test2, test_get);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(data->get_member_id_by_name("14"));
    ASSERT_TRUE(loan_data);
    ASSERT_EQ(1u, loan_data->get_item_count());
    ASSERT_EQ(RETCODE_OK,
            loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key1")));
    ASSERT_EQ(test2, test_get);
    ASSERT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));

    // Test clone.
    auto clone = data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    }
    EXPECT_EQ(RETCODE_OK, data->get_complex_value(complex_data, data->get_member_id_by_name("-2")));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(2u, complex_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            complex_data->get_string_value(test_get, complex_data->get_member_id_by_name("key3")));
    EXPECT_EQ(test1, test_get);
    EXPECT_EQ(RETCODE_OK,
            complex_data->get_string_value(test_get, complex_data->get_member_id_by_name("key4")));
    EXPECT_EQ(test2, test_get);


    // Test set_complex_value
    std::string test3 {"456"};
    std::string test4 {"345"};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    }
    EXPECT_EQ(RETCODE_OK,
            complex_data->set_string_value(complex_data->get_member_id_by_name("key3"), test3));
    EXPECT_EQ(RETCODE_OK,
            complex_data->set_string_value(complex_data->get_member_id_by_name("key4"), test4));
    EXPECT_EQ(RETCODE_OK, data->set_complex_value(data->get_member_id_by_name("-2"), complex_data));

    // Test get_item_count().
    EXPECT_EQ(2u, data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(created_type)};
        encoding_decoding_test(created_type, data, data2, encoding);
        EXPECT_EQ(2u, data2->get_item_count());
        loan_data = data2->loan_value(data2->get_member_id_by_name("-2"));
        ASSERT_TRUE(loan_data);
        EXPECT_EQ(2u, loan_data->get_item_count());
        EXPECT_EQ(RETCODE_OK,
                loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key3")));
        EXPECT_EQ(test3, test_get);
        EXPECT_EQ(RETCODE_OK,
                loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key4")));
        EXPECT_EQ(test4, test_get);
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(loan_data));
        loan_data = data2->loan_value(data2->get_member_id_by_name("14"));
        ASSERT_TRUE(loan_data);
        EXPECT_EQ(1u, loan_data->get_item_count());
        EXPECT_EQ(RETCODE_OK,
                loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key1")));
        EXPECT_EQ(test2, test_get);
        EXPECT_EQ(RETCODE_OK, data2->return_loaned_value(loan_data));
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, data->clear_all_values());
    EXPECT_EQ(0u, data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(data->loan_value(0));
        EXPECT_FALSE(data->loan_value(1));
    }

    loan_data = data->loan_value(data->get_member_id_by_name("-2"));
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(0u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key3"), test1));
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key4"), test2));
    EXPECT_EQ(2u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(data->get_member_id_by_name("14"));
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(0u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key1"), test2));
    EXPECT_EQ(1u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->clear_nonkey_values());
    EXPECT_EQ(0u, data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(data->loan_value(0));
        EXPECT_FALSE(data->loan_value(1));
    }

    loan_data = data->loan_value(data->get_member_id_by_name("-2"));
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(0u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key3"), test1));
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key4"), test2));
    EXPECT_EQ(2u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    loan_data = data->loan_value(data->get_member_id_by_name("14"));
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(0u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->set_string_value(loan_data->get_member_id_by_name("key1"), test2));
    EXPECT_EQ(1u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));
    EXPECT_EQ(2u, data->get_item_count());
    EXPECT_EQ(RETCODE_OK, data->clear_value(0));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, data->clear_value(100));
    }
    EXPECT_EQ(1u, data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(data->loan_value(0));
    }
    loan_data = data->loan_value(1);
    ASSERT_TRUE(loan_data);
    EXPECT_EQ(1u, loan_data->get_item_count());
    EXPECT_EQ(RETCODE_OK,
            loan_data->get_string_value(test_get, loan_data->get_member_id_by_name("key1")));
    EXPECT_EQ(test2, test_get);
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(loan_data));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_structure)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("StructTest");

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with bound.
        type_descriptor->bound({10});
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({});
        // Test with invalid base_type.
        type_descriptor->base_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with element_type.
        type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(nullptr);
        // Test with discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(nullptr);
        // Test with key_element_type
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
    }

    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("int32");
    member_descriptor->id(0);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->default_value("3");
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        member_descriptor->is_default_label(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_default_label(false);
        member_descriptor->label({0, 1});
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->label({});
    }
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type struct_data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(struct_data);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(struct_data->set_int64_value(0, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(struct_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
    }


    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_by_name(""));
    EXPECT_EQ(0u, struct_data->get_member_id_by_name("int32"));
    EXPECT_EQ(1u, struct_data->get_member_id_by_name("int64"));
    EXPECT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_at_index(2));
    EXPECT_EQ(0u, struct_data->get_member_id_at_index(0));
    EXPECT_EQ(1u, struct_data->get_member_id_at_index(1));

    // Test getters and setters.
    int32_t test1 {234};
    EXPECT_EQ(struct_data->set_int32_value(0, test1), RETCODE_OK);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        uint32_t wrong {0};
        EXPECT_EQ(struct_data->get_uint32_value(wrong, 0), RETCODE_BAD_PARAMETER);
    }
    int32_t test2 {0};
    EXPECT_EQ(struct_data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(test1, test2);
    int64_t test3 {234};
    EXPECT_EQ(struct_data->set_int64_value(1, test3), RETCODE_OK);
    int64_t test4 {0};
    EXPECT_EQ(struct_data->get_int64_value(test4, 1), RETCODE_OK);
    EXPECT_EQ(test3, test4);

    // Test clone.
    auto clone = struct_data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(struct_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    }
    EXPECT_EQ(RETCODE_OK, struct_data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    // Test set_complex_value
    test1 = 456;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        ASSERT_EQ(RETCODE_BAD_PARAMETER, struct_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    }
    ASSERT_EQ(RETCODE_OK,
            complex_data->set_int32_value(MEMBER_ID_INVALID, test1));
    ASSERT_EQ(RETCODE_OK, struct_data->set_complex_value(0, complex_data));

    // Test loan_value
    DynamicData::_ref_type loan_data = struct_data->loan_value(struct_data->get_member_id_by_name("int64"));
    ASSERT_TRUE(loan_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(struct_data->loan_value(struct_data->get_member_id_by_name("int64")));
    }
    EXPECT_EQ(loan_data->get_int64_value(test4, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test3, test4);
    EXPECT_EQ(struct_data->return_loaned_value(loan_data), RETCODE_OK);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(struct_data->loan_value(MEMBER_ID_INVALID));
    }

    // Test get_item_count.
    EXPECT_EQ(2u, struct_data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(struct_type)};
        encoding_decoding_test(struct_type, struct_data, data2, encoding);
        EXPECT_EQ(data2->get_int32_value(test2, 0), RETCODE_OK);
        EXPECT_EQ(test1, test2);
        EXPECT_EQ(data2->get_int64_value(test4, 1), RETCODE_OK);
        EXPECT_EQ(test3, test4);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, struct_data->clear_all_values());
    EXPECT_EQ(2u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    EXPECT_EQ(3, test4);

    EXPECT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    EXPECT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    EXPECT_EQ(RETCODE_OK, struct_data->clear_nonkey_values());
    EXPECT_EQ(2u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    EXPECT_EQ(3, test4);

    EXPECT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    EXPECT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    EXPECT_EQ(RETCODE_OK, struct_data->clear_value(1));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(100));
    EXPECT_EQ(2u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    EXPECT_EQ(test1, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    EXPECT_EQ(3, test4);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(struct_data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_structure_inheritance)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create the base struct.
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("BaseStructTest");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("int32");
    member_descriptor->id(0);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->id(1);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type base_struct_type {builder->build()};
    ASSERT_TRUE(base_struct_type);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Check cannot set extensibility if base type didn't do it.
        type_descriptor = traits<TypeDescriptor>::make_shared();
        type_descriptor->kind(TK_STRUCTURE);
        type_descriptor->name("DerivedStructTest");
        type_descriptor->base_type(base_struct_type);
        type_descriptor->extensibility_kind(ExtensibilityKind::FINAL);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
    }

    // Create the derived struct.
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("DerivedStructTest");
    type_descriptor->base_type(base_struct_type);
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");     // avoid expected errors logging
        member_descriptor->name("child_int32");
        member_descriptor->id(1);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->name("int32");
        member_descriptor->id(2);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        // Derived struct cannot have key members.
        member_descriptor->name("child_int32");
        member_descriptor->is_key(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_key(false);
    }

    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("child_string");
    member_descriptor->id(4);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type derived_struct_type {builder->build()};
    ASSERT_TRUE(derived_struct_type);

    EXPECT_EQ(4u, derived_struct_type->get_member_count());

    DynamicTypeMember::_ref_type member;
    DynamicTypeMember::_ref_type member_aux;
    MemberDescriptor::_ref_type descriptor = traits<MemberDescriptor>::make_shared();

    EXPECT_EQ(derived_struct_type->get_member(member, 0), RETCODE_OK);
    EXPECT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    EXPECT_EQ(descriptor->name(), "int32");
    EXPECT_EQ(descriptor->index(), 0u);
    EXPECT_EQ(descriptor->id(), 0u);
    EXPECT_TRUE(descriptor->type()->equals(factory->get_primitive_type(TK_INT32)));
    EXPECT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));
    EXPECT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));

    EXPECT_EQ(derived_struct_type->get_member(member, 1), RETCODE_OK);
    EXPECT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    EXPECT_EQ(descriptor->name(), "int64");
    EXPECT_EQ(descriptor->index(), 1u);
    EXPECT_EQ(descriptor->id(), 1u);
    EXPECT_TRUE(descriptor->type()->equals(factory->get_primitive_type(TK_INT64)));
    EXPECT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));
    EXPECT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));

    EXPECT_EQ(derived_struct_type->get_member(member, 2), RETCODE_OK);
    EXPECT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    EXPECT_EQ(descriptor->name(), "child_int32");
    EXPECT_EQ(descriptor->index(), 2u);
    EXPECT_EQ(descriptor->id(), 2u);
    EXPECT_TRUE(descriptor->type()->equals(factory->get_primitive_type(TK_INT32)));
    EXPECT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));
    EXPECT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));

    EXPECT_EQ(derived_struct_type->get_member(member, 4), RETCODE_OK);
    EXPECT_EQ(member->get_descriptor(descriptor), RETCODE_OK);
    EXPECT_EQ(descriptor->name(), "child_string");
    EXPECT_EQ(descriptor->index(), 3u);
    EXPECT_EQ(descriptor->id(), 4u);
    EXPECT_TRUE(descriptor->type()->equals(factory->create_string_type(
                static_cast<uint32_t>(LENGTH_UNLIMITED))->build()));
    EXPECT_EQ(derived_struct_type->get_member_by_name(member_aux,
            descriptor->name()), RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));
    EXPECT_EQ(derived_struct_type->get_member_by_index(member_aux, descriptor->index()),
            RETCODE_OK);
    EXPECT_TRUE(member->equals(member_aux));


    DynamicTypeMembersById members_by_id;
    EXPECT_EQ(derived_struct_type->get_all_members(members_by_id), RETCODE_OK);
    EXPECT_EQ(members_by_id.size(), 4u);

    DynamicTypeMembersByName members_by_name;
    EXPECT_EQ(derived_struct_type->get_all_members_by_name(members_by_name), RETCODE_OK);
    EXPECT_EQ(members_by_name.size(), 4u);


    // Validating data management
    DynamicData::_ref_type struct_data {DynamicDataFactory::get_instance()->create_data(derived_struct_type)};
    ASSERT_TRUE(derived_struct_type);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_by_name(""));
    EXPECT_EQ(0u, struct_data->get_member_id_by_name("int32"));
    EXPECT_EQ(1u, struct_data->get_member_id_by_name("int64"));
    EXPECT_EQ(2u, struct_data->get_member_id_by_name("child_int32"));
    EXPECT_EQ(4u, struct_data->get_member_id_by_name("child_string"));
    EXPECT_EQ(0u, struct_data->get_member_id_at_index(0));
    EXPECT_EQ(1u, struct_data->get_member_id_at_index(1));
    EXPECT_EQ(2u, struct_data->get_member_id_at_index(2));
    EXPECT_EQ(4u, struct_data->get_member_id_at_index(3));
    EXPECT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_at_index(4));

    // Testing getters and setters.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Setting invalid types should fail
        EXPECT_EQ(struct_data->set_int64_value(0, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(struct_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
    }

    // Set and get the parent values.
    int32_t test1 {234};
    EXPECT_EQ(struct_data->set_int32_value(0, test1), RETCODE_OK);
    int32_t test2 {0};
    EXPECT_EQ(struct_data->get_int32_value(test2, 0), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    int64_t test3 {234};
    EXPECT_EQ(struct_data->set_int64_value(1, test3), RETCODE_OK);
    int64_t test4 {0};
    EXPECT_EQ(struct_data->get_int64_value(test4, 1), RETCODE_OK);
    EXPECT_EQ(test3, test4);

    // Set and get the child value.
    EXPECT_EQ(struct_data->set_int32_value(2, test1), RETCODE_OK);
    EXPECT_EQ(struct_data->get_int32_value(test2, 2), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    std::string test5 {"Testing"};
    EXPECT_EQ(struct_data->set_string_value(4, test5), RETCODE_OK);
    std::string test6;
    EXPECT_EQ(struct_data->get_string_value(test6, 4), RETCODE_OK);
    EXPECT_EQ(test5, test6);

    // Test clone.
    auto clone = struct_data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(struct_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    }
    EXPECT_EQ(RETCODE_OK, struct_data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_int32_value(test2, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    // Test set_complex_value
    int32_t test11 {456};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    }
    EXPECT_EQ(RETCODE_OK,
            complex_data->set_int32_value(MEMBER_ID_INVALID, test11));
    EXPECT_EQ(RETCODE_OK, struct_data->set_complex_value(0, complex_data));

    // Test loan_value
    DynamicData::_ref_type loan_data = struct_data->loan_value(1);
    ASSERT_TRUE(loan_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(struct_data->loan_value(1));
    }
    EXPECT_EQ(loan_data->get_int64_value(test4, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(test3, test4);
    EXPECT_EQ(struct_data->return_loaned_value(loan_data), RETCODE_OK);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(struct_data->loan_value(MEMBER_ID_INVALID));
    }

    // Testing get_item_count
    EXPECT_EQ(4u, struct_data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(derived_struct_type)};
        encoding_decoding_test(derived_struct_type, struct_data, data2, encoding);
        EXPECT_EQ(data2->get_int32_value(test2, 0), RETCODE_OK);
        EXPECT_EQ(test11, test2);
        EXPECT_EQ(data2->get_int64_value(test4, 1), RETCODE_OK);
        EXPECT_EQ(test3, test4);
        EXPECT_EQ(data2->get_int32_value(test2, 2), RETCODE_OK);
        EXPECT_EQ(test1, test2);
        EXPECT_EQ(data2->get_string_value(test6, 4), RETCODE_OK);
        EXPECT_EQ(test5, test6);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, struct_data->clear_all_values());
    EXPECT_EQ(4u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    EXPECT_EQ(0, test4);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 2));
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_string_value(test6, 4));
    EXPECT_EQ("", test6);

    EXPECT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    EXPECT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    EXPECT_EQ(RETCODE_OK, struct_data->set_int32_value(2, test1));
    EXPECT_EQ(RETCODE_OK, struct_data->set_string_value(4, test5));
    EXPECT_EQ(RETCODE_OK, struct_data->clear_nonkey_values());
    EXPECT_EQ(4u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    EXPECT_EQ(0, test4);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 2));
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_string_value(test6, 4));
    EXPECT_EQ("", test6);

    EXPECT_EQ(RETCODE_OK, struct_data->set_int32_value(0, test1));
    EXPECT_EQ(RETCODE_OK, struct_data->set_int64_value(1, test3));
    EXPECT_EQ(RETCODE_OK, struct_data->set_int32_value(2, test1));
    EXPECT_EQ(RETCODE_OK, struct_data->set_string_value(4, test5));
    EXPECT_EQ(RETCODE_OK, struct_data->clear_value(1));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(3));
    }
    EXPECT_EQ(RETCODE_OK, struct_data->clear_value(4));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(100));
    }
    EXPECT_EQ(4u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 0));
    EXPECT_EQ(test1, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test4, 1));
    EXPECT_EQ(0, test4);
    EXPECT_EQ(RETCODE_OK, struct_data->get_int32_value(test2, 2));
    EXPECT_EQ(test1, test2);
    EXPECT_EQ(RETCODE_OK, struct_data->get_string_value(test6, 4));
    EXPECT_EQ("", test6);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(struct_data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_multi_structure)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("InnerStructTest");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("int32");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("int64");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type inner_struct_type {builder->build()};
    ASSERT_TRUE(inner_struct_type);

    // Create the parent struct.
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("StructTest");
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    // Add members to the struct.
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(inner_struct_type);
    member_descriptor->name("Structure");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("int64");
    member_descriptor->id(10);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type struct_data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(struct_data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_by_name(""));
    EXPECT_EQ(0u, struct_data->get_member_id_by_name("Structure"));
    EXPECT_EQ(10u, struct_data->get_member_id_by_name("int64"));
    EXPECT_EQ(0u, struct_data->get_member_id_at_index(0));
    EXPECT_EQ(10u, struct_data->get_member_id_at_index(1));
    EXPECT_EQ(MEMBER_ID_INVALID, struct_data->get_member_id_at_index(2));

    // Testing getter, setters and loan_value.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(struct_data->set_int32_value(1, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(struct_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
    }

    // Set and get the child values.
    const int64_t test1 {234};
    EXPECT_EQ(struct_data->set_int64_value(10, test1), RETCODE_OK);
    int64_t test2 {0};
    EXPECT_EQ(struct_data->get_int64_value(test2, 10), RETCODE_OK);
    EXPECT_EQ(test1, test2);

    auto inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);

    // Set and get from child structure.
    int32_t test3 {234};
    EXPECT_EQ(inner_struct_data->set_int32_value(0, test3), RETCODE_OK);
    int32_t test4 {0};
    EXPECT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    EXPECT_EQ(test3, test4);
    int64_t test5 {234};
    EXPECT_EQ(inner_struct_data->set_int64_value(1, test5), RETCODE_OK);
    int64_t test6 {0};
    EXPECT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    EXPECT_EQ(test5, test6);

    EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    // Test clone.
    auto clone = struct_data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(struct_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
    }
    EXPECT_EQ(RETCODE_OK, struct_data->get_complex_value(complex_data, 0));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_int32_value(test4, 0), RETCODE_OK);
    EXPECT_EQ(test3, test4);
    EXPECT_EQ(complex_data->get_int64_value(test6, 1), RETCODE_OK);
    EXPECT_EQ(test5, test6);

    // Test set_complex_value
    test3 = 456;
    test5 = 567;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
    }
    EXPECT_EQ(complex_data->set_int32_value(0, test3), RETCODE_OK);
    EXPECT_EQ(complex_data->set_int64_value(1, test5), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, struct_data->set_complex_value(0, complex_data));

    // Testing get_item_count.
    EXPECT_EQ(2u, struct_data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(struct_type)};
        encoding_decoding_test(struct_type, struct_data, data2, encoding);
        EXPECT_EQ(data2->get_int64_value(test2, 10), RETCODE_OK);
        EXPECT_EQ(test1, test2);
        inner_struct_data = struct_data->loan_value(0);
        ASSERT_TRUE(inner_struct_data);
        EXPECT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
        EXPECT_EQ(test3, test4);
        EXPECT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
        EXPECT_EQ(test5, test6);
        EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, struct_data->clear_all_values());
    EXPECT_EQ(2u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test2, 10));
    EXPECT_EQ(0, test2);
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    EXPECT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    EXPECT_EQ(0, test4);
    EXPECT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    EXPECT_EQ(0, test6);
    EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    EXPECT_EQ(RETCODE_OK, struct_data->set_int64_value(10, test1));
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    EXPECT_EQ(inner_struct_data->set_int32_value(0, test3), RETCODE_OK);
    EXPECT_EQ(inner_struct_data->set_int64_value(1, test5), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));
    EXPECT_EQ(RETCODE_OK, struct_data->clear_nonkey_values());
    EXPECT_EQ(2u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test2, 10));
    EXPECT_EQ(0, test2);
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    EXPECT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    EXPECT_EQ(0, test4);
    EXPECT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    EXPECT_EQ(0, test6);
    EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    EXPECT_EQ(RETCODE_OK, struct_data->set_int64_value(10, test1));
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    EXPECT_EQ(inner_struct_data->set_int32_value(0, test3), RETCODE_OK);
    EXPECT_EQ(inner_struct_data->set_int64_value(1, test5), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));
    EXPECT_EQ(RETCODE_OK, struct_data->clear_value(0));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, struct_data->clear_value(100));
    EXPECT_EQ(2u, struct_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, struct_data->get_int64_value(test2, 10));
    EXPECT_EQ(test1, test2);
    inner_struct_data = struct_data->loan_value(0);
    ASSERT_TRUE(inner_struct_data);
    EXPECT_EQ(inner_struct_data->get_int32_value(test4, 0), RETCODE_OK);
    EXPECT_EQ(0, test4);
    EXPECT_EQ(inner_struct_data->get_int64_value(test6, 1), RETCODE_OK);
    EXPECT_EQ(0, test6);
    EXPECT_EQ(RETCODE_OK, struct_data->return_loaned_value(inner_struct_data));

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(struct_data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_union)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create the base struct.
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_UNION);
    type_descriptor->name("UnionTest");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        // Test with bound.
        type_descriptor->bound({10});
        DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
        EXPECT_FALSE(builder);
        type_descriptor->bound({});
        // Test with invalid base_type.
        type_descriptor->base_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->base_type(nullptr);
        // Test with element_type.
        type_descriptor->element_type(factory->get_primitive_type(TK_INT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->element_type(nullptr);
        // Test with key_element_type
        type_descriptor->key_element_type(factory->get_primitive_type(TK_UINT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->key_element_type(nullptr);
        // Test without discriminator_type set
        type_descriptor->discriminator_type(nullptr);
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        // Test with invalid discriminator_type set
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_FLOAT32));
        builder = factory->create_type(type_descriptor);
        EXPECT_FALSE(builder);
        type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    }
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);


    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("first");
    member_descriptor->id(1);
    member_descriptor->label({0, 1});
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("second");
    member_descriptor->id(3);
    member_descriptor->label({4});
    member_descriptor->default_value("my string");
    member_descriptor->is_default_label(true);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");     // avoid expected errors logging

        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->get_primitive_type(TK_INT32));
        // Testing setting a member with repeated name.
        member_descriptor->name("second");
        member_descriptor->id(4);
        member_descriptor->label({5});
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);

        // Testing setting a member with the discriminator's id.
        member_descriptor->name("third");
        member_descriptor->id(0);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);

        // Testing setting a member with repeated id.
        member_descriptor->id(3);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);

        // Testing setting a member without label and is_default_label.
        member_descriptor->id(5);
        member_descriptor->label({});
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);

        // Testing setting a member with a repeated label.
        member_descriptor->label({4, 5});
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);

        // Testing setting a second member with is_default_labael.
        member_descriptor->label({5, 6});
        member_descriptor->is_default_label(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
        member_descriptor->is_default_label(false);

        // Testing setting a member as key.
        member_descriptor->label({10});
        member_descriptor->is_key(true);
        EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_BAD_PARAMETER);
    }

    // Create a data of this union
    DynamicType::_ref_type union_type {builder->build()};
    ASSERT_TRUE(union_type);

    DynamicData::_ref_type union_data {DynamicDataFactory::get_instance()->create_data(union_type)};
    ASSERT_TRUE(union_data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_by_name(""));
    EXPECT_EQ(0u, union_data->get_member_id_by_name("discriminator"));
    EXPECT_EQ(1u, union_data->get_member_id_by_name("first"));
    EXPECT_EQ(3u, union_data->get_member_id_by_name("second"));
    EXPECT_EQ(0u, union_data->get_member_id_at_index(0));
    EXPECT_EQ(1u, union_data->get_member_id_at_index(1));
    EXPECT_EQ(3u, union_data->get_member_id_at_index(2));
    EXPECT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_at_index(3));

    // Testing getters and setters.
    int32_t discriminator_value {0};
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);

    int64_t int64_get {0};
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_BAD_PARAMETER);
    }
    std::string string_get;
    EXPECT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    EXPECT_EQ("my string", string_get);

    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->set_int32_value(0, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->set_int32_value(0, 4), RETCODE_OK);

    const int64_t int64_set {234};

    EXPECT_EQ(union_data->set_int64_value(1, int64_set), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 == discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_string_value(string_get, 3), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    EXPECT_EQ(int64_set, int64_get);

    // Discriminator is only allowed to change to any value that selects the same member.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->set_int32_value(0, 4), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->set_int32_value(0, 1), RETCODE_OK);

    std::string string_set {"testing_value"};
    EXPECT_EQ(union_data->set_string_value(3, string_set), RETCODE_OK);
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    EXPECT_EQ(string_set, string_get);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->set_int32_value(0, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->set_int32_value(0, 4), RETCODE_OK);

    // Test clone.
    auto clone = union_data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(union_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, 1));
    }
    EXPECT_EQ(RETCODE_OK, union_data->get_complex_value(complex_data, 3));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_string_value(string_get, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(string_set, string_get);

    // Test set_complex_value
    string_set = "retesting";
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(0, complex_data));
    }
    EXPECT_EQ(complex_data->set_string_value(MEMBER_ID_INVALID, string_set), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, union_data->set_complex_value(3, complex_data));

    // Testing loan_value.
    DynamicData::_ref_type loan_data = union_data->loan_value(1);
    ASSERT_TRUE(loan_data);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(union_data->loan_value(1));
    }
    EXPECT_EQ(loan_data->get_int64_value(int64_get, MEMBER_ID_INVALID), RETCODE_OK);
    EXPECT_EQ(int64_set, int64_get);
    EXPECT_EQ(union_data->return_loaned_value(loan_data), RETCODE_OK);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_FALSE(union_data->loan_value(MEMBER_ID_INVALID));
    }
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_EQ(0, discriminator_value);

    // Testing loan_value.

    // Testing get_item_count.
    EXPECT_EQ(2u, union_data->get_item_count());

    // Encoding/decoding
    EXPECT_EQ(union_data->set_string_value(3, string_set), RETCODE_OK);
    EXPECT_EQ(union_data->set_int32_value(0, 4), RETCODE_OK);
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(union_type)};
        encoding_decoding_test(union_type, union_data, data2, encoding);
        EXPECT_EQ(data2->get_int32_value(discriminator_value, 0), RETCODE_OK);
        EXPECT_EQ(4, discriminator_value);
        {
            eprosima::fastdds::testing::ScopeLogs _("disable");
            EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_BAD_PARAMETER);
        }
        EXPECT_EQ(data2->get_string_value(string_get, 3), RETCODE_OK);
        EXPECT_EQ(string_set, string_get);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, union_data->clear_all_values());
    EXPECT_EQ(2u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    EXPECT_EQ("my string", string_get);

    EXPECT_EQ(union_data->set_int64_value(1, int64_set), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, union_data->clear_nonkey_values());
    EXPECT_EQ(2u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    EXPECT_EQ("my string", string_get);

    EXPECT_EQ(union_data->set_int64_value(1, int64_set), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, union_data->clear_value(1));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->clear_value(3));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->clear_value(100));
    }
    EXPECT_EQ(2u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_OK);
    EXPECT_EQ(0, int64_get);
    EXPECT_EQ(RETCODE_OK, union_data->clear_value(0));
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(int64_get, 1), RETCODE_BAD_PARAMETER);
    }
    EXPECT_EQ(union_data->get_string_value(string_get, 3), RETCODE_OK);
    EXPECT_EQ("my string", string_get);

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(union_data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_union_with_unions)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Create the base struct.
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_UNION);
    type_descriptor->name("InnerUnionTest");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);


    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("first");
    member_descriptor->id(1);
    member_descriptor->label({0, 1});
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("second");
    member_descriptor->label({4});
    member_descriptor->default_value("my string");
    member_descriptor->is_default_label(true);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type inner_union_type {builder->build()};
    ASSERT_TRUE(inner_union_type);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_UNION);
    type_descriptor->name("UnionTest");
    type_descriptor->discriminator_type(factory->get_primitive_type(TK_INT32));
    builder = factory->create_type(type_descriptor);
    ASSERT_TRUE(builder);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("first");
    member_descriptor->id(1);
    member_descriptor->label({1});
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(inner_union_type);
    member_descriptor->name("second");
    member_descriptor->id(2);
    member_descriptor->label({4});
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type union_type {builder->build()};
    ASSERT_TRUE(union_type);

    DynamicData::_ref_type union_data {DynamicDataFactory::get_instance()->create_data(union_type)};
    ASSERT_TRUE(union_data);

    // Test get_member_by_name and get_member_by_index.
    EXPECT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_by_name(""));
    EXPECT_EQ(0u, union_data->get_member_id_by_name("discriminator"));
    EXPECT_EQ(1u, union_data->get_member_id_by_name("first"));
    EXPECT_EQ(2u, union_data->get_member_id_by_name("second"));
    EXPECT_EQ(0u, union_data->get_member_id_at_index(0));
    EXPECT_EQ(1u, union_data->get_member_id_at_index(1));
    EXPECT_EQ(2u, union_data->get_member_id_at_index(2));
    EXPECT_EQ(MEMBER_ID_INVALID, union_data->get_member_id_at_index(3));

    // Testing getters, setters and loan_value.
    // Set and get the child values.
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->set_int32_value(2, 10), RETCODE_BAD_PARAMETER);
        EXPECT_EQ(union_data->set_string_value(MEMBER_ID_INVALID, ""), RETCODE_BAD_PARAMETER);
    }

    int32_t discriminator_value {0};
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(1 != discriminator_value && 4 != discriminator_value);

    const int64_t test1 {234};
    EXPECT_EQ(union_data->set_int64_value(1, test1), RETCODE_OK);
    int64_t test2 {0};
    EXPECT_EQ(union_data->get_int64_value(test2, 1), RETCODE_OK);
    EXPECT_EQ(test1, test2);
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_EQ(1, discriminator_value);

    DynamicData::_ref_type child_data {union_data->loan_value(2)};
    ASSERT_TRUE(child_data);

    std::string test3;

    EXPECT_EQ(child_data->get_string_value(test3, 2), RETCODE_OK);
    EXPECT_EQ("my string", test3);

    EXPECT_EQ(union_data->return_loaned_value(child_data), RETCODE_OK);

    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_EQ(4, discriminator_value);

    // Test clone.
    auto clone = union_data->clone();
    ASSERT_TRUE(clone);
    EXPECT_TRUE(union_data->equals(clone));

    // Test get_complex_value
    DynamicData::_ref_type complex_data;
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, MEMBER_ID_INVALID));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->get_complex_value(complex_data, 1));
    }
    EXPECT_EQ(RETCODE_OK, union_data->get_complex_value(complex_data, 2));
    ASSERT_TRUE(complex_data);
    EXPECT_EQ(complex_data->get_string_value(test3, 2), RETCODE_OK);
    EXPECT_EQ("my string", test3);

    // Test set_complex_value
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(MEMBER_ID_INVALID, complex_data));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->set_complex_value(0, complex_data));
    }
    EXPECT_EQ(complex_data->set_string_value(2, "another"), RETCODE_OK);
    EXPECT_EQ(RETCODE_OK, union_data->set_complex_value(2, complex_data));

    // Testing get_item_count.
    EXPECT_EQ(2u, union_data->get_item_count());

    // Encoding/decoding
    for (auto encoding : encodings)
    {
        DynamicData::_ref_type data2 {DynamicDataFactory::get_instance()->create_data(union_type)};
        encoding_decoding_test(union_type, union_data, data2, encoding);
        EXPECT_EQ(data2->get_int32_value(discriminator_value, 0), RETCODE_OK);
        EXPECT_EQ(4, discriminator_value);
        child_data = union_data->loan_value(2);
        ASSERT_TRUE(child_data);
        EXPECT_EQ(child_data->get_string_value(test3, 2), RETCODE_OK);
        EXPECT_EQ("another", test3);
        EXPECT_EQ(union_data->return_loaned_value(child_data), RETCODE_OK);
        EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(data2), RETCODE_OK);
    }

    // Remove the elements.
    EXPECT_EQ(RETCODE_OK, union_data->clear_all_values());
    EXPECT_EQ(1u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(test2, 1), RETCODE_BAD_PARAMETER);
    }

    EXPECT_EQ(union_data->set_int64_value(1, test1), RETCODE_OK);
    EXPECT_EQ(2u, union_data->get_item_count());
    EXPECT_EQ(RETCODE_OK, union_data->clear_nonkey_values());
    EXPECT_EQ(1u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(test2, 1), RETCODE_BAD_PARAMETER);
    }

    EXPECT_EQ(union_data->set_int64_value(1, test1), RETCODE_OK);
    EXPECT_EQ(2u, union_data->get_item_count());
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->clear_value(2));
        EXPECT_EQ(RETCODE_BAD_PARAMETER, union_data->clear_value(100));
    }
    EXPECT_EQ(RETCODE_OK, union_data->clear_value(1));
    EXPECT_EQ(2u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int64_value(test2, 1), RETCODE_OK);
    EXPECT_EQ(0, test2);
    EXPECT_EQ(RETCODE_OK, union_data->clear_value(0));
    EXPECT_EQ(1u, union_data->get_item_count());
    EXPECT_EQ(union_data->get_int32_value(discriminator_value, 0), RETCODE_OK);
    EXPECT_TRUE(0 != discriminator_value && 1 != discriminator_value && 4 != discriminator_value);
    {
        eprosima::fastdds::testing::ScopeLogs _("disable");
        EXPECT_EQ(union_data->get_int64_value(test2, 1), RETCODE_BAD_PARAMETER);
    }

    EXPECT_EQ(DynamicDataFactory::get_instance()->delete_data(union_data), RETCODE_OK);
}

TEST_F(DynamicTypesTests, DynamicType_KeyHash_standard_example_1)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("Foo");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("id");
    member_descriptor->is_key(true);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("x");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("y");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    EXPECT_EQ(RETCODE_OK, data->set_int32_value(0, 0x12345678));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(1, 10));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(1, 20));

    TypeSupport pubsubType {new DynamicPubSubType(struct_type)};
    eprosima::fastdds::rtps::InstanceHandle_t instance_handle;
    ASSERT_TRUE(pubsubType.compute_key(&data, instance_handle));

    const uint8_t expected_key_hash[] {
        0x12, 0x34, 0x56, 0x78,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    ASSERT_EQ(0, memcmp(expected_key_hash, &instance_handle, 16));

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(
        0, PARTICIPANT_QOS_DEFAULT);
    if (0 == struct_type->get_name().size())
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType, "test"));
    }
    else
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType));
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

TEST_F(DynamicTypesTests, DynamicType_KeyHash_standard_example_2)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("Foo");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->create_string_type(12)->build());
    member_descriptor->name("label");
    member_descriptor->is_key(true);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("id");
    member_descriptor->is_key(true);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("x");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("y");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    EXPECT_EQ(RETCODE_OK, data->set_string_value(0, "BLUE"));
    EXPECT_EQ(RETCODE_OK, data->set_int64_value(1, 0x123456789abcdef0ll));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(2, 10));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(3, 20));

    TypeSupport pubsubType {new DynamicPubSubType(struct_type)};
    eprosima::fastdds::rtps::InstanceHandle_t instance_handle;
    ASSERT_TRUE(pubsubType.compute_key(&data, instance_handle));

    const uint8_t expected_key_hash[] {
        0xf9, 0x1a, 0x59, 0xe3,
        0x2e, 0x45, 0x35, 0xd9,
        0xa6, 0x9c, 0xd5, 0xd9,
        0xf5, 0xb6, 0xe3, 0x6e
    };

    ASSERT_EQ(0, memcmp(expected_key_hash, &instance_handle, 16));

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(
        0, PARTICIPANT_QOS_DEFAULT);
    if (0 == struct_type->get_name().size())
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType, "test"));
    }
    else
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType));
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

TEST_F(DynamicTypesTests, DynamicType_KeyHash_standard_example_3)
{
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("Nested");
    type_descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    ASSERT_TRUE(builder);

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("m_long");
    member_descriptor->is_key(true);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("u");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("w");
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type nested_type {builder->build()};
    ASSERT_TRUE(nested_type);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
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
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(nested_type);
    member_descriptor->name("m_nested");
    member_descriptor->is_key(true);
    member_descriptor->id(30);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("x");
    member_descriptor->id(20);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("y");
    member_descriptor->id(10);
    EXPECT_EQ(builder->add_member(member_descriptor), RETCODE_OK);

    DynamicType::_ref_type struct_type {builder->build()};
    ASSERT_TRUE(struct_type);

    DynamicData::_ref_type data {DynamicDataFactory::get_instance()->create_data(struct_type)};
    ASSERT_TRUE(data);

    EXPECT_EQ(RETCODE_OK, data->set_string_value(40, "BLUE"));
    auto nested_data = data->loan_value(30);
    ASSERT_TRUE(nested_data);
    EXPECT_EQ(RETCODE_OK, nested_data->set_int32_value(0, 0x12345678l));
    EXPECT_EQ(RETCODE_OK, nested_data->set_int32_value(1, 10));
    EXPECT_EQ(RETCODE_OK, nested_data->set_int32_value(2, 20));
    EXPECT_EQ(RETCODE_OK, data->return_loaned_value(nested_data));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(20, 100));
    EXPECT_EQ(RETCODE_OK, data->set_int32_value(10, 200));

    TypeSupport pubsubType {new DynamicPubSubType(struct_type)};
    eprosima::fastdds::rtps::InstanceHandle_t instance_handle;
    ASSERT_TRUE(pubsubType.compute_key(&data, instance_handle));

    const uint8_t expected_key_hash[] {
        0x37, 0x4b, 0x96, 0xe2,
        0xe7, 0x27, 0x23, 0x7f,
        0x01, 0x6c, 0xc4, 0xce,
        0xbb, 0x6e, 0xb7, 0x1e
    };

    ASSERT_EQ(0, memcmp(expected_key_hash, &instance_handle, 16));

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(
        0, PARTICIPANT_QOS_DEFAULT);
    if (0 == struct_type->get_name().size())
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType, "test"));
    }
    else
    {
        EXPECT_EQ(RETCODE_OK, participant->register_type(pubsubType));
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_enum)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("EnumStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Enum
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ENUM);
    type_descriptor->name("MyEnum");
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("A");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("B");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("C");
    enum_builder->add_member(member_descriptor);

    // Struct EnumStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("EnumStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(enum_builder->build());
    member_descriptor->name("my_enum");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));

}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("AliasStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Enum
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ENUM);
    type_descriptor->name("MyEnum");
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("A");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("B");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("C");
    enum_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyAliasEnum");
    type_descriptor->base_type(enum_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    // Struct AliasStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("AliasStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(alias_builder->build());
    member_descriptor->name("my_alias");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias_with_alias)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("AliasAliasStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Enum
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ENUM);
    type_descriptor->name("MyEnum");
    DynamicTypeBuilder::_ref_type enum_builder {factory->create_type(type_descriptor)};

    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("A");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("B");
    enum_builder->add_member(member_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("C");
    enum_builder->add_member(member_descriptor);

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyAliasEnum");
    type_descriptor->base_type(enum_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyAliasAliasEnum");
    type_descriptor->base_type(alias_builder->build());
    DynamicTypeBuilder::_ref_type alias_alias_builder {factory->create_type(type_descriptor)};

    // Struct AliasStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("AliasAliasStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(alias_alias_builder->build());
    member_descriptor->name("my_alias_alias");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_boolean)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("BoolStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct BoolStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("BoolStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BOOLEAN));
    member_descriptor->name("my_bool");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_octet)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("OctetStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct OctetStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("OctetStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_BYTE));
    member_descriptor->name("my_octet");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_short)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ShortStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ShortStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ShortStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT16));
    member_descriptor->name("my_int16");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_long)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("LongStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("LongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT32));
    member_descriptor->name("my_int32");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_longlong)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("LongLongStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LongLongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("LongLongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_INT64));
    member_descriptor->name("my_int64");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_ushort)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("UShortStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct UShortStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("UShortStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_UINT16));
    member_descriptor->name("my_uint16");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_ulong)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ULongStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ULongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ULongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    member_descriptor->name("my_uint32");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_ulonglong)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ULongLongStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ULongLongStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ULongLongStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_UINT64));
    member_descriptor->name("my_uint64");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_float)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("FloatStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct FloatStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("FloatStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT32));
    member_descriptor->name("my_float32");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_double)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("DoubleStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct DoubleStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("DoubleStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT64));
    member_descriptor->name("my_float64");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_longdouble)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("LongDoubleStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LongDoubleStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("LongDoubleStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_FLOAT128));
    member_descriptor->name("my_float128");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_char)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("CharStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct CharStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("CharStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_CHAR8));
    member_descriptor->name("my_char");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_wchar)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("WCharStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct WCharStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("WCharStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->get_primitive_type(TK_CHAR16));
    member_descriptor->name("my_wchar");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_string)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("StringStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct StringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("StringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("my_string");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_wstring)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("WStringStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct WStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("WStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    member_descriptor->name("my_wstring");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}


TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_large_string)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("LargeStringStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LargeStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("LargeStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(41925)->build());
    member_descriptor->name("my_large_string");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_large_wstring)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("LargeWStringStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct LargeWStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("LargeWStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_wstring_type(41925)->build());
    member_descriptor->name("my_large_wstring");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_short_string)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ShortStringStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ShortStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ShortStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_string_type(15)->build());
    member_descriptor->name("my_short_string");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_short_wstring)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ShortWStringStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Struct ShortWStringStruct
    TypeDescriptor::_ref_type type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ShortWStringStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(factory->create_wstring_type(15)->build());
    member_descriptor->name("my_short_wstring");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias_of_string)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("StructAliasString",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Alias
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyAliasString");
    type_descriptor->base_type(factory->create_string_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    // Struct StructAliasString
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("StructAliasString");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(alias_builder->build());
    member_descriptor->name("my_alias_string");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_alias_of_wstring)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("StructAliasWString",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Alias
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyAliasWString");
    type_descriptor->base_type(factory->create_wstring_type(static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    // Struct StructAliasWString
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("StructAliasWString");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(alias_builder->build());
    member_descriptor->name("my_alias_wstring");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_array)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ArrayStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Array
    DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(factory->get_primitive_type(
                                                             TK_INT32), {2, 2, 2})};

    // Struct ArrayStruct
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ArrayStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(array_builder->build());
    member_descriptor->name("my_array");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_array_of_arrays)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ArrayArrayStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Typedef aka Alias
    DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(factory->get_primitive_type(
                                                             TK_INT32), {2, 2})};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyArray");
    type_descriptor->base_type(array_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type array_array_builder {factory->create_array_type(alias_builder->build(), {2, 2})};

    // Struct ArrayArrayStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ArrayArrayStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(array_array_builder->build());
    member_descriptor->name("my_array_array");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_array_struct_with_array_of_arrays)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("ArrayArrayArrayStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Typedef aka Alias
    DynamicTypeBuilder::_ref_type array_builder {factory->create_array_type(factory->get_primitive_type(
                                                             TK_INT32), {2, 2})};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("MyArray");
    type_descriptor->base_type(array_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type array_array_builder {factory->create_array_type(alias_builder->build(), {2, 2})};

    // Struct ArrayArrayStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ArrayArrayStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(array_array_builder->build());
    member_descriptor->name("my_array_array");
    builder->add_member(member_descriptor);

    DynamicTypeBuilder::_ref_type array_struct_builder {factory->create_array_type(builder->build(), {2, 2})};


    // Struct ArrayArrayArrayStruct
    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("ArrayArrayArrayStruct");
    builder = factory->create_type(type_descriptor);
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(array_struct_builder->build());
    member_descriptor->name("my_array_array_array");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_sequence)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("SequenceStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type sequence_builder {factory->create_sequence_type(factory->get_primitive_type(
                                                                TK_INT32), 2)};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("SequenceStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(sequence_builder->build());
    member_descriptor->name("my_sequence");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}


TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_sequence_of_sequences)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("SequenceSequenceStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type sequence_builder {factory->create_sequence_type(factory->get_primitive_type(
                                                                TK_INT32), 2)};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("my_sequence_sequence_inner");
    type_descriptor->base_type(sequence_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type sequence_sequence_builder {factory->create_sequence_type(alias_builder->build(), 2)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("SequenceSequenceStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(sequence_sequence_builder->build());
    member_descriptor->name("my_sequence_sequence");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_map)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("MapStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type map_builder {factory->create_map_type(factory->get_primitive_type(TK_INT32),
                                                       factory->get_primitive_type(TK_INT32), 7)};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("MapStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(map_builder->build());
    member_descriptor->name("my_map");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_map_of_maps)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("MapMapStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    DynamicTypeBuilder::_ref_type map_builder {factory->create_map_type(factory->get_primitive_type(TK_INT32),
                                                       factory->get_primitive_type(TK_INT32), 2)};
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_ALIAS);
    type_descriptor->name("my_map_map_inner");
    type_descriptor->base_type(map_builder->build());
    DynamicTypeBuilder::_ref_type alias_builder {factory->create_type(type_descriptor)};

    DynamicTypeBuilder::_ref_type map_map_builder {factory->create_map_type(factory->get_primitive_type(TK_INT32),
                                                           alias_builder->build(), 2)};

    type_descriptor = traits<TypeDescriptor>::make_shared();
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("MapMapStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->type(map_map_builder->build());
    member_descriptor->name("my_map_map");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_two_members)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("StructStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
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

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_struct)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("StructStructStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
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
    type_descriptor->kind(TK_STRUCTURE);
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

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_union)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("SimpleUnionStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_UNION);
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
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("SimpleUnionStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_builder->build());
    member_descriptor->name("my_union");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_struct_with_union_with_union)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("UnionUnionStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_UNION);
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
    type_descriptor->kind(TK_UNION);
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
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("UnionUnionStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_union_builder->build());
    member_descriptor->name("my_union");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_WCharUnionStruct_test)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("WCharUnionStruct",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_UNION);
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
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name("WCharUnionStruct");
    DynamicTypeBuilder::_ref_type builder {factory->create_type(type_descriptor)};
    member_descriptor = traits<MemberDescriptor>::make_shared();
    member_descriptor->type(union_builder->build());
    member_descriptor->name("my_union");
    builder->add_member(member_descriptor);

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_Bitset_test)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("MyBitSet",
            xml_type_builder));

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

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
}

TEST_F(DynamicTypesTests, DynamicType_XML_Bitmask_test)
{
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->load_XML_profiles_file(DynamicTypesTests::config_file()));

    DynamicTypeBuilder::_ref_type xml_type_builder;
    ASSERT_EQ(RETCODE_OK,
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("MyBitMask",
            xml_type_builder));

    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    // Bitmask
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_BITMASK);
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

    ASSERT_TRUE(xml_type_builder->build()->equals(builder->build()));
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
    Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
