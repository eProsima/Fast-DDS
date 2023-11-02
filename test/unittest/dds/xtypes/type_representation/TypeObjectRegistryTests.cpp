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
#include <fastdds/dds/xtypes/type_representation/TypeObject.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

TEST(TypeObjectRegistryTests, register_type_identifier)
{
    TypeIdentifier type_id;
    type_id.equivalence_hash(EquivalenceHash());
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("hash_type_id",
            type_id));
    StringSTypeDefn small_string;
    small_string.bound(10);
    type_id.string_sdefn(small_string);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("",
            type_id));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("string_type_id",
            type_id));
    type_id.string_sdefn().bound(5);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
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

// Test TypeObjectRegistry::get_type_objects
TEST(TypeObjectRegistryTests, get_type_objects)
{
    TypeObjectPair type_objects;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects("", type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_NO_DATA,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects("test_name",
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(boolean_type_name,
            type_objects));

    // Builtin annotations
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(id_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(autoid_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(optional_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(position_annotation_name,
            type_objects));
    /* TODO: pending implementation
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(value_annotation_name,
            type_objects));
     */
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                extensibility_annotation_name,
                type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(final_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(appendable_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(mutable_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                must_understand_annotation_name, type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                default_literal_annotation_name, type_objects));
    /* TODO: pending implementation
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(default_annotation_name,
            type_objects));
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(range_annotation_name,
            type_objects));
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(min_annotation_name,
            type_objects));
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(max_annotation_name,
            type_objects));
     */
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(unit_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(bit_bound_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(external_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(nested_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(verbatim_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(service_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(oneway_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(ami_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(hashid_annotation_name,
            type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                default_nested_annotation_name, type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                ignore_literal_names_annotation_name, type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                try_construct_annotation_name,
                type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                non_serialized_annotation_name, type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
                data_representation_annotation_name, type_objects));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(topic_annotation_name,
            type_objects));

    // User TypeObject
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_NO_DATA,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects("alias", type_objects));
    TypeIdentifier alias_type_id;
    alias_type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    complete_alias_type.body().common().related_type(alias_type_id);
    CompleteTypeObject type_object;
    type_object.alias_type(complete_alias_type);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias",
            type_object));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects("alias", type_objects));
    EXPECT_EQ(type_objects.complete_type_object, type_object);
}

// Test TypeObjectRegistry::get_type_identifiers
TEST(TypeObjectRegistryTests, get_type_identifiers)
{
    TypeIdentifierPair type_ids;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("", type_ids));
    TypeIdentifier none_type_id;
    TypeIdentifier type_id;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_NO_DATA,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("test_type",
            type_ids));

    // Check that builtin TypeIdentifiers are found
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(boolean_type_name,
            type_ids));
    type_id._d(TK_BOOLEAN);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(byte_type_name,
            type_ids));
    type_id._d(TK_BYTE);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int16_type_name,
            type_ids));
    type_id._d(TK_INT16);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int32_type_name,
            type_ids));
    type_id._d(TK_INT32);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int64_type_name,
            type_ids));
    type_id._d(TK_INT64);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint16_type_name,
            type_ids));
    type_id._d(TK_UINT16);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint32_type_name,
            type_ids));
    type_id._d(TK_UINT32);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint64_type_name,
            type_ids));
    type_id._d(TK_UINT64);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(float32_type_name,
            type_ids));
    type_id._d(TK_FLOAT32);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(float64_type_name,
            type_ids));
    type_id._d(TK_FLOAT64);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(float128_type_name,
            type_ids));
    type_id._d(TK_FLOAT128);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(int8_type_name,
            type_ids));
    type_id._d(TK_INT8);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(uint8_type_name,
            type_ids));
    type_id._d(TK_INT8);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(char8_type_name,
            type_ids));
    type_id._d(TK_CHAR8);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(char16_type_name,
            type_ids));
    type_id._d(TK_CHAR16);
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);

    // Builtin annotations: returned TypeIdentifiers are checked in Fast DDS-Gen CI.
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(id_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(autoid_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                optional_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                position_annotation_name, type_ids));
    /* TODO: pending implementation
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(value_annotation_name,
            type_ids));
     */
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                extensibility_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(final_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                appendable_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                mutable_annotation_name,
                type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(key_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                must_understand_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                default_literal_annotation_name, type_ids));
    /* TODO: pending implementation
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(default_annotation_name,
            type_ids));
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(range_annotation_name,
            type_ids));
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(min_annotation_name,
            type_ids));
        EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(max_annotation_name,
            type_ids));
     */
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(unit_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                bit_bound_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                external_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(nested_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                verbatim_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                service_annotation_name,
                type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(oneway_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(ami_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(hashid_annotation_name,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                default_nested_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                ignore_literal_names_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                try_construct_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                non_serialized_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                data_representation_annotation_name, type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(topic_annotation_name,
            type_ids));

    // Register fully descriptive TypeIdentifier
    StringSTypeDefn small_string = TypeObjectUtils::build_string_s_type_defn(32);
    type_id.string_sdefn(small_string);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("test_name",
            type_id));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("test_name",
            type_ids));
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);

    // Register hash TypeIdentifier
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_NO_DATA,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("alias", type_ids));
    TypeIdentifier alias_type_id;
    alias_type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    complete_alias_type.body().common().related_type(alias_type_id);
    CompleteTypeObject type_object;
    type_object.alias_type(complete_alias_type);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias",
            type_object));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
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
