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
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1(type_id);
#if !defined(NDEBUG)
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias",
            type_object, type_ids));
#endif // if !defined(NDEBUG)
    complete_alias_type.body().common().related_type(type_id);
    type_object.alias_type(complete_alias_type);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("", type_object,
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias",
            type_object, type_ids));
    complete_alias_type.header().detail().type_name("other_name");
    type_object.alias_type(complete_alias_type);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("alias",
            type_object, type_ids));
}

// Test TypeObjectRegistry::register_type_identifier
TEST(TypeObjectRegistryTests, register_type_identifier)
{
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1().equivalence_hash(EquivalenceHash());
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("hash_type_id",
            type_ids));
    StringSTypeDefn small_string;
    small_string.bound(10);
    type_ids.type_identifier1().string_sdefn(small_string);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("",
            type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("string_type_id",
            type_ids));
    type_ids.type_identifier1().string_sdefn().bound(5);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("string_type_id",
            type_ids));
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
    TypeIdentifier alias_type_id;
    alias_type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    complete_alias_type.body().common().related_type(alias_type_id);
    TypeObject type_object;
    CompleteTypeObject complete_type_object;
    complete_type_object.alias_type(complete_alias_type);
    type_object.complete(complete_type_object);
    TypeIdentifierPair type_ids;
    type_ids.type_identifier1(alias_type_id);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object("test_name",
            type_object.complete(), type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects("test_name",
            type_objects));
    EXPECT_EQ(type_objects.complete_type_object, type_object);
}

// Test TypeObjectRegistry::get_type_identifiers
TEST(TypeObjectRegistryTests, get_type_identifiers)
{
    TypeIdentifierPair out_type_ids, type_ids;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("", type_ids));
    TypeIdentifier none_type_id;
    TypeIdentifier type_id;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_NO_DATA,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("test_type",
            type_ids));

    // Register fully descriptive TypeIdentifier
    StringSTypeDefn small_string = TypeObjectUtils::build_string_s_type_defn(32);
    type_id.string_sdefn(small_string);
    out_type_ids.type_identifier1(type_id);
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_identifier("test_type",
            out_type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("test_type",
            type_ids));
    EXPECT_EQ(type_ids.type_identifier1(), type_id);
    EXPECT_EQ(type_ids.type_identifier2(), none_type_id);
    EXPECT_EQ(type_ids.type_identifier1(), out_type_ids.type_identifier1());
    EXPECT_EQ(type_ids.type_identifier2(), out_type_ids.type_identifier2());

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
            type_object, out_type_ids));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers("alias", type_ids));
    EXPECT_TRUE((type_ids.type_identifier1()._d() == EK_MINIMAL && type_ids.type_identifier2()._d() == EK_COMPLETE) ||
            (type_ids.type_identifier1()._d() == EK_COMPLETE && type_ids.type_identifier2()._d() == EK_MINIMAL));
}

// Test TypeObjectRegistry::get_type_identifiers for primitive types
TEST(TypeObjectRegistryTests, get_type_identifiers_primitive_types)
{
    TypeIdentifierPair type_ids;
    TypeIdentifier none_type_id;
    TypeIdentifier type_id;
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
    type_id._d(TK_UINT8);
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
