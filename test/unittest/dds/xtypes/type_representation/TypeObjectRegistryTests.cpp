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

#include <limits>
#include <string>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TestTypeObjectRegistry : public TypeObjectRegistry
{

public:

    using TypeObjectRegistry::set_annotation_parameter_value;
};

// Registry counting the TypeObject lookups done while resolving dependencies
class LookupCountingTypeObjectRegistry : public TypeObjectRegistry
{

public:

    ReturnCode_t get_type_object(
            const TypeIdentifier& type_identifier,
            TypeObject& type_object) override
    {
        // Cut the traversal once the limit is exceeded so that an exponential resolution finishes quickly
        if (++lookups > max_lookups)
        {
            return RETCODE_NO_DATA;
        }
        return TypeObjectRegistry::get_type_object(type_identifier, type_object);
    }

    size_t lookups {0};
    size_t max_lookups {std::numeric_limits<size_t>::max()};
};

// Direct hash TypeIdentifier which is not registered
static TypeIdentifier unknown_hash_type_identifier()
{
    EquivalenceHash hash;
    hash.fill(0xAA);
    TypeIdentifier type_id;
    type_id.equivalence_hash(hash);
    type_id._d(EK_MINIMAL);
    return type_id;
}

static TypeObject minimal_struct_type_object(
        const std::vector<TypeIdentifier>& member_types,
        const TypeIdentifier& base_type = TypeIdentifier())
{
    MinimalStructType struct_type;
    struct_type.header().base_type(base_type);
    MemberId member_id {0};
    for (const TypeIdentifier& member_type : member_types)
    {
        MinimalStructMember member;
        member.common().member_id(member_id++);
        member.common().member_type_id(member_type);
        struct_type.member_seq().push_back(member);
    }
    MinimalTypeObject minimal;
    minimal.struct_type(struct_type);
    TypeObject type_object;
    type_object.minimal(minimal);
    return type_object;
}

static TypeObject minimal_alias_type_object(
        const TypeIdentifier& related_type)
{
    MinimalAliasType alias_type;
    alias_type.body().common().related_type(related_type);
    MinimalTypeObject minimal;
    minimal.alias_type(alias_type);
    TypeObject type_object;
    type_object.minimal(minimal);
    return type_object;
}

static TypeObject minimal_sequence_type_object(
        const TypeIdentifier& element_type)
{
    MinimalSequenceType sequence_type;
    sequence_type.element().common().type(element_type);
    MinimalTypeObject minimal;
    minimal.sequence_type(sequence_type);
    TypeObject type_object;
    type_object.minimal(minimal);
    return type_object;
}

// Register the TypeObject as the TypeLookup service does with the remote ones (no dependency validation)
static TypeIdentifier register_remote_type_object(
        TypeObjectRegistry& registry,
        const TypeObject& type_object)
{
    TypeIdentifierPair type_ids;
    EXPECT_EQ(RETCODE_OK, registry.register_type_object(type_object, type_ids, false));
    return type_ids.type_identifier1();
}

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

// Test TypeObjectRegistry::register_type_object
TEST(TypeObjectRegistryTests, register_type_object_no_name)
{
    TypeIdentifier type_id;
    type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    CompleteTypeObject type_object;
    type_object.alias_type(complete_alias_type);
    complete_alias_type.body().common().related_type(type_id);
    type_object.alias_type(complete_alias_type);

    TypeIdentifierPair type_ids;
    TypeObject t;
    t.complete(type_object);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(t, type_ids));
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

// Test TypeObjectRegistry::get_complete_type_object
TEST(TypeObjectRegistryTests, get_complete_type_object)
{
    CompleteTypeObject type_object;
    TypeIdentifierPair type_ids;
    auto& registry = DomainParticipantFactory::get_instance()->type_object_registry();

    EXPECT_EQ(RETCODE_BAD_PARAMETER, registry.get_complete_type_object(type_ids, type_object));

    EXPECT_EQ(RETCODE_OK, registry.get_type_identifiers(boolean_type_name, type_ids));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, registry.get_complete_type_object(type_ids, type_object));

    TypeIdentifier alias_type_id;
    alias_type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    complete_alias_type.body().common().related_type(alias_type_id);
    CompleteTypeObject input_type_object;
    input_type_object.alias_type(complete_alias_type);
    TypeObject reg_type_object;
    reg_type_object.complete(input_type_object);

    TypeIdentifierPair reg_type_object_ids;
    EXPECT_EQ(RETCODE_OK, registry.register_type_object(reg_type_object, reg_type_object_ids));
    EXPECT_EQ(RETCODE_OK, registry.get_complete_type_object(reg_type_object_ids, type_object));
    EXPECT_EQ(type_object, input_type_object);
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

// Test TypeObjectRegistry::get_type_information
TEST(TypeObjectRegistryTests, get_type_information)
{
    auto& registry = DomainParticipantFactory::get_instance()->type_object_registry();

    TypeIdentifier type_id;
    type_id._d(TK_BYTE);
    CompleteAliasType complete_alias_type;
    complete_alias_type.header().detail().type_name("alias_name");
    CompleteTypeObject type_object;
    type_object.alias_type(complete_alias_type);
    complete_alias_type.body().common().related_type(type_id);
    type_object.alias_type(complete_alias_type);

    TypeIdentifierPair type_ids;
    TypeObject t;
    t.complete(type_object);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK,
            DomainParticipantFactory::get_instance()->type_object_registry().register_type_object(t, type_ids));

    TypeInformation type_info;
    EXPECT_EQ(RETCODE_OK, registry.get_type_information(type_ids, type_info, false));
}

/**
 * @test Regression test for #25766.
 * A registered type referencing an unregistered TypeIdentifier must make get_type_dependencies
 * return RETCODE_NO_DATA instead of throwing.
 */
TEST(TypeObjectRegistryTests, get_type_dependencies_unknown_dependency)
{
    TypeObjectRegistry registry;
    const TypeIdentifier unknown_id {unknown_hash_type_identifier()};
    TypeIdentifier int32_id;
    int32_id._d(TK_INT32);

    const TypeIdentifier known_struct_id {
        register_remote_type_object(registry, minimal_struct_type_object({int32_id}))};
    const TypeIdentifier unknown_member_struct_id {
        register_remote_type_object(registry, minimal_struct_type_object({unknown_id}))};

    // The unknown TypeIdentifier is referenced from different positions
    const std::vector<TypeIdentifier> type_ids {
        unknown_member_struct_id,
        register_remote_type_object(registry, minimal_struct_type_object({int32_id}, unknown_id)),
        register_remote_type_object(registry, minimal_struct_type_object({known_struct_id, unknown_id})),
        register_remote_type_object(registry, minimal_alias_type_object(unknown_id)),
        register_remote_type_object(registry, minimal_sequence_type_object(unknown_id)),
        // Nested: only an indirect dependency is unknown
        register_remote_type_object(registry, minimal_struct_type_object({unknown_member_struct_id}))
    };

    for (size_t i = 0; i < type_ids.size(); ++i)
    {
        SCOPED_TRACE("type_ids[" + std::to_string(i) + "]");
        std::unordered_set<TypeIdentfierWithSize> type_dependencies;
        ReturnCode_t ret {RETCODE_OK};
        EXPECT_NO_THROW(ret = registry.get_type_dependencies({type_ids[i]}, type_dependencies));
        EXPECT_EQ(RETCODE_NO_DATA, ret);
    }
}

/**
 * @test Regression test for #25766.
 * Dependencies shared by several members must be resolved once, so the work grows linearly with the
 * number of distinct types instead of exponentially.
 */
TEST(TypeObjectRegistryTests, get_type_dependencies_shared_dependencies)
{
    constexpr size_t depth {30};
    LookupCountingTypeObjectRegistry registry;
    TypeIdentifier int32_id;
    int32_id._d(TK_INT32);

    // S_1 has primitive members and S_i has two members of type S_(i-1)
    TypeIdentifier head_id {register_remote_type_object(registry, minimal_struct_type_object({int32_id, int32_id}))};
    for (size_t i = 2; i <= depth; ++i)
    {
        head_id = register_remote_type_object(registry, minimal_struct_type_object({head_id, head_id}));
    }

    // Without memoization 2^(depth-1) lookups would be required
    registry.lookups = 0;
    registry.max_lookups = 4 * depth;
    std::unordered_set<TypeIdentfierWithSize> type_dependencies;
    EXPECT_EQ(RETCODE_OK, registry.get_type_dependencies({head_id}, type_dependencies));
    EXPECT_EQ(depth - 1, type_dependencies.size());
    EXPECT_LE(registry.lookups, registry.max_lookups);
}

/**
 * @test Regression test for #25766.
 * Resolving a long chain of dependencies must not exhaust the stack.
 */
TEST(TypeObjectRegistryTests, get_type_dependencies_long_dependency_chain)
{
    constexpr size_t chain_length {100000};
    TypeObjectRegistry registry;

    // A_1 aliases a primitive type and A_i aliases A_(i-1)
    TypeIdentifier head_id;
    head_id._d(TK_INT32);
    for (size_t i = 0; i < chain_length; ++i)
    {
        head_id = register_remote_type_object(registry, minimal_alias_type_object(head_id));
    }

    std::unordered_set<TypeIdentfierWithSize> type_dependencies;
    EXPECT_EQ(RETCODE_OK, registry.get_type_dependencies({head_id}, type_dependencies));
    EXPECT_EQ(chain_length - 1, type_dependencies.size());
}

// (MacOS) Regression test for clang/libc++ builds: empty char8 annotation
// parameters must not route through the string overload
// and must produce a null character default value instead
TEST(TypeObjectRegistryTests, set_annotation_parameter_value_empty_char8)
{
    TestTypeObjectRegistry registry;
    AnnotationParameterValue param_value;
    // Build the primitive char8 type explicitly so the conversion logic is exercised on the
    // single-character annotation path instead of the string one
    auto char8_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8);

    ASSERT_NE(nullptr, char8_type);
    // An empty textual value for a char8 annotation must still succeed and yield the default
    // null character rather than being treated as a string payload.
    EXPECT_EQ(RETCODE_OK, registry.set_annotation_parameter_value(char8_type, "", param_value));
    EXPECT_EQ(TK_CHAR8, param_value._d());
    EXPECT_EQ('\0', param_value.char_value());
}

// (MacOS) Regression test for clang/libc++ builds: non empty char8 annotation
TEST(TypeObjectRegistryTests, set_annotation_parameter_value_non_empty_char8_uses_first_character)
{
    TestTypeObjectRegistry registry;
    AnnotationParameterValue param_value;
    // Use the same primitive char8 path to verify how textual input is narrowed into a single
    // annotation character
    auto char8_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_CHAR8);

    ASSERT_NE(nullptr, char8_type);
    // For char8 annotations only the first character is significant, even if the provided text
    // contains more bytes
    EXPECT_EQ(RETCODE_OK, registry.set_annotation_parameter_value(char8_type, "FastDDS", param_value));
    EXPECT_EQ(TK_CHAR8, param_value._d());
    EXPECT_EQ('F', param_value.char_value());
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
