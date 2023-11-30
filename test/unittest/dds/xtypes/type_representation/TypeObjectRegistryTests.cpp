// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file
 * This file contains unit tests related to the TypeObjectRegistry API.
 */

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

// Test TypeObjectRegistry::register_type_object
TEST(TypeObjectRegistryTests, register_type_object)
{
    TypeIdentifier type_id;
    type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    CompleteTypeObject type_object;
    type_object.alias_type(complete_alias_type);
#if !defined(NDEBUG)
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias", type_object));
#endif
    complete_alias_type.body().common().related_type(type_id);
    type_object.alias_type(complete_alias_type);
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("", type_object));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias", type_object));
    complete_alias_type.header().detail().type_name("other_name");
    type_object.alias_type(complete_alias_type);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias", type_object));
}

// Test TypeObjectRegistry::register_type_identifier
TEST(TypeObjectRegistryTests, register_type_identifier)
{
    TypeIdentifier type_id;
    type_id.equivalence_hash(EquivalenceHash());
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("hash_type_id",
            type_id));
    StringSTypeDefn small_string;
    small_string.bound(10);
    type_id.string_sdefn(small_string);
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("",
            type_id));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("string_type_id",
            type_id));
    type_id.string_sdefn().bound(5);
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("string_type_id",
            type_id));
}

// Test TypeObjectRegistry::get_type_identifiers
TEST(TypeObjectRegistryTests, get_type_identifiers)
{
    TypeIdentifierPair type_ids;
    TypeIdentifier none_type_id;
    TypeIdentifier type_id;
    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("test_type", type_ids));

    // Check that builtin TypeIdentifiers are found
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(boolean_type_name,
        type_ids));
    type_id._d(TK_BOOLEAN);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(byte_type_name,
        type_ids));
    type_id._d(TK_BYTE);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int16_type_name,
        type_ids));
    type_id._d(TK_INT16);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int32_type_name,
        type_ids));
    type_id._d(TK_INT32);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int64_type_name,
        type_ids));
    type_id._d(TK_INT64);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint16_type_name,
        type_ids));
    type_id._d(TK_UINT16);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint32_type_name,
        type_ids));
    type_id._d(TK_UINT32);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint64_type_name,
        type_ids));
    type_id._d(TK_UINT64);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(float32_type_name,
        type_ids));
    type_id._d(TK_FLOAT32);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(float64_type_name,
        type_ids));
    type_id._d(TK_FLOAT64);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(float128_type_name,
        type_ids));
    type_id._d(TK_FLOAT128);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int8_type_name,
        type_ids));
    type_id._d(TK_INT8);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint8_type_name,
        type_ids));
    type_id._d(TK_INT8);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(char8_type_name,
        type_ids));
    type_id._d(TK_CHAR8);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(char16_type_name,
        type_ids));
    type_id._d(TK_CHAR16);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);

    // Builtin annotations: returned TypeIdentifiers are checked in Fast DDS-Gen CI.
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(id_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(autoid_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            optional_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            position_annotation_name, type_ids));
/* TODO: pending implementation
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(value_annotation_name,
        type_ids));
*/
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            extensibility_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(final_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            appendable_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(mutable_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(key_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            must_understand_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            default_literal_annotation_name, type_ids));
/* TODO: pending implementation
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(default_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(range_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(min_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(max_annotation_name,
        type_ids));
*/
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(unit_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            bit_bound_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            external_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(nested_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            verbatim_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(service_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(oneway_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(ami_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(hashid_annotation_name,
        type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            default_nested_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            ignore_literal_names_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            try_construct_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            non_serialized_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
            data_representation_annotation_name, type_ids));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(topic_annotation_name,
        type_ids));

    // Register fully descriptive TypeIdentifier
    StringSTypeDefn small_string = TypeObjectUtils::build_string_s_type_defn(32);
    type_id.string_sdefn(small_string);
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("test_name",
        type_id));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("test_name", type_ids));
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);

    // Register hash TypeIdentifier
    EXPECT_EQ(ReturnCode_t::RETCODE_NO_DATA,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("alias", type_ids));
    TypeIdentifier alias_type_id;
    alias_type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    complete_alias_type.body().common().related_type(type_id);
    CompleteTypeObject type_object;
    type_object.alias_type(complete_alias_type);
    ASSERT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias", type_object));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK,
        DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("alias", type_ids));
    EXPECT_TRUE((type_ids.type_identifier1()._d() == EK_MINIMAL && type_ids.type_identifier2()._d() == EK_COMPLETE) ||
        (type_ids.type_identifier1()._d() == EK_COMPLETE && type_ids.type_identifier2()._d() == EK_MINIMAL));
}

} // xtypes
} // dds
} // fastdds
} // eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
