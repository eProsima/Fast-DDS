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
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

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

} // xtypes1_3
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
